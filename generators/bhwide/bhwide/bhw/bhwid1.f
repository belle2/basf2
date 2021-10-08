      SUBROUTINE BHWID1(MODE,XPAR,NPAR)  
*     *********************************  
!----------------------------------------------------------------------!
!          **************************************************          !
!          *       **********************************       *          !
!          *       *      *******************       *       *          !
!          *       *      *                 *       *       *          !
!          *       *      *   B H W I D 1   *       *       *          !
!          *       *      *                 *       *       *          !
!          *       *      *******************       *       *          !
!          *       **********************************       *          !
!          **************************************************          !
!======================================================================!
!=======================     AUTHORS      =============================!
!============   S. Jadach, W. Placzek and B.F.L. Ward  ================!
!===================  VERSION 1.05 December 2010  =====================!
!======================================================================!
!                                                                      !
! Main subprogram in MC multiphoton generator for Bhabha scattering.   !
! It is multiphoton generator with Yennie-Frautschi-Suura first        !
! order exponentiation based on refs. [1-3].                           !
! Electroweak virtual and soft photon correction are taken from        !
! the program BABAMC [4,5] or the program ALIBABA [6] (the latter      !
! is recommended in this version).                                     !
! [1] S. Jadach, W. Placzek and B.F.L. Ward, UTHEP-95-1001 (Oct. 1995) !
!     hep-ph/9608412; Phys. Lett. B390 (1997) 298.                     !
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
!----------------------------------------------------------------------!
!                 INPUT and OUTPUT of BHWID1                           !
!----------------------------------------------------------------------!
! All input and output goes through parameters in                      !
!                 CALL BHWID1(MODE,XPAR,NPAR)                          !
! and through /MOMSET/ and /WGTALL/ common blocks.                     !
! In the following we shall  briefly indicate the meaning of the       !
! above parameters/variables.                                          !
!                                                                      !
! IF( MODE =-1 ) THEN                                                  !
! ===================                                                  !
! Initialization is performed, all input parameters are transfered     !
! through XPAR and NPAR.                                               !
! In the following table we indicate the meaning of NPAR, XPAR.        !
!                                                                      !
!      Table,           Input parameters of BHWID1                     !
!----------------------------------------------------------------------! 
!  Entry    Variable   Meaning                                         !
!----------------------------------------------------------------------!
!  NPAR( 1)  KeyOpt = 1000*KeyZof +100*KeyCha +10*KeyWgt +KeyRnd       !
!                    General option switch:                            !
!            KeyZof - Z-contribution switch-off:                       !
!                   =0 Z-ON (DEFAULT)                                  !
!                   =1 Z-OFF (for tests!)                              !
!            KeyCha - channel choice:                                  !
!                   =0 both s and t-channels + interferences (DEFAULT) !
!                   =1 s-channel only (for tests!)                     !
!                   =2 t-channel only (for tests!)                     !
!            KeyWgt - switch for constant, variable weight WTMOD:      !
!                   =0 WTMOD=1 useful for apparatus Monte Carlo,       !
!                      To make it more efficient the user may need     !
!                      to adjust a value of WTMAX (input parameter)    !
!                   =1 WTMOD varying, faster/safer, RECOMMENDED        !
!            KeyRnd =1,2 type of random number generator RANMAR,RANECU !
!  NPAR( 2) =KeyRad =1000*KeyEWC +100*KeyLib +10*KeyMod +KeyPia        !
!                    is option switch for ElectroWeak Radiative Corr.  !
!            KeyEWC - switching ON/OFF weak corrections:               !
!                   =0 only QED corrections included                   !
!                      (here both KeyLib =1,2 should be equivalent)    !
!                   =1 all ElectroWeak Corrections included            !
!            KeyLib - option for ElectroWeak Corrections Library:      !
!                   =1 ElectroWeak Corr. from BABAMC (obsolete)        !
!                   =2 ElectroWeak Corr. from ALIBABA, RECOMMENDED     !   
!            KeyMod - type of MODEL subprogram and QED matrix element  !
!                     for hard bremsstrahlung:                         !
!                   =1 obtained by the authors (helicity amplitudes)   !
!                   =2 from CALKUL, Nucl. Phys. B206 (1982) 61.        ! 
!                      Checked to be in a very good agreement!         !
!            KeyPia - photon vacuum polarization switch:               ! 
!                   =0 OFF,                                            !
!                   =1 ON, Burkhardt et.al. 1989, as in BHLUMI 2.0x    !
!                   =2 ON, S. Eidelman, F. Jegerlehner, Z.Phys.C(1995) !
!                   =3 ON, Burkhardt and Pietrzyk 1995 (Moriond).      !
!                   NOTE: Now it works for both ALIBABA and BABAMC     !
!                         rutines as well as for KeyEWC=0!             !
!  XPAR( 1) =CMSENE Total center mass energy [GeV]                     !
!  XPAR( 2) =THMINP Minimum scattering angle [deg] for positron        !
!  XPAR( 3) =THMAXP Maximum scattering angle [deg] for positron        !
!  XPAR( 4) =THMINE Minimum scattering angle [deg] for electron        !
!  XPAR( 5) =THMAXE Maximum scattering angle [deg] for electron        !
!  XPAR( 6) =ENMINP Minimum energy [GeV] for final state positron      ! 
!  XPAR( 7) =ENMINE Minimum energy [GeV] for final state electron      !
!  XPAR( 8) =ACOLLI Maximum acollinearity [deg] of final state e+e-    !
!  XPAR( 9) =EPSCMS Dimensionless infrared cut on CMS energy of soft   !
!                   photons, ( E_phot > CMSENE*EPSCMS/2 )              !
!  XPAR(10) =WTMAX  Maximum Weight for rejection (if <= 0, it is reset !
!                   inside the program)                                !    
!  >>> IMPOTRANT NOTE: In the case of generating UNWEIGHTED events,    !
!      the user should check in the OUTPUT if the number of            !
!      OVERWEIGHTED events and/or the fraction of the corresponding    !
!      cross section is ACCEPTABLE for his/her desired PRECISION.      !
!      If not, the value of WTMAX has to be adjusted (increased)       !
!      accordingly (making a histogram of the weight might be helpful).!    
!  XPAR(11) =AMAZ   Z mass  [GeV]                                      !
!  XPAR(12) =GAMMZ  Z width [GeV]  (may be recalculated by EW library) !
!  XPAR(13) =SINW2  sin^2(theta_W) (may be recalculated by EW library) !
!  XPAR(14) =AMTOP  top quark mass  [GeV]                              !
!  XPAR(15) =AMHIG  Higgs     mass  [GeV]                              !
! Note: All the above angular limits are given with respect to the     !
!       corresponding incoming particle (positron or electron, resp.). !   
!                                                                      !
! Some other parameters (like other fermion masses, alpha_s) are       !
! set up in the EW libraries (alibaba.f, babamc.f).                    !
! Note: In some cases the user may also need to adjust transfer limits !
!       (trmin and/or trmid) in routine FILBHW.                        ! 
!----------------------------------------------------------------------! 
!                                                                      !
! ELSE IF( MODE = 0 ) THEN                                             !
! ========================                                             !
! Generation of the single Monte Carlo event.                          !
! The four momenta of the final state electron, positron and photon    !
! and of real photons are encoded in                                   !
!      COMMON / MOMSET / P1(4),Q1(4),P2(4),Q2(4),PHOT(100,4),NPHOT     !
! where P1 and Q1 are four-momenta of positron and elecron beams.      !
! P2 and Q2 are four-momenta of outgoing positron and electron.        !
! The list PHOT(100,4) four-momenta contains                           !
! NPHOT four-momenta of real the photons, all in GeV.                  !
! NOTE: +z axis points along incoming e+.                              !
!                                                                      !
! The principal weight WTMOD of the event is placed in:                !
!      COMMON / WGTALL / WTMOD,WTCRUD,WTTRIG,WTSET(300)                !
! It is often of interest to use "paralel weights" from WTSET.         !
! The event weight is constructed then as WT= WTCRUD*WTTRIG*WTSET(J).  !
! Which J is alowed and what version of the QED matrix element         !
! it represents is summarized in the table below.                      !
! (Note that using "parallel weights" makes only sense for calculation !
! with variable weights, KEYWGT=1.)                                    !
! WTTRIG is a "trigger" weight (0 or 1) calculated in routine trigMC   ! 
! (see below) for acceptances defined by the input parameters. It is   !
! evaluated before the model weights WTSET(J) are calculated           !
! (the latter is done only for WTTRIG=1 which increases the efficiency ! 
! of the program).                                                     !
! To avoid unnecessary crashes of the program while using the parallel !
! weights the kinematical calculations should be protected by an       !
! appriopriate conditional statement:                                  !
! IF (WTCRUD*WTTRIG.NE.0D0) THEN ... ENDIF                             !
!                                                                      !
!              Table of WTSETS entries for BHWID1                      !
!----------------------------------------------------------------------!
!  Entry      Type of QED calculation in MODEL1                        !
!----------------------------------------------------------------------!
!  WTSET(10)    O(alpha^0)exp (YFS exponentiated)                      !
!  WTSET(11)    O(alhpa^1)exp (YFS exponentiated)                      !
!  WTSET(20)    O(alpha^0)    (non-exponentiated)                      !
!  WTSET(21)    O(alhpa^1)    (non-exponentiated)                      ! 
! -> Extra:                                                            !  
!  WTSET(31)    O(alhpa^1)exp pure QED cor. for KeyLib=2 and KeyEWC=1  !
!----------------------------------------------------------------------!
! Principal weight from MODEL1 (used to calculate WTMOD) is WTSET( 1). !
! The best and default set-up for MODEL1 is:                           !
!              WTSET( 1) = WTSET(11)                                   !
!----------------------------------------------------------------------!
!  Entry      Type of QED calculation in MODEL2                        !
!----------------------------------------------------------------------!
!  WTSET(110)    O(alpha^0)exp (YFS exponentiated)                     !
!  WTSET(111)    O(alhpa^1)exp (YFS exponentiated)                     !
!  WTSET(120)    O(alpha^0)    (non-exponentiated)                     !
!  WTSET(121)    O(alhpa^1)    (non-exponentiated)                     ! 
! -> Extra:                                                            !  
!  WTSET(131)    O(alhpa^1)exp pure QED cor. for KeyLib=2 and KeyEWC=1 !
!----------------------------------------------------------------------!
! Principal weight from MODEL2 (used to calculate WTMOD) is WTSET(101).!
! The best and default set-up for MODEL2 is:                           !
!              WTSET(101) = WTSET(111)                                 !
!                                                                      !
! ELSE IF( MODE = 1 ) THEN                                             !
! ========================                                             !
! The total cross section corresponding to generated series of event,  !
! i.e. resulting from MC integrartion is calculated and stored in XPAR !
! and NPAR, see table below.                                           !
!----------------------------------------------------------------------! 
!  Entry    Variable   Meaning                                         !
!----------------------------------------------------------------------! 
!  NPAR(10)  NEVGEN  Number of generated MC events                     !
!  NPAR(20)  IEVENT  Number of generated MC events                     !
!  XPAR(10)   XMCPB  Total x-section [pb]                              !
!  XPAR(11)    EREL  The relative error of XPAR(10)                    !
!  XPAR(12)     XMC  Total x-section [GEV**(-2)]                       !
!  XPAR(20)  SIG0PB  Crude total MC x-section [pb] which is necessary  !
!                    for rescaling histograms in run with              !
!                    weighted events.                                  !
!  XPAR(21)          =0, error of XPAR(20) is zero                     !
!  XPAR(20)    SIG0  Crude x-section as XPAR(20) but in [GeV**(-2)]    !
!----------------------------------------------------------------------!
! For constant weight option KEYWGT=0 (convevience in rescaling histos)!
! we put XPAR(20,21,22)=XPAR(10,11,12)                                 !   
! For MODE=1 program is called upon many times in the process of       !
! rescaling histograms, therefore, there is no output printed          !
! in this mode.                                                        !
!                                                                      !
! ELSE IF( MODE = 2 ) THEN                                             !
! ========================                                             !
! Only in this MODE=2 in addition to filling XPAR and NPAR as for      !
! MODE=1 the values of various x-sections are printed on standard      !
! output file.                                                         !
!                                                                      !
! ENDIF                                                                !
! ====                                                                 !
!----------------------------------------------------------------------!
! Last update: 08.12.2010                   by: W. Placzek             !
!======================================================================!
      IMPLICIT REAL*8(A-H,O-Z)   
      PARAMETER( PI = 3.1415926535897932D0, ALFINV = 137.0359895D0)
      PARAMETER( ALFPI=  1D0/PI/ALFINV ,ALFA=1D0/ALFINV)
      PARAMETER( GNANOB=389.385D-30*1.D33 )
      CHARACTER*80   BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G 
      PARAMETER(
     $BXOPE =  '(//1X,15(5H=====)    )',
     $BXTXT =  '(1X,1H=,                  A48,25X,    1H=)',
     $BXL1I =  '(1X,1H=,I17,                 16X, A20,A12,A7, 1X,1H=)',
     $BXL1F =  '(1X,1H=,F17.8,               16X, A20,A12,A7, 1X,1H=)',
     $BXL2F =  '(1X,1H=,F17.8, 4H  +-, F11.8, 1X, A20,A12,A7, 1X,1H=)',
     $BXL1G =  '(1X,1H=,G17.8,               16X, A20,A12,A7, 1X,1H=)',
     $BXL2G =  '(1X,1H=,G17.8, 4H  +-, F11.8, 1X, A20,A12,A7, 1X,1H=)',
     $BXCLO =  '(1X,15(5H=====)/   )'    )   
      DIMENSION  XPAR(*), NPAR(*)                    
      COMMON / BHPAR1 / DEL,EPSCMS,THMIN,XMIVIS        
      COMMON / BHPAR2 / CMSENE,AMEL       
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHPARZ / AMAZ,GAMMZ,SINW2,GV,GA
      COMMON / BHCRUD / trmid,crufla,Zprof,sg01,sg02,sg03,sig0
      COMMON / TRANSR / TRAN,TRMIN,TRMAX
      COMMON / MOMS1  / TRANP,P1(4),P2(4),PHOT1(100,4),PHSU1(4),NPHOT1
      COMMON / MOMS2  / TRANQ,Q1(4),Q2(4),PHOT2(100,4),PHSU2(4),NPHOT2
      COMMON / MOMZ1  / AL1(50),BE1(50),WTM1(50),MK1(50)
      COMMON / MOMZ2  / AL2(50),BE2(50),WTM2(50),MK2(50)
      COMMON / MOMSET / PX1(4),QX1(4),PX2(4),QX2(4),PHOT(100,4),NPHOT
      COMMON / WGTALL / WTMOD,WTCRUD,WTTRIG,WTSET(300)        
      COMMON / WGTSUP / WKP,WKQ,WTT1,WTT2,FPHS,FYFSU,FYFSD,WT3
      COMMON / INOUT  / NINP,NOUT 
      SAVE   / BHPAR1 /, / BHPAR2 /, / BHPAR3 /, / TRANSR / 
      SAVE   / MOMS1  /, / MOMS2  /, / MOMZ1  /, / MOMZ2 /, / MOMSET /
      SAVE   / WGTALL /, / WGTSUP /, / INOUT  /, / BHPARZ/, / BHCRUD /
      SAVE   SVAR, WTMAX, TRMX2, EMIN, IDA,IDC, KEYWGT, KEYREM, KEYUPD
      SAVE   IDGEN, NEVGEN, IEVENT, SIG0PB
      REAL*8 DRVEC(100)
