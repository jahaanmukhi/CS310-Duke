#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"


using namespace std;

unsigned int arg_num = 161;

void a_thread(void *arg);
void b_thread(void *arg);

//TEST: thread exists while still holding lock, this should be ok, thread library should still exit

int main(int argc, char *argv[]){
	thread_libinit((thread_startfunc_t) a_thread, NULL);
}
void a_thread(void *arg) {
	cout << "Start a_thread\n";
	unsigned int n = arg_num;
	cout << "signaling without lock: " << n-1 << endl;
	thread_signal(n-1, 1);
	cout << "acquiring lock: " << n << endl;
	thread_lock(n);
	cout << "got lock: " << n << endl;
	thread_create((thread_startfunc_t) b_thread, NULL);
	cout << "done creating b_thread\n";
	thread_yield();
	cout << "End a_thread\n";
}

void b_thread(void *arg) {
	cout << "Start b_thread\n";
	unsigned int n = arg_num;
	cout << "acquiring lock: " << n << endl;
	thread_lock(n+1);
	cout << "got lock: " << n << endl;
	cout << "b_thread yield " << endl;
	thread_yield();
	cout << "b_thread return from yield" << endl;
	cout << "broadcast lock: " << n << endl;
	thread_broadcast(n, 1);
	cout << "broadcast without lock: " << n+1 << endl;
	thread_broadcast(n+1, 1);
	cout << "request lock: " << n << endl;
	thread_lock(n);
	cout << "got lock: " << n << endl;
	thread_unlock(n);
	cout << "unlock: " << n << endl;
	cout << "create a_thread " << endl;
	thread_create((thread_startfunc_t) a_thread, NULL);
	cout << "b_thread yield" << endl;
	thread_yield();
	cout << "b_thread return from yield" << endl;
	thread_unlock(n+1);
	cout << "unlock: " << n+1 << endl;
	cout << "End b_thread\n";
}
