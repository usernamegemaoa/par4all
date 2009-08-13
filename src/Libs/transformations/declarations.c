/*

  $Id$

  Copyright 1989-2009 MINES ParisTech

  This file is part of PIPS.

  PIPS is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  PIPS is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.

  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with PIPS.  If not, see <http://www.gnu.org/licenses/>.

*/
/*
 * clean the declarations of a module.
 * to be called from pipsmake.
 *
 * its not really a transformation, because declarations
 * are associated to the entity, not to the code.
 * the code is put so as to reinforce the prettyprint...
 *
 * clean_declarations > ## MODULE.code
 *     < PROGRAM.entities
 *     < MODULE.code
 */

#include <stdio.h>
#include "genC.h"
#include "linear.h"
#include "ri.h"
#include "ri-util.h"
#include "resources.h"
#include "pipsdbm.h"
#include "misc.h"
#include "properties.h"

/**
 * recursively call statement_remove_unused_declarations on all module statement
 *
 * @param module_name name of the processed module
 * @return alaways successful
 */
bool
clean_declarations(char * module_name)
{
  /* prelude*/
  set_current_module_entity(module_name_to_entity( module_name ));
  set_current_module_statement
    ((statement) db_get_memory_resource(DBR_CODE, module_name, TRUE) );

  /* body*/
  entity_clean_declarations(get_current_module_entity(),
			    get_current_module_statement());

  gen_recurse(get_current_module_statement(),
	      statement_domain, gen_true, statement_clean_declarations);

  DB_PUT_MEMORY_RESOURCE(DBR_CODE, module_name, get_current_module_statement());

  /*postlude */
  reset_current_module_entity();
  reset_current_module_statement();
  return true;
}

/************************************* REMOVE USELESS CONTINUES IN SEQUENCES */

static void remove_continues(sequence seq)
{
  list ls = NIL;
  string_buffer comments = string_buffer_make(true);
  FOREACH(STATEMENT, s, sequence_statements(seq))
  {
    if (!continue_statement_p(s))
    {
      if (!string_buffer_empty_p(comments))
      {
	if (!statement_with_empty_comment_p(s))
	{
	  string_buffer_append(comments, statement_comments(s));
	  free(statement_comments(s));
	  statement_comments(s) = NULL;
	}
	statement_comments(s) = string_buffer_to_string(comments);
	string_buffer_reset(comments);
      }
      ls = CONS(STATEMENT, s, ls);
    }
    else // continue statement, removed
    {
      // ??? what about labels?
      // keep comments! attach them to the next available statement in
      // the sequence, otherwise goodbye!
      if (!statement_with_empty_comment_p(s))
	string_buffer_append(comments, statement_comments(s));
      free_statement(s);
    }
  }
  ls = gen_nreverse(ls);
  gen_free_list(sequence_statements(seq));
  string_buffer_free(&comments);
  sequence_statements(seq) = ls;
}

void cleanup_continues(statement stat)
{
  gen_recurse(stat, sequence_domain, gen_true, remove_continues);
}

/****************************************************** DYNAMIC DECLARATIONS */

#define call_assign_p(c)						\
  same_string_p(entity_local_name(call_function(c)), ASSIGN_OPERATOR_NAME)

struct helper {
  string module;
  string func_malloc;
  string func_free;
  set referenced;
};

// Pass 1: collect local referenced variables
static bool ignore_call_flt(call c, struct helper * ctx)
{
  // ignores free(var)
  if (same_string_p(entity_local_name(call_function(c)), ctx->func_free))
    return false;

  // ignores "var =  malloc(...)" (but not malloc arguments!)
  if (call_assign_p(c))
  {
    list args = call_arguments(c);
    pips_assert("2 args to assign", gen_length(args)==2);
    expression val = EXPRESSION(CAR(CDR(args)));

    if (expression_call_p(val) &&
	same_string_p(entity_local_name(call_function(expression_call(val))),
		      ctx->func_malloc))
    {
      pips_debug(5, "malloc called\n");
      gen_recurse_stop(EXPRESSION(CAR(args)));
    }

    // ??? should I also ignore "var = NULL" ?
  }

  return true;
}

static bool reference_flt(reference r, struct helper * ctx)
{
  set_add_element(ctx->referenced, ctx->referenced, reference_variable(r));
  return true;
}

static bool loop_flt(loop l, struct helper * ctx)
{
  set_add_element(ctx->referenced, ctx->referenced, loop_index(l));
  return true;
}

static bool unused_local_variable_p(entity var, set used, string module)
{
  return same_string_p(entity_module_name(var), module)
    && ! same_string_p(entity_local_name(var), module)
    && ! set_belong_p(used, var)
    && type_variable_p(ultimate_type(entity_type(var)));
}

