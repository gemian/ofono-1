/*
 *
 *  oFono - Open Source Telephony
 *
 *  Copyright (C) 2008-2010  Intel Corporation. All rights reserved.
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <glib.h>
#include <gatchat.h>
#include <gatmux.h>
#include <gattty.h>

#define OFONO_API_SUBJECT_TO_CHANGE
#include <ofono/plugin.h>
#include <ofono/modem.h>
#include <ofono/devinfo.h>
#include <ofono/netreg.h>
#include <ofono/phonebook.h>
#include <ofono/voicecall.h>
#include <ofono/call-barring.h>
#include <ofono/call-forwarding.h>
#include <ofono/call-meter.h>
#include <ofono/call-settings.h>
#include <ofono/call-volume.h>
#include <ofono/message-waiting.h>
#include <ofono/ssn.h>
#include <ofono/sim.h>
#include <ofono/cbs.h>
#include <ofono/sms.h>
#include <ofono/ussd.h>
#include <ofono/gprs.h>
#include <ofono/gprs-context.h>
#include <ofono/radio-settings.h>
#include <ofono/stk.h>
#include <ofono/log.h>

#include <drivers/atmodem/atutil.h>
#include <drivers/atmodem/vendor.h>

#define NUM_DLC  4

#define VOICE_DLC   0
#define NETREG_DLC  1
#define GPRS_DLC    2
#define AUX_DLC     3

static char *dlc_prefixes[NUM_DLC] = { "Voice: ", "Net: ", "GPRS: ", "Aux: " };

static const char *dlc_nodes[NUM_DLC] = { "/dev/ttyGSM1", "/dev/ttyGSM2",
					"/dev/ttyGSM7", "/dev/ttyGSM8" };

static const char *none_prefix[] = { NULL };
static const char *xdrv_prefix[] = { "+XDRV:", NULL };
static const char *xgendata_prefix[] = { "+XGENDATA:", NULL };

struct ifx_data {
	GIOChannel *device;
	GAtMux *mux;
	GAtChat *dlcs[NUM_DLC];
	guint dlc_poll_count;
	guint dlc_poll_source;
	guint frame_size;
	int mux_ldisc;
	int saved_ldisc;
	int audio_source;
	int audio_dest;
	int audio_context;
	const char *audio_setting;
	int audio_loopback;
	struct ofono_sim *sim;
	gboolean have_sim;
};

static void ifx_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	ofono_info("%s%s", prefix, str);
}

static int ifx_probe(struct ofono_modem *modem)
{
	struct ifx_data *data;

	DBG("%p", modem);

	data = g_try_new0(struct ifx_data, 1);
	if (!data)
		return -ENOMEM;

	data->mux_ldisc = -1;
	data->saved_ldisc = -1;

	ofono_modem_set_data(modem, data);

	return 0;
}

static void ifx_remove(struct ofono_modem *modem)
{
	struct ifx_data *data = ofono_modem_get_data(modem);

	DBG("%p", modem);

	ofono_modem_set_data(modem, NULL);

	g_free(data);
}

static void xsim_notify(GAtResult *result, gpointer user_data)
{
	struct ofono_modem *modem = user_data;
	struct ifx_data *data = ofono_modem_get_data(modem);

	GAtResultIter iter;
	int state;

	if (!data->sim)
		return;

	g_at_result_iter_init(&iter, result);

	if (!g_at_result_iter_next(&iter, "+XSIM:"))
		return;

	if (!g_at_result_iter_next_number(&iter, &state))
		return;

	DBG("state %d", state);

	switch (state) {
	case 0:	/* SIM not present */
	case 9:	/* SIM Removed */
		if (data->have_sim == TRUE) {
			ofono_sim_inserted_notify(data->sim, FALSE);
			data->have_sim = FALSE;
		}
		break;
	case 1:	/* PIN verification needed */
	case 2:	/* PIN verification not needed – Ready */
	case 3:	/* PIN verified – Ready */
	case 4:	/* PUK verification needed */
	case 5:	/* SIM permanently blocked */
	case 6:	/* SIM Error */
	case 7:	/* ready for attach (+COPS) */
	case 8:	/* SIM Technical Problem */
		if (data->have_sim == FALSE) {
			ofono_sim_inserted_notify(data->sim, TRUE);
			data->have_sim = TRUE;
		}
		break;
	default:
		ofono_warn("Unknown SIM state %d received", state);
		break;
	}
}

