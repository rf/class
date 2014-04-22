// Russ Frank - CS416 homework 3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "third.h"

// runs thirds, then marks their state as done when they exit
void
third_runner (third_t * run, void * arg) {
  run->entry(run, arg);
  run->state = DONE;
  run->scheduler->done += 1;
}

void
third_exit (third_t * me) {
  me->state = DONE;
  me->scheduler->done += 1;
  third_yield(me);
}

third_scheduler_t * global_scheduler;

// setup a scheduler
third_scheduler_t *
third_setup () {
  third_scheduler_t * scheduler = create(third_scheduler_t);
  ucontext_t * context = calloc(sizeof(ucontext_t), 1);
  scheduler->context = context;
}

// Create new third
third_t *
third_create (third_scheduler_t * scheduler, third_entry_t entry, void * arg) {
  third_t * new = create(third_t);
  new->entry = entry;
  new->state = RUNNING;

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

  makecontext(context, (void (*)()) third_runner, 2, new, arg);
  new->context = context;

  new->scheduler = scheduler;
  scheduler->total += 1;

  return new;
}

third_mutex_t * 
third_mutex_create () {
  third_mutex_t * new = create(third_mutex_t);
  new->state = UNLOCKED;
  return new;
}

void
third_yield (third_t * third) {
  swapcontext(third->context, third->scheduler->context);
}

void
third_join (third_t * me, third_t * him) {
  me->state = JOIN;
  me->blocked.join = him;
  third_yield(me);
}

int
third_mutex_lock (third_t * locker, third_mutex_t * mutex) {
  if (mutex->state != LOCKED) {
    locker->scheduler->disabled = true;
    mutex->state = LOCKED;
    mutex->locked_by = locker;
    locker->scheduler->disabled = false;
    return 0;
  } else {
    locker->blocked.mutex = mutex;
    locker->state = BLOCKED;
    third_yield(locker);

    locker->scheduler->disabled = true;
    // when we're rescheduled, we should be able to grab the mutex
    if (mutex->state == UNLOCKED) {
      mutex->state = LOCKED;
      mutex->locked_by = locker;
    }
    locker->scheduler->disabled = false;
    return 0;
  }
}

int
third_mutex_trylock (third_t * locker, third_mutex_t * mutex) {
  locker->scheduler->disabled = true;
  if (mutex->state == LOCKED) {
    locker->scheduler->disabled = false;
    return 1;
  } else {
    third_mutex_lock(locker, mutex);
    locker->scheduler->disabled = false;
    return 0;
  }
}

void
third_mutex_unlock (third_t * locker, third_mutex_t * mutex) {
  locker->scheduler->disabled = true;
  if (mutex->state != LOCKED) return;
  else {
    if (mutex->locked_by == locker) {
      mutex->state = UNLOCKED;
      mutex->locked_by = NULL;
    }
  }
  locker->scheduler->disabled = false;
}

void
third_timer (int signum) {
  if (global_scheduler->disabled) return;

  uint64_t sched_rbp = global_scheduler->context->uc_mcontext.gregs[REG_RBP];
  uint64_t rbp;
  asm("movq %%rbp, %0" : "=r" (rbp) : /* no inputs */ );

  // Check to see if the stack frame ptr is within 10mb of the scheduler's
  // stack frame ptr. This is a hacky but okay solution for determining
  // whether the interrupt occurred in the scheduler code.
  if (abs(rbp - sched_rbp) < 10485760) {
    return;
  }

  // Swap into the scheduler
  swapcontext(global_scheduler->current->context, global_scheduler->context);
}

void
third_begin (third_scheduler_t * scheduler, bool preemption) {
  // setup the timer
  if (preemption) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = third_timer;

    sigaction(SIGVTALRM, &sa, NULL);

    struct itimerval tout_val;
    tout_val.it_interval.tv_sec = 0;
    tout_val.it_interval.tv_usec = 10000;
    tout_val.it_value.tv_sec = 0;
    tout_val.it_value.tv_usec = 10000;
    setitimer(ITIMER_VIRTUAL, &tout_val, NULL);

    global_scheduler = scheduler;
  }

  while (scheduler->done < scheduler->total) {
    third_node_t * n;
    foreach (scheduler->queue, n) {
      if (n->third->state == BLOCKED) {
        if (n->third->blocked.mutex->state == UNLOCKED)
          n->third->state = RUNNING;
        else continue;
      }

      else if (n->third->state == BOX_SEND) {
        // box send block. check to see if there are available slots, if so,
        // reschedule the third.
        third_box_t * box = n->third->blocked.box;
        if (box->slot_state[box->next_empty] == READ)
          n->third->state = RUNNING; // unblock it
        else continue;
      }

      else if (n->third->state == BOX_RECV) {
        // box recv block. check to see if there is available data, if so,
        // reschedule the third.
        third_box_t * box = n->third->blocked.box;
        if (box->slot_state[box->current_unread] == UNREAD)
          n->third->state = RUNNING; // unblock it
        else continue;
      }

      else if (n->third->state == JOIN) {
        if (n->third->blocked.join->state == DONE) {
          n->third->state = RUNNING;
        }
        else continue;
      }

      else if (n->third->state == DONE) continue;

      scheduler->current = n->third;
      handle_error(swapcontext(scheduler->context, n->third->context));
    }
  }
}

third_box_t *
third_box_create (int num_slots, int slot_size) {
  third_box_t * new = create(third_box_t);
  new->mutex = third_mutex_create();
  new->num_slots = num_slots;
  new->slot_size = slot_size;

  new->slots = calloc(num_slots, slot_size);
  new->current_unread = 0;
  new->next_empty = 0;

  new->slot_state = calloc(sizeof(enum { READ, UNREAD }), num_slots);
  int i;
  for (i = 0; i < num_slots; i++) new->slot_state[i] = READ;
  return new;
}

void
third_box_send (third_box_t * box, third_t * from, void * data) {
  third_mutex_lock(from, box->mutex);

  if (box->slot_state[box->next_empty] == UNREAD) {
    third_mutex_unlock(from, box->mutex);
    from->blocked.box = box;
    from->state = BOX_SEND;
    third_yield(from);
    third_mutex_lock(from, box->mutex);
  } 

  memcpy(box->slots + (box->next_empty * box->slot_size), data, box->slot_size);
  box->slot_state[box->next_empty] = UNREAD;
  box->next_empty += 1;
  if (box->next_empty >= box->num_slots) box->next_empty = 0;

  third_mutex_unlock(from, box->mutex);
}

void *
third_box_recv (third_box_t * box, third_t * me) {
  third_mutex_lock(me, box->mutex);

  if (box->slot_state[box->current_unread] == READ) {
    third_mutex_unlock(me, box->mutex);
    me->blocked.box = box;
    me->state = BOX_RECV;
    third_yield(me);
    third_mutex_lock(me, box->mutex);
  }

  void * data = box->slots + (box->current_unread * box->slot_size);
  box->slot_state[box->current_unread] = READ;
  box->current_unread += 1;
  if (box->current_unread >= box->num_slots) box->current_unread = 0;

  third_mutex_unlock(me, box->mutex);
  return data;
}
