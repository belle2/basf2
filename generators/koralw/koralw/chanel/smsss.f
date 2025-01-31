************************************************************************
      SUBROUTINE SMSSS(CPL, LT, AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
*   * dummy array size.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      DOUBLE COMPLEX CPL
      DOUBLE COMPLEX AV(0:LASIZE)
*     DOUBLE COMPLEX AV(0:L2*L1-1)
      INTEGER    LT(0:LTSIZE)
*
*    Calculate scalar-scalar-scalar-scalar vertex.
*
*           ! 3
*           V
*      -->--+--<---
*        1     2
*
*     CPL      : input  : coupling constant.
*     AV       : output : table of amplitudes
*     LT       : output : table of sizes in AV
*-----------------------------------------------------------------------
      LT(0) = 3
      LT(1) = 1
      LT(2) = 1
      LT(3) = 1
      AV(0) = CPL
*     CALL CTIME('SMSSS ')
      RETURN
      END