!    
      IF(MODE.EQ.-1) THEN        
*     ===================
! Set input parameters
      CALL FILBHW(XPAR,NPAR)       
      SVAR  = CMSENE**2      
! Switches/keys
      KeyOpt = NPAR(1)
      KeyCha = MOD(KeyOpt,1000)/100   
! Calculate total crude x-section
      IF (KeyCha.NE.2) THEN
         Zprof = (GV**2+GA**2)**2 *SVAR**2
     $        /( (SVAR-AMAZ**2)**2 + (AMAZ*GAMMZ)**2 )
      ELSE
         Zprof = 0d0
      ENDIF
      crufla= Zprof + (SVAR/trmid)**2
      sg01  = 4*PI*ALFA**2 *Zprof*(TRMAX-trmid)/SVAR**2
      sg02  = 4*PI*ALFA**2 *(1/trmid - 1/TRMAX)
      sg03  = 4*PI*ALFA**2 *crufla*(trmid-TRMIN)/SVAR**2
      sig0  = sg01 + sg02 + sg03
      SIG0PB= sig0*GNANOB *1d3
! Maximum weight 
      WTMAX  = XPAR(10)
! Important histo which remembers total x-section 
      IDGEN = 1
      CALL GMONIT(  -1, IDGEN,0D0,SIG0PB*WTMAX,1D0)          
!-- maximum transfer for photon angular distributions
!-- TRMX2=svar is a safe choice, for low  angles 
!-- (thmin,thmax << 10mrad)
!-- it can be lowered to speed up generation (but with great care). 
!--      TRMX2 = TRMAX
! (over)conservative and safe choice is TRMX2=SVAR 
      TRMX2 = SVAR
!--      IF(TRMX2.GT.SVAR) TRMX2=SVAR
      EMIN = CMSENE/2D0*EPSCMS
! WEIGHT MONITORING INITIALIZATION
      IDA=50
      DO 11 I=IDA+1,IDA+20
  11  CALL GMONIT(  -1, I,0D0,1D0,1D0)          
      IDC = 90
      DO 12 I=IDC+1,IDC+9
  12  CALL GMONIT(  -1, I,0D0,1D0,1D0)  
      CALL GBOOK1(9001,' Total weight distribution $',60,-1D0,11D0)
      KEYWGT = MOD(KEYOPT,100)/10   
      KEYREM = MOD(KEYOPT,1000)/100   
      KEYUPD = MOD(KEYRAD,1000)/100   
      IEVENT=0  
      NEVGEN=0
      ELSEIF(MODE.EQ.0) THEN     
*     ======================     
      NEVGEN = NEVGEN+1
  200 CONTINUE
      IEVENT=IEVENT+1
      WT1    =0
      WT2    =0
      WT3    =0
      WT4    =0
      WT5    =0
      WT6    =0
      WTKIN  =0
      WTCRUD =0
      WTTRIG =0
!--------- Generate t-channel transfer (true one) ---------
      CALL gentra(TRMIN,TRMX2,TRAN)
!--------------------  Photon generation ------------------
      CALL MLTIBR(TRAN,TRMX2,AMEL,DEL,
     $            NPHOT1,PHOT1,PHSU1,AL1,BE1,TRANP,AMSP,MK1,WKP,WTM1)
      CALL MLTIBR(TRAN,TRMX2,AMEL,DEL,
     $            NPHOT2,PHOT2,PHSU2,AL2,BE2,TRANQ,AMSQ,MK2,WKQ,WTM2)
      IF(WKP*WKQ.EQ.0D0) GOTO 140
!-- Construct fermions, transform photons and fermions to CMS frame
      CALL KINO4(SVAR,TRAN,AMEL,AMSP,AMSQ,WT3)
      CALL GMONIT(   0,IDA+3,  WT3, 1D0,5D-4) 
      IF(WT3.EQ.0D0) GOTO 140
!-- Beyond this point only events conserving four-momentum !!!
      WTKIN=1D0  
!-- Manipulations on mass weights, removal of soft photons
      CALL PIATEK(CMSENE,TRMX2,AMEL,EMIN,DEL,
     $      NPHOT1,P1,P2,PHOT1,PHSU1,WTM1,WTT1,WTMR1,WCTA1,WCTB1)
      CALL PIATEK(CMSENE,TRMX2,AMEL,EMIN,DEL,
     $      NPHOT2,Q1,Q2,PHOT2,PHSU2,WTM2,WTT2,WTMR2,WCTA2,WCTB2)
!-- Removing photons < EPSCMS from the record 
!-- Mass weight WTMR1,2 is product of mass weights for ENE>EminCM times
!-- Average weight for photons with  ENE<EminCM.
      CALL REMPHO(EMIN,NPHOT1,PHOT1,P2,AL1,BE1,WTM1,MK1)
      CALL REMPHO(EMIN,NPHOT2,PHOT2,Q2,AL2,BE2,WTM2,MK2)
!---------- monitoring control weights
      CALL GMONIT(   0,IDC+1,       WCTA1, 1D0,5D-4)   
      CALL GMONIT(   0,IDC+2,       WCTA2, 1D0,5D-4)   
      CALL GMONIT(   0,IDC+3, WCTA1*WCTA2, 1D0,5D-4)        
      CALL GMONIT(   0,IDC+4,       WCTB1, 1D0,5D-4)   
      CALL GMONIT(   0,IDC+5,       WCTB2, 1D0,5D-4)   
      CALL GMONIT(   0,IDC+6, WCTB1*WCTB2, 1D0,5D-4)        
      WTM1T2 = WTMR1*WTMR2
      CALL GMONIT(   0,IDA+1,      WTM1T2,  2D0,5D-4) 
      WT1 = WTMR1*WKP
      WT2 = WTMR2*WKQ 
!-- Merge photons/fermion into one common block
      CALL MERGIK
!-- Crude weight before calculating a trigger weight 
      WTCRUD = WT1*WT2*WT3
!-- M.C. trigger weight
      CALL trigMC(WTTRIG)
      IF (WTTRIG.EQ.0D0) GOTO 140
!----------------- YFS FORMFACTOR ----------------------  
! Crude
      fPHS  = EXP( 4*ALFPI*LOG(TRMX2/AMEL**2)*LOG(1/DEL) )
! Exact
      pdel = DEL*BCUD(P1,P2,PHSU1)
      qdel = DEL*BCUD(Q1,Q2,PHSU2) 
      Blogp = LOG(TRANP/AMEL**2)
      Blogq = LOG(TRANQ/AMEL**2) 
      fYFSu = EXP( ALFPI*( -2*(Blogp -1)*LOG(1/pdel) +0.5*Blogp -1 ) )
      fYFSd = EXP( ALFPI*( -2*(Blogq -1)*LOG(1/qdel) +0.5*Blogq -1 ) )
      fYFSr = YFSfsu(EPSCMS)
      fYFS  = fYFSu*fYFSd*fYFSr
! Weight 
      WT4 = fYFS*fPHS
      CALL GMONIT(   0,IDA+4,WT4,  1D0,5D-4)  
!-- Restoring up-down interference
      CALL WTinte(WT5)
      CALL GMONIT(   0,IDA+5,WT5,  1D0,5D-4)
!-- Crude weight before including a model weight
      WTCRUD = WTCRUD*WT4*WT5 
!---------------------- MODEL ----------------------------------
      CALL MODEL(1,WT6)
 140  CONTINUE
!-- Total weight
      WT  = WTCRUD*WTTRIG*WT6
!-- Monitoring model weight       
      CALL GMONIT(   0,IDA+20,WT,WTMAX,RN)
      WTOVR = MAX(0D0,WT-WTMAX)
      CALL GMONIT(   0,IDA+18,  WTOVR,0D0,0D0)
      WTNEG = MIN(WT,0D0)
      CALL GMONIT(   0,IDA+19,  WTNEG,0D0,0D0)
      CALL GF1(9001,WT,1D0)
! ...Rejection according to principal weight  
      IF (KEYWGT.EQ.0) THEN  
        IF (WT.EQ.0D0) GOTO 200
! ...unweihgted events with WT=1 
        CALL VARRAN(DRVEC,1)
        RN = DRVEC(1)
        IF(WT.LT.RN*WTMAX) GOTO 200
        WTMOD = 1 
! ...WTCRUD, WTTRIG  weights are RESET to zero for control
        WTCRUD = 0
        WTTRIG = 0
      ELSE 
! ...weighted events  
        WTMOD  = WT
        CALL GMONIT(  0, IDGEN, SIG0PB, WTMAX,1D0)
      ENDIF
      ELSE 
