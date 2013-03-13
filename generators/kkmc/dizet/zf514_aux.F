*
* Below are auxiliary functions for ZFITTER package
*
      SUBROUTINE FDSIMP (AA1,BB1,HH1,REPS1,AEPS1,FUNCT,DFUN,DFUNIN,
     +                                               DUMMY,AI,AIH,AIABS)
*     ==================================================================
C        B1
C AI=INT {FUNCT(X)*DDFUN(X)Y}
C        A1
C A1,B1 -THE LIMITS OF INTEGRATION
C H1    -AN INITIAL STEP OF INTEGRATION
C REPS1,AEPS1 - RELATIVE AND ABSOLUTE PRECISION OF INTEGRATION
C FUNCT -A NAME OF FUNCTION SUBPROGRAM FOR CALCULATION OF INTEGRAND +
C X - AN ARGUMENT OF THE INTEGRAND
C DFUNIN - INVERSE ( DFUN ). SHOULD BE DFUNIN DFUN(X)Y=X.
C AI - THE VALUE OF INTEGRAL
C AIH- THE VALUE OF INTEGRAL WITH THE STEP OF INTEGRATION
C AIABS- THE VALUE OF INTEGRAL FOR MODULE OF THE INTEGRAND
C THIS SUBROGRAM CALCULATES THE DEFINITE INTEGRAL WITH THE RELATIVE OR
C ABSOLUTE PRECISION BY SIMPSON+S METHOD WITH THE AUTOMATICAL CHOICE
C OF THE STEP OF INTEGRATION
C IF AEPS1    IS VERY SMALL(LIKE 1.E-17),THEN CALCULATION OF INTEGRAL
C WITH REPS1,AND IF REPS1 IS VERY SMALL (LIKE 1.E-10),THEN CALCULATION
C OF INTEGRAL WITH AEPS1
C WHEN  AEPS1=REPS1=0. THEN CALCULATION WITH THE CONSTANT STEP H1
C
      IMPLICIT REAL*8(A-H,O-Z)
      DIMENSION F(7),P(5)
      EXTERNAL FUNCT,DFUN,DFUNIN
*
      DUM=DUMMY
      H1=(DFUN(BB1)-DFUN(AA1))/(BB1-AA1+1.654876596E-20)*HH1
      A1=DFUN(AA1)
      B1=DFUN(BB1)
      H=DSIGN(H1,B1-A1+1.654876596E-20)
      S=DSIGN(1.D0,H)
      A=A1
      B=B1
      AI=0.D0
      AIH=0.D0
      AIABS=0.D0
      P(2)=4.D0
      P(4)=4.D0
      P(3)=2.D0
      P(5)=1.D0
      IF(B-A) 1,2,1
    1 REPS=DABS(REPS1)
      AEPS=DABS(AEPS1)
      DO 3 K=1,7
  3   F(K)=10.D16
      X=A
      C=0.D0
      F(1)=FUNCT(DFUNIN(X))/3.
    4 X0=X
      IF((X0+4.*H-B)*S) 5,5,6
    6 H=(B-X0)/4.
      IF(H) 7,2,7
    7 DO 8 K=2,7
  8   F(K)=10.D16
      C=1.D0
    5 DI2=F(1)
      DI3=DABS(F(1))
      DO 9 K=2,5
      X=X+H
      IF((X-B)*S) 23,24,24
   24 X=B
   23 IF(F(K)-10.D16) 10,11,10
   11 F(K)=FUNCT(DFUNIN(X))/3.
   10 DI2=DI2+P(K)*F(K)
    9 DI3=DI3+P(K)*ABS(F(K))
      DI1=(F(1)+4.*F(3)+F(5))*2.*H
      DI2=DI2*H
      DI3=DI3*H
      IF(REPS) 12,13,12
   13 IF(AEPS) 12,14,12
   12 EPS=DABS((AIABS+DI3)*REPS)
      IF(EPS-AEPS) 15,16,16
   15 EPS=AEPS
   16 DELTA=DABS(DI2-DI1)
      IF(DELTA-EPS) 20,21,21
   20 IF(DELTA-EPS/8.) 17,14,14
   17 H=2.*H
      F(1)=F(5)
      F(2)=F(6)
      F(3)=F(7)
      DO 19 K=4,7
  19  F(K)=10.D16
      GO TO 18
   14 F(1)=F(5)
      F(3)=F(6)
      F(5)=F(7)
      F(2)=10.D16
      F(4)=10.D16
      F(6)=10.D16
      F(7)=10.D16
   18 DI1=DI2+(DI2-DI1)/15.
      AI=AI+DI1
      AIH=AIH+DI2
      AIABS=AIABS+DI3
      GO TO 22
   21 H=H/2.
      F(7)=F(5)
      F(6)=F(4)
      F(5)=F(3)
      F(3)=F(2)
      F(2)=10.D16
      F(4)=10.D16
      X=X0
      C=0.D0
      GO TO 5
   22 IF(C) 2,4,2
    2 RETURN
      END
 
      DOUBLE PRECISION FUNCTION DDILOG(X)
*     ====== ========= ======== =========
C
      DOUBLE PRECISION X,Y,T,S,A,PI3,PI6,ZERO,ONE,HALF,MALF,MONE,MTWO
      DOUBLE PRECISION C(0:18),H,ALFA,B0,B1,B2
C
      DATA ZERO /0.0D0/, ONE /1.0D0/
      DATA HALF /0.5D0/, MALF /-0.5D0/, MONE /-1.0D0/, MTWO /-2.0D0/
      DATA PI3 /3.28986 81336 96453D0/, PI6 /1.64493 40668 48226D0/
C
      DATA C( 0) / 0.42996 69356 08137 0D0/
      DATA C( 1) / 0.40975 98753 30771 1D0/
      DATA C( 2) /-0.01858 84366 50146 0D0/
      DATA C( 3) / 0.00145 75108 40622 7D0/
      DATA C( 4) /-0.00014 30418 44423 4D0/
      DATA C( 5) / 0.00001 58841 55418 8D0/
      DATA C( 6) /-0.00000 19078 49593 9D0/
      DATA C( 7) / 0.00000 02419 51808 5D0/
      DATA C( 8) /-0.00000 00319 33412 7D0/
      DATA C( 9) / 0.00000 00043 45450 6D0/
      DATA C(10) /-0.00000 00006 05784 8D0/
      DATA C(11) / 0.00000 00000 86121 0D0/
      DATA C(12) /-0.00000 00000 12443 3D0/
      DATA C(13) / 0.00000 00000 01822 6D0/
      DATA C(14) /-0.00000 00000 00270 1D0/
      DATA C(15) / 0.00000 00000 00040 4D0/
      DATA C(16) /-0.00000 00000 00006 1D0/
      DATA C(17) / 0.00000 00000 00000 9D0/
      DATA C(18) /-0.00000 00000 00000 1D0/
 
      IF(X .EQ. ONE) THEN
       DDILOG=PI6
       RETURN
      ELSE IF(X .EQ. MONE) THEN
       DDILOG=MALF*PI6
       RETURN
      END IF
      T=-X
      IF(T .LE. MTWO) THEN
       Y=MONE/(ONE+T)
       S=ONE
       A=-PI3+HALF*(LOG(-T)**2-LOG(ONE+ONE/T)**2)
      ELSE IF(T .LT. MONE) THEN
       Y=MONE-T
       S=MONE
       A=LOG(-T)
       A=-PI6+A*(A+LOG(ONE+ONE/T))
      ELSE IF(T .LE. MALF) THEN
       Y=(MONE-T)/T
       S=ONE
       A=LOG(-T)
       A=-PI6+A*(MALF*A+LOG(ONE+T))
      ELSE IF(T .LT. ZERO) THEN
       Y=-T/(ONE+T)
       S=MONE
       A=HALF*LOG(ONE+T)**2
      ELSE IF(T .LE. ONE) THEN
       Y=T
       S=ONE
       A=ZERO
      ELSE
       Y=ONE/T
       S=MONE
       A=PI6+HALF*LOG(T)**2
      END IF
 
      H=Y+Y-ONE
      ALFA=H+H
      B1=ZERO
      B2=ZERO
      DO 1 I = 18,0,-1
      B0=C(I)+ALFA*B1-B2
      B2=B1
    1 B1=B0
      DDILOG=-(S*(B0-H*B2)+A)
      RETURN
      END
 
      DOUBLE PRECISION FUNCTION TRILOG(X)
*     ====== ========= ======== =========
C Tsuyoshi Matsuura 1987
C
C     TRILOG: Li3   between 0 and 1  !!!
C
      DOUBLE PRECISION X,S(0:10),L(0:20),U,Z,HELP,Z3
      DOUBLE PRECISION DDILOG
