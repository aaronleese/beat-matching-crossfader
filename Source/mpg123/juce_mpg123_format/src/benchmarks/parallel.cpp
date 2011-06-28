#include <time.h>
#include <stdio.h>
#include <stdlib.h>

typedef long long int int64;

static const int64 SIZE = 100000000L;
static const int64 BYTE_SIZE = sizeof(float) * SIZE;

int64 test(float* interleaved, float** parallel, bool isParallel) {
  float* out = (float*) malloc(BYTE_SIZE);
  int64 time = -int64(clock());

  if (isParallel) {
    for (int64 i = 0; i < SIZE; ++i)
      out[i] = (parallel[0][i] + parallel[1][i]) / 2;
  } else {
    for (int64 i = 0; i < SIZE; ++i)
      out[i] = (interleaved[2 * i] + interleaved[2 * i + 1]) / 2;
  }

  time += clock();
  free(out);

  return time;
}

/*
int main() {
  float* interleaved = (float*) malloc(2 * BYTE_SIZE);
  float* parallel[2] = {interleaved, interleaved + SIZE};

  printf("parallel: %lld\n", test(interleaved, parallel, false));
  printf("interleaved: %lld\n", test(interleaved, parallel, true));

  free(interleaved);
}
*/