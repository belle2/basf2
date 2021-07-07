
CDECK  ID>, TEEGGP. 
*.
*...TEEGGP   prints out a summary of input parameters and total x-sect
*.
*. INPUT     : OLUN   logical unit number to write summary
*.
*. SEQUENCE  : TEGCOM
*. CALLED    : USLAST,TEEGGM
*.
*. BANKS L   : Names of banks lifted
*. BANKS U   : Names of banks used
*. BANKS M   : Names of banks modified
*. BANKS D   : Names of banks dropped
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.2
*. CREATED   : 29-Jun-87
*. LAST MOD  : 10-Apr-89
*.
*. Modification Log.
*. 28-Sep-88   Dean Karlen   Change to conform to OPAL standards.
*. 10-Apr-89   Dean Karlen   Add FRAPHI and EPSPHI to output.
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C SUBROUTINE TEEGGP: Prints out a summary of input params, total x-sect.
C-----------------------------------------------------------------------

      SUBROUTINE TEEGGP(OLUN)
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

C    TF, 2015

      INTEGER RESNGEN, RESNTRIALS
      DOUBLE PRECISION RESEFF, RESCROSS, RESCROSSERROR
      COMMON / TEEGGRESULTS / RESNGEN, RESNTRIALS, RESEFF, RESCROSS, RESCROSSERROR



      INTEGER   OLUN

      LOGICAL      LWWARN
      CHARACTER*4  DESCOR
      CHARACTER*7  DESCON
      CHARACTER*13 WARN,WARN1
      CHARACTER*30 DESCME

      IF(CONFIG.EQ.EGAMMA)THEN
         DESCON='e-gamma'
      ELSE IF(CONFIG.EQ.GAMMA)THEN
         DESCON='gamma  '
      ELSE IF(CONFIG.EQ.ETRON)THEN
         DESCON='etron  '
      ELSE IF(CONFIG.EQ.GAMMAE)THEN
         DESCON='gamma e'
      ENDIF

      IF(RADCOR.NE.HARD)THEN
         IF(MATRIX.EQ.BK)THEN
            DESCME='Berends & Kleiss             '
         ELSE IF(MATRIX.EQ.BKM2)THEN
            DESCME='Berends & Kleiss with m term '
         ELSE IF(MATRIX.EQ.TCHAN)THEN
            DESCME='t channel only (inc. m term) '
         ELSE IF(MATRIX.EQ.EPA)THEN
            DESCME='Equivalent photon approx.    '
         ENDIF
      ELSE
         IF(MTRXGG.EQ.EPADC)THEN
            DESCME='EPA with double compton      '
         ELSE IF(MTRXGG.EQ.BEEGG)THEN
            DESCME='Berends et al e-e-gamma-gamma'
         ELSE IF(MTRXGG.EQ.MEEGG)THEN
            DESCME='Martinez/Miquel e-e-g-g      '
         ELSE IF(MTRXGG.EQ.HEEGG)THEN
            DESCME='EPA/Berends et al hybrid eegg'
         ENDIF
      ENDIF

      IF(RADCOR.EQ.NONE)THEN
         DESCOR='none'
      ELSE IF(RADCOR.EQ.SOFT)THEN
         DESCOR='soft'
      ELSE IF(RADCOR.EQ.HARD)THEN
         DESCOR='hard'
      ENDIF

      LWWARN=.FALSE.
      IF(W1MAX.GT.WGHT1M)THEN
         WARN1='** Warning **'
         LWWARN=.TRUE.
      ELSE
         WARN1='             '
      ENDIF

      IF(WMAX.GT.WGHTMX.AND.UNWGHT)THEN
         WARN='** Warning **'
         LWWARN=.TRUE.
      ELSE
         WARN='             '
      ENDIF

      IF(RADCOR.EQ.SOFT .AND. WMINSF.LT.0.D0)THEN
         WRITE(OLUN,120)
 120     FORMAT(' ** Warning ** The choice of CUTOFF is too small,'
     >,         ' causing the soft correction',/,' weight to be < 0.',/
     >,         ' Increase CUTOFF, and try again.',/)
      ENDIF

      IF(LWWARN)THEN
         WRITE(OLUN,121)
 121     FORMAT(' ** Warning ** The choice of a maximum weight is too'
     >,         ' small (see below).',/,' Increase as necessary.',/)
      ENDIF

      IF(( (RADCOR.EQ.HARD.AND.MTRXGG.EQ.EPADC) .OR. RADCOR.EQ.SOFT .OR.
     >     (RADCOR.EQ.NONE.AND.MATRIX.EQ.EPA)) .AND. Q2W2MX.GE..1D0)THEN
         WRITE(OLUN,122)Q2W2MX
 122     FORMAT(' ** Warning ** The equivalent photon approximation'
     >,         ' may be invalid for the event',/,' sample generated:',/
     >,         ' Q**2/W**2 << 1 is not always true. max(Q**2/W**2)='
     >,         F7.3,/)
      ENDIF

      WRITE(OLUN,100)DESCOR,DESCME,DESCON,BSEED,NXSEED,EB
      IF(RADCOR.NE.NONE)WRITE(OLUN,101)CUTOFF
      WRITE(OLUN,102)TEVETO
      IF(CONFIG.EQ.EGAMMA)THEN
         WRITE(OLUN,103)TEMIN,TGMIN,EEMIN,EGMIN
      ELSE IF(CONFIG.EQ.GAMMA.AND.RADCOR.NE.HARD)THEN
         WRITE(OLUN,104)TGMIN,EGMIN
      ELSE IF(CONFIG.EQ.GAMMA.AND.RADCOR.EQ.HARD)THEN
         WRITE(OLUN,105)TGMIN,TGVETO,EGMIN
      ELSE IF(CONFIG.EQ.GAMMAE.AND.RADCOR.EQ.HARD)THEN
         WRITE(OLUN,105)TGMIN,TGVETO,EGMIN
      ELSE IF(CONFIG.EQ.ETRON)THEN
         WRITE(OLUN,106)TEMIN,TGVETO,EEMIN
      ENDIF
      IF(RADCOR.EQ.HARD)THEN
         IF(CONFIG.EQ.EGAMMA)WRITE(OLUN,107)PEGMIN
         IF(CONFIG.NE.EGAMMA)WRITE(OLUN,108)PHVETO
         IF(CONFIG.EQ.GAMMA .AND.EEVETO.NE.0.D0)WRITE(OLUN,109)EEVETO
         IF(CONFIG.EQ.GAMMAE.AND.EEVETO.NE.0.D0)WRITE(OLUN,109)EEVETO
         IF(CONFIG.NE.EGAMMA.AND.EGVETO.NE.0.D0)WRITE(OLUN,110)EGVETO
         IF(EPS.NE.0.01D0)WRITE(OLUN,111)EPS
         IF(FRAPHI.GT.0.0D0)WRITE(OLUN,117)FRAPHI,EPSPHI
      ENDIF
      WRITE(OLUN,112)WGHT1M,W1MAX,WARN1
      IF(UNWGHT)THEN
         WRITE(OLUN,113)WGHTMX,WMAX,WARN
         WRITE(OLUN,114)NACC,NTRIAL,EFFIC,SIGE,ERSIGE
      ELSE
         WRITE(OLUN,115)WMAX
         WRITE(OLUN,116)NACC,NTRIAL,EFFIC,SIGE,ERSIGE
      ENDIF
      
      RESNGEN = NACC
      RESNTRIALS = NTRIAL
      RESEFF = EFFIC
      RESCROSS = SIGE
      RESCROSSERROR = ERSIGE     

 100  FORMAT(' '
     >,/,    ' TEEGG (7.2) - an e e gamma (gamma) event generator.'
     >,                                   ' Rad. correction: ',A4
     >,/,    ' Matrix element: ',A30,'      Configuration: ',A7
     >,/,/,  ' Initial seed =',I12,' next seed =',I12
     >,/,    ' Parameter: EB     = ',F7.3,' GeV')
 101  FORMAT(' Parameter: CUTOFF = ',F7.5,' GeV')
 102  FORMAT(' Parameter: TEVETO = ',F7.5,' rad')
 103  FORMAT(' Parameter: TEMIN  = ',F7.5,' rad'
     >,/,    ' Parameter: TGMIN  = ',F7.5,' rad'
     >,/,    ' Parameter: EEMIN  = ',F7.3,' GeV'
     >,/,    ' Parameter: EGMIN  = ',F7.3,' GeV')
 104  FORMAT(' Parameter: TGMIN  = ',F7.5,' rad'
     >,/,    ' Parameter: EGMIN  = ',F7.3,' GeV')
 105  FORMAT(' Parameter: TGMIN  = ',F7.5,' rad'
     >,/,    ' Parameter: TGVETO = ',F7.5,' rad'
     >,/,    ' Parameter: EGMIN  = ',F7.3,' GeV')
 106  FORMAT(' Parameter: TEMIN  = ',F7.5,' rad'
     >,/,    ' Parameter: TGVETO = ',F7.5,' rad'
     >,/,    ' Parameter: EEMIN  = ',F7.3,' GeV')
 107  FORMAT(' Parameter: PEGMIN = ',F7.5,' rad')
 108  FORMAT(' Parameter: PHVETO = ',F7.5,' rad')
 109  FORMAT(' Parameter: EEVETO = ',F7.4,' GeV')
 110  FORMAT(' Parameter: EGVETO = ',F7.4,' GeV')
 111  FORMAT(' Parameter: EPS    = ',F7.5,'    ')
 117  FORMAT(' Parameter: FRAPHI = ',F7.5,'    '
     >,/,    ' Parameter: EPSPHI = ',F7.6,'    ')
 112  FORMAT(' Parameter: WGHT1M = ',F7.3,' ; Observed maximum ',F7.3
     >,                                   ' ',A13)
 113  FORMAT(' Parameter: WGHTMX = ',F7.3,' ; Observed maximum ',F7.3
     >,                                   ' ',A13)
 114  FORMAT(' No. of events generated=',I7,','
     >,      ' No. of attempts=',I8,','
     >,      ' Efficiency=',F8.6
     >,/,    ' Total cross section =',E12.6,' +/- ',E12.6,' pb',/,/)
 115  FORMAT(' Weighted events generated   ; Observed maximum ',F7.3)
 116  FORMAT(' No. of weighted events =',I7,','
     >,      ' No. of attempts=',I8,','
     >,      ' Efficiency=',F8.6
     >,/,    ' Total cross section =',E12.6,' +/- ',E12.6,' pb',/,/)

      RETURN
      END
