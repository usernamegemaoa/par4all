/* Try to simplify the control graph.

   Ronan Keryell, 1995.
   */
/* 	%A% ($Date: 1997/03/11 15:35:57 $, ) version $Revision$, got on %D%, %T% [%P%].\n Copyright (c) �cole des Mines de Paris Proprietary.	 */

#ifndef lint
char vcid_unspaghettify[] = "%A% ($Date: 1997/03/11 15:35:57 $, ) version $Revision$, got on %D%, %T% [%P%].\n Copyright (c) �cole des Mines de Paris Proprietary.";
#endif /* lint */

#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 

#include "genC.h"
#include "ri.h"
#include "ri-util.h"
#include "text-util.h"
#include "database.h"
#include "misc.h"
#include "pipsdbm.h"
#include "resources.h"
#include "properties.h"
#include "control.h"


typedef enum {
    STRUCTURED_NULL_IF = 1901,
    STRUCTURED_IF_THEN = 1966,
    STRUCTURED_IF_ELSE = 1997,
    STRUCTURED_IF_THEN_ELSE = 2000
} structured_test_type;


static bool currently_apply_test_restructuring;
static bool currently_apply_recursive_decomposition;

/* To print some statistics about control graph restructuration: */
static int number_of_restructured_if_then;
static int number_of_restructured_if_else;
static int number_of_restructured_if_then_else;
static int number_of_restructured_null_if;


static void
initialize_unspaghettify_statistics()
{
    number_of_restructured_if_then = 0;
    number_of_restructured_if_else = 0;
    number_of_restructured_if_then_else = 0;
    number_of_restructured_null_if = 0;
}


static void
display_unspaghettify_statistics()
{
    if (currently_apply_test_restructuring
	&& get_bool_property("UNSPAGHETTIFY_DISPLAY_STATISTICS")) {
	int number_of_restructured_tests = number_of_restructured_if_then
	    + number_of_restructured_if_else
		+ number_of_restructured_if_then_else
		    + number_of_restructured_null_if;
	if (number_of_restructured_tests != 0) {
	    user_log("%d test%s %s been restructured:\n",
		     number_of_restructured_tests,
		     number_of_restructured_tests > 1 ? "s" : "",
		     number_of_restructured_tests > 1 ? "have" : "has");
	    user_log("\t %d IF/THEN/ELSE, %d IF/THEN, %d IF/ELSE & %d null IF.\n",
		     number_of_restructured_if_then_else,
		     number_of_restructured_if_then,
		     number_of_restructured_if_else,
		     number_of_restructured_null_if);
	}
    }
}


/* Add an edge between 2 control nodes.
   Assume that this edge does not already exist. */
void
link_2_control_nodes(control source,
		     control target)
{
    control_successors(source) = CONS(CONTROL,
				      target,
				      control_successors(source));
    control_predecessors(target) = CONS(CONTROL,
					source,
					control_predecessors(target));
}


/* Remove an edge between 2 control nodes.
   Assume that this edge does already exist. */
void
unlink_2_control_nodes(control source,
		       control target)
{
    gen_remove(&control_successors(source), target);
    gen_remove(&control_predecessors(target), source);
}


/* Fuse a 2 control node and add the statement of the second one to
   the statement of the first one. Assumes that the second node is the
   only successor of the first one:\. Do not update the entry or exit
   field of the unstructured. */
void
fuse_2_control_nodes(control first,
		     control second)
{
    if (gen_length(control_successors(second)) == 2) {
	/* If the second node has 2 successors, it is a test node. The
	   fused node has 2 successors and must be a test too. So, the
	   only thing I can do is to remove the first statement, just
	   keeping its comments: */
	string first_comment =
	    gather_all_comments_of_a_statement(control_statement(first));
	insert_comments_to_statement(control_statement(second),
				     first_comment);
	control_statement(first) = control_statement(second);	
    }
    else {
	/* If not, build a block with the 2 statements: */
	statement st = make_empty_statement();
	statement_instruction(st) =
	    make_instruction_block(CONS(STATEMENT,
					control_statement(first),
					CONS(STATEMENT,
					     control_statement(second), NIL)));
	/* Reconnect the new statement to the node to fuse: */
	control_statement(first) = st;
    }
    control_statement(second) = statement_undefined;

    /* Unlink the second node from the first one: */
    gen_free_list(control_successors(first));
    gen_remove(&control_predecessors(second), first);
	       
    /* Link the first node with the successors of the second one in
       the forward direction: */
    control_successors(first) =
	control_successors(second);
    /* Update all the predecessors of the successors: */
    MAP(CONTROL, c,
	{
	    MAPL(cp,
		 {
		     if (CONTROL(CAR(cp)) == second)
			 CONTROL(CAR(cp)) = first;
		 }, control_predecessors(c));
	}, control_successors(first));
	       
    /* Transfer the predecessors of the second node to the first one.
       Note that the original second -> first predecessor link has
       already been removed. But a loop from second to second appear
       at this point as a link from first to second. Nasty bug... */
    MAP(CONTROL, c,
	{
	    control_predecessors(first) = CONS(CONTROL,
					       c,
					       control_predecessors(first));
	    MAPL(cp,
		 {
		     if (CONTROL(CAR(cp)) == second) {
			 CONTROL(CAR(cp)) = first;
		     }
		 }, control_successors(c));
	}, control_predecessors(second));
    
    /* Now we remove the useless intermediate node "second": */
    /* Do not gen_free_list(control_successors(second)) since it is
       used as control_successors(first) now: */
    control_successors(second) = NIL;
    gen_free_list(control_predecessors(second));
    control_predecessors(second) = NIL;
    free_control(second);
}


