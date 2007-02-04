/*
 * @file ns_connection.c
 *
 * gaim-netsoul Protocol Plugin
 *
 * Copyright (C) 2004, 2007, Edward Hervey <bilboed@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <string.h>
#include "netsoul.h"

int	netsoul_send_passwd(GaimConnection *gc)
{
  char	*seed;
  char	*res;
  char	*loc;
  char	*com;
  NetsoulData	*ns = gc->proto_data;
  GaimAccount	*account;

  account = gaim_connection_get_account(gc);
  seed = g_strdup_printf("%s-%s/%i%s", ns->challenge, ns->host,
			 ns->port, gaim_account_get_password(account));
  res = crypt_pass(seed);
  g_free(seed);
  loc = (char *) gaim_account_get_string(account,
					 "location",
					 NETSOUL_DEFAULT_LOCATION);
  com = (char *) gaim_account_get_string(account, "comment", NETSOUL_DEFAULT_COMMENT);
  seed = g_strdup_printf("ext_user_log %s %s %s %s\n", gaim_account_get_username(account), res,
			 url_encode(loc), url_encode(com));
  return (netsoul_write(ns, seed));
}

static void auth_response (gpointer data, gint source, GaimInputCondition cond)
{
  GaimConnection *gc = data;
  NetsoulData *ns = gc->proto_data;
  char	buf[1024];
  char	**tab;

  if ((read(source, buf, 1024)) < 0) {
    gaim_connection_error(gc, _("Connection Error\n"));
    return;
  }
  gaim_debug_info("netsoul", "auth_response got : %s\n", buf);
  tab = g_strsplit(buf, " ", 6);
  if (strcmp(*tab, "rep")) {
    gaim_connection_error(gc, _("Wrong Answer from server\n"));
    return;
  }
  g_strfreev(tab);
  if (netsoul_send_passwd(gc) < 0) {
    gaim_connection_error(gc, _("Error while sending password\n"));
    return;
  }
  gaim_input_remove(gc->inpa);
  ns->state = NS_STATE_SENT_EXTUSERLOG;
  gaim_debug_info("netsoul", "password sent\n");
  gc->inpa = gaim_input_add(source, GAIM_INPUT_READ, ns_listen, gc);
  gaim_connection_update_progress(gc, _("Password sent\n"), 2, 3);
  netsoul_get_buddies (gc);
}

static void netsoul_login_greeting (gpointer data, gint source, GaimInputCondition cond)
{
  GaimConnection *gc = data;
  NetsoulData *ns = gc->proto_data;
  char	buf[1024];
  char	**tab;

  if ((read(source, buf, 1024)) < 0) {
    gaim_debug_info ("netsoul", "putain!! %s %u\n", strerror (errno), GAIM_INPUT_READ & cond);
    gaim_connection_error(gc, _("Couldn't read from server"));
    return;
  }
  gaim_debug_info("netsoul", "netsoul_login_connect received: %s\n", buf);
  tab = g_strsplit(buf, " ", 6);
  if (strncmp(tab[0], "salut", 5) || !tab[4]) {
    gaim_connection_error(gc, _("Wrong greetings from server\n"));
    gaim_debug_info("netsoul", "Error on str : %s\n", buf);
    g_strfreev(tab);
    return;
  }
  ns->id = atoi(tab[1]);
  ns->challenge = strdup(tab[2]);
  ns->host = strdup(tab[3]);
  ns->port = atoi(tab[4]);
  ns->fd = source;
  if (netsoul_write(ns, "auth_ag ext_user none none\n") < 0) {
    gaim_connection_error(gc, _("Connection Error"));
    g_strfreev(tab);
    return;
  }
  gaim_input_remove (gc->inpa);
  gc->inpa = gaim_input_add(source, GAIM_INPUT_READ, auth_response, gc);
  gaim_debug_info("netsoul", "auth_ag sent, waiting for response\n");
  ns->state = NS_STATE_SENT_AUTH;
  gaim_connection_update_progress(gc, _("auth_ag sent, waiting for response\n"), 1, 3);
}

static void netsoul_login_connect (gpointer data, gint source, const gchar *error)
{
  GaimConnection *gc = data;

  gaim_debug_info ("netsoul", "netsoul_login_connect starts %u\n", source);
  if (source < 0 || !g_list_find(gaim_connections_get_all(), gc)) {
    gaim_connection_error(gc, _("Couldn't connect to host"));
    close(source);
    gaim_debug_info("netsoul", "error \"%s\"", error);
    return;
  }
  gc->inpa = gaim_input_add(source, GAIM_INPUT_READ, netsoul_login_greeting, gc);
}

void netsoul_login (GaimAccount *account)
{
  GaimConnection *gc = gaim_account_get_connection(account);
  NetsoulData *ns = gc->proto_data = g_new0(NetsoulData, 1);
  GaimProxyConnectData *err;

  gaim_debug_info("netsoul", "netsoul_login\n");

  ns->account = account;
  gaim_debug_info ("netsoul", "Connection on %s on port %u\n",
		   gaim_account_get_string(account, "server", NETSOUL_DEFAULT_SERVER),
		   gaim_account_get_int(account, "port", NETSOUL_DEFAULT_PORT));
  err = gaim_proxy_connect(NULL, account, gaim_account_get_string(account, "server", NETSOUL_DEFAULT_SERVER),
			   gaim_account_get_int(account, "port", NETSOUL_DEFAULT_PORT),
			   netsoul_login_connect, gc);
  if (err == NULL || !gaim_account_get_connection (account)) {
    gaim_connection_error(gc, _("Couldn't create connection"));
    return;
  }

  gaim_connection_update_progress(gc, _("Connecting"), 0, 3);
  ns->state = NS_STATE_CONNECTING;
}
