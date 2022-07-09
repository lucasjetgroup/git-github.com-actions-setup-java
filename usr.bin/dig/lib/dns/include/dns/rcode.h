/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: rcode.h,v 1.4 2020/02/23 19:54:25 jung Exp $ */

#ifndef DNS_RCODE_H
#define DNS_RCODE_H 1

/*! \file dns/rcode.h */

#include <dns/types.h>

isc_result_t dns_tsigrcode_totext(dns_rcode_t rcode, isc_buffer_t *target);
/*%<
 * Put a textual representation of TSIG/TKEY error 'rcode' into 'target'.
 *
 * Requires:
 *\li	'rcode' is a valid TSIG/TKEY error code.
 *
 *\li	'target' is a valid text buffer.
 *
 * Ensures:
 *\li	If the result is success:
 *		The used space in 'target' is updated.
 *
 * Returns:
 *\li	#ISC_R_SUCCESS			on success
 *\li	#ISC_R_NOSPACE			target buffer is too small
 */

#endif /* DNS_RCODE_H */
