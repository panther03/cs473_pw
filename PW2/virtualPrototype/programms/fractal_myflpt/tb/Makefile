.SUFFIXES:
tb: ../build-release/tb
../build-release/tb: tb.c  ../src/myflpt.c ../include/myflpt.h
	gcc -o $@ -I../include/ tb.c ../src/myflpt.c