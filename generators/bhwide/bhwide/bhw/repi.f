      SUBROUTINE  vacpol(KeyPia,Q2,SINW2,RePiE,dRePiE)
!     ************************************************
!  Interface to various calculations of real part RePi
!  of cacuum polarization on the photon line.
!  Input:
!     Q2 = transfer GeV**2, negative for spacelike
!  Output:
!     RePiE  = Real Part Vacuum polarization on photon line
!              Leptonic + hadronic.
!     dRePiE = absolute error of RePiE
!     ***************************
      IMPLICIT REAL*8(A-H,O-Z)
      SAVE   
      REAL *4 enq,st2,der,errder,deg,errdeg

      QQ=Q2
      IF(KeyPia .EQ. 0) THEN
        RePiE  = 0d0
        dRePiE = 0d0
      ELSEIF(KeyPia .EQ. 1) THEN
! From Burkhardt 1981,1989
        RePiE  = REPI(QQ)
        dRePiE = 0d0
      ELSEIF(KeyPia .EQ. 2) THEN
! From S. Eidelman, F. Jegerlehner, Z. Phys. C (1995)
        enq= QQ/sqrt(abs(QQ))
        st2= SINW2
        CALL hadr5(enq,st2,der,errder,deg,errdeg)
! Leptonic part taken from Burkhardt
        RePiE  = -der + REPIL(QQ)  ! total
        dRePiE = errder
      ELSEIF(KeyPia .EQ. 3) THEN
! From Burkhardt and Pietrzyk 1995 (Moriond)
        RePiE  = REPI95(QQ)
        dRePiE = 0d0
      ELSE
        WRITE(6,*) 'STOP in VACPOL ++++ WRONG KeyPia=', KeyPia
        STOP
      ENDIF
      END

      FUNCTION REPIL(S)  
!-------------------------------------------- REMARKS ---------------
! VACUUM POLARIZATION IN QED. THE LEPTONIC CONTRIBUTION IS AN ANALY
! EXPRESSION INVOLVING THE LEPTON MASS; THE HADRONIC CONTRIBUTION IS
! A DISPERSION INTEGRAL OVER THE KNOWN HADRONIC CROSS SECTION. THE 
! RESULT USED HERE IS A PARAMETRIZATION GIVEN BY      
! H.BURKHARDT, TASSO NOTE 192(1981).      
! updated see H.Burkhardt et al. Pol. at Lep CERN 88-06 VOL I  
! lepton masses now fully taken into account, H.Burkhardt June 89  
!-------------------------------------------------------------------- 
      IMPLICIT REAL*8(A-H,O-Z)   
      COMPLEX*16 BETA   
      REAL*8 M(3)       
!  
      DATA A1,B1,C1/   0.0   ,   0.00835,  1.0   /    
      DATA A2,B2,C2/   0.0   ,   0.00238,  3.927 /    
      DATA A3,B3,C3/ 0.00165 ,   0.00300,  1.0   /    
      DATA A4,B4,C4/ 0.00221 ,   0.00293,  1.0   /    
!  
      DATA M/0.51099906D-3,0.10565839D0,1.7841D0/ 
    
      SAVE A1,B1,C1,A2,B2,C2,A3,B3,C3,A4,B4,C4,M
      SAVE I, AL3PI
!  
!     for leptons use F,P functions see Burgers, Hollik etc.   
!     F(s,m1,m2) for m1=m2 depends only on beta = sqrt((1-4m**2)/s) 
      FSYM(BETA)=2.D0+BETA*LOG( (BETA-1.D0)/(BETA+1.D0) )      
      P(S,XM,BETA)=1.D0/3.D0-(1.D0+2.D0*XM**2/S) * FSYM(BETA)  
!     asymptotic formula for high energies (real part)         
      PASYM(S,XM)=-5.D0/3.D0 - LOG (ABS(XM**2/S))     
!  
!---------------------------------- init and  CHECK FOR S VALUES ---- 
      DATA I/0/         
      IF(I.EQ.0) THEN   
        I=1    
        AL3PI=1./ (3.D0 * 137.0359895D0 * 3.141592653589793D0) 
        IF(S.GT.0.D0.AND.S.LT.100.D0)     
     .  WRITE(6,'(3H0S=,F6.3,7H GEV**2,/, 
     .    46H VACUUM POLARIZATION MAY BE BADLY APPROXIMATED)') 
      ENDIF    
