/* Deals with constant expressions and constant entities
 *
 * $Id$
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linear.h"

#include "genC.h"
#include "misc.h"
#include "ri.h"
#include "ri-util.h"

extern string make_entity_fullname();

/* 
  returns the top-level entity that represents the integer constant 1.
  its name is "TOP-LEVEL:1". 
  this entity must exist because it is necessarily created by the parser.
*/
entity 
find_entity_1()
{
    entity e_1;
    string n_1 = make_entity_fullname(TOP_LEVEL_MODULE_NAME, "1");

    e_1 = gen_find_tabulated(n_1, entity_domain);

    pips_assert("find_entity_1", e_1 != entity_undefined);

    return(e_1);
}

/*
  returns the integer constant expression of value 1.
*/
expression 
make_expression_1()
{
    entity e_1 = find_entity_1();

    return(make_expression(make_syntax(is_syntax_call, make_call(e_1, NIL)),
			   normalized_undefined));
}

int 
DefaultLengthOfBasic(t)
tag t;
{
	int e=-1;

	switch (t) {
	    case is_basic_overloaded:
		e = 0;
		break;
	    case is_basic_int:
		e = DEFAULT_INTEGER_TYPE_SIZE;
		break;
	    case is_basic_float:
		e = DEFAULT_REAL_TYPE_SIZE;
		break;
	    case is_basic_logical:
		e = DEFAULT_LOGICAL_TYPE_SIZE;
		break;
	    case is_basic_complex:
		e = DEFAULT_COMPLEX_TYPE_SIZE;
		break;
	    case is_basic_string:
		e = DEFAULT_CHARACTER_TYPE_SIZE;
		break;
	    default:
		pips_error("DefaultLengthOfBasic", "case default\n");
		break;
	}

	return(e);
}


/* BEGIN_EOLE */ /* - please do not remove this line */
/* Lines between BEGIN_EOLE and END_EOLE tags are automatically included
   in the EOLE project (JZ - 11/98) */


/* This function creates a constant. a constant is represented in our
internal representation by a function. Its name is the name of the
constant, its type is a functional that gives the type of the constant,
its storage is rom.

Its initial value is the value of the constant. In case of integer
constant, the actual value is stored (as an integer) in constant_int.
values of other constants have to be computed with the name, if
necessary.

name is the name of the constant 12, 123E10, '3I12', ...
Initial and final quotes are included in the names of string constants.

basic is the basic type of the constant: int, float, ... 
*/

entity 
make_constant_entity(
    string name,
    tag bt,
    size_t size)
{
    entity e;

    e = FindOrCreateEntity(TOP_LEVEL_MODULE_NAME, name);

    if (entity_type(e) == type_undefined) {
	functional fe;
	constant ce;
	basic be;
	
	if (bt == is_basic_string) {
	    be = make_basic(bt, (make_value(is_value_constant, 
					    (make_constant(is_constant_int, 
					      (void*) strlen(name)-2)))));
	}
	else {
	    be = make_basic(bt, (void*) size);
	}

	fe = make_functional(NIL, MakeTypeVariable(be, NIL));

	if (bt == is_basic_int)
	{
	  ce = make_constant(is_constant_int, (void*) atol(name));
	}
	else
	{
	  ce = make_constant(is_constant_call, e);
	}

	entity_type(e) = make_type(is_type_functional, fe);
	entity_storage(e) = MakeStorageRom();
	entity_initial(e) = make_value(is_value_constant, ce);
    }

    return(e);
}

/* END_EOLE */


entity 
MakeConstant(name, bt)
string name;
tag bt;
{
    entity e;

    e = make_constant_entity(name, bt, DefaultLengthOfBasic(bt));

    /* The LengthOfBasic should be updated for type "string" */

    return e;
}

/* make a complex constant from two calls to real or integer constants
 *
 * Problem: does not work if either of the components is negative because
 * negative constants are stored as expressions. For instance, (0, -1) is
 * not a complex constant for PIPS but an expression:
 * cmplx(0,unary_minus(1)).
 *
 * Note: I might have changed that to store DATA statements... (FI)
 */
entity 
MakeComplexConstant(r, i)
expression r;
expression i;
{
    entity re = call_function(syntax_call(expression_syntax(r)));
    entity ie = call_function(syntax_call(expression_syntax(i)));
    entity e;
    char * name = strdup(concatenate("(",entity_local_name(re), ",",
				     entity_local_name(ie),")", NULL));
    type rt = entity_type(re);
    type it = entity_type(ie);
    type ert = functional_result(type_functional(rt));
    type eit = functional_result(type_functional(it));
    basic rb = variable_basic(type_variable(ert));
    basic ib = variable_basic(type_variable(eit));
    int rsize = basic_type_size(rb);
    int isize = basic_type_size(ib);
    int size = rsize>isize? rsize: isize;

    e = make_constant_entity(name, is_basic_complex, size);
    /* name has to be allocated by strdup because of nested calls to
       concatenate */
    free(name);
    return e;
}

expression
MakeComplexConstantExpression(
			      expression r,
			      expression i)
{
    expression cce = expression_undefined;

    if(signed_constant_expression_p(r) && signed_constant_expression_p(i)) {
	basic rb = basic_of_expression(r);
	basic ib = basic_of_expression(i);
	int rsize = basic_type_size(rb);
	int isize = basic_type_size(ib);
	int size = rsize>isize? rsize: isize;

	cce = MakeBinaryCall(local_name_to_top_level_entity
			     (size==4? IMPLIED_COMPLEX_NAME: IMPLIED_DCOMPLEX_NAME), r, i);
    }

    return cce;
}

