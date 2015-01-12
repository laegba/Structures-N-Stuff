/*

  Program to test spawning additional processes and assigning jobs to them.
  
    M. Salay 20150111

  
   It involves tests of successively more complex job assignment methods.
  

   Eventually a derivative of this program can eventually serve as a manager
   for external calculations for a KSP mod. 


   Establish and test the following capabilities
 
     1) Transfer job to a separate thread operating on same memory.  Done!
     2) Query system for available processors and cores.  Done!
     3) Query system for core loading. Done!
          XXX Can do in Linux by reading /proc/stat - is there a generalized way
              to find this out
     4) Develop workers and managers.  In progress...

Types of jobs:
  1) Multiple structural calculations for sets of non-connected/colliding ships
  2) Rasterization of vector graphics for texture mapping.
  3) Others?
 


Condition variables used so that 
Workers should wait until jobs are available.




   The first job that is being transferred is convert first element of vector to 8.

   main.cpp acts as the calling program

Thread creation is somewhat time consuming so it seems that it would be preferable to 
start threads (one per processor other than the one running the main program)


   Found a few different types of threads.  Comments found on them: 
     - thread             - higher level of abstraction (doesn't work on all platforms)
	     -   operates on top of pthread
	     - provides portability across different platforms like Windows, MacOS, and Linux.
     - pthread (Posix threads) - more reliable for many platforms
     - boost::thread           - introduces dependency from third party library (can't just install)
     - async           - introduces dependency from third party library
     
     


 */

#include <iostream>
#include <vector>
#include <thread>
#include <time.h> // XXX LINUX SPECIFIC - needs -lrt as compile option (for clock_gettime)
// Don't have boost library 
#include <mutex>  // to block threads from performing same operation.
#include <condition_variable>

  
using namespace std;

/* functions */
void foo1( vector<int>& blort) { blort[0]=8; }    // As a reference
void foo2( vector<int>* blort) { (*blort)[0]=8; } // As a pointer

void taska(string msg)
{
    cout << "taska says: " << msg;
}



// check multiple simultaneous calls of same routine
static const int num_threads = 10;
void call_from_thread(int tid) {
    cout << "Launched by thread " << tid << endl;
}

// try to modify variable with thread
void foo( vector<int>& blort) { 


  for (int i=1; i <= 10; i++){
     blort[0]=i;
//     sleep (0.1);  // sleep doesn't work right in thread - simulates work anyways
  }
  return;
 } 


/***************************************************************/
// Interleave tests
struct Counter {
    int value;

    Counter() : value(0){}

    void increment(){
        ++value;
    }
};

struct Counterm { //mutex to block operation
    mutex mutx;
    int value;

    Counterm() : value(0){}

    void increment(){
        mutx.lock(); // blocks two threads from performing this operation at same time
        ++value;
        mutx.unlock();         
    }
};




// worker tests
// should eventually use array for job info, vector for data

/*
 ***************************************************************
 **************************  TASK  *****************************
 ***************************************************************
 */

 
/* task for worker  */
// task1 is to add 1 to number 10,000 times.
void task1( vector<int>& job )
{
  for (int i=1; i <= 10; i++) job[2]=job[2]+1;  // update data
}

 

/*
 ****************************************************************
 ************************  WORKER1  *****************************
 ****************************************************************
 */



/* 

  First worker test

  Worker has 3 states:
     1) wait for job
     2) work on job
     3) exit
     
  Size 3 vector expected:   ( job done or not, thread exit, data)   
  
  Should use  - http://en.cppreference.com/w/cpp/thread/condition_variable
    to have thread wait for job  
 */


void workr1( vector<int>& job ) { 
  while (job[1] == 0){ // thread exit condition
    if (job[0] == 1) { // wait until job available
      sleep(0);
    } else {
      task1(job);       // perform job
      job[0]=1;         // indicate that job is complete

    }
  }
  cout << "Worker Exiting..." << endl; // perhaps should have a thread identifier
}

/*
 ****************************************************************
 ************************  WORKER2  *****************************
 ****************************************************************
 */
/*   Second worker test...  Listen instead of sleep.            */


mutex m;          // XXX it seems that mutexes must be known to both so declared outside.
                 /* should check to see if they can be transferred as a variable */
 
condition_variable cv;
string data;
bool ready = false;
bool processed = false;
// The boolean ready and processed serve the same function as job[0].
//   job[0]=1 means job processed
//   job[0]=0 means data ready
//   It does seem like two signals are needed, one for data already read 
//     and one for new job.


