#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include "matrix.h"

void print_matrix(FILE * fd, matrix_t * matrix_p) {

    bool all_ones = true;

    unsigned max_size = 0;
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            char buffer[100];
            sprintf(buffer, "%ld", matrix_p->matrix[row][col]);
            if(strlen(buffer) > max_size) max_size = strlen(buffer);

            if(abs64(matrix_p->matrix[row][col]) != 1) all_ones = false;
        }
    }
    bool show_balance = all_ones;

    fprintf(fd, "size=%dx%d, denominator=%ld\n", matrix_p->nrows, matrix_p->ncols, matrix_p->denominator);

    char template_string[100];
    sprintf(template_string, "%% %dld ", max_size);

    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        unsigned pos_count = 0;
        unsigned neg_count = 0;
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            fprintf(fd, template_string, matrix_p->matrix[row][col]);
            if(matrix_p->matrix[row][col] > 0) {
                pos_count += 1;
            }
            if(matrix_p->matrix[row][col] < 0) {
                neg_count += 1;
            }
        }
        if(show_balance) {
            fprintf(fd, "(%d/%d)", pos_count, neg_count);
        }
        fprintf(fd, "\n");
    }
}


static int numberOfSetBits(uint32_t i)
{
     i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     i = (i + (i >> 4)) & 0x0F0F0F0F;
     i *= 0x01010101;
     return  i >> 24;
}

unsigned server_state[100];
unsigned server_reduction[100];

static unsigned server_next_row(unsigned N, unsigned row) {
    do {
        row += 1;
        int difference = (N/2+1) - numberOfSetBits(row);
        if(difference > 0) {
            // we need to add more bits
            row |= (1<<difference) - 1;
        }
        if(difference < 0) {
            // we need to remove some bits
            unsigned mask = 1;
            for(unsigned ix = 0; row && mask == 0; ix+=1) {
                mask = (1<<ix) - 1;
            }
            row |= mask;
        }
    } while(numberOfSetBits(row) != (N/2+1));

    return row;
}

static bool server_make_row_good(unsigned N, unsigned ix) {
    unsigned mask = (1 << N) - 1;
    while(server_state[ix] < mask) {
        server_reduction[ix] = server_state[ix];
        for(unsigned pivot_ix = N-1; pivot_ix > ix; pivot_ix -= 1) {
            if(server_reduction[ix] & (1<<pivot_ix)) {
                server_reduction[ix] ^= server_reduction[pivot_ix];
            }
        }
        if(server_reduction[ix] & (1<<ix)) {
            return true;
        } else {
            server_state[ix] = server_next_row(N, server_state[ix]);
        }
    }
    return false;
}

static void server_fill_out_matrix_no_alloc(unsigned N, matrix_t * matrix_p) {

    assert(matrix_p->alloced_rows >= N);
    assert(matrix_p->alloced_cols >= N);

    matrix_p->ncols = N;
    matrix_p->nrows = N;
    matrix_p->denominator = 1;

    for(unsigned row = 0; row < N; row += 1) {
        for(unsigned col = 0; col < N; col += 1) {
            if(server_state[row] & (1<<col)) {
                matrix_p->matrix[row][col] = 1;
            } else {
                matrix_p->matrix[row][col] = -1;
            }
        }
    }
}

void server_print_row(unsigned N, unsigned row) {
    for(unsigned col = 0; col < N; col += 1) {
        if(row & (1<<col)) {
            printf("+");
        } else {
            printf("-");
        }
    }
    printf(" %08X\n", row);
}

void server_print_state(unsigned N) {
    for(unsigned row_ix = 0; row_ix < N; row_ix += 1) {
        server_print_row(N, server_state[row_ix]);
    }
    printf("\n");
}

void server_print_reduction(unsigned N) {
    for(unsigned row_ix = 0; row_ix < N; row_ix += 1) {
        server_print_row(N, server_reduction[row_ix]);
    }
    printf("\n");
}


