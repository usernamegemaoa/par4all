/* RI-UTIL Library: Functions dealing with and constants related to
 * PIPS intermediate representation ri.newgen,
 * that are NOT generated by NewGen
 *
 * $Id$
 */

 /* Pvecteur is an external type for NewGen and the ri data structure
  * and is not included in ri.h
  *
  * It is included here to avoid changes in many PIPS modules which
  * use ri-util
  *
  * Idem for type Ppolynome (PB 25/07/90); FI: polynomials are not
  * used in ri.newgen; they should not be mentionned here
  *
  * Idem for type Psysteme (FI 3 November 1990)
  */

#include "linear.h"
#include "text.h"
#include "newgen.h"

/*  special characters
 */
#define MODULE_SEP 			':'

#define MODULE_SEP_STRING 		":"
#define LABEL_PREFIX 			"@" // Often assumed to be one character long
#define MAIN_PREFIX 			"%"
#define COMMON_PREFIX 			"~"
#define BLOCKDATA_PREFIX 		"&"

#define FILE_SEP 		        '!'

#define FILE_SEP_STRING 		"!"
#define BLOCK_SEP_STRING 		"`"
#define BLOCK_SEP_CHAR 			'`'
#define MEMBER_SEP_STRING 		"^"
#define MEMBER_SEP_CHAR 		'^'
#define STRUCT_PREFIX 			"#" // Conflict with value naming in transformer
#define STRUCT_PREFIX_CHAR		'#'
#define UNION_PREFIX 			"'" /* Conflict with C character constants */
#define UNION_PREFIX_CHAR		'\''
#define ENUM_PREFIX 			"?"
#define TYPEDEF_PREFIX 			"$"

/*  constant names
 */
#define BLANK_COMMON_LOCAL_NAME 	"*BLANK*"
#define DYNAMIC_AREA_LOCAL_NAME 	"*DYNAMIC*"
#define STATIC_AREA_LOCAL_NAME 		"*STATIC*"
#define HEAP_AREA_LOCAL_NAME 		"*HEAP*"
#define STACK_AREA_LOCAL_NAME 		"*STACK*"

/* For enum and struct and union without names (see c_syntaxcyacc.y */
#define DUMMY_ENUM_PREFIX               "_PIPS_ENUM_"
#define DUMMY_STRUCT_PREFIX             "_PIPS_STRUCT_"
#define DUMMY_UNION_PREFIX              "_PIPS_UNION_"
#define DUMMY_ABSTRACT_PREFIX           "_PIPS_ABSTRACT_"
#define DUMMY_MEMBER_PREFIX             "_PIPS_MEMBER_" /* For bit fields */

/* For dmmmy parameters in functions declarations */
#define DUMMY_PARAMETER_PREFIX         "DUMMY-PARAMETER-"

#define EMPTY_LABEL_NAME                LABEL_PREFIX
#define LIST_DIRECTED_FORMAT_NAME 	"LIST-DIRECTED"

#define TOP_LEVEL_MODULE_NAME 		"TOP-LEVEL"
#define UNBOUNDED_DIMENSION_NAME 	"UNBOUNDED-DIMENSION"
#define IMPLIED_DO_NAME 		"IMPLIED-DO"
#define IMPLIED_DO_FUNCTION_NAME 	"IMPLIED-DO"
#define REPEAT_VALUE_NAME 		"REPEAT-VALUE"
#define REPEAT_VALUE_FUNCTION_NAME 	"REPEAT-VALUE"
#define STATIC_INITIALIZATION_NAME 	"STATIC-INITIALIZATION"
#define STATIC_INITIALIZATION_FUNCTION_NAME 	"STATIC-INITIALIZATION"
#define DATA_LIST_FUNCTION_NAME 	"DATA-LIST="
#define IO_LIST_STRING_NAME             "IOLIST="
#define RETURN_LABEL_NAME 		"00000"


#define BRACE_INTRINSIC                 "BRACE_INTRINSIC"
#define NULL_STATEMENT_INTRINSIC        "NULL_STATEMENT_INTRINSIC"

#define IMPLIED_COMPLEX_NAME		"CMPLX_"
#define IMPLIED_DCOMPLEX_NAME		"DCMPLX_"

#define FIELD_OPERATOR_NAME             "."
#define POINT_TO_OPERATOR_NAME          "->"
#define DEREFERENCING_OPERATOR_NAME     "*indirection"

#define ASSIGN_OPERATOR_NAME 		"="
#define POST_INCREMENT_OPERATOR_NAME    "post++"
#define POST_DECREMENT_OPERATOR_NAME    "post--"
#define PRE_INCREMENT_OPERATOR_NAME     "++pre"
#define PRE_DECREMENT_OPERATOR_NAME     "--pre"
#define MULTIPLY_UPDATE_OPERATOR_NAME   "*=" 
#define DIVIDE_UPDATE_OPERATOR_NAME   	"/=" 
#define PLUS_UPDATE_OPERATOR_NAME   	"+=" 
#define MINUS_UPDATE_OPERATOR_NAME   	"-=" 
#define LEFT_SHIFT_UPDATE_OPERATOR_NAME "<<="
#define RIGHT_SHIFT_UPDATE_OPERATOR_NAME   	">>="
#define BITWISE_OR_UPDATE_OPERATOR_NAME  "|="

#define COMMA_OPERATOR_NAME             ","

