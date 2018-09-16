#include <stdio.h>
#include <stdlib.h> //for exit
#include <string.h>
#include <assert.h>

#include "dmm.h"

int main(int argc, char *argv[]) {
	char *array1, *array2, *array3;
	int i;

	printf("malloc(10)\n");
	array1 = (char*)dmalloc(10);
	if(array1 == NULL)
	{
		fprintf(stderr,"call to dmalloc() failed\n");
		fflush(stderr);
		exit(1);
	}
	for(i=0; i < 9; i++)
	{
		array1[i] = 'a';
	}
	array1[9] = '\0';

	printf("String: %s\n",array1);
    assert(strncmp(array1,"aaaaaaaaa",9)==0);
	
	printf("malloc(100)\n");
	array2 = (char*)dmalloc(100);
	if(array2 == NULL)
	{
		fprintf(stderr,"call to dmalloc() failed\n");
		fflush(stderr);
		exit(1);
	}
	for(i=0; i < 99; i++)
	{
		array2[i] = 'b';
	}
	array2[99] = '\0';
	printf("String: %s\n",array2);
    assert(strncmp(array2,"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",99)==0);

	printf("malloc(50)\n");
	array3 = (char*)dmalloc(50);

	if(array3 == NULL)
	{
		fprintf(stderr,"call to dmalloc() failed\n");
		fflush(stderr);
		exit(1);
	}
	for(i=0; i < 49; i++)
	{
		array3[i] = 'c';
	}
	array3[49] = '\0';

	printf("String: %s\n",array3);
    assert(strncmp(array3,"ccccccccccccccccccccccccccccccccccccccccccccccccc",49)==0);

	printf("free(10)\n");
	dfree(array1);
	printf("After free\n");
	print_freelist();
	printf("free(50)\n");
	dfree(array3);
	printf("After free\n");
	print_freelist();
	printf("free(100)\n");
	dfree(array2);
	printf("After free\n");
	print_freelist();

	printf("malloc(510)\n");
	array3 = (char*)dmalloc(510);

	if(array3 == NULL)
	{
		fprintf(stderr,"call to dmalloc() failed\n");
		fflush(stderr);
		exit(1);
	}
	for(i=0; i < 509; i++)
	{
		array3[i] = 'c';
	}
	array3[509] = '\0';
    //assert(strncmp(array3,"ccccccccccccccccccccccccccccccccccccccccccccccccc")==0);

	printf("malloc(510)\n");
	array3 = (char*)dmalloc(510);
    if(array3 == NULL)
	{
		fprintf(stderr,"call to dmalloc() failed\n");
		fflush(stderr);
		//exit(1); // We expect the fail here. So do not exit
	}

	printf("Coalescing test cases passed!\n");
	return(0);
}
