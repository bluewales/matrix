#include "math_expressions.h"
#include "matrix.h"

const ratio_t one = {
    1, 1
};
const ratio_t zero = {
    0, 1
};
const ratio_t neg_one = {
    -1, 1
};

void inline ratio_simplify(ratio_t * r) {
    if (r->n == 0) {
        r->d = 1;
        return;
    }
    r_type d = gcd(r->n, r->d);
    d = abs64(d);
    if (d != 1) {
        r->n = r->n / d;
        r->d = r->d / d;
    }
}

void ratio_multiply(ratio_t *result_p, const ratio_t *left_p, const ratio_t *right_p) {
    result_p->n = left_p->n * right_p->n; 
    result_p->d = left_p->d * right_p->d; 
    // ratio_simplify(result_p);

    // printf("%ld/%ld * %ld/%ld = %ld/%ld\n", left_p->n, left_p->d, right_p->n, right_p->d, result_p->n, result_p->d);
}

void ratio_add(ratio_t *result_p, const ratio_t *left_p, const ratio_t *right_p) {
    r_type d = gcd(left_p->d, right_p->d); 
    r_type left_m = right_p->d/d; 
    r_type right_m = left_p->d/d; 
    result_p->n = left_p->n * left_m + right_p->n * right_m; 
    result_p->d = left_p->d * left_m; 
    ratio_simplify(result_p); 

    // printf("%ld/%ld + %ld/%ld = %ld/%ld\n", left_p->n, left_p->d, right_p->n, right_p->d, result_p->n, result_p->d);
}


void ratio_subtract(ratio_t *result_p, const ratio_t *left_p, const ratio_t *right_p) {
    r_type d = gcd(left_p->d, right_p->d); 
    r_type left_m = right_p->d/d; 
    r_type right_m = left_p->d/d; 
    result_p->n = left_p->n * left_m - right_p->n * right_m; 
    result_p->d = left_p->d * left_m;
    ratio_simplify(result_p); 

    // printf("%ld/%ld - %ld/%ld = %ld/%ld\n", left_p->n, left_p->d, right_p->n, right_p->d, result_p->n, result_p->d);
}

void ratio_if_add(bool condition, ratio_t *result_p, const ratio_t *left_p, const ratio_t *right_p) {
    if(right_p->n == 0) {
        result_p->n = left_p->n;
        result_p->d = left_p->d;
        return;
    }
    if(condition) {
        ratio_add(result_p, left_p, right_p);
    } else {
        result_p->n = left_p->n;
        result_p->d = left_p->d;

        // printf("%ld/%ld = %ld/%ld\n", left_p->n, left_p->d, result_p->n, result_p->d);
    }
}

void ratio_divide(ratio_t *result_p, const ratio_t *left_p, const ratio_t *right_p) {
    result_p->n = left_p->n * right_p->d; 
    result_p->d = left_p->d * right_p->n; 
    // ratio_simplify(result_p);

    // printf("%ld/%ld / %ld/%ld = %ld/%ld\n", left_p->n, left_p->d, right_p->n, right_p->d, result_p->n, result_p->d);
}
