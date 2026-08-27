#ifndef __MLBMODE_HEADER_FILE__
#define __MLBMODE_HEADER_FILE__

#include "singleton.hpp"
//#include "MoonlightBoxLogger.hpp"
//#include "MoonlightBoxRecurer.hpp"
#include <xchar.hpp>
namespace faith{
	namespace common{

		enum MLB_MODE
		{
			MM_NORMAL,
			MM_LOGGING,
			MM_RECURRENCE
		};
		class MoonlightBoxBase;

		class MlbMode:
			public singleton<MlbMode>
		{
			friend class singleton<MlbMode>;
			friend class MoonlightBoxLogger;
			friend class MoonlightBoxRecurer;
		public:
			bool in_logging_mode() const;
			bool in_recurrence_mode() const;
			bool moonlightbox_enabled() const;
			//MoonlightBoxBase * const get_moonlightbox() const;
			MoonlightBoxLogger & get_logger() const;
			MoonlightBoxRecurer & get_recurer() const;
			void set_logger(MoonlightBoxLogger* logger);
			void set_recurer(MoonlightBoxRecurer* recurer);
			void set_mode(MLB_MODE mode);
		private:
			MlbMode();
			
			MLB_MODE m_mode;
			MoonlightBoxLogger*		m_logger;
			MoonlightBoxRecurer*	m_recurer;
		};
	}
}

#endif
