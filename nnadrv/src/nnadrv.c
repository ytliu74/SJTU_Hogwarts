#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/gfp.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/time.h> 
#include <linux/spinlock.h> 
#include <linux/platform_device.h>
#include <linux/completion.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include <asm/neon.h>
#include <asm/cacheflush.h>

#include "nnadrv.h"

/*-------------------------------------------------------------------------------------*/
/* Macros */

// Registers
#define NNA_CRM_BASE 0xFF200080
#define NNA_CRM_SIZE 0x0001000

// Global Control Register
#define NNA_GCR_OFST       0x0000
#define NNA_GCR_START     (1 << 0)
#define NNA_GCR_DONE      (1 << 1)
#define NNA_GCR_IDLE      (1 << 2)
#define NNA_GCR_READY     (1 << 3)
#define NNA_GCR_RESET     (1 << 7)

// Global Interrupt Enable Register (Read/Write)
#define NNA_GIE_OFFSET    0x0004
#define NNA_GIE_ENABLE    (1 << 0)

// Interrupt Enable Register (Read/Write)
#define NNA_IER_OFST      0x0008
#define NNA_IER_DONE      (1 << 0)
#define NNA_IER_READY     (1 << 1)

// Ip start Register.
#define NNA_START_OFST     0x14

// Input Base Regisger
#define NNA_IN_OFST      0x00

// Kernel Base Regisger
#define NNA_WEIGHT_OFST  0x0c

// Output Base Regisger
#define NNA_OUT_OFST     0x04

// Scale Regisger
#define NNA_SCALE_OFST   0x08

// Param Regisger
#define NNA_PARAM_OFST   0x10

// Input Height/width Regisger
// [9:0]: input width, [25:16]: input height
#define NNA_HWR_OFST      0x0038
#define NNA_HWR_H_SHIFT   0
#define NNA_HWR_W_SHIFT   16

// Computation Cycle Regisger: 1 / 2 / 4 / 8
#define NNA_CCR_OFST      0x003C
#define NNA_CYC_VALUE     8100

// Data memory offset
#define NNA_IDM_SIZE 0x00800000
#define NNA_KDM_SIZE 0x00800000
#define NNA_ODM_SIZE 0x00800000
#define NNA_BSDM_SIZE 0x3000 
#define NNA_PDM_SIZE 0x1000

// Wait counter
#define NNA_COUNTER 5000000

/*-------------------------------------------------------------------------------------*/

struct nna_pdata {
    /* character device */
    dev_t devno;
    struct cdev cdev;
    struct class* class;
    struct device* device;

    // DMA related
    struct dma_chan * chan;
    struct completion done;

    // Control register address, CRB
    void __iomem* cra;

    // Data memory block for IDM, KDM, ODM, SCALE, PARAM
    struct nna_mblk_s imblk[2];
    struct nna_mblk_s kmblk[2];
    struct nna_mblk_s omblk[3];
    struct nna_mblk_s pmblk[2];
    struct nna_mblk_s bsblk[2];
    struct nna_mblk_s omblk_extra[10];

    int pid; /* User process ID */
    struct task_struct *task;
    spinlock_t lock;
    struct mutex mutex; /* Access mutex */
};

// Locals
static struct DeviceGraphNode next_node, parent_node, cur_node;
static struct nna_conv_s conv_param, conv_next_param;
static struct nna_pdata *objId = NULL;

/*-------------------------------------------------------------------------------------*/

void remove_pad(int8_t* din, int8_t* dout, 
        int ch, int w, int h, int pad) {
    int r, c, w_2pad, w_stride;
    int8_t* dout_int8 = dout; 
    int8_t* din_int8;
    w_2pad = w + 2 * pad;
    w_stride = w_2pad * (h + 2 * pad);

    for(c = 0; c < ch; c++) {
        din_int8 = din + w_2pad + c * w_stride;
        for(r = 0; r < h; r++) {
            memcpy(dout_int8, din_int8 + pad, w);
            din_int8 += w_2pad;
            dout_int8 += w;
        }
    }
}

