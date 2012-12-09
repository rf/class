#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "third.h"

third_scheduler_t * global_scheduler;

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
third_timer (int signum) {
  uint64_t sched_rbp = global_scheduler->context->uc_mcontext.gregs[REG_RBP];
  uint64_t rbp;
  asm("movq %%rbp, %0" : "=r" (rbp) : /* no inputs */ : /* no clobbers */);

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
    tout_val.it_interval.tv_usec = 1000;
    tout_val.it_value.tv_sec = 0;
    tout_val.it_value.tv_usec = 1000;
    setitimer(ITIMER_VIRTUAL, &tout_val, NULL);

    global_scheduler = scheduler;
  }

  scheduler->running = true;
  while (scheduler->running) {
    third_node_t * n;
    foreach (scheduler->queue, n) {
      if (n->third->state == S_BLOCKED) {
        if (n->third->current_mutex->state == M_UNLOCKED)
          n->third->state = S_RUNNING;
        else continue;
      }

      else if (n->third->state == S_BOX_SEND) {
        // box send block. check to see if there are available slots, if so,
        // reschedule the third.
        third_box_t * box = n->third->current_box;
        if (box->slot_state[box->next_empty] == S_READ)
          n->third->state = S_RUNNING; // unblock it
        else continue;
      }

      else if (n->third->state == S_BOX_RECV) {
        // box recv block. check to see if there is available data, if so,
        // reschedule the third.
        third_box_t * box = n->third->current_box;
        if (box->slot_state[box->current_unread] == S_UNREAD)
          n->third->state = S_RUNNING; // unblock it
        else continue;
      }

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

  new->slot_state = calloc(sizeof(enum { S_READ, S_UNREAD }), num_slots);
  int i;
  for (i = 0; i < num_slots; i++) new->slot_state[i] = S_READ;
  return new;
}

void
third_box_send (third_box_t * box, third_t * from, void * data) {
  third_mutex_lock(from, box->mutex);

  if (box->slot_state[box->next_empty] == S_UNREAD) {
    third_mutex_unlock(from, box->mutex);
    from->current_box = box;
    from->state = S_BOX_SEND;
    third_yield(from);
    third_mutex_lock(from, box->mutex);
  } 

  memcpy(box->slots + (box->next_empty * box->slot_size), data, box->slot_size);
  box->slot_state[box->next_empty] = S_UNREAD;
  box->next_empty += 1;
  if (box->next_empty >= box->num_slots) box->next_empty = 0;

  third_mutex_unlock(from, box->mutex);
}

void *
third_box_recv (third_box_t * box, third_t * me) {
  third_mutex_lock(me, box->mutex);

  if (box->slot_state[box->current_unread] == S_READ) {
    third_mutex_unlock(me, box->mutex);
    me->current_box = box;
    me->state = S_BOX_RECV;
    third_yield(me);
    third_mutex_lock(me, box->mutex);
  }

  void * data = box->slots + (box->current_unread * box->slot_size);
  box->slot_state[box->current_unread] = S_READ;
  box->current_unread += 1;
  if (box->current_unread >= box->num_slots) box->current_unread = 0;

  third_mutex_unlock(me, box->mutex);
  return data;
}
