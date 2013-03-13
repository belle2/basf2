*
* New mixed QCD-corrections based on B.Kniehl QCD-library
*
 
      FUNCTION AQCDBK(ALST,AMZ2,AMW2,AMT2)
*
      IMPLICIT REAL*8(A-H,O-W,Y-Z)
      IMPLICIT COMPLEX*16(X)
      COMPLEX*16 CV1
*
      DATA PI/3.1415926535898D0/
      DATA D3/1.2020569031596D0/
*
* WITHOUT FACTOR ALPI/4/PI
*
      QTM =2D0/3D0
*
      XCR   =DCMPLX(AMZ2/(4D0*AMT2))
      AQCDBK=-ALST/PI*4D0*QTM**2
     &      *(DREAL(CV1(XCR)/XCR)-(4D0*D3-5D0/6D0))
*
      END

      FUNCTION DRMQCD(AMZ2,AMW2,AMT2)
*
      IMPLICIT REAL*8(A-W,Y-Z)
      IMPLICIT COMPLEX*16(X)
      COMPLEX*16 CV1,CA1,CF1
*
      DATA D2/1.6449340668482D0/
      DATA D3/1.2020569031596D0/
*
      CW2 =AMW2/AMZ2
      SW2 =1D0-CW2
      QTM=2D0/3D0
      QBM=1D0/3D0
      XCR  =DCMPLX(AMZ2/(4D0*AMT2))
      XCX  =DCMPLX(AMW2/AMT2)
      XZERO=DCMPLX(0D0)
*
      V1P=4D0*D3-5D0/6D0
*
      DRMQCD=QTM**2*V1P 
     &      +CW2/SW2**2*AMT2/AMw2/4*(D2+1D0/2D0)
     &-1D0/4/SW2**2*AMT2/AMZ2*((1D0-4D0*QTM*SW2)**2*DREAL(CV1(XCR))
     &                            +DREAL(CA1(XCR))-DREAL(CA1(XZERO)))
     &+(CW2-SW2)/SW2**2*AMT2/AMW2*(DREAL(CF1(XCX))-DREAL(CF1(XZERO)))
     &      -1D0/8*(1D0-4D0*QBM*SW2)/SW2**2*LOG(AMT2/AMZ2)
*
      END
*
      FUNCTION XKMQCD(AMZ2,AMW2,AMT2,S)
*
      IMPLICIT REAL*8(A-W,Y-Z)
      IMPLICIT COMPLEX*16(X)
      COMPLEX*16 CV1,CA1,CF1
*
      CW2 =AMW2/AMZ2
      SW2 =1D0-CW2
*
      QTM=2D0/3D0
      QBM=1D0/3D0
      XCR=DCMPLX(AMZ2/(4D0*AMT2))
      XCS=DCMPLX( S  /(4D0*AMT2))
      XCX=DCMPLX(AMW2/AMT2)
*
      XKMQCD=CW2/SW2**2/4D0*AMT2/AMW2
     &      *((1D0-4D0*QTM*SW2)**2*CV1(XCR)+CA1(XCR))
     &      -CW2/SW2**2*AMT2/AMW2*CF1(XCX)
     &      +1D0/SW2*(1D0-4D0*QTM*SW2)*QTM*AMT2/S*CV1(XCS)
     &      -1D0/16D0/SW2**2*(2D0*(1D0-2D0*QBM*SW2)*LOG(AMZ2/AMT2)
     &      +4D0*(1D0-4D0*QBM*SW2)*QBM*SW2*LOG(S/AMZ2))
*
      END

      FUNCTION XRMQCD(AMZ2,AMW2,AMT2,S)
*
      IMPLICIT REAL*8(A-W,Y-Z)
      IMPLICIT COMPLEX*16(X)
      COMPLEX*16 CV1,CA1
*
      DATA D2/1.6449340668482D0/
      DATA D3/1.2020569031596D0/
      DATA EPS/1D-3/