/* Remove a useless continue in a sequence of control since it is
   typically the kind of useless things generated by the
   controlizer... */
static void
remove_useless_continue_or_empty_code_in_unstructured(unstructured u)
{
   list blocs = NIL;
   list remove_continue_list = NIL;
   
   /* The entry point of the unstructured: */
   control entry_node = unstructured_control(u);
   /* and its exit point: */
   control exit_node = unstructured_exit(u);

   pips_debug(7, "Dealing with unstructured %#x (begin: %#x, end: %#x)\n",
	      (int) u, (int) entry_node, (int) exit_node);     

   CONTROL_MAP(c,
               {
		   ifdebug (1)
                     pips_assert("control inconsistants...",
                                 gen_consistent_p(c));

                  /* Do not remove the exit nor the entry node node
                     since it is boring to relink the entry and exit
                     node... That is not important since there is
                     another pass that fuse the sequences. Dead code
                     elimination should remove these structured
                     CONTINUE afterward... */
                  if (c != exit_node && c != entry_node)
                     if (gen_length(control_successors(c)) == 1) {
                        /* Do not deal with any number of predecessor
                           since we do not want to remove the node 100
                           in :
                           goto 100
                           100 goto 200
                           200 goto 100

                           There may be also some unreachable
                           continues, that are without predecessors
                           (0)... We want to remove them also. 

Well with this modification, I am not sure that this procedure is
still useful...

                           So only 0 or 1 predecessor: */
                        if (gen_length(control_predecessors(c)) <= 1) {
                           statement st = control_statement(c);

			   pips_debug(7, "\tNode %#x has candidate links\n",
				      (int) c);
                           if (empty_statement_or_continue_without_comment_p(st)) {
                              /* It is some useless code with no
                                 comment to spare: put it in the
                                 remove list: */
			       remove_continue_list = CONS(CONTROL,
							   c,
							   remove_continue_list);
			       pips_debug(7, "\tNode %#x to be removed\n",
					  (int) c);     
                           }
                        }
                     }
               },
                  entry_node,
                  blocs);
   gen_free_list(blocs);

   /* Now we have the list of the control node to discard. Do the
      cleaning: */
   MAPL(a_control_list,
        {
           control c = CONTROL(CAR(a_control_list));

           debug(3, "remove_useless_continue_or_empty_code_in_unstructured",
                 "Remove control 0x%x\n", c);           
           remove_a_control_from_an_unstructured(c);
        },
           remove_continue_list);

   gen_free_list(remove_continue_list);
}


/* The controlizer of PIPS seems to have a bug:
   it put an empty successor node to the so-called exit node.
   Correct this fact: */
static void
clean_up_exit_node(unstructured u)
{
   control exit_node = unstructured_exit(u);
   list l = control_successors(exit_node);
   
   if (gen_length(l) == 1) {
      control c = CONTROL(CAR(l));
      pips_assert("clean_up_exit_node",
                  gen_length(control_successors(c)) == 0
                  && gen_length(control_predecessors(c)) == 1
                  && empty_statement_or_continue_p(control_statement(c)));

      /* Remove the useless node: */
      CONTROL(CAR(control_predecessors(c))) = control_undefined;
      free_control(c);
      
      /* Now the exit node has no longer a successor: */
      control_successors(exit_node) = NIL;
   }

   pips_assert("clean_up_exit_node",
               gen_length(control_successors(exit_node)) == 0);
}


/* Try to transform each sequence in a single statement instead of a
   list of control nodes: */