*     ==== 
      NPAR(10)= NEVGEN  
      NPAR(20)= NEVGEN
      CALL GMONIT(   1,IDA+20,AWTOT,DWTOT,XNTOT )
      CALL GMONIT(   2,IDA+20,XNACC,XNNEG,XNOVE )
      NEVNEG = NINT(XNNEG)
      NEVOVE = NINT(XNOVE)
      XSMC   = SIG0  *AWTOT 
      XSMCPB = SIG0PB*AWTOT 
      EREL   = DWTOT 
      ERMC   = XSMCPB*EREL 
      XPAR(10) = XSMCPB
      XPAR(11) = EREL
      XPAR(12) = XSMC 
      IF(KEYWGT.EQ.0) THEN 
! ...WT=1  events, normal option... 
         XPAR(20)=XSMCPB 
         XPAR(21)=EREL 
         XPAR(22)=XSMC  
      ELSE
! ...Weighted events, additional information on x-sections 
         XPAR(20)= SIG0PB
         XPAR(21)= 0D0  
         XPAR(22)= SIG0
      ENDIF    
! Printout only for MODE=2
      IF(MODE.EQ.1) RETURN    
!=====(((
!#     CALL GPRINT(9001) 
!=====)))
      TEMPNOUT = NOUT
      NOUT = 6
      WRITE(NOUT,BXOPE)
      WRITE(NOUT,BXTXT) '*********************************'
      WRITE(NOUT,BXTXT) '  BHWIDE:        WINDOW A        '
      WRITE(NOUT,BXTXT) '*********************************'
      WRITE(NOUT,BXL1I) NEVGEN,     ' Accepted total    ','NEVGEN','A1'
      WRITE(NOUT,BXL1I) IEVENT,     ' Raw prior reject. ','IEVENT','A2'
      WRITE(NOUT,BXL2G) XSMCPB,ERMC,' Xsec M.C. [pb]    ','XSECMC','A3'
      WRITE(NOUT,BXL1F) EREL,       ' relat. error      ','ERELMC','A4'
      WRITE(NOUT,BXL2F) AWTOT,DWTOT,' weight  M.C.      ','AWT   ','A5'
      WRITE(NOUT,BXL1I) NEVNEG,     ' WT<0              ','NEVNEG','A6'
      WRITE(NOUT,BXL1I) NEVOVE,     ' WT>WTMAX          ','NEVOVE','A7'
      WRITE(NOUT,BXL1F) WTMAX ,     ' Maximum WT        ','WWMX  ','A8'
      WRITE(NOUT,BXCLO)
      NOUT = TEMPNOUT
! PRINT ADDITIONAL INFOS                  
!------------------------------------------------------------       
      CALL GMONIT(   1,IDA+1, AWT1 ,DWT1 ,DUMM3)                  
      CALL GMONIT(   1,IDA+2, AWT2 ,DWT2 ,DUMM3)         
      CALL GMONIT(   1,IDA+3, AWT3 ,DWT3 ,DUMM3)         
      CALL GMONIT(   1,IDA+4, AWT4 ,DWT4 ,DUMM3)
      
      TEMPNOUT = NOUT
      NOUT = 6
      WRITE(NOUT,BXOPE)
      WRITE(NOUT,BXTXT) '*********************************'
      WRITE(NOUT,BXTXT) '  BHWIDE:        WINDOW B        '
      WRITE(NOUT,BXTXT) '*********************************'
      WRITE(NOUT,BXL2F) AWT1,DWT1  ,'WT1*WT2*T/TP*T/TQ  ','      ','B1'
      WRITE(NOUT,BXL2F) AWT3,DWT3  ,'WT3 from KINO4     ','      ','B2'
      WRITE(NOUT,BXL2F) AWT4,DWT4  ,'YFS formfac        ','WT    ','B4'
      WRITE(NOUT,BXL2F) AWTOT,DWTOT,'TOTAL              ','      ','B5'
      CALL GMONIT(   1,IDA+18, AWT18 ,RWT18 ,DUMM3)     
      XWT18 = AWT18/AWTOT
      DWT18 = XWT18*RWT18
      WRITE(NOUT,BXL2F) XWT18,DWT18,'xsec/xtot: WT>WTMAX','WT    ','B6'
      CALL GMONIT(   1,IDA+19, AWT19 ,RWT19 ,DUMM3)     
      XWT19 = AWT19/AWTOT
      DWT19 = XWT19*RWT19
      WRITE(NOUT,BXL2F) XWT19,DWT19,'xsec/xtot: WT<0    ','WT    ','B7'
      WRITE(NOUT,BXCLO)  
      NOUT = TEMPNOUT
! ---------------------------------------------------------------
      CALL GMONIT( 1,IDC+1,AWT1,DWT1,DUMM3)                            
      CALL GMONIT( 1,IDC+2,AWT2,DWT2,DUMM3)          
      CALL GMONIT( 1,IDC+3,AWT3,DWT3,DUMM3)          
      CALL GMONIT( 1,IDC+4,AWT4,DWT4,DUMM3)          
      CALL GMONIT( 1,IDC+5,AWT5,DWT5,DUMM3)          
      CALL GMONIT( 1,IDC+6,AWT6,DWT6,DUMM3)
      
      TEMPNOUT = NOUT
      NOUT = 6
      WRITE(NOUT,BXOPE)
      WRITE(NOUT,BXTXT) '*********************************'
      WRITE(NOUT,BXTXT) '  BHWIDE:        WINDOW C        '
      WRITE(NOUT,BXTXT) 'Built-in average control weights.'
      WRITE(NOUT,BXTXT) 'Should equal one +- statist. err.'
      WRITE(NOUT,BXTXT) '*********************************'
      WRITE(NOUT,BXL2F) AWT1,DWT1  ,'<WCTA1>            ','      ','C1'
      WRITE(NOUT,BXL2F) AWT2,DWT2  ,'<WCTA2>            ','      ','C2'
      WRITE(NOUT,BXL2F) AWT3,DWT3  ,'<WCTA1*WCTA2>      ','      ','C3'
      WRITE(NOUT,BXL2F) AWT4,DWT4  ,'<WCTB1>            ','      ','C4'
      WRITE(NOUT,BXL2F) AWT5,DWT5  ,'<WCTB2>            ','      ','C5'
      WRITE(NOUT,BXL2F) AWT6,DWT6  ,'<WCTB1*WCTB2>      ','      ','C6'
      WRITE(NOUT,BXCLO) 
      NOUT = TEMPNOUT

      ENDIF 
*     =====
      END  

      SUBROUTINE FILBHW(XPAR,NPAR) 
*     **************************** 
!----------------------------------------------------------------------!
! In this routine input parameters are set up and stored in            !
! appriopriate COMMON blocks.                                          !
!----------------------------------------------------------------------!
! Last update: 14.03.2001                   by: W. Placzek             !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (A-H,O-Z)   
      DIMENSION  XPAR(*), NPAR(*) 
      PARAMETER( PI = 3.1415926535897932D0, ALFINV = 137.0359895D0)
      CHARACTER*80   BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G 
      PARAMETER(
     $BXOPE =  '(//1X,15(5H=====)    )',
     $BXTXT =  '(1X,1H=,                  A48,25X,    1H=)',
     $BXL1I =  '(1X,1H=,I17,                 16X, A20,A12,A7, 1X,1H=)',
     $BXL1F =  '(1X,1H=,F17.8,               16X, A20,A12,A7, 1X,1H=)',
     $BXL2F =  '(1X,1H=,F17.8, 4H  +-, F11.8, 1X, A20,A12,A7, 1X,1H=)',
     $BXL1G =  '(1X,1H=,G17.8,               16X, A20,A12,A7, 1X,1H=)',
     $BXL2G =  '(1X,1H=,G17.8, 4H  +-, F11.8, 1X, A20,A12,A7, 1X,1H=)',
     $BXCLO =  '(1X,15(5H=====)/   )'    )   
      COMMON / TRANSR / TRAN,TRMIN,TRMAX
      COMMON / BHPAR1 / DEL,EPSCMS,THMIN,XMIVIS 
      COMMON / BHPAR2 / CMSENE,AMEL  
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHPAR4 / THMIRP,THMARP,THMIRE,THMARE,ENMINP,ENMINE,ACOLLR
      COMMON / BHPARZ / AMAZ,GAMMZ,SINW2,GV,GA
      COMMON / BHCRUD / trmid,crufla,Zprof,sg01,sg02,sg03,sig0
! Communicates with VARRAN
      COMMON / RANPAR / KEYRND
      COMMON / INOUT  / NINP,NOUT
! Commons from BABAMC
      COMMON /INPUT1/EB
      COMMON /INPUT2/XMZ,S2W,XMH,XMT
      COMMON /INPUT3/TMIND,TMAXD,XKMAX
      COMMON /NEVCOM/ NEVT
      COMMON /XK0COM/ XK0
      COMMON /WMXCOM/ WMAX
      COMMON /REJCOM/ IREJEC
      COMMON /UNICOM/IIN,IUT
      COMMON /DBCOM/ IFLDB
! Commons from ALIBABA
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
!--
      SAVE   / INOUT  /, / RANPAR /, / TRANSR /, / BHCRUD /
      SAVE   / BHPAR1 /, / BHPAR2 /, / BHPAR3 /, / BHPAR4 /, / BHPARZ /
!       SAVE
      EXTERNAL dsigbo
!--
      CMSENE = XPAR(1)
      THMINP = XPAR(2)
      THMAXP = XPAR(3)
      THMINE = XPAR(4)
      THMAXE = XPAR(5)
      ENMINP = XPAR(6)
      ENMINE = XPAR(7)
      ACOLLI = XPAR(8)
      EPSCMS = XPAR(9)
      WTMAX  = XPAR(10)
! Re-set WTMAX if non-positive
      IF (WTMAX.LE.0D0) THEN
         WTMAX = 10D0
         XPAR(10) = WTMAX
      ENDIF
! Z parameters   
      AMAZ  = XPAR(11)   
      GAMMZ = XPAR(12)          
! Weak mixing angle
      SINW2 = XPAR(13) 
! Masses of top quark and Higgs boson
      AMTOP = XPAR(14)
      AMHIG = XPAR(15)  
! Switches/keys
      KeyOpt = NPAR(1)
      KeyRad = NPAR(2)
      KeyRnd = MOD(KeyOpt,10)   
      KeyWgt = MOD(KeyOpt,100)/10   
      KeyCha = MOD(KeyOpt,1000)/100   
      KeyZof = MOD(KeyOpt,10000)/1000   
      KeyPia = MOD(KeyRad,10)
      KeyMod = MOD(KeyRad,100)/10 
      KeyLib = MOD(KeyRad,1000)/100 
      KeyEWC = MOD(KeyRad,10000)/1000
      IF (KeyCha.GT.0) THEN
        IF (KeyMod.NE.2) THEN
          KeyMod = 2
          WRITE(6,   *)'>> KeyMod reset to KeyMod =',KeyMod
          WRITE(6,   *)'>> KeyCha =',KeyCha,' works only for KeyMod =',
     &                 KeyMod,' !!!'
          KeyMod = 2
          WRITE(NOUT,*)'>> KeyMod reset to KeyMod =',KeyMod
          WRITE(NOUT,*)'>> KeyCha =',KeyCha,' works only for KeyMod =',
     &                 KeyMod,' !!!'
        ENDIF
        IF (KeyLib.NE.2) THEN
          KeyLib = 2
          WRITE(6,   *)'>> KeyLib reset to KeyLib =',KeyLib
          WRITE(6,   *)'>> KeyCha =',KeyCha,' works only for KeyLib =',
     &                 KeyLib,' !!!'
          WRITE(NOUT,*)'>> KeyLib reset to KeyLib =',KeyLib
          WRITE(NOUT,*)'>> KeyCha =',KeyCha,' works only for KeyLib =',
     &                 KeyLib,' !!!'
        ENDIF
      ENDIF
      IF (KeyZof.GT.0) THEN
        IF (KeyEWC.NE.0) THEN
          KeyEWC = 0
          WRITE(6,   *)'>> KeyEWC reset to KeyEWC =',KeyEWC
          WRITE(6,   *)'>> KeyZof =',KeyZof,' works only for KeyEWC =',
     &                 KeyEWC,' !!!'
          WRITE(NOUT,*)'>> KeyEWC reset to KeyEWC =',KeyEWC
          WRITE(NOUT,*)'>> KeyZof =',KeyZof,' works only for KeyEWC =',
     &                 KeyEWC,' !!!'
        ENDIF
      ENDIF
      KeyRad = 1000*KeyEWC +100*KeyLib +10*KeyMod +KeyPia 
      IF (KeyEWC.GT.0 .AND. KeyPia.EQ.0) THEN
        WRITE(6   ,*)'>> FILBHW: Wrong KeyPia !!!, KeyPia=',KeyPia
        WRITE(6   ,*)'>> For KeyEWC >0, KeyPia must be >0 too!'
        WRITE(NOUT,*)'>> FILBHW: Wrong KeyPia !!!, KeyPia=',KeyPia
        WRITE(NOUT,*)'>> For KeyEWC >0, KeyPia must be >0 too!'
        STOP
      ENDIF
