#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;


void c_thread(void* ptr){
  cout<<"Start c_thread\n";
  cout<<"request lock c\n";
  thread_lock(0);

  cout<<"c_thread signal\n";
  thread_signal(0,0);
  cout<<"unlock: c\n";
  thread_unlock(0);
  cout<<"Finish c_thread\n";
}




void b_thread(void* ptr){
  cout<<"Start b_thread\n";
  cout<<"request lock b\n";
  thread_lock(0);

  cout<<"b_thread wait\n";
  thread_wait(0,0);
  cout<<"b_thread awake\n";
  cout<<"unlock: b"<<endl;
  thread_unlock(0);
  cout<<"Finish b_thread\n";
}

void a_thread(void* ptr){
  cout<<"Start a_thread\n";
  cout<<"create b_thread\n";
  thread_create((thread_startfunc_t) b_thread,NULL);

  cout<<"create c_thread\n";
  thread_create((thread_startfunc_t) c_thread,NULL);

  //cout<<"unlock: "<<endl;
  //thread_unlock(0);
  cout<<"Finish a_thread\n";
}

int main() {
  if (thread_libinit( (thread_startfunc_t) a_thread, (void *) 100)) {
    cout << "thread_libinit failed\n";
    exit(1);
  }
}
