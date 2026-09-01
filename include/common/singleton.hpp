/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:38
	file base:	singleton
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _OMP_SINGLETON_H_
#define _OMP_SINGLETON_H_

#include <boost/utility.hpp>
//#include <boost/thread/once.hpp>
//	*** COPY FROM *** http://www.boostcookbook.com/Recipe:/1235044

// Warning: If T's constructor throws, instance() will return a null reference.

namespace faith 
{
	template<class T> class singleton : private boost::noncopyable
	{
	public:
		static T& getInstance()
		{
			init();
			//boost::call_once(init, flag);
			//static delete_helper<T> helper;
			//helper.ptr = t = new T();
			return *t;
		}
		static void init() // never throws
		{
			static bool b_init = false;
			if(b_init == false)
			{
				static delete_helper<T> helper;
				helper.ptr = t = new T();
			}
			b_init = true;
		}

	protected:
		~singleton() {}
		singleton() {}

	private:
		template<class M>
		struct delete_helper 
		{
			delete_helper()
			{
				ptr = NULL;
			}
			~delete_helper()
			{
				delete ptr;
			}
			M * ptr ;
		};
		static T * t;
		//static boost::once_flag flag;

	};

	template<class T> T * singleton<T>::t = NULL;
	//template<class T> boost::once_flag singleton<T>::flag = BOOST_ONCE_INIT;

}// end of namespace faith


#endif// #define  __OMP_SINGLETON_HEADER__


/*-------------------------------------

	How to use this class?

--------------------------------------*/

/*

	myclass.hpp

#ifndef MYCLASS_HPP
#define MYCLASS_HPP

#include "singleton.hpp"

using namespace Templates;

class MyClass : public singleton<MyClass>
{ 
	friend class singleton<MyClass>;
public:
	void doSomething() { xcout << "Something"; }
private:
	MyClass();
};

#endif

	main.cpp

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>

#include “myclass.hpp”

void test()
{ 
	MyClass::instance().doSomething(); 
}

int main(int argc, char* []argv) 
{    
	boost::thread thread1(&test);    
	boost::thread thread2(&test);    
	thread1.join();    
	thread2.join();    
	return 0; 
}

*/