static void
fuse_sequences_in_unstructured(statement s)
{
   control the_successor;
   unstructured u = instruction_unstructured(statement_instruction(s));
   list blocs = NIL;

   /* The entry point of the unstructured: */
   control entry_node = unstructured_control(u);
   /* and its exit point: */
   control exit_node = unstructured_exit(u);

   /* To store the list of the controls to fuse we use a mapping since
      we need to keep track of eventual previous fuse on a control: */
   hash_table controls_to_fuse_with_their_successors =
      hash_table_make(hash_pointer, 0);
   
   ifdebug (1)
       pips_assert("unstructured inconsistants...",
		   gen_consistent_p(u));
   pips_debug(5, "Unstructured %x\n", (unsigned int) u);
   
   CONTROL_MAP(c,
               {
                  int number_of_successors_of_the_successor;
                  int number_of_predecessors_of_the_successor;
   
                  ifdebug (1)
                     pips_assert("control inconsistants...",
                                 gen_consistent_p(c));

                  /* Select a node with only one successor: */      
                  if (gen_length(control_successors(c)) == 1) {
                     the_successor = CONTROL(CAR(control_successors(c)));

                     number_of_successors_of_the_successor = gen_length(control_successors(the_successor));
                     number_of_predecessors_of_the_successor = gen_length(control_predecessors(the_successor));
                     debug(3, "fuse_sequences_in_unstructured",
                           "(gen_length(control_successors(c)) == 1), number_of_successors_of_the_successor = %d, number_of_predecessors_of_the_successor = %d, the successor is the entry node: %d, empty_statement_or_continue_p(control_statement(c)) = %d\n",
                           number_of_successors_of_the_successor,
                           number_of_predecessors_of_the_successor,
			   the_successor == entry_node,
			   empty_statement_or_continue_p(control_statement(c)));
                     if ((number_of_successors_of_the_successor <= 1
			  /* ...Accept the exit node */
			  && the_successor != entry_node
			  && number_of_predecessors_of_the_successor == 1)
			 ||
			 empty_statement_or_continue_p(control_statement(c)))
                        /* Ok, we have found a node in a
                           sequence. Note that we cannot fuse with the
                           entry node since it must keep this
                           role... */
			 /* But if c is empty, we can fuse it with any
                            successor without changing the semantincs,
                            even if the successor is the entry
                            node. */
                        /* The fact we can have a cycle is considered
                           a page below... */
                        /* Put the control in the fuse
                           list. Since no fusing occurs yet, the
                           address of a control node is itself: */
                        hash_put(controls_to_fuse_with_their_successors,
                                 (char *) c,
                                 (char *) c);
                  }
                  else {
                     debug(3, "fuse_sequences_in_unstructured",
                           "(gen_length(control_successors(c)) == %d)\n",
                           gen_length(control_successors(c)));
                  }
               },
                  entry_node,
                  blocs);
   gen_free_list(blocs);

   /* Now, since we have the list of the control nodes to fuse with
      their successors, do the fusion: */
   HASH_MAP(the_original_control,
            its_address_now,
            {
               /* Just for fun, the following line gets CPP lost
                  macro `HASH_MAP' used with too many (6) args
                  control a_control_to_fuse, its_successor, the_second_successor; */
               control a_control_to_fuse;
               control its_successor;
               char * old_address;
               /* Find the address of a control node to fuse even if
                  it has already been fused with predecessors through
                  a transitive closure: */
	       for(old_address = the_original_control;;) {
		   pips_debug(5, "Control %x (originally %x):\n",
			      (unsigned int) its_address_now,
			      (unsigned int) the_original_control);
		   if (old_address == its_address_now
		       /* ...The control node has not been moved */
		       || !hash_defined_p(controls_to_fuse_with_their_successors,
					  (char *) its_address_now)
		       /* ...or it has not been moved because it is
                          not a control node to fuse anyway. */
		       )
		       /* Ok, the node has been located: */
		       break;
		   else {
		       /* Follow a former control movement: */
		       old_address = its_address_now;
		       its_address_now
			   = hash_get(controls_to_fuse_with_their_successors,
				      (char *) its_address_now);
		   }
	       }
               a_control_to_fuse = (control) its_address_now;
	       
	       ifdebug(5)
		   print_text(stderr, text_statement(get_current_module_entity(), 0, control_statement(a_control_to_fuse)));
	       ifdebug(6) {
		   pips_debug(5, "All the unstructured %x:\n",
			      (unsigned int) u);
		   print_text(stderr, text_statement(get_current_module_entity(), 0, s));
	       }
	       ifdebug (3)
		   fprintf(stderr, "Want to fuse control %x",
			   (unsigned int) a_control_to_fuse);
               ifdebug (1)
		   pips_assert("control a_control_to_fuse inconsistants...",
			       gen_consistent_p(a_control_to_fuse));
               
               its_successor = CONTROL(CAR(control_successors(a_control_to_fuse)));
	       ifdebug (3)
		   fprintf(stderr, " with control %x\n",
			   (unsigned int) its_successor);
               ifdebug (1)
		   pips_assert("control its_successor inconsistants...",
			       gen_consistent_p(its_successor));

               if (a_control_to_fuse == its_successor)
                  debug(3, "fuse_sequences_in_unstructured",
                        "\tA loop of control has been found... Do not fuse the control %#x\n", (int) a_control_to_fuse);     

               else {
                  /* Well, it seems to be a real sequence, at most a
                     loop with 2 control nodes... */
		  debug(3, "fuse_sequences_in_unstructured",
                        "\tOk fuse them.\n");

		  fuse_2_control_nodes(a_control_to_fuse, its_successor);
                  /* make st with the statements of 2 following nodes: */

		  if (its_successor == entry_node)
		      /* Update the entry node if we fuse with it: */
		      entry_node = a_control_to_fuse;		      
		  if (its_successor == exit_node)
		      exit_node = a_control_to_fuse;
		  
		  /* If the node "its_successor" is in the fuse list,
                     we want to keep track of its new place, that is
                     in fact fused in "a_control_to_fuse", so that an
                     eventual fusion will use "a_control_to_fuse"
                     instead of "its_successor": */
                  if (hash_defined_p(controls_to_fuse_with_their_successors,
                                     (char *) its_successor)) {
                     /* Ok, "its_successor" is a node to fuse or that has
                        been already fused (in this case the following is
                        useless, but anyway...). Thus we keep track of
                        its new location: */
                     hash_update(controls_to_fuse_with_their_successors,
                                 (char *) its_successor,
                                 (char *) a_control_to_fuse);
                  }
               }
	       ifdebug (1)
		   pips_assert("control after fuse inconsistants...",
			       gen_consistent_p(a_control_to_fuse));

            },
               controls_to_fuse_with_their_successors);

   /* Update the potentially modified entry and exit nodes: */
   unstructured_control(u)= entry_node;
   unstructured_exit(u)= exit_node;

   hash_table_free(controls_to_fuse_with_their_successors);
}


