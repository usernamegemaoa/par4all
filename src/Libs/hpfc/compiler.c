/*
 * HPF Compiler
 * 
 * Fabien Coelho, May 1993
 *
 * SCCS stuff
 * $RCSfile: compiler.c,v $ ($Date: 1994/12/02 15:00:57 $, )
 * version $Revision$
 * got on %D%, %T%
 * $Id$
 */

/*
 * included files, from C libraries, newgen and pips libraries.
 */

#include <stdio.h>
#include <string.h>
extern int fprintf();

#include "genC.h"

#include "ri.h"
#include "database.h"
#include "hpf.h"
#include "hpf_private.h"

/* #include "compiler_parameters.h" */

#include "misc.h"
#include "ri-util.h"
#include "control.h"     /* for CONTROL_MAP() */
#include "text-util.h"
#include "properties.h"
#include "hpfc.h"
#include "defines-local.h"

/*
 * global variables
 */

entity 
    host_module,
    node_module;

GENERIC_CURRENT_MAPPING(host_gotos, statement, statement);
GENERIC_CURRENT_MAPPING(node_gotos, statement, statement);

/*
 * Compiler
 *
 * stat is the current statement to be compiled, and there are
 * pointers to the current statement building of the node and host
 * codes. the module of these are also kept in order to adds
 * the needed declarations generated by the compilation.
 *
 * however, every entities of the compiled program, and of
 * both generated programs will be mixed, due to the
 * tabulated nature of these objects. 
 * some objects will be shared.
 * I don't think this is a problem.
 */


/*
 * hpfcompiler
 *
 * drive the compilation of the statement to the relevent
 * procedure.
 *
 * Recursive calls used in the top-down walk of the program.
 */
void hpfcompiler(stat, hoststatp, nodestatp)
statement stat;
statement *hoststatp,*nodestatp;
{
    if (get_bool_property("HPFC_NEW_IO_COMPILATION") &&
	load_statement_only_io(stat)==TRUE) /* necessary */
	if (io_efficient_compilable_p(stat))
	{
	    io_efficient_compile(stat,  hoststatp, nodestatp);
	    return;
	}
    
    /* else usual stuff */

    switch(instruction_tag(statement_instruction(stat)))
    {
    case is_instruction_block:
	hpfcompileblock(stat, hoststatp, nodestatp);
	break;
    case is_instruction_test:
	hpfcompiletest(stat, hoststatp, nodestatp);
	break;
    case is_instruction_loop:
	hpfcompileloop(stat, hoststatp, nodestatp);
	break;
    case is_instruction_goto:
	hpfcompilegoto(stat, hoststatp, nodestatp);
	break;
    case is_instruction_call:
	hpfcompilecall(stat, hoststatp, nodestatp);
	break;
    case is_instruction_unstructured:
	hpfcompileunstructured(stat, hoststatp, nodestatp);
	break;
    default:
	pips_error("hpfcompiler",
		   "unexpected instruction tag\n");
	break;
    }
}

/*
 * hpfcompileblock
 */
void hpfcompileblock(stat,hoststatp,nodestatp)
statement stat;
statement *hoststatp,*nodestatp;
{
    list 
	lhost=NULL,
	lnode=NULL;
    statement 
	hostcd,
	nodecd;

    pips_assert("hpfcompileblock",
		(instruction_block_p(statement_instruction(stat))));

    (*hoststatp) =
	MakeStatementLike(stat,
			  is_instruction_block,
			  get_host_gotos_map());

    (*nodestatp) =
	MakeStatementLike(stat,
			  is_instruction_block,
			  get_node_gotos_map());

    MAPL(cs,
     {
	 hpfcompiler(STATEMENT(CAR(cs)),&hostcd,&nodecd);
	 
	 lhost = gen_nconc(lhost,CONS(STATEMENT,hostcd,NULL));
	 lnode = gen_nconc(lnode,CONS(STATEMENT,nodecd,NULL));
     },
	 instruction_block(statement_instruction(stat)));

    instruction_block(statement_instruction(*hoststatp)) = lhost;
    instruction_block(statement_instruction(*nodestatp)) = lnode;
}

