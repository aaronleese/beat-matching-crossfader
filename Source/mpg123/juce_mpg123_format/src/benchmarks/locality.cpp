#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef long long int int64;

const int64 PRIME = 51234577L;

char* buffer = (char*) malloc(PRIME);

// permute() permutes the numbers between 0 and PRIME - 1 inclusive.
// This is based on modular arithmetic:
// http://en.wikipedia.org/wiki/Modular_arithmetic
//
// Suppose p is a prime, and m is any non-zero integer multiplier.  As i
// iterates through all numbers in [0, p), (m * i) mod p also iterates through
// all of [0, p), but in a completely different permutation.
//
// An easy way to see this is with smaller numbers.  Suppose p = 7 -
// if  i iterates through [0, 1, 2, 3, 4, 5, 6],
// 2 * i iterates through [0, 2, 4, 6, 1, 3, 5]
// 3 * i iterates through [0, 3, 6, 2, 5, 1, 4]
// 4 * i iterates through [0, 4, 1, 5, 2, 6, 3]
inline int64 permute(int64 x) {
  static const int64 MULT = 20000003L;

  return (x * MULT) % PRIME;
}

int64 test(bool linear) {
  int64 time = -int64(clock());

  for (int64 i = 0; i < PRIME; ++i) {
    int64 permuted = permute(i);
    buffer[linear ? i : permuted] = char(permuted);
  }

  return time + clock();;
}

/*
int main() {
  int64 permuted = test(false);
  int64 linear = test(true);

  printf("\npermuted:  %lldms\nlinear:    %lldms\nratio:     %.2f\n\n",
         permuted / 1000, linear / 1000, permuted / double(linear));
}
*/