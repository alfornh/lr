#include "utils.h"

#include <math.h>
//#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlog.h"

inline bool is_host_big_endian() {
  union {
    uint16_t      i;
    uint8_t       c;
  } v;

  v.i = 0xAABB;

  if (v.c == 0xBB) {
    return true;
  }

  return false;

  /*
  if (v == htons(v)) {
    return true;
  }

  return false;
  */
}

uint16_t ntoh16(uint16_t u16) {
  if (is_host_big_endian()) {
    return u16;
  }
  return ((u16 & 0x00FF) << 8) | ((u16 & 0xFF00) >> 8);
  //return ntohs(u16);
}

uint32_t ntoh32(uint32_t u32) {
  if (is_host_big_endian()) {
      return u32;
  }
  return ((u32 & 0x000000FF) << 24) | ((u32 & 0x0000FF00) << 8) | ((u32 & 0x00FF0000) >> 8) | ((u32 & 0xFF000000) >> 24);
  //return ntohl(u32);
}

uint64_t ntoh64(uint64_t u64) {
  if (is_host_big_endian()) {
    return u64;
  }

  uint32_t u64_l = u64 & 0xFFFFFFFF;
  uint32_t u64_h = (u64 >> 32) & 0xFFFFFFFF;

  uint64_t u64_ = ntoh32(u64_l);
  return (u64_ << 32) | ntoh32(u64_h);
}

uint16_t hton16(uint16_t u16) {
  if (is_host_big_endian()) {
    return u16;
  }

  return ((u16 & 0x00FF) << 8) | ((u16 & 0xFF00) >> 8);
  //return htons(u16);
}

uint32_t hton32(uint32_t u32) {
  if (is_host_big_endian()) {
    return u32;
  }
  return ((u32 & 0x000000FF) << 24) | ((u32 & 0x0000FF00) << 8 ) | ((u32 & 0x00FF0000) >> 8) | ((u32 & 0xFF000000) >> 24) ;
  //return htonl(u32);
}

uint64_t hton64(uint64_t u64) {
  if (is_host_big_endian()) {
    return u64;
  }

  uint32_t u64_l = u64 & 0xFFFFFFFF;
  uint32_t u64_h = (u64 >> 32) & 0xFFFFFFFF;

  uint64_t u64_ = hton32(u64_l);
  return (u64_ << 32) | hton32(u64_h);
}

bool isspace(const char c) {
  return (c == ' ') || (c == '\t') || (c == '\n');
}

const char *extract_section_from_str(const char *str, char *word, int wlen, const char end) {
  int len = 0;
  if (!str || !word || (wlen < 1)) {
    return NULL;
  }

  memset(word, 0x0, wlen);
  while (*str && isspace(*str)) ++str;

  while (*str && *str != end && (len < wlen - 1)) {
    word[len++] = *str++;
  }
  word[len] = '\0';

  while (len > 0 && isspace(word[len])) {
    word[len--] = '\0';
  }

  if ( *str != '\0')
    ++str;

  return str;
}
 
inline uint32_t rotl(const uint32_t &n, const uint32_t &b){
  return (n << b) | (n >> (32 - b));
}
  
inline uint32_t ft(const uint32_t &x, const uint32_t &y, const uint32_t &z, const uint32_t &t){
  switch (t/20){
  case 0:
    return (x & y) | ((~x) & z);
  break;

  case 1: case 3:
    return x ^ y ^ z;
  break;

  case 2:
    return (x & y) | (x & z) | (y & z);
  break;
  }

  return t;
}

inline uint64_t lendian64(uint64_t u64) {
  if (!is_host_big_endian()) {
    return u64;
  }

  uint8_t ui;
  union {
    uint64_t _u64;
    uint8_t _u8[8];
  } t;

  t._u64 = u64;

  ui = t._u8[0];
  t._u8[0] = t._u8[7];
  t._u8[7] = ui;

  ui = t._u8[1];
  t._u8[1] = t._u8[6];
  t._u8[6] = ui;

  ui = t._u8[2];
  t._u8[2] = t._u8[5];
  t._u8[5] = ui;

  ui = t._u8[3];
  t._u8[3] = t._u8[4];
  t._u8[4] = ui;

  return t._u64;
}

inline uint32_t lendian32(uint32_t u32) {
  if (!is_host_big_endian()) {
    return u32;
  }
  return (((u32 & 0xFF000000) >> 24) |
         ((u32 & 0x00FF0000) >> 8)   |
         ((u32 & 0x0000FF00) << 8)   |
         ((u32 & 0x000000FF) << 24));
}

