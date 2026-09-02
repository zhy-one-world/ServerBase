/********************************************************************
	created:	2011/05/24
	created:	24:5:2011   18:36
	file base:	MemoryPool
	file ext:	hpp
	author:		zhanmging
	
	purpose:	
*********************************************************************/
#ifndef _MEMORY_POOL_
#define _MEMORY_POOL_

#include <iostream>
#include <list>
#include <deque>
#include <mutex>

namespace faith
{
	namespace net
	{	
		class memory_block
		{
		public:
			memory_block(unsigned int blockSize);
			~memory_block();
		public:
			void*			malloc(unsigned int chunkSize);
			bool			valid();
		private:
			void*			m_buffer;
			void*			m_end;
			void*			m_allocate;
		};

		inline bool	memory_block::valid() {return m_buffer!=NULL;}

		class memory_pool
		{
		private:
			typedef std::list<memory_block*>	block_list;
			typedef std::deque<void*>			free_chunk_list;

		public:
			memory_pool(unsigned int chunkSize);
			~memory_pool();
		public:
			void*			malloc();
			void			free(void* chunk);
			unsigned int	get_requested_size();
		private:
			memory_block*	create_block();
			void			release();
		private:
			unsigned int	m_chunk_size;
			unsigned int	m_chunk_num;
			unsigned int	m_block_size;
			free_chunk_list	m_free_list;
			block_list		m_block_list;
			memory_block*	m_current_block;
			mutable std::mutex m_mutex;
		};

		inline unsigned int memory_pool::get_requested_size() {return m_chunk_size;}
	}
}

#endif
