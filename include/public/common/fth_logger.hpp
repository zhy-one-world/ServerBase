//
//	most of below code was borrowed from 'pion lib'
//

#ifndef __FTH_LOGGER_HEADER__
#define __FTH_LOGGER_HEADER__

#include "xchar.hpp"

#define FTH_USE_LOG4CXX
#define FILE_ENCODING _XTEXT("utf-16")

#if defined(FTH_USE_LOG4CXX)

#ifndef  LOG4CXX_STATIC
#define  LOG4CXX_STATIC
#endif

#include <locale.h>
#include <log4cxx/logger.h>
#include <log4cxx/stream.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/simplelayout.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/ndc.h>

#ifdef LOG4CXX_STATIC
#undef  LOG4CXX_STATIC
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;

//==========================================Log4cxx===============================================
namespace faith
{
	class fth_logger
	{
	public:
		enum e_log_level_type 
		{
			LOG_LEVEL_DEBUG=1, LOG_LEVEL_INFO, LOG_LEVEL_WARN,
			LOG_LEVEL_ERROR, LOG_LEVEL_FATAL
		};
		fth_logger(xstring name=_XTEXT("FAITH"));
		~fth_logger();
		LoggerPtr get_logger()    {  return m_logger;  }

	private:
		LoggerPtr m_logger;
		e_log_level_type m_level_type;
	};
}

//autolink begin
#define FAITH_LIB_NAME "xml"
#include "faith_auto_link.hpp"
#define FAITH_LIB_NAME "apr"
#include "faith_auto_link.hpp"
#define FAITH_LIB_NAME "aprutil"
#include "faith_auto_link.hpp"
#define FAITH_LIB_NAME "log4cxx"
#include "faith_auto_link.hpp"
//autolink end


#define FTH_LOG_CONFIG_BASIC	{}
//	#define OMP_GET_LOGGER(NAME)	omp::OmpLogger(NAME)

#define FTH_LOG_SETLEVEL_DEBUG(LOG)	{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_DEBUG); }
#define FTH_LOG_SETLEVEL_INFO(LOG)	{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_INFO); }
#define FTH_LOG_SETLEVEL_WARN(LOG)	{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_WARN); }
#define FTH_LOG_SETLEVEL_ERROR(LOG)	{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_ERROR); }
#define FTH_LOG_SETLEVEL_FATAL(LOG)	{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_FATAL); }

#ifndef FTH_NO_LOG
#define FTH_LOG_DEBUG(logger, MSG)    LOG4CXX_DEBUG((logger).get_logger(), MSG)
#define FTH_LOG_INFO(logger, MSG)     LOG4CXX_INFO((logger).get_logger(), MSG)
#define FTH_LOG_WARN(logger, MSG)     LOG4CXX_WARN((logger).get_logger(), MSG)
#define FTH_LOG_ERROR(logger, MSG)    LOG4CXX_ERROR((logger).get_logger(), MSG)
#define FTH_LOG_FATAL(logger, MSG)    LOG4CXX_FATAL((logger).get_logger(), MSG)
#define FTH_LOG_RELEASE(logger, MSG)  LOG4CXX_FATAL((logger).get_logger(), MSG)	
#else
#define FTH_LOG_WARN(logger, MSG)
#define FTH_LOG_DEBUG(logger, MSG)
#define FTH_LOG_INFO(logger, MSG)
#define FTH_LOG_ERROR(logger, MSG)
#define FTH_LOG_FATAL(logger, MSG)
#define FTH_LOG_RELEASE(logger, MSG)    LOG4CXX_FATAL((logger).get_logger(), MSG)
#endif

#if defined(FTH_UNICODE)

/* exception */
namespace log4cxx
{
	namespace helpers
	{
		class WideMessageBuffer;
		class MessageBuffer;
		WideMessageBuffer& operator<<(MessageBuffer& os, const std::exception& val);
		WideMessageBuffer& operator<<(WideMessageBuffer& os, const std::exception& val);
	}
}

namespace std
{
	std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& os, const std::exception& val);
}

#else

/* exception */
namespace log4cxx
{
	namespace helpers
	{
		class CharMessageBuffer;
		class MessageBuffer;
		CharMessageBuffer& operator<<(MessageBuffer& os, const std::exception& val);
		CharMessageBuffer& operator<<(CharMessageBuffer& os, const std::exception& val);
	}
}


#endif


//==========================================Log4cxx===============================================

#elif defined(FTH_USE_LOG4CPLUS)

#ifndef  LOG4CPLUS_STATIC
#define  LOG4CPLUS_STATIC
#endif

#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>

#ifdef LOG4CPLUS_STATIC
#undef  LOG4CPLUS_STATIC
#endif

using namespace log4cplus;
using namespace log4cplus::helpers;