#define PLUS_OPERATOR_NAME 		"+"
#define PLUS_C_OPERATOR_NAME 		"+C" /* includes pointer arithmetic */
#define MINUS_OPERATOR_NAME 		"-"
#define MINUS_C_OPERATOR_NAME 		"-C" /* includes pointer arithmetic */
#define UNARY_MINUS_OPERATOR_NAME 	"--"
#define UNARY_PLUS_OPERATOR_NAME 	"+unary"
#define MULTIPLY_OPERATOR_NAME 		"*"
#define DIVIDE_OPERATOR_NAME 		"/"
#define INVERSE_OPERATOR_NAME           "_INV_" /* internal stuff */
#define POWER_OPERATOR_NAME 		"**"
#define MODULO_OPERATOR_NAME 		"MOD"
#define REAL_MODULO_OPERATOR_NAME 	"AMOD"
#define DOUBLE_MODULO_OPERATOR_NAME 	"DMOD"
#define SIGN_OPERATOR_NAME 	        "SIGN"
#define ISIGN_OPERATOR_NAME 	        "ISIGN"
#define DSIGN_OPERATOR_NAME 	        "DSIGN"
#define DIM_OPERATOR_NAME 	        "DIM"
#define IDIM_OPERATOR_NAME 	        "IDIM"
#define DDIM_OPERATOR_NAME 	        "DDIM"
#define DPROD_OPERATOR_NAME 	        "DPROD"
#define CONJG_OPERATOR_NAME 	        "CONJG"
#define DCONJG_OPERATOR_NAME 	        "DCONJG"
#define SQRT_OPERATOR_NAME 	        "SQRT"
#define DSQRT_OPERATOR_NAME 	        "DSQRT"
#define CSQRT_OPERATOR_NAME 	        "CSQRT"
#define CDSQRT_OPERATOR_NAME 	        "CDSQRT"
#define EXP_OPERATOR_NAME 	        "EXP"
#define DEXP_OPERATOR_NAME 	        "DEXP"
#define CEXP_OPERATOR_NAME 	        "CEXP"
#define CDEXP_OPERATOR_NAME 	        "CDEXP"
#define LOG_OPERATOR_NAME 	        "LOG"
#define ALOG_OPERATOR_NAME 	        "ALOG"
#define DLOG_OPERATOR_NAME 	        "DLOG"
#define CLOG_OPERATOR_NAME 	        "CLOG"
#define CDLOG_OPERATOR_NAME 	        "CDLOG"
#define LOG10_OPERATOR_NAME 	        "LOG10"
#define ALOG10_OPERATOR_NAME 	        "ALOG10"
#define DLOG10_OPERATOR_NAME 	        "DLOG10"
#define SIN_OPERATOR_NAME 	        "SIN"
#define DSIN_OPERATOR_NAME 	        "DSIN"
#define CSIN_OPERATOR_NAME 	        "CSIN"
#define CDSIN_OPERATOR_NAME 	        "CDSIN"
#define COS_OPERATOR_NAME 	        "COS"
#define DCOS_OPERATOR_NAME 	        "DCOS"
#define CCOS_OPERATOR_NAME 	        "CCOS"
#define CDCOS_OPERATOR_NAME 	        "CDCOS"
#define TAN_OPERATOR_NAME 	        "TAN"
#define DTAN_OPERATOR_NAME 	        "DTAN"
#define ASIN_OPERATOR_NAME 	        "ASIN"
#define DASIN_OPERATOR_NAME 	        "DASIN"
#define ACOS_OPERATOR_NAME 	        "ACOS"
#define DACOS_OPERATOR_NAME 	        "DACOS"
#define ATAN_OPERATOR_NAME 	        "ATAN"
#define DATAN_OPERATOR_NAME 	        "DATAN"
#define ATAN2_OPERATOR_NAME 	        "ATAN2"
#define DATAN2_OPERATOR_NAME 	        "DATAN2"
#define SINH_OPERATOR_NAME 	        "SINH"
#define DSINH_OPERATOR_NAME 	        "DSINH"
#define COSH_OPERATOR_NAME 	        "COSH"
#define DCOSH_OPERATOR_NAME 	        "DCOSH"
#define TANH_OPERATOR_NAME 	        "TANH"
#define DTANH_OPERATOR_NAME 	        "DTANH"
#define LEFT_SHIFT_OPERATOR_NAME 	"<<"
#define RIGHT_SHIFT_OPERATOR_NAME 	">>"

// Conflicts with reserved prefixes for BLOCKDATA (&), COMMON (~),
// MAIN (%), FILESEP (!), MEMBER_SEP_STRING (^), ENUM (?) & al.
// Use "-" to avoid conflict with user defined symbols
#define ADDRESS_OF_OPERATOR_NAME         "__address-of__"       // &
#define BITWISE_AND_OPERATOR_NAME 	 "__bit-and__"          // &
#define BITWISE_NOT_OPERATOR_NAME 	 "__bit-not__"          // ~
#define BITWISE_XOR_OPERATOR_NAME 	 "__C-xor__"            // ^
#define C_AND_OPERATOR_NAME 		 "__C-and__"            // &&
#define MODULO_UPDATE_OPERATOR_NAME   	 "__C-modulo-update__"  // %=
#define BITWISE_AND_UPDATE_OPERATOR_NAME "__C-bit-and-update__" // &=
#define BITWISE_XOR_UPDATE_OPERATOR_NAME "__C-bit-xor-update__" // ^=
#define CONDITIONAL_OPERATOR_NAME        "__C-conditional__"    // ?
#define C_NOT_OPERATOR_NAME 		 "__C-logical-not__"    // !
#define C_NON_EQUAL_OPERATOR_NAME 	 "__C-non-equal__"      // !=
#define C_MODULO_OPERATOR_NAME 		 "__C-modulo__"         // %

#define BITWISE_OR_OPERATOR_NAME 	"|"
#define MIN_OPERATOR_NAME 		"MIN"
#define MIN0_OPERATOR_NAME 		"MIN0"
#define MIN1_OPERATOR_NAME 		"MIN1"
#define AMIN0_OPERATOR_NAME 		"AMIN0"
#define AMIN1_OPERATOR_NAME 		"AMIN1"
#define DMIN1_OPERATOR_NAME 		"DMIN1"
#define MAX_OPERATOR_NAME 		"MAX"
#define MAX0_OPERATOR_NAME 		"MAX0"
#define AMAX0_OPERATOR_NAME 		"AMAX0"
#define MAX1_OPERATOR_NAME 		"MAX1"
#define AMAX1_OPERATOR_NAME 		"AMAX1"
#define DMAX1_OPERATOR_NAME 		"DMAX1"
#define ABS_OPERATOR_NAME 		"ABS"
#define IABS_OPERATOR_NAME 		"IABS"
#define DABS_OPERATOR_NAME 		"DABS"
#define CABS_OPERATOR_NAME 		"CABS"
#define CDABS_OPERATOR_NAME 		"CDABS"

#define LENGTH_OPERATOR_NAME 		"LEN"
#define INDEX_OPERATOR_NAME 		"INDEX"
#define LGE_OPERATOR_NAME 		"LGE"
#define LGT_OPERATOR_NAME 		"LGT"
#define LLE_OPERATOR_NAME 		"LLE"
#define LLT_OPERATOR_NAME 		"LLT"

