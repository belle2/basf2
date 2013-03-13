*//////////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                              //
*//                                                                                              //
*//                     Pseudo-CLASS  Mathlib                                                    //
*//                                                                                              //
*//      Purpose:  library of math utilies                                                       //
*//                                                                                              //
*//      SUBROUTINE Mathlib_GausJad(fun,aa,bb,eeps,result)     : Gauss integration               //
*//      DOUBLE PRECISION FUNCTION Mathlib_Gauss(f,a,b,eeps)   : Gauss integration               //
*//      DOUBLE PRECISION FUNCTION Mathlib_dilogy(x)           : Dilog function Li_2             //
*//      DOUBLE PRECISION FUNCTION Mathlib_dpgamm(z)           : Euler Gamma function            //
*//                                                                                              //
*//////////////////////////////////////////////////////////////////////////////////////////////////


      SUBROUTINE Mathlib_GausJad(fun,aa,bb,eeps,result)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//  Gauss-type integration by S. Jadach, Oct. 1990, June 1997               //
*//  this is non-adaptive (!!!!) unoptimized (!!!) integration subprogram.   //
*//                                                                          //
*//  Eeps>0 treated as ABSOLUTE requested error                              //
*//  Eeps<0 treated as RELATIVE requested error                              //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      DOUBLE PRECISION  fun,aa,bb,eeps,result
      EXTERNAL fun
*
      DOUBLE PRECISION  a,b,xplus,sum16,range,sum8,erabs,erela,fminu,xminu
      DOUBLE PRECISION  fplus,xmidle,calk8,eps,x1,x2,delta,calk16
      INTEGER           iter,ndivi,itermx,k,i
      DOUBLE PRECISION  wg(12),xx(12)
      DATA wg
     $/0.101228536290376d0, 0.222381034453374d0, 0.313706645877887d0,
     $ 0.362683783378362d0, 0.027152459411754d0, 0.062253523938648d0,
     $ 0.095158511682493d0, 0.124628971255534d0, 0.149595988816577d0,
     $ 0.169156519395003d0, 0.182603415044924d0, 0.189450610455069d0/
      DATA xx
     $/0.960289856497536d0, 0.796666477413627d0, 0.525532409916329d0,
     $ 0.183434642495650d0, 0.989400934991650d0, 0.944575023073233d0,
     $ 0.865631202387832d0, 0.755404408355003d0, 0.617876244402644d0,
     $ 0.458016777657227d0, 0.281603550779259d0, 0.095012509837637d0/
      DATA itermx / 15/
*-------------------------------------------------------------------------------
      a  = aa
      b  = bb
      eps= ABS(eeps)
      ndivi=1
*** iteration over subdivisions terminated by precision requirement
      DO iter=1,itermx
         calk8  =0d0
         calk16 =0d0
***   sum over delta subintegrals
         DO k = 1,ndivi
            delta = (b-a)/ndivi
            x1    =  a + (k-1)*delta
            x2    =  x1+ delta
            xmidle= 0.5d0*(x2+x1)
            range = 0.5d0*(x2-x1)
            sum8 =0d0
            sum16=0d0
***   8- and 12-point   gauss integration over single delta subinterval
            DO i=1,12
               xplus= xmidle+range*xx(i)
               xminu= xmidle-range*xx(i)
               fplus=fun(xplus)
               fminu=fun(xminu)
               IF(i .LE. 4) THEN
                  sum8 =sum8  +(fplus+fminu)*wg(i)/2d0
               ELSE
                  sum16=sum16 +(fplus+fminu)*wg(i)/2d0
               ENDIF
            ENDDO
            calk8 = calk8 + sum8 *(x2-x1)
            calk16= calk16+ sum16*(x2-x1)
         ENDDO
         erabs = ABS(calk16-calk8)
         erela = 0d0
         IF(calk16 .NE. 0d0) erela= erabs/ABS(calk16)
