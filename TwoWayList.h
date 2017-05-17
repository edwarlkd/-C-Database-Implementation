#ifndef _TWO_WAY_LIST_H
#define _TWO_WAY_LIST_H

// Basic doubly linked list
// Type requires Swap

#include "Swap.h"
#include "Swapify.h"
#include "Keyify.h"

using namespace std;


template <class Type>
class TwoWayList {

public:
	// type definitions to make our life easier
	typedef Type element;

	// basic constructor function
	TwoWayList ();

	// destructor function
	virtual ~TwoWayList ();

	// swap operator
	void Swap (TwoWayList& List);

	// get the content from another list (without destroying it)
	void CopyFrom(TwoWayList& other);

	// add to current pointer position
	void Insert (Type & Item);

	// add at the end of the list
	void Append (Type & Item);

	// look at the current item
	Type& Current ();

	// remove from current position
	void Remove (Type & Item);

	// move the current pointer position backward through the list
	void Retreat ();

	// move the current pointer position forward through the list
	void Advance ();

	// operations to check the size of both sides
	int LeftLength ();
	int RightLength ();
	int Length();

	// operations to consult state
	bool AtStart ();
	bool AtEnd ();

	// operations to swap the left and right sides of two lists
	void SwapLefts (TwoWayList & List);
	void SwapRights (TwoWayList & List);

	// operations to move the the start of end of a list
	void MoveToStart ();
	void MoveToFinish ();

private:

	struct Node {
		// data
		Type data;
		Node *next;
		Node *previous;

		// constructor
		Node () : next (0), previous (0) {}
		virtual ~Node () {}
	};

	struct Header {
		// data
		Node* first;
		Node* last;
		Node* current;
		int leftSize;
		int rightSize;
	};

	// the list itself is pointed to by this pointer
	Header*list;

	// making the default constructor and = operator private so  the list
	// can only be propagated by Swap. Otherwise subtle bugs can appear
	TwoWayList(TwoWayList&);
	TwoWayList operator= (TwoWayList&);
};

typedef TwoWayList<SwapDouble> DoubleContainer;
typedef TwoWayList<SwapInt> IntContainer;
typedef TwoWayList<SwapString> StringContainer;

#endif