!--------------------------------------------------
! Electron mass and s-variable
      AMEL = 0.5111D-3
      SVAR = CMSENE**2
! Angles in radians
      DEGRAD = PI/180D0
      THMIRP = THMINP *DEGRAD
      THMARP = THMAXP *DEGRAD
      THMIRE = THMINE *DEGRAD
      THMARE = THMAXE *DEGRAD
      ACOLLR = ACOLLI *DEGRAD
! Find the minimum transfer for M.C. generation
      THMIN  = DMIN1(THMIRP,THMIRE)
      Ene    = CMSENE/2D0
      XEMINP = ENMINP/Ene
      XEMINE = ENMINE/Ene
cc      xmivis = DMIN1(XEMINP,XEMINE)
cc      xmivis = xmivis**2
      xmivis = XEMINP*XEMINE
      AMER   = AMEL/Ene
      beteli = DSQRT(1 - AMER**2)
      betelf = DSQRT(1 - AMER**2/xmivis)
! Minimum transfer in LL approx.
      TRMILL = SVAR*xmivis*(1 -beteli*betelf*COS(THMIN))/2 -2*AMEL**2
! Safe value of minimum transfer for MC generation
      trmin  = 0.5*TRMILL
! Find some intermediate transfer (to increase efficiency of MC generation).
! For trmin < tran < trmid, transfer is generated from a flat distribution.  
! The choice of trmid and trmin may depend on a given case (CMS energy,
! selection criteria, etc.).
      thmid = DMAX1(THMIRP,THMIRE)
cc      trmid = trmin
cc      trmid = SVAR*xmivis*(1 -beteli*betelf*COS(thmid))/2 -2*AMEL**2
cc      trmid = SVAR*       (1 -beteli*betelf*COS(thmid))/2 -2*AMEL**2
      trmid = SVAR*(1 - COS(thmid) )/2
! Maximum transfer (safe value: TRMAX=SVAR)
      TRMAX = SVAR
      XIMIN = TRMIN/SVAR
      XIMAX = TRMAX/SVAR
! Internal soft photon cut-off (the same as in BHLUMI) 
      DEL = EPSCMS*1d-2
!---------------------------------------------------------------------
! Weak couplings
      GA = -1/(4*DSQRT(SINW2*(1-SINW2)))
      GV = GA*(1-4*SINW2)
! Switch OFF Z contribution
      IF (KeyZof.EQ.1) THEN
        GA = 0
        GV = 0
      ENDIF
      AMT = AMTOP
      AMH = AMHIG
!---------------------------------------------------------------------
! Initialization of ElectroWeak Libraries (virtual+soft corrections)
      IF (KeyLib.EQ.2) THEN
!... ALIBABA routines
! NOTE: The above values of weak parameters are replaced by ones 
!       calculated by ALIBABA! 
        CALL INITBH(AMAZ,AMH,AMT)
        IWEAK = KeyEWC
! Weak parameters as calculated by ALIBABA 
        GAMMZ = ZWID
        SINW2 = SIN2TH
        GA = -1/(4*DSQRT(SINW2*(1-SINW2)))
        GV = GA*(1-4*SINW2)
! Switch OFF Z contribution
        IF (KeyZof.EQ.1) THEN
          CALL ZbcOFF
          GA = 0
          GV = 0
        ENDIF
      ELSEIF (KeyLib.EQ.1) THEN
!... BABAMC routines 
        EB  = CMSENE/2
        XMZ = AMAZ
        XGZ = GAMMZ
        XMT = AMT
        XMH = AMH
        S2W = SINW2
        TMIND = DMAX1(THMINP,THMINE)
        TMAXD = DMIN1(THMAXP,THMAXE) 
        XKMAX = 1.0
        IIN = 5
        IUT = 6
        IREJEC = 0
        WMAX = 0.01
        IFLDB = 0
        CALL BAREAD
        XK0 = EPSCMS
        GVB = GV*DSQRT(4*PI/ALFINV)
        GAB = GA*DSQRT(4*PI/ALFINV)
        CALL SETUPS(EB,XMZ,XGZ,S2W,XMH,XMT,XK0)
      ELSE
        WRITE(6   ,*)'>> FILBHW: Wrong KeyLib !!!, Keylib=',KeyLib
        WRITE(NOUT,*)'>> FILBHW: Wrong KeyLib !!!, Keylib=',KeyLib
        STOP
      ENDIF
!---------------------------------------------------------------------
! Total Born cross section
      thminb = DMAX1(THMIRE,THMIRP)
      thmaxb = DMIN1(THMARE,THMARP)
      trminb = SVAR/2 *(1 -beteli**2*COS(thminb)) -2*AMEL**2
      trmaxb = SVAR/2 *(1 -beteli**2*COS(thmaxb)) -2*AMEL**2
      eeps = -1d-8
      KeyTem = KeyRad 
      KeyRad = 100*KeyLib 
      CALL GAUSJD(dsigbo,trminb,trmaxb,eeps,BORNXS)
      KeyRad = KeyTem
!---------------------------------------------------------------------
      TEMPNOUT = NOUT
      NOUT = 6
      WRITE(NOUT,BXOPE)
      WRITE(NOUT,BXTXT) '*********************************'
      WRITE(NOUT,BXTXT) '  BHWIDE: INPUT PARAMETRES       '
      WRITE(NOUT,BXTXT) '*********************************'
      WRITE(NOUT,BXL1I) KeyOpt,     ' OPTIONS  Switches ','KeyOpt','N1'
      WRITE(NOUT,BXL1I) KeyZof,     ' Z Switch-OFF      ','KeyZof','  '
      WRITE(NOUT,BXL1I) KeyCha,     ' Channel   Choice  ','KeyCha','  '
      WRITE(NOUT,BXL1I) KeyWgt,     ' Weighting Switch  ','KeyWgt','  '
      WRITE(NOUT,BXL1I) KeyRnd,     ' Rand. Numb. Switch','KeyRnd','  '
      WRITE(NOUT,BXL1I) KeyRad,     ' RAD. COR. Switches','KeyRad','N2'
      WRITE(NOUT,BXL1I) KeyEWC,     ' EW Correct. Switch','KeyEWC','  '
      WRITE(NOUT,BXL1I) KeyLib,     ' EWC Libary choice ','KeyLib','  '
      WRITE(NOUT,BXL1I) KeyMod,     ' QED Mat.El. choice','KeyMod','  '
      WRITE(NOUT,BXL1I) KeyPia,     ' Vac. Pol. Switch  ','KeyPia','  '
      WRITE(NOUT,BXL1F) CMSENE,     ' CMS energy   [GeV]','CMSENE','X1'
      WRITE(NOUT,BXL1G) SVAR  ,     ' CMSENE^2   [GeV^2]','SVAR  ','  '
      WRITE(NOUT,BXL1F) THMINP,     ' Theta_Min_e+ [deg]','THMINP','X2'
      WRITE(NOUT,BXL1F) THMAXP,     ' Theta_Max_e+ [deg]','THMAXP','X3'
      WRITE(NOUT,BXL1F) THMINE,     ' Theta_Min_e- [deg]','THMINE','X4'
      WRITE(NOUT,BXL1F) THMAXE,     ' Theta_Max_e- [deg]','THMAXE','X5'
      WRITE(NOUT,BXL1F) THMIRP,     ' Theta_Min_e+ [rad]','THMIRP','  '
      WRITE(NOUT,BXL1F) THMARP,     ' Theta_Max_e+ [rad]','THMARP','  '
      WRITE(NOUT,BXL1F) THMIRE,     ' Theta_Min_e- [rad]','THMIRE','  '
      WRITE(NOUT,BXL1F) THMARE,     ' Theta_Max_e- [rad]','THMARE','  '
      WRITE(NOUT,BXL1F) ENMINP,     ' Energy_Min_e+[GeV]','ENMINP','X6'
      WRITE(NOUT,BXL1F) ENMINE,     ' Energy_Min_e-[GeV]','ENMINE','X7'
      WRITE(NOUT,BXL1F) XEMINP,     ' E_Min_e+/E_beam   ','XEMINP','  '
      WRITE(NOUT,BXL1F) XEMINE,     ' E_Min_e-/E_beam   ','XEMINE','  '
      WRITE(NOUT,BXL1F) ACOLLI,     ' Acollinearity[deg]','ACOLLI','X8'
      WRITE(NOUT,BXL1F) ACOLLR,     ' Acollinearity[rad]','ACOLLR','  '
      WRITE(NOUT,BXL1G) TRMIN ,     ' Trasf_Min [GeV^2] ','TRMIN ','  '
      WRITE(NOUT,BXL1G) TRMAX ,     ' Trasf_Max [GeV^2] ','TRMAX ','  '
      WRITE(NOUT,BXL1G) XIMIN ,     ' Xi_Min=TRMIN/SVAR ','XIMIN ','  '
      WRITE(NOUT,BXL1G) XIMAX ,     ' Xi_Max=TRMAX/SVAR ','XIMAX ','  '
      WRITE(NOUT,BXL1G) EPSCMS,     ' Soft-Photon Cut   ','EPSCMS','X9'
      WRITE(NOUT,BXL1G) DEL   ,     ' Internal Soft Cut ','DEL   ','  '
      WRITE(NOUT,BXL1F) WTMAX ,     ' Maximum Weight    ','WTMAX ','X10'
      WRITE(NOUT,BXL1F) AMAZ  ,     ' Z mass [GeV]      ','AMAZ  ','X11'
      WRITE(NOUT,BXL1F) GAMMZ ,     ' Z width [GeV]     ','GAMMZ ','X12'
      WRITE(NOUT,BXL1F) SINW2 ,     ' sin^2(theta_W)    ','SINW2 ','X13'
      WRITE(NOUT,BXL1F) AMTOP ,     ' t-quark mass [GeV]','AMTOP ','X14'
      WRITE(NOUT,BXL1F) AMHIG ,     ' Higgs   mass [GeV]','AMHIG ','X15'
      WRITE(NOUT,BXL1G) BORNXS,     ' Born Xsecion [pb] ','BORNXS','  '
      WRITE(NOUT,BXCLO)
      NOUT = TEMPNOUT
      END 

      FUNCTION dsigbo(tran)
*     *********************
! Differential Born cross section dsigma/dt in pb.
      IMPLICIT REAL*8 (a-h,o-z)
      PARAMETER( pi = 3.1415926535897932d0, alfinv=137.0359895D0)
      PARAMETER( alfpi=  1/pi/alfinv, alfa= 1d0/alfinv)
      PARAMETER( Gnanob=389.385D3 )
      COMMON / BHPAR2 / CMSENE,AMEL  
      SAVE   / BHPAR2 /
!
      s = CMSENE**2
      t = -tran
      u = 4*AMEL**2 -s -t
      dsig0 = xmate0(s,t,u)/(4*pi*alfa)**2 
!WP: Genuine weak corrections
      xweak = xmatwc(s,t)/(4*pi*alfa)**2
cc      print*,' dsig0, xweak=', dsig0,xweak
cc      dsig0 = xweak
      dsigbo = Gnanob*1d3 *pi*alfa**2/s**2 *dsig0 
      END

      SUBROUTINE gentra(TRMIN,TRMAX,TRAN)
