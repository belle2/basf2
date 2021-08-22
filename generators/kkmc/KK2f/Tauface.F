*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  !!!!!!! WARNING!!!!!   This source is agressive !!!!                           //
*//                                                                                 //
*//  Due to short common block names it  owerwrites variables in other parts        //
*//  of the code.                                                                   //
*//                                                                                 //
*//  One should add suffix c_Taul_ to names of all commons as soon as possible!!!!  //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////

*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Standard Tauola interface/initialization routines of functionality exactly    //
*//   as in Tauola CPC  but input is partially from xpar(*) matrix                  //
*//   ITAUXPAR is for indirect adressing                                            //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////


      SUBROUTINE INIETC(ITAUXPAR,xpar)
      include '../tauola/RChL-currents/parameter.inc'
      INCLUDE "BXformat.fi"
      REAL*8 xpar(*)
      INTEGER   INUT,IOUT
      COMMON /INOUT/  
     $     INUT,         ! Input  unit  number (not used)
     $     IOUT          ! Ounput unit  number
      COMMON / IDFC  / IDFF
      COMMON / TAURAD / XK0DEC,ITDKRC
      DOUBLE PRECISION            XK0DEC
      COMMON / JAKI   /  JAK1,JAK2,JAKP,JAKM,KTOM
* Note: I dont see KeyA1=2,3 realy implemented in the code SJ. ??????
      INTEGER  KeyA1
      COMMON /TESTA1/
     $     KeyA1           ! Special switch for tests of dGamma/dQ**2 in a1 decay
* KeyA1=1 constant width of a1 and rho
* KeyA1=2 free choice of rho propagator (defined in function FPIK)
*         and free choice of a1 mass and width. function g(Q**2)
*         (see formula 3.48 in Comp. Phys. Comm. 64 (1991) 275)
*         hard coded both in Monte Carlo and in testing distribution.
* KeyA1=3 function g(Q**2) hardcoded in the Monte Carlo
*         (it is timy to calculate!), but appropriately adjusted in testing distribution.
C SWITCHES FOR OPTIONS IN INITIALIZATION:
C     =======================================================================
C switch to initialize that all modes are represented equally.
      INTEGER IFEQUALBR
      DATA    IFEQUALBR /0/ 
      COMMON /IFEQUALBR/ IFEQUALBR
      
C     FF2PIRHO ! switch for the two pion form factor
C     = 0 CLEO parametrization
C     = 1 RChL parametrization (is same as = 0 in this implementation)
C     = 2, 3 Belle parametrization
C     pi- pi0 -> Two options of Belle parametrization (PRD 78, 2008, 072006)
C FF2PIRHO = 2  Belle parametrization (suggested default = 2)
C               all parameters par (1...11) of fit are free
C FF2PIRHO = 3  Belle parametrization,
C               parameters of fit are free
C     except for fixed par(1)=F_pi(0)=1
      DATA   FF2PIRHO /0/
      
C     pi- pi- pi+, pi- pi0 pi0 -> Resonance Chiral Lagrangian initialization (PRD 88, 2013, 093012)
C IRCHL3PI=0  TAUOLA CLEO COEF(I,J) =  COEFc(I,J)
C IRCLH3PI=1  TAUOLA RChL COEF(I,J) =  COEFr(I,J) [suggested default]
      INTEGER IRCHL3PI
      DATA    IRCHL3PI /0/
      COMMON /SETRCHL/ IRCHL3PI

      
! switch on initialization of FF mixing in tau- -> K-  PI- PI+ nu, PI- K0B PI0 nu 
C IFKPIPI = 0 CLEO initialization (suggested default)
C IFKPIPI = 1 BaBar initialization 
C-- Improve Form Factors for tau->K-pi-pi+nu 
C   New modelling using parameters from CLEO, PRD62, 072006 (2000):
C   M(1270) = 1.254 GeV, M(1400) = 1.463 GeV
C   G(1270) = 0.26 GeV,  G(1400) = 0.30 GeV
C   F1 = BW[K1(1270)] X BW[rho], 
C   F2 = (eta BW[K1(1270)]+BW[K1(1400)]) X BW[K*], 
C   eta=0.33 (M. Finkemeier, E. Mirkes, ZPC69, 243 (1996))
C Similar tuning also applied to tau- -> PI- K0B PI0 nu 
      INTEGER IFKPIPI
      DATA    IFKPIPI /0/
      COMMON /IFKPIPI/ IFKPIPI

C     pi- pi0 pi0 pi0, pi- pi- pi+ pi0:
C     IFCURR4PI = 0  CPC 146 2002, hep-ph/0201149 [Novosibirsk] [suggested default]
C     IFCURR4PI = 1  hep-ph/9410260 (probably) [Karlsruhe]
C     IFCURR4PI = 2  R. Decker, M. Finkemeier, P. Heiliger, H.H.Jonsson, TTP94-13
C     IFCURR4PI = 3  CLEO initialization
      INTEGER IFCURR4PI
      DATA    IFCURR4PI /0/
      COMMON /IFCURR4PI/ IFCURR4PI

      REAL    AMINVI            ! mass
      DATA    AMINVI /0/
      INTEGER ISPNVI            ! spin
      DATA    ISPNVI /0/
      REAL    VCINVI,ACINVI     ! coupling co-efficients in (A+ gamma5 B) term or (A + gamma_5 B) gamma-mu terms for Spin0/Spin1
      DATA    VCINVI /0.0/      ! suggested value: 0.5
      DATA    ACINVI /0.0/      ! suggested value: 0.5
      COMMON /AMINVI/ AMINVI,ISPNVI,VCINVI,ACINVI
C
C     DarkPhoton parameters: epsilon (kinematic mixing), mass and width
C     
      REAL*8  AMDARK, GMDARK, EPDARK
      DATA    AMDARK /0.0D0/
      DATA    GMDARK /0.0D0/
      DATA    EPDARK /1.0D0/
      COMMON /CMDARK/ AMDARK, GMDARK, EPDARK
      REAL*8 DARKWIDTH
      EXTERNAL DARKWIDTH
C     
C     Optional test flag to decay Lambda or not inside TAUOLA
C     [Note: LAMDEC=1 will reset its lifetime, so turn the flag should be turned off by default]   
      INTEGER LAMDEC
      DATA    LAMDEC /0/
      COMMON /LAMDEC/ LAMDEC
