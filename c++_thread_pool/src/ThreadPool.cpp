/*
 * @name    ThreadPool.cpp
 * @brief   Implementation file for ThreadPool class
 * @author  Feng Cao
 */

// LOCAL
#include "ThreadPool.h"

/**
 * @name        ThreadPool
 * @brief       Constructor.
 */
ThreadPool::ThreadPool() : available_threads_(default_num_threads_), work_(make_unique<boost::asio::io_service::work>(service_))
{
}

/**
 * @name        ThreadPool
 * @brief       Constructor with configurable thread count.
 */
ThreadPool::ThreadPool(std::size_t num) : total_threads_(num), available_threads_(num), work_(make_unique<boost::asio::io_service::work>(service_))
{
}

/**
 * @name        SMManager
 * @brief       Destructor.
 */
ThreadPool::~ThreadPool()
{
	try 
	{
		service_.stop();
		while (!threads_vector_.empty())
		{
			if ((threads_vector_.back())->joinable())
			{
				(threads_vector_.back())->join();
			}
			boost::thread *tmp_thread = threads_vector_.back();
			threads_vector_.pop_back();
			group_.remove_thread(tmp_thread);
			delete tmp_thread;			
		}
	}
	catch(const std::exception& e)
	{
		std::cout << "ThreadPool::~ThreadPool - Caught exception: " << e.what() << std::endl;
	}
}

/**
 * @name        	start
 * @brief       	IWorker Implementation -- start
 * @param[in]   	void
 * @param[out]   	bool 
 */
bool ThreadPool::start(void)
{
#ifdef TEST
	std::cout << "<<" << __FUNCTION__ << ">>" << std::endl;
#endif
	try
	{
		mutex_.lock();		
#ifdef TEST		
		std::cout << "group_.size " << group_.size() << ", threads_vector_.size " << threads_vector_.size() 
			<< ",total_threads_ " << total_threads_ <<", available_threads_ " << available_threads_ << std::endl;
#endif			
		for (std::size_t i = group_.size(); i<total_threads_; i++)
		{
			boost::thread *tmp_thread = new boost::thread(boost::bind(&boost::asio::io_service::run, &service_));
			threads_vector_.push_back(tmp_thread); 
			group_.add_thread(tmp_thread);
		}
		mutex_.unlock();
	}
	catch (const std::exception& e) 
	{
		std::cout << "ThreadPool::start - Caught exception: " << e.what() << std::endl;
		mutex_.unlock();
		return false;
	}
#ifdef TEST	
	std::cout << "group_.size " << group_.size() << ", threads_vector_.size " << threads_vector_.size() 
		<< ",total_threads_ " << total_threads_ <<", available_threads_ " << available_threads_ << std::endl;
#endif		
	return true;
}

/**
 * @name        	stop
 * @brief       	IWorker Implementation -- stop
 * @param[in]   	void
 * @param[out]   	bool 
 */
bool ThreadPool::stop(void)
{
#ifdef TEST
	std::cout << "<<" << __FUNCTION__ << ">>" << std::endl;
#endif
	try
	{		
		mutex_.lock();
		work_.reset();
		service_.stop();	
		
		/* interrupt_all will end the thread function sooner but need assign interrupt point in thread function, 
		** otherwise wait until the task done using join_all, but should end within the scope of stop function 
		*/
		group_.interrupt_all();
		//group_.join_all();
		
		while (!threads_vector_.empty())
		{
			if ((threads_vector_.back())->joinable())
			{
				(threads_vector_.back())->join();
#ifdef TEST				
				std::cout << "joinable" << std::endl;
#endif				
			}			
			boost::thread *tmp_thread = threads_vector_.back();
			threads_vector_.pop_back();			
			group_.remove_thread(tmp_thread);
#ifdef TEST			
			std::cout << "deleted" << std::endl;
#endif			
			delete tmp_thread;
			available_threads_++;
		}	
		mutex_.unlock();
	}
	catch (const std::exception& e) 
	{
		std::cout << "ThreadPool::stop - Caught exception: " << e.what() << std::endl;
		mutex_.unlock();
		return false;
	}
#ifdef TEST	
	std::cout << "group_.size " << group_.size() << ", threads_vector_.size " << threads_vector_.size() 
		<< ",total_threads_ " << total_threads_ <<", available_threads_ " << available_threads_ << std::endl;
#endif		
	return true;
}

/**
 * @name        	started
 * @brief       	IWorker Implementation -- started
 * @param[in]   	void
 * @param[out]   	bool const 
 */
