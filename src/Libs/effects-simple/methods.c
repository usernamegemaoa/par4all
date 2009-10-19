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
/* package simple effects :  Be'atrice Creusillet 5/97
 *
 * File: methods.c
 * ~~~~~~~~~~~~~~~
 *
 * This File contains the intanciation of the generic functions necessary 
 * for the computation of all types of simple effects.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genC.h"

#include "linear.h"
#include "ri.h"
#include "ri-util.h"
#include "database.h"
#include "resources.h"
#include "pipsdbm.h"

#include "effects-generic.h"
#include "effects-simple.h"

/******************************************************* PIPSDBM INTERFACES */

DB_GETPUT_SE(proper_references, PROPER_REFERENCES)
DB_GETNOPUT_LS(summary_references, SUMMARY_EFFECTS)
DB_GETPUT_SE(cumulated_references, CUMULATED_REFERENCES)
DB_GETNOPUT_SE(invariant_references, CUMULATED_REFERENCES)
DB_GETPUT_SE(simple_proper_rw_effects, PROPER_EFFECTS)
DB_GETNOPUT_SE(simple_invariant_rw_effects, CUMULATED_EFFECTS)
DB_GETPUT_SE(simple_rw_effects, CUMULATED_EFFECTS)
DB_GETPUT_LS(simple_summary_rw_effects, SUMMARY_EFFECTS)
DB_GETPUT_SE(simple_in_effects, IN_EFFECTS)
DB_GETPUT_SE(simple_out_effects, OUT_EFFECTS)
DB_GETPUT_SE(simple_cumulated_in_effects, CUMULATED_IN_EFFECTS)
DB_GET_SE(simple_invariant_in_effects, IN_EFFECTS)
DB_NOPUT_SE(simple_invariant_in_effects)
DB_GETPUT_LS(simple_summary_in_effects, IN_SUMMARY_EFFECTS)
DB_GETPUT_LS(simple_summary_out_effects, OUT_SUMMARY_EFFECTS)

void
set_methods_for_proper_references()
{

  effect_consistent_p_func = effect_consistent_p;
    effects_computation_init_func = effects_computation_no_init;
    effects_computation_reset_func = effects_computation_no_reset;

    effect_dup_func = copy_effect;
    effect_free_func = free_effect;

    reference_to_effect_func = reference_to_reference_effect;
    effect_to_store_independent_effect_list_func = 
      effect_to_store_independent_sdfi_list;

    /* FI: no longer good with conditional expressions? Should we use
       EffectsMustUnion or ReferenceMustUnion? Does not seem to have
       an impact... */
    //effects_union_op = effects_undefined_binary_operator;
    //effects_test_union_op = effects_undefined_binary_operator;
    effects_union_op = EffectsMustUnion;
    effects_test_union_op = EffectsMayUnion;
    effects_intersection_op = effects_undefined_binary_operator;
    effects_sup_difference_op = effects_undefined_binary_operator;
    effects_inf_difference_op = effects_undefined_binary_operator;

    effects_transformer_composition_op =
	effects_undefined_composition_with_transformer;
    effects_transformer_inverse_composition_op =
	effects_undefined_composition_with_transformer;
    effects_precondition_composition_op =
	effects_composition_with_preconditions_nop;

    effects_descriptors_variable_change_func =
	effects_undefined_descriptors_variable_change;
    loop_descriptor_make_func = loop_undefined_descriptor_make;
    effects_loop_normalize_func = effects_undefined_loop_normalize;
    effects_union_over_range_op = simple_effects_union_over_range;
    vector_to_descriptor_func = effects_undefined_vector_to_descriptor;

    effects_translation_init_func = simple_effects_translation_init;
    effects_translation_end_func = simple_effects_translation_end;
    effect_descriptor_interprocedural_translation_op = simple_effect_descriptor_interprocedural_translation ;

    effects_backward_translation_op = generic_effects_backward_translation;
    fortran_effects_backward_translation_op = simple_effects_backward_translation;
    effects_forward_translation_op = simple_effects_forward_translation;/*FC*/
    effects_local_to_global_translation_op = effects_dynamic_elim;
    c_effects_on_formal_parameter_backward_translation_func =
      c_simple_effects_on_formal_parameter_backward_translation;

    load_context_func = load_undefined_context;
    load_transformer_func = load_undefined_transformer;
    empty_context_test = empty_context_test_false;

    effects_descriptor_normalize_func = simple_effects_descriptor_normalize;

    proper_to_summary_effect_func = effect_nop; /* FC */

    db_get_summary_rw_effects_func = db_get_summary_references;

    db_get_proper_rw_effects_func = db_get_proper_references;
    db_put_proper_rw_effects_func = db_put_proper_references;

    set_contracted_proper_effects(FALSE);
    set_descriptor_range_p(FALSE);
    set_methods_for_rw_effects_prettyprint(string_undefined);
}


