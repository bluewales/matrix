import random
import time
import os
import sys

from Expression import Division
from Expression import Multiplication
from Expression import Subtraction
from Expression import Variable
from Expression import ConditionalAddition
from Expression import Condition

N = 16
if len(sys.argv) > 1:
    N = int(sys.argv[1])

def n_matrix_divide_row(matrix, row_ix_tow_divide, divisor):
    row_ix = row_ix_tow_divide
    for col_ix in range(len(matrix[row_ix])):
        matrix[row_ix][col_ix] = matrix[row_ix][col_ix] / divisor

def n_matrix_subtract_row(matrix, dst_row_ix, src_row_ix, scalar):
    for col_ix in range(len(matrix[row_ix])):
        matrix[dst_row_ix][col_ix] = matrix[dst_row_ix][col_ix] - (matrix[src_row_ix][col_ix] * scalar)

def n_matrix_conditional_add_row(matrix, dst_row_ix, src_row_ix, condition):
    if condition:
        for col_ix in range(len(matrix[row_ix])):
            matrix[dst_row_ix][col_ix] = matrix[dst_row_ix][col_ix] + matrix[src_row_ix][col_ix]

def matrix_divide_row(matrix, row_ix_tow_divide, divisor):
    row_ix = row_ix_tow_divide
    for col_ix in range(len(matrix[row_ix])):
        matrix[row_ix][col_ix] = Division(matrix[row_ix][col_ix], divisor).simplify()

def matrix_subtract_row(matrix, dst_row_ix, src_row_ix, scalar):
    for col_ix in range(len(matrix[row_ix])):
        matrix[dst_row_ix][col_ix] = Subtraction(matrix[dst_row_ix][col_ix], Multiplication(matrix[src_row_ix][col_ix], scalar)).simplify()

def matrix_conditional_add_row(matrix, dst_row_ix, src_row_ix, condition):
    for col_ix in range(len(matrix[row_ix])):
        matrix[dst_row_ix][col_ix] = ConditionalAddition(matrix[dst_row_ix][col_ix], matrix[src_row_ix][col_ix], condition).simplify()

def cell_name(row_ix, col_ix):
    key_str ="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    return key_str[row_ix] + key_str[col_ix]

def make_c_header(N):

    c = "#ifndef MATRIX_%d_%d_H__\n" % (N, N)
    c += "#define MATRIX_%d_%d_H__\n\n" % (N, N)

    c += "#include <stdbool.h>\n"
    c += "#include <stdint.h>\n\n"
    c += "#include \"matrix.h\"\n\n"

    c += "bool invert_%dx%d_matrix(matrix_t * matrix_p, double ** invert_d);\n\n"  % (N, N)

    c += "#endif\n"

    return c

def make_c_function(expression_matrix, N):
    c = "#include <stdbool.h>\n"
    c += "#include <stdint.h>\n\n"
    c += "#include \"math_expressions.h\"\n"
    c += "#include \"matrix.h\"\n\n"
    c += "bool invert_%dx%d_matrix(matrix_t * matrix_p, double ** invert_d) {\n"  % (N, N)

    for row_ix in range(N):
        for col_ix in range(N):
            # c += "    double x" + cell_name(row_ix, col_ix) + " = matrix_p->matrix[" + str(row_ix) + "][" + str(col_ix) + "];\n"

            c += "    SET_FROM_INT(x" + cell_name(row_ix, col_ix) + ",matrix_p->matrix[" + str(row_ix) + "][" + str(col_ix) + "]);\n"
    c += "\n"

    lookup_cache = {}

    for row_ix in range(N):
        for col_ix in range(N):
            c += expression_matrix[row_ix][col_ix+N].get_c(lookup_cache)

            c += "\n"


    for row_ix in range(N):
        for col_ix in range(N):
            name = expression_matrix[row_ix][col_ix+N].get_c_name()
            c += "    invert_d[" + str(row_ix) + "][" + str(col_ix) + "] = TO_DOUBLE(" + name + ");\n"
    c += "\n    return true;\n"
    c += "}\n"

    return c


matrix = []
for row_ix in range(N):
    matrix.append([])
    for col_ix in range(N):
        matrix[row_ix].append(Variable(cell_name(row_ix, col_ix)))
    for col_ix in range(N):
        if row_ix == col_ix:
            matrix[row_ix].append(Variable(random.random(), 1))
        else:
            matrix[row_ix].append(Variable(random.random(), 0))

start_time = time.time()

problem_matrix = []
problem_matrix.append([ -1,  1,  1, -1,  1, -1, -1, -1, -1,  1])
problem_matrix.append([ -1, -1,  1,  1,  1, -1,  1,  1,  1,  1])
problem_matrix.append([ -1, -1, -1, -1, -1,  1,  1, -1,  1,  1])
problem_matrix.append([ -1,  1,  1,  1, -1, -1,  1, -1,  1, -1])
problem_matrix.append([  1,  1, -1, -1, -1, -1, -1,  1,  1, -1])
problem_matrix.append([ -1,  1, -1,  1,  1, -1, -1, -1, -1,  1])
problem_matrix.append([ -1, -1,  1,  1,  1,  1,  1, -1,  1, -1])
problem_matrix.append([ -1,  1,  1,  1,  1,  1,  1, -1, -1, -1])
problem_matrix.append([ -1,  1,  1, -1, -1, -1, -1, -1,  1, -1])
problem_matrix.append([ -1,  1, -1,  1, -1,  1, -1,  1,  1,  1])



for pivot_ix in range(N):

    # make sure the pivot is not zero
    if pivot_ix > 0:
        for row_ix in range(pivot_ix+1, N):
            condition = Condition(matrix[pivot_ix][pivot_ix], matrix[row_ix][pivot_ix])
            matrix_conditional_add_row(matrix, pivot_ix, row_ix, condition)

    
    # normalize the pivot
    matrix_divide_row(matrix, pivot_ix, matrix[pivot_ix][pivot_ix])

    # zero out everything else in that column so we're left with a diagonal matrix
    for row_ix in range(N):
        if row_ix == pivot_ix:
            continue
        scalar = Division(matrix[row_ix][pivot_ix], matrix[pivot_ix][pivot_ix])
        matrix_subtract_row(matrix, row_ix, pivot_ix, scalar)


# print("reduced " + str(time.time() - start_time))


dirname = "generated" 
filename = "invert_%dx%d_matrix.c" % (N, N)

full_filename = os.path.join(dirname, filename)
# print(full_filename)
os.makedirs(dirname, exist_ok=True)

header_full_filename = full_filename.replace(".c", ".h")
# print(header_full_filename)

with open(full_filename, "w") as fd:
    fd.write(make_c_function(matrix, N))

with open(header_full_filename, "w") as fd:
    fd.write(make_c_header(N))