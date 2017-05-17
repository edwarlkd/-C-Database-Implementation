#include <iostream>
#include <cstring>
#include <sstream>
#include <map>

#include "RelOp.h"
#include "Config.h"

using namespace std;


ostream& operator<<(ostream& _os, RelationalOp& _op) {
	return _op.print(_os);
}

Scan::Scan(Schema& _schema, DBFile& _file):
	schema(_schema),
	file(_file) {
}

Scan::~Scan() {}

bool Scan::GetNext(Record& _record) {
	if (file.GetNext(_record) == 0) {
		return true;
	}
	else {
		return false;
	}
}

ostream& Scan::print(ostream& _os) {
	return _os << file.GetTableName();
}


//////////////////////SCAN INDEX///////////////////////////


ScanIndex::ScanIndex(Schema& _schema, Schema _schema2, CNF& _predicate, Record& _constants, string& _tab, string& _att, string& _k, Type _typ, File _in, File _pgheader)
{
	schema = _schema;						//Schema used later in query compiler
	schema2 = _schema2;						//schema used later in query compiler
	predicate = _predicate;					//predicates in the where clause
	constants = _constants;					//record constants to be used in querycompiler
	tab = _tab;								//tab is for the table being read in 
	att = _att;								//att is for the attributes being read in
	k = _k;									//string k is used later to search through the vectors
	typ = _typ;								//typ of the attributes
	in = _in;								//internal parameter				
	pgheader = _pgheader;					//page header used to find the head of the page
	check = false;							//check to stop running getnext
	vectorCount = 0;						//counter to keep track and compare to the vector size

	vector<unsigned int> dis;				//vector for distincts
	vector<string> attribtyps;				//vector for attribute types
	vector<string> attribs;					//vector for attributes

	attribs.push_back("key");				//push key into attributes
	attribtyps.push_back("STRING");			//type is string
	dis.push_back(1);						//distinct is 1

	attribs.push_back("page");				//pushes page into attributes
	attribtyps.push_back("INTEGER");		//type is integer
	dis.push_back(1);						//distinct is 1

	attribs.push_back("record");			//pushes record into attributes
	attribtyps.push_back("INTEGER");		//type is integer
	dis.push_back(1);						//distinct is 1

	Schema S1(attribs, attribtyps, dis);	//creates schema using the previous parameters this is for leaves

	attribs.clear();						// CLEAR ALL VECTORS
	attribtyps.clear();
	dis.clear();							

	attribs.push_back("key");				//pushes key into attributes
	attribtyps.push_back("STRING");			//type is string
	dis.push_back(1);						//distincts is 1

	attribs.push_back("child");				//pushes child into attributes
	attribtyps.push_back("INTEGER");		//type is integer
	dis.push_back(1);						//distinct is 1

	Schema S2(attribs, attribtyps, dis);	//Creates another schema using the previous parameters this is for internal nodes in tree

	Leaf = S1;								
	Internal = S2;

}

ScanIndex::~ScanIndex(){
}

