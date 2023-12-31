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

#include "mbconfig.h"           /* This goes first in every *.c,*.cc file */
#include "dataman.h"
#include "sockdrv.h"

#ifdef USE_SOCKET

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

#ifdef _WIN32
  /* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501  /* Windows XP. */
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <errno.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <netdb.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sys/un.h>
  #include <arpa/inet.h>
#endif /* _WIN32 */

#include "sock.h"

const size_t USERLEN = 32;
const size_t CREDLEN = 128;
const size_t BUFSIZE = 1024;
const char *MBDynSocketDrivePath = "/var/mbdyn/mbdyn.sock";

SocketDrive::SocketDrive(unsigned int uL, const DriveHandler* pDH,
	unsigned short int p, AuthMethod* a,
	integer nd, const std::vector<doublereal>& v0)
: FileDrive(uL, pDH, "socket", nd, v0),
type(AF_INET),
auth(a),
pFlags(NULL)
{
	int			save_errno;
	
   	ASSERT(p > 0);
   	ASSERT(auth != NULL);
   	ASSERT(nd > 0);

   	/* Create the socket and set it up to accept connections. */
	data.Port = p;
   	int serr = mbdyn_make_inet_socket(&sock, 0, NULL, data.Port, 1, &save_errno);
   	if (serr == -1) {
		const char	*err_msg = strerror(save_errno);

      		silent_cerr("SocketDrive(" << GetLabel()
			<< "): socket failed "
			"(" << save_errno << ": "<< err_msg << ")"
			<< std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);

   	} else if (serr == -2) {
		const char	*err_msg = strerror(save_errno);

      		silent_cerr("SocketDrive(" << GetLabel()
			<< "): bind failed "
			"(" << save_errno << ": "<< err_msg << ")"
			<< std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
   	}

   	Init();
}

#ifndef _WIN32
SocketDrive::SocketDrive(unsigned int uL, const DriveHandler* pDH,
	const char *path,
	integer nd, const std::vector<doublereal>& v0)
: FileDrive(uL, pDH, "socket", nd, v0),
type(AF_LOCAL),
auth(NULL),
pFlags(NULL)
{
	int save_errno;
	int rc;

   	ASSERT(path != NULL);
   	ASSERT(nd > 0);

	SAFENEW(auth, NoAuth);

   	/* Create the socket and set it up to accept connections. */
	SAFESTRDUP(data.Path, path);
   	rc = mbdyn_make_named_socket(&sock, 0, data.Path, 1, &save_errno);
   	if (rc == -1) {
		const char	*err_msg = strerror(save_errno);

      		silent_cerr("SocketDrive(" << GetLabel()
			<< "): socket(\"" << data.Path << "\") failed "
			"(" << save_errno << ": "<< err_msg << ")"
			<< std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);

   	} else if (rc == -2) {
		const char	*err_msg = strerror(save_errno);

      		silent_cerr("SocketDrive(" << GetLabel()
			<< "): bind(\"" << data.Path << "\") failed "
			"(" << save_errno << ": "<< err_msg << ")"
			<< std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);

   	} else if (rc < 0) {
		const char	*err_msg = strerror(save_errno);

      		silent_cerr("SocketDrive(" << GetLabel()
			<< "): mbdyn_make_named_socket(\"" << data.Path << "\") failed "
			"(" << save_errno << ": "<< err_msg << ")"
			<< std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
   	}

	Init();
}
#endif /* ! _WIN32 */

void
SocketDrive::Init(void)
{
   	/* non-blocking */
#ifdef _WIN32
    bool blocking = false;
    unsigned long mode = blocking ? 0 : 1;
   	int result = ioctlsocket(sock, FIONBIO, &mode);

   	if (result != 0) {
		silent_cerr("SocketDrive(" << GetLabel()
				<< ": unable to set socket to non-blocking mode"
				<< std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
   	}
#else
   	int oldflags = fcntl(sock, F_GETFL, 0);
   	if (oldflags == -1) {
		silent_cerr("SocketDrive(" << GetLabel()
				<< ": unable to get socket flags"
				<< std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
   	}
   	oldflags |= O_NONBLOCK;
   	if (fcntl(sock, F_SETFL, oldflags) == -1) {
		silent_cerr("SocketDrive(" << GetLabel()
				<< ": unable to set socket flags"
				<< std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
   	}
#endif

   	if (listen(sock, 1) < 0) {
      		silent_cerr("SocketDrive(" << GetLabel()
			<< "): listen failed" << std::endl);
      		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
   	}

   	SAFENEWARR(pFlags, int, iNumDrives + 1);
   	for (int iCnt = 0; iCnt <= iNumDrives; iCnt++) {
      		pFlags[iCnt] = SocketDrive::DEFAULT;
   	}
}


SocketDrive::~SocketDrive(void)
{
   	/* some shutdown stuff ... */
#ifdef _WIN32
   	shutdown(sock, SD_BOTH /* 2 */ );
#else
   	shutdown(sock, SHUT_RDWR /* 2 */ );
#endif /* _WIN32 */

	switch (type) {
#ifndef _WIN32
	case AF_LOCAL:
		if (data.Path) {
			unlink(data.Path);
			SAFEDELETEARR(data.Path);
		}
		break;
#endif /* _WIN32 */
	default:
		NO_OP;
		break;
	}

   	if (auth != NULL) {
      		SAFEDELETE(auth);
   	}

        SAFEDELETEARR(pFlags);
}

static char *
get_line(char *buf, size_t bufsize,  FILE *fd)
{
   	int len;

   	if (fgets(buf, bufsize, fd) == NULL) {
      		return NULL;
  	}

   	len = strlen(buf);
   	if (len > 0 && buf[len-1] == '\n') {
      		buf[len-1] = '\0';
      		if (len > 1 && buf[len-2] == '\r') {
	 		buf[len-2] = '\0';
      		}
   	} else {
      		fprintf(stderr, "buffer overflow\n");
      		return NULL;
   	}

   	return buf;
}

/*
 * in/out:
 *     user : puntatore a buffer di 9 bytes (8+'\0')
 *     cred : puntatore a buffer di 129 bytes (128+'\0')
 *
 * out:
 *     buf : puntatore a buffer statico che contiene una copia della nuova
 *           linea nel caso sia stata accidentalmente letta
 */
int
get_auth_token(FILE *fd, char *user, char *cred, char **nextline)
{
   	char buf[BUFSIZE];

   	if (get_line(buf, BUFSIZE, fd) == NULL) {
      		return -1;
   	}

   	user[0] = '\0';
   	cred[0] = '\0';

   	if (strncasecmp(buf, "user:", 5) != 0) {
      		*nextline = (char *)malloc(sizeof(char)*(strlen(buf)+1));
		if ((*nextline) == NULL) {
	 		return -1;
      		}
      		strcpy(*nextline, buf);
      		return 0;
   	} else {
      		char *p;
      		unsigned int i;

      		p = buf+5;
      		while (isspace(*p)) {
	 		p++;
      		}
      		for (i = 0; i < USERLEN; i++) {
	 		if ((user[i] = p[i]) == '\0' || isspace(user[i])) {
	    			break;
	 		}
      		}
      		user[i] = '\0';
   	}

   	if (get_line(buf, BUFSIZE, fd) == NULL) {
      		return -1;
   	}

   	if (strncasecmp(buf, "password:", 9) != 0) {
      		*nextline = (char *)malloc(sizeof(char)*(strlen(buf)+1));
		if ((*nextline) == NULL) {
	 		return -1;
      		}
      		strcpy(*nextline, buf);
      		return 0;
   	} else {
      		char *p;
      		unsigned int i;

      		p = buf+9;
      		while (isspace(*p)) {
	 		p++;
      		}
      		for (i = 0; i < CREDLEN; i++) {
	 		if ((cred[i] = p[i]) == '\0' || isspace(cred[i])) {
	    			break;
	 		}
      		}
      		cred[i] = '\0';
   	}

   	return 1;
}

void
SocketDrive::ServePending(const doublereal& /* t */ )
{
   	struct sockaddr_in client_name;
        socklen_t socklen = sizeof(client_name);

   	/* prova */
   	for (integer iCnt = 1; iCnt <= iNumDrives; iCnt++) {
		if (pFlags[iCnt] == SocketDrive::IMPULSIVE) {
	 		pdVal[iCnt] = 0.;
      		}
   	}

   	while (true) {
      		int got_value = 0;
      		char *nextline = NULL;
      		const size_t bufsize = BUFSIZE;
      		char buf[bufsize];

      		int label;
      		doublereal value;

   		int cur_sock = accept(sock,
				  (struct sockaddr *)&client_name,
				  &socklen);

      		if (cur_sock == -1) {
			int save_errno = errno;
	 		if (save_errno != EWOULDBLOCK) {
	    			silent_cerr(
					"SocketDrive(" << GetLabel() << "): "
					"accept failed "
					"(" << save_errno << ": "
					<< strerror(save_errno) << ")"
					<< std::endl);
	 		}
	 		return;
      		}

      		silent_cout("SocketDrive(" << GetLabel() << "): "
			"connect from " << inet_ntoa(client_name.sin_addr)
		  	<< ":" << ntohs(client_name.sin_port) << std::endl);

		bool bAuthc = false;
#ifdef HAVE_SASL2
		if (dynamic_cast<SASL2_Auth*>(auth)) {
     	 		if (auth->Auth(cur_sock) != AuthMethod::AUTH_OK) {
		 		silent_cerr(
					"SocketDrive(" << GetLabel() << "): "
					"authentication failed" << std::endl);
		 		continue;
      			}
			bAuthc = true;
		}
#endif /* HAVE_SASL2 */
		
   		FILE* fd = fdopen(cur_sock, "r");

		if (!bAuthc) {
      			char user[USERLEN + 1];
      			char cred[CREDLEN + 1];

      			if (get_auth_token(fd, user, cred, &nextline) == -1) {
	 			silent_cerr(
					"SocketDrive(" << GetLabel() << "): "
					"corrupted stream" << std::endl);
	 			fclose(fd);
	 			continue;
      			}

      			DEBUGCOUT("got auth token: user=\"" << user
				<< "\", cred=\"" << cred << "\"" << std::endl);

      			if (auth->Auth(user, cred) != AuthMethod::AUTH_OK) {
	 			silent_cerr(
					"SocketDrive(" << GetLabel() << "): "
					"authentication failed" << std::endl);
	 			fclose(fd);
	 			continue;
      			}
		}

      		DEBUGCOUT("authenticated" << std::endl);

      		/*
		 * la nuova linea puo' essere gia' stata letta
		 * da get_auth_token
		 */
      		if (nextline == NULL) {
	 		if (get_line(buf, bufsize, fd) == NULL) {
	    			silent_cerr(
					"SocketDrive(" << GetLabel() << "): "
					"corrupted stream" << std::endl);
	    			fclose(fd);
	    			continue;
	 		}
      		} else {
			size_t l_nextline = strlen(nextline);
			if (l_nextline >= bufsize) {
	 			silent_cerr(
					"SocketDrive(" << GetLabel() << "): "
					"nextline too long for buffer" << std::endl);
	 			fclose(fd);
	 			continue;
			}
	 		strcpy(buf, nextline);
	 		free(nextline);
      		}
      		nextline = buf;

      		/* legge la label */
      		if (strncasecmp(nextline, "label:", STRLENOF("label:")) != 0) {
	 		silent_cerr("SocketDrive(" << GetLabel() << "): "
				"missing label" << std::endl);
	 		fclose(fd);
	 		continue;
      		}

	 	char *p = nextline + STRLENOF("label:");
	 	while (isspace(p[0])) {
	    		p++;
	 	}

	 	if (sscanf(p, "%d", &label) != 1) {
	    		silent_cerr("SocketDrive(" << GetLabel() << "): "
				"unable to read label" << std::endl);
	    		fclose(fd);
	    		continue;
	 	}

	 	if (label <= 0 || label > iNumDrives) {
	    		silent_cerr("SocketDrive(" << GetLabel() << "): "
				"illegal label " << label << std::endl);
	    		fclose(fd);
	    		continue;
	 	}

	 	while (true) {
	    		if (get_line(buf, bufsize, fd) == NULL) {
	       			silent_cerr(
					"SocketDrive(" << GetLabel() << "): "
					"corrupted stream" << std::endl);
	       			fclose(fd);
	       			break;
	    		}

	    		nextline = buf;

	    		if (nextline[0] == '.') {
	       			fclose(fd);
	       			break;
	    		}

	    		if (strncasecmp(nextline, "value:", STRLENOF("value:")) == 0) {
	       			char *p = nextline + STRLENOF("value:");
	       			while (isspace(p[0])) {
	 				p++;
				}

	       			if (sscanf(p, "%lf", &value) != 1) {
	  				silent_cerr("SocketDrive(" << GetLabel() << "): "
						"unable to read value"
						<< std::endl);
	  				fclose(fd);
	  				break;
				}
				got_value = 1;

    			} else if (strncasecmp(nextline, "inc:", STRLENOF("inc:")) == 0) {
       				char *p = nextline + STRLENOF("inc:");
       				while (isspace(p[0])) {
	  				p++;
       				}

       				if (strncasecmp(p, "yes", STRLENOF("yes")) == 0) {
					pFlags[label] = SocketDrive::INCREMENTAL;

				} else if (strncasecmp(p, "no", STRLENOF("no")) == 0) {
					pFlags[label] = SocketDrive::DEFAULT;

				} else {
					silent_cerr("SocketDrive(" << GetLabel() << "): "
						"\"inc\" line in "
						"\"" << nextline << "\" "
						"looks corrupted"
						<< std::endl);
					fclose(fd);
					break;
				}
				nextline = NULL;

			} else if (strncasecmp(nextline, "imp:", STRLENOF("imp:")) == 0) {
				char *p = nextline + STRLENOF("imp:");
				while (isspace(p[0])) {
					p++;
				}

				if (strncasecmp(p, "yes", STRLENOF("yes")) == 0) {
					pFlags[label] = SocketDrive::IMPULSIVE;

				} else if (strncasecmp(p, "no", STRLENOF("no")) == 0) {
					pFlags[label] = SocketDrive::DEFAULT;

				} else {
					silent_cerr("SocketDrive(" << GetLabel() << "): "
						"\"imp\" line" " in "
						"\"" << nextline << "\""
						" looks corrupted"
						<< std::endl);
					fclose(fd);
					break;
				}
				nextline = NULL;
			}

			/* usa i valori */
	 		if (got_value) {
				if (pFlags[label] == SocketDrive::INCREMENTAL) {
					silent_cout("SocketDrive(" << GetLabel() << "): "
						"adding " << value
						<< " to label " << label
						<< std::endl);
					pdVal[label] += value;

				} else {
					silent_cout("SocketDrive(" << GetLabel() << "): "
						"setting label " << label
						<< " to value " << value
						<< std::endl);
					pdVal[label] = value;
				}
	 		}
		}
	}
}

/* Scrive il contributo del DriveCaller al file di restart */
std::ostream&
SocketDrive::Restart(std::ostream& out) const
{
	return out << "SocketDrive not implemented yet" << std::endl;
}

/* legge i drivers tipo socket */

Drive *
SocketDR::Read(unsigned uLabel, const DataManager *pDM, MBDynParser& HP)
{
	Drive* pDr = NULL;

	integer idrives = HP.GetInt();
	unsigned short int port = MBDynSocketDrivePort;
	const char *path = NULL;

	std::vector<doublereal> v0;
	if (HP.IsKeyWord("initial" "values")) {
		v0.resize(idrives);
		for (integer i = 0; i < idrives; i++) {
			v0[i] = HP.GetReal();
		}
	}

	if (HP.IsKeyWord("local")) {
#ifdef _WIN32
     silent_cerr("SocketDrive(" << uLabel << "): "
            "local sockets are not supported on Windows, you must use inet "
            "at line " << HP.GetLineData()
            << std::endl);
        throw ErrGeneric(MBDYN_EXCEPT_ARGS);
#else /* _WIN32 */
		path = HP.GetFileName();
		ASSERT(path != NULL);
#endif /* _WIN32 */
	} else if (HP.IsKeyWord("port")) {
		port = HP.GetInt();
#ifdef IPPORT_USERRESERVED
		if (port < IPPORT_USERRESERVED) {
			silent_cerr("SocketDrive(" << uLabel << "): "
					"cannot listen on port " << port
					<< ": less than IPPORT_USERRESERVED=" 
					<< IPPORT_USERRESERVED
					<< " at line " << HP.GetLineData()
					<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
		/* if #undef'd, don't bother checking;
		 * the OS will do it for us */
#endif /* IPPORT_USERRESERVED */
	}

	if (path == NULL) {
		AuthMethod* pAuth = ReadAuthMethod(pDM, HP);

		if (pAuth == NULL) {
			silent_cerr("need an authentication method "
				"at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		SAFENEWWITHCONSTRUCTOR(pDr,
			SocketDrive,
			SocketDrive(uLabel, pDM->pGetDrvHdl(),
				port, pAuth, idrives, v0));
#ifndef _WIN32
	} else {
		SAFENEWWITHCONSTRUCTOR(pDr,
			SocketDrive,
			SocketDrive(uLabel, pDM->pGetDrvHdl(),
				path, idrives, v0));
#else

#endif /* _WIN32 */
	}

	return pDr;
}

#endif /* USE_SOCKET */
