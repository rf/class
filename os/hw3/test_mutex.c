#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "third.h"

third_mutex_t * mutex;

void
foobar (third_t * me, void * arg) {
  int i = 0;
  while (1) {
    i++;
    if (i % 100000000 == 0)
      printf("foobar %d\n", i);
    if (i == 100000000) {
      printf("foobar is locking mutex\n");
      third_mutex_lock(me, mutex);
      printf("foobar has mutex\n");
    }

    if (i == 1600000000) {
      printf("foobar is unlocking mutex\n");
      third_mutex_unlock(me, mutex);
    }
  }
}

void
barbaz (third_t * me, void * arg) {
  int i = 0;
  while (1) {
    i++;
    if (i % 100000000 == 0)
      printf("barbaz %d\n", i);
    if (i == 500000000) {
      printf("barbaz attempting to lock mutex\n");
      fflush(stdout);
      third_mutex_lock(me, mutex);
      printf("barbaz has mutex\n");
    }
  }
}

int
main (int argc, char ** argv) {
  third_scheduler_t * sched = third_setup();
  third_t * foobar_third = third_create(sched, foobar, NULL);
  third_t * barbaz_third = third_create(sched, barbaz, NULL);

  mutex = third_mutex_create();

  third_begin(sched, true);
}
