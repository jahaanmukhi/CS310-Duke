#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "thread.h"


using namespace std;

unsigned int arg_num = 161;

void a_thread(void* ptr);
void b_thread(void* ptr);

//Test: unlock a lock you don't have and lock then try to acquire it again, and exit with locks still held

int main(int argc, char* argv[]){
	thread_libinit((thread_startfunc_t) a_thread, NULL);
}

void a_thread(void* ptr){ 
	cout << "Start a_thread\n";
	cout << "create b_thread\n";
	thread_create((thread_startfunc_t) b_thread, NULL);			
	cout << "unlock: " << arg_num << endl;
	thread_unlock(arg_num);	
	cout << "Finish a_thread\n";
}
void b_thread(void* ptr){
	cout << "Start b_thread\n";
	cout << "request lock: " << arg_num +1 << endl;
	thread_lock(arg_num+1);
	cout << "got lock: " << arg_num +1 << endl;
	cout << "Unlock: " << arg_num << endl;
	thread_unlock(arg_num);
	cout << "request lock: " << arg_num << endl;
	thread_lock(arg_num);
	cout << "got lock: " << arg_num << endl;
	cout << "Finish b_thread\n";
}
