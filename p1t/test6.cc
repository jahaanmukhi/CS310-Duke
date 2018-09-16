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

void a_thread(void *arg);
void b_thread(void *arg);

unsigned int cv1 = 1;
unsigned int cv2 = 2;

//Test: one lock but multiple CVs
//Test: one CV but multiple locks

int main(int argc, char *argv[]){
    thread_libinit((thread_startfunc_t) a_thread, NULL);
}

void a_thread(void *arg){
    cout << "Start a_thread\n";
    int n = arg_num;
    cout << "create b_thread\n";
    thread_create((thread_startfunc_t) b_thread, NULL);
    
    thread_lock(n);
    cout << "a_thread wait lock and cv: " << n << " , " << cv1 << endl;
    thread_wait(n,cv1);
    cout << "a_thread awake from lock and cv: " << n << " , " << cv1 << endl << endl;
    thread_yield();
    cout << "a_thread wait lock and cv: " << n << " , " << cv2 << endl;
    thread_wait(n,cv2);
    cout << "a_thread awake from lock and cv: " << n << " , " << cv2 << endl << endl;
    thread_unlock(n);

    thread_lock(n+1);
    cout << "a_thread wait lock and cv: " << n+1 << " , " << cv2 << endl;
    thread_wait(n+1,cv2);
    cout << "a_thread awake from lock and cv: " << n+1 << " , " << cv2 << endl << endl;
    thread_unlock(n+1);
    cout << "Finish a_thread\n";
}

void b_thread(void *arg){
    cout <<"Start b_thread\n";
    int n = arg_num;
    thread_lock(n);
    cout << "b_thread signal lock and cv: " << n << " , " << cv1 << endl << endl;
    thread_signal(n, cv1);
    cout << "b_thread signal lock and cv: " << n << " , " << cv2 << endl << endl;
    thread_signal(n, cv2);
    thread_unlock(n);
    
    thread_yield();
    
    thread_lock(n+1);
    cout << "b_thread signal lock and cv: " << n+1 << " , " << cv2 << endl << endl;
    thread_signal(n+1, cv2);
    thread_unlock(n+1);
    cout << "Finish b_thread\n";
}