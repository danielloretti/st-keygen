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

//#define DUMP_BITS

// max name length
#define MAXLEN		64

// the default name to use when none is specified
#define DEFAULT_NAME	"Akira Kurosawa"

// feature mask
#define FEATURES	0x00e788d3
// some bits need to be clear for composite clipper to be activated properly

#ifdef DUMP_BITS
static void dump_bit32(unsigned int value) {
	char bitbuf[32] = "00000000000000000000000000000000";
	for (int i = 0; i < 32; i++) {
		if ((value >> (31 - i)) & 1) bitbuf[i] = '1';
	}
	printf("%08x: %s\n", value, bitbuf);
}
#endif

int main(int argc, char *argv[]) {
	int opt;
	unsigned int features = FEATURES;
	int name_len;
	int default_name = 1;
	int default_features = 1;
	int key_len;
	// key checksum
	int checksum;
	char name[MAXLEN+1];
	unsigned char key[(MAXLEN+1)*4];
	char out_key[(MAXLEN+1)*4];

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
				features = strtoul(optarg, NULL, 16);
				printf("Using custom feature config (0x%08x)\n", features);
				default_features = 0;
				break;
			case 'h':
			case '?':
			default:
				fprintf(stderr,
					"Stereo Tool key generator\n"
					"\n"
					"Usage: %s [ -n name ] [ -f features (hex) ]\n"
					"\n"
					"\t-n name\t\tName of key (default name: %s)\n"
					"\t-f features\tRegistered options in hexadecimal\n"
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

	// needed for name validation
	if (name_len < 5) {
		fprintf(stderr, "Name must be at least 5 characters long.\n");
		return 1;
	}

	// make sure we don't try to divide by 0
	if ((name[2] - name[3]) + 1 == 0) {
		// we can't divide by 0
		fprintf(stderr, "Invalid name.\n");
		return 1;
	}

	if (default_features) {
		printf("Using default features (0x%08x)\n", features);
	}

#ifdef DUMP_BITS
	dump_bit32(features);
#endif

	/*
	 * 18 = the stuff before and after the key (112233445566778899<name>00aabbccddeeffaabb)
	 * 14 (9 + name_len + 1 + 4) is the bare minimum
	 */
	key_len = 9 + name_len + 1 /* null terminator for name string */ + 8;

	// the locations of the important parts
	unsigned char *key_features	= key + 1; // licensed features
	unsigned char *key_checksum	= key + 5;
	unsigned char *key_name		= key + 9; // display name
	unsigned char *key_trailer	= key_name + name_len + 1; // name validation

	key[0] = 1; // doesn't seem to affect anything

	// registered options
	memcpy(key_features, &features, sizeof(int));

	// copy name to key
	memcpy(key_name, name, name_len);

	// the algorithm as found on ghidra
	key_trailer[0] = (((name[0] | name[1]) ^ ((name[2] | name[3]) + name[4])) & 0xf) << 4;
	key_trailer[0] |= (name[0] ^ name[1] ^ name[2] ^ name[3] ^ name[4]) & 0xf;
	key_trailer[1] = (((name[0] * name[1]) / ((name[2] - name[3]) + 1) - name[4]) & 0xf) << 4;
	key_trailer[1] |= ((name[0] * name[1]) / ((name[2] - name[3]) + 1) * name[4]) & 0xf;
	key_trailer[2] = (((name[2] + name[3]) * (name[0] - name[1]) ^ ~name[4]) & 0xf) << 4;
	key_trailer[2] |= ((name[2] - name[3]) * (name[0] + name[1]) ^ name[4]) & 0xf;
	key_trailer[3] = ((((name[0] ^ name[1]) + (name[2] ^ name[3])) ^ name[4]) & 0xf) << 4;
	key_trailer[3] |= (name[0] + name[1] + name[2] - name[3] - name[4]) & 0xf;
#if 0 // these are not determined yet
	key_trailer[4] = 0;
	key_trailer[5] = 0;
	key_trailer[6] = 0;
	key_trailer[7] = 0;
#endif

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

	// output
	out_key[0] = '<';

	for (int i = 0; i < key_len; i++) {
		sprintf(out_key+i*2+1, "%02x", key[i]);
	}

	out_key[key_len*2+1] = '>';

	printf("%s\n", out_key);

	return 0;
}
