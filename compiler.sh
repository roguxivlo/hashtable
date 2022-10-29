#!/bin/sh

g++ -Wall -Wextra -O2 -std=c++17 -c hash.cc -o hash.o
gcc -Wall -Wextra -O2 -std=c17 -c hash_test1.c -o hash_test1.o
g++ -Wall -Wextra -O2 -std=c++17 -c hash_test2.cc -o hash_test2.o
g++ hash_test1.o hash.o -o hash_test1
g++ hash_test2.o hash.o -o hash_test21
g++ hash.o hash_test2.o -o hash_test22