C
      SAVE
C-------------------------------------------------------------------------------
      idff    = xpar(ITAUXPAR+3) ! Lund identifier for first tau (15 for  tau-)
C XK0 for tau decays.
      xk0dec  = xpar(ITAUXPAR+5) ! IR-cut for QED rad. in leptonic decays
C radiative correction switch in tau --> e (mu) decays !
      itdkRC  = xpar(ITAUXPAR+4) ! QED rad. in leptonic decays
C switches of tau+ tau- decay modes !!
      Jak1            = xpar(ITAUXPAR+1) ! Decay Mask for first tau
      Jak2            = xpar(ITAUXPAR+2) ! Decay Mask for second tau
C output file number for TAUOLA
      IOUT    = xpar(4)
C  KeyA1 is used for formfactors actually not in use
      KeyA1   = xpar(ITAUXPAR+6) ! Type of a1 current

      WRITE(iout,bxope)
      WRITE(iout,bxtxt) ' Parameters passed from KK  to Tauola:   '
      WRITE(iout,bxl1i) Jak1,      'dec. type 1-st tau  ','Jak1  ','t01'
      WRITE(iout,bxl1i) Jak2,      'dec. type 2-nd tau  ','Jak2  ','t02'
      WRITE(iout,bxl1i) KeyA1,     'current type a1 dec.','KeyA1 ','t03'
      WRITE(iout,bxl1i) idff,      'PDG id 1-st tau     ','idff  ','t04'
      WRITE(iout,bxl1i) itdkRC,    'R.c. switch lept dec','itdkRC','t05'
      WRITE(iout,bxl1g) xk0dec,    'IR-cut for lept r.c.','xk0dec','t06'
      WRITE(iout,bxclo)

      IFEQUALBR = XPAR(ITAUXPAR+901)
      FF2PIRHO  = XPAR(ITAUXPAR+902)
      IRCHL3PI  = XPAR(ITAUXPAR+903)
      IFKPIPI   = XPAR(ITAUXPAR+904)
      IFCURR4PI = XPAR(ITAUXPAR+905)
      LAMDEC    = XPAR(ITAUXPAR+906)

C     Print functional flags
      WRITE(iout,*) 'FF2PIRHO, IRCHL3PI, IFKPIPI, IFCURR4PI = ',
     $               FF2PIRHO, IRCHL3PI, IFKPIPI, IFCURR4PI

      AMINVI    = XPAR(ITAUXPAR+911)
      ISPNVI    = INT(XPAR(ITAUXPAR+912))
      VCINVI    = XPAR(ITAUXPAR+913)
      ACINVI    = XPAR(ITAUXPAR+914)
C     Print flags for invisible particle
      IF (VCINVI.NE.0.0.OR.ACINVI.NE.0.0)
     $   WRITE(iout,*) 'AMINVI, ISPNVI, VCINVI, ACINVI = ',
     $                  AMINVI, ISPNVI, VCINVI, ACINVI
      
C     Print optional flags 
      WRITE(iout,*) 'IFEQUALBR, LAMDEC = ',
     $               IFEQUALBR, LAMDEC

      AMDARK = DBLE(XPAR(ITAUXPAR+907)) ! Note: unless mass > 0, epsilon = 1
      IF (AMDARK.GT.1.0D-6) THEN
         EPDARK = DBLE(XPAR(ITAUXPAR+908))
         GMDARK = DARKWIDTH(AMDARK,EPDARK)
      ENDIF
      WRITE(iout,*) 'AMDARK, GMDARK, EPDARK = ',AMDARK,GMDARK,EPDARK
      
      if(IRCHL3PI.eq.1) then
         CALL INIRChL(1)        ! (IRCHL)
      endif
      end

      SUBROUTINE INITDK(ITAUXPAR,xpar)
* ----------------------------------------------------------------------
*     INITIALISATION OF TAU DECAY PARAMETERS  and routines
*
*     called by : KORALZ
* ----------------------------------------------------------------------
      include '../tauola/TAUDCD_size.inc'
      INCLUDE "BXformat.fi"      
      INTEGER   INUT,IOUT
      COMMON /INOUT/  
     $     INUT,         ! Input  unit  number (not used)
     $     IOUT          ! Ounput unit  number
      REAL*8 xpar(*)

      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
*
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      REAL*8            GAMPRT,SUM
      COMMON / TAUBRA / GAMPRT(NBMC),JLIST(NBMC),NCHAN
      COMMON / TAUKLE / BRA1,BRK0,BRK0B,BRKS
      REAL*4            BRA1,BRK0,BRK0B,BRKS

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31

      CHARACTER OLDNAMES(7)*31
      CHARACTER*80 bxINIT
      PARAMETER (
     $  bxINIT ='(1x,1h*,g17.8,            16x, a31,a4,a4, 1x,1h*)'
     $ )
      REAL*4 PI,POL1(4)
*
*
* LIST OF BRANCHING RATIOS
CAM normalised to e nu nutau channel
CAM                  enu   munu   pinu  rhonu   A1nu   Knu    K*nu   pi
CAM   DATA JLIST  /    1,     2,     3,     4,     5,     6,     7,

*AM   DATA GAMPRT /1.000,0.9730,0.6054,1.2432,0.8432,0.0432,O.O811,0.616
*AM
*AM  multipion decays
*
*    conventions of particles names
*                 K-,P-,K+,  K0,P-,KB,  K-,P0,K0
*                  3, 1,-3  , 4, 1,-4  , 3, 2, 4  ,
*                 P0,P0,K-,  K-,P-,P+,  P-,KB,P0
*                  2, 2, 3  , 3, 1,-1  , 1,-4, 2  ,
*                 ET,P-,P0   P-,P0,GM
*                  9, 1, 2  , 1, 2, 8
*

C
      DIMENSION NOPIK(6,NMODE),NPIK(NMODE)
