//
// CS252: MyMalloc Project
//
// The current implementation gets memory from the OS
// every time memory is requested and never frees memory.
//
// You will implement the allocator as indicated in the handout.
// 
// Also you will need to add the necessary locking mechanisms to
// support multi-threaded programs.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include "MyMalloc.h"

static pthread_mutex_t mutex;

const int ArenaSize = 2097152;
const int NumberOfFreeLists = 1;

// Header of an object. Used both when the object is allocated and freed
struct ObjectHeader {
	size_t _objectSize;         // Real size of the object.
	int _allocated;             // 1 = yes, 0 = no 2 = sentinel
	struct ObjectHeader * _next;       // Points to the next object in the freelist (if free).
	struct ObjectHeader * _prev;       // Points to the previous object.
};

struct ObjectFooter {
	size_t _objectSize;
	int _allocated;
};

//STATE of the allocator

// Size of the heap
static size_t _heapSize;

// initial memory pool
static void * _memStart;

// number of chunks request from OS
static int _numChunks;

// True if heap has been initialized
static int _initialized;

// Verbose mode
static int _verbose;

// # malloc calls
static int _mallocCalls;

// # free calls
static int _freeCalls;

// # realloc calls
static int _reallocCalls;

// # realloc calls
static int _callocCalls;

// Free list is a sentinel
static struct ObjectHeader _freeListSentinel; // Sentinel is used to simplify list operations
static struct ObjectHeader *_freeList;


//FUNCTIONS

//Initializes the heap
void initialize();

// Allocates an object 
void * allocateObject( size_t size );

// Frees an object
void freeObject( void * ptr );

// Returns the size of an object
size_t objectSize( void * ptr );

// At exit handler
void atExitHandler();

//Prints the heap size and other information about the allocator
void print();
void print_list();

// Gets memory from the OS
void * getMemoryFromOS( size_t size );

void increaseMallocCalls() { _mallocCalls++; }

void increaseReallocCalls() { _reallocCalls++; }

void increaseCallocCalls() { _callocCalls++; }

void increaseFreeCalls() { _freeCalls++; }

	extern void
atExitHandlerInC()
{
	atExitHandler();
}

void initialize()
{
	// Environment var VERBOSE prints stats at end and turns on debugging
	// Default is on
	_verbose = 1;
	const char * envverbose = getenv( "MALLOCVERBOSE" );
	if ( envverbose && !strcmp( envverbose, "NO") ) {
		_verbose = 0;
	}

	pthread_mutex_init(&mutex, NULL);
	void * _mem = getMemoryFromOS( ArenaSize + (2*sizeof(struct ObjectHeader)) + (2*sizeof(struct ObjectFooter)) );

	// In verbose mode register also printing statistics at exit
	atexit( atExitHandlerInC );

	//establish fence posts
	struct ObjectFooter * fencepost1 = (struct ObjectFooter *)_mem;
	fencepost1->_allocated = 1;
	fencepost1->_objectSize = 123456789;
	char * temp = 
		(char *)_mem + (2*sizeof(struct ObjectFooter)) + sizeof(struct ObjectHeader) + ArenaSize;
	struct ObjectHeader * fencepost2 = (struct ObjectHeader *)temp;
	fencepost2->_allocated = 1;
	fencepost2->_objectSize = 123456789;
	fencepost2->_next = NULL;
	fencepost2->_prev = NULL;

	//initialize the list to point to the _mem
	temp = (char *) _mem + sizeof(struct ObjectFooter);
	struct ObjectHeader * currentHeader = (struct ObjectHeader *) temp;
	temp = (char *)_mem + sizeof(struct ObjectFooter) + sizeof(struct ObjectHeader) + ArenaSize;
	struct ObjectFooter * currentFooter = (struct ObjectFooter *) temp;
	_freeList = &_freeListSentinel;
	currentHeader->_objectSize = ArenaSize + sizeof(struct ObjectHeader) + sizeof(struct ObjectFooter); //2MB
	currentHeader->_allocated = 0;
	currentHeader->_next = _freeList;
	currentHeader->_prev = _freeList;
	currentFooter->_allocated = 0;
	currentFooter->_objectSize = currentHeader->_objectSize;
	_freeList->_prev = currentHeader;
	_freeList->_next = currentHeader; 
	_freeList->_allocated = 2; // sentinel. no coalescing.
	_freeList->_objectSize = 0;
	_memStart = (char*) currentHeader;
}

void * allocateObject( size_t size )
{
	//Make sure that allocator is initialized
	if ( !_initialized ) {
		_initialized = 1;
		initialize();
	}

	// Add the ObjectHeader/Footer to the size and round the total size up to a multiple of
	// 8 bytes for alignment.
	
	size_t roundedSize = (size + sizeof(struct ObjectHeader) + sizeof(struct ObjectFooter) + 7) & ~7;

	// creating a temporary pointer and checking through the free list whether this new malloc memory request is satisfied by free memory list
	struct ObjectHeader * ptr = _freeList->_next;
	while(ptr != _freeList){
		if (ptr -> _objectSize >= roundedSize)
			break;
		ptr = ptr -> _next;
	}
	// storing the values of soon to be modified memory chunk into temporary memory
	size_t tobjectSize = ptr -> _objectSize;
	struct ObjectHeader * tnext = ptr -> _next;
	struct ObjectHeader * tprev = ptr -> _prev; 
	
	void * _mem = ptr;
	// shifting current pointer by roundedSize
	ptr = (struct ObjectHeader *)((char*)ptr + roundedSize);
	ptr -> _allocated = 0;
	ptr -> _objectSize = tobjectSize - roundedSize;
	ptr -> _next = tnext;
	ptr -> _prev = tprev;
	ptr -> _next -> _prev = ptr;
	ptr -> _prev -> _next = ptr;
	
	struct ObjectFooter * foot = (struct ObjectFooter *) ((char *) ptr + ptr -> _objectSize - sizeof(struct ObjectFooter));
	foot->_objectSize = ptr -> _objectSize;
	foot->_allocated = 0;
	
	// Naively get memory from the OS every time
	//void * _mem = getMemoryFromOS( roundedSize );

	// Store the size in the header
	struct ObjectHeader * o = (struct ObjectHeader *) _mem;

	o->_objectSize = roundedSize;
	o->_allocated = 1;
	
	struct ObjectFooter * p = (struct ObjectFooter *) ((char *) o + roundedSize - sizeof(struct ObjectFooter));
	p->_objectSize = roundedSize;
	p->_allocated = 1;

	pthread_mutex_unlock(&mutex);

	// Return a pointer to usable memory
	return (void *) (o + 1);

}