C
C     S: COEFFICIENTS OF S1,2
C     L: COEFFICIENTS OF Li3
C
      DATA S/0.5000000000000000D+00,2.0833333333333333D-02,
     1      -2.3148148148148148D-04,4.1335978835978837D-06,
     2      -8.2671957671957673D-08,1.7397297489890083D-09,
     3      -3.7744215276339238D-11,8.3640853316779243D-13,
     4      -1.8831557201792127D-14,4.2930310281389223D-16,
     5      -9.8857668116275541D-18/
      DATA L/1.0000000000000000D+00,-0.3750000000000000D+00,
     1       7.8703703703703705E-02,-8.6805555555555557E-03,
     2       1.2962962962962963E-04, 8.1018518518518520E-05,
     3      -3.4193571608537598E-06,-1.3286564625850340E-06,
     4       8.6608717561098512E-08, 2.5260875955320400E-08,
     5      -2.1446944683640648E-09,-5.1401106220129790E-10,
     6       5.2495821146008296E-11, 1.0887754406636318E-11,
     7      -1.2779396094493696E-12,-2.3698241773087452E-13,
     8       3.1043578879654624E-14, 5.2617586299125060E-15,
     9      -7.5384795499492655E-16,-1.1862322577752285E-16,
     1       1.8316979965491384E-17/
      DATA Z3/1.2020569031595943D+00/
C
      IF(X .LT. 0D0  .OR.  X .GT. 1D0) THEN
         WRITE(*,*)' **************************************'
         WRITE(*,*)' Li3 called with X = ',X
         WRITE(*,*)' This should lie between 0 and 1 !!!'
         STOP' The program stops right here !!!'
      ENDIF
      IF (X.LT.0.5D0) THEN
         IF (X.GT.0.0D0) THEN
            U=-DLOG(1.0D0-X)
            HELP=U*L(20)+L(19)
            DO 10 I=18,0,-1
               HELP=U*HELP+L(I)
   10       CONTINUE
            TRILOG=U*HELP
         ELSE
            TRILOG=0.0D0
         ENDIF
      ELSE
         IF (X.LT.1.0D0) THEN
            U=-DLOG(X)
            Z=U*U
            HELP=Z*S(10)+S(9)
            DO 20 I=8,0,-1
               HELP=Z*HELP+S(I)
   20       CONTINUE
            HELP=1.0D0/2.0D0*(Z*HELP-Z*U/6.0D0)
C           LI3=-HELP+DLOG(X)*DILOG(X)+0.5D0*DLOG(1D0-X)*DLOG(X)**2+Z3
        TRILOG=-HELP+DLOG(X)*DDILOG(X)+0.5D0*DLOG(1D0-X)*DLOG(X)**2+Z3
         ELSE
          TRILOG=Z3
         ENDIF
      ENDIF
      END
 
      DOUBLE PRECISION FUNCTION S12(X)
*     ====== ========= ======== ======
C Tsuyoshi Matsuura 1987
C
C     S1,2   between 0 and 1  !!!
C
      REAL*8 X,S(0:10),L(0:20),U,Z,HELP,Z3
      REAL*8 DDILOG
C
C     S: COEFFICIENTS OF S1,2
C     L: COEFFICIENTS OF Li3
C
      DATA S/0.5000000000000000D+00,2.0833333333333333D-02,
     1      -2.3148148148148148D-04,4.1335978835978837D-06,
     2      -8.2671957671957673D-08,1.7397297489890083D-09,
     3      -3.7744215276339238D-11,8.3640853316779243D-13,
     4      -1.8831557201792127D-14,4.2930310281389223D-16,
     5      -9.8857668116275541D-18/
      DATA L/1.0000000000000000D+00,-0.3750000000000000D+00,
     1       7.8703703703703705E-02,-8.6805555555555557E-03,
     2       1.2962962962962963E-04, 8.1018518518518520E-05,
     3      -3.4193571608537598E-06,-1.3286564625850340E-06,
     4       8.6608717561098512E-08, 2.5260875955320400E-08,
     5      -2.1446944683640648E-09,-5.1401106220129790E-10,
     6       5.2495821146008296E-11, 1.0887754406636318E-11,
     7      -1.2779396094493696E-12,-2.3698241773087452E-13,
     8       3.1043578879654624E-14, 5.2617586299125060E-15,
     9      -7.5384795499492655E-16,-1.1862322577752285E-16,
     1       1.8316979965491384E-17/
      DATA Z3/1.2020569031595943D+00/
C
      IF(X .LT. 0D0  .OR.  X .GT. 1D0) THEN
         WRITE(*,*)' **************************************'
         WRITE(*,*)' S12 called with X = ',X
         WRITE(*,*)' This should lie between 0 and 1 !!!'
         STOP' The program stops right here !!!'
      ENDIF
      IF (X.LT.0.5D0) THEN
         IF (X.GT.0.0D0) THEN
            U=-DLOG(1.0D0-X)
            Z=U*U
            HELP=Z*S(10)+S(9)
            DO 10 I=8,0,-1
               HELP=HELP*Z+S(I)
   10       CONTINUE
            S12=1.0D0/2.0D0*(Z*HELP-Z*U/6.0D0)
         ELSE
            S12=0.0D0
         ENDIF
      ELSE
         IF (X.LT.1.0D0) THEN
            U=-DLOG(X)
            HELP=U*L(20)+L(19)
            DO 20 I=18,0,-1
               HELP=HELP*U+L(I)
   20       CONTINUE
            HELP=U*HELP
            S12=-HELP+DLOG(1.0D0-X)*DDILOG(1.0D0-X)+
     +           0.5D0*DLOG(X)*DLOG(1.0D0-X)**2+Z3
         ELSE
            S12=Z3
         ENDIF
      ENDIF
      END
 
      SUBROUTINE SIMPS(A1,B1,H1,REPS1,AEPS1,FUNCT,X,AI,AIH,AIABS)
C SIMPS
C A1,B1 -THE LIMITS OF INTEGRATION
C H1 -AN INITIAL STEP OF INTEGRATION
C REPS1,AEPS1 - RELATIVE AND ABSOLUTE PRECISION OF INTEGRATION
C FUNCT -A NAME OF FUNCTION SUBPROGRAM FOR CALCULATION OF INTEGRAND +
C X - AN ARGUMENT OF THE INTEGRAND
C AI - THE VALUE OF INTEGRAL
C AIH- THE VALUE OF INTEGRAL WITH THE STEP OF INTEGRATION
C AIABS- THE VALUE OF INTEGRAL FOR MODULE OF THE INTEGRAND
C THIS SUBROGRAM CALCULATES THE DEFINITE INTEGRAL WITH THE RELATIVE OR
C ABSOLUTE PRECISION BY SIMPSON+S METHOD WITH THE AUTOMATICAL CHOICE
C OF THE STEP OF INTEGRATION
C IF AEPS1    IS VERY SMALL(LIKE 1.E-17),THEN CALCULATION OF INTEGRAL
C WITH REPS1,AND IF REPS1 IS VERY SMALL (LIKE 1.E-10),THEN CALCULATION
C OF INTEGRAL WITH AEPS1
C WHEN AEPS1=REPS1=0. THEN CALCULATION WITH THE CONSTANT STEP H1
C
      IMPLICIT REAL*8(A-H,O-Z)
      DIMENSION F(7),P(5)
      EXTERNAL FUNCT
*
      H=DSIGN(H1,B1-A1)
      S=DSIGN(1.D0,H)
      A=A1
      B=B1
      AI=0.D0
      AIH=0.D0
      AIABS=0.D0
      P(2)=4.D0
      P(4)=4.D0
      P(3)=2.D0
      P(5)=1.D0
      IF(B-A) 1,2,1
    1 REPS=DABS(REPS1)
      AEPS=DABS(AEPS1)
      DO 3 K=1,7
  3   F(K)=10.D16
      X=A
      C=0.D0
      F(1)=FUNCT(X)/3.
    4 X0=X
      IF((X0+4.*H-B)*S) 5,5,6
    6 H=(B-X0)/4.
      IF(H) 7,2,7
    7 DO 8 K=2,7
  8   F(K)=10.D16
      C=1.D0
    5 DI2=F(1)
      DI3=DABS(F(1))
      DO 9 K=2,5
      X=X+H
      IF((X-B)*S) 23,24,24
   24 X=B
   23 IF(F(K)-10.D16) 10,11,10
   11 F(K)=FUNCT(X)/3.
   10 DI2=DI2+P(K)*F(K)
    9 DI3=DI3+P(K)*ABS(F(K))
      DI1=(F(1)+4.*F(3)+F(5))*2.*H
      DI2=DI2*H
      DI3=DI3*H
      IF(REPS) 12,13,12
   13 IF(AEPS) 12,14,12
   12 EPS=DABS((AIABS+DI3)*REPS)
      IF(EPS-AEPS) 15,16,16
   15 EPS=AEPS
   16 DELTA=DABS(DI2-DI1)
      IF(DELTA-EPS) 20,21,21
   20 IF(DELTA-EPS/8.) 17,14,14
   17 H=2.*H
      F(1)=F(5)
      F(2)=F(6)
      F(3)=F(7)
      DO 19 K=4,7
  19  F(K)=10.D16
      GO TO 18
   14 F(1)=F(5)
      F(3)=F(6)
      F(5)=F(7)
      F(2)=10.D16
      F(4)=10.D16
      F(6)=10.D16
      F(7)=10.D16
   18 DI1=DI2+(DI2-DI1)/15.
      AI=AI+DI1
      AIH=AIH+DI2
      AIABS=AIABS+DI3
      GO TO 22
   21 H=H/2.
      F(7)=F(5)
      F(6)=F(4)
      F(5)=F(3)
      F(3)=F(2)
      F(2)=10.D16
      F(4)=10.D16
      X=X0
      C=0.D0
      GO TO 5
   22 IF(C) 2,4,2
    2 RETURN
      END
 
      SUBROUTINE SIMPT(A1,B1,H1,REPS1,AEPS1,FUNCT,X,AI,AIH,AIABS)