*AM   outgoing multiplicity and flavors of multi-pion /multi-K modes    
      DATA   NPIK  /                4,                    4,  
     1                              5,                    5,
     2                              6,                    6,
     3                              3,                    3,            
     4                              3,                    3,            
     5                              3,                    3,            
     6                              3,                    3,  
     7                              2                         /         

      DATA  NOPIK / -1,-1, 1, 2, 0, 0,     2, 2, 2,-1, 0, 0,  
     1              -1,-1, 1, 2, 2, 0,    -1,-1,-1, 1, 1, 0,  
     2              -1,-1,-1, 1, 1, 2,    -1,-1, 1, 2, 2, 2, 
     3              -3,-1, 3, 0, 0, 0,    -4,-1, 4, 0, 0, 0,  
     4              -3, 2,-4, 0, 0, 0,     2, 2,-3, 0, 0, 0,  
     5              -3,-1, 1, 0, 0, 0,    -1, 4, 2, 0, 0, 0,  
     6               9,-1, 2, 0, 0, 0,    -1, 2, 8, 0, 0, 0,


C AJWMOD fix sign bug, 2/22/99
     7              -3,-4, 0, 0, 0, 0                         /

* LIST OF BRANCHING RATIOS
      NCHAN = NMODE + 7
      DO 1 I = 1,NBMC
      IF (I.LE.NCHAN) THEN
        JLIST(I) = I

        IF(I.EQ. 1) GAMPRT(I) =0.1800 
        IF(I.EQ. 2) GAMPRT(I) =0.1751 
        IF(I.EQ. 3) GAMPRT(I) =0.1110 
        IF(I.EQ. 4) GAMPRT(I) =0.2515 
        IF(I.EQ. 5) GAMPRT(I) =0.1790 
        IF(I.EQ. 6) GAMPRT(I) =0.0071 
        IF(I.EQ. 7) GAMPRT(I) =0.0134
        IF(I.EQ. 8) GAMPRT(I) =0.0450
        IF(I.EQ. 9) GAMPRT(I) =0.0100
        IF(I.EQ.10) GAMPRT(I) =0.0009
        IF(I.EQ.11) GAMPRT(I) =0.0004 
        IF(I.EQ.12) GAMPRT(I) =0.0003 
        IF(I.EQ.13) GAMPRT(I) =0.0005 
        IF(I.EQ.14) GAMPRT(I) =0.0015 
        IF(I.EQ.15) GAMPRT(I) =0.0015 
        IF(I.EQ.16) GAMPRT(I) =0.0015 
        IF(I.EQ.17) GAMPRT(I) =0.0005
        IF(I.EQ.18) GAMPRT(I) =0.0050
        IF(I.EQ.19) GAMPRT(I) =0.0055
        IF(I.EQ.20) GAMPRT(I) =0.0017 
        IF(I.EQ.21) GAMPRT(I) =0.0013 
        IF(I.EQ.22) GAMPRT(I) =0.0010 

        IF(I.EQ. 1) OLDNAMES(I)='  TAU-  -->   E-               '
        IF(I.EQ. 2) OLDNAMES(I)='  TAU-  -->  MU-               '
        IF(I.EQ. 3) OLDNAMES(I)='  TAU-  -->  PI-               '
        IF(I.EQ. 4) OLDNAMES(I)='  TAU-  -->  PI-, PI0          '
        IF(I.EQ. 5) OLDNAMES(I)='  TAU-  -->  A1- (two subch)   '
        IF(I.EQ. 6) OLDNAMES(I)='  TAU-  -->   K-               '
        IF(I.EQ. 7) OLDNAMES(I)='  TAU-  -->  K*- (two subch)   '
        IF(I.EQ. 8) NAMES(I-7)='  TAU-  --> 2PI-,  PI0,  PI+   '
        IF(I.EQ. 9) NAMES(I-7)='  TAU-  --> 3PI0,        PI-   '
        IF(I.EQ.10) NAMES(I-7)='  TAU-  --> 2PI-,  PI+, 2PI0   '
        IF(I.EQ.11) NAMES(I-7)='  TAU-  --> 3PI-, 2PI+,        '
        IF(I.EQ.12) NAMES(I-7)='  TAU-  --> 3PI-, 2PI+,  PI0   '
        IF(I.EQ.13) NAMES(I-7)='  TAU-  --> 2PI-,  PI+, 3PI0   '
        IF(I.EQ.14) NAMES(I-7)='  TAU-  -->  K-, PI-,  K+      '
        IF(I.EQ.15) NAMES(I-7)='  TAU-  -->  K0, PI-, K0B      '

        IF(I.EQ.16) NAMES(I-7)='  TAU-  -->  K-,  K0, PI0      '

        IF(I.EQ.17) NAMES(I-7)='  TAU-  --> PI0  PI0   K-      '
        IF(I.EQ.18) NAMES(I-7)='  TAU-  -->  K-  PI-  PI+      '
        IF(I.EQ.19) NAMES(I-7)='  TAU-  --> PI-  K0B  PI0      '
        IF(I.EQ.20) NAMES(I-7)='  TAU-  --> ETA  PI-  PI0      '
        IF(I.EQ.21) NAMES(I-7)='  TAU-  --> PI-  PI0  GAM      '
        IF(I.EQ.22) NAMES(I-7)='  TAU-  -->  K-  K0            '
      ELSE
        JLIST(I) = 0
        GAMPRT(I) = 0.D0
      ENDIF
   1  CONTINUE
      DO I=1,NMODE
        MULPIK(I)=NPIK(I)
        DO J=1,MULPIK(I)
         IDFFIN(J,I)=NOPIK(J,I)
        ENDDO
      ENDDO
*
*
* --- COEFFICIENTS TO FIX RATIO OF:
* --- A1 3CHARGED/ A1 1CHARGED 2 NEUTRALS MATRIX ELEMENTS (MASLESS LIM.)
* --- PROBABILITY OF K0 TO BE KS
* --- PROBABILITY OF K0B TO BE KS
* --- RATIO OF COEFFICIENTS FOR K*--> K0 PI-
* --- ALL COEFFICENTS SHOULD BE IN THE RANGE (0.0,1.0)
* --- THEY MEANING IS PROBABILITY OF THE FIRST CHOICE ONLY IF ONE
* --- NEGLECTS MASS-PHASE SPACE EFFECTS
      BRA1=0.5
      BRK0=0.5
      BRK0B=0.5
      BRKS=0.6667
*

      GFERMI = 1.16637E-5
      CCABIB = 0.975
      GV     = 1.0
      GA     =-1.0



      GFERMI = xpar(32)
      IF (XPAR(ITAUXPAR+100+1).GT.-1D0) THEN
