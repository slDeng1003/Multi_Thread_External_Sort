#include "NumberControler.h"

using namespace std;

// 哈希函数，用于生成随机数
int64_t HashFunc(int64_t X) {
    return (3 * X + 5) % modnum;
}

// 生成未排序的随机数据，将数据分为16个相同大小的文件存储到磁盘
// 每个文件大小均为 1GB，共生成16GB文件
// 注：此处GB计算方式为1024*，非计算机1000*，故显示大小为1.1GB。实际上字节数是相同的。
// 调用：int64_t HashFunc(int64_t X)
bool RandNumGenerator(int64_t* mem, FILE ** fPtrUnsort) {
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

	for (j=1;j<16;j++){
		mem[0] = HashFunc(mem[MemSizeBlock-1]);
		for (i = 1;i < MemSizeBlock;i++)
			mem[i] = HashFunc(mem[i-1]);
		nWrited = fwrite(mem,sizeof(int64_t),MemSizeBlock,fPtrUnsort[j]); // 将数据写入文件
        fseek(fPtrUnsort[j],0,SEEK_SET); // 写入后重置文件指针
		nSize = nSize + nWrited;
	}

    // 若生成数据的总量和设定的外部磁盘大小一致，则数据生成成功
	if (nSize == ExSizeBlock)
		return true;
	else
		return false;
} // https://github.com/slDeng1003