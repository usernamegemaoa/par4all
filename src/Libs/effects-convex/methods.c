/* package convex effects :  Be'atrice Creusillet 5/97
 *
 * File: methods.c
 * ~~~~~~~~~~~~~~~
 *
 * This File contains the interfaces with pipsmake which compute the various
 * types of convex regions by using the generic functions.
 *
 */
#include <stdio.h>
#include <string.h>

#include "genC.h"

#include "ri.h"
#include "database.h"
#include "makefile.h"

#include "ri-util.h"
#include "transformer.h"
#include "semantics.h"
#include "properties.h"
#include "resources.h"
#include "pipsmake.h"
#include "pipsdbm.h"
#include "effects-generic.h"
#include "effects-convex.h"

void
set_methods_for_convex_effects()
{

    effect_dup_func = region_dup;
    effect_free_func = region_free;

    reference_to_effect_func = reference_to_convex_region;

    effect_union_op = regions_must_convex_hull;
    effects_union_op = RegionsMustUnion;
    effects_test_union_op = RegionsMayUnion;
    effects_intersection_op = RegionsIntersection;
    effects_sup_difference_op = RegionsSupDifference;
    effects_inf_difference_op = RegionsInfDifference;

    effects_transformer_composition_op =
	convex_regions_transformer_compose;
    effects_transformer_inverse_composition_op =
	convex_regions_inverse_transformer_compose;
    effects_precondition_composition_op =
	convex_regions_precondition_compose;

    effects_descriptors_variable_change_func =
	convex_regions_descriptor_variable_rename;
    loop_descriptor_make_func = loop_convex_descriptor_make;
    effects_loop_normalize_func = convex_regions_loop_normalize;
    effects_union_over_range_op = convex_regions_union_over_range;
    vector_to_descriptor_func = vector_to_convex_descriptor;

    effects_backward_translation_op = convex_regions_backward_translation;
    effects_forward_translation_op = convex_regions_forward_translation;
    effects_local_to_global_translation_op = regions_dynamic_elim;

    effects_prettyprint_func = print_regions;
    effects_to_text_func = text_array_regions;

    load_context_func = load_statement_precondition;
    load_transformer_func = load_statement_transformer;
    empty_context_test = empty_convex_context_p;

    proper_to_summary_effect_func = effect_nop;

    effects_descriptor_normalize_func = convex_effects_descriptor_normalize;

    db_get_proper_rw_effects_func = db_get_convex_proper_rw_regions;
    db_put_proper_rw_effects_func = db_put_convex_proper_rw_regions;

    db_get_invariant_rw_effects_func = db_get_convex_invariant_rw_regions;
    db_put_invariant_rw_effects_func = db_put_convex_invariant_rw_regions;

    db_get_rw_effects_func = db_get_convex_rw_regions;
    db_put_rw_effects_func = db_put_convex_rw_regions;

    db_get_summary_rw_effects_func = db_get_convex_summary_rw_regions;
    db_put_summary_rw_effects_func = db_put_convex_summary_rw_regions;
    
    db_get_in_effects_func = db_get_convex_in_regions;
    db_put_in_effects_func = db_put_convex_in_regions;
    
    db_get_cumulated_in_effects_func = db_get_convex_cumulated_in_regions;
    db_put_cumulated_in_effects_func = db_put_convex_cumulated_in_regions;
    
    db_get_invariant_in_effects_func = db_get_convex_invariant_in_regions;
    db_put_invariant_in_effects_func = db_put_convex_invariant_in_regions;

    db_get_summary_in_effects_func = db_get_convex_summary_in_regions;
    db_put_summary_in_effects_func = db_put_convex_summary_in_regions;

    db_get_out_effects_func = db_get_convex_out_regions;
    db_put_out_effects_func = db_put_convex_out_regions;
    
    db_get_summary_out_effects_func = db_get_convex_summary_out_regions;
    db_put_summary_out_effects_func = db_put_convex_summary_out_regions;

    set_contracted_proper_effects(PROPER_EFFECTS_CONTRACT);
    set_descriptor_range_p(TRUE);
}

