#ifndef MATH_EXPRESSIONS_H__
#define MATH_EXPRESSIONS_H__

#include <stdbool.h>
#include <stdint.h>

#define DOUBLE_MATH

#ifndef DOUBLE_MATH
#ifndef RATIONAL_MATH

#define RATIONAL_MATH

#endif
#endif

#define r_type __int128_t
// #define r_type int64_t

typedef struct {
    r_type n;
    r_type d;
} ratio_t;

void ratio_simplify(ratio_t* r);
void ratio_multiply(ratio_t *result_p, const ratio_t *left_p, const ratio_t *right_p);
void ratio_add(ratio_t *result, const ratio_t *left, const ratio_t *right);
void ratio_subtract(ratio_t *result_p, const ratio_t *left_p, const ratio_t *right_p);
void ratio_divide(ratio_t *result_p, const ratio_t *left_p, const ratio_t *right_p);
void ratio_if_add(bool condition, ratio_t *result, const ratio_t *left, const ratio_t *right);


#ifdef DOUBLE_MATH

#define ONE 1
#define ZERO 0

#define SET_FROM_INT(name, value) double name = value
#define NEGATE(negative, normal) double negative = -normal;

#define MUL(result, left, right) double result = left * right
#define ADD(result, left, right) double result = left + right
#define SUB(result, left, right) double result = left - right
#define NOTZERO(right) if(right == 0) return false
#define DIV(result, left, right) double result = left / right
#define TEST_ZERO(result, left, right) bool result = (left == 0) && (right != 0)
#define IFADD(cond, result, left, right) double result = (cond) ? (left + right) : (left)
#define TO_INT(value) (value)
#define TO_DOUBLE(value) (value)

#endif

#ifdef RATIONAL_MATH

extern const ratio_t one;
extern const ratio_t zero;
extern const ratio_t neg_one;



#define ONE one
#define ZERO zero
#define NEG_ONE neg_one

#define SIMPLIFY(result) if(result.n == 0) { result.d = 1; } else {  \
    int64_t d = gcd(result.n, result.d); d = abs64(d);               \
    if(d != 1) { result.n = result.n / d; result.d = result.d / d; } \
}

#define SET_FROM_INT(name, value) ratio_t name; name.n = value; name.d = 1; 
#define NEGATE(negative, normal) ratio_t negative; negative.n = -normal.n; negative.d = normal.d; 

#define ADD(result, left, right) ratio_t result; ratio_add(&result, &left, &right);
#define SUB(result, left, right) ratio_t result; ratio_subtract(&result, &left, &right);
#define MUL(result, left, right) ratio_t result; ratio_multiply(&result, &left, &right);
#define DIV(result, left, right) ratio_t result; ratio_divide(&result, &left, &right);


#define NOTZERO(right) if(right.n == 0) return false;

#define TEST_ZERO(result, left, right) bool result = (left.n == 0) && (right.n != 0); 
#define IFADD(cond, result, left, right) ratio_t result; ratio_if_add(cond, &result, &left, &right);

#define TO_INT(value) (value.n / value.d)
#define TO_DOUBLE(value) (((double)value.n) / ((double)value.d))

#endif

#endif