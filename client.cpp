#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "FIFOreqchannel.h"
using namespace std;

#include <iostream>
#include <string> 

void * patient_function(void *arg)
{
    /* What will the patient threads do? */
    
}

void *worker_function(void *arg)
{
    /*
		Functionality of the worker threads	
    */

    
}
int main(int argc, char *argv[])
{
    int n = 100;    //default number of requests per "patient"
    int p = 10;     // number of patients [1,15]
    int w = 100;    //default number of worker threads
    int b = 20; 	// default capacity of the request buffer, you should change this default
	int m = MAX_MESSAGE; 	// default capacity of the file buffer
    string f;
    srand(time_t(NULL));

    int c;
     while ((c = getopt (argc, argv, "n:p:w:b:f")) != -1)
    {
        //const auto opt = getopt(argc, argv, short_opts);
        switch (c)
        {
            case 'n':
                n = std::stoi(optarg);
                std::cout << "number of data items set to: " << n << endl;
                break;
            case 'p':
                p = std::stoi(optarg);
                std::cout << "number of patients set to: " << p << endl;
                break;

            case 'w':
                w = std::stoi(optarg);
                std::cout << "number of worker threads set to: " << w << endl;
                break;

            case 'b':
                b = std::stoi(optarg);
                std::cout << "bounded buffer size set to: " << b << endl;
                break;
        
            case 'f':
                f = (string)optarg;
                std::cout << "file name set to: " << f << endl;
                break;

            default:
                int n = 100;    //default number of requests per "patient"
                int p = 10;     // number of patients [1,15]
                int w = 100;    //default number of worker threads
                int b = 20; 	// default capacity of the request buffer, you should change this default
	            int m = MAX_MESSAGE; 	// default capacity of the file buffer
                break;
        }
    }

    int pid = fork();
    if (pid == 0){
		// modify this to pass along m
        execl ("dataserver", "dataserver", (char *)NULL);
        
    }
    
	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
	HistogramCollection hc;
	

    struct timeval start, end;
    gettimeofday (&start, 0);

    /* Start all threads here */
    //patient threads 
    for (int i = 0; i <= p; i++){
        
    }

    //worker threads
    for (int i = 0; i <= p; i++){
         
    }
	

	/* Join all threads here */

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
