!
! inclusions for run-time support functions
!
! $RCSfile: hpfc_commons.h,v $ version $Revision$
! ($Date: 1997/01/02 18:41:25 $, )
!
!
      include 'hpfc_parameters.h'
      include 'hpfc_param.h'
      include 'hpfc_globs.h'
      include 'hpfc_procs.h'
      include 'hpfc_buffers.h'
      include 'hpfc_misc.h'
!
! parameters value to be defined when starting a program.
!
      integer
     $     NBOFARRAYS,
     $     NBOFTEMPLATES,
     $     NBOFPROCESSORS,
     $     MAXSIZEOFPROCS,
     $     SIZEOFBUFFER
      common /hpfc dynamic/ 
     $     NBOFARRAYS,
     $     NBOFTEMPLATES,
     $     NBOFPROCESSORS,
     $     MAXSIZEOFPROCS,
     $     SIZEOFBUFFER
!
! that s all
!
