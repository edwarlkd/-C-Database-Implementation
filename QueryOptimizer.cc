#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits.h>

#include "Schema.h"
#include "Comparison.h"
#include "QueryOptimizer.h"

using namespace std;


QueryOptimizer::QueryOptimizer(Catalog& _catalog) : catalog(&_catalog) {
}

QueryOptimizer::~QueryOptimizer() {
}

void QueryOptimizer::Optimize(TableList* _tables, AndList* _predicate,
	OptimizationTree* _root) {
	predicate = _predicate;
	tables = _tables;
	int numTables = 0;
	vector<string> tableNames;

	//Initializing tableMap from _tables
	for (TableList *tbl = tables; tbl != NULL; tbl = tbl->next, numTables++) {
		string tblname(tbl->tableName);
		tableNames.push_back(tblname);
		unll tblsize = PushDownSelections(tblname);
		tableMap[tblname].size = tblsize;
		tableMap[tblname].cost = 0;
		OptimizationTree *node = new OptimizationTree;
		node->tables.push_back(tblname);
		node->noTuples = tblsize;
		node->tuples.push_back(tblsize);
		node->leftChild = NULL;
		node->rightChild = NULL;
		tableMap[tblname].order = node;
	}

	if(numTables == 0) {
		_root = NULL;
	}
	else if(numTables == 1) {
		*_root = *tableMap[tables->tableName].order;
	}
	else if(numTables == 2) {
		TablePairInitialization();
		string table1 = tables->tableName;
		string table2 = tables->next->tableName;
		if (tableMap.find(table1 + table2) != tableMap.end()){
			*_root = *tableMap[table1+table2].order;
		}
		else {
			*_root = *tableMap[table2+table1].order;
		}
	}
	else {
		TablePairInitialization();
		Partition(numTables, tableNames);
		string tablekey = GetKey(tableNames);
		*_root = *tableMap[tablekey].order;
	}

}

unll QueryOptimizer::PushDownSelections(string &tblname) {
	Schema sch;
	Record rec;
	catalog->GetSchema(tblname, sch);
	tableMap[tblname].schema = sch;
	unsigned int numTuples;
	catalog->GetNoTuples(tblname, numTuples);
	CNF cnf;
	cnf.ExtractCNF(*predicate, sch, rec);
	for(int i = 0; i < cnf.numAnds; i++) { //TODO - need to test cnf, specially because of schema variable duplicacy
		if(cnf.andList[i].op == Equals) {
			vector <Attribute> atts = sch.GetAtts();
			if (cnf.andList[i].operand1 != Literal)
				numTuples /= atts[cnf.andList[i].whichAtt1].noDistinct;
			else
				numTuples /= atts[cnf.andList[i].whichAtt2].noDistinct;
		}
		else {
			numTuples /= 3;
		}
	}
	return numTuples;
}

void QueryOptimizer::TablePairInitialization() {

	for(TableList *tbl1 = tables; tbl1 != NULL; tbl1 = tbl1->next) {
		for(TableList *tbl2 = tbl1->next; tbl2 != NULL; tbl2 = tbl2->next) {

			string table1 = tbl1->tableName;
			Schema sch1 = Schema(tableMap[table1].schema);
			vector<Attribute> attr1 = sch1.GetAtts();

			string table2 = tbl2->tableName;
			Schema sch2 = Schema(tableMap[table2].schema);
			vector<Attribute> attr2 = sch2.GetAtts();

			vector<string> tableNames;
			tableNames.push_back(table1);
			tableNames.push_back(table2);
			string tablekey = GetKey(tableNames);
			tableMap[tablekey].size = tableMap[table1].size * tableMap[table2].size;
			tableMap[tablekey].size = Estimate_Join_Cardinality(sch1, sch2, tableMap[tablekey].size);
			tableMap[tablekey].cost = 0;

			OptimizationTree *node = new OptimizationTree;
			node->tables = tableNames;
			node->tuples.push_back(tableMap[table1].size);
			node->tuples.push_back(tableMap[table2].size);
			node->noTuples = tableMap[tablekey].size;
			node->leftChild = tableMap[table1].order;
			tableMap[table1].order->parent = node;
			node->rightChild = tableMap[table2].order;
			tableMap[table2].order->parent = node;
			node->parent = NULL;
			tableMap[tablekey].order  = node;

			sch1.Append(sch2);
			tableMap[tablekey].schema = sch1;
		}
	}
}