bool server_get_matrix_no_alloc(matrix_t * matrix_p, unsigned N) {
    server_fill_out_matrix_no_alloc(N, matrix_p);

    int ix = 0;
    while(ix >= 0 && ix < N) {
        server_state[ix] = server_next_row(N, server_state[ix]);
        if(server_make_row_good(N, ix)) {
            ix -= 1;
        } else {
            server_state[ix] = 0;
            ix += 1;
        }
    }

    if(ix < 0) return true;
    if(ix >= N) return false;

    printf("THERE'S BEEN A FATAL ERROR\n");

    return false;
}

void server_init(unsigned N) {
    memset(server_state, 0, sizeof(server_state));

    for(int ix = N-1; ix >= 0; ix -= 1) {
        server_make_row_good(N, ix);
    }

    int ix = 0;
    int highest_ix = 0;
    while(ix >= 0 && ix < N) {
        if(server_make_row_good(N, ix)) {
            ix -= 1;
        } else {
            server_state[ix] = server_next_row(N, 0);
            ix += 1;
            if(ix < N) {
                server_state[ix] = server_next_row(N, server_state[ix]);
            }
            if(ix > highest_ix) {
                printf("highest %d\n", ix);
                highest_ix = ix;
            }
        }
    }
}



// Recursive function to return gcd of a and b
int64_t gcd(int64_t a, int64_t b)
{
    if (b == 0) return a;
    return gcd(b, a % b);
}

// Function to return LCM of two numbers
int64_t lcm(int64_t a, int64_t b) {
    return (a / gcd(a, b)) * b;
}

void matrix_factor_reduce(matrix_t * matrix_p) {
    int64_t reduction_factor = matrix_p->denominator;
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            reduction_factor = gcd(matrix_p->matrix[row][col], reduction_factor);
        }

        reduction_factor = abs64(reduction_factor);
        if(reduction_factor == 1) {
            return;
        }
    }
    
    matrix_p->denominator /= reduction_factor;
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            matrix_p->matrix[row][col] /= reduction_factor;
        }
    }
}

void matrix_multiply_row(matrix_t * matrix_p, unsigned row_to_multiply, int64_t multiple) {
    for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
        matrix_p->matrix[row_to_multiply][col] *= multiple;
    }
}

void matrix_divide_row(matrix_t * matrix_p, unsigned row_to_divide, int64_t divisor) {

    int64_t row_d = divisor;
    for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
        row_d = gcd(row_d, matrix_p->matrix[row_to_divide][col]);
    }
    if(divisor > 0) {
        row_d = abs64(row_d);
    } else {
        row_d = -1 * abs64(row_d);
    }

    int64_t multiple = abs64(divisor / row_d);

    matrix_p->denominator *= multiple;
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            if(row == row_to_divide) {
                matrix_p->matrix[row][col] /= row_d;
            } else {
                matrix_p->matrix[row][col] *= multiple;
            }
        }
    }
}

void matrix_normalize_pivot(matrix_t * matrix_p, unsigned pivot_ix) {

    int64_t pivot_value = matrix_p->matrix[pivot_ix][pivot_ix];

    if(pivot_value == matrix_p->denominator) return;

    int64_t multiple = abs64(lcm(pivot_value, matrix_p->denominator));
    int64_t row_m = multiple / pivot_value;
    int64_t matrix_m =  multiple / matrix_p->denominator;

    matrix_p->denominator *= matrix_m;
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            if(row == pivot_ix) {
                matrix_p->matrix[row][col] *= row_m;
            } else {
                matrix_p->matrix[row][col] *= matrix_m;
            }
        }
    }
}

// adds src_row * m/d to dst_row
void matrix_add_row(matrix_t * matrix_p, unsigned dst_row_ix, unsigned src_row_ix, int64_t m, int64_t d) {
    // reduce m/d
    int64_t reduction_factor = gcd(m, d);
    m /= reduction_factor;
    d /= reduction_factor;

    reduction_factor = d;
    for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
        reduction_factor = gcd(matrix_p->matrix[src_row_ix][col], reduction_factor);
    }
    int64_t matrix_m = d / reduction_factor;

    matrix_p->denominator *= matrix_m;
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            matrix_p->matrix[row][col] *= matrix_m;
        }
    }

    for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
        matrix_p->matrix[dst_row_ix][col] += (matrix_p->matrix[src_row_ix][col] / d) * m;
    }

    matrix_factor_reduce(matrix_p);
}

