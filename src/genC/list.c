/*

	-- NewGen Project

	The NewGen software has been designed by Remi Triolet and Pierre
	Jouvelot (Ecole des Mines de Paris). This prototype implementation
	has been written by Pierre Jouvelot.

	This software is provided as is, and no guarantee whatsoever is
	provided regarding its appropriate behavior. Any request or comment
	should be sent to newgen@isatis.ensmp.fr.

	(C) Copyright Ecole des Mines de Paris, 1989

*/

/* 
 * $Id$
 */

/* -- list.c

   The following functions implement a small library of utilities in the
   Lisp tradition. 

   . GEN_EQ is pointer comparison,
   . GEN_LENGTH returns the length of the list CP,
   . GEN_MAPL applies (*FP) to every CDR of CP.
   . GEN_MAP applies (*FP) to every item of the list.
   . GEN_REDUCE successively applies (*FP) on R adn every CRD of CP. 
   . GEN_SOME aplies (*FP) to every CDR of CP and returns the first sublist
     whose CAR verifies (*FP).
   . GEN_INSERT_AFTER inster a new object after a specified boject in the list
   . GEN_MAPC_TABULATED applies (*FP) on every non null element of the
     domain BINDING.
   . GEN_FIND_TABULATED retrieves the object of KEY in the tabulated DOMAIN.
   . GEN_FILTER_TABULATED returns a list of objects from DOMAIN that verify
     the FILTER.
   . GEN_FREE_LIST frees the spine of the list L.     
   . GEN_FULL_FREE_LIST frees the whole list L.     
   . GEN_NCONC physically concatenates CP1 and CP2 (returns CP1).
   . GEN_COPY copies one gen_chunks in another.
   . GEN_FIND_IF returns the leftmost element (extracted from the cons cell 
     by EXTRACT) of the sequence SEQ that satisfies TEST. EXTRACT should 
     be one of the car_to_domain function that are automatically generated 
     by Newgen.
   . GEN_FIND_IF_FROM_END is equivalent to GEN_FIND_IF but returns the 
     rightmost element of SEQ.
   . GEN_FIND returns the leftmost element (extracted from the cons cell 
     by EXTRACT) of the sequence SEQ such that TEST returns TRUE when applied 
     to ITEM and this element. EXTRACT should be one of the car_to_domain 
     function that are automatically generated by Newgen.
   . GEN_FIND_FROM_END is equivalent to GEN_FIND but returns the rightmost 
     element of SEQ.
   . GEN_FIND_EQ
   . GEN_CONCATENATE concatenates two lists. the structures of both lists are
     duplicated.
   . GEN_APPEND concatenates two lists. the structure of the first list is
     duplicated.
   . GEN_COPY_SEQ
   . GEN_FULL_COPY_LIST
   . GEN_COPY_STRING_LIST
   . GEN_FREE_STRING_LIST
   . GEN_LAST returns the last cons of a list.
   . GEN_REMOVE updates the list (pointer) CPP by removing (and freeing) any
     ocurrence of the gen_chunk OBJ.
   . GEN_REMOVE_ONCE : Remove the first occurence of obj in list l.
   . GEN_NTHCDR returns the N-th (beginning at 1) CDR element of L.
     CDR(L) = GEN_NTHCDR(1,L).
   . GEN_NTH returns the N-th (beginning at 0) car of L.
     CAR(L) = GEN_NTH(0,L).
   . GEN_SORT_LIST(L, compare) sorts L in place with compare (see man qsort)
   . GEN_ONCE(ITEM, L) prepends ITEM to L if not already there.
   . GEN_IN_LIST_P(ITEM, L) checks that item ITEM appears in list L
   . GEN_OCCURENCES(ITEM, L) returns the number of occurences of item ITEM in list L
   . GEN_ONCE_P(L) checks that each item in list L appears only once
   . GEN_CLOSURE()
   . GEN_MAKE_LIST(DOMAIN, ...) makes an homogeneous list of the varargs (but
     homogeneity is not checked)
   . gen_list_and(list * a, list b) : Compute A = A inter B
   . gen_list_and_not(list * a, list b) : Compute A = A inter non B
   . gen_list_patch(list l, gen_chunk * x, gen_chunk * y) :
     Replace all the reference to x in list l by a reference to y
   . gen_position(void * item, list l): rank of item in list l, 0 if not present
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "genC.h"
#include "newgen_include.h"

int gen_eq(void * obj1, void * obj2)
{
    return obj1 == obj2;
}

size_t gen_length(list cp)
{
  int i ;
  for( i=0; cp != NIL ; cp = cp->cdr, i++);
  return i;
}

size_t list_own_allocated_memory(list l)
{
    return gen_length(l)*sizeof(cons);
}

/*   MAP
 */
