/*	$OpenBSD: aeadtest.c,v 1.13 2022/01/12 08:54:23 tb Exp $	*/
/* ====================================================================
 * Copyright (c) 2011-2013 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/err.h>

/* This program tests an AEAD against a series of test vectors from a file. The
 * test vector file consists of key-value lines where the key and value are
 * separated by a colon and optional whitespace. The keys are listed in
 * NAMES, below. The values are hex-encoded data.
 *
 * After a number of key-value lines, a blank line indicates the end of the
 * test case.
 *
 * For example, here's a valid test case:
 *
 *   AEAD: chacha20-poly1305
 *   KEY: bcb2639bf989c6251b29bf38d39a9bdce7c55f4b2ac12a39c8a37b5d0a5cc2b5
 *   NONCE: 1e8b4c510f5ca083
 *   IN: 8c8419bc27
 *   AD: 34ab88c265
 *   CT: 1a7c2f33f5
 *   TAG: 2875c659d0f2808de3a40027feff91a4
 */

#define BUF_MAX 1024

/* These are the different types of line that are found in the input file. */
enum {
	AEAD = 0,	/* name of the AEAD algorithm. */
	KEY,		/* hex encoded key. */
	NONCE,		/* hex encoded nonce. */
	IN,		/* hex encoded plaintext. */
	AD,		/* hex encoded additional data. */
	CT,		/* hex encoded ciphertext (not including the
			 * authenticator, which is next. */
	TAG,		/* hex encoded authenticator. */
	NUM_TYPES
};

static const char NAMES[NUM_TYPES][6] = {
	"AEAD",
	"KEY",
	"NONCE",
	"IN",
	"AD",
	"CT",
	"TAG",
};

static unsigned char
hex_digit(char h)
{
	if (h >= '0' && h <= '9')
		return h - '0';
	else if (h >= 'a' && h <= 'f')
		return h - 'a' + 10;
	else if (h >= 'A' && h <= 'F')
		return h - 'A' + 10;
	else
		return 16;
}

static int
aead_from_name(const EVP_AEAD **aead, const char *name)
{
	*aead = NULL;

	if (strcmp(name, "aes-128-gcm") == 0) {
#ifndef OPENSSL_NO_AES
		*aead = EVP_aead_aes_128_gcm();
#else
		fprintf(stderr, "No AES support.\n");
#endif
	} else if (strcmp(name, "aes-256-gcm") == 0) {
#ifndef OPENSSL_NO_AES
		*aead = EVP_aead_aes_256_gcm();
#else
		fprintf(stderr, "No AES support.\n");
#endif
	} else if (strcmp(name, "chacha20-poly1305") == 0) {
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
		*aead = EVP_aead_chacha20_poly1305();
#else
		fprintf(stderr, "No chacha20-poly1305 support.\n");
#endif
	} else if (strcmp(name, "xchacha20-poly1305") == 0) {
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
		*aead = EVP_aead_xchacha20_poly1305();
#else
		fprintf(stderr, "No xchacha20-poly1305 support.\n");
#endif
	} else {
		fprintf(stderr, "Unknown AEAD: %s\n", name);
		return -1;
	}

	if (*aead == NULL)
		return 0;

	return 1;
}

static int
run_test_case(const EVP_AEAD* aead, unsigned char bufs[NUM_TYPES][BUF_MAX],
    const unsigned int lengths[NUM_TYPES], unsigned int line_no)
{
	EVP_AEAD_CTX *ctx;
	unsigned char out[BUF_MAX + EVP_AEAD_MAX_TAG_LENGTH], out2[BUF_MAX];
	size_t out_len, out_len2;
	int ret = 0;

	if ((ctx = EVP_AEAD_CTX_new()) == NULL) {
		fprintf(stderr, "Failed to allocate AEAD context on line %u\n",
		    line_no);
		goto err;
	}

	if (!EVP_AEAD_CTX_init(ctx, aead, bufs[KEY], lengths[KEY],
	    lengths[TAG], NULL)) {
		fprintf(stderr, "Failed to init AEAD on line %u\n", line_no);
		goto err;
	}

	if (!EVP_AEAD_CTX_seal(ctx, out, &out_len, sizeof(out), bufs[NONCE],
	    lengths[NONCE], bufs[IN], lengths[IN], bufs[AD], lengths[AD])) {
		fprintf(stderr, "Failed to run AEAD on line %u\n", line_no);
		goto err;
	}

	if (out_len != lengths[CT] + lengths[TAG]) {
		fprintf(stderr, "Bad output length on line %u: %zu vs %u\n",
		    line_no, out_len, (unsigned)(lengths[CT] + lengths[TAG]));
		goto err;
	}

	if (memcmp(out, bufs[CT], lengths[CT]) != 0) {
		fprintf(stderr, "Bad output on line %u\n", line_no);
		goto err;
	}

	if (memcmp(out + lengths[CT], bufs[TAG], lengths[TAG]) != 0) {
		fprintf(stderr, "Bad tag on line %u\n", line_no);
		goto err;
	}

	if (!EVP_AEAD_CTX_open(ctx, out2, &out_len2, lengths[IN], bufs[NONCE],
	    lengths[NONCE], out, out_len, bufs[AD], lengths[AD])) {
		fprintf(stderr, "Failed to decrypt on line %u\n", line_no);
		goto err;
	}

	if (out_len2 != lengths[IN]) {
		fprintf(stderr, "Bad decrypt on line %u: %zu\n",
		    line_no, out_len2);
		goto err;
	}

	if (memcmp(out2, bufs[IN], out_len2) != 0) {
		fprintf(stderr, "Plaintext mismatch on line %u\n", line_no);
		goto err;
	}

	out[0] ^= 0x80;
	if (EVP_AEAD_CTX_open(ctx, out2, &out_len2, lengths[IN], bufs[NONCE],
	    lengths[NONCE], out, out_len, bufs[AD], lengths[AD])) {
		fprintf(stderr, "Decrypted bad data on line %u\n", line_no);
		goto err;
	}

	ret = 1;

 err:
	EVP_AEAD_CTX_free(ctx);

	return ret;
}