/*
 * hpfcompiletest
 */
void hpfcompiletest(stat,hoststatp,nodestatp)
statement stat;
statement *hoststatp,*nodestatp;
{
    statement
	stattrue,
	stathosttrue,
	statnodetrue,
	statfalse,
	stathostfalse,
	statnodefalse;
    test the_test;
    expression condition;
	
    
    pips_assert("hpfcompiletest",
		(instruction_test_p(statement_instruction(stat))));

    the_test = instruction_test(statement_instruction(stat));
    condition = test_condition(the_test);
    
    (*hoststatp) =
	MakeStatementLike(stat,
			  is_instruction_test,
			  get_host_gotos_map());

    (*nodestatp) =
	MakeStatementLike(stat,
			  is_instruction_test,
			  get_node_gotos_map());

    /*
     * if it may happen that a condition modifies the value
     * of a distributed variable, this condition is to be
     * put out of the statement, for separate compilation.
     */

    stattrue = test_true(the_test);
    statfalse = test_false(the_test);

    hpfcompiler(stattrue, &stathosttrue, &statnodetrue);
    hpfcompiler(statfalse, &stathostfalse, &statnodefalse);

    instruction_test(statement_instruction(*hoststatp)) =
	make_test(UpdateExpressionForModule(host_module,condition),
		  stathosttrue,
		  stathostfalse);

    instruction_test(statement_instruction(*nodestatp))=
	make_test(UpdateExpressionForModule(node_module,condition),
		  statnodetrue,
		  statnodefalse);

    IFDBPRINT(9,"hpfcompiletest",host_module,(*hoststatp));
    IFDBPRINT(9,"hpfcompiletest",node_module,(*nodestatp));
}

/*
 * hpfcompileloop
 */
void hpfcompileloop(stat, hoststatp, nodestatp)
statement stat;
statement *hoststatp, *nodestatp;
{
    loop
	the_loop = instruction_loop(statement_instruction(stat));

    pips_assert("hpfcompileloop", (statement_loop_p(stat)));

    if (execution_parallel_p(loop_execution(the_loop)))
    {
	/*
	 * should verify that only listed in labels and distributed
	 * entities are defined inside the body of the loop
	 */
	bool
	    at_ac = atomic_accesses_only_p(stat),
	    in_in = indirections_inside_statement_p(stat);
	
	debug(5, "hpfcompileloop",
	      "condition results: aa %d, in %d\n",
	      at_ac, in_in);

	if (at_ac && !in_in)
	{
	    statement
		overlapstat;

	    debug(7, "hpfcompileloop", "compiling a parallel loop\n");

	    if (Overlap_Analysis(stat, &overlapstat))
	    {
		debug(7, "hpfcompileloop", "overlap analysis succeeded\n");

		*hoststatp = make_continue_statement(entity_empty_label());
		*nodestatp = overlapstat;
		statement_comments(*nodestatp) = statement_comments(stat);
	    }
	    else
	    {
		debug(7, "hpfcompileloop", "overlap analysis is not ok...\n");

		hpfcompileparallelloop(stat, hoststatp, nodestatp);
	    }
	}
	else
	{
	    debug(7,"hpfcompileloop",
		  "compiling a parallel loop sequential...\n");
	    hpfcompilesequentialloop(stat, hoststatp, nodestatp);
	}
    }
    else
    {
	debug(7,"hpfcompileloop","compiling a sequential loop\n");
    
	hpfcompilesequentialloop(stat, hoststatp, nodestatp);
    }
}

/*
 * hpfcompilegoto
 */
void hpfcompilegoto(stat,hoststatp,nodestatp)
statement stat;
statement *hoststatp,*nodestatp;
{
    pips_assert("hpfcompilegoto",
		(instruction_goto_p(statement_instruction(stat))));

    /*
     * nothing is done here, but the corresponding statement is to be
     * found, by using two mappings of statements...
     * the goto is let with a pointer to the statement of the 
     * code being compiled, and the link will be reduced later.
     */
    
    (*hoststatp) =
	MakeStatementLike(stat,
			  is_instruction_goto,
			  get_host_gotos_map());
    (*nodestatp) =
	MakeStatementLike(stat,
			  is_instruction_goto,
			  get_node_gotos_map());
    
    instruction_goto(statement_instruction(*hoststatp))=
	instruction_goto(statement_instruction(stat));
    instruction_goto(statement_instruction(*nodestatp))=
	instruction_goto(statement_instruction(stat));
}

