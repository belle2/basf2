C
C *******************************************************************
C *                                                                 *
C *                                                                 *
C *                                                                 *
C *   SUBROUTINE FFV0(P1:R*8(4), P2:R*8(4), P:R*8(4), AALL:C*16(2)) *
C *                                                                 *
C *                                                                 *
C *                                                                 *
C *   Purpose: To calculate vertex amplitudes for vector boson-     *
C *            massless fermions vertex.                            *
C *                                                                 *
C *   P1,P2=: Momentum of massless fermions.                        *
C *   P=: Polarization vector of vector boson coupled to fermion.   *
C *   AALL=: Calculated results of vertex amplitudes.               *
C *                                                                 *
C *                                                                 *
C *                                                                 *
C *                                                                 *
C *                                          written by H. Tanaka   *
C *******************************************************************
C
C
C               ============================
                SUBROUTINE FFV0(P1,P2,P,AALL)
C               ============================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        PARAMETER (ONE = 1.0)
        DOUBLE PRECISION P,P1,P2
        DOUBLE COMPLEX AALL
        DIMENSION P(4),P1(4),P2(4),AALL(2)
 
        IF (P1(4).LE.0.0D0 .OR. P2(4).LE.0.0D0) THEN
          AALL(1) = 0.0D0
          AALL(2) = 0.0D0
          RETURN
        END IF
 
        PT1  = SQRT(P1(2)**2 + P1(3)**2)
        IF(PT1.LE.0.0D0) THEN
          RR1Y = SIGN(ONE, P1(1))
          RR1Z = 0.0D0
        ELSE
          RR1Y = P1(2)/PT1
          RR1Z = P1(3)/PT1
        ENDIF
        IF(P1(1).GE.0.D0) THEN
          PS1  = SQRT(P1(4) + P1(1))
          RPP1 = PT1/PS1
        ELSE
          RPP1 = SQRT(P1(4) - P1(1))
          PS1  = PT1/RPP1
        ENDIF
        R1Y = RR1Y*PS1
        R1Z = RR1Z*PS1
 
        PT2  = SQRT(P2(2)**2 + P2(3)**2)
        IF(PT2.LE.0.0D0) THEN
          RR2Y = SIGN(ONE, P2(1))
          RR2Z = 0.0D0
        ELSE
          RR2Y = P2(2)/PT2
          RR2Z = P2(3)/PT2
        ENDIF
        IF(P2(1).GE.0.D0) THEN
          PS2  = SQRT(P2(4) + P2(1))
          RPP2 = PT2/PS2
        ELSE
          RPP2 = SQRT(P2(4) - P2(1))
          PS2  = PT2/RPP2
        ENDIF
        R2Y = RR2Y*PS2
        R2Z = RR2Z*PS2
 
        RR   = RPP1*RPP2*(P(4)+P(1))-RPP1*(P(2)*R2Y+P(3)*R2Z)
     *        -RPP2*(P(2)*R1Y+P(3)*R1Z)+(P(4)-P(1))*(R1Y*R2Y+R1Z*R2Z)
        RIMM = RPP1*(P(2)*R2Z-P(3)*R2Y)+RPP2*(P(3)*R1Y-P(2)*R1Z)
     *        +(P(4)-P(1))*(R1Z*R2Y-R1Y*R2Z)
 
        AALL(1)=DCMPLX(RR,-RIMM)
        AALL(2)=DCMPLX(RR, RIMM)
 
        RETURN
      END
