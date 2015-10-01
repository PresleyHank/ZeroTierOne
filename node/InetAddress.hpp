/*
 * ZeroTier One - Network Virtualization Everywhere
 * Copyright (C) 2011-2015  ZeroTier, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --
 *
 * ZeroTier may be used and distributed under the terms of the GPLv3, which
 * are available at: http://www.gnu.org/licenses/gpl-3.0.html
 *
 * If you would like to embed ZeroTier into a commercial application or
 * redistribute it in a modified binary form, please contact ZeroTier Networks
 * LLC. Start here: http://www.zerotier.com/
 */

#ifndef ZT_INETADDRESS_HPP
#define ZT_INETADDRESS_HPP

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <string>

#include "Constants.hpp"
#include "../include/ZeroTierOne.h"
#include "Utils.hpp"
#include "MAC.hpp"
#include "Buffer.hpp"

namespace ZeroTier {

/**
 * Extends sockaddr_storage with friendly C++ methods
 *
 * This is basically a "mixin" for sockaddr_storage. It adds methods and
 * operators, but does not modify the structure. This can be cast to/from
 * sockaddr_storage and used interchangeably. Don't change this as it's
 * used in a few places.
 */
struct InetAddress : public sockaddr_storage
{
	/**
	 * Loopback IPv4 address (no port)
	 */
	static const InetAddress LO4;

	/**
	 * Loopback IPV6 address (no port)
	 */
	static const InetAddress LO6;

	/**
	 * IP address scope
	 *
	 * Note that these values are in ascending order of path preference and
	 * MUST remain that way or Path must be changed to reflect.
	 */
	enum IpScope
	{
		IP_SCOPE_NONE = 0,          // NULL or not an IP address
		IP_SCOPE_MULTICAST = 1,     // 224.0.0.0 and other V4/V6 multicast IPs
		IP_SCOPE_LOOPBACK = 2,      // 127.0.0.1, ::1, etc.
		IP_SCOPE_PSEUDOPRIVATE = 3, // 28.x.x.x, etc. -- unofficially unrouted IPv4 blocks often "bogarted"
		IP_SCOPE_GLOBAL = 4,        // globally routable IP address (all others)
		IP_SCOPE_LINK_LOCAL = 5,    // 169.254.x.x, IPv6 LL
		IP_SCOPE_SHARED = 6,        // 100.64.0.0/10, shared space for e.g. carrier-grade NAT
		IP_SCOPE_PRIVATE = 7        // 10.x.x.x, 192.168.x.x, etc.
	};

	InetAddress() throw() { memset(this,0,sizeof(InetAddress)); }
	InetAddress(const InetAddress &a) throw() { memcpy(this,&a,sizeof(InetAddress)); }
	InetAddress(const InetAddress *a) throw() { memcpy(this,a,sizeof(InetAddress)); }
	InetAddress(const struct sockaddr_storage &ss) throw() { *this = ss; }
	InetAddress(const struct sockaddr_storage *ss) throw() { *this = ss; }
	InetAddress(const struct sockaddr &sa) throw() { *this = sa; }
	InetAddress(const struct sockaddr *sa) throw() { *this = sa; }
	InetAddress(const struct sockaddr_in &sa) throw() { *this = sa; }
	InetAddress(const struct sockaddr_in *sa) throw() { *this = sa; }
	InetAddress(const struct sockaddr_in6 &sa) throw() { *this = sa; }
	InetAddress(const struct sockaddr_in6 *sa) throw() { *this = sa; }
	InetAddress(const void *ipBytes,unsigned int ipLen,unsigned int port) throw() { this->set(ipBytes,ipLen,port); }
	InetAddress(const uint32_t ipv4,unsigned int port) throw() { this->set(&ipv4,4,port); }
	InetAddress(const std::string &ip,unsigned int port) throw() { this->set(ip,port); }
	InetAddress(const std::string &ipSlashPort) throw() { this->fromString(ipSlashPort); }
	InetAddress(const char *ipSlashPort) throw() { this->fromString(std::string(ipSlashPort)); }