static GAtChat *create_chat(GIOChannel *channel, char *debug)
{
	GAtSyntax *syntax;
	GAtChat *chat;

	if (!channel)
		return NULL;

	syntax = g_at_syntax_new_gsmv1();
	chat = g_at_chat_new(channel, syntax);
	g_at_syntax_unref(syntax);
	g_io_channel_unref(channel);

	if (!chat)
		return NULL;

	if (getenv("OFONO_AT_DEBUG"))
		g_at_chat_set_debug(chat, ifx_debug, debug);

	g_at_chat_send(chat, "ATE0 +CMEE=1", NULL,
					NULL, NULL, NULL);

	return chat;
}

static void shutdown_device(struct ifx_data *data)
{
	int i, fd;

	DBG("");

	for (i = 0; i < NUM_DLC; i++) {
		if (!data->dlcs[i])
			continue;

		g_at_chat_unref(data->dlcs[i]);
		data->dlcs[i] = NULL;
	}

	if (data->mux) {
		g_at_mux_shutdown(data->mux);
		g_at_mux_unref(data->mux);
		data->mux = NULL;
		goto done;
	}

	fd = g_io_channel_unix_get_fd(data->device);

	if (ioctl(fd, TIOCSETD, &data->saved_ldisc) < 0)
		ofono_warn("Failed to restore line discipline");

done:
	g_io_channel_unref(data->device);
	data->device = NULL;
}

static void cfun_enable(gboolean ok, GAtResult *result, gpointer user_data)
{
	struct ofono_modem *modem = user_data;
	struct ifx_data *data = ofono_modem_get_data(modem);
	char buf[64];

	DBG("");

	if (!ok) {
		shutdown_device(data);

		ofono_modem_set_powered(modem, FALSE);
		return;
	}

	g_at_chat_send(data->dlcs[AUX_DLC], "AT+XGENDATA", xgendata_prefix,
							NULL, NULL, NULL);

	if (data->audio_setting && data->audio_source && data->audio_dest) {
		/* configure source */
		snprintf(buf, sizeof(buf), "AT+XDRV=40,4,%d,%d,%s",
						data->audio_source,
						data->audio_context,
						data->audio_setting);
		g_at_chat_send(data->dlcs[AUX_DLC], buf, xdrv_prefix,
						NULL, NULL, NULL);

		/* configure destination */
		snprintf(buf, sizeof(buf), "AT+XDRV=40,5,%d,%d,%s",
						data->audio_dest,
						data->audio_context,
						data->audio_setting);
		g_at_chat_send(data->dlcs[AUX_DLC], buf, xdrv_prefix,
						NULL, NULL, NULL);

		if (data->audio_loopback) {
			/* set destination for source */
			snprintf(buf, sizeof(buf), "AT+XDRV=40,6,%d,%d",
					data->audio_source, data->audio_dest);
			g_at_chat_send(data->dlcs[AUX_DLC], buf, xdrv_prefix,
							NULL, NULL, NULL);

			/* enable source */
			snprintf(buf, sizeof(buf), "AT+XDRV=40,2,%d",
							data->audio_source);
			g_at_chat_send(data->dlcs[AUX_DLC], buf, xdrv_prefix,
							NULL, NULL, NULL);
		}
	}

	data->have_sim = FALSE;

	/* notify that the modem is ready so that pre_sim gets called */
	ofono_modem_set_powered(modem, TRUE);

	g_at_chat_register(data->dlcs[AUX_DLC], "+XSIM:", xsim_notify,
						FALSE, modem, NULL);

	/* enable XSIM and XLOCK notifications */
	g_at_chat_send(data->dlcs[AUX_DLC], "AT+XSIMSTATE=1", none_prefix,
						NULL, NULL, NULL);
}