void matrix_swap_rows(matrix_t * matrix_p, unsigned dst_row, unsigned src_row) {
    for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
        matrix_p->matrix[dst_row][col] ^= matrix_p->matrix[src_row][col];
        matrix_p->matrix[src_row][col] ^= matrix_p->matrix[dst_row][col];
        matrix_p->matrix[dst_row][col] ^= matrix_p->matrix[src_row][col];
    }
}

bool matrix_row_reduce(matrix_t * matrix_p, matrix_t * scratch_p) {

    copy_matrix_no_alloc(matrix_p, scratch_p);
    append_identity_no_alloc(scratch_p);

    for(unsigned pivot_ix = 0; pivot_ix < scratch_p->nrows; pivot_ix += 1) {
        
        // if we get a zero pivot, find a row to add to it
        if(scratch_p->matrix[pivot_ix][pivot_ix] == 0) {
            bool found = false;
            for(unsigned row_ix = pivot_ix+1; row_ix < scratch_p->nrows; row_ix += 1) {
                if(scratch_p->matrix[row_ix][pivot_ix] != 0) {
                    found = true;
                    matrix_add_row(scratch_p, pivot_ix, row_ix, 1, 1);
                    break;
                }
            }

            // if we couldn't find a row to add into the pivot, then we failed to reduce this matrix
            if(!found) return false;
        }

        // normalize the pivot
        matrix_normalize_pivot(scratch_p, pivot_ix);

        // zero out everything else in that column so we're left with a diagonal matrix
        for(unsigned row_ix = 0; row_ix < scratch_p->nrows; row_ix += 1) {
            if(row_ix == pivot_ix) continue;
            matrix_add_row(scratch_p, 
                row_ix, 
                pivot_ix, 
                scratch_p->matrix[row_ix][pivot_ix] * -1, 
                scratch_p->matrix[pivot_ix][pivot_ix]);
        }
    }

    return true;
}

void append_identity_no_alloc(matrix_t * matrix_p) {
    unsigned new_ncols = matrix_p->ncols + matrix_p->nrows;
    assert(matrix_p->alloced_cols >= new_ncols);

    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            if(col == row) {
                matrix_p->matrix[row][matrix_p->ncols + col] = 1;
            } else {
                matrix_p->matrix[row][matrix_p->ncols + col] = 0;
            }
        }
    }

    matrix_p->ncols = new_ncols;
}

void append_identity(matrix_t * matrix_p) {
    unsigned new_ncols = matrix_p->ncols + matrix_p->nrows;
    if(matrix_p->alloced_cols < new_ncols) {
        for(unsigned row = 0; row < matrix_p->alloced_rows; row += 1) {
            matrix_p->matrix[row] = realloc(matrix_p->matrix[row], new_ncols * sizeof(int64_t));
        }
        matrix_p->alloced_cols = new_ncols;
    }
    append_identity_no_alloc(matrix_p);
}

void copy_matrix_no_alloc(matrix_t * src_matrix_p, matrix_t * dst_matrix_p) {
    
    assert(dst_matrix_p->alloced_rows >= src_matrix_p->nrows);
    assert(dst_matrix_p->alloced_cols >= src_matrix_p->ncols);

    dst_matrix_p->nrows = src_matrix_p->nrows;
    dst_matrix_p->ncols = src_matrix_p->ncols;
    dst_matrix_p->denominator = src_matrix_p->denominator;

    for(unsigned row = 0; row < src_matrix_p->nrows; row += 1) {
        memcpy(dst_matrix_p->matrix[row], src_matrix_p->matrix[row], src_matrix_p->nrows * sizeof(int64_t));
    }
}

matrix_t copy_matrix(matrix_t * src_matrix_p) {
    matrix_t dst_matrix;

    dst_matrix.matrix = malloc(src_matrix_p->nrows * sizeof(int64_t *));
    dst_matrix.alloced_rows = src_matrix_p->nrows;
    dst_matrix.alloced_cols = src_matrix_p->ncols;

    for(unsigned row = 0; row < src_matrix_p->nrows; row += 1) {
        dst_matrix.matrix[row] = malloc(src_matrix_p->ncols * sizeof(int64_t));
    }
    copy_matrix_no_alloc(src_matrix_p, &dst_matrix);
    return dst_matrix;
}

