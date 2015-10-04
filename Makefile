
CC = gcc -g

all: MyMalloc.so test0 test1 test1-1 test1-2 test1-3 test1-4 test1 test2 test3 test4 test5 test6 test7

MyMalloc.so: MyMalloc.c
	$(CC) -fPIC -c -g MyMalloc.c
	gcc -shared -o MyMalloc.so MyMalloc.o

test0: test0.c MyMalloc.c
	$(CC) -o test0 test0.c MyMalloc.c

test1-1: test1-1.c MyMalloc.c
	$(CC) -o test1-1 test1-1.c MyMalloc.c

test1-2: test1-2.c MyMalloc.c
	$(CC) -o test1-2 test1-2.c MyMalloc.c

test1-3: test1-3.c MyMalloc.c
	$(CC) -o test1-3 test1-3.c MyMalloc.c

test1-4: test1-4.c MyMalloc.c
	$(CC) -o test1-4 test1-4.c MyMalloc.c

test1: test1.c MyMalloc.c
	$(CC) -o test1 test1.c MyMalloc.c

test2: test2.c MyMalloc.c
	$(CC) -o test2 test2.c MyMalloc.c

test3: test3.c MyMalloc.c
	$(CC) -o test3 test3.c MyMalloc.c

test4: test4.c MyMalloc.c
	$(CC) -o test4 test4.c MyMalloc.c

test5: test5.c MyMalloc.c
	$(CC) -o test5 test5.c MyMalloc.c 

test6: test6.c MyMalloc.c
	$(CC) -o test6 test6.c MyMalloc.c -lpthread

test7: test7.c MyMalloc.so
	$(CC) -o test7 test7.c MyMalloc.c

runtestEXTRA:
	LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:`pwd` && export LD_LIBRARY_PATH && \
	echo "--- Running testEXTRA ---" && \
    ./firefox

clean:
	rm -f *.o test0 test1 test1-1 test1-2 test1-3 test1-4 test2 test3 test4 test5 test6 test7 MyMalloc.so core a.out *.out *.txt
