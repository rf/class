#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>

#define MEMSIZE 0

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
main (int argc, char ** argv) {
  int event = eventfd(0, 0);
  char * memsize = (char *) malloc(MEMSIZE);

  start();
  if (fork() == 0) {
    // child
    long int status;
    read(event, &status, 8);
    // parent is done
    printf("took %ld\n", stop());
  } else {
    // parent
    long int status = 1;
    write(event, &status, 8);
  }
}