!-------------------------------------------- LEPTONIC PART ---------
      REPIL=0.D0         
!     loop over leptons          
      DO 1 I=1,3        
        IF(ABS(S).GT.1.D3*M(I)**2) THEN   
!         asymptotic formula for s,t >> m**2          
          REPIL=REPIL-PASYM(S,M(I))         
        ELSE            
          BETA=1.D0-4.D0*M(I)**2/S        
          BETA=SQRT(BETA)        
          REPIL=REPIL-P(S,M(I),BETA)        
        ENDIF           
    1 CONTINUE          
      REPIL=AL3PI*REPIL   
      END

      FUNCTION REPI95(S)
C-------------------------------------------- REMARKS ---------------
C Vacuum polarization in QED. The leptonic contribution is an analytic
C expression involving the lepton mass; the hadronic contribution is fro
C a dispersion integral over the known hadronic cross section. The
C result used here is a Parametrization given by
C H.Burkhardt, TASSO note 192(1981).
C updated by H.Burkhardt, B.Pietrzyk; see H.Burkhardt in proceedings
C of electroweak Moriond 1995 and   Burkhardt, Pietrzyk to be published
C--------+---------+---------+---------+---------+---------+---------+--
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 BETA
      REAL*8 M(3)
      PARAMETER (NREG=5)
      DIMENSION WREG(0:NREG),AREG(NREG),BREG(NREG),CREG(NREG)
C
      DATA WREG/-0.4,-2.,-4.,-10.,91.2,100 000./
      DATA AREG/0.,0.,0.,0.00122270,0.00164178/
      DATA BREG/0.00228770,0.00251507,0.00279328,0.00296694,0.00292051/
      DATA CREG/4.08041425,3.09624477,2.07463133,1.0,1.0/
C
      DATA M/0.51099906D-3,0.10565839D0,1.7841D0/
C
C     for leptons use F,P functions see Burgers, Hollik etc.
C     F(s,m1,m2) for m1=m2 depends only on beta = sqrt((1-4m**2)/s)
      FSYM(BETA)=2.D0+BETA*LOG( (BETA-1.D0)/(BETA+1.D0) )
      P(S,XM,BETA)=1.D0/3.D0-(1.D0+2.D0*XM**2/S) * FSYM(BETA)
C     asymptotic formula for high energies (real part)
      PASYM(S,XM)=-5.D0/3.D0 - LOG (ABS(XM**2/S))
C
C---------------------------------- init and  CHECK FOR S VALUES ----
      DATA I/0/
      IF(I.EQ.0) THEN
        I=1
        AL3PI=1./ (3.D0 * 137.0359895D0 * 3.141592653589793D0)
        IF(S.GT.0.D0.AND.S.LT.100.D0)
     .  WRITE(6,'(3H0S=,F6.3,7H GeV**2,/,
     .    46H vacuum polarization may be badly approximated)') S
      ENDIF
C-------------------------------------------- LEPTONIC PART ---------
      REPI95=0.D0
C     loop over leptons
      DO I=1,3
        IF(ABS(S).GT.1.D3*M(I)**2) THEN
C         asymptotic formula for s,t >> m**2
          REPI95=REPI95-PASYM(S,M(I))
        ELSE
          BETA=1.D0-4.D0*M(I)**2/S
          BETA=SQRT(BETA)
          REPI95=REPI95-P(S,M(I),BETA)
        ENDIF
      ENDDO
      REPI95=AL3PI*REPI95
C-------------------------------------------- hadronic part ---------
      T=ABS(S)
      DO I=1,NREG
        IREG=I
        IF(T.LT.WREG(I)**2) GOTO 1
      ENDDO
    1 CONTINUE
      HADPI=AREG(IREG)+BREG(IREG)*LOG(1.+CREG(IREG)*T)
C     WRITE(6,*) ' SQRTS=',SQRT(T),' leptonic part=',REPI95,
C    .  ' hadronic part=',HADPI,' total=',REPI95-HADPI
      REPI95=REPI95-HADPI
      END

      FUNCTION REPI(S)  
