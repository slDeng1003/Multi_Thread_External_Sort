#ifndef DATAIO_H
#define DATAIO_H

#include "NumberControler.h"
// #include "quicksort.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <fstream>

using namespace std;

// 获取文件中int_64t数字的数量
int GetFileNums(string filename);
// 将未排序文件读入内存
int ReadUnsortData(int64_t* SortMemSingle, FILE * fPtrUnsortSingle, int64_t fileNums);
// 将排序后数据写入文件
int WriteSortData(int64_t* SortMemSingle, FILE * fPtrSortSingle, int64_t fileNums);
// 创建文件，用于存储未排序数字，允许读和写
void RandNumFileGenerate(int64_t * mem, FILE **fPtrUnsort);
// 适用于单个线程，每次给单个文件排序，将排序结果写入新文件
void RandDataFileSort(int64_t * SortMemSingle, string filenameUnsortSingle, FILE * fPtrUnsortSingle, string filenameSortSingle, FILE *fPtrSortSingle);
// 将排序结果分为8个内存块输出，每个内存块的前30个结果输出
bool SortedResultPrint(int64_t*mem,FILE *fPtrOut, unsigned short fileBlock, unsigned short numsPrint);
// 从排序后文件中读取MergeSizeBlock(8M)大小数据到内存mem
int64_t call(int64_t*,FILE* );
// 归并结果写入文件
int send(int64_t* ,FILE * );

#endif // https://github.com/slDeng1003