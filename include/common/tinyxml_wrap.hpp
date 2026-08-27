#ifndef _TINYXML_WRAP_HPP_
#define _TINYXML_WRAP_HPP_

#include "xchar.hpp"
#include "tinyxml/tinyxml.h"

namespace faith
{
	class tixmldocument_wrap
	{
	public:
		bool load_file(xstring filename);
		bool loadfile_without_mlb(const xchar* filename);
		void parse(const xchar* p);

		TiXmlDocument& document() { return impl; }
		const TiXmlDocument& document() const { return impl; }
	public:
		TiXmlDocument impl;
	};

	class tixmlelement_wrap
	{
	public:
		explicit tixmlelement_wrap(TiXmlElement* element = nullptr) : impl(element) {}
		const xstring attribute(const xchar* name) const;
		const xstring attribute(const xchar* name, int* i) const;
		const xstring attribute(const xchar* name, double* d) const;
		const xstring value() const;
		const xstring get_text() const;
		const char* get_text_ansi() const;
	public:
		TiXmlElement* impl;
	};
}

#endif
