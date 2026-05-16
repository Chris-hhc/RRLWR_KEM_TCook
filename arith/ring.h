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

#ifndef RING_H
#define RING_H

#include "poly.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct{
    poly x[RRLWR_K];
  } ring_element;

  typedef struct{
    poly x[2 * RRLWR_K - 1];
  } ring_element_Awin;

  void ring_uniform_Awin(ring_element_Awin *aw, int32_t bitlen, const unsigned char *seed, int32_t seed_len);
  void ring_to_Awin(ring_element_Awin *aw, const ring_element *a);
  void ring_mul_Awin(poly *r, const ring_element_Awin *a, const ring_element *b, int ncoeffs);
  void ring_mul_Awin_round_p(poly *r, const ring_element_Awin *a, const ring_element *b, int ncoeffs);
  void ring_mul_Awin_add_msg_pack_t(unsigned char *ct,
                                    const ring_element_Awin *a,
                                    const ring_element *b,
                                    const unsigned char msg[RRLWR_PKE_MESSAGE_LEN]);
  void ring_mul_Awin_sub_cm_pack_msg(unsigned char *m,
                                     const ring_element_Awin *a,
                                     const ring_element *b,
                                     const unsigned char *ct);
  void ring_mul(poly *r, const ring_element *a, const ring_element *b, int ncoeffs);
  void ring_round_xtoy(ring_element *r, const ring_element *f, int32_t x, int32_t y);

#ifdef __cplusplus
}
#endif

#endif
