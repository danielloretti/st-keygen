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

/*#define DUMP_BITS*/

/* max name length */
#define MAXLEN		108

/* uncomment for event FM license (3 days) */
/*#define EVENT_FM*/

/* known features */
#define FEATURE_FM_PROC		(0x00000001 | 0x00000004 | 0x00000008)
#define FEATURE_ADV_CLIPPER	0x00000002
#define FEATURE_ADVANCED_RDS	0x00000010
#define FEATURE_FILE_POLLING	0x00000040
#define FEATURE_LOW_LAT_MON	0x00000080
#define FEATURE_DECLIPPER	0x00000800 /* also enables nat dynamics */
#define FEATURE_DECLIPPER_2H	0x00001000 /* also enables nat dynamics */
#define FEATURE_NAT_DYN_ONLY	0x00004000 /* natural dynamics only */
#define FEATURE_EVENT_FM_PROC	0x00008000
#define FEATURE_COMP_CLIP	0x00010000
#define FEATURE_COMP_CLIP_EVENT	0x00020000
#define FEATURE_DELOSSIFIER	0x00040000
#define FEATURE_UMPX		0x00080000 /* disabled when FM and this are set */
#define FEATURE_AGC34_AEQ	0x00200000
#define FEATURE_DYN_SPEEDS	0x00400000
#define FEATURE_BIMP		0x00800000
#define FEATURE_UMPX_SFN_GPS	0x01000000
#define FEATURE_UMPXP		0x10000000 /* disabled when FM and this are set */
#define FEATURE_PPM_WTRMRKNG	0x40000000

/* ST-Enterprise */
#define STE_PROC		0x08000000

#ifdef EVENT_FM
#define FEATURE_FM	FEATURE_EVENT_FM_PROC | \
			FEATURE_ADVANCED_RDS | \
			FEATURE_COMP_CLIP | \
			FEATURE_COMP_CLIP_EVENT | \
			FEATURE_UMPX_SFN_GPS | \
			FEATURE_PPM_WTRMRKNG
#else
#define FEATURE_FM	FEATURE_FM_PROC | \
			FEATURE_ADVANCED_RDS | \
			FEATURE_COMP_CLIP | \
			FEATURE_UMPX_SFN_GPS | \
			FEATURE_PPM_WTRMRKNG
#endif

/* feature mask */
#define FEATURES	FEATURE_ADV_CLIPPER | FEATURE_FILE_POLLING | \
			FEATURE_LOW_LAT_MON | FEATURE_FM | \
			FEATURE_DECLIPPER | FEATURE_DELOSSIFIER | \
			FEATURE_AGC34_AEQ | FEATURE_DYN_SPEEDS | \
			FEATURE_BIMP | STE_PROC

#ifdef DUMP_BITS
static void dump_bit32(unsigned int value) {
	for (int i = 0; i < 8; i++) {
		printf(" %d | %d %d %d %d\n", 7 - i,
			value >> (31 - (i * 4 + 0)) & 1,
			value >> (31 - (i * 4 + 1)) & 1,
			value >> (31 - (i * 4 + 2)) & 1,
			value >> (31 - (i * 4 + 3)) & 1);
	}
}
#endif

