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

#include "pfcommon.h"

static int	pfrules_init(void);
static int	pfrules_read(void);
#ifndef TEST
static void	submit_counter(const char *, const char *, counter_t);
#endif

int
pfrules_init(void)
{
	struct pf_status	status;

	if ((dev = open(PF_SOCKET, O_RDWR)) == -1) {
		return (-1);
	}
	if (ioctl(dev, DIOCGETSTATUS, &status) == -1) {
		return (-1);
	}

	close(dev);
	if (!status.running)
		return (-1);

	return (0);
}

#ifndef TEST
void
submit_counter(const char *rule_number, const char *inst, counter_t val)
{
	value_t		values[1];
	value_list_t	vl = VALUE_LIST_INIT;

	values[0].gauge = val;

	vl.values = values;
	vl.values_len = 1;
	sstrncpy (vl.host, hostname_g, sizeof (vl.host));
	sstrncpy (vl.plugin, "pfrules", sizeof (vl.plugin));
	sstrncpy (vl.type, rule_number, sizeof(vl.type));
	sstrncpy (vl.type_instance, inst, sizeof(vl.type_instance));
	plugin_dispatch_values(&vl);
}
#endif


int
pfrules_read(void)
{
	struct pfioc_rule	 pr;
	struct pf_rule		 rule;
	u_int32_t		 nr, mnr;
	char			*path;
#ifndef TEST
	char			 rule_number[10];
#endif

	if ((path = calloc(1, MAXPATHLEN)) == NULL)
		return (-1);

	memset(&pr, 0, sizeof(pr));
	memcpy(pr.anchor, path, sizeof(pr.anchor));

	if ((dev = open(PF_SOCKET, O_RDWR)) == -1) {
		return (-1);
	}

	pr.rule.action = PF_SCRUB;
	if (ioctl(dev, DIOCGETRULES, &pr)) {
		return (-1);
	}

	mnr = pr.nr;
	for (nr = 0; nr < mnr; ++nr) {
		pr.nr = nr;
		if (ioctl(dev, DIOCGETRULE, &pr)) {
			return (-1);
		}
		rule = pr.rule;
#ifndef TEST
		snprintf(rule_number, sizeof(rule_number), "%i", rule.nr);
		submit_counter("scrub_states_current", rule_number,
		    rule.states_cur);
		submit_counter("scrub_states_total", rule_number,
		    rule.states_tot);
		submit_counter("scrub_evaluations", rule_number,
		    (unsigned long long)rule.evaluations);
		submit_counter("scrub_bytes", rule_number,
		    (unsigned long long)(rule.packets[0] + rule.packets[1]));
#else
		/*
		 * XXX: get rule name
		 *      print_rule() in pfctl_parser.c
		 *      User rule.nr as identifier is a bad idea if user
		 *      changes the ruleset. Use the rule string is more specific.
		 */
		printf("Rule-Number: %i\n", rule.nr);
		printf("States cur: %-6u\n", rule.states_cur);
		printf("States tot: %-6u\n", rule.states_tot);
		printf("Evaluations: %-8llu\n",
		    (unsigned long long)rule.evaluations);
		printf("Bytes: %-10llu\n",
		    (unsigned long long)(rule.packets[0] + rule.packets[1]));
		printf("\n");
#endif
	}

	pr.rule.action = PF_PASS;
	if (ioctl(dev, DIOCGETRULES, &pr)) {
		return (-1);
	}

	mnr = pr.nr;
	for (nr = 0; nr < mnr; ++nr) {
		pr.nr = nr;
		if (ioctl(dev, DIOCGETRULE, &pr)) {
			return (-1);
		}
		rule = pr.rule;
#ifndef TEST
		snprintf(rule_number, sizeof(rule_number), "%i", rule.nr);
		submit_counter("rule_states_current", rule_number,
		    rule.states_cur);
		submit_counter("rule_states_total", rule_number,
		    rule.states_tot);
		submit_counter("rule_evaluations", rule_number,
		    (unsigned long long)rule.evaluations);
		submit_counter("rule_bytes", rule_number,
		    (unsigned long long)(rule.packets[0] + rule.packets[1]));
#else
		printf("Rule-Number: %i\n", rule.nr);
		printf("States cur: %-6u\n", rule.states_cur);
		printf("States tot: %-6u\n", rule.states_tot);
		printf("Evaluations: %-8llu\n",
		    (unsigned long long)rule.evaluations);
		printf("Bytes: %-10llu\n",
		    (unsigned long long)(rule.packets[0] + rule.packets[1]));
		printf("\n");
#endif
	}
	close(dev);
	return (0);
}

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
#endif