	inline InetAddress &operator=(const InetAddress &a)
		throw()
	{
		memcpy(this,&a,sizeof(InetAddress));
		return *this;
	}

	inline InetAddress &operator=(const InetAddress *a)
		throw()
	{
		memcpy(this,a,sizeof(InetAddress));
		return *this;
	}

	inline InetAddress &operator=(const struct sockaddr_storage &ss)
		throw()
	{
		memcpy(this,&ss,sizeof(InetAddress));
		return *this;
	}

	inline InetAddress &operator=(const struct sockaddr_storage *ss)
		throw()
	{
		memcpy(this,ss,sizeof(InetAddress));
		return *this;
	}

	inline InetAddress &operator=(const struct sockaddr_in &sa)
		throw()
	{
		memset(this,0,sizeof(InetAddress));
		memcpy(this,&sa,sizeof(struct sockaddr_in));
		return *this;
	}

	inline InetAddress &operator=(const struct sockaddr_in *sa)
		throw()
	{
		memset(this,0,sizeof(InetAddress));
		memcpy(this,sa,sizeof(struct sockaddr_in));
		return *this;
	}

	inline InetAddress &operator=(const struct sockaddr_in6 &sa)
		throw()
	{
		memset(this,0,sizeof(InetAddress));
		memcpy(this,&sa,sizeof(struct sockaddr_in6));
		return *this;
	}

	inline InetAddress &operator=(const struct sockaddr_in6 *sa)
		throw()
	{
		memset(this,0,sizeof(InetAddress));
		memcpy(this,sa,sizeof(struct sockaddr_in6));
		return *this;
	}

	inline InetAddress &operator=(const struct sockaddr &sa)
		throw()
	{
		memset(this,0,sizeof(InetAddress));
		switch(sa.sa_family) {
			case AF_INET:
				memcpy(this,&sa,sizeof(struct sockaddr_in));
				break;
			case AF_INET6:
				memcpy(this,&sa,sizeof(struct sockaddr_in6));
				break;
		}
		return *this;
	}

	inline InetAddress &operator=(const struct sockaddr *sa)
		throw()
	{
		memset(this,0,sizeof(InetAddress));
		switch(sa->sa_family) {
			case AF_INET:
				memcpy(this,sa,sizeof(struct sockaddr_in));
				break;
			case AF_INET6:
				memcpy(this,sa,sizeof(struct sockaddr_in6));
				break;
		}
		return *this;
	}

	/**
	 * @return IP scope classification (e.g. loopback, link-local, private, global)
	 */
	IpScope ipScope() const
		throw();

	/**
	 * Set from a string-format IP and a port
	 *
	 * @param ip IP address in V4 or V6 ASCII notation
	 * @param port Port or 0 for none
	 */
	void set(const std::string &ip,unsigned int port)
		throw();

	/**
	 * Set from a raw IP and port number
	 *
	 * @param ipBytes Bytes of IP address in network byte order
	 * @param ipLen Length of IP address: 4 or 16
	 * @param port Port number or 0 for none
	 */
	void set(const void *ipBytes,unsigned int ipLen,unsigned int port)
		throw();

	/**
	 * Set the port component
	 *
	 * @param port Port, 0 to 65535
	 */
	inline void setPort(unsigned int port)
		throw()
	{
		switch(ss_family) {
			case AF_INET:
				reinterpret_cast<struct sockaddr_in *>(this)->sin_port = Utils::hton((uint16_t)port);
				break;
			case AF_INET6:
				reinterpret_cast<struct sockaddr_in6 *>(this)->sin6_port = Utils::hton((uint16_t)port);
				break;
		}
	}

	/**
	 * @return ASCII IP/port format representation
	 */
	std::string toString() const;

	/**
	 * @return IP portion only, in ASCII string format
	 */
	std::string toIpString() const;

	/**
	 * @param ipSlashPort ASCII IP/port format notation
	 */
	void fromString(const std::string &ipSlashPort);