*
      CW2 =AMW2/AMZ2
      SW2 =1D0-CW2
*
      QTM=2D0/3D0
      QBM=1D0/3D0
      XCR=DCMPLX(AMZ2/(4D0*AMT2))
      XCS=DCMPLX( S  /(4D0*AMT2))
*
      XV1r =CV1(XCR)
      XA1r =CA1(XCR)
      XV1rs=CV1(XCS)
      XA1rs=CA1(XCS)
*
      SMZ2=S/AMZ2
      DMZ2=1.D0-SMZ2
*
      IF(ABS(DMZ2).LT.EPS) THEN
        VT =1D0-4D0*QTM*SW2
        VB =1D0-4D0*QBM*SW2
        VT2=VT**2
        VB2=VB**2
*
        ALTZ=-AMT2/AMZ2
        ALTS=-AMT2/S
        XPVFTS=XPVFI(ALTS)
        XPVFTZ=XPVFI(ALTZ)
        XPAFTS=XPAFI(ALTS)
        XPAFTZ=XPAFI(ALTZ)
*
        XDVFTZ=XDPVFI(ALTZ)
        XDAFTZ=XDPAFI(ALTZ)                                                   
        XRMQCD=1D0/(12D0*SW2)
     &        *(3D0/4D0/CW2*(1D0+VB2)
     &        +AMT2/4D0/AMW2*(VT2*(XDVFTZ/ALTZ-XPVFTZ)
     &        +XDAFTZ/ALTZ-XPAFTZ)
     &        -AMT2/AMW2*(3D0*D2+105D0/8D0))
      ELSE
        XRMQCD=1D0/4D0/SW2/CW2
     &        *(AMT2/AMZ2*((1D0-4D0*QTM*SW2)**2*XV1r+XA1r)
     &        +AMT2/(AMZ2-S)*((1-4*QTM*SW2)**2*(XV1rs-XV1r)+XA1rs-XA1r)
     &        +2D0*AMT2/AMZ2*(-23D0/8D0+D2+3D0*D3)
     &    -1D0/4D0*(1D0+(1D0-4D0*QBM*SW2)**2)*S/(AMZ2-S)*LOG(S/AMZ2))      
      ENDIF
*
      END
*
* B.Kniehl QCD-library
*
      FUNCTION CV1(CR)
C     ****************
C V_1(r) defined by Eq. (10) in
C B.A. Kniehl, Nucl. Phys. B347 (1990) 86.
C r = (s + i*epsilon)/(4*m^2), where s may be complex.
C
      IMPLICIT LOGICAL (A-Z)
      COMPLEX*16 CR,CV1
      REAL*8 PI
      COMPLEX*16 C1,C2,CD,CF,CFS,CG,CH,CLI2,CLI3,CM,CMQ,CMS,CONE,CP,CRT,
     .           CS,CZERO,CZETA2,CZETA3
      DATA CZERO,CONE/(0.D0,0.D0),(1.D0,0.D0)/
      PI=4.D0*DATAN(1.D0)
      CZETA2=DCMPLX(PI**2/6.D0)
      CZETA3=CLI3(CONE,CONE)
      IF (CR.EQ.CZERO) THEN
          CV1=CZERO
      ELSE IF (CR.EQ.CONE) THEN
          CV1=DCMPLX(0.D0,PI**3)
          WRITE (6,*)
     .        'Warning: Coulomb singularity of V_1(r) at r = 1 !'
      ELSE
          C1=CRT(CONE-CR,-CONE)
          C2=CRT(-CR,-CONE)
          CS=C1/C2
          CP=C1+C2
          CM=C1-C2
          CMS=CM**2
          CMQ=CMS**2
          CF=CDLOG(CP)
          CFS=CF**2
          CG=CDLOG(CP-CM)
          CH=CDLOG(CP+CM)
          CD=CLI2(CMS,CONE)-CLI2(CMQ,CONE)
          CV1=4.D0*(CR-1.D0/(4.D0*CR))*(2.D0*CLI3(CMS,CONE)
     .        -CLI3(CMQ,CONE)+8.D0/3.D0*CF*CD
     .        +4.D0*CFS*(-CF+CG/3.D0+2.D0/3.D0*CH))
     .        +CS*(8.D0/3.D0*(CR+1.D0/2.D0)*(CD
     .        +CF*(-3.D0*CF+2.D0*CG+4.D0*CH))-2.D0*(CR+3.D0/2.D0)*CF)
     .        -8.D0*(CR-1.D0/6.D0-7.D0/(48.D0*CR))*CFS+13.D0/6.D0
     .        +CZETA3/CR
          END IF
      RETURN
