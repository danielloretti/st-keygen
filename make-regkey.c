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

// max name length
#define MAXLEN		64

// the default name to use when none is specified
#define DEFAULT_NAME	"Akira Kurosawa"

// not sure how these are calculated
#define FEATURES	0xffffffff ^ (1 << 14 | 1 << 19)
// bits 4 and 19 need to be clear to make umpx and natural dynamics work

// the bits that need to be set int key_trailer[3] for some names
#define UNKNOWN_BITS	((1 << 5) | (1 << 4)) // b4 + b5 = 48

static void dump_bit32(unsigned int value) {
	printf("%08x: ", value);
	for (int i = 0; i < 32; i++) {
		printf(((value >> (31 - i)) & 1) ? "1" : "0");
		if (i == 3 || i == 7 || i == 11 || i == 15 ||
		    i == 19 || i == 23 || i == 27) printf(" ");
	}
	printf("\n");
}

int main(int argc, char *argv[]) {
	int opt;
	unsigned int features = FEATURES;
	int unknown = 0;
	int name_len;
	int default_name = 1;
	int default_features = 1;
	int key_len;
	// key checksum
	int checksum;
	char name[MAXLEN+1];
	unsigned char key[(MAXLEN+1)*4];
	char out_key[(MAXLEN+1)*4];

	const char *short_opt = "n:f:uh";
	const struct option long_opt[] = {
		{"name",	required_argument,	NULL,	'n'},
		{"features",	required_argument,	NULL,	'f'},
		{"unknown",	no_argument,		NULL,	'u'},
		{"help",	no_argument,		NULL,	'h'},
		{0,		0,			0,	0}
	};

	memset(name, 0, MAXLEN+1);
	memset(key, 0, (MAXLEN+1)*4);
	memset(out_key, 0, (MAXLEN+1)*4);

	while ((opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
		switch (opt) {
			case 'n':
				strncpy(name, optarg, MAXLEN);
				printf("Using name \"%s\".\n", name);
				default_name = 0;
				break;
			case 'f':
				features = strtoul(optarg, NULL, 16);
				printf("Using custom feature config (0x%08x)\n", features);
				default_features = 0;
				break;
			case 'u':
				printf("Setting unknown bits.\n");
				unknown = 1;
				break;
			case 'h':
			case '?':
			default:
				fprintf(stderr,
					"Stereo Tool key generator\n"
					"\n"
					"Usage: %s [ -n name ] [ -f features (hex) ] [ -u ]\n"
					"\n"
					"\t-n name\t\tName of key (default name: %s)\n"
					"\t-f features\tRegistered options in hexadecimal\n"
					"\t-u\t\tSet unknown bits in key check (needed for some names)\n"
					"\n",
				argv[0], DEFAULT_NAME);
				return 1;
		}
	}

	if (default_name) {
		strcpy(name, DEFAULT_NAME);
		printf("Using default name \"%s\".\n", DEFAULT_NAME);
	}

	name_len = strlen(name);

	// input validation
	if (name_len < 5) {
		fprintf(stderr, "Name must be at least 5 characters long.\n");
		return 1;
	}

	if (default_features) {
		printf("Using default features (0x%08x)\n", features);
	}

	dump_bit32(features);

	// 15 = the stuff before and after the key (112233445566778899<name>aabbccddeeff)
	key_len = name_len + 15;

	// the locations of the important parts
	unsigned char *key_features	= key + 1; // licensed features
	unsigned char *key_checksum	= key + 5; // not sure how this is calculated
	unsigned char *key_name		= key + 9; // display name
	unsigned char *key_trailer	= key_name + name_len + 1; // extra stuff

	key[0] = 255; // doesn't seem to affect anything

	// registered options
	memcpy(key_features, &features, sizeof(int));

	// copy name to key
	memcpy(key_name, name, name_len);

	// make sure we don't try to divide by 0
	if ((key_name[2] - key_name[3]) + 1 == 0) {
		// we can't divide by 0
		fprintf(stderr, "Invalid name.\n");
		return 1;
	}

	key_trailer[0] = (((key_name[0] | key_name[1]) ^ ((key_name[2] | key_name[3]) + key_name[4])) & 0xf) << 4;
	key_trailer[0] |= (key_name[0] ^ key_name[1] ^ key_name[2] ^ key_name[3] ^ key_name[4]) & 0xf;
	key_trailer[1] = (((key_name[0] * key_name[1]) / ((key_name[2] - key_name[3]) + 1) - key_name[4]) & 0xf) << 4;
	key_trailer[1] |= ((key_name[0] * key_name[1]) / ((key_name[2] - key_name[3]) + 1) * key_name[4]) & 0xf;
	key_trailer[2] = ((key_name[2] - key_name[3]) * (key_name[0] + key_name[1]) ^ key_name[4]) & 0xf;
	key_trailer[2] |= (((key_name[2] + key_name[3]) * (key_name[0] - key_name[1]) ^ ~key_name[4]) & 0xf) << 4;
	key_trailer[3] = unknown ? UNKNOWN_BITS : 0; // how is this calculated?

	// calculate the checksum
	checksum = 0;
	for (int i = 0; i < key_len; i++) {
		checksum = key[i] * 0x11121 + (checksum << 3);
		checksum += checksum >> 26;
	}

	// copy the checksum
	memcpy(key_checksum, &checksum, sizeof(int));

	// encode the key
	char tmp1, tmp2;
	for (int i = 0; i < key_len; i++) {
		tmp1 = key[i] ^ ((-1 - i) - (1 << (1 << (i & 31) & 7)));
		tmp2 = 0;
		for (int j = 0; j < 8; j++) {
			tmp2 <<= 1;
			tmp2 |= tmp1 & 1;
			tmp1 >>= 1;
		}
		key[i] = tmp2;
	}

	out_key[0] = '<';

	for (int i = 0; i < key_len; i++) {
		sprintf(out_key+i*2+1, "%02x", key[i]);
	}

	out_key[key_len*2+1] = '>';

	printf("%s\n", out_key);

	return 0;
}
