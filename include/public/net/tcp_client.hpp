/********************************************************************

	created:	2014/05/04

	created:	4:5:2014   19:02

	file base:	tcp_client

	file ext:	hpp

	author:		zhy

	

	purpose:	

*********************************************************************/

#ifndef _TCP_CLIENT_H_

#define _TCP_CLIENT_H_



#include <boost/noncopyable.hpp>

#include <boost/function.hpp>

#include <boost/scoped_ptr.hpp>

#include <boost/thread.hpp>

#include <boost/any.hpp>

#include <memory>

#include <string>

#include "xchar.hpp"

#include "plugin.hpp"

#include "datablock.hpp"



namespace faith

{

	namespace net

	{

		class tcp_client_impl;

		class tcp_client_session;

		typedef std::shared_ptr<tcp_client_session> tcp_client_session_ptr;


		//	asynchronous TCP client facade, Singleton class


		class tcp_client : private boost::noncopyable

		{

		public:

			enum e_connect_info

			{

				e_ci_common_error=0,

				e_ci_addr_resovle_failed,

				e_ci_addr_resovle_successed,

				e_ci_connection_failed,

				e_ci_connection_successed

			};



			typedef boost::function<void(tcp_client_session_ptr)>							onclose_handler_type;

			typedef boost::function<void(tcp_client_session_ptr,e_connect_info,xstring)>	connection_handler_type;

			typedef boost::function<void(tcp_client_session_ptr,const void*,size_t)>		onrecv_handler_type;



			struct options

			{

				template<int check_sum,class value_type>

				struct option_item_type

				{

					typedef value_type type_;

					value_type	value;



					option_item_type(value_type v):value(v)	{};

					option_item_type()	{};

				};



				typedef option_item_type<0,unsigned int>						max_packet_size;				//the max logic packet size.default is 8K

				typedef option_item_type<1,unsigned int>						send_buffer_size;				//default is 32K

				typedef option_item_type<2,unsigned int>						recv_buffer_size;				//default is 16K.

				typedef option_item_type<3,unsigned int>						delaysending_size_threshold;	//default is 0(disabled)

			};



		public:

			tcp_client();

			~tcp_client();

		public:

			static tcp_client&					get_instance(void);

			tcp_client_session_ptr				connect_to(

				xstring ip,xstring service_port,

				connection_handler_type connection_handler,

				onclose_handler_type onclose_handler,

				onrecv_handler_type onrecv_handler);



			void								disconnect(const tcp_client_session_ptr& session);

			int									send(const tcp_client_session_ptr& session,const void *data_ptr,size_t data_len);

			int									send_multi(const tcp_client_session_ptr& session,const datablock_queue_type& data_queue);

			bool								set_option(const boost::any& option_item);

			bool								get_option(boost::any& option_item);

			template<class option_type>

			bool								get_option(option_type& dest)

			{

				boost::any any_value=option_type();

				get_option(any_value);

				option_type *ptr=boost::any_cast<option_type>(&any_value);

				if(ptr==NULL) 

					return false;

				dest=*ptr;

				return true;

			}

		private:

			static void							create_instance();

		public:

			static const unsigned int			invalid_conn_index = 0xFFFFFFFF;

		private:

			boost::scoped_ptr<tcp_client_impl>	impl_ptr;

			static tcp_client*					m_instance_ptr;

			static boost::once_flag				m_instance_flag;

		};

	}

}



#endif