*     ***********************************
!--------------------------------------------------------------!
! Generation of momentum transfer squared TRAN=|Q**2| for      !
! Bhabha scattering according to "crude" distribution.         !
! INPUT:  TRMIN,TRMAX  - min. and max. transfer                !
! OUTPUT: TRAN         - generated transfer                    !
!--------------------------------------------------------------!
! Written by: Wieslaw Placzek              Knoxville, May 1995 !
! Last update: 25.05.1995         by: W.P.                     !
!--------------------------------------------------------------!
      implicit REAL*8 (a-h,o-z)
      COMMON / BHCRUD / trmid,crufla,Zprof,sg01,sg02,sg03,sig0
      SAVE   / BHCRUD /
      REAL*8 drvec(100)
!
      cu01 = sg01/sig0
      cu02 = cu01 +sg02/sig0
      call VARRAN(drvec,2)
      rn1 = drvec(1)
      rn2 = drvec(2)
      IF (rn1.LE.cu01) THEN
         TRAN = (1-rn2)*trmid + rn2*TRMAX
      ELSEIF (rn1.LE.cu02) THEN
         TRAN = 1/ ( (1-rn2)/trmid + rn2/TRMAX )
      ELSE
         TRAN = (1-rn2)*TRMIN + rn2*trmid
      ENDIF
      END

      SUBROUTINE trigMC(wt)
*     *********************
!--------------------------------------------------------------!
! Trigger for wide angle Bhabha scattering: cuts imposed on    !
! positron and electron energies, angles and acollinearity.    !
! OUTPUT: wt  - weight corresponding to the above trigger      !
!--------------------------------------------------------------!
! Written by: Wieslaw Placzek              Knoxville, May 1995 !
! Last update: 19.07.1995         by: W.P.                     !
!--------------------------------------------------------------!
      implicit REAL*8 (a-h,o-z)
      PARAMETER( PI = 3.1415926535897932d0 )
      COMMON / BHPAR4 / THMINP,THMAXP,THMINE,THMAXE,ENMINP,ENMINE,ACOLLI
      COMMON / MOMSET / P1(4),Q1(4),P2(4),Q2(4),PHOT(100,4),NPHOT
      SAVE   / BHPAR4 /, / MOMSET /  
      LOGICAL lanp,lane,lenp,lene,laco
!
      wt = 0
! Energy cuts
      lenp = P2(4).gt.ENMINP
      lene = Q2(4).gt.ENMINE
      IF (lenp.AND.lene) THEN
        pmod = SQRT(p2(1)**2+p2(2)**2+p2(3)**2) 
        qmod = SQRT(q2(1)**2+q2(2)**2+q2(3)**2) 
        costhp = p2(3)/pmod
        costhe =-q2(3)/qmod
! Angular cuts
        lanp = costhp.LE.COS(THMINP) .AND. costhp.GE.COS(THMAXP)
        lane = costhe.LE.COS(THMINE) .AND. costhe.GE.COS(THMAXE) 
        IF (lanp.AND.lane) THEN
          pq = p2(1)*q2(1) + p2(2)*q2(2) + p2(3)*q2(3)
          costpe =-pq/(pmod*qmod)
! Acollinearity cut
          laco = costpe.ge.COS(ACOLLI)
          IF (laco) wt = 1
        ENDIF
      ENDIF
      END

      SUBROUTINE REMPHO(EMIN,NPHOT,PHOT,PX,ALF,BET,WTM,MK)
*     ****************************************************
      IMPLICIT REAL*8(A-H,O-Z)       
      DIMENSION PX(4),PHOT(100,4),ALF(50),BET(50),WTM(50),MK(50)
!
      IF (NPHOT.EQ.0) RETURN
      NPH=NPHOT 
      DO 100 J=NPHOT,1,-1 
      IF(PHOT(J,4).LT.EMIN) THEN 
         DO 60 I=J+1,NPH  
         ALF(I-1)=ALF(I)
         BET(I-1)=BET(I)
         WTM(I-1)=WTM(I)
         MK( I-1)=MK( I)
         DO 60 K=1,4      
   60    PHOT(I-1,K)=PHOT(I,K) 
         NPH=NPH-1 
      ENDIF   
  100 CONTINUE
!.....................................................
! Correction by Alex Read and Tiziano Camporesi DELPHI
! Date: Fri, 25 Nov 94 17:50:37 WET
!      Code added by ALR 22.11.94 to fix problem with
!      photon handling. Have to erase the discarded
!      photons or they cause occasional problems in
!      MERGIK when merging the PHOT1 and PHOT2 arrays
!      (REMPHO operates on these here).
!
      DO J=NPH+1,NPHOT
         DO K=1,4
           PHOT(J,K) = 0.D0
         ENDDO
      ENDDO
!.....................................................
      NPHOT=NPH
      END

      SUBROUTINE PIATEK(CMSENE,TRMAX,AMEL,EMIN,DELTA,
     $         NPHOT,P1,P2,PHOT,PHSU,WMAT,WTAL,WTMRE,WCTR1,WCTR2)
C     ***************************************************************
c Input:
c        CMSENE         CMS energy
c        TRMAX          maximum transfer 
c        AMEL           electron mass (GeV)
c        EMIN           CMS minimum photon energy (GeV)
c        DELTA          MC minimum photon energy (dimensionless)
c        NPHOT          photon number
c        P1,P2(4)       fermion momenta
c        PHOT(100,4)    photon four-momenta
c        PHSU(50)       sum of photon four-momenta
c        WMAT(50)       mass weights from MLTIBR
c Output:
c        WTAL       mass weight for all photons
c        WTMRE      In the case of removal the new mass weight
c        WCTR1      Control weight for delta-->epsilon rejection
c        WCTR2      control weight for photons below EMIN removal
*     ************************
      IMPLICIT REAL*8(A-H,O-Z)  
      PARAMETER(PI=3.1415926535897932D0,ALFINV=137.0359895D0)
      PARAMETER(ALF1=1/ALFINV/PI )
      DIMENSION P1(4),P2(4),WMAT(50),PHOT(100,4),PHSU(4)
      DATA ICONT /0/
      ICONT=ICONT+1
C Calculate mass weight for all photons and separately for
C photons with energy below/above EMIN
      EPSCM = 2*EMIN/CMSENE
      WTAL = 1D0   
      WTM1 = 1D0
      WTM2 = 1D0
      WTEPSP=1D0
      DO 100 I=1,NPHOT 
      WTAL = WTAL*WMAT(I)
      IF(WTAL.LT.1D-15) WTAL =0D0
      IF(PHOT(I,4).LT.EMIN) THEN
        WTM1 = WTM1*WMAT(I)
        IF(WTM1.LT.1D-15) WTM1=0D0
        WTEPSP = 0D0
      ELSE
        WTM2 = WTM2*WMAT(I)
        IF(WTM2.LT.1D-15) WTM2=0D0
      ENDIF
  100 CONTINUE   
C Control weight for delta-->epsilon  "REJECTION"
      DELT1 = DELTA*BCUD(P1,P2,PHSU)
      CALL WFORM(TRMAX,P1,P2,AMEL,DELT1,EMIN,PDYFS)
      WCTR1 = WTEPSP*PDYFS   
C control weight for photons ENE<EMIN  "REMOVAL"
      TRANP = 2D0*(P1(4)*P2(4)-P1(3)*P2(3)-P1(2)*P2(2)-P1(1)*P2(1))
      EPS1  =  SQRT(EMIN**2/P1(4)/P2(4))     
      DELB2 = -2*ALF1*(DLOG(TRMAX/TRANP)+1) *DLOG(EPS1/DELT1)
      WCTR2 = WTM1*EXP(-DELB2)   
C In the case of removal the new mass weight is this
      WTMRE = WTM2*EXP(DELB2)
      END

      FUNCTION YFSfsu(epsCMS)
*     ***********************
!-------------------------------------------------------------------!
! Yennie-Frautschi-Suura form-factor for Bhabha scattering in CMS   !
! summed over s and u channels (t-channels is already included      !
! in earlier steps of MC algorithm).                                !
! INPUT: epsCMS - soft photon limit in CMS as a fraction            !
!                 of the beam energy                                !
!-------------------------------------------------------------------!
! Written by: Wieslaw Placzek                Knoxville, Oct. 1995   !
! Last update: 04.10.1995           by: W.P.                        !
!-------------------------------------------------------------------!     
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER ( pi = 3.1415926535897932D0, alfinv = 137.0359895D0)
      PARAMETER ( alfpi = 1/alfinv/pi )
      COMMON / BHPAR2 / CMSEne,amel  
      COMMON / MOMSET / p1(4),q1(4),p2(4),q2(4),phot(100,4),nphot
! Save value of YFSfsu for some further use
      COMMON / sufyfs / fofasu
      SAVE   / BHPAR2 /, / MOMSET /, / sufyfs /
!
      s = CMSEne**2
      Emin = epsCMS*CMSENE/2
      Bslog = LOG(s/amel**2) 
! s-channel: initial state
      Rs  = alfpi*( 2*(Bslog -1)*LOG(epsCMS) +0.5*Bslog -1 +pi**2/3 )
! s-channel: final state
      Rs1 = finfra(p2,q2,Emin,amel) +alfpi*pi**2/2
! u-channel
      Ru  = finfra(p1,q2,Emin,amel)
      Ru1 = finfra(q1,p2,Emin,amel)
! form-factor
      fofasu = EXP(Rs + Rs1 - Ru - Ru1)
      YFSfsu = fofasu 
      END

      FUNCTION YFSfmf(epsCMS)
*     ***********************
!-------------------------------------------------------------------!
! Total Yennie-Frautschi-Suura form-factor for Bhabha scattering    !
! in CMS.                                                           !  
! INPUT: epsCMS - soft photon limit in CMS as a fraction            !
!                 of the beam energy                                !
! NOTE: Function YFSfsu should be called prior this function is     ! 
!       used. This is done to increase efficiency of MC generation, !
!       as YFSfsu is normally called in the earlier step of MC      !
!       algorithm and it provides a major part of the result.       !
!-------------------------------------------------------------------!
! Written by: Wieslaw Placzek                Knoxville, Oct. 1995   !
! Last update: 05.10.1995           by: W.P.                        !
!-------------------------------------------------------------------!     
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER ( pi = 3.1415926535897932D0, alfinv = 137.0359895D0)
      PARAMETER ( alfpi = 1/alfinv/pi )
      COMMON / BHPAR2 / CMSEne,amel  
      COMMON / MOMSET / p1(4),q1(4),p2(4),q2(4),phot(100,4),nphot
! Value of YFS formfactor summed over s and u channels from YFSfsu 
      COMMON / sufyfs / fofasu
      SAVE   / BHPAR2 /, / MOMSET /, / sufyfs /
!
      Emin = epsCMS*CMSENE/2
! t-channels form-factors
      finftp = finfra(p1,p2,Emin,amel) 
      finftq = finfra(q1,q2,Emin,amel) 
      fofatp = EXP(finftp)
      fofatq = EXP(finftq)
! Total YFS form-factor
      YFSfmf = fofasu*fofatp*fofatq
      END

      FUNCTION YFSirf(p1,q1,p2,q2,epsCMS,amel)
*     ****************************************
!-------------------------------------------------------------------!
! Yennie-Frautschi-Suura infrared factor in CMS summed over s, t    !
! and u channels (to be subtructed from O(alpha^1) virtual+soft     !
! correction in order to get function beta_0 in YFS formula).       !
! INPUT: p1,q1,p2,q2 - initial and final lepton 4-momenta           !
!        epsCMS      - soft photon limit in CMS as a fraction       !
!                      of the beam energy                           !
!        amel        - electron mass [in GeV]                       !  
!-------------------------------------------------------------------!
! Written by: Wieslaw Placzek                Knoxville, Oct. 1995   !
! Last update: 04.10.1995           by: W.P.                        !
!-------------------------------------------------------------------!     
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER ( pi = 3.1415926535897932D0, alfinv = 137.0359895D0)
      PARAMETER ( alfpi = 1/alfinv/pi )
      REAL*8 p1(4),q1(4),p2(4),q2(4)