static void show_features(unsigned int feat) {
#define SHOW_FEATURE(a, b) \
	if ((feat & a) == a) \
		printf("\t* " b "\n");

#define SHOW_FEATURE_INVERSE(a, b, c) \
	if (feat & a) \
		printf((feat & b) == b ? "\t* " c " disabled\n" : "\t* " c "\n");

#define SHOW_FEATURE_ONLY(a, b, c) \
	if (!(feat & a)) \
		printf((feat & b) == b ? "\t* " c " only\n" : "\t* " c "\n");

	printf("License: 0x%08x\n", feat);
	if (feat) printf("\t* Dehummer\n");
	SHOW_FEATURE(FEATURE_FM_PROC,		"FM Processing");
	SHOW_FEATURE(FEATURE_ADV_CLIPPER,	"Advanced Clipper");
	SHOW_FEATURE(FEATURE_ADVANCED_RDS,	"Advanced RDS");
	SHOW_FEATURE(FEATURE_FILE_POLLING,	"File Polling");
	SHOW_FEATURE(FEATURE_LOW_LAT_MON,	"Low Latency Monitoring");
	SHOW_FEATURE(FEATURE_DECLIPPER,		"Declipper & Natural Dynamics");
	SHOW_FEATURE(FEATURE_DECLIPPER_2H,	"Declipper (2 hour limit)");
	SHOW_FEATURE_ONLY(FEATURE_DECLIPPER, FEATURE_NAT_DYN_ONLY,	"Natural Dynamics");
	SHOW_FEATURE(FEATURE_EVENT_FM_PROC,	"Event FM (3 days)");
	SHOW_FEATURE(FEATURE_COMP_CLIP,		"Composite Clipper");
	SHOW_FEATURE(FEATURE_COMP_CLIP_EVENT,	"Composite Clipper (Event FM)");
	SHOW_FEATURE(FEATURE_DELOSSIFIER,	"Delossifier");
	SHOW_FEATURE_INVERSE(FEATURE_FM_PROC,	FEATURE_UMPX,	"uMPX");
	SHOW_FEATURE(FEATURE_AGC34_AEQ,		"3/4 AGC & Auto EQ");
	SHOW_FEATURE(FEATURE_DYN_SPEEDS,	"Dynamic Speeds");
	SHOW_FEATURE(FEATURE_BIMP,		"BIMP");
	SHOW_FEATURE(FEATURE_UMPX_SFN_GPS,	"uMPX SFN GPS");
	SHOW_FEATURE_INVERSE(FEATURE_FM_PROC,	FEATURE_UMPXP,	"uMPX+");
	SHOW_FEATURE(FEATURE_PPM_WTRMRKNG,	"Nielsen PPM watermarking");
	SHOW_FEATURE(STE_PROC,			"ST-Enterprise");
}

