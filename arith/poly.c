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

#include "poly.h"

static int16_t reduce_mod_q(int32_t x)
{
  x &= ((int32_t)1 << RRLWR_PKE_LOGQ) - 1;
  if(x >= ((int32_t)1 << (RRLWR_PKE_LOGQ - 1))) {
    x -= (int32_t)1 << RRLWR_PKE_LOGQ;
  }

  return (int16_t)x;
}

void poly_mul_schoolbook(poly *r, const poly *f, const poly *g)
{
  int32_t acc[RRLWR_N] = {0};

  for(unsigned int i = 0; i < RRLWR_N; i++) {
    for(unsigned int j = 0; j < RRLWR_N; j++) {
      int32_t prod = (int32_t)f->coeffs[i] * g->coeffs[j];
      unsigned int d = i + j;

      if(d < RRLWR_N) {
        acc[d] += prod;
      } else {
        acc[d - RRLWR_N] -= prod;
      }
    }
  }

  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = reduce_mod_q(acc[i]);
  }
}

void poly_add(poly *r, poly *f, poly *g) {
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = reduce_mod_q((int32_t)f->coeffs[i] + g->coeffs[i]);
  }
}

void poly_sub(poly *r, poly *f, poly *g) {
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = reduce_mod_q((int32_t)f->coeffs[i] - g->coeffs[i]);
  }
}

/// @brief Reduce the input modulo 2**d into signed interval [-2**d/2, 2**d/2-1]
void poly_reduce_pow2(poly *r, poly *f, int32_t d) {
  int32_t pow2div2 = (int32_t)1 << (d-1); // 2^d/2
  int32_t pow2 = (int32_t)1 << d;         // 2^d
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = (int16_t)(((f->coeffs[i] + pow2div2) & (pow2-1)) - pow2div2);
  }
}

void poly_round_xtoy(poly *r, const poly *f, int32_t x, int32_t y) {
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    int32_t c = f->coeffs[i] + ((int32_t)1 << (x-(y+1))); // Add constant x/(2*y)
    c >>= (x-y);                                          // Divide by x/y and floor
    c &= ((int32_t)1 << y)-1;                             // Reduce mod y
    r->coeffs[i] = (int16_t)c;
  }
}

void poly_compress(poly *r, int32_t x) {
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = (int16_t)(r->coeffs[i] >> x);
  }
}

void poly_decompress(poly *r, int32_t x) {
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = (int16_t)(r->coeffs[i] << x);
  }
}