void gen_mapl(void (*fp)(), list cp)
{
  for (; cp != NIL ; cp = cp->cdr)
      (*fp)(cp);
}

void gen_map(void (*fp)(), list l)
{
    for (; !ENDP(l); l=CDR(l))
	(*fp)(CHUNK(CAR(l)));
}

char * gen_reduce(char * r, char *(*fp)(), list cp)
{
    for( ; cp != NIL ; cp = cp->cdr ) {
	r = (*fp)( r, cp ) ;
    }
    return( r ) ;
}

cons * gen_some(bool (*fp)(), list cp)
{
    for( ; cp!= NIL ; cp = cp->cdr )
	if( (*fp)( cp ))
		return( cp ) ;
    return NIL;
}

/*  SPECIAL INSERTION
 */
static gen_chunk *gen_chunk_of_cons_of_gen_chunk = gen_chunk_undefined;
static bool cons_of_gen_chunk(list cp)
{
    return(CHUNK(CAR(cp))==gen_chunk_of_cons_of_gen_chunk);
}

void gen_insert_after(void * no, void * o, list l)
{
    gen_chunk * new_obj = (gen_chunk*) no, * obj = (gen_chunk*) o;
    cons *obj_cons = NIL;
    gen_chunk_of_cons_of_gen_chunk = obj;

    obj_cons = gen_some(cons_of_gen_chunk, l);
    assert(!ENDP(obj_cons));
    CDR(obj_cons) = CONS(CHUNK, new_obj, CDR(obj_cons));
}

/* 
   insert object "no" before object "o" in the list "l". Return the new
   list.
*/
list gen_insert_before(void * no, void * o, list l)
{
  gen_chunk * new_obj = (gen_chunk*) no;
  gen_chunk * obj = (gen_chunk*) o;
  
  list r = NIL; /* result   */
  list c = l;   /* current  */
  list p = NIL; /* previous */
  
  /* search obj in list */
  for ( ; c!=NIL ; c=c->cdr) 
    if ( CHUNK(CAR(c))==obj )
      break;
    else
      p = c;

  assert(!ENDP(c));

  if (p) { /* obj is not the first object of the list */
    CDR(p) = CONS(CHUNK, new_obj, CDR(p));
    r = l;
  }
  else { /* obj is the first object */
    r = CONS(CHUNK, new_obj, c); 
  }
  return r;
}

#define NEXT(cp) (((cp) == NIL) ? NIL : (cp)->cdr)

list gen_nreverse(list cp)
{
    cons *next, *next_next ;

    if( cp == NIL || cp->cdr == NIL ) return( cp ) ;

    next = cp->cdr ;
    cp->cdr = NIL ;
    next_next = NEXT( next ) ;
        
    for( ; next != NIL ; ) {
	next->cdr = cp ;
	cp = next ;
	next = next_next ;
	next_next = NEXT( next_next ) ;
    }
    return( cp ) ;
}

void gen_free_list(list l)
{
    list p, nextp ;
    for( p = l ; p != NIL ; p = nextp ) {
	nextp = p->cdr ;
	CAR(p).p = NEWGEN_FREED; /* clean */
	CDR(p) = (struct cons*) NEWGEN_FREED;
	free( p ) ;
    }
}

list gen_nconc(list cp1, list cp2)
{
    cons *head = cp1 ;

    if( cp1 == NIL ) 
	return( cp2 ) ;

    for( ; !ENDP( CDR( cp1 )) ; cp1 = CDR( cp1 )) 
	    ;
    CDR( cp1 ) = cp2 ;
    return( head ) ;
}

void gen_copy(void * a, void * b)
{
    * (gen_chunk*)a = * (gen_chunk*)b ;
}

void * gen_car(list l)
{
    return CHUNK(CAR(l));
}

