/*
 * Registration key maker for ST
 * Copyright (C) 2021 Anthony96922
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

/* set to 1 for event FM (4 days) license */
#define EVENT_FM	0

#include "st-common.h"

static void scramble(char *key, int length) {
	char in, out;
	int i, j;

	for (i = 0; i < length; i++) {
		in = key[i] ^ (-1 - i - (1 << (1 << (i & 31) & 7)));
		out = 0;
		for (j = 0; j < 8; j++) {
			out <<= 1;
			out |= in & 1;
			in >>= 1;
		}
		key[i] = out;
	}
}

int main(int argc, char *argv[]) {
	int opt;
	unsigned int features = FEATURES;
	int name_len;
	int key_len;
	char key_trailer[8];
	/* key checksum */
	int checksum;
	char name[MAXLEN + 1];
	char key[9 + MAXLEN + 1 + 8];
	char out_key_text[(9 + MAXLEN + 1 + 8) * 2];
	const char *short_opt = "f:a:";
	int i;

keep_parsing_opts:

	opt = getopt(argc, argv, short_opt);
	if (opt == -1) goto done_parsing_opts;

	switch (opt) {
		case 'f':
			features = strtoul(optarg, NULL, 16);
			break;

		case 'a':
			features |= strtoul(optarg, NULL, 16);
			break;

		default:
			printf("Usage: %s [-f features (hex)] [-a extras (hex)] NAME\n",
				argv[0]);
			return 1;
	}

	goto keep_parsing_opts;

done_parsing_opts:

	name[0] = 0;

	if (optind < argc) {
		name_len = strlen(argv[optind]);
		if (name_len > MAXLEN) {
			printf("Name is too long.\n");
			return 1;
		}
		name[MAXLEN] = 0;
		strncpy(name, argv[optind], MAXLEN);
	}

	if (!name[0]) {
		printf("Please enter a name.\n");
		return 1;
	}

	/* input validation */

	/* pad the name with spaces if it is shorter than 5 chars */
	if (name_len < 5) {
		for (i = 0; i < 5 - name_len; i++)
			name[name_len + i] = ' ';
		name[5] = 0;
		name_len = 5;
	}

	/* make sure we don't try to divide by 0 */
	if ((name[2] - name[3]) + 1 == 0) {
		/* we can't divide by 0 */
		printf("Invalid name.\n");
		return 1;
	}

	/*
	 * 18 = the stuff before and after the key (112233445566778899<name>00aabbccddeeffaabb)
	 * 14 (9 + name_len + 1 + 4) is the bare minimum
	 */
	key_len = 9 + name_len + 1 /* null terminator for name string */ + 8;

	*key = 1; /* doesn't seem to affect anything */

	/* registered options */
	memcpy(key + 1, &features, 4);

	/* copy name to key */
	memcpy(key + 9, name, name_len);

	/* add terminator */
	*(key + 9 + name_len) = 0;

	/* add name check trailer */
	calc_name_check(key + 9 + name_len + 1, name);
	memcpy(key_trailer, key + 9 + name_len + 1, 8);

	/* clear checksum field */
	memset(key + 5, 0, 4);

	/* calculate the checksum */
	checksum = calc_checksum(key, key_len);

	/* copy the checksum */
	memcpy(key + 5, &checksum, 4);

	/* scramble the key */
	scramble(key, key_len);

	for (i = 0; i < key_len; i++)
		sprintf(out_key_text + i * 2, "%02x", key[i]);

	/* output */
	printf("\n");
	printf("==========================================\n");
	printf("Name\t\t: %s\n", name);
	printf("Features\t: 0x%08x\n", features);
	printf("Calc'd checksum\t: 0x%08x\n", checksum);
	printf("Trailing bytes\t: "
		"0x%02x 0x%02x 0x%02x 0x%02x\n"
		"\t\t  0x%02x 0x%02x 0x%02x 0x%02x\n",
		key_trailer[0], key_trailer[1],
		key_trailer[2], key_trailer[3],
		key_trailer[4], key_trailer[5],
		key_trailer[6], key_trailer[7]);
	printf("==========================================\n");
	printf("\n");
	show_features(features);
	printf("\n");
	printf("<%s>\n", out_key_text);
	printf("\n");

	return 0;
}
