/* 
   icfg_scan.c
   module_to_icfg(0, mod) recursively to_icfgs module "mod" and its callees
   and writes its icfg in indented form
*/
#include <stdio.h>
#include <string.h>

#include "genC.h"
#include "list.h"
#include "ri.h"
#include "control.h"      /* CONTROL_MAP is defined there */
#include "text.h"
#include "text-util.h"
#include "ri-util.h"
#include "properties.h"  /* get_bool_property */
#include "misc.h"
#include "database.h"
#include "effects.h"
#include "regions.h"
#include "resources.h"
#include "semantics.h"
#include "prettyprint.h"
#include "complexity_ri.h"
#include "complexity.h"
#include "pipsdbm.h"      /* DB_PUT_FILE_RESOURCE is defined there */
#include "text-util.h"
#include "icfg.h"

#define ICFG_SCAN_INDENT 4
#define MAX_LINE_LENGTH 256

static int current_margin;

static void append_icfg_file(text t,string module_name);

typedef hash_table text_mapping;
#ifndef bool_undefined
#define bool_undefined ((bool) (-15))
#define bool_undefined_p(b) ((b)==bool_undefined)
#endif

#define some_text_p(t) (t!=text_undefined && text_sentences(t)!=NIL)

/* We want to keep track of the current statement inside the recurse */
DEFINE_LOCAL_STACK(current_stmt, statement)

/* We store the text for all statement in a mapping 
   in order to print afterwards */
GENERIC_LOCAL_MAPPING(stmt_map, statement, text)

static bool statement_filter(statement s)
{
    bool res = TRUE;
    text t = make_text (NIL);

    pips_debug (5,"going down\n");

    store_text_stmt_map (s, t);
    res = current_stmt_filter (s);
    return res;
}

static void statement_rewrite(statement s)
{
    pips_debug (5,"going up\n");
    ifdebug(9) {
	print_text(stderr,(text) load_text_stmt_map (s));
    }
    current_stmt_rewrite (s);
    return;
}

static void call_filter(call c)
{
    entity e_callee = call_function(c);
    string callee_name = module_local_name(e_callee);

    /* current_stmt_head() */
    pips_debug (5,"called entity is %s\n", entity_name(e_callee));

    /* If this is a "real function" (defined in the code elsewhere) */
    if (value_code_p(entity_initial(e_callee))) {
	text r = (text) load_text_stmt_map (current_stmt_head());

	entity e_caller = get_current_module_entity();
	reset_current_module_entity();

	switch (get_int_property (ICFG_DECOR)) {
	case ICFG_DECOR_NONE:
	    break;
	case ICFG_DECOR_COMPLEXITIES:
	    MERGE_TEXTS(r,get_text_complexities(callee_name));
	    break;
	case ICFG_DECOR_TRANSFORMERS:
	    MERGE_TEXTS(r,get_text_transformers(callee_name));
	    break;
	case ICFG_DECOR_PRECONDITIONS:
	{
	    transformer call_site_prec = 
		call_site_to_module_precondition(e_caller,
						 e_callee,
						 current_stmt_head(),
						 c);

	    /* Then print the text for the caller preconditions */
	    set_current_module_entity(e_caller);
	    MERGE_TEXTS(r,text_transformer(call_site_prec));
	    reset_current_module_entity();

	    break;
	}
	case ICFG_DECOR_PROPER_EFFECTS:
	    MERGE_TEXTS(r,get_text_proper_effects(callee_name));
	    break;
	case ICFG_DECOR_CUMULATED_EFFECTS:
	    MERGE_TEXTS(r,get_text_cumulated_effects(callee_name));
	    break;
	case ICFG_DECOR_REGIONS:
	    MERGE_TEXTS(r,get_text_regions(callee_name));
	    break;
	case ICFG_DECOR_IN_REGIONS:
	    MERGE_TEXTS(r,get_text_in_regions(callee_name));
	    break;
	case ICFG_DECOR_OUT_REGIONS:
	    MERGE_TEXTS(r,get_text_out_regions(callee_name));
	    break;
	default:
	    pips_error("module_to_icfg",
		       "unknown ICFG decoration for module %s\n",
		       callee_name);
	}
	/* retrieve the caller entity */
	set_current_module_entity(e_caller);
	/* append the callee' icfg */
	append_icfg_file (r, callee_name);
	/* store it to the statement mapping */
	update_text_stmt_map (current_stmt_head(), r);

	pips_debug(9, "text %sdefined\n", r==text_undefined ? "un" : "");
    }
    return;
}

