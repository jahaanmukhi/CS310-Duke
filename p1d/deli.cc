#include "thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>


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

//active cashiers
int running_cashiers;

//max board size
int max_board_size;

//cashier is done posting
bool* done_posting;

unsigned int lock = 88888888;
unsigned int order_ready_CV = 99999999;
unsigned int order_posted_CV = 10000000;

void sandwichMaker(void* args);
void cashiers(void* argv);


//create sandwich maker and cashier threads
void init(void* argv) {

  thread_create((thread_startfunc_t) sandwichMaker, (void*) NULL);
  char** input_files = (char**) argv;
  for (int i = 2; i<max_cashiers+2; i++) {

    //create cashier threads
    char* filename = input_files[i];
    thread_create((thread_startfunc_t) cashiers, (void*) input_files[i]);
    // cout << "created thread " << i-2 << endl;
  }
}

//print sandwich board
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

//check if cashier can post(return false) or needs to wait(return true)
bool check_cashier_wait (int cashier_id_num) {

  //if sandwich board is empty, cashier posts
  if (sandwich_board.size == 0) {
    return false;
  }

  //if cashier id is already on the board, cashier waits
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

  char* files_cashier = (char*) args;
  std::string input_file_name(files_cashier);
  string str = input_file_name.substr(input_file_name.length() - 1, input_file_name.length());
  int cashier_id;
  cashier_id = global_cashier_id;
  global_cashier_id++;

  //for this cashier: put all sandwich orders into a vector 
  vector<int> orders_vector;
  string read_line = "";
  ifstream file;
  file.open(input_file_name.c_str());

  if (file.is_open()) {
    while (getline(file, read_line)) {
      int n = atoi(read_line.c_str());
      orders_vector.push_back(n);
    }
  }
  file.close();

  //while cashier still has sandwiches to post
  while(orders_vector.size() != 0) {

    //wait if board is full or check method is true
    while((sandwich_board.size == max_board_size) || (check_cashier_wait(cashier_id) == true) ) {
      thread_wait(lock, order_ready_CV);
    }

    //remove order from vector
    int order = orders_vector.at(0);
    orders_vector.erase(orders_vector.begin());

    //find where to post on the sandwich board
    int loop = 0;
    while (loop < max_board_size) {
      if (sandwich_board.sandwich_arr[loop] == -1 ) {
      	sandwich_board.sandwich_arr[loop] = order;
      	sandwich_board.cashier_arr[loop] = cashier_id;
      	break;	
      }
      loop++;
    }

    //just posted, increment board size
    sandwich_board.size++;
    cout << "POSTED: cashier "<< cashier_id << " sandwich " << order<< endl;
    //print_board();

    //signal that order was posted
    thread_signal(lock, order_posted_CV);
    
  }
  
  //no more orders to post, this cashier is done posting
  done_posting[cashier_id] = true;
  
  thread_signal(lock, order_ready_CV);

  //cout << "cashier unlock #: " << cashier_id << endl;
  thread_unlock(lock);
  return;
}

//check if maker needs to wait for a sandwich to be posted before making one
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
  //start_preemptions(true,false, 300);
  
  thread_lock(lock);

  int last_made_sandwich = -1;
  
  while( (running_cashiers > 0) || (sandwich_board.size > 0) ) {

    while(check_maker_wait() == true){
      thread_wait(lock, order_posted_CV); 
    }

    //unlock if active cashiers is 0
    if(running_cashiers == 0) {
      thread_unlock(lock);
      return;
    }

    int most_similar_sandwhich = 1001;
    int difference = 1001;
    int loop = 0;
    int this_sandwich;
    int this_cashier;
    int index;

    //find the most similar sandwich to the last made sandwich
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

    //reset sandwich board spot to -1
    sandwich_board.sandwich_arr[index] = -1;
    sandwich_board.cashier_arr[index] = -1;

    //increment sandwiches made
    global_counter++;
    sandwich_board.size--;
    
    //print_board();

    //if the cashier was already done posting, its last sandwich was just made, decrement running cashiers
    if (done_posting[this_cashier] == true) {
        running_cashiers--;
    }

    thread_broadcast(lock, order_ready_CV);
  }
  //cout << "maker unlock\n";
  //cout << "global counter: " << global_counter << endl;
  //free(sandwich_board.sandwich_arr);
  //free(sandwich_board.cashier_arr);
  thread_unlock(lock);
  return;
}


int main(int argc, char* argv[]){
  //start_preemptions(false, true, 999);

  max_cashiers = argc-2;
  max_board_size = atoi(argv[1]);
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
