
#include "Heapdatastructure.h"

// Minheap Constructor
Heapdatastructure::Heapdatastructure() {}

// Heapdatastructure::Heapdatastructure(int* array, int length) : _vector(length)
// {
//     for(int i = 0; i < length; ++i)
//     {
//         _vector[i] = array[i];
//     }

//     Heapify();
// }

void Heapdatastructure::Heapify()
{
    int length = _vector.size();
    for(int i=length-1; i>=0; --i)
    {
        BubbleDown(i);
    }
}

void Heapdatastructure::BubbleDown(int index)
{	
	// Initializing size of vector as length
    int length = _vector.size();
    // Setting left and right childs
    int leftChildIndex = 2*index + 1;
    int rightChildIndex = 2*index + 2;

    // Checking if leaf
    if(leftChildIndex >= length)
        return; //index is a leaf

    int minIndex = index;

    if(_vector[index] > _vector[leftChildIndex])
    {
        minIndex = leftChildIndex;
    }
    
    if((rightChildIndex < length) && (_vector[minIndex] > _vector[rightChildIndex]))
    {
        minIndex = rightChildIndex;
    }
    if(minIndex != index)
    {
        //need to swap
        CompositeKey temp = _vector[index];
        _vector[index] = _vector[minIndex];
        _vector[minIndex] = temp;
        BubbleDown(minIndex);
    }
}

void Heapdatastructure::BubbleUp(int index)
{
    if(index == 0)
        return;

    int parentIndex = (index-1)/2;

    if(_vector[parentIndex] > _vector[index])
    {
        CompositeKey temp = _vector[parentIndex];
        _vector[parentIndex] = _vector[index];
        _vector[index] = temp;
        BubbleUp(parentIndex);
    }
}

void Heapdatastructure::Insert(CompositeKey newValue)
{
    _vector.push_back(newValue);
    int length = _vector.size()-1;
    BubbleUp(length);
}

CompositeKey Heapdatastructure::GetMin()
{
    return _vector[0];
}
    
void Heapdatastructure::DeleteMin()
{
    int length = _vector.size();

    if(length == 0)
    {
        return;
    }
    
    _vector[0] = _vector[length-1];
    _vector.pop_back();

    BubbleDown(0);
}