void workr2( vector<int>& job ) {
  cout << "In Worker thread\n";
  while (job[1] == 0){ // thread exit condition ("all work is done")


    unique_lock<mutex> lk(m);  // set up lock
    cv.wait(lk, []{return ready;});      // wait until main sends data
  
      
    if (job[0] == 0) {
      cout << "Worker thread is processing data\n";
      task1(job);       // perform task
      job[0]=1;         // indicate that job is complete
      processed = true; // performs same function as job[0]=1; (going through cppreference example)
      ready = false; // performs same function as job[0]=1; (going through cppreference example)
    }
  
//    if (job[0] == 1) { // wait until job available ("job is done")
//      sleep(.1);
//    } else {
//      task1(job);
//    }
    lk.unlock();
    cv.notify_one();
  }
  cout << "Worker Exiting..." << endl; // perhaps should have a thread identifier
}
/*
 ****************************************************************
 ************************  WORKER2/  *****************************
 ****************************************************************
 */
/*   Same as second worker test...  No text.            */


void workr2p( vector<int>& job ) {
  while (job[1] == 0){ // thread exit condition ("all work is done")


    unique_lock<mutex> lk(m);  // set up lock
    cv.wait(lk, []{return ready;});      // wait until main sends data
  
      
    if (job[0] == 0) {
      task1(job);       // perform task
      job[0]=1;         // indicate that job is complete
      processed = true; // performs same function as job[0]=1; (going through cppreference example)
      ready = false; // performs same function as job[0]=1; (going through cppreference example)
    }
  
//    if (job[0] == 1) { // wait until job available ("job is done")
//      sleep(.1);
//    } else {
//      task1(job);
//    }
    lk.unlock();
    cv.notify_one();
  }
  cout << "Worker Exiting..." << endl; // perhaps should have a thread identifier
}


/*
 ****************************************************************
 *************************  MAIN  *******************************
 ****************************************************************
 */


