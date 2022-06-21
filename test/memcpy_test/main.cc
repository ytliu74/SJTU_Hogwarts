#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 4)
        return -1;

    int a = stoi(argv[1]);
    int b = stoi(argv[2]);
    int c = stoi(argv[3]);

    int len = a * b * c;

    cout << "--------------------" << endl;
    cout << "For " << len << endl;

    clock_t start, end;
    int* src = new int [len];
    int* dst = new int [len];

    start = clock();

    memcpy(dst, src, sizeof(int) * len);

    end = clock();

    cout << "memcpy time: " <<  end - start << endl;

    start = clock();

    for (int i = 0; i < len; i++)
        dst[i] = src[i];

    end = clock();

    cout << "assignment time: " <<  end - start << endl;


    return 0;
}