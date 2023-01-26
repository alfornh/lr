#ifndef _UTILS_H__
#define _UTILS_H__

#include <stddef.h>
#include <stdint.h>

#define LOCK_GUARD_MUTEX(lock)       std::lock_guard<std::mutex> solgfu(lock);
#define LOCK_GUARD_MUTEX_BEGIN(lock) { std::lock_guard<std::mutex> palgfu(lock);
#define LOCK_GUARD_MUTEX_END         };

#define MAKE_SHARED(c, args...) std::make_shared< c >( args )

#define DYNAMIC_CAST(c,  sp) std::dynamic_pointer_cast< c >( sp )

#define STATIC_CAST(c,  sp)  std::static_pointer_cast< c >( sp )


//#define ISSPACE(c) ((((c) == ' ') || ((c) == '\t') || ((c) == '\n')))

uint16_t ntoh16(uint16_t u16);
uint32_t ntoh32(uint32_t u32);
uint64_t ntoh64(uint64_t u64);

uint16_t hton16(uint16_t u16);
uint32_t hton32(uint32_t u32);
uint64_t hton64(uint64_t u64);

const char * extract_section_from_str(const char *str, char *word, int wlen, const char end);


int SECKEY_SHA1(const char *data, int dlen, char *res);
char *ZBASE64(const char *data, int dlen, char *res);

#endif//_UTILS_H__