C initialization form KK
        CCABIB = XPAR(ITAUXPAR+7)
        GV     = XPAR(ITAUXPAR+8)
        GA     = XPAR(ITAUXPAR+9)

        BRA1   = XPAR(ITAUXPAR+10)
        BRKS   = XPAR(ITAUXPAR+11)
        BRK0   = XPAR(ITAUXPAR+12)
        BRK0B  = XPAR(ITAUXPAR+13)
        DO K=1,NCHAN
         GAMPRT(K)=XPAR(ITAUXPAR+100+K)
        ENDDO
      ENDIF
* ZW 13.04.89 HERE WAS AN ERROR
      SCABIB = SQRT(1.-CCABIB**2)
      PI =4.*ATAN(1.)
      GAMEL  = GFERMI**2*AMTAU**5/(192*PI**3)
*
*      CALL DEXAY(-1,pol1)
*
* PRINTOUTS FOR KK version

      SUM=0.0D0
      DO K=1,NCHAN
       SUM=SUM+GAMPRT(K)
      ENDDO

      
      WRITE(iout,bxope)
      WRITE(iout,bxtxt) ' TAUOLA Initialization SUBROUTINE INITDK:    '
      WRITE(iout,bxtxt) ' Adopted to read from KK                     '
      WRITE(iout,bxtxt) '                      '
      WRITE(iout,bxtxt) ' Choice Probability      --     Decay Channel'
      DO K=1,7      
      WRITE(iout,bxINIT) GAMPRT(K)/SUM,    OLDNAMES(K),'****','***'
      ENDDO
      DO K=8,7+NMODE      
      WRITE(iout,bxINIT) GAMPRT(K)/SUM,     NAMES(K-7),'****','***'
      ENDDO
      WRITE(iout,bxtxt) ' In addition:'
      WRITE(iout,bxINIT) GV,    'Vector W-tau-nu coupl.     ','****','***'
      WRITE(iout,bxINIT) GA,    'Axial  W-tau-nu coupl.     ','****','***'
      WRITE(iout,bxINIT) GFERMI,'Fermi Coupling             ','****','***'
      WRITE(iout,bxINIT) CCABIB,'cabibo angle               ','****','***'
      WRITE(iout,bxINIT) BRA1,  'a1 br ratio (massless)     ','****','***'
      WRITE(iout,bxINIT) BRKS,  'K* br ratio (massless)     ','****','***'
      WRITE(iout,bxclo)
            
      RETURN
      END

      SUBROUTINE INIPHY(XK00)
* ----------------------------------------------------------------------
*     INITIALISATION OF PARAMETERS
*     USED IN QED and/or GSW ROUTINES
* ----------------------------------------------------------------------
      COMMON / QEDPRM /ALFINV,ALFPI,XK0
      REAL*8           ALFINV,ALFPI,XK0
      REAL*8 PI8,XK00
*
      PI8    = 4.D0*DATAN(1.D0)
      ALFINV = 137.03604D0
      ALFPI  = 1D0/(ALFINV*PI8)
      XK0=XK00
      END

      SUBROUTINE INIMAS(ITAUXPAR,xpar)
* ----------------------------------------------------------------------
*     INITIALISATION OF MASSES
*
*     called by : KORALZ
* ----------------------------------------------------------------------
      include '../tauola/RChL-currents/parameter.inc'
      INCLUDE "BXformat.fi"
      INTEGER   INUT,IOUT
      COMMON /INOUT/  
     $     INUT,         ! Input  unit  number (not used)
     $     IOUT          ! Ounput unit  number
      REAL*8 xpar(*)
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
*     
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
*
      COMMON / PARMASD / AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
*
      REAL*8            AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD


      CHARACTER*80 bxINIT
      PARAMETER (
     $  bxINIT ='(1x,1h*,g17.8,            16x, a31,a4,a4, 1x,1h*)'
     $ )
C initialize rchl_parameters first
      CALL rchl_parameters(1)
*     
* IN-COMING / OUT-GOING  FERMION MASSES
      AMTAU  = xpar(656)
      AMNUTA = xpar(666) ! 0.010
      AMEL   = xpar(616)
      AMNUE  = 0.0
      AMMU   = xpar(636)
      AMNUMU = 0.0
*
* MASSES USED IN TAU DECAYS

      AMPIZ  = 0.134964
      AMPI   = 0.139568
      AMRO   = 0.77526
      GAMRO  = 0.1491
*C    GAMRO  = 0.666
      AMA1   = 1.251
      GAMA1  = 0.599
      AMK    = 0.493667
      AMKZ   = 0.497611 
      AMKST  = 0.89555
      GAMKST = 0.0473
C
C IN-COMING / OUT-GOING  FERMION MASSES
!!      AMNUTA = PKORB(1,2)
!!      AMNUE  = PKORB(1,4)
!!      AMNUMU = PKORB(1,6)
C
C MASSES USED IN TAU DECAYS  Cleo settings
!!      AMPIZ  = PKORB(1,7)
!!      AMPI   = PKORB(1,8)
!!      AMRO   = PKORB(1,9)
!!      GAMRO  = PKORB(2,9)
      AMA1   = 1.275   !! PKORB(1,10)
      GAMA1  = 0.615   !! PKORB(2,10)
!!      AMK    = PKORB(1,11)
!!      AMKZ   = PKORB(1,12)
!!      AMKST  = PKORB(1,13)
!!      GAMKST = PKORB(2,13)
C

c --- REPEAT INITIALIZATION FOR DOUBLE PRECISION COMMON BLOCK
      AMTAUD  = AMTAU
      AMNUTAD = AMNUTA
      AMELD   = AMEL
      AMNUED  = AMNUE
      AMMUD   = AMMU
      AMNUMUD = AMNUMU
