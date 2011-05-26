/*
 * trace.h
 *
 * NOT THREAD-SAFE
 *
 * Module de traces:
 * trace les fonctions presentes dans un fichier de traces
 *
 *
 * La macro IN_TRACE declare des variables. 
 * Elle doit donc �tre appelee en fin de bloc de declaration de variables.
 *
 * Si Output_trace_file est positionne, 
 * les traces seront ecrites dans le fichier 
 * de nom $Output_trace_file ou $Output_trace_file_$rang 
 * pour execution du processus de rang $rang.
 *
 *
 * README
 *
 * Activation des traces en compilant avec -DTRACE
 *
 * Inclure le fichier trace.h
 *
 * Dans le code source,
 *   appeler la macro SET_TRACES dans le main
 *   dans chaque fonction
 *     appeler IN_TRACE en debut de fonction
 *     appeler OUT_TRACE en fin de fonction
 *   dans une fonction
 *     appeler TRACE_P avec le message de trace
 *
 * A l'execution, creer un fichier (avec le nom specifier dans SET_TRACES) 
 *   contenant le nom des fonctions a tracer.
 *
 * Structure du fichier:
 * <code1>[arg1]<nom de fonction 1>
 * <code2>[arg2]<nom de fonction 2>
 * <code3>[arg3]<nom de fonction 3>
 * <code4>[arg4]<nom de fonction 4> 
 * . 
 * 
 * ATTENTION le '.' final est IMPORTANT
 *
 * Le code peut etre :
 * + : traces par defaut
 * = : mesures de temps
 * > arg : la fonction n'est tracee que pour le processus de rang arg
 *
 */

#ifndef _TRACE_H_
#define _TRACE_H_


/*
 * ATTENTION ATTENTION: NOT THREAD-SAFE
 *
 * utilisation de variables statiques.
 *
 */

#ifdef __cplusplus
extern "C" 
{
#endif



#ifdef TRACE
extern void set_traces(char *infilename, char *outfilename, int nbprocs, int myrank) ;
extern void trace(char *function, int when, ...);
extern int is_traced(char *function, int *index, int *time_p);
extern void time_trace_in(char *nom, int index);
extern void time_trace_out(char *nom, int index);
extern void timings_dump_traces();
/*
 * Utilisation de la variable IN_TRACE dans OUT_TRACE 
 * pour v�rifier la pr�sence de la macro IN_TRACE
 *
 */

#define FUNC_NAME_MAXLEN 80
#ifdef TRACE_TIMINGS
#define IN_TRACE(...) \
  int IN_TRACE = 1; \
  static int TR_first_p = 1; \
  static int TR_trace_index = -1; \
  static int TR_traced_p = 0; \
  static int TR_time_p = 0; \
  char TRACE_Func_name[FUNC_NAME_MAXLEN]; \
  sprintf(TRACE_Func_name, "%s", __func__); \
  if (TR_first_p) \
    { \
      TR_traced_p = is_traced(TRACE_Func_name, &TR_trace_index, &TR_time_p); \
      TR_first_p = 0; \
    } \
  if (TR_traced_p) \
    { \
      if (TR_time_p) \
        time_trace_in(TRACE_Func_name, TR_trace_index); \
      else \
        trace(TRACE_Func_name, 'i', __VA_ARGS__);	\
    } 

#define OUT_TRACE(...) \
   IN_TRACE = 0; \
   if (TR_traced_p) \
     { \
       if (TR_time_p) \
         time_trace_out(TRACE_Func_name, TR_trace_index); \
       else \
         trace(TRACE_Func_name, 'o', __VA_ARGS__); \
     } 
#else
#define IN_TRACE(...) \
  int IN_TRACE = 1; \
  static int TR_first_p = 1; \
  static int TR_trace_index = -1; \
  static int TR_traced_p = 0; \
  static int TR_time_p = 0; \
  char TRACE_Func_name[FUNC_NAME_MAXLEN]; \
  sprintf(TRACE_Func_name, "%s", __func__); \
  if (TR_first_p) \
    { \
      TR_traced_p = is_traced(TRACE_Func_name, &TR_trace_index, &TR_time_p); \
      TR_first_p = 0; \
    } \
  if (TR_traced_p) \
    { \
      trace(TRACE_Func_name, 'i', __VA_ARGS__);	\
    } 

#define OUT_TRACE(...) \
   IN_TRACE = 0; \
   if (TR_traced_p) \
     { \
       trace(TRACE_Func_name, 'o', __VA_ARGS__); \
     } 
#endif

#define TRACE_P(...) \
   IN_TRACE = 0; \
   if (TR_traced_p && !TR_time_p) \
     trace(TRACE_Func_name, ' ', __VA_ARGS__)

#define SET_TRACES(infilename, outfilename, nbprocs, myrank) \
   set_traces(infilename, outfilename, nbprocs, myrank);

#define TIMINGS_DUMP_TRACES(filename) \
   timings_dump_traces(filename);

#else

#define IN_TRACE(...) {}

#define TRACE_P(...) {}

#define OUT_TRACE(...) {}

#define SET_TRACES(infilename, outfilename, nbprocs, myrank) {}
#define TIMINGS_DUMP_TRACES(filename) {}

#endif

#ifdef __cplusplus
}
#endif

#endif
