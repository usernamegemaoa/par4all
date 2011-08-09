/*
  $Id$

  Copyright 1989-2011 MINES ParisTech

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
#ifdef HAVE_CONFIG_H
    #include "pips_config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "genC.h"

#include "linear.h"
#include "ri.h"
#include "database.h"

#include "ri-util.h"
#include "pipsdbm.h"
#include "properties.h"

#include "control.h" // module_reorder

#include "misc.h"
#include "resources.h"

/************************************************* DETECT REGISTER VARIABLES */

// formal parameters and local variables can be declared registers
// for registers, the cannot be referenced (&i)
// for arrays, the address cannot be used, a[i] and not a, but on sizeof(a)

typedef struct {
  set invalidated; // of entities, which cannot be declared "register"
} drv_context;

// loop indexes are ok as registers.

// detect explicit address-of operators
static bool drv_call_flt(const call c, drv_context * ctx)
{
  entity fun = call_function(c);
  if (ENTITY_ADDRESS_OF_P(fun))
  {
    list la = call_arguments(c);
    pips_assert("one argument to &", gen_length(la)==1);
    entity var = expression_to_entity(EXPRESSION(CAR(la)));
    pips_assert("variable found", var && var!=entity_undefined);
    set_add_element(ctx->invalidated, ctx->invalidated, var);
  }
  return true;
}

// detect implicit address-of for arrays
static bool drv_reference_flt(const reference r, drv_context * ctx)
{
  entity var = reference_variable(r);
  if (entity_array_p(var))
  {
    list dims =
      variable_dimensions(type_variable(ultimate_type(entity_type(var))));
    if (gen_length(dims) != gen_length(reference_indices(r)))
      set_add_element(ctx->invalidated, ctx->invalidated, var);
  }
  return true;
}

// ignore sizeof
static bool drv_sizeof_flt(
  __attribute__((unused)) const sizeofexpression s,
  __attribute__((unused)) drv_context * ctx)
{
  return false;
}

/* recurse in object to detect which variables cannot be declared register
 */
static void drv_collect(gen_chunk * object, drv_context * ctx)
{
  gen_context_multi_recurse(object, ctx,
                            call_domain, drv_call_flt, gen_null,
                            reference_domain, drv_reference_flt, gen_null,
                            sizeofexpression_domain, drv_sizeof_flt, gen_null,
                            NULL);
}

/********************************************************** CUT DECLARATIONS */

typedef struct {
  set switched;
  hash_table newdecls;
} cd_context;

static bool cut_declarations_flt(statement s, cd_context * ctx)
{
  // argh! do not strust statement_declarations necessarily
  if (!declaration_statement_p(s))
    return true;

  list decls = statement_declarations(s);
  int ndecls = gen_length(decls);
  if (ndecls>1)
  {
    // built list of declaration statements
    list lds = NIL;
    // current list of variables
    list nl = NIL;
    // whether statements in nl re switched or not
    bool nswitched = false;
    FOREACH(entity, v, decls)
    {
      pips_debug(6, "%s in statement %"_intFMT" declarations\n",
                 entity_name(v), statement_ordering(s));

      if (set_belong_p(ctx->switched, v))
      {
        if (nl) // a current list is being constructed
        {
          if (nswitched) // it already contains switched variables
            nl = CONS(entity, v, nl);
          else
          {
            pips_debug(7, "cutting on %s\n", entity_name(v));
            // cut list
            ndecls -= gen_length(nl);
            lds =
              CONS(statement,
                   make_declarations_statement
                   (gen_nreverse(nl), statement_number(s), string_undefined),
                   lds);
            // restart
            nl = CONS(entity, v, NIL), nswitched = true;
          }
        }
        else // first new list
          nl = CONS(entity, v, NIL), nswitched = true;
      }
      else // another unchaged variable
      {
        if (nl)
        {
          if (nswitched) // current list contains switched vars
          {
            pips_debug(8, "cutting S on %s\n", entity_name(v));
            // cut list
            ndecls -= gen_length(nl);
            lds =
              CONS(statement,
                   make_declarations_statement
                   (gen_nreverse(nl), statement_number(s), string_undefined),
                   lds);
            // restart
            nl = CONS(entity, v, NIL), nswitched = false;
          }
          else // non switched vars, insert in homogeneous list
            nl = CONS(entity, v, nl);
        }
        else // first new list
          nl = CONS(entity, v, NIL), nswitched = false;
      }
    }

    pips_assert("all declarations are managed", ndecls==gen_length(nl));

    // store lds for later insertions
    // I do not that now because we are still recurring in the statements...
    if (lds)
    {
      hash_put(ctx->newdecls, s, lds);
      // last list kept in current statement
      gen_free_list(statement_declarations(s));
      statement_declarations(s) = gen_nreverse(nl);
    }
    else // no change
      gen_free_list(nl);
  }
  return true;
}

