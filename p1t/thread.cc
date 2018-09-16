#include <iostream>
#include <queue>
#include <deque>
#include <map>
#include <string>
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <assert.h>
#include <ucontext.h>
#include "interrupt.h"
#include "thread.h"

using namespace std;

struct thread {
  char* stack;
  ucontext_t* ucontext;
  bool done;
  unsigned int id;
};

map<unsigned int, deque<thread*> > lockQ;
map<unsigned int, deque<thread*> > cvQ;
map<unsigned int, unsigned int> lockHeld;

int ID_thread = 1;
deque<thread*> readyQ;
bool initialized = false;
thread* current_thread;
ucontext* ucontext_iter;

int thread_libinit(thread_startfunc_t func, void *arg) {
  interrupt_disable();
  if (initialized) {
    interrupt_enable();
    return -1;
  }
  initialized = true;
  interrupt_enable();
  int tc = thread_create(func, arg);
  interrupt_disable();
  if (tc == -1) {
    return -1;
  }
  try {
    ucontext_iter = new ucontext_t;
  }
  catch (std::bad_alloc b) {
    delete ucontext_iter;
    return -1;
  }
  getcontext(ucontext_iter);
  swapcontext(ucontext_iter, current_thread->ucontext);
  while(readyQ.size()>0) {
    if (current_thread->done) {
      delete current_thread->stack;
      delete current_thread->ucontext;
      delete current_thread;
      current_thread = NULL;
    }
    thread* next = readyQ.front();
    readyQ.pop_front();
    current_thread = next;
    swapcontext(ucontext_iter, current_thread->ucontext);
  }
  if (current_thread != NULL) {
    delete current_thread->stack;
    delete current_thread->ucontext;
    delete current_thread;
    current_thread = NULL;
  }
  cout << "Thread library exiting.\n";
  exit(0); 
  return 0;
}


static int func_run (thread_startfunc_t func, void* arg) {
  interrupt_enable();
  func(arg);
  interrupt_disable();
  current_thread->done = true;
  swapcontext(current_thread->ucontext, ucontext_iter);
  return 0;
}


int thread_create(thread_startfunc_t func, void *arg) {
  interrupt_disable();
  if(!initialized) {
    interrupt_enable();
    return -1;
  }
  thread* new_thread;
  try {
    try {
      new_thread = new thread;
    } 
    catch (std::bad_alloc b) {
      interrupt_enable();
      return -1;
    }
    try{
      new_thread->ucontext = new ucontext_t;
    }
    catch(std::bad_alloc b){
      interrupt_enable();
      return -1;
    }
    try {
      new_thread->stack = new char[STACK_SIZE];;
    }
    catch(std::bad_alloc b){
      interrupt_enable();
      return -1;
    }
    getcontext(new_thread->ucontext);
    new_thread->ucontext->uc_stack.ss_sp = new_thread->stack;
    new_thread->ucontext->uc_stack.ss_size = STACK_SIZE;
    new_thread->ucontext->uc_stack.ss_flags = 0;
    new_thread->ucontext->uc_link = NULL;
    new_thread->done = false;
    new_thread->id = ID_thread;
    ID_thread++;
    makecontext(new_thread->ucontext, (void (*)()) func_run, 2, func, arg);
    if (current_thread != NULL) {
      readyQ.push_back(new_thread);
    } else {
      current_thread = new_thread;
    }
    interrupt_enable();
    return 0;
  }
  catch (std::bad_alloc b) {
    //cout << "bad alloc caught while making thread\n";
    delete new_thread->ucontext;
    delete new_thread->stack;
    delete new_thread;
    interrupt_enable();
    return -1;
  }
}

int thread_lock(unsigned int lock_this) {
  interrupt_disable();
  if (!initialized) {
    interrupt_enable();
    return -1;
  }
  if(lockHeld[lock_this] != 0){
    if (lockHeld[lock_this] != current_thread->id) {
      lockQ[lock_this].push_back(current_thread);
      swapcontext(current_thread->ucontext, ucontext_iter);
      interrupt_enable();
      return 0;
    }
    else{
      interrupt_enable();
      return -1;
    }
  }
  else{
    lockHeld[lock_this] = current_thread->id;
    interrupt_enable();
    return 0;
  }
}

