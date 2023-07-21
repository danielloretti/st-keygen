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
#include <stdlib.h>

/* max name length */
#define MAXLEN		108

/* known features */
#define FEATURE_FM_STANDARD	(0x00000001 | 0x00000008)
#define FEATURE_ADV_CLIPPER	0x00000002
#define UNKNOWN_1		0x00000004
#define FEATURE_ADVANCED_RDS	0x00000010
#define UNKNOWN_2		0x00000020
#define FEATURE_FILE_POLLING	0x00000040
#define FEATURE_LOW_LAT_MON	0x00000080
#define UNKNOWN_3		0x00000100
#define UNKNOWN_4		0x00000200
#define UNKNOWN_5		0x00000400
#define FEATURE_DECLIPPER	0x00000800 /* also enables nat dynamics */
#define FEATURE_DECLIPPER_2H	0x00001000 /* also enables nat dynamics */
#define UNKNOWN_6		0x00002000
#define FEATURE_NAT_DYN_ONLY	0x00004000 /* natural dynamics only */
#define FEATURE_FM_EVENT	0x00008000
#define FEATURE_FM_PRO		0x00010000
#define FEATURE_FM_PRO_EVENT	0x00020000
#define FEATURE_DELOSSIFIER	0x00040000
#define FEATURE_UMPX		0x00080000 /* disabled when FM and this are set */
#define UNKNOWN_7		0x00100000
#define FEATURE_AGC34_AEQ	0x00200000
#define FEATURE_DYN_SPEEDS	0x00400000 /* AGC3/4 & Auto EQ needed */
#define FEATURE_BIMP		0x00800000
#define FEATURE_UMPX_SFN_GPS	0x01000000
#define UNKNOWN_8		0x02000000
#define UNKNOWN_9		0x04000000
#define FEATURE_STE_PROC	0x08000000 /* ST-Enterprise */
#define FEATURE_UMPXP		0x10000000 /* disabled when FM and this are set */
#define UNKNOWN_10		0x20000000
#define FEATURE_PPM_WTRMRKNG	0x40000000
#define UNKNOWN_11		0x80000000

/* The following bits are not known or not assigned yet:
 *
 * Some have reported hearing the unregistered message after a period of time
 * has passed. It is not known which of these extra bits are needed to
 * supress the message. See the issues list for more information.
 */
#define UNUSED_BITS	( \
			UNKNOWN_1 | UNKNOWN_2 | UNKNOWN_3 | \
			UNKNOWN_4 | UNKNOWN_5 | UNKNOWN_6 | \
			UNKNOWN_7 | UNKNOWN_8 | UNKNOWN_9 | \
			UNKNOWN_10 | UNKNOWN_11 \
			)


static void show_features(unsigned int feat) {
#define SHOW_FEATURE(a, b) \
	if ((feat & a) == a) \
		printf(" * (0x%08x) " b "\n", a);

#define SHOW_FEATURE_MUT_EX(a, b, c) \
	if ((feat & a) || (feat & b)) \
		printf((feat & b) == b ? \
			" * (0x%08x) " c " disabled\n" : \
			" * (0x%08x) " c "\n", (feat & b) == b ? a : b);

#define SHOW_FEATURE_ONLY(a, b, c) \
	if ((feat & b) && !(feat & a)) \
		printf(" * (0x%08x) " c " only\n", a);

#define SHOW_FEATURE_COND(a, b, c) \
	if (feat & a) \
		printf((feat & b) == b ? \
			" * (0x%08x) " c "\n" : \
			" * (0x%08x) " c " disabled\n", a);

/* for Dehummer only */
#define SHOW_FEATURE_ALWAYS(a) \
	if (feat) \
		printf(" * (  always  ) " a "\n");

#define SHOW_FEATURE_UNKNOWN(a) \
	if (feat & a) \
		printf(" * (0x%08x) Unknown\n", a);

	printf("License: 0x%08x\n", feat);
	SHOW_FEATURE_ALWAYS(			"Dehummer");
	SHOW_FEATURE(FEATURE_FM_STANDARD,	"FM Processing");
	SHOW_FEATURE(FEATURE_ADV_CLIPPER,	"Advanced Clipper");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_1);
	SHOW_FEATURE(FEATURE_ADVANCED_RDS,	"Advanced RDS");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_2);
	SHOW_FEATURE(FEATURE_FILE_POLLING,	"File Polling");
	SHOW_FEATURE(FEATURE_LOW_LAT_MON,	"Low Latency Monitoring");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_3);
	SHOW_FEATURE_UNKNOWN(UNKNOWN_4);
	SHOW_FEATURE_UNKNOWN(UNKNOWN_5);
	SHOW_FEATURE(FEATURE_DECLIPPER,		"Declipper & Natural Dynamics");
	SHOW_FEATURE(FEATURE_DECLIPPER_2H,	"Declipper (2 hours)");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_6);
	SHOW_FEATURE_ONLY(FEATURE_DECLIPPER,
			FEATURE_NAT_DYN_ONLY,	"Natural Dynamics");
	SHOW_FEATURE(FEATURE_FM_EVENT,		"Event FM Processing (3 days)");
	SHOW_FEATURE(FEATURE_FM_PRO,		"FM Professional");
	SHOW_FEATURE(FEATURE_FM_PRO_EVENT,	"FM Professional (Event FM)");
	SHOW_FEATURE(FEATURE_DELOSSIFIER,	"Delossifier");
	SHOW_FEATURE_MUT_EX(FEATURE_FM_STANDARD,
			FEATURE_UMPX,		"uMPX");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_7);
	SHOW_FEATURE(FEATURE_AGC34_AEQ,		"Advanced Dynamics (3/4 AGC & Auto EQ)");
	SHOW_FEATURE_COND(FEATURE_DYN_SPEEDS,
			FEATURE_AGC34_AEQ,	"Advanced Dynamics (Dynamic Speeds)");
	SHOW_FEATURE(FEATURE_BIMP,		"BIMP");
	SHOW_FEATURE(FEATURE_UMPX_SFN_GPS,	"uMPX SFN GPS");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_8);
	SHOW_FEATURE_UNKNOWN(UNKNOWN_9);
	SHOW_FEATURE(FEATURE_STE_PROC,		"ST-Enterprise");
	SHOW_FEATURE_MUT_EX(FEATURE_FM_STANDARD,
			FEATURE_UMPXP,		"uMPX+");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_10);
	SHOW_FEATURE(FEATURE_PPM_WTRMRKNG,	"Nielsen PPM watermarking");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_11);
}