*
* MASSES USED IN TAU DECAYS
      AMPIZD  = AMPIZ
      AMPID   = AMPI
      AMROD   = AMRO
      GAMROD  = GAMRO
      AMA1D   = AMA1
      GAMA1D  = GAMA1
      AMKD    = AMK
      AMKZD   = AMKZ
      AMKSTD  = AMKST
      GAMKSTD = GAMKST

            WRITE(iout,bxope)
      WRITE(iout,bxtxt) ' TAUOLA Initialization SUBROUTINE INIMAS:    '
      WRITE(iout,bxtxt) ' Adopted to read from KK                     '
      WRITE(iout,bxINIT)amtau,'AMTAU tau-mass             ','****','***'
      WRITE(iout,bxINIT)amel ,'AMEL  electron-mass        ','****','***'
      WRITE(iout,bxINIT)ammu ,'AMMU  muon-mass            ','****','***'
      WRITE(iout,bxclo)
      END
      SUBROUTINE CHOICE(MNUM,RR,ICHAN,PROB1,PROB2,PROB3,
     $            AMRX,GAMRX,AMRA,GAMRA,AMRB,GAMRB)
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
C      PRINT *, 'CHOICE : MNUM = ', MNUM 
C
      AMROP=1.1
      GAMROP=0.36
      AMOM=.782
      GAMOM=0.0084
C     XXXXA CORRESPOND TO S2 CHANNEL !
      IF(MNUM.EQ.0) THEN
       PROB1=0.5
       PROB2=0.5
       AMRX =AMA1
       GAMRX=GAMA1
       AMRA =AMRO
       GAMRA=GAMRO
       AMRB =AMRO
       GAMRB=GAMRO
      ELSEIF(MNUM.EQ.1) THEN ! TAU-  -->  K-, PI-,  K+
       PROB1=0.5
       PROB2=0.5
       AMRX =1.57
       GAMRX=0.9
       AMRB =AMKST
       GAMRB=GAMKST
       AMRA =AMRO
       GAMRA=GAMRO
      ELSEIF(MNUM.EQ.2) THEN ! TAU-  -->  K0, PI-, K0B
       PROB1=0.5
       PROB2=0.5
       AMRX =1.57
       GAMRX=0.9
       AMRB =AMKST
       GAMRB=GAMKST
       AMRA =AMRO
       GAMRA=GAMRO
      ELSEIF(MNUM.EQ.3) THEN ! TAU-  -->  K-,  PI0, K0
       PROB1=0.5
       PROB2=0.5
       AMRX =1.27
       GAMRX=0.3
       AMRA =AMKST
       GAMRA=GAMKST
       AMRB =AMKST
       GAMRB=GAMKST
      ELSEIF(MNUM.EQ.4) THEN ! TAU-  --> PI0  PI0   K-
       PROB1=0.5
       PROB2=0.5
       AMRX =1.27
       GAMRX=0.3
       AMRA =AMKST
       GAMRA=GAMKST
       AMRB =AMKST
       GAMRB=GAMKST
      ELSEIF(MNUM.EQ.5) THEN !  TAU-  -->  K-  PI-  PI+
       PROB1=0.5
       PROB2=0.5
       AMRX =1.27
       GAMRX=0.3
       AMRA =AMKST
       GAMRA=GAMKST
       AMRB =AMRO
       GAMRB=GAMRO
      ELSEIF(MNUM.EQ.6) THEN ! TAU-  --> PI-  K0B  PI0
       PROB1=0.4
       PROB2=0.4
       AMRX =1.27
       GAMRX=0.3
       AMRA =AMRO
       GAMRA=GAMRO
       AMRB =AMKST
       GAMRB=GAMKST
      ELSEIF(MNUM.EQ.7) THEN !  TAU-  --> ETA  PI-  PI0
       PROB1=0.0
       PROB2=1.0
       AMRX =1.27
       GAMRX=0.9
       AMRA =AMRO
       GAMRA=GAMRO
       AMRB =AMRO
       GAMRB=GAMRO
      ELSEIF(MNUM.EQ.8) THEN
       PROB1=0.0
       PROB2=1.0
       AMRX =AMROP
       GAMRX=GAMROP
       AMRB =AMOM
       GAMRB=GAMOM
       AMRA =AMRO
       GAMRA=GAMRO
      ELSEIF(MNUM.EQ.101) THEN
       PROB1=.35
       PROB2=.35
       AMRX =1.2
       GAMRX=.46
       AMRB =AMOM
       GAMRB=GAMOM
       AMRA =AMOM
       GAMRA=GAMOM
      ELSEIF(MNUM.EQ.102) THEN
       PROB1=0.0
       PROB2=0.0
       AMRX =1.4
       GAMRX=.6
       AMRB =AMOM
       GAMRB=GAMOM
       AMRA =AMOM
       GAMRA=GAMOM
      ELSE
       PROB1=0.0
       PROB2=0.0
       AMRX =AMA1
       GAMRX=GAMA1
       AMRA =AMRO
       GAMRA=GAMRO
       AMRB =AMRO
       GAMRB=GAMRO
      ENDIF
C
      IF    (RR.LE.PROB1) THEN
       ICHAN=1
      ELSEIF(RR.LE.(PROB1+PROB2)) THEN
       ICHAN=2
        AX   =AMRA
        GX   =GAMRA
        AMRA =AMRB
        GAMRA=GAMRB
        AMRB =AX
        GAMRB=GX
        PX   =PROB1
        PROB1=PROB2
        PROB2=PX
      ELSE
       ICHAN=3
      ENDIF
C
      PROB3=1.0-PROB1-PROB2
      END
      FUNCTION DCDMAS(IDENT,IRANGE)
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
*
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      INTEGER           IRANGE
      REAL              XRANGE,XRANGE1,XRANGE15,XRANGE2,XRANGE25,XRANGE3
      REAL              QQBRWG
      EXTERNAL          QQBRWG
      REAL              AMINVI            ! mass
      INTEGER           ISPNVI            ! spin
      REAL              VCINVI,ACINVI     ! coupling co-efficients in (A+ gamma5 B) term or (A + gamma_5 B) gamma-mu terms for Spin0/Spin1
      COMMON /AMINVI/   AMINVI,ISPNVI,VCINVI,ACINVI
C
      XRANGE=IRANGE
      XRANGE1=XRANGE
      IF (XRANGE1.GT.1.0)  XRANGE1=1.0
      XRANGE15=XRANGE
      IF (XRANGE15.GT.1.5) XRANGE15=1.5
      XRANGE2=XRANGE
      IF (XRANGE2.GT.2.0)  XRANGE2=2.0
      XRANGE25=XRANGE
      IF (XRANGE25.GT.2.5) XRANGE25=2.5
      XRANGE3=XRANGE
      IF (XRANGE3.GT.3.0)  XRANGE3=3.0
