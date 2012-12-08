#ifndef CROW_H
#define CROW_H

#define __USE_GNU // expose the inner structs of ucontext
#include <ucontext.h>
#include <stdbool.h>

#define STACK_SIZE 16384 // 16k stack size by default

// This struct describes a scheduler, which has his own context and a queue of
// crows to run.
typedef struct crow_scheduler {
  struct crow_node * queue;
  ucontext_t * context;
  struct crow * current;
  bool running;
} crow_scheduler_t;

typedef void (*crow_entry_t)(struct crow * me, crow_scheduler_t * scheduler, void * arg);

typedef struct crow {
  ucontext_t * context;
  crow_entry_t entry;
} crow_t;

typedef struct crow_node {
  crow_t * crow;
  struct crow_node * next;
} crow_node_t;

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

#endif // CROW_H
