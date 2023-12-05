# Single-Thread External Sort for Big Data
此版本仅实现单线程-大量外部数据排序与归并，V3.0版本正式实现多线程-小内存-大量外部数据-并行排序与归并。

**run with:**

> cd ./Multi_Thread_External_Sort_v0.2
> g++ -o ExtSortTest dataio.h NumberControler.h Loserheap.h quicksort.h dataio.cpp NumberControler.cpp Loserheap.cpp externalsort.cpp
> ./ExtSortTest

**Environment:**

Ubuntu 22.04.3, g++ 11.4.0, gcc 11.4.0

## Problem

单线程-大量外部数据排序与归并。

外部数据：16GB

排序/归并最大可用内存：2GB

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

2、单线程排序，将排序结果写入新文件。排序算法调用`<algorithm>`库中的`qsort()`

3、比较两个数字大小的函数，被`qsort()`调用

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
// 比较两个数字大小的函数，被qsort()调用
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

#endif
```



### `Loserheap.h`and`Loserheap.cpp`
`DataIO.h`定义败者堆，`DataIO.cpp`实现。

用败者堆对多个文件进行归并。

具体声明如下：

```c++
#include <stdlib.h>

class Loserheap{
	public:
		Loserheap(int64_t* a);
		int adjust(int i,int64_t insertnum);
		int getindex();
		int64_t getwinner();
	private:
		int heap[16]; // 存储index of number
		int64_t leaves[16]; // 存储number
};
```



### `externalsort.cpp`
主程序