// Note: This function assumes dout to be zero space.
// Params: din - comes from fpga without pad.
//         dout - zero ddr space.
void add_pad(int8_t* din, int8_t* dout, 
        int ch, int w, int h, int pad) {
    int r, c, w_2pad, w_stride;
    int8_t* dout_int8; 
    int8_t* din_int8;
    ch = (ch - 1) / 16 + 1;
    w_2pad = (w + 2 * pad) << 4;
    w = w << 4;
    w_stride = w_2pad * (h + 2 * pad);
    pad = pad << 4;
    din_int8 = (int8_t*)din;
    for(c = 0; c < ch; c++) {
        dout_int8 = dout + w_2pad + c * w_stride; 
        for(r = 0; r < h; r++) {
            memcpy(dout_int8 + pad, din_int8, w);
            din_int8 += w;
            dout_int8 += w_2pad;
        }
    }
}

static inline uint32_t nna_read(struct nna_pdata* pdata, int offset)
{
    uint8_t* addr = (uint8_t*)pdata->cra + offset;

    return readl(addr);
}

static inline void nna_write(struct nna_pdata* pdata, int offset, uint32_t data)
{
    uint8_t* addr = (uint8_t*)pdata->cra + offset;

    writel(data, addr);
}

// DMA callback routine
static void nna_dma_callback( void* arg )
{
    struct nna_pdata *pdata = (struct nna_pdata *)arg;

    complete(&pdata->done);
}

static int nna_dma_init( struct nna_pdata* pdata )
{
    dma_cap_mask_t mask;

    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY, mask);
    pdata->chan = dma_request_channel(mask, 0, NULL);
    if (!pdata->chan) {
        return -1;
    }

    return 0;
}

static void nna_dma_cleanup( struct nna_pdata* pdata )
{
    if (pdata->chan) {
        dma_release_channel(pdata->chan);
        pdata->chan = NULL;
    }
}

static int nna_dma_copy( struct nna_pdata* pdata, unsigned long dst, unsigned long src, size_t len )
{
    dma_cookie_t cookie;
    enum dma_ctrl_flags flags;
    struct dma_device *pdev;
    struct dma_async_tx_descriptor *tx;

    if (!pdata->chan) {
        pr_info("^^^ Invalid DMA channel\n");
        return -1;
    }
    pdev = pdata->chan->device;
    flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    tx = pdev->device_prep_dma_memcpy(pdata->chan, dst, src, len, flags);
    if (!tx) {
        pr_info("^^^ Failed to prepare memcpy\n");
        return -1;
    }
    init_completion(&pdata->done);
    tx->callback = nna_dma_callback;
    tx->callback_param = pdata;
    cookie = tx->tx_submit(tx);
    if (dma_submit_error(cookie)) {
        pr_info("^^^ Failed to do tx_submit\n");
        return -1;
    }
    dma_async_issue_pending(pdata->chan);
    wait_for_completion(&pdata->done);

    return 0;
}

/*-------------------------------------------------------------------------------------*/

// INT8[N,Ci,Hi,Wi] * INT8[Co,Ci,Hk,Wk] = INT32[N,Ci,Ho,Wo]
static int nna_conv2d(struct nna_pdata* pdata, struct nna_conv_s *conv)
{
    int i, il, kl, ol, sl, pl;

    // Get data from user-land
    il = conv->extent_in_size;
    if (il>pdata->imblk[0].size) {
        pr_err("^^^ NNA IDM size invalid\n");
        return -EINVAL;
    }
    kl =conv->extent_weight_size; 
    if (kl>pdata->kmblk[0].size) {
        pr_err("^^^ NNA KDM size invalid\n");
        return -EINVAL;
    }
    ol = conv->extent_out_size;
    if (ol>pdata->omblk[0].size) {
        pr_err("^^^ NNA ODM size invalid\n");
        return -EINVAL;
    }
    sl = conv->scale_size;
    if (sl>pdata->bsblk[0].size) {
        pr_err("^^^ NNA  BSDM size invalid\n");
        return -EINVAL;
    }
    pl = conv->param_size;
    if (pl>pdata->pmblk[0].size) {
        pr_err("^^^ NNA  PDM size invalid\n");
        return -EINVAL;
    }
    if (copy_from_user(pdata->bsblk[0].addr, conv->ba, sl)) {
        pr_err("^^^ NNA Copy BSDM failed\n");
        return -EFAULT;
    }
    if (copy_from_user(pdata->imblk[0].addr, conv->ia, il)) {
        pr_err("^^^ NNA Copy IDM failed\n");
        return -EFAULT;
    }
    if (copy_from_user(pdata->kmblk[0].addr, conv->ka, kl)) {
        pr_err("^^^ NNA Copy KDM failed\n");
        return -EFAULT;
    }
    if (copy_from_user(pdata->pmblk[0].addr, conv->ip, pl)) {
        pr_err("^^^ NNA Copy PDM failed\n");
        return -EFAULT;
    }

    // Configure interface addr.
    nna_write(pdata, NNA_IN_OFST, pdata->imblk[0].phys);
    nna_write(pdata, NNA_WEIGHT_OFST, pdata->kmblk[0].phys);
    nna_write(pdata, NNA_OUT_OFST, pdata->omblk[0].phys);
    nna_write(pdata, NNA_SCALE_OFST, pdata->bsblk[0].phys);
    nna_write(pdata, NNA_PARAM_OFST, pdata->pmblk[0].phys);

    // Call ip and wait for done.
    nna_write(pdata, NNA_START_OFST, 1);
    i = 0;
    while (i<NNA_COUNTER) {
        if (nna_read(pdata, NNA_START_OFST) != 1)
            break;
        udelay(1);
        i ++;
    }
    if (i >= NNA_COUNTER) {
        pr_err("^^^ NNA Conv2d failed\n");
        return -ETIMEDOUT;
    }
    // Copy data to user-land
    if (copy_to_user(conv->oa, pdata->omblk[0].addr, ol)) {
        pr_err("^^^ NNA Copy ODM failed\n");
        return -EFAULT;
    }
    return 0;
}

