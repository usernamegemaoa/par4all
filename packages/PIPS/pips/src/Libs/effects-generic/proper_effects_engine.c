/*

  $Id$

  Copyright 1989-2010 MINES ParisTech

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
/* package generic effects :  Be'atrice Creusillet 5/97
 *
 * File: proper_effects_engine.c
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This File contains the generic functions necessary for the computation of
 * all types of proper effects and proper references.
 *
 */
#include <stdio.h>
#include <string.h>

#include "genC.h"

#include "linear.h"
#include "ri.h"
#include "effects.h"
#include "database.h"

#include "misc.h"
#include "ri-util.h"
#include "effects-util.h"
#include "constants.h"
#include "misc.h"
#include "text-util.h"
#include "text.h"

#include "properties.h"

#include "pipsdbm.h"
#include "resources.h"

#include "pointer_values.h"
#include "effects-generic.h"

/* For debuging

static void debug_ctxt(string s, transformer t)
{
  Psysteme p;
  fprintf(stderr, "context %p at %s\n", t, s);
  if (transformer_undefined_p(t))
    fprintf(stderr, "UNDEFINED...");
  else
    {
      p = predicate_system(transformer_relation(t));
      fprintf(stderr, "%p: %d/%d\n", p, sc_nbre_egalites(p), sc_nbre_inegalites(p));
      sc_syst_debug(p);
      assert(sc_weak_consistent_p(p));
    }
}
*/

/************************************************ TO CONTRACT PROPER EFFECTS */

static bool contract_p = TRUE;

void
set_contracted_proper_effects(bool b)
{
    contract_p = b;
}


/**************************************** LOCAL STACK FOR LOOP RANGE EFFECTS */

/* Effects on loop ranges have to be added to inner statements to model
 * control dependances (see loop filter for PUSH).
 */

DEFINE_LOCAL_STACK(current_downward_cumulated_range_effects, effects)

void proper_effects_error_handler()
{
    error_reset_effects_private_current_stmt_stack();
    error_reset_effects_private_current_context_stack();
    error_reset_current_downward_cumulated_range_effects_stack();
}

static list
cumu_range_effects()
{
      list l_cumu_range = NIL;

      if(! current_downward_cumulated_range_effects_empty_p())
      {
	  l_cumu_range =
	      effects_dup(effects_effects
			  (current_downward_cumulated_range_effects_head()));
      }
      return(l_cumu_range);
}

static void
free_cumu_range_effects()
{
    if(! current_downward_cumulated_range_effects_empty_p())
	free_effects(current_downward_cumulated_range_effects_head());
}


/************************************************************** EXPRESSSIONS */

/* list generic_proper_effects_of_range(range r, context)
 * input    : a loop range (bounds and stride) and the context.
 * output   : the corresponding list of effects.
 * modifies : nothing.
 * comment  :
 */
list generic_proper_effects_of_range(range r)
{
    list le;
    expression el = range_lower(r);
    expression eu = range_upper(r);
    expression ei = range_increment(r);

    pips_debug(5, "begin\n");

    le = generic_proper_effects_of_expression(ei);
    le = gen_nconc(generic_proper_effects_of_expression(eu), le);
    le = gen_nconc(generic_proper_effects_of_expression(el), le);

    pips_debug(5, "end\n");
    return(le);
}


static list generic_r_proper_effects_of_derived_reference(effect input_eff, type input_type)
{
  basic b = variable_basic(type_variable(input_type));
  list l_dim =  variable_dimensions(type_variable(input_type));
  list le = NIL;

  pips_debug_effect(8, "input effects:\n", input_eff);

  pips_assert("input entity type basic must be derived", basic_derived_p(b));

  // we first add as many unbounded indices as there are dimensions in the type.
  for(int i=0; i< (int) gen_length(l_dim); i++)
    (*effect_add_expression_dimension_func)(input_eff, make_unbounded_expression());

  pips_debug(8, "type of basic derived : %s\n",words_to_string(words_type(entity_type(basic_derived(b)), NIL)));
  list l_fields = type_fields(entity_type(basic_derived(b)));

  FOREACH(ENTITY, f, l_fields)
    {
      type current_type = basic_concrete_type(entity_type(f));
      basic current_basic = variable_basic(type_variable(current_type));
      effect current_eff = (*effect_dup_func)(input_eff);

      // we add the field index
      effect_add_field_dimension(current_eff, f);

      switch (basic_tag(current_basic))
	{
	case is_basic_derived:
	  pips_debug(8, "derived case : recursing\n");
	  le = gen_nconc(generic_r_proper_effects_of_derived_reference(current_eff, current_type),le);
	  effect_free(current_eff);
	  break;
	case is_basic_typedef:
	  // should not happen
	  pips_internal_error("typedef case should not be possible here! ");
	  break;
	default:
	  {
	    // we have reached a leaf : we just have to add as many unbounded indices as there are dimensions in the field type.
	    list l_dim =  variable_dimensions(type_variable(current_type));
	    pips_debug(8, "default case\n");
	    for(int i=0; i< (int) gen_length(l_dim); i++)
	      (*effect_add_expression_dimension_func)(current_eff, make_unbounded_expression());

	    le = gen_nconc(CONS(EFFECT, current_eff, NIL), le);
	  }
	}
    }
  pips_debug_effects(8, "output effects:\n", le);

  return le;
}

list generic_proper_effects_of_derived_reference(reference ref, bool write_p)
{
  type t = reference_to_type(ref);
  basic b = variable_basic(type_variable(t));
  list le = NIL;

  pips_debug(8, "type of basic derived : %s\n",words_to_string(words_type(entity_type(basic_derived(b)), NIL)));

  /* should'nt it be something more direct here ? ?*/
  effect eff = (*reference_to_effect_func)
    (ref, write_p? make_action_write_memory() : make_action_read_memory(), true);

  if (type_enum_p(entity_type(basic_derived(b))))
    le = CONS(EFFECT, eff, NIL);
  else
    {
      le = generic_r_proper_effects_of_derived_reference(eff, t);
      le = gen_nreverse(le);
    }

  return le;
}

list generic_intermediary_proper_effects_of_reference(reference ref)
{
  list le = NIL;
  entity ent = reference_variable(ref);
  type t = basic_concrete_type(entity_type(ent));
  list l_ind_orig = reference_indices(ref);
  list l_ind = l_ind_orig;

  pips_debug(7, "begin \n");
  /* there should be a while here, I guess. Well work for tomorrow */

  if (type_variable_p(t))
    {
      variable v = type_variable(t);
      basic b = variable_basic(v);

      pips_debug(4, "reference %s to entity %s of basic %s and"
		 " number of dimensions %d.\n",
		 words_to_string(words_reference(ref,NIL)),
		 entity_name(ent),
		 basic_to_string(variable_basic(v)),
		 (int) gen_length(variable_dimensions(v)));

      if (basic_pointer_p(b))
	{
	  reference read_ref = make_reference(ent, NIL);
	  list l_dim_tmp = variable_dimensions(v);

	  pips_debug(8, "it's a pointer type. l_dim_tmp = %d, "
		     "l_inds_tmp = %d\n",
		     (int) gen_length(l_dim_tmp),
		     (int) gen_length(l_ind));

	  /* while there is non partially subscripted references to pointers */
	  while((basic_pointer_p(variable_basic(v)))
		&& gen_length(l_dim_tmp) < gen_length(l_ind) )
	    {
	      effect eff_read;

	      /* first we add the indices corresponding to the current
		 array dimensions if any
	      */

	      pips_debug(8, "l_dim_tmp = %d, l_inds_tmp = %d\n",
			 (int) gen_length(l_dim_tmp),
			 (int) gen_length(l_ind));

	      while (!ENDP(l_dim_tmp))
		{
		  reference_indices(read_ref)=
		    gen_nconc(reference_indices(read_ref),
			      CONS(EXPRESSION, copy_expression(EXPRESSION(CAR(l_ind))),
				   NIL));
		  POP(l_dim_tmp);
		  POP(l_ind);
		}

	      pips_debug(4, "adding a read effect on reference %s\n",
			 words_to_string(words_reference(read_ref,NIL)));

	      eff_read = (*reference_to_effect_func)(copy_reference(read_ref),
						     make_action_read_memory(),
						     false);
	      pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));
	      le = gen_nconc(le, CONS(EFFECT, eff_read, NIL));
	      pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));

	      v = type_variable(basic_pointer(variable_basic(v)));
	      l_dim_tmp = variable_dimensions(v);

	      /* if there are remaining indices, there is necessarily an index
		 to reach the pointed dimension */
	      if(!ENDP(l_ind))
		{
		  pips_debug(4, "adding an index for pointer dimension \n");
		  reference_indices(read_ref)=
		    gen_nconc(reference_indices(read_ref),
			      CONS(EXPRESSION,
				   copy_expression(EXPRESSION(CAR(l_ind))),
				   NIL));
		  POP(l_ind);
		}
	    }
	  free_reference(read_ref);
	}

    }  /* if (type_variable_p(t)) */
  gen_nreverse(le);
  le = gen_nconc(generic_proper_effects_of_expressions(l_ind), le);
  free_type(t);

  pips_debug_effects(7, "end with \n", le);
  return le;
}




