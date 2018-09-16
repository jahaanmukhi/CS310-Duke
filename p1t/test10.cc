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
void c_thread(void *arg);

//TEST: thread exists while still holding lock, this should be ok, thread library should still exit

int main(int argc, char *argv[]){
	thread_libinit((thread_startfunc_t) a_thread, NULL);
}

void a_thread(void *arg){
	cout << "Start a_thread\n";
	cout << "a_thread requests lock " << arg_num << endl;
	thread_lock(arg_num);
	cout << "a_thread acquires lock " << arg_num << endl;
	cout << "create b_thread\n";
	thread_create((thread_startfunc_t) b_thread, NULL);
	thread_create((thread_startfunc_t) c_thread, NULL);
	cout << "a_thread sleep\n";
	thread_yield();
	cout << "a_thread wake\n";
	cout << "Finish a_thread\n";
}

void b_thread(void *arg) {
	cout << "Start b_thread\n";
	cout << "b_thread requests lock " << arg_num << endl;
	thread_lock(arg_num);
	cout << "b_thread acquires lock " << arg_num << endl;
	cout << "b_thread unlocks lock " << arg_num << endl;
	thread_unlock(arg_num);
	cout << "Finish b_thread\n";
}

void c_thread(void *arg) {
	cout << "Start c_thread\n";
	cout << "c_thread requests lock " << arg_num+1 << endl;
	thread_lock(arg_num+1);
	cout << "c_thread acquires lock " << arg_num+1 << endl;
	cout << "c_thread wait on lock and cv " << arg_num+1 << " , " << 1 << endl;
	thread_wait(arg_num+1, 1);
	cout << "c_thread return from wait on lock and cv " << arg_num+1 << " , " << 1 << endl;
	cout << "c_thread unlocks lock " << arg_num+1 << endl;
	thread_unlock(arg_num+1);

	cout << "Finish c_thread\n";
}