C***********************************************************************
      END

      FUNCTION CA1(CR)
C     ****************
C A_1(r) defined by Eq. (11) in
C B.A. Kniehl, Nucl. Phys. B347 (1990) 86.
C r = (s + i*epsilon)/(4*m^2), where s may be complex.
C
      IMPLICIT LOGICAL (A-Z)
      COMPLEX*16 CR,CA1
      REAL*8 PI
      COMPLEX*16 C1,C2,CD,CF,CFS,CG,CH,CLI2,CLI3,CM,CMQ,CMS,CONE,CP,CRT,
     .           CS,CZERO,CZETA2,CZETA3
      DATA CZERO,CONE/(0.D0,0.D0),(1.D0,0.D0)/
      PI=4.D0*DATAN(1.D0)
      CZETA2=DCMPLX(PI**2/6.D0)
      CZETA3=CLI3(CONE,CONE)
      IF (CR.EQ.CZERO) THEN
          CA1=3.D0*(-2.D0*CZETA3-CZETA2+7.D0/4.D0)
      ELSE IF (CR.EQ.CONE) THEN
          CA1=-2.D0*CZETA3-3.D0/8.D0*CZETA2+29.D0/12.D0
      ELSE
          C1=CRT(CONE-CR,-CONE)
          C2=CRT(-CR,-CONE)
          CS=C1/C2
          CP=C1+C2
          CM=C1-C2
          CMS=CM**2
          CMQ=CMS**2
          CF=CDLOG(CP)
          CFS=CF**2
          CG=CDLOG(CP-CM)
          CH=CDLOG(CP+CM)
          CD=CLI2(CMS,CONE)-CLI2(CMQ,CONE)
          CA1=4.D0*(CR-3.D0/2.D0+1.D0/(2.D0*CR))*(2.D0*CLI3(CMS,CONE)
     .        -CLI3(CMQ,CONE)+8.D0/3.D0*CF*CD
     .        +4.D0*CFS*(-CF+CG/3.D0+2.D0/3.D0*CH))
     .        +CS*(8.D0/3.D0*(CR-1.D0)*(CD
     .        +CF*(-3.D0*CF+2.D0*CG+4.D0*CH))
     .        -2.D0*(CR-3.D0+1.D0/(4.D0*CR))*CF)
     .        -8.D0*(CR-11.D0/12.D0+5.D0/(48.D0*CR)
     .        +1.D0/(32.D0*CR**2))*CFS
     .        -3.D0*CZETA2+13.D0/6.D0+(-2.D0*CZETA3+1.D0/4.D0)/CR
          END IF
      RETURN
C***********************************************************************
      END

      FUNCTION CF1(CX)
