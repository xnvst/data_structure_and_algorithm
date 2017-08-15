/**
 * @file main.cpp
 * @author Brad Fewster (brad.fewster@exinda.com)
 * @date 2014-01-09
 *
 * Copyright 2014 Exinda Networks, All Rights Reserved
 *
 * The program entry point for take-home C++ interview exercise.
 */
#include <iostream>
#include "ThreadPool.h"

using namespace std;

//=============================test helper=============================================
#ifdef TEST
#define CNT	10

static void test1()
{
   for (int i=0; i<CNT; i++)
   {
		std::cout << "Test 1: " << i << std::endl;
		sleep(1);
		boost::this_thread::interruption_point();
   }
}

static void test2()
{
   for (int i=0; i<CNT; i++)
   {
		std::cout << "Test 2: " << i << std::endl;
		sleep(1);
		boost::this_thread::interruption_point();
   }
}

static void test3()
{
   for (int i=0; i<CNT; i++)
   {
		std::cout << "Test 3: " << i << std::endl;
		sleep(1);
		boost::this_thread::interruption_point();
   }
}
#endif
//=============================test helper=============================================

int main(int argc, char* argv[])
{
   int result = 0;
   // Print a header
   cout << "Exinda Networks ThreadPool applet stub" << endl;
   cout << "This code is for use for interview purposes only" << endl << endl;

#ifdef TEST   
   // check hardware concurrency
   std::cout << "hardware_concurrency: " << boost::thread::hardware_concurrency() << std::endl;
#endif   
   
   // Synchronous start and stop
#ifdef TEST   
   cout << "Synchronous start and stop" << endl;
#endif   
   {
      ThreadPool threadPool;
	  //ThreadPool threadPool(3);	// can distribute more threads for the instance

      cout << "Starting ThreadPool... ";
      if (threadPool.start())
      {
         cout << "OK";
#ifdef TEST		 
		 threadPool.doWork(test1);
		 threadPool.doWork(test2);
		 threadPool.doWork(test3);
		 sleep(1); // Wait for threads to start
#endif
      }
      else
      {
         cout << "ERROR";
      }
      cout << endl;
      cout << "Stopping ThreadPool... ";
      if (threadPool.stop())
      {
         cout << "OK";
      }
      else
      {
         cout << "ERROR";
      }
      cout << endl;
   }  
   
   // Synchronous start and asynchronous stop
#ifdef TEST   
   cout << "Synchronous start and asynchronous stop" << endl;
#endif   
   {	
      ThreadPool threadPool;
      cout << "Starting ThreadPool... ";
      if (threadPool.start())
      {
         cout << "OK";
#ifdef TEST		 
		 threadPool.doWork(test1);
		 threadPool.doWork(test2);
		 threadPool.doWork(test3);
		 sleep(1); // Wait for threads to start
#endif		 
      }
      else
      {
         cout << "ERROR";
      }
      cout << endl;
      cout << "Requesting ThreadPool stop... ";
      if (threadPool.requestStop())
      {
         cout << "OK";
      }
      else
      {
         cout << "ERROR";
      }
      cout << endl;
      cout << "Blocking for stop... ";
      if (threadPool.wait())
      {
         cout << "OK";
      }
      else
      {
         cout << "ERROR";
      }
      cout << endl;
   }   
   // Done
   cout << endl;
   return result;
}

