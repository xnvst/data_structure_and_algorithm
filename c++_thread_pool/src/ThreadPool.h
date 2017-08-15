/**
 * @file ThreadPool.h
 * @author Brad Fewster (brad.fewster@exinda.com)
 * @date 2013-12-24
 *
 * Copyright 2013 Exinda Networks, All Rights Reserved
 * 
 * The ThreadPool is an implementation of IWorker which owns a set of threads and
 * accepts jobs to be enqueued to the thread pool.  Once a thread becomes available,
 * the next piece of work will be executed in a FIFO order.
 *
 * The actual thread that executes the work is not granateed and may change across
 * different copies of an identical job.
 */
#pragma once

#include <iostream>
#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <src/IWorker.h>

// uncomment to enable test helper and all the test debug printout
#define TEST	

class ThreadPool : public IWorker
{
public:
   /**
    * Constructor
    *
    * @note uses a default thread count
    */
   ThreadPool();
   /**
    * Constructor
    *
    * @note uses with configurable thread count
    */   
   ThreadPool(std::size_t num);
   /**
    * Destructor
    */
   virtual ~ThreadPool();
   
private:
   ThreadPool(const ThreadPool&);
   ThreadPool& operator=(const ThreadPool&);
   
   const std::size_t default_num_threads_ = 2;
   std::size_t total_threads_ = default_num_threads_;
   std::size_t available_threads_;
   boost::asio::io_service service_;
   std::unique_ptr<boost::asio::io_service::work> work_;
   boost::thread_group group_;
   std::vector<boost::thread *> threads_vector_;
   boost::mutex mutex_;
   
   void thread_proc(const WorkItem& work);

public:
   // --- IWorker Implementation
   // <<implement interface here>>
   bool start(void);
   bool stop(void);
   bool started(void) const;
   bool doWork(const WorkItem& work);

public:
   // --- ThreadPool Specific Methods
   /**
    * Waits (block the caller) until the ThreadPool is stopped
    *
    * @return true if a block occurred, false if not
    * @note if !started, can return immediately
    */
   bool wait(void);
   /**
    * Requests an asynchronous stop
    *
    * @return true if requested, false if not
    * @note will not block until stop completes
    * @note you must call wait() at some point to wait for the stop to complete
    */
   bool requestStop(void);
};

template< class T, class... ARGS >
std::unique_ptr<T> make_unique( ARGS&&... my_args )
{
   return std::unique_ptr<T>(new T(std::forward<ARGS>(my_args)...));
}
