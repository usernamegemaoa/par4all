/*
 * $Id$
 *
 * Associated to each resource, the methods to read, write, free or check it.
 * For some reason the is no significant default, although the gen_* methods
 * would fit most needs.
 *
 * NOTE: this is the unload order...
 */

#define NEWGEN_METHODS \
    (READER) gen_read, \
    (WRITER) gen_write, \
    (FREER) gen_free, \
    (CHECKER) gen_consistent_p

#define NEWGENNOCHECK_METHODS \
    (READER) gen_read, \
    (WRITER) gen_write, \
    (FREER) gen_free, \
    (CHECKER) gen_true

#define DONOTHING_METHODS \
	no_read, no_write, no_free, (CHECKER) gen_true

#define STATEMENT_FUNCTION_METHODS 		\
    (READER) pipsdbm_read_statement_function, 	\
    (WRITER) pipsdbm_write_statement_function, 	\
    (FREER) gen_free, 				\
    (CHECKER) pipsdbm_consistent_statement_function 

#define STATEMENT_MAPPING_METHODS 		\
    (READER)  pipsdbm_read_statement_mapping, 	\
    (WRITER)  pipsdbm_write_statement_mapping, 	\
    (FREER)   pipsdbm_free_statement_mapping,	\
    (CHECKER) pipsdbm_check_statement_mapping

#define GENFREE_METHODS \
	no_read, no_write, (FREER) gen_free, (CHECKER) gen_true

#define STRING_METHODS \
	safe_readline, writeln_string, (FREER) free, (CHECKER) gen_true

/* EFFECTS
 */
{ DBR_PROPER_REFERENCES,	STATEMENT_FUNCTION_METHODS },
{ DBR_CUMULATED_REFERENCES,	STATEMENT_FUNCTION_METHODS },
{ DBR_PROPER_EFFECTS,		STATEMENT_FUNCTION_METHODS },
{ DBR_CUMULATED_EFFECTS,	STATEMENT_FUNCTION_METHODS },
{ DBR_SUMMARY_EFFECTS, 		NEWGEN_METHODS },
{ DBR_IN_EFFECTS,		STATEMENT_FUNCTION_METHODS },
{ DBR_CUMULATED_IN_EFFECTS,	STATEMENT_FUNCTION_METHODS },
{ DBR_OUT_EFFECTS, 		STATEMENT_FUNCTION_METHODS },
{ DBR_IN_SUMMARY_EFFECTS, 	NEWGEN_METHODS },
{ DBR_OUT_SUMMARY_EFFECTS, 	NEWGEN_METHODS },
    
/* REDUCTIONS
 */
{ DBR_PROPER_REDUCTIONS, 	STATEMENT_FUNCTION_METHODS },
{ DBR_CUMULATED_REDUCTIONS, 	STATEMENT_FUNCTION_METHODS },
{ DBR_SUMMARY_REDUCTIONS, 	NEWGEN_METHODS },
    
{ DBR_STATIC_CONTROL, 		
      (READER) pipsdbm_read_statement_mapping, 
      (WRITER) pipsdbm_write_statement_mapping,  
      (FREER) free_static_control_mapping,
      (CHECKER) pipsdbm_check_statement_mapping },

/* PAF stuff...
 */
{ DBR_ADFG, 			DONOTHING_METHODS },
{ DBR_ADFG_FILE, 		STRING_METHODS },
{ DBR_BDT, 			DONOTHING_METHODS },
{ DBR_BDT_FILE, 		STRING_METHODS },
{ DBR_PLC, 			DONOTHING_METHODS },
{ DBR_PLC_FILE, 		STRING_METHODS },

/* REGIONS
 */    
{ DBR_REGIONS,			STATEMENT_FUNCTION_METHODS },
{ DBR_INV_REGIONS,		STATEMENT_FUNCTION_METHODS },
{ DBR_PROPER_REGIONS,		STATEMENT_FUNCTION_METHODS },
{ DBR_SUMMARY_REGIONS, 		NEWGEN_METHODS },
{ DBR_IN_REGIONS,		STATEMENT_FUNCTION_METHODS },
{ DBR_INV_IN_REGIONS,		STATEMENT_FUNCTION_METHODS },
{ DBR_CUMULATED_IN_REGIONS, 	STATEMENT_FUNCTION_METHODS },
{ DBR_IN_SUMMARY_REGIONS, 	NEWGEN_METHODS },
{ DBR_OUT_REGIONS, 		STATEMENT_FUNCTION_METHODS },
{ DBR_OUT_SUMMARY_REGIONS, 	NEWGEN_METHODS },
{ DBR_PRIVATIZED_REGIONS, 	STATEMENT_FUNCTION_METHODS },
{ DBR_COPY_OUT_REGIONS, 	STATEMENT_FUNCTION_METHODS },
    
