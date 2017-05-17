#ifndef _QUERY_OPTIMIZER_H
#define _QUERY_OPTIMIZER_H

#include "Schema.h"
#include "Catalog.h"
#include "ParseTree.h"
#include "Comparison.h"
#include "RelOp.h"

#include <string>
#include <vector>
#include <unordered_map>

typedef unsigned long long unll;

using namespace std;


// data structure used by the optimizer to compute join ordering
struct OptimizationTree {
	// list of tables joined up to this node
	vector<string> tables;
	// number of tuples in each of the tables (after selection predicates)
	vector<int> tuples;
	// number of tuples at this node
	int noTuples;

	// connections to children and parent
	OptimizationTree* parent;
	OptimizationTree* leftChild;
	OptimizationTree* rightChild;
};
struct tupleValue {
	unsigned long long size;
	unsigned long long cost;
	OptimizationTree *order;
	Schema schema;
};
class QueryOptimizer {
private:
	Catalog* catalog;

	std::unordered_map <string, tupleValue> tableMap;
	AndList* predicate;
	TableList* tables;
	unsigned long long permsize;

public:
	QueryOptimizer(Catalog& _catalog);
	virtual ~QueryOptimizer();

	void Optimize(TableList* _tables, AndList* _predicate, OptimizationTree* _root);
	unll PushDownSelections(string &tblname);
	void TablePairInitialization();
	void Partition(int N, vector<string> &tables);
	vector<string> fillVector(vector<string> &vec, vector<int> &idx, int x, int y);
	string GetKey(vector<string> tableNames);
	void CalcPermutations(int n, vector<vector<int>> &allPerms);
	void heapPermutation(vector<vector<int>> &allPerms, vector<int> &a, int size, int n);
	unll Estimate_Join_Cardinality(Schema &sch1, Schema &sch2, unll tblsize);
	unll factorial(int n);

};

#endif // _QUERY_OPTIMIZER_H