C
      IF      (IDENT.EQ. 1) THEN
        APKMAS=AMPI
      ELSEIF  (IDENT.EQ.-1) THEN
        APKMAS=AMPI
      ELSEIF  (IDENT.EQ. 2) THEN
        APKMAS=AMPIZ
      ELSEIF  (IDENT.EQ.-2) THEN
        APKMAS=AMPIZ
      ELSEIF  (IDENT.EQ. 3) THEN
        APKMAS=AMK
      ELSEIF  (IDENT.EQ.-3) THEN
        APKMAS=AMK
      ELSEIF  (IDENT.EQ. 4) THEN
        APKMAS=AMKZ
      ELSEIF  (IDENT.EQ.-4) THEN
        APKMAS=AMKZ
      ELSEIF  (IDENT.EQ. 8) THEN
        APKMAS=0.0001
      ELSEIF  (IDENT.EQ.-8) THEN
        APKMAS=0.0001
      ELSEIF  (IDENT.EQ. 9) THEN
        APKMAS=0.5479
      ELSEIF  (IDENT.EQ.-9) THEN
        APKMAS=0.5479
      ELSEIF  (ABS(IDENT).EQ.13) THEN
        APKMAS=AMMU
      ELSEIF  (ABS(IDENT).EQ.11) THEN
        APKMAS=AMEL
      ELSEIF  (ABS(IDENT).EQ.12) THEN
        APKMAS=0.0
      ELSEIF  (ABS(IDENT).EQ.14) THEN
        APKMAS=0.0
      ELSEIF  (ABS(IDENT).EQ.223) THEN
        APKMAS=QQBRWG(0.78265,0.00849,XRANGE)  ! omega(782)
      ELSEIF  (ABS(IDENT).EQ.333) THEN
        APKMAS=QQBRWG(1.019461,0.004249,XRANGE)!  phi(1020)
      ELSEIF  (ABS(IDENT).EQ.113) THEN
        APKMAS=QQBRWG(AMRO,GAMRO,XRANGE3)       ! rho0(770)
      ELSEIF  (ABS(IDENT).EQ.213) THEN
        APKMAS=QQBRWG(AMRO,GAMRO,XRANGE3)       ! rho+(770)
      ELSEIF  (ABS(IDENT).EQ.331) THEN
        APKMAS=QQBRWG(0.95778,0.000188,XRANGE) ! eta(prime)
      ELSEIF  (ABS(IDENT).EQ.22) THEN
        APKMAS=0.000                      ! gamma
      ELSEIF  (ABS(IDENT).EQ.2212) THEN
         APKMAS=0.9382720813              ! proton
      ELSEIF  (ABS(IDENT).EQ.311) THEN
         APKMAS=AMKZ                      ! K0
      ELSEIF  (ABS(IDENT).EQ.111) THEN
         APKMAS=AMPIZ                     ! pi0
      ELSEIF  (ABS(IDENT).EQ.211) THEN
         APKMAS=AMPI                      ! pi+-
      ELSEIF  (ABS(IDENT).EQ. 221) THEN
         APKMAS=0.5488                    ! eta
      ELSEIF  (ABS(IDENT).EQ. 321) THEN
         APKMAS=AMK                       ! K+-
      ELSEIF  (ABS(IDENT).EQ. 310) THEN
         APKMAS=AMKZ                      ! K0S
      ELSEIF  (ABS(IDENT).EQ. 130) THEN
         APKMAS=AMKZ                      ! K0L
      ELSEIF  (ABS(IDENT).EQ. 313) THEN
         APKMAS=QQBRWG(AMKST,GAMKST,XRANGE)! K*0(892)
      ELSEIF  (ABS(IDENT).EQ. 3122) THEN
         APKMAS=1.115683                   ! Lambda
      ELSEIF  (ABS(IDENT).EQ.10211) THEN
         APKMAS=QQBRWG(1.474,0.265,XRANGE1)     ! a0+(1450)
      ELSEIF  (ABS(IDENT).EQ. 10111) THEN
         APKMAS=QQBRWG(1.474,0.265,XRANGE1)     ! a00(1450) 
      ELSEIF  (ABS(IDENT).EQ.10213) THEN
         APKMAS=QQBRWG(1.2295,0.142,XRANGE2)    ! b1+(1235)
      ELSEIF  (ABS(IDENT).EQ. 20223) THEN
         APKMAS=QQBRWG(1.2819,0.0227,XRANGE1)   ! f1(1285)
      ELSEIF  (ABS(IDENT).EQ. 20213) THEN
         APKMAS=QQBRWG(1.230,0.250,XRANGE2)     ! a1+(1260)
      ELSEIF  (ABS(IDENT).EQ.9000211) THEN
         APKMAS=QQBRWG(0.980,0.045,XRANGE25)    ! a0+(980)
      ELSEIF  (ABS(IDENT).EQ.9000111) THEN
         APKMAS=QQBRWG(0.980,0.045,XRANGE25)    ! a00(980)
      ELSEIF  (ABS(IDENT).EQ.9010221) THEN
         APKMAS=QQBRWG(0.9856,0.0342,XRANGE)    ! f00(980)
      ELSEIF  (ABS(IDENT).EQ.94144) THEN
         APKMAS=AMINVI                          ! invi   
      ELSEIF  (ABS(IDENT).EQ.0) THEN 
         APKMAS=0
      ELSE
        PRINT *, 'STOP IN APKMAS, WRONG IDENT=',IDENT
        STOP
      ENDIF
      DCDMAS=APKMAS
      END

      FUNCTION LUNPIK(ID,ISGN)
      COMMON / TAUKLE / BRA1,BRK0,BRK0B,BRKS
      REAL*4            BRA1,BRK0,BRK0B,BRKS
      REAL*4 XIO(1)
      IDENT=ID*ISGN

      IF      (IDENT.EQ. 1) THEN
        IPKDEF=-211       ! pi-
      ELSEIF  (IDENT.EQ.-1) THEN
        IPKDEF= 211       ! pi+
      ELSEIF  (IDENT.EQ. 2) THEN
        IPKDEF=111        ! pi0
      ELSEIF  (IDENT.EQ.-2) THEN
        IPKDEF=111        ! pi0
      ELSEIF  (IDENT.EQ. 3) THEN
        IPKDEF=-321       ! K-
      ELSEIF  (IDENT.EQ.-3) THEN
        IPKDEF= 321       ! K+
        
      ELSEIF  (IDENT.EQ. 4) THEN
