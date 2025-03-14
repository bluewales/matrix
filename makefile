CFLAGS= -Wall -Werror
CC=gcc $(CFLAGS)

ODIR=obj
LDIR =../lib

LIBS=-lm


IFLAGS = -Igenerated -I.


_DEPS = matrix.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = matrix.o  math_expressions.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_NS = 4 6 8 10 12 14 16 20 24 28 32
NxNs = $(join $(_NS:=x),$(_NS))
N_HEADERS = $(patsubst %,generated/invert_%_matrix.h,$(NxNs))
N_SOURCE =  $(patsubst %,generated/invert_%_matrix.c,$(NxNs))
N_OBJ =  $(patsubst %,$(ODIR)/invert_%_matrix.o,$(NxNs))

default: test

generated/invert_4x4_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 4

generated/invert_6x6_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 6

generated/invert_8x8_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 8

generated/invert_10x10_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 10

generated/invert_12x12_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 12

generated/invert_14x14_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 14

generated/invert_16x16_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 16

generated/invert_20x20_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 20

generated/invert_24x24_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 24

generated/invert_28x28_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 28

generated/invert_32x32_matrix.c: Algebra.py Expression.py
	python3 Algebra.py 32

$(ODIR)/%.o: generated/%.c math_expressions.h
	$(CC) -c -o $@ $< $(IFLAGS)

$(ODIR)/%.o: %.c matrix.h math_expressions.h
	$(CC) -c -o $@ $< 

test_matrix: test_matrix.c $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

test: test.c $(OBJ) $(N_OBJ)
	$(CC) -o $@ $^ $(IFLAGS) $(LIBS)

clean:
	rm -rf generated/*
	rm obj/*