// In subgraph conv compute mode, only input and output node 
// need to copy data with driver.
static int nna_conv2d_subgraph(struct nna_pdata* pdata, struct DeviceGraphNode *subgraph) {
    int i, il, kl, ol, sl, pl;
    int flag = 0;
    int pad, c, h, w;
    int out_flag = 0;
    int buff_flag = 0;

    if (copy_from_user(&conv_param, subgraph->device_param_, sizeof(conv_param))) {
        pr_err("^^^ NNA Copy subgraph->device_param_ failed\n");
        return -EFAULT;
    }

    // 1) Copy input node's param to ddr.
    il = conv_param.extent_in_size;
    kl = conv_param.extent_weight_size; 
    ol = conv_param.extent_out_size;
    sl = conv_param.scale_size;
    pl = conv_param.param_size;

    if (copy_from_user(pdata->bsblk[0].addr, conv_param.ba, sl)) {
        pr_err("^^^ NNA Copy BSDM failed\n");
        return -EFAULT;
    }
    if (copy_from_user(pdata->imblk[0].addr, conv_param.ia, il)) {
        pr_err("^^^ NNA Copy IDM failed\n");
        return -EFAULT;
    }
    if (copy_from_user(pdata->kmblk[0].addr, conv_param.ka, kl)) {
        pr_err("^^^ NNA Copy KDM failed\n");
        return -EFAULT;
    }
    if (copy_from_user(pdata->pmblk[0].addr, conv_param.ip, pl)) {
        pr_err("^^^ NNA Copy PDM failed\n");
        return -EFAULT;
    }

    // 2) Execute node in double buffer manner.
    // Configure interface addr.
    nna_write(pdata, NNA_IN_OFST, pdata->imblk[0].phys);
    nna_write(pdata, NNA_WEIGHT_OFST, pdata->kmblk[0].phys);
    nna_write(pdata, NNA_OUT_OFST, pdata->omblk[0].phys);
    nna_write(pdata, NNA_SCALE_OFST, pdata->bsblk[0].phys);
    nna_write(pdata, NNA_PARAM_OFST, pdata->pmblk[0].phys);
    memset(pdata->omblk[out_flag].addr, 0, conv_param.extent_out_size);

    cur_node = *subgraph;
    i = 0;
    while(1) {
        // 2.1) Start execute current op and copy next op's param to ddr
        // while executing current op.
        nna_write(pdata, NNA_START_OFST, 1);

        // 2.2) Wait op to be doned.
        i = 0;
        while (i < NNA_COUNTER) {
            if (nna_read(pdata, NNA_START_OFST) != 1)
                break;
            udelay(1);
            i ++;
        }
        if (i >= NNA_COUNTER) {
            pr_err("^^^ NNA Conv2d failed\n");
            return -ETIMEDOUT;
        }
        if(cur_node.next_) {
            // pr_info("^^^ output ref:%d\n", cur_node.output_ref_count_);
            if (copy_from_user(&next_node, cur_node.next_, sizeof(next_node))) {
                pr_err("^^^ NNA Copy next node failed\n");
                return -EFAULT;
            }
            // pr_info("^^^ output ref:%d\n", cur_node.output_ref_count_);
            if (copy_from_user(&conv_next_param, next_node.device_param_, sizeof(conv_next_param))) {
                pr_err("^^^ NNA Copy next node->device_param_ failed\n");
                return -EFAULT;
            }
            // Copy param to ddr except for input.
            // pr_info("^^^ copy next node param.\n");
            if (copy_from_user(pdata->bsblk[!flag].addr, 
                        conv_next_param.ba,
                        conv_next_param.scale_size)) {
                pr_err("^^^ NNA Copy BSDM failed\n");
                return -EFAULT;
            }
            if (copy_from_user(pdata->kmblk[!flag].addr,
                        conv_next_param.ka,
                        conv_next_param.extent_weight_size)) {
                pr_err("^^^ NNA Copy KDM failed\n");
                return -EFAULT;
            }
            if (copy_from_user(pdata->pmblk[!flag].addr,
                        conv_next_param.ip,
                        conv_next_param.param_size)) {
                pr_err("^^^ NNA Copy PDM failed\n");
                return -EFAULT;
            }

            memset(pdata->omblk[(out_flag + 1) % 3].addr, 0, conv_next_param.extent_out_size);
            if(next_node.parent_) {
                if (copy_from_user(&parent_node, next_node.parent_, sizeof(parent_node))) {
                    pr_err("^^^ NNA Copy param node failed\n");
                    return -EFAULT;
                }
            }
            // 2.4) Specify next op's interface address.
            if((next_node.output_ref_count_ == 1 && next_node.is_output) || next_node.output_ref_count_ > 1) {
                // Where to put this op's output?
                nna_write(pdata, NNA_OUT_OFST, pdata->omblk_extra[next_node.ddr_block_idx].phys);
            } else {
                nna_write(pdata, NNA_OUT_OFST, pdata->omblk[(out_flag + 1) % 3].phys);
            }
            if(next_node.parent_ &&
                    (parent_node.output_ref_count_ > 1 ||
                     (parent_node.output_ref_count_ == 1 && parent_node.is_output))) {
                // 1) where to get this op's input.
                // 2) Add pad.
                // pr_info("^^^ Special node: %d.\n", next_node.id);
                pad = ((struct IntelFpgaConvParam*)pdata->pmblk[!flag].addr)->in_pad;
                if(pad) {
                    c = ((struct IntelFpgaConvParam*)pdata->pmblk[!flag].addr)->in_c;
                    h =  ((struct IntelFpgaConvParam*)pdata->pmblk[!flag].addr)->in_h;
                    w =  ((struct IntelFpgaConvParam*)pdata->pmblk[!flag].addr)->in_w;
                    memset(pdata->omblk_extra[8 + buff_flag].addr, 0, conv_next_param.extent_in_size);
                    add_pad(pdata->omblk_extra[parent_node.ddr_block_idx].addr, pdata->omblk_extra[8 + buff_flag].addr, c, w, h, pad);
                    nna_write(pdata, NNA_IN_OFST, pdata->omblk_extra[8 + buff_flag].phys);
                    buff_flag = !buff_flag;
                } else {
                    nna_write(pdata, NNA_IN_OFST, pdata->omblk_extra[parent_node.ddr_block_idx].phys);
                }
            } else {
                nna_write(pdata, NNA_IN_OFST, pdata->omblk[out_flag].phys);
            }
            nna_write(pdata, NNA_WEIGHT_OFST, pdata->kmblk[!flag].phys);
            nna_write(pdata, NNA_SCALE_OFST, pdata->bsblk[!flag].phys);
            nna_write(pdata, NNA_PARAM_OFST, pdata->pmblk[!flag].phys);
        }
        // 2.3) if this is an output node, copy output to ddr.
        if(cur_node.is_output) {
            //pad = ((IntelFpgaConvParam*)pdata->pmblk[flag].addr)->in_pad;
            if(cur_node.output_ref_count_ == 0) {
                if (copy_to_user(conv_param.oa,
                            pdata->omblk[out_flag].addr,
                            conv_param.extent_out_size)) {
                    pr_err("^^^ NNA Copy ODM failed\n");
                    return -EFAULT;
                }
            } else {
                if (copy_to_user(conv_param.oa,
                            pdata->omblk_extra[cur_node.ddr_block_idx].addr,
                            conv_param.extent_out_size)) {
                    pr_err("^^^ NNA Copy ODM failed\n");
                    return -EFAULT;
                }
            }
        }

        conv_param = conv_next_param;
        if(cur_node.next_) {
            cur_node = next_node;
        } else {
            break;
        }
        flag = !flag;
        out_flag = (out_flag + 1) % 3;
    }

    return 0;
}

