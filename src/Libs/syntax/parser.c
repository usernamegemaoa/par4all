#include <stdio.h>
#include <string.h>

#include "genC.h"
#include "ri.h"
#include "ri-util.h"
#include "parser_private.h"

#include "syntax.h"

#include "resources.h"
#include "database.h"
#include "makefile.h"

#include "misc.h"
#include "pipsdbm.h"

/*
 * declarations of extern variables
 */
/* name of the current file */
char *CurrentFN;

/* the current function */
/* entity CurrentFunction; */

/* list of formal parameters of the current function  */
cons *FormalParameters; 

/* the name of the current package */
char *CurrentPackage; 

/* two areas that will be part of variable and common storage */
entity DynamicArea, StaticArea;

/* the current debugging level. see debug.h */
int debugging_level = 0;

/* where the current instruction starts and ends. its label */
int line_b_I, line_e_I, line_b_C, line_e_C;
char lab_I[6];

/* a string that will contain the value of the format in case of format
statement */
char FormatValue[FORMATLENGTH];



void ParserError(char * f, char * m)
{
    /* reset lex... Might be better to read the whole file like sserror() */
    extern char sssbuf[];
    extern char * sssptr;
    extern int ssprevious;

    sssptr = sssbuf;
# define MMNEWLINE 10
    ssprevious = MMNEWLINE;

    ResetBlockStack();
    reset_current_module_entity();

    debug_off();
    user_error(f,"Parser error between lines %d and %d\n%s\n",
	       line_b_I,line_e_I,m);
}


/* this function is called for each new file */
void BeginingOfParsing()
{
    static bool called = FALSE;

    if (called)
	return;

    /* the current package is initialized */
    CurrentPackage = TOP_LEVEL_MODULE_NAME;

    called = TRUE;
}

/* parse "module.dbr_file"
 */
static bool 
the_actual_parser(
    string module,
    string dbr_file)
{
    extern void ssparse();
    debug_on("SYNTAX_DEBUG_LEVEL");

    /* parser is initialized */
    BeginingOfParsing();

    /* scanner is initialized */
    ScanNewFile();

    pips_assert("parser", CurrentFN==NULL);
    CurrentFN = 
	strdup(concatenate(db_get_current_workspace_directory(),
			   "/",
			   db_get_file_resource(dbr_file, module, TRUE),
			   NULL));

    /* yacc parser is called */
    ssin = safe_fopen(CurrentFN, "r");
    ssparse();
    safe_fclose(ssin, CurrentFN);
    free(CurrentFN);
    CurrentFN = NULL;

    /* This debug_off() occurs too late since pipsdbm has been called
     * before. Initially, the parser was designed to parse more than
     * one subroutine/function/program at a time.
     */
    debug_off();

    return TRUE;
}

/* parser for HPFC.
 * just a different input file not to touch the original source file.
 * this parser should be selected/activated automatically.
 */
bool hpfc_parser(string module)
{
    return the_actual_parser(module, DBR_HPFC_FILTERED_FILE);
}

bool parser(string module)
{
    return the_actual_parser(module, DBR_SOURCE_FILE);
}
