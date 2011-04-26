/*
 * Copyright (c) 2011 Stefan Rinkes <stefan.rinkes@gmail.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include "pfcommon.h"
#include "pfutils.h"

static int	 pfrules_init(void);
static int	 pfrules_read(void);
static int	 get_rulestring(struct pfioc_rule *, char *);
#ifndef TEST
static void	 submit_counter(const char *, const char *, counter_t, int);
#endif

char		*pf_device = "/dev/pf";

int
pfrules_init(void)
{
	struct pf_status	status;
	int			pfrulesdev = -1;

	if ((pfrulesdev = open(pf_device, O_RDONLY)) == -1) {
		ERROR("unable to open %s", pf_device);
		return (-1);
	}
	if (ioctl(pfrulesdev, DIOCGETSTATUS, &status)) {
		ERROR("DIOCGETSTATUS: %i", pfrulesdev);
		close(pfrulesdev);
		return (-1);
	}

	close(pfrulesdev);
	if (!status.running)
		return (-1);

	return (0);
}

int
pfrules_read(void)
{
	struct pfioc_rule	 pr;
	struct pf_rule		 rule;
	u_int32_t		 nr, mnr;
	char			*path;
	char			 rulestring[256];
#if VERSION < 46
	int			 i;
	static int		 nattype[3] = { PF_NAT, PF_RDR, PF_BINAT };
#endif
	char			 anchorname[MAXPATHLEN];
	int			 pfrulesdev = -1;

	memset(anchorname, 0, sizeof(anchorname));

	if ((path = calloc(1, MAXPATHLEN)) == NULL) {
		ERROR("calloc path failed");
		return (-1);
	}

	memset(&pr, 0, sizeof(pr));
	memcpy(pr.anchor, path, sizeof(pr.anchor));

#if VERSION < 46
	if ((pfrulesdev = open(pf_device, O_RDONLY)) == -1) {
		ERROR("unable to open %s", pf_device);
		goto error;
	}

	pr.rule.action = PF_SCRUB;
	if (ioctl(pfrulesdev, DIOCGETRULES, &pr)) {
		ERROR("DIOCGETRULES1: %i", pfrulesdev);
		goto error;
	}

	mnr = pr.nr;
	for (nr = 0; nr < mnr; ++nr) {
		pr.nr = nr;
		if (ioctl(pfrulesdev, DIOCGETRULE, &pr)) {
			ERROR("DIOCGETRULE1: %i", pfrulesdev);
			goto error;
		}
		rule = pr.rule;
		if (get_rulestring(&pr, rulestring) == -1) {
			ERROR("get_rulestring failed");
			goto error;
		}
#ifndef TEST
		submit_counter("states_cur", rulestring,
		    rule.states_cur, 1);
		submit_counter("states_tot", rulestring,
		    rule.states_tot, 0);
		submit_counter("evaluations", rulestring,
		    (unsigned long long)rule.evaluations, 0);
		submit_counter("pf_packets_in", rulestring,
		    (unsigned long long)rule.packets[0], 0);
		submit_counter("pf_packets_out", rulestring,
		    (unsigned long long)rule.packets[1], 0);
		submit_counter("pf_bytes_in", rulestring,
		    (unsigned long long)rule.bytes[0], 0);
		submit_counter("pf_bytes_out", rulestring,
		    (unsigned long long)rule.bytes[1], 0);
#else
		printf("Rule-Number: %i\n", rule.nr);
		printf("Rule: %s\n", rulestring);
		printf("States cur: %-6u\n", rule.states_cur);
		printf("States tot: %-6u\n", rule.states_tot);
		printf("Evaluations: %-8llu\n",
		    (unsigned long long)rule.evaluations);
		printf("Packets in: %-10llu\n",
		    (unsigned long long)rule.packets[0]);
		printf("Packets out: %-10llu\n",
		    (unsigned long long)rule.packets[1]);
		printf("Bytes in: %-10llu\n",
		    (unsigned long long)rule.bytes[0]);
		printf("Bytes out: %-10llu\n",
		    (unsigned long long)rule.bytes[1]);
		printf("\n");
#endif /* TEST */
	}
	close(pfrulesdev);
