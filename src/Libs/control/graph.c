/*
 *  FULL CONTROL GRAPH
 *
 *  $RCSfile: graph.c,v $ ($Date: 1995/03/22 11:31:23 $, )
 *  version $Revision$
 */

#include <stdio.h>
#include <strings.h>
extern int fprintf();

#include "genC.h"
#include "ri.h"
#include "text.h"
#include "text-util.h"
#include "database.h"
#include "properties.h"

#include "misc.h"
#include "ri-util.h"
#include "resources.h"
#include "pipsdbm.h"
#include "control.h"

/* global mapping from statements to their control in the control graph
 */
GENERIC_GLOBAL_FUNCTION(ctrl_graph, controlmap, statement, control);

/*  add (s1) --> (s2), 
 *  that is s2 as successor of s1 and s1 as predecessor of s2.
 *  last added put in first place. 
 *  this property is used for a depth first enumeration.
 */
static void add_arrow_in_ctrl_graph(s1, s2)
statement s1, s2;
{
    control 
	c1 = load_ctrl_graph(s1), 
	c2 = load_ctrl_graph(s2);

    debug(7, "add_arrow_in_ctrl_graph",
	  "0x%x -> 0x%x\n", (unsigned int) s1, (unsigned int) s2);

    control_successors(c1) = gen_once(c2, control_successors(c1));
    control_predecessors(c2) = gen_once(c1, control_predecessors(c2));
}

static void add_arrows_in_ctrl_graph(s, l)
statement s;
list l;
{
    for(; !ENDP(l); l=CDR(l))
	add_arrow_in_ctrl_graph(s, STATEMENT(CAR(l)));
}

static list control_list_to_statement_list(l)
list /* of controls */ l;
{
    list /* of statements */ r;

    if (ENDP(l)) return(NIL);
    
    r = control_list_to_statement_list(CDR(l));
    r = CONS(STATEMENT, control_statement(CONTROL(CAR(l))), r);
    return(r);
}

static void statement_arrows(s, next)
statement s;
list next;
{
    instruction i = statement_instruction(s);
    tag t = instruction_tag(i);

    switch(t)
    {
    case is_instruction_block:
    {
	statement current, succ;
	list 
	    l = instruction_block(i),
	    just_next;
	
	if (ENDP(l)) 
	{
	    add_arrows_in_ctrl_graph(s, next);
	    return;
	}
	/* else
	 */
	add_arrow_in_ctrl_graph(s, STATEMENT(CAR(l)));
	
	for(current = STATEMENT(CAR(l)),
	        l = CDR(l), 
	        succ = ENDP(l) ? statement_undefined : STATEMENT(CAR(l)),
	        just_next = CONS(STATEMENT, succ, NIL);
	    !ENDP(l);
	    current = succ, 
 	        l = CDR(l),
	        succ = ENDP(l) ? statement_undefined : STATEMENT(CAR(l)),
	        STATEMENT(CAR(just_next)) = succ)
	{
	    statement_arrows(current, just_next);
	}
	
	gen_free_list(just_next), just_next=NIL;

	statement_arrows(current, next);
	break;
    }
    case is_instruction_test:
    {
	test
	    x = instruction_test(i);
	statement 
	    true = test_true(x),
	    false = test_false(x);
	
	add_arrow_in_ctrl_graph(s, false),
	statement_arrows(false, next);

	add_arrow_in_ctrl_graph(s, true), /* true is before false */
	statement_arrows(true, next);

	break;
    }
    case is_instruction_loop:
    {
	loop l = instruction_loop(i);
	statement b = loop_body(l);
	list just_next = 
	    gen_nconc(gen_copy_seq(next), CONS(STATEMENT, s, NIL));
	
	add_arrows_in_ctrl_graph(s, next); /* no iteration */
	add_arrow_in_ctrl_graph(s, b);     /* some iterations, first */

	statement_arrows(b, just_next);

	gen_free_list(just_next), just_next=NIL;

	break;
    }
    case is_instruction_call:
	add_arrows_in_ctrl_graph(s, next);
	break;
    case is_instruction_unstructured:
    {
	unstructured u = instruction_unstructured(i);
	list 
	    blocks = NIL,
	    lstat = NIL;	
	statement x;

	/* hmmm... I'm not too confident in this loop.
	 * ??? what should be done with next?
	 * ??? should I trust the graph? I hope I can.
	 */
	CONTROL_MAP(c,
		{
		    x = control_statement(c);
		    lstat = control_list_to_statement_list
			(control_successors(c));

		    statement_arrows(x, lstat);
		    gen_free_list(lstat);
		},
		    unstructured_control(u),
		    blocks);

	gen_free_list(blocks);

	break;
    }
    case is_instruction_goto:
    default:
	pips_error("statement_arrows", "unexpected instruction tag (%d)\n", t);
    }
}