void * gen_find_if(bool (*test)(), list pc, void *(*extract)())
{
    for (; pc!=NIL; pc=pc->cdr)
	if ((*test)((*extract)(CAR(pc))))
	    return (*extract)(CAR(pc));

    return gen_chunk_undefined;
}

/*  the last match is returned
 */
void * gen_find_if_from_end(bool (*test)(), list pc, void *(*extract)())
{
    void * e = gen_chunk_undefined ;

    for (; pc!=NIL; pc=pc->cdr)
	if ((*test)((*extract)(CAR(pc))))
	    e = (*extract)(CAR(pc));

    return e;
}

void * gen_find(void * item, list seq, bool (*test)(), void *(*extract)())
{
    list pc;
    for (pc = seq; pc != NIL; pc = pc->cdr )
	if ((*test)(item, (*extract)(CAR(pc))))
		return (*extract)(CAR(pc));
    return gen_chunk_undefined;
}

void * gen_find_from_end(
    void * item, list seq, bool (*test)(), void *(*extract)())
{
    list pc;
    void * e = gen_chunk_undefined ;

    for (pc = seq; pc != NIL; pc = pc->cdr ) {
	if ((*test)(item, (*extract)(CAR(pc))))
		e = (*extract)(CAR(pc));
    }

    return e;
}

void *gen_find_eq(void * item, list seq)
{
    list pc;
    for (pc = seq; pc != NIL; pc = pc->cdr )
	if (item == CAR(pc).p)
		return CAR(pc).p;
    return gen_chunk_undefined;
}

list gen_concatenate(list l1, list l2)
{
    list l = NIL, q = NIL;

    if (l1 != NIL) {
	l = q = CONS(CHUNK, CHUNK(CAR(l1)), NIL);
	l1 = CDR(l1);
    }
    else if (l2 != NIL) {
	l = q = CONS(CHUNK, CHUNK(CAR(l2)), NIL);
	l2 = CDR(l2);
    }
    else {
	return(NIL);
    }

    while (l1 != NIL) {
	CDR(q) = CONS(CHUNK, CHUNK(CAR(l1)), NIL);
	q = CDR(q);

	l1 = CDR(l1);
    }

    while (l2 != NIL) {
	CDR(q) = CONS(CHUNK, CHUNK(CAR(l2)), NIL);
	q = CDR(q);

	l2 = CDR(l2);
    }

    return(l);
}

list gen_append(list l1, list l2)
{
    cons *l = NIL, *q = NIL;

    if (l1 == NIL)
	return(l2);

    l = q = CONS(CHUNK, CHUNK(CAR(l1)), NIL);
    l1 = CDR(l1);

    while (l1 != NIL) {
	CDR(q) = CONS(CHUNK, CHUNK(CAR(l1)), NIL);
	q = CDR(q);

	l1 = CDR(l1);
    }

    CDR(q) = l2;

    return(l);
}

list gen_copy_seq(list l)
{
    cons *nlb = NIL, *nle = NIL;

    while (! ENDP(l)) {
	cons *p = CONS(CHUNK, CHUNK(CAR(l)), NIL);

	if (nle == NIL)
	    nlb = p;
	else
	    CDR(nle) = p;
	nle = p;
	l = CDR(l);
    }

    return(nlb);
}

list gen_full_copy_list(list l)
{
    cons *nlb = NIL, *nle = NIL;

    while (! ENDP(l)) {
	cons *p = CONS(CHUNK, gen_copy_tree(CHUNK(CAR(l))), NIL);
	
	if (nle == NIL)
	    nlb = p;
	else
	    CDR(nle) = p;
	nle = p;
	l = CDR(l);
    }

    return nlb;
}

list /* of string */
gen_copy_string_list(list /* of string */ ls)
{
    list l = NIL;
    MAP(STRING, s, l = CONS(STRING, strdup(s), l), ls);
    return gen_nreverse(l);
}

void
gen_free_string_list(list /* of string */ ls)
{
    gen_map(free, ls);
    gen_free_list(ls);
}

list gen_last(list l)
{
    if (ENDP(l)) return(l);         /* NIL case */
    while (!ENDP(CDR(l))) l=CDR(l); /* else go to the last */
    return(l);
}

