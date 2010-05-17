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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#ifndef SOL_NETLINK
#define SOL_NETLINK 270 /* libc!? */
#endif
#include "phonet.h"
#include <linux/rtnetlink.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <glib.h>

#include "netlink.h"
#include "modem.h"

#ifndef ARPHRD_PHONET
#define ARPHRD_PHONET (820)
#endif

/*
 * GCC -Wcast-align does not like rtlink alignment macros,
 * fixed macros by Andrzej Zaborowski <balrogg@gmail.com>.
 */
#undef IFA_RTA
#define IFA_RTA(r)  ((struct rtattr*)(void*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))

#undef IFLA_RTA
#define IFLA_RTA(r)  ((struct rtattr*)(void *)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))

#undef NLMSG_NEXT
#define NLMSG_NEXT(nlh,len)	 ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
	(struct nlmsghdr*)(void*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))

#undef RTA_NEXT
#define RTA_NEXT(rta,attrlen)	((attrlen) -= RTA_ALIGN((rta)->rta_len), \
	(struct rtattr*)(void*)(((char*)(rta)) + RTA_ALIGN((rta)->rta_len)))

#define SIZE_NLMSG (16384)

struct _GPhonetNetlink {
	GPhonetNetlinkFunc callback;
	void *opaque;
	guint watch;
	unsigned interface;
};

static inline GIsiModem *make_modem(unsigned idx)
{
	return (void *)(uintptr_t)idx;
}

static GSList *netlink_list = NULL;

GPhonetNetlink *g_pn_netlink_by_modem(GIsiModem *idx)
{
	GSList *m;
	unsigned index = g_isi_modem_index(idx);

	for (m = netlink_list; m; m = m->next) {
		GPhonetNetlink *self = m->data;

		if (index == self->interface)
			return self;
	}

	return NULL;
}

static void bring_up(unsigned ifindex)
{
	struct ifreq req = { .ifr_ifindex = ifindex, };
	int fd = socket(PF_LOCAL, SOCK_DGRAM, 0);

	if (ioctl(fd, SIOCGIFNAME, &req)
		|| ioctl(fd, SIOCGIFFLAGS, &req))
		goto error;

	req.ifr_flags |= IFF_UP | IFF_RUNNING;
	ioctl(fd, SIOCSIFFLAGS, &req);
error:
	close(fd);
}

static int netlink_socket(void)
{
	int fd;
	int bufsize = SIZE_NLMSG;

	fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (fd == -1)
		return -1;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize))) {
		int error = errno;
		close(fd), fd = -1;
		errno = error;
	}

	return fd;
}

