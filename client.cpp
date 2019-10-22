#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "FIFOreqchannel.h"
#include <iostream>
using namespace std;


void* patient_function(void *arg)
{
    /* What will the patient threads do? woowoowow */
    cout << "~~~~~~~~ DATA POINTS REQUESTED ~~~~~~~~" << endl;

    patient_args *patient_thread; 
    patient_thread = (patient_args*) arg;
    double time = 0;

    for(int i = 0; i < patient_thread->num; i++){
        cout << "PATIENT: " << patient_thread->patient << endl;
        // get the data message for the patient and time
        datamsg msg1(patient_thread->patient, time, 1);
        // put in data message request into the bounded buffer 
        patient_thread->buffer->push((char*)& msg1, sizeof(datamsg));
        time += 0.004;
    }

    // exit thread
    pthread_exit(NULL);
}

void *worker_function(void *arg)
{
    /* Functionality of the worker threads	*/
    cout << "~~~~~~~~ COLLECTING DATA POINTS ~~~~~~~~" << endl;
    worker_args* worker_thread;
    worker_thread = (worker_args*) arg;

    while(true){
        vector<char> tempVector = worker_thread->buffer->pop();
        char* tempData = tempVector.data();
        datamsg* data = (datamsg*)tempData;
        worker_thread->channel->cwrite(tempVector.data(), tempVector.size());
        MESSAGE_TYPE quit = QUIT_MSG;

        if (tempVector.size() == sizeof(MESSAGE_TYPE) && tempVector.at(0) == quit){
            cout << "worker quiting..." << endl;
            // delete the channel 
            delete worker_thread->channel;
            // break 'infinate' loop
            break;
        }
        else{
            // get the responses 
            char* response = worker_thread->channel->cread();
            double ecg_num = *((double*) response);
            int prsn = data->person;

            cout << "PERSON: " << prsn << endl;
            cout << "ECG: " << ecg_num << endl;
            
            // update historgram ... must put in critical section 
            pthread_mutex_lock(worker_thread->mtx_histrogram);
            worker_thread->histograms->update(data->person, ecg_num);
            pthread_mutex_unlock(worker_thread->mtx_histrogram);
        }
    }
    // exit thread
    pthread_exit(NULL);
}

// arguments for patient threads
class patient_args{
    public:
    BoundedBuffer* buffer;
    int num;
    int patient;
};

// arguments for worker threads
class worker_args{
    public:
    BoundedBuffer* buffer;
    FIFORequestChannel* channel;
    HistogramCollection* histograms;
    pthread_mutex_t* mtx_histrogram;
};


int main(int argc, char *argv[])
{
    int n = 100;    //default number of requests per "patient"
    int p = 10;     // number of patients [1,15]
    int w = 100;    //default number of worker threads
    int b = 20; 	// default capacity of the request buffer, you should change this default
	int m = MAX_MESSAGE; 	// default capacity of the file buffer
    srand(time_t(NULL));
    
    // command line arguments
    int opt = 0;
    while((opt = getopt(argc,argv, "n:p:w:b:")) != -1){
        switch(opt){
            case 'n': 
                n = atoi(optarg);
                break;
            case 'p':
                p = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
        }
    }

    int pid = fork();
    if (pid == 0){
		// modify this to pass along m
        execl ("dataserver", "dataserver", (char *)NULL);
        
    }
    // create FIFORequestChannel
    else {
        // cout << "~~~~~~~~ CREATING FIFOReqChannel ~~~~~~~~" << endl;
        FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
        BoundedBuffer request_buffer(b);
	    HistogramCollection hc;

        pthread_mutex_t mtx_histogram;
        pthread_mutex_init(&mtx_histogram, NULL);

        struct timeval start, end;
        gettimeofday (&start, 0);
        
        pthread_t patient_threads[p];
        pthread_t worker_threads[w];

        patient_args patient_thread_args[p];
        worker_args worker_threads_args[w];

        /* Start all threads here */

        // STARTING WITH PATIENT (producer) 
        for (int i = 0; i <= p; i++){
            patient_thread_args[i].num = n;
            patient_thread_args[i].patient = i+1;
            patient_thread_args[i].buffer = &request_buffer;

            hc.add(new Histogram(10,-2,2));
            pthread_create(&patient_threads[i], NULL, patient_function, (void*) &patient_thread_args[i]);
        }

        // MAKING WORKER THREADS
        for (int i = 0; i < w; i++){
            worker_threads_args[i].buffer = &request_buffer;
            worker_threads_args[i].histograms = &hc;
            worker_threads_args[i].mtx_histrogram = &mtx_histogram;

            MESSAGE_TYPE* new_chan_req = new MESSAGE_TYPE(NEWCHANNEL_MSG);
            chan->cwrite((char*) new_chan_req, sizeof(MESSAGE_TYPE));
            char* new_chan = chan->cread();

            worker_threads_args[i].channel = new FIFORequestChannel(new_chan, FIFORequestChannel::CLIENT_SIDE);
            pthread_create(&worker_threads[i], NULL, worker_function, (void*) &worker_threads_args[i]);
        }        
        /* Join all threads here */

        // Join patient threads
        for (int i = 0; i < p; i++){
            pthread_join(patient_threads[i], NULL);
        }

        // push quit messages 
        for (int i = 0; i < w; i++){
            MESSAGE_TYPE quit = QUIT_MSG;
            request_buffer.push((char*) &quit, sizeof(MESSAGE_TYPE));
        }

        // join worker threads
        for (int i = 0; i < w; i++){
            pthread_join(worker_threads[i], NULL);
        }

        gettimeofday (&end, 0);
        hc.print ();
        int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
        int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
        cout << "Took " << secs << " seconds and " << usecs << " micor seconds" << endl;

        MESSAGE_TYPE q = QUIT_MSG;
        chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
        cout << "All Done!!!" << endl;
        delete chan;
    }
}