static void gen_remove_from_list(list * pl, void * o, bool once)
{
  list * pc = pl;
  while (*pc)
  {
    if ((gen_chunk*) o == CHUNK(CAR(*pc)))
    {
      list tmp = *pc;
      *pc = CDR(*pc);
      CAR(tmp).p = NEWGEN_FREED;
      CDR(tmp) = NEWGEN_FREED;
      free(tmp);
      if (once) return;
    }
    else 
      pc = &CDR(*pc);
  }
}
	
void gen_remove(list * cpp, void * o)
{
  gen_remove_from_list(cpp, o, FALSE);
}

/* Remove the first occurence of o in list pl: */
void gen_remove_once(list * pl, void * o)
{
  gen_remove_from_list(pl, o, TRUE);
}

/*  caution: the first item is 0!
 *  was:  return( (n<=0) ? l : gen_nthcdr( n-1, CDR( l ))) ;
 *  if n>gen_length(l), NIL is returned.
 */
list gen_nthcdr(int n, list l)
{
    message_assert("valid n", n>=0);
    for (; !ENDP(l) && n>0; l=CDR(l), n--);
    return(l);
}

/* to be used as ENTITY(gen_nth(3, l))...
 */
gen_chunk gen_nth(int n, list l)
{
    list r = gen_nthcdr(n, l);
    message_assert("not NIL", r);
    return CAR(r);
}


/* Prepend an item to a list only if it is not already in the list.

   Return the list anyway.
*/
list gen_once(void * vo, list l)
{
    gen_chunk * item = (gen_chunk*) vo;
    list c;
    for(c=l; c!=NIL; c=CDR(c))
	if (CHUNK(CAR(c))==item) return(l);

    return(CONS(CHUNK, item, l));
}

bool gen_in_list_p(void * vo, list l)
{
    gen_chunk * item = (gen_chunk*) vo;
    for (; !ENDP(l); POP(l))
	if (CHUNK(CAR(l))==item) return(TRUE); /* found! */

    return(FALSE); /* else no found */
}

int gen_occurences(void * vo, list l)
{
    list c;
    int n = 0;
    gen_chunk * item = (gen_chunk*) vo;
    for (c=l; !ENDP(c); POP(c))
	if (CHUNK(CAR(c))==item) n++;

    return n;
}

bool gen_once_p(list l)
{
    list c;
    for(c=l; c!=NIL && CDR(c)!=NIL; c=CDR(c)) {
	gen_chunk * item = CHUNK(CAR(c));
	if(gen_in_list_p(item , CDR(c)))
	    return FALSE;
    }

    return TRUE;
}

/* free an area.
 * @param p pointer to the zone to be freed.
 * @param size size in bytes.
 */
void gen_free_area(void ** p, int size)
{
  int n = size/sizeof(void*);
  int i;
  for (i=0; i<n; i++) {
    *(p+i) = NEWGEN_FREED;
  }
  free(p);
}

/* Sorts a list of gen_chunks in place, to avoid allocations...
 * The list skeleton is not touched, but the items are replaced
 * within the list. If some of the cons are shared, it may trouble
 * the data and the program.
 *
 * See man qsort about the compare function: 
 *  - 2 pointers to the data are passed, 
 *  - and the result is <, =, > 0 if the comparison is lower than, equal...
 *
 * FC 27/12/94
 */
void gen_sort_list(list l, int (*compare)())
{
    list c;
    int n = gen_length(l);
    gen_chunk 
	**table = (gen_chunk**) alloc(n*sizeof(gen_chunk*)),
	**point;

    /*   the list items are first put in the temporary table,
     */
    for (c=l, point=table; !ENDP(c); c=CDR(c), point++)
	*point = CHUNK(CAR(c));
    
    /*    then sorted,
     */
    qsort(table, n, sizeof(gen_chunk*), compare);

    /*    and the list items are updated with the sorted table
     */
    for (c=l, point=table; !ENDP(c); c=CDR(c), point++)
	CHUNK(CAR(c)) = *point;

    gen_free_area((void**) table, n*sizeof(gen_chunk*)); 
}