#endif /* VERSION */

	if ((pfrulesdev = open(pf_device, O_RDONLY)) == -1) {
		ERROR("unable to open %s", pf_device);
		goto error;
	}

	pr.rule.action = PF_PASS;
	if (ioctl(pfrulesdev, DIOCGETRULES, &pr)) {
		ERROR("DIOCGETRULES2: %i", pfrulesdev);
		goto error;
	}
	close(pfrulesdev);

	mnr = pr.nr;
	for (nr = 0; nr < mnr; ++nr) {
		pr.nr = nr;
		if ((pfrulesdev = open(pf_device, O_RDONLY)) == -1) {
			ERROR("unable to open %s", pf_device);
			goto error;
		}
		if (ioctl(pfrulesdev, DIOCGETRULE, &pr)) {
			ERROR("DIOCGETRULE2: %i", pfrulesdev);
			goto error;
		}
		rule = pr.rule;

		if (get_rulestring(&pr, rulestring) == -1) {
			ERROR("get_rulestring failed");
			goto error;
		}
#ifndef TEST
		submit_counter("states_cur", rulestring,
		    rule.states_cur, 1);
		submit_counter("states_tot", rulestring,
		    rule.states_tot, 0);
		submit_counter("evaluations", rulestring,
		    (unsigned long long)rule.evaluations, 0);
		submit_counter("pf_packets_in", rulestring,
		    (unsigned long long)rule.packets[0], 0);
		submit_counter("pf_packets_out", rulestring,
		    (unsigned long long)rule.packets[1], 0);
		submit_counter("pf_bytes_in", rulestring,
		    (unsigned long long)rule.bytes[0], 0);
		submit_counter("pf_bytes_out", rulestring,
		    (unsigned long long)rule.bytes[1], 0);
#else
		printf("Rule-Number: %i\n", rule.nr);
		printf("Rule: %s\n", rulestring);
		printf("States cur: %-6u\n", rule.states_cur);
		printf("States tot: %-6u\n", rule.states_tot);
		printf("Evaluations: %-8llu\n",
		    (unsigned long long)rule.evaluations);
		printf("Packets in: %-10llu\n",
		    (unsigned long long)rule.packets[0]);
		printf("Packets out: %-10llu\n",
		    (unsigned long long)rule.packets[1]);
		printf("Bytes in: %-10llu\n",
		    (unsigned long long)rule.bytes[0]);
		printf("Bytes out: %-10llu\n",
		    (unsigned long long)rule.bytes[1]);
		printf("\n");
#endif /* TEST */
		close(pfrulesdev);
	}