C     ****************
C F_1(x) defined by Eq. (12) in
C B.A. Kniehl, Nucl. Phys. B347 (1990) 86.
C x = (s + i*epsilon)/m^2, where s may be complex.
C
      IMPLICIT LOGICAL (A-Z)
      COMPLEX*16 CX,CF1
      REAL*8 PI
      COMPLEX*16 CA,CB,CBS,CD,CLI2,CLI3,CLN,CXS,CONE,CZERO,CZETA2,CZETA3
      DATA CZERO,CONE/(0.D0,0.D0),(1.D0,0.D0)/
      PI=4.D0*DATAN(1.D0)
      CZETA2=DCMPLX(PI**2/6.D0)
      CZETA3=CLI3(CONE,CONE)
      IF (CX.EQ.CZERO) THEN
          CF1=-3.D0/2.D0*CZETA3-CZETA2/2.D0+23.D0/16.D0
      ELSE IF (CX.EQ.CONE) THEN
          CF1=-CZETA3/2.D0-CZETA2/12.D0+7.D0/8.D0
      ELSE
          CXS=CX**2
          CA=CLN(-CX,-CONE)
          CB=CLN(CONE-CX,-CONE)
          CBS=CB**2
          CD=CLI2(CONE/(CONE-CX),CONE)
          CF1=(CX-3.D0/2.D0+1.D0/(2.D0*CXS))*(CLI3(CONE/(CONE-CX),CONE)
     .        +2.D0/3.D0*CB*CD-CBS/6.D0*(CA-CB))
     .        +(CX+1.D0/2.D0-1.D0/(2.D0*CX))/3.D0*(CD-CA*CB)
     .        +(CX-1.D0/8.D0-1.D0/CX+5.D0/(8.D0*CXS))/3.D0*CBS
     .        -(CX-5.D0/2.D0+2.D0/(3.D0*CX)+5.D0/(6.D0*CXS))/4.D0*CB
     .        -3.D0/4.D0*CZETA2+13.D0/12.D0-5.D0/(24.D0*CX)
     .        -CZETA3/(2.D0*CXS)
          END IF
      RETURN
C***********************************************************************
      END

      FUNCTION CLI2(CY,CE)
C     ********************
C Li_2(y + i*epsilon*sign(Re(e)))
C
      IMPLICIT LOGICAL (A-Z)
      COMPLEX*16 CY,CE,CLI2
      INTEGER I,IFLAG1,IFLAG2,N
      REAL*8 B(0:60),EPS,PI
      COMPLEX*16 CH,CLN,COLD,CONE,CX,CZ,CZERO,CZETA2
      DATA CZERO,CONE/(0.D0,0.D0),(1.D0,0.D0)/
C N = # of terms to be evaluated in the expansion, N <= 61.
      N=30
C Relative precision.
      EPS=1.D-20
      PI=4.D0*DATAN(1.D0)
      CZETA2=DCMPLX(PI**2/6.D0)
      DO 1 I=3,59,2
1         B(I)=0.D0
      B(0)=1.D0
      B(1)=-.5D0
      B(2)=1.D0/6.D0
      B(4)=-1.D0/30.D0
      B(6)=1.D0/42.D0
      B(8)=B(4)
      B(10)=5.D0/66.D0
      B(12)=-691.D0/2730.D0
      B(14)=7.D0/6.D0
      B(16)=-3617.D0/510.D0
      B(18)=43867.D0/798.D0
      B(20)=-174611.D0/330.D0
      B(22)=854513.D0/138.D0
      B(24)=-236364091.D0/2730.D0
      B(26)=8553103.D0/6.D0
      B(28)=-23749461029.D0/870.D0
      B(30)=8615841276005.D0/14322.D0
      B(32)=-7709321041217.D0/510.D0
      B(34)=2577687858367.D0/6.D0
      B(36)=-26315271553053477373.D0/1919190.D0
      B(38)=2929993913841559.D0/6.D0
      B(40)=-261082718496449122051.D0/13530.D0
      B(42)=1520097643918070802691.D0/1806.D0
      B(44)=-27833269579301024235023.D0/690.D0
      B(46)=596451111593912163277961.D0/282.D0
      B(48)=-5609403368997817686249127547.D0/46410.D0
      B(50)=495057205241079648212477525.D0/66.D0
      B(52)=-801165718135489957347924991853.D0/1590.D0
      B(54)=29149963634884862421418123812691.D0/798.D0
      B(56)=-2479392929313226753685415739663229.D0/870.D0
      B(58)=84483613348880041862046775994036021.D0/354.D0
