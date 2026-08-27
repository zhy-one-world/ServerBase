/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:24
	file base:	persistence_id_generator
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _PERSISTENCE_ID_GENERATOR_H_
#define _PERSISTENCE_ID_GENERATOR_H_

#include "singleton.hpp"
#include "unique_id_generator.hpp"
#include <boost/cstdint.hpp>
#include <map>
#include "xchar.hpp"

namespace faith
{
	class persistence_id_generator : public faith::singleton<persistence_id_generator>
	{
		friend class faith::singleton<persistence_id_generator>;
		typedef unique_id_generator<boost::uint32_t>	id_generator;
		typedef std::map< xstring, id_generator >		id_generator_map;
	private:
		persistence_id_generator();
	public:
		~persistence_id_generator();
	public:
		boost::uint32_t									get_id(const xchar* category);
		void											return_id(const xchar* category, boost::uint32_t id);
	private:
		static const boost::uint32_t					s_invalid_id = 0xCCCCCCCC;
		id_generator_map								m_id_generator_map;
	};
}
	
#endif