void
set_methods_for_cumulated_references()
{
  effect_consistent_p_func = effect_consistent_p;
    effects_computation_init_func = effects_computation_no_init;
    effects_computation_reset_func = effects_computation_no_reset;

    effect_dup_func = copy_effect;
    effect_free_func = free_effect;

    reference_to_effect_func = reference_to_reference_effect;
    effect_to_store_independent_effect_list_func = 
      effect_to_store_independent_sdfi_list;

    /* effect_union_op = ; *//* ??? */
    effects_union_op = ReferenceUnion;
    effects_test_union_op = ReferenceTestUnion;
    effects_intersection_op = effects_undefined_binary_operator;
    effects_sup_difference_op = effects_undefined_binary_operator;
    effects_inf_difference_op = effects_undefined_binary_operator;
    effects_transformer_composition_op =
	effects_composition_with_transformer_nop;
	/* FC: effects_undefined_composition_with_transformer; */
    effects_transformer_inverse_composition_op =
	effects_undefined_composition_with_transformer;
    effects_precondition_composition_op =
	effects_composition_with_preconditions_nop;

    effects_descriptors_variable_change_func =
	effects_undefined_descriptors_variable_change;
    loop_descriptor_make_func = loop_undefined_descriptor_make;
    effects_loop_normalize_func = effects_undefined_loop_normalize;
    effects_union_over_range_op = effects_union_over_range_nop;
    /* FC was: effects_undefined_union_over_range */

    vector_to_descriptor_func = effects_undefined_vector_to_descriptor;


    effects_translation_init_func = simple_effects_translation_init;
    effects_translation_end_func = simple_effects_translation_end;
    effect_descriptor_interprocedural_translation_op = simple_effect_descriptor_interprocedural_translation ;

    effects_backward_translation_op = generic_effects_backward_translation;
    fortran_effects_backward_translation_op = simple_effects_backward_translation;
    effects_forward_translation_op = simple_effects_forward_translation;
    effects_local_to_global_translation_op = effects_dynamic_elim;
    c_effects_on_formal_parameter_backward_translation_func =
      c_simple_effects_on_formal_parameter_backward_translation;

    load_context_func = load_undefined_context;
    load_transformer_func = load_undefined_transformer;
    empty_context_test = empty_context_test_false;

    proper_to_summary_effect_func = effect_nop; /* FC */

    effects_descriptor_normalize_func = simple_effects_descriptor_normalize;

    db_get_summary_rw_effects_func = db_get_summary_references;
    db_put_summary_rw_effects_func = db_put_summary_references;

    db_get_proper_rw_effects_func = db_get_proper_references;
    db_put_proper_rw_effects_func = db_put_proper_references;
 
    db_get_invariant_rw_effects_func = db_get_invariant_references;
    db_put_invariant_rw_effects_func = db_put_invariant_references;

    db_get_rw_effects_func = db_get_cumulated_references;
    db_put_rw_effects_func = db_put_cumulated_references;

    /* FC: rm set_contracted_proper_effects(!PROPER_EFFECTS_CONTRACT); */
    set_contracted_rw_effects(FALSE);
    set_descriptor_range_p(FALSE);
    set_methods_for_rw_effects_prettyprint(string_undefined);
}