!-------------------------------------------- REMARKS ---------------
! VACUUM POLARIZATION IN QED. THE LEPTONIC CONTRIBUTION IS AN ANALY
! EXPRESSION INVOLVING THE LEPTON MASS; THE HADRONIC CONTRIBUTION IS
! A DISPERSION INTEGRAL OVER THE KNOWN HADRONIC CROSS SECTION. THE 
! RESULT USED HERE IS A PARAMETRIZATION GIVEN BY      
! H.BURKHARDT, TASSO NOTE 192(1981).      
! updated see H.Burkhardt et al. Pol. at Lep CERN 88-06 VOL I  
! lepton masses now fully taken into account, H.Burkhardt June 89  
!-------------------------------------------------------------------- 
      IMPLICIT REAL*8(A-H,O-Z)   
      COMPLEX*16 BETA   
      REAL*8 M(3)       
!  
      DATA A1,B1,C1/   0.0   ,   0.00835,  1.0   /    
      DATA A2,B2,C2/   0.0   ,   0.00238,  3.927 /    
      DATA A3,B3,C3/ 0.00165 ,   0.00300,  1.0   /    
      DATA A4,B4,C4/ 0.00221 ,   0.00293,  1.0   /    
!  
      DATA M/0.51099906D-3,0.10565839D0,1.7841D0/ 
    
      SAVE A1,B1,C1,A2,B2,C2,A3,B3,C3,A4,B4,C4,M
      SAVE I, AL3PI
!  
!     for leptons use F,P functions see Burgers, Hollik etc.   
!     F(s,m1,m2) for m1=m2 depends only on beta = sqrt((1-4m**2)/s) 
      FSYM(BETA)=2.D0+BETA*LOG( (BETA-1.D0)/(BETA+1.D0) )      
      P(S,XM,BETA)=1.D0/3.D0-(1.D0+2.D0*XM**2/S) * FSYM(BETA)  
!     asymptotic formula for high energies (real part)         
      PASYM(S,XM)=-5.D0/3.D0 - LOG (ABS(XM**2/S))     
!  
!---------------------------------- init and  CHECK FOR S VALUES ---- 
      DATA I/0/         
      IF(I.EQ.0) THEN   
        I=1    
        AL3PI=1./ (3.D0 * 137.0359895D0 * 3.141592653589793D0) 
        IF(S.GT.0.D0.AND.S.LT.100.D0)     
     .  WRITE(6,'(3H0S=,F6.3,7H GEV**2,/, 
     .    46H VACUUM POLARIZATION MAY BE BADLY APPROXIMATED)') 
      ENDIF    
!-------------------------------------------- LEPTONIC PART ---------
      REPI=0.D0         
!     loop over leptons          
      DO 1 I=1,3        
        IF(ABS(S).GT.1.D3*M(I)**2) THEN   
!         asymptotic formula for s,t >> m**2          
          REPI=REPI-PASYM(S,M(I))         
        ELSE            
          BETA=1.D0-4.D0*M(I)**2/S        
          BETA=SQRT(BETA)        
          REPI=REPI-P(S,M(I),BETA)        
        ENDIF           
    1 CONTINUE          
      REPI=AL3PI*REPI   
!-------------------------------------------- HADRONIC PART ---------
      X=DABS(S)         
      IF(X.LT.0.3**2) THEN       
        REPI=REPI- (A1+B1*LOG(1.+C1*X))   
      ELSEIF(X.LT.3.**2) THEN    
        REPI=REPI- (A2+B2*LOG(1.+C2*X))   
      ELSEIF(X.LT.100.**2) THEN  
        REPI=REPI- (A3+B3*LOG(1.+C3*X))   
      ELSE  
        REPI=REPI- (A4+B4*LOG(1.+C4*X))   
      ENDIF    
      END  

!Return-Path: <jegerlehner@PSICLU.CERN.CH>
!Date: Mon, 20 Mar 1995 10:30:49 +0100
!From: jegerlehner@psiclu.cern.ch (Fred Jegerlehner)
!X-Vms-To: DXMINT::jadach@cernvm.cern.ch
!X-Vms-Cc: JEGERLEHNER
!Subject: Re: hadronic vacuum polarization
!X-Mail11-Ostype: VAX/VMS
!Apparently-To: <jadach@cernvm.cern.ch>
!Dear Staszek,
!the uncertainty of course depends on the energy and goes to zero as E -> 0.
!The routine I have sent you gives you the uncertainty for a given energy.
!At 1 GeV spacelike I get
!     E        delta alpha   uncertainty
! -1.0000E+00   3.7362E-03   9.2977E-05
!A copy of our paper you may get by anonymous ftp as follows:
!ftp 129.129.40.58
!anonymous
!username
!cd pub
!cd preprints
!get vapogm2.ps.gz

       subroutine hadr5(e,st2,der,errder,deg,errdeg)
