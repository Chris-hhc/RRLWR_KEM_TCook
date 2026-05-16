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

#include "uniform.h"

#include <stddef.h>
#include <string.h>

/// @brief Generate a polynomial with coefficients pseudo-randomly generated in the uniform distribution [-bitlen/2, bitlen/2-1]
///        The seed (of length seed_len) is appended with coeff that is used as ring coefficient index.
#define RRLWR_MAX_SAMPLING_BITLEN (24)  // Support sampling bit lengths up to 24 bits, only required to define buffer size
#define RRLWR_MAX_SEED_LEN        (128) // Support seed lengths up to 128 bytes, only required to define buffer size
#define RRLWR_MAX_OUTLEN          ((RRLWR_PKE_LOGQ * RRLWR_N/4*RRLWR_K) / 8)
#define RRLWR_XOF_BUFLEN          (((RRLWR_MAX_OUTLEN + SHAKE128_RATE - 1) / SHAKE128_RATE) * SHAKE128_RATE)

void poly_uniform(poly *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len, unsigned char coeff) {

  unsigned char xof_bytes_buffer[RRLWR_MAX_SAMPLING_BITLEN*(RRLWR_N >> 3)]; // Allocate maximum length
  unsigned char seed_buffer[RRLWR_MAX_SEED_LEN+1];                          // Allocate maximum length

  // Concatenate the seed with the ring element index i
  for(int32_t i = 0; i < seed_len; i++) {
    seed_buffer[i] = seed[i];
  }
  seed_buffer[seed_len] = coeff;

  // Generate the output byte stream
  RRLWR_XOF(xof_bytes_buffer, bitlen*(RRLWR_N >> 3), seed_buffer, seed_len+1);

  // Unpack into polynomial coefficients
  poly_unpack(r, xof_bytes_buffer, bitlen);
}

static void squeeze_seed_nonce(uint8_t *out, size_t nblocks,
                               const uint8_t *seed, int32_t seed_len,
                               uint8_t nonce)
{
  uint8_t seed_buffer[RRLWR_MAX_SEED_LEN + 1];
  keccak_state state;

  memcpy(seed_buffer, seed, (size_t)seed_len);
  seed_buffer[seed_len] = nonce;

  shake128_absorb_once(&state, seed_buffer, (size_t)seed_len + 1);
  shake128_squeezeblocks(out, nblocks, &state);
}

static void poly_uniform_kx(poly **r, unsigned int npolys,
                            int32_t bitlen, const uint8_t *seed, int32_t seed_len,
                            uint8_t nonce0, uint8_t nonce1,
                            uint8_t nonce2, uint8_t nonce3)
{
  size_t outlen = (size_t)bitlen * (RRLWR_N >> 3);
  size_t nblocks = (((outlen >> 2) * npolys) + SHAKE128_RATE - 1) / SHAKE128_RATE;
  uint8_t buf[4 * RRLWR_XOF_BUFLEN];

  squeeze_seed_nonce(buf + 0 * nblocks * SHAKE128_RATE, nblocks, seed, seed_len, nonce0);
  squeeze_seed_nonce(buf + 1 * nblocks * SHAKE128_RATE, nblocks, seed, seed_len, nonce1);
  squeeze_seed_nonce(buf + 2 * nblocks * SHAKE128_RATE, nblocks, seed, seed_len, nonce2);
  squeeze_seed_nonce(buf + 3 * nblocks * SHAKE128_RATE, nblocks, seed, seed_len, nonce3);

  for(unsigned int i = 0; i < npolys; i++) {
    poly_unpack(r[i], buf + i * outlen, bitlen);
  }
}

#if (RRLWR_K == 5)
static void poly_uniform_5x(poly *r0, poly *r1, poly *r2, poly *r3, poly *r4,
                            int32_t bitlen, const uint8_t *seed, int32_t seed_len,
                            uint8_t nonce0, uint8_t nonce1,
                            uint8_t nonce2, uint8_t nonce3)
{
  poly *r[5] = {r0, r1, r2, r3, r4};
  poly_uniform_kx(r, 5, bitlen, seed, seed_len, nonce0, nonce1, nonce2, nonce3);
}
#elif (RRLWR_K == 9)
static void poly_uniform_9x(poly *r0, poly *r1, poly *r2, poly *r3, poly *r4,
                            poly *r5, poly *r6, poly *r7, poly *r8,
                            int32_t bitlen, const uint8_t *seed, int32_t seed_len,
                            uint8_t nonce0, uint8_t nonce1,
                            uint8_t nonce2, uint8_t nonce3)
{
  poly *r[9] = {r0, r1, r2, r3, r4, r5, r6, r7, r8};
  poly_uniform_kx(r, 9, bitlen, seed, seed_len, nonce0, nonce1, nonce2, nonce3);
}
#elif (RRLWR_K == 17)
static void poly_uniform_17x(poly *r0, poly *r1, poly *r2, poly *r3, poly *r4,
                             poly *r5, poly *r6, poly *r7, poly *r8, poly *r9,
                             poly *r10, poly *r11, poly *r12, poly *r13,
                             poly *r14, poly *r15, poly *r16,
                             int32_t bitlen, const uint8_t *seed, int32_t seed_len,
                             uint8_t nonce0, uint8_t nonce1,
                             uint8_t nonce2, uint8_t nonce3)
{
  poly *r[17] = {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16};
  poly_uniform_kx(r, 17, bitlen, seed, seed_len, nonce0, nonce1, nonce2, nonce3);
}
#endif

/// @brief Generate a ring element with coefficients pseudo-randomly generated in the uniform distribution [-bitlen/2, bitlen/2-1]
void ring_uniform(ring_element *r, int32_t bitlen, const unsigned char *seed, int32_t seed_len) {
#if (RRLWR_K == 5)
  poly_uniform_5x(&r->x[0], &r->x[1], &r->x[2], &r->x[3], &r->x[4],
                  bitlen, seed, seed_len, 0, 1, 2, 3);
#elif (RRLWR_K == 9)
  poly_uniform_9x(&r->x[0], &r->x[1], &r->x[2], &r->x[3], &r->x[4],
                  &r->x[5], &r->x[6], &r->x[7], &r->x[8],
                  bitlen, seed, seed_len, 0, 1, 2, 3);
#elif (RRLWR_K == 17)
  poly_uniform_17x(&r->x[0], &r->x[1], &r->x[2], &r->x[3], &r->x[4],
                   &r->x[5], &r->x[6], &r->x[7], &r->x[8], &r->x[9],
                   &r->x[10], &r->x[11], &r->x[12], &r->x[13],
                   &r->x[14], &r->x[15], &r->x[16],
                   bitlen, seed, seed_len, 0, 1, 2, 3);
#else
#error "RRLWR_K must be 5, 9, or 17"
#endif
}