*
* K0 --> K0_LONG (IS 130) / K0_SHORT (IS 310) = 1/1
        CALL KKMC_RANMAR(XIO,1)
        IF (XIO(1).GT.BRK0) THEN
          IPKDEF= 130     ! K0L
        ELSE
          IPKDEF= 310     ! K0S
        ENDIF
      ELSEIF  (IDENT.EQ.-4) THEN
*
* K0B--> K0_LONG (IS 130) / K0_SHORT (IS 310) = 1/1
        CALL KKMC_RANMAR(XIO,1)
        IF (XIO(1).GT.BRK0B) THEN
          IPKDEF= 130     ! K0L
        ELSE
          IPKDEF= 310     ! K0S
        ENDIF
      ELSEIF  (IDENT.EQ. 8) THEN
        IPKDEF= 22        ! gamma
      ELSEIF  (IDENT.EQ.-8) THEN
        IPKDEF= 22        ! gamma
      ELSEIF  (IDENT.EQ. 9) THEN
        IPKDEF= 221       ! eta
      ELSEIF  (IDENT.EQ.-9) THEN
        IPKDEF= 221       ! eta
      ELSEIF  (ABS(IDENT).EQ.13) THEN
        IPKDEF=IDENT      ! 13:mu-, -13:mu+
      ELSEIF  (ABS(IDENT).EQ.11) THEN
        IPKDEF=IDENT      ! 11:e-,  -11:e+
      ELSEIF  (ABS(IDENT).EQ.12) THEN
        IPKDEF=IDENT      ! 12:nu_e -12:anti-nu_e
      ELSEIF  (ABS(IDENT).EQ.14) THEN
        IPKDEF=IDENT      ! 14:nu_mu -14:anti-nu_mu
      ELSEIF  (ABS(IDENT).EQ.223) THEN
        IPKDEF=ABS(IDENT) ! omega
      ELSEIF  (ABS(IDENT).EQ.333) THEN
        IPKDEF=ABS(IDENT) !phi
      ELSEIF  (ABS(IDENT).EQ.113) THEN
        IPKDEF=ABS(IDENT) ! rho0
      ELSEIF  (ABS(IDENT).EQ.213) THEN
        IPKDEF=IDENT      ! 213:rho+ -213:rho-
       ELSEIF  (ABS(IDENT).EQ.331) THEN
        IPKDEF=ABS(IDENT) ! eta(prime)
      ELSEIF  ((IDENT).EQ.22) THEN
        IPKDEF=IDENT      ! gamma
      ELSEIF  (ABS(IDENT).EQ.2212) THEN
        IPKDEF=IDENT      ! 2212:proton -2212:anti-proton
      ELSEIF  (ABS(IDENT).EQ.311) THEN
         IPKDEF=ABS(IDENT)! K0
      ELSEIF  (ABS(IDENT).EQ.111) THEN
         IPKDEF=ABS(IDENT)! pi0
      ELSEIF  (ABS(IDENT).EQ.211) THEN
         IPKDEF=IDENT     ! 211:pi+ -211:pi-
      ELSEIF  (ABS(IDENT).EQ.221) THEN
         IPKDEF=ABS(IDENT)! eta
      ELSEIF  (ABS(IDENT).EQ.321) THEN
         IPKDEF=IDENT     ! 321:K+ -321:K-
      ELSEIF  (ABS(IDENT).EQ.310) THEN
         IPKDEF=ABS(IDENT)!K0S
      ELSEIF  (ABS(IDENT).EQ.130) THEN
         IPKDEF=ABS(IDENT)! K0L
      ELSEIF  (ABS(IDENT).EQ.313) THEN
         IPKDEF=IDENT     ! K*0(892)
      ELSEIF  (ABS(IDENT).EQ.3122) THEN
         IPKDEF=IDENT     ! Lambda
      ELSEIF  (ABS(IDENT).EQ.10211) THEN
        IPKDEF=IDENT      ! 10211:a0+(1450) -10211:a0-(1450)
      ELSEIF  (ABS(IDENT).EQ.10111 ) THEN
         IPKDEF=ABS(IDENT)! a00(1450) 
      ELSEIF  (ABS(IDENT).EQ.10213) THEN
        IPKDEF=IDENT      ! 10213:b1+(1235) -10213:b1-(1235)
      ELSEIF  (ABS(IDENT).EQ.20223 ) THEN
         IPKDEF=ABS(IDENT)! f1(1285)
      ELSEIF  (ABS(IDENT).EQ.20213 ) THEN
         IPKDEF=IDENT     ! 20213:a1+(1260) -20213:a1-(1260)
      ELSEIF  (ABS(IDENT).EQ.9000211) THEN
         IPKDEF=IDENT     ! 9000211:a0+(980) -9000211:a0-(980)
      ELSEIF  (ABS(IDENT).EQ.9000111) THEN
         IPKDEF=ABS(IDENT)! a00(980)
      ELSEIF  (ABS(IDENT).EQ.9010221) THEN
         IPKDEF=ABS(IDENT)! f00(980)
      ELSEIF  (ABS(IDENT).EQ.94144) THEN
         IPKDEF=ABS(IDENT)! invi
      ELSE
        PRINT *, 'STOP IN IPKDEF, WRONG IDENT=',IDENT
        STOP
      ENDIF
      LUNPIK=IPKDEF
      END

      SUBROUTINE TAURDF(KTO)
C THIS ROUTINE CAN BE CALLED BEFORE ANY TAU+ OR TAU- EVENT IS GENERATED
C IT CAN BE USED TO GENERATE TAU+ AND TAU- SAMPLES OF DIFFERENT
C CONTENTS
      include '../tauola/TAUDCD_size.inc'
      COMMON / TAUKLE / BRA1,BRK0,BRK0B,BRKS
      REAL*4            BRA1,BRK0,BRK0B,BRKS
      REAL*8            GAMPRT
      COMMON / TAUBRA / GAMPRT(NBMC),JLIST(NBMC),NCHAN
      IF (KTO.EQ.1) THEN
