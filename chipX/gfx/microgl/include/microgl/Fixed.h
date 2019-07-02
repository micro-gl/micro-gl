#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma once

#include <stdlib.h>
#include <math.h>
/* DEFINE THE MACROS */
/* The basic operations perfomed on two numbers a and b of fixed
point q format returning the answer in q format */
#define Q 16 // number of bits for fractional part

#define float_to_fixed_2(a, q) ((int)( (a)*(float)(1<<(q)) ))
#define fixed_to_float_2(a, q) ( (float)(a) / (float)(1<<(q)) )
#define int_to_fixed_2(a, q) ( (a)<<(q) )
#define fixed_to_int_2(a, q) ( (a)>>(q) )

#define float_to_fixed(a) (float_to_fixed_2(a, Q))
#define fixed_to_float(a) (fixed_to_float_2(a, Q))
#define int_to_fixed(a) (int_to_fixed_2(a, Q))
#define fixed_to_int(a) (fixed_to_int_2(a, Q))

#define fixed_add_fixed(a,b) ((a)+(b))
#define fixed_sub_fixed(a,b) (fixed_add_fixed(a,-b))
#define fixed_mul_fixed_2(a,b,q) (((a)*(b))>>(q))
#define fixed_div_fixed_2(a,b,q) (((a)<<(q))/(b))
#define fixed_mul_fixed(a,b) (fixed_mul_fixed_2(a,b,Q))
#define fixed_div_fixed(a,b) (fixed_div_fixed_2(a,b,Q))

/* The basic operations where a is of fixed point q format and b is
an integer */
#define fixed_add_int2(a,b,q) ((a)+int_to_fixed_2((b), (q)))
#define fixed_sub_int2(a,b,q) (fixed_add_int2(a, -b, q))
#define fixed_mul_int(a,b) ((a)*(b))
#define fixed_div_int(a,b) ((a)/(b))
#define fixed_one_over_int(a) fixed_div_int(fixed_one, a)
#define fixed_add_int(a,b) (fixed_add_int2(a,b,Q))
#define fixed_sub_int(a,b) (fixed_add_int(a, -b))

/* convert a from q1 format to q2 format */
#define fixed_convert_fixed(a, q1, q2) (((q2)>(q1)) ? (a)<<((q2)-(q1)) : (a)>>((q1)-(q2)))
/* the general operation between a in q1 format and b in q2 format
returning the result in q3 format */

#define fixed_add_fixed_3(a,b,q1,q2,q3) (fixed_convert_fixed(a,q1,q3)+fixed_convert_fixed(b,q2,q3))
#define fixed_sub_fixed_3(a,b,q1,q2,q3) (fixed_add_fixed_3(a,-b,q1,q2,q3))
#define fixed_mul_fixed_3(a,b,q1,q2,q3) (fixed_convert_fixed((a)*(b), (q1)+(q2), q3))
#define fixed_div_fixed_3(a,b,q1,q2,q3) (fixed_convert_fixed(a, q1, (q2)+(q3))/(b))

#define fixed_half float_to_fixed(0.5f)
#define fixed_one int_to_fixed(1)

#pragma clang diagnostic pop