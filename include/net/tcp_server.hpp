/********************************************************************
	created:	2014/08/19
	created:	19:8:2014   11:01
	file base:	tcp_server
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _FAITH_TCPSERVER_H_
#define _FAITH_TCPSERVER_H_

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/any.hpp>
#include "xchar.hpp"
#include "plugin.hpp"
#include "datablock.hpp"

namespace faith 
{
	namespace net 
	{
		class tcp_server_impl;
		//
		//	asynchronous TCP server facade
		// 
		class tcp_server : private boost::noncopyable
		{			
		public:
			enum e_server_status_type
			{
				e_ss_all_connection_closed = 0xFFFFFFFF,
			};
			typedef boost::function<void(e_server_status_type)>				serverstatus_handler_type;
			typedef boost::function<void(unsigned int)>						onclose_handler_type;
			typedef boost::function<void(unsigned int)>						onconnected_handler_type;
			typedef boost::function<void(unsigned int,const void*,size_t)>	onrecv_handler_type;
			struct options
			{
				template<int check_sum,class value_type>
				struct option_item_type
				{
					typedef value_type type_;
					option_item_type(value_type v) : value(v)
					{

					};
					option_item_type()
					{

					};					
					value_type	value;
				};
				typedef option_item_type<-1,unsigned int>	connections_num_limit;			//the max value of connections number.
				typedef option_item_type<0,unsigned int>	max_packet_size;				//the max logic packet size.default is 8K
				typedef option_item_type<1,unsigned int>	send_buffer_size;				//default is 32K;
				typedef option_item_type<2,unsigned int>	recv_buffer_size;				//default is 16K.
				typedef option_item_type<3,unsigned int>	delaysending_size_threshold;	//default is 0(disabled)
			};
		public:		
			explicit tcp_server(
				serverstatus_handler_type status_handler,
				onconnected_handler_type onconnected_handler,
				onclose_handler_type onclose_handler,
				onrecv_handler_type onrecv_handler,
				xstring ip,
				int tcp_port );
			explicit tcp_server( 
				serverstatus_handler_type status_handler,
				onconnected_handler_type onconnected_handler,
				onclose_handler_type onclose_handler,
				onrecv_handler_type onrecv_handler,
				int tcp_port );
			virtual ~tcp_server();
		public:
			std::size_t							get_conn_count( void );
			xstring								get_ip_addr( unsigned int conn_index );
			unsigned short						get_ip_port( unsigned int conn_index );
			bool								start( void );
			void								stop( bool wait_until_finished = false );
			int									send( unsigned int conn_index,const void *data_ptr,size_t data_len );
			int									send_multi(unsigned int conn_index,const datablock_queue_type& data_queue);
			bool								close( unsigned int conn_index );
			bool								set_option(const boost::any& option_item);
			bool								get_option(boost::any& option_item);
			template<class option_type>
			bool								get_option(option_type& dest)
			{
				boost::any any_value=option_type();
				get_option(any_value);
				option_type *ptr = boost::any_cast<option_type>(&any_value);
				if(ptr == NULL) 
					return false;
				dest = *ptr;
				return true;
			}
			void								init_client_server(unsigned int server_num, unsigned int init_num);
		private:
			boost::scoped_ptr<tcp_server_impl>	impl_ptr;
		};
	}
}

#endif
