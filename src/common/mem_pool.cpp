/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:17
	file base:	mem_pool
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include <map>
#include <boost/detail/lightweight_mutex.hpp>
#include <vector>
#include <new>
#include <string.h>
#include "mem_pool.hpp"

namespace faith
{
		namespace
		{
			class pool_type
			{
			public:
				pool_type() :
					m_total_count(1),
					m_current_count(0)
				{
					m_data = new void * [m_total_count];
				}

				~pool_type()
				{
					delete [] m_data;
				}

			public:
				bool			empty() const
				{
					return m_current_count == 0;
				}

				void*			back()
				{
					return m_data[m_current_count-1];
				}

				void			push_back(void * ptr)
				{
					if(m_current_count == m_total_count)
					{
						unsigned int new_total_count = 2*m_total_count;
						void * * new_data = new void *[new_total_count];
						memcpy(new_data,m_data,sizeof(void *)*m_total_count);
						delete [] m_data;
						m_data = new_data;
						m_total_count = new_total_count;
					}
					m_data[m_current_count] = ptr;
					++ m_current_count;
				}

				void			pop_back()
				{
					-- m_current_count;
				}
			private:
				void**			m_data;				//指针数组
				unsigned int	m_total_count;		//总数
				unsigned int	m_current_count;	//当前计数
			};

			template<unsigned int size>
			class pool
			{
			public:
				static void*							malloc()
				{
					{
						boost::detail::lightweight_mutex::scoped_lock lock(m_mutex);
						if(!m_pool.empty())
						{
							void * result = m_pool.back();
							m_pool.pop_back();
							return result;
						}
					}					
					return new(std::nothrow) char [size];
				}
				static void								free(void * data)
				{
					boost::detail::lightweight_mutex::scoped_lock lock(m_mutex);
					m_pool.push_back(data);
				}
			private:
				static boost::detail::lightweight_mutex	m_mutex;
				static pool_type						m_pool;
			};

			template<unsigned int size>
			boost::detail::lightweight_mutex pool<size>::m_mutex;

			template<unsigned int size>
			pool_type pool<size>::m_pool;

			struct Item
			{
				std::size_t		size;
				void * (*alloc)();
				void (*free)(void *);
			};

			#define DEF_ITEM(SIZE)		{	SIZE,	&pool<SIZE>::malloc,	&pool<SIZE>::free },

			static Item mem_pool_table[]=
			{
				DEF_ITEM(1 << 0)
				DEF_ITEM(1 << 1)
				DEF_ITEM(1 << 2)
				DEF_ITEM(1 << 3)
				DEF_ITEM(1 << 4)
				DEF_ITEM(1 << 5)
				DEF_ITEM(1 << 6)
				DEF_ITEM(1 << 7)
				DEF_ITEM(1 << 8)
				DEF_ITEM(1 << 9)
				DEF_ITEM(1 << 10)
				DEF_ITEM(1 << 11)
				DEF_ITEM(1 << 12)
				DEF_ITEM(1 << 13)
				DEF_ITEM(1 << 14)
				DEF_ITEM(1 << 15)
				DEF_ITEM(1 << 16)
				DEF_ITEM(1 << 17)
				DEF_ITEM(1 << 18)
				DEF_ITEM(1 << 19)
				DEF_ITEM(1 << 20)
				DEF_ITEM(1 << 21)
				DEF_ITEM(1 << 22)
				DEF_ITEM(1 << 23)
				DEF_ITEM(1 << 24)
				DEF_ITEM(1 << 25)
				DEF_ITEM(1 << 26)
				DEF_ITEM(1 << 27)
				DEF_ITEM(1 << 28)
				DEF_ITEM(1 << 29)
				DEF_ITEM(1 << 30)
			};
		}

		//(长度,索引)数组
		static int size_index_map[4][256];

		//使用下一个mem_pool项目
		static void use_mem_pool_item(unsigned int & mem_pool_index,unsigned int &mem_pool_alloc_size)
		{
			++mem_pool_index;
			if( mem_pool_index < sizeof(mem_pool_table)/sizeof(mem_pool_table[0]) )
			{
				mem_pool_alloc_size = mem_pool_table[mem_pool_index].size;
			}
			else
			{
				mem_pool_index = -1;
				mem_pool_alloc_size = 0xFFFFFFFF;
			}
		}

		//初始化索引集合
		static void init_size_index_map()
		{
			unsigned int mem_pool_index=0;
			unsigned int mem_pool_alloc_size=mem_pool_table[0].size;

			for(unsigned int size_index = 0;size_index < 4;++ size_index)
			{
				for(unsigned int index=0;index <256 ;++index)
				{
					unsigned int alloc_size = (index+1) * ( 1 << (size_index*8) );
					if(alloc_size <= mem_pool_alloc_size)
					{
						size_index_map[size_index][index]=mem_pool_index;
					}
					else
					{
						use_mem_pool_item(mem_pool_index,mem_pool_alloc_size);
						size_index_map[size_index][index]=mem_pool_index;
					}
				}
			}
		}

		//获得根本数组索引
		static int get_pool_index(std::size_t size)
		{
			if(size > mem_pool::MAX_MEM_BLOCK_SIZE)
			{
				return -1;
			}
			std::size_t s;
			if(size < mem_pool::MIN_MEM_BLOCK_SIZE)
			{
				s = mem_pool::MIN_MEM_BLOCK_SIZE;
			}
			else
			{
				s = size;
			}

			//警告：仅用于Intel与AMD CPU
			s -=1;	//将长度-1用于以下查询算法

			const unsigned char * size_bytes = reinterpret_cast<const unsigned char *>(&size);
			for(int bytes_index=3;bytes_index>=0;--bytes_index)
			{
				if( size_bytes[bytes_index] > 0)
				{
					return size_index_map[bytes_index][size_bytes[bytes_index]];
				}
			}
			return -1;
		}
		
		mem_pool::mem_pool()
		{
			init_size_index_map();
		}

		void*	mem_pool::alloc(std::size_t size)
		{
			int index = get_pool_index(size);
			if(index < 0)
			{
				return new(std::nothrow) char[size];
			}
			else
			{
				return (*mem_pool_table[index].alloc)();
			}
		}

		void mem_pool::free(void *data,std::size_t size)
		{
			int index = get_pool_index(size);
			if(index < 0)
			{
				return delete [] reinterpret_cast<char *>(data);
			}
			else
			{
				return (*mem_pool_table[index].free)(data);
			}
		}

}
