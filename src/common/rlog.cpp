#include "rlog.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <mutex>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace faith
{
	namespace rlog
	{
		namespace
		{
			std::mutex g_mutex;
			std::shared_ptr<spdlog::logger> g_logger;
			level g_level = MINFO;

			spdlog::level::level_enum to_spd(level lv)
			{
				switch (lv)
				{
				case MTRACE: return spdlog::level::trace;
				case MDEBUG: return spdlog::level::debug;
				case MINFO: return spdlog::level::info;
				case MWARN: return spdlog::level::warn;
				case MERROR: return spdlog::level::err;
				case MCRITICAL: return spdlog::level::critical;
				case MOFF:
				default: return spdlog::level::off;
				}
			}

			// 文件规则示例：
			//   logs/2026-08-27/server_2026-08-27-16-00-00.log
			// 切割条件：跨小时 / 超过单文件大小 / 超过保留数量
			// 每次新建文件都使用“当前时间”命名。
			template <typename Mutex>
			class hour_size_file_sink final : public spdlog::sinks::base_sink<Mutex>
			{
			public:
				hour_size_file_sink(std::string log_dir,
					std::string basename,
					std::size_t max_size,
					std::size_t max_files)
					: log_dir_(std::move(log_dir))
					, basename_(std::move(basename))
					, max_size_(max_size == 0 ? (std::numeric_limits<std::size_t>::max)() : max_size)
					, max_files_(max_files == 0 ? 1 : max_files)
					, current_size_(0)
				{
					open_new_file_(std::chrono::system_clock::now());
				}

				~hour_size_file_sink() override
				{
					if (file_.is_open())
					{
						file_.flush();
						file_.close();
					}
				}

			protected:
				void sink_it_(const spdlog::details::log_msg& msg) override
				{
					spdlog::memory_buf_t formatted;
					spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

					const auto now = std::chrono::system_clock::now();
					const bool need_rotate =
						should_rotate_by_hour_(now)
						|| (current_size_ + formatted.size() > max_size_);

					if (need_rotate)
					{
						// 跨小时或超大小：以当前时间重新生成文件
						open_new_file_(now);
					}

					file_.write(formatted.data(), static_cast<std::streamsize>(formatted.size()));
					current_size_ += formatted.size();
				}

				void flush_() override
				{
					file_.flush();
				}

			private:
				static std::tm to_local_tm_(const std::chrono::system_clock::time_point& tp)
				{
					const std::time_t t = std::chrono::system_clock::to_time_t(tp);
					std::tm tm{};
#if defined(_WIN32)
					localtime_s(&tm, &t);
#else
					localtime_r(&t, &tm);
#endif
					return tm;
				}

				bool should_rotate_by_hour_(const std::chrono::system_clock::time_point& now) const
				{
					const std::tm tm_now = to_local_tm_(now);
					return tm_now.tm_year != current_tm_.tm_year
						|| tm_now.tm_mon != current_tm_.tm_mon
						|| tm_now.tm_mday != current_tm_.tm_mday
						|| tm_now.tm_hour != current_tm_.tm_hour;
				}

				static std::string join_path_(const std::string& a, const std::string& b)
				{
					if (a.empty())
					{
						return b;
					}
					if (a.back() == '/' || a.back() == '\\')
					{
						return a + b;
					}
					return a + '/' + b;
				}

				std::string make_day_dir_(const std::tm& tm) const
				{
					char day_buf[16] = {};
					std::snprintf(day_buf, sizeof(day_buf), "%04d-%02d-%02d",
						tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
					return join_path_(log_dir_, day_buf);
				}

				std::string make_filename_(const std::tm& tm) const
				{
					// logs/2026-08-27/server_2026-08-27-16-00-00.log
					char day_buf[16] = {};
					char time_buf[32] = {};
					std::snprintf(day_buf, sizeof(day_buf), "%04d-%02d-%02d",
						tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
					std::snprintf(time_buf, sizeof(time_buf), "%04d-%02d-%02d-%02d-%02d-%02d",
						tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec);

					const std::string day_dir = join_path_(log_dir_, day_buf);
					return join_path_(day_dir, basename_ + "_" + time_buf + ".log");
				}

				void open_new_file_(const std::chrono::system_clock::time_point& now)
				{
					if (file_.is_open())
					{
						file_.flush();
						file_.close();
						remember_file_(current_filename_);
					}

					// 超过最大数量时，先腾出配额，再以当前时间重新生成文件
					while (history_.size() >= max_files_)
					{
						const std::string old = history_.front();
						history_.erase(history_.begin());
						std::error_code ec;
						std::filesystem::remove(old, ec);
					}

					current_tm_ = to_local_tm_(now);
					const std::string day_dir = make_day_dir_(current_tm_);
					std::filesystem::create_directories(day_dir);

					current_filename_ = make_filename_(current_tm_);
					// 同一秒内多次切割时追加毫秒，避免文件名冲突
					if (std::filesystem::exists(current_filename_))
					{
						const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
							now.time_since_epoch()).count() % 1000;
						char time_buf[40] = {};
						std::snprintf(time_buf, sizeof(time_buf), "%04d-%02d-%02d-%02d-%02d-%02d-%03d",
							current_tm_.tm_year + 1900, current_tm_.tm_mon + 1, current_tm_.tm_mday,
							current_tm_.tm_hour, current_tm_.tm_min, current_tm_.tm_sec,
							static_cast<int>(ms));
						current_filename_ = join_path_(day_dir, basename_ + "_" + time_buf + ".log");
					}

					file_.open(current_filename_, std::ios::app | std::ios::binary);
					if (!file_)
					{
						throw spdlog::spdlog_ex("Failed to open log file: " + current_filename_);
					}

					file_.seekp(0, std::ios::end);
					current_size_ = static_cast<std::size_t>(file_.tellp());
					if (current_size_ == static_cast<std::size_t>(-1))
					{
						current_size_ = 0;
					}
				}

				void remember_file_(const std::string& path)
				{
					if (path.empty())
					{
						return;
					}
					history_.push_back(path);
					while (history_.size() > max_files_)
					{
						const std::string old = history_.front();
						history_.erase(history_.begin());
						std::error_code ec;
						std::filesystem::remove(old, ec);
					}
				}

				std::string log_dir_;
				std::string basename_;
				std::size_t max_size_;
				std::size_t max_files_;
				std::size_t current_size_;
				std::tm current_tm_{};
				std::string current_filename_;
				std::ofstream file_;
				std::vector<std::string> history_;
			};

			using hour_size_file_sink_mt = hour_size_file_sink<std::mutex>;
		}

		bool init(const options& opt)
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			if (g_logger)
			{
				return true;
			}

			try
			{
				std::vector<spdlog::sink_ptr> sinks;
				sinks.push_back(std::make_shared<hour_size_file_sink_mt>(
					opt.log_dir, opt.basename, opt.max_file_size, opt.max_files));

				if (opt.also_console)
				{
					sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
				}

				g_logger = std::make_shared<spdlog::logger>("faith_rlog", sinks.begin(), sinks.end());
				g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
				g_logger->set_level(to_spd(opt.default_level));
				g_logger->flush_on(spdlog::level::warn);
				spdlog::set_default_logger(g_logger);
				g_level = opt.default_level;
				return true;
			}
			catch (const std::exception&)
			{
				g_logger.reset();
				return false;
			}
		}

		void shutdown()
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			if (g_logger)
			{
				g_logger->flush();
				spdlog::drop(g_logger->name());
				g_logger.reset();
			}
			spdlog::shutdown();
		}

		void set_level(level lv)
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			g_level = lv;
			if (g_logger)
			{
				g_logger->set_level(to_spd(lv));
			}
		}

		void flush()
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			if (g_logger)
			{
				g_logger->flush();
			}
		}

		bool should_log(level lv)
		{
			return lv >= MTRACE && lv < MOFF && lv >= g_level;
		}

		void write(level lv, const std::string& msg)
		{
			std::shared_ptr<spdlog::logger> logger;
			{
				std::lock_guard<std::mutex> lock(g_mutex);
				logger = g_logger;
			}
			if (!logger)
			{
				return;
			}

			switch (lv)
			{
			case MTRACE: logger->trace(msg); break;
			case MDEBUG: logger->debug(msg); break;
			case MINFO: logger->info(msg); break;
			case MWARN: logger->warn(msg); break;
			case MERROR: logger->error(msg); break;
			case MCRITICAL: logger->critical(msg); break;
			default: break;
			}
		}
	}
}
