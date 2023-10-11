      FUNCTION FORMOM(XMAA,XMOM)
C     ==================================================================
C     formfactorfor pi-pi0 gamma final state
C      R. Decker, Z. Phys C36 (1987) 487.
C     ==================================================================
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      COMMON /TESTA1/ KEYA1
      COMPLEX BWIGN,FORMOM
      DATA ICONT /1/
* THIS INLINE FUNCT. CALCULATES THE SCALAR PART OF THE PROPAGATOR
      BWIGN(XM,AM,GAMMA)=1./CMPLX(XM**2-AM**2,GAMMA*AM)
* HADRON CURRENT
      FRO  =0.266*AMRO**2
      ELPHA=- 0.1
      AMROP = 1.7
      GAMROP= 0.26
      AMOM  =0.782
      GAMOM =0.0085
      AROMEG= 1.0
      GCOUP=12.924
      GCOUP=GCOUP*AROMEG
      FQED  =SQRT(4.0*3.1415926535/137.03604)
      FORMOM=FQED*FRO**2/SQRT(2.0)*GCOUP**2*BWIGN(XMOM,AMOM,GAMOM)
     $     *(BWIGN(XMAA,AMRO,GAMRO)+ELPHA*BWIGN(XMAA,AMROP,GAMROP))
     $     *(BWIGN( 0.0,AMRO,GAMRO)+ELPHA*BWIGN( 0.0,AMROP,GAMROP))
      END
      FUNCTION FORM1(MNUM,QQ,S1,SDWA)
C     ==================================================================
C     formfactorfor F1 for 3 scalar final state
C     R. Fisher, J. Wess and F. Wagner Z. Phys C3 (1980) 313
C     H. Georgi, Weak interactions and modern particle theory,
C     The Benjamin/Cummings Pub. Co., Inc. 1984.
C     R. Decker, E. Mirkes, R. Sauer, Z. Was Karlsruhe preprint TTP92-25
C     and erratum !!!!!!
C     ==================================================================
C
      COMPLEX FORM1,WIGNER,WIGFOR,FPIKM,BWIGM
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      WIGNER(A,B,C)= CMPLX(1.0,0.0)/CMPLX(A-B**2,B*C)
      IF     (MNUM.EQ.0) THEN
C ------------  3 pi hadronic state (a1)
       GAMAX=GAMA1*GFUN(QQ)/GFUN(AMA1**2)
       FORM1=AMA1**2*WIGNER(QQ,AMA1,GAMAX)*FPIKM(SQRT(S1),AMPI,AMPI)
      ELSEIF (MNUM.EQ.1) THEN
C ------------ K- pi- K+
       FORM1=BWIGM(S1,AMKST,GAMKST,AMPI,AMK)
         GAMAX=GAMA1*GFUN(QQ)/GFUN(AMA1**2)
       FORM1=AMA1**2*WIGNER(QQ,AMA1,GAMAX)*FORM1
      ELSEIF (MNUM.EQ.2) THEN
C ------------ K0 pi- K0B
       FORM1=BWIGM(S1,AMKST,GAMKST,AMPI,AMK)
         GAMAX=GAMA1*GFUN(QQ)/GFUN(AMA1**2)
       FORM1=AMA1**2*WIGNER(QQ,AMA1,GAMAX)*FORM1
      ELSEIF (MNUM.EQ.3) THEN
C ------------ K- K0 pi0
       FORM1=0.0
         GAMAX=GAMA1*GFUN(QQ)/GFUN(AMA1**2)
       FORM1=AMA1**2*WIGNER(QQ,AMA1,GAMAX)*FORM1
      ELSEIF (MNUM.EQ.4) THEN
C ------------ pi0 pi0 K-
       XM2=1.402
       GAM2=0.174
       FORM1=BWIGM(S1,AMKST,GAMKST,AMK,AMPI)
       FORM1=WIGFOR(QQ,XM2,GAM2)*FORM1
      ELSEIF (MNUM.EQ.5) THEN
C ------------ K- pi- pi+
       XM2=1.402
       GAM2=0.174
       FORM1=WIGFOR(QQ,XM2,GAM2)*FPIKM(SQRT(S1),AMPI,AMPI)
      ELSEIF (MNUM.EQ.6) THEN
       FORM1=0.0
      ELSEIF (MNUM.EQ.7) THEN
C -------------- eta pi- pi0 final state
       FORM1=0.0
      ENDIF
C
      END
      FUNCTION FORM2(MNUM,QQ,S1,SDWA)