bool ScanIndex::GetNext(Record& _record)
{
	if(!check)
	{
		check = true;									//check changes to true after it is in
		int ch = 0;										
		int l = 0;
		int recNum;
		Page pg;

		stringstream str;
		string ky = str.str();
		ky.erase(0, ky.find(":") + 2);
		ky = ky.substr(0, ky.find("}"));

		in.Open();
		pgheader.Open();
		pgheader.GetPage(pg, 0);

		while(pg.GetFirst(_record))
		{
			l++;
			stringstream str1;
			string str2 = str1.str();
			str2.erase(0, str2.find(":")+2);
			str2 = str2.substr(0, str2.find(","));

			switch(typ)
			{
				case Integer:
				{
					int one = stoi(ky, nullptr, 10);
					int two = stoi(str2, nullptr, 10);

					if(two <= one)
					{
						ch = l;
					}
					break;
				}
				case Float:
				{
					float one  = stof(ky, nullptr);
					float two  = stof(str2, nullptr);

					if(two <= one)
					{
						ch = l;
					}
					break;
				}
				case String:
				{
					if(str2.compare(ky) <= 0)
					{
						ch = l;
					}
					break;
				}
			}
		}

		in.GetPage(pg, ch-1);
		DBFile DB;
		DB.Open(&tab[0]);

		while(pg.GetFirst(_record))
		{
			stringstream s;
			string s2 = s.str();
			string s3 = s2;
			s2.erase(0, s2.find(":")+2);
			s2 = s2.substr(0, s2.find(","));

			switch(typ)
			{
				case Integer:
				{
					int one = stoi(ky, nullptr, 10);
					int two = stoi(s2, nullptr, 10);
					if(one == two)
					{
						int three;
						int four;
						three = s3.find(",");
						four = s3.find(",", three + 1);

						string pstr = s3.substr(three + 7, four - (three + 7));
						int p = stoi(pstr, nullptr, 10);
						string rstr = s3.substr(four + 10, s3.find("}") - (four + 10));
						int r = stoi(rstr, nullptr, 10);

						DB.GetSpecificRec(p - 1, r, _record);

						Recs.push_back(_record);
					}
					break;
				}
				case Float:
				{
					float one = stof(ky, nullptr);
					float two = stof(s2, nullptr);
					if(one == two)
					{
						int three;
						int four;
						three = s3.find(",");
						four = s3.find(",", three + 1);

						string pstr = s3.substr(three + 7, four - (three + 7));
						int p = stoi(pstr, nullptr, 10);
						string rstr = s3.substr(four + 10, s3.find("}") - (four + 10));
						int r = stoi(rstr, nullptr, 10);

						DB.GetSpecificRec(p - 1, r, _record);

						Recs.push_back(_record);
					}
					break;
				}
				case String:
				{
					if(s2.compare(ky) == 0)
					{
						int three;
						int four;
						three = s3.find(",");
						four = s3.find(",", three + 1);

						string pstr = s3.substr(three + 7, four - (three + 7));
						int p = stoi(pstr, nullptr, 10);
						string rstr = s3.substr(four + 10, s3.find("}") - (four + 10));
						int r = stoi(rstr, nullptr, 10);

						DB.GetSpecificRec(p - 1, r, _record);

						Recs.push_back(_record);
					}
					break;
					
				}
			}
		}

		if(vectorCount == Recs.size())
		{
			return false;
		}
		else
		{
			_record = Recs[vectorCount];
			vectorCount++;
			return true;
		}


	}
}
/*
	if(check == 0)
	{
		DBFile File;						//Initializing a file
		File.Open(&tab[0]);					//Opening the file created
		DBFile Leaf;						//File for the leaf nodes
		DBFile Internal;					//file for the internal nodes
		Leaf.Open(&leafNode[0]);			//Leaf file is open with the path as the first leaf node from the constructor
		Internal.Open(&internalNode[0]);	//Internal file is open with the path as the first internal node from the construtor

		vector<string> Atribs;				//String of Attributes
		Atribs.push_back("value");			//Inserting "value" as a dummy string into the vector
		vector<string> Typs;				//Creating a vector to store the types of the variables
		Typs.push_back("INTEGER");			//Inserting integer as the type into the vector
		vector <unsigned int> Distincts;	//Creating a vector to store the Distincts of the attributes
		Distincts.push_back(0);				//Inserting 0 as the distinct value into the vector
		Schema S1(Atribs, Typs, Distincts);	//Creating a schema that has all of the elements above as parameter

		stringstream str1;
		constants.print(str1, S1);
		string str2 = str1.str();
		size_t Position = str2.find(":");
		string str3 = str2.substr(Position + 2, str2.length() - Position - 3);

		Record InternalRec;
		Internal.MoveFirst();
		int cnt = 1;
		int ChildCnt = 0;

		while(Internal.GetNext(InternalRec))
		{
			Atribs.clear();
			Typs.clear();
			Distincts.clear();

			Atribs.push_back("key");
			Atribs.push_back("child");
			Typs.push_back("INTEGER");
			Typs.push_back("INTEGER");
			Distincts.push_back(0);
			Distincts.push_back(0);
			Schema S2(Atribs, Typs, Distincts);

			stringstream st;
			InternalRec.print(st, S2);
			str2 = st.str();
			Position = str2.find(":");
			size_t Position2 = str2.find(",");
			string inKey = str2.substr(Position + 1, Position2 - Position - 1);
		
			if(stoi(str3) < stoi(inKey))
			{
				ChildCnt = cnt - 1;
			}
			else
			{
				ChildCnt = cnt;
				cnt++;
			}
		}

		Page pg;
		Record rec;
		Leaf.GetPageNumber(ChildCnt - 1, pg);
		int cnt2 = 0;
		int cnt3 = 0;
		vector<Record> RecIndex;

		Atribs.clear();
		Atribs.push_back("key");
		Atribs.push_back("page");
		Atribs.push_back("record");

		Typs.clear();
		Typs.push_back("INTEGER");
		Typs.push_back("INTEGER");
		Typs.push_back("INTEGER");

		Distincts.clear();
		Distincts.push_back(0);
		Distincts.push_back(0);
		Distincts.push_back(0);

		Schema LSchema(Atribs, Typs, Distincts);

		while(pg.GetFirst(rec) != 0)
		{
			cnt2++;
			stringstream RecLeaf;
			rec.print(RecLeaf, LSchema);

			string str4; 
			string str5;
			string str6;
			string Knum;
			string Pnum;
			string Rnum;
			int kcnt = 0;
			int pcnt = 0;
			int rcnt = 0;

			RecLeaf>>str4>>Knum>>str5>>Pnum>>str6>>Rnum;

			Knum.pop_back();
			Pnum.pop_back();
			Rnum.pop_back();

			kcnt = stoi(Knum, nullptr, 10);
			pcnt = stoi(Pnum, nullptr, 10);
			rcnt = stoi(Rnum, nullptr, 10);

			int compare = stoi(str3, nullptr, 10);

			if(compare == kcnt)
			{
				cnt3++;
				Record RootRec;
				File.GetSpecificRec(pcnt - 1, rcnt, RootRec);

				lastRecord.push_back(RootRec);
			}
		}

		check = 1;
	}

	if(vectorCount < lastRecord.size())
	{
		_record = lastRecord[vectorCount];
		vectorCount++;
		return true;
	}
	else
	{
		return false;
	}
}*/

