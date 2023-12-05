#ifndef NUMBERCONTROLER_H
#define NUMBERCONTROLER_H

// #include "dataio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <time.h>

const int64_t MemSize = 8388608; // 8 M，内存大小（用于生成随机数）
const int64_t ExSize = 134217728; // 128 M，外部磁盘大小（存储排序后数据）
const int64_t MergeSize = 67108864; // 64 M，总归并内存大小（为随机数据排序）
const int64_t MemSizeBlock = 1048576; // 1 M
const int64_t ExSizeBlock = 16777216; // 16 M
const int64_t MergeSizeBlock = 8388608; // 8M
/*
int64_t MemSize = 2097152/2*1024; // (2*1024*1024)/2*1024, 1GB for mem, Bytes for malloc()
int64_t ExSize = 16777216*1024; // (16*1024*1024)*1024, 16GB for disk
int64_t MergeSize = 262144/4*1024; // (256*1024)/4*1024, 64MB for Mergemem
int64_t MemSizeBlock = MemSize/8; // 1/8 GB
int64_t ExSizeBlock = ExSize/8; // 16/8=2 GB
int64_t MergeSizeBlock = MergeSize/8; // 64/8=8 MB
*/

const int64_t modnum = 1125899906842679; // 素数作为模数可保证随机性
const int64_t ID = 80316; // 学号后5位

// const int64_t modnum = 46750697; // 7*17*19*23*29*31，素数作为模数可保证随机性

// 哈希函数，用于生成随机数
int64_t HashFunc(int64_t X);

// 生成未排序的随机数据，将数据分为16个相同大小的文件存储到磁盘
// 调用：int64_t HashFunc(int64_t X)
bool RandNumGenerator(int64_t* mem,FILE ** fPtrUnsort);

// 将排序结果输出
// bool SortedResultPrint(int64_t*mem,FILE *fPtrOut);

#endif // https://github.com/slDeng1003