c ******************************************************************
c *                                                                *
c *      subroutine for the evaluation of the light hadron         *
c *           contributions to Delta_r  and  Delta_g               *
c *                    using fits to the                           *
c *          QED vacuum polarization from e^+ e^- data             *
c *                                                                *
c *    F. Jegerlehner, Paul Scherrer Institute, CH-5232 Villigen   *
c *                                                                *
c *    E-mail:jegerlehner@cvax.psi.ch                              *
c *    Phone :   +41-56-993662                                     *
c *                                                                *
c *    Reference: F. Jegerlehner, Z. Phys. C32 (1986) 195          *
c *               H. Burkhardt et al., Z. Phys. C42 (1989) 497     *
c *               S. Eidelman, F. Jegerlehner, Z. Phys. C (1995)   *
c *                                                                *
c ******************************************************************
c       VERSION: 24/02/1995
c
C  Notation: E energy ( momentum transfer ): E>0 timelike , E<0 spacelike
C            st2 is sin^2(Theta); st2=0.2322 is the reference value
C  the routine returns the hadronic contribution of 5 flavors (u,d,s,c,b)
C                 to   DER=Delta_r with hadronic error ERRDER
C                and   DEG=Delta_g with hadronic error ERRDEG
C  The effective value of the fine structure constant alphaQED at energy
C  E is alphaQED(E)=alphaQED(0)/(1-Delta_r) ,similarly for the SU(2)
C  coupling alphaSU2(E)=alphaSU2(0)/(1-Delta_g), where Delta_r(g) is the
C  sum of leptonic, hadronic contributions (top to be added).
C
C  This program does not yet know how to compute Delta r and Delta g for
C  energies in the ranges  |E|>1TeV and 2m_pi < E < 40(13) GeV !!!!!!!!!
C
       implicit none
       integer *2 nf,ns,i,j
       parameter(nf=9,ns=4)
       real *4 e,st2,st20,der,deg,errder,errdeg,s,s0,x1,xi,x2,xlog,xlar
       real *4 m1(nf),c1(nf,ns),c2(nf,ns),c3(nf,ns),c4(nf,ns),ae(nf,ns)
       real *4 eu(nf),eo(nf),res(ns),l1(nf,ns),fx,gx,hx,xx,u,Se
       do i=1,nf
         do j=1,ns
           ae(i,j)=0.0
         enddo
       enddo
c #1# Delta_r
c Fit parameters spacelike  -1000 to  -200 GeV
      eu(1)  =-1000.
      eo(1)  = -200.
      m1(1)=  -1000.000
      c1(1,1)=  4.2069394e-02
      c2(1,1)=  2.9253566e-03
      c3(1,1)= -6.7782454e-04
      c4(1,1)=  9.3214130e-06
c   chi2=  2.5763808e-05
c Fit parameters spacelike  -200 to  -20 GeV
      eu(2)  = -200.
      eo(2)  =  -20.
      m1(2)  =  -100.0000
      c1(2,1)=  2.8526291e-02
      c2(2,1)=  2.9520725e-03
      c3(2,1)= -2.7906310e-03
      c4(2,1)=  6.4174528e-05
c   chi2=  6.6264300e-04
c Fit parameters spacelike   -20 to   -2 GeV
      eu(3)  =  -20.
      eo(3)  =   -2.
      m1(3)  =   -20.0000
      l1(3,1)=  9.3055e-3
      c1(3,1)=  2.8668314e-03
      c2(3,1)=  0.3514608
      c3(3,1)=  0.5496359
      c4(3,1)=  1.9892334e-04
c   chi2=  4.2017717e-03
      ae(3,1)=  3.0
