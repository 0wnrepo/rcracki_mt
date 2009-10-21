#ifndef _GLOBAL_H
#define _GLOBAL_H

//#include <stdint.h>

#ifdef _WIN32
	#define uint64 unsigned __int64
#else
	#ifndef u_int64_t
		#define uint64 unsigned long long
	#else
		#define uint64 u_int64_t
	#endif
#endif

#ifdef _WIN32
	#define UINT4 unsigned __int32
#else
	#ifndef u_int32_t
		#define UINT4 unsigned int
	#else
		#define UINT4 u_int32_t
	#endif
#endif

#endif /* !GLOBAL_H */
