/* $OpenBSD: evp_locl.h,v 1.23 2022/05/05 08:42:27 tb Exp $ */
/* Written by Dr Stephen N Henson (steve@openssl.org) for the OpenSSL
 * project 2000.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
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
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#ifndef HEADER_EVP_LOCL_H
#define HEADER_EVP_LOCL_H

__BEGIN_HIDDEN_DECLS

/*
 * Don't free md_ctx->pctx in EVP_MD_CTX_cleanup().  Needed for ownership
 * handling in EVP_MD_CTX_set_pkey_ctx().
 */
#define EVP_MD_CTX_FLAG_KEEP_PKEY_CTX   0x0400

typedef int evp_sign_method(int type, const unsigned char *m,
    unsigned int m_length, unsigned char *sigret, unsigned int *siglen,
    void *key);
typedef int evp_verify_method(int type, const unsigned char *m,
    unsigned int m_length, const unsigned char *sigbuf, unsigned int siglen,
    void *key);

/* Type needs to be a bit field
 * Sub-type needs to be for variations on the method, as in, can it do
 * arbitrary encryption.... */
struct evp_pkey_st {
	int type;
	int save_type;
	int references;
	const EVP_PKEY_ASN1_METHOD *ameth;
	ENGINE *engine;
	union	{
		char *ptr;
#ifndef OPENSSL_NO_RSA
		struct rsa_st *rsa;	/* RSA */
#endif
#ifndef OPENSSL_NO_DSA
		struct dsa_st *dsa;	/* DSA */
#endif
#ifndef OPENSSL_NO_DH
		struct dh_st *dh;	/* DH */
#endif
#ifndef OPENSSL_NO_EC
		struct ec_key_st *ec;	/* ECC */
#endif
#ifndef OPENSSL_NO_GOST
		struct gost_key_st *gost; /* GOST */
#endif
	} pkey;
	int save_parameters;
	STACK_OF(X509_ATTRIBUTE) *attributes; /* [ 0 ] */
} /* EVP_PKEY */;

struct env_md_st {
	int type;
	int pkey_type;
	int md_size;
	unsigned long flags;
	int (*init)(EVP_MD_CTX *ctx);
	int (*update)(EVP_MD_CTX *ctx, const void *data, size_t count);
	int (*final)(EVP_MD_CTX *ctx, unsigned char *md);
	int (*copy)(EVP_MD_CTX *to, const EVP_MD_CTX *from);
	int (*cleanup)(EVP_MD_CTX *ctx);

	int block_size;
	int ctx_size; /* how big does the ctx->md_data need to be */
	/* control function */
	int (*md_ctrl)(EVP_MD_CTX *ctx, int cmd, int p1, void *p2);
} /* EVP_MD */;

struct env_md_ctx_st {
	const EVP_MD *digest;
	ENGINE *engine; /* functional reference if 'digest' is ENGINE-provided */
	unsigned long flags;
	void *md_data;
	/* Public key context for sign/verify */
	EVP_PKEY_CTX *pctx;
	/* Update function: usually copied from EVP_MD */
	int (*update)(EVP_MD_CTX *ctx, const void *data, size_t count);
} /* EVP_MD_CTX */;

struct evp_cipher_st {
	int nid;
	int block_size;
	int key_len;		/* Default value for variable length ciphers */
	int iv_len;
	unsigned long flags;	/* Various flags */
	int (*init)(EVP_CIPHER_CTX *ctx, const unsigned char *key,
	    const unsigned char *iv, int enc);	/* init key */
	int (*do_cipher)(EVP_CIPHER_CTX *ctx, unsigned char *out,
	    const unsigned char *in, size_t inl);/* encrypt/decrypt data */
	int (*cleanup)(EVP_CIPHER_CTX *); /* cleanup ctx */
	int ctx_size;		/* how big ctx->cipher_data needs to be */
	int (*set_asn1_parameters)(EVP_CIPHER_CTX *, ASN1_TYPE *); /* Populate a ASN1_TYPE with parameters */
	int (*get_asn1_parameters)(EVP_CIPHER_CTX *, ASN1_TYPE *); /* Get parameters from a ASN1_TYPE */
	int (*ctrl)(EVP_CIPHER_CTX *, int type, int arg, void *ptr); /* Miscellaneous operations */
	void *app_data;		/* Application data */
} /* EVP_CIPHER */;