int SECKEY_SHA1(const char *data, const int dlen, char *out) {
  #define HASH_BLOCK_SIZE   64  // = 512 bits
  #define HASH_LEN_OFFSET   56  // = 448 / 8
  #define MAX_SECKEY_LEN    128

  uint32_t i = 0, j = 0, t = 0;
  uint32_t K[4] = {
    0x5A827999,
    0x6ED9EBA1,
    0x8F1BBCDC,
    0xCA62C1D6
  };

  uint32_t A, B, C, D, E, temp;
  uint32_t H0 = 0x67452301;
  uint32_t H1 = 0xEFCDAB89;
  uint32_t H2 = 0x98BADCFE;
  uint32_t H3 = 0x10325476;
  uint32_t H4 = 0xC3D2E1F0;
 
  uint32_t M[HASH_BLOCK_SIZE / 4] = { 0 };
  uint32_t W[80] = { 0 };
  unsigned char X[MAX_SECKEY_LEN] = {0};

  uint64_t nbits = dlen << 3;
  uint64_t *lenp;

  uint32_t nblocks = dlen / HASH_BLOCK_SIZE;
  if (dlen % HASH_BLOCK_SIZE >= HASH_LEN_OFFSET) {
    nblocks += 1;
  }
 
  if (dlen > MAX_SECKEY_LEN) {
    return -1;
  }

  memcpy(X, data, dlen);
  X[dlen] = 0x80;
  for (i = dlen + 1; i < (nblocks * HASH_BLOCK_SIZE + HASH_LEN_OFFSET); i++) {
    X[i] = 0;
  }

  lenp = (uint64_t*)(X + (nblocks * HASH_BLOCK_SIZE + HASH_LEN_OFFSET));
  (*lenp) = lendian64(nbits);

  for (i = 0; i <= nblocks; i++) {
    for (j = 0; j < HASH_BLOCK_SIZE; j = j + 4) {
      temp = i * HASH_BLOCK_SIZE + j;
      M[j / 4] = (X[temp] << 24) | (X[temp + 1] << 16) | (X[temp + 2] << 8) | X[temp + 3];
    }
 
    for (t = 0; t < 80; t++) {
      if (t < 16) {
        W[t] = M[t];
      } else {
        W[t] = rotl(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);
      }
    }
 
    A = H0;
    B = H1;
    C = H2;
    D = H3;
    E = H4;
 
    for (t = 0; t < 80; ++t) {
      temp = rotl(A, 5) + ft(B, C, D, t) + E + W[t] + K[t/20];
      E = D;
      D = C;
      C = rotl(B, 30);
      B = A;
      A = temp;
    }

    H0 = H0 + A;
    H1 = H1 + B;
    H2 = H2 + C;
    H3 = H3 + D;
    H4 = H4 + E;
  }

  ((uint32_t *)out)[0] = lendian32(H0);
  ((uint32_t *)out)[1] = lendian32(H1);
  ((uint32_t *)out)[2] = lendian32(H2);
  ((uint32_t *)out)[3] = lendian32(H3);
  ((uint32_t *)out)[4] = lendian32(H4);

  return 0;
}

char * ZBASE64(const char *data, int dlen, char *res) {
  static const char *alph = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const unsigned char *beg = (const unsigned char *)data; 
  int len = dlen;
  uint32_t dif;
  uint32_t threec;
  int ires = 0;
  char reminder[4] = { 0 };

  switch (len % 3) {
  case 2:
    reminder[0] = data[dlen - 2];
    reminder[1] = data[dlen - 1];
    len -= 2;
  break;
  case 1:
    reminder[0] = data[dlen - 1];
    --len;
  break;
  }

  for (int i = 0; i < len; i += 3) {
    threec = (((uint32_t)beg[i] & 0xFF) << 16) | (((uint32_t)beg[i + 1] & 0xFF) << 8) | (beg[i + 2] & 0xFF);

    res[ires++] = alph[(threec >> 18) & 0x3F];
    res[ires++] = alph[(threec >> 12) & 0x3F];
    res[ires++] = alph[(threec >> 6 ) & 0x3F];
    res[ires++] = alph[threec         & 0x3F];

    if (ires % 19 == 0){
      res[ires++] = '\n';
    }
  }

  dif = dlen - len;
  if (dif > 0) {
    threec = (((uint32_t)reminder[0] & 0xFF) << 16) | (((uint32_t)reminder[1] & 0xFF) << 8) | (reminder[2] & 0xFF);

    res[ires++] = alph[(threec >> 18) & 0x3F];
    res[ires++] = alph[(threec >> 12) & 0x3F];
    res[ires++] = alph[(threec >> 6 ) & 0x3F];
    res[ires++] = alph[(threec)       & 0x3F];

    switch (dif) {
    case 2:
      res[ires - 1] = '=';
    break;
    case 1:
      res[ires - 2] = '=';
      res[ires - 1] = '=';
    break;
    }    
  }
  return res;
}