/**
 @param ref a reference.
 @param *pme : a pointer on the main effect corresponding to the reference.
               if there is no effect (partially subscripted array for instance)
	       then *pme is set to effect undefined. If the main effect
               could not be computed, *pme is set to a new anywhere effect.
 @param write_p : true if the main effect is write, false otherwise.
 @param allow_read_on_pme : true if we want to allow the generation of
                read effects on reference even if it's a partially subscripted
		array. The default value should be false, but it is useful
                to set it to true when recusrsively building pme).
 @return : a list of read effects corresponding to the intermediate reads
           performed to access the main memory location.

*/
list generic_p_proper_effect_of_reference(reference ref,
					  effect *pme,
					  bool write_p,
					  bool allow_partials_on_pme)
{
  list le = NIL; /* list of read effects */

  entity ent = reference_variable(ref);
  list l_inds = reference_indices(ref);
  list l_inds_tmp = l_inds;

  type t = basic_concrete_type(entity_type(ent));

  *pme = effect_undefined;



  /* now we generate the read effects on intermediate pointer dimensions */
  /* if the entity reference is a pointer, then we scan the dimensions
     until we reach a non-pointer basic.
  */

  if (type_variable_p(t) && c_module_p(get_current_module_entity()))
    {
      variable v = type_variable(t);
      basic b = variable_basic(v);

      pips_debug(4, "reference %s to entity %s of basic %s and"
		 " number of dimensions %d.\n",
		 words_to_string(words_reference(ref,NIL)),
		 entity_name(ent),
		 basic_to_string(variable_basic(v)),
		 (int) gen_length(variable_dimensions(v)));

      if (basic_pointer_p(b))
	{
	  reference read_ref = make_reference(ent, NIL);
	  list l_dim_tmp = variable_dimensions(v);

	  pips_debug(8, "it's a pointer type. l_dim_tmp = %d, "
		     "l_inds_tmp = %d\n",
		     (int) gen_length(l_dim_tmp),
		     (int) gen_length(l_inds_tmp));

	  /* while there is non partially subscripted references to pointers */
	  while((basic_pointer_p(variable_basic(v)))
		&& gen_length(l_dim_tmp) < gen_length(l_inds_tmp) )
	    {
	      effect eff_read;

	      /* first we add the indices corresponding to the current
		 array dimensions if any
	      */

	      pips_debug(8, "l_dim_tmp = %d, l_inds_tmp = %d\n",
			 (int) gen_length(l_dim_tmp),
			 (int) gen_length(l_inds_tmp));

	      while (!ENDP(l_dim_tmp))
		{
		  reference_indices(read_ref)=
		    gen_nconc(reference_indices(read_ref),
			      CONS(EXPRESSION, copy_expression(EXPRESSION(CAR(l_inds_tmp))),
				   NIL));
		  POP(l_dim_tmp);
		  POP(l_inds_tmp);
		}

	      pips_debug(4, "adding a read effect on reference %s\n",
			 words_to_string(words_reference(read_ref,NIL)));

	      eff_read = (*reference_to_effect_func)(copy_reference(read_ref),
						     make_action_read_memory(),
						     false);
	      pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));
	      le = gen_nconc(le, CONS(EFFECT, eff_read, NIL));
	      pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));

	      v = type_variable(basic_pointer(variable_basic(v)));
	      l_dim_tmp = variable_dimensions(v);

	      /* if there are remaining indices, there is necessarily an index
		 to reach the pointed dimension */
	      if(!ENDP(l_inds_tmp))
		{
		  pips_debug(4, "adding an index for pointer dimension \n");
		  reference_indices(read_ref)=
		    gen_nconc(reference_indices(read_ref),
			      CONS(EXPRESSION,
				   copy_expression(EXPRESSION(CAR(l_inds_tmp))),
				   NIL));
		  POP(l_inds_tmp);
		}
	    }
	  free_reference(read_ref);
	}

      /* no read or write effects on partial array if
	 allow_partials_on_pme is false */
      if((basic_pointer_p(b) && gen_length(variable_dimensions(v)) <= gen_length(l_inds))||
	 (!basic_pointer_p(b) &&
	  (allow_partials_on_pme ||
	   gen_length(variable_dimensions(v)) == gen_length(l_inds))))
	{
	  *pme = (*reference_to_effect_func)
	    (ref, write_p? make_action_write_memory() : make_action_read_memory(), true);
	  pips_assert("*pme is wekly consistent", region_weakly_consistent_p(*pme));
	}
    }
  else
    {

      /* just compute the main effect on the reference

       This should maybe be refined ? */

      *pme = (*reference_to_effect_func)
	(ref, write_p? make_action_write_memory() : make_action_read_memory(), true);
      if(!effect_undefined_p(*pme))
	pips_assert("*pme is weakly consistent", region_weakly_consistent_p(*pme));
    }

  /* we must add the read effects on the indices ; these reads are performed
   before the main effect */
  pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));
  le = gen_nconc(generic_proper_effects_of_expressions
		 (reference_indices(ref)), le);
  pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));

  ifdebug(4)
    {
      if(effect_undefined_p(*pme))
	pips_debug(4, "ending no main effect "
		   "(e.g. a non-subscribed reference to an array)\n");
      else
	pips_debug_effect(4, "ending with main effect : \n", *pme);

      pips_debug_effects(4, "and intermediate read effects : \n",le);
    }
  free_type(t);

  return le;
}



/* list generic_proper_effects_of_reference(reference ref, bool written_p)
 * input    : a reference and a boolean true if it is a written reference.
 * output   : the corresponding list of effects.
 * modifies : nothing.
 * comment  : effects of a reference that is either read or written.
 */
list generic_proper_effects_of_reference(reference ref, bool written_p)
{
  list le = NIL;
  entity v = reference_variable(ref);

  pips_debug(3, "begin with reference %s\n",
	     words_to_string(words_reference(ref,NIL)));
  pips_assert("no effect on entity fields\n",!entity_field_p(v));
  transformer context;

  if (effects_private_current_context_empty_p())
    context = transformer_undefined;
  else {
    context = effects_private_current_context_head();
  }

  if (! (*empty_context_test)(context))
    {
      effect eff;
      type ref_type = reference_to_type(ref);
      if (type_variable_p(ref_type))
	{
	  variable ref_type_var = type_variable(ref_type);
	  if (basic_derived_p(variable_basic(ref_type_var)) && ENDP(variable_dimensions(ref_type_var)))
	    {
	      list lint = generic_intermediary_proper_effects_of_reference(ref);
	      le = generic_proper_effects_of_derived_reference(ref, written_p);
	      le = gen_nconc(lint, le);
	      pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));
	    }
	  else
	    {
	      le =  generic_p_proper_effect_of_reference(ref, &eff, written_p,
							 FALSE);
	      pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));

	      if (!effect_undefined_p(eff))
		{
		  le = gen_nconc(le, CONS(EFFECT, eff, NIL));
		  pips_assert("le is weakly consistent", regions_weakly_consistent_p(le));
		}
	    }
      free_type(ref_type);
	}
	else
	  pips_internal_error("case not handled yet ");
      (*effects_precondition_composition_op)(le, context);
    }

  /* Environment and declaration type effects */
  if(!get_bool_property("MEMORY_EFFECTS_ONLY")) {
    /* May not be generic enough altough contexts seem useless for
       environment effects */
    /* FI: these effects do not seem to always combine. A statement
       with a read and a write of v seems to end with two reference
       effects. See Bootstrap/iand01.f, assuming all effects are
       computed, of course. */
    effect re = make_declaration_effect(v, FALSE); // reference effect
    //type vt = entity_type(v);

    le = CONS(EFFECT, re, le);

    /*
    if(typedef_type_p(vt)) {
      entity te = basic_typedef(variable_basic(type_variable(vt)));
      effect tre = make_declaration_effect(te, FALSE); // type
						      // reference effect
      le = CONS(EFFECT, tre, le);
    }
    */
  }

  pips_debug(3, "end\n");
  return(le);
}


/* list generic_proper_effects_of_read_reference(reference ref)
 * input    : a reference that is read.
 * output   : the corresponding list of effects.
 * modifies : nothing.
 * comment  : effects of a reference that is read
 */
list generic_proper_effects_of_read_reference(reference ref)
{
  list le = NIL;

  le = generic_proper_effects_of_reference(ref, FALSE);

  return(le);
}

/* list proper_effects_of_written_reference(reference ref)
 * input    : a reference that is written.
 * output   : the corresponding list of effects.
 * modifies : nothing.
 * comment  : effects of a reference that is written
 */
list generic_proper_effects_of_written_reference(reference ref)
{
  list le = NIL;

  le = generic_proper_effects_of_reference(ref, TRUE);

  return(le);
}