C      B(60)=-1215233140483755572040304994079820246041491.D0/56786730.D0
      CX=CY
      IF (CX.EQ.CZERO) THEN
          CLI2=CZERO
      ELSE IF (CX.EQ.CONE) THEN
          CLI2=CZETA2
      ELSE
          IFLAG1=0
          IFLAG2=0
          IF (CDABS(CX).GT.1.D0) THEN
              CX=CONE/CX
              IFLAG1=1
              END IF
          IF (DREAL(CX).GT..5D0) THEN
              CX=CONE-CX
              IFLAG2=1
              END IF
          CZ=-CDLOG(CONE-CX)
          CH=CONE
          CLI2=CZERO
          DO 2 I=1,N
              COLD=CLI2
              CH=CH*CZ/DCMPLX(DFLOAT(I))
              CLI2=CLI2+DCMPLX(B(I-1))*CH
2             IF (B(I-1).NE.0.D0.AND.CDABS(COLD/CLI2-CONE).LT.EPS)
     .            GOTO 3
3         IF (IFLAG2.EQ.1) THEN
              CLI2=-CLI2+CZETA2-CDLOG(CX)*CDLOG(CONE-CX)
              END IF
          IF (IFLAG1.EQ.1) THEN
              CLI2=-CLI2-CZETA2-DCMPLX(.5D0)*CLN(-CY,-CE)**2
              END IF
          END IF
      RETURN
C***********************************************************************
      END

      FUNCTION CLI3(CY,CE)
C     ********************
C Li_3(y + i*epsilon*sign(Re(e)))
C
      IMPLICIT LOGICAL (A-Z)
      COMPLEX*16 CY,CE,CLI3
      INTEGER IFLAG
      REAL*8 PI
      COMPLEX*16 CH,CLI3H,CLN,CONE,CX,CZERO,CZETA2,CZETA3
      DATA CZERO,CONE/(0.D0,0.D0),(1.D0,0.D0)/
      PI=4.D0*DATAN(1.D0)
      CZETA2=DCMPLX(PI**2/6.D0)
      CZETA3=DCMPLX(1.20205690315959D0)
      CX=CY
      IF (CX.EQ.CZERO) THEN
          CLI3=CZERO
      ELSE IF (CX.EQ.CONE) THEN
          CLI3=CZETA3
      ELSE
          IFLAG=0
          IF (CDABS(CX).GT.1.D0) THEN
              CX=CONE/CX
              IFLAG=1
              END IF
          IF (DREAL(CX).GT..5D0) THEN
              CH=CDLOG(CX)
              CLI3=-CLI3H(CONE-CONE/CX)-CLI3H(CONE-CX)+CZETA3+CZETA2*CH
     .             +CH**3/DCMPLX(6.D0)-DCMPLX(.5D0)*CH**2*CDLOG(CONE-CX)
          ELSE
              CLI3=CLI3H(CX)
              END IF
          IF (IFLAG.EQ.1) THEN
              CH=CLN(-CY,-CE)
              CLI3=CLI3-CZETA2*CH-CH**3/DCMPLX(6.D0)
              END IF
          END IF
      RETURN
C***********************************************************************
      END

      FUNCTION CLI3H(CX)
C     ******************
C Li_3(x) for 0 < |x| <= 1, Re(x) <= 1/2.
C
      IMPLICIT LOGICAL (A-Z)
      COMPLEX*16 CX,CLI3H
      INTEGER I,J,N
