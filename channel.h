#ifndef CHANNEL_H
#define CHANNEL_H

#include <glib.h>

#include "irc_types.h"

void channel_init();

Channel *channel_new(const char *name);
void channel_introduce(Channel *channel);

inline Channel *channel_find(const char *name);
inline GList *channel_find_all();

void channel_clear_modes(Channel *channel);
void channel_clear_topic(Channel *channel);

void channel_push_user(Channel *channel, User *user);
void channel_pop_user(Channel *channel, User *user);
void channel_pop_user_from_all(User *user);

gboolean channel_change_mode(Channel *channel, char mode, const char *arg,
		gboolean add);

inline void channel_set_user_modes(Channel *channel, User *user,
		ChannelUserModes modes);
inline ChannelUserModes channel_get_user_modes(Channel *channel, User *user);

#endif // not CHANNEL_H
