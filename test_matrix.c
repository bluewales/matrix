#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "matrix.h"


int main(int argc, char **argv) {
    printf("HI!!\n");
    srand(time(0));

    unsigned N = 10;

    if(argc == 2) {
        N = atoi(argv[1]);
    }

    bool done = false;
    unsigned try_count = 0;
    unsigned print_count = 0;

    int64_t best_range = INT32_MAX;

    matrix_t matrix = create_random_balanced_matrix(N);
    matrix_t inverse_matrix = create_random_balanced_matrix(N);
    append_identity(&inverse_matrix);

    unsigned start_time = time(0);

    unsigned print_limit = 1000;

    unsigned reducible = 0;
    unsigned irreducible = 0;

    while(!done) {
        try_count += 1;
        
        // create_random_balanced_matrix_no_alloc(&matrix, N);
        construct_iterative(&matrix);
        
        create_inverse_no_alloc(&matrix, &inverse_matrix);

        if(inverse_matrix.denominator) {
            reducible += 1;
        } else {
            irreducible += 1;
        }

        int64_t new_range = measure_rank(&inverse_matrix);
        if(inverse_matrix.denominator != 0 && ( new_range < best_range)) {

            best_range = (best_range + new_range)/2;

            unsigned current_time = time(0);
            unsigned elapsed_time = current_time - start_time;

            char dirname[128];
            sprintf(dirname, "non_hadamard/%dx%d", N, N);

            struct stat st = {0};
            if (stat(dirname, &st) == -1) {
                mkdir(dirname, 0766);
            }

            char filename[128];
            sprintf(filename, "non_hadamard/%dx%d/%09ld-%09d.txt", N, N, new_range, time(0));
            printf("%d:%d: writing file %s\n", try_count, elapsed_time, filename);
            write_plus_minus_matrix(filename, &matrix, &inverse_matrix);

            if(new_range == 0) {
                break;
            }
        }
    }

    destroy_matrix(&matrix);
    destroy_matrix(&inverse_matrix);
}