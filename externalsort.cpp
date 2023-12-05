#include <iostream>
#include <stdlib.h>
#include <string>

#include "dataio.h"
#include "NumberControler.h"
#include "Loserheap.h"


using namespace std;

string data_orgin = "data_orgin.bin";
string data_out = "data_out.bin";

// 分配内存（用于生成随机数、存储排序后的随机数、写入排序数）
int64_t * MemoryAllocate(int64_t MemorySize) {
	int64_t *memory = (int64_t *) malloc(MemorySize); // 1GB
	if (memory == NULL) {
		cout << "memory is not allocated!" << endl;
		}
    else {
		cout << "memory allocate sucess" << endl;
	}

	return memory;
}

void NumPrint(int64_t *mem, string filename) {
	// 抽查排序结果并输出
	// int64_t *mem = (int64_t *) malloc(MemSize);
    FILE * fPtrOut = fopen(filename.data(),"rb");
    if(SortedResultPrint(mem,fPtrOut))
        cout<<"Sorted Results Printed."<<endl;
}

int main(){
	int64_t *SortMem = MemoryAllocate(MemSize); // 分配内存（用于排序）
	int64_t *mergeMem = MemoryAllocate(MergeSize); // 分配内存（用于归并）

	int64_t k,i,j =0;
	string filenameUnsort[16];
	for (i = 0; i<16; i++) {
		filenameUnsort[i] = string("./data_random")+to_string(i);
	}


	// 已生成随机数文件后，运行这几行
	// 指针数组，每个元素都指向一个未排序文件
	FILE ** fPtrUnsort = (FILE**)malloc(16*sizeof(FILE *));
	for (i = 0; i < 16; i++) {
		fPtrUnsort[i] = fopen(filenameUnsort[i].data(),"rb"); 
	}
	
	// // 生成随机数文件用，若随机数文件已经生成，无需运行这几行
	// FILE ** fPtrUnsort = (FILE**)malloc(16*sizeof(FILE *));
	// for (i = 0; i < 16; i++) {
	// 	fPtrUnsort[i] = fopen(filenameUnsort[i].data(),"wb+"); 
	// }
	// int64_t *NumGeneMem = MemoryAllocate(MemSize); // 分配内存（用于生成随机数）
	// RandNumFileGenerate(NumGeneMem, fPtrUnsort);
	
	// 为文件排序，并将排序结果重新写入新文件
	FILE ** fPtrSort = (FILE**)malloc(16*sizeof(FILE *));
	string filenameSort[16];
	for (i = 0; i<16; i++) {
		filenameSort[i] = string("./data_sort")+to_string(i);
	}
	RandDataFileSort(SortMem, fPtrUnsort, fPtrSort);

	for(i=0; i<16; i++){
		fPtrSort[i] = fopen(filenameSort[i].data(),"rb+");
	}

	// int64_t *mem = (int64_t *) malloc(MemSize);
	// NumPrint(mem, "./data_sort0");
	// NumPrint(mem, "./data_sort1");
	// NumPrint(mem, "./data_sort2");
	// NumPrint(mem, "./data_sort5");
	// NumPrint(mem, "./data_sort10");
	// NumPrint(mem, "./data_sort15");

	// return 0;
	
	// 打开输出文件(若不存在则创建)，允许读和写
	// 输出文件为二进制格式，用于存储排序后的所有数字
	FILE * fPtrOut = fopen("./data_out.bin","wb+"); //file Pointer to Output file
	if (fPtrOut == NULL) {
		cerr << "Open error (data_out.bin)!" << endl;
		return 0;
	}

	// 声明并赋予初始值
	int Exflag[16];
	int Memflag[16];
	int64_t length[16];
	memset(Exflag, 0, 16);
	memset(Memflag, 0, 16);
	memset(length, MergeSizeBlock, 16);

	// 分配内存（指针）
	int64_t **memPtr =(int64_t**)malloc(16*sizeof(int64_t *));
	// 指针指向?
	for(i = 0;i < 16; i++)
		memPtr[i] = SortMem + MergeSizeBlock*i; // MemSizeBlock/16=MergeSizeBlock

	// 从排序后文件中读取16组MergeSizeBlock(8M)大小的数据到内存mem
	for(i=0; i<16; i++){
		int64_t t = 0;
		t = call(memPtr[i],fPtrUnsort[i]);
		if (t == 0){
			Exflag[i] = 1;
			cout<<"Call Error or File EOF."<<endl;
		}
		else if (t < MergeSizeBlock){
			Exflag[i] = 1;
			cout <<"FILE EOF."<<endl;
		}
	}

	// 输出每个MergeSizeBlock中第0个数（头数）
	for (i=0; i<16; i++)
		cout<<memPtr[i][0]<<endl;

	// 比较16组Block中，每个Block头数的大小，用k记录具有最小头数Block的index
	int64_t a[16]={0};
	for (i=0; i<16; i++)
			a[i] = memPtr[i][0];
	k = 0;
	for(i = 1; i<16; i++)
		if (a[i]<a[k]) k = i;
	Loserheap lp(a);

	int64_t count = 0;
	int index;

	int64_t cnt[16]; // 为16个文件分别记录取头数的次数
	memset(cnt, 0, 16);
	mergeMem[0] = memPtr[k][0]; // 将16个文件中最小头数作为第一个数
	count++; // 记录归并次数，也就是把最小头数写入mergeMem的次数
	index = k;
	cnt[index] = 1;
	int64_t loop = 1;

	while(true){
		if(cnt[index] == length[index]){
			cout<<"memPtr "<<index<<" is empty!Please Call!"<<endl;
			length[index] = call(memPtr[index],fPtrUnsort[index]);
			if (length[index] == 0){
				Exflag[index] = 1;
				if (Exflag[0]==1 && Exflag[1]==1 && Exflag[2]==1 && Exflag[3]==1 && Exflag[4]==1 && Exflag[5]==1 && Exflag[6]==1 && Exflag[7]==1)//all are 1
					break;
				cout<<"File slice "<<index<<" is over!"<<endl;
				index = lp.adjust(index,1125899906842679); // ?
				mergeMem[loop] = lp.getwinner();
				count++;
				cnt[index]++;
				loop++;
				continue;
			}
			else{
				cnt[index] = 0;
				index = lp.adjust(index,memPtr[index][cnt[index]]);
				mergeMem[loop] = lp.getwinner();
				count++;
				cnt[index]++;
			}
		}
		else{
			index = lp.adjust(index,memPtr[index][cnt[index]]);
			mergeMem[loop] = lp.getwinner();
			count++;
			cnt[index]++;
		}
		loop++;

		// 若归并内存已满，则将归并结果写入输出文件
		if ( loop == MemSizeBlock ){
			loop = 0;
//			if(1 != send(mergeMem,fPtrOut))
            if(1 != WriteSortData(mergeMem,fPtrOut))
			cout<<"Send error"<<endl;
		}
	}

	cout<<count<<endl;
	fclose(fPtrOut);

	// // 抽查排序结果并输出
	// int64_t *mem = (int64_t *) malloc(MemSize);
	// string filename = "./data_out.bin";
	// NumPrint(mem, filename);

	// free...

	return 0;
}

