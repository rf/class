#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "third.h"

third_scheduler_t *
third_setup () {
  third_scheduler_t * scheduler = create(third_scheduler_t);
  scheduler->queue = create(third_node_t);

  ucontext_t * context = calloc(sizeof(ucontext_t), 1);
  scheduler->context = context;
}

third_t *
third_create (third_scheduler_t * scheduler, third_entry_t entry, void * arg) {
  third_t * new = create(third_t);
  new->entry = entry;
  new->state = S_RUNNING;

  // Add to run queue
  third_node_t * node = create(third_node_t);
  node->third = new;
  prepend(scheduler->queue, node);

  // allocate stack
  void * stack = calloc(STACK_SIZE, 1);

  // Create context
  ucontext_t * context = calloc(sizeof(ucontext_t), 1);
  getcontext(context);
  context->uc_stack.ss_sp = stack;
  context->uc_stack.ss_size = STACK_SIZE;
  context->uc_link = scheduler->context;

  makecontext(context, (void (*)()) entry, 2, new);
  new->context = context;

  new->scheduler = scheduler;
}

third_mutex_t * 
third_mutex_create () {
  third_mutex_t * new = create(third_mutex_t);
  new->state = M_UNLOCKED;
  return new;
}

void
third_yield (third_t * third) {
  swapcontext(third->context, third->scheduler->context);
}

void
third_mutex_lock (third_t * locker, third_mutex_t * mutex) {
  if (mutex->state != M_LOCKED) {
    mutex->state = M_LOCKED;
    mutex->locked_by = locker;
  } else {
    locker->current_mutex = mutex;
    locker->state = S_BLOCKED;
    third_yield(locker);

    // when we're rescheduled, we should be able to grab the mutex
    if (mutex->state == M_UNLOCKED) {
      mutex->state = M_LOCKED;
      mutex->locked_by = locker;
    }
  }
}

void
third_mutex_trylock (third_t * locker, third_mutex_t * mutex) {
  if (mutex->state == M_LOCKED) return;
  else {
    third_mutex_lock(locker, mutex);
  }
}

void
third_mutex_unlock (third_t * locker, third_mutex_t * mutex) {
  if (mutex->state != M_LOCKED) return;
  else {
    if (mutex->locked_by == locker) {
      mutex->state = M_UNLOCKED;
      mutex->locked_by = NULL;
    }
  }
}

void
third_begin (third_scheduler_t * scheduler) {
  scheduler->running = true;
  while (scheduler->running) {
    third_node_t * n;
    foreach (scheduler->queue, n) {
      if (n->third->state == S_BLOCKED) {
        if (n->third->current_mutex->state == M_UNLOCKED)
          n->third->state = S_RUNNING;
        else continue;
      }

      scheduler->current = n->third;
      handle_error(swapcontext(scheduler->context, n->third->context));
    }
  }
}

third_scheduler_t * s;

void
third_timer (int signum) {
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

  s = sched;

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = third_timer;

  sigaction(SIGVTALRM, &sa, NULL);

  struct itimerval tout_val;
  tout_val.it_interval.tv_sec = 0;
  tout_val.it_interval.tv_usec = 10;
  tout_val.it_value.tv_sec = 0;
  tout_val.it_value.tv_usec = 10;
  setitimer(ITIMER_VIRTUAL, &tout_val, NULL);

  third_begin(sched);
}