ostream& ScanIndex::print(ostream& _os)
{
	return _os<< "SCAN INDEX";
}



Select::Select(Schema& _schema, CNF& _predicate, Record& _constants,
	RelationalOp* _producer) :
	schema(_schema),
	predicate(_predicate),
	constants(_constants),
	producer(_producer) {
}

Select::~Select() 
{

}

bool Select::GetNext(Record& _record) {
	while (producer->GetNext(_record)) {
		if (predicate.Run(_record, constants)) {
			return true;
		}
	}
	return false;
}

ostream& Select::print(ostream& _os) {
	
	return _os << "SELECT [...] ── " << *producer; // print without predicates
}


Project::Project(Schema& _schemaIn, Schema& _schemaOut, int _numAttsInput,
	int _numAttsOutput, int* _keepMe, RelationalOp* _producer) :
	schemaIn(_schemaIn),
	schemaOut(_schemaOut),
	numAttsInput(_numAttsInput),
	numAttsOutput(_numAttsOutput),
	keepMe(_keepMe),
	producer(_producer) {
}

Project::~Project() {}

bool Project::GetNext(Record& _record) {
	if (producer->GetNext(_record)) {
		_record.Project(keepMe, numAttsOutput, numAttsInput);
		return true;
	}
	else {
		return false;
	}
}

