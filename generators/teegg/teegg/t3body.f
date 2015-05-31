
CDECK  ID>, T3BODY. 
*.
*...T3BODY   generates an e-e-gamma event.
*.
*. SEQUENCE  : TEGCOM
*. CALLS     : DMORK RNDGEN
*. CALLED    : TEEGG7
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.1
*. CREATED   : 29-Jun-87
*. LAST MOD  : 28-Sep-88
*.
*. Modification Log.
*. 28-Sep-88   Dean Karlen   Change to conform to OPAL standards.
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C SUBROUTINE T3BODY: Generates an e-e-gamma event
C-----------------------------------------------------------------------

      SUBROUTINE T3BODY
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

      DOUBLE PRECISION   RND(NRNDMX) ! changed from real to double (TF)
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
      DOUBLE PRECISION Z,Z0,FQPP,FQP,QP0,QPP,WGHT1
     >,      COSQP1,COSK1M,COSK1,PHIK,PHIP,W2,SINQP,SINK,COSKP1
     >,      K0,QM0,QMP,TA,COSQM1,F1,ABK,WBK,WM,TM,TPM,TRIX,D5L
     >,      FQM,COSKM1,DSIGA
     >,      SC,PBC,EBC,BETA1,BETA,GAMA,KCK,ATERM
     >,      KXPKC,KYPKC,KZPKC
     >,      XITERM,SINTR,COSTR,KXCKC,KYCKC,KZCKC,SINKC2,BETAM1,COSKC1
     >,      EMORK,TMORK,YMORK,UMORK,DEMORK,DELTAS

      INTEGER   IMISS,IOBSV
      

C     TF, 2015
      INTEGER TNPHOT
      DOUBLE PRECISION TP1(0:3),TQ1(0:3),TP2(0:3),TQ2(0:3),TPHOT(0:1,0:3)
      COMMON / MOMSET / TP1,TQ1,TP2,TQ2,TPHOT,TNPHOT
      
      COMMON / KINEMATICS / W2

C Externals
      DOUBLE PRECISION   DMORK
      EXTERNAL DMORK

C The starting point for a new trial
C ----------------------------------

 1    CONTINUE

C #1 Generate QP0
C Do until trial QP0 COSQP1 is taken

C Increment the trial event counter here.
         NTRIAL=NTRIAL+1

C Generate all random numbers needed here.
!          CALL RNDGEN(8)
         
         call teegg_rndmarr(RND,8)

         Z=RND(1)*ZMAX

C FQPP=EB-QP ; FQP=EB-QP0
         FQPP=EB/(1.D0+Z)
         FQP =FQPP - EPSLON*EB*(1.D0+Z)/Z
         QP0=EB-FQP
         QPP=SQRT(QP0**2-M**2)
         Z0 = QP0/FQP

C Require QP0 > QP0min
         IF(QP0.LT.QP0MIN)GOTO 1

C Assign a weight for this trial QP0
         WGHT1=(FQPP/FQP)**2 * LOG(  Z0**2*CDELT1/EPSLON+1.D0)/
     >                         LOGZ0M

C Keep track of all weights, to calculate the total cross section.
         SUMW1 = SUMW1 + WGHT1
         SUMW12= SUMW12+ WGHT1**2

      IF(WGHT1.LT.RND(2)*WGHT1M)GOTO 1

C A q+0 has been accepted. Keep track of number that pass qp generation.
      NPASSQ = NPASSQ + 1

C #2 Generate COS(theta QP0)

C COSQP1 = 1 - COS(theta QP0)
      COSQP1 = EPSLON/Z0**2*( (Z0**2/EPSLON*CDELT1+1.D0)**RND(3) -1.D0)

C #3 COS(theta K)

C COSK1M = 1 + COS(theta K) + epsilon
      COSK1M=CTGM1M*FACT3**RND(4)
      COSK1 = COSK1M - EPSLON

C #4 phi K  note that Kx=Ksin(theta_k)cos(phi_k + phi_qp) etc.
      PHIK=TWOPI*RND(5)

C #5 phi QP
      PHIP=TWOPI*RND(6)


C Derive the rest of the quantities that determine the exact x-section

C W2 = Invariant mass **2
      W2 = 2.D0*SQRTS*FQP + M**2

      SINQP = SQRT(COSQP1*(2.D0 - COSQP1))
      SINK  = SQRT(COSK1*(2.D0 - COSK1))

C COSKP1 = 1 + cos(theta gamma-e+)
      COSKP1= (COSK1+COSQP1) - COSK1*COSQP1 + SINK*SINQP*COS(PHIK)

      Y2 = (W2 - M**2)/2.D0

      K0 = SQRTS*FQP / (2.D0*FQP + QPP*COSKP1 + (QP0-QPP))

      QM0 = SQRTS - K0 - QP0
      QMP = SQRT(QM0**2-M**2)