/*     Main     */
int main(int argc, const char * argv[])
{

  // Leave this in...
  cout << "Hello World!\n";
  cout << '\n'; 
  cout << "time: " << time(NULL) << endl; 

  cout << "Test external vector modification:\n";
  // create a vector and print
  vector <int> vec {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  // need to use -std=c++0x compiler options for this format

  for( int i = 0; i < 9; i +=1 ) cout << vec[i] << ' '; cout << endl; 

  // create a vector and print
  foo1(vec) ; cout << vec[0]; vec[0]=1; cout << " " << vec[0] << endl;  // as reference
  foo2(&vec); cout << vec[0]; vec[0]=1; cout << " " << vec[0] << endl;  // as pointer
  cout << endl; 


  cout << "Test start a new thread:\n";
  thread t1(taska, "Hello");  // create new thread
  t1.join();   // wait for thread to be complete before continuing
  cout << endl; 


  cout << endl; 
  cout << "Check number of cores:\n";
  // XXX Should work for different platforms but not implemented on older versions of gcc.
  cout << thread::hardware_concurrency() << endl; // returns 0
  // XXX This is prefereable because it should be cross-platform
  // XXX Returns 0 on gcc 4.6.3 - seems to not be implemented - should upgrade.
  //  cout << boost::thread::hardware_concurrency() << endl; //XXX don't have boost
  // Linux, Solaris, & AIX and Mac OS X (for all OS releases >= 10.4, i.e., Tiger onwards) - per comments:
  int numCPU = sysconf( _SC_NPROCESSORS_ONLN ); // XXX should upgrade and use concurrency
  cout << numCPU << endl;  
  // note: the /proc/stat file provides core info and loading.


  cout << endl; 
  cout << "Start multiple threads of same code simultaneously:\n";
       thread t[num_threads];

        //Launch a group of threads
        for (int i = 0; i < num_threads; ++i) {
            t[i] = thread(call_from_thread, i);
        }

        cout << "Launched from the main\n";

//        sleep (0.5);

        //Join the threads with the main thread
        for (int i = 0; i < num_threads; ++i) {
            t[i].join();
        }

//  cout << endl; 
//  cout << "Other system info:\n";
//  cout << "clock: " << clock() << endl; 
//  cout << "time: " << time(NULL) << endl; 


  cout << endl; 
  cout << "Check to see if main can read data changed from a thread:\n";
  t1 = thread(foo, ref(vec)); // must explicitly state ref for threads
//     sleep (.1);
  for (int i=1; i <= 10; i++){
     cout << " " << vec[0] << endl;
//     sleep (.1);
  }
  //get error upon close if don't join


  t1.join(); // join to make sure complete


  // job: info,data
  // jobs: list of jobs

  /********************************
   * Initial manager worker tests *
   ********************************/
  // define job vector {job complete,thread exit, data}
  // job manager writes posiiton 0, worker writes position 1
  cout << endl; 
  
/***************************************************************/
// Interleave tests
// from tutorial
  
  
      Counter counter;

    vector<thread> threads;
    for(int i = 0; i < 5; ++i){
      cout << "i: "<<i<<endl;
        threads.push_back(thread([&counter](){
            for(int i = 0; i < 100; ++i){
                counter.increment();
            }
        }));
    }

    for(auto& thread : threads){
        thread.join();
    }

    cout << counter.value << endl;
   cout << "Doesn't always add up to 500!"<< endl; 
   cout << "But all processes are operating on same variables!"<< endl; 
   cout << endl; 

    threads.clear();

    Counterm counterm; //mutexed counter
    for(int i = 0; i < 5; ++i){
      cout << "i: "<<i<<endl;
        threads.push_back(thread([&counterm](){
            for(int i = 0; i < 100; ++i){
                counterm.increment();
            }
        }));
    }

    for(auto& thread : threads){
        thread.join();
    }

    cout << counterm.value << endl;
   cout << "Always add up to 500!"<< endl; 
   cout << " This can be seen by using much larger number of threads"<< endl; 
   cout << " It seems that the issue could also have been avoided "<< endl; 
   cout << "   by not having all threads operate on same variable/instance."<< endl; 

  

  cout << endl; 
/***************************************************************
 ***************************************************************/
  cout << "Test simple worker, workr1:\n";
  cout << "  Using sleep(0.1) to wait.\n";
  cout << endl;
/***************************************************************
 ***************************************************************/
  vector <int> job {1, 0, 0};         // start thread with no jobs
  t1 = thread(workr1, ref(job)); // must explicitly state ref for threads

  cout << "data: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;
  cout << "wait: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;

  cout << endl;
    cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  job[2]  << endl;
    auto start = chrono::high_resolution_clock::now();
      job[0]=0;  // assign job
      while (job[0] == 0) sleep(0);  // simulate other tasks
    auto elapsed = chrono::high_resolution_clock::now() - start;
    long long us = chrono::duration_cast<chrono::microseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"us" << endl;

  cout << endl;
  cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  "-5000"  << endl;
    start = chrono::high_resolution_clock::now();
      job[0]=0;  // assign job
      while (job[0] == 0) sleep(0);  // simulate other tasks
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::microseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"us" << endl;

  cout << endl;
  cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  "-5000"  << endl;
    start = chrono::high_resolution_clock::now();
      job[2]=-5000;job[0]=0;  // assign another job
      while (job[0] == 0) sleep(0);  // simulate other tasks
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::microseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"us" << endl;


  cout << endl;
  job[1]=1;  // tell thread to finish
  t1.join(); // join to make sure complete
  cout << "done: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;
  
  
  cout << "Way too slow thread response (previously - now OK!)!!! \n";
  cout << "  Goes through loop updating task but does not indicate task complete \n";
  cout << "  even though no pause between commands exists in workr1!!!  \n";
  cout << "  Saying 1 or 0 us to transmit to thread and back for 10 additions task. \n";

/*
 ****************************************************************
 ************************  WORKER2  *****************************
 ****************************************************************
 */
  /*  Use cppreference mutex and condition variable example */
  /*  Switch to nanoseconds */


  cout << endl;
  cout << endl;
  cout << "Test simple worker, workr2:\n";
  cout << endl;
  
  timespec ts1,ts2;      // this time can calculate ns

  job={1, 0, 0};         // start thread with no jobs
  t1 = thread(workr2, ref(job)); // must explicitly state ref for threads
  data = "Example data";
  t1.detach();

  cout << "data: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;
  cout << "wait: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;

  cout << endl;
  cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  job[2]  << endl;
    start = chrono::high_resolution_clock::now();
    clock_gettime(CLOCK_REALTIME, &ts1);


    // assign job
    job[0]=0;  
    {
    lock_guard<mutex> lk(m);
    ready = true;
    cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();


    // wait for the worker
    {
        unique_lock<mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }
    cout << "Back in main(), data = " << data << '\n';
 

    clock_gettime(CLOCK_REALTIME, &ts2);
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"ns"<<" ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;



  cout << endl;
  cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  job[2]  << endl;
    start = chrono::high_resolution_clock::now();
    clock_gettime(CLOCK_REALTIME, &ts1);

    // assign job
    job[0]=0;  
    {
    lock_guard<mutex> lk(m);
    ready = true;
    processed = false;
    cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();
    // wait for the worker
    {
        unique_lock<mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }
    cout << "Back in main(), data = " << data << '\n';

    clock_gettime(CLOCK_REALTIME, &ts2);
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"ns"<<" ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;


  cout << endl;
    cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  "-5000"  << endl;
    clock_gettime(CLOCK_REALTIME, &ts1);

      while (job[0] == 0) sleep(0.1);
    // assign job
      job[2]=-5000;  job[0]=0;  // set new data and assign another job
    {
    lock_guard<mutex> lk(m);
    ready = true;
    processed = false;
    cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();
    // wait for the worker
    {
        unique_lock<mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }
    cout << "Back in main(), data = " << data << '\n';

    clock_gettime(CLOCK_REALTIME, &ts2);
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"ns"<<" ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;

  cout << endl;
  job[1]=1;  // tell thread to finish
  cout << "done: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;
  ready = true;
  cv.notify_one();  // additional unnecessary notification

  cout << endl;
  cout << "Ha! It takes longer for the muxing and condition than the simple sleep "<< endl;
  cout << "  It seems that the threads didn't sleep correctly anyways. They sometimes "<< endl;
  cout << "  slept a shorter time.  In any case the switching with sleep probably "<< endl;
  cout << "  consumes processing power on the management side which should be reserved for computation. "<< endl;
  cout << endl;


/*
 ****************************************************************
 ************************  WORKER1  *****************************
 ************************   AGAIN   *****************************
 ****************************************************************
 */

  cout << endl;
  cout << "Testing workr1 again with sleep 0 and ns "<< endl;

  timespec tsa,tsb;      // this time can calculate ns



  clock_gettime(CLOCK_REALTIME, &tsa);

  job={1, 0, 0};         // start thread with no jobs
  t1 = thread(workr1, ref(job)); // must explicitly state ref for threads

  cout << "data: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;
  cout << "wait: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;

  cout << endl;
    cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  job[2]  << endl;
    start = chrono::high_resolution_clock::now();
    
    clock_gettime(CLOCK_REALTIME, &ts1);
      job[0]=0;  // assign job
      while (job[0] == 0) sleep(0);  // simulate other tasks
    clock_gettime(CLOCK_REALTIME, &ts2);
    
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::microseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"us"<<" ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;


  cout << endl;
  cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  "-5000"  << endl;
    start = chrono::high_resolution_clock::now();

    clock_gettime(CLOCK_REALTIME, &ts1);
      job[0]=0;  // assign job
      while (job[0] == 0) sleep(0);  // simulate other tasks
    clock_gettime(CLOCK_REALTIME, &ts2);

    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::microseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"us"<<" ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;



  cout << endl;
  cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  "-5000"  << endl;
    start = chrono::high_resolution_clock::now();

    clock_gettime(CLOCK_REALTIME, &ts1);
      job[2]=-5000;job[0]=0;  // assign another job
      while (job[0] == 0) sleep(0);  // simulate other tasks
    clock_gettime(CLOCK_REALTIME, &ts2);

    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"ns" << " ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;


  cout << endl;
  job[1]=1;  // tell thread to finish
  t1.join(); // join to make sure complete
  cout << "done: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;

  clock_gettime(CLOCK_REALTIME, &tsb);



  cout << "sleep(0) seems nearly 100 times faster than mutex! ~230ns! call";
  cout << " Check if whole time is affected and check for extra verbosity";
  cout << "   in workr2 slows it down.\n";
//  cout << "   Total time for workr1 is: " << tsb.tv_nsec-tsa.tv_nsec <<"ns"<< endl;
  cout << "   Total time for workr1 from start to before end text is: "<< tsb.tv_nsec-tsa.tv_nsec <<"ns"<< endl;;

  cout << endl ;




/*
 ****************************************************************
 ************************  WORKER2p *****************************
 ************************   AGAIN   *****************************
 ****************************************************************
 */

  cout << endl;
  cout << endl;
  cout << "Test simple worker, workr2p:\n";
  cout << endl;
  
  ready = false;
  processed = false;
  
  

  clock_gettime(CLOCK_REALTIME, &tsa);
  job={1, 0, 0};         // start thread with no jobs
  t1 = thread(workr2p, ref(job)); // must explicitly state ref for threads
  data = "Example data";
  t1.detach();

  cout << "data: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;
  cout << "wait: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;

  cout << endl;
  cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  job[2]  << endl;
    start = chrono::high_resolution_clock::now();
    clock_gettime(CLOCK_REALTIME, &ts1);


    // assign job
    job[0]=0;  
    {
    lock_guard<mutex> lk(m);
    ready = true;
//    cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();


    // wait for the worker
    {
        unique_lock<mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }
//    cout << "Back in main(), data = " << data << '\n';
 

    clock_gettime(CLOCK_REALTIME, &ts2);
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"ns"<<" ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;



  cout << endl;
  cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  job[2]  << endl;
    start = chrono::high_resolution_clock::now();
    clock_gettime(CLOCK_REALTIME, &ts1);

    // assign job
    job[0]=0;  
    {
    lock_guard<mutex> lk(m);
    ready = true;
    processed = false;
//    cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();
    // wait for the worker
    {
        unique_lock<mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }
//    cout << "Back in main(), data = " << data << '\n';

    clock_gettime(CLOCK_REALTIME, &ts2);
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"ns"<<" ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;


  cout << endl;
    cout << "asin: " <<  "0" <<" "<<  job[1] <<" "<<  "-5000"  << endl;
    clock_gettime(CLOCK_REALTIME, &ts1);

      while (job[0] == 0) sleep(0.1);
    // assign job
      job[2]=-5000;  job[0]=0;  // set new data and assign another job
    {
    lock_guard<mutex> lk(m);
    ready = true;
    processed = false;
//    cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();
    // wait for the worker
    {
        unique_lock<mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }
//    cout << "Back in main(), data = " << data << '\n';

    clock_gettime(CLOCK_REALTIME, &ts2);
    elapsed = chrono::high_resolution_clock::now() - start;
    us = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
  cout <<"post: "<<job[0]<<" "<<job[1]<<" "<<job[2]<<" "<<us<<"ns"<<" ";
  cout << ts2.tv_nsec-ts1.tv_nsec <<"ns"<< endl;

  cout << endl;
  job[1]=1;  // tell thread to finish
  cout << "done: " <<  job[0] <<" "<<  job[1] <<" "<<  job[2]  << endl;
  ready = true;
  cv.notify_one();  // additional unnecessary notification

    clock_gettime(CLOCK_REALTIME, &tsb);
  cout << "   Total time for workr2 from start to before end text is: "<< tsb.tv_nsec-tsa.tv_nsec <<"ns"<< endl;;
  cout << endl;
  cout << " Total job for both ~ 70us. Workr1 (sleep) can go down to (38us) "<< endl; 
  cout << "   whereas workr2 (mutex) always about the same (down to 60us)."<< endl; 
  cout << "   Workr1 actually hit 130us a few times but infrequently."<< endl; 
  cout << "   Keeping in mind that this may be different under load"<< endl; 
  cout << "     so will continue with both approaches."<< endl; 
  cout << endl;
  cout << "     For 60 FPS each frame will be 1.67E-2s."<< endl; 
  cout << "     For 60 FPS each frame will be 3.33E-2s."<< endl; 
  cout << "     This consumes 4 thousandths of the time of a 60FPS frame"<< endl; 
  cout << "     leaving 99.6% of the time for the calculations themselves."<< endl; 
  
  

/*
 ****************************************************************
 ************************   MANAGER  *****************************
 ****************************************************************
 */
  cout << endl;
  cout << "  Doing managers with both sleep(0) and mutex"<< endl;
  cout << "  Rearranging variables to match the structure calculations "<< endl;
  cout << "    A) Job control variabes  "<< endl;
  cout << "    B) State class containing  "<< endl;
  cout << "      1) position vector           "<< endl;
  cout << "      2) velocity vector           "<< endl;
  cout << "      3) inertia                   "<< endl; 
  cout << "      4) failure criteria          "<< endl; 



  sleep(1);

}

