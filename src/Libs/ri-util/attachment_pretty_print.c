#include <stdlib.h>
#include <stdio.h>

#include "genC.h"
#include "text.h"
#include "text-util.h"
#include "ri.h"
#include "ri-util.h"
#include "misc.h"
#include "properties.h"
#include "word_attachment.h"
#include "top-level.h"

/* To store the fact a prettyprinter ask for Emacs attachments: */
static bool is_emacs_pretty_print_asked;

enum {POSITION_UNDEFINED = -1};

#if 0
/* Mapping from an object to a name and from a name to an object: */
static hash_table names_of_almost_everything_in_a_module = NULL;
static hash_table names_to_almost_everything_in_a_module = NULL;

/* The unique (well, I hope so...) integer to name an object in PIPS
   from the extern world. It is not reset since it could be useful to
   name various instance of the same module for example. I hope that
   the 2^32 wrap around will not occur too quickly... :-( */
static int current_name_of_something = 1;
#endif

/* Just to keep the output file through the gen_recurse: */
static FILE * local_output_file;

/* Declare the various mapping between the words and attachments: */
GENERIC_LOCAL_FUNCTION(word_to_attachments_begin, word_to_attachments)
GENERIC_LOCAL_FUNCTION(word_to_attachments_end, word_to_attachments)


#if 0
bool
name_something(chunk * something)
{
    if (hash_defined_p(names_of_almost_everything_in_a_module,
		       (char *) something))
	/* Well, we have already named this object. Return the fact
           for gen_recurse: */
	return FALSE;
    
    hash_put(names_of_almost_everything_in_a_module,
	     (char *) something,
	     (char *) current_name_of_something);
    
    hash_put(names_to_almost_everything_in_a_module,
	     (char *) current_name_of_something,
	     (char *) something);

    current_name_of_something ++;

    pips_assert("current_name_of_something should not wrap up to 0 !",
		current_name_of_something != 0);
    
    /* Go on in the RI: */
    return TRUE;
}


void
name_almost_everything_in_a_module(statement module)
{
   pips_assert("names_of_almost_everything_in_a_module not NULL",
               names_of_almost_everything_in_a_module == NULL);
   pips_assert("names_to_almost_everything_in_a_module not NULL",
               names_to_almost_everything_in_a_module == NULL);

   names_of_almost_everything_in_a_module = hash_table_make(hash_pointer, 0);
   names_to_almost_everything_in_a_module = hash_table_make(hash_int, 0);
   
   gen_multi_recurse(module,
                     statement_domain, name_something, gen_null,
                     NULL);
}


void
free_names_of_almost_everything_in_a_module()
{
   hash_table_free(names_of_almost_everything_in_a_module);
   names_of_almost_everything_in_a_module = NULL;
   hash_table_free(names_to_almost_everything_in_a_module);
   names_to_almost_everything_in_a_module = NULL;
}
#endif


/* The translation functions between unique names and objects: */

/* Initialize some things related with the attachment of propertie. To
   be conservative, if a prettyprinter does not call
   begin_attachment_prettyprint(), all the attachment stuff is
   disable. Thus, old prettyprinter can go on without Emacs mode: */
void
begin_attachment_prettyprint()
{
    if (get_bool_property("PRETTYPRINT_ADD_EMACS_PROPERTIES")) {
	is_emacs_pretty_print_asked = TRUE;
	/*word_text_attachment_mapping = hash_table_make(hash_pointer, 0);*/
	/* Initialize the local mapings: */
	init_word_to_attachments_begin();
	init_word_to_attachments_end();
	/*name_almost_everything_in_a_module(mod_stat);*/
    }
}


/* Clean the things related with the attachment of properties: */
void
end_attachment_prettyprint()
{
    if (is_emacs_pretty_print_asked) {
	is_emacs_pretty_print_asked = FALSE;

	/* Strings in attachments should already have been freed by
	   print_sentence and attachement it-self (witout "s") by
	   output_an_attachment(): */
	close_word_to_attachments_begin();
	/* Should be OK since output_the_attachments_for_emacs() has
	   already unlinked the attachment from
	   ord_to_attachments_begin: */
	close_word_to_attachments_end();

	/*free_names_of_almost_everything_in_a_module();*/
    }
}


