#ifndef CROW_H
#define CROW_H

#include <ucontext.h>
#include <stdbool.h>

#define STACK_SIZE 16384 // 1mb stack size by default

// This struct describes a scheduler, which has his own context and a queue of
// crows to run.
typedef struct crow_scheduler {
  struct crow_node * queue;
  ucontext_t * context;
  bool running;
} crow_scheduler_t;

typedef void (*crow_entry_t)(crow_scheduler_t * scheduler, void * arg);

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
  if (prev != NULL) { object->next = prev; prev = object; } \
  else prev = object; \
  prev->next = NULL;
  
#define insert(prev, object) object->next = prev->next; prev->next = object;

#define foreach(list, node) \
  for (node = list; node != NULL; node = node->next) \

#endif // CROW_H