static void g_pn_nl_addr(GPhonetNetlink *self, struct nlmsghdr *nlh)
{
	int len;
	uint8_t local = 0xff;
	uint8_t remote = 0xff;

	const struct ifaddrmsg *ifa;
	const struct rtattr *rta;

	ifa = NLMSG_DATA(nlh);
	len = IFA_PAYLOAD(nlh);

	/* If Phonet is absent, kernel transmits other families... */
	if (ifa->ifa_family != AF_PHONET)
		return;

	if (ifa->ifa_index != self->interface)
		return;

	for (rta = IFA_RTA(ifa); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {

		if (rta->rta_type == IFA_LOCAL)
			local = *(uint8_t *)RTA_DATA(rta);
		else if (rta->rta_type == IFA_ADDRESS)
			remote = *(uint8_t *)RTA_DATA(rta);
	}
}

static void g_pn_nl_link(GPhonetNetlink *self, struct nlmsghdr *nlh)
{
	const struct ifinfomsg *ifi;
	const struct rtattr *rta;
	int len;
	const char *ifname = NULL;
	GIsiModem *idx = NULL;
	GPhonetLinkState st;

	ifi = NLMSG_DATA(nlh);
	len = IFA_PAYLOAD(nlh);

	if (ifi->ifi_type != ARPHRD_PHONET)
		return;

	if (self->interface != 0 && self->interface != (unsigned)ifi->ifi_index)
		return;

	idx = make_modem(ifi->ifi_index);

#define UP (IFF_UP | IFF_LOWER_UP | IFF_RUNNING)

	if (nlh->nlmsg_type == RTM_DELLINK)
		st = PN_LINK_REMOVED;
	else if ((ifi->ifi_flags & UP) != UP)
		st = PN_LINK_DOWN;
	else
		st = PN_LINK_UP;

	for (rta = IFLA_RTA(ifi); RTA_OK(rta, len);
		rta = RTA_NEXT(rta, len)) {

		if (rta->rta_type == IFLA_IFNAME)
			ifname = RTA_DATA(rta);
	}

	if (ifname && idx)
		self->callback(idx, st, ifname, self->opaque);

#undef UP
}

/* Parser Netlink messages */
static gboolean g_pn_nl_process(GIOChannel *channel, GIOCondition cond,
				gpointer data)
{
	struct {
		struct nlmsghdr nlh;
		char buf[SIZE_NLMSG];
	} resp;
	struct iovec iov = { &resp, sizeof(resp), };
	struct msghdr msg = { .msg_iov = &iov, .msg_iovlen = 1, };
	ssize_t ret;
	struct nlmsghdr *nlh;
	int fd = g_io_channel_unix_get_fd(channel);
	GPhonetNetlink *self = data;

	if (cond & (G_IO_NVAL|G_IO_HUP))
		return FALSE;

	ret = recvmsg(fd, &msg, 0);
	if (ret == -1)
		return TRUE;

	if (msg.msg_flags & MSG_TRUNC) {
		g_printerr("Netlink message of %zu bytes truncated at %zu\n",
				ret, sizeof(resp));
		return TRUE;
	}

	for (nlh = &resp.nlh; NLMSG_OK(nlh, (size_t)ret);
						nlh = NLMSG_NEXT(nlh, ret)) {
		if (nlh->nlmsg_type == NLMSG_DONE)
			break;

		switch (nlh->nlmsg_type) {
		case NLMSG_ERROR: {
			struct nlmsgerr *err = NLMSG_DATA(nlh);
			if (err->error)
				g_printerr("Netlink error: %s",
						strerror(-err->error));
			return TRUE;
		}
		case RTM_NEWADDR:
		case RTM_DELADDR:
			g_pn_nl_addr(self, nlh);
			break;
		case RTM_NEWLINK:
		case RTM_DELLINK:
			g_pn_nl_link(self, nlh);
			break;
		default:
			continue;
		}
	}
	return TRUE;
}

/* Dump current links */
static int g_pn_netlink_getlink(int fd)
{
	struct {
		struct nlmsghdr nlh;
		struct ifinfomsg ifi;
	} req = {
		.nlh = {
			.nlmsg_type = RTM_GETLINK,
			.nlmsg_len = sizeof(req),
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT | NLM_F_MATCH,
			.nlmsg_pid = getpid(),
		},
		.ifi = {
			.ifi_family = AF_UNSPEC,
			.ifi_type = ARPHRD_PHONET,
			.ifi_change = 0xffFFffFF,
		}
	};

	struct sockaddr_nl addr = { .nl_family = AF_NETLINK, };

	return sendto(fd, &req, sizeof(req), 0,
		      (struct sockaddr *)&addr, sizeof(addr));
}

GPhonetNetlink *g_pn_netlink_start(GIsiModem *idx,
				   GPhonetNetlinkFunc callback,
				   void *data)
{
	GIOChannel *chan;
	GPhonetNetlink *self;
	int fd;
	unsigned group = RTNLGRP_LINK;
	unsigned interface = g_isi_modem_index(idx);

	fd = netlink_socket();
	if (fd == -1)
		return NULL;

	self = calloc(1, sizeof(*self));
	if (self == NULL)
		goto error;

	fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));

	if (setsockopt(fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP,
		       &group, sizeof(group)))
		goto error;

	if (interface)
		bring_up(interface);

	g_pn_netlink_getlink(fd);

	chan = g_io_channel_unix_new(fd);
	if (chan == NULL)
		goto error;
	g_io_channel_set_close_on_unref(chan, TRUE);
	g_io_channel_set_encoding(chan, NULL, NULL);
	g_io_channel_set_buffered(chan, FALSE);

	self->callback = callback;
	self->opaque = data;
	self->interface = interface;
	self->watch = g_io_add_watch(chan, G_IO_IN|G_IO_ERR|G_IO_HUP,
					g_pn_nl_process, self);
	g_io_channel_unref(chan);

	netlink_list = g_slist_prepend(netlink_list, self);

	return self;

error:
	close(fd);
	free(self);
	return NULL;
}

void g_pn_netlink_stop(GPhonetNetlink *self)
{
	if (self) {
		netlink_list = g_slist_remove(netlink_list, self);
		g_source_remove(self->watch);
		free(self);
	}
}

static int netlink_getack(int fd)
{
	struct {
		struct nlmsghdr nlh;
		char buf[SIZE_NLMSG];
	} resp;
	struct iovec iov = { &resp, sizeof(resp), };
	struct msghdr msg = { .msg_iov = &iov, .msg_iovlen = 1, };
	ssize_t ret;
	struct nlmsghdr *nlh = &resp.nlh;

	ret = recvmsg(fd, &msg, 0);
	if (ret == -1)
		return -errno;

	if (msg.msg_flags & MSG_TRUNC)
		return -EIO;

	for (; NLMSG_OK(nlh, (size_t)ret); nlh = NLMSG_NEXT(nlh, ret)) {

		if (nlh->nlmsg_type == NLMSG_DONE)
			return 0;

		if (nlh->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *err = NLMSG_DATA(nlh);
			return err->error;
		}
	}

	return -EIO;
}