/* Take the entry node out the unstructured if it is not useful, such
   as not an IF or a node without predecessor.

   Return TRUE if there is still an unstructured, FALSE if the
   unstructured has been replaced by a structured statement.

   If the first node is taked out, then * new_unstructured_statement
   returns the new statement that own the unstructured, else s. */
bool static
take_out_the_entry_node_of_the_unstructured(statement s,
                                            statement * new_unstructured_statement)
{
    instruction i = statement_instruction(s);
    unstructured u = instruction_unstructured(i);
    control entry_node = unstructured_control(u);
    list entry_node_successors = control_successors(entry_node);
    int entry_node_successors_length = gen_length(entry_node_successors);
    *new_unstructured_statement = s;
   
    if (entry_node_successors_length == 2
	|| gen_length(control_predecessors(entry_node)) > 0)
	/* Well, this node is useful here since it is an unstructured IF
	   or there is a GOTO on it. */
	return TRUE;

    if (entry_node_successors_length == 0) {
	/* In fact the unstructured has only one control node! Transform
	   it in a structured block of statements: */
	list l = CONS(STATEMENT, control_statement(entry_node), NIL);
	/* Since the unstructured may have a comment on it, we cannot
	   put the comment on the statement block but on a CONTINUE: */
	if (! empty_comments_p(statement_comments(s))) {
	    statement cs = make_continue_statement(entity_empty_label());
	    statement_comments(cs) = statement_comments(s);
	    statement_comments(s) = empty_comments;
	    l = CONS(STATEMENT, cs, l);
	}
	statement_instruction(s) = make_instruction_block(l);
	/* Remove the unstructured: */
	control_statement(entry_node) = statement_undefined;
	free_instruction(i);
	/* No longer unstructured: */
	return FALSE;
    }
    else {
	/* Take out the entry node: */
	*new_unstructured_statement = make_stmt_of_instr(i);
	statement_instruction(s) =
	    make_instruction_block(CONS(STATEMENT,
					control_statement(entry_node),
					CONS(STATEMENT,
					     *new_unstructured_statement,
					     NIL)));
	/* The entry node is now the second node: */
	pips_assert("take_out_the_entry_node_of_the_unstructured",
		    entry_node_successors_length == 1);
	unstructured_control(u) = CONTROL(CAR(entry_node_successors));
      
	discard_a_control_sequence_without_its_statements(entry_node,
							  entry_node);
	return TRUE;
    }
}


/* If the unstructured begin and/or end with a sequence, move the
   sequence(s) out of the unstructured and return a new statement with
   an equivalent but more structured code. It returns TRUE if the
   unstructured has disappeared and else FALSE.

   If the unstructured is still here, the statement directly owning it
   is returned in new_unstructured_statement: */

/* Still buggy. No longer used. */
static bool
try_to_structure_the_unstructured(statement s,
                                  statement * new_unstructured_statement)
{
   control end_of_first_sequence, c;
   list the_successors, the_predecessors;
   list begin_statement_list = NIL;
   list end_statement_list = NIL;
   control begin_of_last_sequence = control_undefined /* no gcc warning */;
    instruction i = statement_instruction(s);
    unstructured u = instruction_unstructured(i);
   
   /* The entry point of the unstructured: */
   control entry_node = unstructured_control(u);
   /* and its exit point: */
   control exit_node = unstructured_exit(u);

   /* Find the biggest sequence from the begin: */
   if (entry_node == exit_node)
      /* Well, we have an unstructured with one control node, it is
         still a sequence: */
      end_of_first_sequence = entry_node;
   else {
      end_of_first_sequence = control_undefined;
      for(c = entry_node;
          gen_length(the_successors = control_successors(c)) <= 1
             && gen_length(control_predecessors(c)) <= 1;
          c = CONTROL(CAR(the_successors))) {
         end_of_first_sequence = c;
         if (the_successors == NIL)
            /* It is in fact the exit_node: */
            break;
      }
   }
   
   if (end_of_first_sequence != control_undefined)
      /* OK, there is a sequence at the beginning of the unstructured: */
      begin_statement_list =
         generate_a_statement_list_from_a_control_sequence(entry_node,
                                                           end_of_first_sequence);
   
   /* If there is still something in the sequence: */
   if (end_of_first_sequence != exit_node) {
      /* Find the biggest sequence from the end: */
      begin_of_last_sequence = control_undefined;
      for(c = exit_node;
          gen_length(the_predecessors = control_predecessors(c)) == 1
             && gen_length(control_successors(c)) <= 1;
          c = CONTROL(CAR(the_predecessors)))
         begin_of_last_sequence = c;

      /* In fact, an unstructured IF is noted as a control node with 2
         successors, that means that we cannot take the biggest
         sequence since it may move one successor of such an IF and
         PIPS would be out. */
      if (begin_of_last_sequence != control_undefined
          && gen_length(control_successors(c)) == 2) {
         /* OK, it is actually an IF before the end sequence. Try to
            keep a spare node from the sequence. Since the
            controllizer seems to put always a continue as an IF
            successor, it is *seems* sensible. */
         the_successors = control_successors(begin_of_last_sequence);
         
         if (the_successors != NIL)
            /* There is one successor, that is the sequence has at
               least 2 control node. Keep the first node as part as IF
               unstructured: */
            begin_of_last_sequence = CONTROL(CAR(the_successors));
         else
            /* There is only one node in the sequence. It remains in
               the unstructured: */
            begin_of_last_sequence = control_undefined;
      }
      
      if (begin_of_last_sequence != control_undefined)
         /* Then there is a sequence at the end of the unstructured: */
         end_statement_list =
            generate_a_statement_list_from_a_control_sequence(begin_of_last_sequence,
                                                              exit_node);
   }

   /* Now, restructure all the stuff only if there is something to do: */
   if (begin_statement_list != NIL || end_statement_list != NIL) {
      if (end_of_first_sequence == exit_node) {
         /* All the unstructured is a sequence, replace it with the
            equivalent block of statement statement: */
         statement_instruction(s) =
            make_instruction_block(begin_statement_list);

         /* Discard the unstructured instruction: */
         discard_an_unstructured_without_its_statements(u);

         /* Warn that the unstructured no longer exist: */
         *new_unstructured_statement = statement_undefined;
         return TRUE;
      }
      else {
         /* There is still an unstructured, but with one pre- or
            post-sequence: */
         list list_of_the_new_statements;
         /* Put the unstructured in the new statement list: */
         *new_unstructured_statement = make_stmt_of_instr(i);
         
         list_of_the_new_statements = CONS(STATEMENT,
                                           *new_unstructured_statement,
                                           NIL);
         
         if (begin_statement_list != NIL) {
            /* There is a pre-sequence before the unstructured: */

            /* Put the sequence before the unstructured: */
            list_of_the_new_statements = gen_nconc(begin_statement_list,
                                                   list_of_the_new_statements);
            
            /* Update the entry node of the unstructured: */
            unstructured_control(u) =
               CONTROL(CAR(control_successors(end_of_first_sequence)));
            
            /* Clean up the equivalent control sequence: */
            discard_a_control_sequence_without_its_statements(entry_node,
                                                              end_of_first_sequence);
         }

         if (end_statement_list != NIL) {
            /* There is a post-sequence after the unstructured: */
            list_of_the_new_statements = gen_nconc(list_of_the_new_statements,
                                                   end_statement_list);
            
            /* Update the exit node of the unstructured: */
            unstructured_exit(u) =
               CONTROL(CAR(control_predecessors(begin_of_last_sequence)));

            /* Clean up the equivalent control sequence: */
            discard_a_control_sequence_without_its_statements(begin_of_last_sequence, exit_node);
         }

         /* Make the instruction of the old statement with the
            sequence(s) and the stripped-down unstructured: */
         statement_instruction(s) =
            make_instruction_block(list_of_the_new_statements);
         
         return FALSE;
      }
   }
   /* By default the unstructured is not changed, thus return the
      statement owning it: */
   *new_unstructured_statement = s;
   
   return FALSE;
}


