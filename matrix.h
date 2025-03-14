#ifndef MATRIX_H__
#define MATRIX_H__

#include <stdbool.h>
#include <stdio.h>

#define abs64(n) (((n)>=0)?(n):(n*-1))

typedef struct {
    unsigned nrows;
    unsigned ncols;
    unsigned alloced_rows;
    unsigned alloced_cols;
    int64_t ** matrix;
    int64_t denominator;
} matrix_t;

void copy_matrix_no_alloc(matrix_t * src_matrix_p, matrix_t * dst_matrix_p);
void append_identity_no_alloc(matrix_t * matrix_p);
void print_matrix(FILE * fd, matrix_t * matrix_p);

void server_print_row(unsigned N, unsigned row);
void server_print_state(unsigned N);
void server_print_reduction(unsigned N);
bool server_get_matrix_no_alloc(matrix_t * matrix_p, unsigned N);
void server_init(unsigned N);
int64_t gcd(int64_t a, int64_t b);
int64_t lcm(int64_t a, int64_t b);
void matrix_factor_reduce(matrix_t * matrix_p);

void matrix_divide_row(matrix_t * matrix_p, unsigned row_tow_divide, int64_t divisor);
void matrix_add_row(matrix_t * matrix_p, unsigned dst_row_ix, unsigned src_row_ix, int64_t m, int64_t d);
void matrix_swap_rows(matrix_t * matrix_p, unsigned dst_row, unsigned src_row);
bool matrix_row_reduce(matrix_t * matrix_p, matrix_t * scratch_p);
void append_identity_no_alloc(matrix_t * matrix_p);
void append_identity(matrix_t * matrix_p);
void copy_matrix_no_alloc(matrix_t * src_matrix_p, matrix_t * dst_matrix_p);
matrix_t copy_matrix(matrix_t * src_matrix_p);
void destroy_matrix(matrix_t * matrix_p);
bool create_inverse_no_alloc(matrix_t * matrix_p, matrix_t * inverse_matrix_p);
matrix_t create_inverse(matrix_t * matrix_p);
void create_random_balanced_matrix_no_alloc(matrix_t * matrix_p, unsigned N);
matrix_t create_random_balanced_matrix(unsigned N);
void create_matrix_from_string_no_alloc(char * matrix_string, matrix_t * matrix_p);
matrix_t create_matrix_from_string(char * matrix_string);
void write_plus_minus_matrix(char * filename, matrix_t * matrix_p, matrix_t * inverse_p);
int64_t measure_rank(matrix_t * matrix_p);
unsigned read_status(void);
void write_status(unsigned my_id, unsigned my_status);
void construct_iterative(matrix_t * matrix_p);
matrix_t create_matrix_from_plus_minus_file(char * filepath);

#endif