/* Set local address */
static int netlink_setaddr(uint32_t ifa_index, uint8_t ifa_local)
{
	struct ifaddrmsg *ifa;
	struct rtattr *rta;
	uint32_t reqlen = NLMSG_LENGTH(NLMSG_ALIGN(sizeof(*ifa))
				+ RTA_SPACE(1));
	struct req {
		struct nlmsghdr nlh;
		char buf[512];
	} req = {
		.nlh = {
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
			.nlmsg_type = RTM_NEWADDR,
			.nlmsg_pid = getpid(),
			.nlmsg_len = reqlen,
		},
	};
	int fd;
	int error;
	struct sockaddr_nl addr = { .nl_family = AF_NETLINK, };

	ifa = NLMSG_DATA(&req.nlh);
	ifa->ifa_family = AF_PHONET;
	ifa->ifa_prefixlen = 0;
	ifa->ifa_index = ifa_index;

	rta = IFA_RTA(ifa);
	rta->rta_type = IFA_LOCAL;
	rta->rta_len = RTA_LENGTH(1);
	*(uint8_t *)RTA_DATA(rta) = ifa_local;

	fd = netlink_socket();
	if (fd == -1)
		return -errno;

	if (sendto(fd, &req, reqlen, 0, (void *)&addr, sizeof(addr)) == -1)
		error = -errno;
	else
		error = netlink_getack(fd);

	close(fd);

	return error;
}

int g_pn_netlink_set_address(GIsiModem *idx, uint8_t local)
{
	uint32_t ifindex = g_isi_modem_index(idx);

	if (ifindex == 0)
		return -ENODEV;

	if (local != PN_DEV_PC && local != PN_DEV_SOS)
		return -EINVAL;

	return netlink_setaddr(ifindex, local);
}

/* Add remote address */
static int netlink_addroute(uint32_t ifa_index, uint8_t remote)
{
	struct rtmsg *rtm;
	struct rtattr *rta;
	uint32_t reqlen = NLMSG_LENGTH(NLMSG_ALIGN(sizeof(*rtm)) +
				RTA_SPACE(1) +
				RTA_SPACE(sizeof(ifa_index)));
	struct req {
		struct nlmsghdr nlh;
		char buf[512];
	} req = {
		.nlh = {
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK
					| NLM_F_CREATE | NLM_F_APPEND,
			.nlmsg_type = RTM_NEWROUTE,
			.nlmsg_pid = getpid(),
			.nlmsg_len = reqlen,
		},
	};
	size_t buflen = sizeof(req.buf) - sizeof(*rtm);
	int fd;
	int error;
	struct sockaddr_nl addr = { .nl_family = AF_NETLINK, };

	rtm = NLMSG_DATA(&req.nlh);
	rtm->rtm_family = AF_PHONET;
	rtm->rtm_dst_len = 6;
	rtm->rtm_src_len = 0;
	rtm->rtm_tos = 0;

	rtm->rtm_table = RT_TABLE_MAIN;
	rtm->rtm_protocol = RTPROT_STATIC;
	rtm->rtm_scope = RT_SCOPE_UNIVERSE;
	rtm->rtm_type = RTN_UNICAST;
	rtm->rtm_flags = 0;

	rta = IFA_RTA(rtm);
	rta->rta_type = RTA_DST;
	rta->rta_len = RTA_LENGTH(1);
	*(uint8_t *)RTA_DATA(rta) = remote;

	rta = RTA_NEXT(rta, buflen);
	rta->rta_type = RTA_OIF;
	rta->rta_len = RTA_LENGTH(sizeof(ifa_index));
	*(uint32_t *)RTA_DATA(rta) = ifa_index;

	fd = netlink_socket();
	if (fd == -1)
		return -errno;

	if (sendto(fd, &req, reqlen, 0, (void *)&addr, sizeof(addr)) == -1)
		error = -errno;
	else
		error = netlink_getack(fd);

	close(fd);

	return error;
}

int g_pn_netlink_add_route(GIsiModem *idx, uint8_t remote)
{
	uint32_t ifindex = g_isi_modem_index(idx);

	if (ifindex == 0)
		return -ENODEV;

	if (remote != PN_DEV_SOS && remote != PN_DEV_HOST)
		return -EINVAL;

	return netlink_addroute(ifindex, remote);
}
