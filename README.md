# SemiMulti-Thread External Sort for Big Data

v2.2版本。此版本仅实现（不完整）多线程-大量外部数据排序与归并。

由于排序和归并是串行的，所以是不完整的多线程。

V3.0版本正式实现多线程-小内存-大量外部数据-并行排序与归并。

**run with:**

> cd ./Multi_Thread_External_Sort_v2.2
>
> g++ -o MultiThreadExtSortTest DataIO.h NumberControler.h ThreadPool.h KShotMergeMinHeap.h DataIO.cpp KShotMergeMinHeap.cpp MultiThreadExtSortTester.cpp
>
> ./MultiThreadExtSortTest

**Environment:**

Ubuntu 22.04.3, g++ 11.4.0, gcc 11.4.0

## Problem

多线程-大量外部数据排序与归并。由于排序和归并是串行的，所以是不完整的多线程。

外部数据：16GB

排序/归并最大可用总内存：64GB

## v2.2基于v2.1的改动：

### 升级：小内存大文件排序归并

v2.2之前的版本，排序函数对某个文件一次性排序。对于大文件（GB级），内存显然无法满足要求。因此，v2.2版本需要实现少量内存（例如16MB）下的大文件（例如1GB）排序。此外，需要测试小内存大文件归并。

**测试场景：**16个1GB的int64随机数文件，使用16线程排序、4线程4路归并。

排序生成16个1GB的排序后文件；K=4路归并，每个归并线程生成4GB归并文件。

排序内存（单线程）：16MB；归并内存（单线程）：输入缓冲区1MB，输出缓冲区4MB。

## Solution

### `NumControler.h`

`NumControler.h`以全局变量形式定义关键参数。

关键参数：随机数文件个数、随机数文件大小、排序内存大小、归并输入缓冲区大小、归并输出缓冲区大小。

具体声明如下：

```c++
#ifndef NUMBERCONTROLER_H
#define NUMBERCONTROLER_H

#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
#include <iostream>

const unsigned short RandomFileNum = 16; // 随机数文件个数，同时也是排序后文件个数

const int64_t modnum = 1125899906842679; // 素数作为模数可保证随机性
const int64_t ID = 80316; // 学号后5位

// middle file
const int64_t MemSize = 1073741824; // 1GB，单个随机数文件大小（用于生成单个随机数文件的内存）
const int64_t MemSizeBlock = 134217728; // 1/8G,MemSize/8,代表内存中int64_t的数量
const int64_t ExSizeBlock = 2147483648; // 16/8=2G，外部磁盘中随机数的数量
const int64_t SortMemSize = 16777216; // 16MB，用于单线程排序
const int64_t SortMemSizeBlock = 2097152; // 2MB，单线程排序处理的最大数据量

// // small file
// const int64_t MemSize = 8388608; // 8 M，单个随机数文件大小（用于生成单个随机数文件的内存）
// const int64_t MemSizeBlock = 1048576; // 1M,MemSize/8,代表内存中int64_t的数量
// const int64_t ExSize = 134217728; // 128 M，外部磁盘大小（存储排序后数据）
// const int64_t ExSizeBlock = 16777216; // 16 M

// large InBuf, large OutBuf
// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小
// const int64_t MergeOutBufSize = 33554432; // 32M,归并排序输出缓冲区大小
// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量
// const int64_t MergeOutBufSizeBlock = 4194304; // 4M,MergeOutBufSize/8,代表输出缓冲区int64_t容量

// large InBuf, middle OutBuf
// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小
// const int64_t MergeOutBufSize = 16777216; // 16M,归并排序输出缓冲区大小
// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量
// const int64_t MergeOutBufSizeBlock = 2097152; // 2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量

// large InBuf, small OutBuf
// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小
// const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小
// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量
// const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量

// middle InBuf, small OutBuf
// const int64_t MergeInBufSize = 2097152; // 2M,归并排序单个输入缓冲区大小
// const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小
// const int64_t MergeInBufSizeBlock = 262144; // 1/4M,MergeInBufSize/8,代表输入缓冲区int64_t容量
// const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量

// small InBuf, small OutBuf
const int64_t MergeInBufSize = 1048576; // 1M,归并排序单个输入缓冲区大小
const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小
const int64_t MergeInBufSizeBlock = 131072; // 1/8M,MergeInBufSize/8,代表输入缓冲区int64_t容量
const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量

#endif
```



### `DataIO.h`and`DataIO.cpp`

`DataIO.h`定义数据IO操作，`DataIO.cpp`实现。

包括：

1、生成未排序的随机数据，将数据分为16个相同大小的文件存储到磁盘

2、单线程排序，将排序结果写入新文件。排序算法调用`<algorithm>`库中的`std::sort()`

3、获取文件中int_64t数字的数量

4、将未排序文件读入内存

5、将排序结果写入文件

6、输出排序结果

7、读取待归并文件

8、写入归并结果文件

具体声明如下：

```c++
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

#endif
```



### `KShotMergeMinHeap.h`and`KShotMergeMinHeap.cpp`

`KShotMergeMinHeap.h`定义K路归并最小堆，`KShotMergeMinHeap.cpp`实现。

使用最小堆，同时对K个文件执行K路归并操作。

具体声明如下：

```c++
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
```

### `ThreadPool.h`

线程池，用于实现多线程排序与多线程归并。线程池只是一个线程管理工具，排序程序、归并程序、队列和互斥量实现线程同步与互斥才是核心思想。

为避免重复造轮子，使用github上[lzpong](//https://github.com/lzpong/)实现的线程池。



### `MultiThreadExtSortTester.cpp`

主程序

