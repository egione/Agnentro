/*
Dyspoissometer
Copyright 2017 Russell Leidich
http://dyspoissonism.blogspot.com

This collection of files constitutes the Dyspoissometer Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Dyspoissometer Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Dyspoissometer Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Dyspoissometer Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Constants
*/
#define BIT_CLEAR(base, idx) (((base)[(idx)>>ULONG_BITS_LOG2])&=(ULONG)(~((ULONG)(1)<<((idx)&ULONG_BIT_MAX))))
#define BIT_FLIP(base, idx) (((base)[(idx)>>ULONG_BITS_LOG2])^=(ULONG)((ULONG)(1)<<((idx)&ULONG_BIT_MAX)))
#define BIT_GET(base, idx) (((base)[(idx)>>ULONG_BITS_LOG2])>>((idx)&ULONG_BIT_MAX)&(ULONG)(1))
#define BIT_SET(base, idx) (((base)[(idx)>>ULONG_BITS_LOG2])|=(ULONG)((ULONG)(1)<<((idx)&ULONG_BIT_MAX)))
#define DELTA_GET(a, b, delta) (((a)<=(b))?(delta=(b)-(a)):(delta=(a)-(b)))
#define i16 int16_t
#define I16_BIT_MAX 15U
#define I16_BITS 16U
#define I16_BITS_LOG2 4U
#define I16_BYTE_MAX 1U
#define I16_MAX 0x7FFFL
#define I16_SIZE 2U
#define I16_SIZE_LOG2 1U
#define I24_BIT_MAX 23U
#define I24_BITS 24U
#define I24_BYTE_MAX 2U
#define I24_MAX 0x7FFFFFL
#define I24_SIZE 3U
#define i32 int32_t
#define I32_BIT_MAX 31U
#define I32_BITS 32U
#define I32_BITS_LOG2 5U
#define I32_BYTE_MAX 3U
#define I32_MAX 0x7FFFFFFFL
#define I32_SIZE 4U
#define I32_SIZE_LOG2 2U
#define i64 int64_t
#define I64_BIT_MAX 63U
#define I64_BITS 64U
#define I64_BITS_LOG2 6U
#define I64_BYTE_MAX 7U
#define I64_MAX 0x7FFFFFFFFFFFFFFFLL
#define I64_SIZE 8U
#define I64_SIZE_LOG2 3U
#define i8 int8_t
#define I8_BIT_MAX 7U
#define I8_BITS 8U
#define I8_BITS_LOG2 3U
#define I8_MAX 0x7FL
#define I8_SIZE 1U
#define I8_SIZE_LOG2 0U
/*
See https://en.wikipedia.org/wiki/Multiply-with-carry which explains how this Marsaglia pseudorandom number generator works.
*/
#define MARSAGLIA_ITERATE(marsaglia_c_u32, marsaglia_x_u32, random_u64) \
  marsaglia_c_u32=(u32)(random_u64>>U32_BITS); \
  marsaglia_x_u32=(u32)(random_u64); \
  random_u64=(0xFFFFFF4EU*(u64)(marsaglia_x_u32))+marsaglia_c_u32
#define MARSAGLIA_ITERATE_REVERSE(marsaglia_c_u32, marsaglia_x_u32, random_u64) \
  marsaglia_c_u32=(u32)(random_u64/0xFFFFFF4EU); \
  marsaglia_x_u32=(u32)(random_u64%0xFFFFFF4EU); \
  random_u64=((u64)(marsaglia_x_u32)<<U32_BITS)|marsaglia_c_u32
#define MAX(m, n) (((m)>=(n))?(m):(n))
#define MIN(m, n) (((m)<=(n))?(m):(n))
#define quad __float128
#define REVERSE_U8(n) \
  n=(u8)((((n)&0x55U)<<1)|(((n)&0xAAU)>>1)); \
  n=(u8)((((n)&0x33U)<<2)|(((n)&0xCCU)>>2)); \
  n=(u8)((((n)&0x0FU)<<4)|(((n)&0xF0U)>>4))
#define REVERSE_U16(n) \
  n=(u16)((((n)&0x5555U)<<1)|(((n)&0xAAAAU)>>1)); \
  n=(u16)((((n)&0x3333U)<<2)|(((n)&0xCCCCU)>>2)); \
  n=(u16)((((n)&0x0F0FU)<<4)|(((n)&0xF0F0U)>>4)); \
  n=(u16)(((n)<<8)|((n)>>8))
#define REVERSE_U32(n) \
  n=(u32)((((n)&0x55555555U)<<1)|(((n)&0xAAAAAAAAU)>>1)); \
  n=(u32)((((n)&0x33333333U)<<2)|(((n)&0xCCCCCCCCU)>>2)); \
  n=(u32)((((n)&0x0F0F0F0FU)<<4)|(((n)&0xF0F0F0F0U)>>4)); \
  n=(u32)((((n)&0x00FF00FFU)<<8)|(((n)&0xFF00FF00U)>>8)); \
  n=(u32)(((n)>>16)|((n)<<16))
