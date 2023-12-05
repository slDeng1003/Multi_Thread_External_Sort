#ifndef KSHOTMERGEMINHEAP_H
#define KSHOTMERGEMINHEAP_H
#include "NumberControler.h"
#include <iostream>
#include <queue>
using namespace std;

// 初始化最小堆
// void InitMinHeap();

// 读文件到输入缓冲区
int64_t ReadUnmergeData(int64_t* MergeMemInBufSingle, FILE * fPtrMergeSingle, int filePos, int fileNums);

// 输出缓冲区数据写入文件
int64_t WriteMergeData(int64_t* MergeMemOutbuf, FILE * fPtrOut, int64_t OutbufPos);

// 判断是否K个文件中所有数字已经插入最小堆
bool IsAllNumIntoHeap(unsigned short K, int *fileNum, int64_t * headNumCnt);

// K路归并排序
void KShotMerge(unsigned short K, int64_t **MergeMemInbuf, int64_t *MergeMemOutbuf, FILE ** fPtrMerge, vector<string> filenameMergeVec, FILE * fPtrOut);

#endif // https://github.com/slDeng1003