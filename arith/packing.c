/* 
 * Copyright 2026 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "packing.h"

static void poly_pack_2(unsigned char *b, const poly *r)
{
  for(unsigned int i = 0; i < RRLWR_N / 4; i++) {
    uint32_t c0 = (1 - r->coeffs[4 * i + 0]) & 0x3;
    uint32_t c1 = (1 - r->coeffs[4 * i + 1]) & 0x3;
    uint32_t c2 = (1 - r->coeffs[4 * i + 2]) & 0x3;
    uint32_t c3 = (1 - r->coeffs[4 * i + 3]) & 0x3;

    b[i] = (unsigned char)(c0 | (c1 << 2) | (c2 << 4) | (c3 << 6));
  }
}

static void poly_pack_11(unsigned char *b, const poly *r)
{
  for(unsigned int i = 0; i < RRLWR_N / 8; i++) {
    uint32_t c0 = (1023 - r->coeffs[8 * i + 0]) & 0x7ff;
    uint32_t c1 = (1023 - r->coeffs[8 * i + 1]) & 0x7ff;
    uint32_t c2 = (1023 - r->coeffs[8 * i + 2]) & 0x7ff;
    uint32_t c3 = (1023 - r->coeffs[8 * i + 3]) & 0x7ff;
    uint32_t c4 = (1023 - r->coeffs[8 * i + 4]) & 0x7ff;
    uint32_t c5 = (1023 - r->coeffs[8 * i + 5]) & 0x7ff;
    uint32_t c6 = (1023 - r->coeffs[8 * i + 6]) & 0x7ff;
    uint32_t c7 = (1023 - r->coeffs[8 * i + 7]) & 0x7ff;
    unsigned int p = 11 * i;

    b[p + 0] = (unsigned char)c0;
    b[p + 1] = (unsigned char)((c0 >> 8) | (c1 << 3));
    b[p + 2] = (unsigned char)((c1 >> 5) | (c2 << 6));
    b[p + 3] = (unsigned char)(c2 >> 2);
    b[p + 4] = (unsigned char)((c2 >> 10) | (c3 << 1));
    b[p + 5] = (unsigned char)((c3 >> 7) | (c4 << 4));
    b[p + 6] = (unsigned char)((c4 >> 4) | (c5 << 7));
    b[p + 7] = (unsigned char)(c5 >> 1);
    b[p + 8] = (unsigned char)((c5 >> 9) | (c6 << 2));
    b[p + 9] = (unsigned char)((c6 >> 6) | (c7 << 5));
    b[p + 10] = (unsigned char)(c7 >> 3);
  }
}

static void poly_unpack_2(poly *r, const unsigned char *b)
{
  for(unsigned int i = 0; i < RRLWR_N / 4; i++) {
    uint32_t x = b[i];

    r->coeffs[4 * i + 0] = (int16_t)(1 - (int32_t)(x & 0x3));
    r->coeffs[4 * i + 1] = (int16_t)(1 - (int32_t)((x >> 2) & 0x3));
    r->coeffs[4 * i + 2] = (int16_t)(1 - (int32_t)((x >> 4) & 0x3));
    r->coeffs[4 * i + 3] = (int16_t)(1 - (int32_t)(x >> 6));
  }
}

static void poly_unpack_13(poly *r, const unsigned char *b)
{
  for(unsigned int i = 0; i < RRLWR_N / 8; i++) {
    const unsigned char *p = b + 13 * i;
    uint32_t c0 = ((uint32_t)p[0] | ((uint32_t)p[1] << 8)) & 0x1fff;
    uint32_t c1 = (((uint32_t)p[1] >> 5) | ((uint32_t)p[2] << 3) |
                   ((uint32_t)p[3] << 11)) & 0x1fff;
    uint32_t c2 = (((uint32_t)p[3] >> 2) | ((uint32_t)p[4] << 6)) & 0x1fff;
    uint32_t c3 = (((uint32_t)p[4] >> 7) | ((uint32_t)p[5] << 1) |
                   ((uint32_t)p[6] << 9)) & 0x1fff;
    uint32_t c4 = (((uint32_t)p[6] >> 4) | ((uint32_t)p[7] << 4) |
                   ((uint32_t)p[8] << 12)) & 0x1fff;
    uint32_t c5 = (((uint32_t)p[8] >> 1) | ((uint32_t)p[9] << 7)) & 0x1fff;
    uint32_t c6 = (((uint32_t)p[9] >> 6) | ((uint32_t)p[10] << 2) |
                   ((uint32_t)p[11] << 10)) & 0x1fff;
    uint32_t c7 = (((uint32_t)p[11] >> 3) | ((uint32_t)p[12] << 5)) & 0x1fff;

    r->coeffs[8 * i + 0] = (int16_t)(4095 - (int32_t)c0);
    r->coeffs[8 * i + 1] = (int16_t)(4095 - (int32_t)c1);
    r->coeffs[8 * i + 2] = (int16_t)(4095 - (int32_t)c2);
    r->coeffs[8 * i + 3] = (int16_t)(4095 - (int32_t)c3);
    r->coeffs[8 * i + 4] = (int16_t)(4095 - (int32_t)c4);
    r->coeffs[8 * i + 5] = (int16_t)(4095 - (int32_t)c5);
    r->coeffs[8 * i + 6] = (int16_t)(4095 - (int32_t)c6);
    r->coeffs[8 * i + 7] = (int16_t)(4095 - (int32_t)c7);
  }
}

#if RRLWR_PKE_LOGT != 3 && RRLWR_PKE_LOGT != 8
static uint32_t encrypt_t_coeff(uint16_t x,
                                const unsigned char *msg,
                                unsigned int out,
                                unsigned int k)
{
  uint32_t mj = (msg[out * RRLWR_N / 8 + (k >> 3)] >> (k & 0x7)) & 1;
  uint32_t c = (uint32_t)x;

  c = (c + ((uint32_t)1 << (RRLWR_PKE_LOGQ - (RRLWR_PKE_LOGP + 1))) +
       (-mj & ((uint32_t)1 << (RRLWR_PKE_LOGP - 1)))) &
      (RRLWR_PKE_P - 1);
  c >>= RRLWR_PKE_LOGP - RRLWR_PKE_LOGT;
  c = (((uint32_t)1 << (RRLWR_PKE_LOGT - 1)) - 1 - c) &
      (((uint32_t)1 << RRLWR_PKE_LOGT) - 1);

  return c;
}
#endif

void poly_pack_ciphertext_t_from_acc_msg(unsigned char *ct,
                                         const uint16_t acc[RRLWR_N],
                                         const unsigned char msg[RRLWR_PKE_MESSAGE_LEN],
                                         unsigned int out)
{
#if RRLWR_PKE_LOGT == 3
  for(unsigned int i = 0; i < RRLWR_N / 8; i++) {
    unsigned int k = 8 * i;
    uint32_t m = msg[out * RRLWR_N / 8 + i];
    uint32_t c0 = (3 - (((uint32_t)acc[k + 0] + 2) & 0x7ff) / 256 - ((m & 1) << 2)) & 7;
    uint32_t c1 = (3 - (((uint32_t)acc[k + 1] + 2) & 0x7ff) / 256 - (((m >> 1) & 1) << 2)) & 7;
    uint32_t c2 = (3 - (((uint32_t)acc[k + 2] + 2) & 0x7ff) / 256 - (((m >> 2) & 1) << 2)) & 7;
    uint32_t c3 = (3 - (((uint32_t)acc[k + 3] + 2) & 0x7ff) / 256 - (((m >> 3) & 1) << 2)) & 7;
    uint32_t c4 = (3 - (((uint32_t)acc[k + 4] + 2) & 0x7ff) / 256 - (((m >> 4) & 1) << 2)) & 7;
    uint32_t c5 = (3 - (((uint32_t)acc[k + 5] + 2) & 0x7ff) / 256 - (((m >> 5) & 1) << 2)) & 7;
    uint32_t c6 = (3 - (((uint32_t)acc[k + 6] + 2) & 0x7ff) / 256 - (((m >> 6) & 1) << 2)) & 7;
    uint32_t c7 = (3 - (((uint32_t)acc[k + 7] + 2) & 0x7ff) / 256 - ((m >> 7) << 2)) & 7;
    unsigned int p = 3 * i;

    ct[p + 0] = (unsigned char)(c0 | (c1 << 3) | (c2 << 6));
    ct[p + 1] = (unsigned char)((c2 >> 2) | (c3 << 1) | (c4 << 4) | (c5 << 7));
    ct[p + 2] = (unsigned char)((c5 >> 1) | (c6 << 2) | (c7 << 5));
  }
#elif RRLWR_PKE_LOGT == 8
  for(unsigned int i = 0; i < RRLWR_N / 8; i++) {
    unsigned int k = 8 * i;
    uint32_t m = msg[out * RRLWR_N / 8 + i];

    ct[k + 0] = (unsigned char)((127 - ((((uint32_t)acc[k + 0] + 2) & 0x7ff) >> 3) - ((m & 1) << 7)) & 0xff);
    ct[k + 1] = (unsigned char)((127 - ((((uint32_t)acc[k + 1] + 2) & 0x7ff) >> 3) - (((m >> 1) & 1) << 7)) & 0xff);
    ct[k + 2] = (unsigned char)((127 - ((((uint32_t)acc[k + 2] + 2) & 0x7ff) >> 3) - (((m >> 2) & 1) << 7)) & 0xff);
    ct[k + 3] = (unsigned char)((127 - ((((uint32_t)acc[k + 3] + 2) & 0x7ff) >> 3) - (((m >> 3) & 1) << 7)) & 0xff);
    ct[k + 4] = (unsigned char)((127 - ((((uint32_t)acc[k + 4] + 2) & 0x7ff) >> 3) - (((m >> 4) & 1) << 7)) & 0xff);
    ct[k + 5] = (unsigned char)((127 - ((((uint32_t)acc[k + 5] + 2) & 0x7ff) >> 3) - (((m >> 5) & 1) << 7)) & 0xff);
    ct[k + 6] = (unsigned char)((127 - ((((uint32_t)acc[k + 6] + 2) & 0x7ff) >> 3) - (((m >> 6) & 1) << 7)) & 0xff);
    ct[k + 7] = (unsigned char)((127 - ((((uint32_t)acc[k + 7] + 2) & 0x7ff) >> 3) - ((m >> 7) << 7)) & 0xff);
  }
#else
  unsigned int acc_shift = 0;
  unsigned int bpos = 0;
  uint32_t pack_acc = 0;

  for(unsigned int k = 0; k < RRLWR_N; k++) {
    pack_acc |= encrypt_t_coeff(acc[k], msg, out, k) << acc_shift;
    acc_shift += RRLWR_PKE_LOGT;
    while(acc_shift >= 8) {
      ct[bpos++] = (unsigned char)(pack_acc & 0xff);
      pack_acc >>= 8;
      acc_shift -= 8;
    }
  }
#endif
}

void poly_pack_message_from_acc_cm(unsigned char *m,
                                   const uint16_t acc[RRLWR_N],
                                   const unsigned char *cm)
{
#if RRLWR_PKE_LOGT == 3
  for(unsigned int i = 0; i < RRLWR_N / 8; i++) {
    unsigned int k = 8 * i;
    uint32_t c0 = cm[3 * i + 0] & 0x7;
    uint32_t c1 = (cm[3 * i + 0] >> 3) & 0x7;
    uint32_t c2 = ((cm[3 * i + 0] >> 6) | (cm[3 * i + 1] << 2)) & 0x7;
    uint32_t c3 = (cm[3 * i + 1] >> 1) & 0x7;
    uint32_t c4 = (cm[3 * i + 1] >> 4) & 0x7;
    uint32_t c5 = ((cm[3 * i + 1] >> 7) | (cm[3 * i + 2] << 1)) & 0x7;
    uint32_t c6 = (cm[3 * i + 2] >> 2) & 0x7;
    uint32_t c7 = cm[3 * i + 2] >> 5;
    uint32_t b0 = ((((((uint32_t)acc[k + 0] & 0x7ff) - ((3 - c0) << 8) + 126) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b1 = ((((((uint32_t)acc[k + 1] & 0x7ff) - ((3 - c1) << 8) + 126) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b2 = ((((((uint32_t)acc[k + 2] & 0x7ff) - ((3 - c2) << 8) + 126) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b3 = ((((((uint32_t)acc[k + 3] & 0x7ff) - ((3 - c3) << 8) + 126) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b4 = ((((((uint32_t)acc[k + 4] & 0x7ff) - ((3 - c4) << 8) + 126) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b5 = ((((((uint32_t)acc[k + 5] & 0x7ff) - ((3 - c5) << 8) + 126) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b6 = ((((((uint32_t)acc[k + 6] & 0x7ff) - ((3 - c6) << 8) + 126) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b7 = ((((((uint32_t)acc[k + 7] & 0x7ff) - ((3 - c7) << 8) + 126) & 0x7ff) + 512) >> 10) & 1;

    m[i] = (unsigned char)(b0 | (b1 << 1) | (b2 << 2) | (b3 << 3) |
                           (b4 << 4) | (b5 << 5) | (b6 << 6) | (b7 << 7));
  }
#elif RRLWR_PKE_LOGT == 8
  for(unsigned int i = 0; i < RRLWR_N / 8; i++) {
    unsigned int k = 8 * i;
    uint32_t c0 = cm[k + 0];
    uint32_t c1 = cm[k + 1];
    uint32_t c2 = cm[k + 2];
    uint32_t c3 = cm[k + 3];
    uint32_t c4 = cm[k + 4];
    uint32_t c5 = cm[k + 5];
    uint32_t c6 = cm[k + 6];
    uint32_t c7 = cm[k + 7];
    uint32_t b0 = ((((((uint32_t)acc[k + 0] & 0x7ff) - ((127 - c0) << 3) + 2) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b1 = ((((((uint32_t)acc[k + 1] & 0x7ff) - ((127 - c1) << 3) + 2) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b2 = ((((((uint32_t)acc[k + 2] & 0x7ff) - ((127 - c2) << 3) + 2) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b3 = ((((((uint32_t)acc[k + 3] & 0x7ff) - ((127 - c3) << 3) + 2) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b4 = ((((((uint32_t)acc[k + 4] & 0x7ff) - ((127 - c4) << 3) + 2) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b5 = ((((((uint32_t)acc[k + 5] & 0x7ff) - ((127 - c5) << 3) + 2) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b6 = ((((((uint32_t)acc[k + 6] & 0x7ff) - ((127 - c6) << 3) + 2) & 0x7ff) + 512) >> 10) & 1;
    uint32_t b7 = ((((((uint32_t)acc[k + 7] & 0x7ff) - ((127 - c7) << 3) + 2) & 0x7ff) + 512) >> 10) & 1;

    m[i] = (unsigned char)(b0 | (b1 << 1) | (b2 << 2) | (b3 << 3) |
                           (b4 << 4) | (b5 << 5) | (b6 << 6) | (b7 << 7));
  }
#else
  for(unsigned int i = 0; i < RRLWR_N / 8; i++) {
    m[i] = 0;
  }
#endif
}

/// @brief Pack a polynomial with coefficients in [-bitlen/2, bitlen/2-1] into a byte string of bitlen bits per coefficient
void poly_pack(unsigned char *b, poly *r, int32_t bitlen) {
  if(bitlen == 2) {
    poly_pack_2(b, r);
    return;
  }

  if(bitlen == 11) {
    poly_pack_11(b, r);
    return;
  }

  unsigned int i;
  unsigned int acc_shift = 0; 
  unsigned int bpos = 0;
  poly rp;
  int16_t *rc = rp.coeffs;
  uint32_t acc = 0;

  // Make all coefficients from r positive
  for(i = 0; i < RRLWR_N; i++) {
    rp.coeffs[i] = (((int32_t)1<<(bitlen-1))-1)-r->coeffs[i]; // Subtract from 2^bitlen/2-1 to move to interval [0, bitlen-1]
    rp.coeffs[i] &= ((int32_t)1<<bitlen)-1; // Remove any remaining sign bits
  }

  while (rc < rp.coeffs + RRLWR_N) {

      // Main packing loop
      for(i = 0; i < 32; i++) {
        acc |= ((uint32_t)rc[i]) << acc_shift; // Take the next bitlen bits
        acc_shift += bitlen;
        while (acc_shift >= 8) {
          b[bpos++] = (unsigned char)(acc & 0xFF);
          acc >>= 8;
          acc_shift -= 8;
        }
      }

    rc += 32; // Unpack 32 coefficients at a time
  }
}

void ring_pack(unsigned char *b, ring_element *r, int32_t bitlen) {
  unsigned int offset = bitlen*(RRLWR_N>>3);
  for (unsigned int i = 0; i < RRLWR_K; i++) {
    poly_pack(b+i*offset, &r->x[i], bitlen);
  }
}

/// @brief Unpack a polynomial with coefficients in [-bitlen/2, bitlen/2-1] from a byte string of bitlen bits per coefficient
void poly_unpack(poly *r, const unsigned char *b, int32_t bitlen) {
  if(bitlen == 2) {
    poly_unpack_2(r, b);
    return;
  }

  if(bitlen == 13) {
    poly_unpack_13(r, b);
    return;
  }

  unsigned int i;
  int32_t acc_shift = 0; 
  unsigned int bpos = 0;
  int16_t *rc = r->coeffs;
  uint32_t acc = 0;

  while (rc < r->coeffs + RRLWR_N) {

      // Main unpacking loop
      for(i = 0; i < 32; i++) {
        while (acc_shift < bitlen) {
          acc |= ((uint32_t)b[bpos++]) << acc_shift;
          acc_shift += 8;
        }
        rc[i] = (int16_t)(acc & (((int32_t)1 << bitlen)-1));
        acc >>= bitlen;
        acc_shift -= bitlen;
      }

    rc += 32; // Unpack 32 coefficients at a time
  }

  // Make all coefficients from r signed
  for(i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = (int16_t)((((int32_t)1<<(bitlen-1))-1)-r->coeffs[i]); // Subtract from 2^bitlen/2-1 to move to interval [-bitlen/2, bitlen/2-1]
  }
}

void ring_unpack(ring_element *r, const unsigned char *b, int32_t bitlen) {
  unsigned int offset = bitlen*(RRLWR_N>>3);
  for (unsigned int i = 0; i < RRLWR_K; i++) {
    poly_unpack(&r->x[i], b+i*offset, bitlen);
  }
}