static int nna_pool2d(struct nna_pdata* pdata, struct nna_pool_s *pool)
{
    /*  int tmp;

    // Configure input/kernel/output address
    nna_write(pdata, NNA_IBR_OFST, nna_idm(pdata));
    nna_write(pdata, NNA_OBR_OFST, nna_odm(pdata));

    // Start pool2d
    nna_write(pdata, NNA_GCR_OFST, NNA_GCR_START);
    tmp = 0;
    while (nna_read(pdata, NNA_GCR_OFST) & NNA_GCR_DONE) {
    if (++tmp>1000)
    return -ETIMEDOUT;
    ndelay(1);
    }
    */
    return 0;
}

static int nna_fullcon(struct nna_pdata* pdata, struct nna_fcon_s *fcon)
{
    /*  int tmp;

    // Configure input/kernel/output address
    nna_write(pdata, NNA_IBR_OFST, nna_idm(pdata));
    nna_write(pdata, NNA_KBR_OFST, nna_kdm(pdata));
    nna_write(pdata, NNA_OBR_OFST, nna_odm(pdata));

    // Start full connection
    nna_write(pdata, NNA_GCR_OFST, NNA_GCR_START);
    tmp = 0;
    while (nna_read(pdata, NNA_GCR_OFST) & NNA_GCR_DONE) {
    if (++tmp>1000)
    return -ETIMEDOUT;
    ndelay(1);
    }
    */
    return 0;
}

