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

/*
 * This generates valid keys for a given name. Generated keys will be
 * recognized, but the unregistered message still plays every 12 hours.
 */

#include <stdio.h>
#include <string.h>

// the default name to use when none is specified
#define DEFAULT_NAME "Akira Kurosawa"

/*
 * <23a06b710bde7407f92df95901090b25d9f9c119d17991e7697284ad40>
 * <30a00368d7df8aad392df95901090b25d9f9c119d17991e76972848447>
 * <49a02768375e833d592df95901090b25d9f9c119d17991e7697284c44e>
 * <b1a02b799fd78161792df95901090b25d9f9c119d17991e7697284894e>
*/

int make_key(char *name, int length, char *out_key) {
	int i, j;

	// hex representation of one byte (2 chars + terminator)
	char key_char_byte[3];

	// needs better names
	char encoded_key_char;
	char xored_key_char;

	// not sure what these are for
	char some_number;
	char some_other_number;

	// make sure we don't try to divide by 0
	int sanity_check;

	// key prefix chars 1-8 (skip 0)
	unsigned int key_prefix_1_4, key_prefix_5_8;

	int key_length;
	char key[100] = {0};

	// the key prefix: one 1-byte int and two 4-byte ints (9 chars)
	key[0] = 255; /* 8 bit unsigned integer */
	key_prefix_1_4 = (0xffffffff /* 32 bit unsigned integer */ & 0xfffdffff) | 0x109af;

	key[1] = ((key_prefix_1_4 & 0x000000ff) >>  0);
	key[2] = ((key_prefix_1_4 & 0x0000ff00) >>  8);
	key[3] = ((key_prefix_1_4 & 0x00ff0000) >> 16);
	key[4] = ((key_prefix_1_4 & 0xff000000) >> 24);

	// the stuff before and after the key (112233445566778899<name>aabbccddeeff)
	key_length = length + 15;

	// copy name to key at offset 9
	for (i = 0; i < length; i++) {
		key[9+i] = name[i];
	}

	sanity_check = (key[9+2] - key[9+3]) + 1;

	if (sanity_check == 0) {
		// we can't divide by 0
		fprintf(stderr, "Invalid name.\n");
		return 1;
	}

	sanity_check = (key[9+0] * key[9+1]) / sanity_check;

	key[9+length+2] = ((sanity_check - key[9+4]) & 0xf) << 4;
	key[9+length+2] |= (sanity_check * key[9+4]) & 0xf;
	key[9+length+5] = ((key[9+0] + key[9+1] - key[9+2]) - (key[9+3] + key[9+4])) & 0xf;
	key[9+length+5] |= 15 /* 4 bit integer */ << 4;
	key[9+length+1] = ((((key[9+0] | key[9+1]) ^ ((key[9+2] | key[9+3]) + key[9+4]))) & 0xf) << 4;
	key[9+length+1] |= (key[9+0] ^ key[9+1] ^ key[9+2] ^ key[9+3] ^ key[9+4]) & 0xf;
	key[9+length+3] = ((key[9+2] - key[9+3]) * (key[9+0] + key[9+1]) ^ key[9+4]) & 0xf;
	key[9+length+3] |= (((key[9+2] + key[9+3]) * (key[9+0] - key[9+1]) ^ ~key[9+4]) & 0xf) << 4;

	some_number = 255; // 8 bit unsigned integer
	some_other_number = some_number;

	if ((((((key[9+0] + key[9+1] + key[9+2]) - (key[9+3] + key[9+4])) & 0xf) ^ some_number) & 8) != 0) {
		some_other_number ^= 8;
	}

	key[9+length+4] = some_other_number;

	key_prefix_5_8 = 0;

	for (i = 0; i < key_length; i++) {
		key_prefix_5_8 = key[9+i] * 0x11121 /* 69921 */ + key_prefix_5_8 * 8;
		key_prefix_5_8 = key_prefix_5_8 + (key_prefix_5_8 >> 0x1a /* 26 */);
	}

	key[5] = ((key_prefix_5_8 & 0x000000ff) >>  0);
	key[6] = ((key_prefix_5_8 & 0x0000ff00) >>  8);
	key[7] = ((key_prefix_5_8 & 0x00ff0000) >> 16);
	key[8] = ((key_prefix_5_8 & 0xff000000) >> 24);

	// encode the key
	for (i = 0; i < key_length; i++) {
		xored_key_char = key[i] ^ ((-1 - i) - (1 << (1 << (i & 0x1f /* 31 */)) & 7));
		encoded_key_char = 0;

		for (j = 0; j < 8; j++) {
			encoded_key_char = encoded_key_char * 2 | (xored_key_char & 1);
			xored_key_char = xored_key_char >> 1;
		}

		key[i] = encoded_key_char;
	}

	out_key[0] = '<';

	for (i = 0; i < key_length; i++) {
		sprintf(key_char_byte, "%.2x", key[i]);
		strcat(out_key, key_char_byte);
	}

	out_key[1+key_length*2] = '>';

	return 0;
}

int main(int argc, char *argv[]) {
	int ret;

	char user_name[50] = {0};
	int user_name_length;

	// registration key
	char reg_key[200] = {0};

	if (argc < 2) {
		strcpy(user_name, DEFAULT_NAME);
		printf("Using default name \"" DEFAULT_NAME "\".\n");
	} else {
		strncpy(user_name, argv[1], 50);
		printf("Using name \"%s\".\n", user_name);
	}

	user_name_length = strlen(user_name);

	if (user_name_length < 5) {
		fprintf(stderr, "Name must be at least 5 chars long.\n");
		return 1;
	}

	if (user_name_length > 32) {
		fprintf(stderr, "Name is too long.\n");
		return 1;
	}

	ret = make_key(user_name, user_name_length, reg_key);

	if (ret == 0) {
		printf("%s\n", reg_key);
	}

	return ret;
}