//==========================================Log4cplus===============================================
namespace faith
{
	class fth_logger
	{
	public:
		enum e_log_level_type 
		{
			LOG_LEVEL_DEBUG=1, LOG_LEVEL_INFO, LOG_LEVEL_WARN,
			LOG_LEVEL_ERROR, LOG_LEVEL_FATAL
		};
		fth_logger(xstring name=_XTEXT("OMP"), e_log_level_type level_type=LOG_LEVEL_INFO);
		~fth_logger();
		bool add_file_appender(xstring name, xstring filename=_XTEXT("test.log"), xstring pattern=_XTEXT("%m"), bool roll=true, bool append=false);
		bool add_console_appender(xstring name, xstring pattern=_XTEXT("%m"));
		void set_level(e_log_level_type level_type=LOG_LEVEL_DEBUG);
		Logger get_logger()    {  return m_logger;  }

	private:
		Logger m_logger;
		e_log_level_type m_level_type;
	};
}

#if defined _DEBUG
#if defined _DLL
#pragma comment(lib, "log4cplusSDU_DLL")
#else
#pragma comment(lib, "log4cplusSDU")
#endif
#else
#if defined _DLL
#pragma comment(lib, "log4cplusSU_DLL")
#else
#pragma comment(lib, "log4cplusSU")
#endif
#endif

#define FTH_LOG_CONFIG_BASIC	{}
//	#define OMP_GET_LOGGER(NAME)	omp::OmpLogger(NAME)

#define FTH_LOG_SETLEVEL_DEBUG(LOG)			{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_DEBUG); }
#define FTH_LOG_SETLEVEL_INFO(LOG)		{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_INFO); }
#define FTH_LOG_SETLEVEL_WARN(LOG)		{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_WARN); }
#define FTH_LOG_SETLEVEL_ERROR(LOG)	{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_ERROR); }
#define FTH_LOG_SETLEVEL_FATAL(LOG)	{ (LOG).set_level(faith::fth_logger::LOG_LEVEL_FATAL); }

#define FTH_LOG_WARN(logger, MSG)    LOG4CPLUS_WARN((logger).get_logger(), MSG)
#define FTH_LOG_DEBUG(logger, MSG)    LOG4CPLUS_DEBUG((logger).get_logger(), MSG)
#define FTH_LOG_INFO(logger, MSG)    LOG4CPLUS_INFO((logger).get_logger(), MSG)
#define FTH_LOG_ERROR(logger, MSG)    LOG4CPLUS_ERROR((logger).get_logger(), MSG)
#define FTH_LOG_FATAL(logger, MSG)    LOG4CPLUS_FATAL((logger).get_logger(), MSG)
//==========================================Log4cplus===============================================
#elif defined(FTH_USE_LOG4CPP)


// log4cpp headers
#include <log4cpp/Category.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/OstreamAppender.hh>

namespace faith {
	typedef log4cpp::Category*	fth_logger;
}

#define FTH_LOG_CONFIG_BASIC	{ log4cpp::OstreamAppender *app = new log4cpp::OstreamAppender("cout", &xcout); app->setLayout(new log4cpp::BasicLayout()); log4cpp::Category::getRoot().setAppender(app); }
#define FTH_GET_LOGGER(NAME)	(&log4cpp::Category::getInstance(NAME))

#define FTH_LOG_SETLEVEL_DEBUG(LOG)	{ LOG->setPriority(log4cpp::Priority::DEBUG); }
#define FTH_LOG_SETLEVEL_INFO(LOG)		{ LOG->setPriority(log4cpp::Priority::INFO); }
#define FTH_LOG_SETLEVEL_WARN(LOG)		{ LOG->setPriority(log4cpp::Priority::WARN); }
#define FTH_LOG_SETLEVEL_ERROR(LOG)	{ LOG->setPriority(log4cpp::Priority::ERROR); }
#define FTH_LOG_SETLEVEL_FATAL(LOG)	{ LOG->setPriority(log4cpp::Priority::FATAL); }

#define FTH_LOG_DEBUG(LOG, MSG)	if (LOG->getPriority()>=log4cpp::Priority::DEBUG) { LOG->debugStream() << MSG; }
#define FTH_LOG_INFO(LOG, MSG)		if (LOG->getPriority()>=log4cpp::Priority::INFO) { LOG->infoStream() << MSG; }
#define FTH_LOG_WARN(LOG, MSG)		if (LOG->getPriority()>=log4cpp::Priority::WARN) { LOG->warnStream() << MSG; }
#define FTH_LOG_ERROR(LOG, MSG)	if (LOG->getPriority()>=log4cpp::Priority::ERROR) { LOG->errorStream() << MSG; }
#define FTH_LOG_FATAL(LOG, MSG)	if (LOG->getPriority()>=log4cpp::Priority::FATAL) { LOG->fatalStream() << MSG; }

#elif defined(FTH_DISABLE_LOGGING)

// Logging is disabled -> add do-nothing stubs for logging
namespace faith {
	typedef int		fth_logger;
}

