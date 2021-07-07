
************************************************************************
CDECK  ID>, TEGSTD. 
************************************************************************

*.**********************************************************************
*.                                                                     *
*. Main program for the TEEGG generator (stand alone)                  *
*.                                                                     *
*.**********************************************************************

CDECK  ID>, TEEGGM. 
*.
*...TEEGGM   Main program for the stand alone running of TEEGG.
*.
*.  TEEGG can be run in stand alone mode by using the TEEGG EXEC
*.  (or TEEGG.COM on Vax) and supplying the subroutine TEEGGX.
*.  The same subroutine can be used for GOPAL running to generate
*.  the same events.
*.
*. SEQUENCE  : TEGCMX
*. CALLS     : TEEGGI TEEGGX TEEGGL TEEGG7 LPASS USOUT TEEGGP
*. CALLS     : TIMEX USLAST
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 1.1
*. CREATED   : 28-Sep-88
*. LAST MOD  : 01-May-89
*.
*. Modification Log.
*. 01-May-89   Dean Karlen  Stop multiple configurations if NXTPAR < 0
*.
*.**********************************************************************


      SUBROUTINE TEEGGM(MODE,XPAR,NPAR)

      IMPLICIT NONE

      INTEGER PLUN
      PARAMETER (PLUN=7)

      INTEGER I,PARNUM,NSET
      REAL TIMLFT,TIMUSD

      EXTERNAL TEEGGL,LPASS
      LOGICAL  TEEGGL,LPASS

      DOUBLE PRECISION EB,TEVETO,TEMIN,TGMIN,TGVETO,EEMIN,EGMIN
     >,                PEGMIN,EEVETO,EGVETO,PHVETO,CUTOFF,EPS
     >,                WGHT1M,WGHTMX, FRAPHI,EPSPHI
      INTEGER          ISEED, RADCOR,CONFIG,MATRIX,MTRXGG
      LOGICAL          UNWGHT
      
      COMMON/TINPAR/EB,TEVETO,TEMIN,TGMIN,TGVETO,EEMIN,EGMIN
     >,             PEGMIN,EEVETO,EGVETO,PHVETO,CUTOFF,EPS
     >,             WGHT1M,WGHTMX, FRAPHI,EPSPHI
     >,             ISEED, RADCOR,CONFIG,MATRIX,MTRXGG, UNWGHT


      INTEGER NEV,NXTPAR
      REAL XTRATM
      COMMON/TEGCMX/NEV,NXTPAR,XTRATM
      CHARACTER*70 TITLE
      COMMON/TEGCMT/TITLE
      
      ! C     TF, 2015
      INTEGER TNPHOT
      DOUBLE PRECISION TP1(0:3),TQ1(0:3),TP2(0:3),TQ2(0:3),TPHOT(0:1,0:3)
      COMMON / MOMSET / TP1,TQ1,TP2,TQ2,TPHOT,TNPHOT
      
      DOUBLE PRECISION W2
      COMMON / KINEMATICS / W2
      
      INTEGER MODE, NPAR(0:99)
      DOUBLE PRECISION XPAR(0:99)
      
      INTEGER RESNGEN, RESNTRIALS
      DOUBLE PRECISION RESEFF, RESCROSS, RESCROSSERROR, AVGQ2
      COMMON / TEEGGRESULTS / RESNGEN, RESNTRIALS, RESEFF, RESCROSS, RESCROSSERROR, AVGQ2
      
!       REAL REANSK, FULLANSK, REANSK2, FULLANSK2
      DOUBLE PRECISION TT, TW2, TWEIGHT,REANSK, FULLANSK, REANSK2, FULLANSK2
      COMMON / TEEGGEXTRA / TT, TW2, TWEIGHT, REANSK, FULLANSK, REANSK2, FULLANSK2

      DOUBLE PRECISION T,TP,SP,U,UP,X1,X2,Y1,Y2,DSIGE,WGHT,WGHTSF
      COMMON/TEVQUA/T,TP,SP,U,UP,X1,X2,Y1,Y2,DSIGE,WGHT,WGHTSF
      
      DOUBLE PRECISION SUMT
      
      INTEGER VACPOL
      COMMON/VACPOLPAR/VACPOL
      
      double complex ALPHANSK, ALPHANSKRES
      external ALPHANSK
      
      real*8 energy,
     &       vprehadsp, vprehadtm, vpimhad,
     &       vprelepsp, vpreleptm, vpimlep,
     &       vpretopsp, vpretoptm
      integer nrflag
      external vphlmntv2
      
      real*8 vpresult, vpresult2


C NEV    = Number of events to be generated.
C NXTPAR = Next set of parameters to be used. (for stand alone)
C XTRATM = extra time reqd. for ending run in secs. (for stand alone)
C TITLE  = event sample descriptor (for stand alone)

************************************************************************

!       WRITE(*,*) MODE
      
C Set defaults for TEEGG
      IF(MODE.LT.0) CALL TEEGGI
      IF(MODE.LT.0) SUMT=0

C Initialize any other parameters
      IF(MODE.LT.0) CALL TEEGGX(MODE,XPAR,NPAR)
      
      TITLE  = ' TEEGG '
 
C.....NEV =  number of events requested
      NEV =  1      

C Check that the parameters are valid.
      IF(MODE.LT.0.AND.(.NOT.TEEGGL(PLUN))) THEN
        STOP
      ENDIF
      
      IF((MODE.EQ.1))THEN

C Here is the Main loop.
C ---------------------

         DO 2 I=1,NEV

C Call the generating routine. (Generates one event).
 3          CALL TEEGG7
 
            IF(.NOT.LPASS(0))GOTO 3
            
C           Fill extra information            
            TT=T
            IF (UNWGHT) THEN
              TWEIGHT=1.0
            ELSE
	      TWEIGHT=WGHT
	    ENDIF
	      
            TW2=W2
            
C           Vacuum Polarization (TF)
	    vpresult = 1.0
            IF (VACPOL.EQ.42) THEN
              nrflag = 0
              call vphlmntv2(SQRT(-T),vprehadsp,vprehadtm,vpimhad,vprelepsp,vpreleptm,vpimlep,vpretopsp,vpretoptm,nrflag)
              vpresult=(1.d0/(1.d0  - vprehadtm - vpreleptm - vpretoptm))
            ELSEIF (VACPOL.EQ.43) THEN
	      vpresult = DBLE(ABS(ALPHANSK(-T, 1)))
            ENDIF
            vpresult2 = vpresult*vpresult
            TWEIGHT=TWEIGHT*vpresult2

            SUMT = SUMT+T

 2       CONTINUE

      ENDIF

      IF(MODE.EQ.2)THEN
	CALL USLAST
	AVGQ2 = SUMT/(RESNGEN*1d0)
      ENDIF
        
      END