int thread_unlock(unsigned int lock_this) {
  interrupt_disable();
  if (!initialized) {
    interrupt_enable();
    return -1;
  }
  if (lockHeld.count(lock_this) == 0){
    //cout << "no thread holds this lock\n";
    interrupt_enable();
    return -1;
  }
  else if(lockHeld[lock_this] != current_thread->id){
    //cout << "lock doesn't exist\n";
    interrupt_enable();
    return -1;
  }
  else{
    //unlock
    lockHeld[lock_this] = 0;
    //if there's another thread waiting for this lock
    if (!lockQ[lock_this].empty()) {
      thread* waiting_forlock_thread = lockQ[lock_this].front();
      lockQ[lock_this].pop_front();
      lockHeld[lock_this] = waiting_forlock_thread->id;
      readyQ.push_back(waiting_forlock_thread);
    }
    interrupt_enable();
    return 0;
  }
}

int thread_wait(unsigned int lock_this, unsigned int cond_this) {
  interrupt_disable();
  if (!initialized) {
    interrupt_enable();
    return -1;
  }
  int thread_unlock_num;
  // THREAD_UNLOCK
  if (lockHeld.count(lock_this) == 0){
    //cout << "no thread holds this lock\n";
    thread_unlock_num = -1;
  }
  else if(lockHeld[lock_this] != current_thread->id){
    //cout << "lock doesn't exist\n";
    thread_unlock_num = -1;
  }
  else{
    //unlock
    lockHeld[lock_this] = 0;
    //if there's another thread waiting for this lock
    if (!lockQ[lock_this].empty()) {
      thread* waiting_forlock_thread = lockQ[lock_this].front();
      lockQ[lock_this].pop_front();
      lockHeld[lock_this] = waiting_forlock_thread->id;
      readyQ.push_back(waiting_forlock_thread);
    }
    thread_unlock_num = 0;
  }
  if(thread_unlock_num == 0){
    //add current thread to cv queue
    cvQ[cond_this].push_back(current_thread);
    swapcontext(current_thread->ucontext, ucontext_iter);
    // THREAD_LOCK
    int thread_lock_num;
    if(lockHeld[lock_this] != 0){
      if (lockHeld[lock_this] != current_thread->id) {
        lockQ[lock_this].push_back(current_thread);
        swapcontext(current_thread->ucontext, ucontext_iter);
        thread_lock_num = 0;
      }
      else{
        //cout << "ERROR: thread already holds lock\n";
        thread_lock_num = -1;
      }
    }
    else{
      lockHeld[lock_this] = current_thread->id;
      thread_lock_num = 0;
    }
    interrupt_enable();
    return thread_lock_num;
  }
  interrupt_enable();
  return -1;
}

int thread_signal(unsigned int lock_this, unsigned int cond_this) {
  interrupt_disable();
  if (!initialized) {
    interrupt_enable();
    return -1;
  }
  //if there is a thread waiting on the cv, then move it readyQ
  if (!cvQ[cond_this].empty()) {
    thread* woke_thread = cvQ[cond_this].front();
    cvQ[cond_this].pop_front();
    readyQ.push_back(woke_thread);
  }
  interrupt_enable();
        return 0;
}

int thread_broadcast(unsigned int lock_this, unsigned int cond_this) {
  interrupt_disable();
  if (!initialized) {
    interrupt_enable();
    return -1;
  }
  
  //while there are still threads waiting on cv
  while (!cvQ[cond_this].empty()) {
    thread* wait_thread = cvQ[cond_this].front();
    cvQ[cond_this].pop_front();
    readyQ.push_back(wait_thread);
  }
  interrupt_enable();
  return 0;
}

int thread_yield(void) {
  interrupt_disable();
  if (!initialized) {
    interrupt_enable();
    return -1;
  } 
  else {
    readyQ.push_back(current_thread);
    swapcontext(current_thread->ucontext, ucontext_iter);
    interrupt_enable();
    return 0;
  }
}