#if VERSION < 46
	for (i = 0; i < 3; i++) {
		if ((pfrulesdev = open(pf_device, O_RDONLY)) == -1) {
			ERROR("unable to open %s", pf_device);
			goto error;
		}
		pr.rule.action = nattype[i];
		if (ioctl(pfrulesdev, DIOCGETRULES, &pr)) {
			ERROR("DIOCGETRULES3: %i", pfrulesdev);
			goto error;
		}
		close(pfrulesdev);
		mnr = pr.nr;
		for (nr = 0; nr < mnr; ++nr) {
			pr.nr = nr;
			if ((pfrulesdev = open(pf_device, O_RDONLY)) == -1) {
				ERROR("unable to open %s", pf_device);
				goto error;
			}
			if (ioctl(pfrulesdev, DIOCGETRULE, &pr)) {
				ERROR("DIOCGETRULE3: %i", pfrulesdev);
				goto error;
			}
			if (pfctl_get_pool(pfrulesdev, &pr.rule.rpool, nr,
			    pr.ticket, nattype[i], anchorname) != 0) {
				ERROR("pfctl_get_pool failed");
				goto error;
			}
			rule = pr.rule;
			if (get_rulestring(&pr, rulestring) == -1) {
				ERROR("get_rulestring failed");
				goto error;
			}
			pfctl_clear_pool(&pr.rule.rpool);
#ifndef TEST
			submit_counter("states_cur", rulestring,
			    rule.states_cur, 1);
			submit_counter("states_tot", rulestring,
			    rule.states_tot, 0);
			submit_counter("evaluations", rulestring,
			    (unsigned long long)rule.evaluations, 0);
			submit_counter("pf_packets_in", rulestring,
			    (unsigned long long)rule.packets[0], 0);
			submit_counter("pf_packets_out", rulestring,
			    (unsigned long long)rule.packets[1], 0);
			submit_counter("pf_bytes_in", rulestring,
			    (unsigned long long)rule.bytes[0], 0);
			submit_counter("pf_bytes_out", rulestring,
			    (unsigned long long)rule.bytes[1], 0);
#else
			printf("Rule-Number: %i\n", rule.nr);
			printf("Rule: %s\n", rulestring);
			printf("States cur: %-6u\n", rule.states_cur);
			printf("States tot: %-6u\n", rule.states_tot);
			printf("Evaluations: %-8llu\n",
			    (unsigned long long)rule.evaluations);
			printf("Packets in: %-10llu\n",
			    (unsigned long long)rule.packets[0]);
			printf("Packets out: %-10llu\n",
			    (unsigned long long)rule.packets[1]);
			printf("Bytes in: %-10llu\n",
			    (unsigned long long)rule.bytes[0]);
			printf("Bytes out: %-10llu\n",
			    (unsigned long long)rule.bytes[1]);
			printf("\n");
#endif /* TEST */
		close(pfrulesdev);
		}
	}
#endif /* VERSION */
	free(path);
	return (0);
error:
	if (pfrulesdev != -1)
		close(pfrulesdev);
	free(path);
	return (-1);
}

int
get_rulestring(struct pfioc_rule *pr, char *rulestring)
{
	print_rule(&pr->rule, pr->anchor_call, 0, rulestring);

	char *p1 = rulestring;
	char *p2 = rulestring;
	while(*p1 != 0) {
		if(*p1 == '!') {
			*p2++ = *p1++;
			++p1;
		} else if(*p1 == '=') {
			*--p2 = (int)"";
			*p2++ = *p1++;
			++p1;
		} else if(isspace(*p1)) {
			*p2++ = '-';
			++p1;
		} else if((*p1 == '(') || (*p1 == ')') ||
			  (*p1 == '>') || (*p1 == '-')) {
			++p1;
		} else {
			*p2++ = *p1++;
		}
	}
	*p2 = 0;

	return 0;
}

#ifndef TEST
void
submit_counter(const char *rule_number, const char *inst, counter_t val, int usegauge)
{
	value_t		values[1];
	value_list_t	vl = VALUE_LIST_INIT;

	if (usegauge)
		values[0].gauge = val;
	else
		values[0].counter = val;

	vl.values = values;
	vl.values_len = 1;
	sstrncpy (vl.host, hostname_g, sizeof (vl.host));
	sstrncpy (vl.plugin, "pfrules", sizeof (vl.plugin));
	sstrncpy (vl.type, rule_number, sizeof(vl.type));
	sstrncpy (vl.type_instance, inst, sizeof(vl.type_instance));
	plugin_dispatch_values(&vl);
}
#endif /* TEST */

#ifdef TEST
int
main(int argc, char *argv[])
{
	if (pfrules_init())
		err(1, "pfrules_init");
	if (pfrules_read())
		err(1, "pfrules_read");
	return (0);
}
#else
void module_register(void) {
	plugin_register_init("pfrules", pfrules_init);
	plugin_register_read("pfrules", pfrules_read);
}
#endif /* TEST */
