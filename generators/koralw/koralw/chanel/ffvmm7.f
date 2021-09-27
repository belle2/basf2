*-----------------------------------------------------------------------
*     FFV  revised 94/04/08 by T.Ishikawa
*     AAM =0 , AM != 0
*-----------------------------------------------------------------------
      SUBROUTINE FFVMM7(LIND,II,IO,AL,AR,CC,C,Q1,Q2,P1,P2,Q,AALL)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ONE = 1.0D0)
      DOUBLE COMPLEX AALL
      DIMENSION AALL(4,2,2)
      DOUBLE COMPLEX C,CC
      DIMENSION Q(4),P1(4),P2(4),Q1(4)
      DIMENSION Q2(4),C(2),CC(2)
      INTEGER II,IO
      COMMON /CHWORK/
     .        ALL11R,ALL11I,ALL12R,ALL12I,ALL21R,ALL21I,ALL22R,ALL22I,
     .        R,RR,
     .        Q1RPP1,Q1R1Y,Q1R1Z,Q2RPP1,Q2R1Y,Q2R1Z,
     .        P1RPP2,P1R2Y,P1R2Z,P2RPP2,P2R2Y,P2R2Z,
     .        J1,J2,K1,K2,
     .        LQ1,LQ2,LP1,LP2
      LOGICAL    LQ1,LQ2,LP1,LP2

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
          LQ1      = .TRUE.
          LQ2      = .TRUE.
          LP1      = .TRUE.
          LP2      = .TRUE.
          IF( Q1(4) .LE. 0.0D0 ) LQ1 = .FALSE.
          IF( Q2(4) .LE. 0.0D0 ) LQ2 = .FALSE.
          IF( P1(4) .LE. 0.0D0 ) LP1 = .FALSE.
          IF( P2(4) .LE. 0.0D0 ) LP2 = .FALSE.
C-
          IF( LQ1 ) THEN
               PT1        = SQRT(Q1(2)*Q1(2) + Q1(3)*Q1(3))
               IF( PT1 .LE. 0.0D0) THEN
                   Q1RR1Y    = SIGN(ONE, Q1(1))
                   Q1RR1Z    = 0.0D0
               ELSE
                   Q1RR1Y    = Q1(2)/PT1
                   Q1RR1Z    = Q1(3)/PT1
               ENDIF
               IF( Q1(1).GE.0.D0) THEN
                   PS1       = SQRT(Q1(4) + Q1(1))
                   Q1RPP1    = PT1/PS1
               ELSE
                   Q1RPP1    = SQRT(Q1(4) - Q1(1))
                   PS1       = PT1/Q1RPP1
               ENDIF
               Q1R1Y    = Q1RR1Y*PS1
               Q1R1Z    = Q1RR1Z*PS1
           ENDIF
C-
           IF( LP1 ) THEN
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
                   P1RPP2    = PT2/PS2
               ELSE
                   P1RPP2    = SQRT(P1(4) - P1(1))
                   PS2       = PT2/P1RPP2
               ENDIF
               P1R2Y    = P1RR2Y*PS2
               P1R2Z    = P1RR2Z*PS2
           ENDIF

           IF( LP2 ) THEN
               PT2     = SQRT(P2(2)*P2(2) + P2(3)*P2(3))
               IF( PT2 .LE. 0.0D0) THEN
                   P2RR2Y    = SIGN(ONE, P2(1))
                   P2RR2Z    = 0.0D0
               ELSE
                   P2RR2Y    = P2(2)/PT2
                   P2RR2Z    = P2(3)/PT2
               ENDIF
               IF( P2(1) .GE. 0.D0) THEN
                   PS2       = SQRT(P2(4) + P2(1))
                   P2RPP2    = PT2/PS2
               ELSE
                   P2RPP2    = SQRT(P2(4) - P2(1))
                   PS2       = PT2/P2RPP2
               ENDIF
               P2R2Y    = P2RR2Y*PS2
               P2R2Z    = P2RR2Z*PS2
           ENDIF

           R     = DBLE(IO - 2)
           RR    = DBLE(II - 2)

C
           J1    = (5-II)/2
           J2    = 3 - J1
           K1    = (5-IO)/2
           K2    = 3 - K1

      ENDIF
C- Q1,P1
           IF( LQ1.AND.LP1 ) THEN
               ALL11R = Q1RPP1*P1RPP2*(Q(4)+Q(1))
     .                 -Q1RPP1*(Q(2)*P1R2Y+Q(3)*P1R2Z)
     .                 -P1RPP2*(Q(2)*Q1R1Y+Q(3)*Q1R1Z)
     .                 +(Q(4)-Q(1))*(Q1R1Y*P1R2Y+Q1R1Z*P1R2Z)
               ALL11I = Q1RPP1*(Q(2)*P1R2Z-Q(3)*P1R2Y)
     .                 +P1RPP2*(Q(3)*Q1R1Y-Q(2)*Q1R1Z)
     .                 +(Q(4)-Q(1))*(Q1R1Z*P1R2Y-Q1R1Y*P1R2Z)
           ENDIF

C- Q1,P2
           IF( LQ1.AND.LP2 ) THEN
               ALL12R = Q1RPP1*P2RPP2*(Q(4)+Q(1))
     .                 -Q1RPP1*(Q(2)*P2R2Y+Q(3)*P2R2Z)
     .                 -P2RPP2*(Q(2)*Q1R1Y+Q(3)*Q1R1Z)
     .                 +(Q(4)-Q(1))*(Q1R1Y*P2R2Y+Q1R1Z*P2R2Z)
               ALL12I = Q1RPP1*(Q(2)*P2R2Z-Q(3)*P2R2Y)
     .                 +P2RPP2*(Q(3)*Q1R1Y-Q(2)*Q1R1Z)
     .                 +(Q(4)-Q(1))*(Q1R1Z*P2R2Y-Q1R1Y*P2R2Z)
           ENDIF

C---
           AALL(LIND,J1,K1)= AL*DCMPLX(ALL11R,-ALL11I)
           AALL(LIND,J2,K2)= AR*DCMPLX(ALL11R, ALL11I)
           AALL(LIND,J1,K2)= -(AL*R*C(1)*DCMPLX(ALL12R,-ALL12I))
           AALL(LIND,J2,K1)= -(AR*R*C(2)*DCMPLX(ALL12R, ALL12I))

      RETURN
      END