/*
 * hpfcompilecall
 */
void hpfcompilecall(stat, hoststatp, nodestatp)
statement stat;
statement *hoststatp,*nodestatp;
{
    call 
	c = instruction_call(statement_instruction(stat));

    pips_assert("hpfcompilecall",
		(instruction_call_p(statement_instruction(stat))));

    debug(7,"hpfcompilecall", "function %s\n", 
	  entity_name(call_function(c)));

    /*
     * {"WRITE", (MAXINT)},
     * {"REWIND", (MAXINT)},
     * {"OPEN", (MAXINT)},
     * {"CLOSE", (MAXINT)},
     * {"READ", (MAXINT)},
     * {"BUFFERIN", (MAXINT)},
     * {"BUFFEROUT", (MAXINT)},
     * {"ENDFILE", (MAXINT)},
     * {"IMPLIED-DO", (MAXINT)},
     * {"FORMAT", 1},
     */

    if (IO_CALL_P(c)) 
    {
	hpfcompileIO(stat, hoststatp, nodestatp) ;
	return;
    }

    /*
     * no reference to distributed arrays...
     * the call is just translated into local objects.
     */
    if (!call_ref_to_dist_array_p(c))
    {
	list
	    leh=lUpdateExpr(host_module,call_arguments(c)),
	    len=lUpdateExpr(node_module,call_arguments(c));
	
	debug(7,"hpfcompilecall","no reference to distributed variable\n");

	(*hoststatp)=MakeStatementLike(stat,
				       is_instruction_call,
				       get_host_gotos_map());

	(*nodestatp)=MakeStatementLike(stat,
				       is_instruction_call,
				       get_node_gotos_map());
	
	instruction_call(statement_instruction((*hoststatp)))=
	    make_call(call_function(c),leh);

	instruction_call(statement_instruction((*nodestatp)))=
	    make_call(call_function(c),len);

	IFDBPRINT(8,"hpfcompilecall",host_module,(*hoststatp));
	IFDBPRINT(8,"hpfcompilecall",node_module,(*nodestatp));

	return;
    }

    /*
     * should consider read and written variables
     */    
    
    if (ENTITY_ASSIGN_P(call_function(c)))
    {
	list 
	    lh = NIL,
	    ln = NIL,
	    args = call_arguments(c) ;
	expression 
	    w = EXPRESSION(CAR(args)),
	    r = EXPRESSION(CAR(CDR(args)));

	pips_assert("hpfcompilecall",
		    syntax_reference_p(expression_syntax(w)));

	if (array_distributed_p
	    (reference_variable(syntax_reference(expression_syntax(w)))))
	{

	    /* !!! */

	    /*
	     * c1-alpha
	     */
	    debug(8,"hpfcompilecall","c1-alpha\n");
	    
	    generate_c1_alpha(stat,&lh,&ln);
	}
	else
	{
	    syntax 
		s = expression_syntax(r);
	    /* 
	     * reductions are detected here. They are not handled otherwise
	     */
	    
	    if (syntax_call_p(s) && call_reduction_p(syntax_call(s)))
	    {
		statement
		    sh = statement_undefined,
		    sn = statement_undefined;

		if (!compile_reduction(stat, &sh, &sn))
		    pips_error("hpfcompilecall", 
			       "reduction compilation failed\n");

		lh = CONS(STATEMENT, sh, NIL);
		ln = CONS(STATEMENT, sn, NIL);
	    }
	    else
	    {
		/*
		 * c1-beta
		 */
		debug(8,"hpfcompilecall","c1-beta\n");
		
		generate_c1_beta(stat, &lh, &ln);
	    }
	}

	(*hoststatp) =
	    MakeStatementLike(stat, 
			      is_instruction_block, 
			      get_host_gotos_map());

	(*nodestatp) = 
	    MakeStatementLike(stat, 
			      is_instruction_block,
			      get_node_gotos_map());
	
	instruction_block(statement_instruction(*hoststatp)) = lh;
	instruction_block(statement_instruction(*nodestatp)) = ln;
	
	IFDBPRINT(8,"hpfcompilecall", host_module, (*hoststatp));
	IFDBPRINT(8,"hpfcompilecall", node_module, (*nodestatp));
	
	return;
    }

    /*
     * call to something with distributed variables, which is not
     * an assignment. Since I do not use the effects as I should, nothing
     * is done...
     */
    user_warning("hpfcompilecall","not implemented yet\n");
}

