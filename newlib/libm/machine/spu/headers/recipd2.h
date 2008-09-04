/* --------------------------------------------------------------  */
/* (C)Copyright 2001,2008,                                         */
/* International Business Machines Corporation,                    */
/* Sony Computer Entertainment, Incorporated,                      */
/* Toshiba Corporation,                                            */
/*                                                                 */
/* All Rights Reserved.                                            */
/*                                                                 */
/* Redistribution and use in source and binary forms, with or      */
/* without modification, are permitted provided that the           */
/* following conditions are met:                                   */
/*                                                                 */
/* - Redistributions of source code must retain the above copyright*/
/*   notice, this list of conditions and the following disclaimer. */
/*                                                                 */
/* - Redistributions in binary form must reproduce the above       */
/*   copyright notice, this list of conditions and the following   */
/*   disclaimer in the documentation and/or other materials        */
/*   provided with the distribution.                               */
/*                                                                 */
/* - Neither the name of IBM Corporation nor the names of its      */
/*   contributors may be used to endorse or promote products       */
/*   derived from this software without specific prior written     */
/*   permission.                                                   */
/*                                                                 */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND          */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,     */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            */
/* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT    */
/* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    */
/* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN       */
/* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR    */
/* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              */
/* --------------------------------------------------------------  */
/* PROLOG END TAG zYx                                              */
#ifdef __SPU__

#ifndef _RECIPD2_H_
#define _RECIPD2_H_		1

#include <spu_intrinsics.h>


/*
 * FUNCTION
 *	vector double _recipd2(vector double value)
 * 
 * DESCRIPTION
 * 	The _recipd2 function inverts "value" and returns the result. 
 *      Computation is performed using the single precision reciprocal 
 *      estimate and interpolate instructions to produce a 12 accurate 
 *      estimate.
 *
 *	One (1) iteration of a Newton-Raphson is performed to improve 
 *	accuracy to single precision floating point. Two additional double 
 *	precision iterations are  needed to achieve a full double
 *	preicision result.
 *
 *	The Newton-Raphson iteration is of the form:
 *		X[i+1] = X[i] * (2.0 - b*X[i]) 
 * 	where b is the input value to be inverted
 *
 */ 
static __inline vector double _recipd2(vector double value_d)
{
  vector unsigned long long expmask  = (vector unsigned long long) { 0x7FF0000000000000ULL, 0x7FF0000000000000ULL };
  vector float  x0;
  vector float  value;
  vector float  two   = spu_splats(2.0f);
  vector double two_d = spu_splats(2.0);
  vector double x1, x2, x3;
  vector double bias;

  /* Bias the divisor to correct for double precision floating
   * point exponents that are out of single precision range.
   */
  bias = spu_xor(spu_and(value_d, (vector double)expmask), (vector double)expmask);

  value = spu_roundtf(spu_mul(value_d, bias));
  x0 = spu_re(value);
  x1 = spu_extend(spu_mul(x0, spu_nmsub(value, x0, two)));
  x1 = spu_mul(x1, bias);
  x2 = spu_mul(x1, spu_nmsub(value_d, x1, two_d));
  x3 = spu_mul(x2, spu_nmsub(value_d, x2, two_d));

  /* Handle input = +/- infinity or +/-0. */

#ifdef __SPU_EDP__
  vec_ullong2 is0inf = spu_testsv(value_d, SPU_SV_NEG_ZERO     | SPU_SV_POS_ZERO |
                                           SPU_SV_NEG_INFINITY | SPU_SV_POS_INFINITY);
#else
  vec_double2 nzero = spu_splats(-0.0);
  vec_double2 xabs = spu_andc(value_d, nzero);
  vector unsigned char swap  = (vector unsigned char) {4,5,6,7, 0,1,2,3, 12,13,14,15, 8,9,10,11};
  vec_uint4 isinf  = spu_cmpeq((vec_uint4)xabs, (vec_uint4)expmask);
  vec_uint4 iszero = spu_cmpeq((vec_uint4)xabs, 0);
  isinf  = spu_and(isinf,  spu_shuffle(isinf, isinf, swap));
  iszero = spu_and(iszero, spu_shuffle(iszero, iszero, swap));
  vec_ullong2 is0inf = (vec_ullong2)spu_or(isinf, iszero);
#endif /* __SPU_EDP__ */

  x3 = spu_sel(x3, spu_xor(value_d, (vector double)expmask), is0inf);

  return (x3);
}

#endif /* _RECIPD2_H_ */
#endif /* __SPU__ */
