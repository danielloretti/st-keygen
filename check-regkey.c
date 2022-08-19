/*
 * Registration key maker for Stereo Tool
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

/* max name length */
#define MAXLEN		108

static char ascii2nibble(char ascii) {
	char nibble = ascii;

	if (ascii >= '0' && ascii <= '9') {
		nibble -= '0';
	} else if (ascii >= 'a' && ascii <= 'f') {
		nibble -= 'a';
		nibble += 0xa;
	} else {
		nibble = 0;
	}

	return nibble;
}

static void scrambler(char *key, size_t length) {
	char in, out;

	for (size_t i = 0; i < length; i++) {
		in = key[i];
		out = 0;
		for (int j = 0; j < 8; j++) {
			out <<= 1;
			out |= in & 1;
			in >>= 1;
		}
		key[i] = out ^ ((-1 - i) - (1 << (1 << (i & 31) & 7)));
	}
}

int main(int argc, char *argv[]) {
	int opt;
	char key[256];
	char *key_ascii;
	unsigned int features;
	int key_checksum, checksum;
	char *key_name;
	char *key_trailer;
	int key_len;
	int name_len;
	int i, j;

	const char *short_opt = "h";
	const struct option long_opt[] = {
		{"help",	no_argument,		NULL,	'h'},
		{0,		0,			0,	0}
	};

keep_parsing_opts:

	opt = getopt_long(argc, argv, short_opt, long_opt, NULL);
	if (opt == -1) goto done_parsing_opts;

	switch (opt) {
		case 'h':
		case '?':
		default:
			fprintf(stderr,
				"Stereo Tool key checker\n"
				"\n"
				"Checks whether a given license key is valid\n"
				"\n"
				"Usage: %s KEY\n"
				"\n"
				"The \"<\" and \">\" characters are optional\n"
				"\n",
			argv[0]);
			return 1;
	}

	goto keep_parsing_opts;

done_parsing_opts:

	if (argc < 2) {
		printf("Please specify a key.\n");
		return 1;
	}

	if (optind < argc) {
		key_ascii = argv[optind];
	}

	if (key_ascii[0] == '<')
		key_ascii++;

	/* convert key ASCII to bytes */
	i = j = 0;
	while (key_ascii[i] != 0 && i < (9+MAXLEN+1+8)*2) {
		if (key_ascii[i] == '>') break;

		if (i % 2) {
			key[j] <<= 4;
			key[j] |= ascii2nibble(key_ascii[i]);
			j++;
		} else {
			key[j] = ascii2nibble(key_ascii[i]);
		}
		i++;
	}

	/* get length */
	key_len = j;

	key_name = key + 9;

	/* unscramble */
	scrambler(key, key_len);

	/* get the key feature bitmask */
	memcpy(&features, key + 1, 4);

	/* get checksum */
	memcpy(&key_checksum, key + 5, 4);

	/* zero the checksum field */
	key[5] = key[6] = key[7] = key[8] = 0;

	/* calculate checksum */
	checksum = 0;
	for (int i = 0; i < key_len; i++) {
		checksum = key[i] * 0x11121 + (checksum << 3);
		checksum += checksum >> 26;
	}

	/* determine key name's length */
	i = 0;
	while (key_name[i] != 0 && i < MAXLEN) i++;
	name_len = i;

	key_trailer = key_name + name_len + 1;

	/* output results */
	printf("\n");
	printf("==========================================\n");
	printf("Start byte\t: 0x%02x\n", key[0]);
	printf("Name\t\t: %s\n", key_name);
	printf("Features\t: 0x%08x\n", features);
	printf("Calc'd checksum\t: 0x%08x\n", checksum);
	printf("Key's checksum\t: 0x%08x\n", key_checksum);
	printf("Trailing bytes\t: "
		"%02x %02x %02x %02x %02x %02x %02x %02x\n",
		key_trailer[0],
		key_trailer[1],
		key_trailer[2],
		key_trailer[3],
		key_trailer[4],
		key_trailer[5],
		key_trailer[6],
		key_trailer[7]
	);
	printf("==========================================\n");
	printf("\n");
}
