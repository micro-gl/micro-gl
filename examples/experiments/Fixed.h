#pragma once

#define float_to_fixed(a, q) ((long long)(0.0f + (a)*(float)(1<<(q)) ))
#define fixed_to_float(a, q) ( (float)(a) / (float)(1<<(q)) )
#define int_to_fixed(a, q) ( (long)(a)<<(q) )
#define fixed_to_int(a, q) ( ((a))>>(q) )

#define float_to_fixed(a) (float_to_fixed_2(a, Q_))
#define fixed_to_float(a) (fixed_to_float_2(a, Q_))
#define int_to_fixed(a) (int_to_fixed_2(a, Q_))
#define fixed_to_int(a) (fixed_to_int_2(a, Q_))

#define fixed_add(a,b) ((a)+(b))
#define fixed_sub(a,b) (fixed_add(a,-b))
#define fixed_mul(a,b,q) (((((long long)(a))*(b)))>>(q))
#define fixed_div(a,b,q) ((((long long)(a))<<(q))/(b))

/* convert a from q1 format to q2 format */
#define fixed_convert_fixed(a, q1, q2) (((q2)>(q1)) ? (a)<<((q2)-(q1)) : (a)>>((q1)-(q2)))
/* the general operation between a in q1 format and b in q2 format
returning the result in q3 format */

#define fixed_add_fixed_3(a,b,q1,q2,q3) (fixed_convert_fixed(a,q1,q3)+fixed_convert_fixed(b,q2,q3))
#define fixed_sub_fixed_3(a,b,q1,q2,q3) (fixed_add_fixed_3(a,-b,q1,q2,q3))
#define fixed_mul_fixed_3(a,b,q1,q2,q3) (fixed_convert_fixed((a)*(b), (q1)+(q2), q3))
#define fixed_div_fixed_3(a,b,q1,q2,q3) (fixed_convert_fixed(a, q1, (q2)+(q3))/(b))

#define MASK_FRACTION_FIXED(q) ((1<<q) - 1)
#define MASK_INTEGRAL_BITS(q) (~MASK_FRACTION_FIXED(q))

#define fixed_frac_part(a, q) ((a) & (MASK_FRACTION_FIXED(q)))
#define fixed_int_part(a, q) ((a) & (~MASK_FRACTION_FIXED(q)))
#define fixed_half(q) (1<<(q-1))
#define fixed_one(q) (1<<q)
#define fixed_floor(a, q) (fixed_int_part(a, q))
#define fixed_round(a, q) (fixed_floor((a) + fixed_half(q), q))
#define fixed_ceil(a, q) (fixed_frac_part(a, q)==0) ? (a) : fixed_floor((a) + fixed_one)

typedef int fixed_32;
typedef long long fixed_64;