c Fit parameters spacelike    -2 to    0.25 GeV
      eu(4)  =   -2.
      eo(4)  =    0.25
      m1(4)  =    -2.0000
      l1(4,1)=  9.3055e-3
      c1(4,1)=  2.2694240e-03
      c2(4,1)=   8.073429
      c3(4,1)=  0.1636393
      c4(4,1)= -3.3545541e-05
c   chi2=  0.1239052
      ae(4,1)=  2.0
c Fit parameters timelike   0.25 to    2 GeV
      eu(5)  =    0.25
      eo(5)  =    2.
c Fit parameters timelike   2    to   40 GeV
      eu(6)  =    2.
      eo(6)  =   40.
c Fit parameters timelike     40 to   80 GeV
      eu(7)  =   40.
      eo(7)  =   80.
      m1(7)  =   80.00000
      c1(7,1)=  2.7266588e-02
      c2(7,1)=  2.9285045e-03
      c3(7,1)= -4.7720564e-03
      c4(7,1)=  7.7295507e-04
c   chi2=  7.7148885e-05
c Fit parameters timelike     80 to  250 GeV
      eu(8)  =   80.
      eo(8)  =  250.
      m1(8)  =   91.18880
      c1(8,1)=  2.8039809e-02
      c2(8,1)=  2.9373798e-03
      c3(8,1)= -2.8432352e-03
      c4(8,1)= -5.2537734e-04
c   chi2=  4.2241514e-05
c Fit parameters timelike    250 to 1000 GeV
      eu(9)  =  250.
      eo(9)  = 1000.
      m1(9)  = 1000.00000
      c1(9,1)=  4.2092260e-02
      c2(9,1)=  2.9233438e-03
      c3(9,1)= -3.2966913e-04
      c4(9,1)=  3.4324117e-07
c   chi2=  6.0426464e-05
c #2# Delta_g
c Fit parameters spacelike  -1000 to  -200 GeV
c     eu(1)  =-1000.
c     eo(1)  = -200.
c     m1(1)=  -1000.000
      c1(1,2)=  8.6415343e-02
      c2(1,2)=  6.0127582e-03
      c3(1,2)= -6.7379221e-04
      c4(1,2)=  9.0877611e-06
c   chi2=  9.6284139e-06
c Fit parameters spacelike  -200 to  -20 GeV
c     eu(2)  = -200.
c     eo(2)  =  -20.
c     m1(2)  =  -100.0000
      c1(2,2)=  5.8580618e-02
      c2(2,2)=  6.0678599e-03
      c3(2,2)= -2.4153464e-03
      c4(2,2)=  6.1934326e-05
c   chi2=  6.3297758e-04
c Fit parameters spacelike   -20 to   -2 GeV
c     eu(3)  =  -20.
c     eo(3)  =   -2.
c     m1(3)  =   -20.0000
      l1(3,2)=  1.9954e-2
      c1(3,2)=  5.7231588e-03
      c2(3,2)=  0.3588257
      c3(3,2)=  0.5532265
      c4(3,2)=  6.0730567e-04
c   chi2=  7.9884287e-03
      ae(3,2)=  3.0
c   chi2=  4.2017717e-03
c Fit parameters spacelike    -2 to    0.25 GeV
c     eu(4)  =   -2.
c     eo(4)  =    0.25
c     m1(4)  =    -2.0000
      l1(4,2)=  1.9954e-2
      c1(4,2)=  4.8065037e-03
      c2(4,2)=   8.255167
      c3(4,2)=  0.1599882
      c4(4,2)= -1.8624817e-04
c   chi2=  0.1900761
      ae(3,2)=  2.0
c Fit parameters timelike     40 to   80 GeV
c     eu(7)  =   40.
c     eo(7)  =   80.
c     m1(7)  =   80.00000
      c1(7,2)=  5.5985276e-02
      c2(7,2)=  6.0203830e-03
      c3(7,2)= -5.0066952e-03
      c4(7,2)=  7.1363564e-04
c   chi2=  7.6000040e-05
c Fit parameters timelike     80 to  250 GeV
c     eu(8)  =   80.
c     eo(8)  =  250.
c     m1(8)  =   91.18880
      c1(8,2)=  5.7575710e-02
      c2(8,2)=  6.0372148e-03
      c3(8,2)= -3.4556778e-03
      c4(8,2)= -4.9574347e-04