/*-------------------------------------------------------------------------------------*/
/* File operations */

static int open(struct inode *inode, struct file *filp)
{
    struct nna_pdata *pdata;

    /* pointer to containing data structure of the character device inode */
    pdata = container_of(inode->i_cdev, struct nna_pdata, cdev);

    pdata->pid = current->pid;
    pdata->task = current;

    /* create a reference to our device state in the opened file */
    filp->private_data = pdata;

    return 0;
}

static int release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* Memory map operation */
static int mmap(struct file *filp, struct vm_area_struct *vma)
{
    /* Maps to user-land address space according to phys */
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    vma->vm_flags |= VM_IO;

    /* Map VM Zone to continuous physical pages */
    if (remap_pfn_range(vma, 
                vma->vm_start, 
                vma->vm_pgoff,
                vma->vm_end - vma->vm_start, 
                vma->vm_page_prot)) 
        return -EAGAIN;

    return 0;
}

static long ioctl( struct file *filp, unsigned int cmd, unsigned long arg )
{
    struct nna_pdata* pdata = (struct nna_pdata*)filp->private_data;
    struct nna_mcpy_s mcpy;
    struct nna_info_s info;
    struct nna_mblk_s mblk;
    struct nna_conv_s conv;
    struct DeviceGraphNode conv_subgraph;
    struct nna_pool_s pool;
    struct nna_fcon_s fcon;
    struct nna_creg_s creg;
    struct timeval st, et;
    int tminus, err = 0;

    if (!NNA_IOCTL_VALID(cmd)) {
        return -EINVAL;
    }
    switch (NNA_IOCTL_GET(cmd)) {
        case NNA_CMD_INFO:
            if (copy_from_user(&info, (void*)arg, sizeof(info))) {
                return -EFAULT;
            }
            info.ver = NNA_VERSION;
            if (copy_to_user((void*)arg, &info, sizeof(info))) {
                return -EFAULT;
            }
            break;
        case NNA_CMD_RESET:
            break;
        case NNA_CMD_REGRD:
            if (copy_from_user(&creg, (void*)arg, sizeof(creg))) {
                return -EFAULT;
            }
            creg.data = nna_read(pdata, creg.addr);
            if (copy_to_user((void*)arg, &creg, sizeof(creg))) {
                return -EFAULT;
            }
            break;
        case NNA_CMD_REGWR:
            if (copy_from_user(&creg, (void*)arg, sizeof(creg))) {
                return -EFAULT;
            }
            nna_write(pdata, creg.addr, creg.data);
            break;

        case NNA_CMD_MCPY:
            if (copy_from_user(&mcpy, (void*)arg, sizeof(mcpy))) {
                return -EFAULT;
            }
            if (mutex_lock_interruptible(&pdata->mutex)) {
                return -ERESTARTSYS;
            }
            do_gettimeofday(&st);
            err = nna_dma_copy(pdata, (unsigned long)mcpy.dst, 
                    (unsigned long)mcpy.src, mcpy.len);
            do_gettimeofday(&et);
            mutex_unlock(&pdata->mutex);
            tminus = (et.tv_sec - st.tv_sec) * 1000000 + (et.tv_usec - st.tv_usec);
            pr_info("^^^ NNA_CMD_MCPY %d bytes, used %d us\n", mcpy.len, tminus);
            break;
        case NNA_CMD_MGET:
            if (copy_from_user(&mblk, (void*)arg, sizeof(mblk))) {
                return -EFAULT;
            }
            mblk.virt = dma_alloc_coherent(NULL, mblk.size, (dma_addr_t*)&mblk.phys, GFP_KERNEL);
            if (mblk.virt==NULL) {
                return -ENOMEM;
            }
            mblk.phys = mblk.phys;
            mblk.size = mblk.size;
            if (copy_to_user((void*)arg, &mblk, sizeof(mblk))) {
                dma_free_coherent(NULL, mblk.size, mblk.virt, (dma_addr_t)mblk.phys);
                return -EFAULT;
            }
            if (copy_to_user((void*)arg, &mblk, sizeof(mblk))) {
                return -EFAULT;
            }
            break;
        case NNA_CMD_FREE:
            if (copy_from_user(&mblk, (void*)arg, sizeof(mblk))) {
                return -EFAULT;
            }
            dma_free_coherent(NULL, mblk.size, mblk.virt, (dma_addr_t)mblk.phys);
            break;

        case NNA_CMD_CONV:
            if (copy_from_user(&conv, (void*)arg, sizeof(conv))) {
                return -EFAULT;
            }
            if (mutex_lock_interruptible(&pdata->mutex)) {
                return -ERESTARTSYS;
            }
            do_gettimeofday(&st);
            err = nna_conv2d(pdata, &conv);
            do_gettimeofday(&et);
            mutex_unlock(&pdata->mutex);
            tminus = (et.tv_sec - st.tv_sec) * 1000000 + (et.tv_usec - st.tv_usec);
            pr_info("^^^ NNA_CMD_CONV used %d us, errno=%d\n", tminus, err);
            break;

        case NNA_CMD_CONV_SUBGRAPH:
            if (copy_from_user(&conv_subgraph, (void*)arg, sizeof(conv_subgraph))) {
                return -EFAULT;
            }
            if (mutex_lock_interruptible(&pdata->mutex)) {
                return -ERESTARTSYS;
            }
            do_gettimeofday(&st);
            err = nna_conv2d_subgraph(pdata, &conv_subgraph);
            do_gettimeofday(&et);
            mutex_unlock(&pdata->mutex);
            tminus = (et.tv_sec - st.tv_sec) * 1000000 + (et.tv_usec - st.tv_usec);
            pr_info("^^^ NNA_CMD_CONV_subgraph used %d us, errno=%d\n", tminus, err);
            break;

        case NNA_CMD_POOL:
            if (copy_from_user(&pool, (void*)arg, sizeof(pool))) {
                return -EFAULT;
            }
            if (mutex_lock_interruptible(&pdata->mutex)) {
                return -ERESTARTSYS;
            }
            do_gettimeofday(&st);
            err = nna_pool2d(pdata, &pool);
            do_gettimeofday(&et);
            mutex_unlock(&pdata->mutex);
            tminus = (et.tv_sec - st.tv_sec) * 1000000 + (et.tv_usec - st.tv_usec);
            pr_info("^^^ NNA_CMD_POOL used %d us, errno=%d\n", tminus, err);
            break;

        case NNA_CMD_FCON:
            if (copy_from_user(&fcon, (void*)arg, sizeof(fcon))) {
                return -EFAULT;
            }
            if (mutex_lock_interruptible(&pdata->mutex)) {
                return -ERESTARTSYS;
            }
            do_gettimeofday(&st);
            err = nna_fullcon(pdata, &fcon);
            do_gettimeofday(&et);
            mutex_unlock(&pdata->mutex);
            tminus = (et.tv_sec - st.tv_sec) * 1000000 + (et.tv_usec - st.tv_usec);
            pr_info("^^^ NNA_CMD_FCON used %d us, errno=%d\n", tminus, err);
            break;

        default: return -ENOTTY;
    }

    return err;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = open,
    .release = release,
    .mmap    = mmap,
    .unlocked_ioctl = ioctl,
};

