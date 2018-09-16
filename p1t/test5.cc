#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <inttypes.h>
#include "thread.h"

using namespace std;

unsigned int arg_num = 161;
unsigned int num_consumers = 5;
vector<int> consumer_appetite;
unsigned int soda_capacity = 3;
unsigned int num_curr_sodas;
unsigned int num_curr_consumers;

unsigned int lock = 161;
unsigned int hasSoda = 160;
unsigned int hasRoom = 162;

void producer(void *arg);
void consumer(void *arg);
void initialize(void *arg);

//TEST: soda machine

int main(int argc, char *argv[]){
	thread_libinit((thread_startfunc_t) initialize, NULL);
}

void print_vector() {
	for (int i=1; i<consumer_appetite.size(); i++) {
		cout << "consumer " << i << " still wants " << consumer_appetite.at(i) << " sodas\n";
	}
}

void initialize(void *arg) {
	num_curr_consumers = num_consumers;
	consumer_appetite.resize(num_consumers+1);

	for (int i=1; i<num_consumers+1; i++) {
		consumer_appetite.at(i) = i;
		thread_create((thread_startfunc_t) consumer, (void*) (intptr_t) i);	
	}	
	thread_create((thread_startfunc_t) producer, (void*) (intptr_t) soda_capacity);	

}

void producer(void *arg) {
	// start_preemptions(false, true, 200);
	int n = (intptr_t) arg;
	// cout << num_curr_consumers << endl;
	while (num_curr_consumers > 0) {
		print_vector();
		cout << "producer request lock\n";
		thread_lock(lock);
		cout << "producer got lock\n";
		while (num_curr_sodas == soda_capacity) {
			cout << "producer sleep\n";
			thread_wait(lock, hasRoom);
			cout << "producer wake\n";
		}
		//completely fill machine
		num_curr_sodas = soda_capacity;
		thread_yield();
		cout << "producer broadcast machine has " << num_curr_sodas << " sodas\n";
		thread_broadcast(lock, hasSoda);
		thread_unlock(lock);
		cout << "producer unlock\n";
	}
}

//consumer i drinks i number of sodas
void consumer(void *arg) {
	int n = (intptr_t) arg;
	while (consumer_appetite.at(n) > 0){
		cout << "consumer " << n << " request lock\n";
		thread_lock(lock);
		cout << "consumer " << n << " got lock\n";
		while (num_curr_sodas == 0) {
			cout << "consumer " << n << " sleep\n";
			thread_wait(lock, hasSoda);
			cout << "consumer " << n << " wake\n";
		}
		cout << "consumer " << n << " drinks a soda\n";
		num_curr_sodas--;
		consumer_appetite.at(n)--;
		cout << "consumer " << n << " still wants " << consumer_appetite.at(n) << " sodas\n";
		thread_signal(lock, hasRoom);
		cout << "consumer " << n << " signals " << num_curr_sodas << " left in machine\n";
		if (consumer_appetite.at(n) == 0) {
			cout << "consumer " << n << " is done\n";
			num_curr_consumers--;
		}
		thread_unlock(lock);
		cout << "consumer " << n << " unlocks\n";
	}
}