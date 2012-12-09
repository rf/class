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
} third_scheduler_t;

typedef void (*third_entry_t)(struct third * me, void * arg);

typedef struct third {
  ucontext_t * context;
  third_entry_t entry;
  enum { S_RUNNING, S_BLOCKED } state;
  struct third_mutex * current_mutex;
  third_scheduler_t * scheduler;
} third_t;

typedef struct third_node {
  third_t * third;
  struct third_node * next;
} third_node_t;

typedef struct third_mutex {
  enum { M_LOCKED, M_UNLOCKED } state;
  third_t * locked_by;
} third_mutex_t;

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

#endif // third_H