C SIMPT
C A1,B1 -THE LIMITS OF INTEGRATION
C H1 -AN INITIAL STEP OF INTEGRATION
C REPS1,AEPS1 - RELATIVE AND ABSOLUTE PRECISION OF INTEGRATION
C FUNCT -A NAME OF FUNCTION SUBPROGRAM FOR CALCULATION OF INTEGRAND +
C X - AN ARGUMENT OF THE INTEGRAND
C AI - THE VALUE OF INTEGRAL
C AIH- THE VALUE OF INTEGRAL WITH THE STEP OF INTEGRATION
C AIABS- THE VALUE OF INTEGRAL FOR MODULE OF THE INTEGRAND
C THIS SUBROGRAM CALCULATES THE DEFINITE INTEGRAL WITH THE RELATIVE OR
C ABSOLUTE PRECISION BY SIMPSON+S METHOD WITH THE AUTOMATICAL CHOICE
C OF THE STEP OF INTEGRATION
C IF AEPS1    IS VERY SMALL(LIKE 1.E-17),THEN CALCULATION OF INTEGRAL
C WITH REPS1,AND IF REPS1 IS VERY SMALL (LIKE 1.E-10),THEN CALCULATION
C OF INTEGRAL WITH AEPS1
C WHEN AEPS1=REPS1=0. THEN CALCULATION WITH THE CONSTANT STEP H1
C
      IMPLICIT REAL*8(A-H,O-Z)
      DIMENSION F(7),P(5)
      EXTERNAl FUNCT
*
      H=DSIGN(H1,B1-A1)
      S=DSIGN(1.D0,H)
      A=A1
      B=B1
      AI=0.D0
      AIH=0.D0
      AIABS=0.D0
      P(2)=4.D0
      P(4)=4.D0
      P(3)=2.D0
      P(5)=1.D0
      IF(B-A) 1,2,1
    1 REPS=DABS(REPS1)
      AEPS=DABS(AEPS1)
      DO 3 K=1,7
  3   F(K)=10.D16
      X=A
      C=0.D0
      F(1)=FUNCT(X)/3.
    4 X0=X
      IF((X0+4.*H-B)*S) 5,5,6
    6 H=(B-X0)/4.
      IF(H) 7,2,7
    7 DO 8 K=2,7
  8   F(K)=10.D16
      C=1.D0
    5 DI2=F(1)
      DI3=DABS(F(1))
      DO 9 K=2,5
      X=X+H
      IF((X-B)*S) 23,24,24
   24 X=B
   23 IF(F(K)-10.D16) 10,11,10
   11 F(K)=FUNCT(X)/3.
   10 DI2=DI2+P(K)*F(K)
    9 DI3=DI3+P(K)*ABS(F(K))
      DI1=(F(1)+4.*F(3)+F(5))*2.*H
      DI2=DI2*H
      DI3=DI3*H
      IF(REPS) 12,13,12
   13 IF(AEPS) 12,14,12
   12 EPS=DABS((AIABS+DI3)*REPS)
      IF(EPS-AEPS) 15,16,16
   15 EPS=AEPS
   16 DELTA=DABS(DI2-DI1)
      IF(DELTA-EPS) 20,21,21
   20 IF(DELTA-EPS/8.) 17,14,14
   17 H=2.*H
      F(1)=F(5)
      F(2)=F(6)
      F(3)=F(7)
      DO 19 K=4,7
  19  F(K)=10.D16
      GO TO 18
   14 F(1)=F(5)
      F(3)=F(6)
      F(5)=F(7)
      F(2)=10.D16
      F(4)=10.D16
      F(6)=10.D16
      F(7)=10.D16
   18 DI1=DI2+(DI2-DI1)/15.
      AI=AI+DI1
      AIH=AIH+DI2
      AIABS=AIABS+DI3
      GO TO 22
   21 H=H/2.
      F(7)=F(5)
      F(6)=F(4)
      F(5)=F(3)
      F(3)=F(2)
      F(2)=10.D16
      F(4)=10.D16
      X=X0
      C=0.D0
      GO TO 5
   22 IF(C) 2,4,2
    2 RETURN
      END
 
      SUBROUTINE SIMPU(A1,B1,H1,REPS1,AEPS1,FUNCT,X,AI,AIH,AIABS)
C SIMPU
C A1,B1 -THE LIMITS OF INTEGRATION
C H1 -AN INITIAL STEP OF INTEGRATION
C REPS1,AEPS1 - RELATIVE AND ABSOLUTE PRECISION OF INTEGRATION
C FUNCT -A NAME OF FUNCTION SUBPROGRAM FOR CALCULATION OF INTEGRAND +
C X - AN ARGUMENT OF THE INTEGRAND
C AI - THE VALUE OF INTEGRAL
C AIH- THE VALUE OF INTEGRAL WITH THE STEP OF INTEGRATION
C AIABS- THE VALUE OF INTEGRAL FOR MODULE OF THE INTEGRAND
C THIS SUBROGRAM CALCULATES THE DEFINITE INTEGRAL WITH THE RELATIVE OR
C ABSOLUTE PRECISION BY SIMPSON+S METHOD WITH THE AUTOMATICAL CHOICE
C OF THE STEP OF INTEGRATION
C IF AEPS1    IS VERY SMALL(LIKE 1.E-17),THEN CALCULATION OF INTEGRAL
C WITH REPS1,AND IF REPS1 IS VERY SMALL (LIKE 1.E-10),THEN CALCULATION
C OF INTEGRAL WITH AEPS1
C WHEN AEPS1=REPS1=0. THEN CALCULATION WITH THE CONSTANT STEP H1
C
      IMPLICIT REAL*8(A-H,O-Z)
      DIMENSION F(7),P(5)
      EXTERNAl FUNCT
*
      H=DSIGN(H1,B1-A1)
      S=DSIGN(1.D0,H)
      A=A1
      B=B1
      AI=0.D0
      AIH=0.D0
      AIABS=0.D0
      P(2)=4.D0
      P(4)=4.D0
      P(3)=2.D0
      P(5)=1.D0
      IF(B-A) 1,2,1
    1 REPS=DABS(REPS1)
      AEPS=DABS(AEPS1)
      DO 3 K=1,7
  3   F(K)=10.D16
      X=A
      C=0.D0
      F(1)=FUNCT(X)/3.
    4 X0=X
      IF((X0+4.*H-B)*S) 5,5,6
    6 H=(B-X0)/4.
      IF(H) 7,2,7
    7 DO 8 K=2,7
  8   F(K)=10.D16
      C=1.D0
    5 DI2=F(1)
      DI3=DABS(F(1))
      DO 9 K=2,5
      X=X+H
      IF((X-B)*S) 23,24,24
   24 X=B
   23 IF(F(K)-10.D16) 10,11,10
   11 F(K)=FUNCT(X)/3.
   10 DI2=DI2+P(K)*F(K)
    9 DI3=DI3+P(K)*ABS(F(K))
      DI1=(F(1)+4.*F(3)+F(5))*2.*H
      DI2=DI2*H
      DI3=DI3*H
      IF(REPS) 12,13,12
   13 IF(AEPS) 12,14,12
   12 EPS=DABS((AIABS+DI3)*REPS)
      IF(EPS-AEPS) 15,16,16
   15 EPS=AEPS
   16 DELTA=DABS(DI2-DI1)
      IF(DELTA-EPS) 20,21,21
   20 IF(DELTA-EPS/8.) 17,14,14
   17 H=2.*H
      F(1)=F(5)
      F(2)=F(6)
      F(3)=F(7)
      DO 19 K=4,7
  19  F(K)=10.D16
      GO TO 18
   14 F(1)=F(5)
      F(3)=F(6)
      F(5)=F(7)
      F(2)=10.D16
      F(4)=10.D16
      F(6)=10.D16
      F(7)=10.D16
   18 DI1=DI2+(DI2-DI1)/15.
      AI=AI+DI1
      AIH=AIH+DI2
      AIABS=AIABS+DI3
      GO TO 22
   21 H=H/2.
      F(7)=F(5)
      F(6)=F(4)
      F(5)=F(3)
      F(3)=F(2)
      F(2)=10.D16
      F(4)=10.D16
      X=X0
      C=0.D0
      GO TO 5
   22 IF(C) 2,4,2
    2 RETURN
      END

      SUBROUTINE SIMPV(A1,B1,H1,REPS1,AEPS1,FUNCT,X,AI,AIH,AIABS)
