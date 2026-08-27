/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   14:24
	file base:	work_flow
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _WORK_FLOW_H_
#define _WORK_FLOW_H_

#include <boost/thread.hpp>

namespace faith
{
	namespace common
	{
		class work_flow
		{
		public:
			work_flow(void);
			virtual ~work_flow(void);
		public:
			bool			start( bool sync );
			void			terminate( void );
			bool			is_terminated( void );
			bool			is_started( void );
			void			wait( void );
		protected:
			virtual void	execute( void ) = 0;
			virtual void	on_error( int nErrorCode) { }
			virtual void	on_terminated(void) { }
			bool			is_sync() const;
		private:
			bool			has_thread() const;
		protected:
			bool			m_started;
			volatile bool	m_terminated;
			boost::thread	m_thread;
			bool			m_sync;
		};
	}
}
#endif
