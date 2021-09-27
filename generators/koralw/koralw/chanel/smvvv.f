************************************************************************
      SUBROUTINE SMVVV(L1,L2,L3,K1,K2,K3,CPL,P1,P2,P3,E1,E2,E3,LT,AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0)
*   * dummy array size.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      INTEGER    L1, L2, L3
      INTEGER    K1, K2, K3
      DOUBLE COMPLEX CPL
      DIMENSION  P1(4),    P2(4),    P3(4)
      DIMENSION  E1(4,L1), E2(4,L2), E3(4,L3)
      DOUBLE COMPLEX AV(0:LASIZE)
*     DOUBLE COMPLEX AV(0:L3*L2*L1-1)
      INTEGER    LT(0:LTSIZE)
*
*    Calculate vector-vector-vector vertex.
*
*           ! 3
*           V
*      -->--+--<---
*        2     1
*
*     L1,L2,L3 : input  : number of polarization vectors (2, 3 OR 4)
*     K1,K2,K3 : input  : if incoming momentum then 1 else -1
*     CPL      : input  : coupling constant.
*     P1,P2,P3 : input  : momentum vectors
*     E1,E2,E3 : input  : set of polarization vectors
*     AV       : output : table of amplitudes
*     LT       : output : table of sizes in AV
*
      DIMENSION  PP12(4), PP23(4), PP31(4)
*-----------------------------------------------------------------------
      LT(0) = 3
      LT(1) = L1
      LT(2) = L2
      LT(3) = L3
      DO 10 J = 1, 4
        PP12(J) = K1*P1(J) - K2*P2(J)
        PP23(J) = K2*P2(J) - K3*P3(J)
        PP31(J) = K3*P3(J) - K1*P1(J)
   10 CONTINUE
      IA = 0
      DO 500 IL3 = 1, L3
        E3P1P2=E3(4,IL3)*PP12(4)-E3(1,IL3)*PP12(1)
     &        -E3(2,IL3)*PP12(2)-E3(3,IL3)*PP12(3)
      DO 500 IL2 = 1, L2
        E2P3P1=E2(4,IL2)*PP31(4)-E2(1,IL2)*PP31(1)
     &        -E2(2,IL2)*PP31(2)-E2(3,IL2)*PP31(3)
        E2E3=E2(4,IL2)*E3(4,IL3)-E2(1,IL2)*E3(1,IL3)
     &      -E2(2,IL2)*E3(2,IL3)-E2(3,IL2)*E3(3,IL3)
      DO 500 IL1 = 1, L1
CX      CALL VVV(1.0D0,PP1,PP2,PP3,E1(1,IL1),E2(1,IL2),E3(1,IL3),AV0)
        E1P2P3=E1(4,IL1)*PP23(4)-E1(1,IL1)*PP23(1)
     &        -E1(2,IL1)*PP23(2)-E1(3,IL1)*PP23(3)
        E1E2=E1(4,IL1)*E2(4,IL2)-E1(1,IL1)*E2(1,IL2)
     &      -E1(2,IL1)*E2(2,IL2)-E1(3,IL1)*E2(3,IL2)
        E1E3=E1(4,IL1)*E3(4,IL3)-E1(1,IL1)*E3(1,IL3)
     &      -E1(2,IL1)*E3(2,IL3)-E1(3,IL1)*E3(3,IL3)
        AV(IA) = CPL*(E3P1P2*E1E2+E1P2P3*E2E3+E2P3P1*E1E3)
        IA = IA + 1
  500 CONTINUE
*     CALL CTIME('SMVVV ')
      RETURN
      END
