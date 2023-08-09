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
#include <stdlib.h>
#include "st-common.h"

int main(int argc, char *argv[]) {
	unsigned char key[9 + MAXLEN + 1 + 8];
	char *key_ascii;
	int features;
	int key_checksum, checksum;
	unsigned char *key_name;
	unsigned char *key_trailer;
	unsigned char calc_key_trailer[8];
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

	/* calculate key trailer */
	calc_name_check(calc_key_trailer, (char *)key_name);

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
	printf("Calc'd tr. bytes: "
		"0x%02x 0x%02x 0x%02x 0x%02x\n"
		"\t\t  0x%02x 0x%02x 0x%02x 0x%02x\n",
		calc_key_trailer[0], calc_key_trailer[1],
		calc_key_trailer[2], calc_key_trailer[3],
		calc_key_trailer[4], calc_key_trailer[5],
		calc_key_trailer[6], calc_key_trailer[7]);
	printf("==========================================\n");
	printf("\n");
	show_features(features);
	printf("\n");

	return 0;
}
