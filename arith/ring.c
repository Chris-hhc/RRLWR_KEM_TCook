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

#include "ring.h"
#include "uniform.h"

static int16_t reduce_mod_q(int32_t x)
{
  x &= ((int32_t)1 << RRLWR_PKE_LOGQ) - 1;
  if(x >= ((int32_t)1 << (RRLWR_PKE_LOGQ - 1))) {
    x -= (int32_t)1 << RRLWR_PKE_LOGQ;
  }

  return (int16_t)x;
}

static void poly_zero(poly *r)
{
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = 0;
  }
}

static void poly_accumulate(poly *r, const poly *f)
{
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = reduce_mod_q((int32_t)r->coeffs[i] + f->coeffs[i]);
  }
}

static void poly_mul_x_plus_2(poly *r, const poly *f)
{
  int32_t prev = f->coeffs[RRLWR_N - 1];

  for(unsigned int i = 0; i < RRLWR_N; i++) {
    int32_t xterm = (i == 0) ? -prev : f->coeffs[i - 1];
    r->coeffs[i] = reduce_mod_q(2 * (int32_t)f->coeffs[i] + xterm);
  }
}

void ring_to_Awin(ring_element_Awin *aw, const ring_element *a)
{
  for(unsigned int u = 0; u < RRLWR_K; u++) {
    aw->x[RRLWR_K - 1 - u] = a->x[u];
  }

  for(unsigned int u = 1; u < RRLWR_K; u++) {
    unsigned int base = RRLWR_K - 1 - u;
    unsigned int dst = 2 * RRLWR_K - 1 - u;

    poly_mul_x_plus_2(&aw->x[dst], &aw->x[base]);
  }
}

void ring_uniform_Awin(ring_element_Awin *aw,
                       int32_t bitlen,
                       const unsigned char *seed,
                       int32_t seed_len)
{
  poly a;

  for(unsigned int u = 0; u < RRLWR_K; u++) {
    poly_uniform(&a, bitlen, seed, seed_len, (unsigned char)u);
    aw->x[RRLWR_K - 1 - u] = a;
  }

  for(unsigned int u = 1; u < RRLWR_K; u++) {
    unsigned int base = RRLWR_K - 1 - u;
    unsigned int dst = 2 * RRLWR_K - 1 - u;

    poly_mul_x_plus_2(&aw->x[dst], &aw->x[base]);
  }
}

/// @brief Ring multiplication over R_q using precomputed A-window rows.
void ring_mul_Awin(poly *r,
                   const ring_element_Awin *a,
                   const ring_element *b,
                   int ncoeffs)
{
  int row_min = RRLWR_K - ncoeffs;
  poly t;

  for(int i = RRLWR_K - 1; i >= row_min; i--) {
    int out = i - row_min;
    const poly *row = &a->x[RRLWR_K - 1 - i];

    poly_zero(&r[out]);

    for(int j = 0; j < RRLWR_K; j++) {
      poly_mul_toom4(&t, &row[j], &b->x[j]);
      poly_accumulate(&r[out], &t);
    }
  }
}

/// @brief Ring multiplication over R_q with schoolbook polynomial products.
void ring_mul(poly *r, const ring_element *a, const ring_element *b, int ncoeffs)
{
  ring_element_Awin aw;

  ring_to_Awin(&aw, a);
  ring_mul_Awin(r, &aw, b, ncoeffs);
}

void ring_round_xtoy(ring_element *r, const ring_element *f, int32_t x, int32_t y) {
  for(unsigned int i = 0; i < RRLWR_K; i++) {
    poly_round_xtoy(&r->x[i], &f->x[i], x, y);
  }
}
