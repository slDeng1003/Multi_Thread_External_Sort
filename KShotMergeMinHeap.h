#ifndef KSHOTMERGEMINHEAP_H
#define KSHOTMERGEMINHEAP_H

#include "NumberControler.h"
#include "DataIO.h"
#include <queue>
#include <iostream>
#include <memory.h>
using namespace std;


class KShotMergeMinHeap {
public:
    // K路归并排序
    static void KShotMerge(DataIO *dataio, unsigned short K, int64_t **MergeMemInbuf, int64_t *MergeMemOutbuf, FILE ** fPtrMerge, vector<string> filenameMergeVec, FILE * fPtrOut);

private:
    // 判断是否K个文件中所有数字已经插入最小堆
    static bool IsAllFileIntoHeap(unsigned short K, int *fileNum, int64_t * headNumCnt);

};

#endif