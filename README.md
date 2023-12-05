# SemiMulti-Thread External Sort for Big Data

v2.1版本。此版本仅实现（不完整）多线程-大量外部数据排序与归并。

由于排序和归并是串行的，所以是不完整的多线程。

V3.0版本正式实现多线程-小内存-大量外部数据-并行排序与归并。

**run with:**

> cd ./Multi_Thread_External_Sort_v2.1
>
> g++ -o MultiThreadExtSortTest DataIO.h NumberControler.h ThreadPool.h KShotMergeMinHeap.h DataIO.cpp KShotMergeMinHeap.cpp MultiThreadExtSortTester.cpp
>
> ./MultiThreadExtSortTest

**Environment:**

Ubuntu 22.04.3, g++ 11.4.0, gcc 11.4.0

## Problem

多线程-大量外部数据排序与归并。由于排序和归并是串行的，所以是不完整的多线程。

外部数据：64MB

排序/归并最大可用内存：8GB

## v2.1基于v2.0的改动：

### Bug修复：

#### 1、K路归并bug修复与优化。

**Bug描述：**循环开始前未初始化，导致最小堆大小只有1。若简单修复，在循环开始前初始化最小堆，1、时间复杂度上不占优势；2、循环结束后最小堆中剩余K个元素为写入输出缓冲区。

**优化前时间复杂度：**具体来说，每次循环都比较K个输入缓冲区中头数，选出最小头数,插入最小堆，然后弹出1个数到输出缓冲区。每次插入最小堆1个数，需要遍历比较K个数的大小。假设K个文件共有N个数，则需要进行`K*N`次遍历。此外，最小堆插入操作（`push()`）时间复杂度为`O(logK)`，最小堆弹出操作时间复杂度为`O(logK)`。每个数运行一次插入和弹出，则总时间复杂度为`O(N*logK)+O(N*logK)+O(K*N)=O(N*(2logK+K))`。

**优化后时间复杂度：**最小堆元素改为`struct`，而不是`int64`。

```c++
struct {
	int64_t filenum;
    int fileIndex;
};
```

循环开始前，初始化最小堆。每次循环，先弹出最小堆的堆顶元素，根节点数字加入输出缓冲区，获取根节点对应文件编号`topNumFileIndex`，将`topNumFileIndex`文件（和输入缓冲区）索引后移一位。随后，将`InBuffer[topNumFileIndex]`对应元素插入最小堆，循环主体结束。

由于不再选择K个文件中最小的头数插入最小堆，每个数减少K次遍历，共减少`O(KN)`的时间复杂度。海量外部数据情境下（N极大），减少`O(KN)`的时间复杂度可以大大减少程序运行时间。

**优化代价：**维护一个大小为K的最小堆，最小堆元素为结构体，使用额外的`K*sizeof(int)`空间。K一般比较小（例如K=8），少量空间换取大量时间复杂度的减少，是一次成功的优化。

**优化后算法描述：**

```C++
初始化最小堆，将每个输入缓冲区第一个元素插入最小堆。
while (!Minheap.empty()){
    if (输出缓冲区已满) {
        输出缓冲区数据写入输出文件；
        重置输出缓冲区位置；
    }
    弹出堆顶元素，放入输出缓冲区。堆顶元素对应文件编号为topIndex。
    if (!(K个文件中所有数字都插入最小堆（所有文件中数据读取完毕，且所有输入缓冲区已空）)) {     
        if (堆顶元素的数字不是文件最后一个数字) {
            将刚刚取出的堆顶元素对应输入缓冲区的下一个元素插入最小堆，即InBuffer[topIndex]插入最小堆。
            InbufPos[topIndex]++;
        }
        if (输入缓冲区所有数字已插入堆 && 文件中还有数字未读入缓冲区) {
            if (第j个文件剩余数据大于MergeMemInBuf大小) {
                第j个文件中再次读取MergeMemInBuf大小的数据；
            }
            else {
                从第j个文件中读取剩余数据；
            }
        }
       }
}
```



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
const int64_t MemSize = 1073741824; // 1G，单个随机数文件大小（用于生成单个随机数文件的内存）
const int64_t MemSizeBlock = 134217728; // 1/8G,MemSize/8,代表内存中int64_t的数量
const int64_t ExSizeBlock = 2147483648; // 16/8=2G

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

