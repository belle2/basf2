CDECK  ID>, BLANKDEK.   
*CMZ :  3.21/02 29/03/94  15.41.18  by  S.Giani
*-- Author :
CDECK  ID>, TEGCBC. 
************************************************************************
CDECK  ID>, TEGPAT. 
************************************************************************
*.
*.    -----------
*.    Version 7.2
*.    -----------
*.
*.    Dean Karlen
*.    Carleton University, Ottawa, Canada
*.    April 1989
*.
*.**********************************************************************
*.*                                                                    *
*.* TEEGG - A monte carlo event generator to simulate the process,     *
*.*                                                                    *
*.*          +  -      +  -                                            *
*.*         e  e  --> e  e  gamma (gamma)    for t-channel dominated   *
*.*                                          configurations.           *
*.*                                                                    *
*.* Reference: D. Karlen, Nucl. Phys. B289 (1987) 23                   *
*.*                                                                    *
*.* > The three different configurations with their input parameters:  *
*.*    1) e-gamma (Both an electron and gamma are 'seen'):             *
*.*       TEVETO - maximum theta of the 'missing' electron             *
*.*       TEMIN  - minimum theta of the 'seen' electron  |TEMIN and/or *
*.*       TGMIN  - minimum theta of the 'seen' gamma     | TGMIN>TEVETO*
*.*       EEMIN  - minimum energy of the 'seen' electron               *
*.*       EGMIN  - minimum energy of the 'seen' gamma                  *
*.*    2) electron (Only the electron is 'seen'):                      *
*.*       TEVETO - maximum theta of the 'missing' electron             *
*.*       TEMIN  - minimum theta of the 'seen' electron   |TEMIN>TEVETO*
*.*       TGVETO - maximum theta of the 'missing' gamma(s)|TEMIN>TGVETO*
*.*       EEMIN  - minimum energy of the 'seen' electron               *
*.*    3) gamma (Only the gamma is 'seen'):                            *
*.*       TEVETO - maximum theta of the 'missing' electrons            *
*.*       TGMIN  - minimum theta of the 'seen' gamma       TGMIN>TEVETO*
*.*       EGMIN  - minimum energy of the 'seen' gamma                  *
*.*                                                                    *
*.* > The fourth order process is divided into two regions:            *
*.*    1) SOFT PART - second gamma has energy < CUTOFF (in CM frame)   *
*.*                 - only one photon is generated                     *
*.*    2) HARD PART - second gamma has energy > CUTOFF (in CM frame)   *
*.*   The two regions must be generated separately.                    *
*.*                                                                    *
*.**********************************************************************
*.
*. Changes since Version 7.0:
*.
*.    -----------
*.    Version 7.2   April 1989, Carleton Univesity
*.    -----------
*.
*. o redefine the definition of phi-ks in cm frame for 4 body final
*.   state. Change the generation of this angle to include the peak
*.   at phi-ks=pi when refered to phi_k. This introduces two new
*.   parameters:
*.     FRAPHI - the fraction of trial events generated with the
*.              phi_ks peak.
*.     EPSPHI - the cutoff variable in the phi_ks peak distribution
*.   This change was made to eliminate the large weights sometimes
*.   observed for special configurations (GAMMAE type) in version 7.1.
*.   The default, FRAPHI=0., turns off this new peaked generation.
*. o replace the calculation of DILOG by the CERNLIB version.
*.
*.    -----------
*.    Version 7.1   June 1987, Stanford Linear Accelerator Center
*.    -----------
*.
*. o form of 3rd order EPA cross section put in terms of invariants
*. o add Martinez/Miquel matrix element for e-e-g-g (very CPU intensive)
*. o allow generation of weighted events (useful for looking at
*.   distributions and calculating cross sections for many acceptances).
*. o new configuration: GAMMAE, 4th order single photon with a (soft)
*.   electron in the acceptance.
*. o new MTRXGG: HEEGG, a hybrid of EPADC and BEEGG
*.
*.The program consists of the following:
*.
*.User callable routines:
*.    TEEGGI,TEEGGL,TEEGG7,TEEGGC,TEEGGP,TEEGGA
*.Internal routines:
*.    T3BODY,T4BODY,FQPEG,DMORK,SPENCE,TBOORT,BEEGGC,BCOLL,BEEGGM,TPRD
*.    INMART,MEEGGC
*.Routines supplied by Martinez/Miquel:
*.    ELEMAT,AMTOTM,AMPLIM,ZMART,SPININ
*.
*.How to Use This Program With Another Monte Carlo System.
*.--------------------------------------------------------
*.
*.1) Initialization
*.   --------------
*. - Call TEEGGI to set the defaults for all the parameters.
*. - modify parameters as desired
*. - Initialize the random number by: CALL RNDIN(ISEED)
*. - Call the logical function, TEEGGL(OLUN). This checks the validity
*.   of the parameters and calculates some constants to be used later.
*.   Returns .TRUE. if parameters are valid; .FALSE. if not. Use
*.   OLUN to specify the logical unit number for error messages to be
*.   printed. TEEGGL must be called before any new generation of events.
*. > Input parameters are found in common TINPAR
*. > Constants calculated by TEEGGL are in common TCONST
*.
*.2) Generation
*.   ----------
*. - Call TEEGG7 to generate one event.
*. > Event information (4-vectors) are found in common TEVENT
*.
*.3) End of generation
*.   -----------------
*. - Call TEEGGC to calculate efficiencies and total cross section from
*.   the last event sample generated.
*. - Call TEEGGP(OLUN) to print out a summary header of the parameters
*.   and results from the last event generation (on unit OLUN).
*. > Summary information filled by TEEGGC is found in common TSMMRY
*.
*.Routines That Must be Provided (site dependant):
*.------------------------------------------------
*.
*.SUBROUTINE RNDGEN(NRND)
*. - fill the REAL*4 array, RND(i) (i=1,NRND) (in COMMON TRND) with
*.   pseudo random numbers equidistributed between 0 and 1.
*. - optionally modify the varaibles in common TRND as follows:
*.   - fill SEED with the seed required to generate the current set of
*.     random numbers
*.   - fill NXSEED with the seed required to generate the next set of
*.     random numbers
*.
*.SUBROUTINE RNDIN(JSEED)
*. - initialize random number generator with INTEGER*4 seed JSEED
*. - optionally:
*.   - fill NXSEED with JSEED. (Seed required to generate the next set
*.     of random numbers)
*.
*.Unweighted vs. weighted events
*.------------------------------
*.
*.An unweighted event sample is the easiest to use, as it represents the
*.results from a hypothetical experiment.
*.
*.If an integrated cross section with a more complicated acceptance than
*.can be specified with the input parameters is desired (eg. a pt cut),
*.greater efficiency can be obtained using weighted events.
*.This form of generation is specified by setting UNWGHT=.FALSE.
*.The routine, TEEGGA, can be used in this integration, as follows.
*.After an event is generated, call TEEGGA(var1,var2,sig,ersig) if it
*.passes the complicated acceptance. The variables, var1 and var2
*.are REAL*8 variables that are used to accumulate the total weights and
*.should be set to 0 before event generation. By using other pairs of
*.variables, many acceptances can be calculated at once. After the event
*.generation, and after TEEGGC is called, call TEEGGA once again and the
*.total cross section and error is returned in sig and ersig.
*.The use of weighted events in histogramming is more efficient also.
*.The variable WGHT in common TEVENT specifies the event weight.