/**
 @return : a list of read effects corresponding to intermediate read memory
         accesses during the evaluation of add_exp.
 @param add_exp is the expression which memory effects we are looking for
 @param pme is a Pointer towards the Main memory Effect of add_exp.
 @param write_p is a boolean set to true if the main effect is write, false
         otherwise.

 Go down along the first argument till you find a reference or a
 dereferencing and build the effect *pme by side effects as well as the
 auxiliary effect list le on the way back up.
 checks at each step that no effect is generated on partially subscripted
 arrays.
*/
list generic_proper_effects_of_complex_address_expression(expression add_exp, effect * pme, int write_p)
{
  list le = NIL;
  syntax s = expression_syntax(add_exp);
  bool finished_p = FALSE, result_computed_p = FALSE ;
  expression s_exp = expression_undefined;
  reference mr = reference_undefined;

  pips_debug(3, "begin for expression : %s\n",
	     words_to_string(words_expression(add_exp,NIL)));

  /* First step: see if we should recurse or not. Set s_exp if yes.
     If not, set mr. */

  pips_debug(4, "First, see if we should recurse\n");
  if(syntax_reference_p(s))
    {
      reference ref = syntax_reference(s);
      le = generic_p_proper_effect_of_reference(ref, pme,
						write_p, true);

      finished_p = TRUE;
      result_computed_p = TRUE;
    }
  else if(syntax_call_p(s))
    {
      call c = syntax_call(s);
      entity op = call_function(c);
      list args = call_arguments(c);
      /* FI: we assume there it at least one argument */
      pips_debug(4, "This is a call\n");

      if(gen_length(args)==0)
	{
	  /* Problem with *(1) which is syntactically legal;
	     could also happend with hardware*/
	  pips_user_warning("Constant in a lhs expression: \"\%s\"\n",
			    words_to_string(words_expression(add_exp,NIL)));
	  /* Will be converted into an anywhere effect */
	  mr = reference_undefined;
	  finished_p = TRUE;
	}
      else if(ENTITY_FIELD_P(op) || ENTITY_POINT_TO_P(op))
	{
	  pips_debug(4, "Call is a field or a point to operator\n");
	  s_exp = EXPRESSION(CAR(args));
	}
      else if(ENTITY_DEREFERENCING_P(op))
	{
	  pips_debug(4, "Call is a dereferencing operator \n");
	  s_exp = EXPRESSION(CAR(args));

	  if(expression_call_p(s_exp)) {
	    call s_c = syntax_call(expression_syntax(s_exp));
	    entity s_op = call_function(s_c);
	    list s_args = call_arguments(s_c);

	    pips_debug(4,"The dereferenced expression is a call itself (%s)\n",
		       entity_local_name(s_op));
	    ifdebug(8)
	      {
		pips_debug(8,"with arguments : \n");
		 FOREACH(EXPRESSION,x,s_args)
		   {
		     print_expression(x);
		   }
	      }

	    /* MINUS_C should be handled as well BC */
	    if(ENTITY_PLUS_C_P(s_op))
	      {
		/* case *(e1+e2) */
		/* This might be tractable if e1 is a reference to a
		   pointer. For instance, *(p+q-r) can be converted to p[q-r] */
		expression e1 = EXPRESSION(CAR(s_args));
		//syntax s1 = expression_syntax(e1);
		expression e2 = EXPRESSION(CAR(CDR(s_args)));

		pips_debug(8,"*(p+q) case, with p = %s and q = %s\n",
			   words_to_string(words_expression(e1,NIL)),
			   words_to_string(words_expression(e2,NIL)));

		le = generic_proper_effects_of_complex_address_expression
		  (e1, pme, write_p);
		if (! effect_undefined_p(*pme)&& !anywhere_effect_p(*pme))
		  {
		    syntax s2 = expression_syntax(e2);

		    /* we must add a read effect on *pme if it is a pointer
		       type
		    */
		    if (effect_pointer_type_p(*pme))
		      {
			effect eff_read;
			pips_debug(4, "It's a pointer; we add a read effect \n");

			eff_read = (*effect_dup_func)(*pme);
			effect_action(eff_read) = make_action_read_memory();
			le = gen_nconc(le, CONS(EFFECT, eff_read, NIL));
		      }

		    /* deal with case *(p+(i=exp))
		     * the effect is equivalent to an effect on *(p+exp)
		     */
		    if (syntax_call_p(s2))
		      {
			call s2_c = syntax_call(s2);
			entity s2_op = call_function(s2_c);
			list s2_args = call_arguments(s2_c);
			if (ENTITY_ASSIGN_P(s2_op))
			  {
			    e2 =  EXPRESSION(CAR(CDR(s2_args)));
			  }
		      }

		    (*effect_add_expression_dimension_func)(*pme, e2);
		  }
		le = gen_nconc(le,
			       generic_proper_effects_of_expression(e2));

		finished_p = TRUE;
		result_computed_p = true;

	      }
	    /* Other functions to process: p++, ++p, p--, --p */
	    else if(ENTITY_POST_INCREMENT_P(s_op) ||
		    ENTITY_POST_DECREMENT_P(s_op))
	      {
		// case *(e1++) or *(e1--)
		// the is a read and write effect on e1
		expression e1 = EXPRESSION(CAR(s_args));

		le = generic_proper_effects_of_complex_address_expression
		  (e1, pme, write_p);


		if (! effect_undefined_p(*pme)&& !anywhere_effect_p(*pme))
		  {
		    /* we must add a read effect on *pme if it is a pointer
		       type
		    */
		    if (effect_pointer_type_p(*pme))
		      {
			pips_debug(4, "It's a pointer; we add a read and write effect \n");

			    effect eff_read = (*effect_dup_func)(*pme);
			    effect_action(eff_read) = make_action_read_memory();
			    le = gen_nconc(le, CONS(EFFECT, eff_read, NIL));
			    effect eff_write = (*effect_dup_func)(*pme);
			    effect_action(eff_write) = make_action_write_memory();
			    le = gen_nconc(le, CONS(EFFECT, eff_write, NIL));
		      }
		  }

		/* We add a dereferencing */
		effect_add_dereferencing_dimension(* pme);
		finished_p = TRUE;
		result_computed_p = true;
	      }
	    else if(ENTITY_PRE_INCREMENT_P(s_op) ||
		    ENTITY_PRE_DECREMENT_P(s_op))
	      {
		expression e1 = EXPRESSION(CAR(s_args));
		syntax s1 = expression_syntax(e1);
		reference r1 = syntax_reference(s1);
		reference nr1 = reference_undefined;

		/* YOU DO NOT WANT TO GO DOWN RECURSIVELY. DO AS FOR C_PLUS ABOVE */

		pips_assert("The argument is a reference", syntax_reference_p(s1));
		pips_assert("The reference is scalar", ENDP(reference_indices(r1)));

		le = generic_proper_effects_of_expression(EXPRESSION(CAR(args)));
		nr1 = copy_reference(r1);
		if(ENTITY_PRE_INCREMENT_P(s_op))
		  reference_indices(nr1) = CONS(EXPRESSION, int_to_expression(1), NIL);
		else
		  reference_indices(nr1) = CONS(EXPRESSION, int_to_expression(-1), NIL);

		/* Too bad for the memory leaks involved... This s_exp
		   should be freed at exit. */
		mr = nr1;
		finished_p = TRUE;
        }  else if(ENTITY_ADDRESS_OF_P(s_op)) {
          // case *&a
          // this is an effect on a
          expression e1 = EXPRESSION(CAR(s_args));
          le = generic_proper_effects_of_complex_address_expression(e1,
                                                                    pme,
                                                                    write_p);
          finished_p = TRUE;
          result_computed_p = TRUE;
	      }
	    else
	      {
		/* do nothing, go down recursively to handle other calls */
		;
	      }
	  }
	  else if (syntax_va_arg_p(expression_syntax(s_exp)))
	    {
	      /* there could be more work here, but va_arg is very poorly
		 handled everywhere for the moment.
	      */
	      /* we generated an effect on the va_list, and that is all */
	      list vararg_list = syntax_va_arg(expression_syntax(s_exp));
	      sizeofexpression soe = SIZEOFEXPRESSION(CAR(vararg_list));

	      pips_debug(4,"The dereferenced expression is a va_arg\n");

	      le = generic_proper_effects_of_complex_address_expression(sizeofexpression_expression(soe), pme, TRUE);
	      /* and we must add an anywhere effect because we don't know where
		 the dereferenced location is.
	      */
	      le = CONS(EFFECT,
			make_anywhere_effect
			(write_p? make_action_write_memory() : make_action_read_memory()),
			le);
	      result_computed_p = TRUE;
	      finished_p = TRUE;

	    }
	  else
	    {
	      /* This is not a call, go down recursively */
	      pips_debug(4,"The dereferenced expression is not a call itself : we go down recursively\n");
	      ;
	    }
	}
      else
	{
	  /* failure: a user function is called to return a structure or an address */
	  pips_user_warning("PIPS does not know how to handle precisely this %s: \"%s\"\n",
			    write_p? "lhs":"rhs",
			    words_to_string(words_expression(add_exp,NIL)));
	  /* FI: This comes too late. down in the recursion. The effect of
	     the other sub-expressions won't be computed because we've set
	     up finish_p==TRUE and *pme == effect_undefined */
	  le = generic_proper_effects_of_expression(add_exp);

	  finished_p = TRUE;
	}
    }
  else if(syntax_cast_p(s))
    {
      pips_debug(4, "This is a cast\n");
      /* FI: The cast has an impact o pointer arithmetic. I do not know
	 how to take it into account. The cast may also use a typedef
	 type. */
      /* BC : it is a translation. The main effect should be translated
	 accordingly to the cast type on return of the recursion. */
      pips_user_warning("Cast impact on pointer arithmetic and indexing is ignored\n");
	  if(!get_bool_property("MEMORY_EFFECTS_ONLY")) {
	    type ct = cast_type(syntax_cast(s));

	    if(typedef_type_p(ct)) {
	      entity te = basic_typedef(variable_basic(type_variable(ct)));
	      effect tre = make_declaration_effect(te, FALSE); // type
	      le = gen_nconc(le, CONS(EFFECT, tre, NIL));
	    }
	  }
      s_exp = cast_expression(syntax_cast(s));
    }
  else if(syntax_subscript_p(s))
    {
      pips_debug(4,"This is a subscript\n");
      s_exp = subscript_array(syntax_subscript(s));
    }
  else if(syntax_va_arg_p(s))
    {
      pips_debug(4,"This is a va_arg\n");
      /* The built-in can return a pointer which is dereferenced */
      /* va_args is read... */
      finished_p = TRUE;
    }
  else
    {
      /* sizeofexpression, application. va_arg */
      pips_internal_error("Unexpected case");
    }

  if(finished_p)
    {

      pips_debug(3, "Stop recursing\n");

      if (! result_computed_p)
	{
	  if(reference_undefined_p(mr))
	    {
	      pips_debug(4, "mr is undefined -> anywhere effect\n");
	      *pme = make_anywhere_effect
		(write_p ? make_action_write_memory() : make_action_read_memory());
	    }
	  else
	    {
	      effect me;

	      pips_debug(4,"mr is defined\n");

	      me = (*reference_to_effect_func)
		(mr, write_p? make_action_write_memory() : make_action_read_memory(), false);
	      *pme = me;
	    }
	}
    }
  else
    {
      pips_debug(8, "We go down recursively\n");
      le = gen_nconc
	(le, generic_proper_effects_of_complex_address_expression
	 (s_exp, pme, write_p));

      pips_debug(8,"Returning from recursive call of generic_proper_effects_of_complex_address_expression : \n");

      if(!effect_undefined_p(*pme) && !anywhere_effect_p(*pme))
	{
	  /* Let's try to refine *pme with the current expression, the
	     current operator if any and the current second expression
	     when it exists */
	  pips_debug(4, "*pme is defined : try to refine it\n");

	  if(syntax_reference_p(s))
	    {
	      pips_internal_error("A reference should lead to the finished state");
	    }
	  else if(syntax_call_p(s))
	    {
	      call c = syntax_call(s);
	      entity op = call_function(c);
	      list args = call_arguments(c);

	      if(ENTITY_FIELD_P(op))
		{
		  expression e2 = EXPRESSION(CAR(CDR(args)));
		  syntax s2 = expression_syntax(e2);
		  reference r2 = syntax_reference(s2);
		  entity f = reference_variable(r2);

		  pips_assert("e2 is a reference", syntax_reference_p(s2));

		  pips_debug(4, "It's a field operator\n");

		  /* we extend *pme by adding a dimension corresponding
		  * to the field */
		  effect_add_field_dimension(*pme,f);
		  finished_p = TRUE;
		}
	      else if(ENTITY_POINT_TO_P(op))
		{
		  expression e2 = EXPRESSION(CAR(CDR(args)));
		  syntax s2 = expression_syntax(e2);
		  entity f;
		  effect eff_read;

		  pips_assert("e2 is a reference", syntax_reference_p(s2));

		  f = reference_variable(syntax_reference(s2));

		  pips_debug(4, "It's a point to operator\n");

		  /* the pointer is read */
		  pips_debug(4, "we add a read effect on the pointer. \n");

		  eff_read = (*effect_dup_func)(*pme);
		  /* memory leak */
		  effect_action(eff_read) = make_action_read_memory();
		  le = gen_nconc(le, CONS(EFFECT, eff_read, NIL));


		  /* We add a dereferencing */
		   effect_add_dereferencing_dimension(* pme);

		  /* we add the field dimension */
		  effect_add_field_dimension(*pme,f);
		  finished_p = TRUE;
		}
	      else if(ENTITY_DEREFERENCING_P(op))
		{
		  effect eff_read;
		  expression e1 = EXPRESSION(CAR(args));
		  type e1t = expression_to_type(e1);

		  pips_debug(4,"It's a dereferencing operator\n");

		  /* we add the read effect on the dereferenced expression
		     if it's a pointer
		  */
		  if (type_variable_p(e1t) &&
		      basic_pointer_p(variable_basic(type_variable(e1t))))
		    {
		      pips_debug(8, "adding read effect on argument \n");
		      eff_read = (*effect_dup_func)(*pme);
		      /* memory leak? */
		      effect_action_(eff_read) = make_action_read_memory();
		      le = gen_nconc(le, CONS(EFFECT, eff_read, NIL));
		    }

		  pips_debug(8, "adding dereferencing dimension \n");
		  effect_add_dereferencing_dimension(*pme);

		  finished_p = true;
		}
	      else
		{
		  pips_internal_error("Unexpected call to \"\%s\"", entity_name(op));
		}
	    }
	  else if(syntax_subscript_p(s))
	    {
	      subscript ss = syntax_subscript(s);
	      list ind = subscript_indices(ss);
	      type t_s_exp = expression_to_type(s_exp);

	      pips_debug(4, "It's a subscript\n");

	      /* if the array expression is a pointer, we must add a read
		 effect on it, that is to say a read effect on *pme.
	      */
	      if (pointer_type_p (t_s_exp))
		{
		  effect eff_read = (*effect_dup_func)(* pme);

		  pips_debug(5, "adding read effect on array expression\n");
		  /* memory leak? */
		  effect_action(eff_read) = make_action_read_memory();
		  le = gen_nconc(le, CONS(EFFECT, eff_read, NIL));
		}

	      /* We add the corresponding dimensions to the effect *pme
	       * and we should add read effects on pointer subscript indices
	       */
	      FOREACH(EXPRESSION, ind_exp, ind)
		{
		  (*effect_add_expression_dimension_func)(*pme, ind_exp);
		  le = gen_nconc(le, generic_proper_effects_of_expression(ind_exp));

		}
	      finished_p = true;

	    }
	  else if(syntax_cast_p(s))
	    {
	      /* FI->BC: probably a type effect to generate here if a
		 typedef type is used? */
	      /* nothing to do */
	      finished_p = true;
	    }
	  else
	    {
	      /* we should be finished already because we do not know how to
		 handle these constructs and we knew that before going down
		 and up. */
	      pips_internal_error("Something wrong in RI or missing");
	    }
	} /* end of !effect_undefined_p(*pme) */
    } /* */

  if(!finished_p && !effect_undefined_p(*pme))
    {
      /* The sub-effect could not be refined : it's an anywhere effect */
      free_effect(*pme);
      *pme = make_anywhere_effect
	(write_p?make_action_write_memory():make_action_read_memory());
    }

  pips_debug_effects(8, "End with le=\n", le);
  if(effect_undefined_p(*pme))
    pips_debug(8, "And *pme: EFFECT UNDEFINED\n");
  else
    pips_debug_effect(8, "And *pme :\n",*pme);

  return le;
}