/* void gen_closure(iterate, initial)
 * list [of X] (*iterate)([ X, list of X ]), initial;
 * 
 * what: computes the transitive closure of sg starting from sg.
 * how: iterate till stability.
 * input: an iterate function and an initial list for the closure.
 *        the iterate functions performs some computations on X
 *        and should update the list of X to be looked at at the next 
 *        iteration. This list must be returned by the function.
 * output: none.
 * side effects:
 *  - *none* on initial...
 *  - those of iterate.
 * bugs or features:
 *  - not idiot proof. may run into an infinite execution...
 *  - a set base implementation would be nicer, but less deterministic.
 */
void gen_closure(iterate, initial)
list /* of X */ (*iterate)(/* X, list of X */), initial;
{
    list /* of X */ l_next, l_close = gen_copy_seq(initial);
    while (l_close)
    {
	l_next = NIL;
	MAPL(cc, l_next = iterate(CHUNK(CAR(cc)), l_next), l_close);
	gen_free_list(l_close), l_close = l_next;
    }
}

list gen_make_list(int domain, ...)
{
    list l, current;
    gen_chunk *item;
    va_list args;
    va_start(args, domain);

    item = va_arg(args, gen_chunk*);
    if (!item) return NIL;

    NEWGEN_CHECK_TYPE(domain, item);

    l = CONS(CHUNK, item, NIL), current = l;
    while((item=va_arg(args, gen_chunk*))) {
      NEWGEN_CHECK_TYPE(domain, item);
      CDR(current) = CONS(CHUNK, item, NIL), POP(current);
    }
    return l;
    va_end(args);
}

list gen_cons(void * item, list next)
{
  list ncons = (list) alloc(sizeof(struct cons));
  ncons->car.e = item;
  ncons->cdr = next;
  return ncons;
}

/* CONS a list with minimal type checking
 * this cannot be done within the CONS macro because
 * possible functions calls must not be replicated.
 */
list gen_typed_cons(intptr_t type, void * item, list next)
{
  NEWGEN_CHECK_TYPE(type, item);
  return gen_cons(item, next);
}

/* typed cons for "basic" types */
list gen_bool_cons(bool b, list l)
{
  return gen_cons((void *) b, l);
}

list gen_int_cons(int i, list l)
{
  return gen_cons((void *) i, l);
}

list gen_string_cons(string s, list l)
{
  return gen_cons((void *) s, l);
}

list gen_list_cons(list i, list l)
{
  return gen_cons((void *) i, l);
}

list gen_CHUNK_cons(gen_chunk * c, list l)
{
  return gen_cons((void *) c, l);
}

/* Compute A = A inter B: complexity in O(n2) */
void
gen_list_and(list * a,
	     list b)
{
    if (ENDP(*a))
	return ;

    if (!gen_in_list_p(CHUNK(CAR(*a)), b)) {
	/* This element of a is not in list b: delete it: */
	cons *aux = *a;

	*a = CDR(*a);
	CAR(aux).p = NEWGEN_FREED;
	CDR(aux) = NEWGEN_FREED;
	free(aux);
	gen_list_and(a, b);
    }
    else
	gen_list_and(&CDR(*a), b);
}


/* Compute A = A inter non B: */
void
gen_list_and_not(list * a, list b)
{
    if (ENDP(*a))
	return ;

    if (gen_in_list_p(CHUNK(CAR(*a)), b)) {
	/* This element of a is in list b: delete it: */
	cons *aux = *a;

	*a = CDR(*a);
	CAR(aux).p = NEWGEN_FREED;
	CDR(aux) = NEWGEN_FREED;
	free(aux);
	gen_list_and_not(a, b);
    }
    else
	gen_list_and_not(&CDR(*a), b);
}


/* Replace all the reference to x in list l by a reference to y: */
void
gen_list_patch(list l, void * x, void * y)
{
    MAPL(pc, {
	 if (CAR(pc).p == (gen_chunk *) x)
	     CAR(pc).p = (gen_chunk *) y;
     }, l);
}

/* Element ranks are strictly positive as for first, second, and so on. If
   item is not in l, 0 is returned. */
int gen_position(void * item, list l)
{
  list c_item = list_undefined;
  int rank = 0;

  for(c_item = l; !ENDP(c_item); POP(c_item)) {
    rank++;
    if(item==CHUNK(CAR(c_item))) {
      return rank;
    }
  }
  return 0;
}


/*   That is all
 */