void
set_methods_for_proper_simple_effects()
{

  effect_consistent_p_func = effect_consistent_p;
    effects_computation_init_func = effects_computation_no_init;
    effects_computation_reset_func = effects_computation_no_reset;

    effect_dup_func = simple_effect_dup;
    effect_free_func = free_effect;

    reference_to_effect_func = reference_to_simple_effect;
    effect_to_store_independent_effect_list_func = 
      effect_to_store_independent_sdfi_list;
    effect_add_expression_dimension_func =
      simple_effect_add_expression_dimension;
    effect_change_ith_dimension_expression_func =
      simple_effect_change_ith_dimension_expression;
    effect_union_op = effect_must_union;

    /* FIL Changed because of C conditional operator. */
    // effects_union_op = effects_undefined_binary_operator;
    //effects_test_union_op = effects_undefined_binary_operator;
    effects_union_op = EffectsMustUnion;
    effects_test_union_op = EffectsMayUnion;

    effects_intersection_op = effects_undefined_binary_operator;
    effects_sup_difference_op = effects_undefined_binary_operator;
    effects_inf_difference_op = effects_undefined_binary_operator;
    effects_transformer_composition_op =
	effects_composition_with_transformer_nop;
    effects_transformer_inverse_composition_op =
	effects_undefined_composition_with_transformer;
    effects_precondition_composition_op =
	effects_composition_with_preconditions_nop;

    effects_descriptors_variable_change_func =
	effects_undefined_descriptors_variable_change;
    loop_descriptor_make_func = loop_undefined_descriptor_make;
    effects_loop_normalize_func = effects_undefined_loop_normalize;
    effects_union_over_range_op = simple_effects_union_over_range;
    vector_to_descriptor_func = effects_undefined_vector_to_descriptor;



    effects_translation_init_func = simple_effects_translation_init;
    effects_translation_end_func = simple_effects_translation_end;
    effect_descriptor_interprocedural_translation_op = simple_effect_descriptor_interprocedural_translation ;

    effects_backward_translation_op = generic_effects_backward_translation;
    fortran_effects_backward_translation_op = simple_effects_backward_translation;
    effects_forward_translation_op = simple_effects_forward_translation;
    effects_local_to_global_translation_op = effects_dynamic_elim;
    c_effects_on_formal_parameter_backward_translation_func =
      c_simple_effects_on_formal_parameter_backward_translation;

    load_context_func = load_undefined_context;
    load_transformer_func = load_undefined_transformer;
    empty_context_test = empty_context_test_false;
    
    proper_to_summary_effect_func = effect_nop;

    effects_descriptor_normalize_func = simple_effects_descriptor_normalize;

    db_get_proper_rw_effects_func = db_get_simple_proper_rw_effects;
    db_put_proper_rw_effects_func = db_put_simple_proper_rw_effects;
    db_get_summary_rw_effects_func = db_get_simple_summary_rw_effects; /* FC */

    set_contracted_proper_effects(TRUE);
    set_descriptor_range_p(FALSE);
    set_methods_for_rw_effects_prettyprint(string_undefined);
}

