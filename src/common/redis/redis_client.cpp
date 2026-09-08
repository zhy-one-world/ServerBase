#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <BaseTsd.h>
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef SSIZE_T ssize_t;
#endif
#endif

#include <cstdint>
#include <iterator>

#include <sw/redis++/redis++.h>

#include "redis/redis_client.hpp"

namespace faith
{
	redis_client& redis_client::getInstance()
	{
		static redis_client instance;
		return instance;
	}

	redis_client::~redis_client()
	{
		disconnect();
	}

	bool redis_client::connect(const redis_options& options)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		try
		{
			sw::redis::ConnectionOptions conn;
			conn.host = options.host;
			conn.port = options.port;
			conn.db = options.db;
			if (!options.password.empty())
			{
				conn.password = options.password;
			}
			conn.connect_timeout = std::chrono::milliseconds(options.connect_timeout_ms);
			conn.socket_timeout = std::chrono::milliseconds(options.socket_timeout_ms);

			auto redis = std::make_unique<sw::redis::Redis>(conn);
			redis->ping();
			m_redis = std::move(redis);
			return true;
		}
		catch (const sw::redis::Error&)
		{
			m_redis.reset();
			return false;
		}
	}

	void redis_client::disconnect()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_redis.reset();
	}

	bool redis_client::ping()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_redis)
		{
			return false;
		}
		try
		{
			m_redis->ping();
			return true;
		}
		catch (const sw::redis::Error&)
		{
			return false;
		}
	}

	bool redis_client::is_connected() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_redis != nullptr;
	}

	bool redis_client::set(const std::string& key, const std::string& value, std::int64_t ttl_ms, bool nx)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_redis)
		{
			return false;
		}
		try
		{
			if (ttl_ms > 0)
			{
				const auto ttl = std::chrono::milliseconds(ttl_ms);
				if (nx)
				{
					return m_redis->set(key, value, ttl, sw::redis::UpdateType::NOT_EXIST);
				}
				m_redis->set(key, value, ttl);
				return true;
			}
			if (nx)
			{
				return m_redis->setnx(key, value);
			}
			m_redis->set(key, value);
			return true;
		}
		catch (const sw::redis::Error&)
		{
			return false;
		}
	}

	std::optional<std::string> redis_client::get(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_redis)
		{
			return std::nullopt;
		}
		try
		{
			const auto value = m_redis->get(key);
			if (!value)
			{
				return std::nullopt;
			}
			return *value;
		}
		catch (const sw::redis::Error&)
		{
			return std::nullopt;
		}
	}

	bool redis_client::del(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_redis)
		{
			return false;
		}
		try
		{
			return m_redis->del(key) > 0;
		}
		catch (const sw::redis::Error&)
		{
			return false;
		}
	}

	bool redis_client::exists(const std::string& key)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_redis)
		{
			return false;
		}
		try
		{
			return m_redis->exists(key) > 0;
		}
		catch (const sw::redis::Error&)
		{
			return false;
		}
	}

	bool redis_client::expire(const std::string& key, std::int64_t ttl_sec)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_redis)
		{
			return false;
		}
		try
		{
			return m_redis->expire(key, std::chrono::seconds(ttl_sec));
		}
		catch (const sw::redis::Error&)
		{
			return false;
		}
	}

	bool redis_client::hset(const std::string& key, const std::string& field, const std::string& value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_redis)
		{
			return false;
		}
		try
		{
			m_redis->hset(key, field, value);
			return true;
		}
		catch (const sw::redis::Error&)
		{
			return false;
		}
	}

	std::optional<std::string> redis_client::hget(const std::string& key, const std::string& field)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_redis)
		{
			return std::nullopt;
		}
		try
		{
			const auto value = m_redis->hget(key, field);
			if (!value)
			{
				return std::nullopt;
			}
			return *value;
		}
		catch (const sw::redis::Error&)
		{
			return std::nullopt;
		}
	}

	bool redis_client::hgetall(const std::string& key, std::map<std::string, std::string>& out)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		out.clear();
		if (!m_redis)
		{
			return false;
		}
		try
		{
			m_redis->hgetall(key, std::inserter(out, out.end()));
			return true;
		}
		catch (const sw::redis::Error&)
		{
			return false;
		}
	}

	bool redis_client::scan(const std::string& pattern, std::vector<std::string>& out, long long count)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		out.clear();
		if (!m_redis)
		{
			return false;
		}
		try
		{
			long long cursor = 0;
			do
			{
				cursor = m_redis->scan(cursor, pattern, count, std::back_inserter(out));
			} while (cursor != 0);
			return true;
		}
		catch (const sw::redis::Error&)
		{
			return false;
		}
	}
}