{ DBR_ALIAS_CLASSES, 		NEWGEN_METHODS },
{ DBR_ALIAS_LISTS, 		NEWGEN_METHODS },
{ DBR_IN_ALIAS_PAIRS, 		NEWGEN_METHODS },
{ DBR_OUT_ALIAS_PAIRS, 		NEWGEN_METHODS },
    
{ DBR_COMPSEC,			STATEMENT_MAPPING_METHODS },
{ DBR_SUMMARY_COMPSEC, 		NEWGEN_METHODS },

/* SEMANTICS
 */
{ DBR_TRANSFORMERS, 		STATEMENT_MAPPING_METHODS },
{ DBR_PRECONDITIONS,		STATEMENT_MAPPING_METHODS },
{ DBR_SUMMARY_TRANSFORMER, 	NEWGENNOCHECK_METHODS },
{ DBR_SUMMARY_PRECONDITION, 	NEWGENNOCHECK_METHODS },
{ DBR_INITIAL_PRECONDITION, 	NEWGENNOCHECK_METHODS },
{ DBR_PROGRAM_PRECONDITION, 	NEWGENNOCHECK_METHODS },
    
{ DBR_MUST_CONTINUATION,	STATEMENT_MAPPING_METHODS },
{ DBR_MAY_CONTINUATION,		STATEMENT_MAPPING_METHODS },
{ DBR_MUST_SUMMARY_CONTINUATION,NEWGENNOCHECK_METHODS },
{ DBR_MAY_SUMMARY_CONTINUATION, NEWGENNOCHECK_METHODS },

/* COMPLEXITY
 */
{ DBR_COMPLEXITIES,		STATEMENT_MAPPING_METHODS },
{ DBR_SUMMARY_COMPLEXITY, 	NEWGEN_METHODS },

/* FILE NAMES
 */
{ DBR_USER_FILE,		STRING_METHODS },
{ DBR_INITIAL_FILE,		STRING_METHODS },
{ DBR_SOURCE_FILE,		STRING_METHODS },
{ DBR_PARSED_PRINTED_FILE,	STRING_METHODS },
{ DBR_PRINTED_FILE,		STRING_METHODS },
{ DBR_GRAPH_PRINTED_FILE,	STRING_METHODS },
{ DBR_FLINTED_FILE,		STRING_METHODS },
{ DBR_PARALLELPRINTED_FILE,	STRING_METHODS },
{ DBR_CALLGRAPH_FILE,		STRING_METHODS },
{ DBR_WP65_BANK_FILE,		STRING_METHODS },
{ DBR_WP65_COMPUTE_FILE,	STRING_METHODS },
{ DBR_ICFG_FILE,		STRING_METHODS },
{ DBR_DG_FILE,			STRING_METHODS },
{ DBR_ALIAS_FILE,		STRING_METHODS },
{ DBR_DVCG_FILE,		STRING_METHODS },

/* HPFC GENERATED FILES
 */
{ DBR_HPFC_FILTERED_FILE, 	STRING_METHODS },
{ DBR_HPFC_PARAMETERS,		STRING_METHODS },
{ DBR_HPFC_HOST,	 	STRING_METHODS },
{ DBR_HPFC_NODE,	 	STRING_METHODS },
{ DBR_HPFC_RTINIT,	 	STRING_METHODS },
{ DBR_HPFC_COMMONS,	 	DONOTHING_METHODS },
{ DBR_HPFC_INSTALLATION, 	DONOTHING_METHODS },

/* LOST
 */
{ DBR_CHAINS,			GENFREE_METHODS },
{ DBR_DG,			GENFREE_METHODS },

/* CODE...
 */
{ DBR_PARSED_CODE, 		NEWGEN_METHODS },
{ DBR_PARALLELIZED_CODE, 	NEWGEN_METHODS },
{ DBR_REINDEXED_CODE, 		NEWGEN_METHODS },
{ DBR_CALLEES, 			NEWGEN_METHODS },
{ DBR_CALLERS, 			NEWGEN_METHODS },
{ DBR_CODE, 			NEWGEN_METHODS },

/* Misc files.
 */
{ DBR_CROUGH,			STRING_METHODS },
{ DBR_C,			STRING_METHODS },

/* PROGRAM
 */
/* entities are tabulated... here are some wrappers that expect
 * entity_domain as the pointer value... see bootstrap/bootstrap.c
 */
#define ENTITY_METHODS \
    (READER) pipsdbm_read_entities, \
    (WRITER) gen_write_tabulated, \
    (FREER) pipsdbm_free_entities, \
    (CHECKER) gen_true

{ DBR_HPFC_STATUS, 		NEWGEN_METHODS },
{ DBR_ENTITIES,			ENTITY_METHODS },

#define UNEXPECTED_METHODS \
	(READER) unexpected, (WRITER) unexpected, \
	(FREER) unexpected, (CHECKER) unexpected

/* this one MUST be the last one.
 */    
{ NULL,				UNEXPECTED_METHODS }