/* The exit node is the landing pad of the control graph. But if it is
   not a continue, that means that its statement is a sequence at the
   end of the unstructured and we can take it out of the
   unstructured. We just return the statement directly containing the
   unstructured. */
statement
take_out_the_exit_node_if_not_a_continue(statement s)
{
    /* To return and keep track of the unstructured: */
    statement the_unstructured = s;
    instruction i = statement_instruction(s);
    unstructured u = instruction_unstructured(i);
    control exit_node = unstructured_exit(u);
    statement the_exit_statement = control_statement(exit_node);
    instruction the_exit_instruction;

    pips_assert("take_out_the_exit_node_if_not_a_continue :"
		"i != statement_instruction(s) || u != instruction_unstructured(i) !",
		instruction_unstructured_p(i)
		&& u == instruction_unstructured(i));

    /* First, linearize the exit statement since
       fuse_sequences_in_unstructured() may have gathered many
       statements in a messy way: */
    clean_up_sequences_internal(the_exit_statement);
    the_exit_instruction =
	statement_instruction(the_exit_statement);

    /* Then normalize to have only one non-sequence statement as the
       exit node: */
    if (instruction_sequence_p(the_exit_instruction)) {
	list first_statement_list;
	statement first_statement, last_statements;
	
	list the_statements =
	    sequence_statements(instruction_sequence(the_exit_instruction));
	pips_assert("the_statements must be a true sequence",
		    gen_length(the_statements) >= 2);
	
	/* Well, this should be always true if the sequence
	   survived to clean_up_sequences_rewrite()... */
	first_statement_list = the_statements;
	first_statement = STATEMENT(CAR(first_statement_list));
	the_statements = CDR(the_statements);
	CDR(first_statement_list) = NIL;
	gen_free_list(first_statement_list);
	    
	last_statements = the_exit_statement;
	sequence_statements(instruction_sequence(the_exit_instruction)) =
	    the_statements;
	    
	control_statement(exit_node) = first_statement;
	/* Then, append the last statements at the end of the
	   unstructured: */
	the_unstructured = make_stmt_of_instr(i);
	statement_instruction(s) =
	    make_instruction_block(CONS(STATEMENT,
					the_unstructured,
					/* ...followed by the last
					   statements: */
					CONS(STATEMENT,
					     last_statements,
					     NIL)));
	/* Fix the variables for the following pass: */
	the_exit_statement = first_statement;
	the_exit_instruction = statement_instruction(the_exit_statement);
    }
    /* Here the_exit_statement is not a sequence. */
    if (! empty_statement_or_continue_p(the_exit_statement)
	&& ! return_statement_p(the_exit_statement)) {
	statement new_statement;
	statement out_keeping;
	/* Put an empty exit node and keep the statement for the
           label: */
	statement_instruction(the_exit_statement) =
	    make_continue_instruction();
	ifdebug (1)
	    pips_assert("Statements inconsistants...", gen_consistent_p(the_exit_statement));
	/* Replace the unstructured by an unstructured followed by the
	   out-keeped instruction: */
	new_statement = make_stmt_of_instr(i);
	out_keeping = make_stmt_of_instr(the_exit_instruction);
	/* Keep track of the statement number: */
	statement_number(out_keeping) = statement_number(the_exit_statement);
	statement_instruction(the_unstructured) =
	    make_instruction_block(CONS(STATEMENT,
					new_statement,
					CONS(STATEMENT, out_keeping, NIL)));
	the_unstructured = new_statement;
    }
    /* Heavily rely on a later clean_up_sequences to normalize the
       above... */

    ifdebug (1)
	pips_assert("Statements inconsistants...", gen_consistent_p(s));

    return the_unstructured;
}


