************************************************************************
      SUBROUTINE SMSVV (L1,L2,CPL,E1,E2,LT,AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0)
*   * dummy array size.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      INTEGER    L1, L2
      DOUBLE COMPLEX CPL
      DIMENSION  E1(4,L1), E2(4,L2)
      DOUBLE COMPLEX AV(0:LASIZE)
*     DOUBLE COMPLEX AV(0:L2*L1-1)
      INTEGER    LT(0:LTSIZE)
*
*    Calculate vector-vector-scalar vertex.
*
*           ! 3(S)
*           V
*      -->--+--<---
*      2(V)     1(V)
*
*     L1,L2    : input  : number of polarization vectors (2, 3 OR 4)
*     CPL      : input  : coupling constant.
*     E1,E2    : input  : set of polarization vectors
*     AV       : output : table of amplitudes
*     LT       : output : table of sizes in AV
*
*-----------------------------------------------------------------------
      LT(0) = 3
      LT(1) = 1
      LT(2) = L1
      LT(3) = L2
      IA = 0
      DO 500 IL2 = 1, L2
      DO 500 IL1 = 1, L1
        CALL VVS(1.0D0,E1(1,IL1),E2(1,IL2),AV0)
        AV(IA) = CPL*AV0
        IA = IA + 1
  500 CONTINUE
*     CALL CTIME('SMSVV ')
      RETURN
      END