static void stmt_rewrite(s)
statement s;
{
    if (!bound_ctrl_graph_p(s))
	store_ctrl_graph(s, make_control(s, NIL, NIL));
}

void build_full_ctrl_graph(s)
statement s;
{
    debug(3, "build_full_ctrl_graph", "statement 0x%x\n", (unsigned int) s);

    /*  first pass to initialize the ctrl_graph controls
     */
    init_ctrl_graph();
    gen_multi_recurse(s,
		      /*  STATEMENT
		       */
		      statement_domain,
		      gen_true,
		      stmt_rewrite,
		      NULL);

    /*  second pass to link the statements
     */
    statement_arrows(s, NIL);
}

/* FULL CONTROL GRAPH for module NAME
 */
void full_control_graph(name)
string name;
{
    statement s = (statement) db_get_resource(DBR_CODE, name, TRUE);
    build_full_ctrl_graph(s);
    
    /*  should put something in the db if made as a pass
     */
}

/* TRAVELLING on the control graph
 * 
 * init, next, close functions.
 * - the init function is given a starting point (statement) s and a 
 *   decision function decision. The function should say whether to
 *   go on with the successors of its arguments.
 * - the next function gives the next visited statement.
 * - the close function frees the static data.
 */

#ifndef bool_undefined
#define bool_undefined ((bool) (-15))
#define bool_undefined_p(b) ((b)==bool_undefined)
#endif

/* Static data for the travel.
 * - the stack stores the statements to see
 * - the mapping stores the already stacked statements
 * - decision function...
 */
DEFINE_LOCAL_STACK(to_see, statement);
GENERIC_LOCAL_MAPPING(stacked, bool, statement);
static bool (*travel_decision)();

static void push_if_necessary(s)
statement s;
{
    if (load_statement_stacked(s)!=TRUE)
	to_see_push(s),
	store_statement_stacked(s, TRUE);
}

/* it is pushed in *reverse* order to preserve the depth first view.
 */
static void push(l) 
/* control */ list l;
{
    if (!ENDP(l)) 
	push(CDR(l)),
	push_if_necessary(control_statement(CONTROL(CAR(l))));
}

static void push_successors(s)
statement s;
{
    push(control_successors(load_ctrl_graph(s)));
}

void init_ctrl_graph_travel(s, decision)
statement s;
bool (*decision)();
{
    make_to_see_stack();             /* initializations */
    make_stacked_map();
    travel_decision = decision;

    store_statement_stacked(s, TRUE); /* no loop back */
    push_successors(s);
}

bool next_ctrl_graph_travel(ps)
statement *ps;
{
    if (to_see_empty_p()) /* nothing else to visit */
	return(FALSE);

    /*  else get the next and push its successors if needed
     */
    *ps = to_see_pop();
    if ((*travel_decision)(*ps))
	push_successors(*ps);

    return(TRUE);
}

void close_ctrl_graph_travel()
{
    free_to_see_stack();
    free_stacked_map();
}


/*  That's all
 */