void QueryOptimizer::Partition(int N, vector<string> &tables){
	string tablekey = GetKey(tables);
	if (tableMap.find(tablekey) == tableMap.end()) {
		vector<vector<int>> allPerms;
		CalcPermutations(N, allPerms);
		unll min_cost = ULLONG_MAX, cost = 0;
		unll permsize = factorial(N);
		for (unll i = 0; i < permsize; i++) {
			vector<int> perm_i = allPerms[i];
			for (int j = 1; j < N; j++) {
				vector<string> left = fillVector(tables, perm_i, 0, j-1);
				string leftKey = GetKey(left);
				if (tableMap.find(leftKey) == tableMap.end())
					Partition(j, left);
				vector<string> right = fillVector(tables, perm_i, j, N-1);
				string rightKey = GetKey(right);
				if (tableMap.find(rightKey) == tableMap.end())
					Partition(N-j, right);

				cost = tableMap[leftKey].cost + tableMap[rightKey].cost;
				if (j != 1) cost += tableMap[leftKey].size;
				if (j != N-1) cost += tableMap[rightKey].size;
				if (cost < min_cost) {
					min_cost = cost;
					Schema sch1 = Schema(tableMap[leftKey].schema);
					Schema sch2 = Schema(tableMap[rightKey].schema);
					tableMap[tablekey].size = Estimate_Join_Cardinality(sch1, sch2, tableMap[leftKey].size * tableMap[rightKey].size);
					tableMap[tablekey].cost = min_cost;
					sch1.Append(sch2);
					tableMap[tablekey].schema = sch1;

					OptimizationTree *node = new OptimizationTree;
					node->tables = tables;
					node->noTuples = tableMap[tablekey].size;
					node->leftChild = tableMap[leftKey].order;
					tableMap[leftKey].order->parent = node;
					node->rightChild = tableMap[rightKey].order;
					tableMap[rightKey].order->parent = node;
					node->parent = NULL;
					vector<int> ltuples = vector<int>(tableMap[leftKey].order->tuples);
					vector<int> rtuples = vector<int>(tableMap[rightKey].order->tuples);
					ltuples.insert(ltuples.end(), rtuples.begin(), rtuples.end());
					node->tuples = ltuples;
					tableMap[tablekey].order = node;
				}
			}
		}
	}
}

vector<string> QueryOptimizer::fillVector(vector<string> &vec, vector<int> &idx, int x, int y) {
	vector<string> newVec;
	for(int i = x; i < y+1; i++) {
		newVec.push_back(vec[idx[i]]);
	}
	return newVec;
}

string QueryOptimizer::GetKey(vector<string> tableNames) {
	string tablekey = "";
	sort(tableNames.begin(), tableNames.end());
	for(vector<string>::iterator it1 = tableNames.begin(); it1 != tableNames.end(); ++it1) {
		tablekey += *it1;
	}
	return tablekey;
}


void QueryOptimizer::CalcPermutations(int n, vector<vector<int>> &allPerms) {
	vector<int> arr;
	for(int i = 0; i < n; i++) {
		arr.push_back(i);
	}
	heapPermutation(allPerms, arr, n, n);
}
// Heap's Algorithm for generating permutations - fastest
void QueryOptimizer::heapPermutation(vector<vector<int>> &allPerms, vector<int> &a, int size, int n)
{
    if (size == 1)
    {
				vector<int> newvec(a);
				allPerms.push_back(newvec);
        return;
    }

    for (int i=0; i<size; i++)
    {
        heapPermutation(allPerms, a,size-1,n);

        if (size%2==1)
            swap(a[0], a[size-1]);

        else
            swap(a[i], a[size-1]);
    }
}

unll QueryOptimizer::Estimate_Join_Cardinality(Schema &sch1, Schema &sch2, unll tblsize) {
	CNF cnf;
	cnf.ExtractCNF(*predicate, sch1, sch2);
	vector<Attribute> attr1 = sch1.GetAtts();
	vector<Attribute> attr2 = sch2.GetAtts();
	for (int i = 0; i < cnf.numAnds; i++) {
		if (cnf.andList[i].operand1 == Left)
		{
			tblsize /= max(attr1[cnf.andList[i].whichAtt1].noDistinct, attr2[cnf.andList[i].whichAtt2].noDistinct);
		}
		if (cnf.andList[i].operand1 == Right)
		{
			tblsize /= max(attr2[cnf.andList[i].whichAtt1].noDistinct, attr1[cnf.andList[i].whichAtt2].noDistinct);
		}
	}
	return tblsize;
}


unll QueryOptimizer::factorial(int n) {
	unll ans;
	if (n > 11) {
		ans = 1;
		for (int i = 2; i <= n; i++){
			ans *= i;
		}
	}
	else {
		unll factorial[12] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800};
		ans = factorial[n];
	}
	return ans;
}


// void QueryOptimizer::HardPermutations(int n, vector<vector<int>> &allPerms) {
// 	switch(n){
// 		case 2: allPerms = {
// 												{0, 1},
// 												{1, 0}
// 											 };
// 						break;
// 		case 3: allPerms = {
// 												{0, 1, 2},
// 												{1, 0, 2},
// 												{0, 2, 1},
// 												{1, 2, 0},
// 												{2, 1, 0},
// 												{2, 0, 1}
// 											 };
// 						break;
// 		default: CalcPermutations(n, allPerms);
// 	}
// }

// void QueryOptimizer::CalcPermutations(int n, vector<vector<int>> &allPerms) {
// 	// str filename = to_string(factorial(n)) + "X" + to_string(n);
// 	// ifstream file;
// 	// file.open(filename);
// 	// if (file.is_open()){
// 	// 	file.
// 	// }
// 	// else {
// 		vector<int> arr;
// 		for(int i = 0; i < n; i++) {
// 			arr.push_back(i);
// 		}
// 		heapPermutation(allPerms, arr, n, n);
// 		// ofstream file
// 		// for(vector<vector<int>>::iterator row = allPerms.begin(); row != allPerms.end(); ++row) {
// 		// 	for(vector<int>::iterator col = row->begin(); col != row->end(); ++col) {
// 		// 		cout<<*col;
// 		// 	}
// 		// 	cout<<'\n';
// 		// }
// 	// }
// }
