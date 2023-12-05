#ifndef DATAIO_H
#define DATAIO_H

#include "NumberControler.h"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <string>
using namespace std;

// 类成员函数必须是static才能使用线程池
class DataIO {
public:  
    // 生成未排序的随机数据，将数据分为fileNum个相同大小的文件存储到磁盘
    void RandNumFileGenerate(int64_t * mem, FILE **fPtrUnsort);
    // 适用于单个线程，每次给单个文件排序，将排序结果写入新文件
    static void RandDataFileSort(int64_t * SortMemSingle, string filenameUnsortSingle, FILE * fPtrUnsortSingle, string filenameSortSingle, FILE *fPtrSortSingle);
    // 将排序结果分为fileBlock个内存块输出，每个内存块的前numsPrint个结果输出
    void FileNumPrint(int64_t*mem, string filename, unsigned short fileBlock, unsigned short numsPrint);
    // 获取文件中int_64t数字的数量
    static int GetFileNums(string filename);
    // 将未排序文件读入内存
    static int ReadUnsortData(int64_t* SortMemSingle, FILE * fPtrUnsortSingle, int64_t fileNums);
    // 将排序后数据写入文件
    static int WriteSortData(int64_t* SortMemSingle, FILE * fPtrSortSingle, int64_t fileNums);
    // 读文件到输入缓冲区
    int64_t ReadUnmergeData(int64_t* MergeMemInBufSingle, FILE * fPtrMergeSingle, int filePos, int fileNums);
    // 输出缓冲区数据写入文件
    int64_t WriteMergeData(int64_t* MergeMemOutbuf, FILE * fPtrOut, int64_t OutbufPos);

private:
    // 哈希函数，用于生成随机数
    int64_t HashFunc(int64_t X);
};

#endif // https://github.com/slDeng1003