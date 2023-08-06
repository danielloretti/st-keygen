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

/* max name length */
#define MAXLEN		108

/* known features */
#define FEATURE_FM_STANDARD	(0x00000001 | 0x00000004 | 0x00000008)
#define FEATURE_ADV_CLIPPER	0x00000002
#define FEATURE_ADVANCED_RDS	0x00000010
#define UNKNOWN_1		0x00000020
#define FEATURE_FILE_POLLING	0x00000040
#define FEATURE_LOW_LAT_MON	0x00000080
#define UNKNOWN_2		0x00000100
#define UNKNOWN_3		0x00000200
#define UNKNOWN_4		0x00000400
#define FEATURE_DECLIPPER	0x00000800 /* also enables nat dynamics */
#define FEATURE_DECLIPPER_2H	0x00001000 /* also enables nat dynamics */
#define UNKNOWN_5		0x00002000
#define FEATURE_NAT_DYN		0x00004000 /* not needed if Declipper is set */
#define FEATURE_FM_EVENT	0x00008000
#define FEATURE_FM_PRO		0x00010000
#define FEATURE_FM_PRO_EVENT	0x00020000
#define FEATURE_DELOSSIFIER	0x00040000
#define FEATURE_UMPX		0x00080000 /* disabled when FM and this are set */
#define UNKNOWN_6		0x00100000
#define FEATURE_ADV_DYNAMICS	0x00200000
#define FEATURE_DYN_SPEEDS	0x00400000 /* Dynamic Speeds (Advanced Dynamics) */
#define FEATURE_BIMP		0x00800000
#define FEATURE_UMPX_SFN_GPS	0x01000000
#define UNKNOWN_7		0x02000000
#define UNKNOWN_8		0x04000000
#define FEATURE_STE_PROC	0x08000000 /* ST-Enterprise */
#define FEATURE_UMPXP		0x10000000 /* disabled when FM and this are set */
#define UNKNOWN_9		0x20000000
#define FEATURE_PPM_WTRMRKNG	0x40000000
#define UNKNOWN_10		0x80000000

/*
 * The following bits are not known or not assigned yet:
 *
 */
#define UNUSED_BITS	( \
			UNKNOWN_1 | UNKNOWN_2 | UNKNOWN_3 | \
			UNKNOWN_4 | UNKNOWN_5 | UNKNOWN_6 | \
			UNKNOWN_7 | UNKNOWN_8 | UNKNOWN_9 | \
			UNKNOWN_10 \
			)

#if EVENT_FM
#define FEATURE_FM	FEATURE_FM_EVENT | \
			FEATURE_ADVANCED_RDS | \
			FEATURE_FM_PRO | \
			FEATURE_FM_PRO_EVENT | \
			FEATURE_UMPX_SFN_GPS | \
			FEATURE_PPM_WTRMRKNG
#else
#define FEATURE_FM	FEATURE_FM_STANDARD | \
			FEATURE_ADVANCED_RDS | \
			FEATURE_FM_PRO | \
			FEATURE_UMPX_SFN_GPS | \
			FEATURE_PPM_WTRMRKNG
#endif

/* feature mask */
#define FEATURES	FEATURE_ADV_CLIPPER | FEATURE_FILE_POLLING | \
			FEATURE_LOW_LAT_MON | FEATURE_FM | \
			FEATURE_DECLIPPER | FEATURE_DELOSSIFIER | \
			FEATURE_ADV_DYNAMICS | FEATURE_DYN_SPEEDS | \
			FEATURE_BIMP | FEATURE_STE_PROC