!
      Emin = epsCMS*p1(4)
      s = 2*( p1(4)*q1(4) -p1(3)*q1(3) -p1(2)*q1(2) -p1(1)*q1(1) )
      Bslog = LOG(s/amel**2) 
! infrared factors for s, t, u channels 
      BBs = alfpi*( 2*(Bslog -1)*LOG(epsCMS) +0.5*Bslog -1 +pi**2/3 )
      BBt = finfra(p1,p2,Emin,amel)
      BBu = finfra(p1,q2,Emin,amel)
! total infrared factor
      YFSirf = 2*(BBs + BBt - BBu)
      END

      FUNCTION finfra(p1,p2,Emin,amel)
*     ********************************
!-------------------------------------------------------------------!
! Yennie-Frautschi-Suura infrared factor (virtual + real soft)      !
! in CMS for t and u channels, for any pair of electrons            !
! and/or positrons.                                                 !
! To get the result for the s-channel one has to add to the result  !
! of this function only the constant term: (alpha/pi)*(pi**2/2).    !
! INPUT: p1,p2 - leptons four-momenta                               !
!        Emin  - minimum energy of hard photons in CMS [in GeV]     !
!        amel  - electron mass [in GeV]                             ! 
!-------------------------------------------------------------------!
! Written by: Wieslaw Placzek                Knoxville, Oct. 1995   !
! Last update: 04.10.1995           by: W.P.                        !
!-------------------------------------------------------------------!     
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER ( pi = 3.1415926535897932D0, alfinv = 137.0359895D0)
      PARAMETER ( alfpi = 1/alfinv/pi )
      REAL*8 p1(4),p2(4)
!
      E1 = p1(4)
      E2 = p2(4)
      QQ = 2*( p1(4)*p2(4) -p1(3)*p2(3) -p1(2)*p2(2) -p1(1)*p2(1) )
      Biglog = LOG(QQ/amel**2)
      eps2 = Emin**2/E1/E2
      om = E1 + E2
      de = E1 - E2
      Dt = SQRT(QQ + de**2)
      Root = (Biglog -1)*LOG(eps2) +0.5*Biglog -1
! At low angles Remn should be approximately equal to: 
!           -pi**2/6,  for s, u channels; 
!            0,        for t channel. 
      Remn = pi**2/3 -0.5*LOG(E2/E1)**2 
     &      -0.25*LOG( (Dt+de)**2/(4*E1*E2) )**2
     &      -0.25*LOG( (Dt-de)**2/(4*E1*E2) )**2 
     &      -DILOGY( (Dt+om)/(Dt+de) ) -DILOGY( (Dt+om)/(Dt-de) )
     &      -DILOGY( (Dt-om)/(Dt+de) ) -DILOGY( (Dt-om)/(Dt-de) )
      finfra = alfpi*(Root + Remn) 
      END

      SUBROUTINE WFORM(TRMAX,Q1,Q2,AMF,DELTA,EMIN,DYFS)
C     *************************************************  
C For tests only.     
C Yennie-Frautschi-Suura Formfactors for the single fermion pair 
C This is for crude distribition before mass weights
C The triangle effect included (pi**2/6)
C     ***********************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      PARAMETER(PI=3.1415926535897932D0,ALFINV=137.0359895D0)
      PARAMETER(ALF1=1/ALFINV/PI )
      DIMENSION Q1(4),Q2(4)       
C ...Momenta q1,q2 should be in CMS
      Q1Q2  = Q1(4)*Q2(4)-Q1(3)*Q2(3)-Q1(2)*Q2(2)-Q1(1)*Q2(1) 
      E1 = Q1(4) 
      E2 = Q2(4)
      BETF2 = 2*ALF1* DLOG(TRMAX /AMF**2) 
      DELB  = BETF2*DLOG(EMIN/SQRT(E1*E2)/DELTA)
      EP    = E1+E2
      EM    = E1-E2  
      DL    = SQRT( 2*Q1Q2 +EM**2 )     
C Note that approximately REMN= +(1./6.)*PI**2 for t-channel
      REMN  = PI**2/2 
     $        -0.50*DLOG(E1/E2)**2  
     $        -0.25*DLOG((DL+EM)**2/(4*E1*E2))**2 
     $        -0.25*DLOG((DL-EM)**2/(4*E1*E2))**2    
     $        - DILOGY((DL+EP)/(DL+EM)) -DILOGY((DL-EP)/(DL-EM))
     $        - DILOGY((DL-EP)/(DL+EM)) -DILOGY((DL+EP)/(DL-EM)) 
C This (alf/pi)*pi**2/6 is related to replacing (y+z)>epsilon
C by max(y,z)>epsilon.   (Rejection delta=> epsilon over-estimated)
      TRIANG = -PI**2/6D0 
      DYFS   = EXP( DELB +ALF1*REMN +ALF1*TRIANG) 
      END

      FUNCTION BCUD(P1,P2,SF)
*     ************************
      IMPLICIT REAL*8(A-H,O-Z)
      DIMENSION P1(4),P2(4),SF(4)
      XPP  = P2(4)*P1(4)-P2(3)*P1(3)-P2(2)*P1(2)-P2(1)*P1(1)
      XPR  = P1(4)*(P2(4)+SF(4)) - P1(3)*(P2(3)+SF(3))
     $     - P1(2)*(P2(2)+SF(2)) - P1(1)*(P2(1)+SF(1))
      BCUD= XPR/XPP
      END

      SUBROUTINE MLTIBR(TRAN,TRMAX,AMEL,DEL,
     $      NPH,PHOT,PHSU,ALF1,BET1,TRANP,AMSP,MK,WT1,WTM)   
*     ****************************************************   
* This provides momenta of photons in a fermion proper frame 
C Input : TRAN    = principal t-channel transfer     (GEV**2)
C         TRMAX   = max. transf. (>0) for angular phot. dist. [GEV**2]
C         AMEL    = electron energy         (GEV)
C         DEL     = low energy photon limit   (dimensionless)
C Output: NPH     = photon multiplicity
C         PHOT    = list of photon four-momenta
C         PHSU    = sum of photon momenta
C         ALF1,BET1   = Sudakov variables
C         TRANP   = (P2-P1)**2
C         AMSP    = (P2+PHSU)**2  
C         MK      = marked photons
C         WT1     = TRANP/TRAN is Jacobian, =0 outside ph.sp.
C         WTM     = list of mass weights
*     ************************
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER (PI=3.1415926535897932D0, ALFINV=137.0359895D0)
      PARAMETER (NMAX=20)
      DIMENSION PHOT(100,4),PHSU(4),PH(4)
      DIMENSION MK(50),WTM(50)   
      DIMENSION ALF(50),BET(50),ALF1(50),BET1(50),Y(50),Z(50)
      DIMENSION RR(100),P2(4)
      DOUBLE PRECISION DRVEC(100)
      DATA ICONT /0/ 
      ICONT=ICONT+1 
    
      DELS  = AMEL**2/TRMAX
      BILGS = LOG(TRMAX/AMEL**2)
      DELL  = LOG(DEL) 
      WT1    = 1D0
      DO  11 I=1,50 
      DO  11 K=1,4 
   11 PHOT(I,K)=0D0
      AVERG=2D0/(PI*ALFINV)*BILGS*LOG(1D0/DEL)
      CALL POISSG(AVERG,NMAX,NPH,RR) 
      IF(NPH.GT.NMAX) GOTO 900  
* No photon        
      DO 45 K=1,4
   45 PHSU(K)=0D0
      IF(NPH.EQ.0) THEN 
        TRANP=TRAN  
      ELSE  
* One or more photons 
   50   CALL VARRAN(DRVEC,NPH) 
        BSUM=0D0 
        DO 80 I=1,NPH  
* We define R=LOG(MAX(YGR,ZET)) 
        R=DELL*RR(I) 
* Photons close to lower infrared boundry are marked for tests 
        MK(I)=0 
        IF(EXP(R).LT.DEL*3D0) MK(I)=1  
        T= 2D0*DRVEC(I)
        IF(T.GT.1D0) THEN  
           YGR=R  
           ZET=R-(T-1D0)*BILGS  
        ELSE  
           ZET=R     
           YGR=R-T*BILGS 
        ENDIF  
        YGR=EXP(YGR) 
        ZET=EXP(ZET) 
        Y(I)=YGR 
        Z(I)=ZET 
* Define ALPHA and BETA (prim)
        ALF1(I)=YGR-ZET*DELS
        BET1(I)=ZET-YGR*DELS 
        IF(ALF1(I).LE.0D0.OR.BET1(I).LE.0D0) GOTO 50
   80   BSUM=BSUM+BET1(I)   
        IF(BSUM.GT.1D0) GOTO 800
* Rescale ALPHA and BETA        
        CALL VARRAN(DRVEC,NPH) 
        DO 90 I=1,NPH 
        ALF(I)=ALF1(I)/(1D0-BSUM) 
   90   BET(I)=BET1(I)/(1D0-BSUM)  
* Define photon four momenta in SQRT(TRANP)/2 units 
        DO 100 I=1,NPH 
        PHOT(I,4)= ALF(I)+BET(I) 
        PHOT(I,3)=-ALF(I)+BET(I)
        R1 = DRVEC(I)
        PHI=2D0*PI*R1
        PTRAN=2D0*DSQRT(DABS(ALF(I)*BET(I)))
        PHOT(I,1)=PTRAN*COS(PHI) 
        PHOT(I,2)=PTRAN*SIN(PHI) 
        DO 100 K=1,4 
  100   PHSU(K)=PHSU(K)+PHOT(I,K)
* Define factor for rescaling photon momenta    
        XMK2=PHSU(4)**2-PHSU(3)**2-PHSU(2)**2-PHSU(1)**2 
        YY2=1D0/(1D0+PHSU(3)-.25D0*XMK2)      
* YY2 negative when outside phase space (abs(t)>abs(s))
        IF(YY2.LE.0D0) GOTO 900 
        TRANP=TRAN*YY2      
        ENER =SQRT(TRANP)/2D0        
* RESCALE ALL PHOTON MOMENTA         
        DO 120 K=1,4        
        PHSU(K)=PHSU(K)*ENER         
        DO 120 I=1,NPH      
  120   PHOT(I,K)=PHOT(I,K)*ENER     
* This rotation makes PHSU(2)=0 
* (we get rid her of dummy angle, see "poprawka" in the notes)
        PSIT=ANGFIX(PHSU(1),PHSU(2))  
        CALL RXTOD3(-PSIT, PHSU,PHSU)         
        DO 140 I=1,NPH      
        DO 135 K=1,4        
  135   PH(K)=PHOT(I,K)     
        CALL RXTOD3(-PSIT, PH,PH)    
        DO 140 K=1,4        
  140   PHOT(I,K)=PH(K)     
      ENDIF         
c+++      IF(TRANP.EQ.0D0) GO TO 900      
      IF(TRANP.LE. 4*AMEL**2) GO TO 900      
      BETEL=SQRT(1D0-4D0*AMEL**2/(TRANP+4D0*AMEL**2))          
      P2(3)=SQRT(TRANP)/2D0          
      P2(4)=SQRT(P2(3)*P2(3)+AMEL**2)         
      P2(2)=0D0     
      P2(1)=0D0     
      AMSP=(P2(4)+PHSU(4))**2-(P2(3)+PHSU(3))**2      
     $    -(P2(2)+PHSU(2))**2-(P2(1)+PHSU(1))**2      
* And the weight finally    
      WT1 = TRANP/TRAN  
      DELT=AMEL**2/TRANP    
      DO 200 I=1,NPH        
* Generated distribution         
C here some numerical regularization  
      DIST0 = 1D0/((ALF1(I)+DELS*BET1(I))*(BET1(I)+DELS*ALF1(I)))
      YGR=ALF1(I)+DELT*BET1(I)
      ZET=BET1(I)+DELT*ALF1(I)       