/*
 * hpfcompileunstructured
 */
void hpfcompileunstructured(stat,hoststatp,nodestatp)
statement stat;
statement *hoststatp,*nodestatp;
{
    instruction inst=statement_instruction(stat);

    pips_assert("hpfcompileunstructured",
		(instruction_unstructured_p(inst)));

    if (one_statement_unstructured(instruction_unstructured(inst)))
    {
	debug(7,"hpfcompileunstructured","one statement recognize\n");
	/* 
	 * nothing spacial is done! 
	 *
	 * ??? there may be a problem with the label of the statement, if any.
	 */
	hpfcompiler
	    (control_statement
	     (unstructured_control(instruction_unstructured(inst))),
	     hoststatp,
	     nodestatp);
    }
    else
    {
	control_mapping 
	    hostmap = MAKE_CONTROL_MAPPING(),
	    nodemap = MAKE_CONTROL_MAPPING();
	unstructured 
	    u=instruction_unstructured(inst);
	control 
	    ct = unstructured_control(u),
	    ce = unstructured_exit(u),
	    new_ct = control_undefined,
	    new_ce = control_undefined,
	    nodec = control_undefined,
	    hostc = control_undefined;
	statement 
	    statc = statement_undefined,
	    stath = statement_undefined,
	    statn = statement_undefined;

	list 
	    blocks = NIL;

	debug(6, "hpfcompileunstructured", "beginning\n");

	CONTROL_MAP
	    (c,
	 {
	     statc=control_statement(c);
	     
	     hpfcompiler(statc, &stath, &statn);
	     
	     ifdebug(7)
	     {
		 fprintf(stderr, "[hpfcompileunstructured] statements:\n");
		 fprintf(stderr, "statc = \n");
		 print_statement(statc);
		 fprintf(stderr, "host stat = \n");
		 print_statement(stath);
		 fprintf(stderr, "node stat = \n");
		 print_statement(statn);
	     }
	     
	     hostc = make_control(stath, NIL, NIL);
	     SET_CONTROL_MAPPING(hostmap, c, hostc);
	     
	     nodec = make_control(statn, NIL, NIL);
	     SET_CONTROL_MAPPING(nodemap, c, nodec);
	 },
	     ct,
	     blocks);
	
	MAPL(cc,
	 {
	     control
		 c = CONTROL(CAR(cc));
	     
	     update_control_lists(c, hostmap);
	     update_control_lists(c, nodemap);
	 },
	     blocks);

#define debug_print_control(c)\
  fprintf(stderr, \
	  "control 0x%x (stat 0x%x) , %d predecessors, %d successors\n", \
          (unsigned int) c, (unsigned int) control_statement(c), \
	  gen_length(control_predecessors(c)), \
	  gen_length(control_successors(c))); \
  print_statement(control_statement(c));

	ifdebug(9)
	{
	    control
		c_tmp = control_undefined,
		h_tmp = control_undefined,
		n_tmp = control_undefined;

	    fprintf(stderr, "[hpfcompileunstructured] controls:\n");
	    
	    MAPL(cc,
	     {
		 c_tmp = CONTROL(CAR(cc));
		 h_tmp = (control) GET_CONTROL_MAPPING(hostmap, c_tmp);
		 n_tmp = (control) GET_CONTROL_MAPPING(nodemap, c_tmp);
		 
		 fprintf(stderr, "\nOriginal:\n");
		 debug_print_control(c_tmp);
		 fprintf(stderr, "Host:\n");
		 debug_print_control(h_tmp);
		 fprintf(stderr, "Node:\n");
		 debug_print_control(n_tmp);
	     },
		 blocks);
	}

	/*    HOST statement
	 */
	(*hoststatp) = 
	    MakeStatementLike(stat,
			      is_instruction_unstructured,
			      get_host_gotos_map());

	new_ct = (control) GET_CONTROL_MAPPING(hostmap, ct);
	new_ce = (control) GET_CONTROL_MAPPING(hostmap, ce);

	pips_assert("hpfcompileunstructured",
		    !control_undefined_p(new_ct) || 
		    !control_undefined_p(new_ce));

	ifdebug(9)
	{
	    fprintf(stderr,
		    "[hpfcompileunstructured] host unstructured controls:\n");
	    
		 fprintf(stderr, "Main:\n");
		 debug_print_control(new_ct);
		 fprintf(stderr, "Exit:\n");
		 debug_print_control(new_ce);
	}

	instruction_unstructured(statement_instruction(*hoststatp)) =
	    make_unstructured(new_ct, new_ce);

	ifdebug(7)
	{
	    fprintf(stderr, "[hpfcompileunstructured] host new stat:\n");
	    print_statement(*hoststatp);
	}

	/*    NODE statement
	 */
	(*nodestatp) =
	    MakeStatementLike(stat,
			      is_instruction_unstructured,
			      get_node_gotos_map());

	new_ct = (control) GET_CONTROL_MAPPING(nodemap, ct);
	new_ce = (control) GET_CONTROL_MAPPING(nodemap, ce);

	pips_assert("hpfcompileunstructured",
		    !control_undefined_p(new_ct) || 
		    !control_undefined_p(new_ce));
	instruction_unstructured(statement_instruction(*nodestatp)) =
	    make_unstructured(new_ct, new_ce);

	ifdebug(7)
	{
	    fprintf(stderr, "[hpfcompileunstructured] host new stat:\n");
	    print_statement(*hoststatp);
	}
	
	gen_free_list(blocks);
	FREE_CONTROL_MAPPING(hostmap);
	FREE_CONTROL_MAPPING(nodemap);
    }
}