list generic_proper_effects_of_any_lhs(expression lhs)
{
  return generic_proper_effects_of_address_expression(lhs, TRUE);
}


/**
 @return : a list of read effects corresponding to intermediate read memory
         accesses during the evaluation of add_exp.
 @param add_exp is the expression which memory effects we are looking for
 @param pme is a Pointer towards the Main memory Effect of add_exp.
 @param lpme is a pointer towars the memory effects of the expression if it is
        of type struct or union (but not an array of structs or union).
 @param write_p is a boolean set to true if the main effect is write, false
         otherwise.

 This function is an interface to generic_proper_effects_of_complex_address_expression
 that also generates effects on paths accessible from struct and union fields down
 to pointers if the expression type is a struct or union (basic derived but not enum, and
 no dimensions).
 If *lpme is not set to NIL, then the main effect has no real sense (effect on a variable name),
 and should be freed.
*/
list generic_proper_effects_of_complex_memory_access_expression(expression addexp, effect * pme, list *lpme, int write_p)
{
  list le = NIL;
  *pme = effect_undefined; /* main data read-write effect: p[*] */
  *lpme = NIL;

  pips_debug(5, "call or subscript case\n");
  /* Look for a main read-write effect of the lhs and for its
     secondary effects */
  le = generic_proper_effects_of_complex_address_expression(addexp, pme, write_p);

  if(!effect_undefined_p(*pme))
    {
      transformer context = transformer_undefined;

      if( !effects_private_current_context_empty_p())
	context = effects_private_current_context_head();

      type addexp_t = expression_to_type(addexp);

      if (type_variable_p(addexp_t))
	{
	  variable addexp_tv = type_variable(addexp_t);
	  if (ENDP(variable_dimensions(addexp_tv)))
	    {
	      basic addexp_tvb = variable_basic(addexp_tv);
	      if (basic_derived_p(addexp_tvb) && !(type_enum_p(entity_type(basic_derived(addexp_tvb)))))
		{
		  *lpme = generic_r_proper_effects_of_derived_reference(*pme, addexp_t);
		}
	    }
	  else
	    {
	      pips_debug(8, "main effect is on array name \n");

	    }
	}
      else
	{
	  pips_internal_error("case not handled yet ");
	}

      free_type(addexp_t);
      if (!transformer_undefined_p(context))
	(*effects_precondition_composition_op)(*lpme, context);

    }
  return le;

}

