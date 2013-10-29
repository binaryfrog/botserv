#ifndef IRC_MSG_H
#define IRC_MSG_H

#define MAX_ARGS		32

typedef struct _IrcMsg IrcMsg;

struct _IrcMsg
{
	char *source;
	char *args[MAX_ARGS];
	int nargs;
};

IrcMsg *irc_msg_new(char *s);
void irc_msg_delete(IrcMsg *msg);

#endif /* not IRC_MSG_H */