C SIMPV
C A1,B1 -THE LIMITS OF INTEGRATION
C H1 -AN INITIAL STEP OF INTEGRATION
C REPS1,AEPS1 - RELATIVE AND ABSOLUTE PRECISION OF INTEGRATION
C FUNCT -A NAME OF FUNCTION SUBPROGRAM FOR CALCULATION OF INTEGRAND +
C X - AN ARGUMENT OF THE INTEGRAND
C AI - THE VALUE OF INTEGRAL
C AIH- THE VALUE OF INTEGRAL WITH THE STEP OF INTEGRATION
C AIABS- THE VALUE OF INTEGRAL FOR MODULE OF THE INTEGRAND
C THIS SUBROGRAM CALCULATES THE DEFINITE INTEGRAL WITH THE RELATIVE OR
C ABSOLUTE PRECISION BY SIMPSON+S METHOD WITH THE AUTOMATICAL CHOICE
C OF THE STEP OF INTEGRATION
C IF AEPS1    IS VERY SMALL(LIKE 1.E-17),THEN CALCULATION OF INTEGRAL
C WITH REPS1,AND IF REPS1 IS VERY SMALL (LIKE 1.E-10),THEN CALCULATION
C OF INTEGRAL WITH AEPS1
C WHEN AEPS1=REPS1=0. THEN CALCULATION WITH THE CONSTANT STEP H1
C
      IMPLICIT REAL*8(A-H,O-Z)
      DIMENSION F(7),P(5)
      EXTERNAl FUNCT
*
      H=DSIGN(H1,B1-A1)
      S=DSIGN(1.D0,H)
      A=A1
      B=B1
      AI=0.D0
      AIH=0.D0
      AIABS=0.D0
      P(2)=4.D0
      P(4)=4.D0
      P(3)=2.D0
      P(5)=1.D0
      IF(B-A) 1,2,1
    1 REPS=DABS(REPS1)
      AEPS=DABS(AEPS1)
      DO 3 K=1,7
  3   F(K)=10.D16
      X=A
      C=0.D0
      F(1)=FUNCT(X)/3.
    4 X0=X
      IF((X0+4.*H-B)*S) 5,5,6
    6 H=(B-X0)/4.
      IF(H) 7,2,7
    7 DO 8 K=2,7
  8   F(K)=10.D16
      C=1.D0
    5 DI2=F(1)
      DI3=DABS(F(1))
      DO 9 K=2,5
      X=X+H
      IF((X-B)*S) 23,24,24
   24 X=B
   23 IF(F(K)-10.D16) 10,11,10
   11 F(K)=FUNCT(X)/3.
   10 DI2=DI2+P(K)*F(K)
    9 DI3=DI3+P(K)*ABS(F(K))
      DI1=(F(1)+4.*F(3)+F(5))*2.*H
      DI2=DI2*H
      DI3=DI3*H
      IF(REPS) 12,13,12
   13 IF(AEPS) 12,14,12
   12 EPS=DABS((AIABS+DI3)*REPS)
      IF(EPS-AEPS) 15,16,16
   15 EPS=AEPS
   16 DELTA=DABS(DI2-DI1)
      IF(DELTA-EPS) 20,21,21
   20 IF(DELTA-EPS/8.) 17,14,14
   17 H=2.*H
      F(1)=F(5)
      F(2)=F(6)
      F(3)=F(7)
      DO 19 K=4,7
  19  F(K)=10.D16
      GO TO 18
   14 F(1)=F(5)
      F(3)=F(6)
      F(5)=F(7)
      F(2)=10.D16
      F(4)=10.D16
      F(6)=10.D16
      F(7)=10.D16
   18 DI1=DI2+(DI2-DI1)/15.
      AI=AI+DI1
      AIH=AIH+DI2
      AIABS=AIABS+DI3
      GO TO 22
   21 H=H/2.
      F(7)=F(5)
      F(6)=F(4)
      F(5)=F(3)
      F(3)=F(2)
      F(2)=10.D16
      F(4)=10.D16
      X=X0
      C=0.D0
      GO TO 5
   22 IF(C) 2,4,2
    2 RETURN
      END
 
      FUNCTION SPENCE(X)
*
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER (F1=1.64493406684822618D0)
*
      IF(X)8,1,1
1     IF(X-.5D0)2,2,3
2     SPENCE=FSPENS(X)
      RETURN
3     IF(X-1.D0)4,4,5
4     SPENCE=F1-LOG(X)*LOG(1D0-X+1D-15)-FSPENS(1D0-X)
      RETURN
5     IF(X-2.D0)6,6,7
6     SPENCE=F1-.5D0*LOG(X)*LOG((X-1D0)**2/X)+FSPENS(1D0-1D0/X)
      RETURN
7     SPENCE=2D0*F1-.5D0*LOG(X)**2-FSPENS(1D0/X)
      RETURN
8     IF(X+1.D0)10,9,9
9     SPENCE=-.5D0*LOG(1D0-X)**2-FSPENS(X/(X-1D0))
      RETURN
10    SPENCE=-.5D0*LOG(1D0-X)*LOG(X**2/(1D0-X))-F1+FSPENS(1D0/(1D0-X))
      END
 
      FUNCTION FSPENS(X)
*
      IMPLICIT REAL*8(A-H,O-Z)
*
      A=1D0
      F=0D0
      AN=0D0
      TCH=1D-16
1     AN=AN+1D0
      A=A*X
      B=A/AN**2
      F=F+B
      IF(B-TCH)2,2,1
2     FSPENS=F
      END
 
      FUNCTION TET(X)
*
      IMPLICIT REAL*8(A-H,O-Z)
*
      IF(X.LT.0D0) THEN
        TET=0D0
      ELSE
        TET=1D0
      ENDIF
*
      END
 
      FUNCTION XSPENZ(Z)
C================================================================
C
      IMPLICIT COMPLEX*16(X,Y)
      IMPLICIT REAL*8(A-H,O-W,Z)
      COMPLEX*16 Z,XCDIL,CSPENZ,LOG
      COMMON/CDZPIF/PI,F1
      EXTERNAL XCDIL
      DATA N/0/
C
      JPRINT=0
C
      N=N+1
      IF(N-1) 71,71,72
 71   PI=2.*(ACOS(0.D0)+ASIN(0.D0))
      F1=PI**2/6.
 72   CONTINUE
      REZ=DREAL(Z)
      AIZ=DIMAG(Z)
      AAZ=CDABS(Z)
      IF(AAZ) 11,9,11
 9    CSPENZ=DCMPLX(0.D0,0.D0)
      GOTO 99
 11   IF(AAZ-1.) 6,4,1
 1    RE1=DREAL(1./Z)
      IF(RE1-.5) 3,3,2
2     CONTINUE
      CSPENZ=XCDIL(1.-1./Z)-2.*F1-LOG(Z)*LOG(1.-1./Z)
     U      -.5*(LOG(-Z))**2
      GOTO 99
3     CONTINUE
      CSPENZ=-XCDIL(1./Z)-F1-.5*LOG(-Z)**2
      GOTO 99
 4    IF(REZ-1.) 6,5,1
 5    CSPENZ=DCMPLX(F1,0.D0)
      GOTO 99
 6    IF(REZ-.5) 7,7,8
7     CONTINUE
      CSPENZ=XCDIL(Z)
      GOTO 99
8     CONTINUE
      CSPENZ=-XCDIL(1.-Z)+F1-LOG(Z)*LOG(1.-Z)
 99   CONTINUE
      AAS= CDABS(CSPENZ)
      RES=DREAL(CSPENZ)
      AIS=DIMAG(CSPENZ)
      IF(JPRINT) 97,97,98
 98   CONTINUE
 97   CONTINUE
      XSPENZ=CSPENZ
      END
 
      FUNCTION XCDIL(Z)
C================================================================
      IMPLICIT COMPLEX*16(X,Y)
      IMPLICIT REAL*8(A-H,O-W,Z)
C
      COMPLEX*16 Z,Z1,CLZ,CLZP,CADD,LOG
      COMMON/CDZPIF/PI,F1
      DIMENSION ZETA(15)
      EXTERNAL FZETA
      DATA N/0/,TCH/1.D-16/
      SAVE ZETA
