/* Unix/Cygwin network code for Xconq kernel.
   Copyright (C) 1996, 1997, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Note that this file does not include all Xconq .h files, since
   it may be used with auxiliary programs. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "lisp.h"
#include "module.h"
#include "system.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#if (defined (UNIX) || defined (MAC) || defined (__CYGWIN32__) || defined (__MINGW32__))
#include <unistd.h>
#endif

#if (defined (UNIX) || defined (__CYGWIN32__) || defined (__MINGW32__)) 
#include <sys/file.h>
#include <sys/stat.h>
#endif

#if (defined (UNIX) || defined (__CYGWIN32__) || defined (MAC))
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#if (defined (UNIX) || defined (__CYGWIN32__))
#include <sys/ioctl.h>
#include <pwd.h>
#endif

#if (defined (UNIX) || defined (MAC))
# include <netinet/tcp.h>
#endif

#if defined(_MSC_VER)
#include <io.h>
#endif

#if (defined (WIN32) && !defined (__CYGWIN32__) && !defined(__MWERKS__))

#include <winsock2.h>

/* Needed by MS Visual C. */
static unsigned int sleep(unsigned int seconds)
{
	Sleep(seconds * 1000);
	return 0;
}

#endif

#if (defined (WIN32) && !defined (__CYGWIN32__))

#define ECONNREFUSED WSAECONNREFUSED
 
static int wsa_initialized = FALSE;

/* Initialize WinSock requesting Version 2.0 */
static int wsa_open ()
{
  WORD wsaVersion;
  WSADATA wsaData;
          
  wsaVersion = MAKEWORD (2, 0);
  return WSAStartup (wsaVersion, &wsaData);	
}

/* Close WinSock */
static int wsa_close ()
{
  return WSACleanup ();
}

#endif /* WIN32 */

/* Handle deficiencies of strict ANSI compliance. */
#ifndef O_RDONLY
#define O_RDONLY 0
#endif
#ifndef O_WRONLY
#define O_WRONLY 1
#endif
#ifndef O_RDWR
#define O_RDWR 2
#endif

static int accept_remote_connection(void);
static int wait_for(int timeout);

/* The file descriptor that a host makes available for other programs
   to connect into. */

int public_fd;

/* Flag indicating whether the public file descriptor is currently
   valid. */

int public_fd_valid;

/* The next id for a remote program. 0 means invalid, 1 is reserved for
   the first host of a game, so we start at 2. */

int nextrid = 2;

/* Array of file descriptors used with connections to remote programs,
   indexed by the remote programs' ids. */

int remote_fd[100];

int fd_valid[100];

namespace Xconq {
    int host_is_localhost = FALSE;
}

/* Given a specification of how to open a remote connection (such as a
   hostname:port), and a host/join flag, set up a connection.  Return
   a number for the method being used, or 0 for failure. */