C Carefully calculate t.If QP0 is large enough, expand in powers of m**2
      IF(QP0.GE.100.D0*M)THEN
         T   = -2.D0*EBP*QPP*COSQP1 -M**2*FQP**2/EB/QP0
      ELSE
         T   = -2.D0*EBP*QPP*COSQP1 + 2.D0*M**2 - 2.D0*(EB*QP0-EBP*QPP)
      ENDIF
      
C Also calculate the approximate t, to be used to evaluate DSIGA
      TA  = -2.D0*EB*QP0*(COSQP1 + EPSLON/Z0**2)

C COSQM1 = 1 + cos(theta e-)
      COSQM1 = (M**2 + W2 - T - 2.D0*EBP*K0*COSK1 -2.D0*K0*(EB-EBP)
     >          -2.D0*(EB*QM0-EBP*QMP))/(2.D0*EBP*QMP)

C Require that the electron is in the right place
C and that 'seen' particles have enough energy

      IF(CONFIG.EQ.EGAMMA)THEN
         IF(ABS(COSQM1-1.D0).GT.ACTEM)GOTO 1
         IF(QM0.LT.EEMIN .OR. K0.LT.EGMIN)GOTO 1
      ELSE IF(CONFIG.EQ.ETRON)THEN
         IF(ABS(COSQM1-1.D0).GT.ACTEM)GOTO 1
         IF(QM0.LT.EEMIN)GOTO 1
      ELSE IF(CONFIG.EQ.GAMMA)THEN
         IF(K0.LT.EGMIN)GOTO 1
         IF(COSQM1.GT.CDELT1)GOTO 1
      ENDIF

      TP  = 2*M**2 - 2.D0*EBP*QMP*COSQM1 - 2.D0*(EB*QM0-EBP*QMP)
      SP  = S - 2.D0 * SQRTS * K0
      U   = 2.D0 * M**2 - 2.D0*(EB*QM0+EBP*QMP) + 2.D0*EBP*QMP*COSQM1
      UP  = 2.D0 * M**2 - 2.D0*(EB*QP0+EBP*QPP) + 2.D0*EBP*QPP*COSQP1
      X1  = (EB+EBP)*K0 - EBP*K0*COSK1
      X2  = (EB-EBP)*K0 + EBP*K0*COSK1
      Y1  = (QP0+QPP)*K0 - QPP*K0*COSKP1

C Calculate the exact cross section.

      F1 = ALPHA3/PI**2 /S

C Calculate some M terms

      WM = M**2*(S-SP)/(S**2+SP**2)*(SP/X1+SP/X2+S/Y1+S/Y2)
      TM = -8.D0*M**2/T**2*(X2/Y2+Y2/X2)
      TPM= -8.D0*M**2/TP**2*(X1/Y1+Y1/X1)

C Decide which matrix element to use
C This is the Berends Kleiss terms only

      IF(MATRIX .EQ. BK )THEN
         ABK = (  S*SP*(S**2+SP**2) + T*TP*(T**2+TP**2)
     >          + U*UP*(U**2+UP**2)   ) / (S*SP*T*TP)
         WBK =  S/X1/X2 + SP/Y1/Y2 - T/X1/Y1 - TP/X2/Y2
     >        + U/X1/Y2 + UP/X2/Y1
         TRIX= ABK * WBK * (1.D0-WM)

C This is the Berends Kleiss with me**2 term

      ELSE IF(MATRIX .EQ. BKM2)THEN

         TRIX = (  S*SP*(S**2+SP**2) + T*TP*(T**2+TP**2)
     >           + U*UP*(U**2+UP**2)   ) / (S*SP*T*TP)
     >         *(S/X1/X2+SP/Y1/Y2-T/X1/Y1-TP/X2/Y2+U/X1/Y2+UP/X2/Y1)
     >         *(1.D0-WM)
     >         + TM + TPM

C This is the t-channel term with me**2 term. This can be used to judge
C the size of interference terms.

      ELSE IF(MATRIX .EQ. TCHAN)THEN

         TRIX=
     >     -(S**2+SP**2+U**2+UP**2)/T/X2/Y2 + TM

C This is the EPA matrix element (for testing only)

      ELSE IF(MATRIX .EQ. EPA)THEN

         TRIX= -4.D0*(X2**2 + Y2**2)
     >              *((S**2 + (S-W2)**2)/W2**2 + 2.D0*M**2/T)
     >              /T/X2/Y2

      ELSE

         TRIX= 1.D9

      ENDIF

      D5L = QP0 * K0 * K0 / Y2 / 8.D0

      DSIGE = F1 * TRIX * D5L

C Calculate approximate cross section.
C Here the approximate sigma is symmetrized.

      FQM=EB-QM0
      COSKM1=Y2/QM0/K0
      DSIGA =-2.D0*ALPHA3*S/PI**2 *
     >        (  1.D0/X2/Y2/TA*(SQRTS-QP0*(2.D0-COSKP1))/FQP
     >         + 1.D0/X1/Y1/TP*(SQRTS-QM0*(2.D0-COSKM1))/FQM )
     >        * D5L * ASOFTC