/*
 * DeduceGotos
 *
 * Goto Deduction Pass
 */
void DeduceGotos(stat,map)
statement stat;
statement_mapping map;
{
    instruction inst=statement_instruction(stat);

    switch (instruction_tag(inst))
    {
    case is_instruction_block:
	MAPL(cs,{DeduceGotos(STATEMENT(CAR(cs)),map);},instruction_block(inst));
	break;
    case is_instruction_test:
	DeduceGotos(test_true(instruction_test(inst)),map);
	DeduceGotos(test_false(instruction_test(inst)),map);
	break;
    case is_instruction_loop:
	DeduceGotos(loop_body(instruction_loop(inst)),map);	     
	break;
    case is_instruction_goto:
	instruction_goto(inst)=
	    (statement) GET_STATEMENT_MAPPING(map,instruction_goto(inst));
	break;
    case is_instruction_call:
	/* nothing */
	break;
    case is_instruction_unstructured:
    {
	list blocks=NIL;
	control ct=unstructured_control(instruction_unstructured(inst));

	CONTROL_MAP(c,{DeduceGotos(control_statement(c),map);},ct,blocks);
	gen_free_list(blocks);

	break;
    }
    default:
	pips_error("hpfcompiler","unexpected instruction tag\n");
	break;
    }
}

/*
 * hpfcompilesequentialloop
 */