C     ==================================================================
C     formfactorfor F2 for 3 scalar final state
C     R. Fisher, J. Wess and F. Wagner Z. Phys C3 (1980) 313
C     H. Georgi, Weak interactions and modern particle theory,
C     The Benjamin/Cummings Pub. Co., Inc. 1984.
C     R. Decker, E. Mirkes, R. Sauer, Z. Was Karlsruhe preprint TTP92-25
C     and erratum !!!!!!
C     ==================================================================
C
      COMPLEX FORM2,WIGNER,WIGFOR,FPIKM,BWIGM
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      WIGNER(A,B,C)= CMPLX(1.0,0.0)/CMPLX(A-B**2,B*C)
      IF     (MNUM.EQ.0) THEN
C ------------  3 pi hadronic state (a1)
       GAMAX=GAMA1*GFUN(QQ)/GFUN(AMA1**2)
       FORM2=AMA1**2*WIGNER(QQ,AMA1,GAMAX)*FPIKM(SQRT(S1),AMPI,AMPI)
      ELSEIF (MNUM.EQ.1) THEN
C ------------ K- pi- K+
         GAMAX=GAMA1*GFUN(QQ)/GFUN(AMA1**2)
       FORM2=AMA1**2*WIGNER(QQ,AMA1,GAMAX)*FPIKM(SQRT(S1),AMPI,AMPI)
      ELSEIF (MNUM.EQ.2) THEN
C ------------ K0 pi- K0B
         GAMAX=GAMA1*GFUN(QQ)/GFUN(AMA1**2)
       FORM2=AMA1**2*WIGNER(QQ,AMA1,GAMAX)*FPIKM(SQRT(S1),AMPI,AMPI)
      ELSEIF (MNUM.EQ.3) THEN
C ------------ K- K0 pi0
         GAMAX=GAMA1*GFUN(QQ)/GFUN(AMA1**2)
       FORM2=AMA1**2*WIGNER(QQ,AMA1,GAMAX)*FPIKM(SQRT(S1),AMPI,AMPI)
      ELSEIF (MNUM.EQ.4) THEN
C ------------ pi0 pi0 K-
       XM2=1.402
       GAM2=0.174
       FORM2=BWIGM(S1,AMKST,GAMKST,AMK,AMPI)
       FORM2=WIGFOR(QQ,XM2,GAM2)*FORM2
      ELSEIF (MNUM.EQ.5) THEN
C ------------ K- pi- pi+
       XM2=1.402
       GAM2=0.174
       FORM2=BWIGM(S1,AMKST,GAMKST,AMK,AMPI)
       FORM2=WIGFOR(QQ,XM2,GAM2)*FORM2
C
      ELSEIF (MNUM.EQ.6) THEN
       XM2=1.402
       GAM2=0.174
       FORM2=WIGFOR(QQ,XM2,GAM2)*FPIKM(SQRT(S1),AMPI,AMPI)
C
      ELSEIF (MNUM.EQ.7) THEN
C -------------- eta pi- pi0 final state
       FORM2=0.0
      ENDIF
C
      END
      COMPLEX FUNCTION BWIGM(S,M,G,XM1,XM2)
C **********************************************************
C     P-WAVE BREIT-WIGNER  FOR RHO
C **********************************************************
      REAL S,M,G,XM1,XM2
      REAL PI,QS,QM,W,GS
      DATA INIT /0/
C ------------ PARAMETERS --------------------
      IF (INIT.EQ.0) THEN
      INIT=1
      PI=3.141592654
C -------  BREIT-WIGNER -----------------------
         ENDIF
       IF (S.GT.(XM1+XM2)**2) THEN
         QS=SQRT(ABS((S   -(XM1+XM2)**2)*(S   -(XM1-XM2)**2)))/SQRT(S)
         QM=SQRT(ABS((M**2-(XM1+XM2)**2)*(M**2-(XM1-XM2)**2)))/M
         W=SQRT(S)
         GS=G*(M/W)**2*(QS/QM)**3
       ELSE
         GS=0.0
       ENDIF
         BWIGM=M**2/CMPLX(M**2-S,-SQRT(S)*GS)
      RETURN
      END
      COMPLEX FUNCTION FPIKM(W,XM1,XM2)
C **********************************************************
C     PION FORM FACTOR
C **********************************************************
      COMPLEX BWIGM
      REAL ROM,ROG,ROM1,ROG1,BETA1,PI,PIM,S,W
      EXTERNAL BWIG
      DATA  INIT /0/
