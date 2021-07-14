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
#define FEATURE_1	0xff
#define FEATURE_2	0xfff7bfff

int main(int argc, char *argv[]) {
	int opt;
	unsigned char features_1 = FEATURE_1;
	unsigned int features_2 = FEATURE_2;
	int name_len;
	int default_name = 1;
	int default_features = 1;
	int key_len;
	// key checksum
        int checksum;
	char name[MAXLEN+1];
	char key[(MAXLEN+1)*4];
	char out_key[(MAXLEN+1)*4];
	// hex representation of one byte (2 chars + terminator)
	char out_key_byte[3];

	const char *short_opt = "n:f:h";
	const struct option long_opt[] = {
		{"name",	required_argument,	NULL,	'n'},
		{"features",	required_argument,	NULL,	'f'},
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
				features_2 = strtoul(optarg, NULL, 16);
				printf("Using custom feature config (0x%08x)\n", features_2);
				default_features = 0;
				break;
			case 'h':
			case '?':
			default:
				fprintf(stderr,
					"Stereo Tool key generator\n"
					"\n"
					"Usage: %s [ -n name ] [ -f features ]\n"
					"\n",
				argv[0]);
				return 1;
		}
	}

	if (default_name) {
		strcpy(name, DEFAULT_NAME);
		printf("Using default name \"%s\".\n", DEFAULT_NAME);
	}

	// input validation
	name_len = strlen(name);

	if (name_len < 5) {
		fprintf(stderr, "Name must be at least 5 characters long.\n");
		return 1;
	}

	if (default_features) {
		printf("Using default features (0x%08x)\n", features_2);
	}

	// 15 = the stuff before and after the key (112233445566778899<name>aabbccddeeff)
	key_len = name_len + 15;

	// the locations of the important parts
	char *key_features	= key + 1; // licensed features
	char *key_checksum	= key + 5; // not sure how this is calculated
	char *key_name		= key + 9; // display name
	char *key_trailer	= key_name + name_len; // extra stuff

	// registered options
	key[0] = features_1;
	memcpy(key_features, &features_2, sizeof(int));

	// copy name to key
	memcpy(key_name, name, name_len);

	// make sure we don't try to divide by 0
	if ((key_name[2] - key_name[3]) + 1 == 0) {
		// we can't divide by 0
		fprintf(stderr, "Invalid name.\n");
		return 1;
	}

	key_trailer[1] = (((key_name[0] | key_name[1]) ^ ((key_name[2] | key_name[3]) + key_name[4])) & 0xf) << 4;
	key_trailer[1] |= (key_name[0] ^ key_name[1] ^ key_name[2] ^ key_name[3] ^ key_name[4]) & 0xf;
	key_trailer[2] = (((key_name[0] * key_name[1]) / ((key_name[2] - key_name[3]) + 1) - key_name[4]) & 0xf) << 4;
	key_trailer[2] |= ((key_name[0] * key_name[1]) / ((key_name[2] - key_name[3]) + 1) * key_name[4]) & 0xf;
	key_trailer[3] = ((key_name[2] - key_name[3]) * (key_name[0] + key_name[1]) ^ key_name[4]) & 0xf;
	key_trailer[3] |= (((key_name[2] + key_name[3]) * (key_name[0] - key_name[1]) ^ ~key_name[4]) & 0xf) << 4;

	int uVar8 = 192; // how is this calculated?
	int bVar3 = uVar8;
	if ((((key_name[0] + key_name[1] + key_name[2] - key_name[3] + key_name[4]) & 0xf) ^ uVar8) & 8) {
		bVar3 ^= 8;
	}

	key_trailer[4] = bVar3;

	key_trailer[5] = ((key_name[0] + key_name[1] - key_name[2]) - (key_name[3] + key_name[4])) & 0xf;
	key_trailer[5] |= 15 << 4;

	checksum = 0;

	// calculate the checksum
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
		sprintf(out_key_byte, "%02x", key[i]);
		strcat(out_key, out_key_byte);
	}

	out_key[key_len*2+1] = '>';

	printf("%s\n", out_key);

	return 0;
}