bool complex_constant_expression_p(expression cce)
{
  bool is_complex_constant_p = FALSE;
  if(expression_call_p(cce)) {
    entity f = call_function(syntax_call(expression_syntax(cce)));
    string fn = entity_local_name(f);

    is_complex_constant_p = (strcmp(fn, IMPLIED_COMPLEX_NAME)==0 
			     || strcmp(fn, IMPLIED_DCOMPLEX_NAME)==0);
  }

  return is_complex_constant_p;
}

/* this function creates an integer constant and then a call to that
constant. */

expression 
MakeIntegerConstantExpression(s)
string s;
{
    return(MakeNullaryCall(MakeConstant(s, is_basic_int)));
}

expression 
make_constant_boolean_expression(bool b)
{
    return MakeNullaryCall
        (MakeConstant(b ? TRUE_OPERATOR_NAME : FALSE_OPERATOR_NAME,
		      is_basic_logical));
}

expression 
int_expr(i)
int i;
{
    /* What should be the length of buffer? */
    char buffer[20];

    sprintf(buffer, "%d", i);
    return(MakeIntegerConstantExpression(buffer));
}

/* BEGIN_EOLE */ /* - please do not remove this line */
/* Lines between BEGIN_EOLE and END_EOLE tags are automatically included
   in the EOLE project (JZ - 11/98) */

/* (*int_p) gets integer constant if any */
bool 
integer_constant_p(ent, int_p)
entity ent;
int *int_p;
{
    if( type_tag(entity_type(ent))==is_type_functional
       && value_tag(entity_initial(ent))==is_value_constant
       && constant_tag(value_constant(entity_initial(ent)))==is_constant_int ) {
	*int_p = constant_int(value_constant(entity_initial(ent)));
	return(TRUE);
    }
    else
	return(FALSE);
}


/* (*int_p) gets integer constant if any */
bool 
integer_symbolic_constant_p(ent, int_p)
entity ent;
int *int_p;
{
    if( type_tag(entity_type(ent))==is_type_functional
       && value_tag(entity_initial(ent))==is_value_symbolic
       && constant_tag(symbolic_constant(value_symbolic(entity_initial(ent))))==is_constant_int ) {
	*int_p = constant_int(symbolic_constant(value_symbolic(entity_initial(ent))));
	return(TRUE);
    }
    else
	return(FALSE);
}

/* END_EOLE */

/* this function creates an character constant and then a call to that
constant. */

expression 
MakeCharacterConstantExpression(s)
string s;
{
    return(MakeNullaryCall(MakeConstant(s, is_basic_string)));
}


/* this function creates a value for a symbolic constant. the expression
e *must* be evaluable. */

value MakeValueSymbolic(expression e)
{
  symbolic s;
  value v;

  if (value_unknown_p(v = EvalExpression(e))) {
    /* pips_error("MakeValueSymbolic", 
       "value of parameter must be constant\n"); */
    free_value(v);
    s = make_symbolic(e, make_constant_unknown());
    /* s = make_symbolic(e, make_constant(is_constant_unknown, UU)); */
  }
  else {
    pips_assert("MakeValueSymbolic", value_constant_p(v));

    s = make_symbolic(e, value_constant(v));

    value_constant(v) = constant_undefined;
    free_value(v);
  }

  return make_value(is_value_symbolic, s);
}

bool
signed_constant_expression_p(expression e)
{
    syntax es = expression_syntax(e);
    bool ok = TRUE;

    if(syntax_call_p(es)) {
	entity ce = call_function(syntax_call(es));

	if(!entity_constant_p(ce)) {
	    list args = call_arguments(syntax_call(es));
	    
	    if(ce==CreateIntrinsic(UNARY_MINUS_OPERATOR_NAME)) {
		syntax arg = expression_syntax(EXPRESSION(CAR(args)));
		if( syntax_call_p(arg)) {
		    entity mce = call_function(syntax_call(arg));
		    ok = entity_constant_p(mce);
		}
		else {
		    ok = FALSE;
		}
	    }
	    else {
		ok = FALSE;
	    }
	}
    }
    return ok;
}

basic constant_basic(entity c)
{
  basic b = variable_basic(type_variable(functional_result(type_functional(entity_type(c)))));
  return b;
}

double float_constant_to_double(entity c)
{
  double d = 0.0;
  int i = 0;

  pips_assert("entity is constant", entity_constant_p(c));
  pips_assert("constant is float", basic_float_p(constant_basic(c)));

  i = sscanf(module_local_name(c), "%lf", &d);
  if(i!=1)
    i = sscanf(module_local_name(c), "%le", &d);
  if(i!=1)
    i = sscanf(module_local_name(c), "%lg", &d);
  if(i!=1)
    pips_internal_error("No adequate format for float constant");

  return d;
}

/* BEGIN_EOLE */ /* - please do not remove this line */
/* Lines between BEGIN_EOLE and END_EOLE tags are automatically included
   in the EOLE project (JZ - 11/98) */


/* whether the given function is a constant expression, whatever the type.
 * FI -> JZ: unsigned numerical constant expression?
 */
bool 
expression_is_constant_p(expression e)
{
    syntax s = expression_syntax(e);

    return syntax_call_p(s) ? 
	entity_constant_p(call_function(syntax_call(s))) : FALSE ;
    
}
/* END_EOLE */