C
C ------------ PARAMETERS --------------------
      IF (INIT.EQ.0 ) THEN
      INIT=1
      PI=3.141592654
      PIM=.140
      ROM=0.773
      ROG=0.145
      ROM1=1.370
      ROG1=0.510
      BETA1=-0.145
      ENDIF
C -----------------------------------------------
      S=W**2
      FPIKM=(BWIGM(S,ROM,ROG,XM1,XM2)+BETA1*BWIGM(S,ROM1,ROG1,XM1,XM2))
     & /(1+BETA1)
      RETURN
      END
      COMPLEX FUNCTION FPIKMD(W,XM1,XM2)
C **********************************************************
C     PION FORM FACTOR
C **********************************************************
      COMPLEX BWIGM
      REAL ROM,ROG,ROM1,ROG1,PI,PIM,S,W
      EXTERNAL BWIG
      DATA  INIT /0/
C
C ------------ PARAMETERS --------------------
      IF (INIT.EQ.0 ) THEN
      INIT=1
      PI=3.141592654
      PIM=.140
      ROM=0.773
      ROG=0.145
      ROM1=1.500
      ROG1=0.220
      ROM2=1.750
      ROG2=0.120
      BETA=6.5
      DELTA=-26.0
      ENDIF
C -----------------------------------------------
      S=W**2
      FPIKMD=(DELTA*BWIGM(S,ROM,ROG,XM1,XM2)
     $      +BETA*BWIGM(S,ROM1,ROG1,XM1,XM2)
     $      +     BWIGM(S,ROM2,ROG2,XM1,XM2))
     & /(1+BETA+DELTA)
      RETURN
      END
 
      FUNCTION FORM3(MNUM,QQ,S1,SDWA)
C     ==================================================================
C     formfactorfor F3 for 3 scalar final state
C     R. Fisher, J. Wess and F. Wagner Z. Phys C3 (1980) 313
C     H. Georgi, Weak interactions and modern particle theory,
C     The Benjamin/Cummings Pub. Co., Inc. 1984.
C     R. Decker, E. Mirkes, R. Sauer, Z. Was Karlsruhe preprint TTP92-25
C     and erratum !!!!!!
C     ==================================================================
C
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMPLEX FORM3
      IF (MNUM.EQ.6) THEN
       FORM3=CMPLX(0.0)
      ELSE
       FORM3=CMPLX(0.0)
      ENDIF
        FORM3=0
      END
      FUNCTION FORM4(MNUM,QQ,S1,S2,S3)
C     ==================================================================
C     formfactorfor F4 for 3 scalar final state
C     R. Decker, in preparation
C     R. Decker, E. Mirkes, R. Sauer, Z. Was Karlsruhe preprint TTP92-25
C     and erratum !!!!!!
C     ==================================================================
C
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMPLEX FORM4,WIGNER,FPIKM
      REAL*4 M
      WIGNER(A,B,C)=CMPLX(1.0,0.0) /CMPLX(A-B**2,B*C)
      IF (MNUM.EQ.0) THEN
C ------------  3 pi hadronic state (a1)
        G1=5.8
        G2=6.08
        FPIP=0.02
        AMPIP=1.3
        GAMPIP=0.3
        S=QQ
        G=GAMPIP
        XM1=AMPIZ
        XM2=AMRO
        M  =AMPIP
         IF (S.GT.(XM1+XM2)**2) THEN
           QS=SQRT(ABS((S   -(XM1+XM2)**2)*(S   -(XM1-XM2)**2)))/SQRT(S)
           QM=SQRT(ABS((M**2-(XM1+XM2)**2)*(M**2-(XM1-XM2)**2)))/M
           W=SQRT(S)
           GS=G*(M/W)**2*(QS/QM)**5
         ELSE
           GS=0.0
         ENDIF
        GAMX=GS*W/M
        FORM4=G1*G2*FPIP/AMRO**4/AMPIP**2
     $       *AMPIP**2*WIGNER(QQ,AMPIP,GAMX)
     $       *( S1*(S2-S3)*FPIKM(SQRT(S1),AMPIZ,AMPIZ)
     $         +S2*(S1-S3)*FPIKM(SQRT(S2),AMPIZ,AMPIZ) )
      ELSEIF (MNUM.EQ.1) THEN
