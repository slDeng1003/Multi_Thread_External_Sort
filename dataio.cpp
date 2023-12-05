#include "dataio.h"

using namespace std;

// 内部排序算法：使用<algorithm>库中qsort()函数
// qsort()原型: void qsort(void *base, int nelem, int width, int (*fcmp)(const void *,const void *));
// qsort()参数含义：
// 1 void *base: 待排序数组首地址
// 2 int nelem: 数组中待排序元素数量
// 3 int width: 各元素的占用空间大小
// 4 int (*fcmp)(const void *,const void *): 指向“比较函数"的指针，用于确定排序的顺序

// 为externalsort.cpp中使用的qsort()函数定义"比较函数"（第4个参数）
// 比较函数，即比较两个元素大小关系的函数
// 这里比较64位无符号数的大小关系
int compare(const void* a,const void* b){
	if (*(int64_t*)a - *(int64_t*)b > 0) // a > b
		return 1;
	else if (*(int64_t*)a - *(int64_t*)b < 0) // a < b
		return -1;
	else 
		return 0; // a == b
}

// 将未排序文件读入内存
int ReadUnsortData(int64_t* mem,FILE * fPtrUnsort){
	int64_t nReaded=0;
	nReaded = fread(mem,sizeof(int64_t),MemSizeBlock,fPtrUnsort);
	fseek(fPtrUnsort,0,SEEK_SET);
	
	if (nReaded == 0)
		return 0;
	else if (nReaded == MemSizeBlock)
		return 1;
	else
		return -1;
}

// 将排序后数据写入文件
int WriteSortData(int64_t* mem,FILE * fPtrSort){
	int64_t nWrited=0;
	nWrited = fwrite(mem,sizeof(int64_t),MemSizeBlock,fPtrSort);
	fseek(fPtrSort,0,SEEK_SET);
	if (nWrited == 0)
		return 0;
	else if (nWrited == MemSizeBlock)
		return 1;
	else
		return -1;
}


// 创建文件，用于存储未排序数字，允许读和写
void RandNumFileGenerate (int64_t * mem, FILE **fPtrUnsort) {
	// 为fPtrUnsort（指针数组）指向的未排序文件生成随机数字
	if (!RandNumGenerator(mem,fPtrUnsort))
		cout<<"Failed to generate numbers"<<endl;
	else
		cout<<"Success to generate numbers"<<endl;
}


void RandDataFileSort(int64_t * SortMem, FILE ** fPtrUnsort, FILE ** fPtrSort) {
	// 读取未排序文件，为其排序，然后重新写入该文件
	int i;
	string filenameSort[16];
	for (i = 0; i<16; i++) {
		filenameSort[i] = string("./data_sort")+to_string(i);
	}

	for(i = 0; i<16; i++){
		// 将未排序文件读入内存
		if(1 != ReadUnsortData(SortMem,fPtrUnsort[i]))
			cout<<"Reading unsorted data error"<<endl;
		fclose(fPtrUnsort[i]);
		
		// 后面要改为多线程排序
		// 为内存中数据排序
		qsort((void*)SortMem, MemSizeBlock,sizeof(int64_t),compare);

		// 将排序后数据写入新文件
		fPtrSort[i] = fopen(filenameSort[i].data(), "wb+");
		if(1 != WriteSortData(SortMem, fPtrSort[i]))
			cout<<"Writing sorted data error"<<endl;
		fclose(fPtrSort[i]);
	}
}

// 将排序结果分为16个内存块输出，每个内存块的前30个结果输出
bool SortedResultPrint(int64_t*mem, FILE *fPtrOut) {
    int64_t i = 0,j = 0;
    int64_t nReaded=0; // 记录单个文件的数据量
    int64_t nSize=0; // 记录所有文件的数据量
    int64_t tmp =0;
    int64_t cnt=0; // 记录次数

    // 读取排序文件的数据
	cout << "SEEK_CUR: " << ftell(fPtrOut)/8 << " Bytes" << endl;
    nReaded = fread(mem,sizeof(int64_t),MemSizeBlock/8,fPtrOut);
    nSize = nSize + nReaded;
    // 输出每个文件前30个数，用以比较排序结果
    cout << "========== showing the sorting results " 
    << "form index [" << i << " ] "
    << "to [ " << i+29 << " ] "
    << "==========" << endl;

    for (i = 0; i < MemSizeBlock; i++){
        if (i<30) {
            cout << "index " << i << ": " << mem[i] << endl;
        }
        cnt++;
    }
    cout << "========== end ==========" << endl;

    for (j=1;j<8;j++){
		cout << "SEEK_CUR: " << ftell(fPtrOut)/8 << " Bytes" << endl;
        nReaded = fread(mem,sizeof(int64_t),MemSizeBlock/8,fPtrOut);
        nSize = nSize + nReaded;
        cout << "========== showing the sorting results "
        << "from index [ " << j << " * " << MemSizeBlock/1024/1024 << " * 1024 * 1024 ] " 
        << "to [ " << j << " * " << MemSizeBlock/1024/1024 << " * 1024 * 1024 + 29 ] "
        << "==========" << endl;
        for (i = 0; i < MemSizeBlock; i++){
            if (i<30) {
                cout << "index " << j * MemSizeBlock + i << ": " << mem[i] << endl;
            }
            cnt++;
        }
        cout << "========== end ==========" << endl;
    }
	cout << "SEEK_CUR: " << ftell(fPtrOut)/8 << " Bytes" << endl;
    
    // 若输出文件中总数据量和设定的外部磁盘大小一致，则所有数据排序成功
    if (nSize == ExSizeBlock && cnt+1 == ExSizeBlock)
        return true;
    else
        return false;
}

// 从排序后文件中读取MergeSizeBlock(8M)大小数据到内存mem
int64_t call(int64_t* addr,FILE* fPtr){
	int64_t nReaded = 0;
	nReaded = fread(addr,sizeof(int64_t),MergeSizeBlock,fPtr);
//	fseek(fPtr,0,SEEK_SET);
/*	if (nReaded == 0)
		return 0;
	else if (nReaded == MergeSizeBlock)
		return 1;
	else
		return -1;
*/
	return nReaded;
}

int send(int64_t* addr,FILE * fPtr){
	int64_t nWrited=0;
	nWrited = fwrite(addr,sizeof(int64_t),MergeSizeBlock,fPtr);
	if (nWrited == 0)
		return 0;
	else if (nWrited == MergeSizeBlock)
		return 1;
	else
		return -1;
} // https://github.com/slDeng1003