int
open_remote_connection(char *methodname, int willhost)
{
    int port, i, tmp_fd, tmp;
    char *port_str, hostname[100];
    struct hostent *hostent;
    struct sockaddr_in sockaddr;
    struct protoent *protoent;

    if (!strchr(methodname, ':')) {
	/* Try opening a serial port. */
#if 0
	struct sgttyb sg;
#endif
	remote_fd[2] = open(methodname, O_RDWR);
	fd_valid[2] = TRUE;
	if (remote_fd[2] < 0) {
		init_warning("Could not open remote device");
		return 0;
	}
#if 0
	ioctl(remote_fd[2], TIOCGETP, &sg);
	sg.sg_flags = RAW;
	ioctl(remote_fd[2], TIOCSETP, &sg);
#endif
    } else {
#if (defined (WIN32) && !defined (__CYGWIN32__))
      /* Initialize winsock if neccessary. Testing is superfluous since this
         seems to be called once only. Anyway we probably want some initialize
         and shutdown hooks either in the network (socket.c) or the os glue
         (unix.c and friends) implementation. */
      if (!wsa_initialized)
      {
        if (wsa_open () != 0)
        {
          init_warning("Can't initialize winsock");
          return 0;
        }
        wsa_initialized = TRUE;
      }
#endif
	/* Using TCP; extract the port number. */
	port_str = strchr(methodname, ':');
	port = atoi(port_str + 1);

	tmp_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (tmp_fd < 0) {
		init_warning("Can't open socket");
		return 0;
	}
	/* Allow rapid reuse of this port. */
	tmp = 1;
	setsockopt (tmp_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp,
		    sizeof(tmp));

	if (willhost) {
	    sockaddr.sin_family = PF_INET;
	    sockaddr.sin_port = htons(port);
	    sockaddr.sin_addr.s_addr = INADDR_ANY;

	    public_fd = tmp_fd;
	    public_fd_valid = TRUE;

	    if (bind(public_fd, (struct sockaddr *) &sockaddr,
		     sizeof(sockaddr))) {
		init_warning("Can't bind address");
		return 0;
	    }
	    if (listen (public_fd, 1)) {
		init_warning("Can't listen");
		return 0;
	    }
	    /* Our public socket is now available to receive
	       connection attempts.  These will appear as input on
	       public_fd, and then accept_remote_connection will
	       finish making the link. */

	    return 1;
	} else {
	    /* We're trying to join an existing game. */
	    tmp = min (port_str - methodname, (int) sizeof hostname - 1);
	    strncpy (hostname, methodname, tmp);
	    hostname[tmp] = '\0';
	    if (!strncmp(hostname, "localhost", 9))
	      Xconq::host_is_localhost = TRUE;
	    hostent = gethostbyname (hostname);
	    if (!hostent) {
		init_warning("%s: unknown host", hostname);
		return 0;
	    }
	    /* Try making the connection several times. */
	    for (i = 1; i <= 5; i++) {
		tmp_fd = socket(PF_INET, SOCK_STREAM, 0);
		if (tmp_fd < 0) {
		    init_warning("can't create a socket");
		    return 0;
		}

		/* Allow rapid reuse of this port. */
		tmp = 1;
		setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp,
			   sizeof(tmp));

		/* Enable TCP keep alive process. */
		tmp = 1;
		setsockopt(tmp_fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &tmp,
			   sizeof(tmp));

		sockaddr.sin_family = PF_INET;
		sockaddr.sin_port = htons(port);
		memcpy(&sockaddr.sin_addr.s_addr, hostent->h_addr,
		       sizeof (struct in_addr));

		if (connect(tmp_fd, (struct sockaddr *) &sockaddr,
			    sizeof(sockaddr)) == 0)
		  /* Success! Break out of the retry loop. */
		  break;

		close(tmp_fd);
		tmp_fd = -1;

		/* We retry for ECONNREFUSED because that is often a
		   temporary condition, which happens when the server
		   is being restarted.  */
		if (errno != ECONNREFUSED)
		  return 0;
		/* Wait a moment before trying again. */
		/* (should mention to the user) */
		sleep(1);
	    }

	    /* We know the host will be rid 1. */
	    remote_fd[1] = tmp_fd;
	    fd_valid[1] = TRUE;

	    protoent = getprotobyname ("tcp");
	    if (!protoent)
	      return 0;

	    tmp = 1;
	    if (setsockopt(remote_fd[1], protoent->p_proto, TCP_NODELAY,
			   (char *) &tmp, sizeof(tmp)))
	      return 0;
#ifdef SIGPIPE
	    signal(SIGPIPE, SIG_IGN);
#endif
	}
	return 1;
    }
    return 0;
}

/* If a program has attempted to connect to our published socket, accept
   the connection and add the program to our merry little band. */

static int
accept_remote_connection(void)
{
    struct hostent *hostent = NULL;
    struct sockaddr_in sockaddr;
/* (probably _MSC_VER needs to be in the conditional below. If so, then 
   we should rewrite as:
      (defined (WIN32))
 */
#if (defined (WIN32) && (defined (__MWERKS__) || defined (__MINGW32__) || defined (__CYGWIN__)))
    int tmp;
#else
    unsigned int tmp;
#endif
    char hostname[100];
    struct protoent *protoent;
    int new_fd;
    int rid;

    tmp = sizeof (sockaddr);
    new_fd = accept(public_fd, (struct sockaddr *) &sockaddr, &tmp);
    if (new_fd == -1) {
	init_warning("Accept failed");
	return 0;
    }
    /* Record the new file descriptor as a valid remote program. */
    rid = nextrid++;
    remote_fd[rid] = new_fd;
    fd_valid[rid] = TRUE;

    /* This takes more than one minute on the mac and always
     * fails, so we skip it to speed up things. */

#ifndef MAC
    hostent = gethostbyaddr((char*)&sockaddr.sin_addr,
			    sizeof(sizeof(sockaddr.sin_addr)), 
			    AF_INET);
#endif

    if (hostent) {
	memcpy(hostname, hostent->h_name, hostent->h_length);
	hostname[hostent->h_length] = '\0';
    } else {
	strcpy(hostname, "unknown");
    }

    protoent = getprotobyname("tcp");
    if (!protoent)
      init_warning("getprotobyname");

    /* Enable TCP keep alive process. */
    tmp = 1;
    setsockopt(remote_fd[rid], SOL_SOCKET, SO_KEEPALIVE,
	       (char *) &tmp, sizeof(tmp));

    /* Tell TCP not to delay small packets.  This greatly speeds up
       interactive response. */
    tmp = 1;
    setsockopt(remote_fd[rid], protoent->p_proto, TCP_NODELAY,
	       (char *) &tmp, sizeof(tmp));

    /* If we don't do this, then program simply exits when the remote
       side dies.  */
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
#if defined(FASYNC)
    fcntl(remote_fd[rid], F_SETFL, FASYNC);
#endif

    Dprintf("Remote connection from %s (#%d)\n", hostname, rid);
    return 1;
}

