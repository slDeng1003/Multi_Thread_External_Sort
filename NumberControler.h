#ifndef NUMBERCONTROLER_H
#define NUMBERCONTROLER_H

#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
#include <iostream>

const unsigned short RandomFileNum = 16; // 随机数文件个数，同时也是排序后文件个数

const int64_t modnum = 1125899906842679; // 素数作为模数可保证随机性
const int64_t ID = 80316; // 学号后5位

const int64_t MemSize = 8388608; // 8 M，随机数文件大小（用于生成单个随机数文件的内存）
const int64_t MemSizeBlock = 1048576; // 1M,MemSize/8,代表内存中int64_t的数量
const int64_t ExSize = 134217728; // 128 M，外部磁盘大小（存储排序后数据）
const int64_t ExSizeBlock = 16777216; // 16 M
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