/* generic conversion names.
 */
#define INT_GENERIC_CONVERSION_NAME	"INT"
#define IFIX_GENERIC_CONVERSION_NAME	"IFIX"
#define IDINT_GENERIC_CONVERSION_NAME	"IDINT"
#define REAL_GENERIC_CONVERSION_NAME	"REAL"
#define FLOAT_GENERIC_CONVERSION_NAME	"FLOAT"
#define DFLOAT_GENERIC_CONVERSION_NAME	"DFLOAT"
#define SNGL_GENERIC_CONVERSION_NAME	"SNGL"
#define DBLE_GENERIC_CONVERSION_NAME	"DBLE"
#define DREAL_GENERIC_CONVERSION_NAME	"DREAL"
#define CMPLX_GENERIC_CONVERSION_NAME	"CMPLX"
#define DCMPLX_GENERIC_CONVERSION_NAME	"DCMPLX"

#define INT_TO_CHAR_CONVERSION_NAME	"CHAR"
#define CHAR_TO_INT_CONVERSION_NAME	"ICHAR"

#define AINT_CONVERSION_NAME	        "AINT"
#define DINT_CONVERSION_NAME	        "DINT"
#define ANINT_CONVERSION_NAME	        "ANINT"
#define DNINT_CONVERSION_NAME	        "DNINT"
#define NINT_CONVERSION_NAME	        "NINT"
#define IDNINT_CONVERSION_NAME	        "IDNINT"
#define AIMAG_CONVERSION_NAME	        "AIMAG"
#define DIMAG_CONVERSION_NAME	        "DIMAG"

/* FI: intrinsics are defined at a third place after bootstrap and effects!
 *     I guess the name should be defined here and used in table(s) there
 */

#define AND_OPERATOR_NAME 		".AND."
#define OR_OPERATOR_NAME 		".OR."
#define C_OR_OPERATOR_NAME 		"||"
#define NOT_OPERATOR_NAME 		".NOT."
#define NON_EQUAL_OPERATOR_NAME 	".NE."
#define EQUIV_OPERATOR_NAME 		".EQV."
#define NON_EQUIV_OPERATOR_NAME 	".NEQV."

#define TRUE_OPERATOR_NAME 		".TRUE."
#define FALSE_OPERATOR_NAME 		".FALSE."

#define GREATER_OR_EQUAL_OPERATOR_NAME 	".GE."
#define C_GREATER_OR_EQUAL_OPERATOR_NAME 	">="
#define GREATER_THAN_OPERATOR_NAME 	".GT."
#define C_GREATER_THAN_OPERATOR_NAME 	">"
#define LESS_OR_EQUAL_OPERATOR_NAME 	".LE."
#define C_LESS_OR_EQUAL_OPERATOR_NAME 	"<="
#define LESS_THAN_OPERATOR_NAME 	".LT."
#define C_LESS_THAN_OPERATOR_NAME 	"<"
#define EQUAL_OPERATOR_NAME 		".EQ."
#define C_EQUAL_OPERATOR_NAME 		"=="

#define CONTINUE_FUNCTION_NAME 		"CONTINUE"
#define RETURN_FUNCTION_NAME 		"RETURN"
#define STOP_FUNCTION_NAME 		"STOP"
#define PAUSE_FUNCTION_NAME 		"PAUSE"
#define END_FUNCTION_NAME 		"END"

#define BREAK_FUNCTION_NAME 		"break"
#define CASE_FUNCTION_NAME 		"case"
#define DEFAULT_FUNCTION_NAME 		"default"
#define C_RETURN_FUNCTION_NAME 		"return"

#define SUBSTRING_FUNCTION_NAME 	"_SUBSTR"
#define CONCATENATION_FUNCTION_NAME 	"//"
#define ASSIGN_SUBSTRING_FUNCTION_NAME 	"_ASSIGN_SUBSTR"

#define WRITE_FUNCTION_NAME 		"WRITE"
#define PRINT_FUNCTION_NAME 		"PRINT"
#define REWIND_FUNCTION_NAME 		"REWIND"
#define OPEN_FUNCTION_NAME 		"OPEN"
#define CLOSE_FUNCTION_NAME 		"CLOSE"
#define INQUIRE_FUNCTION_NAME 		"INQUIRE"
#define BACKSPACE_FUNCTION_NAME 	"BACKSPACE"
#define READ_FUNCTION_NAME 		"READ"
#define BUFFERIN_FUNCTION_NAME 		"BUFFERIN"
#define BUFFEROUT_FUNCTION_NAME 	"BUFFEROUT"
#define ENDFILE_FUNCTION_NAME 		"ENDFILE"
#define FORMAT_FUNCTION_NAME 		"FORMAT"

#define MOD_INTRINSIC_NAME 		"MOD"

/* #include <ctype.h> */

#define ISALNUM_OPERATOR_NAME 		"isalnum"
#define ISALPHA_OPERATOR_NAME 		"isalpha"
#define ISCNTRL_OPERATOR_NAME 		"iscntrl"
#define ISDIGIT_OPERATOR_NAME 		"isdigit"
#define ISGRAPH_OPERATOR_NAME 		"isgraph"
#define ISLOWER_OPERATOR_NAME 		"islower"
#define ISPRINT_OPERATOR_NAME 		"isprint"
#define ISPUNCT_OPERATOR_NAME 		"ispunct"
#define ISSPACE_OPERATOR_NAME 		"isspace"
#define ISUPPER_OPERATOR_NAME 		"isupper"
#define ISXDIGIT_OPERATOR_NAME 		"isxdigit"
#define TOLOWER_OPERATOR_NAME 		"tolower"
#define TOUPPER_OPERATOR_NAME 		"toupper"
#define ISASCII_OPERATOR_NAME 		"isascii"
#define TOASCII_OPERATOR_NAME 		"toascii"
#define _TOLOWER_OPERATOR_NAME 		"_tolower"
#define _TOUPPER_OPERATOR_NAME 		"_toupper"

  /* #include <math.h>*/