* Desired distribution = soft factor
      DIST1 = ALF1(I)*BET1(I)/(YGR*ZET)**2
      WTM(I)= DIST1/DIST0     
  200 CONTINUE
      RETURN 
C Event outside phase space
C Note that distinction is made (TRANP=-2,-1) to facilitate tests 
c event dropped due to: sum(beta) > 1
 800  CONTINUE
      TRANP = -1D0
      WT1   =  0D0
      RETURN
c event dropped due to: tranp < m^2, or earlier because YY2 < 0
 900  CONTINUE
      TRANP = -2D0
      WT1   =  0D0     
      END  
  
      SUBROUTINE POISSG(AVERG,NMAX,MULT ,RR)      
C     **************************************
* DIFFERS FROM THAT IN EXPAND DEC. 87         
* THIS GENERATES PHOTON MULTIPLICITY ACCORDING TO POISSON DISTRIBUTION
* INPUT:  AVERG = AVERAGE MULTIPLICITY        
*         NMAX  = MAXIMUM MULTIPLICITY        
* OUTPUT: MULT  = GENERATED MULTIPLICITY      
*         RR(1:100) LIST OF ORDERED UNIFORM RANDOM NUMBERS,    
*         A BYPRODUCT RESULT, TO BE EVENTUALLY USED FOR SOME FURTHER  
*         PURPOSE (I.E.  GENERATION OF PHOTON ENERGIES).       
*     **************************************      
      IMPLICIT REAL*8(A-H,O-Z)       
      REAL*8 RR(100)        
      DOUBLE PRECISION DRVEC(100)
      SAVE NFAIL
      DATA NFAIL/0/         
   50 NN=0    
      IF(NMAX.GT.100) GOTO 900       
      CALL VARRAN(DRVEC,NMAX)
      SUM=0D0         
      DO 100 IT=1,NMAX      
      RN = DRVEC(IT)
      Y= LOG(RN)    
      SUM=SUM+Y     
      NN=NN+1       
      IF(SUM.LT.-AVERG) GOTO 130     
      RR(NN)=SUM/(-AVERG)   
  100 CONTINUE      
      NFAIL=NFAIL+1         
      IF(NFAIL.GT.100) GOTO 900      
      GOTO 50       
  130 MULT =NN-1    
      RETURN        
  900 WRITE(6,*) ' POISSG: TO SMALL OR TO BIG NMAX',NMAX  
      STOP 
      END  
  
      SUBROUTINE KINO4(SVAR,TRAN,AMEL,AMSP,AMSQ,WTKK)   
*     ************************************************
* Kinematics, construction of momenta in CMS   
*     ************************************************
      IMPLICIT REAL*8(A-H,O-Z)      
      PARAMETER( PI =3.1415926535897932D0)
      COMMON / MOMS1  / TRANP,P1(4),P2(4),PHOT1(100,4),PHSU1(4),NPHOT1
      COMMON / MOMS2  / TRANQ,Q1(4),Q2(4),PHOT2(100,4),PHSU2(4),NPHOT2
      COMMON / MOMZ1  / AL1(50),BE1(50),WTM1(50),MK1(50)
      COMMON / MOMZ2  / AL2(50),BE2(50),WTM2(50),MK2(50)
      COMMON / PSIPHI / TH1,EXT1,EXB1,PSI1,EXW1,
     $                  TH2,EXT2,EXB2,PSI2,EXW2,EXE2,FIF,PHI
      SAVE   / MOMS1  /, / MOMS2  /, / MOMZ1  /, / MOMZ2  /, /PSIPHI/
      DOUBLE PRECISION DRVEC(100)
      REAL*8 PH(4)  
      DIMENSION QCM(4)

      BTEL=DSQRT(1D0-4D0*AMEL**2/SVAR)        
      WTKK=1D0
* Three azimuthal angles        
      CALL VARRAN(DRVEC,3)      
      PSI1= 2D0*PI*DRVEC(1)
      PSI2= 2D0*PI*DRVEC(2)
      PHI = 2D0*PI*DRVEC(3)
* Upper vertex: transf. from P2-P1 proper frame       
      CALL KLIPER(TRANP,AMEL,PHSU1,P2,TH1,EXT1,EXB1)  
* Lower vertex: transf. from q2-q1 proper frame       
      CALL KLIPER(TRANQ,AMEL,PHSU2,Q2,TH2,EXT2,EXB2)
* Define P1, Q1  in central QMS      
      P1(3)= -(TRAN+AMSP-AMEL**2)/SQRT(TRAN)/2D0      
      Q1(3)=  (TRAN+AMSQ-AMEL**2)/SQRT(TRAN)/2D0      
      RPQK=(Q1(3)+P1(3))/DSQRT(SVAR) 
* Correcting for electron mass       
C     PX2=SVAR*(SVAR+4D0*P1(3)*Q1(3))/((Q1(3)+P1(3))**2+SVAR)/4D0
C     PX2=PX2-AMEL**2       
      GPQK= P1(3)-Q1(3)     
      PX2=(BTEL**2*SVAR*(1D0+RPQK*RPQK)-GPQK*GPQK)/(1D0+RPQK*RPQK)/4D0
      IF(PX2.LE.0D0) GOTO 900  
      PX=SQRT(PX2)  
      P1(2)=  0D0   
      Q1(2)=  0D0   
      P1(1)=  -PX   
      Q1(1)=   PX   
      P1(4)=  SQRT(P1(1)**2+P1(2)**2+P1(3)**2+AMEL**2)         
      Q1(4)=  SQRT(Q1(1)**2+Q1(2)**2+Q1(3)**2+AMEL**2)         
* Correcting for electron mass       
C     BETP = SQRT(1D0-(AMEL/P1(4))**2)        
C     BETQ = SQRT(1D0-(AMEL/Q1(4))**2)        
C     DO 7 K=1,3    
C     P1(K)=BETP* P1(K)     
C   7 Q1(K)=BETQ* Q1(K)     
      EXW1=SQRT((P1(4)+P1(1))/(P1(4)-P1(1)))  
      EXW2=SQRT((Q1(4)+Q1(1))/(Q1(4)-Q1(1)))  
* Construct momentum transfer Q in CMS        
      QCM(4)=(AMSP-AMSQ)/SQRT(SVAR)/2D0       
      QMOD=SQRT(TRAN+QCM(4)**2)      
      QCM(3)=(-TRAN-AMSP/2D0-AMSQ/2D0+AMEL**2)/SQRT(SVAR-4D0*AMEL**2)
      QCM(2)=0D0    
      QCM(1)=SQRT(QMOD**2-QCM(3)**2) 
      FIF =ANGFIX(QCM(3),QCM(1))      
      EXE2=SQRT((QMOD+QCM(4))/(QMOD-QCM(4)))  
  
* Final set of transformations from QMSP and QMSQ to CMS       
* First branch, tranformed are P2, PHSU1, PHOT1
      CALL  PTRAL(TH1,EXT1,EXB1,PSI1,EXW1,EXE2,FIF,PHI,P2)     
      IF(NPHOT1.NE.0) THEN  
       CALL PTRAL(TH1,EXT1,EXB1,PSI1,EXW1,EXE2,FIF,PHI,PHSU1)  
       DO 20 I=1,NPHOT1     
       DO 15 K=1,4  
   15  PH(K)=PHOT1(I,K)     
       CALL PTRAL(TH1,EXT1,EXB1,PSI1,EXW1,EXE2,FIF,PHI,PH)     
       DO 16 K=1,4  
   16  PHOT1(I,K)=PH(K)     
   20  CONTINUE     
      ENDIF         
* Second branch, tranformed are Q2, PHSU2, PHOT2
      CALL  QTRAL(TH2,EXT2,EXB2,PSI2,EXW2,EXE2,FIF,PHI,Q2)     
      IF(NPHOT2.NE.0) THEN  
       CALL QTRAL(TH2,EXT2,EXB2,PSI2,EXW2,EXE2,FIF,PHI,PHSU2)  
       DO 30 I=1,NPHOT2     
       DO 25 K=1,4  
   25  PH(K)=PHOT2(I,K)     
       CALL QTRAL(TH2,EXT2,EXB2,PSI2,EXW2,EXE2,FIF,PHI,PH)     
       DO 26 K=1,4  
   26  PHOT2(I,K)=PH(K)     
   30  CONTINUE     
      ENDIF     
* Finally, beams P1 and Q1   
      CALL BXSTD3(EXE2,P1,P1)        
      CALL RXTOD2( FIF,P1,P1)        
      CALL BXSTD3(EXE2,Q1,Q1)        
      CALL RXTOD2( FIF,Q1,Q1)
      RETURN        
* Event outside phase space          
  900 WTKK=0D0      
      END  
  
      SUBROUTINE PTRAL(TH,EXT,EXB,PSI,EXW,EXE,FIF,PHI,P)       
*     **************************************************       
      IMPLICIT REAL*8(A-H,O-Z)       
      REAL*8 P(4)   
      CALL RXTOD2( -TH, P, P)        
      CALL BXSTD3( EXT, P, P)        
      CALL BOSTD1( EXB, P, P)        
      CALL RXTOD3( PSI, P, P)        
      CALL BOSTD1( EXW, P, P)        
      CALL BXSTD3( EXE, P, P)        
      CALL RXTOD2( FIF, P, P)        
      CALL RXTOD3( PHI, P, P)        
      END  
  
      SUBROUTINE QTRAL(TH,EXT,EXB,PSI,EXW,EXE,FIF,PHI,P)       
*     **************************************************       
      IMPLICIT REAL*8(A-H,O-Z)       
      PARAMETER( PI =3.1415926535897932D0) 
      REAL*8 P(4)   
      CALL RXTOD2( -TH, P, P)        
      CALL BXSTD3( EXT, P, P)        
      CALL BOSTD1( EXB, P, P)        
      CALL RXTOD3( PSI, P, P)        
      CALL RXTOD2(  PI, P, P)        
      CALL BOSTD1( EXW, P, P)        
      CALL BXSTD3( EXE, P, P)        
      CALL RXTOD2( FIF, P, P)        
      CALL RXTOD3( PHI, P, P)        
      END  
  
      SUBROUTINE KLIPER(TRANP,AMEL,PHSUM,P2,TH,EXT,EXB)
*     **************************************************
* Deals with Lorentz transf. from QQ1 to QQ frame
* where QQ1=P2-P1, QQ=P2+PHSUM-P1, TRANP=QQ1**2, P1**2=P2**2=AMEL**2 
* Input: TRANP,AMEL,PHSUM
* Output: P2,TH,EXT,EXB,PHSUM
* Here, TH, EXT, EXB are transformation params.
*     **************************************************
      IMPLICIT REAL*8(A-H,O-Z)       
      REAL*8 PHSUM(4),P2(4)          
      REAL*8 P1(4),QQ1(4)   
  
      BETEL=SQRT(1D0-4D0*AMEL**2/(TRANP+4D0*AMEL**2))          
* No photon         
      IF(PHSUM(4).EQ.0D0) THEN       
        P2(3)= SQRT(TRANP)/2D0       
        P2(4)= SQRT(P2(3)*P2(3)+AMEL**2)      
        P2(2)=0D0   
        P2(1)=0D0           
        TH =0D0         
        EXT=1D0         
        EXB=1D0         
      ELSE  
* One photon or more    
        ENER1=SQRT(TRANP)/2D0    
   
        P1(1)=0D0       
        P1(2)=0D0       
        P1(3)=-ENER1    
        P1(4)= SQRT(P1(3)*P1(3)+AMEL**2)  
   
        P2(1)=0D0       
        P2(2)=0D0       
        P2(3)= ENER1    
        P2(4)= SQRT(P2(3)*P2(3)+AMEL**2)  
   
        DO 33 I=1,4     
  33    QQ1(I)=P2(I)+PHSUM(I)-P1(I)       
   
* Rotation 2 puts QQ1 paralel to axis 3   
* Note that  PHSUM(2)=0 is already assured in MLTIBR!
        TH  =ANGFIX(QQ1(3),QQ1(1))         
        CALL RXTOD2(-TH ,QQ1,QQ1)         
        CALL RXTOD2(-TH ,P1,P1)  