ostream& Project::print(ostream& _os) {
	
	return _os << "Project [...]\n\t │\n\t" << *producer; // print without predicates
}


Join::Join(Schema& _schemaLeft, Schema& _schemaRight, Schema& _schemaOut,
	CNF& _predicate, RelationalOp* _left, RelationalOp* _right) :
	schemaLeft(_schemaLeft),
	schemaRight(_schemaRight),
	schemaOut(_schemaOut),
	predicate(_predicate),
	left(_left),
	right(_right),
	isFirst(true) {
}

Join::~Join() {}

bool Join::GetNext(Record& _record) {
	// sort-merge join
	// 1. build DBFiles with sorted records
	//    within the available number of pages, NUM_PAGES_AVAILABLE
	if(isFirst) {
		if(!CreateSortedDBFiles(left, true) || !CreateSortedDBFiles(right, false)) {
			return false;
		}
		//Get one record each from every DBFile and divide into two groups (for left and right relation)
		leftidx = 0; rightidx = 0;
		bool _isLeft = true;
		for(int i = 0; i < 2; i++) {
			vector<DBFile*>& dbfilevec = _isLeft ? DBFilesLeft : DBFilesRight;
			FibHeap& ds = _isLeft ? left_ds : right_ds;
			vector<DBFile*>::iterator it = dbfilevec.begin();
			for(; it != dbfilevec.end(); it++) {
				Record rec;
				CompositeKey key;
				(*it)->MoveFirst();
				if((*it)->GetNext(rec) == 0) {
					char* bits = rec.GetBits();
					key.extractRecord(bits, predicate, _isLeft);
					ds.insert(key, rec, (*it));
				}
				else {
					RemoveDBFile((*it));
					dbfilevec.erase(it);
					--it;
				}
			}
			_isLeft = false;
		}
		isFirst = false;
	}

	if (leftidx < recs_left.size()) {
		Record lrec = recs_left[leftidx];
		Record rrec = recs_right[rightidx];
		_record.MergeRecords(lrec,\
			 rrec,\
			 schemaLeft.GetNumAtts(),\
			 schemaRight.GetNumAtts(),\
			 GenerateAttsToKeep(),\
			 schemaOut.GetNumAtts(),\
			 schemaLeft.GetNumAtts());
		rightidx++;
		if (rightidx >= recs_right.size()) {
			leftidx++;
			rightidx = 0;
		}
		// _record = Output.back();
		// Output.pop_back();
		return true;
	}
	else {
		//Check if dbfiles are left in left and right vectors
		if (DBFilesLeft.size() == 0 || DBFilesRight.size() == 0) {
			RemoveSortedDBFiles(DBFilesLeft); RemoveSortedDBFiles(DBFilesRight);
			return false;
		}
		leftidx = 0; rightidx = 0;
		recs_left.clear(); recs_right.clear();
		//Determine minimum for both groups and iterate until minimums match

		CompositeKey minleft, minright, k;
		Record recleft, recright;
		bool _isLeft;
		ExtractMinAndInsert(minleft, recleft, true, false, k);
		ExtractMinAndInsert(minright, recright, false, false, k);
		while (!(minleft == minright)) {
			_isLeft = minleft < minright;

			//if left table is going to be polled for records, it shouldn't be empty
			//same for the right table

			if ((_isLeft && DBFilesLeft.size() == 0) || \
			(!_isLeft && DBFilesRight.size() == 0)) {
				RemoveSortedDBFiles(DBFilesLeft); RemoveSortedDBFiles(DBFilesRight);
				return false;
			}

			//Determine the smaller key and poll its relation until the key becomes
			//equal or greater then the other relation's key

			CompositeKey &smallkey = _isLeft ? minleft : minright;
			CompositeKey &bigkey = _isLeft ? minright : minleft;
			Record& rec = _isLeft? recleft: recright;

			while(smallkey < bigkey) {
				if(!ExtractMinAndInsert(smallkey, rec, _isLeft, false, k)) {
					break;
				}
			}
		}

		//When minimums are equal, take out all the records from both relations with
		//min value

		// vector<Record> recs_left, recs_right;
		_isLeft = true;

		for (int i = 0; i < 2; i++) {
			CompositeKey& min = _isLeft? minleft: minright;
			vector<DBFile*>& dbvec = _isLeft? DBFilesLeft: DBFilesRight;
			vector<Record>& recvec = _isLeft? recs_left: recs_right;
			Record& rec = _isLeft ? recleft : recright;

			CompositeKey key = min;
			while (min == key) {
				recvec.push_back(rec);
				if (dbvec.size() == 0){
					break;
				}
				ExtractMinAndInsert(key, rec, _isLeft, true, min);
			}

			_isLeft = false;
		}

		//Adding to Output


		Record lrec = recs_left[leftidx];
		Record rrec = recs_right[rightidx];
		_record.MergeRecords(lrec,\
			 rrec,\
			 schemaLeft.GetNumAtts(),\
			 schemaRight.GetNumAtts(),\
			 GenerateAttsToKeep(),\
			 schemaOut.GetNumAtts(),\
			 schemaLeft.GetNumAtts());
		rightidx++;
		if(rightidx >= recs_right.size()){
			leftidx++;
			rightidx = 0;
		}


		return true;
	}
}

