#ifndef __MOONLIGHTBOXRECURER_HEADER_FILE__
#define __MOONLIGHTBOXRECURER_HEADER_FILE__

#include "singleton.hpp"
#include "MoonlightBoxBase.hpp"
#include "IMLB_Reader_Stream.hpp"

namespace faith {
	namespace common {
		class simple_binary_oarchive;
	}
}

namespace faith{
	namespace common
	{
		class MoonlightBoxRecurer:
			public MoonlightBoxBase,
			public common::singleton<MoonlightBoxRecurer>
		{
			friend class common::singleton<MoonlightBoxRecurer>;
		public:
			typedef boost::function<void (const xchar * class_name,const xchar * function_name,boost::uint64_t instance_it,boost::int32_t time,boost::uint64_t index)> on_callback_handler_type;

			MoonlightBoxRecurer();
			~MoonlightBoxRecurer();

			bool start(on_callback_handler_type on_callback_handler);
			void stop();

			IMLB_Reader_Stream * get_recurer_stream(){ return m_log_reader; };
			void set_recurer_stream(IMLB_Reader_Stream * log_reader){ m_log_reader = log_reader; };
			bool execute_callback();
			void fetch_calling_result(
				const xstring & name,
				MlbParam & result,
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

			/*获得近似的运行时时间戳
				该时间戳是在记录模式时记下的当前callback过程开始时的时间戳
			*/
			boost::int32_t get_approx_runtime_timestamp() const;

		private:

			void call(const MlbFile::CallbackName &name,const MlbParams & params);

			template <class CHUNK>
			bool read_chunk(CHUNK & chunk);

			size_t read_from(void * data_ptr, size_t size, int data_type);

			void adjust_buffer_size(boost::uint32_t size);

			on_callback_handler_type	m_on_callback_handler;
			bool						m_started;

			simple_binary_iarchive *			m_simple_binary_iarchive;
			xchar *								m_buffer;				//读取文件所用缓存
			boost::uint32_t						m_buffer_size;
			boost::int32_t						m_running_time;			//运行时时间
			boost::int32_t						m_running_time_print;	//打印运行时时间

			IMLB_Reader_Stream	*				m_log_reader;

			MlbFile::ChunkCalling	m_calling_chunk;
			MlbFile::ChunkCallback	m_callback_chunk;
		};
	}
}
#endif