* Boost 3 puts QQ1(4)=0          
        EXT = SQRT((QQ1(3)-QQ1(4))/(QQ1(3)+QQ1(4)))   
        CALL BXSTD3( EXT ,QQ1,QQ1)        
        CALL BXSTD3( EXT , P1, P1)        
        EXB = SQRT((P1(4)-P1(1))/(P1(4)+P1(1)))       
CC Testing obsolete appendix
CC Boost 1 puts P1 antiparallel to axis 3  
CC      CALL RXTOD2( -TH , P2, P2)
CC      CALL BXSTD3( EXT , P2, P2)
CC      CALL BOSTD1( EXB , P2, P2)
      ENDIF    
      END   

      SUBROUTINE MERGIK
*     *****************
* Transfer momenta and mark into proper commons        
* photons ordered according to cms energy 
* (the hardest in the first position)
!----------------------------------------------
!--- Modified by W. Placzek, Sept. 1995
!----------------------------------------------
      IMPLICIT REAL*8(A-H,O-Z)    
      COMMON / BHPAR2 / CMSENE,AMEL   
      SAVE   / BHPAR2 /
      COMMON / MOMS1  / TRANP,P1(4),P2(4),PHOT1(100,4),PHSU1(4),NPHOT1
      COMMON / MOMS2  / TRANQ,Q1(4),Q2(4),PHOT2(100,4),PHSU2(4),NPHOT2
      COMMON / MOMZ1  / AL1(50),BE1(50),WTM1(50),MK1(50)
      COMMON / MOMZ2  / AL2(50),BE2(50),WTM2(50),MK2(50)
      COMMON / MOMSET / PX1(4),QX1(4),PX2(4),QX2(4),PHOT(100,4),NPHOT
      COMMON / MARPKP / MARKP(100) 
      SAVE   / MOMS1/,/ MOMS2/,/ MOMZ1/,/ MOMZ2/,/ MOMSET/,/ MARPKP /
!WP: photon radiation tags
      Common / RadTag / idrad(100)
      SAVE   / RadTag /
      REAL*8 phx(4),pk(4)
!
      NPHOT=NPHOT1+NPHOT2   
      I1=1 
      I2=1 
      DO 207 I=1,NPHOT      
      IF(PHOT1(I1,4).GT.PHOT2(I2,4)) THEN     
         DO 205 K=1,4       
  205    PHOT( I,K)=PHOT1(I1,K)      
         MARKP(I)  =  MK1(I1)        
!WP: initial state radiation (positron line)
         IF (al1(I1)-be1(I1).gt.0) THEN
           idrad(I) = 1
!WP: final state radiation (positron line)
         ELSE
           idrad(I) = 3
         ENDIF
         I1=I1+1    
      ELSE          
         DO 206 K=1,4       
  206    PHOT( I,K)=PHOT2(I2,K)      
         MARKP(I)  =  MK2(I2)        
!WP: initial state radiation (electron line)
         IF (al2(I2)-be2(I2).gt.0) THEN
           idrad(I) = 2
!WP: final state radiation (electron line)
         ELSE
           idrad(I) = 4  
         ENDIF
         I2=I2+1 
      ENDIF         
  207 CONTINUE
      DO 300 K=1,4
      PX1(K)=P1(K)
      PX2(K)=P2(K)
      QX1(K)=Q1(K)
      QX2(K)=Q2(K)
  300 CONTINUE
!WP: balance initial-final energy-momentum when some photons removed
      DO k = 1,4
        phx(k) = PX2(k) + QX2(k)
        DO i = 1,NPHOT
          phx(k) = phx(k) + PHOT(i,k)
        ENDDO
      ENDDO
      ECMSf = DSQRT( phx(4)**2 -phx(3)**2 -phx(2)**2 -phx(1)**2 )
      IF (CMSENE-ECMSf.gt.1d-6) THEN
        CALL BOSTDQ( 1,phx,PX2,PX2)
        CALL BOSTDQ( 1,phx,QX2,QX2)
        refa = CMSENE/ECMSf 
        PX2(4) = PX2(4)*refa
        QX2(4) = QX2(4)*refa
        p2m  = DSQRT( PX2(1)**2 + PX2(2)**2 + PX2(3)**2 )
        q2m  = DSQRT( QX2(1)**2 + QX2(2)**2 + QX2(3)**2 )
        p2mr = DSQRT( PX2(4)**2 - AMEL**2 )
        q2mr = DSQRT( QX2(4)**2 - AMEL**2 )
        DO k = 1,3
          PX2(k) = PX2(k)*p2mr/p2m
          QX2(k) = QX2(k)*q2mr/q2m
        ENDDO
        DO i = 1,NPHOT
          DO k = 1,4
            pk(k) = PHOT(i,k)
          ENDDO
          CALL BOSTDQ( 1,phx,pk,pk)
          DO k = 1,4
            PHOT(i,k) = pk(k)*refa
          ENDDO
        ENDDO
      ENDIF
      END      

      SUBROUTINE WTinte(wt)
*     *********************
!---------------------------------------------------------------!
! This routine provides a weight for restoring up-down          !
! interference in YFS forrmula for Bhabha scattering.           !
! OUTPUT: wt  - the up-down interference weight                 !
!---------------------------------------------------------------!
! Written by: Wieslaw Placzek              Knoxville, May 1995  !
! Last update: 23.05.1995       by: W.P.                        !
!---------------------------------------------------------------!
      implicit REAL*8 (a-h,o-z)
      COMMON / BHPAR2 / CMSENE,AMEL      
      COMMON / MOMSET / p1(4),q1(4),p2(4),q2(4),phot(100,4),nphot
      SAVE  / BHPAR2 /, / MOMSET /
      COMMON / MOMZ1  / AL1(50),BE1(50),WTM1(50),MK1(50)
      COMMON / MOMZ2  / AL2(50),BE2(50),WTM2(50),MK2(50)
      save   / MOMZ1  /, / MOMZ2 /
!
      wt = 1
      if (nphot.EQ.0) RETURN
      ame = AMEL
      p1q1 = p1(4)*q1(4)-p1(3)*q1(3)-p1(2)*q1(2)-p1(1)*q1(1)
      p1p2 = p1(4)*p2(4)-p1(3)*p2(3)-p1(2)*p2(2)-p1(1)*p2(1)
      p1q2 = p1(4)*q2(4)-p1(3)*q2(3)-p1(2)*q2(2)-p1(1)*q2(1)
      q1p2 = q1(4)*p2(4)-q1(3)*p2(3)-q1(2)*p2(2)-q1(1)*p2(1)
      q1q2 = q1(4)*q2(4)-q1(3)*q2(3)-q1(2)*q2(2)-q1(1)*q2(1)
      p2q2 = p2(4)*q2(4)-p2(3)*q2(3)-p2(2)*q2(2)-p2(1)*q2(1)
      DO i = 1,nphot
! Scalar products of photon 4-momentum with other 4-momenta
        p1k = p1(4)*phot(i,4) -p1(3)*phot(i,3)
     &       -p1(2)*phot(i,2) -p1(1)*phot(i,1)
        q1k = q1(4)*phot(i,4) -q1(3)*phot(i,3)
     &       -q1(2)*phot(i,2) -q1(1)*phot(i,1)
        p2k = p2(4)*phot(i,4) -p2(3)*phot(i,3)
     &       -p2(2)*phot(i,2) -p2(1)*phot(i,1)
        q2k = q2(4)*phot(i,4) -q2(3)*phot(i,3)
     &       -q2(2)*phot(i,2) -q2(1)*phot(i,1) 
! Soft factors S-tilde (factor alpha/4pi^2 omitted!)
! Upper-line only
        sfu = 2*p1p2/(p1k*p2k) -(ame/p1k)**2 -(ame/p2k)**2
! Lower-line only
        sfl = 2*q1q2/(q1k*q2k) -(ame/q1k)**2 -(ame/q2k)**2
! Interference terms 
        sfi = 2*( p1q1/(p1k*q1k) -p1q2/(p1k*q2k) 
     &           -q1p2/(q1k*p2k) +p2q2/(p2k*q2k) )
! Weight
        wt = wt *( 1 + sfi/(sfu+sfl) )
      ENDDO
      END

C==================================================================
C======================== MODEL====================================
C==================================================================
      SUBROUTINE MODEL(MODE,WTM)
*     ****************************
! Interface to various models for hard bremsstrhlung matrix element
      IMPLICIT REAL*8(A-H,O-Z)   
      COMMON / WGTALL / WTMOD,WTCRUD,WTTRIG,WTSET(300)        
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / INOUT  / NINP,NOUT 
      SAVE   / WGTALL /, / INOUT  /, / BHPAR3 / 
      
      KEYMOD = MOD(KEYRAD,100)/10 
      IF (KEYMOD.EQ.1) THEN
        CALL MODEL1(MODE)
        WTM = WTSET(  1)
      ELSEIF (KEYMOD.EQ.2) THEN
        CALL MODEL2(MODE)
        WTM = WTSET(101)
      ELSE
        WRITE(NOUT,*) " +++++ MODEL: wrong keymod=",KEYMOD
        STOP 
      ENDIF
      END
      
      SUBROUTINE DUMPS(NOUT)     
*     **********************     
* THIS PRINTS OUT FOUR MOMENTA OF PHOTONS 
* ON OUTPUT UNIT NOUT
      IMPLICIT REAL*8(A-H,O-Z)   
      COMMON / MOMSET / P1(4),Q1(4),P2(4),Q2(4),PHOT(100,4),NPHOT  
      SAVE   / MOMSET /
      REAL*8 SUM(4)     
      WRITE(NOUT,*) '=====================DUMPS====================' 
      WRITE(NOUT,3100) ' P2',(P2(K),K=1,4)   
      WRITE(NOUT,3100) ' Q2',(Q2(K),K=1,4)   
      DO 100 I=1,NPHOT  
  100 WRITE(NOUT,3100) 'PHO',(PHOT(I,K),K=1,4)        
      DO 200 K=1,4      
  200 SUM(K)=P2(K)+Q2(K)         
      DO 210 I=1,NPHOT  
      DO 210 K=1,4      
  210 SUM(K)=SUM(K)+PHOT(I,K)    
      WRITE(NOUT,3100) 'SUM',(SUM(K),K=1,4)           
 3100 FORMAT(1X,A3,1X,5F18.13)   
      END      

      SUBROUTINE DUMPR(NUNIT,WORD,PP,QQ)    
C     **********************************
C 15 Jan 90 (SJ)
C prints twice dot-products of two four momentum PP and QQ
C more precisely:   2*PP.QQ  and  (PP+QQ).(PP+QQ)
C     ************************   
      IMPLICIT REAL*8(A-H,O-Z)
      CHARACTER*8 WORD          
      REAL*8 PP(4),QQ(4)  
      DOT1=2*(PP(4)*QQ(4)-PP(3)*QQ(3)-PP(2)*QQ(2)-PP(1)*QQ(1))
      DOT2=(PP(4)+QQ(4))**2-(PP(3)+QQ(3))**2
     $    -(PP(2)+QQ(2))**2-(PP(1)+QQ(1))**2
      WRITE(NUNIT,'(1X,A8,5(1X,F20.10))') WORD,DOT1,DOT2        
      END  

      SUBROUTINE GIBEA(CMSENE,AMEL,P1,P2)         
C     ***********************************         
C 15 Jan 90 (SJ)
C this originates from yfs302
C GIVEN CMS ENERGY (CMSENE) DEFINES BEAM MOMENTA IN CMS       
C     ***********************************         
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)         
      DIMENSION P1(*),P2(*)     
      BETEL=SQRT(1D0-4D0*AMEL**2/CMSENE**2)       
      P1(1)=  0D0   
      P1(2)=  0D0   
      P1(3)=  CMSENE/2D0*BETEL  
      P1(4)=  CMSENE/2D0        
      P2(1)=  0D0   
      P2(2)=  0D0   
      P2(3)= -CMSENE/2D0*BETEL  
      P2(4)=  CMSENE/2D0        
C----------------------------------------------------------------C
C                      The end of BHWID1                         C
C----------------------------------------------------------------C
      END           


