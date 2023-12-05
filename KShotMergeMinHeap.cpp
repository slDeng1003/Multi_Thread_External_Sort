#include "KShotMergeMinHeap.h"
#include "DataIO.h"


// 判断是否K个文件中所有数字已经插入最小堆
bool KShotMergeMinHeap::IsAllFileIntoHeap(unsigned short K, int *fileNum, int64_t * pushNumCnt) {
    bool allNumOutFlag = (fileNum[0] == pushNumCnt[0]);
    for (int i=1; i<K; i++) {
        allNumOutFlag = (allNumOutFlag && (fileNum[i] == pushNumCnt[i]) );
    }
    // cout << "*** allNumOutFlag: " << allNumOutFlag << endl;
    return allNumOutFlag;
}

// K路归并
// 待归并文件在main()中打开和关闭,其指针在main()中分配和释放
void KShotMergeMinHeap::KShotMerge(DataIO *dataio, unsigned short K, int64_t **MergeMemInbuf, int64_t *MergeMemOutbuf, FILE ** fPtrMerge, vector<string> filenameMergeVec, FILE * fPtrOut) {
    // 声明一个最小堆，元素类型为struct
    struct Node
    {
        int64_t num;
        int bufIndex; // 数字对应输入缓冲区的序号
    };
    // 自定义比较函数
    struct cmp {
        bool operator ()(const Node& a, const Node& b)
        {
            return a.num > b.num; //将num的值由小到大排列，形成Node的小根堆
        }
    };
    
    priority_queue<Node, vector<Node>, cmp> MinHeap;
    int fileNum[K]; // 记录k个文件中数字数量
    int64_t filePos[K]; // 记录k个文件当前索引位置
    int64_t InbufPos[K]; // 记录k个缓冲区当前索引位置
    int64_t pushNumCnt[K]; // 记录k个文件各自向堆中插入数字的次数
    Node topNode; // 堆顶节点
    int64_t topNum; // 堆顶数字
    int topIndex; // 堆定数字对应输入缓冲区序号
    int64_t OutbufPos = 0; // 记录输出缓冲区当前索引位置
    int64_t numRead; // 记录累积从排序文件中读取的数字量
    int64_t numWrited = 0; // 记录累积写入输出文件的数字量

    int i;

    // memset(fileNum, 0, K); // 这样是错误的！第三个参数是字节数，不是索引数！
    memset(fileNum, 0, K * sizeof(int));
    memset(filePos, 0, K * sizeof(int64_t));
    memset(InbufPos, 0, K * sizeof(int64_t));
    memset(pushNumCnt, 0, K * sizeof(int64_t));

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
        MinHeap.push(Node{MergeMemInbuf[i][0], i});
        InbufPos[i]++; // 位置后移
        pushNumCnt[i]++; // 插入次数增加
    }

    // 若某个文件读完了,缓冲区也空了,其数据不能再读取,否则会越界
    while ( !(MinHeap.empty())) {
        // 若输出缓冲区已满,写入输出文件并重置输出缓冲区位置
        if (OutbufPos == MergeOutBufSizeBlock) {
            numWrited += dataio->WriteMergeData(MergeMemOutbuf, fPtrOut, OutbufPos);
            OutbufPos = 0;
            cout << "--- Output buffer is full, " << numWrited << " nums (accumulated) writen to outfile" << endl;
        }
        // 弹出堆顶数字,放入输出缓冲区
        topNode = MinHeap.top();
        topNum = topNode.num;
        topIndex = topNode.bufIndex;
        MergeMemOutbuf[OutbufPos] = topNum;
        OutbufPos++;
        MinHeap.pop();


        // 若文件中还有数字未插入最小堆（输入缓冲区未取完），则继续插入
        if ( !IsAllFileIntoHeap(K, fileNum, pushNumCnt)) {
            // 如果堆顶节点不是文件最后一个数字，才能继续push
            if (fileNum[topIndex] != filePos[topIndex]) {
                // 将刚刚弹出的堆顶数字对应输入缓冲区的下一个数字插入最小堆，确保最小堆中包含所有缓冲区中的第一个数字
                MinHeap.push(Node{MergeMemInbuf[topIndex][InbufPos[topIndex]], topIndex});
                InbufPos[topIndex]++; // 插入后,对应缓冲区头指针后移
                pushNumCnt[topIndex]++; // 该缓冲区对应的文件插入次数增加
            }

            // 判断堆顶数字对应的输入缓冲区是否全部取完,若全部取完,需要重新从文件读取，并重置输入缓冲区指针
            // 若文件中所有数字已经读入缓冲区中，无需再次读取
            if ((InbufPos[topIndex] == MergeInBufSizeBlock) && !(fileNum[topIndex] == filePos[topIndex]) ) {
                cout << "+++ All num in input buffer[" << topIndex << "] is writen,  reload num from file." << endl;
                numRead = dataio->ReadUnmergeData(MergeMemInbuf[topIndex], fPtrMerge[topIndex], filePos[topIndex], fileNum[topIndex]);
                filePos[topIndex] += numRead;
                InbufPos[topIndex] = 0;
            }
        }
    }
    cout << "======== merge loop is over ========" << endl;

    // 若循环结束后，输出缓冲区不为空,写入文件
    if (OutbufPos != 0) {
        cout << "!!!!! OutBuffer is not empty (after loop), writing it to file!" << endl;
        numWrited += dataio->WriteMergeData(MergeMemOutbuf, fPtrOut, OutbufPos);
        OutbufPos = 0;
    }

    cout << "======== All num merged ========" << endl;
    for (i=0; i<K; i++) {
        cout <<"fileNum[" << i << "]: " << fileNum[i] << ", filePos[" << i << "]: " << filePos[i] << endl;
    }
    for (i=0; i<K; i++) {
        cout <<"pushNumCnt[" << i << "]: " << pushNumCnt[i] << endl;
    }
    cout << "Nums write to out file: " << numWrited << endl;

} // https://github.com/slDeng1003