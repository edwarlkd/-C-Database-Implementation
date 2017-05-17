#include <iostream>
#include <cstring>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

#include "PairingHeap.h"

using namespace std;

int main (int argc, char* argv[]) {
	PairingHeap<int> heap;

	srand(time(NULL)); int num = 1000;
	for(int i = 0; i < num; i++) {
		int randInt = rand()%num + 1;
		cout << "insert " << randInt << "... " << flush;
		heap.insert(randInt);
		cout << "min: " << heap.findMin() << endl;
	}
}