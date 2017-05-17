#ifndef _HEAP_DATA_STRUCTURE_H
#define _HEAP_DATA_STRUCTURE_H
#include "CompositeKey.h"
using namespace std;
#include "vector"
using namespace std;

// template <class Type>
class Heapdatastructure
{
private:
    vector<CompositeKey> _vector;


public:
    Heapdatastructure();
    // Heapdatastructure(int* array, int length);
    void Insert(CompositeKey newValue);
    CompositeKey GetMin();
    void DeleteMin();
    void BubbleDown(int index);
    void BubbleUp(int index);
    void Heapify();
};


#endif