int* Join::GenerateAttsToKeep() {
	int* attsToKeep = new int[schemaOut.GetNumAtts()];

	// append the indices of the attributes into the vector pointer
	for(int i=0;i<schemaLeft.GetNumAtts();i++){
		attsToKeep[i] = i;
	}

	// Check the index where right record is encountered (schemaLeft.GetNumAtts())
	for(int i=0; i<schemaRight.GetNumAtts();i++){
		attsToKeep[schemaLeft.GetNumAtts()+i] = i;
	}
	return attsToKeep;
}

bool Join::ExtractMinAndInsert(CompositeKey& key, Record& rec, bool _isLeft,\
	 bool _ismin, CompositeKey& min) {
	FibHeap& ds = _isLeft? left_ds: right_ds;
	DBFile* dbf;

	//extract min
	ds.extractmin(key, rec, dbf);
	if (_ismin) {
		if (!(min == key)) {
			ds.insert(key, rec, dbf);
			return true;
		}
	}
	//insert
	Record recnext; CompositeKey keynext;
	if(dbf->GetNext(recnext) == 0) {
		char* bits = recnext.GetBits();
		keynext.extractRecord(bits, predicate, _isLeft);
		ds.insert(keynext, recnext, dbf);
		return true;
	}
	else {
		//Remove DBFile
		vector<DBFile*>& dbvec = _isLeft? DBFilesLeft: DBFilesRight;
		vector<DBFile*>::iterator itdbvec = dbvec.begin();
		for(; itdbvec != dbvec.end(); ++itdbvec) {
			if ((*itdbvec) == dbf) {
				RemoveDBFile(dbf);
				dbvec.erase(itdbvec);
				break;
			}
		}
		return false;
	}
}

