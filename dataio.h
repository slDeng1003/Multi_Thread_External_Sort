#ifndef DATAIO_H
#define DATAIO_H

#include "NumberControler.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
// 比较两个数字大小的函数，被`std::sort()`调用
int compare(const void* a,const void* b);
// 将未排序文件读入内存
int ReadUnsortData(int64_t* ,FILE *);
// 将排序结果写入文件
int WriteSortData(int64_t* ,FILE *);
// 生成未排序的随机数据，将数据分为16个相同大小的文件存储到磁盘
void RandNumFileGenerate(int64_t * mem, FILE **fPtrUnsort);
// 单线程排序，将排序结果写入新文件。排序算法调用<algorithm>库中的std::sort()
void RandDataFileSort(int64_t * SortMem, FILE ** fPtrUnsort, FILE ** fPtrSort);
// 输出排序结果
bool SortedResultPrint(int64_t*mem,FILE *fPtrOut);
// 读取待归并文件
int64_t call(int64_t*,FILE* );
// 写入归并结果文件
int send(int64_t* ,FILE * );

#endif // https://github.com/slDeng1003