static bool loop_filter (loop l)
{
    bool print_do = get_bool_property(ICFG_DOs);
    pips_debug (5, "Loop begin\n");
    if (print_do) current_margin += ICFG_SCAN_INDENT;
    return TRUE;
}

static void loop_rewrite (loop l)
{
    text inside_the_loop = text_undefined;
    text inside_the_do = text_undefined;
    bool print_do = get_bool_property(ICFG_DOs), 
         text_in_do_p, text_in_loop_p;
    text t = make_text (NIL);
    char textbuf[MAX_LINE_LENGTH];

    pips_debug (5,"Loop end\n");

    if (print_do) current_margin -= ICFG_SCAN_INDENT;

    inside_the_do = (text) load_text_stmt_map (current_stmt_head());
    text_in_do_p = some_text_p(inside_the_do);

    inside_the_loop = (text) load_text_stmt_map (loop_body (l));
    text_in_loop_p = some_text_p(inside_the_loop);

    /* Print the DO 
     */
    if ((text_in_loop_p || text_in_do_p) && print_do) 
    {
	sprintf(textbuf, "%*sDO\n", current_margin, "");
	ADD_SENTENCE_TO_TEXT(t, make_sentence(is_sentence_formatted,
					      strdup(textbuf)));
    }

    /* Print the text inside do expressions
     */
    if (text_in_do_p) 
    {
	pips_debug(9, "something inside_the_do\n");
	MERGE_TEXTS (t, inside_the_do);

	/* a blank line. not very wellcome to xtree, but allows to 
	 * separate DO and BODY calls...
	 */
	if (print_do)
	{
	    ADD_SENTENCE_TO_TEXT(t, make_sentence(is_sentence_formatted,
						  strdup("\n"))); 
	}
    }

    /* Print the text inside the loop
     */
    if (text_in_loop_p) {
	pips_debug(9, "something inside_the_loop\n");
	MERGE_TEXTS (t, inside_the_loop);
    }

    /* Print the ENDDO 
     */
    if ((text_in_loop_p || text_in_do_p) && print_do) 
    {
	sprintf(textbuf, "%*sENDDO\n", current_margin, "");
	ADD_SENTENCE_TO_TEXT(t, make_sentence(is_sentence_formatted,
					      strdup(textbuf)));
    }

    /* store it to the statement mapping */
    update_text_stmt_map (current_stmt_head(), t);
    
    return ;
}

static void instruction_rewrite (instruction i)
{
    text t = make_text (NIL);

    pips_debug (5,"going up\n");
    pips_debug (9,"instruction tag = %d\n", instruction_tag (i));

    switch (instruction_tag (i)) {
    case is_instruction_block:
    {
	pips_debug (5,"dealing with a block, appending texts\n");

	MAPL(pm, {   
	    statement s = STATEMENT(CAR(pm));
	    MERGE_TEXTS(t, (text) load_text_stmt_map (s));
	}, instruction_block (i));

	/* store it to the statement mapping */
	update_text_stmt_map (current_stmt_head (), t);
	break;
    }
    case is_instruction_unstructured:
    {
	unstructured u = instruction_unstructured (i);
	list blocs = NIL ;
	control ct = unstructured_control(u) ;

	pips_debug (5,"dealing with an unstructured, appending texts\n");

	/* SHARING! Every statement gets a pointer to the same precondition!
	   I do not know if it's good or not but beware the bugs!!! */
	CONTROL_MAP(c, {
	    statement st = control_statement(c) ;
	    MERGE_TEXTS(t, (text) load_text_stmt_map (st));
	}, ct, blocs) ;
	
	gen_free_list(blocs) ;
	update_text_stmt_map (current_stmt_head (), t);
	break;
    }
    }
}

static bool test_filter (test l)
{
    bool print_if_p = get_bool_property (ICFG_IFs);

    pips_debug (5, "Test begin\n");
    if (print_if_p) current_margin += ICFG_SCAN_INDENT;
    return TRUE;
}