int
main(int argc, char **argv)
{
	FILE *f;
	const EVP_AEAD *aead = NULL;
	unsigned int line_no = 0, num_tests = 0, j;

	unsigned char bufs[NUM_TYPES][BUF_MAX];
	unsigned int lengths[NUM_TYPES];

	if (argc != 2) {
		fprintf(stderr, "%s <test file.txt>\n", argv[0]);
		return 1;
	}

	f = fopen(argv[1], "r");
	if (f == NULL) {
		perror("failed to open input");
		return 1;
	}

	for (j = 0; j < NUM_TYPES; j++)
		lengths[j] = 0;

	for (;;) {
		char line[4096];
		unsigned int i, type_len = 0;

		unsigned char *buf = NULL;
		unsigned int *buf_len = NULL;

		if (!fgets(line, sizeof(line), f))
			break;

		line_no++;
		if (line[0] == '#')
			continue;

		if (line[0] == '\n' || line[0] == 0) {
			/* Run a test, if possible. */
			char any_values_set = 0;
			for (j = 0; j < NUM_TYPES; j++) {
				if (lengths[j] != 0) {
					any_values_set = 1;
					break;
				}
			}

			if (!any_values_set)
				continue;

			switch (aead_from_name(&aead, bufs[AEAD])) {
			case 0:
				fprintf(stderr, "Skipping test...\n");
				continue;
			case -1:
				fprintf(stderr, "Aborting...\n");
				return 4;
			}

			if (!run_test_case(aead, bufs, lengths, line_no))
				return 4;

			for (j = 0; j < NUM_TYPES; j++)
				lengths[j] = 0;

			num_tests++;
			continue;
		}

		/* Each line looks like:
		 *   TYPE: 0123abc
		 * Where "TYPE" is the type of the data on the line,
		 * e.g. "KEY". */
		for (i = 0; line[i] != 0 && line[i] != '\n'; i++) {
			if (line[i] == ':') {
				type_len = i;
				break;
			}
		}
		i++;

		if (type_len == 0) {
			fprintf(stderr, "Parse error on line %u\n", line_no);
			return 3;
		}

		/* After the colon, there's optional whitespace. */
		for (; line[i] != 0 && line[i] != '\n'; i++) {
			if (line[i] != ' ' && line[i] != '\t')
				break;
		}

		line[type_len] = 0;
		for (j = 0; j < NUM_TYPES; j++) {
			if (strcmp(line, NAMES[j]) != 0)
				continue;
			if (lengths[j] != 0) {
				fprintf(stderr, "Duplicate value on line %u\n",
				    line_no);
				return 3;
			}
			buf = bufs[j];
			buf_len = &lengths[j];
			break;
		}

		if (buf == NULL) {
			fprintf(stderr, "Unknown line type on line %u\n",
			    line_no);
			return 3;
		}

		if (j == AEAD) {
			*buf_len = strlcpy(buf, line + i, BUF_MAX);
			for (j = 0; j < BUF_MAX; j++) {
				if (buf[j] == '\n')
					buf[j] = '\0';
			}
			continue;
		}

		for (j = 0; line[i] != 0 && line[i] != '\n'; i++) {
			unsigned char v, v2;
			v = hex_digit(line[i++]);
			if (line[i] == 0 || line[i] == '\n') {
				fprintf(stderr, "Odd-length hex data on "
				    "line %u\n", line_no);
				return 3;
			}
			v2 = hex_digit(line[i]);
			if (v > 15 || v2 > 15) {
				fprintf(stderr, "Invalid hex char on line %u\n",
				    line_no);
				return 3;
			}
			v <<= 4;
			v |= v2;

			if (j == BUF_MAX) {
				fprintf(stderr, "Too much hex data on line %u "
				    "(max is %u bytes)\n",
				    line_no, (unsigned) BUF_MAX);
				return 3;
			}
			buf[j++] = v;
			*buf_len = *buf_len + 1;
		}
	}

	printf("Completed %u test cases\n", num_tests);
	printf("PASS\n");
	fclose(f);

	return 0;
}