C ------------  3 pi hadronic state (a1)
        G1=5.8
        G2=6.08
        FPIP=0.02
        AMPIP=1.3
        GAMPIP=0.3
        S=QQ
        G=GAMPIP
        XM1=AMPIZ
        XM2=AMRO
        M  =AMPIP
         IF (S.GT.(XM1+XM2)**2) THEN
           QS=SQRT(ABS((S   -(XM1+XM2)**2)*(S   -(XM1-XM2)**2)))/SQRT(S)
           QM=SQRT(ABS((M**2-(XM1+XM2)**2)*(M**2-(XM1-XM2)**2)))/M
           W=SQRT(S)
           GS=G*(M/W)**2*(QS/QM)**5
         ELSE
           GS=0.0
         ENDIF
        GAMX=GS*W/M
        FORM4=G1*G2*FPIP/AMRO**4/AMPIP**2
     $       *AMPIP**2*WIGNER(QQ,AMPIP,GAMX)
     $       *( S1*(S2-S3)*FPIKM(SQRT(S1),AMPIZ,AMPIZ)
     $         +S2*(S1-S3)*FPIKM(SQRT(S2),AMPIZ,AMPIZ) )
      ELSE
        FORM4=CMPLX(0.0,0.0)
      ENDIF
C ---- this formfactor is switched off .. .
       FORM4=CMPLX(0.0,0.0)
      END
      FUNCTION FORM5(MNUM,QQ,S1,S2)
C     ==================================================================
C     formfactorfor F5 for 3 scalar final state
C     G. Kramer, W. Palmer, S. Pinsky, Phys. Rev. D30 (1984) 89.
C     G. Kramer, W. Palmer             Z. Phys. C25 (1984) 195.
C     R. Decker, E. Mirkes, R. Sauer, Z. Was Karlsruhe preprint TTP92-25
C     and erratum !!!!!!
C     ==================================================================
C
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMPLEX FORM5,WIGNER,FPIKM,FPIKMD,BWIGM
      WIGNER(A,B,C)=CMPLX(1.0,0.0)/CMPLX(A-B**2,B*C)
      IF     (MNUM.EQ.0) THEN
C ------------  3 pi hadronic state (a1)
        FORM5=0.0
      ELSEIF (MNUM.EQ.1) THEN
C ------------ K- pi- K+
         ELPHA=-0.2
         FORM5=FPIKMD(SQRT(QQ),AMPI,AMPI)/(1+ELPHA)
     $        *(       FPIKM(SQRT(S2),AMPI,AMPI)
     $          +ELPHA*BWIGM(S1,AMKST,GAMKST,AMPI,AMK))
      ELSEIF (MNUM.EQ.2) THEN
C ------------ K0 pi- K0B
         ELPHA=-0.2
         FORM5=FPIKMD(SQRT(QQ),AMPI,AMPI)/(1+ELPHA)
     $        *(       FPIKM(SQRT(S2),AMPI,AMPI)
     $          +ELPHA*BWIGM(S1,AMKST,GAMKST,AMPI,AMK))
      ELSEIF (MNUM.EQ.3) THEN
C ------------ K- K0 pi0
        FORM5=0.0
      ELSEIF (MNUM.EQ.4) THEN
C ------------ pi0 pi0 K-
        FORM5=0.0
      ELSEIF (MNUM.EQ.5) THEN
C ------------ K- pi- pi+
        ELPHA=-0.2
        FORM5=BWIGM(QQ,AMKST,GAMKST,AMPI,AMK)/(1+ELPHA)
     $       *(       FPIKM(SQRT(S1),AMPI,AMPI)
     $         +ELPHA*BWIGM(S2,AMKST,GAMKST,AMPI,AMK))
      ELSEIF (MNUM.EQ.6) THEN
C ------------ pi- K0B pi0
        ELPHA=-0.2
        FORM5=BWIGM(QQ,AMKST,GAMKST,AMPI,AMKZ)/(1+ELPHA)
     $       *(       FPIKM(SQRT(S2),AMPI,AMPI)
     $         +ELPHA*BWIGM(S1,AMKST,GAMKST,AMPI,AMK))
      ELSEIF (MNUM.EQ.7) THEN
C -------------- eta pi- pi0 final state
       FORM5=FPIKMD(SQRT(QQ),AMPI,AMPI)*FPIKM(SQRT(S1),AMPI,AMPI)
      ENDIF