list generic_proper_effects_of_address_expression(expression addexp, int write_p)
{
  list le = NIL;
  syntax s = expression_syntax(addexp);


  pips_debug(5, "begin for expression : %s\n",
	     words_to_string(words_expression(addexp,NIL)));

  switch (syntax_tag(s))
    {
    case is_syntax_reference:
      {
	pips_debug(5, "reference case\n");
	if(write_p)
	  le = generic_proper_effects_of_written_reference(syntax_reference(s));
	else
	  pips_internal_error("Case not taken into account");
	break;
      }
    case is_syntax_call:
      pips_debug(5, "call case\n");
    case is_syntax_subscript:
      {
	effect e = effect_undefined; /* main data read-write effect: p[*] */

	pips_debug(5, "call or subscript case\n");
	/* Look for a main read-write effect of the lhs and for its
	   secondary effects */
	le = generic_proper_effects_of_complex_address_expression(addexp, &e, write_p);

	if(!effect_undefined_p(e))
	  {
	    transformer context = transformer_undefined;

	    if( !effects_private_current_context_empty_p())
	      context = effects_private_current_context_head();

	    type addexp_t = expression_to_type(addexp);

	    /* we add the read effect if it's not an array name
	    */
	    if (type_variable_p(addexp_t))
	      {
		variable addexp_tv = type_variable(addexp_t);
		if (ENDP(variable_dimensions(addexp_tv)))
		  {
		    basic addexp_tvb = variable_basic(addexp_tv);
		    pips_debug(8, "adding main read effect \n");
		    if (basic_derived_p(addexp_tvb) && !(type_enum_p(entity_type(basic_derived(addexp_tvb)))))
		      {
			list l_tmp = generic_r_proper_effects_of_derived_reference(e, addexp_t);
			le = gen_nconc(le, l_tmp);
		      }
		      else
			le = CONS(EFFECT, e, le);
		  }
		else
		  {
		    pips_debug(8, "main read effect is on array name : discarded\n");
		    free_effect(e);
		  }
	      }
	    else
	      {
		pips_internal_error("case not handled yet ");
	      }

	    free_type(addexp_t);
	    if (!transformer_undefined_p(context))
	      (*effects_precondition_composition_op)(le, context);

	  }

	ifdebug(8) {
	  pips_debug(8, "Effect for a call or a subscripted expression:\n");
	  (*effects_prettyprint_func)(le);
	}
	break;
      }
    case is_syntax_cast:
      {
	pips_user_error("use of cast expressions as lvalues is deprecated\n");
	break;
      }
    case is_syntax_sizeofexpression:
      {
	pips_user_error("sizeof cannot be a lhs\n");
	break;
      }
    case is_syntax_application:
      {
	/* I assume this not any more possible than a standard call */
	pips_user_error
	  ("use of indirect function call as lhs is not allowed\n");
	break;
      }
    default:
      pips_internal_error
	("lhs is not a reference and is not handled yet: syntax tag=%d\n",
	 syntax_tag(s));

    } /* end switch */


  ifdebug(8) {
    pips_debug(8, "End with le=\n");
    (*effects_prettyprint_func)(le);
    fprintf(stderr, "\n");
  }

  return le;
}


/* TO VERIFY !!!!!!!!!!!!!*/
/* UNUSED ? */
list
generic_proper_effects_of_subscript(subscript s)
{
    list inds = subscript_indices(s);
    list le = NIL;
    transformer context;

    if (effects_private_current_context_empty_p())
	context = transformer_undefined;
    else
      {
	context = effects_private_current_context_head();
      }


    pips_debug(3, "begin\n");

    if (! (*empty_context_test)(context))
    {
      le = generic_proper_effects_of_expression(subscript_array(s));

      if (! ENDP(inds))
	le = gen_nconc(le, generic_proper_effects_of_expressions(inds));


	(*effects_precondition_composition_op)(le, context);
    }

    pips_debug(3, "end\n");
    return(le);
}

list generic_proper_effects_of_application(application a __attribute__((__unused__)))
{
  list le = NIL;

  pips_user_warning("Effect of indirect calls not implemented -> returning anywhere\n");

  le = make_anywhere_read_write_memory_effects();
  return(le);
}



/* Compute the proper effects of an expression

   @param[in] e is the expression we want the effects

   @return the corresponding list of effects.

   It calls store_expr_prw_effects() to keep track of expression effects
   if needed
*/
list
generic_proper_effects_of_expression(expression e)
{
  list le = NIL;
  syntax s;

  pips_debug(3, "begin\n");

  s = expression_syntax(e);

  switch(syntax_tag(s))
    {
    case is_syntax_reference:
      le = generic_proper_effects_of_read_reference(syntax_reference(s));
      break;
    case is_syntax_range:
      le = generic_proper_effects_of_range(syntax_range(s));
      break;
    case is_syntax_call:
      {
	entity op = call_function(syntax_call(s));

	/* first the case of an adressing operator : this could also be done
	 * by calling generic_r_proper_effects_of_call, but then the expression
	 * is lost and must be rebuild later to call
	 * g_p_e_of_address_expression.
	 */
	if (ENTITY_FIELD_P(op) ||
	    ENTITY_POINT_TO_P(op) ||
	    ENTITY_DEREFERENCING_P(op))
	  le = generic_proper_effects_of_address_expression(e, FALSE);
	else
	  le = generic_r_proper_effects_of_call(syntax_call(s));
	break;
      }
    case is_syntax_cast:
      {
	le = generic_proper_effects_of_expression(cast_expression(syntax_cast(s)));
	if(!get_bool_property("MEMORY_EFFECTS_ONLY")) {
	  type ct = cast_type(syntax_cast(s));

	  if(typedef_type_p(ct)) {
	    entity te = basic_typedef(variable_basic(type_variable(ct)));
	    effect tre = make_declaration_effect(te, FALSE); // type
	    le = gen_nconc(le, CONS(EFFECT, tre, NIL));
	  }
	}

	break;
      }
    case is_syntax_sizeofexpression:
      {
	sizeofexpression se = syntax_sizeofexpression(s);
	if (sizeofexpression_expression_p(se))
	  {
	    /* FI: If the type of the reference is a dependent type, this
	       may imply the reading of some expressions... See for
	       instance type_supporting_entities()? Is sizeof(a[i]) ok? */
	    /* The type of the variable is read, not the variable itself.*/
	    /* le = generic_proper_effects_of_expression(sizeofexpression_expression(se)); */
	  ;
	  }
	else {
	  if(!get_bool_property("MEMORY_EFFECTS_ONLY")) {
	    type sot = sizeofexpression_type(se);

	    if(typedef_type_p(sot)) {
	      entity te = basic_typedef(variable_basic(type_variable(sot)));
	      effect tre = make_declaration_effect(te, FALSE); // type
	      le = gen_nconc(le, CONS(EFFECT, tre, NIL));
	    }
	  }
	}
	break;
      }
    case is_syntax_subscript:
      {
	le = generic_proper_effects_of_address_expression(e, FALSE);
	break;
      }
    case is_syntax_application:
      le = generic_proper_effects_of_application(syntax_application(s));
      break;
    case is_syntax_va_arg:
      {
	/* there is first a read of the first argument, and
	   subsequent write effects on the va_list depths are simulated
	   by write effects on the va_list itself.
	*/
	list al = syntax_va_arg(s);
	expression ae = sizeofexpression_expression(SIZEOFEXPRESSION(CAR(al)));
	le = generic_proper_effects_of_expression(ae);
	le = gen_nconc(le, generic_proper_effects_of_any_lhs(ae));
	break;
      }
    default:
      pips_internal_error("unexpected tag %d", syntax_tag(s));
    }

  ifdebug(8)
    {
	pips_debug(8, "Proper effects of expression \"%s\":\n",
		   words_to_string(words_syntax(s,NIL)));
	(*effects_prettyprint_func)(le);
    }

  /* keep track of proper effects associated to sub-expressions if required.
   */
  if (!expr_prw_effects_undefined_p())
    {
      /* in IO lists, the effects are computed twice,
     * once as LHS, once as a REFERENCE...
     * so something may already be in. Let's skip it.
     * I should investigate further maybe. FC.
     */
      if (!bound_expr_prw_effects_p(e))
	store_expr_prw_effects(e, make_effects(gen_full_copy_list(le)));
    }

  return le;
}

