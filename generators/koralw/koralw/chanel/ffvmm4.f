      SUBROUTINE FFVMM4(LIND,AL,AR,Q,AALL)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DOUBLE COMPLEX AALL
      DIMENSION AALL(4,2,2)
      DIMENSION Q(4)
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

      DO 90 I3= 1 , 2
      DO 90 I2= 1 , 2
         AALL(LIND,I2,I3)=(0.0D0,0.0D0)
   90 CONTINUE
C- Q1,P1
           IF( LP13.AND.LP23 ) THEN
               ALL11R = P13RPP*P23RPP*(Q(4)+Q(1))
     .                 -P13RPP*(Q(2)*P23R2Y+Q(3)*P23R2Z)
     .                 -P23RPP*(Q(2)*P13R1Y+Q(3)*P13R1Z)
     .                 +(Q(4)-Q(1))*(P13R1Y*P23R2Y+P13R1Z*P23R2Z)
               ALL11I = P13RPP*(Q(2)*P23R2Z-Q(3)*P23R2Y)
     .                 +P23RPP*(Q(3)*P13R1Y-Q(2)*P13R1Z)
     .                 +(Q(4)-Q(1))*(P13R1Z*P23R2Y-P13R1Y*P23R2Z)
           ENDIF

           AALL(LIND,1,1) = AL*DCMPLX(ALL11R,-ALL11I)
           AALL(LIND,2,2) = AR*DCMPLX(ALL11R, ALL11I)

        RETURN
      END