static void show_features(unsigned int feat) {
#define SHOW_FEATURE(a, b) \
	if ((feat & a) == a) \
		printf(" * (0x%08x) " b "\n", a);

#define SHOW_FEATURE_MUT_EX(a, b, c) \
	if ((feat & a) || (feat & b)) \
		printf((feat & b) == b ? \
			" * (0x%08x) " c " disabled\n" : \
			" * (0x%08x) " c "\n", (feat & b) == 0 ? a : b);

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
	SHOW_FEATURE(FEATURE_ADVANCED_RDS,	"Advanced RDS");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_1);
	SHOW_FEATURE(FEATURE_FILE_POLLING,	"File Polling");
	SHOW_FEATURE(FEATURE_LOW_LAT_MON,	"Low Latency Monitoring");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_2);
	SHOW_FEATURE_UNKNOWN(UNKNOWN_3);
	SHOW_FEATURE_UNKNOWN(UNKNOWN_4);
	SHOW_FEATURE(FEATURE_DECLIPPER,		"Declipper & Natural Dynamics");
	SHOW_FEATURE(FEATURE_DECLIPPER_2H,	"Declipper & Natural Dynamics (2 hours)");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_5);
	SHOW_FEATURE(FEATURE_NAT_DYN,		"Natural Dynamics (only)");
	SHOW_FEATURE(FEATURE_FM_EVENT,		"Event FM Processing (3 days)");
	SHOW_FEATURE(FEATURE_FM_PRO,		"FM Professional");
	SHOW_FEATURE(FEATURE_FM_PRO_EVENT,	"FM Professional (Event FM)");
	SHOW_FEATURE(FEATURE_DELOSSIFIER,	"Delossifier");
	SHOW_FEATURE_MUT_EX(FEATURE_FM_PRO,
			FEATURE_UMPX,		"uMPX");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_6);
	SHOW_FEATURE(FEATURE_ADV_DYNAMICS,	"Advanced Dynamics");
	SHOW_FEATURE(FEATURE_DYN_SPEEDS,	"Dynamic Speeds (Advanced Dynamics)");
	SHOW_FEATURE(FEATURE_BIMP,		"BIMP");
	SHOW_FEATURE(FEATURE_UMPX_SFN_GPS,	"uMPX SFN GPS");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_7);
	SHOW_FEATURE_UNKNOWN(UNKNOWN_8);
	SHOW_FEATURE(FEATURE_STE_PROC,		"ST-Enterprise");
	SHOW_FEATURE_MUT_EX(FEATURE_FM_PRO,
			FEATURE_UMPXP,		"uMPX+");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_9);
	SHOW_FEATURE(FEATURE_PPM_WTRMRKNG,	"Nielsen PPM watermarking");
	SHOW_FEATURE_UNKNOWN(UNKNOWN_10);
}

static int calc_checksum(unsigned char *key, int length) {
	int checksum = 0;
	int i;

	for (i = 0; i < length; i++) {
		checksum = key[i] * 0x11121 + (checksum << 3);
		checksum += checksum >> 26;
	}
	return checksum;
}

static void calc_name_check(unsigned char *trailer, char *name) {
	/* the algorithm as found on ghidra */
	trailer[0] = (((name[0] | name[1]) ^ ((name[2] | name[3]) + name[4])) & 0xf) << 4;
	trailer[0] |= (name[0] ^ name[1] ^ name[2] ^ name[3] ^ name[4]) & 0xf;
	trailer[1] = (((name[0] * name[1]) / ((name[2] - name[3]) + 1) - name[4]) & 0xf) << 4;
	trailer[1] |= ((name[0] * name[1]) / ((name[2] - name[3]) + 1) * name[4]) & 0xf;
	trailer[2] = (((name[2] + name[3]) * (name[0] - name[1]) ^ ~name[4]) & 0xf) << 4;
	trailer[2] |= ((name[2] - name[3]) * (name[0] + name[1]) ^ name[4]) & 0xf;
	trailer[3] = ((((name[0] ^ name[1]) + (name[2] ^ name[3])) ^ name[4]) & 0xf) << 4;
	trailer[3] |= (name[0] + name[1] + name[2] - name[3] - name[4]) & 0xf;
	trailer[4] = (((name[0] + name[1]) - (name[2] + name[3])) - name[4]) & 0xf;
	trailer[4] |= (((name[2] & name[3]) ^ (name[0] & name[1]) ^ name[4]) & 0xf) << 4;

	/* reserved */
	trailer[5] = 0;
	trailer[6] = 0;
	trailer[7] = 0;
}