#define C_ACOS_OPERATOR_NAME "acos"
#define C_ASIN_OPERATOR_NAME "asin"
#define C_ATAN_OPERATOR_NAME "atan"
#define C_ATAN2_OPERATOR_NAME "atan2"
#define C_COS_OPERATOR_NAME "cos"
#define C_SIN_OPERATOR_NAME "sin"
#define C_TAN_OPERATOR_NAME "tan"
#define C_COSH_OPERATOR_NAME "cosh"
#define C_SINH_OPERATOR_NAME "sinh"
#define C_TANH_OPERATOR_NAME "tanh"
#define C_EXP_OPERATOR_NAME "exp"
#define FREXP_OPERATOR_NAME "frexp"
#define LDEXP_OPERATOR_NAME "ldexp"
#define C_LOG_OPERATOR_NAME "log"
#define C_LOG10_OPERATOR_NAME "log10"
#define MODF_OPERATOR_NAME "modf"
#define POW_OPERATOR_NAME "pow"
#define C_SQRT_OPERATOR_NAME "sqrt"
#define CEIL_OPERATOR_NAME "ceil"
#define FABS_OPERATOR_NAME "fabs"
#define FLOOR_OPERATOR_NAME "floor"
#define FMOD_OPERATOR_NAME "fmod"
#define ERF_OPERATOR_NAME "erf"
#define ERFC_OPERATOR_NAME "erfc"
#define GAMMA_OPERATOR_NAME "gamma"
#define HYPOT_OPERATOR_NAME "hypot"
#define ISNAN_OPERATOR_NAME "isnan"
#define J0_OPERATOR_NAME "j0"
#define J1_OPERATOR_NAME "j1"
#define JN_OPERATOR_NAME "jn"
#define LGAMMA_OPERATOR_NAME "lgamma"
#define Y0_OPERATOR_NAME "y0"
#define Y1_OPERATOR_NAME "y1"
#define YN_OPERATOR_NAME "yn"
#define C_ACOSH_OPERATOR_NAME "acosh"
#define C_ASINH_OPERATOR_NAME "asinh"
#define C_ATANH_OPERATOR_NAME "atanh"
#define CBRT_OPERATOR_NAME "cbrt"
#define LOGB_OPERATOR_NAME "logb"
#define NEXTAFTER_OPERATOR_NAME "nextafter"
#define REMAINDER_OPERATOR_NAME "remainder"
#define SCALB_OPERATOR_NAME "scalb"
#define EXPM1_OPERATOR_NAME "expm1"
#define ILOGB_OPERATOR_NAME "ilogb"
#define LOG1P_OPERATOR_NAME "log1p"
#define RINT_OPERATOR_NAME "rint"
#define MATHERR_OPERATOR_NAME "matherr"
#define SIGNIFICAND_OPERATOR_NAME "significand"
#define COPYSIGN_OPERATOR_NAME "copysign"
#define SCALBN_OPERATOR_NAME "scalbn"
#define MODFF_OPERATOR_NAME "modff"
#define SIGFPE_OPERATOR_NAME "sigfpe"
#define SINGLE_TO_DECIMAL_OPERATOR_NAME "single_to_decimal"
#define DOUBLE_TO_DECIMAL_OPERATOR_NAME "double_to_decimal"
#define EXTENDED_TO_DECIMAL_OPERATOR_NAME "extended_to_decimal"
#define QUADRUPLE_TO_DECIMAL_OPERATOR_NAME "quadruple_to_decimal"
#define DECIMAL_TO_SINGLE_OPERATOR_NAME "decimal_to_single"
#define DECIMAL_TO_DOUBLE_OPERATOR_NAME "decimal_to_double"
#define DECIMAL_TO_EXTENDED_OPERATOR_NAME "decimal_to_extended"
#define DECIMAL_TO_QUADRUPLE_OPERATOR_NAME "decimal_to_quadruple"
#define STRING_TO_DECIMAL_OPERATOR_NAME "string_to_decimal"
#define FUNC_TO_DECIMAL_OPERATOR_NAME "func_to_decimal"
#define FILE_TO_DECIMAL_OPERATOR_NAME "file_to_decimal"
#define SECONVERT_OPERATOR_NAME "seconvert"
#define SFCONVERT_OPERATOR_NAME "sfconvert"
#define SGCONVERT_OPERATOR_NAME "sgconvert"
#define ECONVERT_OPERATOR_NAME "econvert"
#define FCONVERT_OPERATOR_NAME "fconvert"
#define GCONVERT_OPERATOR_NAME "gconvert"
#define QECONVERT_OPERATOR_NAME "qeconvert"
#define QFCONVERT_OPERATOR_NAME "qfconvert"
#define QGCONVERT_OPERATOR_NAME "qgconvert"

/* These operators are used within the optimize transformation in
   order to manipulate operators such as n-ary add and multiply or
   multiply-add operators ( JZ - sept 98)
 */
#define EOLE_FMA_OPERATOR_NAME          "EOLE-FMA-OP"
#define EOLE_FMS_OPERATOR_NAME          "EOLE-FMS-OP"
#define EOLE_PROD_OPERATOR_NAME         "EOLE-PROD-OP"
#define EOLE_SUM_OPERATOR_NAME          "EOLE-SUM-OP"

/* Integer Multiply Add and Sub, FC 27/10/2005 for FI
 */
#define IMA_OPERATOR_NAME				"IMA-OP"
#define IMS_OPERATOR_NAME				"IMS-OP"

/* moved from ricedg-local.h */
#define LOOP_COUNTER_MODULE_NAME 	"LOOP-COUNTER"
#define DI_VAR_MODULE_NAME 		"DI-VAR"

/* macros */
/* memory leak: entity_user_name() allocates a new string which is
   lost right away */