C If soft corrections are requested, modify the weight.

      WGHTSF=1.D0

      IF(RADCOR.EQ.SOFT)THEN
         SC = W2
         PBC= SQRT(W2-2.D0*M**2)/2.D0
         EBC= SQRT(PBC**2+M**2)

C Boost the photon's angles to the gamma-e center of mass.
C (allowing for transverse momentum, as done in the 4 body generator)
C Beta = -qp/(sqrts-qp0)

         BETA1 = (2.D0*FQP + .5D0*M**2/QP0)/(EB + FQP)
         BETA  = 1.D0-BETA1
         GAMA  = (SQRTS - QP0)/SQRT(SC)

C KXPKC = kx'/k' etc.  KCK = k'/k = k''/k
         KCK   = GAMA*(BETA*SINK*COS(PHIK)*SINQP
     >                  + BETA1*(1.D0 - COSK1 - COSQP1 + COSK1*COSQP1)
     >                  + COSK1 + COSQP1 - COSK1*COSQP1)
         ATERM = SINK*COS(PHIK)*SINQP - COSK1*COSQP1 +COSK1+COSQP1-BETA1
         KXPKC =(SINK*COS(PHIK) +BETA*SINQP+(GAMA-1.D0)*ATERM*SINQP)/KCK
         KYPKC =(SINK*SIN(PHIK))/KCK
         KZPKC =(COSK1 + BETA1*COSQP1 - BETA1 - COSQP1
     >           + (GAMA - 1.D0)*ATERM*(1.D0-COSQP1))/KCK

C KXCKC= kx''/k'' etc. this is the frame where kinematics a solved.
         XITERM = (GAMA-1.D0)*COSQP1 - GAMA*(BETA1 + BETA*EPSLON)
         SINTR  = (1.D0+XITERM)*SINQP /
     >             SQRT(2.D0*(1.D0 + XITERM)*COSQP1 + XITERM**2)
         COSTR  = SIGN(SQRT(1.D0-SINTR**2),COSQP1-XITERM*(1.D0-COSQP1))
         KXCKC = KXPKC*COSTR + KZPKC*SINTR
         KYCKC = KYPKC
         KZCKC = KZPKC*COSTR - KXPKC*SINTR

         SINKC2= KXCKC**2 + KYCKC**2

C BETAM1 = 1 - beta(e-) in cm frame
         BETAM1=(M**2/2.D0/PBC**2 - M**4/8.D0/PBC**4)*PBC/EBC
         COSKC1=SINKC2/(1.D0-KZCKC)

         EMORK=SQRT(SC)/M
         TMORK=.5D0*(BETAM1 + COSKC1 -BETAM1*COSKC1)
         YMORK=.5D0*LOG(.5D0* EMORK**2 * (1.D0 - KZCKC))
         UMORK=TMORK+1.D0/TMORK
         DEMORK=CUTOFF/M
         DELTAS=DMORK(TMORK,YMORK,UMORK,EMORK,DEMORK)
         WGHTSF=(1.D0+DELTAS)
         WMINSF=MIN(WMINSF,WGHTSF)
      ENDIF

C The weight of this trial event is,

      WGHT = DSIGE/DSIGA * WGHTSF
      WMAX = MAX(WMAX,WGHT)
      W1MAX= MAX(W1MAX,WGHT1)

C Keep track of quantities for the total cross section calculation.

      SUMWGT = SUMWGT + WGHT
      SUMW2  = SUMW2  + WGHT**2

      IF(UNWGHT.AND.WGHT.LT.RND(7)*WGHTMX)GOTO 1

C An event has been accepted at this point
C ----------------------------------------

      NACC=NACC+1

C Calculate the 4 vectors. Decide whether to interchange or not here.

      IF(RND(8).LT..5)THEN
         IMISS=0
         IOBSV=4
         RSIGN=+1.D0
      ELSE
         IMISS=4
         IOBSV=0
         RSIGN=-1.D0
      ENDIF

C 'missing'
      P(IMISS+1)=QPP*SINQP*COS(PHIP)  * RSIGN
      P(IMISS+2)=QPP*SINQP*SIN(PHIP)  * RSIGN
      P(IMISS+3)=QPP*(1.D0-COSQP1)    * RSIGN
      P(IMISS+4)=QP0
C Gamma
      P(9) =K0*SINK*COS(PHIK+PHIP)    * RSIGN
      P(10)=K0*SINK*SIN(PHIK+PHIP)    * RSIGN
      P(11)=K0*(COSK1-1.D0)           * RSIGN
      P(12)=K0
C 'observed'
      P(IOBSV+1)=-P(IMISS+1)-P(9)
      P(IOBSV+2)=-P(IMISS+2)-P(10)
      P(IOBSV+3)=QMP*(COSQM1-1.D0)    * RSIGN
      P(IOBSV+4)=QM0
      
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
