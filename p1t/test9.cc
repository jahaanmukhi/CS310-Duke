#include "thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <inttypes.h>

using namespace std;

// sandwich board 
// sandwich_arr is an array of the sandwich numbers
// cashier_arr is an array of cashier ids of the cashiers who posted the respective sandwiches 
// size is current number of orders posted to the board
// upon ready: change sanwich_arr[i] and cashier_arr[i] to -1 and decrement size
struct board {
	int* sandwich_arr;
	int* cashier_arr;
	int size;
};

struct board sandwich_board;

// counter for how many total orders have been readied
int global_counter;

// counter for keeping track of how many cashiers
int global_cashier_id;

// total cashiers
int max_cashiers;
int running_cashiers;
int max_board_size;

bool* done_posting;

unsigned int lock = 88888888;
unsigned int order_ready_CV = 99999999;
unsigned int order_posted_CV = 10000000;

int order_arr[5][2] = {{52, 782}, {911, 354}, {824, 563}, {301, 234}, {632, 14}};
int orders_per_cashier = 2;

void sandwichMaker(void* args);
void cashiers(void* argv);
 
void init(void* argv) {
  thread_create((thread_startfunc_t) sandwichMaker, (void*) NULL);
  for (int i = 2; i<max_cashiers+2; i++) {
    //create cashier threads
    thread_create((thread_startfunc_t) cashiers, (void*) (intptr_t) i);
  }
}
  

void print_board() {
  cout << "sandwich_board.size: " << sandwich_board.size << endl;
  cout << "running_cashiers: " << running_cashiers << endl;
  cout << "sandwich board: \n";
  for (int i=0; i<max_board_size; i++) {
    cout << "cashier ids: " << sandwich_board.cashier_arr[i] << " sandwich ids: " << sandwich_board.sandwich_arr[i] << endl;
  }
  //cout << "done_posting: \n";
  for (int i=0; i<max_cashiers; i++) {
    //cout << "cashier: " << i << " done posting: " << done_posting[i] << endl;
  }
  cout << "\n"; 
}

bool check_cashier_wait (int cashier_id_num) {
  if (sandwich_board.size == 0) {
    return false;
  }
  for (int i = 0; i < max_board_size; i++) {
    // cout <<  "cashier_id_num: " << cashier_id_num << endl;

    if (cashier_id_num == sandwich_board.cashier_arr[i]) { 
      return true;
    }
  }
  return false;
}

void cashiers (void* args) {
  thread_lock(lock);
  int cashier_id = (intptr_t) args;
  cashier_id -= 2;
  // cashier_id = global_cashier_id;
  global_cashier_id++;

  //for this cashier: put all sandwich orders into vector 
  vector<int> orders_vector;
  int* arr = order_arr[cashier_id];
  for (int i=0; i<orders_per_cashier; i++) {
    orders_vector.push_back(arr[i]);
  }

  while(orders_vector.size() != 0) {
    // thread_lock(lock);
    //cout<<"cashier_id " <<cashier_id<<endl;
    while((sandwich_board.size == max_board_size) || (check_cashier_wait(cashier_id) == true) ) {
      //cout << "cashier wait " << cashier_id << endl;
      // cout << check_cashier_wait(cashier_id) << endl;
      // cout << "before wait, cashier_id: " << cashier_id <<endl;
      thread_wait(lock, order_ready_CV);
      // cout << "after wait, cashier_id: " << cashier_id <<endl;
    }
    // cout << "orders_vector.size(): " << orders_vector.size() << endl;
    int order = orders_vector.at(0);
    orders_vector.erase(orders_vector.begin());
    int loop = 0;
    while (loop < max_board_size) {
      //for(int loop = 0; loop<board_size; loop++){
      if (sandwich_board.sandwich_arr[loop] == -1 ) {
      	sandwich_board.sandwich_arr[loop] = order;
      	sandwich_board.cashier_arr[loop] = cashier_id;
      	break;	
      }
      loop++;
    }
    sandwich_board.size++;
    cout << "POSTED: cashier "<< cashier_id << " sandwich " << order<< endl;
    //print_board();
    // cout << "on board: " << on_board << endl;
    

    thread_signal(lock, order_posted_CV);
        
  }
  done_posting[cashier_id] = true;
  
  thread_signal(lock, order_ready_CV);
  //cout << "cashier unlock #: " << cashier_id << endl;
  thread_unlock(lock);
  return;
}

