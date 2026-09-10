#include "host_ip.hpp"

#include <cstdint>
#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace faith
{
	namespace net
	{
		namespace
		{
			constexpr const char* k_loopback = "127.0.0.1";

			char* host_ip_buffer()
			{
				static char ip[INET_ADDRSTRLEN] = { 0 };
				return ip;
			}

			bool is_usable_v4(const in_addr& addr)
			{
				const std::uint32_t host = ntohl(addr.s_addr);
				if (host == 0)
				{
					return false;
				}
				if ((host & 0xFF000000u) == 0x7F000000u) // 127.0.0.0/8
				{
					return false;
				}
				if ((host & 0xFFFF0000u) == 0xA9FE0000u) // 169.254.0.0/16 link-local
				{
					return false;
				}
				return true;
			}

			bool store_v4(const in_addr& addr, char* out, std::size_t out_size)
			{
#ifdef _WIN32
				return InetNtopA(AF_INET, &addr, out, static_cast<ULONG>(out_size)) != nullptr;
#else
				return inet_ntop(AF_INET, &addr, out, out_size) != nullptr;
#endif
			}

#ifdef _WIN32
			bool ensure_wsa()
			{
				static bool ready = false;
				static bool ok = false;
				if (!ready)
				{
					WSADATA wsa;
					ok = (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
					ready = true;
				}
				return ok;
			}

			bool fill_from_adapters(char* out, std::size_t out_size)
			{
				ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
				ULONG size = 0;
				DWORD ret = GetAdaptersAddresses(AF_INET, flags, nullptr, nullptr, &size);
				if (ret != ERROR_BUFFER_OVERFLOW || size == 0)
				{
					return false;
				}

				auto* buffer = static_cast<IP_ADAPTER_ADDRESSES*>(operator new(size));
				ret = GetAdaptersAddresses(AF_INET, flags, nullptr, buffer, &size);
				if (ret != NO_ERROR)
				{
					operator delete(buffer);
					return false;
				}

				bool found = false;
				for (auto* adapter = buffer; adapter != nullptr && !found; adapter = adapter->Next)
				{
					if (adapter->OperStatus != IfOperStatusUp)
					{
						continue;
					}
					if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
					{
						continue;
					}
					for (auto* unicast = adapter->FirstUnicastAddress;
						unicast != nullptr;
						unicast = unicast->Next)
					{
						if (unicast->Address.lpSockaddr == nullptr ||
							unicast->Address.lpSockaddr->sa_family != AF_INET)
						{
							continue;
						}
						auto* sin = reinterpret_cast<sockaddr_in*>(unicast->Address.lpSockaddr);
						if (!is_usable_v4(sin->sin_addr))
						{
							continue;
						}
						if (store_v4(sin->sin_addr, out, out_size))
						{
							found = true;
							break;
						}
					}
				}

				operator delete(buffer);
				return found;
			}
#else
			bool fill_from_ifaddrs(char* out, std::size_t out_size)
			{
				ifaddrs* ifaddr = nullptr;
				if (getifaddrs(&ifaddr) != 0 || ifaddr == nullptr)
				{
					return false;
				}

				bool found = false;
				for (ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
				{
					if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_INET)
					{
						continue;
					}
					auto* sin = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
					if (!is_usable_v4(sin->sin_addr))
					{
						continue;
					}
					if (store_v4(sin->sin_addr, out, out_size))
					{
						found = true;
						break;
					}
				}

				freeifaddrs(ifaddr);
				return found;
			}
#endif

			bool fill_from_hostname(char* out, std::size_t out_size)
			{
				char hostname[256] = { 0 };
				if (gethostname(hostname, sizeof(hostname)) != 0)
				{
					return false;
				}

				addrinfo hints;
				std::memset(&hints, 0, sizeof(hints));
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;

				addrinfo* result = nullptr;
				if (getaddrinfo(hostname, nullptr, &hints, &result) != 0 || result == nullptr)
				{
					return false;
				}

				bool found = false;
				in_addr fallback{};
				bool has_fallback = false;
				for (addrinfo* item = result; item != nullptr; item = item->ai_next)
				{
					if (item->ai_addr == nullptr || item->ai_family != AF_INET)
					{
						continue;
					}
					auto* sin = reinterpret_cast<sockaddr_in*>(item->ai_addr);
					if (!has_fallback)
					{
						fallback = sin->sin_addr;
						has_fallback = true;
					}
					if (!is_usable_v4(sin->sin_addr))
					{
						continue;
					}
					if (store_v4(sin->sin_addr, out, out_size))
					{
						found = true;
						break;
					}
				}

				if (!found && has_fallback)
				{
					found = store_v4(fallback, out, out_size);
				}

				freeaddrinfo(result);
				return found;
			}
		}

		const char* get_host_ip()
		{
			char* out = host_ip_buffer();
			if (out[0] != '\0')
			{
				return out;
			}

#ifdef _WIN32
			ensure_wsa();
			if (!fill_from_adapters(out, INET_ADDRSTRLEN) &&
				!fill_from_hostname(out, INET_ADDRSTRLEN))
			{
				std::strncpy(out, k_loopback, INET_ADDRSTRLEN - 1);
				out[INET_ADDRSTRLEN - 1] = '\0';
			}
#else
			if (!fill_from_ifaddrs(out, INET_ADDRSTRLEN) &&
				!fill_from_hostname(out, INET_ADDRSTRLEN))
			{
				std::strncpy(out, k_loopback, INET_ADDRSTRLEN - 1);
				out[INET_ADDRSTRLEN - 1] = '\0';
			}
#endif
			return out;
		}
	}
}
