C   08/03/94 403291234  MEMBER NAME  FFV0M    *.FORT        E2FORT
      SUBROUTINE FFVMM6(LIND,AL,AR,C,Q1,Q,AALL)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ONE = 1.0D0)
      DOUBLE COMPLEX AALL
      DIMENSION  AALL(4,2,2)
      DOUBLE COMPLEX C
      DIMENSION  Q(4),Q1(4)
      DIMENSION  C(2)
      COMMON /CHWORK/
     .        ALL11R,ALL11I,ALL12R,ALL12I,ALL21R,ALL21I,ALL22R,ALL22I,
     .        R,RR,
     .        Q1RPP1,Q1R1Y,Q1R1Z,Q2RPP1,Q2R1Y,Q2R1Z,
     .        P1RPP2,P1R2Y,P1R2Z,P2RPP2,P2R2Y,P2R2Z,
     .        J1,J2,K1,K2,
     .        LQ1,LQ2,LP1,LP2
      LOGICAL    LQ1,LQ2,LP1,LP2
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
          LP13   = .TRUE.
          IF( Q1(4) .LE. 0.0D0 ) LP13 = .FALSE.
C-
          IF( LP13 ) THEN
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
                  P13RPP    = PT1/PS1
              ELSE
                  P13RPP    = SQRT(Q1(4) - Q1(1))
                  PS1       = PT1/P13RPP
              ENDIF
              P13R1Y    = Q1RR1Y*PS1
              P13R1Z    = Q1RR1Z*PS1
          ENDIF
      ENDIF
C- Q1,P1
           IF( LP13.AND.LP1 ) THEN
               ALL11R = P13RPP*P1RPP2*(Q(4)+Q(1))
     .                 -P13RPP*(Q(2)*P1R2Y +Q(3)*P1R2Z)
     .                 -P1RPP2*(Q(2)*P13R1Y+Q(3)*P13R1Z)
     .                 +(Q(4)-Q(1))*(P13R1Y*P1R2Y+P13R1Z*P1R2Z)
               ALL11I = P13RPP*(Q(2)*P1R2Z -Q(3)*P1R2Y)
     .                 +P1RPP2*(Q(3)*P13R1Y-Q(2)*P13R1Z)
     .                 +(Q(4)-Q(1))*(P13R1Z*P1R2Y-P13R1Y*P1R2Z)
           ENDIF

           AALL(LIND,1,K1)= AL*DCMPLX(ALL11R,-ALL11I)
           AALL(LIND,2,K2)= AR*DCMPLX(ALL11R, ALL11I)

      RETURN
      END
