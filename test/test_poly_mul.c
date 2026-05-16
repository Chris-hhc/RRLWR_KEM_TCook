/*
 * Test polynomial multiplication against an int64 schoolbook reference over
 * R_q = Z_q[x]/(x^N + 1), using the active parameters.h configuration.
 */

#include <stdint.h>
#include <stdio.h>

#include "parameters.h"
#include "poly.h"

#define NTESTS 1000

static uint32_t rng_state = 1;

static uint32_t xorshift32(void)
{
  uint32_t x = rng_state;

  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng_state = x;

  return x;
}

static int16_t reduce_mod_q_i64(int64_t x)
{
  int64_t q = (int64_t)1 << RRLWR_PKE_LOGQ;

  x &= q - 1;
  if(x >= q / 2) {
    x -= q;
  }

  return (int16_t)x;
}

static void random_poly_mod_q(poly *r)
{
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = reduce_mod_q_i64((int64_t)xorshift32());
  }
}

static void clear_poly(poly *r)
{
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = 0;
  }
}

static void ref_mul_negacyclic(poly *r, const poly *a, const poly *b)
{
  int64_t acc[RRLWR_N] = {0};

  for(unsigned int i = 0; i < RRLWR_N; i++) {
    for(unsigned int j = 0; j < RRLWR_N; j++) {
      int64_t prod = (int64_t)a->coeffs[i] * b->coeffs[j];
      unsigned int d = i + j;

      if(d < RRLWR_N) {
        acc[d] += prod;
      } else {
        acc[d - RRLWR_N] -= prod;
      }
    }
  }

  for(unsigned int i = 0; i < RRLWR_N; i++) {
    r->coeffs[i] = reduce_mod_q_i64(acc[i]);
  }
}

static int coeffs_equal(const poly *a, const poly *b, unsigned int *bad)
{
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    if(a->coeffs[i] != b->coeffs[i]) {
      *bad = i;
      return 0;
    }
  }

  return 1;
}

static int check_case(const char *name, const poly *a, const poly *b)
{
  poly ref;
  poly toom;
  poly school;
  unsigned int bad = 0;

  ref_mul_negacyclic(&ref, a, b);
  poly_mul_toom4(&toom, a, b);
  poly_mul_schoolbook(&school, a, b);

  if(!coeffs_equal(&toom, &ref, &bad)) {
    printf("FAIL %s: toom coeff[%u]=%d ref=%d\n",
           name,
           bad,
           toom.coeffs[bad],
           ref.coeffs[bad]);
    return -1;
  }

  if(!coeffs_equal(&school, &ref, &bad)) {
    printf("FAIL %s: school coeff[%u]=%d ref=%d\n",
           name,
           bad,
           school.coeffs[bad],
           ref.coeffs[bad]);
    return -1;
  }

  return 0;
}

static int check_fixed_cases(void)
{
  poly a;
  poly b;

  clear_poly(&a);
  clear_poly(&b);
  if(check_case("zero", &a, &b) != 0) {
    return -1;
  }

  a.coeffs[0] = 1;
  b.coeffs[0] = 1;
  if(check_case("one", &a, &b) != 0) {
    return -1;
  }

  clear_poly(&a);
  clear_poly(&b);
  a.coeffs[RRLWR_N - 1] = 1;
  b.coeffs[1] = 1;
  if(check_case("negacyclic_wrap", &a, &b) != 0) {
    return -1;
  }

  clear_poly(&a);
  clear_poly(&b);
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    a.coeffs[i] = (int16_t)((int)i % 17 - 8);
    b.coeffs[i] = (int16_t)(7 - (int)i % 13);
  }
  if(check_case("small_pattern", &a, &b) != 0) {
    return -1;
  }

  return 0;
}

int main(void)
{
  poly a;
  poly b;

  if(check_fixed_cases() != 0) {
    return 1;
  }

  for(unsigned int t = 0; t < NTESTS; t++) {
    rng_state = 0x9e3779b9u ^ (uint32_t)(RRLWR_SECURITY_LEVEL * 1009u + t);
    random_poly_mod_q(&a);
    random_poly_mod_q(&b);

    if(check_case("random", &a, &b) != 0) {
      printf("test index: %u, security level: %d, N: %d, logq: %d\n",
             t,
             RRLWR_SECURITY_LEVEL,
             RRLWR_N,
             RRLWR_PKE_LOGQ);
      return 1;
    }
  }

  printf("Success: poly_mul_toom4 and poly_mul_schoolbook match int64 reference ");
  printf("for %u tests at security level %d (N=%d, logq=%d).\n",
         NTESTS,
         RRLWR_SECURITY_LEVEL,
         RRLWR_N,
         RRLWR_PKE_LOGQ);

  return 0;
}
