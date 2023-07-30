#ifndef _ENDIAN_FLIP_H
#define _ENDIAN_FLIP_H

#ifndef BIG_ENDIAN_PLATFORM
#define BE16(i) ((((i) & 0xFF) << 8 | ((i) >> 8) & 0xFF) & 0xFFFF)
#define BE(i)   (((i) & 0xFF) << 24 | ((i) & 0xFF00) << 8 | ((i) & 0xFF0000) >> 8 | ((i) >> 24) & 0xFF)
#else
#define BE16(i) i
#define BE(i) i
#define BE64(i) i
#endif

#define WriteBE8(target, value)  target = value;
#define WriteBE16(target, value) target = BE16(value)
#define WriteBE32(target, value) target = BE(value)

#endif // _ENDIAN_FLIP_H
