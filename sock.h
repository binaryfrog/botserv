#ifndef SOCK_H
#define SOCK_H

typedef struct _Sock Sock;

Sock *sock_new();
void sock_delete(Sock *sock);

int sock_open(Sock *sock, const char *hostname, int port);
int sock_close(Sock *sock);

int sock_poll(Sock *sock);
size_t sock_read(Sock *sock);
int sock_write(Sock *sock, char *buf, size_t buf_size);
int sock_writef(Sock *sock, const char *format, ...);

char *sock_getline(Sock *sock);

#endif /* not SOCK_H */