***   WRITE(*,*) 'gausjad: calk8,calk16=',iter,calk8,calk16,erela
***   precision check to terminate integration
         IF(eeps .GT. 0d0) THEN
            IF(erabs .LT.  eps) GOTO 800
         ELSE
            IF(erela .LT.  eps) GOTO 800
         ENDIF
         ndivi=ndivi*2
      ENDDO
      WRITE(*,*) '++++ Mathlib_GausJad: required precision to high!'
      WRITE(*,*) '++++ Mathlib_GausJad: eeps,erela,erabs=',eeps,erela,erabs
  800 CONTINUE
      result = calk16
      END


      DOUBLE PRECISION FUNCTION Mathlib_Gauss(f,a,b,eeps)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// this is iterative integration procedure                                  //
*// originates  probably from cern library                                   //
*// it subdivides inegration range until required PRECISION is reached       //
*// PRECISION is a difference from 8 and 16 point gauss itegr. result        //
*// eeps positive treated as absolute PRECISION                              //
*// eeps negative treated as relative PRECISION                              //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  f,a,b,eeps
*
      DOUBLE PRECISION  c1,c2,bb,s8,s16,y,aa,const,delta,eps,u
      INTEGER           i
*
      DOUBLE PRECISION  w(12),x(12)
      EXTERNAL f
      DATA const /1.0d-19/
      DATA w
     1/0.10122 85362 90376, 0.22238 10344 53374, 0.31370 66458 77887,
     2 0.36268 37833 78362, 0.02715 24594 11754, 0.06225 35239 38648,
     3 0.09515 85116 82493, 0.12462 89712 55534, 0.14959 59888 16577,
     4 0.16915 65193 95003, 0.18260 34150 44924, 0.18945 06104 55069/
      DATA x
     1/0.96028 98564 97536, 0.79666 64774 13627, 0.52553 24099 16329,
     2 0.18343 46424 95650, 0.98940 09349 91650, 0.94457 50230 73233,
     3 0.86563 12023 87832, 0.75540 44083 55003, 0.61787 62444 02644,
     4 0.45801 67776 57227, 0.28160 35507 79259, 0.09501 25098 37637/
*-----------------------------------------------------------------------------
      eps=abs(eeps)
      delta=const*abs(a-b)
      Mathlib_Gauss=0d0
      aa=a
    5 y=b-aa
      IF(abs(y)  .LE.  delta) RETURN
    2 bb=aa+y
      c1=0.5d0*(aa+bb)
      c2=c1-aa
      s8=0d0
      s16=0d0
      DO 1 i=1,4
      u=x(i)*c2
    1 s8=s8+w(i)*(f(c1+u)+f(c1-u))
      DO 3 i=5,12
      u=x(i)*c2
    3 s16=s16+w(i)*(f(c1+u)+f(c1-u))
      s8=s8*c2
      s16=s16*c2
      IF(eeps .LT. 0d0) THEN
        IF(abs(s16-s8)  .GT.  eps*abs(s16)) GOTO 4
      ELSE
        IF(abs(s16-s8)  .GT.  eps) GOTO 4
      ENDIF
      Mathlib_Gauss=Mathlib_Gauss+s16
      aa=bb
      GOTO 5
    4 y=0.5d0*y
      IF(abs(y)  .GT.  delta) GOTO 2
      WRITE(*,7)
      Mathlib_Gauss=0d0
      RETURN
    7 FORMAT(1x,36hgaus  ... too high accuracy required)
      END


      DOUBLE PRECISION FUNCTION Mathlib_dilogy(x)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*// dilogarithm FUNCTION: dilog(x)=int( -ln(1-z)/z ) , 0 < z < x .           //
*// this is the cernlib version.                                             //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION x
* locals
      DOUBLE PRECISION a,b,s,t,y,z
