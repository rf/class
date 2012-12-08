#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "crow.h"

crow_scheduler_t *
crow_setup () {
  crow_scheduler_t * scheduler = create(crow_scheduler_t);
  scheduler->queue = create(crow_node_t);

  ucontext_t * context = calloc(sizeof(ucontext_t), 1);
  scheduler->context = context;
}

crow_t *
crow_create (crow_scheduler_t * scheduler, crow_entry_t entry, void * arg) {
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

  makecontext(context, entry, 2, new, scheduler);
  new->context = context;
}

void
crow_yield (crow_t * crow, crow_scheduler_t * scheduler) {
  swapcontext(crow->context, scheduler->context);
}

void
crow_begin (crow_scheduler_t * scheduler) {
  scheduler->running = true;
  while (scheduler->running) {
    crow_node_t * n;
    foreach (scheduler->queue, n) {
      scheduler->current = n->crow;
      handle_error(swapcontext(scheduler->context, n->crow->context));
    }
  }
}

crow_scheduler_t * s;

void
crow_timer (int signum) {
  uint64_t sched_rbp = s->context->uc_mcontext.gregs[REG_RBP];
  uint64_t rbp;
  asm("movq %%rbp, %0" : "=r" (rbp) : /* no inputs */ : /* no clobbers */);

  // Check to see if the stack frame ptr is within 10mb of the scheduler's
  // stack frame ptr. This is a hacky but okay solution for determining
  // whether the interrupt occurred in the scheduler code.
  if (abs(rbp - sched_rbp) < 10485760) {
    return;
  }

  // Swap into the scheduler
  swapcontext(s->current->context, s->context);
}

void
foobar (crow_t * me, crow_scheduler_t * scheduler, void * arg) {
  int i = 0;
  while (1) {
    i++;
    if (i % 10000000 == 0)
      printf("in foobar: %d\n", i);
  }
}

void
barbaz (crow_t * me, crow_scheduler_t * scheduler, void * arg) {
  int i = 0;
  while (1) {
    i++;
    if (i % 10000000 == 0)
      printf("in barbaz: %d\n", i);
  }
}

int
main (int argc, char ** argv) {
  crow_scheduler_t * sched = crow_setup();
  crow_t * foobar_crow = crow_create(sched, foobar, NULL);
  crow_t * barbaz_crow = crow_create(sched, barbaz, NULL);

  s = sched;

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = crow_timer;

  sigaction(SIGVTALRM, &sa, NULL);

  struct itimerval tout_val;
  tout_val.it_interval.tv_sec = 0;
  tout_val.it_interval.tv_usec = 10;
  tout_val.it_value.tv_sec = 0;
  tout_val.it_value.tv_usec = 10;
  setitimer(ITIMER_VIRTUAL, &tout_val, NULL);

  crow_begin(sched);
}
