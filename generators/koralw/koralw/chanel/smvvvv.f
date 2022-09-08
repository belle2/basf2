************************************************************************
      SUBROUTINE SMVVVV(L1,L2,L3,L4,CPL,E1,E2,E3,E4,LT,AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0)
*   * dummy array size.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      INTEGER    L1, L2, L3, L4
      DOUBLE COMPLEX CPL
      DIMENSION  E1(4,L1), E2(4,L2), E3(4,L3), E4(4,L4)
      DOUBLE COMPLEX AV(0:LASIZE)
*     DOUBLE COMPLEX AV(0:L2*L1-1)
      INTEGER    LT(0:LTSIZE)
*
*    Calculate vector-vector-vector-vector vertex.
*
*           ! 4
*           V
*      -->--+--<---
*        1  ^  3
*           !2
*
*     L1,L2,L3,L4 : input  : number of polarization vectors (2, 3 OR 4)
*     CPL         : input  : coupling constant.
*     E1,E2,E3,E4 : input  : set of polarization vectors
*     AV          : output : table of amplitudes
*     LT          : output : table of sizes in AV
*
*-----------------------------------------------------------------------
      LT(0) = 4
      LT(1) = L1
      LT(2) = L2
      LT(3) = L3
      LT(4) = L4
*TI
*      write(6,*) ' <<SMVVVV>> LT = ',LT
*      write(6,*) ' <<SMVVVV>> L4 = ',L4
      IA = 0
      DO 500 IL4 = 1, L4
      DO 500 IL3 = 1, L3
      DO 500 IL2 = 1, L2
      DO 500 IL1 = 1, L1
        CALL VVVV(1.0D0,E1(1,IL1),E2(1,IL2),E3(1,IL3),E4(1,IL4),AV0)
        AV(IA) = CPL*AV0
        IA = IA + 1
  500 CONTINUE
*TI 25th Nov. 1991 for INTEL
*      LT(0) = 4
*      LT(1) = L1
*      LT(2) = L2
*      LT(3) = L3
*      LT(4) = L4
*      write(6,*) ' <<SMVVVV>> LT = ',LT
*      write(6,*) ' <<SMVVVV>> L4 = ',L4
*     CALL CTIME('SMVVVV')
      RETURN
      END
