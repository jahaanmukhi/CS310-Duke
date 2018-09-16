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

//Test: calling anything before thread_libinit is bad

int main(int argc, char* argv[]){
  cout << "Start main: " << endl;
  int n = arg_num;
  cout << "thread create" << endl;
  int tc = thread_create((thread_startfunc_t) a_thread, NULL);
  cout << "output from calling thread_create: " << tc << endl << endl;

  cout << "thread lock: " << n << endl;
  int tl = thread_lock(n);
  cout << "output from calling thread_lock: " << tl << endl << endl;

  cout << "thread unlock: " << n << endl;
  int tul = thread_unlock(n);
  cout << "output from calling thread_unlock: " << tul << endl << endl;

  cout << "thread signal lock and cv: " << n << " , " << n/2 << endl;
  int ts = thread_signal(n, n/2);
  cout << "output from calling thread_signal: " << ts << endl << endl;

  cout << "thread broadcast lock and cv: " <<  n << " , " << n*2 << endl;
  int tb = thread_broadcast(n, n*2);
  cout << "output from calling thread_broadcast: " << tb << endl << endl;

  cout << "thread wait lock and cv: " <<  n << " , " << sqrt(n) << endl;
  int tw = thread_wait(n, sqrt(n));
  cout << "output from calling thread_wait: " << tw << endl << endl;

  assert(thread_create((thread_startfunc_t) a_thread, NULL) == -1);

  cout << "libinit" << endl;  
  thread_libinit((thread_startfunc_t) a_thread, NULL);
}

void a_thread(void* ptr) {
  cout << "Start a_thread" << endl;
  cout << "Finish a_thread" << endl;
}