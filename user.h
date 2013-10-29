#ifndef USER_H
#define USER_H

#include <glib.h>

#include "irc_types.h"

void user_init();

User *user_new(const char *nickname);
void user_delete(User *user);
inline User *user_find(const char *nickname);
inline GList *user_find_all();
void user_introduce(User *user);
void user_set_nickname(User *user, const char *nickname);
void user_change_mode(User *user, char mode, gboolean add);

#endif // not USER_H
