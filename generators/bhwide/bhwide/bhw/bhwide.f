      SUBROUTINE BHWIDE(MODE,XPAR,NPAR)        
*     *********************************
!                                                                      !
!   BBBBBBB   BBB   BBB  BBB       BBB  BBB  BBBBBBB    BBBBBBB        !
!   BBB  BBB  BBB   BBB  BBB       BBB  BBB  BBB  BBB   BBB            !
!   BBB  BBB  BBB   BBB  BBB       BBB  BBB  BBB   BBB  BBB            !
!   BBBBBBB   BBBBBBBBB  BBB       BBB  BBB  BBB   BBB  BBBBBB         !
!   BBBBBBB   BBBBBBBBB  BBB   B   BBB  BBB  BBB   BBB  BBBBBB         !
!   BBB  BBB  BBB   BBB  BBB  BBB  BBB  BBB  BBB   BBB  BBB            !
!   BBBBBBBB  BBB   BBB  BBBBBB BBBBBB  BBB  BBB  BBB   BBB            !
!   BBBBBBB   BBB   BBB  BBBBB    BBBB  BBB  BBBBBBB    BBBBBBB        !
!                                                                      !
!======================================================================!
!======================================================================!
!======================================================================!
!===============             B H W I D E            ===================!
!======================================================================!
!======================================================================!
!=============== MONTE CARLO FOR WIDE-ANGLE BHABHA ====================!
!===============            VERSION 1.05            ===================!
!======================================================================!
!======================  December  2010     = =========================!
!======================================================================!
!======================================================================!
!=======================     AUTHORS      =============================!
!============  S. Jadach, W. Placzek, B.F.L. Ward  ====================!
!======================================================================!
!======================================================================!
!= Last modification: 08.12.2010  ================== by: WP ===========!
!======================================================================!
!                                                                      !
! BHWIDE is based on the following papers:                             !
! [1] S. Jadach, W. Placzek and B.F.L. Ward,                           !
!     Phys. Lett. B390 (1997) 298;                                     !
!     UTHEP-95-1001 (Oct. 1995); hep-ph/9608412.                       !
! [2] S. Jadach and B.F.L. Ward,                                       !
!     Phys. Rev. D40 (1989) 3582.                                      !
! [3] S. Jadach, E. Richter-Was, B.F.L. Ward  and Z. Was,              !
!     Comput. Phys. Commun. 70 (1992) 305; TH-6230, Sept. 1991.        !
! [4] M. Bohm, A. Denner and W. Hollik,                                !
!     Nucl. Phys. B304 (1988) 687.                                     !
! [5] F.A. Berends, R. Kleiss and W. Hollik,                           !
!     Nucl. Phys. B304 (1988) 712.                                     !
! [6] W. Beenakker, F.A. Berends and S.C. van der Marck,               !
!     Nucl. Phys. B349 (1991) 323.                                     !  
!                                                                      !
!                  IMPORTANT NOTE                                      !
!                  --------------                                      !
! The user is kindly requested to cite at least ref. [1].              !
!----------------------------------------------------------------------! 
!                        INPUT and OUTPUT                              !
!----------------------------------------------------------------------! 
! All input and output goes through parameters in                      !
!                 CALL BHWIDE(MODE,XPAR,NPAR)                          !
! and through /MOMSET/ and /WGTALL/ common blocks.                     ! 
! In the following we shall  briefly indicate the meaning of the       !
! above parameters/variables.                                          !
!                                                                      !
! IF( MODE =-1 ) THEN                                                  !
! ===================                                                  !
! Initialization is performed, all input parameters are transfered     !
! through XPAR and NPAR.                                               !
!----------------------------------------------------------------------!
!  Entry    Variable   Meaning                                         !
!----------------------------------------------------------------------! 
!  NPAR( 1)  KeyOpt = 1000*KeyZof +100*KeyCha +10*KeyWgt +KeyRnd       !
!                    General option switch                             !
!            KeyZof =0,1: Z contribution switched ON/OFF               !
!            KeyCha =0,1,2: channel choice: s+t, s-only, t-only        !
!            KeyRnd =1,2: type of random number generator RANMAR,RANECU!
!            KeyWgt =0,1: for constant, variable weight WTMOD          !
!  NPAR( 2)  KeyRad =1000*KeyEWC +100*KeyLib +10*KeyMod + KeyPia       !
!                    is option switch for ElectroWeak Radiative Corr.; !
!                    see tables in BHWID1 for more details.            !
!  XPAR( 1)  CMSENE Total center mass energy [GeV]                     !
!  XPAR( 2)         see tables in BHWID1,                              !
!  XPAR( 3)         see tables in BHWID1,                              !
!  XPAR( 4)         see tables in BHWID1,                              !
!  XPAR( 5)         see tables in BHWID1,                              !
!  XPAR( 6)         see tables in BHWID1,                              !
!  XPAR( 7)         see tables in BHWID1,                              !
!  XPAR( 8)         see tables in BHWID1,                              !
!  XPAR( 9)         see tables in BHWID1,                              !
!  XPAR(10)         see tables in BHWID1,                              !
!  XPAR(11)         see tables in BHWID1,                              !
!  XPAR(12)         see tables in BHWID1,                              !
!  XPAR(13)         see tables in BHWID1,                              !
!  XPAR(14)         see tables in BHWID1,                              !
!  XPAR(15)         see tables in BHWID1,                              !
!----------------------------------------------------------------------!
!                                                                      !
! ELSE IF( MODE = 0 ) THEN                                             !
! ========================                                             !
! Generation of the single Monte Carlo event                           !
! The four momenta of the final state electron positron and photon     !
! are encoded in                                                       !
!      COMMON / MOMSET / P1(4),Q1(4),P2(4),Q2(4),PHOT(100,4),NPHOT     !
! where P1 and Q1 are four-momenta of positron and elecron beams,      !
! P2 and Q2 are four-momenta of outgoing positron and electron,        !
! PHOT(100,4) contains list of photon four-momenta                     !
! and NPHOT is the number of real photons in PHOT.                     !
! NOTE: +z axis points along incoming e+.                              !
!                                                                      !
! For weighted events it may be profitable to use "paralel weights"    !
! from                                                                 !
!      COMMON / WGTALL / WTMOD,WTCRUD,WTTRIG,WTSET(300)                !
! where WTMOD is the principal model weight and another useful weights !
! representing some interesting version of the matrix element          !
! can be constructed as WT= WTCRU1*WTCRU2*WTSET(J).                    !
!                                                                      !
! ELSE IF( MODE = 1 ) THEN                                             !
! ========================                                             !
! The total cross section corresponding to generated series of event,  !
! i.e. resulting from MC integrartion is calculated and stored in XPAR !
! and NPAR.                                                            !
! In the table below we describe their most essential entries.         !
!----------------------------------------------------------------------!
!  Entry    Variable   Meaning                                         !
!----------------------------------------------------------------------!
!  NPAR(10)  NEVGEN  Number of generated MC events                     !
!  NPAR(20)  NEVGEN  Number of generated MC events                     !
!  XPAR(10)    XSEC  Total x-section [pb]                              !
!  XPAR(11)   RXSEC  The relative (statistical) error of XSEC          !
!  XPAR(20)          Crude total MC x-section [pb] which is necessary  !
!                    for rescaling histograms in run                   !
!                    with weighted events.                             !
!  XPAR(21)          =0, error of XPAR(20) is zero                     !
!----------------------------------------------------------------------!
! For constant weight option KEYWGT=0 (convevience in rescaling histos)!
! we put XPAR(20,21)=XPAR(10,11)                                       !
! For MODE=1 program is called upon many times in the process of       !
! rescaling histograms and therefore no output is printed.             !
!                                                                      !
! ELSE IF( MODE = 2 ) THEN                                             !
! ========================                                             !
! Only in this MODE=2 in addition to filling XPAR and NPAR             !
! (as for MODE=1)                                                      !
! the values of various x-sections are printed on the standard         !
! output file.                                                         !
!                                                                      !
! ENDIF                                                                !
! ====                                                                 !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8(A-H,O-Z)   
      CHARACTER*80    BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G 
      PARAMETER(
     $BXOPE =  '(//1X,15(5H=====)    )',
     $BXTXT =  '(1X,1H=,                  A48,25X,    1H=)',
     $BXL1I =  '(1X,1H=,I17,                 16X, A20,A12,A7, 1X,1H=)',
     $BXL1F =  '(1X,1H=,F17.8,               16X, A20,A12,A7, 1X,1H=)',
     $BXL2F =  '(1X,1H=,F17.8, 4H  +-, F11.8, 1X, A20,A12,A7, 1X,1H=)',
     $BXL1G =  '(1X,1H=,G17.8,               16X, A20,A12,A7, 1X,1H=)',
     $BXL2G =  '(1X,1H=,G17.8, 4H  +-, F11.8, 1X, A20,A12,A7, 1X,1H=)',
     $BXCLO =  '(1X,15(5H=====)/   )'    )   
      DIMENSION  XPAR(*),NPAR(*)
      COMMON / INOUT  / NINP,NOUT    
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      SAVE   / INOUT  /, / BHPAR3 /
      SAVE   NEVG
