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

//Test: waiting on a lock that you don't have

int main(int argc, char *argv[]){
    thread_libinit((thread_startfunc_t) a_thread, NULL);    
}

void a_thread(void *arg){
    cout << "Start a_thread\n";
    int n = arg_num;
    cout << "create b_thread" << endl;
    thread_create((thread_startfunc_t) b_thread, NULL);
    cout << "wait on lock and cv: " << n << " " << 1 << endl;
    thread_wait(n,1);
    cout << "return from wait on lock and cv " << n << " , " << 1 << endl;
    cout<<"request lock: " << n << endl;
    thread_lock(n);
    cout<<"got lock: " << n << endl;
    cout << "a_thread yield" << endl;
    thread_yield();
    cout << "a_thread return from yield " << endl;
    thread_wait(n,1);
    thread_unlock(n);
    cout<<"unlock: " << n << endl;
    cout << "Finish a_thread\n";
}
void b_thread(void *arg){
    cout<<"Start b_thread\n";
    int n = arg_num;
    cout <<"b_thread wait on lock and cv: " << n << " , " << 1 << endl;
    thread_wait(n,1);
    cout<<"b_thread return from wait\n";
    cout<<"request lock: " << n << endl;
    thread_lock(n);
    cout<<"got lock: " << n << endl;
    cout<<"Finished b_thread\n";
}

