#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

class BoundedBuffer
{
private:
  	int cap;
	int size; 
  	queue<vector<char>> q;

	/* mutex to protect the queue from simultaneous producer accesses
	or simultaneous consumer accesses */
	pthread_mutex_t mtx;
	pthread_mutex_t mtx_push;
	pthread_mutex_t mtx_pop;
	
	/* condition that tells the consumers that some data is there */
	pthread_cond_t data_available;
	/* condition that tells the producers that there is some slot available */
	pthread_cond_t slot_available;

public:
	int size();

	BoundedBuffer(int _cap) {
		cap = _cap;
		pthread_cond_init(&data_available, NULL);
		pthread_cond_init(&slot_available, NULL);
	}

	~BoundedBuffer(){
		pthread_mutex_destroy(&mtx);
		pthread_cond_destroy(&data_available);
		pthread_cond_destroy(&slot_available);
	}

	void push(char* data, int size){
		// unique_lock<mutex> lck (mtx);
		// while (q.size() == cap +1){
		// 	slot_available.wait(lck);
		// }
		// q.push(data);
		// mtx.unlock();
		// data_available.notify_one();
		pthread_mutex_lock(&mtx_push);
		vector<char> realData(data, data+size);
		while(q.size() == cap){
			pthread_cond_wait(&slot_available, &mtx_push);
		}
		q.push(realData);
		pthread_cond_signal(&data_available);
		pthread_mutex_unlock(&mtx_push);
		return;
	}

	vector<char> pop(){
		// unique_lock <mutex> lck (mtx); 
		// // keep waiting as long as q.size() == 0
		// data_available.wait (lck, [this]{return q.size() > 0;});
		// // pop from the queue
		// vector<char> data = q.front();
		// q.pop();
		// // notify any potential producer(s)
		// slot_available.notify_one();
		// // unlock the mutex so that others can go in
		// lck.unlock();
		pthread_mutex_lock(&mtx_pop);
		vector<char> temp;
		while(q.size() == 0){
			pthread_cond_wait(&data_available, &mtx_pop);
		}
		temp = q.front();
		q.pop();
		pthread_cond_signal(&slot_available);
		pthread_mutex_unlock(&mtx_pop);
		return temp; 
	}
};

#endif /* BoundedBuffer_ */