#define entity_an_operator_p(e,name) \
  (strcmp(entity_user_name(e), name##_OPERATOR_NAME)==0)
#define entity_a_function_p(e,name) \
  (strcmp(entity_user_name(e), name##_FUNCTION_NAME)==0)

#define ENTITY_CONVERSION_P(e,name) \
  (strcmp(entity_local_name(e), name##_GENERIC_CONVERSION_NAME)==0)
#define ENTITY_CONVERSION_CMPLX_P(e) ENTITY_CONVERSION_P(e, CMPLX)
#define ENTITY_CONVERSION_DCMPLX_P(e) ENTITY_CONVERSION_P(e, DCMPLX)

#define ENTITY_CONTINUE_P(e) entity_a_function_p(e, CONTINUE)
#define ENTITY_STOP_P(e) entity_a_function_p(e, STOP)
#define ENTITY_RETURN_P(e) entity_a_function_p(e, RETURN)

#define ENTITY_ASSIGN_P(e) entity_an_operator_p(e, ASSIGN)
#define ENTITY_POST_INCREMENT_P(e) entity_an_operator_p(e, POST_INCREMENT)
#define ENTITY_POST_DECREMENT_P(e) entity_an_operator_p(e, POST_DECREMENT)
#define ENTITY_PRE_INCREMENT_P(e) entity_an_operator_p(e, PRE_INCREMENT)
#define ENTITY_PRE_DECREMENT_P(e) entity_an_operator_p(e, PRE_DECREMENT)
#define ENTITY_MULTIPLY_UPDATE_P(e) entity_an_operator_p(e, MULTIPLY_UPDATE)
#define ENTITY_DIVIDE_UPDATE_P(e) entity_an_operator_p(e, DIVIDE_UPDATE)
#define ENTITY_MODULO_UPDATE_P(e) entity_an_operator_p(e, MODULO_UPDATE)
#define ENTITY_PLUS_UPDATE_P(e) entity_an_operator_p(e, PLUS_UPDATE)
#define ENTITY_MINUS_UPDATE_P(e) entity_an_operator_p(e, MINUS_UPDATE)
#define ENTITY_LEFT_SHIFT_UPDATE_P(e) entity_an_operator_p(e, LEFT_SHIFT_UPDATE)
#define ENTITY_RIGHT_SHIFT_UPDATE_P(e) entity_an_operator_p(e, RIGHT_SHIFT_UPDATE)
#define ENTITY_BITWISE_AND_UPDATE_P(e) entity_an_operator_p(e, BITWISE_AND_UPDATE)
#define ENTITY_BITWISE_XOR_UPDATE_P(e) entity_an_operator_p(e, BITWISE_XOR_UPDATE)
#define ENTITY_BITWISE_OR_UPDATE_P(e) entity_an_operator_p(e, BITWISE_OR_UPDATE)
#define ENTITY_ADDRESS_OF_P(e) entity_an_operator_p(e, ADDRESS_OF)
#define ENTITY_CONDITIONAL_P(e) entity_an_operator_p(e, CONDITIONAL)

#define ENTITY_PLUS_P(e) entity_an_operator_p(e, PLUS)
#define ENTITY_PLUS_C_P(e) entity_an_operator_p(e, PLUS_C)
#define ENTITY_MINUS_P(e) entity_an_operator_p(e, MINUS)
#define ENTITY_MINUS_C_P(e) entity_an_operator_p(e, MINUS_C)
#define ENTITY_UNARY_MINUS_P(e) entity_an_operator_p(e, UNARY_MINUS)
#define ENTITY_MULTIPLY_P(e) entity_an_operator_p(e, MULTIPLY)
#define ENTITY_MODULO_P(e) entity_an_operator_p(e, MODULO)
#define ENTITY_POWER_P(e) entity_an_operator_p(e, POWER)
#define ENTITY_DIVIDE_P(e) entity_an_operator_p(e, DIVIDE)
#define ENTITY_MIN_P(e) entity_an_operator_p(e, MIN)
#define ENTITY_MAX_P(e) entity_an_operator_p(e, MAX)
#define ENTITY_MIN0_P(e) entity_an_operator_p(e, MIN0)
#define ENTITY_MAX0_P(e) entity_an_operator_p(e, MAX0)
#define ENTITY_AMIN1_P(e) entity_an_operator_p(e, AMIN1)
#define ENTITY_AMAX1_P(e) entity_an_operator_p(e, AMAX1)
#define ENTITY_DMIN1_P(e) entity_an_operator_p(e, DMIN1)
#define ENTITY_DMAX1_P(e) entity_an_operator_p(e, DMAX1)
#define ENTITY_MIN_OR_MAX_P(e) (ENTITY_MIN_P(e) || ENTITY_MAX_P(e) )
#define ENTITY_ABS_P(e) entity_an_operator_p(e, ABS)
#define ENTITY_IABS_P(e) entity_an_operator_p(e, IABS)
#define ENTITY_DABS_P(e) entity_an_operator_p(e, DABS)
#define ENTITY_CABS_P(e) entity_an_operator_p(e, CABS)

#define ENTITY_AND_P(e) (entity_an_operator_p(e, AND) || entity_an_operator_p(e, C_AND))

#define ENTITY_OR_P(e) (entity_an_operator_p(e, OR) || entity_an_operator_p(e, C_OR))
#define ENTITY_NOT_P(e) (entity_an_operator_p(e, NOT) || entity_an_operator_p(e, C_NOT))
#define ENTITY_NON_EQUAL_P(e) (entity_an_operator_p(e, NON_EQUAL) || entity_an_operator_p(e, C_NON_EQUAL))
#define ENTITY_EQUIV_P(e) entity_an_operator_p(e, EQUIV)
#define ENTITY_NON_EQUIV_P(e) entity_an_operator_p(e, NON_EQUIV)

/* Attention : 
   This definition is different with the Fortran Standard where the logical 
   operators are the following only: AND, OR, NOT, EQUIV, NEQUIV (NN-Mars 2000)*/

#define ENTITY_LOGICAL_OPERATOR_P(e) ( ENTITY_RELATIONAL_OPERATOR_P(e) || \
                                       ENTITY_AND_P(e) || \
                                       ENTITY_OR_P(e) || \
                                       ENTITY_NOT_P(e) || \
                                       ENTITY_NON_EQUAL_P(e) || \
                                       ENTITY_EQUIV_P(e) || \
                                       ENTITY_NON_EQUIV_P(e) )

#define ENTITY_TRUE_P(e) entity_an_operator_p(e, TRUE)
#define ENTITY_FALSE_P(e) entity_an_operator_p(e, FALSE)

#define ENTITY_GREATER_OR_EQUAL_P(e) (entity_an_operator_p(e, GREATER_OR_EQUAL) || entity_an_operator_p(e, C_GREATER_OR_EQUAL))
#define ENTITY_GREATER_THAN_P(e)  (entity_an_operator_p(e, GREATER_THAN) || entity_an_operator_p(e, C_GREATER_THAN))
#define ENTITY_LESS_OR_EQUAL_P(e) (entity_an_operator_p(e, LESS_OR_EQUAL) || entity_an_operator_p(e, C_LESS_OR_EQUAL))
#define ENTITY_LESS_THAN_P(e) (entity_an_operator_p(e, LESS_THAN) || entity_an_operator_p(e, C_LESS_THAN))

#define ENTITY_EQUAL_P(e) (entity_an_operator_p(e, EQUAL) || entity_an_operator_p(e, C_EQUAL))

#define ENTITY_RELATIONAL_OPERATOR_P(e) ( \
					 ENTITY_GREATER_OR_EQUAL_P(e) || \
					 ENTITY_GREATER_THAN_P(e) || \
					 ENTITY_LESS_OR_EQUAL_P(e) || \
					 ENTITY_LESS_THAN_P(e) || \
					 ENTITY_NON_EQUAL_P(e) || \
					 ENTITY_EQUAL_P(e) )

/* Special areas are sometimes tested by top_level_entity_p() because they
   are not top level entities whereas user commons are. */
#define SPECIAL_AREA_P(e) \
    ((same_string_p(module_local_name(e), DYNAMIC_AREA_LOCAL_NAME)) || \
     (same_string_p(module_local_name(e), STATIC_AREA_LOCAL_NAME)) || \
     (same_string_p(module_local_name(e), STACK_AREA_LOCAL_NAME)) || \
     (same_string_p(module_local_name(e), HEAP_AREA_LOCAL_NAME)))

#define SPECIAL_COMMON_P(e) SPECIAL_AREA_P(e)

/* IO Management
 */
#define ENTITY_NAME_P(e, name)(same_string_p(entity_user_name(e),name))

#define ENTITY_WRITE_P(e) ENTITY_NAME_P(e, "WRITE")
#define ENTITY_REWIND_P(e) ENTITY_NAME_P(e, "REWIND")
#define ENTITY_OPEN_P(e) ENTITY_NAME_P(e, "OPEN")
#define ENTITY_CLOSE_P(e) ENTITY_NAME_P(e, "CLOSE")
#define ENTITY_READ_P(e) ENTITY_NAME_P(e, "READ")
#define ENTITY_BUFFERIN_P(e) ENTITY_NAME_P(e, "BUFFERIN")
#define ENTITY_BUFFEROUT_P(e) ENTITY_NAME_P(e, "BUFFEROUT")
#define ENTITY_ENDFILE_P(e) ENTITY_NAME_P(e, "ENDFILE")
#define ENTITY_IMPLIEDDO_P(e) ENTITY_NAME_P(e, IMPLIED_DO_NAME)
#define ENTITY_IO_LIST_P(e) ENTITY_NAME_P(e, IO_LIST_STRING_NAME)
#define ENTITY_FORMAT_P(e) ENTITY_NAME_P(e, "FORMAT")

/* DATA management
 */
#define ENTITY_STATIC_INITIALIZATION_P(e) ENTITY_NAME_P(e, STATIC_INITIALIZATION_FUNCTION_NAME)
#define ENTITY_REPEAT_VALUE_P(e) ENTITY_NAME_P(e, REPEAT_VALUE_FUNCTION_NAME)
#define ENTITY_DATA_LIST_P(e) ENTITY_NAME_P(e, DATA_LIST_FUNCTION_NAME)

/* C initialization expression */
#define ENTITY_BRACE_INTRINSIC_P(e) ENTITY_NAME_P(e, BRACE_INTRINSIC)

#define ENTITY_IMPLIED_CMPLX_P(e) ENTITY_NAME_P(e, IMPLIED_COMPLEX_NAME)
#define ENTITY_IMPLIED_DCMPLX_P(e) ENTITY_NAME_P(e, IMPLIED_DCOMPLEX_NAME)

#define ENTITY_FOUR_OPERATION_P(e) ( ENTITY_PLUS_P(e) || \
                                     ENTITY_MINUS_P(e) || \
				     ENTITY_UNARY_MINUS_P(e) || \
                                     ENTITY_MULTIPLY_P(e) || \
                                     ENTITY_DIVIDE_P(e) )

#define IO_CALL_P(call) io_intrinsic_p(call_function(call))

/* classification of basics
 */
#define basic_numeric_simple_p(b) (basic_int_p(b) || basic_float_p(b))
#define basic_numeric_p(b) (basic_numeric_simple_p(b) || basic_complex_p(b))
#define basic_compatible_simple_p(b1, b2) (\
                (basic_numeric_simple_p(b1) && basic_numeric_simple_p(b2)) ||\
                (basic_string_p(b1) && basic_string_p(b2)) ||\
                (basic_logical_p(b1) && basic_logical_p(b2)) ||\
                (basic_overloaded_p(b1) && basic_overloaded_p(b2)) ||\
                (basic_undefined_p(b1) && basic_undefined_p(b2)))
#define basic_compatible_p(b1, b2) (\
                (basic_numeric_p(b1) && basic_numeric_p(b2)) ||\
                (basic_string_p(b1) && basic_string_p(b2)) ||\
                (basic_logical_p(b1) && basic_logical_p(b2)) ||\
                (basic_overloaded_p(b1) && basic_overloaded_p(b2)) ||\
                (basic_undefined_p(b1) && basic_undefined_p(b2)))

/*  constant sizes
 */
#define LABEL_SIZE 5
#define INDENTATION (get_int_property("PRETTYPRINT_INDENTATION"))
#define MAXIMAL_MODULE_NAME_SIZE 36

/*   default values
 */
#define STATEMENT_NUMBER_UNDEFINED (-1)
#define UNKNOWN_RAM_OFFSET (-1)
// It is for extern variables whose ram offset is not known
#define UNDEFINED_RAM_OFFSET (-2)
// It is for dynamic allocation of arrays whose ram offset is not known either.
#define DYNAMIC_RAM_OFFSET (-3)


/* On devrait utiliser Newgen pour cela, mais comme on ne doit pas
   les utiliser directement (mais via statement_less_p), cela devrait
   decourager les temeraires */

#define MAKE_ORDERING(u,s) (u<<16|s)
#define ORDERING_NUMBER(o) (o>>16)
#define ORDERING_STATEMENT(o) (o & 0xffff)

#define NORMALIZE_EXPRESSION(e) \
    ((expression_normalized(e) == normalized_undefined) ? \
         (expression_normalized(e) = NormalizeExpression(e)) : \
         (expression_normalized(e)))

/*   MISC: newgen shorthands
 */
#define entity_declarations(e) (code_declarations(entity_code(e)))

#define effect_system(e) \
	(descriptor_convex_p(effect_descriptor(e))? \
	 descriptor_convex(effect_descriptor(e)) : SC_UNDEFINED)

#define effect_reference(e) \
	 preference_reference(cell_preference(effect_cell(e)))

#define entity_variable_p(e) (type_variable_p(entity_type(e)))

#define entity_symbolic_p(e) (type_functional_p(entity_type(e)) && \
  storage_rom_p(entity_storage(e)) && value_symbolic_p(entity_initial(e)))

