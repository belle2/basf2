*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                                                                                 //
*//                         Pseudo-CLASS  TauPair                                   //
*//                                                                                 //
*//       Purpose:                                                                  //
*//       (a) Interface to Simulation of TWO tau decays                             //
*//       (b) Calculates spin weight wt using GPS_MakeRho2 and introduces           //
*//           spin effects in tau decays by means of rejection with <wt>=1          //
*//       (c) Transforms decay products to CMS frame                                //
*//       (d) Interfaces Photos to Tauola                                           //
*//                                                                                 //
*//   Notes:                                                                        //
*//   The class is initialized by KK2f_Initialize                                   //
*//   It is called from KK2f_Make                                                   //
*//   It needs GPS to be initialized in order to calculate spin weight (final)      //
*//                                                                                 //
*//                                                                                 //
*//   For the moment this file contains the interface to tauola                     //
*//   The rest of code is in tauface.f and tauola.f                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////

      SUBROUTINE Taupair_Initialize(xpar)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//    Class initialization                                                         //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      INCLUDE 'BXformat.fi'
      INCLUDE 'Taupair.fi'
*
      DOUBLE PRECISION  xpar(*)
      DOUBLE PRECISION  HvecDummy(4)
      DOUBLE PRECISION  xk0qed
      INTEGER Jak1,Jak2
      INTEGER ITAUXPAR
      PARAMETER ( ITAUXPAR=2000)
      INTEGER TAUOLA_BBB
      COMMON /TAUOLA_BBB/ TAUOLA_BBB
*=====================================================================================

      m_out   = xpar(4)
      m_idyfs = xpar(8)   ! pointer for internal histograming

      m_IsInitialized = xpar(415)  ! General mask for tau chanel

C switches of tau+ tau- decay modes !!
      m_IFPHOT        = xpar(ITAUXPAR+4)   ! QED rad. in hadronic decays (PHOTOS)
      Jak1            = xpar(ITAUXPAR+1)   ! Decay Mask for first tau
      Jak2            = xpar(ITAUXPAR+2)   ! Decay Mask for second tau
      IF( (Jak1.EQ.-1) .AND. (Jak2.EQ.-1) ) m_IsInitialized = 0
      TAUOLA_BBB = xpar(ITAUXPAR+900)
      WRITE(m_out,*) 'Taupair_Init: Jak1, Jak2, IsInitialized, BBB = ', 
     c                              Jak1, Jak2, m_IsInitialized, TAUOLA_BBB
      m_KeyClone      = 1       ! dip-switch for cloning procedure, =1,2
      m_KeyClone      = 2       ! dip-switch for cloning procedure, =1,2
      WRITE(m_out,bxope)
      WRITE(m_out,bxtxt) ' KK interface of Tauola                  '
      WRITE(m_out,bxl1i) m_KeyClone,
     c                  'Cloning procedure   ','KeyClone','t01'
      WRITE(m_out,bxclo)

* Initialisation of tau decay package TAUOLA; ITAUXPAR is for indirect adressing.
      CALL INIETC(ITAUXPAR,xpar)
      IF( m_IsInitialized .EQ. 0) THEN
         WRITE(m_out,bxope) 
         WRITE(m_out,bxtxt) ' !!!!! Tauola inhibited !!!!    '
         WRITE(m_out,bxclo)
      ELSE
        CALL INIMAS(ITAUXPAR,xpar)
        IF (TAUOLA_BBB.EQ.1) THEN
           CALL INITDK_BBB(ITAUXPAR,xpar)
        ELSE
           CALL INITDK(ITAUXPAR,xpar)
        ENDIF
        xk0qed = 0.1D0            ! <=== It seems to be never used
        CALL INIPHY(xk0qed)
        IF (TAUOLA_BBB.EQ.1) THEN
           CALL DEKAY_BBB(-1,HvecDummy)
        ELSE
           CALL DEKAY(-1,HvecDummy)
        ENDIF