static void
restructure_this_test(control c,
		      structured_test_type t)
{
    instruction the_test_statement = control_statement(c);
    test the_test = instruction_test(statement_instruction(the_test_statement));
    control then_node = CONTROL(CAR(control_successors(c)));
    control else_node = CONTROL(CAR(CDR(control_successors(c))));
    statement then_statement = control_statement(then_node);
    statement else_statement = control_statement(else_node);
    control test_exit;

    ifdebug(9) {
	pips_debug(9, "the test statement:\n");
	print_text(stderr, text_statement(get_current_module_entity(),
					  0, the_test_statement));
	pips_assert("Statements inconsistants...",
		    gen_consistent_p(the_test_statement));
    }

    /* Find the exit node of the test: */
    if (t == STRUCTURED_NULL_IF)
	test_exit = then_node;
    else if (t == STRUCTURED_IF_THEN || t == STRUCTURED_IF_THEN_ELSE)
	test_exit = CONTROL(CAR(control_successors(then_node)));
    else
	test_exit = CONTROL(CAR(control_successors(else_node)));
    pips_debug(9, "exit node=%#x\n", (unsigned int) test_exit);
		
    /* Discard and unlink the then_node and else_node if any: */
    if (t == STRUCTURED_IF_THEN || t == STRUCTURED_IF_THEN_ELSE)
	discard_a_control_sequence_without_its_statements(then_node,
							  then_node);
    if (t == STRUCTURED_IF_ELSE || t == STRUCTURED_IF_THEN_ELSE)
	discard_a_control_sequence_without_its_statements(else_node,
							  else_node);

    /* Now rebuild a structured test in the node c from the previous
       test branch contents: */
    if (t == STRUCTURED_IF_THEN || t == STRUCTURED_IF_THEN_ELSE) {
	free_statement(test_true(the_test));
	test_true(the_test) = then_statement;
	ifdebug(9) {
	    pips_debug(9, "then statement:\n");
	    print_text(stderr, text_statement(get_current_module_entity(),
					      0, then_statement));
	    pips_assert("Statements inconsistants...",
			gen_consistent_p(then_statement));
	}
    }
    if (t == STRUCTURED_IF_ELSE || t == STRUCTURED_IF_THEN_ELSE) {
	free_statement(test_false(the_test));
	test_false(the_test) = else_statement;
	ifdebug(9) {
	    pips_debug(9, "else statement:\n");
	    print_text(stderr, text_statement(get_current_module_entity(),
					      0, else_statement));
	    pips_assert("Statements inconsistants...",
			gen_consistent_p(else_statement));
	}
    }
    
    /* Replace the useless CONTINUE by a NOP to improve the
       prettyprinted code: */
    if (t == STRUCTURED_IF_THEN || t == STRUCTURED_NULL_IF) {
	free_statement(test_false(the_test));
	test_false(the_test) = make_empty_statement();
    }
    if (t == STRUCTURED_IF_ELSE || t == STRUCTURED_NULL_IF) {
	free_statement(test_true(the_test));
	test_true(the_test) = make_empty_statement();
    }

    if (t == STRUCTURED_NULL_IF)
	/* Remove one of the 2 branches, since it is symetrical. In
           fact, since unlink_2_control_nodes() removes all the
           redundant branches, need to relink later: */
	unlink_2_control_nodes(c, test_exit);
    if (t == STRUCTURED_IF_THEN_ELSE || t == STRUCTURED_NULL_IF) {
	/* Relink the structured test node to the successor since both
           pathes have been removed: */
	link_2_control_nodes(c, test_exit);
    }
    /* In the other case, the remaining link is just what needed. */
    
    ifdebug(9) {
	pips_debug(9, "the test statement:\n");
	print_text(stderr, text_statement(get_current_module_entity(),
					  0, the_test_statement));
	pips_assert("Statements inconsistants...",
		    gen_consistent_p(the_test_statement));
    }
}


/* Try to restructure the unstructured IF/THEN/ELSE.
   Assume that all the sequences has been previously fused.
   */
