#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bots.h"
#include "conf.h"
#include "irc.h"

/* Maximum line length in bot database file */
#define BUF_SIZE	1024

GSList *Bots;

static int FileLine;

/* Processes a single line from the bot database file */
static void bots_parse_line(char *buf);

/*
 * Reads the bot database from disk
 */
void bots_read_db()
{
	FILE *fp = fopen(ConfBotsDatabase, "r");

	if(fp == NULL)
	{
		fprintf(stderr, "Error opening %s for reading: %s\n",
				ConfBotsDatabase, strerror(errno));
		exit(1);
	}

	char buf[BUF_SIZE];
	FileLine = 0;

	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		++FileLine;

		if(strlen(buf))
		{
			if(buf[strlen(buf) - 1] == '\n')
				buf[strlen(buf) - 1] = '\0';

			bots_parse_line(buf);
		}
	}

	fclose(fp);
}

/*
 * Writes the bot database to disk
 */
void bots_write_db()
{
	FILE *fp = fopen(ConfBotsDatabase, "w");

	if(fp == NULL)
	{
		fprintf(stderr, "Error opening %s for writing: %s\n",
				ConfBotsDatabase, strerror(errno));
		exit(1);
	}
}

/*
 * Processes a single line from the bot database
 * Line format is:
 * <nickname>:<channel>[,<channel>]...
 */
static void bots_parse_line(char *buf)
{
	if(*buf == ';')
		return;

	Bot *bot = malloc(sizeof(Bot));
	memset(bot, 0, sizeof(Bot));

	bot->nickname = g_strdup(buf);
	
	buf = bot->nickname;
	while(*buf && *buf != ':')
		++buf;

	if(!*buf)
	{
		fprintf(stderr, "%s:%d: missing ':'\n",
				ConfBotsDatabase, FileLine);
		free(bot->nickname);
		free(bot);
		return;
	}

	for(;;)
	{
		while(*buf == ':')
			*buf++ = '\0';

		if(!*buf)
			break;

		bot->channels = g_slist_append(bot->channels, buf);

		while(*buf && *buf != ':')
			++buf;
	}

	if(!*bot->nickname)
	{
		fprintf(stderr, "%s:%d: Nickname is unset\n",
				ConfBotsDatabase, FileLine);
		free(bot->nickname);
		free(bot);
		return;
	}

	Bots = g_slist_append(Bots, bot);
}