struct evp_cipher_ctx_st {
	const EVP_CIPHER *cipher;
	ENGINE *engine;	/* functional reference if 'cipher' is ENGINE-provided */
	int encrypt;		/* encrypt or decrypt */
	int buf_len;		/* number we have left */

	unsigned char  oiv[EVP_MAX_IV_LENGTH];	/* original iv */
	unsigned char  iv[EVP_MAX_IV_LENGTH];	/* working iv */
	unsigned char buf[EVP_MAX_BLOCK_LENGTH];/* saved partial block */
	int num;				/* used by cfb/ofb/ctr mode */

	void *app_data;		/* application stuff */
	int key_len;		/* May change for variable length cipher */
	unsigned long flags;	/* Various flags */
	void *cipher_data; /* per EVP data */
	int final_used;
	int block_mask;
	unsigned char final[EVP_MAX_BLOCK_LENGTH];/* possible final block */
} /* EVP_CIPHER_CTX */;

struct evp_Encode_Ctx_st {

	int num;	/* number saved in a partial encode/decode */
	int length;	/* The length is either the output line length
			 * (in input bytes) or the shortest input line
			 * length that is ok.  Once decoding begins,
			 * the length is adjusted up each time a longer
			 * line is decoded */
	unsigned char enc_data[80];	/* data to encode */
	int line_num;	/* number read on current line */
	int expect_nl;
} /* EVP_ENCODE_CTX */;

/* Macros to code block cipher wrappers */

/* Wrapper functions for each cipher mode */

#define BLOCK_CIPHER_ecb_loop() \
	size_t i, bl; \
	bl = ctx->cipher->block_size;\
	if(inl < bl) return 1;\
	inl -= bl; \
	for(i=0; i <= inl; i+=bl)

#define BLOCK_CIPHER_func_ecb(cname, cprefix, kstruct, ksched) \
static int cname##_ecb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
	BLOCK_CIPHER_ecb_loop() \
		cprefix##_ecb_encrypt(in + i, out + i, &((kstruct *)ctx->cipher_data)->ksched, ctx->encrypt);\
	return 1;\
}

#define EVP_MAXCHUNK ((size_t)1<<(sizeof(long)*8-2))

#define BLOCK_CIPHER_func_ofb(cname, cprefix, cbits, kstruct, ksched) \
static int cname##_ofb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
	while(inl>=EVP_MAXCHUNK)\
	    {\
	    cprefix##_ofb##cbits##_encrypt(in, out, (long)EVP_MAXCHUNK, &((kstruct *)ctx->cipher_data)->ksched, ctx->iv, &ctx->num);\
	    inl-=EVP_MAXCHUNK;\
	    in +=EVP_MAXCHUNK;\
	    out+=EVP_MAXCHUNK;\
	    }\
	if (inl)\
	    cprefix##_ofb##cbits##_encrypt(in, out, (long)inl, &((kstruct *)ctx->cipher_data)->ksched, ctx->iv, &ctx->num);\
	return 1;\
}

#define BLOCK_CIPHER_func_cbc(cname, cprefix, kstruct, ksched) \
static int cname##_cbc_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
	while(inl>=EVP_MAXCHUNK) \
	    {\
	    cprefix##_cbc_encrypt(in, out, (long)EVP_MAXCHUNK, &((kstruct *)ctx->cipher_data)->ksched, ctx->iv, ctx->encrypt);\
	    inl-=EVP_MAXCHUNK;\
	    in +=EVP_MAXCHUNK;\
	    out+=EVP_MAXCHUNK;\
	    }\
	if (inl)\
	    cprefix##_cbc_encrypt(in, out, (long)inl, &((kstruct *)ctx->cipher_data)->ksched, ctx->iv, ctx->encrypt);\
	return 1;\
}

#define BLOCK_CIPHER_func_cfb(cname, cprefix, cbits, kstruct, ksched) \
static int cname##_cfb##cbits##_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
	size_t chunk=EVP_MAXCHUNK;\
	if (cbits==1)  chunk>>=3;\
	if (inl<chunk) chunk=inl;\
	while(inl && inl>=chunk)\
	    {\
            cprefix##_cfb##cbits##_encrypt(in, out, (long)((cbits==1) && !(ctx->flags & EVP_CIPH_FLAG_LENGTH_BITS) ?inl*8:inl), &((kstruct *)ctx->cipher_data)->ksched, ctx->iv, &ctx->num, ctx->encrypt);\
	    inl-=chunk;\
	    in +=chunk;\
	    out+=chunk;\
	    if(inl<chunk) chunk=inl;\
	    }\
	return 1;\
}