CDECK  ID>, TEEGGI. 
*.
*...TEEGGI   initilizes parameters for TEEGG.
*.
*. SEQUENCE  : TEGCOM
*. CALLS     : RNDGEN
*. CALLED    : USINIT,TEEGM
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
C SUBROUTINE TEEGGI : Sets default values for the parameters.
C-----------------------------------------------------------------------

      SUBROUTINE TEEGGI
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


C.....EB = energy of e beam (in GeV)
      EB = 14.5 D0

C.....RADCOR= HARD  generate e+ e- gamma gamma
C           = SOFT  e+ e- gamma(with soft brem and virtual corrections)
C           = NONE  e+ e- gamma(according to lowest order diagrams only)
      RADCOR= NONE

C.....CONFIG= EGAMMA  then generate according to e-gamma configuration
C           = ETRON   then generate according to single electron config.
C           = GAMMA   then generate according to single gamma config.
C           = GAMMAE  then generate accrdng to sngl gamma/soft e config.
      CONFIG= EGAMMA

C.....MATRIX = BK    then use Berends & Kleiss matrix element
C            = BKM2  then use    "        "      "    " with m**2/t term
C            = TCHAN then use t channel only matrix element (/w m term)
C            = EPA   then use matrix element from EPA (for testing)
      MATRIX = BKM2