static bool
restructure_if_then_else(statement s)
{
    list blocs = NIL;
    bool code_modified_p = FALSE;
    unstructured u = instruction_unstructured(statement_instruction(s));
    /* The entry point of the unstructured: */
    control entry_node = unstructured_control(u);

    /* To store the tests that can be restructured with their test
       types: */
    hash_table structured_tests = hash_table_make(hash_int, 0);

    pips_assert("Control inconsistants...", gen_consistent_p(entry_node));

    /* First mark the IF that can be restructured: */
    CONTROL_MAP(c,
		{
		    /* Only look at test nodes: */
		    if (gen_length(control_successors(c)) == 2) {
			control then_node =
			    CONTROL(CAR(control_successors(c)));
			control else_node =
			    CONTROL(CAR(CDR(control_successors(c))));
			int then_node_fan_out =
			    gen_length(control_successors(then_node));
			/* Note that if a node is the entry node, its
                           fan in is once more! Nasty bug... */
			int then_node_fan_in =
			    gen_length(control_predecessors(then_node))
			    + (then_node == entry_node);
			int else_node_fan_out =
			    gen_length(control_successors(else_node));
			int else_node_fan_in =
			    gen_length(control_predecessors(else_node))
			    + (else_node == entry_node);

			if (then_node == else_node) {
			    /* We've found a structured null if, that
                               is with the then and else branches
                               pointing to the same target: */
			    hash_put(structured_tests,
				     (char *) c,
				     (char *) STRUCTURED_NULL_IF);
			    number_of_restructured_null_if++;
			}
			else if (then_node_fan_out == 1
				 && else_node_fan_out == 1
				 && then_node_fan_in == 1
				 && else_node_fan_in == 1
				 && CONTROL(CAR(control_successors(then_node))) == CONTROL(CAR(control_successors(else_node)))) {
			    /* We've found a structured if/then/else: */
			    hash_put(structured_tests,
				     (char *) c,
				     (char *) STRUCTURED_IF_THEN_ELSE);
			    number_of_restructured_if_then_else++;
			}
			else if (then_node_fan_out == 1
				 && then_node_fan_in == 1
				 && CONTROL(CAR(control_successors(then_node))) == else_node) {
			    /* We've found a structured if/then
                               without else: */
			    hash_put(structured_tests,
				     (char *) c,
				     (char *) STRUCTURED_IF_THEN);
			    number_of_restructured_if_then++;
			}
			else if (else_node_fan_out == 1
				 && else_node_fan_in == 1
				 && CONTROL(CAR(control_successors(else_node))) == then_node) {
			    /* We've found a structured if/else
                               without then: */
			    hash_put(structured_tests,
				     (char *) c,
				     (char *) STRUCTURED_IF_ELSE);
			    number_of_restructured_if_else++;
			}
		    }
		},
		entry_node,
		blocs);
    gen_free_list(blocs);

    debug(5, "restructure_if_then_else",
	  "%d if/then, %d if/else, %d if/then/else, %d null if\n",
	  number_of_restructured_if_then,
	  number_of_restructured_if_else,
	  number_of_restructured_if_then_else,
	  number_of_restructured_null_if);
    
    /* Then restructure if needed: */

    HASH_MAP(key, value,
	{
	    control c = (control) key;
	    structured_test_type t = (structured_test_type) value;
	    /* Hidden in a function to ease debugging... */
	    restructure_this_test(c, t);
	    /* The code has been modified: */
	    code_modified_p = TRUE;
	},
	structured_tests);

    /* Return the number of tests that has been restructured: */
    return code_modified_p;
}


/* Use an interval graph partitionning method to recursively
   decompose the control graph: */
void
control_graph_recursive_decomposition(statement s)
{
  /*
    graph interval = interval_graph(entry_node);
    */
}



/* Try to recursively restructure the unstructured: */
static void
recursively_restructure_an_unstructured(statement s)
{
    /* Used to keep track of the statement that contains directly the
       unstructured: */
    statement new_unstructured_statement;
    instruction i = statement_instruction(s);
    if (!instruction_unstructured_p(i))
	/* Just stop, it is no longer an unstructured. */	
	return;

    /* Replace control sequences by simple nodes: */
    fuse_sequences_in_unstructured(s);
    ifdebug(5) {
	pips_debug(5, "after fuse_sequences_in_unstructured\n");
	print_text(stderr, text_statement(get_current_module_entity(), 0, s));
	pips_assert("Statements inconsistants...", gen_consistent_p(s));
    }
    
    if (take_out_the_entry_node_of_the_unstructured(s, &new_unstructured_statement)) {
	/* If take_out_the_entry_node_of_the_unstructured() has not been
	   able to discard the unstructured, go on with some other
	   optimizations: */
	ifdebug(5) {
	    pips_debug(5,
		       "after take_out_the_entry_node_of_the_unstructured\n");
	    print_text(stderr, text_statement(get_current_module_entity(), 0, s));
	    pips_assert("Statements inconsistants...", gen_consistent_p(s));
	}

	new_unstructured_statement =
	    take_out_the_exit_node_if_not_a_continue(new_unstructured_statement);
	
	ifdebug(5) {
	    pips_debug(5, "after take_out_the_exit_node_if_not_a_continue\n");
	    print_text(stderr, text_statement(get_current_module_entity(), 0, s));
	    pips_assert("Statements inconsistants...", gen_consistent_p(s));
	}

	/* If we ask for, try to restructure the tests: */
	if (currently_apply_test_restructuring
	    && restructure_if_then_else(new_unstructured_statement)) {
	    ifdebug(5) {
		pips_debug(5, "after restructure_if_then_else\n");
		print_text(stderr, text_statement(get_current_module_entity(), 0, s));
		pips_assert("Statements inconsistants...", gen_consistent_p(s));
	    }
	    /* Well, some tests has been restructured, recurse: */
	    recursively_restructure_an_unstructured(new_unstructured_statement);
	}
    }
}