C
      JPRINT=0
C
      PI2=PI*PI
      PI4=PI2*PI2
      AAZ=CDABS(Z)
      REZ=DREAL(Z)
      IF(AAZ-1.) 4,2,3
 3    PRINT 1000
 1000 FORMAT(3X,6 (15HERROR MODULUS Z) )
      GOTO 881
 2    IF(REZ-.5) 4,4,3
 4    CONTINUE
      N=N+1
      IF(N-1) 5,5,6
 5    DO 11 I=4,15
      ZETA(I)=0.D0
 11   CONTINUE
      ZETA(1)=F1
      ZETA(2)=PI4/90.
      ZETA(3)=PI4*PI2/945.
 6    CONTINUE
      Z1=DCMPLX(1.D0,0.D0)-Z
      CLZ=LOG(Z1)
      XCDIL=-CLZ-.25*(CLZ)**2
      M=0
      CLZP=CLZ/(2.*PI)
 88   M=M+1
      IF(M-15) 882,882,883
 883  PRINT 1001
 1001 FORMAT(2X,3 (24HERROR-YOU NEED MORE ZETA) )
      GOTO 881
 882  IF(ZETA(M)) 884,884,885
 884  ZETA(M)=FZETA(2*M)
 885  HZETA=ZETA(M)
      CLZP=CLZP*(CLZ/(2.*PI))**2
      CADD=(-1.)**M/(2.*M+1)*CLZP*HZETA
      XCDIL=XCDIL+4.*PI*CADD
      ACL=CDABS(CLZP)
      IF(ACL-TCH) 881,881,88
 881  CONTINUE
      IF(JPRINT) 626,626,625
 625  CONTINUE
      DO 10 I10=1,15
      PRINT 1002,I10,ZETA(I10)
 1002 FORMAT(2X,2HI=,I4,5X,9HZETA(2I)=,D16.8)
 10   CONTINUE
 626  CONTINUE
      RETURN
      END
 
      FUNCTION FZETA(K)
C================================================================
C
      IMPLICIT REAL*8(A-H,O-Z)
      IF(K.GT.1) GOTO 10
*USOUT PRINT 1000
1000  FORMAT(18H ERROR IN FZETA** )
      STOP
10    F=0.D0
      AN=0.D0
      TCH=1.D-16
 1    AN=AN+1
      B=1./AN**K
      F=F+B
      IF(B-TCH) 2,2,1
 2    FZETA=F
      END
**********************************************************************
* SERVICE & BURKHARD ET AL ROUTINES
* (SHOULD NOT BE REPEATED WHEN IN ZFITTER)
**********************************************************************
      SUBROUTINE PIGINI
*-INITIALIZATION ROUTINE FOR BURKHARD ET AL. VACUUM POLARIZATION
      IMPLICIT REAL*8(A-Z)
      COMMON /BHANUM/ PI,F1,AL2,ZET3
      COMMON /BHAPHY/ ALFAI,AL1PI,ALQE2,GFERMI,CSIGNB
      COMMON /BHAPAR /AMZ,GAMZ,GAMEE,SW2,GV,GA
      COMMON /BHAELE/ AME,AME2,QE,QEM,QE2,COMB1,COMB2,COMB3
      COMMON /LEPT/ME,MMU,MTAU/HAD/MU,MD,MS,MC,MB,MT
      COMMON /ALF/AL,ALPHA,ALFA
      COMMON /BOS/MZ,MW,MH
C NUMERICAL CONSTANTS
      ALFA=1D0/ALFAI
      AL=AL1PI
      ALPHA=AL/4.
 
C MASS OF THE Z0
      MZ=AMZ
C
C SIN**2 OF THE ELECTROWEAK MIXING ANGLE
      SW=SW2
      SW=0.227018D0
C
C MASS OF THE W BOSON
      CW=1.-SW
      MW=MZ*DSQRT(CW)
      MW=80.1561D0
C
C MASS OF THE HIGGS BOSON
      MH=100D0
C
C MASSES OF THE FERMIONS: LEPTONS...
      ME=AME
      MMU=.106D0
      MTAU=1.785D0
C
C ...AND QUARKS (THE TOP QUARK MASS IS STILL A FREE PARAMETER)
      MU=.032D0
      MD=.0321D0
      MS=.15D0
      MC=1.5D0
      MB=4.5D0
      MT=130D0
C
      END
 
      FUNCTION XPIG(S)
CHBU  routine for BABAMC/MUONMC
CHBU  modified to use the hadron vacuum polarization as described in :
CHBU  H.Burkhardt et al. Pol. at Lep CERN 88-06 VOL I
CHBU  the old result, thats known to be inadequate for small q**2
CHBU  like in forward Bhabha scattering, can still be obtained
CHBU  by putting DISPFL to false
CHBU  Uses the complex function FC(s,m1,m2) for leptons,top
CHBU  to get the threshold behaviour consistent in the imaginary part
CHBU  (and to write the routine more compact)
CHBU  timing : old pig  .15 msec /call
CHBU           new pig  .27 msec /call    DISPFL FALSE
CHBU           new pig  .15 msec /call    DISPFL TRUE
CHBU                                   H.Burkhardt, June 1989
      IMPLICIT REAL*8(A-Z)
      COMPLEX*16 P,XPIG,PIL,PIH,HADRQQ,FC
      COMMON /LEPT/ME,MMU,MTAU/HAD/MU,MD,MS,MC,MB,MT
      COMMON /ALF/AL,ALPHA,ALFA
      COMMON /BOS/MZ,MW,MH
      LOGICAL DISPFL
      DATA DISPFL/.TRUE./
C     statement function P(s,m):
      P(S,XM)=1.D0/3.D0-(1.D0+2.D0*XM**2/S)*FC(S,XM,XM)
C  square of quark charges including colour faktor 3
      QU=4.D0/3.D0
      QD=1.D0/3.D0
      X=DABS(S)
      W=DSQRT(X)
C     lepton contribution
      PIL=AL/3.D0*(P(S,ME)+P(S,MMU)+P(S,MTAU))
C     hadron contribution
      IF(DISPFL) THEN
C       use dispersion relation result for udscb
        PIH=HADRQQ(S)
     .     + QU*AL/3.D0*P(S,MT)
      ELSE
C       use free field result with udscbt masses
        PIH=AL/3.D0*(QU*(P(S,MU)+P(S,MC)+P(S,MT))
     .              +QD*(P(S,MD)+P(S,MS)+P(S,MB)))
      ENDIF
C     gauge dependent W loop contribution
      PIW=ALPHA*((3.D0+4.D0*MW*MW/S)*FC(S,MW,MW)-2.D0/3.D0)
      XPIG=PIL+PIH+PIW
      END
C--------+---------+---------+---------+---------+---------+---------+--
      FUNCTION HADRQQ(S)
C  HADRONIC IRREDUCIBLE QQ SELF-ENERGY: TRANSVERSE
C     parametrize the real part of the photon self energy function
C     by  a + b ln(1+C*|S|) , as in my 1981 TASSO note but using
C     updated values, extended using RQCD up to 100 TeV
C     for details see:
C     H.Burkhardt, F.Jegerlehner, G.Penso and C.Verzegnassi
C     in CERN Yellow Report on "Polarization at LEP" 1988
C               H.BURKHARDT, CERN/ALEPH, AUGUST 1988
C
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 HADRQQ
C
      DATA A1,B1,C1/   0.0   ,   0.00835,  1.0   /
      DATA A2,B2,C2/   0.0   ,   0.00238,  3.927 /
      DATA A3,B3,C3/ 0.00165 ,   0.00300,  1.0   /
      DATA A4,B4,C4/ 0.00221 ,   0.00293,  1.0   /
C
      DATA PI/3.141592653589793/,ALFAIN/137.0359895D0/,INIT/0/
C
      IF(INIT.EQ.0) THEN
        INIT=1
        ALFA=1./ALFAIN
      ENDIF
      T=ABS(S)
      IF(T.LT.0.3**2) THEN
        REPIAA=A1+B1*LOG(1.+C1*T)
      ELSEIF(T.LT.3.**2) THEN
        REPIAA=A2+B2*LOG(1.+C2*T)
      ELSEIF(T.LT.100.**2) THEN
        REPIAA=A3+B3*LOG(1.+C3*T)
      ELSE
        REPIAA=A4+B4*LOG(1.+C4*T)
      ENDIF
C     as imaginary part take -i alfa/3 Rexpbh
      HADRQQ=REPIAA-(0.,1.)*ALFA/3.*REXPBH(S)
CEXPO HADRQQ=HADRQQ/(4.D0*PI*ALFA)  ! EXPOstar divides by 4 pi alfa
      END
C--------+---------+---------+---------+---------+---------+---------+--
      FUNCTION REXPBH(S)
