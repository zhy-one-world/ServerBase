/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   22:03
	file base:	fast_allocator
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _FAST_ALLOCATOR_H_
#define _FAST_ALLOCATOR_H_

/*
	本文件中的分配器提供非线程安全的std窗口快速分配,正因为如果，所以才比boost::fast_allocator快一些。
	如果想用线程安全的分配器，请使用 boost::fast_allocator
*/
#include <vector>
namespace faith
{
	template <class T, unsigned int N = 1>
	class fast_allocator
	{
	public:
		typedef T                 value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t       size_type;
		typedef std::ptrdiff_t    difference_type;

		fast_allocator() {}
		fast_allocator(const fast_allocator&) {}
		~fast_allocator() {}
		template <class X>
		fast_allocator(const fast_allocator<X, N>&) {}
		template <class X>
		struct rebind { typedef fast_allocator<X, N> other; };

		pointer address(reference x) const
		{
			return &x;
		}

		const_pointer address(const_reference x) const
		{
			return &x;
		}

		pointer allocate(size_type n, const_pointer = 0)
		{
			if (n <= N)
			{
				if (m_unused.empty())
				{
					pointer ret = reinterpret_cast<pointer>(new char[sizeof(T) * N]);
					return ret;
				}
				else
				{
					pointer ret = reinterpret_cast<pointer>(m_unused.back());
					m_unused.pop_back();
					return ret;
				}
			}
			else
			{
				return reinterpret_cast<pointer>(new char[sizeof(T) * n]);
			}
		};
		void deallocate(pointer p, size_type n)
		{
			if (n <= N)
			{
				m_unused.push_back(p);
			}
			else
			{
				delete[] reinterpret_cast<char*>(p);
			}
		}
		size_type max_size() const
		{
			return static_cast<size_type>(-1) / sizeof(value_type);
		}
		void construct(pointer p, const value_type& x)
		{
			new(p) value_type(x);
		}
		void destroy(pointer p)
		{
			p->~value_type();
		}
	private:
		void operator=(const fast_allocator&);
		static std::vector< void* >		m_unused;
	};

	template <class T, unsigned int N>
	inline bool operator==(const fast_allocator<T, N>&, const fast_allocator<T, N>&)
	{
		return true;
	}
	template <class T, unsigned int N>
	inline bool operator!=(const fast_allocator<T, N>&, const fast_allocator<T, N>&)
	{
		return false;
	}
	template <class T, unsigned int N>
	std::vector< void* > fast_allocator<T, N>::m_unused;
}

#endif