bool Join::CreateSortedDBFiles(RelationalOp*& _rel, bool _isLeft) {
	int fileNum = 0;
	string prefix = ".tmp/TMP_" + to_string(depth) + (_isLeft ? "_L_" : "_R_");
	string postfix = ".dat";

	// create DBFile
	DBFile* dbFile = new DBFile();
	string path = prefix + to_string(fileNum++) + postfix;
	char* pathC = new char[path.length()+1];
	strcpy(pathC, path.c_str());
	if(dbFile->Create(pathC, Sorted) == -1) {
		cerr << "ERROR: Failed to create sorted DBFile" << endl << endl;
		return false;
	}
	dbFile->MoveFirst();

	// append records to a Page
	multimap<CompositeKey, Record> bst; // handles a Page
	multimap<CompositeKey, Record>::iterator it;

	int pageSizeNow = 0; // current size of the records
	int numPagesNow = 1; // current number of pages in DBFile

	Record rec;
	while(_rel->GetNext(rec)) {
		char* bits = rec.GetBits();
		// check size overflow with PAGE_SIZE
		if (pageSizeNow + ((int *) bits)[0] > PAGE_SIZE) {
			// check Page limit with NUM_PAGES_AVAILABLE
			if(numPagesNow == NUM_PAGES_AVAILABLE) { // overflow
				// first, append records in BST into Pages in the current DBFile
				it = bst.begin();
				while(it != bst.end()) {
					dbFile->AppendRecord((*it).second);
					it = bst.erase(it); // and erase it right away
				}
				dbFile->WriteToFile();

				// put current DBFile into vector
				if(_isLeft) {
					DBFilesLeft.push_back(dbFile);
				} else {
					DBFilesRight.push_back(dbFile);
				}

				// and create new DBFile
				dbFile = new DBFile();
				path = prefix + to_string(fileNum++) + postfix;
				pathC = new char[path.length()+1];
				strcpy(pathC, path.c_str());
				if(dbFile->Create(pathC, Sorted) == -1) {
					cerr << "ERROR: Failed to create sorted DBFile" << endl << endl;
					return false;
				}
				dbFile->MoveFirst();

				// reset number of pages in DBFile after insertion
				numPagesNow = 1;
			} else {
				// increase current number of pages in DBFile
				numPagesNow++;
			}

			// reset current page size after insertion
			pageSizeNow = 0;
		}

		// insert record into BST
		// create key from the record using predicate
		CompositeKey key;
		key.extractRecord(bits, predicate, _isLeft);

		// insert tuple into BST and increase current page size
		bst.insert(pair<CompositeKey, Record>(key, rec));
		pageSizeNow += ((int *) bits)[0];
	}

	// append records left in the BST into the last Page
	it = bst.begin();
	while(it != bst.end()) {
		dbFile->AppendRecord((*it).second);
		it = bst.erase(it); // and erase it right away
	}
	dbFile->WriteToFile();

	// and put the last DBFile into vector
	if(_isLeft) {
		DBFilesLeft.push_back(dbFile);
	} else {
		DBFilesRight.push_back(dbFile);
	}

	return true;
}

bool Join::RemoveSortedDBFiles(vector<DBFile*>& _DBFiles) {
	vector<DBFile*>::iterator it;
	for (it = _DBFiles.begin(); it != _DBFiles.end(); ++it) {

		if(!RemoveDBFile((*it))) return false;

	}

	_DBFiles.clear();

	return true;
}

bool Join::RemoveDBFile(DBFile* _dbfile) {
	const char* DBFileName = _dbfile->GetFileName();

	// close the DBFile first
	if(_dbfile->Close() == -1) {
		cerr << "ERROR: Failed to close " << DBFileName << endl << endl;
		return false;
	}

	// and remove from the disk
	if(remove(DBFileName) != 0) {
		cerr << "ERROR: Failed to remove " << DBFileName << endl << endl;
		return false;
	}
	return true;
}

