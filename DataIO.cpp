#include "DataIO.h"

// 哈希函数，用于生成随机数
int64_t DataIO::HashFunc(int64_t X) {
    return (3 * X + 5) % modnum;
}

// 生成未排序的随机数据，将数据分为fileNum个相同大小的文件存储到磁盘
// 注：此处GB计算方式为1024*，非计算机1000*，故OS中文件大小可能“不同”。实际上字节数是相同的。
// 调用：int64_t HashFunc(int64_t X)
void DataIO::RandNumFileGenerate(int64_t* mem, FILE ** fPtrUnsort) {
    int64_t i,j = 0;
	int64_t nWrited=0; // 记录单个文件的数据量
	int64_t nSize=0; // 记录所有文件的数据量

    // 每个文件生成MemSizeBlock(1/8G)个随机整数，共生成(1/8)*16=2G个64位随机整数
    // 每个64位随机整数占64/8=8 Bytes，故文件大小为2*(64/8)=16 GB
	mem[0] = ID;
	for (i = 1; i < MemSizeBlock; i++)
		mem[i] = HashFunc(mem[i-1]);	
	nWrited = fwrite(mem,sizeof(int64_t),MemSizeBlock,fPtrUnsort[0]); // 将数据写入文件
	fseek(fPtrUnsort[0],0,SEEK_SET); // 写入后重置文件指针
	nSize = nSize + nWrited;

	for (j=1; j<RandomFileNum; j++){
		mem[0] = HashFunc(mem[MemSizeBlock-1]);
		for (i = 1;i < MemSizeBlock;i++)
			mem[i] = HashFunc(mem[i-1]);
		nWrited = fwrite(mem,sizeof(int64_t),MemSizeBlock,fPtrUnsort[j]); // 将数据写入文件
        fseek(fPtrUnsort[j],0,SEEK_SET); // 写入后重置文件指针
		nSize = nSize + nWrited;
	}

    // 若生成数据的总量和设定的外部磁盘大小一致，则数据生成成功
	if (nSize == ExSizeBlock) {
		cout<<"Failed to generate numbers"<<endl;
	}
	else {
		cout<<"Success to generate numbers"<<endl;
	}
}

// 获取文件中int_64t数字的数量
int DataIO::GetFileNums(string filename) {
	ifstream ifs(filename, std::ios::binary | std::ios::in);
	if (!ifs.is_open())
	{
        cout << "file not open!" << endl;
		return 0;
	}
	ifs.seekg(0, std::ios::end);
	int fileNums = (ifs.tellg())/8;
	ifs.seekg(0, std::ios::beg);
	ifs.close();
    // cout << "fileNums of " << filename << ": " << fileNums  << " Numbers"<< endl;
	return fileNums;
}

// 将未排序文件读入内存
int DataIO::ReadUnsortData(int64_t* SortMemSingle, FILE * fPtrUnsortSingle, int64_t fileNums){
	int64_t nRead=0;
	// cout << "SEEK_CUR before fread: " << ftell(fPtrUnsortSingle)/8 << " Numbers" << endl;
	nRead = fread(SortMemSingle, sizeof(int64_t), MemSizeBlock, fPtrUnsortSingle);
	// cout << "SEEK_CUR after fread: " << ftell(fPtrUnsortSingle)/8 << " Numbers" << endl;
	fseek(fPtrUnsortSingle,0,SEEK_SET);
	// cout << "nRead from file: " << nRead << endl;
	
	if (nRead == 0)
		return 0;
	else if (nRead == fileNums)
		return 1;
	else
		return -1;
}

// 将排序后数据写入文件
int DataIO::WriteSortData(int64_t* SortMemSingle, FILE * fPtrSortSingle, int64_t fileNums){
	int64_t nWrited=0;
	nWrited = fwrite(SortMemSingle,sizeof(int64_t),fileNums,fPtrSortSingle);
	fseek(fPtrSortSingle,0,SEEK_SET);
	// cout << "nWrited to file: " << nWrited << endl;
	if (nWrited == 0)
		return 0;
	else if (nWrited == fileNums)
		return 1;
	else
		return -1;
}