!-- Data
      CHARACTER*64 Logo(40)
      DATA Logo /
     &'***************************************************************',
     &'***************************************************************',
     &'* BBBBBBB   BBB   BBB  BBB       BBB  BBB  BBBBBBB    BBBBBBB *',      
     &'* BBB  BBB  BBB   BBB  BBB       BBB  BBB  BBB  BBB   BBB     *',      
     &'* BBB  BBB  BBB   BBB  BBB       BBB  BBB  BBB   BBB  BBB     *',       
     &'* BBBBBBB   BBBBBBBBB  BBB       BBB  BBB  BBB   BBB  BBBBBB  *',       
     &'* BBBBBBB   BBBBBBBBB  BBB   B   BBB  BBB  BBB   BBB  BBBBBB  *',       
     &'* BBB  BBB  BBB   BBB  BBB  BBB  BBB  BBB  BBB   BBB  BBB     *',       
     &'* BBBBBBBB  BBB   BBB  BBBBBB BBBBBB  BBB  BBB  BBB   BBB     *',       
     &'* BBBBBBB   BBB   BBB  BBBBB    BBBB  BBB  BBBBBBB    BBBBBBB *',       
     &'***************************************************************',
     &'*     MC Event Generator for Wide-Angle Bhabha Scattering     *',
     &'*                    BHWIDE version 1.05                      *',
     &'***************************************************************',
     &'*********************** December 2010 *************************',
     &'***************************************************************',
     &'*              Last modification: 08.12.2010                  *',
     &'***************************************************************',
     &'*                        AUTHORS:                             *',
     &'*          S. Jadach              (Stanislaw.Jadach@cern.ch)  *',
     &'*          W. Placzek             (Wieslaw.Placzek@cern.ch)   *',
     &'*          B.F.L. Ward            (bflw@slac.stanford.edu)    *',
     &'***************************************************************',
     &'*                        PAPERS:                              *',
     &'* [1] S. Jadach, W. Placzek, B.F.L. Ward,                     *',
     &'*     Phys. Lett. B390 (1997) 298; hep-ph/9608412.            *',
     &'***************************************************************',
     &'*                        HISTORY:                             *',
     &'* (1) September 1995:  version 1.00                           *',
     &'* (2) September 1996:  version 1.01                           *',
     &'* (3) February  2001:  version 1.02                           *',
     &'* (4) March     2001:  version 1.03                           *',
     &'* (5) May       2001:  version 1.04                           *',
     &'* (6) December  2010:  version 1.05                           *',
     &'***************************************************************',
     &'*                          WWW:                               *',
     &'*  http://cern.ch/placzek/bhwide/                             *',
     &'***************************************************************',
     &'***************************************************************',
     &' '/ 
   
      IF (MODE.EQ.-1) THEN        
        NINP = 15   
        NOUT = 16