static gboolean dlc_ready_check(gpointer user_data)
{
	struct ofono_modem *modem = user_data;
	struct ifx_data *data = ofono_modem_get_data(modem);
	struct stat st;
	int i;

	DBG("");

	data->dlc_poll_count++;

	if (stat(dlc_nodes[AUX_DLC], &st) < 0) {
		/* only possible error is ENOENT */
		if (data->dlc_poll_count > 6)
			goto error;

		return TRUE;
	}

	for (i = 0; i < NUM_DLC; i++) {
		GIOChannel *channel = g_at_tty_open(dlc_nodes[i], NULL);

		data->dlcs[i] = create_chat(channel, dlc_prefixes[i]);
		if (!data->dlcs[i]) {
			ofono_error("Failed to open %s", dlc_nodes[i]);
			goto error;
		}
	}

	g_at_chat_send(data->dlcs[AUX_DLC], "AT+CFUN=4", NULL,
					cfun_enable, modem, NULL);

	data->dlc_poll_source = 0;

	return FALSE;

error:
	data->dlc_poll_source = 0;

	shutdown_device(data);

	ofono_modem_set_powered(modem, FALSE);

	return FALSE;
}

static void setup_internal_mux(struct ofono_modem *modem)
{
	struct ifx_data *data = ofono_modem_get_data(modem);
	GIOFlags flags;
	int i;

	DBG("");

	flags = g_io_channel_get_flags(data->device) | G_IO_FLAG_NONBLOCK;
	g_io_channel_set_flags(data->device, flags, NULL);

	g_io_channel_set_encoding(data->device, NULL, NULL);
	g_io_channel_set_buffered(data->device, FALSE);

	data->mux = g_at_mux_new_gsm0710_basic(data->device, data->frame_size);
	if (!data->mux)
		goto error;

	if (getenv("OFONO_AT_DEBUG"))
		g_at_mux_set_debug(data->mux, ifx_debug, "MUX: ");

	g_at_mux_start(data->mux);

	for (i = 0; i < NUM_DLC; i++) {
		GIOChannel *channel = g_at_mux_create_channel(data->mux);

		data->dlcs[i] = create_chat(channel, dlc_prefixes[i]);
		if (!data->dlcs[i]) {
			ofono_error("Failed to create channel");
			goto error;
		}
        }

	g_at_chat_send(data->dlcs[AUX_DLC], "AT+CFUN=4", NULL,
					cfun_enable, modem, NULL);

	return;

error:
	shutdown_device(data);

	ofono_modem_set_powered(modem, FALSE);
}

static void mux_setup_cb(gboolean ok, GAtResult *result, gpointer user_data)
{
	struct ofono_modem *modem = user_data;
	struct ifx_data *data = ofono_modem_get_data(modem);
	int fd;

	DBG("");

	g_at_chat_unref(data->dlcs[AUX_DLC]);
	data->dlcs[AUX_DLC] = NULL;

	if (!ok)
		goto error;

	if (data->mux_ldisc < 0) {
		ofono_info("Using internal multiplexer");
		setup_internal_mux(modem);
		return;
	}

	fd = g_io_channel_unix_get_fd(data->device);

	if (ioctl(fd, TIOCGETD, &data->saved_ldisc) < 0) {
		ofono_error("Failed to get current line discipline");
		goto error;
	}

	if (ioctl(fd, TIOCSETD, &data->mux_ldisc) < 0) {
		ofono_error("Failed to set multiplexer line discipline");
		goto error;
	}

	data->dlc_poll_count = 0;
	data->dlc_poll_source = g_timeout_add_seconds(1, dlc_ready_check,
								modem);

	return;

error:
	data->saved_ldisc = -1;

	g_io_channel_unref(data->device);
	data->device = NULL;

	ofono_modem_set_powered(modem, FALSE);
}