/*-------------------------------------------------------------------------------------*/

static int __init nna_init( void )
{
    int err = 0;
    int i;

    objId = kzalloc(sizeof(struct nna_pdata), GFP_KERNEL);
    if (!objId) {
        pr_err("^^^ kzalloc failed\n");
        return -ENOMEM;
    }
    // allocate memory from CMA
    // Input.
    for(i = 0; i < 2; i++) {
        objId->imblk[i].addr = (uint8_t*)dma_alloc_coherent(
                NULL, NNA_IDM_SIZE, (dma_addr_t*)&objId->imblk[i].phys, GFP_KERNEL);
        if (!objId->imblk[i].addr) {
            pr_err("^^^ dma_alloc_coherent(imblk) failed\n");
            err = -ENOMEM;
            goto fail_0;
        }
        objId->imblk[i].size = NNA_IDM_SIZE;
    }

    // Filter.
    for(i = 0; i < 2; i++) {
        objId->kmblk[i].addr = (uint8_t*)dma_alloc_coherent(
                NULL, NNA_KDM_SIZE, (dma_addr_t*)&objId->kmblk[i].phys, GFP_KERNEL);
        if (!objId->kmblk[i].addr) {
            pr_err("^^^ dma_alloc_coherent(kmblk) failed\n");
            err = -ENOMEM;
            goto fail_0;
        }
        objId->kmblk[i].size = NNA_KDM_SIZE;
    }

    // Output.
    for(i = 0; i < 3; i++) {
        objId->omblk[i].addr = (uint8_t*)dma_alloc_coherent(
                NULL, NNA_ODM_SIZE, (dma_addr_t*)&objId->omblk[i].phys, GFP_KERNEL);
        if (!objId->omblk[i].addr) {
            pr_err("^^^ dma_alloc_coherent(omblk) failed\n");
            err = -ENOMEM;
            goto fail_0;
        }
        objId->omblk[i].size = NNA_ODM_SIZE;
    }
    for(i = 0; i < 10; i++) {
        objId->omblk_extra[i].addr = (uint8_t*)dma_alloc_coherent(
                NULL, NNA_ODM_SIZE, (dma_addr_t*)&objId->omblk_extra[i].phys, GFP_KERNEL);
        if (!objId->omblk_extra[i].addr) {
            pr_err("^^^ dma_alloc_coherent(omblk_extra) failed\n");
            err = -ENOMEM;
            goto fail_0;
        }
        objId->omblk_extra[i].size = NNA_ODM_SIZE;
    }

    // Bias.
    // allocate memory for bias scale.
    for(i = 0; i < 2; i++) {
        objId->bsblk[i].addr = (uint8_t*)dma_alloc_coherent(
                NULL, NNA_BSDM_SIZE, (dma_addr_t*)&objId->bsblk[i].phys, GFP_KERNEL);
        if (!objId->bsblk[i].addr) {
            pr_err("^^^ dma_alloc_coherent(bsblk) failed\n");
            err = -ENOMEM;
            goto fail_0;
        }
        objId->bsblk[i].size = NNA_BSDM_SIZE;
    }

    // Allocate memory for param.
    for(i = 0; i < 2; i++) {
        objId->pmblk[i].addr = (uint8_t*)dma_alloc_coherent(
                NULL, NNA_PDM_SIZE, (dma_addr_t*)&objId->pmblk[i].phys, GFP_KERNEL);
        if (!objId->pmblk[i].addr) {
            pr_err("^^^ dma_alloc_coherent(pmblk) failed\n");
            err = -ENOMEM;
            goto fail_0;
        }
        objId->pmblk[i].size = NNA_PDM_SIZE;
    }

    // init dma
    err = nna_dma_init(objId);
    if (err<0) {
        pr_err("^^^ nna_dma_init failed\n");
        goto fail_1;
    }
    // Remap configuration register address
    /*  err = request_mem_region(NNA_CRM_BASE, NNA_CRM_SIZE, "nna-crm")
        if (err<0) {
        pr_err("request mem region for nna failed\n");
        goto fail_2;
        }*/
    objId->cra = ioremap_nocache(NNA_CRM_BASE, NNA_CRM_SIZE);
    if (!objId->cra) {
        pr_err("^^^ ioremap failed\n");
        err = -EIO;
        goto fail_2;
    }
    // Initialize character device
    err = alloc_chrdev_region(&objId->devno, 0, 1, NNA_DRVNAME);
    if (err<0) {
        pr_err("^^^ alloc_chrdev_region failed\n");
        goto fail_3;
    }
    objId->class = class_create(THIS_MODULE, NNA_BRDNAME);
    if (IS_ERR(objId->class)) {
        pr_err(KERN_ERR "^^^ class_create failed\n");
        goto fail_4;
    }
    cdev_init(&objId->cdev, &fops); 
    objId->cdev.owner = THIS_MODULE;
    objId->cdev.ops = &fops;
    err = cdev_add(&objId->cdev, objId->devno, 1);
    if (err) {
        pr_err("^^^ cdev_add failed\n");
        goto fail_5;
    }
    objId->device = device_create(objId->class, NULL, objId->devno, NULL, NNA_DRVNAME "%d", 0);
    if (IS_ERR(objId->device)) {
        pr_err("^^^ Can't create device\n");
        goto fail_6;
    }

    pr_info("^^^ NNA driver loading done.\n");

    mutex_init(&objId->mutex);
    spin_lock_init(&objId->lock);

    return 0;

    // Error handling
fail_6:
    cdev_del(&objId->cdev);
fail_5:
    class_destroy(objId->class);
fail_4:
    unregister_chrdev_region(objId->devno, 1);
fail_3: 
    iounmap(objId->cra);
fail_2:
    nna_dma_cleanup(objId);
fail_1:
    for(i = 0; i < 2; i++) {
        dma_free_coherent(NULL, objId->imblk[i].size, 
                objId->imblk[i].addr, (dma_addr_t)objId->imblk[i].phys);
        dma_free_coherent(NULL, objId->kmblk[i].size, 
                objId->kmblk[i].addr, (dma_addr_t)objId->kmblk[i].phys);
        dma_free_coherent(NULL, objId->omblk[i].size, 
                objId->omblk[i].addr, (dma_addr_t)objId->omblk[i].phys);
        dma_free_coherent(NULL, objId->bsblk[i].size, 
                objId->bsblk[i].addr, (dma_addr_t)objId->bsblk[i].phys);
        dma_free_coherent(NULL, objId->pmblk[i].size, 
                objId->pmblk[i].addr, (dma_addr_t)objId->pmblk[i].phys);
    }
    dma_free_coherent(NULL, objId->omblk[2].size, 
            objId->omblk[2].addr, (dma_addr_t)objId->omblk[2].phys);
    for(i = 0; i < 10; i++) {
        dma_free_coherent(NULL, objId->omblk_extra[i].size,
                objId->omblk_extra[i].addr, (dma_addr_t)objId->omblk_extra[i].phys);
    }
fail_0:
    kfree(objId);
    objId = NULL;

    return err;
}

