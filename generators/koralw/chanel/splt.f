C
C      ************************************************************
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *             ============================                 *
C      *                CHANEL   (Version. 2)                     *
C      *             ============================                 *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                   Written by H. Tanaka                   *
C      *                                                          *
C      *                                                          *
C      *                        ---------                         *
C      *                        Reference                         *
C      *                        ---------                         *
C      *    ' Numerical Calculation of Helicity Amplitudes        *
C      *      for Processes Involving Massive Fermions'           *
C      *                                                          *
C      *      H. Tanaka, Hiroshima University preprint            *
C      *         Accepted to Comput. Phys. Commun.                *
C      *         for version 1.                                   *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *       Version 1 should be added.                         *
C      *                                                          *
C      *                                                          *
C      ************************************************************
C
C
C ********************************************************************
C *                                                                  *
C *  SUBROUTINE SPLT(AM:R*8, P:R*8(4), S1:R*8, S2:R*8,               *
C * &                P1:R*8(4), P2:R*8(4))                           *
C *                                                                  *
C * Decompose off-shell momentum P to two on-shell momenta P1 and P2 *
C *                                                                  *
C *   P = S1*P1 + S2*P2                                              *
C *   P1**2 = 0,      S1 = +-1 (or = 0 for P1 = 0)                   *
C *   P2**2 = AM**2,  S2 = +-1 (or = 0 for P2 = 0)                   *
C *                                        ??????                    *
C *                                        consistent?               *
C *   This subroutine is not included in CHANTL version 1.1.         *
C *                                                                  *
C *                                            written by H. Tanaka  *
C ********************************************************************
C
        SUBROUTINE SPLT(AM,P,S1,S2,P1,P2)
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        PARAMETER (ONE = 1.0)
 
CT      REAL*16 PP,P0,P10,P20,AA,DEN0,DEN,A,B,P00,PPP
        DIMENSION P(4),P1(4),P2(4)
        DATA DD/1.0D-7/
 
CX      DDD=P(4)**2+P(1)**2+P(2)**2+P(1)**2
        DEN0  = P(4) - P(2)
        ADEN0 = ABS(DEN0)
CX      IF (P(4).EQ.0.0D0) THEN
CX        P10 = 0.0D0
CX        S1  = 0.0D0
CX      ELSE
          IF (ADEN0.LE.DD) THEN
            C   = 0.75D0
            A   = SQRT(C)
            B   = 0.5D0
            DEN = P(4)-A*P(1)-B*P(2)
          ELSE
            A   = 0.D0
            B   = 1.D0
            DEN = DEN0
          END IF
          PP   = P(4)*P(4) - P(1)*P(1) - P(2)*P(2) - P(3)*P(3)
          P0   = (PP - AM**2)/(2.0D0*DEN)
          P10  = ABS(P0)
          IF (P10.LE.0.0D0) THEN
            S1  = 0.D0
          ELSE
            S1  = SIGN(ONE, P0)
          END IF
CX      END IF
        P1(1) = A*P10
        P1(2) = B*P10
        P1(3) =   0.D0
        P1(4) =   P10
 
        P00=P(4)-S1*P1(4)
        P20=ABS(P00)
        IF (P20.LE.0.0D0) THEN
          S2 = 0.0D0
        ELSE
          S2 = SIGN(ONE, P00)
        END IF
 
CT      P2(1) = S2*(QEXTD(P(1)) - QEXTD(S1*P1(1)))
CT      P2(2) = S2*(QEXTD(P(2)) - QEXTD(S1*P1(2)))
CT      P2(3) = S2*(QEXTD(P(3)) - QEXTD(S1*P1(3)))
CT      P2(4) = S2*(QEXTD(P(4)) - QEXTD(S1*P1(4)))
        P2(1) = S2*(P(1) - S1*P1(1))
        P2(2) = S2*(P(2) - S1*P1(2))
        P2(3) = S2*(P(3) - S1*P1(3))
        P2(4) = S2*(P(4) - S1*P1(4))
CX      PPP   = AM**2 + P2(1)**2 + P2(2)**2 + P2(3)**2
CX      P2(4) = SQRT(PPP)
 
        RETURN
      END
