# SemiMulti-Thread External Sort for Big Data
此版本仅实现（不完整）多线程-大量外部数据排序与归并。

由于排序和归并是串行的，所以是不完整的多线程。

V3.0版本正式实现多线程-小内存-大量外部数据-并行排序与归并。

**run with:**

> cd ./Multi_Thread_External_Sort_v1.0
> g++ -o ExtSortThreadTest dataio.h NumberControler.h ThreadPool.h KShotMergeMinHeap.h dataio.cpp NumberControler.cpp KShotMergeMinHeap.cpp externalsort.cpp
> ./ExtSortThreadTest

**Environment:**

Ubuntu 22.04.3, g++ 11.4.0, gcc 11.4.0

**v1.0基于v0.2的改动：**
1、增添多线程功能（排序、归并）
2、抛弃原"Loserheap"，重写Minheap，用于多文件归并排序。

## Problem
多线程-大量外部数据排序与归并。由于排序和归并是串行的，所以是不完整的多线程。

外部数据：64MB

排序/归并最大可用内存：8GB

## v1.0基于v0.2的改动：

注：v1.0仅测试少量数据、少量文件、文件大小相同情景。具体来说，16个未排序数据文件，每个文件8MB。

### 升级：

#### 1、存储路径优化。

生成的数据文件单独用文件夹存储，不再与代码文件存储与同个目录之下。

#### 2、**增添线程池**，实现多线程功能（排序、归并）。

具体来说，将排序和归并抽象化为单个线程可执行的任务，打包交给线程池，由线程池进行处理。

#### 3、`RandDataFileSort()`函数修改。

将原本单线程-多文件排序的函数，改为单线程-单文件排序，函数可被抽象为任务提交给线程池。

3-1 文件指针指向单个文件，修改为`FILE * fPtrUnsortSingle`，即`FILE ** fPtrUnsort[i]`

3-2 传入新参数：待排序文件文件名、排序后输出文件名。为排序大小不同的文件做准备，便于扩展。

原：`void RandDataFileSort(int64_t * SortMem, FILE ** fPtrUnsort, FILE ** fPtrSort)`

新：`void RandDataFileSort(int64_t * SortMemSingle, string filenameUnsortSingle, FILE * fPtrUnsortSingle, string filenameSortSingle, FILE *fPtrSortSingle)`

#### 4、新增K路归并函数`KShotMerge`，函数可被抽象为任务提交给线程池。

抛弃原`Loserheap()`，调用priority_queue中的`Minheap()`作为最小堆，用于K个文件归并排序（单线程，K路归并）。此外，考虑到文件大小和缓冲区大小不一定相等，实现了任意文件大小和输入输出缓冲区大小时，K路归并算法，且K可自定义。为今后版本中实现读取目录文件，自行排序铺路。

输入缓冲区`MergeMemInBuf`: 4MB

输出缓冲区`MergeMemOutBuf`: 32MB（考虑到实践中适当增大输出缓冲区能获得更好的效果）

总内存：4*8+32=64MB（K=8）

1、打开K个排序后的文件，每个文件前4MB读入各自输入缓冲区。

2、单线程执行K路外部归并算法

```c++
循环，直到所有文件中数据读取完毕，且所有输入缓冲区已空：
    比较K个输入缓冲区中头数字（HeadNum），选出K个头数字中最小者，即MinHeadNum；
    假设MinHeapNum位于第j个输入缓冲区，头指针后移；
    将MinHeadNum插入最小堆，即MinHeap.push(MinHeadNum);
	弹出堆顶元素，放入输出缓冲区。
    if (输出缓冲区已满) {
        输出缓冲区数据写入输出文件；
        重置输出缓冲区位置；
    } 
	if (第j个输入缓冲区头指针为空) {
        if (第j个文件剩余数据大于MergeMemInBuf大小) {
            第j个文件中再次读取MergeMemInBuf大小的数据；
        }
        else {
            从第j个文件中读取剩余数据；
        }
    }
```



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



### `dataio.h`and`dataio.cpp`

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

#endif
```

### `ThreadPool.h`

线程池，用于实现多线程排序与多线程归并。线程池只是一个线程管理工具，排序程序、归并程序、队列和互斥量实现线程同步与互斥才是核心思想。

为避免重复造轮子，使用github上[lzpong](//https://github.com/lzpong/)实现的线程池。



### `externalsort.cpp`

主程序

