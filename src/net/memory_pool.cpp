/********************************************************************
	created:	2011/05/24
	created:	24:5:2011   18:57
	file base:	MemoryPool
	file ext:	cpp
	author:		zhangming
	
	purpose:	
*********************************************************************/
#include "memory_pool.hpp"
//#include "faith_logger.hpp"
#include "scheduler.hpp"

namespace faith
{
	namespace net
	{	
		//------------------------------------------------------------------------
		// MemoryBlock members
		//------------------------------------------------------------------------
		memory_block::memory_block(unsigned int blockSize)
		{
			m_allocate = m_buffer = ::malloc(blockSize);
			if (m_buffer)
			{
				m_end = (char*)m_buffer + blockSize;
			}
		}

		memory_block::~memory_block()
		{
			if (m_buffer)
			{
				::free(m_buffer);
			}
			m_buffer = m_allocate = m_end = NULL;
		}

		void* memory_block::malloc(unsigned int chunkSize)
		{
			if ((char*)m_allocate+chunkSize <= m_end)
			{
				void* chunk = m_allocate;
				m_allocate = (char*)m_allocate + chunkSize;
				return chunk;
			}
			return NULL;
		}

		//------------------------------------------------------------------------
		// MemoryPool members
		//------------------------------------------------------------------------
		memory_pool::memory_pool(unsigned int chunkSize)
			:m_chunk_size(chunkSize)
		{
			if (m_chunk_size <= (256*1024))
			{
				m_chunk_num = 512;
			}
			else
			{
				m_chunk_num = 1;
			}

			m_block_size = m_chunk_size * m_chunk_num;
			m_current_block = create_block();
		}

		memory_pool::~memory_pool()
		{
			release();
		}

		memory_block* memory_pool::create_block()
		{
			memory_block* pNewBlock = new memory_block(m_block_size);
			if (!pNewBlock)
			{
				return NULL;
			}
			else if (!pNewBlock->valid())
			{
				delete pNewBlock;
				return NULL;
			}

			m_block_list.push_back(pNewBlock);
			return pNewBlock;
		}

		void memory_pool::release()
		{
			block_list::iterator iter = m_block_list.begin();
			for (; iter!=m_block_list.end(); ++iter)
			{
				memory_block* pBlock = *iter;
				if (pBlock)
				{
					delete pBlock;
				}
			}
			m_block_list.clear();
		}

		void* memory_pool::malloc()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			void* pChunk = NULL;
			if (!m_free_list.empty())
			{
				pChunk = m_free_list.front();
				m_free_list.pop_front();
				return pChunk;
			}

			if (!m_current_block)
			{
				return NULL;
			}

			pChunk = m_current_block->malloc(m_chunk_size);
			if (pChunk)
			{
				return pChunk;
			}

			m_current_block = create_block();
			if (!m_current_block)
			{
				return NULL;
			}

			pChunk = m_current_block->malloc(m_chunk_size);
			return pChunk;
		}

		void memory_pool::free(void* chunk)
		{
			if (!chunk)
			{
				return;
			}

			std::lock_guard<std::mutex> lock(m_mutex);
			m_free_list.push_back(chunk);
		}
	}
}