	/**
	 * @return Port or 0 if no port component defined
	 */
	inline unsigned int port() const
		throw()
	{
		switch(ss_family) {
			case AF_INET: return Utils::ntoh((uint16_t)(reinterpret_cast<const struct sockaddr_in *>(this)->sin_port));
			case AF_INET6: return Utils::ntoh((uint16_t)(reinterpret_cast<const struct sockaddr_in6 *>(this)->sin6_port));
			default: return 0;
		}
	}

	/**
	 * Alias for port()
	 *
	 * This just aliases port() to make code more readable when netmask bits
	 * are stuffed there, as they are in Network, EthernetTap, and a few other
	 * spots.
	 *
	 * @return Netmask bits
	 */
	inline unsigned int netmaskBits() const throw() { return port(); }

	/**
	 * Alias for port()
	 *
	 * This just aliases port() because for gateways we use this field to
	 * store the gateway metric.
	 *
	 * @return Gateway metric
	 */
	inline unsigned int metric() const throw() { return port(); }

	/**
	 * Construct a full netmask as an InetAddress
	 */
	InetAddress netmask() const
		throw();

	/**
	 * Constructs a broadcast address from a network/netmask address
	 *
	 * @return Broadcast address (only IP portion is meaningful)
	 */
	InetAddress broadcast() const
		throw();

	/**
	 * @return True if this is an IPv4 address
	 */
	inline bool isV4() const throw() { return (ss_family == AF_INET); }

	/**
	 * @return True if this is an IPv6 address
	 */
	inline bool isV6() const throw() { return (ss_family == AF_INET6); }

	/**
	 * @return pointer to raw IP address bytes
	 */
	inline const void *rawIpData() const
		throw()
	{
		switch(ss_family) {
			case AF_INET: return (const void *)&(reinterpret_cast<const struct sockaddr_in *>(this)->sin_addr.s_addr);
			case AF_INET6: return (const void *)(reinterpret_cast<const struct sockaddr_in6 *>(this)->sin6_addr.s6_addr);
			default: return 0;
		}
	}

	/**
	 * @return pointer to raw IP address bytes
	 */
	inline void *rawIpData()
		throw()
	{
		switch(ss_family) {
			case AF_INET: return (void *)&(reinterpret_cast<struct sockaddr_in *>(this)->sin_addr.s_addr);
			case AF_INET6: return (void *)(reinterpret_cast<struct sockaddr_in6 *>(this)->sin6_addr.s6_addr);
			default: return 0;
		}
	}

	/**
	 * @param a InetAddress to compare again
	 * @return True if only IP portions are equal (false for non-IP or null addresses)
	 */
	inline bool ipsEqual(const InetAddress &a) const
	{
		switch(ss_family) {
			case AF_INET: return (reinterpret_cast<const struct sockaddr_in *>(this)->sin_addr.s_addr == reinterpret_cast<const struct sockaddr_in *>(&a)->sin_addr.s_addr);
			case AF_INET6: return (memcmp(reinterpret_cast<const struct sockaddr_in6 *>(this)->sin6_addr.s6_addr,reinterpret_cast<const struct sockaddr_in6 *>(&a)->sin6_addr.s6_addr,16) == 0);
		}
		return false;
	}

	/**
	 * Set to null/zero
	 */
	inline void zero() throw() { memset(this,0,sizeof(InetAddress)); }

	/**
	 * Check whether this is a network/route rather than an IP assignment
	 *
	 * A network is an IP/netmask where everything after the netmask is
	 * zero e.g. 10.0.0.0/8.
	 *
	 * @return True if everything after netmask bits is zero
	 */
	bool isNetwork() const
		throw();

	/**
	 * @return True if address family is non-zero
	 */
	inline operator bool() const throw() { return (ss_family != 0); }

	template<unsigned int C>
	inline void serialize(Buffer<C> &b) const
	{
		// Format is the same as in VERB_HELLO in Packet.hpp
		switch(ss_family) {
			case AF_INET:
				b.append((uint8_t)0x04);
				b.append(&(reinterpret_cast<const struct sockaddr_in *>(this)->sin_addr.s_addr),4);
				b.append((uint16_t)port()); // just in case sin_port != uint16_t
				return;
			case AF_INET6:
				b.append((uint8_t)0x06);
				b.append(reinterpret_cast<const struct sockaddr_in6 *>(this)->sin6_addr.s6_addr,16);
				b.append((uint16_t)port()); // just in case sin_port != uint16_t
				return;
			default:
				b.append((uint8_t)0);
				return;
		}
	}

