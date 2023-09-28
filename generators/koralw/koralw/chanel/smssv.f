************************************************************************
      SUBROUTINE SMSSV(L1,K2,K3,CPL,P2,P3,E1,LT,AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0)
*   * dummy array size.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      INTEGER    L1
      INTEGER    K2, K3
      DOUBLE COMPLEX CPL
      DIMENSION  P2(4),    P3(4)
      DIMENSION  E1(4,L1)
      DOUBLE COMPLEX AV(0:LASIZE)
*     DOUBLE COMPLEX AV(0:L1-1)
      INTEGER    LT(0:LTSIZE)
*
*    Calculate vector-scalar-scalar vertex.
*
*           ! 3(S)
*           V
*      -->--+--<---
*      2(S)     1(V)
*
*     L1       : input  : number of polarization vectors (2, 3 OR 4)
*     K2,K3    : input  : if incoming momentum then 1 else -1
*     CPL      : input  : coupling constant.
*     P2,P3    : input  : momentum vectors
*     E1       : input  : set of polarization vectors
*     AV       : output : table of amplitudes
*     LT       : output : table of sizes in AV
*
      DIMENSION  PP2(4), PP3(4)
*-----------------------------------------------------------------------
      LT(0) = 3
      LT(1) = 1
      LT(2) = 1
      LT(3) = L1
      DO 10 J = 1, 4
        PP2(J) = K2*P2(J)
        PP3(J) = K3*P3(J)
   10 CONTINUE
      IA = 0
      DO 500 IL1 = 1, L1
        CALL SSV(1.0D0,PP2,PP3,E1(1,IL1),AV0)
        AV(IA) = CPL*AV0
        IA = IA + 1
  500 CONTINUE
*     CALL CTIME('SMSSV ')
      RETURN
      END
