/*
 *
 *  oFono - Open Source Telephony
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
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

#include <drivers/atmodem/atutil.h>

#define XMM7MODEM "xmm7modem"

extern void xmm_radio_settings_init(void);
extern void xmm_radio_settings_exit(void);

extern void xmm_ims_init(void);
extern void xmm_ims_exit(void);