// C99 6.7.1 2: variable can be at most in 1 storage class in
//               auto, extern, static, register, (typedef)
static bool variable_can_switch_to_register(entity var)
{
  if ((!get_bool_property("FORCE_REGISTER_POINTER") && entity_pointer_p(var)) ||
      (!get_bool_property("FORCE_REGISTER_ARRAY") && entity_array_p(var)) ||
      (!get_bool_property("FORCE_REGISTER_FORMAL") && entity_formal_p(var)))
    return false;
  else if (top_level_entity_p(var) || // extern
           entity_static_variable_p(var)) // static
    return false;
  else if (entity_variable_p(var)) // others
  {
    FOREACH(qualifier, q, variable_qualifiers(type_variable(entity_type(var))))
      if (qualifier_register_p(q))
        return false;

    return true;
  }
  else
    return false;
}

/* @brief modify list of qualifiers so that it includes register and *not* auto
 * @param lq initial list, which is freed on the fly
 * @return a newly allocated qualifier list
 */
static list make_it_register(list lq)
{
  list nlq = NIL;
  FOREACH(qualifier, q, lq)
  {
    if (!qualifier_register_p(q) && !qualifier_auto_p(q))
      nlq = CONS(qualifier, q, nlq);
    else
      free_qualifier(q);
  }
  gen_free_list(lq);
  return CONS(qualifier, make_qualifier_register(), gen_nreverse(nlq));
}

bool force_register_declarations(const char * module_name)
{
  statement stat = (statement)
    db_get_memory_resource(DBR_CODE, module_name, true);
  entity module = module_name_to_entity(module_name);

  set_current_module_statement(stat);
  set_current_module_entity(module);

  debug_on("PIPS_REGISTERS_DEBUG_LEVEL");

  // collect variables that cannot be registers
  drv_context ctx = { set_make(hash_pointer) };
  set switched = set_make(hash_pointer);

  // in the code
  drv_collect((void *) stat, &ctx);

  // and in variable initializations
  list vars = entity_declarations(module);

  FOREACH(entity, var, vars)
    drv_collect((void *) entity_initial(var), &ctx);

  // now switch those variables that could be
  FOREACH(entity, var, vars)
  {
    if (entity_variable_p(var) &&
        (entity_formal_p(var) || storage_ram_p(entity_storage(var))) &&
        !set_belong_p(ctx.invalidated, var) &&
        variable_can_switch_to_register(var))
    {
      pips_debug(7, "force %s as register\n", entity_name(var));
      variable v = type_variable(entity_type(var));
      variable_qualifiers(v) = make_it_register(variable_qualifiers(v));
      set_add_element(switched, switched, var);
    }
  }

  // now we have to possibly cut declaration lists as they must be homogeneous
  // think of "int i1, r1, i2;" where only r1 switched to register.
  cd_context cd = { switched, hash_table_make(hash_pointer, 0) };
  gen_context_recurse(stat, &cd,
                      statement_domain, cut_declarations_flt, gen_null);

  // perform actual insertions.
  // attempt at using insert_statement where quite unproductive.
  // I do an in-place substitution of the master statement instead.
  HASH_FOREACH(statement, s, list, l, cd.newdecls)
  {
    statement last = make_declarations_statement(statement_declarations(s),
                                                 statement_number(s),
                                                 string_undefined);
    statement_declarations(s) = NIL;
    l = CONS(statement, instruction_to_statement(statement_instruction(s)),
             CONS(statement, last, l));
    l = gen_nreverse(l);
    statement_comments(STATEMENT(CAR(l))) = statement_comments(s);
    statement_comments(s) = string_undefined;
    statement_instruction(s) = make_instruction_sequence(make_sequence(l));
  }

  // cleanup allocated stuff
  set_free(ctx.invalidated), ctx.invalidated = NULL;
  set_free(switched), switched = NULL;
  hash_table_free(cd.newdecls);

  debug_off();

  // return result to pipsdbm
  module_reorder(stat);
  DB_PUT_MEMORY_RESOURCE(DBR_CODE, module_name, stat);

  // cleanup
  reset_current_module_statement();
  reset_current_module_entity();
  return true;
}