#define make_simple_effect(reference,action,approximation)\
    make_effect(make_cell(is_cell_preference, make_preference(reference)),\
    (action),(approximation), \
    make_descriptor(is_descriptor_none,UU))  

#define make_convex_effect(reference,action,approximation,system)\
    make_effect(make_cell(is_cell_preference, make_preference(reference)),\
    (action),(approximation), \
    make_descriptor(is_descriptor_convex,system))  


#define statement_block_p(stat) \
	(instruction_block_p(statement_instruction(stat)))

#define entity_constant_p(e) (type_functional_p(entity_type(e)) && \
  storage_rom_p(entity_storage(e)) && value_constant_p(entity_initial(e)))

/* building instruction and statements...
 */
#define instruction_to_statement(i) \
   make_statement(entity_empty_label(),\
		  STATEMENT_NUMBER_UNDEFINED, STATEMENT_ORDERING_UNDEFINED,\
		  empty_comments, i,NIL,NULL)

#define loop_to_instruction(l) make_instruction(is_instruction_loop, l)
#define test_to_instruction(t) make_instruction(is_instruction_test, t)
#define call_to_instruction(c) make_instruction(is_instruction_call, c)

#define loop_to_statement(l) instruction_to_statement(loop_to_instruction(l))
#define test_to_statement(t) instruction_to_statement(test_to_instruction(t))
#define call_to_statement(c) instruction_to_statement(call_to_instruction(c))


