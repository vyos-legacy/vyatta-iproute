/*
 * iplink_vxlan.c	VXLAN device support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Stephen Hemminger <shemminger@vyatta.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ip.h>
#include <linux/if_link.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "tunnel.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... vxlan id VNI [ local ADDR ] [ group ADDR ]\n\n");
	fprintf(stderr, "Where: VNI := 0-16777215\n");
	fprintf(stderr, "       ADDR := { IP_ADDRESS | any }\n");
}

static int vxlan_parse_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *n)
{

	while (argc > 0) {
		if (!matches(*argv, "id")) {
			__u32 vni;

			NEXT_ARG();
			if (get_u32(&vni, *argv, 0))
				invarg("vni is invalid", *argv);
			addattr_l(n, 1024, IFLA_VXLAN_ID, &vni, sizeof(vni));
		} else if (!matches(*argv, "group")) {
			inet_prefix addr;

			NEXT_ARG();
			get_addr(&addr, *argv, preferred_family);
			addattr_l(n, 1024, IFLA_VXLAN_GROUP,
				  &addr.data, addr.bytelen);
		} else if (matches(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "vxlan: what is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--, argv++;
	}

	return 0;
}

static void vxlan_print_opt(struct link_util *lu, FILE *f, struct rtattr *tb[])
{
	__u32 vni;
	char ab[INET_ADDRSTRLEN];

	if (!tb)
		return;

	if (!tb[IFLA_VXLAN_ID] ||
	    RTA_PAYLOAD(tb[IFLA_VXLAN_ID]) < sizeof(__u32))
		return;

	vni = rta_getattr_u32(tb[IFLA_VXLAN_ID]);
	fprintf(f, "id %u ", vni);
	if (tb[IFLA_VXLAN_GROUP]) {
		__u32 addr = rta_getattr_u32(tb[IFLA_VXLAN_GROUP]);

		if (addr)
			fprintf(f, "group %s ",
				format_host(AF_INET, 4, &addr, ab, sizeof(ab)));
	}
}

struct link_util vxlan_link_util = {
	.id		= "vxlan",
	.maxattr	= IFLA_VXLAN_MAX,
	.parse_opt	= vxlan_parse_opt,
	.print_opt	= vxlan_print_opt,
};
