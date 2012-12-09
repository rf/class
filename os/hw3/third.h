#ifndef third_H
#define third_H

#define __USE_GNU // expose the inner structs of ucontext
#include <ucontext.h>
#include <stdbool.h>

#define STACK_SIZE 16384 // 16k stack size by default

// This struct describes a scheduler, which has his own context and a queue of
// thirds to run.
typedef struct third_scheduler {
  struct third_node * queue;
  ucontext_t * context;
  struct third * current;
  bool running;
  // for disabling the scheduler during mutex operations, to make them atomic.
  // This is a silly solution, but it should work since dropping an int into
  // memory (as long as it's aligned) should be atomic on x86
  bool disabled;
} third_scheduler_t;

typedef void (*third_entry_t)(struct third * me, void * arg);

typedef struct third {
  ucontext_t * context;
  third_entry_t entry;
  third_scheduler_t * scheduler;

  enum { RUNNING, BLOCKED, BOX_SEND, BOX_RECV, DONE, JOIN } state;

  // stuff it can be blocked by
  union {
    struct third_mutex * mutex;
    struct third_box * box;
    struct third * join;
  } blocked;
} third_t;

typedef struct third_node {
  third_t * third;
  struct third_node * next;
} third_node_t;

typedef struct third_mutex {
  enum { LOCKED, UNLOCKED } state;
  third_t * locked_by;
} third_mutex_t;

typedef struct third_box {
  third_mutex_t * mutex;
  void * slots;
  enum { READ, UNREAD } * slot_state;
  int slot_size;
  int num_slots;
  int next_empty;
  int current_unread;   // points to the next slot to fill
} third_box_t;

#define create(object) (object *) calloc(sizeof(object), 1)

#define prepend(prev, object) \
  if (prev != NULL) { object->next = prev; prev->next = NULL; prev = object; } \
  else prev = object; \
  
#define insert(prev, object) object->next = prev->next; prev->next = object;

#define foreach(list, node) \
  for (node = list; node != NULL; node = node->next) \

#define handle_error(stuff) \
  if (stuff != 0) { \
    perror("stuff"); \
    exit(EXIT_FAILURE); \
  } 

third_scheduler_t * third_setup ();
third_t * third_create (third_scheduler_t * scheduler, third_entry_t entry, void * arg);

void third_yield (third_t * third);

third_mutex_t * third_mutex_create ();
void third_mutex_lock (third_t * locker, third_mutex_t * mutex);
void third_mutex_trylock (third_t * locker, third_mutex_t * mutex);
void third_mutex_unlock (third_t * locker, third_mutex_t * mutex);

void third_begin (third_scheduler_t * scheduler, bool preemption);

void * third_box_recv (third_box_t * box, third_t * me);
void third_box_send (third_box_t * box, third_t * from, void * data);
third_box_t * third_box_create (int num_slots, int slot_size);
void third_join (third_t * me, third_t * him);

#endif // third_H