ostream& Join::print(ostream& _os) {
	// _os << "⋈ [";
	// for(int i = 0; i < predicate.numAnds; i++) {
	// 	if(i > 0) {
	// 		_os << " AND ";
	// 	}

	// 	Comparison comp = predicate.andList[i];

	// 	if(comp.operand1 == Left) {
	// 		_os << schemaLeft.GetAtts()[comp.whichAtt1].name;
	// 	} else if(comp.operand1 == Right) {
	// 		_os << schemaRight.GetAtts()[comp.whichAtt1].name;
	// 	}

	// 	if (comp.op == LessThan) {
	// 		_os << " < ";
	// 	} else if (comp.op == GreaterThan) {
	// 		_os << " > ";
	// 	} else if (comp.op == Equals) {
	// 		_os << " = ";
	// 	} else {
	// 		_os << " ? ";
	// 	}

	// 	if(comp.operand2 == Left) {
	// 		_os << schemaLeft.GetAtts()[comp.whichAtt2].name;
	// 	} else if(comp.operand2 == Right) {
	// 		_os << schemaRight.GetAtts()[comp.whichAtt2].name;
	// 	}
	// }
	// _os << "]";
	// _os << ", Number of Tuples = "<<numTuples;
	_os << "Join [...]"; // print without predicates

	_os << "\n";
	for(int i = 0; i < depth+1; i++)
		_os << "\t";
	_os << " ├──── " << *right;

	_os << "\n";
	for(int i = 0; i < depth+1; i++)
		_os << "\t";
	_os << " └──── " << *left;

	return _os;
}


DuplicateRemoval::DuplicateRemoval(Schema& _schema, RelationalOp* _producer) {
	schema = _schema;
	producer = _producer;
}

DuplicateRemoval::~DuplicateRemoval() {}
//Test - queries 4, 6, 7 of p4
bool DuplicateRemoval::GetNext(Record& _record) {
	while(producer->GetNext(_record)) {
		stringstream os;
		_record.print(os, schema);
		unordered_set<string>::iterator it = hash_tbl.find(os.str());
		if (it == hash_tbl.end()) {
			hash_tbl.insert(os.str());
			return true;
		}
	}
	return false;
}

ostream& DuplicateRemoval::print(ostream& _os) {
	return _os << "δ \n\t │\n\t" << *producer;
}


Sum::Sum(Schema& _schemaIn, Schema& _schemaOut, Function& _compute,
	RelationalOp* _producer) :
	schemaIn(_schemaIn),
	schemaOut(_schemaOut),
	compute(_compute),
	producer(_producer) {
}

Sum::~Sum() {}

bool Sum::GetNext(Record& _record) {
	char* recBits = new char[1];
	double result = 0; Type resType;
	bool hasRes = false;

	Record tmp;
	while(producer->GetNext(tmp)) {
		int resInt = 0; double resDbl = 0;
		resType = compute.Apply(tmp, resInt, resDbl);
		result += resInt + resDbl;

		hasRes = true;
	}

	if(hasRes) {
		int recSize;
		if(resType == Float) {
			*((double *) recBits) = result;
			recSize = sizeof(double);
		} else { // resType == Integer
			*((int *) recBits) = (int)result;
			recSize = sizeof(int);
		}

		char* recComplete = new char[sizeof(int) + sizeof(int) + recSize];
		((int*) recComplete)[0] = 2*sizeof(int) + recSize;
		((int*) recComplete)[1] = 2*sizeof(int);
		memcpy(recComplete+2*sizeof(int), recBits, recSize);

		_record.Consume(recComplete);
		return true;
	} else {
		return false;
	}


}

ostream& Sum::print(ostream& _os) {
	// _os << "SUM(";
	// // do something
	// _os << ")\n\t │\n\t" << *producer;;
	// return _os;

	return _os << "SUM(...)\n\t │\n\t" << *producer; // print without predicates
}


GroupBy::GroupBy(Schema& _schemaIn, Schema& _schemaOut, OrderMaker& _groupingAtts,
	Function& _compute,	RelationalOp* _producer) :
	schemaIn(_schemaIn),
	schemaOut(_schemaOut),
	groupingAtts(_groupingAtts),
	compute(_compute),
	producer(_producer),
	isFirst(true) {
}

GroupBy::~GroupBy() {}

