#ifndef MD4_H
#define MD4_H

//typedef unsigned long uint32;
#if defined(_WIN64) || defined(_LP64)
typedef unsigned int UINT4;
#else
typedef unsigned long UINT4;
#endif

//Main function
void MD4_NEW( unsigned char * buf, int len, unsigned char * pDigest);

#endif /* !MD4_H */
