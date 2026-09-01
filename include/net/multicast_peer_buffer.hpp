/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:21
	file base:	multicast_peer_buffer
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _MULTICAST_PEER_BUFFER_H_
#define _MULTICAST_PEER_BUFFER_H_

#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <vector>

namespace faith 
{
    namespace net
    {
        static const unsigned int sc_ubuffer_size_default = 1024*32;
        class buffer_base
        {
        protected:
            struct tdata_head
            {
                unsigned int  length;
            };
        protected:
            buffer_base(unsigned int buf_size)
            {
                m_buf_size = buf_size;
                m_data = new char[buf_size];
                this->reset();
            }
            virtual ~buffer_base()
            {
				delete []m_data;           
			}
		protected:
            bool            is_empty()           { return m_pos_head==m_pos_tail?true:false; }
            void*           get_DataTailPtr()   { return ((char*)m_data + m_pos_tail);     }
            void*           get_DataHeadPtr()   { return ((char*)m_data + m_pos_head);     }
            virtual void*   get_bodyPtr_length( unsigned int data_len)
            {
                unsigned int uLen = data_len+sizeof(tdata_head);
                if(m_buf_size<=uLen)
                    return 0;

                if( 0!=m_pos_head && m_pos_head==m_pos_tail)
                {
                    m_pos_head=m_pos_tail = 0;
                    m_pre_pos_tail        = 0;
                }

                if( m_pos_tail>m_pos_head)
                {
                    unsigned int iTailFreeLen = m_buf_size-m_pos_tail;
                    unsigned int iHeadFreeLen = m_pos_head;
                    if( iTailFreeLen>uLen)
                    {
                    }
					else if( iHeadFreeLen>uLen)
                    {//这里调头了
                        m_pre_pos_tail = m_pos_tail;
                        m_pos_tail    = 0;
                    }
					else
                        return 0;
                }
				else if( m_pos_tail<m_pos_head )
                {
                    if( m_pos_head-m_pos_tail<=uLen )
                        return 0;
                }
                return ( (char*)m_data + m_pos_tail);
            }

            void            reset()
            {
                m_pos_tail = m_pos_head=m_pre_pos_tail = 0;
            }
        public:
            void            pop()
            {
                if( this->is_empty() )
                    return;

                tdata_head* pHeader = (buffer_base::tdata_head*)((char*)m_data+m_pos_head);
                m_pos_head += (pHeader->length+sizeof(tdata_head));
                if( m_pos_head>m_pos_tail && m_pos_head==m_pre_pos_tail )
                {
                    assert(m_pre_pos_tail>=m_pos_head);
                    m_pos_head    = 0;
                    m_pre_pos_tail = 0;
                }
            }
		protected:
			char *          m_data;
			unsigned int    m_pos_head;
			unsigned int    m_pos_tail;
			unsigned int    m_buf_size;
			unsigned int    m_pre_pos_tail; //是否应该掉头
        };

        class send_buffer : public buffer_base
        {
        public:
            send_buffer(unsigned int buf_size=sc_ubuffer_size_default):buffer_base(buf_size)
            {   
			}
            virtual ~send_buffer()
            {   
			}
		public:
            bool  push(const void *data_ptr, unsigned int data_len)
            {
                if( data_len>= m_buf_size-sizeof(tdata_head)-1 )
                    return false;

                tdata_head  m_Header;
                m_Header.length = data_len;
                void *pFreeData = this->get_bodyPtr_length( data_len+sizeof(tdata_head) );
                if( pFreeData )
                {
                    memcpy(pFreeData,&m_Header,sizeof(tdata_head));
                    memcpy( (char*)pFreeData+sizeof(tdata_head),data_ptr,data_len );
                    m_pos_tail += data_len+sizeof(tdata_head);
                    return true;
                }
                return false;
            }
            void* front(unsigned int &data_len)
            {
                if( this->is_empty() )
                    return 0;
                tdata_head* pHeader = (tdata_head*)((char*)m_data+m_pos_head);
                data_len = pHeader->length+sizeof(tdata_head);
                assert(data_len<m_buf_size);
                if( data_len>=m_buf_size )
                {
                    this->reset();
                    return 0;
                }
                return (char*)m_data+m_pos_head;
            }
        };

        class receive_buffer : public buffer_base
        {
        public:
            receive_buffer(unsigned int buf_size=sc_ubuffer_size_default):buffer_base(buf_size)
            {
            }
            virtual ~receive_buffer()
            {
            }
		public:
            void* open_for_push( unsigned int data_len )
            {
                void *pFreeData = this->get_bodyPtr_length( data_len );
                if( pFreeData )
                {
                    return pFreeData;
                }
                return 0;
            }
            void   finish_push(unsigned int data_len)
            {
                if( data_len<=0 )
                {
                    return;
                }

                tdata_head *pHead = (tdata_head*)this->get_DataTailPtr();
                if( pHead->length==data_len-sizeof(tdata_head) )
                {
                    m_pos_tail += data_len;
                }else
                {//error
                    assert(0);
                }
            }

            void* front(unsigned int &data_len)
            {
                data_len = 0;
                if( this->is_empty() )
                    return 0;

                tdata_head* pHeader = (tdata_head*)((char*)m_data+m_pos_head);
                data_len = pHeader->length;
                if( data_len>m_buf_size )
                {
                    this->reset();
                    data_len = 0;
                    return 0;
                }
                return (char*)m_data+m_pos_head+sizeof(tdata_head);
            }

        };
    }
}

#endif