/* list generic_proper_effects_of_expressions(list exprs)
 * input    : a list of expressions and the current context.
 * outpproper_ut   : the correpsonding list of effects.
 * modifies : nothing.
 * comment  :
 */
list
generic_proper_effects_of_expressions(list exprs)
{
    list le = NIL;

    pips_debug(5, "begin\n");

    MAP(EXPRESSION, exp,
	/* le may be long... */
	le = gen_nconc(generic_proper_effects_of_expression(exp), le),
	exprs);

    pips_debug(5, "end\n");
    return(le);
}

bool check_sdfi_effects_p(entity func, list func_sdfi)
{
  list ce = list_undefined;
  bool check_p = TRUE;
  type ut = ultimate_type(entity_type(func));

  pips_assert("func is a function", type_functional_p(ut));

  /* Check the SDFI effects */
  for(ce = func_sdfi; !ENDP(ce); POP(ce)) {
    effect eff = EFFECT(CAR(ce));
    reference r = effect_any_reference(eff);
    entity v = reference_variable(r);

    if(formal_parameter_p(v)) {
      storage s = entity_storage(v);
      formal fs = storage_formal(s);
      int rank = formal_offset(fs);
      entity called_function = formal_function(fs);

      if(called_function!=func) {
	fprintf(stderr, "Summary effect %p for function \"%s\" refers to "
		"formal parameter \"%s\" of function \"%s\"\n",
		eff, entity_name(func), entity_name(v), entity_name(called_function));
	check_p = FALSE;
      }

      if(rank> (int) gen_length(functional_parameters(type_functional(ut)))) {
	fprintf(stderr, "Formal parameter \"%s\" is ranked %d out of %zd!\n",
		entity_name(v), rank, gen_length(functional_parameters(type_functional(ut))));
	check_p = FALSE;
      }
    }
  }
  return check_p;
}

static list
generic_proper_effects_of_external(entity func, list args)
{
    list le = NIL;
    char *func_name = module_local_name(func);

    pips_debug(4, "translating effects for %s\n", func_name);

    if (! entity_module_p(func))
    {
	pips_internal_error("%s: bad function", func_name);
    }
    else
    {
	list func_eff;
	transformer context;

        /* Get the summary effects of "func". */
	func_eff = (*db_get_summary_rw_effects_func)(func_name);

	if(!check_sdfi_effects_p(func, func_eff))
	  pips_internal_error("SDFI effects for \"%s\" are corrupted in the data base",
			      entity_name(func));

	/* Translate them using context information. */
	context = effects_private_current_context_head();
	le = generic_effects_backward_translation(func, args, func_eff, context);

	if(!check_sdfi_effects_p(func, func_eff))
	  pips_internal_error("SDFI effects for \"%s\" have been corrupted by the translation",
			      entity_name(func));
    }
    ifdebug(1) pips_assert("All effects in \"le\" are consistent",
			     effect_list_consistent_p(le));
    return le;
}

list generic_proper_effects_of_c_function_call_argument(expression arg)
{
  list le = NIL;
  type arg_t = type_undefined;
  pips_debug(3, "begin for actual argument: %s\n", words_to_string(words_expression(arg, NIL)));

  arg_t = expression_to_type(arg);

  /* If it's a sub-array, there are only intermediate effects */
  if(type_variable_p(arg_t) && !ENDP(variable_dimensions(type_variable(arg_t))))
    {
      effect eff = effect_undefined;
      /* I'm not sure it is OK for all type of arguments, in particular function calls */
      le = generic_proper_effects_of_complex_address_expression(arg, &eff, false);
      if (!effect_undefined_p(eff))
	free_effect(eff);
    }
  else
    {
      le = generic_proper_effects_of_expression(arg);
    }


  pips_debug_effects(3, "ouput effects: \n", le);

  return le;
}


/**

 * @return the list of effects found.
 * @param c, a call, which can be a call to a subroutine, but also
 * to an function, or to an intrinsic, or even an assignement.
 */
list
generic_r_proper_effects_of_call(call c)
{
  list le = NIL;
  entity e = call_function(c);
  tag t = value_tag(entity_initial(e));
  string n = module_local_name(e);
  list pc = call_arguments(c);
  type uet = ultimate_type(entity_type(e));

  pips_debug(2, "begin for %s\n", entity_local_name(e));

  if(type_functional_p(uet)) {
    switch (t) {
    case is_value_code:
      pips_debug(5, "external function %s\n", n);
      le = generic_proper_effects_of_external(e, pc);
      break;

    case is_value_intrinsic:
      pips_debug(5, "intrinsic function %s\n", n);
      le = generic_proper_effects_of_intrinsic(e, pc);
      break;

    case is_value_symbolic:
      pips_debug(5, "symbolic\n");
      break;

    case is_value_constant:
      pips_debug(5, "constant\n");
      break;

    case is_value_unknown:
      if (get_bool_property("HPFC_FILTER_CALLEES"))
	/* hpfc specials are managed here... */
	le = NIL;
      else
	pips_internal_error("unknown function %s", entity_name(e));
      break;

    default:
      pips_internal_error("unknown tag %d", t);
      break;
    }
  }
  else if(type_variable_p(uet)) {
    /* We could be less optimistic even when no information about the function called is known.
     *
     * We could look up all functions with the same type and make the union of their effects.
     *
     * We could assume that all parameters are read.
     *
     * We could assume that all pointers are used to produce indirect write.
     */
    pips_user_warning("Effects of call thru functional pointers are ignored\n");
  }
  else if(type_statement_p(uet)) {
    le = NIL;
  }
  else {
    pips_internal_error("Unexpected case");
  }

  pips_debug(2, "end\n");

  return(le);
}


/**************************************************************** STATEMENTS */

static void
proper_effects_of_call(call c)
{
    list l_proper = NIL;
    statement current_stat = effects_private_current_stmt_head();
    instruction inst = statement_instruction(current_stat);
    list l_cumu_range = cumu_range_effects();

    /* Is the call an instruction, or a sub-expression? */
    if (instruction_call_p(inst) && (instruction_call(inst) == c))
    {
      pips_debug(2, "Effects for statement %03zd:\n",
		   statement_ordering(current_stat));

	l_proper = generic_r_proper_effects_of_call(c);

	l_proper = gen_nconc(l_proper, effects_dup(l_cumu_range));

	if (contract_p)
	    l_proper = proper_effects_contract(l_proper);

	ifdebug(2) {
	  pips_debug(2, "Proper effects for statement %03zd:\n",
		     statement_ordering(current_stat));
	  (*effects_prettyprint_func)(l_proper);
	  pips_debug(2, "end\n");
	}

	/* This change is not compatible with previous Fortran
	   oriented phases such as hpfc. We can either forbid this
	   feature for all Fortran codes, or add a new property to
	   control this effect elimination, specific to C but with an
	   impact on CONTINUE, for instance, in Fortran. */
	if(!ENDP(l_proper)
	   && effects_all_read_p(l_proper)
	   && !statement_may_have_control_effects_p(current_stat)
	   && !format_statement_p(current_stat)
	   && !fortran_module_p(get_current_module_entity())) {
	  /* The current statement should be ignored as it does not
	     impact the store, nor the control, nor the
	     formatting. Examples in C; "0;" or "i;" or "(void)
	     i". Because PIPS is interprocedural, it could ignore some
	     more statements than gcc, but control effects are not
	     analyzed. Such statements can be created by program
	     transformations. */
	  pips_user_warning("Statement %d is ignored because it does not "
			    "modify the store.\n", statement_number(current_stat));
	  gen_full_free_list(l_proper);
	  l_proper = NIL;
	}

	if (get_constant_paths_p())
	  {
	    list l_tmp = l_proper;
	    l_proper = pointer_effects_to_constant_path_effects(l_proper);
	    effects_free(l_tmp);
	  }

	store_proper_rw_effects_list(current_stat, l_proper);
    }
}

