        SUBROUTINE FFVMM8(LIND,AL,AR,CC,P1,Q,AALL)
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        PARAMETER (ONE = 1.0D0)
        DOUBLE COMPLEX AALL
        DIMENSION  AALL(4,2,2)
        DOUBLE COMPLEX CC
        DIMENSION  Q(4),P1(4)
        DIMENSION  CC(2)
        COMMON /CHWORK/
     .        ALL11R,ALL11I,ALL12R,ALL12I,ALL21R,ALL21I,ALL22R,ALL22I,
     .        R,RR,
     .        Q1RPP1,Q1R1Y,Q1R1Z,Q2RPP1,Q2R1Y,Q2R1Z,
     .        P1RPP2,P1R2Y,P1R2Z,P2RPP2,P2R2Y,P2R2Z,
     .        J1,J2,K1,K2,
     .        LQ1,LQ2,LP1,LP2
      LOGICAL    LQ1,LQ2,LP1,LP2
C
      COMMON /WRKFFX/P13RPP,P13R1Y,P13R1Z,P23RPP,P23R2Y,P23R2Z,LP13,LP23
      LOGICAL    LP13,LP23

*------------------------ Entry point ----------------------------------
         ALL11R = 0.0D0
         ALL11I = 0.0D0
         ALL12R = 0.0D0
         ALL12I = 0.0D0
         ALL21R = 0.0D0
         ALL21I = 0.0D0
         ALL22R = 0.0D0
         ALL22I = 0.0D0

      DO 90 I3= 1 , 2
      DO 90 I2= 1 , 2
         AALL(LIND,I2,I3)=(0.0D0,0.0D0)
   90 CONTINUE
      IF( LIND .EQ. 1 ) THEN
           LP23      = .TRUE.
           IF( P1(4) .LE. 0.0D0 ) LP23 = .FALSE.
C-
           IF( LP23 ) THEN
               PT2     = SQRT(P1(2)*P1(2) + P1(3)*P1(3))
               IF( PT2 .LE. 0.0D0) THEN
                   P1RR2Y    = SIGN(ONE, P1(1))
                   P1RR2Z    = 0.0D0
               ELSE
                   P1RR2Y    = P1(2)/PT2
                   P1RR2Z    = P1(3)/PT2
               ENDIF
               IF( P1(1) .GE. 0.D0) THEN
                   PS2       = SQRT(P1(4) + P1(1))
                   P23RPP    = PT2/PS2
               ELSE
                   P23RPP    = SQRT(P1(4) - P1(1))
                   PS2       = PT2/P23RPP
               ENDIF
               P23R2Y    = P1RR2Y*PS2
               P23R2Z    = P1RR2Z*PS2
           ENDIF
      ENDIF
C- Q1,P1
           IF( LQ1.AND.LP23 ) THEN
               ALL11R = Q1RPP1*P23RPP*(Q(4)+Q(1))
     .                 -Q1RPP1*(Q(2)*P23R2Y+Q(3)*P23R2Z)
     .                 -P23RPP*(Q(2)*Q1R1Y +Q(3)*Q1R1Z)
     .                 +(Q(4)-Q(1))*(Q1R1Y*P23R2Y+Q1R1Z*P23R2Z)
               ALL11I = Q1RPP1*(Q(2)*P23R2Z-Q(3)*P23R2Y)
     .                 +P23RPP*(Q(3)*Q1R1Y -Q(2)*Q1R1Z)
     .                 +(Q(4)-Q(1))*(Q1R1Z*P23R2Y-Q1R1Y*P23R2Z)
           ENDIF
C---
           AALL(LIND,J1,1) = AL*DCMPLX(ALL11R,-ALL11I)
           AALL(LIND,J2,2) = AR*DCMPLX(ALL11R, ALL11I)

      RETURN
      END