C N = # of terms to be evaluated in the expansion, N <= 61.
      PARAMETER (N=30)
      REAL*8 B(0:60),EPS,F(0:N)
      COMPLEX*16 CC,COLD,CONE,CP,CZ,CZERO
      DATA CZERO,CONE/(0.D0,0.D0),(1.D0,0.D0)/
C Relative precision.
      EPS=1.D-20
      DO 1 I=3,59,2
1         B(I)=0.D0
      B(0)=1.D0
      B(1)=-.5D0
      B(2)=1.D0/6.D0
      B(4)=-1.D0/30.D0
      B(6)=1.D0/42.D0
      B(8)=B(4)
      B(10)=5.D0/66.D0
      B(12)=-691.D0/2730.D0
      B(14)=7.D0/6.D0
      B(16)=-3617.D0/510.D0
      B(18)=43867.D0/798.D0
      B(20)=-174611.D0/330.D0
      B(22)=854513.D0/138.D0
      B(24)=-236364091.D0/2730.D0
      B(26)=8553103.D0/6.D0
      B(28)=-23749461029.D0/870.D0
      B(30)=8615841276005.D0/14322.D0
      B(32)=-7709321041217.D0/510.D0
      B(34)=2577687858367.D0/6.D0
      B(36)=-26315271553053477373.D0/1919190.D0
      B(38)=2929993913841559.D0/6.D0
      B(40)=-261082718496449122051.D0/13530.D0
      B(42)=1520097643918070802691.D0/1806.D0
      B(44)=-27833269579301024235023.D0/690.D0
      B(46)=596451111593912163277961.D0/282.D0
      B(48)=-5609403368997817686249127547.D0/46410.D0
      B(50)=495057205241079648212477525.D0/66.D0
      B(52)=-801165718135489957347924991853.D0/1590.D0
      B(54)=29149963634884862421418123812691.D0/798.D0
      B(56)=-2479392929313226753685415739663229.D0/870.D0
      B(58)=84483613348880041862046775994036021.D0/354.D0
C      B(60)=-1215233140483755572040304994079820246041491.D0/56786730.D0
      F(0)=1.D0
      DO 2 I=1,N
2         F(I)=F(I-1)*DFLOAT(I)
      CZ=-CDLOG(CONE-CX)
      CP=CONE
      CLI3H=CZERO
      DO 3 I=1,N
          COLD=CLI3H
          CP=CP*CZ
          CC=CZERO
          DO 4 J=1,I
4             CC=CC+DCMPLX(B(J-1)*B(I-J)/(F(J)*F(I-J)))
          CLI3H=CLI3H+CP*CC/DCMPLX(DFLOAT(I))
3     IF (CDABS(COLD/CLI3H-CONE).LT.EPS) GOTO 5
5     RETURN
C***********************************************************************
      END
      FUNCTION CRT(CX,CE)
C     *******************
C sqrt(x + i*epsilon*sign(Re(e)))
C
      IMPLICIT LOGICAL (A-Z)
      COMPLEX*16 CX,CE,CRT
      IF (DREAL(CX).LT.0.D0.AND.DIMAG(CX).EQ.0.D0) THEN
          CRT=DSIGN(1.D0,DREAL(CE))*(0.D0,1.D0)*CDSQRT(-CX)
      ELSE
          CRT=CDSQRT(CX)
          END IF
      RETURN
C***********************************************************************
      END

      FUNCTION CLN(CX,CE)
C     *******************
C ln(x + i*epsilon*sign(Re(e)))
C
      IMPLICIT LOGICAL (A-Z)
      COMPLEX*16 CX,CE,CLN
      REAL*8 PI
      PI=4.D0*DATAN(1.D0)
      IF (DREAL(CX).LT.0.D0.AND.DIMAG(CX).EQ.0.D0) THEN
          CLN=CDLOG(-CX)+DSIGN(1.D0,DREAL(CE))*(0.D0,1.D0)*PI
      ELSE
          CLN=CDLOG(CX)
          END IF
      RETURN
C***********************************************************************
      END