#endifxxxxxxxxxx #ifndef NUMBERCONTROLER_H#define NUMBERCONTROLER_H#include <stdio.h>#include <stdlib.h>// #include <string.h>#include <iostream>const unsigned short RandomFileNum = 16; // 随机数文件个数，同时也是排序后文件个数const int64_t modnum = 1125899906842679; // 素数作为模数可保证随机性const int64_t ID = 80316; // 学号后5位// middle fileconst int64_t MemSize = 1073741824; // 1G，单个随机数文件大小（用于生成单个随机数文件的内存）const int64_t MemSizeBlock = 134217728; // 1/8G,MemSize/8,代表内存中int64_t的数量const int64_t ExSizeBlock = 2147483648; // 16/8=2G// // small file// const int64_t MemSize = 8388608; // 8 M，单个随机数文件大小（用于生成单个随机数文件的内存）// const int64_t MemSizeBlock = 1048576; // 1M,MemSize/8,代表内存中int64_t的数量// const int64_t ExSize = 134217728; // 128 M，外部磁盘大小（存储排序后数据）// const int64_t ExSizeBlock = 16777216; // 16 M// large InBuf, large OutBuf// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小// const int64_t MergeOutBufSize = 33554432; // 32M,归并排序输出缓冲区大小// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量// const int64_t MergeOutBufSizeBlock = 4194304; // 4M,MergeOutBufSize/8,代表输出缓冲区int64_t容量// large InBuf, middle OutBuf// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小// const int64_t MergeOutBufSize = 16777216; // 16M,归并排序输出缓冲区大小// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量// const int64_t MergeOutBufSizeBlock = 2097152; // 2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量// large InBuf, small OutBuf// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小// const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量// const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量// middle InBuf, small OutBuf// const int64_t MergeInBufSize = 2097152; // 2M,归并排序单个输入缓冲区大小// const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小// const int64_t MergeInBufSizeBlock = 262144; // 1/4M,MergeInBufSize/8,代表输入缓冲区int64_t容量// const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量// small InBuf, small OutBufconst int64_t MergeInBufSize = 1048576; // 1M,归并排序单个输入缓冲区大小const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小const int64_t MergeInBufSizeBlock = 131072; // 1/8M,MergeInBufSize/8,代表输入缓冲区int64_t容量const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量#endif#ifndef NUMBERCONTROLER_H#define NUMBERCONTROLER_H#include <stdio.h>#include <stdlib.h>// #include <string.h>#include <iostream>const unsigned short RandomFileNum = 16; // 随机数文件个数，同时也是排序后文件个数const int64_t modnum = 1125899906842679; // 素数作为模数可保证随机性const int64_t ID = 80316; // 学号后5位const int64_t MemSize = 8388608; // 8 M，随机数文件大小（用于生成单个随机数文件的内存）const int64_t MemSizeBlock = 1048576; // 1M,MemSize/8,代表内存中int64_t的数量const int64_t ExSize = 134217728; // 128 M，外部磁盘大小（存储排序后数据）const int64_t ExSizeBlock = 16777216; // 16 M// large InBuf, large OutBuf// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小// const int64_t MergeOutBufSize = 33554432; // 32M,归并排序输出缓冲区大小// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量// const int64_t MergeOutBufSizeBlock = 4194304; // 4M,MergeOutBufSize/8,代表输出缓冲区int64_t容量// large InBuf, middle OutBuf// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小// const int64_t MergeOutBufSize = 16777216; // 16M,归并排序输出缓冲区大小// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量// const int64_t MergeOutBufSizeBlock = 2097152; // 2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量// large InBuf, small OutBuf// const int64_t MergeInBufSize = 4194304; // 4M,归并排序单个输入缓冲区大小// const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小// const int64_t MergeInBufSizeBlock = 524288; // 1/2M,MergeInBufSize/8,代表输入缓冲区int64_t容量// const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量// middle InBuf, small OutBuf// const int64_t MergeInBufSize = 2097152; // 2M,归并排序单个输入缓冲区大小// const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小// const int64_t MergeInBufSizeBlock = 262144; // 1/4M,MergeInBufSize/8,代表输入缓冲区int64_t容量// const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量// small InBuf, small OutBufconst int64_t MergeInBufSize = 1048576; // 1M,归并排序单个输入缓冲区大小const int64_t MergeOutBufSize = 4194304; // 4M,归并排序输出缓冲区大小const int64_t MergeInBufSizeBlock = 131072; // 1/8M,MergeInBufSize/8,代表输入缓冲区int64_t容量const int64_t MergeOutBufSizeBlock = 524288; // 1/2M,MergeOutBufSize/8,代表输出缓冲区int64_t容量#endif
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