void hpfcompilesequentialloop(stat,hoststatp,nodestatp)
statement stat, *hoststatp, *nodestatp;
{
    loop
	the_loop=statement_loop(stat);
    statement
	body=loop_body(the_loop),
	hostbody,
	nodebody;
    range
	r=loop_range(the_loop);
    list
	locals=loop_locals(the_loop);
    entity
	label=loop_label(the_loop),
	index=loop_index(the_loop),
	nindex=NewVariableForModule(node_module,index),
	hindex=NewVariableForModule(host_module,index);
    expression
	lower=range_lower(r),
	upper=range_upper(r),
	increment=range_increment(r);
    
    pips_assert("hpfcompilesequentialloop",TRUE);

    hpfcompiler(body,&hostbody,&nodebody);
    
    if (hpfc_empty_statement_p(hostbody))
    {
	/*
	 * ???
	 *
	 * memory leak, hostbody is lost whatever it was.
	 */
	(*hoststatp)=make_continue_statement(entity_undefined);
    }
    else
    {
	(*hoststatp)=MakeStatementLike(stat,
				       is_instruction_loop,
				       get_host_gotos_map());
	instruction_loop(statement_instruction(*hoststatp))=
	  make_loop(hindex,
		    make_range(UpdateExpressionForModule(host_module,lower),
			       UpdateExpressionForModule(host_module,upper),
			    UpdateExpressionForModule(host_module,increment)),
		    hostbody,
		    label,
		    make_execution(is_execution_sequential,UU),
		    lNewVariableForModule(host_module,locals));
    }
    
    (*nodestatp)=MakeStatementLike(stat,
				   is_instruction_loop,
				   get_node_gotos_map());
    instruction_loop(statement_instruction(*nodestatp))=
	make_loop(nindex,
		  make_range(UpdateExpressionForModule(node_module,lower),
			     UpdateExpressionForModule(node_module,upper),
			     UpdateExpressionForModule(node_module,increment)),
		  nodebody,
		  label,
		  make_execution(is_execution_sequential,UU),
		  lNewVariableForModule(node_module,locals));
}


/*
 * hpfcompileparallelloop
 */
void hpfcompileparallelloop(stat, hoststatp, nodestatp)
statement stat, *hoststatp, *nodestatp;
{
    loop
	the_loop = statement_loop(stat);
    statement
	s,
	nodebody,
	body = loop_body(the_loop);
    instruction
	bodyinst = statement_instruction(body);
    entity
	label=loop_label(the_loop),
	index=loop_index(the_loop),
	nindex=NewVariableForModule(node_module,index);
    range
	r = loop_range(the_loop);
    expression
	lower=range_lower(r),
	upper=range_upper(r),
	increment=range_increment(r);
    
    pips_assert("hpfcompileparallelloop",
		execution_parallel_p(loop_execution(the_loop)));

    if ((instruction_loop_p(bodyinst)) &&
	(execution_parallel_p(loop_execution(instruction_loop(bodyinst)))))
    {
	hpfcompileparallelloop(body, &s, &nodebody);
    }
    else
    {
	hpfcompileparallelbody(body, &s, &nodebody);
    }
    
    (*hoststatp) = make_continue_statement(entity_undefined);
    (*nodestatp)=MakeStatementLike(stat,
				   is_instruction_loop,
				   get_node_gotos_map());
    instruction_loop(statement_instruction(*nodestatp))=
	make_loop(nindex,
		  make_range(UpdateExpressionForModule(node_module,lower),
			     UpdateExpressionForModule(node_module,upper),
			     UpdateExpressionForModule(node_module,increment)),
		  nodebody,
		  label,
		  make_execution(is_execution_sequential,UU),
		  NULL);
}

/*
 * hpfcompileparallelbody
 */
void hpfcompileparallelbody(body, hoststatp, nodestatp)
statement body, *hoststatp, *nodestatp;
{
    list
	lw = NULL,
	lr = NULL,
	li = NULL,
	ls = NULL,
	lbs = NULL;
    /*
     * ???
     * dependances are not surely respected respected in the definitions list...
     * should check that only locals variables, that are not replicated,
     * may be defined during the body of the loop...
     */
    FindRefToDistArrayInStatement(body, &lw, &lr);
    li = AddOnceToIndicesList(lIndicesOfRef(lw), lIndicesOfRef(lr));
    ls = FindDefinitionsOf(body, li);

    generate_parallel_body(body, &lbs, lw, lr);

    (*hoststatp) = NULL;
    (*nodestatp) = make_block_statement(gen_nconc(ls, lbs));
}