*------------------------------------------------------------------------------
      z=-1.644934066848226d0
      IF(x  .LT. -1.d0) go to 1
      IF(x  .LE.  0.5d0) go to 2
      IF(x  .EQ.  1.d0) go to 3
      IF(x  .LE.  2.d0) go to 4
      z=3.289868133696453d0
    1 t=1.d0/x
      s=-0.5d0
      z=z-0.5d0*dlog(dabs(x))**2
      go to 5
    2 t=x
      s=0.5d0
      z=0.d0
      go to 5
    3 Mathlib_Dilogy=1.644934066848226d0
      RETURN
    4 t=1.d0-x
      s=-0.5d0
      z=1.644934066848226d0-dlog(x)*dlog(dabs(t))
    5 y=2.666666666666667d0*t+0.666666666666667d0
      b=      0.000000000000001d0
      a=y*b  +0.000000000000004d0
      b=y*a-b+0.000000000000011d0
      a=y*b-a+0.000000000000037d0
      b=y*a-b+0.000000000000121d0
      a=y*b-a+0.000000000000398d0
      b=y*a-b+0.000000000001312d0
      a=y*b-a+0.000000000004342d0
      b=y*a-b+0.000000000014437d0
      a=y*b-a+0.000000000048274d0
      b=y*a-b+0.000000000162421d0
      a=y*b-a+0.000000000550291d0
      b=y*a-b+0.000000001879117d0
      a=y*b-a+0.000000006474338d0
      b=y*a-b+0.000000022536705d0
      a=y*b-a+0.000000079387055d0
      b=y*a-b+0.000000283575385d0
      a=y*b-a+0.000001029904264d0
      b=y*a-b+0.000003816329463d0
      a=y*b-a+0.000014496300557d0
      b=y*a-b+0.000056817822718d0
      a=y*b-a+0.000232002196094d0
      b=y*a-b+0.001001627496164d0
      a=y*b-a+0.004686361959447d0
      b=y*a-b+0.024879322924228d0
      a=y*b-a+0.166073032927855d0
      a=y*a-b+1.935064300869969d0
      Mathlib_Dilogy = s*t*(a-b)+z
      END


      DOUBLE PRECISION FUNCTION Mathlib_dpgamm(z)
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//            Double precision gamma function                               //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
      DOUBLE PRECISION z,z1,x,x1,x2,d1,d2,s1,s2,s3,pi,c(20),const
      SAVE c,pi,const
      DATA c( 1) / 8.3333333333333333333333333332d-02/
      DATA c( 2) /-2.7777777777777777777777777777d-03/
      DATA c( 3) / 7.9365079365079365079365079364d-04/
      DATA c( 4) /-5.9523809523809523809523809523d-04/
      DATA c( 5) / 8.4175084175084175084175084175d-04/
      DATA c( 6) /-1.9175269175269175269175269175d-03/
      DATA c( 7) / 6.4102564102564102564102564102d-03/
      DATA c( 8) /-2.9550653594771241830065359477d-02/
      DATA c( 9) / 1.7964437236883057316493849001d-01/
      DATA c(10) /-1.3924322169059011164274322169d+00/
      DATA c(11) / 1.3402864044168391994478951001d+01/
      DATA c(12) /-1.5684828462600201730636513245d+02/
      DATA c(13) / 2.1931033333333333333333333333d+03/
      DATA c(14) /-3.6108771253724989357173265219d+04/
      DATA c(15) / 6.9147226885131306710839525077d+05/
      DATA c(16) /-1.5238221539407416192283364959d+07/
      DATA c(17) / 3.8290075139141414141414141414d+08/
      DATA c(18) /-1.0882266035784391089015149165d+10/
      DATA c(19) / 3.4732028376500225225225225224d+11/
      DATA c(20) /-1.2369602142269274454251710349d+13/
      DATA pi    / 3.1415926535897932384626433832d+00/
      DATA const / 9.1893853320467274178032973641d-01/
      IF(z .GT. 5.75d 1)                                     GOTO  6666
      nn = z
      IF (z  -  dble(float(nn)))                 3,1,3
    1 IF (z     .LE.     0.d 0)                    GOTO 6667
      Mathlib_dpgamm = 1.d 0
      IF (z     .LE.     2.d 0)                    RETURN
      z1 = z
    2 z1 = z1  -  1.d 0
      Mathlib_dpgamm = Mathlib_dpgamm * z1
      IF (z1  -  2.d 0)                          61,61,2
    3 IF (dabs(z)     .LT.     1.d-29)             GOTO 60
      IF (z     .LT.     0.d 0)                    GOTO 4
      x  = z
      kk = 1
      GOTO 10
    4 x  = 1.d 0  -  z
      kk = 2
   10 x1 = x
      IF (x     .GT.     19.d 0)                   GOTO 13
      d1 = x
   11 x1 = x1  +  1.d 0
      IF (x1     .GE.     19.d 0)                  GOTO 12
      d1 = d1 * x1
      GOTO 11
   12 s3 = -dlog(d1)
      GOTO 14
   13 s3 = 0.d 0
   14 d1 = x1 * x1
      s1 = (x1  -  5.d-1) * dlog(x1)  -  x1  +  const
      DO 20                  k=1,20
      s2 = s1  +  c(k)/x1
      IF (dabs(s2  -  s1)     .LT.     1.d-28)     GOTO 21
      x1 = x1 * d1
   20 s1 = s2
   21 s3 = s3  +  s2
      GOTO (50,22),    kk
   22 d2 = dabs(z  -  nn)
      d1 = d2 * pi
      IF (d1     .LT.     1.d-15)                  GOTO 31
   30 x2 =  dlog(pi/dsin(d1))  -  s3
      GOTO 40
   31 x2 = -dlog(d2)
   40 mm = dabs(z)
      IF(x2       .GT.       1.74d2)                  GOTO 6666
      Mathlib_dpgamm = dexp(x2)
      IF (mm    .ne.    (mm/2) * 2)              RETURN
      Mathlib_dpgamm = -Mathlib_dpgamm
      RETURN
   50 IF(s3       .GT.       1.74d2)                  GOTO 6666
      Mathlib_dpgamm = dexp(s3)
      RETURN
 6666 print *, 2000
      RETURN
 6667 print *, 2001
      RETURN
   60 Mathlib_dpgamm = 0.d 0
      IF(dabs(z)    .LT.    1.d-77)   RETURN
      Mathlib_dpgamm = 1.d 0/z
   61 RETURN
 2000 FORMAT (/////, 2x, 32hdpgamm ..... argument too large., /////)
 2001 FORMAT (/////, 2x, 32hdpgamm ..... argument is a pole., /////)
      END



      SUBROUTINE Mathlib_Gaus8(fun,aa,bb,result)
*//////////////////////////////////////////////////////////////////////////////
*//   8-point Gauss                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  fun,aa,bb,result
      EXTERNAL fun
      DOUBLE PRECISION  a,b,sum8,xmidle,range,xplus,xminu
      INTEGER           k,i
      DOUBLE PRECISION  wg(4),xx(4)
      DATA wg /0.101228536290376d0, 0.222381034453374d0, 0.313706645877887d0, 0.362683783378362d0/
      DATA xx /0.960289856497536d0, 0.796666477413627d0, 0.525532409916329d0, 0.183434642495650d0/
*-------------------------------------------------------------------------------
      a  = aa
      b  = bb
      xmidle= 0.5d0*(a+b)
      range = 0.5d0*(b-a)
      sum8 =0d0
      DO i=1,4
         xplus= xmidle+range*xx(i)
         xminu= xmidle-range*xx(i)
         sum8 =sum8  +(fun(xplus)+fun(xminu))*wg(i)/2d0
      ENDDO
      result = sum8*(b-a)
      END

      SUBROUTINE Mathlib_Gaus16(fun,aa,bb,result)
*//////////////////////////////////////////////////////////////////////////////
*//   12-point Gauss                                                          //
*//////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  fun,aa,bb,result
      EXTERNAL fun
      DOUBLE PRECISION  a,b,sum16,xmidle,range,xplus,xminu
      INTEGER           k,i
      DOUBLE PRECISION  wg(8),xx(8)
      DATA wg              /0.027152459411754d0, 0.062253523938648d0,
     $ 0.095158511682493d0, 0.124628971255534d0, 0.149595988816577d0,
     $ 0.169156519395003d0, 0.182603415044924d0, 0.189450610455069d0/
      DATA xx              /0.989400934991650d0, 0.944575023073233d0,
     $ 0.865631202387832d0, 0.755404408355003d0, 0.617876244402644d0,
     $ 0.458016777657227d0, 0.281603550779259d0, 0.095012509837637d0/
*-------------------------------------------------------------------------------
      a  = aa
      b  = bb
      xmidle= 0.5d0*(a+b)
      range = 0.5d0*(b-a)
      sum16 =0d0
      DO i=1,8
         xplus= xmidle+range*xx(i)
         xminu= xmidle-range*xx(i)
         sum16 =sum16  +(fun(xplus)+fun(xminu))*wg(i)/2d0
      ENDDO
      result = sum16*(b-a)
      END


      SUBROUTINE Mathlib_FLPSOR(A,N)
C (M.B.) taken from CERNLIB/KERNLIB
C        moved to DOUBLE PRECISION
C
C CERN PROGLIB# M103    FLPSOR          .VERSION KERNFOR  3.15  820113
C ORIG. 29/04/78
C
C   SORT THE ONE-DIMENSIONAL FLOATING POINT ARRAY A(1),...,A(N) BY
C   INCREASING VALUES
C
C-    PROGRAM  M103  TAKEN FROM CERN PROGRAM LIBRARY,  29-APR-78
C
      IMPLICIT DOUBLE PRECISION(a-h,o-z)
      DOUBLE PRECISION A(N)
C (M.B.) this common is not needed
*      COMMON /SLATE/ LT(20),RT(20)
      INTEGER LT(20)
      INTEGER R,RT(20)
C end (M.B.)
C
      LEVEL=1
      LT(1)=1
      RT(1)=N
   10 L=LT(LEVEL)
      R=RT(LEVEL)
      LEVEL=LEVEL-1
   20 IF(R.GT.L) GO TO 200
      IF(LEVEL) 50,50,10
C
C   SUBDIVIDE THE INTERVAL L,R
C     L : LOWER LIMIT OF THE INTERVAL (INPUT)
C     R : UPPER LIMIT OF THE INTERVAL (INPUT)
C     J : UPPER LIMIT OF LOWER SUB-INTERVAL (OUTPUT)
C     I : LOWER LIMIT OF UPPER SUB-INTERVAL (OUTPUT)
C
  200 I=L
      J=R
      M=(L+R)/2
      X=A(M)
  220 IF(A(I).GE.X) GO TO 230
      I=I+1
      GO TO 220
  230 IF(A(J).LE.X) GO TO 231
      J=J-1
      GO TO 230
C
  231 IF(I.GT.J) GO TO 232
      W=A(I)
      A(I)=A(J)
      A(J)=W
      I=I+1
      J=J-1
      IF(I.LE.J) GO TO 220
C
  232 LEVEL=LEVEL+1
      IF((R-I).GE.(J-L)) GO TO 30
      LT(LEVEL)=L
      RT(LEVEL)=J
      L=I
      GO TO 20
   30 LT(LEVEL)=I
      RT(LEVEL)=R
      R=J
      GO TO 20
   50 RETURN
      END


*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                End  Pseudo-CLASS  Mathlib                                //
*//////////////////////////////////////////////////////////////////////////////