void freeObject( void * ptr )
{
	// Add your code here
	struct ObjectHeader * hdr = (struct ObjectHeader*) ((char *)ptr - sizeof(struct ObjectHeader));
	struct ObjectFooter * ftr = (struct ObjectFooter*) ((char *)hdr + hdr->_objectSize - sizeof(struct ObjectFooter));
	hdr -> _allocated = 0;
	ftr -> _allocated = 0;
	/*struct ObjectHeader * ptr = _freeList->_next;
	while(ptr <= hdr){
		ptr = ptr -> _next;
	}*/
	struct ObjectHeader * nexthdr = (struct ObjectHeader*) ((char*)ftr + sizeof(struct ObjectFooter));
	struct ObjectHeader *temphdr = NULL;
	struct ObjectFooter *prevftr = (struct ObjectFooter*) ((char *) ftr - ftr->_objectSize);
	if (prevftr > _memStart) {
		if(prevftr -> _allocated == 0) {
			temphdr = (struct ObjectHeader *) ((char*) prevftr - prevftr-> _objectSize + sizeof(struct ObjectFooter)); 
			prevftr -> _objectSize = prevftr -> _objectSize + ftr -> _objectSize;
			temphdr -> _objectSize = prevftr -> _objectSize;
		}
		else if (nexthdr -> _allocated == 0) {
			hdr -> _objectSize = hdr -> _objectSize + nexthdr -> _objectSize;
			hdr -> _next = nexthdr -> _next;
			hdr -> _prev = nexthdr -> _prev;
			hdr -> _next -> _prev = hdr;
			hdr -> _prev -> _next = hdr;
			ftr = (struct ObjectFooter*) ((char *)hdr + hdr->_objectSize - sizeof(struct ObjectFooter));
			ftr -> _objectSize = hdr -> _objectSize;
		}
	}	
		
	return;

}

size_t objectSize( void * ptr )
{
	// Return the size of the object pointed by ptr. We assume that ptr is a valid obejct.
	struct ObjectHeader * o =
		(struct ObjectHeader *) ( (char *) ptr - sizeof(struct ObjectHeader) );

	// Substract the size of the header
	return o->_objectSize;
}

void print()
{
	printf("\n-------------------\n");

	printf("HeapSize:\t%zd bytes\n", _heapSize );
	printf("# mallocs:\t%d\n", _mallocCalls );
	printf("# reallocs:\t%d\n", _reallocCalls );
	printf("# callocs:\t%d\n", _callocCalls );
	printf("# frees:\t%d\n", _freeCalls );

	printf("\n-------------------\n");
}

void print_list()
{
	printf("FreeList: ");
	if ( !_initialized ) {
		_initialized = 1;
		initialize();
	}
	struct ObjectHeader * ptr = _freeList->_next;
	while(ptr != _freeList){
		long offset = (long)ptr - (long)_memStart;
		printf("[offset:%ld,size:%zd]",offset,ptr->_objectSize);
		ptr = ptr->_next;
		if(ptr != NULL){
			printf("->");
		}
	}
	printf("\n");
}

void * getMemoryFromOS( size_t size )
{
	// Use sbrk() to get memory from OS
	_heapSize += size;

	void * _mem = sbrk( size );

	if(!_initialized){
		_memStart = _mem;
	}

	_numChunks++;

	return _mem;
}

void atExitHandler()
{
	// Print statistics when exit
	if ( _verbose ) {
		print();
	}
}

//
// C interface
//

	extern void *
malloc(size_t size)
{
	pthread_mutex_lock(&mutex);
	increaseMallocCalls();

	return allocateObject( size );
}

	extern void
free(void *ptr)
{
	pthread_mutex_lock(&mutex);
	increaseFreeCalls();

	if ( ptr == 0 ) {
		// No object to free
		pthread_mutex_unlock(&mutex);
		return;
	}

	freeObject( ptr );
}

	extern void *
realloc(void *ptr, size_t size)
{
	pthread_mutex_lock(&mutex);
	increaseReallocCalls();

	// Allocate new object
	void * newptr = allocateObject( size );

	// Copy old object only if ptr != 0
	if ( ptr != 0 ) {

		// copy only the minimum number of bytes
		size_t sizeToCopy =  objectSize( ptr );
		if ( sizeToCopy > size ) {
			sizeToCopy = size;
		}

		memcpy( newptr, ptr, sizeToCopy );

		//Free old object
		freeObject( ptr );
	}

	return newptr;
}

	extern void *
calloc(size_t nelem, size_t elsize)
{
	pthread_mutex_lock(&mutex);
	increaseCallocCalls();

	// calloc allocates and initializes
	size_t size = nelem * elsize;

	void * ptr = allocateObject( size );

	if ( ptr ) {
		// No error
		// Initialize chunk with 0s
		memset( ptr, 0, size );
	}

	return ptr;
}

