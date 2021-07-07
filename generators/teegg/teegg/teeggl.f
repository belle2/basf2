
CDECK  ID>, TEEGGL. 
*.
*...TEEGGL   checks that the parameters for TEEGG are valid.
*.
*.     TEEGGL is a logical function that returns true if the set
*.     of parameters supplied to TEEGG is valid. TEEGGL returns
*.     false and error messages generated otherwise.
*.
*. INPUT     : OLUN  logical unit number to write error messages (if any
*.
*. SEQUENCE  : TEGCOM
*. CALLS     : FQPEG,INMART
*. CALLED    : USINIT,TEEGGM
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.2
*. CREATED   : 29-Jun-87
*. LAST MOD  : 10-Apr-89
*.
*. Modification Log.
*. 28-Sep-88   Dean Karlen   Change to conform to OPAL standards.
*. 10-Apr-89   Dean Karlen   Add checks for FRAPHI and EPSPHI
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C LOGICAL FUNCTION TEEGGL: Checks parameters and calculates constants.
C-----------------------------------------------------------------------

      FUNCTION TEEGGL(OLUN)

C Returns .TRUE. if okay ; .FALSE. if parameters are invalid.
C OLUN is the unit number to write out any error messages.

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

      REAL        RND(NRNDMX)
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


      LOGICAL TEEGGL
      INTEGER   OLUN
      DOUBLE PRECISION
     >       SINTST,QP0MAX,MINEE,MINEG,MINQP,ECMMIN,SINT1,SINT2,TKMIN
     >,      PBCMIN

      EXTERNAL FQPEG
      DOUBLE PRECISION FQPEG

      TEEGGL=.TRUE.

C Run through various parameters, and check for validity

      IF(EB .LT. 0.1 D0 )THEN
         TEEGGL=.FALSE.
         WRITE(*,*)'EB is too small. EB must be greater than .1 GeV'
      ENDIF

      IF(MAX(MAX(MAX(TEVETO,TEMIN),TGMIN),TGVETO) .GT. PI/2.D0)THEN
         TEEGGL=.FALSE.
         WRITE(*,*)'All ANGLE parameters must be less than pi/2'
      ENDIF

      IF(MIN(MIN(MIN(TEVETO,TEMIN),TGMIN),TGVETO) .LE.  0.D0)THEN
         TEEGGL=.FALSE.
         WRITE(*,*)'All ANGLE parameters must be greater than 0'
      ENDIF

      IF(CUTOFF .LE.  0.D0 .AND. RADCOR.NE.NONE)THEN
         TEEGGL=.FALSE.
         WRITE(*,*)'CUTOFF must be greater than 0'
      ENDIF

      IF(EEMIN.LT.1.D-4*EB .AND. CONFIG.EQ.ETRON)THEN
         TEEGGL=.FALSE.
         WRITE(*,*)'EEMIN must be at least EB/10000. '
      ENDIF

      IF(EGMIN.LT.1.D-4*EB .AND. CONFIG.NE.ETRON)THEN
         TEEGGL=.FALSE.
         WRITE(*,*)'EGMIN must be at least EB/10000. '
      ENDIF

      IF(CONFIG.EQ.EGAMMA)THEN

         IF(TEVETO.GE.TEMIN .AND. TEVETO.GE.TGMIN)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'TEMIN or TGMIN must be greater than TEVETO'
         ENDIF

      ELSE IF(CONFIG.EQ.GAMMA)THEN

         IF(TEVETO  .GE. TGMIN)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'TEVETO must be less than TGMIN'
         ENDIF

      ELSE IF(CONFIG.EQ.GAMMAE)THEN

         IF(RADCOR.NE.HARD)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'GAMMAE only valid for RADCOR = HARD'
         ENDIF
         IF(TEVETO  .GE. TGMIN)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'TEVETO must be less than TGMIN'
         ENDIF
         IF(TGVETO  .GE. TGMIN)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'TGVETO must be less than TGMIN'
         ENDIF
         IF(TGVETO  .GT. TEVETO)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'TGVETO must be less than or equal to TEVETO'
         ENDIF
         IF(EEVETO .LT. M)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'EEVETO must be greater than m_e'
         ENDIF

      ELSE IF(CONFIG.EQ.ETRON)THEN

         IF(TEVETO.GE.TEMIN)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'TEVETO must be less than TEMIN'
         ENDIF
C The following is made for convenience. If necessary it can be removed,
C but then theta-gamma generation must allow +z side final state.
         IF(TGVETO.GE.TEMIN)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'TGVETO must be less than TEMIN'
         ENDIF

      ELSE

         TEEGGL=.FALSE.
         WRITE(*,*)'Invalid configuration. Choose EGAMMA, GAMMA, ',
     >                ' or ETRON.'

      ENDIF

      IF(RADCOR.NE.HARD .AND. RADCOR.NE.SOFT .AND. RADCOR.NE.NONE)THEN
         TEEGGL=.FALSE.
         WRITE(*,*)'Invalid RADCOR. Choose HARD, SOFT,or NONE.'
      ENDIF

C Check that MATRIX element is consistent with RADCOR

      IF(RADCOR.NE.HARD)THEN
         IF(MATRIX.NE.BK    .AND. MATRIX.NE.BKM2 .AND.
     >      MATRIX.NE.TCHAN .AND. MATRIX.NE.EPA       )THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'Invalid MATRIX. Choose BK, BKM2, TCHAN,or EPA'
         ENDIF
      ELSE
         IF(MTRXGG.NE.EPADC .AND. MTRXGG.NE.BEEGG .AND.
     >      MTRXGG.NE.MEEGG .AND. MTRXGG.NE.HEEGG      )THEN
            TEEGGL=.FALSE.
            WRITE(*,*)
     >      'Invalid MTRXGG. Choose EPADC, BEEGG, MEEGG, or HEEGG'
         ENDIF

         IF(MTRXGG.EQ.BEEGG)THEN
          IF(CONFIG.EQ.GAMMA)
     >    WRITE(*,*)'MTRXGG=BEEGG may not valid be for CONFIG=GAMMA'
          IF(CONFIG.EQ.ETRON)
     >    WRITE(*,*)'MTRXGG=BEEGG may not valid be for CONFIG=ETRON'
          IF(CONFIG.EQ.GAMMAE)
     >    WRITE(*,*)'MTRXGG=BEEGG may not valid be for CONFIG=GAMMAE'
         ENDIF

      ENDIF

      IF(RADCOR.EQ.HARD .AND. EPS.LE.0.D0)THEN
         TEEGGL=.FALSE.
         WRITE(*,*)'EPS must be greater than 0.'
      ENDIF

      IF(RADCOR.EQ.HARD)THEN
         IF(FRAPHI.LT.0.D0 .OR. FRAPHI.GT.1.D0)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'FRAPHI must be between 0. and 1.'
         ELSE IF(EPSPHI.LT.1.D-6 .OR. EPSPHI.GT.1.D0)THEN
            TEEGGL=.FALSE.
            WRITE(*,*)'EPSPHI must be between 1.D-6 and 1.'
         ENDIF
      ENDIF


C If the parameters look good, continue on...

      IF(TEEGGL)THEN

C Keep track of the initial seed

         BSEED=NXSEED

C Initialize some variables.

         NTRIAL=0
         NPASSQ=0
         NACC  =0
         W1MAX =0.D0
         WMAX  =0.D0
         WMINSF=1.D20
         Q2W2MX=0.D0
         SUMW1 =0.D0
         SUMW12=0.D0
         SUMWGT=0.D0
         SUMW2 =0.D0
         ASOFTC=1.D0
         CONVER=0.D0

         P(13)=0.D0
         P(14)=0.D0
         P(15)=0.D0
         P(16)=0.D0

C Some useful constants
         S=4.D0*EB**2
         SQRTS=2.D0*EB
         EBP = SQRT(EB**2 -M**2)
         EPSLON=2.D0*M**2/S

         CDELT=COS(TEVETO)
C CDELT1 = 1 - COS(TEVETO)
         CDELT1 = 2.D0 * SIN(TEVETO/2.D0)**2
C CTGVT1 = 1 - COS(TGVETO)
         CTGVT1 = 2.D0 * SIN(TGVETO/2.D0)**2
         ACTEM = ABS(COS(TEMIN))
         ACTK  = ABS(COS(TGMIN))

C Calculate constants required for generation of the phase space vars
C First for the soft generation.

         IF(RADCOR.EQ.NONE .OR. RADCOR.EQ.SOFT)THEN

C #1 - QP0
C FQPMAX = EB - QP0max
C Note that this can be a small number compared to EB, so care must be
C taken.
C Also, a check is made to see if the config is kinematically allowed

            IF(CONFIG.EQ.EGAMMA)THEN

               FQPMAX = FQPEG(TEMIN,TGMIN,EEMIN,EGMIN,TEVETO,EB)

            ELSE IF(CONFIG.EQ.GAMMA)THEN

               FQPMAX = EB*( EGMIN*SIN((TGMIN-TEVETO)/2.D0)**2 /
     >                       (EB-EGMIN*COS((TGMIN-TEVETO)/2.D0)**2) )
               SINTST = EGMIN*SIN(TGMIN-TEVETO)/(EB+FQPMAX-EGMIN)
               IF(SINTST.GT.SIN(2.D0*TEVETO))FQPMAX=-1.

            ELSE IF(CONFIG.EQ.ETRON)THEN

               FQPMAX = SQRTS*EEMIN*SIN((TEMIN-TEVETO)/2.D0)**2 /
     >                  (SQRTS - EEMIN*(1.D0+COS(TEMIN-TEVETO)))
               SINTST = EEMIN*SIN(TEMIN-TEVETO)/(EB+FQPMAX-EEMIN)
               IF(SINTST.GT.SIN(TGVETO+TEVETO))FQPMAX=-1.

            ENDIF

C Here we check if the choice is kinematically impossible.

            IF(FQPMAX.LT.0.D0)THEN
               WRITE(*,*)'Sorry, your choice of parameters is',
     >                      ' kinematically impossible!'
               TEEGGL=.FALSE.
            ELSE
               QP0MAX=EB-FQPMAX
               QP0MIN=10.*M

               ZMAX=QP0MAX/FQPMAX
               LOGZ0M=LOG(1.D0 + CDELT1/EPSLON*ZMAX**2)

C #2 - COS(thetaQP)

C #3 - COS(thetaK)

               IF(CONFIG.EQ.EGAMMA .OR. CONFIG.EQ.ETRON)THEN

C Check if kinematics give a stronger limit on theta-gamma than given

                  SINT1=EEMIN * SIN(TEMIN+TEVETO) *
     >                  ( EB - EEMIN*SIN((TEMIN+TEVETO)/2.D0)**2 )
     >              /   ( EB**2 - EEMIN*(SQRTS-EEMIN) *
     >                          SIN((TEMIN+TEVETO)/2.D0)**2 )
                  SINT2=EEMIN * SIN(TEMIN-TEVETO) *
     >                  ( EB - EEMIN*COS((TEMIN-TEVETO)/2.D0)**2 )
     >              /   ( EB**2 - EEMIN*(SQRTS-EEMIN) *
     >                          COS((TEMIN-TEVETO)/2.D0)**2 )
                  TKMIN=ASIN(MIN(SINT1,SINT2))-TEVETO
                  IF(CONFIG.EQ.ETRON )TKMIN=MAX(0.D0,TKMIN)
                  IF(CONFIG.EQ.EGAMMA)TKMIN=MAX(TGMIN,TKMIN)

                  CTGM1M = 2.D0 * SIN(TKMIN/2.D0)**2 + EPSLON
                  IF(CONFIG.EQ.ETRON)
     >               FACT3=(2.D0*SIN(TGVETO/2.D0)**2 + EPSLON)/CTGM1M
                  IF(CONFIG.EQ.EGAMMA)
     >               FACT3=(1.D0+COS(TKMIN) + EPSLON)/CTGM1M

               ELSE

                  CTGM1M = 2.D0 * SIN(TGMIN/2.D0)**2 + EPSLON
                  FACT3=(1.D0+COS(TGMIN)+EPSLON)/CTGM1M

               ENDIF

C #4 - PHI K

C #5 - PHI QP

            ENDIF

C Now for the hard generation

         ELSE IF(RADCOR.EQ.HARD)THEN

C #1 - QP0
C FQPMAX = EB - QP0max

            IF(CONFIG.EQ.EGAMMA)THEN

               FQPMAX = MIN( ( EB*EGMIN*SIN((TGMIN+TEVETO)/2.D0)**2
     >                        +EB*EEMIN*SIN((TEMIN-TEVETO)/2.D0)**2
     >                        -EGMIN*EEMIN*SIN((TGMIN+TEMIN)/2.D0)**2 )
     >                      /( EB - EGMIN*COS(TGMIN+TEVETO/2.D0)**2
     >                            - EEMIN*COS(TEMIN-TEVETO/2.D0)**2 )  ,

     >                       ( EB*EEMIN*SIN((TEMIN+TEVETO)/2.D0)**2
     >                        +EB*EGMIN*SIN((TGMIN-TEVETO)/2.D0)**2
     >                        -EEMIN*EGMIN*SIN((TEMIN+TGMIN)/2.D0)**2 )
     >                      /( EB - EEMIN*COS(TEMIN+TEVETO/2.D0)**2
     >                            - EGMIN*COS(TGMIN-TEVETO/2.D0)**2 )  )

            ELSE IF(CONFIG.EQ.GAMMA .OR. CONFIG.EQ.GAMMAE)THEN

               FQPMAX = EB*( EGMIN*SIN((TGMIN-TEVETO)/2.D0)**2 /
     >                       (EB-EGMIN*COS((TGMIN-TEVETO)/2.D0)**2) )
               SINTST = EGMIN*SIN(TGMIN-TEVETO)/(EB+FQPMAX-EGMIN)
               IF(SINTST.GT.SIN(2.D0*TEVETO))FQPMAX=-1.

            ELSE IF(CONFIG.EQ.ETRON)THEN

               FQPMAX = SQRTS*EEMIN*SIN((TEMIN-TEVETO)/2.D0)**2 /
     >                  (SQRTS - EEMIN*(1.D0+COS(TEMIN-TEVETO)))
               SINTST = EEMIN*SIN(TEMIN-TEVETO)/(EB+FQPMAX-EEMIN)
               IF(SINTST.GT.SIN(TGVETO+TEVETO))FQPMAX=-1.

            ENDIF

C Here we check if the choice is kinematically impossible.

            IF(FQPMAX.LT.0.D0)THEN
               WRITE(*,*)'Sorry, your choice of parameters is',
     >                      ' kinematically impossible!', FQPMAX
               TEEGGL=.FALSE.
            ELSE

               QP0MAX=EB-FQPMAX
               QP0MIN=10.*M

               ZMAX=QP0MAX/FQPMAX
               PBCMIN= SQRT( (S+M**2)*FQPMAX/EB - 2.D0*M**2)/2.D0
               LOGZ0M=LOG(1.D0 + CDELT1/EPSLON*ZMAX**2)

               IF(CUTOFF.GE.PBCMIN*(1.D0+EPS))THEN
                  WRITE(*,*)'Your choice of CUTOFF is too large.', CUTOFF
                  TEEGGL=.FALSE.
               ELSE
                  LOGRSM=LOG( (PBCMIN*(1.D0+EPS) - CUTOFF)/(EPS*CUTOFF))
               ENDIF

C #2 - COS(thetaQP)

C #3 - COS(thetaK)

               IF(CONFIG.EQ.ETRON .OR. CONFIG.EQ.GAMMAE)THEN

                  CTGM1M = EPSLON
                  FACT3=(2.D0*SIN(TGVETO/2.D0)**2 + EPSLON)/CTGM1M

               ELSE

                  CTGM1M = 2.D0 * SIN(TGMIN/2.D0)**2 + EPSLON
                  FACT3=(1.D0+COS(TGMIN)+EPSLON)/CTGM1M

               ENDIF

C #4 - PHI K
C #5 - PHI QP
C #6 - KS

C #7 - Theta KS

               FACT7=(1.D0+2.D0/EPSLON)

C #8 - Phi KS

C Additional initialization is required for the Martinez/Miquel ME.
               IF(MTRXGG.EQ.MEEGG)CALL INMART

            ENDIF

         ENDIF

C Now finished with the constant calculations. Check few more parameters

         IF(TEEGGL)THEN

C Let the user know if he's working inefficiently

            IF(CDELT1 .LT. 10.D0 * 2.D0*M**2/QP0MIN/SQRTS)
     >         WRITE(*,*)'Warning: The choice of TEVETO is so '
     >,        'small as to cause this program to be inefficient'
            IF(FQPMAX .LT. 10.D0 * 2.D0*M**2/SQRTS)
     >         WRITE(*,*)'Warning: TEMIN or TGMIN '
     >,       'so close to TEVETO causes this program to be inefficient'

C Here a check on the minimum energy of the missing e or gamma is made.

           IF(CONFIG.EQ.GAMMA .OR. CONFIG.EQ.GAMMAE)THEN
               MINQP = SQRTS/(1.D0+ (SIN(2.D0*TEVETO)+SIN(TGMIN-TEVETO))
     >                 /SIN(TGMIN+TEVETO))
               MINEE = (SQRTS-MINQP)*SIN(TGMIN-TEVETO) /
     >                 ( SIN(2.D0*TEVETO) + SIN(TGMIN-TEVETO) )
               IF(MINEE.LT. 1.D-3 * EB)
     >            WRITE(*,*)'SEVERE Warning: TGMIN'
     >,           ' may be too close to TEVETO to trust the results!!!'
            ELSE IF(CONFIG.EQ.ETRON)THEN
               MINQP =SQRTS/(1.D0+(SIN(TGVETO+TEVETO)+SIN(TEMIN-TEVETO))
     >                 /SIN(TEMIN+TGVETO))
               MINEG = (SQRTS-MINQP)*SIN(TEMIN-TEVETO) /
     >                 ( SIN(TGVETO+TEVETO) + SIN(TEMIN-TEVETO) )
               IF(MINEG.LT. 1.D-3 * EB)
     >            WRITE(*,*)'SEVERE Warning: TEMIN'
     >,           ' may be too close to TEVETO to trust the results!!!'
            ENDIF

C Here check that CUTOFF is much less than the minimum total CM energy

            IF(RADCOR.EQ.SOFT.OR.RADCOR.EQ.HARD)THEN
               ECMMIN=2.D0*SQRT(EB*FQPMAX)
               IF(CUTOFF.GT.ECMMIN/2.)THEN
                  WRITE(*,*)'SEVERE Warning: CUTOFF is much '
     >,           'too large for the chosen set of parameters.'
                  WRITE(*,*)'Do not trust the results.'
                  WRITE(*,800)ECMMIN/10.D0
               ELSE IF(CUTOFF.GT.ECMMIN/10.)THEN
                  WRITE(*,*)'Warning: CUTOFF may be too large'
     >,           ' for the chosen set of parameters.'
                  WRITE(*,*)'The results are not guaranteed.'
                  WRITE(*,800)ECMMIN/10.D0
               ENDIF
 800           FORMAT(' Suggest choosing CUTOFF= ',E8.2,' GeV or less'/)

C Calculate the approximate soft correction here.(to stabilize wght)
C 3.5 seems to work better than 4
               ASOFTC = 1.D0 + 3.5D0 * ALPHA/PI * LOG(ECMMIN/M)
     >                                          * LOG(CUTOFF/ECMMIN)
            ENDIF

         ENDIF
      ENDIF

      RETURN
      END
