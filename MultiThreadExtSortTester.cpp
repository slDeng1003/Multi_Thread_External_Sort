#include <iostream>
#include <stdlib.h>
#include <string>
#include <functional>

#include "DataIO.h"
#include "NumberControler.h"
#include "ThreadPool.h"
#include "KShotMergeMinHeap.h"

using namespace std;


// 分配内存（用于生成随机数、存储排序后的随机数、写入排序数）
int64_t ** MemoryAllocate(unsigned short MemNum, int64_t MemorySize) {
	int64_t **memory = (int64_t **) malloc(MemNum * sizeof(int64_t *));
	if (memory == NULL) {
		cout << "memory is not allocated!" << endl;
		}
    else {
		cout << "memory allocate sucess" << endl;
	}
	for (int i = 0; i<MemNum; i++) {
		
		memory[i] = (int64_t *) malloc(MemorySize);
		if (memory == NULL) {
			cout << "memory[ " << i << " ] is not allocated!" << endl;
		}
		else {
			cout << "memory[ " << i << " ] allocate sucess" << endl;
		}
	}
	return memory;
}


int main(){
	int i,j;
	DataIO dataio; // 声明类，用于Data IO操作
	DataIO *ptrDataIO = &dataio; // 指向类的指针，用于归并传递参数
	

	vector <string> filenameUnsort;
	for (i = 0; i<16; i++) {
		filenameUnsort.push_back(string("./data/data_random/data_random")+to_string(i)+string(".bin"));
	}

	// 已生成随机数文件后，运行这几行
	// 指针数组，每个元素都指向一个未排序文件
	FILE ** fPtrUnsort = (FILE**)malloc(RandomFileNum*sizeof(FILE *));
	for (i = 0; i < RandomFileNum; i++) {
		fPtrUnsort[i] = fopen(filenameUnsort[i].data(),"rb"); 
	}
	
	// // 生成随机数文件用，若随机数文件已经生成，无需运行这几行
	// FILE ** fPtrUnsort = (FILE**)malloc(RandomFileNum*sizeof(FILE *));
	// for (i = 0; i < RandomFileNum; i++) {
	// 	fPtrUnsort[i] = fopen(filenameUnsort[i].data(),"wb+"); 
	// }
	// int64_t *NumGeneMem = (int64_t *)malloc(MemSize); // 分配内存（用于生成随机数）
	// dataio.RandNumFileGenerate(NumGeneMem, fPtrUnsort);
	// free(NumGeneMem);


	// 利用线程池多线程排序,排序结果写入新文件
	// RandDataFileSort()提交给线程池;
	// SortResult存储排序结果(void),调用.get()函数等待排序完成,以免在排序完成前释放内存/访问文件
	int64_t **SortMem = MemoryAllocate(RandomFileNum, MemSize); // 分配内存（用于排序）
	FILE ** fPtrSort = (FILE**)malloc(16*sizeof(FILE *));
	unsigned short ThreadNum = 16;
	ThreadPool ThreadPoolExecutor{ ThreadNum }; // 定义线程池
	unsigned short SortThread = 4;
	vector <string> filenameSort;
	vector< future<void> > SortResult;
	for (i=0; i<SortThread; i++) {
		filenameSort.push_back(string("./data/data_sort/data_sort")+to_string(i)+string(".bin"));
		SortResult.emplace_back(ThreadPoolExecutor.commit(dataio.RandDataFileSort, SortMem[i],filenameUnsort[i], fPtrUnsort[i], filenameSort[i], fPtrSort[i]));
	}
	// 等待线程排序完成
	for (i=0; i<SortThread; i++) {
		SortResult[i].get();
	}

	// 抽样输出中间排序结果
	int64_t *SortPrintmem = (int64_t *) malloc(MemSize);
	unsigned short fileBlock = 1;
	unsigned short numsPrint = 5;
	for (i=0; i<RandomFileNum; i++) {
		dataio.FileNumPrint(SortPrintmem, filenameSort[i], fileBlock, numsPrint);
	}
	free(SortPrintmem);

	// 释放内存
	free(SortMem);
	free(fPtrUnsort);
	free(fPtrSort);
	vector<string>().swap(filenameSort);
	vector< future<void> >().swap(SortResult);
	vector <string>().swap(filenameUnsort);


	// 多线程K路归并排序
	// unsigned short ThreadNum = 8;
	unsigned short K = 4;
	unsigned short MergeThread = 4; // filenum/K=16/4=4

	// 分配内存
	int64_t ***MergeMemInbuf = (int64_t ***) malloc(MergeThread * sizeof(int64_t **));
	int64_t **MergeMemOutbuf = MemoryAllocate(MergeThread, MergeOutBufSize);

	FILE ***fPtrMerge = (FILE***)malloc(MergeThread * sizeof(FILE **));
	vector<vector<string>> filenameMergeVec;

	FILE ** fPtrOut = (FILE **)malloc(K * sizeof(FILE *));
	vector<string> filenameOutVec;
	vector<string> tmpVec;

	// ThreadPool ThreadPoolExecutor{ ThreadNum }; // 定义线程池
	vector< future<void> > MergeResult;

	for (i=0; i<MergeThread; i++) {
		
		MergeMemInbuf[i] = MemoryAllocate(K, MergeInBufSize);
		fPtrMerge[i] = (FILE**)malloc(K*sizeof(FILE *));
		// 打开要归并的文件
		cout << "start to open sorted (to merge) file" << endl;
		for (j=0; j<K; j++) {
			tmpVec.push_back(string("./data/data_sort/data_sort")+to_string(i*K+j)+string(".bin"));
			cout << "filenameMergeVec[" << i << "][" << j << "]: " << tmpVec[j] << endl;
			// fPtrMerge[i][j] = (FILE *)malloc(sizeof(FILE));
			fPtrMerge[i][j] = fopen(tmpVec[j].data(),"rb"); 
			if (fPtrMerge[i][j] == NULL) {
				cerr << "Open error (data_sort)!" << endl;
			}
		}
		filenameMergeVec.push_back(tmpVec);
		tmpVec.clear();

		// 打开存储归并结果的文件
		cout << "start to create output file" << endl;
		filenameOutVec.push_back(string("./data/data_out/data_out")+to_string(i*K+0)+string("_")+to_string((i+1)*K-1)+string(".bin"));
		// fPtrOut[i] = (FILE *)malloc(sizeof(FILE));
		fPtrOut[i] = fopen(filenameOutVec[i].data(),"wb+");
		if (fPtrOut[i] == NULL) {
			cerr << "Open error (data_out)!" << endl;
		}

		// // K路归并任务提交给线程池
		cout << "start to submit to threadpool" << endl;
		// KShotMergeMinHeap Mergeheap;
		// MergeResult.emplace_back(ThreadPoolExecutor.commit(Mergeheap.KShotMerge, ptrDataIO, K, MergeMemInbuf[i], MergeMemOutbuf[i], fPtrMerge[i], filenameMergeVec[i], fPtrOut[i]));
		MergeResult.emplace_back(ThreadPoolExecutor.commit(KShotMergeMinHeap::KShotMerge, ptrDataIO, K, MergeMemInbuf[i], MergeMemOutbuf[i], fPtrMerge[i], filenameMergeVec[i], fPtrOut[i]));

	}
	
	// 等待线程归并完成，关闭文件，指针置空
	for (i=0; i<MergeThread; i++) {
		MergeResult[i].get();
		//关闭文件
		for (j=0; j<K; j++) {
			fclose(fPtrMerge[i][j]);
			fPtrMerge[i][j] = NULL;
		}
		fPtrMerge[i] = NULL;
		fclose(fPtrOut[i]);
		fPtrOut[i] = NULL;
	}
	fPtrOut = NULL;
	fPtrMerge = NULL;

	// 抽样输出归并结果
	int64_t *MergePrintMem = (int64_t *) malloc(MemSize*(8));
	fileBlock = 3;
	numsPrint = 5;
	for (i=0; i<MergeThread; i++) {
		dataio.FileNumPrint(MergePrintMem, filenameOutVec[i], fileBlock, numsPrint);
	}
	free(MergePrintMem);

	// 释放内存
	// 释放vector内存
	vector<vector<string>>().swap(filenameMergeVec);
	vector<string>().swap(filenameOutVec);
	vector<string>().swap(tmpVec);
	// 释放缓冲区内存
	for (i=0; i<MergeThread; i++) {
		for (j=0; j<K; j++) {
			free(MergeMemInbuf[i][j]);
		}
		free(MergeMemInbuf[i]);
		free(MergeMemOutbuf[i]);
	}
	free(MergeMemInbuf);
	free(MergeMemOutbuf);

	return 0;
} // https://github.com/slDeng1003