#define REVERSE_U64(n) \
  n=(u64)((((n)&0x5555555555555555ULL)<<1)|(((n)&0xAAAAAAAAAAAAAAAAULL)>>1)); \
  n=(u64)((((n)&0x3333333333333333ULL)<<2)|(((n)&0xCCCCCCCCCCCCCCCCULL)>>2)); \
  n=(u64)((((n)&0x0F0F0F0F0F0F0F0FULL)<<4)|(((n)&0xF0F0F0F0F0F0F0F0ULL)>>4)); \
  n=(u64)((((n)&0x00FF00FF00FF00FFULL)<<8)|(((n)&0xFF00FF00FF00FF00ULL)>>8)); \
  n=(u64)((((n)&0x0000FFFF0000FFFFULL)<<16)|(((n)&0xFFFF0000FFFF0000ULL)>>16)); \
  n=(u64)(((n)>>32)|((n)<<32))
#ifdef _64_
  #define REVERSE_ULONG REVERSE_U64
#elif defined(_32_)
  #define REVERSE_ULONG REVERSE_U32
#else
  #define REVERSE_ULONG REVERSE_U16
#endif
/*
The following macros are used in lieu of typedef so that they can be globally overridden with __attribute__ flags. Previously, ((packed)) was used for efficiency, but it caused problems (and is generally unhelpful anyway because data structures are manually engineered to have the most aligned variables first. ((packed)) also causes inefficient code to be generated on some platforms.
*/
#define TYPEDEF_END(typedef_struct_name) }typedef_struct_name;
#define TYPEDEF_START typedef struct {
/*
u128, the unsigned 128-bit integer type, is defined differently on 32-bit and 64-bit platforms. The main reason is that some compilers for the former have no such intrinsic support. Also, the compiler may do a better optimization job if u128 encapsulation is not used, allowing it to interleave operations with finer granularity.
*/
#ifdef _64_
  #define u128 __uint128_t
#else
  typedef struct{
    uint64_t a;
    uint64_t b;
  }u128;
#endif
#define U128_BIT_MAX 127U
#define U128_BITS 128U
#define U128_BITS_LOG2 7U
#define U128_BYTE_MAX 15U
#define U128_SIZE 16U
#define U128_SIZE_LOG2 4U
#define u16 uint16_t
#define U16_BIT_MAX 15U
#define U16_BITS 16U
#define U16_BITS_LOG2 4U
#define U16_BYTE_MAX 1U
#define U16_MAX 0xFFFFU
#define U16_SIZE 2U
#define U16_SIZE_LOG2 1U
#define U16_SPAN 0x10000U
#define U16_SPAN_HALF 0x8000U
#define U24_BIT_MAX 23U
#define U24_BITS 24U
#define U24_BYTE_MAX 2U
#define U24_MAX 0xFFFFFFU
#define U24_SIZE 3U
#define U24_SPAN 0x1000000U
#define U24_SPAN_HALF 0x800000U
#define u32 uint32_t
#define U32_BIT_MAX 31U
#define U32_BITS 32U
#define U32_BITS_LOG2 5U
#define U32_BYTE_MAX 3U
#define U32_MAX 0xFFFFFFFFU
#define U32_SIZE 4U
#define U32_SIZE_LOG2 2U
#define U32_SPAN 0x100000000ULL
#define U32_SPAN_HALF 0x80000000U
#define u64 uint64_t
#define U64_BIT_MAX 63U
#define U64_BITS 64U
#define U64_BITS_LOG2 6U
#define U64_BYTE_MAX 7U
#define U64_MAX 0xFFFFFFFFFFFFFFFFULL
#define U64_PRODUCT_HI(factor0, factor1, product_hi) \
  do{ \
    u32 _factor0_hi; \
    u32 _factor0_lo; \
    u32 _factor1_hi; \
    u32 _factor1_lo; \
    u64 _product_lo; \
    u64 _product0; \
    u64 _product1; \
    u64 _product2; \
    \
    _factor0_lo=(u32)(factor0); \
    _factor1_lo=(u32)(factor1); \
    _product0=(_factor0_lo)*(u64)(_factor1_lo); \
    _factor0_hi=(u32)((factor0)>>U32_BITS); \
    _product1=(_factor1_lo)*(u64)(_factor0_hi); \
    _factor1_hi=(u32)((factor1)>>U32_BITS); \
    _product2=(_factor0_lo)*(u64)(_factor1_hi); \
    product_hi=(_factor0_hi)*(u64)(_factor1_hi); \
    _product_lo=_product0; \
    _product_lo+=(_product1)<<U32_BITS; \
    product_hi+=(_product1)>>U32_BITS; \
    product_hi+=((_product_lo)<(_product0)); \
    _product0=_product_lo; \
    _product_lo+=(_product2)<<U32_BITS; \
    product_hi+=(_product2)>>U32_BITS; \
    product_hi+=((_product_lo)<(_product0)); \
  }while(0)
