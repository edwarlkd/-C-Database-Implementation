CC = g++ -g -O0 -Wno-deprecated -std=gnu++11
LIBS = -lsqlite3 -lfl

tag = -i

ifdef linux
	tag = -n
endif

### main.out ###
main.out: QueryParser.o QueryLexer.o Schema.o Record.o File.o DBFile.o Comparison.o Function.o RelOp.o Catalog.o QueryOptimizer.o QueryCompiler.o TableDataStructure.o InefficientMap.o CompositeKey.o FibHeap.o main.o
	$(CC) -o main.out main.o QueryParser.o QueryLexer.o Schema.o Record.o File.o DBFile.o Comparison.o Function.o RelOp.o Catalog.o QueryOptimizer.o QueryCompiler.o TableDataStructure.o InefficientMap.o CompositeKey.o FibHeap.o $(LIBS)

main.o:	main.cc
	$(CC) -c main.cc

Schema.o: Schema.cc
	$(CC) -c Schema.cc

Record.o: Schema.cc Record.cc
	$(CC) -c Record.cc

File.o: Schema.cc Record.cc File.cc
	$(CC) -c File.cc

DBFile.o: Schema.cc Record.cc File.cc DBFile.cc
	$(CC) -c DBFile.cc

Comparison.o: Schema.cc Record.cc Comparison.cc
	$(CC) -c Comparison.cc

Function.o: Schema.cc Record.cc Function.cc
	$(CC) -c Function.cc

RelOp.o: Schema.cc Record.cc Comparison.cc CompositeKey.cc RelOp.cc
	$(CC) -c RelOp.cc

QueryOptimizer.o: Schema.cc Record.cc Comparison.cc RelOp.cc QueryOptimizer.cc
	$(CC) -c QueryOptimizer.cc

QueryCompiler.o: Schema.cc Record.cc Comparison.cc RelOp.cc QueryOptimizer.cc QueryCompiler.cc
	$(CC) -c QueryCompiler.cc

Catalog.o: TableDataStructure.cc InefficientMap.cc Schema.cc DBFile.cc Catalog.cc
	$(CC) -c Catalog.cc

TableDataStructure.o: Schema.cc TableDataStructure.cc
	$(CC) -c TableDataStructure.cc

FibHeap.o: Record.cc DBFile.cc CompositeKey.cc FibHeap.cc
	$(CC) -c FibHeap.cc

PairingHeap.o: PairingHeap.cc
	$(CC) -c PairingHeap.cc

Heapdatastructure.o: Heapdatastructure.cc CompositeKey.cc
	$(CC) -c Heapdatastructure.cc

# EfficientMap.o: EfficientMap.cc
# 	$(CC) -c EfficientMap.cc

InefficientMap.o: InefficientMap.cc
	$(CC) -c InefficientMap.cc

QueryParser.o: QueryParser.y
	yacc --defines=QueryParser.h -o QueryParser.c QueryParser.y
	sed $(tag) QueryParser.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/"
	g++ -c -Wno-write-strings QueryParser.c

QueryLexer.o: QueryLexer.l
	lex -o QueryLexer.c QueryLexer.l
	gcc -c QueryLexer.c

CompositeKey.o: Record.cc Comparison.cc CompositeKey.cc
	$(CC) -c CompositeKey.cc

### dbgen ###
dbgen: Schema.o File.o DBFile.o Record.o Catalog.o TableDataStructure.o InefficientMap.o dbgen.o
	$(CC) -o dbgen dbgen.o Schema.o File.o DBFile.o Record.o Catalog.o TableDataStructure.o InefficientMap.o $(LIBS)

dbgen.o: Schema.cc DBFile.cc Catalog.cc dbgen.cc
	$(CC) -c dbgen.cc

dbtest: Schema.o File.o DBFile.o Record.o Catalog.o TableDataStructure.o InefficientMap.o dbtest.o
	$(CC) -o dbtest.out dbtest.o Schema.o File.o DBFile.o Record.o Catalog.o TableDataStructure.o InefficientMap.o $(LIBS)

dbtest.o: Schema.cc DBFile.cc Catalog.cc dbtest.cc
	$(CC) -c dbtest.cc

### test ###
testbh: Heapdatastructure.o CompositeKey.o testbh.o
		$(CC) -o testbh.out testbh.o Heapdatastructure.o CompositeKey.o $(LIBS)

testbh.o: CompositeKey.cc testbh.cc
		$(CC) -c testbh.cc

cktest: Schema.o File.o DBFile.o Record.o Catalog.o TableDataStructure.o InefficientMap.o CompositeKey.o cktest.o
	$(CC) -o cktest.out cktest.o Schema.o File.o DBFile.o Record.o Catalog.o TableDataStructure.o InefficientMap.o CompositeKey.o $(LIBS)

cktest.o: Schema.cc DBFile.cc Catalog.cc CompositeKey.cc cktest.cc
	$(CC) -c cktest.cc

fhtest: Schema.o File.o DBFile.o Record.o Catalog.o TableDataStructure.o InefficientMap.o CompositeKey.o FibHeap.o fhtest.o
	$(CC) -o fhtest.out fhtest.o Schema.o File.o DBFile.o Record.o Catalog.o TableDataStructure.o InefficientMap.o CompositeKey.o FibHeap.o $(LIBS)

fhtest.o: Schema.cc DBFile.cc Catalog.cc CompositeKey.cc FibHeap.cc fhtest.cc
	$(CC) -c fhtest.cc

### clean ###
clean:
	rm -f *.o
	rm -f *.out
	rm -f QueryLexer.c
	rm -f QueryParser.c
	rm -f QueryParser.h
	rm -f dbgen
	rm -f .tmp/*
