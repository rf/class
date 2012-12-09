#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "third.h"

typedef struct message {
  int type;
  int payload;
} message_t;

third_box_t * box;

void
foobar (third_t * me, void * arg) {
  int i = 0;
  while (1) {
    i++;
    if (i % 100000000 == 0)
      printf("foobar %d\n", i);
    if (i % 500000000 == 0) {
      printf("foobar sending message\n");
      message_t m;
      m.type = 0;
      m.payload = i;
      third_box_send(box, me, &m);
      printf("foobar sent message\n");
    }

    if (i % 50000000 == 0) {
      message_t m;
      m.type = 1;
      m.payload = i;
      third_box_send(box, me, &m);

      m.type = 2;
      m.payload = i;
      third_box_send(box, me, &m);

      m.type = 3;
      m.payload = i;
      third_box_send(box, me, &m);
    }
  }
}

void
barbaz (third_t * me, void * arg) {
  int i = 0;
  while (1) {
    i++;
    if (i % 100000000 == 0) {
      message_t * m = (message_t *) third_box_recv(box, me);
      printf("barbaz got msg type: %d payload: %d\n", m->type, m->payload);
    }

    if (i == 500000000) {

    }
  }
}

int
main (int argc, char ** argv) {
  third_scheduler_t * sched = third_setup();
  third_t * foobar_third = third_create(sched, foobar, NULL);
  third_t * barbaz_third = third_create(sched, barbaz, NULL);

  box = third_box_create(100, sizeof(message_t));

  third_begin(sched, true);
}
