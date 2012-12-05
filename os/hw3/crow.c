#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "crow.h"

crow_t * foobar_crow;
crow_t * barbaz_crow;
crow_scheduler_t * global_scheduler_lol;

crow_scheduler_t *
crow_setup () {
  crow_scheduler_t * scheduler = create(crow_scheduler_t);
  scheduler->queue = create(crow_node_t);

  ucontext_t * context = calloc(sizeof(ucontext_t), 1);
  scheduler->context = context;
}

crow_t *
crow_create (crow_scheduler_t * scheduler, crow_entry_t entry) {
  crow_t * new = create(crow_t);
  new->entry = entry;

  // Add to run queue
  crow_node_t * node = create(crow_node_t);
  node->crow = new;
  prepend(scheduler->queue, node);

  // allocate stack
  void * stack = calloc(STACK_SIZE, 1);

  // Create context
  ucontext_t * context = calloc(sizeof(ucontext_t), 1);
  getcontext(context);
  context->uc_stack.ss_sp = stack;
  context->uc_stack.ss_size = STACK_SIZE;
  context->uc_link = scheduler->context;
  
  // We need to pass the scheduler pointer in, so we have to split it since
  // makecontext expects int args
  makecontext(context, entry, 0);
  new->context = context;
}

void
crow_yield (crow_t * crow, crow_scheduler_t * scheduler) {
  swapcontext(crow->context, scheduler->context);
}

void
crow_begin (crow_scheduler_t * scheduler, void * arg) {
  scheduler->running = true;
  while (scheduler->running) {
    crow_node_t * n;
    foreach (scheduler->queue, n) {
      // reset the context so it starts the coro from the beginning each time
      makecontext(n->crow->context, n->crow->entry, 0);
      swapcontext(scheduler->context, n->crow->context);
    }
  }
}

void
foobar (crow_scheduler_t * scheduler, void * arg) {
  printf("in foobar\n");
  sleep(1);
  crow_yield(foobar_crow, global_scheduler_lol);
}

void
barbaz (crow_scheduler_t * scheduler, void * arg) {
  printf("in barbaz\n");
  sleep(1);
  crow_yield(barbaz_crow, global_scheduler_lol);
}

int
main (int argc, char ** argv) {
  crow_scheduler_t * sched = crow_setup();
  foobar_crow = crow_create(sched, foobar);
  barbaz_crow = crow_create(sched, barbaz);

  global_scheduler_lol = sched;

  crow_begin(sched, NULL);
}