static void scramble(char *key, size_t length) {
	char in, out;

	for (size_t i = 0; i < length; i++) {
		in = key[i] ^ (-1 - i - (1 << (1 << (i & 31) & 7)));
		out = 0;
		for (int j = 0; j < 8; j++) {
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
	/* key checksum */
	int checksum;
	char name[MAXLEN+1];
	char key[9+MAXLEN+1+8];
	char out_key_text[(9+MAXLEN+1+8)*2];
	char key_trailer_plain[8];

	char *key_features;
	char *key_checksum;
	char *key_name;
	char *key_trailer;

	const char *short_opt = "f:h";
	const struct option long_opt[] = {
		{"features",	required_argument,	NULL,	'f'},
		{"help",	no_argument,		NULL,	'h'},
		{0,		0,			0,	0}
	};

keep_parsing_opts:

	opt = getopt_long(argc, argv, short_opt, long_opt, NULL);
	if (opt == -1) goto done_parsing_opts;

	switch (opt) {
		case 'f':
			features = strtoul(optarg, NULL, 16);
			break;

		case 'h':
		case '?':
		default:
			fprintf(stderr,
				"Stereo Tool key generator\n"
				"\n"
				"Generates a valid registration key for a given name\n"
				"\n"
				"Usage: %s [ -f features (hex) ] NAME\n"
				"\n"
				"\t-f features\tRegistered options in hexadecimal (optional)\n"
				"\n",
			argv[0]);
			return 1;
	}

	goto keep_parsing_opts;

done_parsing_opts:

	if (optind < argc) {
		name_len = strlen(argv[optind]);
		if (name_len > MAXLEN) {
			printf("Name is too long.\n");
			return 1;
		}
		strncat(name, argv[optind], MAXLEN);
	}

	if (!name[0]) {
		printf("Please specify a name.\n");
		return 1;
	}

	/* input validation */

	/* needed for name validation */
	if (name_len < 5) {
		printf("Name must be at least 5 characters long.\n");
		return 1;
	}

	/* make sure we don't try to divide by 0 */
	if ((name[2] - name[3]) + 1 == 0) {
		/* we can't divide by 0 */
		printf("Invalid name.\n");
		return 1;
	}

#ifdef DUMP_BITS
	dump_bit32(features);
#endif

	/*
	 * 18 = the stuff before and after the key (112233445566778899<name>00aabbccddeeffaabb)
	 * 14 (9 + name_len + 1 + 4) is the bare minimum
	 */
	key_len = 9 + name_len + 1 /* null terminator for name string */ + 8;

	/* the locations of the important parts */
	key_features	= key + 1; /* licensed features */
	key_checksum	= key + 5;
	key_name	= key + 9; /* display name */
	key_trailer	= key_name + name_len + 1; /* name validation */

	key[0] = 1; /* doesn't seem to affect anything */

	/* registered options */
	memcpy(key_features, &features, sizeof(int));

	/* copy name to key */
	memcpy(key_name, name, name_len);

	/* add terminator */
	(key_name + name_len)[0] = 0;

	/* the algorithm as found on ghidra */
	key_trailer_plain[0] = (((name[0] | name[1]) ^ ((name[2] | name[3]) + name[4])) & 0xf) << 4;
	key_trailer_plain[0] |= (name[0] ^ name[1] ^ name[2] ^ name[3] ^ name[4]) & 0xf;
	key_trailer_plain[1] = (((name[0] * name[1]) / ((name[2] - name[3]) + 1) - name[4]) & 0xf) << 4;
	key_trailer_plain[1] |= ((name[0] * name[1]) / ((name[2] - name[3]) + 1) * name[4]) & 0xf;
	key_trailer_plain[2] = (((name[2] + name[3]) * (name[0] - name[1]) ^ ~name[4]) & 0xf) << 4;
	key_trailer_plain[2] |= ((name[2] - name[3]) * (name[0] + name[1]) ^ name[4]) & 0xf;
	key_trailer_plain[3] = ((((name[0] ^ name[1]) + (name[2] ^ name[3])) ^ name[4]) & 0xf) << 4;
	key_trailer_plain[3] |= (name[0] + name[1] + name[2] - name[3] - name[4]) & 0xf;
	/* these are not determined yet */
	key_trailer_plain[4] = 0;
	key_trailer_plain[5] = 0;
	key_trailer_plain[6] = 0;
	key_trailer_plain[7] = 0;

	/* copy the key trailer before scrambling */
	memcpy(key_trailer, key_trailer_plain, 8);

	/* clear checksum field */
	memset(key_checksum, 0, sizeof(int));

	/* calculate the checksum */
	checksum = 0;
	for (int i = 0; i < key_len; i++) {
		checksum = key[i] * 0x11121 + (checksum << 3);
		checksum += checksum >> 26;
	}

	/* copy the checksum */
	memcpy(key_checksum, &checksum, sizeof(int));

	/* scramble the key */
	scramble(key, key_len);

	for (int i = 0; i < key_len; i++)
		sprintf(out_key_text+i*2, "%02x", key[i]);

	/* output */
	printf("\n");
	printf("==========================================\n");
	printf("Name\t\t: %s\n", name);
	printf("Features\t: 0x%08x\n", features);
	printf("Calc'd checksum\t: 0x%08x\n", checksum);
	printf("Trailing bytes\t: "
		"%02x %02x %02x %02x %02x %02x %02x %02x\n",
		key_trailer_plain[0],
		key_trailer_plain[1],
		key_trailer_plain[2],
		key_trailer_plain[3],
		key_trailer_plain[4],
		key_trailer_plain[5],
		key_trailer_plain[6],
		key_trailer_plain[7]
	);
	printf("==========================================\n");
	printf("\n");
	show_features(features);
	printf("\n");
	printf("<%s>\n", out_key_text);
	printf("\n");

	return 0;
}
