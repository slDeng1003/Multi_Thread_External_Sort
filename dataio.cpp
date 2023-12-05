#include "dataio.h"

using namespace std;

// 创建文件，用于存储未排序数字，允许读和写
void RandNumFileGenerate (int64_t * mem, FILE **fPtrUnsort) {
	// 为fPtrUnsort（指针数组）指向的未排序文件生成随机数字
	if (!RandNumGenerator(mem,fPtrUnsort))
		cout<<"Failed to generate numbers"<<endl;
	else
		cout<<"Success to generate numbers"<<endl;
}

// 获取文件中int_64t数字的数量
int GetFileNums(string filename) {
	ifstream ifs(filename, std::ios::binary | std::ios::in);
	if (!ifs.is_open())
	{
        cout << "file not open!" << endl;
		return 0;
	}
	ifs.seekg(0, std::ios::end);
	int fileNums = (ifs.tellg())/8;
	ifs.seekg(0, std::ios::beg);
    cout << "fileNums of " << filename << ": " << fileNums  << " Numbers"<< endl;
	return fileNums;
}

// 将未排序文件读入内存
int ReadUnsortData(int64_t* SortMemSingle, FILE * fPtrUnsortSingle, int64_t fileNums){
	int64_t nRead=0;
	// cout << "SEEK_CUR before fread: " << ftell(fPtrUnsortSingle)/8 << " Numbers" << endl;
	nRead = fread(SortMemSingle, sizeof(int64_t), MemSizeBlock, fPtrUnsortSingle);
	// cout << "SEEK_CUR after fread: " << ftell(fPtrUnsortSingle)/8 << " Numbers" << endl;
	fseek(fPtrUnsortSingle,0,SEEK_SET);
	cout << "nRead from file: " << nRead << endl;
	
	if (nRead == 0)
		return 0;
	else if (nRead == fileNums)
		return 1;
	else
		return -1;
}

// 将排序后数据写入文件
int WriteSortData(int64_t* SortMemSingle, FILE * fPtrSortSingle, int64_t fileNums){
	int64_t nWrited=0;
	nWrited = fwrite(SortMemSingle,sizeof(int64_t),fileNums,fPtrSortSingle);
	fseek(fPtrSortSingle,0,SEEK_SET);
	cout << "nWrited to file: " << nWrited << endl;
	if (nWrited == 0)
		return 0;
	else if (nWrited == fileNums)
		return 1;
	else
		return -1;
}


// 适用于单个线程，每次给单个文件排序，将排序结果写入新文件
void RandDataFileSort(int64_t * SortMemSingle, string filenameUnsortSingle, FILE * fPtrUnsortSingle, string filenameSortSingle, FILE *fPtrSortSingle) {
	int fileNums = GetFileNums(filenameUnsortSingle);
	// 读取未排序文件
	if(1 != ReadUnsortData(SortMemSingle, fPtrUnsortSingle, fileNums))
		cout<<"Reading unsorted data error"<<endl;
	fclose(fPtrUnsortSingle);

	// 排序
	cout << "======== start sort for " << filenameUnsortSingle << endl;
	sort(SortMemSingle, SortMemSingle + fileNums);
	cout << filenameUnsortSingle << " sort over ========" << endl;
	// 将排序后数据写入新文件
	fPtrSortSingle = fopen(filenameSortSingle.data(), "wb+");
	if(1 != WriteSortData(SortMemSingle, fPtrSortSingle, fileNums))
		cout<<"Writing sorted data error"<<endl;
	fclose(fPtrSortSingle);

}


// 将排序结果分为8个内存块输出，每个内存块的前30个结果输出
bool SortedResultPrint(int64_t*mem, FILE *fPtrOut, unsigned short fileBlock, unsigned short numsPrint) {
    int64_t i = 0,j = 0;
    int64_t nReaded=0; // 记录单个文件的数据量
    int64_t nSize=0; // 记录所有文件的数据量
    int64_t tmp =0;
    int64_t cnt=0; // 记录次数

    // 读取排序文件的数据
	// cout << "SEEK_CUR: " << ftell(fPtrOut)/8 << " Numbers" << endl;
    nReaded = fread(mem,sizeof(int64_t), MemSizeBlock/8, fPtrOut);
    nSize = nSize + nReaded;
    // 输出每个文件前numsPrint个数，用以比较排序结果
    cout << "========== showing the sorting results " 
    << "form index [" << i << " ] "
    << "to [ " << i+numsPrint << " ] "
    << "==========" << endl;

    for (i = 0; i < MemSizeBlock; i++){
        if (i<numsPrint) {
            cout << "index " << i << ": " << mem[i] << endl;
        }
        cnt++;
    }
    cout << "========== end ==========" << endl;

    for (j=1;j<fileBlock;j++){
		// cout << "SEEK_CUR: " << ftell(fPtrOut)/8 << " Numbers" << endl;
        nReaded = fread(mem,sizeof(int64_t),MemSizeBlock/8,fPtrOut);
        nSize = nSize + nReaded;
        cout << "========== showing the sorting results "
        << "from index [ " << j << " * " << MemSizeBlock/1024/1024 << " * 1024 * 1024 ] " 
        << "to [ " << j << " * " << MemSizeBlock/1024/1024 << " * 1024 * 1024 + " << numsPrint << " ] "
        << "==========" << endl;
        for (i = 0; i < MemSizeBlock; i++){
            if (i<numsPrint) {
                cout << "index " << j * MemSizeBlock + i << ": " << mem[i] << endl;
            }
            cnt++;
        }
        cout << "========== end ==========" << endl;
    }
	// cout << "SEEK_CUR: " << ftell(fPtrOut)/8 << " Numbers" << endl;
    
    // 若输出文件中总数据量和设定的外部磁盘大小一致，则所有数据排序成功
    if (nSize == ExSizeBlock && cnt+1 == ExSizeBlock)
        return true;
    else
        return false;
}

// 从排序后文件中读取MergeSizeBlock(8M)大小数据到内存mem
int64_t call(int64_t* addr,FILE* fPtr){
	int64_t nReaded = 0;
	nReaded = fread(addr,sizeof(int64_t), MemSizeBlock,fPtr);
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
	nWrited = fwrite(addr,sizeof(int64_t), MemSizeBlock, fPtr);
	if (nWrited == 0)
		return 0;
	else if (nWrited == MemSizeBlock)
		return 1;
	else
		return -1;
} // https://github.com/slDeng1003