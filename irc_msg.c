#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "irc_msg.h"

IrcMsg *irc_msg_new(char *s)
{
	if(!s || !*s)
		return NULL;

	fprintf(stderr, "<<%s\n", s);

	IrcMsg *msg = malloc(sizeof(IrcMsg));
	memset(msg, 0, sizeof(IrcMsg));

	if(*s == ':')
	{
		msg->source = ++s;

		while(*s && *s != ' ')
			++s;

		while(*s == ' ')
			*s++ = '\0';

		if(!*s)
		{
			free(msg);
			return NULL;
		}
	}

	char *start, *end;
	start = end = s;

	for(;;)
	{
		while(*end && *end != ' ')
			++end;

		if(*start == ':' || msg->nargs == MAX_ARGS)
		{
			msg->args[msg->nargs++] = ++start;
			break;
		}

		if(!*end)
		{
			if(start < end)
				msg->args[msg->nargs++] = start;
			break;
		}

		while(*end == ' ')
			*end++ = '\0';

		msg->args[msg->nargs++] = start;
		
		start = end;
	}

	return msg;
}

void irc_msg_delete(IrcMsg *msg)
{
	if(msg)
	{
		free(msg);
	}
}

