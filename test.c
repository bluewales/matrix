#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "matrix.h"

#include "invert_4x4_matrix.h"
#include "invert_6x6_matrix.h"
#include "invert_8x8_matrix.h"
#include "invert_10x10_matrix.h"
#include "invert_12x12_matrix.h"
#include "invert_14x14_matrix.h"
#include "invert_16x16_matrix.h"
#include "invert_20x20_matrix.h"
#include "invert_24x24_matrix.h"
#include "invert_28x28_matrix.h"
#include "invert_32x32_matrix.h"


bool (*inverters[]) (matrix_t * x, double ** y) = {
    NULL, // 0
    NULL, // 1
    NULL, // 2
    NULL, // 3
    invert_4x4_matrix,
    NULL, // 5
    invert_6x6_matrix,
    NULL, // 7
    invert_8x8_matrix,
    NULL, // 9
    invert_10x10_matrix,
    NULL, // 11
    invert_12x12_matrix,
    NULL, // 13
    invert_14x14_matrix,
    NULL, // 15
    invert_16x16_matrix,
    NULL, // 17
    NULL, // 18
    NULL, // 19
    invert_20x20_matrix,
    NULL, // 21
    NULL, // 22
    NULL, // 23
    invert_24x24_matrix,
    NULL, // 25
    NULL, // 26
    NULL, // 27
    invert_28x28_matrix,
    NULL, // 29
    NULL, // 30
    NULL, // 31
    invert_32x32_matrix
    
};

/* This is just a sample code, modify it to meet your need */
int main(int argc, char **argv)
{
    uint64_t N = 6;

    matrix_t matrix = create_random_balanced_matrix(N);
    matrix_t inverse_matrix = create_random_balanced_matrix(N);
    append_identity(&inverse_matrix);


    double *other_inverse_matrix[N];
    for(unsigned ix = 0; ix < N; ix += 1) {
        other_inverse_matrix[ix] = malloc(sizeof(double) * N);
    }

    bool success = false;
    uint64_t tries = 0;

    unsigned new_invert_time = 0;
    unsigned old_invert_time = 0;

    unsigned match_count = 0;
    unsigned missmatch_count = 0;
    unsigned should_have_failed_count = 0;
    unsigned should_have_succeeded_count = 0;
    
    unsigned last_print = 0;

    do {
        srand(tries);
        // create_random_balanced_matrix_no_alloc(&matrix, N);

        int pos_count = 0;
        int neg_count = 0;
        int target_unbalance = 2;

        int target_pos_count = (N + target_unbalance) / 2;
        int target_neg_count = (N - target_unbalance) / 2;
        
        matrix.denominator = 1;
        for(unsigned ix = 0; ix < N; ix += 1) {
            for(unsigned jx = 0; jx < N; jx += 1) {
                bool coin_flip = rand()%2;

                if(N == 4) {
                    unsigned shift = ix + jx*N;
                    coin_flip = tries & (1 << shift);
                }

                if((coin_flip && (pos_count < target_pos_count)) || neg_count >= target_neg_count) {
                    pos_count += 1;
                    matrix.matrix[ix][jx] = 1;
                } else {
                    neg_count += 1;
                    matrix.matrix[ix][jx] = -1;
                }

                if(coin_flip) {
                    matrix.matrix[ix][jx] = 1;
                } else {
                    matrix.matrix[ix][jx] = -1;
                }
            }
        }

        tries += 1;

        unsigned start_time = time(0);
        success = inverters[N](&matrix, other_inverse_matrix);
        new_invert_time += time(0) - start_time;

        start_time = time(0);
        create_inverse_no_alloc(&matrix, &inverse_matrix);
        old_invert_time += time(0) - start_time;

        if(success && inverse_matrix.denominator==0) {
            // printf("should have failed\n");
            should_have_failed_count += 1;
        } else if(!success && inverse_matrix.denominator!=0) {
            // printf("should have succeeded\n");
            should_have_succeeded_count += 1;
        }
        
        if(success && inverse_matrix.denominator != 0) {
            bool missmatch = false;
            for(unsigned ix = 0; ix < N; ix += 1) {
                for(unsigned jx = 0; jx < N; jx += 1) {
                    double template_n = inverse_matrix.matrix[ix][jx];
                    double template_d = inverse_matrix.denominator;

                    double other_v = other_inverse_matrix[ix][jx];

                    if(template_n / template_d != other_v) {
                        // printf("%d %d  %f %f\n", ix, jx, template_n / template_d, other_v);
                        missmatch = true;
                    }
                }
            }
            if(missmatch) {
                missmatch_count += 1;
            } else {
                match_count += 1;
            }
        }



        if(last_print != time(0)) {
            last_print = time(0);
            printf("N=%ld new: %d, old: %d\n", N, new_invert_time, old_invert_time);
            
            printf("%ld tries, %d should have failed, %d should have succeeded, %d:%d matched\n", tries, should_have_failed_count, should_have_succeeded_count, match_count, missmatch_count);
            // print_matrix(stdout, &matrix);
            // print_matrix(stdout, &inverse_matrix);
        }

        if(old_invert_time > 99 && new_invert_time > 99) {
            printf("DONE\n");
            printf("new: %d, old: %d\n", new_invert_time, old_invert_time);
            
            printf("%ld tries, %d should have failed, %d should have succeeded, %d:%d matched\n", tries, should_have_failed_count, should_have_succeeded_count, match_count, missmatch_count);
            break;
        }

    } while(true);
}