bool ThreadPool::started(void) const
{
#ifdef TEST
	std::cout << "<<" << __FUNCTION__ << ">>" << std::endl;
#endif
	bool ret;
	(available_threads_ == total_threads_) ? ret = false : ret = true;
	return ret;
}

/**
 * @name        	doWork
 * @brief       	IWorker Implementation -- doWork
 * @param[in]   	const WorkItem& work
 * @param[out]   	bool 
 */
bool ThreadPool::doWork(const WorkItem& work)
{
#ifdef TEST
	std::cout << "<<" << __FUNCTION__ << ">>" << std::endl;
#endif
	try
	{
		mutex_.lock();	
		if (available_threads_ == 0)
		{ 
			mutex_.unlock();
			return false;
		}
		available_threads_--;
		service_.post(boost::bind(&ThreadPool::thread_proc, this, work));	
		mutex_.unlock();
	}
	catch (const std::exception& e) 
	{
		std::cout << "ThreadPool::doWork - Caught exception: " << e.what() << std::endl;
		mutex_.unlock();
		return false;
	}
#ifdef TEST	
	std::cout << "group_.size " << group_.size() << ", threads_vector_.size " << threads_vector_.size() 
		<< ",total_threads_ " << total_threads_ <<", available_threads_ " << available_threads_ << std::endl;
#endif		
	return true;
}

/**
 * @name        	thread_proc
 * @brief       	Wrap a work so that the thread count can be tracked.
 * @param[in]   	const WorkItem& work
 * @param[out]   	bool 
 */
void ThreadPool::thread_proc(const WorkItem& work)
{
	while (!service_.stopped())
	{
		try {
			work();
			//boost::this_thread::interruption_point();
		}
		catch(const std::exception& e)
		{
			std::cout << "ThreadPool::thread_proc - Caught exception: " << e.what() << std::endl;
		}
	}

	mutex_.lock();
	available_threads_++;
	mutex_.unlock();
#ifdef TEST
	std::cout << "<<" << __FUNCTION__ << ">>" << std::endl;	
	std::cout << "group_.size " << group_.size() << ", threads_vector_.size " << threads_vector_.size() 
		<< ",total_threads_ " << total_threads_ <<", available_threads_ " << available_threads_ << std::endl;
#endif		
}

/**
 * @name        	wait
 * @brief       	Waits (block the caller) until the ThreadPool is stopped
 * @param[in]   	void
 * @param[out]   	bool 
 */
bool ThreadPool::wait(void)
{
#ifdef TEST
	std::cout << "<<" << __FUNCTION__ << ">>" << std::endl;
#endif
	bool ret = false;
	try 
	{	
		while (!threads_vector_.empty())
		{
			if ((threads_vector_.back())->joinable())
			{
				(threads_vector_.back())->join();
#ifdef TEST				
				std::cout << "joinable" << std::endl;
#endif				
				ret = true;
			}			
			boost::thread *tmp_thread = threads_vector_.back();
			threads_vector_.pop_back();			
			group_.remove_thread(tmp_thread);
#ifdef TEST			
			std::cout << "deleted" << std::endl;
#endif			
			delete tmp_thread;			
		}
	}
	catch(const std::exception& e)
	{
		std::cout << "ThreadPool::wait - Caught exception: " << e.what() << std::endl;
		return ret;
	}
#ifdef TEST	
	std::cout << "group_.size " << group_.size() << ", threads_vector_.size " << threads_vector_.size() 
		<< ",total_threads_ " << total_threads_ <<", available_threads_ " << available_threads_ << std::endl;
#endif		
	return ret;
}

/**
 * @name        	requestStop
 * @brief       	Requests an asynchronous stop
 * @param[in]   	void
 * @param[out]   	bool 
 */
bool ThreadPool::requestStop(void)
{
#ifdef TEST
	std::cout << "<<" << __FUNCTION__ << ">>" << std::endl;
#endif
	try 
	{
		mutex_.lock();
		work_.reset();
		service_.stop();
		mutex_.unlock();
	}
	catch(const std::exception& e)
	{
		std::cout << "ThreadPool::requestStop - Caught exception: " << e.what() << std::endl;
		mutex_.unlock();
		return false;
	}
#ifdef TEST	
	std::cout << "group_.size " << group_.size() << ", threads_vector_.size " << threads_vector_.size() 
		<< ",total_threads_ " << total_threads_ <<", available_threads_ " << available_threads_ << std::endl;
#endif		
	return true;	
}