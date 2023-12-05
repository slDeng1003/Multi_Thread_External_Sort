# SemiMulti-Thread External Sort for Big Data
此版本仅实现（不完整）多线程-大量外部数据排序与归并。

由于排序和归并是串行的，所以是不完整的多线程。

V3.0版本正式实现多线程-小内存-大量外部数据-并行排序与归并。

**run with:**

> cd ./Multi_Thread_External_Sort_v1.1
> g++ -o ExtSortThreadTest dataio.h NumberControler.h ThreadPool.h KShotMergeMinHeap.h dataio.cpp NumberControler.cpp KShotMergeMinHeap.cpp MultiThreadExtSortTester.cpp
> ./ExtSortThreadTest

**Environment:**

Ubuntu 22.04.3, g++ 11.4.0, gcc 11.4.0

## Problem
多线程-大量外部数据排序与归并。由于排序和归并是串行的，所以是不完整的多线程。

外部数据：64MB

排序/归并最大可用内存：8GB

## v1.1基于v1.0的改动：

场景：单个随机文件（排序文件）均为8MB，共16个随机（排序）文件

run with:

`g++ -o MultiThreadExtSortTest DataIO.h NumberControler.h ThreadPool.h KShotMergeMinHeap.h DataIO.cpp KShotMergeMinHeap.cpp MultiThreadExtSortTester.cpp`

### 测试：

#### **1、测试不同的K值**

​	1-1，K=8，`MergeThread`=2正常

​	1-2，K=4，`MergeThread`=4报错：double free or corruption (out) /n 已放弃 (核心已转储)，归并结果存储正常。

​		Debug：`main()`中`FILE ** fPtrOut = (FILE **)malloc(sizeof(FILE *));`，忘了写`K*sizeof(...)`。

​		正确写法：`FILE ** fPtrOut = (FILE **)malloc(K * sizeof(FILE *));`，改正后不报错。

​	1-3，K=2, `MergeThread`=8报错：double free or corruption (out) /n 已放弃 (核心已转储)，归并结果存储正常。（K=2, `MergeThread`=2,3不报错，`MergeThread`>=4后报错）。该错误只出现在多线程归并中，推断为多线程运行时内存出错导致的，可能是内存越界。结合归并正常运行、归并结果正常存储，认为是Vector内存越界导致的。

​	后续检查参考思路：1、[glibc调试工具](https://www.cnblogs.com/arnoldlu/p/10827884.html)

#### **2、测试不同的缓冲区大小**

PS：采用K=4，`MergeThread`=4测试。

​	2-1：`MergeInBufSize =4MB`，`MergeOutBufSize =32MB`，（large InBuf, large OutBuf）测试通过

​	2-2：`MergeInBufSize =4MB`，`MergeOutBufSize =16MB`，（large InBuf, middle OutBuf）测试通过

​	2-3：`MergeInBufSize =4MB`，`MergeOutBufSize =4MB`，（large InBuf, small OutBuf）测试通过

​	2-4：`MergeInBufSize =2MB`，`MergeOutBufSize =4MB`，（middle InBuf, small OutBuf）测试通过

​	2-5：`MergeInBufSize =1MB`，`MergeOutBufSize =4MB`，（small InBuf, small OutBuf）测试通过

### **其他修改：**

1、`NumPrint()`函数调用了`SortedResultPrint()`函数，`SortedResultPrint()`函数出口条件是为输出所有排序结果设计的，需要修改，满足普适性。

2、文件名、文件关系、函数关系优化。

## Solution

### `NumControler.h`and`NumControler.cpp`

`NumControler.h`以全局变量形式定义关键参数，并定义函数以生成随机数。

关键参数：随机数文件个数、随机数文件大小、排序内存大小、归并输入缓冲区大小、归并输出缓冲区大小。

生成随机数：自制Hash函数，保证随机性。

具体声明如下：

```c++
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

#endif
```



### `DataIO.h`and`DataIO.cpp`

`DataIO.h`定义数据IO操作，`DataIO.cpp`实现。

包括：

1、生成未排序的随机数据，将数据分为16个相同大小的文件存储到磁盘

2、单线程排序，将排序结果写入新文件。排序算法调用`<algorithm>`库中的`std::sort()`

3、比较两个数字大小的函数，被`std::sort()`调用

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
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <fstream>

using namespace std;
// 哈希函数，用于生成随机数
int64_t HashFunc(int64_t X);

// 生成未排序的随机数据，将数据分为fileNum个相同大小的文件存储到磁盘
void RandNumFileGenerate(int64_t * mem, FILE **fPtrUnsort);

// 获取文件中int_64t数字的数量
int GetFileNums(string filename);

// 将未排序文件读入内存
int ReadUnsortData(int64_t* SortMemSingle, FILE * fPtrUnsortSingle, int64_t fileNums);

// 将排序后数据写入文件
int WriteSortData(int64_t* SortMemSingle, FILE * fPtrSortSingle, int64_t fileNums);

// 适用于单个线程，每次给单个文件排序，将排序结果写入新文件
void RandDataFileSort(int64_t * SortMemSingle, string filenameUnsortSingle, FILE * fPtrUnsortSingle, string filenameSortSingle, FILE *fPtrSortSingle);

// 将排序结果分为fileBlock个内存块输出，每个内存块的前numsPrint个结果输出
void FileNumPrint(int64_t*mem, string filename, unsigned short fileBlock, unsigned short numsPrint);

// 读文件到输入缓冲区
int64_t ReadUnmergeData(int64_t* MergeMemInBufSingle, FILE * fPtrMergeSingle, int filePos, int fileNums);

// 输出缓冲区数据写入文件
int64_t WriteMergeData(int64_t* MergeMemOutbuf, FILE * fPtrOut, int64_t OutbufPos);

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
#include <iostream>
#include <queue>
using namespace std;

// 判断是否K个文件中所有数字已经插入最小堆
bool IsAllFileIntoHeap(unsigned short K, int *fileNum, int64_t * headNumCnt);

// K路归并排序
void KShotMerge(unsigned short K, int64_t **MergeMemInbuf, int64_t *MergeMemOutbuf, FILE ** fPtrMerge, vector<string> filenameMergeVec, FILE * fPtrOut);

#endif
```

### `ThreadPool.h`

线程池，用于实现多线程排序与多线程归并。线程池只是一个线程管理工具，排序程序、归并程序、队列和互斥量实现线程同步与互斥才是核心思想。

为避免重复造轮子，使用github上[lzpong](//https://github.com/lzpong/)实现的线程池。



### `MultiThreadExtSortTester.cpp`

主程序

