#include <iostream>
#include <stdlib.h>
#include <string>

#include "dataio.h"
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

// 文件均分为fileBlock个块，输出前numsPrint个数
void NumPrint(int64_t *mem, string filename,unsigned short fileBlock, unsigned short numsPrint) {
	// 抽查排序结果并输出
	// int64_t *mem = (int64_t *) malloc(MemSize);
    FILE * fPtrPrint = fopen(filename.data(),"rb");
    if(SortedResultPrint(mem, fPtrPrint, fileBlock, numsPrint))
        cout<<"Sorted Results Printed."<<endl;
	fclose(fPtrPrint);
	// free(fPtrPrint); // fclose已经回收资源，无需free
}


int main(){
	// unsigned short fileNum = 16;
	// int64_t **SortMem = MemoryAllocate(fileNum, MemSize); // 分配内存（用于排序）
	// // int64_t **mergeMem = MemoryAllocate(1, MergeSize); // 分配内存（用于归并）
	
	// int i;
	// vector <string> filenameUnsort;
	// for (i = 0; i<16; i++) {
	// 	filenameUnsort.push_back(string("./data/data_random/data_random")+to_string(i)+string(".bin"));
	// 	// filenameUnsort[i] = string("./data/data_random/data_random")+to_string(i)+string(".bin");
	// }


	// // 已生成随机数文件后，运行这几行
	// // 指针数组，每个元素都指向一个未排序文件
	// FILE ** fPtrUnsort = (FILE**)malloc(16*sizeof(FILE *));
	// for (i = 0; i < 16; i++) {
	// 	fPtrUnsort[i] = fopen(filenameUnsort[i].data(),"rb"); 
	// }
	
	// // // 生成随机数文件用，若随机数文件已经生成，无需运行这几行
	// // FILE ** fPtrUnsort = (FILE**)malloc(16*sizeof(FILE *));
	// // for (i = 0; i < 16; i++) {
	// // 	fPtrUnsort[i] = fopen(filenameUnsort[i].data(),"wb+"); 
	// // }
	// // int64_t *NumGeneMem = MemoryAllocate(MemSize); // 分配内存（用于生成随机数）
	// // RandNumFileGenerate(NumGeneMem, fPtrUnsort);


	// // 利用线程池多线程排序,排序结果写入新文件
	// // RandDataFileSort()提交给线程池;
	// // SortResult存储排序结果(void),调用.get()函数等待排序完成,以免在排序完成前释放内存/访问文件
	// FILE ** fPtrSort = (FILE**)malloc(16*sizeof(FILE *));
	// unsigned short ThreadNum = 16;
	// ThreadPool ThreadPoolExecutor{ ThreadNum }; // 定义线程池
	// vector <string> filenameSort;
	// vector< future<void> > SortResult;
	// for (i=0; i<16; i++) {
	// 	filenameSort.push_back(string("./data/data_sort/data_sort")+to_string(i)+string(".bin"));
	// 	// filenameSort[i] = string("./data/data_sort/data_sort")+to_string(i)+string(".bin");
	// 	// ThreadPoolExecutor.commit(RandDataFileSort, SortMem[i],filenameUnsort[i], fPtrUnsort[i], filenameSort[i], fPtrSort[i]);
	// 	SortResult.emplace_back(ThreadPoolExecutor.commit(RandDataFileSort, SortMem[i],filenameUnsort[i], fPtrUnsort[i], filenameSort[i], fPtrSort[i]));
	// }
	// // 等待线程排序完成
	// for (i=0;i<16;i++) {
	// 	SortResult[i].get();
	// }

	
	// free(SortMem);
	// free(fPtrUnsort);
	// free(fPtrSort);


	// // 抽样输出中间排序结果
	// int64_t *mem = (int64_t *) malloc(MemSize);
	// unsigned short fileBlock = 1;
	// unsigned short numsPrint = 5;
	// NumPrint(mem, "./data/data_sort/data_sort0.bin", fileBlock, numsPrint);
	// NumPrint(mem, "./data/data_sort/data_sort1.bin", fileBlock, numsPrint);
	// NumPrint(mem, "./data/data_sort/data_sort2.bin", fileBlock, numsPrint);
	// NumPrint(mem, "./data/data_sort/data_sort3.bin", fileBlock, numsPrint);
	// NumPrint(mem, "./data/data_sort/data_sort4.bin", fileBlock, numsPrint);
	// NumPrint(mem, "./data/data_sort/data_sort5.bin", fileBlock, numsPrint);
	// NumPrint(mem, "./data/data_sort/data_sort6.bin", fileBlock, numsPrint);
	// NumPrint(mem, "./data/data_sort/data_sort7.bin", fileBlock, numsPrint);

	// free(mem);
	// return 0;

	// 双线程K路归并排序
	unsigned short ThreadNum = 16;
	unsigned short K = 8;
	unsigned short MergeThread = 2; // filenum/K=16/8=2
	int i,j;
	// 分配内存
	int64_t ***MergeMemInbuf = (int64_t ***) malloc(MergeThread * sizeof(int64_t **));
	int64_t **MergeMemOutbuf = MemoryAllocate(MergeThread, MergeOutBufSize);

	FILE ***fPtrMerge = (FILE***)malloc(MergeThread * sizeof(FILE **));
	vector<vector<string>> filenameMergeVec;

	FILE ** fPtrOut = (FILE **)malloc(K * sizeof(FILE *));
	vector<string> filenameOutVec;
	vector<string> tmpVec;

	ThreadPool ThreadPoolExecutor{ ThreadNum }; // 定义线程池
	vector< future<void> > MergeResult;

	for (i=0; i<MergeThread; i++) {
		MergeMemInbuf[i] = MemoryAllocate(K, MergeInBufSize);
		fPtrMerge[i] = (FILE**)malloc(K*sizeof(FILE *));
		// 打开要归并的文件
		cout << "start to open sorted (to merge) file" << endl;
		for (j=0; j<K; j++) {
			tmpVec.push_back(string("./data/data_sort/data_sort")+to_string(i*K+j)+string(".bin"));
			cout << "filenameMergeVec[" << i << "][" << j << "]: " << tmpVec[j] << endl;
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
		fPtrOut[i] = fopen(filenameOutVec[i].data(),"wb+");
		if (fPtrOut[i] == NULL) {
			cerr << "Open error (data_out)!" << endl;
		}

		// K路归并任务提交给线程池
		cout << "start to submit to threadpool" << endl;
		MergeResult.emplace_back(ThreadPoolExecutor.commit(KShotMerge, K, MergeMemInbuf[i], MergeMemOutbuf[i], fPtrMerge[i], filenameMergeVec[i], fPtrOut[i]));
		// KShotMerge(K, MergeMemInbuf[i], MergeMemOutbuf[i], fPtrMerge[i], filenameMergeVec[i], fPtrOut[i]);
	}

	// 等待线程归并完成
	for (i=0; i<MergeThread; i++) {
		MergeResult[i].get();
		//关闭文件
		for (j=0; j<K; j++) {
			fclose(fPtrMerge[i][j]);
		}
		fclose(fPtrOut[i]);
	}

	// 抽样输出归并结果
	int64_t *mem = (int64_t *) malloc(MemSize*8);
	unsigned short fileBlock = 8;
	unsigned short numsPrint = 10;
	for (i=0; i<MergeThread; i++) {
		NumPrint(mem, filenameOutVec[i], fileBlock, numsPrint);
	}
	free(mem);

	// 释放内存
	free(MergeMemInbuf);
	free(MergeMemOutbuf);

	// // 单线程K路归并排序
	// // KShotMerge(unsigned short K, int64_t **MergeMemInbuf, int64_t *MergeMemOutbuf, FILE ** fPtrMerge, vector<string> filenameMergeVec, FILE * fPtrOut)
	// unsigned short K = 8;
	// int i;
	// int64_t **MergeMemInbuf = MemoryAllocate(K, MergeInBufSize);
	// int64_t *MergeMemOutbuf = MemoryAllocate(1, MergeOutBufSize)[0];
	// FILE ** fPtrMerge = (FILE**)malloc(K*sizeof(FILE *));
	// vector<string> filenameMergeVec;
	// for (i=0; i<K; i++) { // 暂时读取前K个文件，后面版本再更新ls检索文件功能之类的
	// 	filenameMergeVec.push_back(string("./data/data_sort/data_sort")+to_string(i)+string(".bin"));
	// 	fPtrMerge[i] = fopen(filenameMergeVec[i].data(),"rb"); 
	// 	if (fPtrMerge[i] == NULL) {
	// 		cerr << "Open error (data_sort)!" << endl;
	// 	}
	// }
	// FILE * fPtrOut = (FILE *)malloc(sizeof(FILE *));
	// string filenameOut = string("./data/data_out/data_out")+to_string(0)+string("_")+to_string(K-1)+string(".bin");
	// fPtrOut = fopen(filenameOut.data(),"wb+");
	// if (fPtrOut == NULL) {
	// 	cerr << "Open error (data_out)!" << endl;
	// }
	// // K路归并
	// KShotMerge(K, MergeMemInbuf, MergeMemOutbuf,  fPtrMerge, filenameMergeVec, fPtrOut);

	// //关闭文件
	// for (i=0; i<K; i++) {
	// 	fclose(fPtrMerge[i]);
	// }
	// fclose(fPtrOut);

	// // 释放内存
	// free(MergeMemInbuf);
	// free(MergeMemOutbuf);

	// // 抽样输出归并结果
	// int64_t *mem = (int64_t *) malloc(MemSize*8);
	// unsigned short fileBlock = 8;
	// unsigned short numsPrint = 10;
	// NumPrint(mem, filenameOut, fileBlock, numsPrint);
	// free(mem);


	// free...

	return 0;
} // https://github.com/slDeng1003

