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