* Initialization of PHOTOS
        IF(m_IFPHOT .EQ. 1)   CALL PHOINI

* control weight
        CALL GLK_Mbook(m_idyfs+30,'Tau Pair: wt1, Spin Imprint weight $', 40, 4d0)
      ENDIF
      END

      SUBROUTINE Taupair_Finalize
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//    Printout of final statistics                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      INCLUDE 'BXformat.fi'
      INCLUDE 'Taupair.fi'
*
      DOUBLE PRECISION  HvecDummy(4)
      DOUBLE PRECISION   awt30,dwt30,WtSup30
      INTEGER TAUOLA_BBB
      COMMON /TAUOLA_BBB/ TAUOLA_BBB
*-------------------------------------------------------------------------------------
      IF( m_IsInitialized .EQ. 0) RETURN

      IF (TAUOLA_BBB.EQ.1) THEN
         CALL DEKAY_BBB(100,HvecDummy)
      ELSE
         CALL DEKAY(100,HvecDummy)
      ENDIF

      CALL  GLK_MgetAve(m_idyfs+30,awt30,dwt30,WtSup30)

      CALL  GLK_Mprint(m_idyfs+30)

      WRITE(m_out,bxope)
      WRITE(m_out,bxtxt) 'Tau Pair Finalize         '
      WRITE(m_out,bxl2f) awt30,dwt30,'Spin Imprint <wt1> ','wt1ave','a1'
      WRITE(m_out,bxl1f) WtSup30    ,'Maximum value wt1  ','wt1max','a2'
      WRITE(m_out,bxclo)

      END


      SUBROUTINE Taupair_Make1
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Main step in tau decau generation                                             //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
      INTEGER TAUOLA_BBB
      COMMON /TAUOLA_BBB/ TAUOLA_BBB
*-------------------------------------------------------------------------------------
      IF( m_IsInitialized .EQ. 0) RETURN
      IF (TAUOLA_BBB.EQ.1) THEN
         CALL DEKAY_BBB(1,m_HvecTau1)
         CALL DEKAY_BBB(2,m_HvecTau2)
      ELSE
         CALL DEKAY(1,m_HvecTau1)
         CALL DEKAY(2,m_HvecTau2)
      ENDIF
      END

      SUBROUTINE Taupair_ImprintSpin
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Impose spin effects in tau decays with rejection method.                      //
*//   In order to save CPU time each tau decay is cloned by Euler rotation.         //
*//   This is safe and mathematicaly correct!                                       //
*//   Remember:  the average weight due to introduction of spin effects             //
*//   in tau+ tau- system has be EXACTLY 1/4.                                       //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
*
      DOUBLE PRECISION       wt, wtmax, wt0,wt1,wt2
      DOUBLE PRECISION       HvecFer1(4), HvecFer2(4)
      INTEGER     loop
      REAL                   rvec(10)
*-------------------------------------------------------------------------------------
      INTEGER icont
      DATA icont /0/
*-------------------------------------------------------------------------------------
      IF( m_IsInitialized .EQ. 0) RETURN
      icont = icont+1
      loop=0
 1099 CONTINUE
         loop=loop+1
* Cloning tau decay by Euler rotation
         CALL   Taupair_Clone
         CALL   Taupair_GetHvectors( HvecFer1,HvecFer2)
         CALL      GPS_SetHvectors( HvecFer1,HvecFer2)
*
         CALL GPS_MakeRho2(wt0,wt1,wt2)
*        --------------------------
         wt = wt1
