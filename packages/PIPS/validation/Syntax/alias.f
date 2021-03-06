      DOUBLE PRECISION FUNCTION ALIAS(ANAM)

c     Problem: detection of Hollerith constants in DATA statements

      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION ANAM1(14),ANAM2(14)
      DATA ANAM1 /3HVA ,3HVB ,3HCCS,3HNS ,3HC2 ,3HPT ,3HC4 ,
     1     3HPE ,3HME ,3HPC ,3HMC ,3HPS ,3HMS ,3HIK /
      DATA ANAM2 /3HVAF,3HVAR,3HCJS,3HNSS,3HISE,3HXTI,3HISC,
     1            3HVJE,3HMJE,3HVJC,3HMJC,3HVJS,3HMJS,3HIKF/
C     
C     THIS FUNCTION RETURNS THE MGP EQUIVALENT OF THE GP PARAMETERS
C     (THOSE WHICH APPLY)
C     
      IKNT=0
      ALIAS=ANAM
 10   IKNT=IKNT+1
      IF(IKNT.GT.14) RETURN
      IF(ANAM1(IKNT).NE.ANAM) GO TO 10
      ALIAS=ANAM2(IKNT)
      RETURN
      END