/***************************************************** BOOLEAN EXPRESSIONS */
/* Building quickly boolean expressions, FC.
 */

#define unary_intrinsic_expression(name, e)\
 call_to_expression(make_call(entity_intrinsic(name),CONS(EXPRESSION,e,NIL)))

#define binary_intrinsic_expression(name, e1, e2)\
 call_to_expression(make_call(entity_intrinsic(name),\
 CONS(EXPRESSION, e1, CONS(EXPRESSION, e2, NIL))))
  
#define not_expression(e) \
    unary_intrinsic_expression(NOT_OPERATOR_NAME, e)
#define or_expression(e1, e2) \
    binary_intrinsic_expression(OR_OPERATOR_NAME, e1, e2)
#define and_expression(e1, e2) \
    binary_intrinsic_expression(AND_OPERATOR_NAME, e1, e2)
#define ne_expression(e1, e2) \
    binary_intrinsic_expression(NON_EQUAL_OPERATOR_NAME, e1, e2)
#define eq_expression(e1, e2) \
    binary_intrinsic_expression(EQUAL_OPERATOR_NAME, e1, e2)
#define gt_expression(e1, e2) \
    binary_intrinsic_expression(GREATER_THAN_OPERATOR_NAME, e1, e2)
#define ge_expression(e1, e2) \
    binary_intrinsic_expression(GREATER_OR_EQUAL_OPERATOR_NAME, e1, e2)
#define lt_expression(e1, e2) \
    binary_intrinsic_expression(LESS_THAN_OPERATOR_NAME, e1, e2)
#define le_expression(e1, e2) \
    binary_intrinsic_expression(LESS_OR_EQUAL_OPERATOR_NAME, e1, e2)


/******************************************************** CONTROL GRAPH... */
/* For the control graph modifiers: */

/* To specify the way that remove_a_control_from_a_list_and_relink
   acts: */
enum remove_a_control_from_a_list_and_relink_direction 
{
   /* Put some strange number to avoid random clash as much as
      possible... */
   source_is_predecessor_and_dest_is_successor = 119,
      source_is_successor_and_dest_is_predecessor = -123
      };
typedef enum remove_a_control_from_a_list_and_relink_direction
remove_a_control_from_a_list_and_relink_direction;


/********************************************************* DUMMY VARIABLES */
#define PRIME_LETTER_FOR_VARIABLES	"p"

/* define to build the _dummy and _prime of a variable.
 */
#define GET_DUMMY_VARIABLE_ENTITY(MODULE, NAME, lname)\
entity get_ith_##lname##_dummy(int i)\
    {return(get_ith_dummy(MODULE, NAME, i));}\
entity get_ith_##lname##_prime(int i)\
    {return(get_ith_dummy(MODULE, NAME PRIME_LETTER_FOR_VARIABLES, i));}

/* Constants for some ex-atomizer variable generation */
#define TMP_ENT 1
#define AUX_ENT 2
#define DOUBLE_PRECISION_SIZE 8


/***************************************************** HPF/HPFC DIRECTIVES */

/* moved here because needed by syntax:-(
 */
/* Directive names encoding: HPF_PREFIX + one character.
 * This encoding is achieved thru a sed script that transforms directives 
 * into calls that can be parsed by the PIPS F77 parser. It's a hack but 
 * it greatly reduced the number of lines for directive analysis, and 
 * it allowed quite simply to figure out where the executable directives
 * are in the code.
 * However the syntax allowed in mapping directives is restricted to F77.
 */

/* prefix for spacial directive calls
 */
#define HPF_PREFIX		"HPFC"

/* suffixies for encoded hpf keywords
 */
#define BLOCK_SUFFIX		"K"
#define CYCLIC_SUFFIX		"C"
#define STAR_SUFFIX		"S"

/* suffixes for HPF directives managed by HPFC
 */
#define ALIGN_SUFFIX		"A"
#define REALIGN_SUFFIX		"B"
#define DISTRIBUTE_SUFFIX	"D"
#define REDISTRIBUTE_SUFFIX	"E"
#define INDEPENDENT_SUFFIX	"I"
#define NEW_SUFFIX		"N"
#define REDUCTION_SUFFIX	"R"
#define PROCESSORS_SUFFIX	"P"
#define TEMPLATE_SUFFIX		"T"
#define PURE_SUFFIX		"U"
#define DYNAMIC_SUFFIX		"Y"