/* Send a given string out to a given program.  Return when the whole
   string has been delivered. */

void
low_send(int rid, char *buf)
{
    int fd, len, cc;

    fd = (rid > 0 ? remote_fd[rid] : remote_fd[1]);
    Dprintf("low_send %d (fd %d) \"%s\"\n", rid, fd, buf);
    len = strlen(buf);
    while (len > 0) {
#if (defined (WIN32) && !defined (__CYGWIN32__))
	cc = send(fd, buf, len, 0);
#else
	cc = write(fd, buf, len);
#endif /* WIN32 */

	if (cc < 0)
	  return;
	len -= cc;
	buf += cc;
    }
}

/* Wait for input, for a given period of time.  Return an rid or a
   special code (negative values). */

static int
wait_for(int timeout)
{
    int rid, numfds, maxfd;
    struct timeval tv;
    fd_set readfds, exceptfds;

    FD_ZERO (&readfds);
    FD_ZERO (&exceptfds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    maxfd = -2;
    if (public_fd_valid) {
	FD_SET(public_fd, &readfds);
	maxfd = max(public_fd, maxfd);
    }
    for (rid = 1; rid < nextrid; ++rid) {
	if (fd_valid[rid]) {
	    FD_SET(remote_fd[rid], &readfds);
	    FD_SET(remote_fd[rid], &exceptfds);
	    maxfd = max(remote_fd[rid], maxfd);
	}
    }
    /* (should test if maxfd < 0 still) */

    while (1) {
	numfds = select(maxfd+1, &readfds, 0, &exceptfds,
			(timeout >= 0 ? &tv : 0));

	if (numfds <= 0) {
	    if (numfds == 0)
	      return -1;
	    else if (errno == EINTR)
	      continue;
	    else
	      return -2;	/* Got an error from select or poll */
	}
	if (public_fd_valid) {
	    if (FD_ISSET(public_fd, &readfds)) {
		return 0;
	    }
	}
	for (rid = 1; rid < nextrid; ++rid) {
	    if (fd_valid[rid]) {
		if (FD_ISSET(remote_fd[rid], &readfds)) {
		    return rid;
		}
	    }
	}
	/* (should never happen) */
	return -1;
    }
}

/* For a given amount of time, listen for any data from our other
   programs. */

int
low_receive(int *ridp, char *buf, int maxchars, int timeout)
{
    int status, rid, fd, n;
    time_t start_time, now;

    time(&start_time);

    while (1) {
	status = wait_for(timeout);
	if (status < 0) {
	    return FALSE;
	} else if (status == 0) {
	    accept_remote_connection();
	    continue;
	}
	/* We have data from one of the other programs. */
	rid = status;
	fd = remote_fd[rid] ;
#if (defined (WIN32) && !defined (__CYGWIN32__))
	n = recv(fd, buf, maxchars, 0);
#else
	n = read(fd, buf, maxchars);
#endif /* WIN32 */

	if (n > 0) {
	    /* Make a string out of the received data. */
	    buf[n] = '\0';
	    *ridp = rid;
	    return TRUE;
	} else if (timeout == 0) {
	    /* We were just sniffing for anything laying around, and
	       there wasn't anything, so return emptyhanded. */
	    return FALSE;
	} else if (timeout == -1) {
	    /* Go around again. */
	} else {
	    time(&now);
	    if (now > start_time + timeout) {
		Dprintf("%ul > %ul + %d\n", now, start_time, timeout);
		return FALSE /* timed out */;
	    }
	}
    }
}

void
close_remote_connection(int rid)
{
    /* If we're closing before rids assigned, assume that an arg of 0
       means we were trying to join and failed. */
    if (rid == 0)
      rid = 1;

    if (!fd_valid[rid])
      return;

    close(remote_fd[rid]);
    fd_valid[rid] = FALSE;
}