void
set_methods_for_simple_effects()
{
  effect_consistent_p_func = effect_consistent_p;
    effects_computation_init_func = effects_computation_no_init;
    effects_computation_reset_func = effects_computation_no_reset;

    effect_dup_func = simple_effect_dup;
    effect_free_func = free_effect;

    reference_to_effect_func = reference_to_simple_effect;
    effect_to_store_independent_effect_list_func = 
      effect_to_store_independent_sdfi_list;
    effect_add_expression_dimension_func =
      simple_effect_add_expression_dimension;
    effect_change_ith_dimension_expression_func =
      simple_effect_change_ith_dimension_expression;

    effect_union_op = effect_must_union;
    effects_union_op = EffectsMustUnion;
    effects_test_union_op = EffectsMayUnion;
    effects_intersection_op = effects_entities_intersection;
    effects_sup_difference_op = EffectsSupDifference;
    effects_inf_difference_op = EffectsInfDifference;
    effects_transformer_composition_op =
	simple_effects_composition_with_effect_transformer;
    effects_transformer_inverse_composition_op =
	effects_composition_with_transformer_nop;
    effects_precondition_composition_op =
	effects_composition_with_preconditions_nop;

    effects_descriptors_variable_change_func =
	effects_descriptors_variable_change_nop;
    loop_descriptor_make_func = loop_undefined_descriptor_make;
    effects_loop_normalize_func = effects_loop_normalize_nop;
    effects_union_over_range_op = simple_effects_union_over_range;
    vector_to_descriptor_func = effects_undefined_vector_to_descriptor;


    effects_translation_init_func = simple_effects_translation_init;
    effects_translation_end_func = simple_effects_translation_end;
    effect_descriptor_interprocedural_translation_op = simple_effect_descriptor_interprocedural_translation ;

    effects_backward_translation_op = simple_effects_backward_translation;
    fortran_effects_backward_translation_op = simple_effects_backward_translation;
    effects_forward_translation_op = simple_effects_forward_translation;
    c_effects_on_formal_parameter_backward_translation_func =
      c_simple_effects_on_formal_parameter_backward_translation;

    effects_local_to_global_translation_op = effects_dynamic_elim;

    load_context_func = load_undefined_context;
    load_transformer_func = load_undefined_transformer;
    empty_context_test = empty_context_test_false;
    proper_to_summary_effect_func = proper_to_summary_simple_effect;

    effects_descriptor_normalize_func = simple_effects_descriptor_normalize;

    db_get_proper_rw_effects_func = db_get_simple_proper_rw_effects;
    db_put_proper_rw_effects_func = db_put_simple_proper_rw_effects;

    db_get_invariant_rw_effects_func = db_get_simple_invariant_rw_effects;
    db_put_invariant_rw_effects_func = db_put_simple_invariant_rw_effects;

    db_get_rw_effects_func = db_get_simple_rw_effects;
    db_put_rw_effects_func = db_put_simple_rw_effects;

    db_get_summary_rw_effects_func = db_get_simple_summary_rw_effects;
    db_put_summary_rw_effects_func = db_put_simple_summary_rw_effects;
    
    db_get_in_effects_func = db_get_simple_in_effects;
    db_put_in_effects_func = db_put_simple_in_effects;
    
    db_get_out_effects_func = db_get_simple_out_effects;
    db_put_out_effects_func = db_put_simple_out_effects;
    
    db_get_cumulated_in_effects_func = db_get_simple_cumulated_in_effects;
    db_put_cumulated_in_effects_func = db_put_simple_cumulated_in_effects;
    
    db_get_invariant_in_effects_func = db_get_simple_invariant_in_effects;
    db_put_invariant_in_effects_func = db_put_simple_invariant_in_effects;

    db_get_summary_in_effects_func = db_get_simple_summary_in_effects;
    db_put_summary_in_effects_func = db_put_simple_summary_in_effects;

    db_get_summary_out_effects_func = db_get_simple_summary_out_effects;
    db_put_summary_out_effects_func = db_put_simple_summary_out_effects;

    set_contracted_rw_effects(TRUE);
    set_descriptor_range_p(FALSE);
    set_methods_for_rw_effects_prettyprint(string_undefined);
}

void set_methods_for_inout_effects(string module_name __attribute__ ((unused)))
{
  set_methods_for_simple_effects();

  /* for intermediate values ??? FC 15/05/2002 
   * I'm not really sure it is actually useful for simple effects...
   */
  /*
  set_current_module_entity(module_name_to_entity(module_name));
  set_current_module_statement((statement)
	db_get_memory_resource(DBR_CODE, module_name, TRUE) );

  set_cumulated_rw_effects((statement_effects)
     db_get_memory_resource(DBR_CUMULATED_EFFECTS, module_name, TRUE));

  module_to_value_mappings(module_name_to_entity(module_name));

  reset_current_module_entity();
  reset_current_module_statement();
  */
}

void reset_methods_for_inout_effects()
{
  /*
  reset_cumulated_rw_effects();
  free_value_mappings();
  */
}

void set_methods_for_rw_effects_prettyprint(string module_name __attribute__ ((unused)))
{
  set_action_interpretation(ACTION_READ, ACTION_WRITE);
  effects_prettyprint_func = print_effects;
  effect_prettyprint_func = print_effect;
  effects_to_text_func = simple_rw_effects_to_text;
}

void set_methods_for_inout_effects_prettyprint(string module_name __attribute__ ((unused)))
{
  set_action_interpretation(ACTION_IN, ACTION_OUT);
  effects_prettyprint_func = print_effects;
  effect_prettyprint_func = print_effect;
  effects_to_text_func = simple_inout_effects_to_text;
}

void reset_methods_for_effects_prettyprint(string module_name __attribute__ ((unused)))
{
    effects_prettyprint_func = (generic_prettyprint_function) abort;
    effect_prettyprint_func = (generic_prettyprint_function) abort;
    effects_to_text_func = (generic_text_function) abort;
    reset_action_interpretation();
}
