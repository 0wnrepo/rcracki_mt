#ifndef SHA1_H
#define SHA1_H

#include "global.h"

#define SHA1_DIGEST_LENGTH 20

void SHA1_NEW( unsigned char * pData, int length, unsigned char * pDigest);

#endif /* !SHA1_H */
