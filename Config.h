#ifndef _CONFIG_H
#define _CONFIG_H

using namespace std;


/* This file contains global definitions used throughout the project.
 */

// page size in database file
#define PAGE_SIZE 131072

// pipe buffer size
#define PIPE_BUFFERSIZE 10000

// maximum number of AND in a predicate
#define MAX_ANDS 20

// maximum number of operators in a function
#define MAX_FUNCTION_DEPTH 100


/* source of comparison operand:
 * left relation in join
 * right relation in join
 * literal corresponds to constant
 */
enum Target {Left, Right, Literal};

// type of operand
enum Type {Integer, Float, String, Name};

// comparison operator
enum CompOperator {LessThan, GreaterThan, Equals};

// function operator
enum ArithOperator {PushInt, PushDouble, ToDouble, ToDouble2Down,
	IntUnaryMinus, IntMinus, IntPlus, IntDivide, IntMultiply,
	DblUnaryMinus, DblMinus, DblPlus, DblDivide, DblMultiply};

// file types
enum FileType {Heap, Sorted, Index};

#endif //_CONFIG_H
