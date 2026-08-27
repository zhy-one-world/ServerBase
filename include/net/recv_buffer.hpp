/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:31
	file base:	recv_buffer
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _RECV_BUFFER_H_
#define _RECV_BUFFER_H_

#include <cassert>
#include <boost/pool/pool.hpp>
#include "xchar.hpp"
#include "memory_pool.hpp"
#include "scheduler.hpp"
#include <boost/bind.hpp>
#include "tcp_pak_def.hpp"

namespace faith 
{
	namespace net
	{
		class recv_buffer
		{
		public:
			typedef memory_pool	recv_buffer_allocator;
			recv_buffer(unsigned int buffer_size, unsigned max_packet_size,recv_buffer_allocator& allocator) :
				m_buffer_size(buffer_size),
				m_max_packet_size(max_packet_size),
				m_allocator(&allocator),
				m_total_size(0)
			{
				assert(m_buffer_size + sizeof(tcp_pak_header) + m_max_packet_size == m_allocator->get_requested_size());
				m_begin = m_end = m_buffer = static_cast<char *>(m_allocator->malloc());
				m_parse_header = reinterpret_cast<tcp_pak_header *>(m_buffer);
				if (m_buffer)
				{
					m_buffer_tail = m_buffer + m_buffer_size + sizeof(tcp_pak_header) + m_max_packet_size;
					m_extra_begin = m_buffer + m_buffer_size;
				}
				else
				{
					//FAITH_LOG_INFO(scheduler::getInstance().get_logger(), _XTEXT("recv_buffer error m_buffer_size:") << m_buffer_size << _XTEXT("\n\t"));
					m_buffer_size = 0;
					m_buffer_tail = m_extra_begin = NULL;
				}
			}

			~recv_buffer()
			{
				if(m_allocator && m_buffer)
				{
					m_allocator->free(m_buffer);
				}			
			}
		public:
			void clear_data()
			{
				m_total_size = 0;
				m_begin = m_end = m_buffer;
				m_buffer_tail = m_buffer + m_buffer_size + sizeof(tcp_pak_header) + m_max_packet_size;
				m_extra_begin = m_buffer + m_buffer_size;
			}
		public:
			// return NULL if there is no available buffer
			void*							get_free_buffer(unsigned int & size)
			{
				//static opc::opc_counter_item  buffer_swap_counter(_XTEXT("engine/net/recv buffer swap times"),_XTEXT("net receive buffer roll back times"),true);
				if(m_end > m_begin)
				{
					if(reinterpret_cast<char *>(m_parse_header) <=  m_extra_begin)
					{
						size = m_buffer_tail - m_end;
						return m_end;
					}
					else
					{//try to move the tail data to head
						int unparsed_size = m_end - reinterpret_cast<char *>(m_parse_header);
						if(unparsed_size!=0)
						{
							if( unparsed_size > (m_begin - m_buffer) )
							{
								return NULL;
							}
							else
							{
								memcpy(m_buffer,reinterpret_cast<char *>(m_parse_header),unparsed_size);
							}
						}

						m_end = m_buffer + unparsed_size;
						if(m_begin==reinterpret_cast<char *>(m_parse_header))
						{
							m_begin = m_buffer;
							m_parse_header = reinterpret_cast<tcp_pak_header *>(m_buffer);
							size = m_buffer_tail - m_end;
							return m_end;
						}
						else
						{
							//++buffer_swap_counter;

							m_head_end = reinterpret_cast<char *>(m_parse_header);
							m_parse_header = reinterpret_cast<tcp_pak_header *>(m_buffer);
							
							if(m_end == m_begin)
							{
								return NULL;
							}
							else
							{
								size = m_begin - m_end;
								return m_end;
							}
						}
					}
				}
				else if(m_end < m_begin)
				{
					size = m_begin - m_end;
					return m_end;
				}
				else
				{
					if( m_total_size == 0 )
					{
						m_begin = m_end = m_buffer;
						m_parse_header = reinterpret_cast<tcp_pak_header *>(m_buffer);
						size = m_buffer_tail - m_end; 
						return m_end;
					}
					else
					{
						return NULL;
					}
				}
			}

			// pre: there must be availabe space
			// return -1 if the packet size larger than max_packet_size
			// return 0 if there is no available packet
			// otherwise return the number of packtes
			int								push_data(unsigned int size)
			{
				m_end += size;
				m_total_size += size;
				//split the data to packets
				int packets_count = 0;
				unsigned int unparsed_size = m_end - reinterpret_cast<char *>(m_parse_header);
				while(unparsed_size >= sizeof(tcp_pak_header))
				{
					unparsed_size -= sizeof(tcp_pak_header);
					if(m_parse_header->length > m_max_packet_size)
					{
						return -1;
					}
					if(m_parse_header->length > unparsed_size )
					{
						break;
					}
					unparsed_size -= m_parse_header->length;
					m_parse_header = reinterpret_cast<tcp_pak_header *>( reinterpret_cast<char *>(m_parse_header)+m_parse_header->length+sizeof(tcp_pak_header));
					++ packets_count;					
				}
				return packets_count;
			}

			//pre: there muse be avialabe packet
			const void*						get_head_packet(unsigned int & size) const
			{
				tcp_pak_header * header = reinterpret_cast<tcp_pak_header *>(m_begin);
				size = header->length;
				char* data_begin = m_begin + sizeof(tcp_pak_header);
				if (header->key > 0)
				{
					for (int i = 0; i < size; ++i)
					{
						data_begin[i] ^= header->key;
					}
				}
				return data_begin;
			}

			//pre: there muse be avialabe packet
			void							pop_head_packet()
			{
				tcp_pak_header * header = reinterpret_cast<tcp_pak_header *>(m_begin);
				m_begin += header->length + sizeof(tcp_pak_header);
				m_total_size -= header->length + sizeof(tcp_pak_header);
				if(m_begin > m_end && m_begin == m_head_end)
				{
					m_begin = m_buffer;
				}
			}

		private:			
			recv_buffer_allocator*			m_allocator;
			char*							m_buffer;
			char*							m_begin;
			char*							m_end;
			char*							m_head_end;		//only available when ( m_end < m_begin ) or ( m_end == m_begin and there is some data).
			tcp_pak_header*					m_parse_header;
			char*							m_buffer_tail;
			char*							m_extra_begin;
			unsigned int					m_max_packet_size;
			unsigned int					m_buffer_size;
			unsigned int					m_total_size;
		};
	}
}

#endif
