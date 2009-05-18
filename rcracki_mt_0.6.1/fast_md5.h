/*
 * Fast implementation of the MD5 message-digest algorithm as per RFC
 * (see http://tools.ietf.org/html/rfc1321)
 *
 * Author: Joao Inacio <jcinacio at gmail.com>
 * License: Use and share as you wish at your own risk, please keep this header ;)
 *
 * Optimizations:
 *  - For lengths < 16, transformation steps are "unrolled" using macros/defines
 *  - Constants used whenever possible, it's the compiler's job to sort them out
 *  - Padding is done on 4-byte words, and memory copied as last resort.
 */


#ifndef FAST_MD5_H
#define FAST_MD5_H


void
fast_MD5(unsigned char *pData, int len, unsigned char *pDigest);


#endif // FAST_MD5_H
