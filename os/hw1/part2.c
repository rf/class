#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
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
  int event = eventfd(0, 0);
  if (argc < 2) {
    printf("not enough arguments\n");
    exit(1);
  }
  int count = atoi(argv[1]) * 1000 * 1000;
  char * memsize = (char *) malloc(count);
  int i;
  long int total_time = 0;

  for (i = 0; i < ITERATIONS; i++) {
    start();
    if (fork() == 0) {
      // child
      long int status = 1;
      write(event, &status, 8);
      exit(0);
    } else {
      // parent
      long int status;
      read(event, &status, 8);
      // child is done
      long int t = stop();
      total_time += t;
    }
  }

  printf("Total time for %d iterations allocating %d bytes of memory: %d\n", ITERATIONS, count, total_time);
}
