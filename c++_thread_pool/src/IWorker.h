/**
 * @file IWorker.h
 * @author Brad Fewster (brad.fewster@exinda.com)
 * @date 2013-12-24
 *
 * Copyright 2013 Exinda Networks, All Rights Reserved
 * 
 * The worker is an abstract interface to in some way execute a piece of
 * work on behalf of the caller.  The work is simply a boost bound function
 * that can do whatever the caller needs done.
 *
 * This interface does not enforce the exact mechanism that is used to execute
 * the work, which is exactly why it is an interface.  You should use a concrete
 * implementation of the worker that satisfies the work execution discipline
 * that is needed by the caller (for example deferring to another thread for
 * execution).
 */
#pragma once

#include <boost/function.hpp>


class IWorker
{
public:
   /**
    * A work item that the worker can execute
    *
    * Bind this to a function that does some work to allow it to execute
    */
   typedef boost::function<void()> WorkItem;

protected:
   /**
    * Constructor
    */
   IWorker() {};
public:
   /**
    * Destructor
    */
   virtual ~IWorker() {};
private:
   IWorker(const IWorker&);
   IWorker& operator=(const IWorker&);

public:
   // --- Interface Methods
   /**
    * Starts the worker to accept new work
    *
    * @return true on success, false on failure
    * @note must be thread safe to be called with stop, started, and doWork
    * @note when called concurrently, must run as though it were being run
    *       without the concurrent call, and run safely
    * @note the order that concurrent calls must execute are not defined
    */
   virtual bool start(void)=0;
   /**
    * Stops the worker from accepting new work and stops executing any backlogged work
    *
    * @return true on success, false on failure
    * @note must be thread safe to be called with start, started, and doWork
    * @note when called concurrently, must run as though it were being run
    *       without the concurrent call, and run safely
    * @note the order that concurrent calls must execute are not defined
    * @note if work is currently executing, it must finish to completion before this call
    *       returns.
    * @note must block until the stop is complete
    * @note must support multiple concurrent calls to stop
    */
   virtual bool stop(void)=0;
   /**
    * Determines if the worker is started (can accept new work and/or is executing)
    *
    * @return true if started, false if not
    * @note must be thread safe to be called with start, stop, and doWork
    */
   virtual bool started(void) const=0;
   /**
    * Requests that the passed work be executed
    *
    * @param work the work to execute
    * @return true if the work was accepted for execution, false if not
    * @note must be thread safe to be called with start, stop, and started
    * @note true does not guarantee that the work will be executed, simply that
    *       is has been accepted for execution (i.e. may have been enqueued)
    * @note if the object is stop()'ped before the work is executed, it may not
    *       actually be executed
    */
   virtual bool doWork(const WorkItem& work)=0;
};