c   chi2=  3.3244669e-05
c Fit parameters timelike    250 to 1000 GeV
c     eu(9)  =  250.
c     eo(9)  = 1000.
c     m1(9)  = 1000.00000
      c1(9,2)=  8.6462371e-02
      c2(9,2)=  6.0088057e-03
      c3(9,2)= -3.3235471e-04
      c4(9,2)=  5.9021050e-07
c   chi2=  2.9821187e-05
c #3# error Delta_r
c Fit parameters spacelike  -1000 to  -200 GeV
c     eu(1)  =-1000.
c     eo(1)  = -200.
c     m1(1)=  -1000.000
      c1(1,3)=  6.3289929e-04
      c2(1,3)=  3.3592437e-06
      c3(1,3)=  0.0
      c4(1,3)=  0.0
c   chi2=  2.3007713E-05
c Fit parameters spacelike  -200 to  -20 GeV
c     eu(2)  = -200.
c     eo(2)  =  -20.
c     m1(2)  =  -100.0000
      c1(2,3)=  6.2759849e-04
      c2(2,3)= -1.0816625e-06
      c3(2,3)=   5.050189
      c4(2,3)= -9.6505374e-02
c   chi2=  3.4677869e-04
      ae(2,3)=  1.0
c Fit parameters spacelike   -20 to   -2 GeV
c     eu(3)  =  -20.
c     eo(3)  =   -2.
c     m1(3)  =   -20.0000
      l1(3,3)=  2.0243e-4
      c1(3,3)=  1.0147886e-04
      c2(3,3)=   1.819327
      c3(3,3)= -0.1174904
      c4(3,3)= -1.2404939e-04
c   chi2=  7.1917898e-03
      ae(3,3)=  3.0
c Fit parameters spacelike    -2 to    0.25 GeV
c     eu(4)  =   -2.
c     eo(4)  =    0.25
c     m1(4)  =    -2.0000
      l1(4,3)=  2.0243e-4
      c1(4,3)= -7.1368617e-05
      c2(4,3)=  9.980347e-04
      c3(4,3)=   1.669151
      c4(4,3)=  3.5645600e-05
c   chi2=  0.1939734
      ae(4,3)=  2.0
c Fit parameters timelike     40 to   80 GeV
c     eu(7)  =   40.
c     eo(7)  =   80.
c     m1(7)  =   80.00000
      c1(7,3)=  6.4947648e-04
      c2(7,3)=  4.9386853e-07
      c3(7,3)=  -55.22332
      c4(7,3)=   26.13011
c   chi2=  7.2068366e-04
c Fit parameters timelike     80 to  250 GeV
c     eu(8)  =   80.
c     eo(8)  =  250.
c     m1(8)  =   91.18880
      c1(8,3)=  6.4265809e-04
      c2(8,3)= -2.8453374e-07
      c3(8,3)=  -23.38172
      c4(8,3)=  -6.251794
c   chi2=  1.1478480e-07
c Fit parameters timelike    250 to 1000 GeV
c     eu(9)  =  250.
c     eo(9)  = 1000.
c     m1(9)  = 1000.00000
      c1(9,3)=  6.3369947e-04
      c2(9,3)= -2.0898329e-07
      c3(9,3)=  0.0
      c4(9,3)=  0.0
c   chi2=  2.9124376E-06
c #4# error Delta_g
c Fit parameters spacelike  -1000 to  -200 GeV
c     eu(1)  =-1000.
c     eo(1)  = -200.
c     m1(1)=  -1000.000
      c1(1,4)=  1.2999176e-03
      c2(1,4)=  7.4505529e-06
      c3(1,4)=  0.0
      c4(1,4)=  0.0
c   chi2=  2.5312527E-05
c Fit parameters spacelike  -200 to  -20 GeV
c     eu(2)  = -200.
c     eo(2)  =  -20.
c     m1(2)  =  -100.0000
      c1(2,4)=  1.2883141e-03
      c2(2,4)= -1.3790827e-06
      c3(2,4)=   8.056159
      c4(2,4)= -0.1536313
c   chi2=  2.9774895e-04
      ae(2,4)=  1.0