/* Use to create or extend the attachment list of a word: */
#define ADD_AN_ATTACHMENT_TO_A_MAPPING(a,			\
				       a_word,			\
				       a_mapping)		\
if (bound_##a_mapping##_p(a_word)) {				\
    attachments ats = load_##a_mapping((int) a_word);		\
    list l = attachments_attachment(ats);			\
    l = CONS(ATTACHMENT, a, l);					\
    attachments_attachment(ats) = l;				\
}								\
else {								\
    store_##a_mapping((int) a_word,				\
		      make_attachments(CONS(ATTACHMENT,		\
					    a,			\
					    NIL)));		\
}


/* Attach something to a word list, from begin_word to end_word: */
static void
attach_to_word_list(string begin_word,
		    string end_word,
		    attachee at) 
{
    /* We do not know the position of the attachement in the output
       file yet: */
    attachment a = make_attachment(at,
				   POSITION_UNDEFINED,
				   POSITION_UNDEFINED);

    debug_on("ATTACHMENT_DEBUG_LEVEL");
    debug(6, "attach_to_word_list",
	  "Attach attachment %#x (attachee %#x) from \"%s\" (%#x) to \"%s\" (%#x)\n",
	  (unsigned int) a, (unsigned int) at,
	  begin_word, (unsigned int) begin_word,
	  end_word, (unsigned int)end_word);
    ADD_AN_ATTACHMENT_TO_A_MAPPING(a,
				   begin_word,
				   word_to_attachments_begin);

    ADD_AN_ATTACHMENT_TO_A_MAPPING(a,
				   end_word,
				   word_to_attachments_end);
    debug_off();
}


/* Attach something to a sentence list: */
static void
attach_to_sentence_list(sentence begin,
			sentence end,
			attachee a)
{
    attach_to_word_list(first_word_of_sentence(begin),
			last_word_of_sentence(end),
			a);
}


/* Attach something to a sentence: */
static void
attach_to_sentence(sentence s,
		   attachee a)
{
    attach_to_sentence_list(s, s, a);
}


/* Attach something to a sentence up to the end of the given text: */
static void
attach_to_sentence_up_to_end_of_text(sentence s,
				    text t,
				    attachee a)
{
    attach_to_sentence_list(s, SENTENCE(CAR(gen_last(text_sentences(t)))), a);
}


/* Attach something to all a text: */
static void
attach_to_text(text t,
	       attachee a)
{
    attach_to_sentence_list(SENTENCE(CAR(text_sentences(t))),
			    SENTENCE(CAR(gen_last(text_sentences(t)))),
			    a);
}



/* The user interface: */

/* Attach a loop: */
void
attach_loop_to_sentence_up_to_end_of_text(sentence s,
				    text t,
				    loop l)
{
    if (is_emacs_pretty_print_asked)
	attach_to_sentence_up_to_end_of_text(s, t,
					     make_attachee(is_attachee_loop,
							   l));
}


/* Attach the PROGRAM/FUNCTION head: */
sentence
attach_head_to_sentence(sentence s,
			entity module)
{
    if (is_emacs_pretty_print_asked)
	attach_to_sentence(s, make_attachee(is_attachee_module_head, module));

    return s;
}


/* Attach a reference: */
void
attach_reference_to_word_list(string begin_word,
			      string end_word,
			      reference r)
{
    if (is_emacs_pretty_print_asked)
	attach_to_word_list(begin_word,
			    end_word,
			    make_attachee(is_attachee_reference, r));
}


/* Attach a decoration: */
void
attach_decoration_to_text(text t)
{
    if (is_emacs_pretty_print_asked)
	attach_to_text(t, make_attachee(is_attachee_decoration, UU));
}


/* Attach a preconditions decoration: */
void
attach_preconditions_decoration_to_text(text t)
{
    if (is_emacs_pretty_print_asked)
	attach_to_text(t, make_attachee(is_attachee_preconditions, UU));
}


