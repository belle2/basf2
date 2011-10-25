C
C
C *******************************************************************
C *                                                                 *
C *         S                                                       *
C *         |                                                       *
C *         |                                                       *
C * F1--<---|---<---F2    SUBROUTINE FFS0                           *
C *                                                                 *
C *                                                                 *
C *                                                                 *
C *                                                                 *
C *   Purpose: To calculate vertex amplitudes for scalor boson-     *
C *            massless fermions vertex.                            *
C *                                                                 *
C *   P1,P2=: Momenta of massless fermions.                         *
C *   AALL=: Calculated results for vertex amplitudes.              *
C *                                                                 *
C *                                                                 *
C *                                                                 *
C *                                                                 *
C *                                          written by H. Tanaka   *
C *******************************************************************
C
C
C               ============================
                SUBROUTINE FFS0(P1,P2,AALL)
C               ============================
C
C
C
C    FFS0     FUNCTION S(P1,P2) AND T(P1,P2)
C
C      X(+,P1)X(-,P2)  :  2
C      X(-,P1)X(+,P2)  :  1
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        DOUBLE PRECISION P1,P2
        DOUBLE COMPLEX AALL
        DIMENSION P1(4),P2(4),AALL(2)
 
        IF (P1(4).LE.0.0D0 .OR. P2(4).LE.0.0D0) THEN
          AALL(1) = 0.0D0
          AALL(2) = 0.0D0
CX        WRITE(6,*) ' AALL = 0 '
          RETURN
        END IF
 
        PT1  = SQRT(P1(2)**2 + P1(3)**2)
        IF(PT1.LE.0.0D0) THEN
          RR1Y = SIGN(1.0D0, P1(1))
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
          RR2Y = SIGN(1.0D0, P2(1))
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
 
        RR  =  RPP2*R1Y-RPP1*R2Y
        RIMM = RPP2*R1Z-RPP1*R2Z
 
        AALL(2)=DCMPLX(RR,RIMM)
        AALL(1)=-DCONJG(AALL(2))
 
        RETURN
      END
