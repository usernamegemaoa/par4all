/* package matrice
 *
 * Neil Butler, Corinne Ancourt, Francois Irigoin, Yi-qing Yang
 *
 */

#ifndef MATRIX_PACKAGE
#define MATRIX_PACKAGE

/* Les matrices sont des matrices pleines, a coeffcients rationnels.
 *
 * Les matrices sont representes par des tableaux d'entiers mono-dimensionnels
 * Elles sont  stockees par colonne ("column-major"), comme en Fortran. 
 * Les indices commencent a 1, toujours comme en Fortran et non comme en C.
 * 
 * 
 * Le denominateur doit etre strictement positif, i.e. plus grand ou egal
 * a un. Un denominateur nul n'aurait pas de sens. Un denominateur negatif
 * doublerait le nombre de representations possibles d'une matrice.
 *
 * Les matrices renvoyees par certaines routines, comme matrix_multiply(),
 * ne sont pas normalisees par le pgcd des coefficients et du denominateur
 * pour des raisons d'efficacite. Mais les routines de test, comme
 * matrix_identity_p(), supposent leurs arguments normalises.
 *
 * Il faudrait sans doute introduire deux niveaux de procedure, un niveau
 * externe sur garantissant la normalisation, et un niveau interne efficace
 * sans normalisation automatique.
 *
 * La bibliotheque utilise une notion de sous-matrice, definie systematiquement
 * par un parametre appele "level". Seuls les elements dont les indices
 * de lignes et de colonnes sont superieurs a level+1
 * sont pris en consideration. Il s'agit donc de sous-matrice dont le
 * premier element se trouve sur la diagonale de la matrice complete et
 * dont le dernier element et le dernier element de la matrice complete.
 *
 * Note: en cas detection d'incoherence, Neil Butler renvoyait un code
 * d'erreur particulier; Francois Irigoin a supprime ces codes de retour
 * et a traite les exceptions par des appels a assert(), et indirectement
 * a abort()
 */

typedef struct Smatrix { 
    int denominator; 
    int number_of_lines; 
    int number_of_columns; 
    int * coefficients;
    } * Pmatrix, Smatrix;


#define MATRIX_UNDEFINED ((Pmatrix) NULL)

/* Allocation et desallocation d'une matrice */
#define matrix_free(m) (free((char *) (m)))

/* Macros d'acces aux elements d'une matrice */

/* int MATRIX_ELEM(int * matrix, int i, int j): acces a l'element (i,j)
 * de la matrice matrix.
 */
#define MATRIX_ELEM(matrix,i,j) ((matrix)->coefficients[(((j)-1)*((matrix)->number_of_lines))+(i)])

/* int MATRIX_DENONIMATOR(matrix): acces au denominateur global d'une matrice matrix */
#define MATRIX_DENOMINATOR(matrix) ((matrix)->denominator)
#define MATRIX_NB_LINES(matrix)  ((matrix)->number_of_lines)
#define MATRIX_NB_COLUMNS(matrix)  ((matrix)->number_of_columns)

/* boolean matrix_triangular_inferieure_p(matrice a):
 * test de triangularite de la matrice a
 */
#define matrix_triangular_inferieure_p(a) \
    matrix_triangular_p(a,TRUE)

/* boolean matrix_triangular_superieure_p(matrice a, int n, int m):
 * test de triangularite de la matrice a
 */
#define matrix_triangular_superieure_p(a) \
    matrix_triangular_p(a,FALSE)

/* MATRIX_RIGHT_INF_ELEM Permet d'acceder des sous-matrices dont le 
 * coin infe'rieur droit (i.e. le premier element) se trouve sur la 
 * diagonal
 */
#define SUB_MATRIX_ELEM(matrix,i,j,level) \
    (matrix->coefficients[((j)-1+(level))*((matrix)->number_of_lines) + (i) + (level)])


#endif /* MATRIX_PACKAGE */

