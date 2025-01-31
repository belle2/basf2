C
C ****************************************************************
C *                                                              *
C *                                                              *
C *   SUBROUTINE SPLTQ(AM:R*8, P:R*8(4), P2:R*8(4), P1:R*8(4))   *
C *                                                              *
C * Purpose: To decompose momentum of massive fermions to two    *
C *          light-like vectors                                  *
C *                                                              *
C *                                                              *
C *                                                              *
C *                                                              *
C *      AM= : Mass of fermion.                                  *
C *      P = : Momentum of fermion.                              *
C *      P1,P2= : Decomposed light-like vectors.                 *
C *                                                              *
C *                                                              *
C *                                    written by H. Tanaka      *
C ****************************************************************
C
C
C               ============================
                SUBROUTINE SPLTQ(AM,P,P2,P1)
C               ============================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        DIMENSION P(4),P1(4),P2(4)
        DATA DD/0.0D0/
 
        PT1 = P(1)
        PT2 = P(2)
        PT3 = P(3)
        PN  = SQRT(PT1**2 + PT2**2 + PT3**2)
        P10 = (P(4) + PN)*0.5D0
        P20 = AM**2/(4.0*P10)
        IF (PN.LE.DD) THEN
 
          P10   =  AM*0.5D0
          P20   =  P10
 
          P1(1) =  0.D0
          P1(2) =  P10
          P1(3) =  0.D0
          P1(4) =  P10
 
          P2(1) =  0.D0
          P2(2) = -P20
          P2(3) =  0.D0
          P2(4) =  P20
 
        ELSE
 
          ANX=P(1)/PN
          ANY=P(2)/PN
          ANZ=P(3)/PN
 
          P1(1) =  P10*ANX
          P1(2) =  P10*ANY
          P1(3) =  P10*ANZ
          P1(4) =  P10
 
          P2(1) = -P20*ANX
          P2(2) = -P20*ANY
          P2(3) = -P20*ANZ
          P2(4) =  P20
 
        ENDIF
 
        RETURN
      END