! Print-out logo
        WRITE(6,   '(8X,A)') Logo
        WRITE(NOUT,'(8X,A)') Logo
!
        NEVG = 0 
        CALL BHWID1(-1,XPAR,NPAR) 
      ELSEIF (MODE.EQ.0) THEN
        NEVG = NEVG + 1  
        CALL BHWID1( 0,XPAR,NPAR)
! Clean final state common blocks if necessary (safety reason)
        CALL BHCLEN
      ELSE 
        CALL BHWID1(MODE,XPAR,NPAR)
      ENDIF 
      END

      SUBROUTINE BHCLEN
*     *****************
! This routine prevents user from using zero weight events 
! and parallel weights when they should not be used!
*     ************************
      IMPLICIT REAL*8(A-H,O-Z)   
      COMMON / MOMSET / P1(4),Q1(4),P2(4),Q2(4),PHOT(100,4),NPHOT
      COMMON / WGTALL / WTMOD,WTCRUD,WTTRIG,WTSET(300) 
      SAVE   / MOMSET /, / WGTALL /

! Parallel weights should not be used for constant weight events.
      IF(WTMOD.EQ.1D0) THEN
        DO I = 1,300
          WTSET(I) = 0D0
        ENDDO
! Clean final state momenta for events outside phase space 
      ELSEIF(WTCRUD*WTTRIG.EQ.0D0)  THEN
        DO K = 1,4
          P2(K)=0D0
          Q2(K)=0D0
        ENDDO
        NPHOT=0
        DO J = 1,100
          DO K = 1,4
            PHOT(J,K) = 0D0
          ENDDO
        ENDDO
      ENDIF
      END