c Fit parameters spacelike   -20 to   -2 GeV
c     eu(3)  =  -20.
c     eo(3)  =   -2.
c     m1(3)  =   -20.0000
      l1(3,4)=  4.3408e-4
      c1(3,4)=  2.0489733e-04
      c2(3,4)=   2.065011
      c3(3,4)= -0.6172962
      c4(3,4)= -2.5603661e-04
c   chi2=  7.5258738e-03
      ae(3,4)=  3.0
c Fit parameters spacelike    -2 to    0.25 GeV
c     eu(4)  =   -2.
c     eo(4)  =    0.25
c     m1(4)  =    -2.0000
      l1(4,4)=  4.3408e-4
      c1(4,4)= -1.5095409e-04
      c2(4,4)=  9.9847501e-04
      c3(4,4)=   1.636659
      c4(4,4)=  7.5892596e-05
c   chi2=  0.1959371
      ae(4,4)=  2.0
c Fit parameters timelike     40 to   80 GeV
c     eu(7)  =   40.
c     eo(7)  =   80.
c     m1(7)  =   80.00000
      c1(7,4)=  1.3335156e-03
      c2(7,4)=  2.2939612e-07
      c3(7,4)=  -246.4966
      c4(7,4)=   114.9956
c   chi2=  7.2293193e-04
c Fit parameters timelike     80 to  250 GeV
c     eu(8)  =   80.
c     eo(8)  =  250.
c     m1(8)  =   91.18880
      c1(8,4)=  1.3196438e-03
      c2(8,4)=  2.8937683e-09
      c3(8,4)=   5449.778
      c4(8,4)=   930.3875
c   chi2=  4.2109136e-08
c Fit parameters timelike    250 to 1000 GeV
c     eu(9)  =  250.
c     eo(9)  = 1000.
c     m1(9)  = 1000.00000
      c1(9,4)=  1.3016918e-03
      c2(9,4)= -3.6027674e-07
      c3(9,4)=  0.0
      c4(9,4)=  0.0
c   chi2=  2.8220852E-06
C ######################################################################
       Se=654./643.      ! rescaling error to published version 1995
       st20=0.2322
       s=e**2
       der=0.0
       deg=0.0
       errder=0.0
       errdeg=0.0
       if ((e.gt.1.e3).or.(e.lt.-1.e3)) goto 100
       if ((e.lt.4.e1).and.(e.gt.0.25)) goto 100
       i=1
       do while (e.ge.eo(i))
         i=i+1
       enddo
       if (e.eq.1.e3) i=9
       if (e.eq.0.0 ) goto 100
       s0=sign(1.0,m1(i))*m1(i)**2
       s =sign(1.0,e)*e**2
       x1=s0/s
       xi=1.0/x1
       x2=x1**2
       if (ae(i,1).le.0.0) then
         do j=1,4
           xlar=xi+ae(i,j)*exp(-xi)
           xlog=log(xlar)
           res(j)=c1(i,j)
     .           +c2(i,j)*(xlog+c3(i,j)*(x1-1.0)+c4(i,j)*(x2-1.0))
         enddo
       else if (ae(i,1).eq.2.0) then
         hx     =xi**2
         do j=1,2
           fx     =1.0-c2(i,j)*s
           gx     = c3(i,j)*s/(c3(i,j)-s)
           xx     =log(abs(fx))+c2(i,j)*gx
           res(j)=c1(i,j)*xx-l1(i,j)*gx+c4(i,j)*hx
         enddo
         do j=3,4
           u      =abs(s)
           gx     =-c3(i,j)*u/(c3(i,j)+u)
           xx     =xi**3/(sqrt(abs(xi))**5+c2(i,j))
           res(j)=c1(i,j)*xx-l1(i,j)*gx+c4(i,j)*hx
         enddo
       else if (ae(i,1).eq.3.0) then
         hx     =xi
         do j=1,4
           fx     =1.0-c2(i,j)*s
           gx     = c3(i,j)*s/(c3(i,j)-s)
           xx     =log(abs(fx))+c2(i,j)*gx
           res(j)=c1(i,j)*xx-l1(i,j)*gx+c4(i,j)*hx
         enddo
       endif
       der=res(1)
       deg=res(2)*st20/st2
       errder=res(3)*Se
       errdeg=res(4)*Se
       goto 100
 99    write(*,*) ' out of range! '
100    return
       end
