#ifndef __MOONLIGHTBOXLOGGER_HEADER_FILE__
#define __MOONLIGHTBOXLOGGER_HEADER_FILE__

#include "singleton.hpp"
#include "MoonlightBoxBase.hpp"
#include "xchar.hpp"
#include "IMLB_Writer_Stream.hpp"

namespace faith {
	namespace common {
		class simple_binary_oarchive;
	}
}

namespace faith{
	namespace common
	{
		template <const xchar * Name,class Sig> class mlb_helper;

		class MoonlightBoxLogger:
			public common::MoonlightBoxBase,
			public common::singleton<MoonlightBoxLogger>
		{
			friend class common::singleton<MoonlightBoxLogger>;
			template <const xchar * Name, class Sig> friend class common::mlb_helper;
		public:
			MoonlightBoxLogger();
			~MoonlightBoxLogger();

			bool start();
			void stop();

			void set_logger_stream(IMLB_Writer_Stream * log_writer){ m_log_writer = log_writer; };
			void set_log_calling_params(bool log);
			enum COMPRESS_LEVEL
			{
				COMPRESS_NONE,
				COMPRESS_LVL_LOW,
				COMPRESS_LVL_HIGH
			};
			void set_compress_level(COMPRESS_LEVEL level);
			COMPRESS_LEVEL get_compress_level(){ return m_compress_level; };

			bool start_store_callback(
				const MlbFile::CallbackName &name,
				const MlbParam &param1=empty,
				const MlbParam &param2=empty,
				const MlbParam &param3=empty,
				const MlbParam &param4=empty,
				const MlbParam &param5=empty,
				const MlbParam &param6=empty,
				const MlbParam &param7=empty,
				const MlbParam &param8=empty,
				const MlbParam &param9=empty
				);
			void end_store_callback();

			bool store_calling(
				const xstring & name,
				const MlbParam &result=empty,
				const MlbParam &param1=empty,
				const MlbParam &param2=empty,
				const MlbParam &param3=empty,
				const MlbParam &param4=empty,
				const MlbParam &param5=empty,
				const MlbParam &param6=empty,
				const MlbParam &param7=empty,
				const MlbParam &param8=empty,
				const MlbParam &param9=empty
				);

		private:

			bool write_header();

			template <class CHUNK>
			bool write_chunk(const CHUNK & chunk);

			COMPRESS_LEVEL			m_compress_level;
			bool					m_storing_callback;
			MlbFile::CallbackName	m_storing_callback_name;
			bool					m_log_calling_params;
			boost::uint64_t			m_next_callback_index;
			bool					m_started;

			MlbFile::ChunkCalling	m_calling_chunk;
			MlbFile::ChunkCallback	m_callback_chunk;

			IMLB_Writer_Stream *				m_log_writer;
		};
	}
}
#endif
