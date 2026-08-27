#ifndef _FAITH_RLOG_HPP_
#define _FAITH_RLOG_HPP_

#include <cstddef>
#include <sstream>
#include <string>

namespace faith
{
	namespace rlog
	{
		enum level
		{
			MTRACE = 0,
			MDEBUG = 1,
			MINFO = 2,
			MWARN = 3,
			MERROR = 4,
			MCRITICAL = 5,
			MOFF = 6
		};

		struct options
		{
			// 日志根目录，例如 "logs"
			std::string log_dir = "logs";
			// 文件名前缀，最终形如 logs/2026-08-27/server_2026-08-27-16-00-00.log
			std::string basename = "server";
			// 单文件大小上限（字节），超出后以当前时间新建文件
			std::size_t max_file_size = 100ull * 1024ull * 1024ull;
			// 保留的历史文件数量上限；超出后删除最旧文件，并以当前时间新建
			std::size_t max_files = 24ull * 7ull;
			// 是否同时输出到控制台
			bool also_console = true;
			// 默认日志级别
			level default_level = MINFO;
		};

		// 初始化全局 logger（进程内调用一次）
		bool init(const options& opt = options());
		void shutdown();
		void set_level(level lv);
		void flush();

		bool should_log(level lv);
		void write(level lv, const std::string& msg);

		class stream
		{
		public:
			explicit stream(level lv)
				: level_(lv)
			{
			}

			~stream()
			{
				write(level_, oss_.str());
			}

			template <typename T>
			stream& operator<<(const T& value)
			{
				oss_ << value;
				return *this;
			}

			stream& operator<<(std::ostream& (*manip)(std::ostream&))
			{
				oss_ << manip;
				return *this;
			}

		private:
			level level_;
			std::ostringstream oss_;
		};
	}
}

// 用法：
// _RLOG_(MINFO, "res:" << res << " sys_id:" << sys_id);
// 输出会自动带上函数名与行号，例如：
// [func=Foo::Bar line=128] res:0 sys_id:1
#ifndef _RLOG_
#define _RLOG_(LEVEL, MSG)                                                                 \
	do                                                                                     \
	{                                                                                      \
		if (::faith::rlog::should_log(::faith::rlog::LEVEL))                               \
		{                                                                                  \
			::faith::rlog::stream _faith_rlog_stream_(::faith::rlog::LEVEL);               \
			_faith_rlog_stream_ << "[func:" << __FUNCTION__                                \
				<< " line:" << __LINE__ << "] " << MSG;                                    \
		}                                                                                  \
	} while (0)
#endif

#endif
