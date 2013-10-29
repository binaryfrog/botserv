#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

#include <glib.h>

#include "conf.h"
#include "bots.h"
#include "sock.h"
#include "irc_msg.h"
#include "irc.h"

static void print_usage(const char *command)
{
	fprintf(stderr, "Usage: %s FILE\n", command);
	fprintf(stderr, "Where FILE is the path to a configuration file.\n");
}

int main(int argc, char **argv)
{
	struct rlimit rlim = { RLIM_INFINITY, RLIM_INFINITY };
	setrlimit(RLIMIT_CORE, &rlim);

	if(argc != 2)
	{
		print_usage(argv[0]);
		return 1;
	}
	
	if(conf_read(argv[1]) != 0)
		return 1;

	bots_read_db();

	irc_connect();

	for(;;)
	{
		irc_poll();
		g_usleep(100);
	}

	return 0;
}

