#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include "thread.h"

using namespace std;

unsigned int arg_num = 161;
unsigned int lock = 36;

void a_thread(void *arg);
void b_thread(void *arg);
void c_thread(void *arg);
void initialize(void *arg);

//TEST: wake up other threads even though you don't own lock

int main(int argc, char *argv[]){
	thread_libinit((thread_startfunc_t) initialize, NULL);
}

void initialize(void *arg) {
	thread_create((thread_startfunc_t) a_thread, NULL);	
	thread_create((thread_startfunc_t) b_thread, NULL);	
	thread_create((thread_startfunc_t) c_thread, NULL);	
}

void a_thread(void *arg){
	cout << "Start a_thread\n";
	cout << "a_thread request lock\n";
	thread_lock(lock);
	cout << "a_thread got lock\n";
	cout << "a_thread sleep\n";
	thread_wait(lock, 1);
	cout << "a_thread wake\n";
	arg_num++;
	cout << "increment arg_num to: " << arg_num << endl;
	cout << "a_thread unlock\n";
	thread_unlock(lock);
	cout << "Finish a_thread\n";
}

void b_thread(void *arg) {
	cout << "Start b_thread\n";
	cout << "b_thread request lock\n";
	thread_lock(lock);
	cout << "b_thread got lock\n";
	cout << "b_thread sleep\n";
	thread_wait(lock, 1);
	cout << "b_thread wake\n";
	arg_num++;
	cout << "increment arg_num to: " << arg_num << endl;
	cout << "b_thread unlock\n";
	thread_unlock(lock);
	cout << "Finish b_thread\n";
}

void c_thread(void *arg) {
	cout << "Start c_thread\n";
	cout << "c_thread broadcast\n";
	thread_broadcast(lock, 1);
	cout << "Finish c_thread\n";
}