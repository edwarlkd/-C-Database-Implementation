#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <sstream>
// #include <ctime>
#include "QueryCompiler.h"
#include "QueryOptimizer.h"
#include "Schema.h"
#include "ParseTree.h"
#include "Record.h"
#include "DBFile.h"
#include "Comparison.h"
#include "Function.h"
#include "RelOp.h"

using namespace std;


QueryCompiler::QueryCompiler(Catalog& _catalog, QueryOptimizer& _optimizer) :
	catalog(&_catalog), optimizer(&_optimizer) {
}

QueryCompiler::~QueryCompiler() {
}

TableList* Copy(TableList* table) 
{
    if (table == NULL) 
    {
    	return NULL;
    }

    TableList* NewTable = new TableList;
    NewTable->tableName = table->tableName;
    NewTable->next = Copy(table->next);
    return NewTable;
}


void QueryCompiler::Compile(TableList* _tables, NameList* _attsToSelect,
	FuncOperator* _finalFunction, AndList* _predicate, NameList* _groupingAtts,
	int& _distinctAtts,	QueryExecutionTree& _queryTree, creatingTable* _attsToCreate, int Query) {
	// store Scans and Selects for each table to generate Query Execution Tree


TableList * _tables2;
_tables2 = Copy(_tables);


	if(Query == 4)
	{
		//cout<<"HERE"<<endl;
		//cout<<"_attsToCreate: "<<_attsToCreate->attName<<endl;
		string ScanIndexName = _attsToCreate->attName;
		string ScanIndexTable = _tables->tableName;
		string ScanIndexAttribute = _attsToSelect->name;

		cout<<"Query: CREATE_INDEX "<<ScanIndexName<< " TABLE "<<ScanIndexTable <<" ON "<<ScanIndexAttribute<<endl;

		Schema S;
		if(!catalog->GetSchema(ScanIndexTable, S))
		{
			cout<<"ERROR Table not Present"<<endl;
			return;
		}
		if(S.Index(ScanIndexAttribute) == -1)
		{
			cout<<"ERROR attribute not present"<<endl;
		}

		DBFile DB;
		DB.Open(&ScanIndexTable[0]);
		DB.MoveFirst();

		vector<string> attribs;
		vector<string> attribtyps;
		vector<unsigned int> dis;

		attribs.push_back(ScanIndexName);
		attribtyps.push_back("STRING");
		attribs.push_back(ScanIndexTable);
		attribtyps.push_back("STRING");
		attribs.push_back(ScanIndexAttribute);
		attribtyps.push_back("STRING");
		string add = (ScanIndexTable + ScanIndexAttribute);

		bool dum = catalog->CreateTable(add, attribs, attribtyps);

		if(dum)
		{
			cout<<"INDEX CREATED"<<endl;
		}
		else
		{
			cout<<"INDEX ALREADY EXISTS"<<endl;
			return;
		}

		Record rec;
		int rcnt = 0;
		int pcnt = 1;
		int rsz = 0;

		attribs.clear();
		attribtyps.clear();

		attribs.push_back("key");
		attribtyps.push_back("STRING");
		dis.push_back(1);

		attribs.push_back("page");
		attribtyps.push_back("INTEGER");
		dis.push_back(1);

		attribs.push_back("record");
		attribtyps.push_back("INTEGER");
		dis.push_back(1);

		Schema Lf(attribs, attribtyps, dis);

		attribs.clear();
		attribtyps.clear();
		dis.clear();

		attribs.push_back("key");
		attribtyps.push_back("STRING");
		dis.push_back(1);

		attribs.push_back("child");
		attribtyps.push_back("INTEGER");
		dis.push_back(1);

		Schema Inl(attribs, attribtyps, dis);

		Page dummy;
		int t;
		map<int, vector<Record>> intTree;
		map<float, vector<Record>> floatTree;
		map<string, vector<Record>> stringTree;

		while(DB.GetNext(rec))
		{
			rcnt++;
			rsz += rec.GetSize();
			if(rsz >= PAGE_SIZE)
			{
				rsz = 0;
				pcnt++;
				rcnt = 1;
			}

			cout<<"SCAN: "<<ScanIndexAttribute<<endl;

			cout<<"S: "<<S.Index(ScanIndexAttribute)<<endl;


			int dum2 = S.Index(ScanIndexAttribute);
			rec.GetBits();
			cout<<"HERE"<<endl;
			char* pr = rec.GetColumn(dum2);		//gets position of record 
												//maybe dum2 value is greater than the value of records
												//if we make our schema as record it should work


			string K;

			switch(S.FindType(ScanIndexAttribute))
			{
				case Integer:
				{
					t = 1;
					K = to_string(*((int*)pr));
					break;
				}
				case Float:
				{
					t = 2;
					K = to_string(*((double*)pr));
					break;
				}
				case String:
				{
					t = 3;
					K = pr;
					break;
				}
			}

			Record rec2;


			char* RecordSpace = new char[PAGE_SIZE];
			int PositionInRecord = sizeof(int)*(4);
			((int*)RecordSpace)[1] = PositionInRecord;

			int ln = K.length() + 1;
			if(ln % sizeof(int) != 0)
			{
				ln += sizeof(int) - (ln % sizeof(int));
			}

			strcpy(&(RecordSpace[PositionInRecord]), K.c_str());
			PositionInRecord += ln;

			((int*)RecordSpace)[2] = PositionInRecord;
			*((int*) &(RecordSpace[PositionInRecord])) =pcnt;
			PositionInRecord += sizeof (int);

			((int *) RecordSpace)[3] = PositionInRecord;
			*((int *) &(RecordSpace[PositionInRecord])) = rcnt;
			PositionInRecord += sizeof(int);

			((int *) RecordSpace)[0] = PositionInRecord;
			rec2.CopyBits(RecordSpace, PositionInRecord);
			delete [] RecordSpace;

			if(t == 1)
			{
				int indexkey = stoi(K,nullptr,10);
				auto it = intTree.find(indexkey);
				if(it != intTree.end())
				{
					it->second.push_back(rec2);
				}
				else
				{
					vector <Record> vec;
					vec.push_back(rec2);
					intTree[indexkey] = vec;
				}
			}
			if (t == 2)
			{
				float indexkey =stof(K,nullptr);
				auto it = floatTree.find(indexkey);
				if(it != floatTree.end())
				{
					it->second.push_back(rec2);
				}
				else
				{
					vector <Record> vec;
					vec.push_back(rec2);
					floatTree[indexkey] = vec;
				}
			}
			if (t == 3)
			{
				auto it = stringTree.find(K);
				if ( it != stringTree.end())
				{
					it->second.push_back(rec2);
				}
				else
				{
					vector <Record> vec;
					vec.push_back(rec2);
					stringTree[K] = vec;
				}
			}	

		}

		File F1;
		File indexfile;
		Page Pg; 
		Page indexpage;

		int recordCount = 0;
		string filename = ScanIndexName;
		string indexFileName = ScanIndexName + "Header";

		if( t == 1)
		{
			int Child = 1;
			string K = to_string(intTree.begin()->first);
			Record rec2;
			char* RecordSpace = new char[PAGE_SIZE];
			int PositionInRecord = sizeof (int) * (3);
			((int *) RecordSpace)[1] = PositionInRecord;
			int length = K.length()+1;
			if (length % sizeof (int) != 0)
			{
				length += sizeof(int) - (length % sizeof(int));
			}
			strcpy (&(RecordSpace[PositionInRecord]), K.c_str());
			PositionInRecord += length;

			((int *)RecordSpace)[2] = PositionInRecord;
			*((int *) &(RecordSpace[PositionInRecord])) = Child;
			PositionInRecord += sizeof(int);

			((int *) RecordSpace)[0] = PositionInRecord;
			rec2.CopyBits(RecordSpace,PositionInRecord);
			delete[] RecordSpace;
			Child++;

			indexpage.Append(rec2);
			F1.Open(0, &filename[0]);
			indexfile.Open(0, &indexFileName[0]);

			for(auto j: intTree)
			{
				for(auto it: j.second)
				{
					recordCount++;
					if (Pg.Append(it) == 0)	
					{
						F1.AddPage(Pg, F1.GetLength());
						Pg.EmptyItOut();
						Pg.Append(it);
						K = to_string(j.first);

						Record rec2;
						char* RecordSpace = new char[PAGE_SIZE];
						int PositionInRecord = sizeof(int) * (3);
						((int *)RecordSpace)[1] = PositionInRecord;
						int length = K.length()+1;
						if (length % sizeof(int) != 0)
						{
							length += sizeof(int) - (length % sizeof(int));
						}
						strcpy (&RecordSpace[PositionInRecord],K.c_str());
						PositionInRecord += length;
						((int *) RecordSpace)[2] = PositionInRecord;
						*((int *) &(RecordSpace[PositionInRecord])) = Child;
						((int *) RecordSpace)[0] = PositionInRecord;
						rec2.CopyBits(RecordSpace,PositionInRecord);
						delete[] RecordSpace;
						Child++;

						if (indexpage.Append(rec2) == 0)
						{
							indexfile.AddPage(indexpage,indexfile.GetLength());
							indexpage.EmptyItOut();
							indexpage.Append(rec2);
						}

					}
				}
			}
		F1.AddPage(Pg,F1.GetLength());
		indexfile.AddPage(indexpage, indexfile.GetLength());
		F1.GetPage(dummy, 2);
		F1.Close();
		indexfile.Close();

		cout<< "Childs Added"	<<F1.GetLength()<<endl;
		cout<< "Records Added"<<recordCount<<endl;

		}

		if (t == 2)
		{
			int Childs = 1;
			string K = to_string(floatTree.begin()->first);
			Record rec2;
			char* RecordSpace = new char[PAGE_SIZE];
			int PositionInRecord = sizeof (int) * (3);
			
			((int *) RecordSpace)[1] = PositionInRecord;
			int length = K.length()+1;
			if (length % sizeof (int) != 0) 
			{
				length += sizeof (int) - (length % sizeof (int));
			}
		
			strcpy (&(RecordSpace[PositionInRecord]), K.c_str()); 
			PositionInRecord += length;
		
			((int *) RecordSpace)[2] = PositionInRecord;
			*((int *) &(RecordSpace[PositionInRecord])) = Childs;	
			PositionInRecord += sizeof (int);
		
			((int *) RecordSpace)[0] = PositionInRecord;
			rec2.CopyBits( RecordSpace, PositionInRecord );
			delete [] RecordSpace;

			Childs++;

			indexpage.Append(rec2);
			
			F1.Open(0, &ScanIndexName[0]);
			indexfile.Open(0, &indexFileName[0]);
			
			for (auto j:floatTree)
			{
				for (auto it:j.second)
				{
					recordCount++;
					if (Pg.Append(it) == 0)
					{
						F1.AddPage(Pg, F1.GetLength());
						Pg.EmptyItOut();
						Pg.Append(it);
		
						K = to_string(j.first);
						Record rec2;
						char* RecordSpace = new char[PAGE_SIZE];
						int PositionInRecord = sizeof (int) * (3);
			
						((int *) RecordSpace)[1] = PositionInRecord;
						int length = K.length()+1;
						if (length % sizeof (int) != 0) 
						{
							length += sizeof (int) - (length % sizeof (int));
						}
		
						strcpy (&(RecordSpace[PositionInRecord]), K.c_str()); 
						PositionInRecord += length;
		
						((int *) RecordSpace)[2] = PositionInRecord;
						*((int *) &(RecordSpace[PositionInRecord])) = Childs;	
						PositionInRecord += sizeof (int);
		
						((int *) RecordSpace)[0] = PositionInRecord;
						rec2.CopyBits( RecordSpace, PositionInRecord );
						delete [] RecordSpace;

						Childs++;

						if (indexpage.Append(rec2) == 0)
						{
							indexfile.AddPage(indexpage, indexfile.GetLength());
							indexpage.EmptyItOut();
							indexpage.Append(rec2);
						}
					}
				}
			}			
			F1.AddPage(Pg, F1.GetLength());
			indexfile.AddPage(indexpage, indexfile.GetLength());
			F1.GetPage(dummy, 2);
			F1.Close();
			indexfile.Close();

			cout<<"Childs added: "<<F1.GetLength()<<endl;
			cout<<"Records added: "<<recordCount<<endl;
		}

		if (t == 3)
		{
			int Childs = 1;
			string K = stringTree.begin()->first;
			Record rec2;
			char* RecordSpace = new char[PAGE_SIZE];
			int PositionInRecord = sizeof (int) * (3);
			
			((int *) RecordSpace)[1] = PositionInRecord;
			int length = K.length()+1;
			if (length % sizeof (int) != 0) 
			{
				length += sizeof (int) - (length % sizeof (int));
			}
		
			strcpy (&(RecordSpace[PositionInRecord]), K.c_str()); 
			PositionInRecord += length;
		
			((int *) RecordSpace)[2] = PositionInRecord;
			*((int *) &(RecordSpace[PositionInRecord])) = Childs;	
			PositionInRecord += sizeof (int);
		
			((int *) RecordSpace)[0] = PositionInRecord;
			rec2.CopyBits( RecordSpace, PositionInRecord );
			delete [] RecordSpace;

			Childs++;

			indexpage.Append(rec2);
			
			F1.Open(0, &ScanIndexName[0]);
			indexfile.Open(0, &indexFileName[0]);
			
			for (auto j:stringTree)
			{
				for (auto it:j.second)
				{
					recordCount++;
					if (Pg.Append(it) == 0)
					{
						F1.AddPage(Pg, F1.GetLength());
						Pg.EmptyItOut();
						Pg.Append(it);
		
						K = j.first;
						Record rec2;
						char* RecordSpace = new char[PAGE_SIZE];
						int PositionInRecord = sizeof (int) * (3);
			
						((int *) RecordSpace)[1] = PositionInRecord;
						int length = K.length()+1;
						if (length % sizeof (int) != 0) 
						{
							length += sizeof (int) - (length % sizeof (int));
						}
		
						strcpy (&(RecordSpace[PositionInRecord]), K.c_str()); 
						PositionInRecord += length;
		
						((int *) RecordSpace)[2] = PositionInRecord;
						*((int *) &(RecordSpace[PositionInRecord])) = Childs;	
						PositionInRecord += sizeof (int);
		
						((int *) RecordSpace)[0] = PositionInRecord;
						rec2.CopyBits( RecordSpace, PositionInRecord );
						delete [] RecordSpace;

						Childs++;

						if (indexpage.Append(rec2) == 0)
						{
							indexfile.AddPage(indexpage, indexfile.GetLength());
							indexpage.EmptyItOut();
							indexpage.Append(rec2);
						}
					}
				}
			}			
			F1.AddPage(Pg, F1.GetLength());
			indexfile.AddPage(indexpage, indexfile.GetLength());
			F1.Close();
			indexfile.Close();

			cout<<"Childs added: "<<F1.GetLength()<<endl;
			cout<<"Records added: "<<recordCount<<endl;
		}
	}


	else
	{

		unordered_map<string, RelationalOp*> pushDowns;
		TableList *tblList = _tables;
		while(_tables != NULL) {
			string tableName = string(_tables->tableName);
			DBFile dbFile; string dbFilePath;		
			if(!catalog->GetDataFile(tableName, dbFilePath)) {
				cerr << "ERROR: Table '" << tableName << "' does not exist." << endl << endl;
				exit(-1);
			}
			char* dbFilePathC = new char[dbFilePath.length()+1]; 
			strcpy(dbFilePathC, dbFilePath.c_str());
			if(dbFile.Open(dbFilePathC) == -1) {
				// error message is already shown in File::Open
				exit(-1);
			}

			/** create a SCAN operator for each table in the query **/
			Schema schema;
			if(!catalog->GetSchema(tableName, schema)) {
				cerr << "ERROR: Table '" << tableName << "' does not exist." << endl << endl;
				exit(-1);
			}

			Scan* scan = new Scan(schema, dbFile);
			pushDowns[tableName] = (RelationalOp*) scan;

			/** push-down selects: create a SELECT operator wherever necessary **/
			CNF cnf; Record record;
			// CNF::ExtractCNF returns 0 if success, -1 otherwise
			if(cnf.ExtractCNF(*_predicate, schema, record) == -1) {
				// error message is already shown in CNF::ExtractCNF
				exit(-1);
			}

			if(cnf.numAnds > 0) {
				Select* select = new Select(schema, cnf, record, (RelationalOp*) scan);
				pushDowns[tableName] = (RelationalOp*) select;
			}

			// move on to the next table
			_tables = _tables->next;
		}

		/** call the optimizer to compute the join order **/
		OptimizationTree root;
		optimizer->Optimize(tblList, _predicate, &root);
		OptimizationTree *rootTree = &root;
		/** create join operators based on the optimal order computed by the optimizer **/
		// get actual Query eXecution Tree by joining them
		RelationalOp* qxTree = buildJoinTree(rootTree, _predicate, pushDowns, 0);

		/** create the remaining operators based on the query **/
		// qxTreeRoot will be the root of query execution tree
		RelationalOp* qxTreeRoot = (RelationalOp*) qxTree;

		// case 1. SELECT-WHERE-FROM
		// since there is no _groupingAtts, check _finalFunction first.
		// if _finalFunction does not exist, Project and check _distinctAtts for DuplicateRemoval
		// else, append Sum at the root.

		// schemaIn always comes from join tree
		Schema schemaIn = qxTree->GetSchema();

		if(_groupingAtts == NULL) {

			if(_finalFunction == NULL) { // check _finalFunction first
				// a Project operator is appended at the root
				Schema schemaOut = schemaIn;
				int numAttsInput = schemaIn.GetNumAtts();
				int numAttsOutput = 0;
				vector<int> attsToKeep;
				vector<Attribute> atts = schemaIn.GetAtts();
				bool isFound;

				while(_attsToSelect != NULL) {
					string attrName = string(_attsToSelect->name);
					isFound = false;
					for(int i = 0; i < atts.size(); i++) {
						if(atts[i].name == attrName) {
							isFound = true;
							attsToKeep.push_back(i);
							numAttsOutput++;
							break;
						}
					}

					if(!isFound) {
						cerr << "ERROR: Attribute '" << attrName << "' does not exist." << endl << endl;
						exit(-1);
					}

					_attsToSelect = _attsToSelect->next;
				}

				// reverse the vector; _attsToSelect has reverse order of attributes
				reverse(attsToKeep.begin(), attsToKeep.end());

				if(schemaOut.Project(attsToKeep) == -1) {
					cerr << "ERROR: Project failed:\n" << schemaOut << endl << endl;
					exit(-1);
				}

				int* keepMe = new int[attsToKeep.size()];
				copy(attsToKeep.begin(), attsToKeep.end(), keepMe);

				Project* project = new Project(schemaIn, schemaOut, numAttsInput, numAttsOutput,
					keepMe, qxTree);

				// in case of DISTINCT, a DuplicateRemoval operator is further inserted
				if(_distinctAtts != 0) {
					Schema schemaIn = project->GetSchema();
					DuplicateRemoval* distinct = new DuplicateRemoval(schemaIn, project);
					qxTreeRoot = (RelationalOp*) distinct;
				} else {
					qxTreeRoot = (RelationalOp*) project;
				}
			} else { // a Sum operator is insert at the root
				// the output schema consists of a single attribute 'sum'.
				vector<string> attributes, attributeTypes;
				vector<unsigned int> distincts;

				Function compute; compute.GrowFromParseTree(_finalFunction, schemaIn);

				attributes.push_back("sum");
				attributeTypes.push_back(compute.GetTypeAsString());
				distincts.push_back(1);
				Schema schemaOut(attributes, attributeTypes, distincts);

				Sum* sum = new Sum(schemaIn, schemaOut, compute, qxTree);
				qxTreeRoot = (RelationalOp*) sum;
			}
		} else { // case 2. SELECT-FROM-WHERE-GROUPBY
			// if query has GROUP BY, a GroupBy operator is appended at the root

			// the output schema contains the aggregate attribute 'sum' on the first position
			vector<string> attributes, attributeTypes;
			vector<unsigned int> distincts;

			// followed by the grouping attributes
			vector<int> attsToGroup; int attsNo = 0;

			while(_groupingAtts != NULL) {
				string attrName = string(_groupingAtts->name);
				int noDistinct = schemaIn.GetDistincts(attrName);
				if(noDistinct == -1) {
					cerr << "ERROR: Attribute '" << attrName << "' does not exist." << endl << endl;
					exit(-1);
				}
				Type attrType = schemaIn.FindType(attrName); string attrTypeStr;
				switch(attrType) {
					case Integer:
						attrTypeStr = "INTEGER";
						break;
					case Float:
						attrTypeStr = "FLOAT";
						break;
					case String:
						attrTypeStr = "STRING";
						break;
					default:
						attrTypeStr = "UNKNOWN";
						break;
				}

				attributes.push_back(attrName);
				attributeTypes.push_back(attrTypeStr);
				distincts.push_back(noDistinct);

				attsToGroup.push_back(schemaIn.Index(attrName));
				attsNo++;

				_groupingAtts = _groupingAtts->next;
			}

			// put aggregate function at the end due to reverse afterward
			Function compute;
			if(_finalFunction != NULL) {
				compute.GrowFromParseTree(_finalFunction, schemaIn);

				attributes.push_back("sum");
				attributeTypes.push_back(compute.GetTypeAsString());
				distincts.push_back(1);
			}

			// reverse the vectors; _groupingAtts has reverse order of attributes
			reverse(attributes.begin(), attributes.end());
			reverse(attributeTypes.begin(), attributeTypes.end());
			reverse(distincts.begin(), distincts.end());
			reverse(attsToGroup.begin(), attsToGroup.end());

			Schema schemaOut(attributes, attributeTypes, distincts);

			int* attsOrder = new int[attsToGroup.size()];
			copy(attsToGroup.begin(), attsToGroup.end(), attsOrder);
			OrderMaker groupingAtts(schemaIn, attsOrder, attsNo);

			GroupBy* group = new GroupBy(schemaIn, schemaOut, groupingAtts, compute, qxTree);
			qxTreeRoot = (RelationalOp*) group;
		}

		// in the end, create WriteOut at the root of qxTree
		
		// for in cases of comparing results,
		// get current datetime to store result in the separate file
		// comment these lines out when submit
		// time_t timeT; struct tm* timeInfo; char buf[80]; string timeS;
		// time(&timeT); timeInfo = localtime(&timeT);
		// strftime(buf, sizeof(buf),"%Y%m%d_%H%M%S", timeInfo);
		// timeS = string(buf);
		// string outFile = "output/" + timeS + ".txt";
		string outFile = "output.txt";
		Schema outSchema = qxTreeRoot->GetSchema();
		WriteOut* writeOut = new WriteOut(outSchema, outFile, qxTreeRoot);
		qxTreeRoot = (RelationalOp*) writeOut;

		/** connect everything in the query execution tree and return **/
		_queryTree.SetRoot(*qxTreeRoot);

		/** free the memory occupied by the parse tree since it is not necessary anymore **/
		_tables = NULL; _attsToSelect = NULL; _finalFunction = NULL;
		_predicate = NULL; _groupingAtts = NULL; rootTree = NULL;
	}
}





// a recursive function to create Join operators (w/ Select & Scan) from optimization result
RelationalOp* QueryCompiler::buildJoinTree(OptimizationTree*& _tree,
	AndList* _predicate, unordered_map<string, RelationalOp*>& _pushDowns, int depth) {
	// at leaf, do push-down (or just return table itself)
	if(_tree->leftChild == NULL && _tree->rightChild == NULL) {
		return _pushDowns.find(_tree->tables[0])->second;
	} else { // recursively do join from left/right RelOps
		Schema lSchema, rSchema, oSchema; CNF cnf;

		RelationalOp* lOp = buildJoinTree(_tree->leftChild, _predicate, _pushDowns, depth+1);
		RelationalOp* rOp = buildJoinTree(_tree->rightChild, _predicate, _pushDowns, depth+1);

		lSchema = lOp->GetSchema();
		rSchema = rOp->GetSchema();
		cnf.ExtractCNF(*_predicate, lSchema, rSchema);
		oSchema.Append(lSchema); oSchema.Append(rSchema);
		Join* join = new Join(lSchema, rSchema, oSchema, cnf, lOp, rOp);

		// set current depth for join operation
		join->depth = depth;
		join->numTuples = _tree->noTuples;

		return (RelationalOp*) join;
	}
}