C
      END
      SUBROUTINE CURR(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
C     ==================================================================
C     hadronic current for 4 pi final state
C     R. Fisher, J. Wess and F. Wagner Z. Phys C3 (1980) 313
C     R. Decker Z. Phys C36 (1987) 487.
C     M. Gell-Mann, D. Sharp, W. Wagner Phys. Rev. Lett 8 (1962) 261.
C     ==================================================================
 
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
C ARBITRARY FIXING OF THE FOUR PI X-SECTION NORMALIZATION
      COMMON /ARBIT/ ARFLAT,AROMEG
      REAL  PIM1(4),PIM2(4),PIM3(4),PIM4(4),PAA(4)
      COMPLEX HADCUR(4),FORM1,FORM2,FORM3,FPIKM
      COMPLEX BWIGN
      REAL PA(4),PB(4)
      REAL AA(4,4),PP(4,4)
      DATA PI /3.141592653589793238462643/
      DATA  FPI /93.3E-3/
      BWIGN(A,XM,XG)=1.0/CMPLX(A-XM**2,XM*XG)
C
C --- masses and constants
      G1=12.924
      G2=1475.98
      G =G1*G2
      ELPHA=-.1
      AMROP=1.7
      GAMROP=0.26
      AMOM=.782
      GAMOM=0.0085
      ARFLAT=1.0
      AROMEG=1.0
C
      FRO=0.266*AMRO**2
      COEF1=2.0*SQRT(3.0)/FPI**2*ARFLAT
      COEF2=FRO*G*AROMEG
C --- initialization of four vectors
      DO 7 K=1,4
      DO 8 L=1,4
 8    AA(K,L)=0.0
      HADCUR(K)=CMPLX(0.0)
      PAA(K)=PIM1(K)+PIM2(K)+PIM3(K)+PIM4(K)
      PP(1,K)=PIM1(K)
      PP(2,K)=PIM2(K)
      PP(3,K)=PIM3(K)
 7    PP(4,K)=PIM4(K)
C
      IF (MNUM.EQ.1) THEN
C ===================================================================
C pi- pi- p0 pi+ case                                            ====
C ===================================================================
       QQ=PAA(4)**2-PAA(3)**2-PAA(2)**2-PAA(1)**2
C --- loop over thre contribution of the non-omega current
       DO 201 K=1,3
        SK=(PP(K,4)+PIM4(4))**2-(PP(K,3)+PIM4(3))**2
     $    -(PP(K,2)+PIM4(2))**2-(PP(K,1)+PIM4(1))**2
C -- definition of AA matrix
C -- cronecker delta
        DO 202 I=1,4
         DO 203 J=1,4
 203     AA(I,J)=0.0
 202    AA(I,I)=1.0
C ... and the rest ...
        DO 204 L=1,3
         IF (L.NE.K) THEN
          DENOM=(PAA(4)-PP(L,4))**2-(PAA(3)-PP(L,3))**2
     $         -(PAA(2)-PP(L,2))**2-(PAA(1)-PP(L,1))**2
          DO 205 I=1,4
          DO 205 J=1,4
                      SIG= 1.0
           IF(J.NE.4) SIG=-SIG
           AA(I,J)=AA(I,J)
     $            -SIG*(PAA(I)-2.0*PP(L,I))*(PAA(J)-PP(L,J))/DENOM
 205      CONTINUE
         ENDIF
 204    CONTINUE
C --- let's add something to HADCURR
       FORM1= FPIKM(SQRT(SK),AMPI,AMPI) *FPIKM(SQRT(QQ),AMPI,AMPI)
C       FORM1= FPIKM(SQRT(SK),AMPI,AMPI) *FPIKMD(SQRT(QQ),AMPI,AMPI)
CCCCCCCCCCCCCCCCC       FORM1=WIGFOR(SK,AMRO,GAMRO)      (tests)
C
       FIX=1.0
       IF (K.EQ.3) FIX=-2.0
       DO 206 I=1,4
       DO 206 J=1,4
        HADCUR(I)=
     $  HADCUR(I)+CMPLX(FIX*COEF1)*FORM1*AA(I,J)*(PP(K,J)-PP(4,J))
 206   CONTINUE
C --- end of the non omega current (3 possibilities)
 201   CONTINUE
C
C
C --- there are two possibilities for omega current
C --- PA PB are corresponding first and second pi-'s
       DO 301 KK=1,2
        DO 302 I=1,4
         PA(I)=PP(KK,I)
         PB(I)=PP(3-KK,I)
 302    CONTINUE
C --- lorentz invariants
         QQA=0.0
         SS23=0.0
         SS24=0.0
         SS34=0.0
         QP1P2=0.0
         QP1P3=0.0
         QP1P4=0.0
         P1P2 =0.0
         P1P3 =0.0
         P1P4 =0.0
        DO 303 K=1,4
                     SIGN=-1.0
         IF (K.EQ.4) SIGN= 1.0
         QQA=QQA+SIGN*(PAA(K)-PA(K))**2
         SS23=SS23+SIGN*(PB(K)  +PIM3(K))**2
         SS24=SS24+SIGN*(PB(K)  +PIM4(K))**2
         SS34=SS34+SIGN*(PIM3(K)+PIM4(K))**2
         QP1P2=QP1P2+SIGN*(PAA(K)-PA(K))*PB(K)
         QP1P3=QP1P3+SIGN*(PAA(K)-PA(K))*PIM3(K)
         QP1P4=QP1P4+SIGN*(PAA(K)-PA(K))*PIM4(K)
         P1P2=P1P2+SIGN*PA(K)*PB(K)
         P1P3=P1P3+SIGN*PA(K)*PIM3(K)
         P1P4=P1P4+SIGN*PA(K)*PIM4(K)
 303    CONTINUE
C
        FORM2=COEF2*(BWIGN(QQ,AMRO,GAMRO)+ELPHA*BWIGN(QQ,AMROP,GAMROP))
C        FORM3=BWIGN(QQA,AMOM,GAMOM)*(BWIGN(SS23,AMRO,GAMRO)+
C     $        BWIGN(SS24,AMRO,GAMRO)+BWIGN(SS34,AMRO,GAMRO))
        FORM3=BWIGN(QQA,AMOM,GAMOM)
C
        DO 304 K=1,4
         HADCUR(K)=HADCUR(K)+FORM2*FORM3*(
     $             PB  (K)*(QP1P3*P1P4-QP1P4*P1P3)
     $            +PIM3(K)*(QP1P4*P1P2-QP1P2*P1P4)
     $            +PIM4(K)*(QP1P2*P1P3-QP1P3*P1P2) )
 304    CONTINUE
 301   CONTINUE
C
      ELSE
C ===================================================================
C pi0 pi0 p0 pi- case                                            ====
C ===================================================================
       QQ=PAA(4)**2-PAA(3)**2-PAA(2)**2-PAA(1)**2
       DO 101 K=1,3
C --- loop over thre contribution of the non-omega current
        SK=(PP(K,4)+PIM4(4))**2-(PP(K,3)+PIM4(3))**2
     $    -(PP(K,2)+PIM4(2))**2-(PP(K,1)+PIM4(1))**2
C -- definition of AA matrix
C -- cronecker delta
        DO 102 I=1,4
         DO 103 J=1,4
 103     AA(I,J)=0.0
 102    AA(I,I)=1.0
C
C ... and the rest ...
        DO 104 L=1,3
         IF (L.NE.K) THEN
          DENOM=(PAA(4)-PP(L,4))**2-(PAA(3)-PP(L,3))**2
     $         -(PAA(2)-PP(L,2))**2-(PAA(1)-PP(L,1))**2
          DO 105 I=1,4
          DO 105 J=1,4
                      SIG=1.0
           IF(J.NE.4) SIG=-SIG
           AA(I,J)=AA(I,J)
     $            -SIG*(PAA(I)-2.0*PP(L,I))*(PAA(J)-PP(L,J))/DENOM
 105      CONTINUE
         ENDIF
 104    CONTINUE
C --- let's add something to HADCURR
       FORM1= FPIKM(SQRT(SK),AMPI,AMPI) *FPIKM(SQRT(QQ),AMPI,AMPI)
C       FORM1= FPIKM(SQRT(SK),AMPI,AMPI) *FPIKMD(SQRT(QQ),AMPI,AMPI)
CCCCCCCCCCCCC       FORM1=WIGFOR(SK,AMRO,GAMRO)        (tests)
        DO 106 I=1,4
        DO 106 J=1,4
         HADCUR(I)=
     $   HADCUR(I)+CMPLX(COEF1)*FORM1*AA(I,J)*(PP(K,J)-PP(4,J))
 106    CONTINUE
C --- end of the non omega current (3 possibilities)
 101   CONTINUE
      ENDIF
      END
      FUNCTION WIGFOR(S,XM,XGAM)
      COMPLEX WIGFOR,WIGNOR
      WIGNOR=CMPLX(-XM**2,XM*XGAM)
      WIGFOR=WIGNOR/CMPLX(S-XM**2,XM*XGAM)
      END
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
