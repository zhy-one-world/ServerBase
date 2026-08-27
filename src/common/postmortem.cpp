/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   14:46
	file base:	postmortem
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "postmortem.hpp"
#if defined _MSC_VER
#include "postmortem_impl_win32.hpp"

namespace faith 
{
	postmortem::postmortem(void) : m_impl_ptr(new postmortem_impl())
	{

	}

	postmortem::~postmortem(void)
	{

	}

	bool postmortem::init( xstring dumpfile_prefix,xstring exec_afterdump, int dump_type, bool use_dump_callback )
	{
		return m_impl_ptr->init(dumpfile_prefix, exec_afterdump, dump_type, use_dump_callback);
	}

	void postmortem::release()
	{
		return m_impl_ptr->release();
	}

	void postmortem::register_extern_callback(cb_t handler, xchar * desc)
	{
		m_impl_ptr->register_extern_callback(handler, desc);
	}
}

#elif defined __GNUC__
namespace faith 
{
	class postmortem_impl{};

	postmortem::postmortem(void)
		:m_impl_ptr(0)
	{

	}

	postmortem::~postmortem(void)
	{

	}

	bool postmortem::init( xstring dumpfile_prefix,xstring exec_afterdump, int dump_type, bool use_dump_callback )
	{
		return true;
	}

	void postmortem::release()
	{
		return;
	}


}// end of namespace faith
#else

# error "Unknown compiler, only support for msvc and gcc."

#endif

