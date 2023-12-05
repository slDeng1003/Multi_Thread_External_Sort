#include "KShotMergeMinHeap.h"
#include "DataIO.h"


// 判断是否K个文件中所有数字已经插入最小堆
bool KShotMergeMinHeap::IsAllFileIntoHeap(unsigned short K, int *fileNum, int64_t * headNumCnt) {
    bool allNumOutFlag = (fileNum[0] == headNumCnt[0]);
    for (int i=1; i<K; i++) {
        allNumOutFlag = (allNumOutFlag && (fileNum[i] == headNumCnt[i]) );
    }
    // cout << "*** allNumOutFlag: " << allNumOutFlag << endl;
    return allNumOutFlag;
}

// K路归并
// 待归并文件在main()中打开和关闭,其指针在main()中分配和释放
void KShotMergeMinHeap::KShotMerge(DataIO *dataio, unsigned short K, int64_t **MergeMemInbuf, int64_t *MergeMemOutbuf, FILE ** fPtrMerge, vector<string> filenameMergeVec, FILE * fPtrOut) {
    // 声明一个最小堆，元素类型为int
    priority_queue<int64_t, vector<int64_t>, greater <int64_t>> MinHeap;
    int fileNum[K]; // 记录k个文件中数字数量
    int64_t filePos[K]; // 记录k个文件当前索引位置
    int64_t InbufPos[K]; // 记录k个缓冲区当前索引位置
    int64_t headNum[K]; // 缓冲区中第一个数字,即头数
    int64_t headNumCnt[K]; // 记录k个文件各自取头数的次数
    int64_t MinHeadNum = modnum; // 最小头数
    int MinHeadNumBufIdx; // 最小头数对应输入缓冲区索引
    int64_t topNum; // 堆顶数字
    int64_t OutbufPos = 0; // 记录输出缓冲区当前索引位置
    int64_t numRead; // 记录累积从排序文件中读取的数字量
    int64_t numWrited = 0; // 记录累积写入输出文件的数字量

    int i;

    // memset(fileNum, 0, K); // 这样是错误的！K是字节数，不是索引数！
    memset(fileNum, 0, K * sizeof(int));
    memset(filePos, 0, K * sizeof(int64_t));
    memset(InbufPos, 0, K * sizeof(int64_t));
    memset(headNum, 0, K * sizeof(int64_t));
    memset(headNumCnt, 0, K * sizeof(int64_t));

    // 将待归并文件读入输入缓冲区
    // 若某个文件读完了,其数据不能再读取,否则会越界
    for (i=0; i<K; i++) {
        fileNum[i] = dataio->GetFileNums(filenameMergeVec[i]);
        if (filePos[i] < fileNum[i]) {
            numRead = dataio->ReadUnmergeData(MergeMemInbuf[i], fPtrMerge[i], filePos[i], fileNum[i]);
            filePos[i] += numRead;
        }
    }
    // 初始化最小堆:把k个输入缓冲区中第一个数字插入最小堆
    for(i=0; i<K; i++) {
        MinHeap.push(MergeMemInbuf[i][0]);
        InbufPos[i]++; // 位置后移
        headNumCnt[i]++; // 取头数次数增加
    }

    // 若某个文件读完了,缓冲区也空了,其数据不能再读取,否则会越界
    while ( !IsAllFileIntoHeap(K, fileNum, headNumCnt)) {
        // 比较K个输入缓冲区中头数,选出最小头数,插入最小堆
        MinHeadNum = modnum; // 最小头数重置初始值
        for (i=0; i<K; i++) {
            // 若某个文件所有数字都写入堆中，则不能再读取残留在堆中数字
            if (!(fileNum[i] == headNumCnt[i])) {
                headNum[i] = MergeMemInbuf[i][InbufPos[i]];
                // cout << "headNum[" << i << "]: " << headNum[i] << endl;
                if (headNum[i] < MinHeadNum) {
                    MinHeadNum = headNum[i];
                    MinHeadNumBufIdx = i;
                }
            }
        }

        MinHeap.push(MinHeadNum);
        InbufPos[MinHeadNumBufIdx]++; // 插入最小头数后,对应缓冲区头指针后移
        headNumCnt[MinHeadNumBufIdx]++; // 该缓冲区对应的文件取头数次数增加
        
        // 弹出堆顶数字,放入输出缓冲区
        topNum = MinHeap.top();

        MergeMemOutbuf[OutbufPos] = topNum;
        OutbufPos++;
        MinHeap.pop();
        // 若输出缓冲区已满,写入输出文件并重置输出缓冲区位置
        if (OutbufPos == MergeOutBufSizeBlock) {
            numWrited += dataio->WriteMergeData(MergeMemOutbuf, fPtrOut, OutbufPos);
            OutbufPos = 0;
            cout << "--- Output buffer is full, " << numWrited << " nums (accumulated) writen to outfile" << endl;
        }

        // 判断最小头数对应的输入缓冲区是否全部取完,若全部取完,需要重新从文件读取，并重置输入缓冲区指针
        // 若文件中所有数字已经读入缓冲区中，无需再次读取
        if ((InbufPos[MinHeadNumBufIdx] == MergeInBufSizeBlock) && !(fileNum[MinHeadNumBufIdx] == filePos[MinHeadNumBufIdx]) ) {
            cout << "+++ All num in input buffer[" << MinHeadNumBufIdx << "] is writen,  reload num from file." << endl;
            numRead = dataio->ReadUnmergeData(MergeMemInbuf[MinHeadNumBufIdx], fPtrMerge[MinHeadNumBufIdx], filePos[MinHeadNumBufIdx], fileNum[MinHeadNumBufIdx]);
            filePos[MinHeadNumBufIdx] += numRead;
            InbufPos[MinHeadNumBufIdx] = 0;
        }
    }
    cout << "======== merge loop is over ========" << endl;

    // 若输出缓冲区不为空,写入文件
    if (OutbufPos != 0) {
        cout << "!!!!! OutBuffer is not empty (after loop), writing it to file!" << endl;
        numWrited += dataio->WriteMergeData(MergeMemOutbuf, fPtrOut, OutbufPos);
    }

    cout << "======== All num merged ========" << endl;
    for (i=0; i<K; i++) {
        cout <<"fileNum[" << i << "]: " << fileNum[i] << ", filePos[" << i << "]: " << filePos[i] << endl;
    }
    for (i=0; i<K; i++) {
        cout <<"headNumCnt[" << i << "]: " << headNumCnt[i] << endl;
    }
    cout << "Nums write to out file: " << numWrited << endl;

}