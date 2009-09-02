/*
 *  oFono - Open Source Telephony
 *
 *  Copyright (C) 2008-2009  Intel Corporation. All rights reserved.
 *  Copyright (C) 2009  Collabora Ltd. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <errno.h>

#include <glib.h>
#include <gatchat.h>
#include <gatsyntax.h>

#define OFONO_API_SUBJECT_TO_CHANGE
#include <ofono/plugin.h>
#include <ofono/log.h>
#include <ofono/modem.h>
#include <ofono/call-barring.h>
#include <ofono/call-forwarding.h>
#include <ofono/call-meter.h>
#include <ofono/call-settings.h>
#include <ofono/devinfo.h>
#include <ofono/message-waiting.h>
#include <ofono/netreg.h>
#include <ofono/phonebook.h>
#include <ofono/sim.h>
#include <ofono/sms.h>
#include <ofono/ssn.h>
#include <ofono/ussd.h>
#include <ofono/voicecall.h>

static void g1_debug(const char *str, void *data)
{
	DBG("%s", str);
}

/* Detect hardware, and initialize if found */
static int g1_probe(struct ofono_modem *modem)
{
	GAtSyntax *syntax;
	GAtChat *chat;

	DBG("");

	syntax = g_at_syntax_new_gsmv1();
	chat = g_at_chat_new_from_tty("/dev/smd0", syntax);
	g_at_syntax_unref(syntax);

	if (chat == NULL)
		return -EIO;

	if (getenv("OFONO_AT_DEBUG") != NULL)
		g_at_chat_set_debug(chat, g1_debug, NULL);

	ofono_modem_set_data(modem, chat);

	return 0;
}

static void g1_remove(struct ofono_modem *modem)
{
	GAtChat *chat = ofono_modem_get_data(modem);

	DBG("");

	ofono_modem_set_data(modem, NULL);
	g_at_chat_unref(chat);
}

static void cfun_set_on_cb(gboolean ok, GAtResult *result, gpointer user_data)
{
	struct ofono_modem *modem = user_data;
 
	DBG("");

	if (ok)
		ofono_modem_set_powered(modem, TRUE);
}

/* power up hardware */
static int g1_enable(struct ofono_modem *modem)
{
	GAtChat *chat = ofono_modem_get_data(modem);

	DBG("");

	/* ensure modem is in a known state; verbose on, echo/quiet off */
	g_at_chat_send(chat, "ATE0Q0V1", NULL, NULL, NULL, NULL);

	/* power up modem */
	g_at_chat_send(chat, "AT+CFUN=1", NULL, cfun_set_on_cb, modem, NULL);

	return 0;
}

static void cfun_set_off_cb(gboolean ok, GAtResult *result, gpointer user_data)
{
	struct ofono_modem *modem = user_data;
 
	DBG("");
 
	if (ok)
		ofono_modem_set_powered(modem, FALSE);
}

static int g1_disable(struct ofono_modem *modem)
{
	GAtChat *chat = ofono_modem_get_data(modem);

	DBG("");

	/* power down modem */
	g_at_chat_send(chat, "AT+CFUN=0", NULL, cfun_set_off_cb, modem, NULL);

	return 0;
}

static void g1_populate(struct ofono_modem *modem)
{
	GAtChat *chat = ofono_modem_get_data(modem);
	struct ofono_message_waiting *mw;

	DBG("");

	ofono_devinfo_create(modem, 0, "atmodem", chat);
	ofono_ussd_create(modem, 0, "atmodem", chat);
	ofono_sim_create(modem, 0, "atmodem", chat);
	ofono_call_forwarding_create(modem, 0, "atmodem", chat);
	ofono_call_settings_create(modem, 0, "atmodem", chat);
	ofono_netreg_create(modem, 0, "atmodem", chat);
	ofono_voicecall_create(modem, 0, "atmodem", chat);
	ofono_call_meter_create(modem, 0, "atmodem", chat);
	ofono_call_barring_create(modem, 0, "atmodem", chat);
	ofono_ssn_create(modem, 0, "atmodem", chat);
	ofono_phonebook_create(modem, 0, "atmodem", chat);

	mw = ofono_message_waiting_create(modem);
	if (mw)
		ofono_message_waiting_register(mw);
}

static struct ofono_modem_driver g1_driver = {
	.name = "g1",
	.probe = g1_probe,
	.remove = g1_remove,
	.enable = g1_enable,
	.disable = g1_disable,
	.populate = g1_populate,
};

static int g1_init(void)
{
	return ofono_modem_driver_register(&g1_driver);
}

static void g1_exit(void)
{
	ofono_modem_driver_unregister(&g1_driver);
}

OFONO_PLUGIN_DEFINE(g1, "HTC G1 modem driver", VERSION,
			OFONO_PLUGIN_PRIORITY_DEFAULT,
			g1_init, g1_exit)
