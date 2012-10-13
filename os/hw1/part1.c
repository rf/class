#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define TESTSIZE 64999
#define INCREMENT 1024
#define ITERATIONS 5000
#define STRIDES 7

long int timer_start_usec;
long int timer_start_sec;

inline void
start() {
  struct timeval start;
  gettimeofday(&start, NULL);
  timer_start_usec = start.tv_usec;
  timer_start_sec = start.tv_sec;
}

inline long int
stop() {
  struct timeval end;
  gettimeofday(&end, NULL);
  long int time = (end.tv_usec - timer_start_usec) / 1000;
  time += (end.tv_sec - timer_start_sec) * 1000;
  return time;
}

int
main(int argc, const char **argv) {
  int i, temp;

  int size;
  // We will try accessing `size` bytes from the array a few thousand times,
  // timing how long it takes for all of the accesses. Eventually, this should
  // become much slower; at this point we've hit the size of the cache.

  for (size = INCREMENT; size <= TESTSIZE; size += INCREMENT) {
    int j;
    long int time;
    char * space = (char *) malloc(size);
    // loop over whole array 10 times
    for (i = 0; i < size * 10; i++) temp = space[i % size];
    printf("Timing %d iterations of accessing %d bytes of array striding across %d: ", ITERATIONS, size, STRIDES);
    start();
    for (j = 0; j < ITERATIONS; j++) {
      int k;

      // We actually loop to TESTSIZE and then take that modulus the size
      // we're testing. This way we'll only access the first 64 bytes but we'll
      // access the same number of bytes total as when we go through 8192 bytes.
      for (k = 0; k < TESTSIZE; k++) temp = space[(k * (STRIDES)) % size];
    }
    time = stop();
    printf("Took %ld ms.\n", time);
    free(space);
  }

  return 0;
}
