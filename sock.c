#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>

#include <glib.h>

#include "sock.h"

#define RBUF_SIZE		4096
#define WBUF_SIZE		4096
#define WRITEF_BUF_SIZE		1024

struct _Sock
{
	int fd;
	struct sockaddr_in dest_addr;
	char rbuf[RBUF_SIZE];
	size_t rbuf_used;
	char wbuf[WBUF_SIZE];
	size_t wbuf_used;
};

Sock *sock_new()
{
	Sock *sock = malloc(sizeof(Sock));
	memset(sock, 0, sizeof(Sock));
	sock->fd = -1;
	return sock;
}

void sock_delete(Sock *sock)
{
	if(sock)
	{
		if(sock->fd >= 0)
			sock_close(sock);

		free(sock);
	}
}

int sock_open(Sock *sock, const char *hostname, int port)
{
	struct hostent *hent;

	hent = gethostbyname(hostname);
	if(hent == NULL)
	{
		fprintf(stderr, "%s: Host error:  %s\n",
				hostname, hstrerror(h_errno));
		return -1;
	}

	sock->fd = socket(PF_INET, SOCK_STREAM, 0);
	if(sock->fd == -1)
	{
		fprintf(stderr, "Socket error: %s\n", strerror(errno));
		return -1;
	}

	sock->dest_addr.sin_family = AF_INET;
	sock->dest_addr.sin_port = htons(port);
	memcpy(&sock->dest_addr.sin_addr, hent->h_addr_list[0],
			sizeof(struct in_addr));

	if(connect(sock->fd, (struct sockaddr *) &sock->dest_addr,
				sizeof(struct sockaddr_in)) != 0)
	{
		fprintf(stderr, "Connect error: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int sock_close(Sock *sock)
{
	if(sock->fd >= 0)
	{
		int result = close(sock->fd);
		sock->fd = -1;
		return result;
	}

	return -1;
}

int sock_poll(Sock *sock)
{
	struct pollfd pollset = { sock->fd, POLLIN | POLLOUT, 0 };
	int result = poll(&pollset, 1, 100);

	if(result == -1)
	{
		fprintf(stderr, "Poll error: %s\n", strerror(errno));
		exit(1);
	}

	if(result == 1)
	{
		if(pollset.revents & POLLHUP || pollset.revents & POLLERR)
		{
			fprintf(stderr, "Socket error: %s:\n",
					strerror(errno));
			exit(1);
		}

		if(sock->wbuf_used && pollset.revents & POLLOUT)
		{
			result = write(sock->fd, sock->wbuf, sock->wbuf_used);

			if(result <= 0)
			{
				fprintf(stderr, "Socket write error: %s\n",
						strerror(errno));
				exit(1);
			}

			if(result < sock->wbuf_used)
			{
				memmove(sock->wbuf, sock->wbuf + result,
						sock->wbuf_used - result);
			}

			sock->wbuf_used -= result;
		}

		return pollset.revents & POLLIN;
	}

	return 0;
}

size_t sock_read(Sock *sock)
{
	if(sock->rbuf_used == RBUF_SIZE)
		return RBUF_SIZE;

	int result = read(sock->fd, sock->rbuf + sock->rbuf_used,
			RBUF_SIZE - sock->rbuf_used);
	if(result < 0)
	{
		fprintf(stderr, "Socket read error: %s\n", strerror(errno));
		exit(1);
	} else if(result == 0)
	{
		fprintf(stderr, "Connection reset by peer\n");
		exit(1);
	}

	sock->rbuf_used += result;

	return sock->rbuf_used;
}

int sock_write(Sock *sock, char *buf, size_t buf_size)
{
	if(WBUF_SIZE - sock->wbuf_used < buf_size)
	{
		fprintf(stderr, "Socket write buffer overflow: %ld > %ld\n",
				buf_size, WBUF_SIZE - sock->wbuf_used);
		abort();
	}

	memcpy(sock->wbuf + sock->wbuf_used, buf, buf_size);
	sock->wbuf_used += buf_size;

	return 0;
}

int sock_writef(Sock *sock, const char *format, ...)
{
	char buf[WRITEF_BUF_SIZE];
	va_list args;

	va_start(args, format);
	vsnprintf(buf, WRITEF_BUF_SIZE, format, args);
	va_end(args);

	fprintf(stderr, ">>%s", buf);

	return sock_write(sock, buf, strlen(buf));
}

char *sock_getline(Sock *sock)
{
	if(sock->rbuf_used == 0)
		return NULL;

	char *ptr = memchr(sock->rbuf, '\n', sock->rbuf_used);
	if(ptr)
	{
		if(ptr != sock->rbuf && ptr[-1] == '\r')
			--ptr;

		if(ptr == sock->rbuf)
		{
			while(ptr < sock->rbuf + sock->rbuf_used &&
					(*ptr == '\r' || *ptr == '\n'))
				++ptr;
			memmove(sock->rbuf, ptr,
					sock->rbuf_used - (ptr - sock->rbuf));
			sock->rbuf_used -= (ptr - sock->rbuf);
			return sock_getline(sock);
		}

		char *line = malloc((ptr - sock->rbuf) + 1);
		memcpy(line, sock->rbuf, ptr - sock->rbuf);
		line[ptr - sock->rbuf] = '\0';

		while(ptr < sock->rbuf + sock->rbuf_used &&
				(*ptr == '\r' || *ptr == '\n'))
			++ptr;
		memmove(sock->rbuf, ptr, sock->rbuf_used - (ptr - sock->rbuf));
		sock->rbuf_used -= (ptr - sock->rbuf);

		return line;
	}

	return NULL;
}