#define BLOCK_CIPHER_all_funcs(cname, cprefix, cbits, kstruct, ksched) \
	BLOCK_CIPHER_func_cbc(cname, cprefix, kstruct, ksched) \
	BLOCK_CIPHER_func_cfb(cname, cprefix, cbits, kstruct, ksched) \
	BLOCK_CIPHER_func_ecb(cname, cprefix, kstruct, ksched) \
	BLOCK_CIPHER_func_ofb(cname, cprefix, cbits, kstruct, ksched)

#define BLOCK_CIPHER_def1(cname, nmode, mode, MODE, kstruct, nid, block_size, \
			  key_len, iv_len, flags, init_key, cleanup, \
			  set_asn1, get_asn1, ctrl) \
static const EVP_CIPHER cname##_##mode = { \
	nid##_##nmode, block_size, key_len, iv_len, \
	flags | EVP_CIPH_##MODE##_MODE, \
	init_key, \
	cname##_##mode##_cipher, \
	cleanup, \
	sizeof(kstruct), \
	set_asn1, get_asn1,\
	ctrl, \
	NULL \
}; \
const EVP_CIPHER *EVP_##cname##_##mode(void) { return &cname##_##mode; }

#define BLOCK_CIPHER_def_cbc(cname, kstruct, nid, block_size, key_len, \
			     iv_len, flags, init_key, cleanup, set_asn1, \
			     get_asn1, ctrl) \
BLOCK_CIPHER_def1(cname, cbc, cbc, CBC, kstruct, nid, block_size, key_len, \
		  iv_len, flags, init_key, cleanup, set_asn1, get_asn1, ctrl)

#define BLOCK_CIPHER_def_cfb(cname, kstruct, nid, key_len, \
			     iv_len, cbits, flags, init_key, cleanup, \
			     set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def1(cname, cfb##cbits, cfb##cbits, CFB, kstruct, nid, 1, \
		  key_len, iv_len, flags, init_key, cleanup, set_asn1, \
		  get_asn1, ctrl)

#define BLOCK_CIPHER_def_ofb(cname, kstruct, nid, key_len, \
			     iv_len, cbits, flags, init_key, cleanup, \
			     set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def1(cname, ofb##cbits, ofb, OFB, kstruct, nid, 1, \
		  key_len, iv_len, flags, init_key, cleanup, set_asn1, \
		  get_asn1, ctrl)

#define BLOCK_CIPHER_def_ecb(cname, kstruct, nid, block_size, key_len, \
			     flags, init_key, cleanup, set_asn1, \
			     get_asn1, ctrl) \
BLOCK_CIPHER_def1(cname, ecb, ecb, ECB, kstruct, nid, block_size, key_len, \
		  0, flags, init_key, cleanup, set_asn1, get_asn1, ctrl)