static void test_rewrite (test l)
{
    text inside_then = text_undefined;
    text inside_else = text_undefined;
    text inside_if = text_undefined;
    text t = make_text (NIL);
    char textbuf[MAX_LINE_LENGTH];
    bool something_to_print, print_if_p;
    
    pips_debug (5,"Test end\n");
    
    inside_if = copy_text((text) load_text_stmt_map (current_stmt_head ()));
    inside_then = copy_text((text) load_text_stmt_map (test_true (l)));
    inside_else = copy_text((text) load_text_stmt_map (test_false (l)));

    print_if_p = get_bool_property (ICFG_IFs);
    something_to_print = (some_text_p(inside_else) ||
			  some_text_p(inside_then) ||
			  some_text_p(inside_if));
    
    if (print_if_p) current_margin -= ICFG_SCAN_INDENT;
    
    /* Print the IF */
    if (something_to_print && print_if_p) {
	sprintf(textbuf, "%*sIF\n", current_margin, "");
	ADD_SENTENCE_TO_TEXT(t, make_sentence(is_sentence_formatted,
					      strdup(textbuf)));
    }
    
    /* print things in the if expression*/
    if (some_text_p(inside_if))
	MERGE_TEXTS (t, inside_if);

    
    /* print then statements */
    if (some_text_p(inside_then)) {
	/* Print the THEN */
	if (something_to_print && print_if_p) {
	    sprintf(textbuf, "%*sTHEN\n", current_margin, "");
	    ADD_SENTENCE_TO_TEXT(t, make_sentence(is_sentence_formatted,
						  strdup(textbuf)));
	}
	MERGE_TEXTS (t, inside_then);
    }    

    /* print then statements */
    if (some_text_p(inside_else)){
	/* Print the ELSE */
	if (something_to_print && print_if_p) {
	    sprintf(textbuf, "%*sELSE\n", current_margin, "");
	    ADD_SENTENCE_TO_TEXT(t, make_sentence(is_sentence_formatted,
						  strdup(textbuf)));
	}
	MERGE_TEXTS (t, inside_else);
    }    

    /* Print the ENDIF */
    if (something_to_print && print_if_p) {
	sprintf(textbuf, "%*sENDIF\n", current_margin, "");
	ADD_SENTENCE_TO_TEXT(t, make_sentence(is_sentence_formatted,
					      strdup(textbuf)));
    }
    
    /* store it to the statement mapping */
    update_text_stmt_map (current_stmt_head(), t);
    
    return;
}

void print_module_icfg(entity module)
{
    string module_name = module_local_name(module);
    statement s =(statement)db_get_memory_resource(DBR_CODE,module_name,TRUE);
    text txt = make_text (NIL);
    char buf[MAX_LINE_LENGTH];

    set_current_module_entity (module);

    /* allocate the mapping  */
    make_stmt_map_map();
    make_current_stmt_stack();

    sprintf(buf,"%s\n",module_name);
    ADD_SENTENCE_TO_TEXT(txt, make_sentence(is_sentence_formatted,
					    strdup(buf)));

    current_margin = ICFG_SCAN_INDENT;

    gen_multi_recurse
	(s,
	 statement_domain, statement_filter, statement_rewrite,
	 call_domain     , call_filter     , gen_null,
	 loop_domain     , loop_filter     , loop_rewrite,
	 instruction_domain    , gen_null  , instruction_rewrite,
	 test_domain     , test_filter     , test_rewrite,
	 NULL);

    pips_assert("stack is empty", current_stmt_empty_p());

    MERGE_TEXTS (txt, (text) load_text_stmt_map (s));

    make_text_resource(module_name, DBR_ICFG_FILE,
		       get_bool_property(ICFG_IFs) ? ".icfgc" :
		       (get_bool_property(ICFG_DOs) ? ".icfgl" : 
			".icfg"),
		       txt);
    
    free_text (txt);
    free_stmt_map_map();
    free_current_stmt_stack();
    reset_current_module_entity();
}

static void append_icfg_file(text t,string module_name)
{
    string filename = NULL;
    string localfilename = NULL;
    FILE *f_called;
    char buf[MAX_LINE_LENGTH];
    char textbuf[MAX_LINE_LENGTH];

    /* create filename */
    localfilename = strdup(concatenate
			   (module_name,
			    get_bool_property(ICFG_IFs) ? ".icfgc" :
			    ( get_bool_property(ICFG_DOs) ? ".icfgl" : 
			     ".icfg") ,
			    NULL));
    filename = strdup(concatenate
		      (db_get_current_workspace_directory(), 
		       "/", localfilename, NULL));

    pips_debug (2, "Inserting ICFG for module %s\n", module_name);

    /* Get the Icfg from the callee */
    f_called = safe_fopen (filename, "r");

    while (fgets (buf, MAX_LINE_LENGTH, f_called)) {
	/* add sentences ... */
	sprintf(textbuf, "%*s%s", current_margin ,"",buf);
	ADD_SENTENCE_TO_TEXT(t, make_sentence(is_sentence_formatted,
					      strdup(textbuf)));
    }
    
    /* push resulting text */
    safe_fclose (f_called, filename);
}