static int ifx_enable(struct ofono_modem *modem)
{
	struct ifx_data *data = ofono_modem_get_data(modem);
	const char *device, *ldisc, *model, *audio, *loopback;
	GAtSyntax *syntax;
	GAtChat *chat;

	DBG("%p", modem);

	device = ofono_modem_get_string(modem, "Device");
	if (device == NULL)
		return -EINVAL;

	DBG("%s", device);

	model = ofono_modem_get_string(modem, "Model");
	if (g_strcmp0(model, "XMM6260") == 0) {
		data->audio_source = 4;
		data->audio_dest = 3;
		data->audio_context = 0;
	}

	audio = ofono_modem_get_string(modem, "AudioSetting");
	if (g_strcmp0(audio, "FULL_DUPLEX") == 0)
		data->audio_setting = "0,0,0,0,0,0,0,0,0";
	else if (g_strcmp0(audio, "BURSTMODE_48KHZ") == 0)
		data->audio_setting = "0,0,8,0,2,0,0,0,0";
	else if (g_strcmp0(audio, "BURSTMODE_96KHZ") == 0)
		data->audio_setting = "0,0,9,0,2,0,0,0,0";

	loopback = ofono_modem_get_string(modem, "AudioLoopback");
	if (loopback != NULL)
		data->audio_loopback = atoi(loopback);

	ldisc = ofono_modem_get_string(modem, "LineDiscipline");
	if (ldisc != NULL) {
		data->mux_ldisc = atoi(ldisc);
		ofono_info("Using multiplexer line discipline %d",
							data->mux_ldisc);
	}

	data->device = g_at_tty_open(device, NULL);
	if (!data->device)
                return -EIO;

	syntax = g_at_syntax_new_gsmv1();
	chat = g_at_chat_new(data->device, syntax);
	g_at_syntax_unref(syntax);

	if (!chat) {
		g_io_channel_unref(data->device);
                return -EIO;
	}

	if (getenv("OFONO_AT_DEBUG"))
		g_at_chat_set_debug(chat, ifx_debug, "Master: ");

	g_at_chat_send(chat, "ATE0 +CMEE=1", NULL,
					NULL, NULL, NULL);

	data->frame_size = 1509;

	g_at_chat_send(chat, "AT+CMUX=0,0,,1509,10,3,30,,", NULL,
					mux_setup_cb, modem, NULL);

	data->dlcs[AUX_DLC] = chat;

	return -EINPROGRESS;
}

static void cfun_disable(gboolean ok, GAtResult *result, gpointer user_data)
{
	struct ofono_modem *modem = user_data;
	struct ifx_data *data = ofono_modem_get_data(modem);

	DBG("");

	if (data->dlc_poll_source > 0) {
		g_source_remove(data->dlc_poll_source);
		data->dlc_poll_source = 0;
	}

	shutdown_device(data);

	if (ok)
		ofono_modem_set_powered(modem, FALSE);
}

static int ifx_disable(struct ofono_modem *modem)
{
	struct ifx_data *data = ofono_modem_get_data(modem);
	int i;

	DBG("%p", modem);

	for (i = 0; i < NUM_DLC; i++) {
		g_at_chat_cancel_all(data->dlcs[i]);
		g_at_chat_unregister_all(data->dlcs[i]);
	}

	g_at_chat_send(data->dlcs[AUX_DLC], "AT+CFUN=4", NULL,
					cfun_disable, modem, NULL);

	return -EINPROGRESS;
}