void destroy_matrix(matrix_t * matrix_p) {
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        free(matrix_p->matrix[row]);
    }
    free(matrix_p->matrix);
    matrix_p->matrix = 0;
    matrix_p->ncols = 0;
    matrix_p->nrows = 0;
    matrix_p->denominator = 0;
    matrix_p->alloced_cols = 0;
    matrix_p->alloced_rows = 0;
}


bool create_inverse_no_alloc(matrix_t * matrix_p, matrix_t * inverse_matrix_p) {

    assert(inverse_matrix_p->alloced_rows >= matrix_p->nrows);
    assert(inverse_matrix_p->alloced_cols >= matrix_p->ncols*2);

    if(matrix_p->ncols != matrix_p->nrows) {
        printf("ERROR Cannot invert non square matrix");
        inverse_matrix_p->denominator = 0;
        return false;
    }
    
    bool reduce_worked = matrix_row_reduce(matrix_p, inverse_matrix_p);

    if(!reduce_worked) {
        // printf("ERROR reduce failed when creating inverse");
        inverse_matrix_p->denominator = 0;
        return false;
    }

    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        memcpy(&inverse_matrix_p->matrix[row][0], &inverse_matrix_p->matrix[row][matrix_p->nrows], matrix_p->nrows * sizeof(int64_t));
    }
    inverse_matrix_p->nrows = matrix_p->nrows;
    inverse_matrix_p->ncols = matrix_p->ncols;
    return inverse_matrix_p;
}


matrix_t create_inverse(matrix_t * matrix_p) {
    matrix_t inverse_matrix = {0};
    if(matrix_p->ncols != matrix_p->nrows) {
        printf("ERROR Cannot invert non square matrix");
        return inverse_matrix;
    }
    inverse_matrix = copy_matrix(matrix_p);
    append_identity(&inverse_matrix);
    bool reduce_worked = matrix_row_reduce(matrix_p, &inverse_matrix);

    if(!reduce_worked) {
        // printf("ERROR reduce failed when creating inverse");
        destroy_matrix(&inverse_matrix);
        return inverse_matrix;
    }

    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        memcpy(&inverse_matrix.matrix[row][0], &inverse_matrix.matrix[row][matrix_p->nrows], matrix_p->nrows * sizeof(int64_t));
    }
    inverse_matrix.nrows = matrix_p->nrows;
    inverse_matrix.ncols = matrix_p->ncols;
    return inverse_matrix;
}

void create_random_balanced_matrix_no_alloc(matrix_t * matrix_p, unsigned N) {

    assert(matrix_p->alloced_rows >= N);
    assert(matrix_p->alloced_cols >= N);

    matrix_p->ncols = N;
    matrix_p->nrows = N;
    matrix_p->denominator = 1;

    for(unsigned row = 0; row < N; row += 1) {
        int pos_count = 0;
        int neg_count = 0;
        int target_unbalance = 2;

        int target_pos_count = (N + target_unbalance) / 2;
        int target_neg_count = (N - target_unbalance) / 2;
        
        for(unsigned col = 0; col < N; col += 1) {
            bool coin_flip = (random() & 1);
            if((coin_flip && (pos_count < target_pos_count)) || neg_count >= target_neg_count) {
                pos_count += 1;
                matrix_p->matrix[row][col] = 1;
            } else {
                neg_count += 1;
                matrix_p->matrix[row][col] = -1;
            }

        }

    }

}

matrix_t create_random_balanced_matrix(unsigned N) {
    matrix_t matrix = {0};
    matrix.matrix = malloc(N * sizeof(int64_t *));
    for(unsigned row = 0; row < N; row += 1) {
        matrix.matrix[row] = malloc(N * sizeof(int64_t));
    }
    matrix.alloced_cols = N;
    matrix.alloced_rows = N;

    create_random_balanced_matrix_no_alloc(&matrix, N);

    return matrix;
}