#define FTH_LOG_CONFIG_BASIC	{}
#define FTH_GET_LOGGER(NAME)	0

// use "++LOG" to avoid warnings about LOG not being used
#define FTH_LOG_SETLEVEL_DEBUG(LOG)	{ if (false) ++LOG; }
#define FTH_LOG_SETLEVEL_INFO(LOG)		{ if (false) ++LOG; }
#define FTH_LOG_SETLEVEL_WARN(LOG)		{ if (false) ++LOG; }
#define FTH_LOG_SETLEVEL_ERROR(LOG)	{ if (false) ++LOG; }
#define FTH_LOG_SETLEVEL_FATAL(LOG)	{ if (false) ++LOG; }

// use "++LOG" to avoid warnings about LOG not being used
#define FTH_LOG_DEBUG(LOG, MSG)	{ if (false) ++LOG; }
#define FTH_LOG_INFO(LOG, MSG)		{ if (false) ++LOG; }
#define FTH_LOG_WARN(LOG, MSG)		{ if (false) ++LOG; }
#define FTH_LOG_ERROR(LOG, MSG)	{ if (false) ++LOG; }
#define FTH_LOG_FATAL(LOG, MSG)	{ if (false) ++LOG; }

#else

#define FTH_USE_OSTREAM_LOGGING

// Logging uses std::cout and std::cerr
#include <iostream>
#include <string>
#include <ctime>

namespace faith {
	struct fth_logger {
		enum e_log_level_type 
		{
			LOG_LEVEL_DEBUG=1, LOG_LEVEL_INFO, LOG_LEVEL_WARN,
			LOG_LEVEL_ERROR, LOG_LEVEL_FATAL
		};
		~fth_logger() {}
		fth_logger(void) : m_name(_XTEXT("faith")) {}
		fth_logger(const xstring& name) : m_name(name) {}
		fth_logger(const fth_logger& p) : m_name(p.m_name) {}
		bool add_file_appender(xstring name, xstring filename=_XTEXT("test.log"), xstring pattern=_XTEXT("%m %n"), bool append=false){return true;}
		bool add_console_appender(xstring name, xstring pattern=_XTEXT("%m %n")){return true;}
		void set_level(e_log_level_type level_type=LOG_LEVEL_DEBUG){m_priority = level_type;}
		xstring					m_name;
		static e_log_level_type			m_priority;
	};
}

#define FTH_LOG_CONFIG_BASIC	{}
#define FTH_GET_LOGGER(NAME)	faith::fth_logger(NAME)

#define FTH_LOG_SETLEVEL_DEBUG(LOG)	{ LOG.m_priority = faith::fth_logger::LOG_LEVEL_DEBUG; }
#define FTH_LOG_SETLEVEL_INFO(LOG)		{ LOG.m_priority = faith::fth_logger::LOG_LEVEL_INFO; }
#define FTH_LOG_SETLEVEL_WARN(LOG)		{ LOG.m_priority = faith::fth_logger::LOG_LEVEL_WARN; }
#define FTH_LOG_SETLEVEL_ERROR(LOG)	{ LOG.m_priority = faith::fth_logger::LOG_LEVEL_ERROR; }
#define FTH_LOG_SETLEVEL_FATAL(LOG)	{ LOG.m_priority = faith::fth_logger::LOG_LEVEL_FATAL; }

#define FTH_LOG_DEBUG(LOG, MSG)	if (LOG.m_priority <= faith::fth_logger::LOG_LEVEL_DEBUG) { xcout << time(NULL) << _XTEXT(" DEBUG ") << LOG.m_name << _XTEXT(' ') << MSG << std::endl; }
#define FTH_LOG_INFO(LOG, MSG)		if (LOG.m_priority <= faith::fth_logger::LOG_LEVEL_INFO) { xcout << time(NULL) << _XTEXT(" INFO ") << LOG.m_name << _XTEXT(' ') << MSG << std::endl; }
#define FTH_LOG_WARN(LOG, MSG)		if (LOG.m_priority <= faith::fth_logger::LOG_LEVEL_WARN) { xcerr << time(NULL) << _XTEXT(" WARN ") << LOG.m_name << _XTEXT(' ') << MSG << std::endl; }
#define FTH_LOG_ERROR(LOG, MSG)	if (LOG.m_priority <= faith::fth_logger::LOG_LEVEL_ERROR) { xcerr << time(NULL) << _XTEXT(" ERROR ") << LOG.m_name << _XTEXT(' ') << MSG << std::endl; }
#define FTH_LOG_FATAL(LOG, MSG)	if (LOG.m_priority <= faith::fth_logger::LOG_LEVEL_FATAL) { xcerr << time(NULL) << _XTEXT(" FATAL ") << LOG.m_name << _XTEXT(' ') << MSG << std::endl; }

#endif

#endif	// #define __FTH_LOGGER_HEADER__