C  HADRONIC IRREDUCIBLE QQ SELF-ENERGY: IMAGINARY
      IMPLICIT REAL*8(A-H,O-Z)
C     continuum R = Ai+Bi W ,  this + resonances was used to calculate
C     the dispersion integral. Used in the imag part of HADRQQ
      PARAMETER (NDIM=18)
      DIMENSION WW(NDIM),RR(NDIM),AA(NDIM),BB(NDIM)
      DATA WW/1.,1.5,2.0,2.3,3.73,4.0,4.5,5.0,7.0,8.0,9.,10.55,
     .  12.,50.,100.,1000.,10 000.,100 000./
      DATA RR/0.,2.3,1.5,2.7,2.7,3.6,3.6,4.0,4.0,3.66,3.66,3.66,
     .   4.,3.87,3.84, 3.79, 3.76,    3.75/
      DATA INIT/0/
      IF(INIT.EQ.0) THEN
        INIT=1
C       calculate A,B from straight lines between R measurements
        BB(NDIM)=0.
        DO 4 I=1,NDIM
          IF(I.LT.NDIM) BB(I)=(RR(I)-RR(I+1))/(WW(I)-WW(I+1))
          AA(I)=RR(I)-BB(I)*WW(I)
    4   CONTINUE
      ENDIF
      REXPBH=0.D0
      IF(S.GT.0.D0) THEN
        W=SQRT(S)
        IF(W.GT.WW(1)) THEN
          DO 2 I=1,NDIM
C           find out between which points of the RR array W is
            K=I
            IF(I.LT.NDIM) THEN
              IF(W.LT.WW(I+1)) GOTO 3
            ENDIF
    2     CONTINUE
    3     CONTINUE
          REXPBH=AA(K)+BB(K)*W
C         WRITE(6,'('' K='',I2,'' AA='',F10.2,'' BB='',F10.3)')
C    .    K,AA(K),BB(K)
        ENDIF
      ENDIF
      END
C--------+---------+---------+---------+---------+---------+---------+--
      FUNCTION FC(S,A,B)
C     complex function F(S,m1,m2)          H.Burkhardt 13-6-89
      IMPLICIT REAL*8(A-Z)
      COMPLEX*16 R,T,FC,F2,DIF
      Q=(A+B)**2
      P=(A-B)**2
      F1=(A-B)*(A**2-B**2)/S - (A**2+B**2)/(A+B)
      IF(1.D6*ABS(A-B).LT.A+B) THEN ! masses about equal
        F1=1.D0-F1/A
      ELSE
        F1=1.D0+F1*LOG(B/A)/(A-B)
      ENDIF
CSLOW R=SQRT(DCMPLX(S-Q))
CSLOW T=SQRT(DCMPLX(S-P))
C     to be faster use real arithmetic in this step
      IF(S-Q.GT.0.D0) THEN
        R=SQRT(S-Q)
      ELSE
        R=(0.D0,1.D0)*SQRT(Q-S)
      ENDIF
      IF(S-P.GT.0.D0) THEN
        T=SQRT(S-P)
      ELSE
        T=(0.D0,1.D0)*SQRT(P-S)
      ENDIF
C
      EPS=2.D0*A*B/(S-A**2-B**2)
      IF(ABS(EPS).LT.1.D-6) THEN
        DIF=-EPS*SQRT(DCMPLX(S-A**2-B**2))
      ELSE
        DIF=R-T
      ENDIF
      F2= R*T * LOG( DIF/(R+T) ) / S
      FC=F1+F2
      END
 
*      DOUBLE PRECISION FUNCTION F(Y,A,B)
*      IMPLICIT REAL*8(A-Z)
*      IF(A.LT.1.0D-05) GO TO 50
*      F1=2.D0
*      IF(A.EQ.B) GO TO 10
*      F1=1.D0+((A*A-B*B)/Y-(A*A+B*B)/(A*A-B*B))*DLOG(B/A)
*   10 CONTINUE
*      Q=(A+B)*(A+B)
*      P=(A-B)*(A-B)
*      IF(Y.LT.P) GO TO 20
*      IF(Y.GE.Q) GO TO 30
*      F2=DSQRT((Q-Y)*(Y-P))*(-2.D0)*DATAN(DSQRT((Y-P)/(Q-Y)))
*      GO TO 40
*   20 CONTINUE
*      F2=DSQRT((Q-Y)*(P-Y))*DLOG((DSQRT(Q-Y)+DSQRT(P-Y))**2/
*     &                                               (4.D0*A*B))
*      GO TO 40
*   30 CONTINUE
*      F2=DSQRT((Y-Q)*(Y-P))*(-1.D0)*DLOG((DSQRT(Y-P)+DSQRT(Y-Q))**2/
*     &(4.D0*A*B))
*   40 CONTINUE
*      F=F1+F2/Y
*      GO TO 70
*   50 CONTINUE
*      IF(Y.EQ.(B*B)) GO TO 65
*      IF(Y.LT.(B*B)) GO TO 60
*      F=1.D0+(1.D0-B*B/Y)*DLOG(B*B/(Y-B*B))
*      GO TO 70
*   60 CONTINUE
*      F=1.D0+(1.D0-B*B/Y)*DLOG(B*B/(B*B-Y))
*      GO TO 70
*   65 CONTINUE
*      F=1.D0
*   70 CONTINUE
*      RETURN
*      END
 
       SUBROUTINE INTERB(NPOINT,XMIN,XMAX,Y,XI,YI)
*=======================================================================
*     ROUTINE MAKES QUADRATIC INTERPOLATION FOR ARRAY "Y" OF "NPOINT"
*     EQUIDISTANT POINTS IN THE INTERVAL (XMIN,XMAX)
*-----------------------------------------------------------------------
       IMPLICIT REAL*8(A-H,O-Z)
*
       PARAMETER (NR=20)
       DIMENSION Y(NR)
*
       IF(NR.LT.NPOINT) STOP 'WRONG NUMBER OF POINTS'
       XSTEP=(XMAX-XMIN)/(NPOINT-1)
       N=INT((XI-XMIN)/XSTEP+0.5)+1
       IF(N.LT.2) THEN
         N=2
       ELSE IF(N.GT.NPOINT-1) THEN
         N=NPOINT-1
       END IF
*
       XCLOSE=XMIN+XSTEP*(N-1)
       W1=(XI-XCLOSE+XSTEP)
       W2=(XI-XCLOSE)
       W3=(XI-XCLOSE-XSTEP)
*
       YI=(0.5D0*W2*W3*Y(N-1)-W3*W1*Y(N)+0.5D0*W1*W2*Y(N+1))/XSTEP**2
       END
 
      SUBROUTINE FESIMP (AA1,BB1,HH1,REPS1,AEPS1,FUNCT,DFUN,DFUNIN,
     + DUMMY,AI,AIH,AIABS)
*=======================================================================
C        B1
C AI=INT {FUNCT(X)*D[DFUN(X)]}
C        A1
C A1,B1 -THE LIMITS OF INTEGRATION
C H1    -AN INITIAL STEP OF INTEGRATION
C REPS1,AEPS1 - RELATIVE AND ABSOLUTE PRECISION OF INTEGRATION
C FUNCT -A NAME OF FUNCTION SUBPROGRAM FOR CALCULATION OF INTEGRAND +
C X - AN ARGUMENT OF THE INTEGRAND
C DFUNIN - INVERSE ( DFUN ). SHOULD BE DFUNIN[DFUN(X)]=X.
C AI - THE VALUE OF INTEGRAL
C AIH- THE VALUE OF INTEGRAL WITH THE STEP OF INTEGRATION
C AIABS- THE VALUE OF INTEGRAL FOR MODULE OF THE INTEGRAND
C THIS SUBROGRAM CALCULATES THE DEFINITE INTEGRAL WITH THE RELATIVE OR
C ABSOLUTE PRECISION BY SIMPSON+S METHOD WITH THE AUTOMATICAL CHOICE
C OF THE STEP OF INTEGRATION
C IF AEPS1    IS VERY SMALL(LIKE 1.E-17),THEN CALCULATION OF INTEGRAL
C WITH REPS1,AND IF REPS1 IS VERY SMALL (LIKE 1.E-10),THEN CALCULATION
C OF INTEGRAL WITH AEPS1
C WHEN  AEPS1=REPS1=0. THEN CALCULATION WITH THE CONSTANT STEP H1
C
       IMPLICIT REAL*8(A-H,O-Z)
       DIMENSION F(7),P(5)
       EXTERNAL FUNCT,DFUN,DFUNIN