void
init_convex_rw_regions(string module_name)
{
    regions_init();
    get_regions_properties();
    region_translation_statistics_init
	(get_bool_property("REGIONS_TRANSLATION_STATISTICS"));

    /* Get the transformers and preconditions of the module. */
    set_transformer_map( (statement_mapping)
	db_get_memory_resource(DBR_TRANSFORMERS, module_name, TRUE) );	
    set_precondition_map( (statement_mapping) 
	db_get_memory_resource(DBR_PRECONDITIONS, module_name, TRUE) );

    /* for intermediate values */
    set_cumulated_rw_effects((statement_effects)
	   db_get_memory_resource(DBR_CUMULATED_EFFECTS, module_name, TRUE));
    module_to_value_mappings( local_name_to_top_level_entity(module_name));

    set_read_action_interpretation(READ_IS_READ);
    set_write_action_interpretation(WRITE_IS_WRITE);
}

void
init_convex_in_out_regions(string module_name)
{
    regions_init();
    if (!same_string_p(rule_phase(find_rule_by_resource("REGIONS")),
		       "MUST_REGIONS"))
	pips_user_warning("\nMUST REGIONS not selected - "
			  "Do not expect wonderful results\n");
    set_bool_property("MUST_REGIONS", TRUE);
    set_bool_property("EXACT_REGIONS", TRUE);
    get_in_out_regions_properties();
			  
    /* Get the transformers and preconditions of the module. */
    set_transformer_map( (statement_mapping)
	db_get_memory_resource(DBR_TRANSFORMERS, module_name, TRUE) );	
    set_precondition_map( (statement_mapping) 
	db_get_memory_resource(DBR_PRECONDITIONS, module_name, TRUE) );

    /* for intermediate values */
    set_cumulated_rw_effects((statement_effects)
	   db_get_memory_resource(DBR_CUMULATED_EFFECTS, module_name, TRUE));
    module_to_value_mappings( local_name_to_top_level_entity(module_name));
    set_read_action_interpretation(READ_IS_IN);
    set_write_action_interpretation(WRITE_IS_OUT);
}

void 
reset_convex_rw_regions(string module_name)
{
    regions_end();
    region_translation_statistics_close(module_name, "rw");
    reset_transformer_map();
    reset_precondition_map();
    reset_cumulated_rw_effects();
}

void 
reset_convex_in_out_regions(string module_name)
{
    regions_end();
    reset_transformer_map();
    reset_precondition_map();
    reset_cumulated_rw_effects();
}

void
init_convex_summary_rw_regions(string module_name)
{
    regions_init();
    /* for intermediate values */
    set_cumulated_rw_effects((statement_effects)
	   db_get_memory_resource(DBR_CUMULATED_EFFECTS, module_name, TRUE));
    module_to_value_mappings( local_name_to_top_level_entity(module_name));

    set_read_action_interpretation(READ_IS_READ);
    set_write_action_interpretation(WRITE_IS_WRITE);
}

void
reset_convex_summary_rw_regions(string module_name)
{
    regions_end();
    reset_cumulated_rw_effects();

}

void
init_convex_summary_in_out_regions(string module_name)
{
    regions_init();
    set_bool_property("MUST_REGIONS", TRUE);
    set_bool_property("EXACT_REGIONS", TRUE);
    get_in_out_regions_properties();
    /* for intermediate values */
    set_cumulated_rw_effects((statement_effects)
	   db_get_memory_resource(DBR_CUMULATED_EFFECTS, module_name, TRUE));
    module_to_value_mappings( local_name_to_top_level_entity(module_name));

    set_read_action_interpretation(READ_IS_IN);
    set_write_action_interpretation(WRITE_IS_OUT);

}

void
reset_convex_summary_in_out_regions(string module_name)
{
    regions_end();
    reset_cumulated_rw_effects();

}