/* suffixes for my own (FCD:-) directives.
 * these directives are used to instrument the code. 
 * must be used carefully. may be ignore with some properties.
 */
#define TELL_SUFFIX		"0"
#define SYNCHRO_SUFFIX		"1"
#define TIMEON_SUFFIX		"2"
#define TIMEOFF_SUFFIX		"3"
#define SETBOOL_SUFFIX		"4"
#define SETINT_SUFFIX		"5"
#define HPFCIO_SUFFIX		"6"
#define HOSTSECTION_SUFFIX	"7"
#define DEAD_SUFFIX		"8"
#define FAKE_SUFFIX		"9"

/* property prefix for ignoring FCD directives
 * TIME, SYNCHRO and SET exists.
 */
#define FCD_IGNORE_PREFIX	"HPFC_IGNORE_FCD_"

/* soft block->sequence transition
 */
#ifdef is_instruction_sequence
#define is_instruction_block is_instruction_sequence
#define instruction_block_p(i) instruction_sequence_p(i)
#define instruction_block(i) sequence_statements(instruction_sequence(i))
#endif

/* After the modification in Newgen:  unstructured = entry:control x exit:control
   we have create a macro to transform automatically unstructured_control to unstructured_entry */
#define unstructured_control unstructured_entry

/* Default type sizes */

#define DEFAULT_CHARACTER_TYPE_SIZE (1)
#define DEFAULT_SHORT_INTEGER_TYPE_SIZE (2)
#define DEFAULT_INTEGER_TYPE_SIZE (4)
#define DEFAULT_LONG_INTEGER_TYPE_SIZE (6)
#define DEFAULT_LONG_LONG_INTEGER_TYPE_SIZE (8)

#define DEFAULT_POINTER_TYPE_SIZE (4) // could be 4 or 8
#define DEFAULT_UNSIGNED_TYPE_SIZE (1)
#define DEFAULT_SIGNED_TYPE_SIZE (2)

/* The standard integer types are represented as follow
char                   = 1
short_int              = 2
int                    = 4
long_int               = 6
long_long_int          = 8

unsigned_char          = 11
unsigned_short_int     = 12
unsigned_int           = 14
unsigned_long_int      = 16
unsigned_long_long_int = 18

signed_char            = 21
signed_short_int       = 22
signed_int             = 24
signed_long_int        = 26
signed_long_long_int   = 28

mod(i,10) refers to the basic size and div(i,10) refers that 
the variable is unsigned, signed or not */

#define DEFAULT_REAL_TYPE_SIZE (4)
#define DEFAULT_DOUBLEPRECISION_TYPE_SIZE (8)
#define DEFAULT_COMPLEX_TYPE_SIZE (8)
#define DEFAULT_DOUBLECOMPLEX_TYPE_SIZE (16)
#define DEFAULT_LOGICAL_TYPE_SIZE (4)

#define DEFAULT_POINTER_TYPE_SIZE (4)


/* Implicit variables to handle IO effetcs */

/* package name for io routines */
#define IO_EFFECTS_PACKAGE_NAME "__IO_EFFECTS"
/* array of Logical UNits; it is more or less handled as the current file pointer */
#define IO_EFFECTS_ARRAY_NAME "LUNS"
/* array of end of file codes */
#define IO_EOF_ARRAY_NAME "END_LUNS"
/* array of error codes for LUNs */
#define IO_ERROR_ARRAY_NAME "ERR_LUNS"
/* size of the unit specifier */
#define IO_EFFECTS_UNIT_SPECIFIER_LENGTH 4
/* Standard unit numbers depend on the operating system. Here are UNIX definitions. */
#define STDERR_LUN (0)
#define STDIN_LUN (5)
#define STDOUT_LUN (6)

/* Empty comments (i.e. default comments) */

#define empty_comments string_undefined

/* Macro to walk through controls reachable from the entry node of an
 * unstructured. Reachability is defined by successors and predecessors
 * (i.e. the control flow graph is seen as non-directed.
 *
 * A list of control is built. Most of the time, it must be freed
 * on exit from CONTROL_MAP().
 */

/*
#define CONTROL_MAP( ctl, code, c, list ) \
{ \
    cons *_cm_list_init = (list) ; \
    cons *_cm_list = _cm_list_init ; \
    if( _cm_list == NIL ) {\
	 get_blocs( c, &_cm_list ) ; \
         _cm_list = gen_nreverse( _cm_list ) ; \
    }\
    MAPL( _cm_ctls, {control ctl = CONTROL( CAR( _cm_ctls )) ; \
 \
		 code ;}, \
	  _cm_list ) ; \
   if( _cm_list_init == NIL ) \
        list = _cm_list ; \
}
*/

#define CONTROL_MAP( ctl, code, c, list ) \
    GENERIC_CONTROL_MAP( control_map_get_blocs, ctl, code, c, list ) 

#define BACKWARD_CONTROL_MAP( ctl, code, c, list ) \
    GENERIC_CONTROL_MAP( backward_control_map_get_blocs, ctl, code, c, list ) 

#define FORWARD_CONTROL_MAP( ctl, code, c, list ) \
    GENERIC_CONTROL_MAP( forward_control_map_get_blocs, ctl, code, c, list ) 

#define WIDE_FORWARD_CONTROL_MAP( ctl, code, c, list ) \
    GENERIC_CONTROL_MAP( wide_forward_control_map_get_blocs, ctl, code, c, list ) 

#define GENERIC_CONTROL_MAP( get_controls, ctl, code, c, list ) \
{ \
    cons *_cm_list_init = (list) ; \
    cons *_cm_list = _cm_list_init ; \
    if( _cm_list == NIL ) {\
	 get_controls( c, &_cm_list ) ; \
         _cm_list = gen_nreverse( _cm_list ) ; \
    }\
    MAPL( _cm_ctls, {control ctl = CONTROL( CAR( _cm_ctls )) ; \
 \
		 code ;}, \
	  _cm_list ) ; \
   if( _cm_list_init == NIL ) \
        list = _cm_list ; \
}

/* that is all for ri-util-local.h
 */