#define U64_SIZE 8U
#define U64_SIZE_LOG2 3U
#define U64_SPAN_HALF 0x8000000000000000ULL
#define u8 uint8_t
#define U8_BIT_MAX 7U
#define U8_BITS 8U
#define U8_BITS_LOG2 3U
#define U8_BYTE_MAX 0U
#define U8_MAX 0xFFU
#define U8_SIZE 1U
#define U8_SIZE_LOG2 0U
#define U8_SPAN 0x100U
#define U8_SPAN_HALF 0x80U
#define UINT_IS_NOT_POWER_OF_2(n) (!!((n)&((n)-1U)))
#define UINT_IS_NOT_POWER_OF_2_MINUS_1(n) (!((n)&((n)+1U)))
#define UINT_IS_POWER_OF_2(n) (!((n)&((n)-1U)))
#define UINT_IS_POWER_OF_2_MINUS_1(n) (!((n)&((n)+1U)))
#undef ULONG_MAX
#ifdef _64_
  #define UDOUBLE __uint128_t
  #define UDOUBLE_BIT_MAX 127U
  #define UDOUBLE_BITS 128U
  #define UDOUBLE_BITS_LOG2 7U
  #define UDOUBLE_BYTE_MAX 15U
  #define UDOUBLE_SIZE 16U
  #define UDOUBLE_SIZE_LOG2 4U
  #define UHALF uint32_t
  #define UHALF_BIT_MAX 31U
  #define UHALF_BITS 32U
  #define UHALF_BITS_LOG2 5U
  #define UHALF_BYTE_MAX 3U
  #define UHALF_MAX 0xFFFFFFFFU
  #define UHALF_SIZE 4U
  #define UHALF_SPAN_HALF 0x80000000U
  #define ULONG uint64_t
  #define ULONG_BIT_MAX 63U
  #define ULONG_BITS 64U
  #define ULONG_BITS_LOG2 6U
  #define ULONG_BYTE_MAX 7U
  #define ULONG_MAX 0xFFFFFFFFFFFFFFFFULL
  #define ULONG_SIZE 8U
  #define ULONG_SIZE_LOG2 3U
  #define ULONG_SPAN_HALF 0x8000000000000000ULL
#elif defined(_32_)
  #define UDOUBLE uint64_t
  #define UDOUBLE_BIT_MAX 63U
  #define UDOUBLE_BITS 64U
  #define UDOUBLE_BITS_LOG2 6U
  #define UDOUBLE_BYTE_MAX 7U
  #define UDOUBLE_SIZE 8U
  #define UDOUBLE_SIZE_LOG2 3U
  #define UHALF uint16_t
  #define UHALF_BIT_MAX 15U
  #define UHALF_BITS 16U
  #define UHALF_BITS_LOG2 4U
  #define UHALF_BYTE_MAX 1U
  #define UHALF_MAX 0xFFFFU
  #define UHALF_SIZE 2U
  #define UHALF_SPAN_HALF 0x8000U
  #define ULONG uint32_t
  #define ULONG_BIT_MAX 31U
  #define ULONG_BITS 32U
  #define ULONG_BITS_LOG2 5U
  #define ULONG_BYTE_MAX 3U
  #define ULONG_MAX 0xFFFFFFFFU
  #define ULONG_SIZE 4U
  #define ULONG_SIZE_LOG2 2U
  #define ULONG_SPAN_HALF 0x80000000U
#else
  #define UDOUBLE uint32_t
  #define UDOUBLE_BIT_MAX 31U
  #define UDOUBLE_BITS 32U
  #define UDOUBLE_BITS_LOG2 5U
  #define UDOUBLE_BYTE_MAX 3U
  #define UDOUBLE_SIZE 4U
  #define UDOUBLE_SIZE_LOG2 2U
  #define UHALF uint8_t
  #define UHALF_BIT_MAX 7U
  #define UHALF_BITS 8U
  #define UHALF_BITS_LOG2 3U
  #define UHALF_BYTE_MAX 0U
  #define UHALF_MAX 0xFFU
  #define UHALF_SIZE 1U
  #define UHALF_SPAN_HALF 0x80U
  #define ULONG uint16_t
  #define ULONG_BIT_MAX 15U
  #define ULONG_BITS 16U
  #define ULONG_BITS_LOG2 4U
  #define ULONG_BYTE_MAX 1U
  #define ULONG_MAX 0xFFFFU
  #define ULONG_SIZE 2U
  #define ULONG_SIZE_LOG2 1U
  #define ULONG_SPAN_HALF 0x8000U
#endif