#define BLOCK_CIPHER_defs(cname, kstruct, \
			  nid, block_size, key_len, iv_len, cbits, flags, \
			  init_key, cleanup, set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def_cbc(cname, kstruct, nid, block_size, key_len, iv_len, flags, \
		     init_key, cleanup, set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def_cfb(cname, kstruct, nid, key_len, iv_len, cbits, \
		     flags, init_key, cleanup, set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def_ofb(cname, kstruct, nid, key_len, iv_len, cbits, \
		     flags, init_key, cleanup, set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def_ecb(cname, kstruct, nid, block_size, key_len, flags, \
		     init_key, cleanup, set_asn1, get_asn1, ctrl)


/*
#define BLOCK_CIPHER_defs(cname, kstruct, \
				nid, block_size, key_len, iv_len, flags,\
				 init_key, cleanup, set_asn1, get_asn1, ctrl)\
static const EVP_CIPHER cname##_cbc = {\
	nid##_cbc, block_size, key_len, iv_len, \
	flags | EVP_CIPH_CBC_MODE,\
	init_key,\
	cname##_cbc_cipher,\
	cleanup,\
	sizeof(EVP_CIPHER_CTX)-sizeof((((EVP_CIPHER_CTX *)NULL)->c))+\
		sizeof((((EVP_CIPHER_CTX *)NULL)->c.kstruct)),\
	set_asn1, get_asn1,\
	ctrl, \
	NULL \
};\
const EVP_CIPHER *EVP_##cname##_cbc(void) { return &cname##_cbc; }\
static const EVP_CIPHER cname##_cfb = {\
	nid##_cfb64, 1, key_len, iv_len, \
	flags | EVP_CIPH_CFB_MODE,\
	init_key,\
	cname##_cfb_cipher,\
	cleanup,\
	sizeof(EVP_CIPHER_CTX)-sizeof((((EVP_CIPHER_CTX *)NULL)->c))+\
		sizeof((((EVP_CIPHER_CTX *)NULL)->c.kstruct)),\
	set_asn1, get_asn1,\
	ctrl,\
	NULL \
};\
const EVP_CIPHER *EVP_##cname##_cfb(void) { return &cname##_cfb; }\
static const EVP_CIPHER cname##_ofb = {\
	nid##_ofb64, 1, key_len, iv_len, \
	flags | EVP_CIPH_OFB_MODE,\
	init_key,\
	cname##_ofb_cipher,\
	cleanup,\
	sizeof(EVP_CIPHER_CTX)-sizeof((((EVP_CIPHER_CTX *)NULL)->c))+\
		sizeof((((EVP_CIPHER_CTX *)NULL)->c.kstruct)),\
	set_asn1, get_asn1,\
	ctrl,\
	NULL \
};\
const EVP_CIPHER *EVP_##cname##_ofb(void) { return &cname##_ofb; }\
static const EVP_CIPHER cname##_ecb = {\
	nid##_ecb, block_size, key_len, iv_len, \
	flags | EVP_CIPH_ECB_MODE,\
	init_key,\
	cname##_ecb_cipher,\
	cleanup,\
	sizeof(EVP_CIPHER_CTX)-sizeof((((EVP_CIPHER_CTX *)NULL)->c))+\
		sizeof((((EVP_CIPHER_CTX *)NULL)->c.kstruct)),\
	set_asn1, get_asn1,\
	ctrl,\
	NULL \
};\
const EVP_CIPHER *EVP_##cname##_ecb(void) { return &cname##_ecb; }
*/

#define IMPLEMENT_BLOCK_CIPHER(cname, ksched, cprefix, kstruct, nid, \
			       block_size, key_len, iv_len, cbits, \
			       flags, init_key, \
			       cleanup, set_asn1, get_asn1, ctrl) \
	BLOCK_CIPHER_all_funcs(cname, cprefix, cbits, kstruct, ksched) \
	BLOCK_CIPHER_defs(cname, kstruct, nid, block_size, key_len, iv_len, \
			  cbits, flags, init_key, cleanup, set_asn1, \
			  get_asn1, ctrl)

#define EVP_C_DATA(kstruct, ctx)	((kstruct *)(ctx)->cipher_data)

#define IMPLEMENT_CFBR(cipher,cprefix,kstruct,ksched,keysize,cbits,iv_len) \
	BLOCK_CIPHER_func_cfb(cipher##_##keysize,cprefix,cbits,kstruct,ksched) \
	BLOCK_CIPHER_def_cfb(cipher##_##keysize,kstruct, \
			     NID_##cipher##_##keysize, keysize/8, iv_len, cbits, \
			     0, cipher##_init_key, NULL, \
			     EVP_CIPHER_set_asn1_iv, \
			     EVP_CIPHER_get_asn1_iv, \
			     NULL)

struct evp_pkey_ctx_st {
	/* Method associated with this operation */
	const EVP_PKEY_METHOD *pmeth;
	/* Engine that implements this method or NULL if builtin */
	ENGINE *engine;
	/* Key: may be NULL */
	EVP_PKEY *pkey;
	/* Peer key for key agreement, may be NULL */
	EVP_PKEY *peerkey;
	/* Actual operation */
	int operation;
	/* Algorithm specific data */
	void *data;
	/* Application specific data */
	void *app_data;
	/* Keygen callback */
	EVP_PKEY_gen_cb *pkey_gencb;
	/* implementation specific keygen data */
	int *keygen_info;
	int keygen_info_count;
} /* EVP_PKEY_CTX */;

#define EVP_PKEY_FLAG_DYNAMIC	1

struct evp_pkey_method_st {
	int pkey_id;
	int flags;

	int (*init)(EVP_PKEY_CTX *ctx);
	int (*copy)(EVP_PKEY_CTX *dst, EVP_PKEY_CTX *src);
	void (*cleanup)(EVP_PKEY_CTX *ctx);

	int (*paramgen_init)(EVP_PKEY_CTX *ctx);
	int (*paramgen)(EVP_PKEY_CTX *ctx, EVP_PKEY *pkey);

	int (*keygen_init)(EVP_PKEY_CTX *ctx);
	int (*keygen)(EVP_PKEY_CTX *ctx, EVP_PKEY *pkey);

	int (*sign_init)(EVP_PKEY_CTX *ctx);
	int (*sign)(EVP_PKEY_CTX *ctx, unsigned char *sig, size_t *siglen,
	    const unsigned char *tbs, size_t tbslen);

	int (*verify_init)(EVP_PKEY_CTX *ctx);
	int (*verify)(EVP_PKEY_CTX *ctx,
	    const unsigned char *sig, size_t siglen,
	    const unsigned char *tbs, size_t tbslen);

	int (*verify_recover_init)(EVP_PKEY_CTX *ctx);
	int (*verify_recover)(EVP_PKEY_CTX *ctx,
	    unsigned char *rout, size_t *routlen,
	    const unsigned char *sig, size_t siglen);

	int (*signctx_init)(EVP_PKEY_CTX *ctx, EVP_MD_CTX *mctx);
	int (*signctx)(EVP_PKEY_CTX *ctx, unsigned char *sig, size_t *siglen,
	    EVP_MD_CTX *mctx);

	int (*verifyctx_init)(EVP_PKEY_CTX *ctx, EVP_MD_CTX *mctx);
	int (*verifyctx)(EVP_PKEY_CTX *ctx, const unsigned char *sig,
	    int siglen, EVP_MD_CTX *mctx);

	int (*encrypt_init)(EVP_PKEY_CTX *ctx);
	int (*encrypt)(EVP_PKEY_CTX *ctx, unsigned char *out, size_t *outlen,
	    const unsigned char *in, size_t inlen);

	int (*decrypt_init)(EVP_PKEY_CTX *ctx);
	int (*decrypt)(EVP_PKEY_CTX *ctx, unsigned char *out, size_t *outlen,
	    const unsigned char *in, size_t inlen);

	int (*derive_init)(EVP_PKEY_CTX *ctx);
	int (*derive)(EVP_PKEY_CTX *ctx, unsigned char *key, size_t *keylen);

	int (*ctrl)(EVP_PKEY_CTX *ctx, int type, int p1, void *p2);
	int (*ctrl_str)(EVP_PKEY_CTX *ctx, const char *type, const char *value);

	int (*check)(EVP_PKEY *pkey);
	int (*public_check)(EVP_PKEY *pkey);
	int (*param_check)(EVP_PKEY *pkey);
} /* EVP_PKEY_METHOD */;

void evp_pkey_set_cb_translate(BN_GENCB *cb, EVP_PKEY_CTX *ctx);

int PKCS5_v2_PBKDF2_keyivgen(EVP_CIPHER_CTX *ctx, const char *pass, int passlen,
    ASN1_TYPE *param, const EVP_CIPHER *c, const EVP_MD *md, int en_de);

/* EVP_AEAD represents a specific AEAD algorithm. */
struct evp_aead_st {
	unsigned char key_len;
	unsigned char nonce_len;
	unsigned char overhead;
	unsigned char max_tag_len;

	int (*init)(struct evp_aead_ctx_st*, const unsigned char *key,
	    size_t key_len, size_t tag_len);
	void (*cleanup)(struct evp_aead_ctx_st*);

	int (*seal)(const struct evp_aead_ctx_st *ctx, unsigned char *out,
	    size_t *out_len, size_t max_out_len, const unsigned char *nonce,
	    size_t nonce_len, const unsigned char *in, size_t in_len,
	    const unsigned char *ad, size_t ad_len);

	int (*open)(const struct evp_aead_ctx_st *ctx, unsigned char *out,
	    size_t *out_len, size_t max_out_len, const unsigned char *nonce,
	    size_t nonce_len, const unsigned char *in, size_t in_len,
	    const unsigned char *ad, size_t ad_len);
};

/* An EVP_AEAD_CTX represents an AEAD algorithm configured with a specific key
 * and message-independent IV. */
struct evp_aead_ctx_st {
	const EVP_AEAD *aead;
	/* aead_state is an opaque pointer to the AEAD specific state. */
	void *aead_state;
};

int EVP_PKEY_CTX_str2ctrl(EVP_PKEY_CTX *ctx, int cmd, const char *str);
int EVP_PKEY_CTX_hex2ctrl(EVP_PKEY_CTX *ctx, int cmd, const char *hex);
int EVP_PKEY_CTX_md(EVP_PKEY_CTX *ctx, int optype, int cmd, const char *md_name);

__END_HIDDEN_DECLS

#endif /* !HEADER_EVP_LOCL_H */