/* just to handle one kind of instruction, expressions which are not calls */
static void proper_effects_of_expression_instruction(instruction i)
{
  list l_proper = NIL;
  statement current_stat = effects_private_current_stmt_head();
  //instruction inst = statement_instruction(current_stat);
  list l_cumu_range = cumu_range_effects();

  /* Is the call an instruction, or a sub-expression? */
  if (instruction_expression_p(i)) {
    expression ie = instruction_expression(i);
    syntax is = expression_syntax(ie);
    call c = call_undefined;

    switch (syntax_tag(is))
    {
    case is_syntax_cast :
      {
	expression ce = cast_expression(syntax_cast(is));
	syntax sc = expression_syntax(ce);

	if(syntax_call_p(sc)) {
	  c = syntax_call(sc);
	  l_proper = generic_r_proper_effects_of_call(c);
	}
	else if(syntax_reference_p(sc)) {
	  /* FI: I guess you do not end up here if the cast appears in
	     the lhs, assuming this is till compatible with the
	     standard. */
	  reference r = syntax_reference(sc);
	  l_proper = generic_proper_effects_of_read_reference(r);
	}
	else {
	  pips_internal_error("Cast case not implemented");
	}
	break;
      }
    case is_syntax_call :
      {
	/* This may happen when a loop is unstructured by the controlizer */
	c = syntax_call(is);
	l_proper = generic_r_proper_effects_of_call(c);
	break;
      }
    case is_syntax_application :
      {
	  /* This may happen when a structure field contains a pointer to
	     a function. We do not know which function is is... */
	  //application a = syntax_application(is);
	  //expression fe = application_function(a);

	  /* we should try here to retrieve the name of the function through pointer analysis */
	  /* NOT IMPLEMENTED -> returning anywhere read and write, as the function
	     can read and write any global variable as well as it's pointer arguments targets.
           */

	  l_proper = make_anywhere_read_write_memory_effects();

	  /* More effects should be added to take the call site into account */
	  /* Same as for pointer-based call: use type, assume worst case,... */
	  /* A new function is needed to retrieve all functions with a
	     given signature. Then the effects of all the candidates must
	     be unioned. */
	  pips_user_warning("call through a function pointer in a structure -> anywhere effects\n");

	  break;
      }
    case is_syntax_reference:
      {
	// someone typed "i;" in the code... or "a[i++];". It is
	//allowed and may happen in automatic code transformations such as
	//inlining.
	reference r = syntax_reference(is);
	l_proper = generic_proper_effects_of_read_reference(r);
	break;
      }
    case is_syntax_range:
    case is_syntax_sizeofexpression:
    case is_syntax_subscript:
    default :
      pips_internal_error("Instruction expression case %d not implemented",
			  syntax_tag(is));
    }

    pips_debug(2, "Effects for expression instruction in statement%03zd:\n",
	       statement_ordering(current_stat));

    l_proper = gen_nconc(l_proper, effects_dup(l_cumu_range));

    if (contract_p)
      l_proper = proper_effects_contract(l_proper);
    ifdebug(2) {
      pips_debug(2, "Proper effects for statement%03zd:\n",
		 statement_ordering(current_stat));
      (*effects_prettyprint_func)(l_proper);
      pips_debug(2, "end\n");
    }

    if(!ENDP(l_proper) && effects_all_read_p(l_proper)) {
      /* The current statement should be ignored as it does not impact
	 the store. Examples in C; "0;" or "i;" or "(void) i". Because
	 PIPS is interprocedural, it may ignore some more statements
	 than gcc. Such statements can be created by program
	 transformations. */
      pips_user_warning("Statement %d is ignored because it does not "
			"modify the store.\n", statement_number(current_stat));
      gen_full_free_list(l_proper);
      l_proper = NIL;
    }
    if (get_constant_paths_p())
      {
	list l_tmp = l_proper;
	l_proper = pointer_effects_to_constant_path_effects(l_proper);
	effects_free(l_tmp);
      }
    store_proper_rw_effects_list(current_stat, l_proper);
  }
}

static void
proper_effects_of_unstructured(unstructured u __attribute__((__unused__)))
{
    statement current_stat = effects_private_current_stmt_head();
    store_proper_rw_effects_list(current_stat,NIL);
}

static bool
loop_filter(loop l)
{
    list l_proper = generic_proper_effects_of_range(loop_range(l));
    list l_eff = cumu_range_effects();

    l_eff = gen_nconc(l_proper, l_eff);
    current_downward_cumulated_range_effects_push(make_effects(l_eff));
    return(TRUE);
}

static void proper_effects_of_loop(loop l)
{
    statement current_stat = effects_private_current_stmt_head();
    list l_proper = NIL;
    list l_cumu_range = NIL;

    entity i = loop_index(l);
    range r = loop_range(l);

    list li = NIL, lb = NIL;

    pips_debug(2, "Effects for statement%03zd:\n",
	       statement_ordering(current_stat));

    free_cumu_range_effects();
    current_downward_cumulated_range_effects_pop();
    l_cumu_range = cumu_range_effects();

    /* proper_effects first */

    /* Effects of loop on loop index.
     * loop index is must-written but may-read because the loop might
     * execute no iterations.
     */
    /* FI, RK: the may-read effect on the index variable is masked by
     * the initial unconditional write on it (see standard page 11-7, 11.10.3);
     * if masking is not performed, the read may prevent privatization
     * somewhere else in the module (12 March 1993)
     */
    /* Parallel case
     *
     * as I need the same effects on a parallel loop to remove
     * unused private variable in rice/codegen.c, I put the
     * same code to compute parallel loop proper effects.
     * this may not be correct, but I should be the only one to use
     * such a feature. FC, 23/09/93
     */
    li = generic_proper_effects_of_written_reference(make_reference(i, NIL));

    /* effects of loop bound expressions. */
    lb = generic_proper_effects_of_range(r);

    l_proper = gen_nconc(li, lb);
    l_proper = gen_nconc(l_proper, effects_dup(l_cumu_range));

    ifdebug(2)
    {
	pips_debug(2, "Proper effects for statement%03zd:\n",
		   statement_ordering(current_stat));
	(*effects_prettyprint_func)(l_proper);
	pips_debug(2, "end\n");
    }

    ifdebug(1) pips_assert("l_proper is consistent",
			   effect_list_consistent_p(l_proper));

    if (contract_p)
	l_proper = proper_effects_contract(l_proper);

    ifdebug(1) pips_assert("l_proper is consistent",
			   effect_list_consistent_p(l_proper));

    if (get_constant_paths_p())
      {
	list l_tmp = l_proper;
	l_proper = pointer_effects_to_constant_path_effects(l_proper);
	effects_free(l_tmp);
      }
    store_proper_rw_effects_list(current_stat, l_proper);
}

static void proper_effects_of_forloop(forloop l)
{
    statement current_stat = effects_private_current_stmt_head();
    list l_proper = NIL;
    list l_cumu_range = NIL;

    //    entity i = loop_index(l);
    // range r = loop_range(l);

    list li = NIL, lc = NIL, linc = NIL;

    pips_debug(2, "Effects for statement%03zd:\n",
	       statement_ordering(current_stat));

    // What is this about? See Fabien...
    // free_cumu_range_effects();
    //current_downward_cumulated_range_effects_pop();
    //l_cumu_range = cumu_range_effects();

    /* proper_effects first */

    li = generic_proper_effects_of_expression(forloop_initialization(l));

    /* effects of condition expression */
    lc = generic_proper_effects_of_expression(forloop_condition(l));
    /* effects of incrementation expression  */
    /* we do not know if the incrementation expression will be evaluated -> may effects (see ticket 446) */
    linc = generic_proper_effects_of_expression(forloop_increment(l));
    effects_to_may_effects(linc);

    l_proper = gen_nconc(li, lc);
    l_proper = gen_nconc(l_proper, linc);
    l_proper = gen_nconc(l_proper, effects_dup(l_cumu_range));

    ifdebug(2)
    {
	pips_debug(2, "Proper effects for statement%03zd:\n",
		   statement_ordering(current_stat));
	(*effects_prettyprint_func)(l_proper);
	pips_debug(2, "end\n");
    }

    if (contract_p)
	l_proper = proper_effects_contract(l_proper);

    if (get_constant_paths_p())
      {
	list l_tmp = l_proper;
	l_proper = pointer_effects_to_constant_path_effects(l_proper);
	effects_free(l_tmp);
      }
    store_proper_rw_effects_list(current_stat, l_proper);
}

static void proper_effects_of_while(whileloop w)
{
    statement current_stat = effects_private_current_stmt_head();
    list /* of effect */ l_proper =
	generic_proper_effects_of_expression(whileloop_condition(w));
    if (get_constant_paths_p())
      {
	list l_tmp = l_proper;
	l_proper = pointer_effects_to_constant_path_effects(l_proper);
	effects_free(l_tmp);
      }

    store_proper_rw_effects_list(current_stat, l_proper);
}

static void proper_effects_of_test(test t)
{
    list l_proper=NIL;
    statement current_stat = effects_private_current_stmt_head();
    list l_cumu_range = cumu_range_effects();

    pips_debug(2, "Effects for statement%03zd:\n",
	       statement_ordering(current_stat));

    /* effects of the condition */
    l_proper = generic_proper_effects_of_expression(test_condition(t));
    l_proper = gen_nconc(l_proper, effects_dup(l_cumu_range));

    ifdebug(2)
    {
	pips_debug(2, "Proper effects for statement%03zd:\n",
		   statement_ordering(current_stat));
	(*effects_prettyprint_func)(l_proper);
	pips_debug(2, "end\n");
    }

    if (contract_p)
	l_proper = proper_effects_contract(l_proper);
    if (get_constant_paths_p())
      {
	list l_tmp = l_proper;
	l_proper = pointer_effects_to_constant_path_effects(l_proper);
	effects_free(l_tmp);
      }

    store_proper_rw_effects_list(current_stat, l_proper);
}