void create_matrix_from_string_no_alloc(char * matrix_string, matrix_t * matrix_p) {

    matrix_p->denominator = 1;
    for(unsigned ix = 0; matrix_string[ix]; ix += 1) {
        if(matrix_string[ix] == '\n') {
            matrix_p->nrows = ix;
            matrix_p->ncols = ix;
            break;
        }
    }

    assert(matrix_p->alloced_rows >= matrix_p->nrows);
    assert(matrix_p->alloced_cols >= matrix_p->ncols);

    unsigned col = 0;
    unsigned row = 0;
    for(unsigned ix = 0; matrix_string[ix]; ix += 1) {

        switch (matrix_string[ix]) {
            case '-':
                matrix_p->matrix[row][col] = -1;
                col += 1;
                break;
            case '+':
                matrix_p->matrix[row][col] = 1;
                col += 1;
                break;
            case '\n':
                row += 1;
                col = 0;
                break;
            default:
                printf("ERROR matrix parse failed\n");
                break;
        }
    }
}

matrix_t create_matrix_from_string(char * matrix_string) {
    matrix_t matrix = {0};
    matrix.denominator = 1;
    for(unsigned ix = 0; matrix_string[ix]; ix += 1) {
        if(matrix_string[ix] == '\n') {
            matrix.nrows = ix;
            matrix.ncols = ix;
            break;
        }
    }
    matrix.matrix = malloc(matrix.nrows * sizeof(int64_t *));
    for(unsigned rows = 0; rows < matrix.nrows; rows += 1) {
        matrix.matrix[rows] = malloc(matrix.ncols * sizeof(int64_t));
    }

    matrix.alloced_rows = matrix.nrows;
    matrix.alloced_cols = matrix.ncols;

    create_matrix_from_string_no_alloc(matrix_string, &matrix);
    return matrix;
}

void write_plus_minus_matrix(char * filename, matrix_t * matrix_p, matrix_t * inverse_p) {
    FILE *fd = fopen(filename, "w");

    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            char c = '0';
            if(matrix_p->matrix[row][col] > 0) {
                c = '+';
            } else if(matrix_p->matrix[row][col] < 0) {
                c = '-';
            }
            fputc(c, fd);
        }
        fputc('\n', fd);
    }
    
    fprintf(fd, "\nMATRIX:\n");
    print_matrix(fd, matrix_p);

    fprintf(fd, "\nINVERSE:\n");
    print_matrix(fd, inverse_p);
    fclose(fd);
}

matrix_t create_matrix_from_plus_minus_file(char * filepath) {

    FILE *fd = fopen(filepath, "r");

    char file_contents[2024];
    size_t bytes = fread(file_contents, 1, sizeof(file_contents), fd);
    fclose(fd);

    for(unsigned ix = 1; ix < bytes; ix += 1) {
        if(file_contents[ix] == '\n' && file_contents[ix-1] == '\n') {
            file_contents[ix] = 0;
        }
    }

    matrix_t matrix = create_matrix_from_string(file_contents);
    return matrix;
}



int64_t measure_rank(matrix_t * matrix_p) {

    int64_t sum = 0;
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            int64_t value = abs64(matrix_p->matrix[row][col]);
            sum += value;
        }
    }

    int64_t average = sum / (matrix_p->nrows*matrix_p->ncols);

    int64_t target_level = average;
    if(target_level < 1) target_level = 1;

    int64_t variation = 0;
    for(unsigned row = 0; row < matrix_p->nrows; row += 1) {
        for(unsigned col = 0; col < matrix_p->ncols; col += 1) {
            int64_t value = abs64(matrix_p->matrix[row][col]);
            int64_t difference = value - target_level;
            variation += (difference * difference);

            if(variation < 0) return INT64_MAX;
        }
    }

    int64_t shifted_variation = variation << 16;
    if((shifted_variation >> 16) != variation)  return INT64_MAX;

    variation <<= 16;
    variation = sqrt(variation);
    variation /= (target_level);

    return variation;
}