*
       DUM=DUMMY
       H1=(DFUN(BB1)-DFUN(AA1))/(BB1-AA1+1.654876596E-20)*HH1
       A1=DFUN(AA1)
       B1=DFUN(BB1)
       H=DSIGN(H1,B1-A1+1.654876596E-20)
       S=DSIGN(1.D0,H)
       A=A1
       B=B1
       AI=0.D0
       AIH=0.D0
       AIABS=0.D0
       P(2)=4.D0
       P(4)=4.D0
       P(3)=2.D0
       P(5)=1.D0
       IF(B-A) 1,2,1
    1  REPS=DABS(REPS1)
       AEPS=DABS(AEPS1)
       DO 3 K=1,7
    3  F(K)=10.D16
       X=A
       C=0.D0
       F(1)=FUNCT(DFUNIN(X))/3.
    4  X0=X
       IF((X0+4.*H-B)*S) 5,5,6
    6  H=(B-X0)/4.
       IF(H) 7,2,7
    7  DO 8 K=2,7
    8  F(K)=10.D16
       C=1.D0
    5  DI2=F(1)
       DI3=DABS(F(1))
       DO 9 K=2,5
         X=X+H
         IF((X-B)*S) 23,24,24
   24    X=B
   23    IF(F(K)-10.D16) 10,11,10
   11    F(K)=FUNCT(DFUNIN(X))/3.
   10    DI2=DI2+P(K)*F(K)
    9  DI3=DI3+P(K)*ABS(F(K))
       DI1=(F(1)+4.*F(3)+F(5))*2.*H
       DI2=DI2*H
       DI3=DI3*H
       IF(REPS) 12,13,12
   13  IF(AEPS) 12,14,12
   12  EPS=DABS((AIABS+DI3)*REPS)
       IF(EPS-AEPS) 15,16,16
   15  EPS=AEPS
   16  DELTA=DABS(DI2-DI1)
       IF(DELTA-EPS) 20,21,21
   20  IF(DELTA-EPS/8.) 17,14,14
   17  H=2.*H
       F(1)=F(5)
       F(2)=F(6)
       F(3)=F(7)
       DO 19 K=4,7
   19  F(K)=10.D16
       GO TO 18
   14  F(1)=F(5)
       F(3)=F(6)
       F(5)=F(7)
       F(2)=10.D16
       F(4)=10.D16
       F(6)=10.D16
       F(7)=10.D16
   18  DI1=DI2+(DI2-DI1)/15.
       AI=AI+DI1
       AIH=AIH+DI2
       AIABS=AIABS+DI3
       GO TO 22
   21  H=H/2.
       F(7)=F(5)
       F(6)=F(4)
       F(5)=F(3)
       F(3)=F(2)
       F(2)=10.D16
       F(4)=10.D16
       X=X0
       C=0.D0
       GO TO 5
   22  IF(C) 2,4,2
    2  RETURN
       END
 
      SUBROUTINE DZERO(A,B,X0,R,EPS,MXF,F)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
C     LOGICAL MFLAG,RFLAG
      EXTERNAL F
 
      PARAMETER (ONE = 1, HALF = ONE/2)
 
      XA=MIN(A,B)
      XB=MAX(A,B)
      FA=F(XA,1)
      FB=F(XB,2)
      IF(FA*FB .GT. 0) GO TO 5
      MC=0
 
    1 X0=HALF*(XA+XB)
      R=X0-XA
      EE=EPS*(ABS(X0)+1)
      IF(R .LE. EE) GO TO 4
      F1=FA
      X1=XA
      F2=FB
      X2=XB
 
    2 FX=F(X0,2)
      MC=MC+1
      IF(MC .GT. MXF) GO TO 6
      IF(FX*FA .GT. 0) THEN
       XA=X0
       FA=FX
      ELSE
       XB=X0
       FB=FX
      END IF
 
    3 U1=F1-F2
      U2=X1-X2
      U3=F2-FX
      U4=X2-X0
      IF(U2 .EQ. 0 .OR. U4 .EQ. 0) GO TO 1
      F3=FX
      X3=X0
      U1=U1/U2
      U2=U3/U4
      CA=U1-U2
      CB=(X1+X2)*U2-(X2+X0)*U1
      CC=(X1-X0)*F1-X1*(CA*X1+CB)
      IF(CA .EQ. 0) THEN
       IF(CB .EQ. 0) GO TO 1
       X0=-CC/CB
      ELSE
       U3=CB/(2*CA)
       U4=U3*U3-CC/CA
       IF(U4 .LT. 0) GO TO 1
       X0=-U3+SIGN(SQRT(U4),X0+U3)
      END IF
      IF(X0 .LT. XA .OR. X0 .GT. XB) GO TO 1
 
      R=MIN(ABS(X0-X3),ABS(X0-X2))
      EE=EPS*(ABS(X0)+1)
      IF(R .GT. EE) THEN
       F1=F2
       X1=X2
       F2=F3
       X2=X3
       GO TO 2
      END IF
 
      FX=F(X0,2)
      IF(FX .EQ. 0) GO TO 4
      IF(FX*FA .LT. 0) THEN
       XX=X0-EE
       IF(XX .LE. XA) GO TO 4
       FF=F(XX,2)
       FB=FF
       XB=XX
      ELSE
       XX=X0+EE
       IF(XX .GE. XB) GO TO 4
       FF=F(XX,2)
       FA=FF
       XA=XX
      END IF
      IF(FX*FF .GT. 0) THEN
       MC=MC+2
       IF(MC .GT. MXF) GO TO 6
       F1=F3
       X1=X3
       F2=FX
       X2=X0
       X0=XX
       FX=FF
       GO TO 3
      END IF
 
    4 R=EE
      FF=F(X0,3)
      RETURN
C+
C    5 CALL KERMTR('C205.1',LGFILE,MFLAG,RFLAG)
C      IF(MFLAG) THEN
C       IF(LGFILE .EQ. 0) WRITE(*,100)
C       IF(LGFILE .NE. 0) WRITE(LGFILE,100)
C      END IF
C      IF(.NOT.RFLAG) CALL ABEND
    5 PRINT *,'***** DZERO ... F(A) AND F(B) HAVE THE SAME SIGN'
C-
      R=-2*(XB-XA)
      X0=0
      RETURN
C+
C    6 CALL KERMTR('C205.2',LGFILE,MFLAG,RFLAG)
C      IF(MFLAG) THEN
C       IF(LGFILE .EQ. 0) WRITE(*,101)
C       IF(LGFILE .NE. 0) WRITE(LGFILE,101)
C      END IF
C      IF(.NOT.RFLAG) CALL ABEND
    6 PRINT *,'***** DZERO ... TOO MANY FUNCTION CALLS'
C-
      R=-HALF*ABS(XB-XA)
      X0=0
      RETURN
  100 FORMAT(1X,'***** CERN C205 DZERO ... F(A) AND F(B)',
     1          ' HAVE THE SAME SIGN')
  101 FORMAT(1X,'***** CERN C205 DZERO ... TOO MANY FUNCTION CALLS')
      END

      DOUBLE PRECISION FUNCTION DGAMMA(X)
C
C		Tue Feb 21 14:15:50 MET 1995
c from cernpams at cernvm, file: kernnum car
c changes by t.riemann for a HP workstation.
c another computer may be more precise and then use of the higher
c accuracy may be recommended. 8 oct 1992
      LOGICAL MFLAG,RFLAG
      REAL SX
      DOUBLE PRECISION X,U,F,ZERO,ONE,THREE,FOUR,PI
      DOUBLE PRECISION C(0:24),H,ALFA,B0,B1,B2
      DATA ZERO /0.0D0/, ONE /1.0D0/, THREE /3.0D0/, FOUR /4.0D0/