static void proper_effects_of_sequence(sequence block __attribute__((__unused__)))
{
    statement current_stat = effects_private_current_stmt_head();
    store_proper_rw_effects_list(current_stat, NIL);
}

static bool stmt_filter(statement s)
{
  pips_debug(1, "Entering statement with ordering: %03zd and number: %03zd\n",
	     statement_ordering(s), statement_number(s));
  effects_private_current_stmt_push(s);
  effects_private_current_context_push((*load_context_func)(s));
  return(TRUE);
}

/**
 @param entity is a
 @param
 @return : a list of effects corresponding to the effects of the rhs if there
           is an initialization in the declaration., plus the effects of the lhs
*/
static list generic_proper_effects_of_declaration(entity decl)
{
  list l_eff = NIL;
  pips_debug(1, "declaration of entity %s \n", entity_local_name(decl));

  if(type_variable_p(entity_type(decl)))
    {
      value v_init = entity_initial(decl);

      pips_debug(1, "begin\n");
      /* generate effects due to the initialisation */
      if (value_expression_p(v_init))
	{
	  expression exp_init = value_expression(v_init);
	  l_eff = generic_proper_effects_of_expression(exp_init);
	}

      /* if there is an initial value, and if the variable is not a static one,
	 then there is a write on the entity (well on  the reference constituted
	 by the entity name with no indices !).
	 There may be a memory leak here because we do not want a preference in
	 the effect. However, I do not have a good solution for the time being
	 because in declarations, the left hand side is not a reference. BC.
	 I should may be call generic_proper_effects_of lhs instead, but the case is
	 slightly different for arrays. Or directly (*reference_to_effect_func) in case of a scalar ?
      */
      /* To avoid problems with loop distribution, we need a (write)
	 effect whether there is an initialization or not. This is
	 consistent with transformers and preconditions, but not with
	 the used before set analysis. In the short term, you can
	 comment out the second part of the condition to solve the
	 loop distribution issue. */
      if (!variable_static_p(decl) && !value_unknown_p(v_init))
	{
	  type decl_t = basic_concrete_type(entity_type(decl));
	  list l_tmp = NIL;

	  if (!ENDP(variable_dimensions(type_variable(decl_t))))
	    {
	      effect decl_eff = (*reference_to_effect_func)(make_reference(decl,NIL), make_action_write_memory(), true);
	      l_tmp =  generic_effect_generate_all_accessible_paths_effects(decl_eff, decl_t, is_action_write);
	    }
	  else
	    {
	      // make sure the make_reference does not create a memory leak
	      l_tmp = generic_proper_effects_of_reference(make_reference(decl,
									 NIL),
							  true);
	    }
	  l_eff= gen_nconc(l_eff, l_tmp);

	}
      pips_debug_effects(1, "ending with:", l_eff);
    }
  return l_eff;
}

effect make_declaration_effect(entity e, bool written_p)
{
  effect eff = effect_undefined;
  /* FI: generate a declaration or a type write */

  reference r = make_reference(e, NIL);
  action a = action_undefined;
  action_kind ak = action_kind_undefined;
  /* FI: I'm not sure this is a very generic decision; I do not
     foresee how predicates could refine dependences. But we already
     missed that for scalar variables:-( */
  descriptor d = make_descriptor_none();
  cell c = make_cell_reference(r);
  approximation ap = make_approximation_exact();

  if(typedef_entity_p(e)) {
    /* FI: more work here; you have to check that other typedefed
       types or variables are not used in this typedef. Quite a
       recursive search... */
    ak = make_action_kind_type_declaration();
  }
  else
    ak = make_action_kind_environment();

  if(written_p)
    a = make_action_write(ak);
  else
    a = make_action_read(ak);

  eff = make_effect(c, a, ap, d);

  return eff;
}

static void proper_effects_of_statement(statement s)
{
  /* Handling of declarations attached to a declaration statement

  */
  pips_debug(1, "statement%03zd :\n", statement_number(s));
  if (c_module_p(get_current_module_entity()) &&
      (declaration_statement_p(s) /*|| block_statement_p(s)*/ ))
    {
      list l_eff = NIL;
      list l_decls = statement_declarations(s);

      pips_debug(1, "declaration statement \n");

      FOREACH(ENTITY, e, l_decls)
	{
	  if(!get_bool_property("MEMORY_EFFECTS_ONLY")) {
	    effect de = make_declaration_effect(e, TRUE);
	    type vt = entity_type(e);

	    l_eff = gen_nconc(l_eff, CONS(EFFECT, de, NIL));
	    // Should be put first or last?
	    //l_eff = CONS(EFFECT, re, l_eff);

	    if(typedef_type_p(vt)) {
	      entity te = basic_typedef(variable_basic(type_variable(vt)));
	      effect tre = make_declaration_effect(te, FALSE); // type
	      // reference effect
	      l_eff = gen_nconc(l_eff, CONS(EFFECT, tre, NIL));
	      //l_eff = CONS(EFFECT, tre, l_eff);
	    }
	  }
	  l_eff = gen_nconc(l_eff,generic_proper_effects_of_declaration(e));
	}
      if (get_constant_paths_p())
	{
	  list l_tmp = l_eff;
	  l_eff = pointer_effects_to_constant_path_effects(l_eff);
	  effects_free(l_tmp);
	}

      if(bound_proper_rw_effects_p(s))
	{
	  l_eff = gen_nconc(l_eff,
			    load_proper_rw_effects_list(s));
	  update_proper_rw_effects_list(s, l_eff);
	}
      else
	store_proper_rw_effects_list(s,l_eff);
    }

  if (!bound_proper_rw_effects_p(s))
    {
      pips_debug(2, "Warning, proper effects undefined, set to NIL\n");
      store_proper_rw_effects_list(s,NIL);
    }
  effects_private_current_stmt_pop();
  effects_private_current_context_pop();

  pips_debug(1, "End statement%03zd :\n", statement_number(s));

}

void proper_effects_of_module_statement(statement module_stat)
{
    make_effects_private_current_stmt_stack();
    make_effects_private_current_context_stack();
    make_current_downward_cumulated_range_effects_stack();
    pips_debug(1,"begin\n");

    gen_multi_recurse
	(module_stat,
	 statement_domain, stmt_filter, proper_effects_of_statement,
	 sequence_domain, gen_true, proper_effects_of_sequence,
	 test_domain, gen_true, proper_effects_of_test,
	 /* Reached only through syntax (see expression rule) */
	 call_domain, gen_true, proper_effects_of_call,
	 loop_domain, loop_filter, proper_effects_of_loop,
	 whileloop_domain, gen_true, proper_effects_of_while,
	 forloop_domain, gen_true, proper_effects_of_forloop,
	 unstructured_domain, gen_true, proper_effects_of_unstructured,
         /* Just to retrieve effects of instructions with kind
	    expression since they are ruled out by the next clause */
	 instruction_domain, gen_true, proper_effects_of_expression_instruction,
	 expression_domain, gen_false, gen_null, /* NOT THESE CALLS */
	 NULL);

    pips_debug(1,"end\n");
    free_effects_private_current_stmt_stack();
    free_effects_private_current_context_stack();
    free_current_downward_cumulated_range_effects_stack();
}

bool proper_effects_engine(char *module_name)
{
    /* Get the code of the module. */
    set_current_module_statement( (statement)
		      db_get_memory_resource(DBR_CODE, module_name, TRUE));

    set_current_module_entity(module_name_to_entity(module_name));

    (*effects_computation_init_func)(module_name);

    /* Compute the effects or references of the module. */
    init_proper_rw_effects();

    if (get_pointer_info_kind() == with_points_to)
      set_pt_to_list( (statement_points_to)
			   db_get_memory_resource(DBR_POINTS_TO_LIST, module_name, TRUE) );
    else if (get_pointer_info_kind() == with_pointer_values)
      set_pv( db_get_simple_pv(module_name));

    debug_on("PROPER_EFFECTS_DEBUG_LEVEL");
    pips_debug(1, "begin\n");

    proper_effects_of_module_statement(get_current_module_statement());

    pips_debug(1, "end\n");
    debug_off();

    (*db_put_proper_rw_effects_func)(module_name, get_proper_rw_effects());

     if (get_pointer_info_kind() == with_points_to)
       reset_pt_to_list();
     else if (get_pointer_info_kind() == with_pointer_values)
       reset_pv();

    reset_current_module_entity();
    reset_current_module_statement();
    reset_proper_rw_effects();

    (*effects_computation_reset_func)(module_name);

    return(TRUE);
}




/* compute proper effects for both expressions and statements
   WARNING: the functions are set as a side effect.
 */
void
expression_proper_effects_engine(
    string module_name,
    statement current)
{
    (*effects_computation_init_func)(module_name);

    init_proper_rw_effects();
    init_expr_prw_effects();

    debug_on("PROPER_EFFECTS_DEBUG_LEVEL");
    pips_debug(1, "begin\n");

    proper_effects_of_module_statement(current);

    pips_debug(1, "end\n");
    debug_off();

    (*effects_computation_reset_func)(module_name);
}