C.....MTRXGG = EPADC then use EPA with double compton (RADCOR=HARD only)
C            = BEEGG then use Berends et al. e-e-g-g  (RADCOR=HARD only)
C            = MEEGG then use Martinez/Miquel e-e-g-g (RADCOR=HARD only)
C            = HEEGG then use EPA for low Q**2, BEEGG otherwise ("   " )
      MTRXGG = EPADC

C.....TEVETO = maximum theta of 'missing' e's in final state (in radians
      TEVETO = 100.D-3

C.....TEMIN = minimum angle between the 'seen' electron and beam line
C             (used for e-gamma & etron configurations)     (in radians)
      TEMIN = ACOS(.75D0)

C.....TGMIN = minimum angle between the 1st gamma and the beam line
C             (used for the e-gamma & gamma configurations) (in radians)
      TGMIN = ACOS(.75D0)

C.....TGVETO = maximum angle between the 'missing' 1st gamma & beam line
C             (only used for etron configuration)           (in radians)
C             (also used in for the gamma configuration to veto 2nd g)
      TGVETO = 50.D-3

C.....EEMIN = minimum energy of the 'observed' electron (in GeV)
C             (used for the e-gamma & electron configurations)
      EEMIN = 2.00 D0

C.....EGMIN = minimum energy of the 'observed' 1st photon (in GeV)
C             (used for the e-gamma & gamma configurations)
      EGMIN = 2.00 D0

C.....PEGMIN= minimum separation of e and gamma in phi (in radians)
C             (used for egamma configuration with hard rad. correction)
      PEGMIN= PI/4.D0

C.....EEVETO= energy of electron required to act as a veto (in GeV)
C             (used for gamma configuration with hard rad. correction)
      EEVETO= 0.0 D0

C.....EGVETO= energy of gamma required to act as a veto (in GeV)
C             (used for etron&gamma configs with hard rad. correction)
      EGVETO= 0.0 D0

C.....PHVETO= separation of two particles in phi reqd. to act as a veto
C             (used for e-g separation in etron config. and g-gs separ.
C             in gamma config... all for hard rod. correction only)
      PHVETO= PI/4.D0

C.....CUTOFF = CM cutoff energy for radiative correction (in GeV)
      CUTOFF = 0.25 D0

C.....WGHT1M = maximum weight for generation of QP0 & cos(theta QP)
      WGHT1M = 1.001 D0

C.....WGHTMX = maximum weight of the trial events
      WGHTMX = 1.00 D0

C.....EPS = arbitrary small parameter, used to stabilize weights.
C           It determines the relative sampling of ks^pbc vs. ks^0; if
C           large weights occur due to very hard 2nd photon,decrease EPS
      EPS = 0.01 D0

C.....FRAPHI = defines the fraction of trial events generated with the
C              peaked distribution for phi_ks. This is useful to reduce
C              large weights for configurations like GAMMAE. By default,
C              phi_ks is generated only with a flat distribution.
      FRAPHI = 0.0D0

C.....EPSPHI = small parameter that determines the 'cutoff' of the
C              peaked distribution for phi_ks.
      EPSPHI = 1.0 D-4

C.....ISEED = initial seed
      ISEED = 123456789

C Initialize random number generator
      CALL  RNDIN(ISEED)

C.....UNWGHT = logical variable, specifies if unweighted events are reqd
C            = TRUE  Then events are unweighted (each event weight=1)
C            = FALSE Then events are weighted(event weight given by WGHT
      UNWGHT = .TRUE.

      RETURN
      END