*****    IF(icont. LE. 10 ) WRITE(16,*) ' LOOP ,wt = ', loop, wt
         wtmax = 4d0
         CALL GLK_Mfill(m_idyfs+30, wt,  wtmax)
         CALL PseuMar_MakeVec(rvec,1)
      IF (wt .LT. wtmax*rvec(1)) GOTO 1099
      END
 
      SUBROUTINE Taupair_Make2
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//    Transform tau decay products to CMS, PDG book-keeping and Photos             //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
      DOUBLE PRECISION     p3(4),p4(4)
      INTEGER ih1,ih2
      INTEGER TAUOLA_BBB
      COMMON /TAUOLA_BBB/ TAUOLA_BBB

      CALL KarFin_GetFermions( p3,p4)
      CALL GPS_tralorPrepare(p3,1)
      CALL GPS_tralorPrepare(p4,2)

      CALL HepEvt_GetF(   ih1)  ! fermion is here &&&
      CALL HepEvt_GetFbar(ih2)  ! antifermion is here
      CALL Taupair_SetFermPos(ih1,ih2)  ! set ffbar positions in Tauola &&&

      IF (TAUOLA_BBB.EQ.1) THEN
         CALL DEKAY_BBB(1+10,m_HvecTau1)
         CALL DEKAY_BBB(2+10,m_HvecTau2)
      ELSE
         CALL DEKAY(1+10,m_HvecTau1)
         CALL DEKAY(2+10,m_HvecTau2)
      ENDIF
*/////////////////////////////////////////////////////////////////////////////////////
*//                    Photos comes last                                            //
*/////////////////////////////////////////////////////////////////////////////////////
      IF(m_IFPHOT .EQ. 1) THEN
         CALL PHOTOS(ih1)  ! Photos works on HepEvt
         CALL PHOTOS(ih2)
      ENDIF
      CALL pyhepc(2)       ! HepEvt-->Pythia
      END

      SUBROUTINE Taupair_Clone
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   This routine is strongly interrelated with Tralor  !!!                        //
*//                                                                                 //
*//   Cloning tau decays by additional rotation tau decay products with respect     //
*//   to frames  initialy used in the decay simulation.                             //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
*
      DOUBLE PRECISION       KinLib_AngPhi
      DOUBLE PRECISION       Habs1,Habs2
      DOUBLE PRECISION       hb1(4),hb2(4)
      REAL                   rrr(10)
*-------------------------------------------------------------------------------------
      INTEGER icont
      DATA icont /0/
*-------------------------------------------------------------------------------------
*/////////////////////////////////////////////////////////////////////////////////////
*//   Generation of random two independent Euler rotations                          //
*/////////////////////////////////////////////////////////////////////////////////////
      CALL PseuMar_MakeVec(rrr,3)
      m_alfa1  = 2d0*m_pi*rrr(3)        ! azimuthal angle in (0,2*pi)
      m_beta1  = ACOS(2d0*rrr(1)-1d0)   ! polar angle     in (0,  pi)
      m_gamma1 = 2d0*m_pi*rrr(2)        ! azimuthal angle in (0,2*pi)
*------------------------------------------------
      CALL PseuMar_MakeVec(rrr,3)
      m_alfa2  = 2d0*m_pi*rrr(3)        ! azimuthal angle in (0,2*pi)
      m_beta2  = ACOS(2d0*rrr(1)-1d0)   ! polar angle     in (0,  pi)
      m_gamma2 = 2d0*m_pi*rrr(2)        ! azimuthal angle in (0,2*pi)
      IF(m_KeyClone .EQ. 1) THEN
*/////////////////////////////////////////////////////////////////////////////////////
*//   Cloning tau decay with help of  Euler rotations FIRST method                  //
*/////////////////////////////////////////////////////////////////////////////////////
         Habs1 = DSQRT( m_HvecTau1(1)**2 +m_HvecTau1(2)**2 +m_HvecTau1(3)**2 )
         Habs2 = DSQRT( m_HvecTau2(1)**2 +m_HvecTau2(2)**2 +m_HvecTau2(3)**2 )
