#ifndef _MINHEAP_H
#define _MINHEAP_H

#include <stdbool.h>


/*********************/
/* Client interface  */
/*********************/

typedef void* elem;  // Supplied by client

// f(x,y) returns true if e1 is STRICTLY higher priority than e2
typedef bool has_higher_priority_fn(elem e1, elem e2);
//@requires e1 != NULL && e2 != NULL;

typedef void elem_free_fn(elem e);
//@requires e != NULL;


/*********************/
/* Library interface */
/*********************/

typedef struct heap_header* pq_t;

bool pq_empty(pq_t Q)
  /*@requires Q != NULL; @*/ ;

bool pq_full(pq_t Q)
  /*@requires Q != NULL; @*/ ;

pq_t pq_new(int capacity, has_higher_priority_fn* prior, elem_free_fn* fr)
  /*@requires capacity > 0 && prior != NULL && fr != NULL; @*/
  /*@ensures \result != NULL && pq_empty(\result); @*/ ;

void pq_add(pq_t Q, elem x)
  /*@requires Q != NULL && !pq_full(Q) && x != NULL; @*/ ;

elem pq_rem(pq_t Q)
  /*@requires Q != NULL && !pq_empty(Q); @*/
  /*@ensures \result != NULL; @*/ ;

elem pq_peek(pq_t Q)
  /*@requires Q != NULL && !pq_empty(Q); @*/
  /*@ensures \result != NULL; @*/ ;

void pq_free(pq_t Q)
  /*@requires Q != NULL; @*/ ;
#endif
