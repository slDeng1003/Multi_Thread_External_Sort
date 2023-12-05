#include "Loserheap.h"
#include <iostream>

using namespace std;

// 最小堆
Loserheap::Loserheap(int64_t *a){
	int i = 0;
	int k = 0;
	// k记录最小数字的index
	for(i = 1;i<16;i++) {
		if (a[i]<a[k]) k = i;
	}

	for (i=0;i<16;i++) {
		leaves[i] = a[i];
	}

	for (i=0;i<16;i++) {
		heap[i] = k;
	}

	for (i=15;i>=0;i--) {
		adjust(i,leaves[i]);
	}

	for (i=0;i<16;i++) {
		cout<<heap[i]<<":"<<leaves[heap[i]]<<endl;
	}
	cout<<endl;

	for (i=0;i<16;i++) {
		cout<<leaves[i]<<endl;
	}

}

int Loserheap::getindex(){
	return heap[0];
}

// 获取最小堆中堆顶数字，即最小数字
int64_t Loserheap::getwinner(){
	return leaves[heap[0]];
}

int Loserheap::adjust(int i,int64_t insertnum){
	leaves[i] = insertnum;
	int parent = (i+16)/2;
	while (parent>0){
		// i记录最小数字index
		if (leaves[i] > leaves[heap[parent]]){
			int tmp = heap[parent];
			heap[parent] = i;
			i = tmp;
		}
		// parent = parent / 2;
		parent = (parent-1) / 2;
	}
	// 堆顶记录最小数字index
	heap[0] = i;
	return i;
}