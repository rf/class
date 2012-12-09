#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "third.h"

third_mutex_t * mutex;
third_t * barbaz_third;

void
foobar (third_t * me, void * arg) {
  int i = 0;
  while (1) {
    i++;
    if (i % 100000000 == 0)
      printf("foobar %d\n", i);
    if (i == 100000000) {
      printf("foobar is joining\n");
      third_join(me, barbaz_third);
      printf("foobar join done\n");
    }
  }
}

void
barbaz (third_t * me, void * arg) {
  int i = 0;
  while (i < 1000000000) {
    i++;
    if (i % 100000000 == 0)
      printf("barbaz %d\n", i);
  }
  printf("barbaz exiting\n");
}

int
main (int argc, char ** argv) {
  third_scheduler_t * sched = third_setup();
  third_t * foobar_third = third_create(sched, foobar, NULL);
  barbaz_third = third_create(sched, barbaz, NULL);

  mutex = third_mutex_create();

  third_begin(sched, true);
}