* Standart phi, theta for polarimeter fectors, phi in (0,2*pi), theta in (0,pi)
         IF(Habs1 .GT. 1d-5) THEN
            m_phi1  = KinLib_AngPhi( m_HvecTau1(1), m_HvecTau1(2) )
            m_thet1 = KinLib_AngPhi( m_HvecTau1(3), DSQRT(m_HvecTau1(1)**2+m_HvecTau1(2)**2) )
         ELSE
            m_phi1  =0d0
            m_thet1 =0d0
         ENDIF
         IF(Habs2 .GT. 1d-5) THEN
            m_phi2  = KinLib_AngPhi( m_HvecTau2(1), m_HvecTau2(2) )
            m_thet2 = KinLib_AngPhi( m_HvecTau2(3), DSQRT(m_HvecTau2(1)**2+m_HvecTau2(2)**2) )
         ELSE
            m_phi2  =0d0
            m_thet2 =0d0
         ENDIF
*(((((((((((((
* Test of angle definition, resulting hb1,hb2 should be on z-axis
*         IF(icont .LE. 60 ) THEN
*            icont=icont+1
*            CALL  KinLib_RotEulInv( m_thet1, m_phi1, m_HvecTau1, hb1)
*            CALL  KinLib_RotEulInv( m_thet2, m_phi2, m_HvecTau2, hb2)
*            CALL KinLib_VecPrint(6,'hb1=    ',hb1)
*            CALL KinLib_VecPrint(6,'hb2=    ',hb2)
*         ENDIF
*)))))))))))))
         m_HvClone1(1) =0d0
         m_HvClone1(2) =0d0
         m_HvClone1(3) =Habs1
         m_HvClone1(4) =1d0
         m_HvClone2(1) =0d0
         m_HvClone2(2) =0d0
         m_HvClone2(3) =Habs2
         m_HvClone2(4) =1d0
         CALL  KinLib_RotEul( m_beta1, m_gamma1, m_HvClone1, m_HvClone1)
         CALL  KinLib_RotEul( m_beta2, m_gamma2, m_HvClone2, m_HvClone2)
      ELSEIF(m_KeyClone .EQ. 2) THEN
*/////////////////////////////////////////////////////////////////////////////////////
*//   Cloning tau decay with help of  Euler rotations, SECOND method                //
*/////////////////////////////////////////////////////////////////////////////////////
         CALL KinLib_RotEuler(m_alfa1, m_beta1, m_gamma1, m_HvecTau1, m_HvClone1)
         CALL KinLib_RotEuler(m_alfa2, m_beta2, m_gamma2, m_HvecTau2, m_HvClone2)
      ELSE
         WRITE(m_out,*) ' ##### STOP in Taupair_Clone: wrong KeyClone= ',m_KeyClone
         WRITE(    *,*) ' ##### STOP in Taupair_Clone: wrong KeyClone= ',m_KeyClone
         STOP
      ENDIF
      END

      SUBROUTINE Tralo4(Kto,P,Q,AM)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   This routine is strongly interrelated with Taupair_Clone!!!                    //
*//                                                                                 //
*//  SUBSITUTE OF TRALO4                                                            // 
*//  TRALO4 is called in TAUOLA /hepevt/ interface to boost from tau+-              //
*//  restframe to lab. It includes rotations in tau restframe due to spin effect    //
*//  implementation                                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
*
      INTEGER   Kto
      REAL                 P(4),Q(4),AM
* locals
      DOUBLE PRECISION     Pd(4)
      INTEGER   k
* ------------------------------------------------------------
      AM = SQRT(ABS( P(4)**2 -P(3)**2 -P(2)**2 -P(1)**2 ))
