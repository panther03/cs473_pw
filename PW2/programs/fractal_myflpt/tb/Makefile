.SUFFIXES:
tb: ../build-release/tb
../build-release/tb: tb.c  ../src/myflpt.c ../src/fractal_myflpt.c ../include/myflpt.h
	gcc -o $@ -I../include/ -I. tb.c ../src/myflpt.c ../src/fractal_myflpt.c