static char ascii2nibble(char ascii) {
	char nibble = ascii;

	if (ascii >= '0' && ascii <= '9')
		nibble -= '0';
	else if (ascii >= 'a' && ascii <= 'f')
		nibble -= 'a' - 0xa;
	else
		nibble = 0;

	return nibble;
}

static void descramble(unsigned char *key, size_t length) {
	unsigned char in, out;

	for (size_t i = 0; i < length; i++) {
		in = key[i];
		out = 0;
		for (int j = 0; j < 8; j++) {
			out <<= 1;
			out |= in & 1;
			in >>= 1;
		}
		key[i] = out ^ (-1 - i - (1 << (1 << (i & 31) & 7)));
	}
}

static int calc_checksum(unsigned char *key, size_t length) {
	int checksum = 0;

	for (unsigned int i = 0; i < length; i++) {
		checksum = key[i] * 0x11121 + (checksum << 3);
		checksum += checksum >> 26;
	}
	return checksum;
}

int main(int argc, char *argv[]) {
	unsigned char key[9+MAXLEN+1+8];
	char *key_ascii;
	int features;
	int key_checksum, checksum;
	unsigned char *key_name;
	unsigned char *key_trailer;
	int key_len;
	int name_len;
	int i, j;

	if (argc < 2) {
		printf("Usage: %s '<key>'\n", argv[0]);
		return 1;
	}

	key_ascii = argv[1];

	/* convert key ASCII to bytes */
	i = j = 0;
	while (key_ascii[i] != 0 && i < ((9 + MAXLEN + 1 + 8) * 2) + 2) {
		if (key_ascii[i] == '<') key_ascii++;
		if (key_ascii[i] == '>') break;

		if (i % 2) {
			key[j] = ascii2nibble(key_ascii[i - 1]);
			key[j] <<= 4;
			key[j] |= ascii2nibble(key_ascii[i]);
			j++;
		}
		i++;
	}

	/* get length */
	key_len = j;

	/* get name */
	key_name = key + 9;

	/* unscramble */
	descramble(key, key_len);

	/* get the key feature bitmask */
	memcpy(&features, key + 1, sizeof(int));

	/* get checksum */
	memcpy(&key_checksum, key + 5, sizeof(int));

	/* clear checksum field */
	memset(key + 5, 0, sizeof(int));

	/* calculate checksum */
	checksum = calc_checksum(key, key_len);

	/* determine key name's length */
	i = 0;
	while (key_name[i] != 0 && i < MAXLEN) i++;
	name_len = i;

	key_trailer = key_name + name_len + 1;

	/* output results */
	printf("\n");
	printf("==========================================\n");
	printf("Name\t\t: %s\n", key_name);
	printf("Features\t: 0x%08x\n", features);
	printf("Calc'd checksum\t: 0x%08x\n", checksum);
	printf("Key's checksum\t: 0x%08x\n", key_checksum);
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
}
