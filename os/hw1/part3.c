#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define ITERATIONS 1000

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
  long int time = (end.tv_usec - timer_start_usec);
  time += (end.tv_sec - timer_start_sec) * 1000 * 1000;
  return time;
}

int
main (int argc, char ** argv) {
  if (argc < 2) {
    printf("not enough arguments\n");
    exit(1);
  }
  int count = atoi(argv[1]) * 1000 * 1000;

  int down[2];    // parent down to child
  int up[2];      // child up to parent
  pipe(down);
  pipe(up);

  start();

  int i;
  if (fork() == 0) {
    // child
    char * mem = (char *) malloc(count);
    for (i = 0; i < ITERATIONS; i++) {
      read(down[0], mem, count);
      write(up[1], mem, count);
    }
    exit(0);
  }

  else {
    char * mem = (char *) malloc(count);
    // parent
    for (i = 0; i < ITERATIONS; i++) {
      write(down[1], mem, count);
      read(up[0], mem, count);
    }
  }

  printf("Wrote %d bytes across pipe %d times, took %ld\n", count, ITERATIONS,stop());
}