// Pass 2:
static void cleanup_call(call c, struct helper * ctx)
{
  pips_debug(6, "call to %s\n", entity_name(call_function(c)));

  bool replace = false;

  if (same_string_p(entity_local_name(call_function(c)), ctx->func_free))
  {
    // get FREE-d variable
    list args = call_arguments(c);
    if (gen_length(args)==1)
    {
      expression arg = EXPRESSION(CAR(args));
      if (expression_reference_p(arg))
      {
	if (unused_local_variable_p
	    (reference_variable(expression_reference(arg)),
	     ctx->referenced, ctx->module))
	  replace = true;
      }
    }
  }
  else if (call_assign_p(c))
  {
    list args = call_arguments(c);
    expression val = EXPRESSION(CAR(CDR(args)));
    if (expression_call_p(val) &&
	same_string_p(entity_local_name(call_function(expression_call(val))),
		      ctx->func_malloc))
    {
      expression arg = EXPRESSION(CAR(args));
      if (expression_reference_p(arg))
      {
	if (unused_local_variable_p
	    (reference_variable(expression_reference(arg)),
	     ctx->referenced, ctx->module))
	  replace = true;
      }
    }
    // var = NULL?
  }

  if (replace)
  {
    pips_debug(6, "replacing...\n");
    call_function(c) = entity_intrinsic(CONTINUE_FUNCTION_NAME);
    gen_full_free_list(call_arguments(c));
    call_arguments(c) = NIL;
  }
}

static void cleanup_stat_decls(statement s, struct helper * ctx)
{
  list decls = statement_declarations(s), kept = NIL;
  FOREACH(ENTITY, var, decls)
    if (!unused_local_variable_p(var, ctx->referenced, ctx->module))
      kept = CONS(ENTITY, var, kept);
  statement_declarations(s) = gen_nreverse(kept);
  gen_free_list(decls);
}

static void dynamic_cleanup(string module, statement stat)
{
  entity mod = local_name_to_top_level_entity(module);

  struct helper help;
  help.module = module;
  help.func_malloc = get_string_property("DYNAMIC_ALLOCATION");
  help.func_free = get_string_property("DYNAMIC_DEALLOCATION");
  help.referenced = set_make(set_pointer);

  // pass 1: collect references in code
  gen_context_multi_recurse
    (stat, &help,
     reference_domain, reference_flt, gen_null,
     loop_domain, loop_flt, gen_null,
     call_domain, ignore_call_flt, gen_null,
     NULL);

  // and in initializations
  FOREACH(ENTITY, var, entity_declarations(mod))
  {
    gen_context_multi_recurse
      (entity_initial(var), &help,
       reference_domain, reference_flt, gen_null,
       // loop_domain, loop_flt, gen_null,
       call_domain, ignore_call_flt, gen_null,
       NULL);
  }

  ifdebug(4) {
    set_fprint(stderr, "referenced", help.referenced, entity_local_name);
  }

  // pass 2: cleanup calls to  "= malloc" and "free" in code
  gen_context_multi_recurse
    (stat, &help,
     call_domain, gen_true, cleanup_call,
     statement_domain, gen_true, cleanup_stat_decls,
     NULL);

  // and in declarations
  list decls = entity_declarations(mod), kept = NIL;
  pips_debug(5, "%d declarations in %s\n", (int) gen_length(decls), module);
  FOREACH(ENTITY, var, decls)
  {
    if (unused_local_variable_p(var, help.referenced, module))
    {
      pips_debug(3, "cleaning entity %s\n", entity_name(var));
      value init = entity_initial(var);
      if (value_expression_p(init))
      {
	free_value(init);
	entity_initial(var) = make_value_unknown();
      }
    }
    else
      kept = CONS(ENTITY, var, kept);
  }
  // ??? this does not seem to work:-(
  entity_declarations(mod) = gen_nreverse(kept);
  gen_free_list(decls), decls = NIL;
  pips_debug(5, "now %d declarations in %s\n",
	     gen_length(entity_declarations(mod)), module);

  set_free(help.referenced);
}

bool
clean_unused_dynamic_variables(string module)
{
  debug_on("PIPS_CUDV_DEBUG_LEVEL");

  // get stuff from pipsdbm
  set_current_module_entity(module_name_to_entity( module ));
  set_current_module_statement
    ((statement) db_get_memory_resource(DBR_CODE, module, TRUE) );

  dynamic_cleanup(module, get_current_module_statement());
  cleanup_continues(get_current_module_statement());

  // results
  DB_PUT_MEMORY_RESOURCE(DBR_CODE, module, get_current_module_statement());

  // cleanup
  reset_current_module_entity();
  reset_current_module_statement();
  debug_off();

  return true;
}