static void set_online_cb(gboolean ok, GAtResult *result, gpointer user_data)
{
	struct cb_data *cbd = user_data;
	ofono_modem_online_cb_t cb = cbd->cb;

	if (ok)
		CALLBACK_WITH_SUCCESS(cb, cbd->data);
	else
		CALLBACK_WITH_FAILURE(cb, cbd->data);
}

static void ifx_set_online(struct ofono_modem *modem, ofono_bool_t online,
				ofono_modem_online_cb_t cb, void *user_data)
{
	struct ifx_data *data = ofono_modem_get_data(modem);
	struct cb_data *cbd = cb_data_new(cb, user_data);
	char const *command = online ? "AT+CFUN=1" : "AT+CFUN=4";

	DBG("%p %s", modem, online ? "online" : "offline");

	if (!cbd)
		goto error;

	if (g_at_chat_send(data->dlcs[AUX_DLC], command, NULL,
					set_online_cb, cbd, g_free) > 0)
		return;

error:
	g_free(cbd);

	CALLBACK_WITH_FAILURE(cb, cbd->data);
}

static void ifx_pre_sim(struct ofono_modem *modem)
{
	struct ifx_data *data = ofono_modem_get_data(modem);

	DBG("%p", modem);

	ofono_devinfo_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);
	data->sim = ofono_sim_create(modem, OFONO_VENDOR_IFX,
					"atmodem", data->dlcs[AUX_DLC]);
	ofono_voicecall_create(modem, 0, "ifxmodem", data->dlcs[VOICE_DLC]);
}

static void ifx_post_sim(struct ofono_modem *modem)
{
	struct ifx_data *data = ofono_modem_get_data(modem);

	DBG("%p", modem);

	ofono_stk_create(modem, 0, "ifxmodem", data->dlcs[AUX_DLC]);
	ofono_phonebook_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);
}

static void ifx_post_online(struct ofono_modem *modem)
{
	struct ifx_data *data = ofono_modem_get_data(modem);
	struct ofono_message_waiting *mw;
	struct ofono_gprs *gprs;
	struct ofono_gprs_context *gc;

	DBG("%p", modem);

	ofono_radio_settings_create(modem, 0, "ifxmodem", data->dlcs[AUX_DLC]);
	ofono_netreg_create(modem, OFONO_VENDOR_IFX,
					"atmodem", data->dlcs[NETREG_DLC]);

	ofono_sms_create(modem, OFONO_VENDOR_IFX,
					"atmodem", data->dlcs[AUX_DLC]);
	ofono_cbs_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);
	ofono_ussd_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);

	ofono_ssn_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);
	ofono_call_forwarding_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);
	ofono_call_settings_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);
	ofono_call_meter_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);
	ofono_call_barring_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);
	ofono_call_volume_create(modem, 0, "atmodem", data->dlcs[AUX_DLC]);

	mw = ofono_message_waiting_create(modem);
	if (mw)
		ofono_message_waiting_register(mw);

	gprs = ofono_gprs_create(modem, 0, "atmodem", data->dlcs[NETREG_DLC]);
	gc = ofono_gprs_context_create(modem, 0,
					"atmodem", data->dlcs[GPRS_DLC]);

	if (gprs && gc)
		ofono_gprs_add_context(gprs, gc);
}

static struct ofono_modem_driver ifx_driver = {
	.name		= "ifx",
	.probe		= ifx_probe,
	.remove		= ifx_remove,
	.enable		= ifx_enable,
	.disable	= ifx_disable,
	.set_online	= ifx_set_online,
	.pre_sim	= ifx_pre_sim,
	.post_sim	= ifx_post_sim,
	.post_online	= ifx_post_online,
};

static int ifx_init(void)
{
	return ofono_modem_driver_register(&ifx_driver);
}

static void ifx_exit(void)
{
	ofono_modem_driver_unregister(&ifx_driver);
}

OFONO_PLUGIN_DEFINE(ifx, "Infineon modem driver", VERSION,
			OFONO_PLUGIN_PRIORITY_DEFAULT, ifx_init, ifx_exit)