bool check_maker_wait() {
  int min_num = min(running_cashiers, max_board_size);
  // cout << "min_num: " << min_num << endl;
  // cout << "sandwich_board size: " << sandwich_board.size << endl;
  if (sandwich_board.size < min_num) {
    return true;
  } 
  return false;
}

void sandwichMaker(void* argv) {
  // start_preemptions(false, true, 123);

  thread_lock(lock);
  int last_made_sandwich = -1;
  
  //while running cashiers >0 or 
  // cout << "running cashiers: " << running_cashiers << endl;
  while( (running_cashiers > 0) || (sandwich_board.size > 0) ) {
    // cout << "running_cashiers: " << running_cashiers << endl;
    // thread_lock(lock);
    while(check_maker_wait() == true){
      //the board isn't full yet, wait for it to fill up
      //cout<<"sandwich wait" << endl;
      thread_wait(lock, order_posted_CV);
      // cout<<"wait maker after" << endl;
      //}
    }
    if (running_cashiers == 0) {
      thread_unlock(lock);
      return;
    }
    int most_similar_sandwhich = 1001;
    int difference = 1001;
    int loop = 0;
    int this_sandwich;
    int this_cashier;
    int index;
    while (loop < max_board_size) {
      if ((abs(last_made_sandwich-sandwich_board.sandwich_arr[loop])<difference) && (sandwich_board.sandwich_arr[loop] != -1) ) {
          difference = abs(last_made_sandwich-sandwich_board.sandwich_arr[loop]);
          most_similar_sandwhich = sandwich_board.sandwich_arr[loop];	
          this_sandwich = most_similar_sandwhich;
          this_cashier = sandwich_board.cashier_arr[loop];
	  index = loop;
      }
      loop++;
    }
    last_made_sandwich = most_similar_sandwhich;
    //cout<<"this_sandwich " << most_similar_sandwhich<< endl;
    //cout<<"this cashier " << this_cashier<<endl;
    cout << "READY: cashier "<< this_cashier << " sandwich " << most_similar_sandwhich << endl;
    //cout << "sandwich_board.sandwich_arr[this_cashier]: " << sandwich_board.sandwich_arr[this_cashier] << endl;
    //cout << "sandwich_board.cashier_arr[this_cashier]: " << sandwich_board.cashier_arr[this_cashier] << endl;
    sandwich_board.sandwich_arr[index] = -1;
    sandwich_board.cashier_arr[index] = -1;
    global_counter++;
    sandwich_board.size--;
    //print_board();

    if (done_posting[this_cashier] == true) {
        running_cashiers--;
    }

    thread_broadcast(lock, order_ready_CV);
    // thread_wait(lock, order_posted_CV);
    // cout<< "sandwich#: " << global_counter <<endl;
    // thread_unlock(lock);
    // cout << "board size: " << board_size << endl;
    // cout << "running cashiers: " << running_cashiers << endl;
  }
  //cout << "maker unlock\n";
  // cout << "global counter: " << global_counter << endl;
  free(sandwich_board.sandwich_arr);
  free(sandwich_board.cashier_arr);
  free(done_posting);
  thread_unlock(lock);
  return;
}






int main(int argc, char* argv[]){
  //start_preemptions(false, true, 999);
  max_cashiers = 5;
  max_board_size = 3;
  //cout << "max_board_size: " << max_board_size << endl;
  running_cashiers = max_cashiers;
  global_cashier_id = 0;
  sandwich_board.sandwich_arr = (int*) malloc(max_board_size*sizeof(int));
  sandwich_board.cashier_arr = (int*) malloc(max_board_size*sizeof(int));
  sandwich_board.size = 0;
  done_posting = (bool*) malloc(max_cashiers*sizeof(bool));
  //initialize sandwich numbers on sandwich board
  for (int i=0; i<max_board_size; i++) {
    sandwich_board.sandwich_arr[i] = -1;
  }
  
  //initialize cashierIDs on sandwich board
  for (int i=0; i<max_board_size; i++) {
    sandwich_board.cashier_arr[i] = -1;
  }
  for (int i=0; i<max_cashiers; i++) {
    done_posting[i] = 0;
  }
  thread_libinit( (thread_startfunc_t) init, argv);
}





