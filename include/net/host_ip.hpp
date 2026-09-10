#pragma once

namespace faith
{
	namespace net
	{
		// Returns a pointer to an internal static buffer with a preferred local IPv4.
		// Prefer non-loopback LAN address; fall back to "127.0.0.1".
		// Not thread-mutating beyond lazy init of the static buffer; safe for typical startup use.
		const char* get_host_ip();
	}
}