C     ==================
C AJWMOD: Set the BRs for (A1+ -> rho+ pi0) and (K*+ -> K0 pi+)
      BRA1 = PKORB(4,1)
      BRKS = PKORB(4,3)
      BRK0  = PKORB(4,5)
      BRK0B  = PKORB(4,6)
      ELSE
C     ====
C AJWMOD: Set the BRs for (A1+ -> rho+ pi0) and (K*+ -> K0 pi+)
      BRA1 = PKORB(4,2)
      BRKS = PKORB(4,4)
      BRK0  = PKORB(4,5)
      BRK0B  = PKORB(4,6)
      ENDIF
C     =====
      END
      FUNCTION QQBRWG(CEN, FWID, XRANGE)
C  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
C   Generate exact Breit Wigner distribution with Mean=CEN, FWHM=FWID
C   between the limits XMIN, XMAX.
C 
C Adapted from /afs/cern.ch/lhcb/generator/QQ/qqlhcb/qqlib/qqbrwg.F (SwB Feb 2006)
C  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      IMPLICIT NONE
      REAL QQBRWG, CEN, FWID, XRANGE
      REAL XMIN, XMAX, RRR(1)
      REAL YMIN, YMAX
C  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      XMIN=CEN-XRANGE*FWID
      XMAX=CEN+XRANGE*FWID
      IF(FWID .EQ. 0.OR.XRANGE.EQ.0) THEN
        QQBRWG = CEN
      ELSE
        YMIN = ATAN(2.*(XMIN-CEN)/FWID)
        YMAX = ATAN(2.*(XMAX-CEN)/FWID)
        CALL KKMC_RANMAR(RRR,1)
        QQBRWG = CEN + FWID/2. * TAN(RRR(1)*(YMAX-YMIN)+YMIN)
        IF (QQBRWG.GT.XMAX) QQBRWG=XMAX
        IF (QQBRWG.LT.XMIN) QQBRWG=XMIN
      ENDIF
      RETURN
      END
      FUNCTION DARKWIDTH(AMDARK,EPDARK)
C  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      IMPLICIT NONE
      INTEGER I, IFIRST, II
      REAL*8 DARKWIDTH
      REAL*8 AMDARK             ! in GeV
      REAL*8 M(24),W(24)
      REAL*8 EPDARK
      DATA IFIRST/0/
      SAVE IFIRST, M, W
C     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      IF (IFIRST.EQ.0) THEN
C     A' mass
         M( 1) =  1.1 * 1D-3    ! GeV
         M( 2) =    3 * 1D-3    ! GeV
         M( 3) =    5 * 1D-3    ! GeV
         M( 4) =    7 * 1D-3    ! GeV
         M( 5) =   10 * 1D-3    ! GeV
         M( 6) =   15 * 1D-3    ! GeV
         M( 7) =   20 * 1D-3    ! GeV
         M( 8) =   30 * 1D-3    ! GeV
         M( 9) =   40 * 1D-3    ! GeV
         M(10) =   50 * 1D-3    ! GeV
         M(11) =   60 * 1D-3    ! GeV
         M(12) =   70 * 1D-3    ! GeV
         M(13) =   80 * 1D-3    ! GeV
         M(14) =   90 * 1D-3    ! GeV
         M(15) =  100 * 1D-3    ! GeV
         M(16) =  150 * 1D-3    ! GeV
         M(17) =  200 * 1D-3    ! GeV
         M(18) =  250 * 1D-3    ! GeV
         M(19) =  500 * 1D-3    ! GeV
         M(20) =  750 * 1D-3    ! GeV
         M(21) = 1000 * 1D-3    ! GeV
         M(22) = 1250 * 1D-3    ! GeV
         M(23) = 1500 * 1D-3    ! GeV
         M(24) = 1750 * 1D-3    ! GeV
C     A' Width
         W( 1) = 1.416700D-06   ! GeV
         W( 2) = 7.258800D-06   ! GeV
         W( 3) = 1.215400D-05   ! GeV
         W( 4) = 6.129800D-05   ! GeV
         W( 5) = 9.525400D-05   ! GeV
         W( 6) = 2.447360D-04   ! GeV
         W( 7) = 3.364170D-04   ! GeV
         W( 8) = 5.096410D-04   ! GeV
         W( 9) = 6.805850D-04   ! GeV
         W(10) = 8.510900D-04   ! GeV
         W(11) = 1.021460D-03   ! GeV
         W(12) = 1.191780D-03   ! GeV
         W(13) = 1.362080D-03   ! GeV
         W(14) = 1.532350D-03   ! GeV
         W(15) = 1.702640D-03   ! GeV
         W(16) = 2.554060D-03   ! GeV
         W(17) = 3.405280D-03   ! GeV
         W(18) = 6.123390D-03   ! GeV
         W(19) = 1.349763D-02   ! GeV
         W(20) = 2.041647D-02   ! GeV
         W(21) = 2.728802D-02   ! GeV
         W(22) = 3.414587D-02   ! GeV
         W(23) = 4.099777D-02   ! GeV
         W(24) = 5.000000D-02   ! GeV
C
         IFIRST = 1
      ENDIF
      II=0
      DO I = 1,24
         IF (AMDARK.GT.M(I)) THEN
            II = I
         ELSE
            GO TO 100
         ENDIF
      ENDDO
 100  CONTINUE
      IF (II.EQ.0) THEN
         DARKWIDTH = 1D-3
      ELSE IF (II.GT.0.AND.II.LT.24) THEN
        DARKWIDTH=W(II)+((AMDARK-M(II))/(M(II+1)-M(II)))*(W(II+1)-W(II))
      ELSE
         DARKWIDTH = 5.0D-2
      ENDIF
      IF (DARKWIDTH.LT.1.0D-3) DARKWIDTH = 1D-3
      IF (EPDARK.EQ.-1.0D0.AND.AMDARK.GT.1000D-3) THEN
         DARKWIDTH = DARKWIDTH/10.
         EPDARK = 1.0D0
      ELSEIF (EPDARK.EQ.-1.0D1.AND.AMDARK.GT.1000D-3) THEN
         DARKWIDTH = DARKWIDTH/100.
         EPDARK = 1.0D0
      ENDIF
      RETURN
      END
