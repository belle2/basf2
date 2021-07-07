
CDECK  ID>, T4BODY. 
*.
*...T4BODY   generates an e-e-gamma-gamma event.
*.
*. SEQUENCE  : TEGCOM
*. CALLS     : RNDGEN TBOORT TPRD BEEGGC MEEGGC
*. CALLED    : TEEGG7
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.2
*. CREATED   : 29-Jun-87
*. LAST MOD  : 10-Apr-89
*.
*. Modification Log.
*. 28-Sep-88   Dean Karlen   Change to conform to OPAL standards.
*. 10-Apr-89   Dean Karlen   Handle the peak in phi-ks properly
*. 21-May-89   Dean Karlen   remove recalculation of CPHKKS (IBM problem
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C SUBROUTINE T4BODY: Generates an e-e-gamma-gamma event.
C-----------------------------------------------------------------------

      SUBROUTINE T4BODY
      IMPLICIT NONE


C Version 7.2 - Common/Include file.

C Fundamental constants
      DOUBLE PRECISION ALPHA,ALPHA3,ALPHA4,M,PBARN,PI,TWOPI

      PARAMETER
     >(          ALPHA = 1.D0/137.036D0, ALPHA3=ALPHA**3,ALPHA4=ALPHA**4
     >,          M     = 0.5110034 D-3
     >,          PBARN = .389386 D9
     >,          PI    = 3.14159265358979 D0 , TWOPI=2.0D0*PI
     >)

C M     mass of the electron in GeV
C PBARN conversion of GeV-2 to pb

C Other constants

      INTEGER   HARD,SOFT,NONE    , EGAMMA,GAMMA,ETRON,GAMMAE
     >,         BK,BKM2,TCHAN,EPA , EPADC,BEEGG,MEEGG,HEEGG
      PARAMETER
     >(          HARD  = 1 , EGAMMA = 11 , BK    = 21 , EPADC = 31
     >,          SOFT  = 2 , ETRON  = 12 , BKM2  = 22 , BEEGG = 32
     >,          NONE  = 3 , GAMMA  = 13 , TCHAN = 23 , MEEGG = 33
     >,                      GAMMAE = 14 , EPA   = 24 , HEEGG = 34
     >)

C HARD generate e+ e- gamma gamma
C SOFT generate e+ e- gamma with soft and virt corrections
C NONE generate e+ e- gamma according to lowest order only
C EGAMMA e-gamma configuration
C ETRON  single electron configuration
C GAMMA  single gamma configuration
C GAMMAE single gamma configuration (modified for low Ee 4th order only)
C BK    Berends and Kleiss matrix element
C BKM2  Berends and Kleiss matrix element with m**2/t term
C TCHAN t channel matrix element only (two diagrams)
C EPA   equivalent photon approximation matrix element (for testing)
C EPADC EPA matrix element with double Compton (for RADCOR=HARD only)
C BEEGG Berends et al. e-e-gamma-gamma m.e.    (for RADCOR=HARD only)
C MEEGG Martinez/Miquel e-e-gamma-gamma m.e.   (for RADCOR=HARD only)
C HEEGG Hybrid of EPADC and BEEGG              (for RADCOR=HARD only)

C Input parameters common:

      DOUBLE PRECISION EB,TEVETO,TEMIN,TGMIN,TGVETO,EEMIN,EGMIN
     >,                PEGMIN,EEVETO,EGVETO,PHVETO,CUTOFF,EPS
     >,                WGHT1M,WGHTMX, FRAPHI,EPSPHI
      INTEGER          ISEED, RADCOR,CONFIG,MATRIX,MTRXGG
      LOGICAL          UNWGHT

      COMMON/TINPAR/EB,TEVETO,TEMIN,TGMIN,TGVETO,EEMIN,EGMIN
     >,             PEGMIN,EEVETO,EGVETO,PHVETO,CUTOFF,EPS
     >,             WGHT1M,WGHTMX, FRAPHI,EPSPHI
     >,             ISEED, RADCOR,CONFIG,MATRIX,MTRXGG, UNWGHT

C EB     = energy of electron beam in GeV
C TEVETO = maximum theta of e+ in final state (in radians)
C TEMIN  = minimum angle between the e- and -z axis (egamma conf. only)
C TGMIN  = minimum angle between the gamma and -z axis
C TGVETO = maximum angle between the gamma and -z axis(etron conf. only)
C EEMIN  = minimum energy of the e- (egamma & etron configurations)
C EGMIN  = minimum energy of the gamma (egamma & gamma configurations)
C PEGMIN = minimum phi sep of e-gamma (egamma config with hard rad corr)
C EEVETO = minimum energy to veto(gamma  config with hard rad corr)
C EGVETO = minimum energy to veto(etron/gamma config with hard rad corr)
C PHVETO = minimum phi sep to veto(etron/gamma config with hard rad corr
C CUTOFF = cutoff energy for radiative corrections (in CM frame)
C EPS    = param. epsilon_s (smaller val. increases sampling of k_s^pbc)
C FRAPHI = fraction of time phi_ks is generated with peak(hard rad corr)
C EPSPHI = param. epsilon_phi ('cutoff' of the phi_ks peak)
C WGHT1M = maximum weight for generation of QP0, cos(theta QP)
C WGHTMX = maximum weight for the trial events
C ISEED  = initial seed
C RADCOR = specifies radiative correction (NONE SOFT or HARD)
C CONFIG = specifies the event configuration (EGAMMA GAMMA or ETRON)
C MATRIX = specifies which eeg matrix element (BK BKM2 TCHAN or EPA)
C MTRXGG = specifies which eegg matrix element (EPADC BEEGG or MEEGG)
C UNWGHT = logical variable. If true then generate unweighted events.

C Variable array sizes

      INTEGER    NRNDMX
      PARAMETER (NRNDMX=20)

C NRNDMX maximum number of random numbers that may be generated at once.

C Common for random number generator

      DOUBLE PRECISION        RND(NRNDMX) ! changed from real to double (TF)
      INTEGER         SEED,NXSEED,BSEED
      COMMON/TRND/RND,SEED,NXSEED,BSEED

C RND    = random numbers used for an event.
C SEED   = seed that generates the sequence of random numbers coming up.
C NXSEED = next seed that should be used if the sequence is to continue.
C BSEED  = beginning seed (same as ISEED except for multiple runs)

C Derived constants common

      DOUBLE PRECISION
     >       S,SQRTS,EBP,EPSLON,CDELT,CDELT1,CTGVT1,ACTEM,ACTK
     >,      FQPMAX,QP0MIN,ZMAX,LOGZ0M,LOGRSM,FACT3,FACT7,CTGM1M,ASOFTC
      COMMON/TCONST/
     >       S,SQRTS,EBP,EPSLON,CDELT,CDELT1,CTGVT1,ACTEM,ACTK
     >,      FQPMAX,QP0MIN,ZMAX,LOGZ0M,LOGRSM,FACT3,FACT7,CTGM1M,ASOFTC

C Calculated event quantities.

      DOUBLE PRECISION P(16),RSIGN
      COMMON/TEVENT/P,RSIGN
      DOUBLE PRECISION T,TP,SP,U,UP,X1,X2,Y1,Y2,DSIGE,WGHT,WGHTSF
      COMMON/TEVQUA/T,TP,SP,U,UP,X1,X2,Y1,Y2,DSIGE,WGHT,WGHTSF

C P(16) = 4 four-vectors. (e+ e- gamma1 gamma2) (px,py,pz,E)
C RSIGN = indicates the charge that of the 'missing electron'.
C S,SP,T,TP,U,UP,X1,X2,Y1,Y2 = Berends/Kliess invariant products.
C DSIGE = calculated differential cross section
C WGHT  = weight (dsige/dsiga)
C WGHTSF= weight (1+delta)

C Event summary quantities

      DOUBLE PRECISION EFFIC,SIGE,ERSIGE,W1MAX,WMAX,WMINSF,Q2W2MX
     >,                SUMW1,SUMW12,SUMWGT,SUMW2,CONVER
      INTEGER       NTRIAL,NPASSQ,NACC
      COMMON/TSMMRY/EFFIC,SIGE,ERSIGE,W1MAX,WMAX,WMINSF,Q2W2MX
     >,             SUMW1,SUMW12,SUMWGT,SUMW2,CONVER
     >,             NTRIAL,NPASSQ,NACC

C EFFIC = efficiency of event generation = NACC/NTRIAL
C SIGE  = total cross section (pb)
C ERSIGE= error in the total cross section
C W1MAX = maximum observed weight for q+0 and cos(0q+) generation
C WMAX  = maximum event weight observed
C WMINSF= minimum soft correction weight observed
C Q2W2MX= maximum ratio of Q**2 to W**2  observed
C SUMW1 = sum of all weights of QP generation
C SUMW12= sum of all (weights of QP generation)**2
C SUMWGT= sum of all weights
C SUMW2 = sum of all (weights)**2
C CONVER= conversion factor of SUMWGT to (pb)
C NTRIAL= number of trials so far
C NPASSQ= number of times qp accepted
C NACC  = number of accepted events so far


C Variables
      DOUBLE PRECISION
     >       ZP,Z0,FQPP,FQP,QP0,WGHT1,RS,W2,SQRTSC,SC,PBC,EBC
     >,      COSQP1,SINQP,COSK1M,COSK1,SINK,PHIK,PHIP
     >,      BETA1,BETA,GAMA,KCK,ATERM,KXPKC,KYPKC,KZPKC
     >,      XITERM,SINTR,COSTR,KXCKC,KYCKC,KZCKC,SINKC,PHIKC
     >,      KS,CSKS1P,SINKSP,CSKS1Q,PHIKSP,KC,QM0C,QMPC
     >,      CSKS1K,SINKSK,PHIKSK,KXYCKC,CPKSK,SPKSK,KSXKS,KSYKS,KSZKS
     >,      QPP0,QM0,QMP0,K0,KSL,COSQM,COSQM1,COSKSL,CSKS1L,CSK1CQ
     >,      DELTQ,PHIEL,PHIKL,PHIKSL,SGNPHI,XPHIKS,PHIKSX
     >,      KAP1,KAP2,KAP3,KAP1P,KAP2P,KAP3P,A,B,C,X,Z,UA,UB,RO,XTERM
     >,      DSIGA,TE,BPMK1,BPMK2,BQMK1,BQMK2,BK1K2
     >,      S1,S2,T1,T2,U1,U2,PPK1K2,PMK1K2,QPK1K2,QMK1K2
     >,      K1P,K1M,K2P,K2M,QPP,QPM,QMP,QMM
     >,      XPHIP1,XPHIP2,XPHIK1,XPHIK2,CPHKSK,CPHKKS
     >,      KZCKC1,COSTR1,KZPKC1,CSKS2P

      DOUBLE PRECISION
     >       QMC4(4),GAMC4(4),GAMSC4(4),QPL4(4),QML4(4)
     >,      GAML4(4),GAMSL4(4)

      INTEGER   IMISS,IOBSV
      LOGICAL LKSAXZ,LPKPHI,LPASS,LSWAP
      LOGICAL LAE,LAG,LAGS,LAEG,LAEGS,LRE,LRG,LRGS,LREG,LREGS,LRGGS

      DOUBLE PRECISION HQ2MIN
      PARAMETER (HQ2MIN = 1.D2 * M**2)     
      
      
C     TF, 2015
      INTEGER TNPHOT
      DOUBLE PRECISION TP1(0:3),TQ1(0:3),TP2(0:3),TQ2(0:3),TPHOT(0:1,0:3)
      COMMON / MOMSET / TP1,TQ1,TP2,TQ2,TPHOT,TNPHOT
      
      COMMON / KINEMATICS / W2

C Externals
      DOUBLE PRECISION
     >         TPRD,BEEGGC,MEEGGC
      EXTERNAL TPRD,BEEGGC,MEEGGC

      DOUBLE PRECISION PHISEP,PHI1,PHI2,XPHI,DXPHI,XPHI1,TASQRT,DUM
      PHISEP(PHI1,PHI2)=MIN(ABS(PHI1 - PHI2) , TWOPI - ABS(PHI1 - PHI2))
      XPHI(PHI1)=(PI-ABS(MOD(ABS(PHI1),TWOPI)-PI))/PI
      DXPHI(XPHI1)=1.D0 - FRAPHI +
     >             FRAPHI/LOG(1.D0+1.D0/EPSPHI)/(XPHI1+EPSPHI)
      TASQRT(DUM)=DUM/2.D0+DUM**2/8.D0+DUM**3/16.D0+5.D0*DUM**4/128.D0
     >            +7.D0*DUM**5/256.D0

C The starting point for a new trial
C ----------------------------------

 1    CONTINUE

C #1 Generate QP0
C Do until trial QP0 COSQP1 is taken

C Increment the trial event counter here. The calculation of the total
C cross section depends on this quantity.
         NTRIAL=NTRIAL+1

C Generate all random numbers needed here.
!          CALL RNDGEN(12)
         call teegg_rndmarr(RND,12)
!          write (*,*) RND(1), RND(2), RND(3), RND(4), RND(5), RND(6), RND(7), RND(8), RND(9), RND(10), RND(11), RND(12)

         ZP=RND(1)*ZMAX

C FQPP=EB-QP ; FQP=EB-QP0
         FQPP=EB/(1.D0+ZP)
         FQP =FQPP - EPSLON*EB*(1.D0+ZP)/ZP
         QP0=EB-FQP
         Z0 = QP0/FQP
         
C Require QP0 > QP0min
         IF(QP0.LT.QP0MIN)GOTO 1

C Some handy 'constants'

         W2 = 2.D0*SQRTS*FQP + M**2
         PBC= SQRT(W2-2.D0*M**2)/2.D0
         EBC= SQRT(PBC**2+M**2)
         SQRTSC=SQRT(W2)
         SC = W2

C Assign a weight for this trial QP0

         RS=(PBC*(1.D0+EPS)-CUTOFF)/(EPS*CUTOFF)
         WGHT1=(FQPP/FQP)**2 * LOG(1.D0+CDELT1/EPSLON*Z0**2)*LOG(RS)/
     >                       ( LOGZ0M * LOGRSM )

C Keep track of all weights, to calculate the total cross section.
         SUMW1 = SUMW1 + WGHT1
         SUMW12= SUMW12+ WGHT1**2

! 	  write(*,*) "-->", WGHT1, RND(2)*WGHT1M

                  
      IF(WGHT1.LT.RND(2)*WGHT1M)GOTO 1

C A q+0 has been accepted. Keep track of number that pass qp generation
      NPASSQ = NPASSQ + 1

C #2 Generate COS(theta QP0)

C COSQP1 = 1 - COS(theta QP0)
      COSQP1 = EPSLON/Z0**2*( (Z0**2/EPSLON*CDELT1+1.D0)**RND(3) -1.D0)
      SINQP  = SQRT(COSQP1*(2.D0 - COSQP1))

C #3 COS(theta K)

C COSK1M = 1 + COS(theta K) + epsilon
      COSK1M=CTGM1M*FACT3**RND(4)
      COSK1 = COSK1M - EPSLON
      SINK  = SQRT(COSK1*(2.D0 - COSK1))

C #4 phi K  note that Kx=Ksin(theta_k)cos(phi_k + phi_qp) etc.
      PHIK=TWOPI*RND(5)

C #5 phi QP
      PHIP=TWOPI*RND(6)

C Now boost the first photon's angles to the gamma-e center of mass.
C Beta = -qp/(sqrts-qp0)

      BETA1 = (2.D0*FQP + .5D0*M**2/QP0)/(EB + FQP)
      BETA  = 1.D0-BETA1
      GAMA  = (SQRTS - QP0)/SQRT(SC)

C KXPKC = kx'/k' etc.  KCK = k'/k = k''/k
      KCK   = GAMA*(BETA*SINK*COS(PHIK)*SINQP
     >               + BETA1*(1.D0 - COSK1 - COSQP1 + COSK1*COSQP1)
     >               + COSK1 + COSQP1 - COSK1*COSQP1)
      ATERM = SINK*COS(PHIK)*SINQP - COSK1*COSQP1 + COSK1+COSQP1-BETA1
      KXPKC =(SINK*COS(PHIK) + BETA*SINQP + (GAMA-1.D0)*ATERM*SINQP)/KCK
      KYPKC =(SINK*SIN(PHIK))/KCK
      KZPKC =(COSK1 + BETA1*COSQP1 - BETA1 - COSQP1
     >        + (GAMA - 1.D0)*ATERM*(1.D0-COSQP1))/KCK

C KXCKC= kx''/k'' etc. this is the frame where kinematics a solved.
      XITERM = (GAMA-1.D0)*COSQP1 - GAMA*(BETA1 + BETA*EPSLON)
      SINTR  = (1.D0+XITERM)*SINQP /
     >          SQRT(2.D0*(1.D0 + XITERM)*COSQP1 + XITERM**2)
      COSTR  = SIGN(SQRT(1.D0-SINTR**2),COSQP1-XITERM*(1.D0-COSQP1))
      KXCKC = KXPKC*COSTR + KZPKC*SINTR
      KYCKC = KYPKC
      KZCKC = KZPKC*COSTR - KXPKC*SINTR

      SINKC = SQRT(KXCKC**2 + KYCKC**2)
      PHIKC = ATAN2(KYCKC,KXCKC)

C Now add another photon according to the approximate cross section.

C #6 KS

      KS = PBC * (1.D0+EPS)/(1.D0+EPS*RS**RND(7))

C #7 theta KS

C Determine if the generating axis is the -z axis or the QM0 direction
C also determine if peaked or flat distribution is generated
C and for peaked distribution, determine sign of delta-phi
      LPKPHI=.FALSE.
      SGNPHI=-1.D0
      IF(RND(8).GE..5)THEN
         LKSAXZ=.TRUE.
         IF(RND(8).LT.(FRAPHI+1.D0)/2.D0)THEN
            LPKPHI=.TRUE.
            IF(RND(8).LT.(FRAPHI+2.D0)/4.D0)SGNPHI=1.D0
         ENDIF
      ELSE
         LKSAXZ=.FALSE.
         IF(RND(8).LT.FRAPHI/2.D0)THEN
            LPKPHI=.TRUE.
            IF(RND(8).LT.FRAPHI/4.D0)SGNPHI=1.D0
         ENDIF
      ENDIF

C CSKS1 =1-COS(theta-ks) (about PM or -K direction)

      IF(LKSAXZ)THEN
         CSKS1P=EPSLON*FACT7**RND(9)-EPSLON
         SINKSP=SQRT(CSKS1P*(2.D0 - CSKS1P))
      ELSE
         CSKS1K=EPSLON*FACT7**RND(9)-EPSLON
         SINKSK=SQRT(CSKS1K*(2.D0 - CSKS1K))
      ENDIF

C #8 phi KS (about PM or -K direction)
C phi_ks=0 when ks is in the plane defined PM and -K.

      IF(.NOT.LPKPHI)THEN
         PHIKSX=TWOPI*RND(10)
      ELSE
         XPHIKS=EPSPHI*(1.D0+1.D0/EPSPHI)**RND(10)-EPSPHI
         PHIKSX=XPHIKS*PI*SGNPHI
      ENDIF

C Check that the configuration is even possible.
      IF(KS.GT.PBC)GOTO 1

C Solve the event kinematics
C --------------------------

C Solve for the 1st photon energy and for the electron. First for the
C case where the 2nd photon is generated about the -z axis

      IF(LKSAXZ)THEN

         PHIKSP = PHIKC + PI + PHIKSX

         KC = ( SQRTSC/2.D0*(SQRTSC-2.D0*KS) - M**2/2.D0 ) /
     >        ( SQRTSC - KS*(1.0D0 + KZCKC - CSKS1P+(1.D0-KZCKC)*CSKS1P
     >                       - SINKC*SINKSP*COS(PHIKC-PHIKSP) )  )
         QM0C = SQRTSC - KC - KS
         QMPC = SQRT(QM0C**2-M**2)

         CSKS1Q=( (QMPC+KS)**2 - KC**2 ) / (2.D0*QMPC*KS)

         CSKS1K=( QM0C**2 - M**2 - (KC-KS)**2 ) / (2.D0*KC*KS)
         SINKSK=SQRT(CSKS1K*(2.D0 - CSKS1K))

         GAMC4(1) = KC*KXCKC
         GAMC4(2) = KC*KYCKC
         GAMC4(3) = KC*KZCKC
         GAMC4(4) = KC

         GAMSC4(1) = KS*SINKSP*COS(PHIKSP)
         GAMSC4(2) = KS*SINKSP*SIN(PHIKSP)
         GAMSC4(3) = KS*(CSKS1P-1.D0)
         GAMSC4(4) = KS

         QMC4(1) = -GAMC4(1)-GAMSC4(1)
         QMC4(2) = -GAMC4(2)-GAMSC4(2)
         QMC4(3) = -GAMC4(3)-GAMSC4(3)
         QMC4(4) = QM0C

C Calculate the x_phi for the four cases (phi_ks about PM OR about -K)
C                                       X(regular OR ks <--> k symm)
         XPHIP1 = XPHI(PHIKSX)
         XPHIP2 = XPHIP1

         CPHKSK = ( CSKS1P-1.D0 + KZCKC*(1.D0-CSKS1K) )/
     >            (SINKC*SINKSK)
         CPHKKS = ( KZCKC + (CSKS1P-1.D0)*(1.D0-CSKS1K) )/
     >            (SINKSP*SINKSK)

         CPHKSK = MIN(1.D0,MAX(-1.D0,CPHKSK))
         CPHKKS = MIN(1.D0,MAX(-1.D0,CPHKKS))

         XPHIK1 = XPHI(ACOS(CPHKSK)+PI)
         XPHIK2 = XPHI(ACOS(CPHKKS)+PI)
      ELSE

C Now for the case where the 2nd photon is generated about the -k axis
C First the energies of the participants can be found from CSKS1K

C The phi_ks peak occurs at phi_ks = pi
         PHIKSK = PI + PHIKSX

         KC = (.5D0*(SC-M**2) - SQRTSC*KS)/(SQRTSC - KS*(2.0D0-CSKS1K))
         QM0C = SQRTSC - KC - KS
         QMPC = SQRT(QM0C**2 - M**2)

C KSXKS = Ks_x / Ks etc.

         KXYCKC=SQRT(KXCKC**2 + KYCKC**2)
         CPKSK = COS(PHIKSK)
         SPKSK = SIN(PHIKSK)

         KSXKS = - KXCKC*KZCKC/KXYCKC * SINKSK*CPKSK
     >           - KYCKC/KXYCKC * SINKSK*SPKSK - KXCKC*(1.D0-CSKS1K)
         KSYKS = - KYCKC*KZCKC/KXYCKC * SINKSK*CPKSK
     >           + KXCKC/KXYCKC * SINKSK*SPKSK - KYCKC*(1.D0-CSKS1K)
         KSZKS =   KXYCKC*SINKSK*CPKSK         - KZCKC*(1.D0-CSKS1K)

C Calculate the 4-vectors in the CM frame.

         GAMC4(1) = KC*KXCKC
         GAMC4(2) = KC*KYCKC
         GAMC4(3) = KC*KZCKC
         GAMC4(4) = KC

         GAMSC4(1) = KS*KSXKS
         GAMSC4(2) = KS*KSYKS
         GAMSC4(3) = KS*KSZKS
         GAMSC4(4) = KS

         QMC4(1) = -GAMC4(1)-GAMSC4(1)
         QMC4(2) = -GAMC4(2)-GAMSC4(2)
         QMC4(3) = -GAMC4(3)-GAMSC4(3)
         QMC4(4) = QM0C

         CSKS1P = 1.D0 + GAMSC4(3)/KS
         SINKSP = SQRT(CSKS1P*(2.D0-CSKS1P))

         CSKS1Q=( (QMPC+KS)**2 - KC**2 ) / (2.D0*QMPC*KS)

C Calculate the x_phi for the four cases (phi_ks about PM OR about -K)
C                                       X(regular OR ks <--> k symm)
         XPHIP1 = XPHI(ATAN2(KYCKC,KXCKC)-ATAN2(KSYKS,KSXKS)+PI)
         XPHIP2 = XPHIP1

         CPHKSK = ( CSKS1P-1.D0 + KZCKC*(1.D0-CSKS1K) )/
     >            (SINKC*SINKSK)
         CPHKKS = ( KZCKC + (CSKS1P-1.D0)*(1.D0-CSKS1K) )/
     >            (SINKSP*SINKSK)

C Since CPHKKS can be somewhat unstable, recalculate
         KZCKC1 = 1.D0 + KZCKC
C        IF(KZCKC.LT.-0.99D0)THEN
C           COSTR1 = 1.D0 - COSTR
C           IF(COSTR.GT.0.99D0)COSTR1 = TASQRT(SINTR**2)
C           KZPKC1 = 1.D0 + KZPKC
C           IF(KZPKC.LT.-0.99D0)KZPKC1 = TASQRT(KXPKC**2 + KYPKC**2)
C           KZCKC1 = COSTR1 + KZPKC1 - COSTR1*KZPKC1 - KXPKC*SINTR
C        ENDIF
         CSKS2P = 2.D0 - CSKS1P
         IF(CSKS1P.GT.1.99D0)CSKS2P = TASQRT(KSXKS**2 + KSYKS**2)

         CPHKKS = (KZCKC1 - CSKS2P - CSKS1K + CSKS2P*CSKS1K)/
     >            (SINKSP*SINKSK)

         CPHKSK = MIN(1.D0,MAX(-1.D0,CPHKSK))
         CPHKKS = MIN(1.D0,MAX(-1.D0,CPHKKS))

         XPHIK1 = XPHI(PHIKSX)
         XPHIK2 = XPHI(ACOS(CPHKKS)+PI)

      ENDIF

C Finished with the kinematics......
C Require that the first photon be above the cutoff too.

      IF(KC.LT.CUTOFF)GOTO 1

C Rotate and boost back to lab system, then rotate for phip

      CALL TBOORT(QMC4,SINTR,COSTR,GAMA,BETA,SINQP,COSQP1,PHIP,QML4)
      CALL TBOORT(GAMC4,SINTR,COSTR,GAMA,BETA,SINQP,COSQP1,PHIP,GAML4)
      CALL TBOORT(GAMSC4,SINTR,COSTR,GAMA,BETA,SINQP,COSQP1,PHIP,GAMSL4)

      QPP0    = SQRT(QP0**2-M**2)
      QPL4(1) = QPP0*SINQP*COS(PHIP)
      QPL4(2) = QPP0*SINQP*SIN(PHIP)
      QPL4(3) = QPP0*(1.D0-COSQP1)
      QPL4(4) = QP0

      QM0 = QML4(4)
      QMP0 = SQRT(QM0**2-M**2)
      K0  = GAML4(4)
      KSL = GAMSL4(4)

      COSQM=QML4(3)/QMP0
C COSQM1= 1+COS(theta q-) in lab system (can be small)
      IF(QML4(3).LT.0.D0)THEN
         COSQM1 = ( QML4(1)**2 + QML4(2)**2 )
     >            /QMP0**2 /(1.D0-COSQM)
      ELSE
         COSQM1 = 1.D0+COSQM
      ENDIF

      COSKSL=GAMSL4(3)/GAMSL4(4)
C CSKS1L= 1+COS(theta ks) in lab system (can be small)
      IF(GAMSL4(3).LT.0.D0)THEN
         CSKS1L = ( GAMSL4(1)**2 + GAMSL4(2)**2 )
     >            /GAMSL4(4)**2 /(1.D0-COSKSL)
      ELSE
         CSKS1L = 1.D0+COSKSL
      ENDIF

C Check if the event passes for the chosen configuration.

      PHIEL =ATAN2(QML4(2),QML4(1))
      PHIKL =ATAN2(GAML4(2),GAML4(1))
      PHIKSL=ATAN2(GAMSL4(2),GAMSL4(1))

C Acceptance criteria
      LAE  = QM0.GE.EEMIN .AND. ABS(COSQM).LE.ACTEM
      LAG  =  K0.GE.EGMIN
      LAGS = KSL.GE.EGMIN .AND. ABS(COSKSL).LE.ACTK
      LAEG = PHISEP(PHIEL,PHIKL) .GE. PEGMIN
      LAEGS= PHISEP(PHIEL,PHIKSL) .GE. PEGMIN

C Rejection criteria
      LRE  = QM0.GE.EEVETO .AND. COSQM1.GE.CDELT1
      LRG  =  K0.GE.EGVETO .AND. COSK1.GE.CTGVT1
      LRGS = KSL.GE.EGVETO .AND.
     >       MIN((1.D0+COSKSL),(1.D0-COSKSL)).GE.CTGVT1
      LREG = PHISEP(PHIEL,PHIKL)  .GE. PHVETO
      LREGS= PHISEP(PHIEL,PHIKSL) .GE. PHVETO
      LRGGS= PHISEP(PHIKL,PHIKSL) .GE. PHVETO

C Now decide if event passes all the criteria: LPASS
C and if the (k,ks) swapped event could have been generated: LSWAP

      LPASS=.FALSE.
      LSWAP=.FALSE.

      IF(CONFIG.EQ.EGAMMA)THEN
         LPASS = LAE .AND. ((LAG.AND.LAEG) .OR. (LAGS.AND.LAEGS))
         LSWAP = ABS(COSKSL).LE.ACTK
      ELSE IF(CONFIG.EQ.ETRON)THEN
         LPASS = LAE.AND. .NOT.(LRG.AND.LREG).AND. .NOT.(LRGS.AND.LREGS)
         LSWAP = (1.D0+COSKSL) .LT. CTGVT1
      ELSE IF(CONFIG.EQ.GAMMA)THEN
         LPASS = (LAG .AND. .NOT.LRE .AND. .NOT.(LRGS.AND.LRGGS) ) .OR.
     >           (LAGS.AND. .NOT.LRE .AND. .NOT.(LRG .AND.LRGGS) )
         LSWAP = ABS(COSKSL).LE.ACTK
      ELSE IF(CONFIG.EQ.GAMMAE)THEN
         LPASS = (LAGS.AND. .NOT.LRE .AND. COSQM1.GE.CDELT1)
         LSWAP = .FALSE.
      ENDIF

      IF(.NOT.LPASS)GOTO 1

C Derive some quantities needed for calculating the event weight

      CSK1CQ=( (QMPC+KC)**2 - KS**2 ) / (2.D0*QMPC*KC)

      IF(M/QM0C.LT..01D0)THEN
         DELTQ=QM0C*( 1.D0/2.D0 *(M/QM0C)**2 +  1.D0/8.D0 * (M/QM0C)**4
     >              + 3.D0/48.D0*(M/QM0C)**6 + 15.D0/384.D0*(M/QM0C)**8)
      ELSE
         DELTQ=QM0C-QMPC
      ENDIF

C These invariants are calculated in the gamma-e CM

      KAP1 = (PBC*KS*CSKS1P + M**2/2.D0 * KS/PBC)         / M**2
      KAP2 = (PBC*KC*(1.D0+KZCKC) + M**2/2.D0 * KC/PBC)   / M**2
      KAP3 = (-SC+M**2)/2.D0                              / M**2
      KAP1P= (-QMPC*KS*CSKS1Q - DELTQ*KS)                 / M**2
      KAP2P= (-QMPC*KC*CSK1CQ - DELTQ*KC)                 / M**2
      KAP3P= (QM0C-QMC4(3))*PBC                           / M**2

      A = 1.D0/KAP1  + 1.D0/KAP2  + 1.D0/KAP3
      B = 1.D0/KAP1P + 1.D0/KAP2P + 1.D0/KAP3P
      C = 1.D0/KAP1/KAP1P + 1.D0/KAP2/KAP2P + 1.D0/KAP3/KAP3P
      X = KAP1  + KAP2  + KAP3
      Z = KAP1*KAP1P + KAP2*KAP2P + KAP3*KAP3P
      UA= KAP1  * KAP2  * KAP3
      UB= KAP1P * KAP2P * KAP3P
      RO= KAP1/KAP1P+KAP1P/KAP1 + KAP2/KAP2P+KAP2P/KAP2 +
     >    KAP3/KAP3P+KAP3P/KAP3

      XTERM =  2.D0*(A*B-C)*( (A+B)*(X+2.D0) - (A*B-C) - 8.D0 )
     >       - 2.D0*X*(A**2+B**2 - C*RO ) - 8.D0*C
     >       + 4.D0*X*
     >         ( (1/UA+1/UB)*(X+1.D0)-(A/UB+B/UA)*(2.D0+Z*(1.D0-X)/X)
     >                + X/UA*X/UB*(1.D0-Z) + 2.D0/UA*Z/UB )
     >       - 2.D0*RO*( A*B + C )

C Calculate the 'exact' cross section
C It is not symmetrized about + <--> - .

C Here is the slightly approximated t
      T   = -SQRTS*QP0*COSQP1 - M**2*FQP**2/EB/QP0
C Here is the more exact t:
      TE  = -2.D0*EBP*QPP0*COSQP1 -M**2*FQP**2/EB/QP0

      DSIGE = (ALPHA/PI)**4 * XTERM/M**2 /S /(-TE) *
     >        ( (S**2 + (S-SC)**2)/SC**2 + 2.D0*M**2/TE )

C If the Berends et al. eegg calculation is requested:

      IF(MTRXGG.EQ.BEEGG .OR. (MTRXGG.EQ.HEEGG .AND. -T.GE.HQ2MIN))THEN

C Invariant products of photons and the electron.
C Work with the LAB 4 vectors only.
C The CM does not have a physical value for p-
C and was only developed for EPA methods.

         BPMK1 = EB*GAML4(4) + EBP*GAML4(3)
         BPMK2 = EB*GAMSL4(4) + EBP*GAMSL4(3)
         BQMK1 = TPRD(QML4,GAML4)
         BQMK2 = -KAP1P*M**2
         BK1K2 = TPRD(GAML4,GAMSL4)

C Calculate the BK invariants:

         U = 2.D0*M**2 - SQRTS*QM0 + 2.D0*EBP*QMP0*COSQM
         SP= S - 2.D0*SQRTS*(K0+KSL) + 2.D0*BK1K2
         TP= 2.D0*M**2 - SQRTS*QM0 - 2.D0*EBP*QMP0*COSQM
         UP= 2.D0*M**2 - SQRTS*QP0
     >                 - 2.D0*EBP*QPP0*(1.D0-COSQP1)
         S1= 4.D0*EB*(EB-K0)
         S2= 4.D0*EB*(EB-KSL)
         T1= TE- 2.D0*(BQMK1-BPMK1+BK1K2)
         T2= TE- 2.D0*(BQMK2-BPMK2+BK1K2)
         U1= U - SQRTS*K0*(2.D0-COSK1) - M**2*K0/EB*(COSK1-1.D0)
     >       + 2.D0*BQMK1
         U2= U - SQRTS*KSL*(1.D0-COSKSL) - M**2*KSL/EB*COSKSL
     >       + 2.D0*BQMK2
         PPK1K2 = SP+UP+TP - 3.D0*M**2
         PMK1K2 = SP+U +TE - 3.D0*M**2
         QPK1K2 = S +U +TP - 3.D0*M**2
         QMK1K2 = S +UP+TE - 3.D0*M**2

         K1P = GAML4(4)+GAML4(3)
         K1M = GAML4(4)-GAML4(3)
         K2P = GAMSL4(4)+GAMSL4(3)
         K2M = GAMSL4(4)-GAMSL4(3)
         QPP = QPL4(4)+QPL4(3)
         QPM = QP0*COSQP1 - (QPP0-QP0)*(1.D0-COSQP1)
         QMP = QML4(4) + QML4(3)
         QMM = QML4(4) - QML4(3)

         DSIGE=BEEGGC(M,EB,QPL4,QML4,GAML4,GAMSL4
     >,               S1,S2,TE,T1,T2,U1,U2,PPK1K2,PMK1K2,QPK1K2,QMK1K2
     >,               K1P,K1M,K2P,K2M,QPP,QPM,QMP,QMM
     >,               BPMK1,BPMK2,BQMK1,BQMK2,BK1K2)

      ELSE IF(MTRXGG.EQ.MEEGG)THEN

         DSIGE=MEEGGC(QPL4,QML4,GAML4,GAMSL4)

      ENDIF

C Calculate approximate cross section.
C Here the approximate sigma is symmetrized for k <--> ks

      DSIGA= 2.D0*(ALPHA/PI)**4 * QP0 / COSK1M / FQP**2 / (-T)
     >           *(  1.D0/(CSKS1P+EPSLON)*DXPHI(XPHIP1)
     >             + 1.D0/(CSKS1K+EPSLON)*DXPHI(XPHIK1) )
     >           /(1.D0 - KS/PBC + EPS) / KS**2 / K0**2
     >           *(-M**2*KAP2P) * QP0/(QP0**2-M**2) * (1.D0 + EPS)

      IF(LSWAP)THEN

         DSIGA= DSIGA
     >       +2.D0*(ALPHA/PI)**4 * QP0 / (CSKS1L+EPSLON) / FQP**2 / (-T)
     >       *(  1.D0/(1.D0+KZCKC+EPSLON)*DXPHI(XPHIP2)
     >         + 1.D0/(CSKS1K+EPSLON)*DXPHI(XPHIK2)     )
     >       /(1.D0 - KC/PBC + EPS) / KC**2 / KSL**2
     >       *(-M**2*KAP1P) * QP0/(QP0**2-M**2) * (1.D0 + EPS)

      ENDIF

C The weight of this trial event is,

      WGHT = DSIGE/DSIGA
      WMAX = MAX(WMAX,WGHT)
      W1MAX= MAX(W1MAX,WGHT1)

C Keep track of quantities for the total cross section calculation.

      SUMWGT = SUMWGT + WGHT
      SUMW2  = SUMW2  + WGHT**2

      IF(UNWGHT.AND.WGHT.LT.RND(11)*WGHTMX)GOTO 1

C An event has been accepted at this point
C ----------------------------------------

      NACC=NACC+1

C Look to see if the EPA is valid for the event sample

      Q2W2MX=MAX(Q2W2MX,-T/W2)

C Calculate the 4 vectors. Decide whether to interchange or not here.

      IF(RND(12).LT..5)THEN
         IMISS=0
         IOBSV=4
         RSIGN=+1.D0
      ELSE
         IMISS=4
         IOBSV=0
         RSIGN=-1.D0
      ENDIF

      P(IMISS+1)=QPL4(1) * RSIGN
      P(IMISS+2)=QPL4(2) * RSIGN
      P(IMISS+3)=QPL4(3) * RSIGN
      P(IMISS+4)=QPL4(4)

      P(IOBSV+1)=QML4(1) * RSIGN
      P(IOBSV+2)=QML4(2) * RSIGN
      P(IOBSV+3)=QML4(3) * RSIGN
      P(IOBSV+4)=QML4(4)

      P(9) =GAML4(1) * RSIGN
      P(10)=GAML4(2) * RSIGN
      P(11)=GAML4(3) * RSIGN
      P(12)=GAML4(4)

      P(13)=GAMSL4(1) * RSIGN
      P(14)=GAMSL4(2) * RSIGN
      P(15)=GAMSL4(3) * RSIGN
      P(16)=GAMSL4(4)
      
! (TF)
!       write (*,*) "e+     ", P(1), P(2), P(3), P(4)
!       write (*,*) "e-     ", P(5), P(6), P(7), P(8)
!       write (*,*) "GAMMA1 ", P(9), P(10), P(11), P(12)
!       write (*,*) "GAMMA2 ", P(13), P(14), P(15), P(16)
!       write (*,*) " "

      TP2(0)=P(1)
      TP2(1)=P(2)
      TP2(2)=P(3)
      TP2(3)=P(4)
      
      TQ2(0)=P(5)
      TQ2(1)=P(6)
      TQ2(2)=P(7)
      TQ2(3)=P(8)
      
      TNPHOT=0
      IF(P(12).gt.0d0) THEN
	TPHOT(0,0)=P(9)
	TPHOT(0,1)=P(10)
	TPHOT(0,2)=P(11)
	TPHOT(0,3)=P(12)
	TNPHOT=TNPHOT+1
      ENDIF
      
      IF(P(16).gt.0d0) THEN
	TPHOT(1,0)=P(13)
	TPHOT(1,1)=P(14)
	TPHOT(1,2)=P(15)
	TPHOT(1,3)=P(16)
	TNPHOT=TNPHOT+1
      ENDIF

      RETURN
      END