unsigned read_status(void) {
    FILE *fd = fopen("thread_status.txt", "r");
    if(fd == NULL) return 0;
    char status_string[1024];
    fgets(status_string, 1024, fd);
    fclose(fd);

    unsigned lowest_4[] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX};

    unsigned sum = 0;
    unsigned count = 0;
    for(unsigned ix = 0; status_string[ix]; ix += 1) {
        if(status_string[ix] == ' ' && status_string[ix + 1]) {
            unsigned n = atoi(&status_string[ix]);
            if(n == 0) continue;

            if(n < lowest_4[0]) {
                lowest_4[3] = lowest_4[2];
                lowest_4[2] = lowest_4[1];
                lowest_4[1] = lowest_4[0];
                lowest_4[0] = n;
            } else if(n < lowest_4[1]) {
                lowest_4[3] = lowest_4[2];
                lowest_4[2] = lowest_4[1];
                lowest_4[1] = n;
            } else if(n < lowest_4[2]) {
                lowest_4[3] = lowest_4[2];
                lowest_4[2] = n;
            } else if(n < lowest_4[3]) {
                lowest_4[3] = n;
            }

            if(n > 0) {
                sum += n;
                count += 1;
            }
        }
    }
    unsigned average = 0;
    if(count > 0) {
        average = sum/count;
    }

    return average;
}

void write_status(unsigned my_id, unsigned my_status) {
    FILE *read_fd = fopen("thread_status.txt", "r");
    char status_string[1024] = "";
    if(read_fd) {
        fgets(status_string, 1024, read_fd);
        fclose(read_fd);
    }

    unsigned data[100];
    unsigned data_ix = 0;

    for(unsigned ix = 0; status_string[ix]; ix += 1) {
        if(status_string[ix] == ' ' && status_string[ix + 1]) {
            data[data_ix] = atoi(&status_string[ix]);
            data_ix += 1;
        }
    }

    while(data_ix <= my_id) {
        data[data_ix] = 0;
        data_ix += 1;
    }

    data[my_id] = my_status;
    
    FILE *write_fd = fopen("thread_status.txt", "w");
    for(unsigned ix = 0; ix < data_ix; ix += 1) {
        fprintf(write_fd, " %u", data[ix]);
    }
    fclose(write_fd);
}

void construct_iterative(matrix_t * matrix_p) {

    unsigned N = matrix_p->nrows;

    matrix_t scratch_matrix = create_random_balanced_matrix(N);
    matrix_t inverse_matrix = create_random_balanced_matrix(N);
    append_identity(&inverse_matrix);

    matrix_t * scratch_p = &scratch_matrix;
    matrix_t * inverse_p = &inverse_matrix;

    int64_t best_rank = INT64_MAX;
    while(best_rank == INT64_MAX) {
        create_random_balanced_matrix_no_alloc(matrix_p, N);

        unsigned toggle_ix = 0;
        unsigned iteration_count = 0;
        while(best_rank > 0) {
            unsigned toggle_row_ix = toggle_ix % matrix_p->nrows;
            unsigned toggle_col1_ix = (toggle_ix / matrix_p->nrows) % matrix_p->ncols;
            unsigned toggle_col2_ix = (toggle_ix / (matrix_p->nrows * matrix_p->ncols)) % matrix_p->ncols;

            toggle_ix += 1;
            iteration_count += 1;
            if(iteration_count > matrix_p->nrows * matrix_p->ncols * matrix_p->ncols) {
                break;
            }

            if(toggle_col1_ix >= toggle_col2_ix) {
                continue;
            }
            if(scratch_p->matrix[toggle_row_ix][toggle_col1_ix] == scratch_p->matrix[toggle_row_ix][toggle_col2_ix]) {
                continue;
            }

            copy_matrix_no_alloc(matrix_p, scratch_p);

            scratch_p->matrix[toggle_row_ix][toggle_col1_ix] *= -1;
            scratch_p->matrix[toggle_row_ix][toggle_col2_ix] *= -1;

            copy_matrix_no_alloc(scratch_p, inverse_p);
            append_identity_no_alloc(inverse_p);

            create_inverse_no_alloc(scratch_p, inverse_p);

            int64_t rank = INT64_MAX;
            if(inverse_p->denominator) {
                rank = measure_rank(inverse_p);
            }

            if(rank < best_rank) {
                best_rank = rank;
                copy_matrix_no_alloc(scratch_p, matrix_p);
                iteration_count = 0;
            }
        }
    }

    destroy_matrix(&inverse_matrix);
    destroy_matrix(&scratch_matrix);
}
