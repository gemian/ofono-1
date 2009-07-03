/*
 * This file is part of oFono - Open Source Telephony
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Rémi Denis-Courmont <remi.denis-courmont@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "phonet.h"
#include <glib.h>

#include "socket.h"

GIOChannel *phonet_new(uint8_t resource)
{
	GIOChannel *channel;
	struct sockaddr_pn addr = {
		.spn_family = AF_PHONET,
		.spn_resource = resource,
	};

	int fd = socket(PF_PHONET, SOCK_DGRAM, 0);
	if (fd == -1)
		return NULL;
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	/* Use blocking mode on purpose. */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr))) {
		close(fd);
		return NULL;
	}

	channel = g_io_channel_unix_new(fd);
	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);
	return channel;
}

size_t phonet_peek_length(GIOChannel *channel)
{
	int len;
	int fd = g_io_channel_unix_get_fd(channel);
	return ioctl(fd, FIONREAD, &len) ? 0 : len;
}

ssize_t phonet_read(GIOChannel *channel, void *restrict buf, size_t len,
			uint16_t *restrict obj, uint8_t *restrict res)
{
	struct sockaddr_pn addr;
	socklen_t addrlen = sizeof(addr);
	ssize_t ret;

	ret = recvfrom(g_io_channel_unix_get_fd(channel), buf, len,
			MSG_DONTWAIT, (struct sockaddr *)&addr, &addrlen);
	if (ret == -1)
		return -1;

	if (obj != NULL)
		*obj = pn_sockaddr_get_object(&addr);
	if (res != NULL)
		*res = addr.spn_resource;
	return ret;
}