static void __exit nna_exit(void)
{
    int i;
    if (!objId)
        return;

    device_destroy(objId->class, objId->devno);
    class_destroy(objId->class);
    cdev_del(&objId->cdev);
    unregister_chrdev_region(objId->devno, 1);

    iounmap(objId->cra);
    release_mem_region(NNA_CRM_BASE, NNA_CRM_SIZE);

    nna_dma_cleanup(objId);
    for(i = 0; i < 2; i++) {
        dma_free_coherent(NULL, objId->imblk[i].size, 
                objId->imblk[i].addr, (dma_addr_t)objId->imblk[i].phys);
        dma_free_coherent(NULL, objId->kmblk[i].size, 
                objId->kmblk[i].addr, (dma_addr_t)objId->kmblk[i].phys);
        dma_free_coherent(NULL, objId->omblk[i].size, 
                objId->omblk[i].addr, (dma_addr_t)objId->omblk[i].phys);
        dma_free_coherent(NULL, objId->bsblk[i].size, 
                objId->bsblk[i].addr, (dma_addr_t)objId->bsblk[i].phys);
        dma_free_coherent(NULL, objId->pmblk[i].size, 
                objId->pmblk[i].addr, (dma_addr_t)objId->pmblk[i].phys);
    }
    dma_free_coherent(NULL, objId->omblk[2].size, 
            objId->omblk[2].addr, (dma_addr_t)objId->omblk[2].phys);
    for(i = 0; i < 10; i++) {
        dma_free_coherent(NULL, objId->omblk_extra[i].size,
                objId->omblk_extra[i].addr, (dma_addr_t)objId->omblk_extra[i].phys);
    }
    kfree(objId);
    objId = NULL;

    pr_err("^^^ NNA driver unloading done\n");
}

module_init(nna_init);
module_exit(nna_exit);
MODULE_LICENSE ("Dual BSD/GPL");

