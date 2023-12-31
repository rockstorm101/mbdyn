/* $Header$ */
/*
 * MBDyn (C) is a multibody analysis code.
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2023
 *
 * Pierangelo Masarati	<pierangelo.masarati@polimi.it>
 * Paolo Mantegazza	<paolo.mantegazza@polimi.it>
 *
 * Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
 * via La Masa, 34 - 20156 Milano, Italy
 * http://www.aero.polimi.it
 *
 * Changing this copyright notice is forbidden.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation (version 2 of the License).
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef USESOCK_H
#define USESOCK_H
#ifdef USE_SOCKET

#ifdef _WIN32
  /* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501  /* Windows XP. */
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  /* Assume that any non-Windows platform uses POSIX-style sockets instead. */
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sys/un.h>
#endif

#include "sock.h"

class UseSocket {
protected:
	SOCKET sock;
	int socket_type;
	bool create;
	bool connected;
	bool abandoned;
	bool isblocking;

	mutable socklen_t socklen;

	void PostConnect(void);

public:
	UseSocket(bool c);
	UseSocket(int t, bool c);
	virtual ~UseSocket(void);

	virtual std::ostream& Restart(std::ostream& out) const;

	SOCKET GetSock(void) const;
	void SetSock(int s);
	virtual void Connect(void);
	virtual void ConnectSock(int s);
	bool Create(void) const;
	bool Connected(void) const;
	void Abandon(void);
	bool Abandoned(void) const;
	bool IsBlocking(void) { return isblocking; }

	socklen_t& GetSocklen(void) const;
	virtual struct sockaddr *GetSockaddr(void) const = 0;
	virtual std::string GetSockaddrStr (void) const = 0;

	ssize_t send(const void *buf, size_t len, int flags);
	ssize_t recv(void *buf, size_t len, int flags, bool bMsgDontWait);
};

class UseInetSocket : public UseSocket {
protected:
	std::string host;
 	unsigned short int port;
	struct sockaddr_in addr;

	void UseInetSocket_int(void);

public:
	UseInetSocket(const std::string& h, unsigned short p, bool c);
	UseInetSocket(const std::string& h, unsigned short p, int t, bool c);
	virtual ~UseInetSocket(void);

	std::ostream& Restart(std::ostream& out) const;

	void Connect(void);
	void ConnectSock(int s);
	struct sockaddr *GetSockaddr(void) const;
	std::string GetSockaddrStr(void) const;
};

#ifndef _WIN32
class UseLocalSocket : public UseSocket {
protected:
	std::string path;
	struct sockaddr_un addr;

	void UseLocalSocket_int(void);

public:
	UseLocalSocket(const std::string& p, bool c);
	UseLocalSocket(const std::string& p, int t, bool c);
	virtual ~UseLocalSocket(void);

	std::ostream& Restart(std::ostream& out) const;

	void Connect(void);
	void ConnectSock(int s);
	struct sockaddr *GetSockaddr(void) const;
	std::string GetSockaddrStr(void) const;
};
#endif /* _WIN32 */

#endif
#endif /* USESOCK_H */