bool GroupBy::GetNext(Record& _record) {
	bool hasCompute = compute.HasOps();
	// Phase 1. build a map for each group
	if(isFirst) { // this step is done only once
		Record rec;
		while(producer->GetNext(rec)) {
			// check whether aggregate function exist
			double result = 0;
			Schema schemaTmp = schemaOut;
			if(hasCompute) { // calculate aggregate function (sum)
				int resInt = 0; double resDbl = 0;
				compute.Apply(rec, resInt, resDbl);
				result = resDbl + resInt;

				// create schema having grouping atts only to get right key
				vector<int> attsToKeep;
				for(int i = 1; i < schemaOut.GetNumAtts(); i++)
					attsToKeep.push_back(i);

				schemaTmp.Project(attsToKeep);
			}

			// project record with grouping attributes
			rec.Project(&groupingAtts.whichAtts[0], groupingAtts.numAtts, schemaIn.GetNumAtts());

			// create key from the current record
			string key = rec.createKeyFromRecord(schemaTmp);
			// CompositeKey key;
			// key.extractRecord(rec, schemaTmp);

			// find key in the map and create new if not exist
			if(groups.find(key) == groups.end()) {
				GroupVal val;
				val.sum = result; val.rec = rec;
				groups[key] = val;
			} else { // do aggregate if group already exists
				groups[key].sum += result;
			}
		}

		// end of preprocessing
		groupsIt = groups.begin();
		isFirst = false;
	}

	// Phase 2. iterate groups and return each group
	if(groupsIt != groups.end()) { // get next record from the map
		Record recNew;
		if(hasCompute) {
			// create record for sum
			Record recSum;
			char* recBits = new char[1];
			int recSize;

			bool resType = compute.GetType();
			if(resType == Float) {
				*((double *) recBits) = (*groupsIt).second.sum;
				recSize = sizeof(double);
			} else { // resType == Integer
				*((int *) recBits) = (int)(*groupsIt).second.sum;
				recSize = sizeof(int);
			}

			char* recComplete = new char[sizeof(int) + sizeof(int) + recSize];
			((int*) recComplete)[0] = 2*sizeof(int) + recSize;
			((int*) recComplete)[1] = 2*sizeof(int);
			memcpy(recComplete+2*sizeof(int), recBits, recSize);

			recSum.Consume(recComplete);

			// merge sum and other attributes into new record
			recNew.AppendRecords(recSum, (*groupsIt).second.rec, 1, schemaOut.GetNumAtts()-1);

			// remove obsolete record
			(*groupsIt).second.rec.Nullify();
		} else { // if there is no aggreagate function
			// just get current record (also removes obsolete record in map by swap)
			recNew.Swap((*groupsIt).second.rec);
		}

		// return new record and advance iterator
		_record = recNew;
		groupsIt++;

		return true;
	} else { // map is empty
		return false;
	}
}

ostream& GroupBy::print(ostream& _os) {
	
	return _os << "GroupBy [...]\n\t │\n\t" << *producer; // print without predicates
}


WriteOut::WriteOut(Schema& _schema, string& _outFile, RelationalOp* _producer) :
	schema(_schema),
	outFile(_outFile),
	producer(_producer) {
	// open(file) is moved from GetNext() to here
	// so that we don't have to check
	// whether the file is opened or not for each call
	// let just the file be empty if nothing is returned
	outFileStream.open(outFile);
	
}

WriteOut::~WriteOut() {
	if (outFileStream.is_open()) {
		outFileStream.close();
	}
}

bool WriteOut::GetNext(Record& _record) {
	if (producer->GetNext(_record)) {
		_record.print(outFileStream, schema);
		outFileStream << endl;
		return true;
	}
	//If producer returns nothing, return false
	else {
		outFileStream.close();
		return false;
	}
}

ostream& WriteOut::print(ostream& _os) {
	return _os << endl << "\t" << *producer << endl << endl;
}


ostream& operator<<(ostream& _os, QueryExecutionTree& _op) {
	return _os << "QUERY EXECUTION TREE {" << endl << *_op.root << "}" << endl;
}
