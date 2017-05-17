#ifndef _TWO_WAY_LIST_C
#define _TWO_WAY_LIST_C

#include "TwoWayList.h"
#include "Swapify.cc"

#include <cstdlib>
#include <iostream>

using namespace std;


// basic constructor function
template <class Type>
TwoWayList <Type> :: TwoWayList () {
	// allocate space for the header
	list = new Header;

	// set up the initial values for an empty list
	list->first = new Node;
	list->last = new Node;
	list->current = list->first;
	list->leftSize = 0;
	list->rightSize = 0;
	list->first->next = list->last;
	list->last->previous = list->first;
}

// basic destructor function
template <class Type>
TwoWayList <Type> :: ~TwoWayList () {
	MoveToStart ();
	while (RightLength ()) {
		Type temp;
		Remove (temp);
	}

	// kill all the nodes
	for (int i = 0; i <= list->leftSize + list->rightSize; i++) {
		list->first = list->first->next;
		delete list->first->previous;
	}
	delete list->first;

	// kill the header
	delete list;
}

// swap operator
template <class Type> void
TwoWayList <Type> :: Swap (TwoWayList& List) {
	Header *temp = List.list;
	List.list = list;
	list = temp;
}

// copy from operator
template <class Type> void
TwoWayList <Type> :: CopyFrom (TwoWayList& List) {
	// first clean the current content of the list
	MoveToStart();
	while (!AtEnd()) {
		Remove(Current());
	}

	// then copy the elements from the input list
	List.MoveToStart();
	while (!List.AtEnd()) {
		Type element;
		element.CopyFrom(List.Current());
		Append(element);

		List.Advance();
	}
}

// make the first node the current node
template <class Type> void
TwoWayList <Type> :: MoveToStart () {
	list->current = list->first;
	list->rightSize += list->leftSize;
	list->leftSize = 0;
}

// make the first node the current node
template <class Type> void
TwoWayList <Type> :: MoveToFinish () {
	list->current = list->last->previous;
	list->leftSize += list->rightSize;
	list->rightSize = 0;
}

// determine the number of items to the left of the current node
template <class Type> int
TwoWayList <Type> :: LeftLength () {
	return (list->leftSize);
}

// determine the number of items to the right of the current node
template <class Type> int
TwoWayList <Type> :: RightLength () {
	return (list->rightSize);
}

template <class Type> int
TwoWayList <Type> :: Length () {
	return (list->leftSize+list->rightSize);
}

template <class Type> bool
TwoWayList <Type> :: AtStart () {
	return (list->leftSize == 0);
}


template <class Type> bool
TwoWayList <Type> :: AtEnd () {
	return (list->rightSize == 0);
}

// swap the right sides of two lists
template <class Type> void
TwoWayList <Type> :: SwapRights (TwoWayList & List) {
	// swap out everything after the current nodes
	Node *left_1 = list->current;
	Node *right_1 = list->current->next;
	Node *left_2 = List.list->current;
	Node *right_2 = List.list->current->next;

	left_1->next = right_2;
	right_2->previous = left_1;
	left_2->next = right_1;
	right_1->previous = left_2;

	// set the new endpoints
	Node *temp = list->last;
	list->last = List.list->last;
	List.list->last = temp;

	int tempint = List.list->rightSize;
	List.list->rightSize = list->rightSize;
	list->rightSize = tempint;
}

// swap the leftt sides of the two lists
template <class Type> void
TwoWayList <Type> :: SwapLefts (TwoWayList & List) {
	// swap out everything after the current nodes
	Node *left_1 = list->current;
	Node *right_1 = list->current->next;
	Node *left_2 = List.list->current;
	Node *right_2 = List.list->current->next;

	left_1->next = right_2;
	right_2->previous = left_1;
	left_2->next = right_1;
	right_1->previous = left_2;

	// set the new front points
	Node *temp = list->first;
	list->first = List.list->first;
	List.list->first = temp;

	// set the new current nodes
	temp = list->current;
	list->current = List.list->current;
	List.list->current = temp;

	int tempint = List.list->leftSize;
	List.list->leftSize = list->leftSize;
	list->leftSize = tempint;
}

// move forwards through the list
template <class Type> void
TwoWayList <Type> :: Advance () {
	(list->rightSize)--;
	(list->leftSize)++;
	list->current = list->current->next;
}

// move backwards through the list
template <class Type> void
TwoWayList <Type> :: Retreat () {
	(list->rightSize)++;
	(list->leftSize)--;
	list->current = list->current->previous;
}

// insert an item at the current position
template <class Type> void
TwoWayList <Type> :: Insert (Type & Item) {
	Node *temp = new Node;
	Node *left = list->current;
	Node *right = list->current->next;

	left->next = temp;
	temp->previous = left;
	temp->next = right;
	right->previous = temp;

	temp->data.Swap (Item);

	list->rightSize += 1;
}

template <class Type> void
TwoWayList <Type> :: Append (Type & Item) {
	MoveToFinish();
	Insert(Item);
}

template <class Type> Type&
TwoWayList <Type> :: Current () {
	return list->current->next->data;
}

// remove an item from the current position
template <class Type> void
TwoWayList <Type> :: Remove (Type & Item) {
	Node *temp = list->current->next;
	list->current->next = temp->next;
	temp->next->previous = list->current;

	Item.Swap (temp->data);

	delete temp;

	(list->rightSize)--;
}

// redefine operator << for printing
template <class Type> ostream&
operator<<(ostream& output, const TwoWayList<Type>& _list) {
	TwoWayList<Type> newObject;
	newObject.Swap(const_cast<TwoWayList<Type>&>(_list));

	output << "(";
	for (newObject.MoveToStart(); !newObject.AtEnd(); newObject.Advance()) {
		if (!newObject.AtStart()) {
			output << ", ";
		}

		output << newObject.Current();
	}
	output << ") : " << newObject.Length();

	newObject.Swap(const_cast<TwoWayList<Type>&>(_list));

	return output;
}

#endif
