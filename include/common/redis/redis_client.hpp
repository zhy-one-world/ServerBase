#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace sw::redis
{
	class Redis;
}

namespace faith
{
	struct redis_options
	{
		std::string host = "127.0.0.1";
		int port = 6379;
		int db = 0;
		std::string password;
		int connect_timeout_ms = 1000;
		int socket_timeout_ms = 1000;
	};

	class redis_client
	{
	public:
		redis_client() = default;
		~redis_client();

		redis_client(const redis_client&) = delete;
		redis_client& operator=(const redis_client&) = delete;

		static redis_client& getInstance();

		bool connect(const redis_options& options);
		void disconnect();
		bool ping();
		bool is_connected() const;

		bool set(const std::string& key, const std::string& value, std::int64_t ttl_ms = 0, bool nx = false);
		std::optional<std::string> get(const std::string& key);
		bool del(const std::string& key);
		bool exists(const std::string& key);
		bool expire(const std::string& key, std::int64_t ttl_sec);

		bool hset(const std::string& key, const std::string& field, const std::string& value);
		std::optional<std::string> hget(const std::string& key, const std::string& field);
		bool hgetall(const std::string& key, std::map<std::string, std::string>& out);

		bool scan(const std::string& pattern, std::vector<std::string>& out, long long count = 100);

	private:
		mutable std::mutex m_mutex;
		std::unique_ptr<sw::redis::Redis> m_redis;
	};
}