/* Attach a transformers decoration: */
void
attach_transformers_decoration_to_text(text t)
{
    if (is_emacs_pretty_print_asked)
	attach_to_text(t, make_attachee(is_attachee_transformers, UU));
}


/* Try to find some attachments in the words that are printed out. If
   any, note the begin position: */
void
deal_with_sentence_word_begin(string a_word,
			      int position_in_the_output)
{
    if (is_emacs_pretty_print_asked) {	    
	debug_on("ATTACHMENT_DEBUG_LEVEL");

	debug(8, "deal_with_sentence_word_begin",
	      "Looking for \"%s\" (%#x) at %d\n",
	      a_word, a_word, position_in_the_output);

	if (bound_word_to_attachments_begin_p((int) a_word)) {
	    /* Well, this word is an attachment begin: */
	    list some_attachments =
		attachments_attachment(load_word_to_attachments_begin((int) a_word));
	    debug(4, "deal_with_sentence_word_begin", "begin = %d:\n",
		  position_in_the_output);

	    MAP(ATTACHMENT, an_attachment,
		{
		    debug(4, "deal_with_sentence_word_begin",
			  "attachment = %#x:\n", an_attachment);

		    /* Remind the position of the begin of the
		       attachment: */
		    attachment_begin(an_attachment) = position_in_the_output;
		},
		some_attachments);
	};
    
	debug_off();
    }
}


/* Try to find some attachments in the words that are printed out. If
   any, note the end position: */
void
deal_with_sentence_word_end(string a_word,
			    int position_in_the_output)
{
    if (is_emacs_pretty_print_asked) {	    
	debug_on("ATTACHMENT_DEBUG_LEVEL");

	debug(8, "deal_with_sentence_word_end",
	      "Looking for \"%s\" (%#x) at %d\n",
	      a_word, a_word, position_in_the_output);

	if (bound_word_to_attachments_end_p((int) a_word)) {
	    /* Well, this word is an attachment end: */
	    list some_attachments = attachments_attachment(load_word_to_attachments_end((int) a_word));
	    debug(4, "deal_with_sentence_word_end",
		  "end = %d:\n", position_in_the_output);

	    MAP(ATTACHMENT, an_attachment,
		{
		    debug(4, "deal_with_sentence_word_end",
			  "attachment = %#x:\n", an_attachment);

		    /* Remind the position of the end of the
		       attachment: */
		    attachment_end(an_attachment) = position_in_the_output;
		},
		some_attachments);
	};
    
	debug_off();
    }
}



/* Output an attachment to the output file: */
static bool
output_an_attachment(attachment a)
{
    attachee at = attachment_attachee(a);
    int begin = attachment_begin(a);
    int end = attachment_end(a);
    
    pips_debug(5, "begin: %d, end: %d, attachment %#x (attachee %#x)\n",
	       begin, end, (unsigned int) a, (unsigned int) at);

    pips_assert("begin and end should be initialized.",
		begin != POSITION_UNDEFINED && end != POSITION_UNDEFINED);
		
    /* Begin an Emacs Lisp properties: */
    fprintf(local_output_file, "\n\t\t%d %d (", begin, end);

    switch(attachee_tag(at))
    {
    case is_attachee_reference:
	{	    
	    reference r = attachee_reference(at);
	    pips_debug(5, "\treference %#x\n", (unsigned int) r);
	    /* Output the address as a string because Emacs cannot
               store 32-bit numbers: */
	    fprintf(local_output_file, "face epips-face-reference epips-property-reference \"%#x\"",
		    (unsigned int) r);
	    break;
	}
	
    case is_attachee_loop: 
	{	    
	    loop l = attachee_loop(at);
	    pips_debug(5, "\tloop %#x\n", (unsigned int) l);
	    if (execution_parallel_p(loop_execution(l)))
		fprintf(local_output_file, "face epips-face-parallel-loop epips-property-loop \"%#x\"",
			(unsigned int) l);
	    break;
	}
	
    case is_attachee_module_head:
	{
	    entity head = attachee_module_head(at);
	    pips_debug(5, "\tmodule_head %#x\n", (unsigned int) head);
	    fprintf(local_output_file,
		    "face epips-face-module-head epips-module-head-name \"%s\"",
		    module_local_name(head));
	    break;
	}

    case is_attachee_decoration:
	{
	    pips_debug(5, "\tdecoration\n");
	    fprintf(local_output_file,
		    "invisible epips-invisible-decoration");
	    break;
	}

    case is_attachee_preconditions:
	{
	    pips_debug(5, "\tpreconditions\n");
	    fprintf(local_output_file,
		    "face epips-face-preconditions invisible epips-invisible-preconditions");
	    break;
	}

    case is_attachee_transformers:
	{
	    pips_debug(5, "\ttransformers\n");
	    fprintf(local_output_file,
		    "face epips-face-transformers invisible epips-invisible-transformers");
	    break;
	}

    default:
	pips_assert("attachee_tag inconsistent", FALSE);
    }

    /* End an Emacs Lisp properties: */
    fprintf(local_output_file, ")");

    /* We do not want to go on the recursion: */
    return FALSE;
}