// 适用于单个线程，每次给单个文件排序，将排序结果写入新文件
void DataIO::RandDataFileSort(int64_t * SortMemSingle, string filenameUnsortSingle, FILE * fPtrUnsortSingle, string filenameSortSingle, FILE *fPtrSortSingle) {
	int fileNums = GetFileNums(filenameUnsortSingle);
	// 读取未排序文件
	if(1 != ReadUnsortData(SortMemSingle, fPtrUnsortSingle, fileNums))
		cout<<"Reading unsorted data error"<<endl;
	fclose(fPtrUnsortSingle);

	// 排序
	cout << "======== start sort for " << filenameUnsortSingle << "=======" << endl;
	sort(SortMemSingle, SortMemSingle + fileNums);
	cout << "=======" << filenameUnsortSingle << " sort over ========" << endl;
	// 将排序后数据写入新文件
	fPtrSortSingle = fopen(filenameSortSingle.data(), "wb+");
	if(1 != WriteSortData(SortMemSingle, fPtrSortSingle, fileNums))
		cout<<"Writing sorted data error"<<endl;
	fclose(fPtrSortSingle);

}

// 将排序结果分为fileBlock个内存块输出，每个内存块的前numsPrint个结果输出
void DataIO::FileNumPrint(int64_t*mem, string filename, unsigned short fileBlock, unsigned short numsPrint) {
    FILE * fPtrPrint = fopen(filename.data(),"rb");
	int64_t i = 0,j = 0;
	int fileNum = GetFileNums(filename);
	int blockSize = fileNum/fileBlock;

	// 分块读取文件数据
    for (j=0; j<fileBlock; j++){
        fread(mem,sizeof(int64_t),blockSize, fPtrPrint);
        cout << "========== showing nums in " << filename
        << " from index [ " << j << " * " << blockSize << " ] " 
        << "to [ " << j << " * " << blockSize << " + " << numsPrint << " ] "
        << "==========" << endl;
        for (i = 0; i < blockSize; i++){
			if (i>=numsPrint) break;
            cout << "index " << j * blockSize + i << ": " << mem[i] << endl;
        }
        cout << "========== end ==========" << endl;
    }
    fclose(fPtrPrint);
}

// 读文件到输入缓冲区
int64_t DataIO::ReadUnmergeData(int64_t* MergeMemInBufSingle, FILE * fPtrMergeSingle, int filePos, int fileNums) {
	int64_t numRead=0;
    // 若文件中剩余数字多于缓冲区可容纳数字,缓冲区读满;若剩余数字少于,则读完剩余数字
	if ((fileNums-filePos) >= MergeInBufSizeBlock) { 
        numRead = fread(MergeMemInBufSingle, sizeof(int64_t), MergeInBufSizeBlock, fPtrMergeSingle);
        if(numRead != MergeInBufSizeBlock) {
            cout<<"Reading unmerged data error"<<endl;
        }
    }
    else {
        numRead = fread(MergeMemInBufSingle, sizeof(int64_t), fileNums-filePos, fPtrMergeSingle);
        if(numRead != fileNums-filePos) {
            cout<<"Reading unmerged data error"<<endl;
        }
    }
	cout << "*** Nums read from Sortfile to MergeMem input Buffer: " << numRead << " Nums" << endl;

    return numRead;
}

// 输出缓冲区数据写入文件
int64_t DataIO::WriteMergeData(int64_t* MergeMemOutbuf, FILE * fPtrOut, int64_t OutbufPos) {
	int64_t numWrited=0;
    // 若缓冲区已满,全部写入;若未满,则把剩余内容写入
    if (OutbufPos == MergeOutBufSizeBlock) {
        numWrited = fwrite(MergeMemOutbuf,sizeof(int64_t),MergeOutBufSizeBlock,fPtrOut);
    }
    else { // 缓冲区未满
        numWrited = fwrite(MergeMemOutbuf,sizeof(int64_t),OutbufPos,fPtrOut);
    }
	cout << "*** nWrited to Out file: " << numWrited << endl;
    return numWrited;
} // https://github.com/slDeng1003