#pragma once

// workaround for the beXXtoh family of functions not being available on
// the build bot machine.

#include <netinet/in.h>
#include <stdint.h>

static inline uint64_t ntohll(uint64_t _x)
{
#if __BYTE_ORDER == __BIG_ENDIAN
  return _x;
#else
  return (((uint64_t)ntohl(_x & (uint64_t)0xFFFFFFFFULL)) << 32) | ((uint64_t)ntohl((_x & (uint64_t)0xFFFFFFFF00000000ULL) >> 32));
#endif
}

static inline uint64_t htonll(uint64_t _x)
{
#if __BYTE_ORDER == __BIG_ENDIAN
  return _x;
#else
  return ntohll(_x);
#endif
}