/* Nothing to do... */
static void
rewrite_an_attachment(attachment a)
{
}


/* Begin the Emacs Lisp file: */
static void
init_output_the_attachments_for_emacs(FILE * output_file)
{
    /* Begin a string with Emacs Lisp properties: */
    fprintf(output_file, "(insert\n\t#(\"");
}


/* Output the list of all the attachments found in the text file with
   Emacs Lisp syntax: */
static void
output_the_attachments_for_emacs(FILE * output_file)
{
    debug_on("ATTACHMENT_DEBUG_LEVEL");

    local_output_file = output_file;
    
    /* End the string part: */
    fprintf(local_output_file, "\"");

    /* Enumerate all the attachments: */
    gen_recurse(word_to_attachments_begin,
		attachment_domain,
		output_an_attachment,
		rewrite_an_attachment);
    /* Just for fun, the previous gen_recurse will also recurse
       through the words of the mapping but it is not deep and thus it
       does not worth using a gen_multi_recurse with a string_domain
       and a filter returning always stop to avoid this unnecessary
       recursion. */

    WORD_TO_ATTACHMENTS_MAP(word_pointer, attachment_list,
    {
	pips_debug_function(6, "Key %#x, value %#x\n",
			    (unsigned int) word_pointer,
			    (unsigned int) attachment_list);
	/* Unlink the attachment from
	   attachments to avoid double free
	   later since referenced both by
	   word_to_attachments_begin and
	   word_to_attachments_end: */
	MAPL(ats,
	     {
		 ATTACHMENT(CAR(ats)) =
		     attachment_undefined;
	     },
	     attachments_attachment(attachment_list));
    },
       word_to_attachments_begin);

    /* End the property part: */
    fprintf(local_output_file, "\n\t)\n)\n");

    debug_off();
}


    /* Add the attachment in Emacs mode by creating a twin file that
       is decorated with Emacs properties: */
void
write_an_attachment_file(string file_name)
{
    if (is_emacs_pretty_print_asked) {
	FILE * file_stream;
	char * emacs_file_name = strdup(concatenate(file_name, EMACS_FILE_EXT, NULL));
	FILE * emacs_file_stream = safe_fopen(emacs_file_name, "w");
	init_output_the_attachments_for_emacs(emacs_file_stream);
	/* Now include the original plain file: */
	file_stream = safe_fopen(file_name, "r");
	for(;;) {
	    char c = getc(file_stream);
	    /* Strange semantics: must have read the character first: */
	    if (feof(file_stream))
		break;
	    
	    /* Just backslashify the '"' and '\': */
	    if (c == '"' || c == '\\')
		(void) putc('\\', file_stream);
	    
	    (void) putc(c, emacs_file_stream);	  
	}
	safe_fclose(file_stream, file_name);
	/* Actually add the interesting stuff: */
	output_the_attachments_for_emacs(emacs_file_stream);
	safe_fclose(emacs_file_stream, emacs_file_name);
	free(emacs_file_name);
    }
}