* Translation from REAL              to DOUBLE PRECISION 
      DO k=1,4
         Pd(K)=P(K)
      ENDDO
      IF(m_KeyClone .EQ. 1) THEN
         IF(   Kto .EQ. 1) THEN
            CALL  KinLib_RotEulInv( m_thet1, m_phi1,   Pd,Pd)
            CALL  KinLib_RotEul(    m_beta1, m_gamma1, Pd,Pd)
         ELSEIF( Kto .EQ. 2) THEN
            CALL  KinLib_RotEulInv( m_thet2, m_phi2,   Pd,Pd)
            CALL  KinLib_RotEul(    m_beta2, m_gamma2, Pd,Pd)
         ELSE
            GOTO 900
         ENDIF
      ELSEIF(m_KeyClone .EQ. 2) THEN
         IF(     Kto .EQ. 1) THEN
            CALL KinLib_RotEuler( m_alfa1, m_beta1, m_gamma1, Pd,Pd)
         ELSEIF( Kto .EQ. 2) THEN
            CALL KinLib_RotEuler( m_alfa2, m_beta2, m_gamma2, Pd,Pd)
         ELSE
            GOTO 900
         ENDIF
      ELSE
         GOTO 901
      ENDIF
      CALL  GPS_TralorDoIt(KTO,Pd,Pd)
* Translation from DOUBLE PRECISION  to REAL             
      DO k=1,4
         Q(K)=Pd(K)
      ENDDO
      RETURN
*----------------------------------------------
 900  CONTINUE
      WRITE(*,*) ' ###### STOP in TRALO4: Wrong Kto = ',Kto
      WRITE(*,*) ' ###### STOP in TRALO4: Wrong Kto = ',Kto
      STOP
 901  CONTINUE
      WRITE(m_out,*) ' ##### STOP in Taupair_Tralo4: wrong KeyClone= ',m_KeyClone
      WRITE(    *,*) ' ##### STOP in Taupair_Tralo4: wrong KeyClone= ',m_KeyClone
      STOP
      END


      SUBROUTINE FILHEP(N,IST,ID,JMO1,JMO2,JDA1,JDA2,P4,PINV,PHFLAG)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  FILHEP of TAUOLA must be in single precission but double precision             // 
*//  HepEvt_Fil1 is its functional copy                                             //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      LOGICAL PHFLAG
      DOUBLE PRECISION  PINVD,P4D(4)
      REAL P4(4)
      DO k=1,4
        P4D(k)=P4(k)
      ENDDO
      PINVD=PINV
      CALL HepEvt_Fil1(N,IST,ID,JMO1,JMO2,JDA1,JDA2,P4D,PINVD,PHFLAG)
      END

*/////////////////////////////////////////////////////////////////////////////////////
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//           Setters and Getters of CLASS  Tauola                                  //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
*/////////////////////////////////////////////////////////////////////////////////////

      SUBROUTINE Taupair_SetKeyClone(KeyClone)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Setter to KeyClone                                                            //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
      INTEGER KeyClone
*
      m_KeyClone = KeyClone
      IF( KeyClone .LT. 1 .OR. KeyClone .GT. 2) THEN
         WRITE(m_out,*) ' ##### STOP in Taupair_SetKeyClone: wrong KeyClone= ',KeyClone
         WRITE(    *,*) ' ##### STOP in Taupair_SetKeyClone: wrong KeyClone= ',KeyClone
         STOP
      ENDIF
      END

      SUBROUTINE Taupair_GetIsInitialized(IsInitialized)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Get to know if Tauola is active (IsInitialized=1)                             //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
      INTEGER IsInitialized
*
      IsInitialized = m_IsInitialized
      END

      SUBROUTINE Taupair_GetHvectors(HvecFer1,HvecFer2)
*/////////////////////////////////////////////////////////////////////////////////////
*//   Geting clone of the tau lepton POLARIMETER vectors                            //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
      INTEGER k
      DOUBLE PRECISION  HvecFer1(4),HvecFer2(4)
*
      DO k=1,4
         HvecFer1( k) = m_HvClone1(k)
         HvecFer2( k) = m_HvClone2(k)
      ENDDO
      END

      SUBROUTINE Taupair_SetFermPos(np1,np2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Get to know if Tauola is active (IsInitialized=1)                             //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'Taupair.fi'
      INTEGER np1,np2
*
      m_np1 = np1
      m_np2 = np2
      END
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                End of   Pseudo-CLASS  Taoula                                    //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
 