/* All optimizations for unstructured during the bottom-up phase */
void
unspaghettify_rewrite_unstructured(statement s)
{
    instruction i = statement_instruction(s);
    unstructured u = instruction_unstructured(i);
 
    clean_up_exit_node(u);
   
    remove_the_unreachable_controls_of_an_unstructured(u);

    ifdebug(5) {
	pips_debug(5, "after remove_the_unreachable_controls_of_an_unstructured\n");
	print_text(stderr, text_statement(get_current_module_entity(), 0, s));
    }

    remove_useless_continue_or_empty_code_in_unstructured(u);
   
    ifdebug(5) {
	pips_debug(5, "after remove_useless_continue_or_empty_code_in_unstructured\n");
	print_text(stderr, text_statement(get_current_module_entity(), 0, s));
    }
   
    recursively_restructure_an_unstructured(s);
       
    ifdebug(5) {
	pips_debug(5, "End.\n");
	print_text(stderr, text_statement(get_current_module_entity(), 0, s));
    }
}


/* This is the function that is applied on each statement of the code
   in a bottom-up way. Even if we deal with the control graph, that is
   the "unstructured" instruction, we need to deal with the statement
   over the unstructured since the restructuration can move some code
   outside of the unstructured in the statement. */
static void
unspaghettify_rewrite(statement s)
{
   instruction i = statement_instruction(s);

   pips_debug(2, "enter\n");
   ifdebug (3) {
      fprintf(stderr, "[ The current statement : ]\n");
      print_text(stderr, text_statement(get_current_module_entity(), 0, s));
   }

   if (instruction_unstructured_p(i)) {
      unspaghettify_rewrite_unstructured(s);
      ifdebug (5) {
         fprintf(stderr, "After dead_rewrite_unstructured:\n");
         print_text(stderr, text_statement(get_current_module_entity(), 0, s));
         fprintf(stderr, "-----\n");
      }
   }
   pips_debug(2, "exit\n");
}


/* Always go down in the module before doing something: */
static bool
unspaghettify_filter(statement s)
{
   return TRUE;
}


/* The entry point common to unspaghettify or restructure a module: */
void
unspaghettify_or_restructure_statement(statement mod_stmt)
{
   debug_on("UNSPAGHETTIFY_DEBUG_LEVEL");

   ifdebug (1)
      pips_assert("Statements inconsistants...", gen_consistent_p(mod_stmt));
   
   initialize_unspaghettify_statistics();
   gen_recurse(mod_stmt, statement_domain,
               unspaghettify_filter, unspaghettify_rewrite);
   display_unspaghettify_statistics();

   /* End by removing parasitic sequences: */
   clean_up_sequences(mod_stmt);
   
   ifdebug (1)
      pips_assert("Statements inconsistants...", gen_consistent_p(mod_stmt));

   pips_debug(2, "done\n");
   debug_off();
}


/* The real entry point of unspaghettify: */
void
unspaghettify_statement(statement mod_stmt)
{
   currently_apply_test_restructuring =
       get_bool_property("UNSPAGHETTIFY_TEST_RESTRUCTURING");
   currently_apply_recursive_decomposition =
       get_bool_property("UNSPAGHETTIFY_RECURSIVE_DECOMPOSITION");
   
   unspaghettify_or_restructure_statement(mod_stmt);
}


/* Try to simplify the control graph of a module by removing useless
   CONTINUEs, unreachable code. Try to structure a little bit more and
   so on according to some properties.

   Unspaguettify is now targetted to be included in the controlizer.
   */
bool
unspaghettify(char * mod_name)  
{
   statement mod_stmt;

   /* Get the true resource, not a copy. */
   mod_stmt = (statement) db_get_memory_resource(DBR_CODE, mod_name, TRUE);
   set_current_module_statement(mod_stmt);

   set_current_module_entity(local_name_to_top_level_entity(mod_name));
  
   unspaghettify_statement(mod_stmt);

   /* Reorder the module, because new statements may have been
      changed. */
   module_reorder(mod_stmt);

   DB_PUT_MEMORY_RESOURCE(DBR_CODE, strdup(mod_name), mod_stmt);

   reset_current_module_statement();
   reset_current_module_entity();

   return TRUE;
}


/* The real entry point of control graph restructuring: */
void
restructure_statement(statement mod_stmt)
{
   currently_apply_test_restructuring = TRUE;
   currently_apply_recursive_decomposition = TRUE;
   
   unspaghettify_or_restructure_statement(mod_stmt);
}


/* Try to simplify the control graph of a module by removing useless
   CONTINUEs, unreachable code, etc. as in unspaghettify.

   Do also test restructuring and control graph recursive graph
   decomposition.
   */
bool
restructure_control(char * mod_name)  
{
   statement mod_stmt;

   /* Get the true resource, not a copy. */
   mod_stmt = (statement) db_get_memory_resource(DBR_CODE, mod_name, TRUE);
   set_current_module_statement(mod_stmt);

   set_current_module_entity(local_name_to_top_level_entity(mod_name));
  
   restructure_statement(mod_stmt);

   /* Reorder the module, because new statements may have been
      changed. */
   module_reorder(mod_stmt);

   DB_PUT_MEMORY_RESOURCE(DBR_CODE, strdup(mod_name), mod_stmt);

   reset_current_module_statement();
   reset_current_module_entity();

   return TRUE;
}
