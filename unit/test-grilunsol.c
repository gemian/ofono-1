/*
 *
 *  oFono - Open Source Telephony
 *
 *  Copyright (C) 2013 Canonical Ltd.
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

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <glib.h>
#include <errno.h>

#include <ofono/modem.h>
#include <ofono/gprs-context.h>
#include <ofono/types.h>

#include "gril.h"
#include "grilunsol.h"

/*
 * TODO: It may make sense to split this file into
 * domain-specific files ( eg. test-grilrequest-gprs-context.c )
 * once more tests are added.
 */

/*
 * As all our architectures are little-endian except for
 * PowerPC, and the Binder wire-format differs slightly
 * depending on endian-ness, the following guards against test
 * failures when run on PowerPC.
 */
#if BYTE_ORDER == LITTLE_ENDIAN

typedef struct signal_strength_test signal_strength_test;
struct signal_strength_test {
	int strength;
	const struct ril_msg msg;
};

static const struct ril_msg reply_data_call_invalid_1 = {
	.buf = "",
	.buf_len = 0,
	.unsolicited = FALSE,
	.req = RIL_REQUEST_SETUP_DATA_CALL,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data equates te the following
 * RIL_REQUEST_SETUP_DATA_CALL reply parameters:
 *
 * {version=2,num=2 [status=0,retry=-1,cid=0,active=2,type=IP}
 * Parcel is truncated, as num=2 should trigger a failure.
 */
static const guchar reply_data_call_invalid_parcel2[] = {
	0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x49, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg reply_data_call_invalid_2 = {
	.buf = (gchar *) &reply_data_call_invalid_parcel2,
	.buf_len = sizeof(reply_data_call_invalid_parcel2),
	.unsolicited = FALSE,
	.req = RIL_REQUEST_SETUP_DATA_CALL,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data is a binary representation of
 * a parcel containing an invalid RIL_REQUEST_SETUP_DATA_CALL reply
 * with a NULL string specified for 'type':
 *
 * {version=7,num=1 [status=0,retry=-1,cid=0,active=2,type=NULL
 * ifname=rmnet_usb0,address=10.181.235.154/30,
 * dns=172.16.145.103 172.16.145.103,gateways=10.181.235.153]}
 */
static const guchar reply_data_call_invalid_parcel3[] = {
	0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff,
	0x0a, 0x00, 0x00, 0x00, 0x72, 0x00, 0x6d, 0x00, 0x6e, 0x00, 0x65, 0x00,
	0x74, 0x00, 0x5f, 0x00, 0x75, 0x00, 0x73, 0x00, 0x62, 0x00, 0x30, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00,
	0x33, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x34, 0x00,
	0x2f, 0x00, 0x33, 0x00, 0x30, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
	0x31, 0x00, 0x37, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x36, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x34, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x30, 0x00, 0x33, 0x00, 0x20, 0x00, 0x31, 0x00, 0x37, 0x00, 0x32, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x36, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x34, 0x00,
	0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x33, 0x00, 0x00, 0x00,
	0x0e, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00, 0x33, 0x00, 0x35, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg reply_data_call_invalid_3 = {
	.buf = (gchar *) &reply_data_call_invalid_parcel3,
	.buf_len = sizeof(reply_data_call_invalid_parcel3),
	.unsolicited = FALSE,
	.req = RIL_REQUEST_SETUP_DATA_CALL,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data is a binary representation of
 * a parcel containing an invalid RIL_REQUEST_SETUP_DATA_CALL reply
 * with a NULL string specified for 'ifname':
 *
 * {version=7,num=1 [status=0,retry=-1,cid=0,active=2,type=IP
 * ifname=NULL,address=10.181.235.154/30,
 * dns=172.16.145.103 172.16.145.103,gateways=10.181.235.153]}
 */
static const guchar reply_data_call_invalid_parcel4[] = {
	0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x49, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x11, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00,
	0x33, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x34, 0x00,
	0x2f, 0x00, 0x33, 0x00, 0x30, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
	0x31, 0x00, 0x37, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x36, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x34, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x30, 0x00, 0x33, 0x00, 0x20, 0x00, 0x31, 0x00, 0x37, 0x00, 0x32, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x36, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x34, 0x00,
	0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x33, 0x00, 0x00, 0x00,
	0x0e, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00, 0x33, 0x00, 0x35, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg reply_data_call_invalid_4 = {
	.buf = (gchar *) &reply_data_call_invalid_parcel4,
	.buf_len = sizeof(reply_data_call_invalid_parcel4),
	.unsolicited = FALSE,
	.req = RIL_REQUEST_SETUP_DATA_CALL,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data is a binary representation of
 * a parcel containing an invalid RIL_REQUEST_SETUP_DATA_CALL reply
 * with a NULL string specified for 'address':
 *
 * {version=7,num=1 [status=0,retry=-1,cid=0,active=2,type=IP
 * ifname=rmnet_usb0,address=NULL,
 * dns=172.16.145.103 172.16.145.103,gateways=10.181.235.153]}
 */
static const guchar reply_data_call_invalid_parcel5[] = {
	0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x49, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x00, 0x72, 0x00, 0x6d, 0x00, 0x6e, 0x00, 0x65, 0x00,
	0x74, 0x00, 0x5f, 0x00, 0x75, 0x00, 0x73, 0x00, 0x62, 0x00, 0x30, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x1d, 0x00, 0x00, 0x00,
	0x31, 0x00, 0x37, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x36, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x34, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x30, 0x00, 0x33, 0x00, 0x20, 0x00, 0x31, 0x00, 0x37, 0x00, 0x32, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x36, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x34, 0x00,
	0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x33, 0x00, 0x00, 0x00,
	0x0e, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00, 0x33, 0x00, 0x35, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg reply_data_call_invalid_5 = {
	.buf = (gchar *) &reply_data_call_invalid_parcel5,
	.buf_len = sizeof(reply_data_call_invalid_parcel5),
	.unsolicited = FALSE,
	.req = RIL_REQUEST_SETUP_DATA_CALL,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing an invalid RIL_REQUEST_SETUP_DATA_CALL reply
 * with a NULL string specified for 'gateways':
 *
 * {version=7,num=1 [status=0,retry=-1,cid=0,active=2,type=IP
 * ifname=rmnet_usb0,address=10.181.235.154/30,
 * dns=172.16.145.103 172.16.145.103,gateways=NULL]}
 */
static const guchar reply_data_call_invalid_parcel6[] = {
	0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x49, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x00, 0x72, 0x00, 0x6d, 0x00, 0x6e, 0x00, 0x65, 0x00,
	0x74, 0x00, 0x5f, 0x00, 0x75, 0x00, 0x73, 0x00, 0x62, 0x00, 0x30, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00,
	0x33, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x34, 0x00,
	0x2f, 0x00, 0x33, 0x00, 0x30, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
	0x31, 0x00, 0x37, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x36, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x34, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x30, 0x00, 0x33, 0x00, 0x20, 0x00, 0x31, 0x00, 0x37, 0x00, 0x32, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x36, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x34, 0x00,
	0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x33, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff
};

static const struct ril_msg reply_data_call_invalid_6 = {
	.buf = (gchar *) &reply_data_call_invalid_parcel6,
	.buf_len = sizeof(reply_data_call_invalid_parcel6),
	.unsolicited = FALSE,
	.req = RIL_REQUEST_SETUP_DATA_CALL,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing a valid RIL_REQUEST_SETUP_DATA_CALL reply with the
 * following parameters:
 *
 * {version=7,num=1 [status=0,retry=-1,cid=0,active=2,type=IP,
 * ifname=rmnet_usb0,address=10.181.235.154/30,
 * dns=172.16.145.103 172.16.145.103,gateways=10.181.235.153]}
 */
static const guchar reply_data_call_valid_parcel1[] = {
	0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x49, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x00, 0x72, 0x00, 0x6d, 0x00, 0x6e, 0x00, 0x65, 0x00,
	0x74, 0x00, 0x5f, 0x00, 0x75, 0x00, 0x73, 0x00, 0x62, 0x00, 0x30, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00,
	0x33, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x34, 0x00,
	0x2f, 0x00, 0x33, 0x00, 0x30, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
	0x31, 0x00, 0x37, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x36, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x34, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x30, 0x00, 0x33, 0x00, 0x20, 0x00, 0x31, 0x00, 0x37, 0x00, 0x32, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x36, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x34, 0x00,
	0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x33, 0x00, 0x00, 0x00,
	0x0e, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00, 0x33, 0x00, 0x35, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg reply_data_call_valid_1 = {
	.buf = (gchar *) &reply_data_call_valid_parcel1,
	.buf_len = sizeof(reply_data_call_valid_parcel1),
	.unsolicited = FALSE,
	.req = RIL_REQUEST_SETUP_DATA_CALL,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing an valid RIL_REQUEST_SETUP_DATA_CALL reply with
 * a NULL string specified for 'dns' (note that some data calls without DNS
 * information might still be rejected by the GPRS context driver):
 *
 * {version=7,num=1 [status=0,retry=-1,cid=0,active=2,type=IP
 * ifname=rmnet_usb0,address=10.181.235.154/30,
 * dns=NULL,gateways=10.181.235.153]}
 */
static const guchar reply_data_call_valid_parcel2[] = {
	0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x49, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x00, 0x72, 0x00, 0x6d, 0x00, 0x6e, 0x00, 0x65, 0x00,
	0x74, 0x00, 0x5f, 0x00, 0x75, 0x00, 0x73, 0x00, 0x62, 0x00, 0x30, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00,
	0x33, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x34, 0x00,
	0x2f, 0x00, 0x33, 0x00, 0x30, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0x0e, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x38, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x32, 0x00, 0x33, 0x00, 0x35, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x35, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg reply_data_call_valid_2 = {
	.buf = (gchar *) &reply_data_call_valid_parcel2,
	.buf_len = sizeof(reply_data_call_valid_parcel2),
	.unsolicited = FALSE,
	.req = RIL_REQUEST_SETUP_DATA_CALL,
	.serial_no = 0,
	.error = 0,
};

static const struct ril_msg unsol_data_call_list_changed_invalid_1 = {
	.buf = "",
	.buf_len = 0,
	.unsolicited = TRUE,
	.req = RIL_UNSOL_DATA_CALL_LIST_CHANGED,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing a valid RIL_UNSOL_DATA_CALL_LIST_CHANGED message
 * with the following parameters:
 *
 * (version=7,num=1 [status=0,retry=-1,cid=0,active=1,type=IP,
 * ifname=rmnet_usb0,address=10.209.114.102/30,
 * dns=172.16.145.103 172.16.145.103,gateways=10.209.114.101]}
 */
static const char unsol_data_call_list_changed_parcel1[] = {
	0x00, 0x00, 0x00, 0xd4, 0x01, 0x00, 0x00, 0x00, 0xf2, 0x03, 0x00, 0x00,
	0x07, 0x00, 0x00, 0x00,	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0x01, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x49, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x00, 0x72, 0x00, 0x6d, 0x00, 0x6e, 0x00, 0x65, 0x00,
	0x74, 0x00, 0x5f, 0x00,	0x75, 0x00, 0x73, 0x00, 0x62, 0x00, 0x30, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,	0x31, 0x00, 0x30, 0x00,
	0x2e, 0x00, 0x32, 0x00, 0x30, 0x00, 0x39, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x31, 0x00, 0x34, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00, 0x32, 0x00,
	0x2f, 0x00, 0x33, 0x00,	0x30, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
	0x31, 0x00, 0x37, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x36, 0x00,
	0x2e, 0x00, 0x31, 0x00, 0x34, 0x00, 0x35, 0x00, 0x2e, 0x00, 0x31, 0x00,
	0x30, 0x00, 0x33, 0x00, 0x20, 0x00, 0x31, 0x00, 0x37, 0x00, 0x32, 0x00,
	0x2e, 0x00, 0x31, 0x00,	0x36, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x34, 0x00,
	0x35, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x30, 0x00,	0x33, 0x00, 0x00, 0x00,
	0x0e, 0x00, 0x00, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x32, 0x00,
	0x30, 0x00, 0x39, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x31, 0x00, 0x34, 0x00,
	0x2e, 0x00, 0x31, 0x00,	0x30, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg unsol_data_call_list_changed_valid_1 = {
	.buf = (gchar *) &unsol_data_call_list_changed_parcel1,
	.buf_len = sizeof(unsol_data_call_list_changed_parcel1),
	.unsolicited = TRUE,
	.req = RIL_UNSOL_DATA_CALL_LIST_CHANGED,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing a valid RIL_UNSOL_RESPONSE_NEW_SMS message
 * with the following parameter (SMSC address length is 7):
 *
 * {07914306073011F0040B914336543980F50000310113212002400AC8373B0C6AD7DDE437}
 */
static const char unsol_response_new_sms_parcel1[] = {
	0x48, 0x00, 0x00, 0x00, 0x30, 0x00, 0x37, 0x00, 0x39, 0x00, 0x31, 0x00,
	0x34, 0x00, 0x33, 0x00, 0x30, 0x00, 0x36, 0x00, 0x30, 0x00, 0x37, 0x00,
	0x33, 0x00, 0x30, 0x00, 0x31, 0x00, 0x31, 0x00, 0x46, 0x00, 0x30, 0x00,
	0x30, 0x00, 0x34, 0x00, 0x30, 0x00, 0x42, 0x00, 0x39, 0x00, 0x31, 0x00,
	0x34, 0x00, 0x33, 0x00, 0x33, 0x00, 0x36, 0x00, 0x35, 0x00, 0x34, 0x00,
	0x33, 0x00, 0x39, 0x00, 0x38, 0x00, 0x30, 0x00, 0x46, 0x00, 0x35, 0x00,
	0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x33, 0x00, 0x31, 0x00,
	0x30, 0x00, 0x31, 0x00, 0x31, 0x00, 0x33, 0x00, 0x32, 0x00, 0x31, 0x00,
	0x32, 0x00, 0x30, 0x00, 0x30, 0x00, 0x32, 0x00, 0x34, 0x00, 0x30, 0x00,
	0x30, 0x00, 0x41, 0x00, 0x43, 0x00, 0x38, 0x00, 0x33, 0x00, 0x37, 0x00,
	0x33, 0x00, 0x42, 0x00, 0x30, 0x00, 0x43, 0x00, 0x36, 0x00, 0x41, 0x00,
	0x44, 0x00, 0x37, 0x00, 0x44, 0x00, 0x44, 0x00, 0x45, 0x00, 0x34, 0x00,
	0x33, 0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg unsol_response_new_sms_valid_1 = {
	.buf = (gchar *) &unsol_response_new_sms_parcel1,
	.buf_len = sizeof(unsol_response_new_sms_parcel1),
	.unsolicited = TRUE,
	.req = RIL_UNSOL_RESPONSE_NEW_SMS,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing a valid RIL_UNSOL_RESPONSE_NEW_SMS message
 * with the following parameter (SMSC address length is 6):
 *
 * {069143060730F0040B914336543980F50000310113212002400AC8373B0C6AD7DDE437}
 */
static const char unsol_response_new_sms_parcel2[] = {
	0x46, 0x00, 0x00, 0x00, 0x30, 0x00, 0x36, 0x00, 0x39, 0x00, 0x31, 0x00,
	0x34, 0x00, 0x33, 0x00, 0x30, 0x00, 0x36, 0x00, 0x30, 0x00, 0x37, 0x00,
	0x33, 0x00, 0x30, 0x00, 0x46, 0x00, 0x30, 0x00,	0x30, 0x00, 0x34, 0x00,
	0x30, 0x00, 0x42, 0x00, 0x39, 0x00, 0x31, 0x00,	0x34, 0x00, 0x33, 0x00,
	0x33, 0x00, 0x36, 0x00, 0x35, 0x00, 0x34, 0x00,	0x33, 0x00, 0x39, 0x00,
	0x38, 0x00, 0x30, 0x00, 0x46, 0x00, 0x35, 0x00,	0x30, 0x00, 0x30, 0x00,
	0x30, 0x00, 0x30, 0x00, 0x33, 0x00, 0x31, 0x00,	0x30, 0x00, 0x31, 0x00,
	0x31, 0x00, 0x33, 0x00, 0x32, 0x00, 0x31, 0x00,	0x32, 0x00, 0x30, 0x00,
	0x30, 0x00, 0x32, 0x00, 0x34, 0x00, 0x30, 0x00,	0x30, 0x00, 0x41, 0x00,
	0x43, 0x00, 0x38, 0x00, 0x33, 0x00, 0x37, 0x00,	0x33, 0x00, 0x42, 0x00,
	0x30, 0x00, 0x43, 0x00, 0x36, 0x00, 0x41, 0x00,	0x44, 0x00, 0x37, 0x00,
	0x44, 0x00, 0x44, 0x00, 0x45, 0x00, 0x34, 0x00,	0x33, 0x00, 0x37, 0x00,
	0x00, 0x00, 0x00, 0x00
};

static const struct ril_msg unsol_response_new_sms_valid_2 = {
	.buf = (gchar *) &unsol_response_new_sms_parcel2,
	.buf_len = sizeof(unsol_response_new_sms_parcel2),
	.unsolicited = TRUE,
	.req = RIL_UNSOL_RESPONSE_NEW_SMS,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing a valid RIL_UNSOL_SUPP_SVC_NOTIFICATION message
 * with the following parameters:
 *
 * {1,2,0,0,} -> call has been put on hold
 */
static const guchar unsol_supp_svc_notif_parcel1[] = {
	0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};

static const struct ril_msg unsol_supp_svc_notif_valid_1 = {
	.buf = (gchar *) &unsol_supp_svc_notif_parcel1,
	.buf_len = sizeof(unsol_supp_svc_notif_parcel1),
	.unsolicited = TRUE,
	.req = RIL_UNSOL_SUPP_SVC_NOTIFICATION,
	.serial_no = 0,
	.error = 0,
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing a valid RIL_UNSOL_SIGNAL_STRENGTH message
 * with the following parameters:
 *
 * (gw: 14, cdma: -1, evdo: -1, lte: 99)
 *
 * Note, the return value for gw sigmal is: (gw * 100) / 31, which
 * in this case equals 45.
 */
static const guchar unsol_signal_strength_parcel1[] = {
	0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x63, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x7f,
	0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f
};

static const signal_strength_test unsol_signal_strength_valid_1 = {
	.strength = 45,
	.msg = {
		.buf = (gchar *) &unsol_signal_strength_parcel1,
		.buf_len = sizeof(unsol_signal_strength_parcel1),
		.unsolicited = TRUE,
		.req = RIL_UNSOL_SIGNAL_STRENGTH,
		.serial_no = 0,
		.error = 0,
	}
};

/*
 * The following hexadecimal data represents a serialized Binder parcel
 * instance containing a valid RIL_UNSOL_ON_USSD message with the following
 * parameters:
 *
 * {0,Spain 12:56 09/12/13  Canary 11:56 09/12/13  }
 */
static const guchar unsol_on_ussd_parcel1[] = {
	0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00,
	0x2d, 0x00, 0x00, 0x00, 0x53, 0x00, 0x70, 0x00, 0x61, 0x00, 0x69, 0x00,
	0x6e, 0x00, 0x20, 0x00, 0x31, 0x00, 0x32, 0x00, 0x3a, 0x00, 0x35, 0x00,
	0x36, 0x00, 0x20, 0x00, 0x30, 0x00, 0x39, 0x00, 0x2f, 0x00, 0x31, 0x00,
	0x32, 0x00, 0x2f, 0x00, 0x31, 0x00, 0x33, 0x00, 0x20, 0x00, 0x20, 0x00,
	0x43, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x61, 0x00, 0x72, 0x00, 0x79, 0x00,
	0x20, 0x00, 0x31, 0x00, 0x31, 0x00, 0x3a, 0x00, 0x35, 0x00, 0x36, 0x00,
	0x20, 0x00, 0x30, 0x00, 0x39, 0x00, 0x2f, 0x00, 0x31, 0x00, 0x32, 0x00,
	0x2f, 0x00, 0x31, 0x00, 0x33, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00
};

static const struct ril_msg unsol_on_ussd_valid_1 = {
	.buf = (gchar *) &unsol_on_ussd_parcel1,
	.buf_len = sizeof(unsol_on_ussd_parcel1),
	.unsolicited = TRUE,
	.req = RIL_UNSOL_ON_USSD,
	.serial_no = 0,
	.error = 0,
};

static void test_reply_data_call_invalid(gconstpointer data)
{
	struct ril_data_call_list *call_list;

	call_list = g_ril_unsol_parse_data_call_list(NULL, data);
	g_assert(call_list == NULL);
}

static void test_reply_data_call_valid(gconstpointer data)
{
	struct ril_data_call_list *call_list =
		g_ril_unsol_parse_data_call_list(NULL, data);

	g_assert(call_list != NULL);
	g_ril_unsol_free_data_call_list(call_list);
}

static void test_unsol_data_call_list_changed_invalid(gconstpointer data)
{
	struct ril_data_call_list *unsol;

	unsol = g_ril_unsol_parse_data_call_list(NULL, data);
	g_assert(unsol == NULL);
}

static void test_unsol_data_call_list_changed_valid(gconstpointer data)
{
	struct ril_data_call_list *unsol;

	unsol = g_ril_unsol_parse_data_call_list(NULL, data);
	g_assert(unsol != NULL);
	g_ril_unsol_free_data_call_list(unsol);
}

static void test_signal_strength_valid(gconstpointer data)
{
	const signal_strength_test *test = data;
	int strength = g_ril_unsol_parse_signal_strength(NULL, &test->msg);
	g_assert(strength == test->strength);
}

static void test_unsol_response_new_sms_valid(gconstpointer data)
{
	struct unsol_sms_data *sms_data;

	sms_data = g_ril_unsol_parse_new_sms(NULL, data);

	g_assert(sms_data != NULL);
	g_assert(sms_data->data != NULL);
	g_assert(sms_data->length > 0);

	g_ril_unsol_free_sms_data(sms_data);
}

static void test_unsol_supp_svc_notif_valid(gconstpointer data)
{
	struct unsol_supp_svc_notif *unsol;

	unsol = g_ril_unsol_parse_supp_svc_notif(NULL,
						(struct ril_msg *) data);
	g_assert(unsol != NULL);
	g_ril_unsol_free_supp_svc_notif(unsol);
}

static void test_unsol_on_ussd_valid(gconstpointer data)
{
	struct unsol_ussd *unsol;

	unsol = g_ril_unsol_parse_ussd(NULL, (struct ril_msg *) data);

	g_assert(unsol != NULL);
	g_ril_unsol_free_ussd(unsol);
}
#endif

int main(int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);

/*
 * As all our architectures are little-endian except for
 * PowerPC, and the Binder wire-format differs slightly
 * depending on endian-ness, the following guards against test
 * failures when run on PowerPC.
 */
#if BYTE_ORDER == LITTLE_ENDIAN

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"invalid DATA_CALL_LIST_CHANGED Test 1",
				&unsol_data_call_list_changed_invalid_1,
				test_unsol_data_call_list_changed_invalid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"valid DATA_CALL_LIST_CHANGED Test 1",
				&unsol_data_call_list_changed_valid_1,
				test_unsol_data_call_list_changed_valid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"invalid SETUP_DATA_CALL Test 1",
				&reply_data_call_invalid_1,
				test_reply_data_call_invalid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"invalid SETUP_DATA_CALL Test 2",
				&reply_data_call_invalid_2,
				test_reply_data_call_invalid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"invalid SETUP_DATA_CALL Test 3",
				&reply_data_call_invalid_3,
				test_reply_data_call_invalid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"invalid SETUP_DATA_CALL Test 4",
				&reply_data_call_invalid_4,
				test_reply_data_call_invalid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"invalid SETUP_DATA_CALL Test 5",
				&reply_data_call_invalid_5,
				test_reply_data_call_invalid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"invalid SETUP_DATA_CALL Test 6",
				&reply_data_call_invalid_6,
				test_reply_data_call_invalid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"valid SETUP_DATA_CALL Test 1",
				&reply_data_call_valid_1,
				test_reply_data_call_valid);

	g_test_add_data_func("/testgrilunsol/gprs-context: "
				"valid SETUP_DATA_CALL Test 2",
				&reply_data_call_valid_2,
				test_reply_data_call_valid);

	g_test_add_data_func("/testgrilunsol/sms: "
				"valid RESPONSE_NEW_SMS Test 1",
				&unsol_response_new_sms_valid_1,
				test_unsol_response_new_sms_valid);

	g_test_add_data_func("/testgrilunsol/sms: "
				"valid RESPONSE_NEW_SMS Test 2",
				&unsol_response_new_sms_valid_2,
				test_unsol_response_new_sms_valid);

	g_test_add_data_func("/testgrilunsol/voicecall: "
				"valid SUPP_SVC_NOTIF Test 1",
				&unsol_supp_svc_notif_valid_1,
				test_unsol_supp_svc_notif_valid);

	g_test_add_data_func("/testgrilunsol/voicecall: "
				"valid SIGNAL_STRENGTH Test 1",
				&unsol_signal_strength_valid_1,
				test_signal_strength_valid);

	g_test_add_data_func("/testgrilunsol/ussd: "
				"valid ON_USSD Test 1",
				&unsol_on_ussd_valid_1,
				test_unsol_on_ussd_valid);

#endif
	return g_test_run();
}