cHP  +SELF,IF=NUMHIPRE.
c     DATA NC /24/
c     DATA PI    /3.14159 26535 89793 23846 26433 83D0/
c     DATA C( 0) /3.65738 77250 83382 43849 88068 39D0/
c     DATA C( 1) /1.95754 34566 61268 26928 33742 26D0/
c     DATA C( 2) / .33829 71138 26160 38915 58510 73D0/
c     DATA C( 3) / .04208 95127 65575 49198 51083 97D0/
c     DATA C( 4) / .00428 76504 82129 08770 04289 08D0/
c     DATA C( 5) / .00036 52121 69294 61767 02198 22D0/
c     DATA C( 6) / .00002 74006 42226 42200 27170 66D0/
c     DATA C( 7) / .00000 18124 02333 65124 44603 05D0/
c     DATA C( 8) / .00000 01096 57758 65997 06993 06D0/
c     DATA C( 9) / .00000 00059 87184 04552 00046 95D0/
c     DATA C(10) / .00000 00003 07690 80535 24777 71D0/
c     DATA C(11) / .00000 00000 14317 93029 61915 76D0/
c     DATA C(12) / .00000 00000 00651 08773 34803 70D0/
c     DATA C(13) / .00000 00000 00025 95849 89822 28D0/
c     DATA C(14) / .00000 00000 00001 10789 38922 59D0/
c     DATA C(15) / .00000 00000 00000 03547 43620 17D0/
c     DATA C(16) / .00000 00000 00000 00168 86075 04D0/
c     DATA C(17) / .00000 00000 00000 00002 73543 58D0/
c     DATA C(18) / .00000 00000 00000 00000 30297 74D0/
c     DATA C(19) /-.00000 00000 00000 00000 00571 22D0/
c     DATA C(20) / .00000 00000 00000 00000 00090 77D0/
c     DATA C(21) /-.00000 00000 00000 00000 00005 05D0/
c     DATA C(22) / .00000 00000 00000 00000 00000 41D0/
c     DATA C(23) /-.00000 00000 00000 00000 00000 03D0/
c     DATA C(24) / .00000 00000 00000 00000 00000 01D0/
cHP +SELF,IF=NUMLOPRE.
      DATA NC /15/
      DATA PI    /3.14159 26535 89793 24D0/
      DATA C( 0) /3.65738 77250 83382 44D0/
      DATA C( 1) /1.95754 34566 61268 27D0/
      DATA C( 2) / .33829 71138 26160 39D0/
      DATA C( 3) / .04208 95127 65575 49D0/
      DATA C( 4) / .00428 76504 82129 09D0/
      DATA C( 5) / .00036 52121 69294 62D0/
      DATA C( 6) / .00002 74006 42226 42D0/
      DATA C( 7) / .00000 18124 02333 65D0/
      DATA C( 8) / .00000 01096 57758 66D0/
      DATA C( 9) / .00000 00059 87184 05D0/
      DATA C(10) / .00000 00003 07690 81D0/
      DATA C(11) / .00000 00000 14317 93D0/
      DATA C(12) / .00000 00000 00651 09D0/
      DATA C(13) / .00000 00000 00025 96D0/
      DATA C(14) / .00000 00000 00001 11D0/
      DATA C(15) / .00000 00000 00000 04D0/
cHP  +SELF.
      U=X
      IF(X .LE. ZERO) THEN
       IF(X .EQ. INT(X)) THEN
        CALL KERMTR('C305.1',LGFILE,MFLAG,RFLAG)
        IF(MFLAG) THEN
         SX=X
         IF(LGFILE .EQ. 0) THEN
          WRITE(*,100) SX
         ELSE
          WRITE(LGFILE,100) SX
         END IF
        END IF
        IF(.NOT.RFLAG) CALL ABEND
        DGAMMA=ZERO
        RETURN
       ELSE
        U=ONE-U
       END IF
      END IF
      F=ONE
      IF(U .LT. THREE) THEN
       DO 1 I = 1,INT(FOUR-U)
       F=F/U
    1  U=U+ONE
      ELSE
       DO 2 I = 1,INT(U-THREE)
       U=U-ONE
    2  F=F*U
      END IF
      U=U-THREE
      H=U+U-ONE
      ALFA=H+H
      B1=ZERO
      B2=ZERO
      DO 3 I = NC,0,-1
      B0=C(I)+ALFA*B1-B2
      B2=B1
    3 B1=B0
      U=F*(B0-H*B2)
      IF(X .LT. ZERO) U=PI/(SIN(PI*X)*U)
      DGAMMA=U
      RETURN
  100 FORMAT(1X,'DGAMMA ... ARGUMENT IS NON-POSITIVE INTEGER = ',E15.1)
      END
C
       subroutine abend
       stop
       end

          SUBROUTINE KERSET(ERCODE,LGFILE,LIMITM,LIMITR)
C
C	Tue Feb 21 14:15:50 MET 1995
                    PARAMETER(KOUNTE  =  27)
          CHARACTER*6         ERCODE,   CODE(KOUNTE)
          LOGICAL             MFLAG,    RFLAG
          INTEGER             KNTM(KOUNTE),       KNTR(KOUNTE)
          DATA      LOGF      /  0  /
          DATA      CODE(1), KNTM(1), KNTR(1)  / 'C204.1', 255, 255 /
          DATA      CODE(2), KNTM(2), KNTR(2)  / 'C204.2', 255, 255 /
          DATA      CODE(3), KNTM(3), KNTR(3)  / 'C204.3', 255, 255 /
          DATA      CODE(4), KNTM(4), KNTR(4)  / 'C205.1', 255, 255 /
          DATA      CODE(5), KNTM(5), KNTR(5)  / 'C205.2', 255, 255 /
          DATA      CODE(6), KNTM(6), KNTR(6)  / 'C305.1', 255, 255 /
          DATA      CODE(7), KNTM(7), KNTR(7)  / 'C308.1', 255, 255 /
          DATA      CODE(8), KNTM(8), KNTR(8)  / 'C312.1', 255, 255 /
          DATA      CODE(9), KNTM(9), KNTR(9)  / 'C313.1', 255, 255 /
          DATA      CODE(10),KNTM(10),KNTR(10) / 'C336.1', 255, 255 /
          DATA      CODE(11),KNTM(11),KNTR(11) / 'C337.1', 255, 255 /
          DATA      CODE(12),KNTM(12),KNTR(12) / 'C341.1', 255, 255 /
          DATA      CODE(13),KNTM(13),KNTR(13) / 'D103.1', 255, 255 /
          DATA      CODE(14),KNTM(14),KNTR(14) / 'D106.1', 255, 255 /
          DATA      CODE(15),KNTM(15),KNTR(15) / 'D209.1', 255, 255 /
          DATA      CODE(16),KNTM(16),KNTR(16) / 'D509.1', 255, 255 /
          DATA      CODE(17),KNTM(17),KNTR(17) / 'E100.1', 255, 255 /
          DATA      CODE(18),KNTM(18),KNTR(18) / 'E104.1', 255, 255 /
          DATA      CODE(19),KNTM(19),KNTR(19) / 'E105.1', 255, 255 /
          DATA      CODE(20),KNTM(20),KNTR(20) / 'E208.1', 255, 255 /
          DATA      CODE(21),KNTM(21),KNTR(21) / 'E208.2', 255, 255 /
          DATA      CODE(22),KNTM(22),KNTR(22) / 'F010.1', 255,   0 /
          DATA      CODE(23),KNTM(23),KNTR(23) / 'F011.1', 255,   0 /
          DATA      CODE(24),KNTM(24),KNTR(24) / 'F012.1', 255,   0 /
          DATA      CODE(25),KNTM(25),KNTR(25) / 'F406.1', 255,   0 /
          DATA      CODE(26),KNTM(26),KNTR(26) / 'G100.1', 255, 255 /
          DATA      CODE(27),KNTM(27),KNTR(27) / 'G100.2', 255, 255 /
          LOGF  =  LGFILE
             L  =  0
          IF(ERCODE .NE. ' ')  THEN
             DO 10  L = 1, 6
                IF(ERCODE(1:L) .EQ. ERCODE)  GOTO 12
  10            CONTINUE
  12         CONTINUE
          ENDIF
          DO 14     I  =  1, KOUNTE
             IF(L .EQ. 0)  GOTO 13
             IF(CODE(I)(1:L) .NE. ERCODE(1:L))  GOTO 14
  13         IF(LIMITM.GE.0) KNTM(I)  =  LIMITM
             IF(LIMITR.GE.0) KNTR(I)  =  LIMITR
  14         CONTINUE
          RETURN
          ENTRY KERMTR(ERCODE,LOG,MFLAG,RFLAG)
          LOG  =  LOGF
          DO 20     I  =  1, KOUNTE
             IF(ERCODE .EQ. CODE(I))  GOTO 21
  20         CONTINUE
          WRITE(*,1000)  ERCODE
          CALL ABEND
          RETURN
  21      RFLAG  =  KNTR(I) .GE. 1
          IF(RFLAG  .AND.  (KNTR(I) .LT. 255))  KNTR(I)  =  KNTR(I) - 1
          MFLAG  =  KNTM(I) .GE. 1
          IF(MFLAG  .AND.  (KNTM(I) .LT. 255))  KNTM(I)  =  KNTM(I) - 1
          IF(.NOT. RFLAG)  THEN
             IF(LOGF .LT. 1)  THEN
                WRITE(*,1001)  CODE(I)
             ELSE
                WRITE(LOGF,1001)  CODE(I)
             ENDIF
          ENDIF
          IF(MFLAG .AND. RFLAG)  THEN
             IF(LOGF .LT. 1)  THEN
                WRITE(*,1002)  CODE(I)
             ELSE
                WRITE(LOGF,1002)  CODE(I)
             ENDIF
          ENDIF
          RETURN
1000      FORMAT(' KERNLIB LIBRARY ERROR. ' /
     +           ' ERROR CODE ',A6,' NOT RECOGNIZED BY KERMTR',
     +           ' ERROR MONITOR. RUN ABORTED.')
1001      FORMAT(/' ***** RUN TERMINATED BY CERN LIBRARY ERROR ',
     +           'CONDITION ',A6)
1002      FORMAT(/' ***** CERN LIBRARY ERROR CONDITION ',A6)
          END
