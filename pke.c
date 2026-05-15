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

#include "pke.h"

void poly_subp(poly *r, const poly *f, const poly *g) {
  for(unsigned int i = 0; i < RRLWR_N; i++) {
    int32_t h2 = (int32_t)1 << (RRLWR_PKE_LOGP-(RRLWR_PKE_LOGT+1));      // p/(2*t)
    h2 -= (int32_t)1 << (RRLWR_PKE_LOGQ-(RRLWR_PKE_LOGP+1));             // p/(2*t) - q/(2*p)
    r->coeffs[i] = (f->coeffs[i] - g->coeffs[i] + h2) & (RRLWR_PKE_P-1); // Reduce mod p
  }
}

void poly_add_msg(poly *r, const unsigned char msg[RRLWR_PKE_MESSAGE_LEN]) {
  int32_t mj;

  for(unsigned int i = 0; i < RRLWR_PKE_ELL; i++) {
    for(unsigned int j = 0; j < RRLWR_N; j++) {
      mj = (msg[i*RRLWR_N/8 + (j >> 3)] >> (j & 0x7)) & 1;                 // Select coefficient j of polynomial i
      r[i].coeffs[j] += (int32_t)1 << (RRLWR_PKE_LOGQ-(RRLWR_PKE_LOGP+1)); // Add q/(2*p)
      r[i].coeffs[j] += -mj & ((int32_t)1 << (RRLWR_PKE_LOGP-1));          // Add p/2 if message bit is 1, otherwise not
      r[i].coeffs[j] &= (RRLWR_PKE_P-1);                                   // Reduce mod p
    }
  }
}

uint8_t ct_cmp(const unsigned char *c1, const unsigned char *c2) {
  uint8_t r = 0;
  for(unsigned int i = 0; i < RRLWR_KEM_CT_LEN; i++) {
    r |= (c1[i] ^ c2[i]); // Only non-zero if c1 != c2
  }

  return -((-(uint32_t)r) >> 31);
}

int pke_keygen(unsigned char pk[RRLWR_PKE_PK_LEN], unsigned char sk[RRLWR_PKE_SK_LEN],
               const unsigned char seedA[RRLWR_PKE_SEED_A_LEN], const unsigned char seedS[RRLWR_SEED_S_LEN]) {

  ring_element_Awin a;
  ring_element s, b;

  // Generate a with coefficients in [-q/2+1, q/2]
  ring_uniform_Awin(&a, RRLWR_PKE_LOGQ, seedA, RRLWR_PKE_SEED_A_LEN);

  // Generate s with coefficients in [-2, 1] and pack
  ring_uniform(&s, RRLWR_PKE_LOG_ETA+1, seedS, RRLWR_SEED_S_LEN);
  ring_pack(sk, &s, RRLWR_PKE_LOG_ETA+1);

  // Compute A*s
  ring_mul_Awin(b.x, &a, &s, RRLWR_K);

  // Compute b = round(p/q*A*s) computed as (A*s + q/(2*p)) >> (eq - ep)
  ring_round_xtoy(&b, &b, RRLWR_PKE_LOGQ, RRLWR_PKE_LOGP);

  // Pack the public key
  for (unsigned int i = 0; i < RRLWR_PKE_SEED_A_LEN; i++) {
    pk[i] = seedA[i];
  }
  ring_pack(pk + RRLWR_PKE_SEED_A_LEN, &b, RRLWR_PKE_LOGP);

  return 0;
}

int pke_encrypt(unsigned char ct[RRLWR_PKE_CT_LEN], const unsigned char pk[RRLWR_PKE_PK_LEN],
                const unsigned char m[RRLWR_PKE_MESSAGE_LEN], const unsigned char seedSp[RRLWR_SEED_S_LEN]) {

  ring_element_Awin a;
  ring_element_Awin bw;
  ring_element sp, b, bp;
  poly vp[RRLWR_PKE_ELL];
  const unsigned char *seedA = &pk[0];

  // Generate a with coefficients in [-q/2+1, q/2]
  ring_uniform_Awin(&a, RRLWR_PKE_LOGQ, seedA, RRLWR_PKE_SEED_A_LEN);

  // Generate s_prime with coefficients in [-2, 1]
  ring_uniform(&sp, RRLWR_PKE_LOG_ETA+1, seedSp, RRLWR_SEED_S_LEN);

  // Compute A*s_prime
  ring_mul_Awin(bp.x, &a, &sp, RRLWR_K);

  // Compute b_prime = round(p/q*A*s_prime)
  ring_round_xtoy(&bp, &bp, RRLWR_PKE_LOGQ, RRLWR_PKE_LOGP);

  // Compute v_prime = b*s_prime
  ring_unpack(&b, pk + RRLWR_PKE_SEED_A_LEN, RRLWR_PKE_LOGP);
  ring_to_Awin(&bw, &b);
  ring_mul_Awin(vp, &bw, &sp, RRLWR_PKE_ELL);

  // Convert message to polynomial representation and compute (v_prime + q/(2*p) + p/2*m) mod p
  poly_add_msg(vp, m);

  // Round from R_p to R_t with result in [-t/2+1, t/2] and pack into ciphertext buffer
  for(unsigned int i = 0; i < RRLWR_PKE_ELL; i++) {
    poly_compress(&vp[i], RRLWR_PKE_LOGP - RRLWR_PKE_LOGT); // Multiply by t/p and floor
    poly_pack(ct + i*RRLWR_PKE_PACKED_POLYT_LEN, &vp[i], RRLWR_PKE_LOGT);
  }

  // Return ct = (cm = vp, b) and sk = s
  ring_pack(ct + RRLWR_PKE_ELL*RRLWR_PKE_PACKED_POLYT_LEN, &bp, RRLWR_PKE_LOGP);

  return 0;
}

int pke_decrypt(unsigned char m[RRLWR_PKE_MESSAGE_LEN], 
                const unsigned char ct[RRLWR_PKE_CT_LEN], const unsigned char sk[RRLWR_PKE_SK_LEN]) {
  ring_element bp, s;
  ring_element_Awin bpw;
  poly v[RRLWR_PKE_ELL];
  poly cm[RRLWR_PKE_ELL];

  // Unpack s, b and cm and decompress cm
  ring_unpack(&s, sk, RRLWR_PKE_LOG_ETA+1);
  ring_unpack(&bp, ct + RRLWR_PKE_ELL*RRLWR_PKE_PACKED_POLYT_LEN, RRLWR_PKE_LOGP);
  for(unsigned int i = 0; i < RRLWR_PKE_ELL; i++) {
    poly_unpack(&cm[i], ct + i*RRLWR_PKE_PACKED_POLYT_LEN, RRLWR_PKE_LOGT);
    poly_decompress(&cm[i], RRLWR_PKE_LOGP - RRLWR_PKE_LOGT); // Multiply by p/t
  }

  // Compute v = b_prime*s
  ring_to_Awin(&bpw, &bp);
  ring_mul_Awin(v, &bpw, &s, RRLWR_PKE_ELL); 

  for(unsigned int i = 0; i < RRLWR_PKE_ELL; i++) {
    poly_subp(&v[i], &v[i], &cm[i]);                     // Compute (v - (p/t)*cm) mod p
    poly_round_xtoy(&v[i], &v[i], RRLWR_PKE_LOGP, 1);    // Round to a single-bit message polynomial m'
    poly_pack(m + i*RRLWR_PKE_PACKED_POLY1_LEN, &v[i], 1); // Convert the message polynomial to a bit string
  }

  return 0;
}
