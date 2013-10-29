#ifndef IRC_H
#define IRC_H

#include "sock.h"
#include "irc_msg.h"

extern Sock *IrcSock;

// Initiate connection to remote server
void irc_connect();

// Read from remote server and process messages
void irc_poll();

#endif // not IRC_H