	template<unsigned int C>
	inline unsigned int deserialize(const Buffer<C> &b,unsigned int startAt = 0)
	{
		unsigned int p = startAt;
		memset(this,0,sizeof(InetAddress));
		switch(b[p++]) {
			case 0:
				return 1;
			case 0x04:
				ss_family = AF_INET;
				memcpy(&(reinterpret_cast<struct sockaddr_in *>(this)->sin_addr.s_addr),b.field(p,4),4); p += 4;
				reinterpret_cast<struct sockaddr_in *>(this)->sin_port = Utils::hton(b.template at<uint16_t>(p)); p += 2;
				break;
			case 0x06:
				ss_family = AF_INET6;
				memcpy(reinterpret_cast<struct sockaddr_in6 *>(this)->sin6_addr.s6_addr,b.field(p,16),16); p += 16;
				reinterpret_cast<struct sockaddr_in *>(this)->sin_port = Utils::hton(b.template at<uint16_t>(p)); p += 2;
				break;
			default:
				throw std::invalid_argument("invalid serialized InetAddress");
		}
		return (p - startAt);
	}

	bool operator==(const InetAddress &a) const throw();
	bool operator<(const InetAddress &a) const throw();
	inline bool operator!=(const InetAddress &a) const throw() { return !(*this == a); }
	inline bool operator>(const InetAddress &a) const throw() { return (a < *this); }
	inline bool operator<=(const InetAddress &a) const throw() { return !(a < *this); }
	inline bool operator>=(const InetAddress &a) const throw() { return !(*this < a); }

	/**
	 * @param mac MAC address seed
	 * @return IPv6 link-local address
	 */
	static InetAddress makeIpv6LinkLocal(const MAC &mac)
		throw();

	/**
	 * Compute private IPv6 unicast address from network ID and ZeroTier address
	 *
	 * This generates a private unicast IPv6 address that is mostly compliant
	 * with the letter of RFC4193 and certainly compliant in spirit.
	 *
	 * RFC4193 specifies a format of:
	 *
	 * | 7 bits |1|  40 bits   |  16 bits  |          64 bits           |
	 * | Prefix |L| Global ID  | Subnet ID |        Interface ID        |
	 *
	 * The 'L' bit is set to 1, yielding an address beginning with 0xfd. Then
	 * the network ID is filled into the global ID, subnet ID, and first byte
	 * of the "interface ID" field. Since the first 40 bits of the network ID
	 * is the unique ZeroTier address of its controller, this makes a very
	 * good random global ID. Since network IDs have 24 more bits, we let it
	 * overflow into the interface ID.
	 *
	 * After that we pad with two bytes: 0x99, 0x93, namely the default ZeroTier
	 * port in hex.
	 *
	 * Finally we fill the remaining 40 bits of the interface ID field with
	 * the 40-bit unique ZeroTier device ID of the network member.
	 *
	 * This yields a valid RFC4193 address with a random global ID, a
	 * meaningful subnet ID, and a unique interface ID, all mappable back onto
	 * ZeroTier space.
	 *
	 * This in turn could allow us, on networks numbered this way, to emulate
	 * IPv6 NDP and eliminate all multicast. This could be beneficial for
	 * small devices and huge networks, e.g. IoT applications.
	 *
	 * The returned address is given an odd prefix length of /88, since within
	 * a given network only the last 40 bits (device ID) are variable. This
	 * is a bit unusual but as far as we know should not cause any problems with
	 * any non-braindead IPv6 stack.
	 *
	 * @param nwid 64-bit network ID
	 * @param zeroTierAddress 40-bit device address (in least significant 40 bits, highest 24 bits ignored)
	 * @return IPv6 private unicast address with /88 netmask
	 */
	static InetAddress makeIpv6rfc4193(uint64_t nwid,uint64_t zeroTierAddress)
		throw();
};

} // namespace ZeroTier

#endif
