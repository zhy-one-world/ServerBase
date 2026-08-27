/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:39
	file base:	send_buffer
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _SEND_BUFFER_H_
#define _SEND_BUFFER_H_

#include <cassert>
#include <boost/pool/pool.hpp>
#include "xchar.hpp"
#include "memory_pool.hpp"

namespace faith
{
	namespace net
	{
		class send_buffer
		{
		public:
			typedef memory_pool	send_buffer_allocator;
			send_buffer(unsigned int buffer_size,unsigned int delaysending_size_threshold,send_buffer_allocator & allocator):
				m_buffer_size(buffer_size),
				m_total_size(0),
				m_allocator(allocator),
				m_delaysending_size_threshold(delaysending_size_threshold)
			{
				m_begin = m_end = m_buffer = static_cast<char *>(m_allocator.malloc());
				if (m_buffer)
				{
					m_buffer_tail = m_buffer + m_buffer_size;
				}
				else
				{
					//FAITH_LOG_INFO(scheduler::getInstance().get_logger(), _XTEXT("send_buffer init error m_buffer_size:") << m_buffer_size << _XTEXT("\n\t"));
					m_buffer_size = 0;
					m_buffer_tail = NULL;
				}
			}

			~send_buffer()
			{
				if (m_buffer)
					m_allocator.free(m_buffer);
			}
		public:
			void clear_data()
			{
				m_total_size = 0;
				m_begin = m_end = m_buffer;
				m_buffer_tail = m_buffer + m_buffer_size;
			}
		public:
			bool							empty() const
			{
				return m_total_size == 0;
			}
			unsigned int					get_total_size() { return m_total_size; }
			unsigned int					get_buff_size() { return m_buffer_size; }
			bool							can_push(unsigned int size) const
			{
				return m_total_size + size <= m_buffer_size;
			}

			unsigned int					get_total_size()const
			{
				return m_total_size;
			}
			unsigned int					get_buffer_size()const
			{
				return m_buffer_size;
			}
			//It's the caller's responsibility for assuring enough space.
			//The caller can call can_push at first before calling push_pack.
			void							push(const void * data,unsigned int size)
			{
				//static opc::opc_counter_item  buffer_swap_counter(_XTEXT("engine/net/send buffer swap times"),_XTEXT("net send buffer roll back counter"),true);
				if(m_total_size == 0)
				{
					assert(m_begin == m_buffer);
					assert(m_end == m_buffer);
					memcpy(m_end,data,size);
					m_end += size;
					if(m_end == m_buffer_tail)
					{
						m_end = m_buffer;
					}
				}
				else
				{
					assert(m_end < m_buffer_tail);
					int over_count = m_end + size - m_buffer_tail;
					if(over_count <= 0)
					{
						memcpy(m_end,data,size);
						m_end += size;
						if(m_end == m_buffer_tail)
						{
							m_end = m_buffer;
						}
					}
					else
					{
						memcpy(m_end,data,size-over_count);
						memcpy(m_buffer,static_cast<const char *>(data) + size-over_count,over_count);
						m_end = m_buffer + over_count;
						//++buffer_swap_counter;
					}
				}
				m_total_size += size;
			}

			//It's the caller's responsibility for assuring that there's somethineg
			void							pop(unsigned int size)
			{
				assert(m_total_size >= size);
				m_total_size -= size;
				if(m_total_size == 0)
				{
					m_begin = m_end = m_buffer;
				}
				else
				{
					m_begin += size;
					if(m_begin >= m_buffer_tail)
					{
						m_begin -= m_buffer_size;
					}
				}
			}

			void*							header_data(unsigned int & size) const
			{
				if(m_total_size == 0)
				{
					return NULL;
				}
				else
				{
					if(m_end > m_begin)
					{
						size = m_total_size;
					}
					else
					{
						size = m_buffer_tail - m_begin;
					}
					return m_begin;
				}
			}
			
			//取出可以发送的累积的数据
			void*							nagle_data(unsigned int & size) const
			{
				if(m_total_size == 0)
				{
					return NULL;
				}
				else
				{
					if(m_end > m_begin)
					{//正常情况，判断数据包大小
						if(m_total_size >= m_delaysending_size_threshold)
						{
							size = m_total_size;
						}
						else
						{
							return NULL;
						}
					}
					else
					{//回绕产生时，不作检测
						size = m_buffer_tail - m_begin;
					}
					return m_begin;
				}
			}
		private:
			send_buffer_allocator&			m_allocator;
			char*							m_buffer;
			char*							m_begin;
			char*							m_end;
			char*							m_buffer_tail;
			unsigned int					m_buffer_size;
			unsigned int					m_total_size;
			unsigned int					m_delaysending_size_threshold;
		};
	}
}
#endif
