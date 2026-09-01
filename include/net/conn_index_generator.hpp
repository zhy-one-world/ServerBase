/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:05
	file base:	conn_index_generator
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _CONN_INDEX_GENERATOR_H_
#define _CONN_INDEX_GENERATOR_H_

#include "unique_id_generator.hpp"

namespace faith
{
	namespace net
	{
		class conn_index_generator
		{
			union conn_index
			{
				struct
				{
					boost::uint16_t		index;
					boost::uint16_t		serial;
				};
				boost::uint32_t			index_32;
			};
			static const boost::uint16_t invalid_index_16 = 0xFFFF;
			static const boost::uint32_t invalid_index_32 = 0xFFFFFFFF;
		public:
			conn_index_generator():
				m_generator(invalid_index_16),
				m_next_serial(0)
			{

			}
		public:
			boost::uint32_t get_id()
			{
				conn_index index;
				index.index = m_generator.get_id();
				if(index.index==invalid_index_16)
				{
					return invalid_index_32;
				}
				else
				{
					index.serial = m_next_serial ++;
					return index.index_32;
				}
			}

			void  return_id(boost::uint32_t id)
			{
				conn_index * index = reinterpret_cast<conn_index *>(&id);
				m_generator.return_id(index->index);
			}

			void set_max_count(boost::uint16_t max_count)
			{
				m_generator.set_max_count(max_count);
			}

		private:
			::faith::unique_id_generator<boost::uint16_t>	m_generator;	//生成器
			boost::uint16_t										m_next_serial;	//下一个序列号
		};
	}
}
#endif
