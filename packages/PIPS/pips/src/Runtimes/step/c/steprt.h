/* 
   Private interface
*/
#ifndef _STEPRT_H_
#define _STEPRT_H_

#include <stdlib.h>
#include "step_private.h"
#include "regions.h"
#include "communications.h"

extern Step_internals steprt_params;
extern Array steprt_userArrayTable;
extern Array steprt_worksharingTable;

extern void steprt_print();
extern void steprt_init(int language);
extern void steprt_finalize();
extern void steprt_worksharing_set(worksharing_type type);
extern void steprt_worksharing_unset(void);

extern void steprt_set_userArrayTable(void *userArray, uint32_t type, uint32_t nbdims, INDEX_TYPE *bounds);
extern void steprt_userArrayDescriptor_unset(Descriptor_userArray *desc);
extern Descriptor_userArray *steprt_find_in_userArrayTable(void *userArray);

extern composedRegion *steprt_compute_workchunks(INDEX_TYPE begin, INDEX_TYPE end, INDEX_TYPE incr, STEP_ARG nb_workchunks);
extern void steprt_set_sharedTable(void *userArray, uint32_t nb_workchunks, STEP_ARG *regionsReceive,
				    STEP_ARG *regionsSend, bool is_interlaced);
extern Descriptor_shared *steprt_find_in_sharedTable(void *userArray);

extern void steprt_alltoall(void * userArray, bool full, uint32_t algorithm, int_MPI tag);
extern Descriptor_reduction *steprt_find_reduction(void *scalar);
extern void steprt_initreduction(void *variable, uint32_t op, uint32_t type);
extern void steprt_set_reduction_sendregions(Descriptor_reduction *desc_reduction, uint32_t nb_workchunks, STEP_ARG *regions);
extern void steprt_reduction(void *variable);
extern void util_print_rank(int rank, char *input_format, ...);
#endif
