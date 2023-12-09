*////////////////////////////////////////////////////////////////////////////////
*//   ======================================================================   //
*//   ======================================================================   //
*//   ===========================KoralW=====================================   //
*//   ======================WW pair production==============================   //
*//   ======================================================================   //
*//   ======================================================================   //
*//   ======================= Version 1.42.3 ===============================   //
*//   ======================================================================   //
*//   =======================   March 1999   ===============================   //
*//   ======================================================================   //
*////////////////////////////////////////////////////////////////////////////////
*
*     Author list:
*          S. Jadach      (Stanislaw.Jadach@cern.ch)
*          W. Placzek     (Wieslaw.Placzek@cern.ch)
*          M. Skrzypek    (Maciej.Skrzypek@cern.ch)
*          B.F.L. Ward    (bflw@slac.stanford.edu)
*          Z. Was         (Zbigniew.Was@cern.ch)
*
*//////////////////////////////////////////////////////////////////////////////////////////
*// Principal entries:                                                                   //
*//   CALL KW_ReaDataX('./data_DEFAULTS', Length 1, 10000, xpar)  ! read defaults   //
*//   CALL KW_ReaDataX('./user.input', Length, 0, 10000, xpar)  ! read user input   //
*//   CALL KW_SetDataPath('path to data', path_len) ! sets the path to the data directory//
*//   CALL KW_Initialize(xpar)            ! Initialization                               //
*//   CALL KW_Make                        ! Generate one event                           //
*//   CALL KW_Finalize                    ! Final bookkeeping/printouts                  //
*//   CALL KW_GetXSecMC(XSecMC,XErrMC)    ! Get MC total xsection [pb]                   //
*//   CALL KW_GetNevMC(NevMC)             ! Get no. of MC events                         //
*// For more information see:                                                            //
*//   Complete documentation:            LongWriteUp,                                    //
*//   Highlights:                        README file                                     //
*//   Differences with previous vers.:   RELEASE.NOTES                                   //
*//   How to use the program:            Example of the main program KWdemo.f            //
*//   All possible Input data:           Data_DEFAULTS file                              //
*//////////////////////////////////////////////////////////////////////////////////////////

      SUBROUTINE KW_SetDataPath(data_path, path_len)
      IMPLICIT NONE
      BYTE           data_path(250)
      INTEGER        path_len, i
      CHARACTER*250  path
      COMMON / datapath / path
      WRITE (path,'(250a)') (data_path(i),i=1,path_len)
      END

      SUBROUTINE KW_Initialize(ecm, xpar_input)
*////////////////////////////////////////////////////////////////////////////////
*//                                                                            //
*//   Compulsory initialization of KoralW Mote Carlo Generator                 //
*//                                                                            //
*////////////////////////////////////////////////////////////////////////////////
      IMPLICIT DOUBLE PRECISION  (A-H,O-Z)
      SAVE
      INCLUDE   'KW.inc'
      REAL*8 ecm
      REAL*8  xpar_input( 10000) ! in main program
*////////////////////////////////////////////////////////////////////////////////
*//    Common blocks sorted in alphabetic order!!!!                            //
*//    They shoud GET OUT of here.                                             //
*////////////////////////////////////////////////////////////////////////////////
*   --Anomalous WWV Couplings, for WWamgc only
      DOUBLE COMPLEX    g1,kap,lam,g4,g5,kapt,lamt
      COMMON / ancoco / g1(2),kap(2),lam(2),g4(2),g5(2),kapt(2),lamt(2)
      COMMON / articut/ arbitr,arbitr1,themin,arbitr2  !   <-- cuts for selecto
      COMMON / bxfmts / bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g 
      CHARACTER*80      bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g       
      CHARACTER*80      bxl2c      
      COMMON / cumask / user_umask(202) ! user mask on final states
      COMMON / decdat / amafin(20), br(20)
      COMMON / inout  / ninp,nout     
      COMMON / KeyKey / KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      COMMON / libra  / jak1,jak2,itdkrc,ifphot,ifhadm,ifhadp 
                        ! TAUOLA, PHOTOS and JETSET
      COMMON / matpar / pi,ceuler     
      COMMON / phypar / alfinv,gpicob     
      COMMON / ranpar / Keyrnd
      COMMON / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf  
      COMMON / wekin2 / amaw,gammw,gmu,alphaw
* /wekin3/ params of non-established particles such as higgs 
* (to be activated by Ihig)
      COMMON / wekin3 / amhig,gamhig,Ihig
      COMMON / wgtall / wtcrud,wtmod,wtset(100) ! this common should go away
      COMMON / wt_max / wtmax,wtmax_cc03     
      COMMON / vvrec  / vvmin,vvmax,vv,beti                   
*//////////////////////////////////////////////////////////////////////
*//    Other variables and data statements                           //
*//////////////////////////////////////////////////////////////////////
      DIMENSION wt4f(9)         !<-- 4fermion weights
!-- The CKM mixing matrix and VV+ which should be =1 (V - unitary) 
      DIMENSION VCKM(3,3),VVH(3,3)
*//////////////////////////////////////////////////////////////////////
      CHARACTER*64 Logo(47)
      DATA Logo /
     $'***************************************************************',
     $'***************************************************************',
     $'***************************************************************',
     $'*  ###   ###                                   ###       ###  *',
     $'*  ###  ###  ####    ######      ##     ##     ###       ###  *',
     $'*  ### ###  ##  ##   ##   ##    ####    ##     ###       ###  *',
     $'*  ######  ##    ##  ##   ##   ##  ##   ##     ###       ###  *',
     $'*  ######  ##    ##  #####    ##    ##  ##     ###   #   ###  *',
     $'*  ### ###  ##  ##   ##  ##   ########  ##      ### ### ###   *',
     $'*  ###  ###  ####    ##   ##  ##    ##  #######  #### ####    *',
     $'*  ###   ###           version 1.42.3             ##   ##     *',
     $'***************************************************************',
     $'**********************   March 1999   *************************',
     $'***************************************************************',
     $'               Last modification:   3.17.1999                  ',
     $'***************************************************************',
     $'*  Written by:                                                *',
     $'*    S. Jadach      (Stanislaw.Jadach@cern.ch)                *',
     $'*    W. Placzek     (Wieslaw.Placzek@cern.ch)                 *',
     $'*    M. Skrzypek    (Maciej.Skrzypek@cern.ch)                 *',
     $'*    B.F.L. Ward    (bflw@slac.stanford.edu)                  *',
     $'*    Z. Was         (Zbigniew.Was@cern.ch)                    *',
     $'*  Papers:                                                    *',
     $'*    M. Skrzypek, S. Jadach, W. Placzek, Z. Was               *',
     $'*      CERN-TH/95-205, Jul 1995, CPC 94 (1996) 216            *',
     $'*    M. Skrzypek, S. Jadach, M. Martinez, W. Placzek, Z. Was  *',
     $'*      CERN-TH/95-246, Sep 1995, Phys. Lett. B372 (1996) 289  *',
     $'*    S. Jadach, W. Placzek, M. Skrzypek, B.F.L. Ward, Z. Was  *',
     $'*   CERN-TH/98-242, UTHEP-98-0702, Jul 1998, submitted to CPC *',
     $'*    M. Skrzypek, S. Jadach, W. Placzek, B.F.L. Ward, Z. Was  *',
     $'*   CERN-TH/99-06, UTHEP-98-1001, Jan 1999, proc. of RADCOR98 *',
     $'*  Related papers:                                            *',
     $'*    T. Ishikawa, Y. Kurihara, M. Skrzypek, Z. Was            *',
     $'*      CERN-TH/97-11, Jan 1997, Eur. Phys. J. C4 (1998) 75    *',
     $'*    S. Jadach, K. Zalewski                                   *',
     $'*    CERN-TH/97-29, Jan 1997, Acta Phys. Pol. B28 (1997) 1363 *',
     $'*  WWW:                                                       *',
     $'*    http://hpjmiady.ifj.edu.pl/                              *',
     $'*  Acknowledgements:                                          *',
     $'*    We acknowledge warmly very useful help of:               *',
     $'*      M. Martinez in testing versions 1.01 and 1.02,         *',
     $'*      M. Gruenewald and A. Valassi in testing version 1.21   *',
     $'*      S. Jezequel in testing versions 1.31-1.33              *',
     $'*      M. Witek in testing version 1.41                       *',
     $'*      M. Verzocchi in testing version 1.42                   *',
     $'***************************************************************',
     $' '/ 

* ...BX-formats for nice and flexible outputs                 
      BXOPE =  '(//1X,15(5H*****)    )'     
      BXTXT =  '(1X,1H*,                  A48,25X,    1H*)'   
      BXL1I =  '(1X,1H*,I17,                 16X, A20,A12,A7, 1X,1H*)'
      BXL1F =  '(1X,1H*,F17.8,               16X, A20,A12,A7, 1X,1H*)' 
      BXL2F =  '(1X,1H*,F17.8, 4H  +-, F11.8, 1X, A20,A12,A7, 1X,1H*)'
      BXL2C ='(1X,1H*,1H(,F14.8,3H +i,F13.7,1H),1X,A20,A12,A7,1X,1H*)'
      BXL1G =  '(1X,1H*,G17.8,               16X, A20,A12,A7, 1X,1H*)'
      BXL2G =  '(1X,1H*,G17.8, 4H  +-, G11.5, 1X, A20,A12,A7, 1X,1H*)'
      BXCLO =  '(1X,15(5H*****)/   )'       
*///////////////////////////////////////////////////////////
*//   Math. constants, should go to PARAMETER statement
      Pi=3.1415926535897932D0
      CEuler = 0.57721566D0                  
*////////////////////////////////////////////////////////////////////////////
*// xpar should be  essentialy an image of the input in the main program   //
*// In the present version it also plays role of additional "common block" //
*// communicating between subprograms, vide filexp.f and setmas_koralw.f   //
*// In order to split this double role I introduce xpar_input which is not //
*// modified (see exception below)  and local xpar which acts as additional//
*// address area residing in THIS program and sent as a pointer downwards. //
*// Now the main program (including Korwan) knows nothing about changes    //
*// in local the xpar done in filexp.                                      //
*// Exception is that xpar_input sends information outside for mode=1,2.   //
*// This role should disappear, and final XsecMC should have its "getter"  //
*// Of course, xpar_input is copied into xpar, see below:                  //
*////////////////////////////////////////////////////////////////////////////
      DO i = 1, 10000
         m_Xpar(i) = xpar_input(i)
      ENDDO
*////////////////////////////////////////////////////////////////
*//                                                            //
*//  m_Npar is now pure internal object (obsolete)             //
*//  below we translate m_Xpar--> m_Npar fot internal use      //
*//                                                            //
*////////////////////////////////////////////////////////////////
*     KeyRad = 1000*KeyCul +100*KeyNLL +10*KeyFSR +KeyISR
      m_Npar(1)= 
     $           +NINT(m_Xpar(1011))  ! KeyISR
     $        +10*NINT(m_Xpar(1012))  ! KeyFSR
     $       +100*NINT(m_Xpar(1013))  ! KeyNLL
     $      +1000*NINT(m_Xpar(1014))  ! KeyCul
      KeyRad = m_Npar(1)
*----------------------------------------------------------------
*     KeyPhy = 100000*KeyWu +10000*KeyRed +1000*KeySpn+100*KeyZet+10*KeyMas+KeyBra 
      m_Npar(2)= 
     $           +NINT(m_Xpar(1021))  ! KeyBra 
     $        +10*NINT(m_Xpar(1022))  ! KeyMas
     $       +100*NINT(m_Xpar(1023))  ! KeyZet
     $      +1000*NINT(m_Xpar(1024))  ! KeySpn
     $     +10000*NINT(m_Xpar(1025))  ! KeyRed
     $    +100000*NINT(m_Xpar(1026))  ! KeyWu
      KeyPhy = m_Npar(2)
*----------------------------------------------------------------
*     KeyTek = 100*KeySmp +10*KeyRnd +KeyWgt
      m_Npar(3)= 
     $           +NINT(m_Xpar(1031))  ! KeyWgt
     $        +10*NINT(m_Xpar(1032))  ! KeyRnd
     $       +100*NINT(m_Xpar(1033))  ! KeySmp
      KeyTek = m_Npar(3)
*----------------------------------------------------------------
*     KeyMis = 10000*KeyWon +1000*KeyZon+100*KeyAcc+10*Key4f +KeyMix
      m_Npar(4)= 
     $           +NINT(m_Xpar(1041))  ! KeyMix
     $        +10*NINT(m_Xpar(1042))  ! Key4f
     $       +100*NINT(m_Xpar(1043))  ! KeyAcc
     $      +1000*NINT(m_Xpar(1044))  ! KeyZon
     $     +10000*NINT(m_Xpar(1045))  ! KeyWon
      KeyMis = m_Npar(4)
*----------------------------------------------------------------
*     Other Keys
      m_Npar( 5)= NINT(m_Xpar(1055))  ! KeyDWM
      m_Npar( 6)= NINT(m_Xpar(1056))  ! KeyDWP
      m_Npar( 7)= NINT(m_Xpar(1057))  ! Nout
*
      m_Npar(21)= NINT(m_Xpar(1071))  ! JAK1
      m_Npar(22)= NINT(m_Xpar(1072))  ! JAK2
      m_Npar(23)= NINT(m_Xpar(1073))  ! ITDKRC
      m_Npar(24)= NINT(m_Xpar(1074))  ! IFPHOT
      m_Npar(25)= NINT(m_Xpar(1075))  ! IFHADM
      m_Npar(26)= NINT(m_Xpar(1076))  ! IFHADP
*----------------------------------------------------------------
* Umask
      DO i=101,302
         m_Npar(i)=NINT(m_Xpar(i+1000))
      ENDDO
*///////////////////////////////////////////////////////////
*//         end of translation m_Xpar-->m_Npar            //
*///////////////////////////////////////////////////////////
*-----------------------------------------------------------------------
* Below this line Npar Replaced with Xpar wherever possible!!!
*-----------------------------------------------------------------------
* these are realy used in KW class
      m_KeyIsr =NINT(m_Xpar(1011))
      m_KeyWgt =NINT(m_Xpar(1031))
      m_Key4f  =NINT(m_Xpar(1042))
      m_KeyAcc =NINT(m_Xpar(1043))
      m_KeySmp =NINT(m_Xpar(1033))
* for printout only, to be shifted to other classes
      KeyFSR =NINT(m_Xpar(1012))
      KeyNLL =NINT(m_Xpar(1013))
      KeyCul =NINT(m_Xpar(1014))
      KeyBra =NINT(m_Xpar(1021))
      KeyMas =NINT(m_Xpar(1022))
      KeyZet =NINT(m_Xpar(1023))
      KeySpn =NINT(m_Xpar(1024))
      KeyRed =NINT(m_Xpar(1025))
      KeyWu  =NINT(m_Xpar(1026))
      KeyRnd =NINT(m_Xpar(1032))
      KeyMix =NINT(m_Xpar(1041))
      KeyZon =NINT(m_Xpar(1044))
      KeyWon =NINT(m_Xpar(1045))
      KeyDWM =NINT(m_Xpar(1055))
      KeyDWP =NINT(m_Xpar(1056))
*-----------------------------------------------------------------------
* To be mooved to data???
* Higgs pre-sampler dipswitch
      IHIG=0
*-----------------------------------------------------------------------
      IF((KeyWon*KeyZon*(KeyDWP+KeyDWM) .NE. 0) .OR.
     $   (KeyWon .EQ. 0 .AND. KeyZon.eq.0       )     ) THEN
          WRITE(6,*) 'FILEXP==> inconsistent input: '
          WRITE(6,*) 'KeyWon=',KeyWon,'KeyZon=',KeyZon
          WRITE(6,*) 'KeyDWP=',KeyDWP,'KeyDWM=',KeyDWM
          STOP
      ENDIF
*///////////////////////////////////////////////////////////
*//            Loading Common blocks, cont.               //                  
*///////////////////////////////////////////////////////////
      amel    =m_Xpar(100)
      AlfInv  =m_Xpar(101)
      Gpicob  =m_Xpar(102)
      Nout   = NINT(m_xpar(1057))
      Jak1   = NINT(m_Xpar(1071))
      Jak2   = NINT(m_Xpar(1072))
      itdkrc = NINT(m_Xpar(1073))
      ifphot = NINT(m_Xpar(1074))
      ifhadm = NINT(m_Xpar(1075))
      ifhadp = NINT(m_Xpar(1076))

!       cmsene = m_Xpar(1)      
      print *, m_Xpar(1)
      print *, ecm
      IF ((m_Xpar(1).gt.0d0).and.(abs(m_Xpar(1)-ecm).gt.0.1d0)) THEN
        CALL koralw_warning_ecm(m_Xpar(1), ecm)
      ENDIF
      cmsene = ecm
      
      gmu    = m_Xpar(2)
      alfwin = m_Xpar(3)
      amaz   = m_Xpar(4)
      gammz  = m_Xpar(5)
      amaw   = m_Xpar(6)
      gammw  = m_Xpar(7)
      vvmin  = m_Xpar(8)
      vvmax  = m_Xpar(9)
      wtmax  = m_Xpar(10)
      amhig  = m_Xpar(11)
      gamhig = m_Xpar(12)
      m_alpha_s= m_Xpar(13)
      arbitr = m_Xpar(14)
      arbitr1= m_Xpar(15)
      themin = m_Xpar(16)
      arbitr2= m_Xpar(17)
      wtmax_cc03= m_Xpar(18)
      PReco  = m_Xpar(19)
      ene    = CMSene/2d0      
*????????????????????????????????????????????????????????
* ?????????????  too small for yfs3 !!!!!!!!!!!!!!!!!!!!!
*????????????????????????????????????????????????????????
      vvmax  = MIN( vvmax, 1d0-(amel/ene)**2 )                   
c?????????????????????????????????????????????
c re-used in KoralW
      m_Xpar(9) =VVMAX ! send it back !!!
c?????????????????????????????????????????????

*/////////////////////////////////////////////////////////////////////////
*//               If arbitr2=<0 reset it to the maximum value           //
      IF (arbitr2 .LE. 0d0) THEN
        arbitr2 = cmsene**2
c?????????????????????????????????????????????
c seems to be unused
c        m_Xpar(17) = arbitr2
c?????????????????????????????????????????????
      ENDIF
*/////////////////////////////////////////////////////////////////////////
*//           Is this realy used in Karlud???                           //
      IDE=2               
      IDF=2               
      XK0=3.D-3         
*/////////////////////////////////////////////////////////////////////////
*//                       users mask                                    //
      DO i=1,202
         user_umask(i)= NINT(m_Xpar(1100+i))
      ENDDO
*/////////////////////////////////////////////////////////////////////////
*//                         alpha_w                                     //
      alphaw = 1d0/ AlfWin
*/////////////////////////////////////////////////////////////////////////
*//         Electroweak renormalisation schemes                         //
*/////////////////////////////////////////////////////////////////////////
      IF(KeyMix .EQ. 2) THEN
* this option is turned into 'bare Born' 
* so, we reset ALFWIN to alfa_QED
         SINW2 = 1D0 -AMAW**2/AMAZ**2
         ALFWIN = alfinv
c??????????????????????????????????????
c re-used in setmas_koralw.f
         m_Xpar(3) = alfwin
c??????????????????????????????????????
         ALPHAW = 1D0/ ALFWIN
      ELSEIF(KeyMix .EQ. 1) THEN
!.. this option is turned into G_mu scheme, 
!.. so, we recalculate ALFWIN
         SINW2 = 1D0 -AMAW**2/AMAZ**2
         ALFWIN = pi/( sqrt(2d0)*gmu*amaw**2*sinw2 )
c??????????????????????????????????????
c re-used in setmas_koralw.f
         m_Xpar(3) = alfwin
c??????????????????????????????????????
         ALPHAW = 1D0/ ALFWIN
      ELSE 
* LEP2 workshop definition
         sinw2 = pi * alphaw /( sqrt(2d0) * amaw**2 * gmu )
      ENDIF
*/////////////////////////////////////////////////////////////////////////
*//            cuts for selecto removed for CC03                        //
*/////////////////////////////////////////////////////////////////////////
      IF (m_Key4f .EQ. 0) THEN
* no cuts for CC03 presampler
        arbitr = 0d0  !  min. vis p_t**2 
        arbitr1= 0d0  !  add. cut for e+e-ch+ 
        themin = 0d0  !  min. theta [rad] with beam   
        arbitr2= cmsene**2  !  max p_t**2 of photons for e+e-ch+ 
c????????????????????????????????????????
c seems to be unused
c        m_Xpar(14)=arbitr    
c        m_Xpar(15)=arbitr1   
c        m_Xpar(16)=themin    
c        m_Xpar(17)=arbitr2  
c????????????????????????????????????????
      ENDIF
*/////////////////////////////////////////////////////////////////////////
*//             alpha_s/pi for naive QCD corrections                    //
*/////////////////////////////////////////////////////////////////////////
      aspi = m_alpha_s/pi
*/////////////////////////////////////////////////////////////////////////
*//                                                                     //
*//           Branching ratios for W decay channels:                    //
*//                                                                     //
*/////////////////////////////////////////////////////////////////////////
      IF(  KeyBra .EQ. 0 )THEN
*/////////////////////////////////////////////////////////////////////////
*//                    Born values                                      //
*/////////////////////////////////////////////////////////////////////////
         BR(1)=(1D0/3D0)*(1D0+aspi)/(1D0+2D0/3D0*aspi) !  <== ud
         BR(2)=0D0                                     !  <== cd
         BR(3)=0D0                                     !  <== us
         BR(4)=(1D0/3D0)*(1D0+aspi)/(1D0+2D0/3D0*aspi) !  <== cs
         BR(5)=0D0                                     !  <== ub
         BR(6)=0D0                                     !  <== cb
         BR(7)=(1D0/9D0)           /(1D0+2D0/3D0*aspi) !  <== e
         BR(8)=(1D0/9D0)           /(1D0+2D0/3D0*aspi) !  <== mu
         BR(9)=(1D0/9D0)           /(1D0+2D0/3D0*aspi) !  <== tau
      ELSEIF(  KeyBra .EQ. 1 )THEN
*/////////////////////////////////////////////////////////////////////////
*//          Values of CKM and BRanchings for KeyBra = 1                //
*// note that these br ratios correspond to alfa_s=0.12 (gamma_W->el    //
*// constant) and to nontrivial CKM matrix simultaneously               //
*// this is 'bullet proof' default setting                              //
*/////////////////////////////////////////////////////////////////////////
         m_alpha_s = 0.12d0  ! make sure alpha_s is consistent
c??????????????????????????????????????????????
c re-used in setmas_koralw.f and KoralW
         m_Xpar(13)=m_alpha_s  ! <== send it back
c??????????????????????????????????????????????
         aspi = m_alpha_s/pi
         gammw=-1d0        ! make sure W width will be recalculated
         DO i=1,9
            BR(i) = m_Xpar(130 +i)
         ENDDO
      ELSEIF(  KeyBra .EQ. 2 )THEN
*/////////////////////////////////////////////////////////////////////////
*//              Default values of CKM and BRanchings                   //
*// Recalculate br. ratios from the CKM matrix and alpha_s according to //
*// theoretical formula of A. Denner, Fortschr. Phys. 41 (1993) 307.    //
*// Values of the CKM matrix elements from 1996 PDG Review:             //
*//  http://www-pdg.lbl.gov/pdg.html (mean values of given ranges)      //
*/////////////////////////////////////////////////////////////////////////
         VCKM(1,1) =m_Xpar(111)   ! V_ud  real part
         VCKM(1,2) =m_Xpar(112)   ! V_us  real part
         VCKM(1,3) =m_Xpar(113)   ! V_ub  real part
         VCKM(2,1) =m_Xpar(114)   ! V_cd  real part
         VCKM(2,2) =m_Xpar(115)   ! V_cs  real part
         VCKM(2,3) =m_Xpar(116)   ! V_cb  real part
         VCKM(3,1) =m_Xpar(117)   ! V_td  real part
         VCKM(3,2) =m_Xpar(118)   ! V_ts  real part
         VCKM(3,3) =m_Xpar(119)   ! V_tb  real part
* Unitarity check of the CKM matrix: VVH should be =1
         DO i = 1,3
         DO j = 1,3
           sum = 0d0
           DO k = 1,3
             sum = sum + VCKM(i,k)*VCKM(j,k)
           ENDDO
           VVH(i,j) = sum
         ENDDO
         ENDDO
* IBA formulae for branching ratios
         brlep = 1d0/9d0/(1 + 2d0/3d0*aspi)
         brqua = 3*brlep*(1 + aspi)
         BR(1) = VCKM(1,1)**2 *brqua  !  <== ud
         BR(2) = VCKM(2,1)**2 *brqua  !  <== cd
         BR(3) = VCKM(1,2)**2 *brqua  !  <== us
         BR(4) = VCKM(2,2)**2 *brqua  !  <== cs
         BR(5) = VCKM(1,3)**2 *brqua  !  <== ub
         BR(6) = VCKM(2,3)**2 *brqua  !  <== cb
         BR(7) = brlep                !  <== e
         BR(8) = brlep                !  <== mu
         BR(9) = brlep                !  <== tau  
* make sure W width will be recalculated       
         gammw =-1d0        
      ELSE
        WRITE(6,*)'filexp=> wrong KeyBra: ',Keybra
        STOP
      ENDIF
*///////////////////////////////////////////////////////////
*//  Check if requested final state has a ZERO br. ratio  //
      IF(KeyWon.NE.0 .AND. KeyZon.EQ.0) THEN
        IF(Keydwm.NE.0 .AND. Keydwp.NE.0 .AND. Keydwp.NE.Keydwm) THEN
          IF(br(Keydwm) .EQ.0d0 .OR. br(Keydwp) .EQ. 0d0 ) THEN
           WRITE(6,*)'filexp=> requested CKM-nondiagonal WW final state'
           WRITE(6,*)'has zero xsect if used with br. ratios specified'
           STOP
          ENDIF
        ENDIF
      ENDIF
*/////////////////////////////////////////////////////////////////////////
*//             W width recalculated on request                         //
*/////////////////////////////////////////////////////////////////////////
      IF ( gammw .LE. 0d0 ) THEN
         gwc  =  9d0 * Gmu * amaw**2 /( 6d0 * sqrt(2d0) * pi)
         gammw = amaw * gwc
*-- Naive QCD correction to the width
         gammw=gammw*(1D0+2D0/3D0*ASPI) 
c????????????????????????????????????????????
c re-used in setmas_koralw.f
         m_Xpar(7) = GAMMW  ! send it back !!!
c????????????????????????????????????????????
      ENDIF
*///////////////////////////////////////////////////////////////////
*//               final fermions masses                           //
*///////////////////////////////////////////////////////////////////
      DO i = 1,6
         amafin(   i) = m_Xpar(500+i*10 +6) ! leptons
         amafin(10+i) = m_Xpar(600+i*10 +6) ! quarks
      ENDDO
      amel   = amafin(11)       ! <---now probably not necessary ?????
      IF(  KeyMas .EQ. 0 ) THEN
         DO i = 1,6
            amafin(   i) = 0d0
            amafin(10+i) = 0d0
         ENDDO
      ENDIF
      IF(wtmax.LE.0d0) THEN
        wtmax=2d0
      ENDIF
      IF(wtmax_cc03 .LE. 0d0) THEN
        wtmax_cc03 = m_Xpar(151)
        IF(cmsene.GT.162) wtmax_cc03 = m_Xpar(152)
        IF(cmsene.GT.175) wtmax_cc03 = m_Xpar(153)
        IF(cmsene.GT.200) wtmax_cc03 = m_Xpar(154)
        IF(cmsene.GT.250) wtmax_cc03 = m_Xpar(155)
        IF(cmsene.GT.350) wtmax_cc03 = m_Xpar(156)
        IF(cmsene.GT.700) wtmax_cc03 = m_Xpar(157)
c?????????????????????????????????????????????
c seems to be unused
c        m_Xpar(18) = wtmax_cc03   ! send it back !!!
c?????????????????????????????????????????????
      ENDIF
!-- if WW-CC03 matrix el. is requested, use wtmax_cc03 instead of wtmax
      IF(m_Key4f .EQ. 0) THEN
        wtmax=wtmax_cc03
c?????????????????????????????????????????????
c seems to be unused
c        m_Xpar(10) = wtmax   ! send it back !!!
c?????????????????????????????????????????????
      ENDIF
*
      WRITE(6,   '(10X,A)') Logo
      WRITE(NOUT,'(10X,A)') Logo

      WRITE(NOUT,BXOPE)         
      WRITE(NOUT,BXTXT) '           KORALW input parameters used    '
      WRITE(NOUT,BXL1F) CMSENE,     'CMS energy total   ','CMSENE','I.0'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1I) KeyRad,     'QED super-switch   ','KeyRad','IQ1'
      WRITE(NOUT,BXL1I) m_KeyIsr,   'Init. state Rad.   ','KeyISR','IQ2'
      WRITE(NOUT,BXL1I) KeyFSR,     'Final state Rad.   ','KeyFSR','IQ3'
      WRITE(NOUT,BXL1I) KeyNLL,     'Next. To Leading   ','KeyNLL','IQ4'
      WRITE(NOUT,BXL1I) KeyCul,     'Coulomb corr.      ','KeyCul','IQ5'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1I) KeyPhy,     'Physics super-switc','KeyPhy','IP1'
      WRITE(NOUT,BXL1I) KeyRed,     'FS mass reduction  ','KeyRed','IP2'
      WRITE(NOUT,BXL1I) KeySpn,     'Spin in W decays   ','KeySpn','IP3'
      WRITE(NOUT,BXL1I) KeyZet,     'Z propag.          ','KeyZet','IP4'
      WRITE(NOUT,BXL1I) KeyMas,     'Mass kinematics.   ','KeyMas','IP5'
      WRITE(NOUT,BXL1I) KeyBra,     'Branching Rat.     ','KeyBra','IP6'
      WRITE(NOUT,BXL1I) KeyWu,      'W propag.          ','KeyWu ','IP7'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1I) KeyTek,     'Technical super-swi','KeyTek','IT1'
      WRITE(NOUT,BXL1I) m_KeySmp,   'presampler type    ','KeySmp','IT2'
      WRITE(NOUT,BXL1I) KeyRnd,     'rand Numb type     ','KeyRnd','IT3'
      WRITE(NOUT,BXL1I) m_KeyWgt,   'weighting  switch  ','KeyWgt','IT4'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1I) KeyMis,     'Miscelaneous       ','KeyMis','IM1'
      WRITE(NOUT,BXL1I) KeyMix,     'sinW2 input type   ','KeyMix','IM2'
      WRITE(NOUT,BXL1I) m_Key4f,    '4 fermion matr el  ','Key4f ','IM3'
      WRITE(NOUT,BXL1I) m_KeyAcc,   'Anomalous couplings','KeyAcc','IM4'
      WRITE(NOUT,BXL1I) KeyWon,     'WW type final state','KeyWon','IM5'
      WRITE(NOUT,BXL1I) KeyZon,     'ZZ type final state','KeyZon','IM6'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1I) KEYDWM,     'W-/Z decay mode    ','KEYDWM','ID1'
      WRITE(NOUT,BXL1I) KEYDWP,     'W+/Z decay mode    ','KEYDWP','ID2'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1F) GMU*1d5,    'G_mu * 1d5         ','GMU   ','I.1'
      WRITE(NOUT,BXL1F) ALFWIN,     'inv alpha_w        ','ALFWIN','I.2'
      WRITE(NOUT,BXL1F) AMAZ,       'Z mass   [GeV]     ','AMAZ  ','I.3'
      WRITE(NOUT,BXL1F) GAMMZ,      'Z width  [GeV]     ','GAMMZ ','I.4'
      WRITE(NOUT,BXL1F) AMAW,       'W mass   [GeV]     ','AMAW  ','I.5'
      WRITE(NOUT,BXL1F) GAMMW,      'W width  [GeV]     ','GAMMW ','I.6'
      WRITE(NOUT,BXL1F) VVMIN,      'dummy infrared cut ','VVMIN ','I.7'
      WRITE(NOUT,BXL1F) VVMAX,      'v_max ( =1 )       ','VVMAX ','I.8'
      WRITE(NOUT,BXL1F) WTMAX,      'max wt for rejectn.','WTMAX ','I.9'
      WRITE(NOUT,BXL1F) WTMAX_CC03, 'max wt for CC03 rej','WTMAX ','I10'
      WRITE(NOUT,BXL1F) m_alpha_s,  'alpha_s: QCD coupl.','ALPHAS','I11'
      WRITE(NOUT,BXL1F) PReco  ,    'Color Re-Con. Prob.','PReco ','I12'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1F) SINW2,      'sin(theta_W)**2    ','SINW2 ','I13'
      WRITE(NOUT,BXTXT)'***********************************************'
*!-----------------------------------------------------------------------
      WRITE(NOUT,BXTXT)'***********************************************'
      IF(Keyzet.eq.0) THEN 
        WRITE(NOUT,BXTXT) '  Z width in Z propagator: s/M_Z *GAMM_Z '
      ELSEIF(Keyzet.eq.1) THEN 
        WRITE(NOUT,BXTXT) '  Z width in Z propagator:   M_Z *GAMM_Z '
      ELSEIF(Keyzet.eq.2) THEN 
        WRITE(NOUT,BXTXT) '  Z width in Z propagator:   0           '
      ELSE
        WRITE(NOUT,BXTXT) '  FILEXP ==> wrong KeyZET =',Keyzet
        STOP
      ENDIF
      WRITE(NOUT,BXTXT)'***********************************************'
      IF(Keyspn.ne.1) THEN 
        WRITE(NOUT,BXTXT) '         WARNING!  spin in decays is OFF: '
        WRITE(NOUT,BXL1I) KeySPN, 'spin in decays switch','KeySPN','A13'
      WRITE(NOUT,BXTXT)'***********************************************'
      ENDIF
      IF (KeyBra.EQ.2) THEN
       WRITE(NOUT,BXTXT) '                                    '
       WRITE(NOUT,BXTXT) '                CKM matrix elements:'
       WRITE(NOUT,BXL1F) VCKM(1,1),       'V_ud','VCKM(1,1)','IV1'
       WRITE(NOUT,BXL1F) VCKM(1,2),       'V_us','VCKM(1,2)','IV2'
       WRITE(NOUT,BXL1F) VCKM(1,3),       'V_ub','VCKM(1,3)','IV3'
       WRITE(NOUT,BXL1F) VCKM(2,1),       'V_cd','VCKM(2,1)','IV4'
       WRITE(NOUT,BXL1F) VCKM(2,2),       'V_cs','VCKM(2,2)','IV5'
       WRITE(NOUT,BXL1F) VCKM(2,3),       'V_cb','VCKM(2,3)','IV6'
       WRITE(NOUT,BXL1F) VCKM(3,1),       'V_td','VCKM(3,1)','IV7'
       WRITE(NOUT,BXL1F) VCKM(3,2),       'V_ts','VCKM(3,2)','IV8'
       WRITE(NOUT,BXL1F) VCKM(3,3),       'V_tb','VCKM(3,3)','IV9'
       WRITE(NOUT,BXTXT)
     $          '              Unitarity check of the CKM matrix:'
       WRITE(NOUT,'(1X,1H*,20X,3f10.3,23X,1H*)')(VVH(1,j),j=1,3)
       WRITE(NOUT,'(1X,1H*,15X,5HVV+ =,3f10.3,23X,1H*)')(VVH(2,j),j=1,3)     
       WRITE(NOUT,'(1X,1H*,20X,3f10.3,23X,1H*)')(VVH(3,j),j=1,3)
      ENDIF
      WRITE(NOUT,BXTXT) '                                             '
      WRITE(NOUT,BXTXT) '                Branching ratios of W decays:'
      WRITE(NOUT,BXL1F) BR(1),            'ud','BR(1)','IB1'
      WRITE(NOUT,BXL1F) BR(2),            'cd','BR(2)','IB2'
      WRITE(NOUT,BXL1F) BR(3),            'us','BR(3)','IB3'
      WRITE(NOUT,BXL1F) BR(4),            'cs','BR(4)','IB4'
      WRITE(NOUT,BXL1F) BR(5),            'ub','BR(5)','IB5'
      WRITE(NOUT,BXL1F) BR(6),            'cb','BR(6)','IB6'
      WRITE(NOUT,BXL1F) BR(7),            ' e','BR(7)','IB7'
      WRITE(NOUT,BXL1F) BR(8),           ' mu','BR(8)','IB8'
      WRITE(NOUT,BXL1F) BR(9),          ' tau','BR(9)','IB9'

      WRITE(NOUT,BXTXT) '                              fermion masses:'
      WRITE(NOUT,BXL1F) AMAFIN(1),     ' d','AMAFIN(1)','IM1'
      WRITE(NOUT,BXL1F) AMAFIN(2),     ' u','AMAFIN(2)','IM2'
      WRITE(NOUT,BXL1F) AMAFIN(3),     ' s','AMAFIN(3)','IM3'
      WRITE(NOUT,BXL1F) AMAFIN(4),     ' c','AMAFIN(4)','IM4'
      WRITE(NOUT,BXL1F) AMAFIN(5),     ' b','AMAFIN(5)','IM5'
      WRITE(NOUT,BXL1F) AMAFIN(11),    ' e','AMAFIN(11)','IM6'
      WRITE(NOUT,BXL1F) AMAFIN(12),    've','AMAFIN(12)','IM7'
      WRITE(NOUT,BXL1F) AMAFIN(13),    'mu','AMAFIN(13)','IM8'
      WRITE(NOUT,BXL1F) AMAFIN(14),   'vmu','AMAFIN(14)','IM9'
      WRITE(NOUT,BXL1F) AMAFIN(15),   'tau','AMAFIN(15)','IM10'
      WRITE(NOUT,BXL1F) AMAFIN(16),  'vtau','AMAFIN(16)','IM11'
      WRITE(NOUT,BXTXT) '                                             '
      IF (m_KeySmp.NE.0) THEN
        WRITE(NOUT,BXTXT) ' Predefined cuts on final state fermions'
        WRITE(NOUT,BXL1F)arbitr, 'min. vis p_t**2    ','GeV^2','X2'
        WRITE(NOUT,BXL1F)arbitr1,'add. cut for e+e-ch+ch-','GeV^2','X3'
        WRITE(NOUT,BXL1G)themin, 'min. theta with beam','rad ','X6'
        WRITE(NOUT,BXL1F)arbitr2,'max. p_t**2 phot eexx','GeV^2','X3'
      ENDIF

      IF( m_KeyAcc .NE.0 ) THEN 
*!----------------------------------------------------------------------!
*! Setting up the anomalous couplings as given in the paper:            !
*!     K. Hagiwara, R.D. Peccei, D. Zeppenfeld and K. Hikasa,           !
*!                 Nucl. Phys. B282 (1987) 253;                         !
*!     see also: YR CERN-96-01, "Physics at LEP2" Vol. 1, p. 525.       !
*! The variables used in this routine correspond to the following       !
*! contants defined in the above paper:                                 !
*!           constant name     corresponding variable                   ! 
*!                g_1^V                g1(2)                            !
*!                kappa_V              kap(2)                           !
*!                lambda_V             lam(2)                           !
*!                g_4^V                g4(2)                            !
*!                g_5^V                g5(2)                            !
*!                kappa-tilde_V        kapt(2)                          !
*!                lambda-tilde_V       lamt(2)                          ! 
*!----------------------------------------------------------------------!      
         IF( m_KeyAcc .EQ. 1) THEN 
*!-- Set 1:
*!       --Set up constants OTHER than SM:
*!       --for WWgamma vertex
          g1(1)   = DCMPLX(m_Xpar(21),m_Xpar(31))
          kap(1)  = DCMPLX(m_Xpar(22),m_Xpar(32))
          lam(1)  = DCMPLX(m_Xpar(23),m_Xpar(33))
          g4(1)   = DCMPLX(m_Xpar(24),m_Xpar(34))
          g5(1)   = DCMPLX(m_Xpar(25),m_Xpar(35))
          kapt(1) = DCMPLX(m_Xpar(26),m_Xpar(36))
          lamt(1) = DCMPLX(m_Xpar(27),m_Xpar(37))
*!       --WWZ vertex
          g1(2)   = DCMPLX(m_Xpar(41),m_Xpar(51))
          kap(2)  = DCMPLX(m_Xpar(42),m_Xpar(52))
          lam(2)  = DCMPLX(m_Xpar(43),m_Xpar(53))
          g4(2)   = DCMPLX(m_Xpar(44),m_Xpar(54))
          g5(2)   = DCMPLX(m_Xpar(45),m_Xpar(55))
          kapt(2) = DCMPLX(m_Xpar(46),m_Xpar(56))
          lamt(2) = DCMPLX(m_Xpar(47),m_Xpar(57))
*!======================================================
*!====== Other TGC parametrizations disussed in: ======= 
*!== YR CERN-96-01, "Physics at LEP2" Vol. 1, p. 525. ==
*!======================================================
        ELSEIF (m_KeyAcc.EQ.2) THEN
*!-- Set 2:  
	  delta_Z = m_Xpar(61)
	  x_gamma = m_Xpar(62)
	  x_Z     = m_Xpar(63)
	  y_gamma = m_Xpar(64)
	  y_Z     = m_Xpar(65)
*!... Calculate general (internal) TGC's (cf. Hagiwara et al.)  
          tW = SQRT(SINW2/(1-SINW2))
*!       --for WWgamma vertex
          g1(1)   = 1
          kap(1)  = 1 + x_gamma 
          lam(1)  = y_gamma
          g4(1)   = 0
          g5(1)   = 0
          kapt(1) = 0
          lamt(1) = 0
*!       --WWZ vertex
          g1(2)   = 1 + tW*delta_Z 
          kap(2)  = 1 + tW*(x_Z + delta_Z)
          lam(2)  = y_Z
          g4(2)   = 0
          g5(2)   = 0
          kapt(2) = 0
          lamt(2) = 0
        ELSEIF (m_KeyAcc.EQ.3) THEN
*!-- Set 3:  
	  alpha_Wphi = m_Xpar(71)
	  alpha_Bphi = m_Xpar(72)
	  alpha_W    = m_Xpar(73)
*!... Calculate general (internal) TGC's (cf. Hagiwara et al.)  
          sW2 = SINW2
          cW2 = 1 - SINW2
*!       --for WWgamma vertex
          g1(1)   = 1
          kap(1)  = 1 + alpha_Wphi + alpha_Bphi
          lam(1)  = alpha_W
          g4(1)   = 0
          g5(1)   = 0
          kapt(1) = 0
          lamt(1) = 0
*!       --WWZ vertex
          g1(2)   = 1 + alpha_Wphi/cW2 
          kap(2)  = 1 + alpha_Wphi - sW2/cW2*alpha_Bphi
          lam(2)  = alpha_W
          g4(2)   = 0
          g5(2)   = 0
          kapt(2) = 0
          lamt(2) = 0
        ELSE
          write(6,*)'FILEXP==> Wrong KeyAcc: ',m_KeyAcc
          STOP
        ENDIF
*!
        WRITE(NOUT,BXTXT)' '
	IF (m_KeyAcc.EQ.2) THEN
          WRITE(NOUT,BXTXT)'Anomalous Couplings - set 2; YR CERN 96-01'
          WRITE(NOUT,BXTXT)'******************************************'
          WRITE(NOUT,BXL1F) delta_Z,'delta_Z','delta_Z','IA21'
          WRITE(NOUT,BXL1F) x_gamma,'x_gamma','x_gamma','IA22'
          WRITE(NOUT,BXL1F) x_Z    ,'x_Z    ','x_Z    ','IA23'
          WRITE(NOUT,BXL1F) y_gamma,'y_gamma','y_gamma','IA24'
          WRITE(NOUT,BXL1F) y_Z    ,'y_Z    ','y_Z    ','IA25'
	ELSEIF (m_KeyAcc.EQ.3) THEN
          WRITE(NOUT,BXTXT)'Anomalous Couplings - set 3; YR CERN 96-01'
          WRITE(NOUT,BXTXT)'******************************************'
          WRITE(NOUT,BXL1F) alpha_Wphi,'alpha_Wphi','alpha_Wphi','IA21'
          WRITE(NOUT,BXL1F) alpha_Bphi,'alpha_Bphi','alpha_Bphi','IA22'
          WRITE(NOUT,BXL1F) alpha_W   ,'alpha_W   ','alpha_W   ','IA23'
	ENDIF
        WRITE(NOUT,BXTXT)' '
*!
        WRITE(NOUT,BXTXT)'Internal Anomalous Couplings Activated'
        WRITE(NOUT,BXTXT)'Convention from:'
        WRITE(NOUT,BXTXT)
     $         'K.Hagiwara, R.D.Peccei, D.Zeppenfeld, K.Hikasa,'
        WRITE(NOUT,BXTXT)'                Nucl. Phys. B282 (1987) 253.'
        WRITE(NOUT,BXTXT)'                        for WWZ vertex'  
        WRITE(NOUT,BXL2C) g1(2),             'g_1^Z','g1(2)  ','IC21'
        WRITE(NOUT,BXL2C) kap(2),          'kappa_Z','kap(2) ','IC22'
        WRITE(NOUT,BXL2C) lam(2),         'lambda_Z','lam(2) ','IC23' 
        WRITE(NOUT,BXL2C) g4(2),             'g_4^Z','g4(2)  ','IC24'
        WRITE(NOUT,BXL2C) g5(2),             'g_5^Z','g5(2)  ','IC25'     
        WRITE(NOUT,BXL2C) kapt(2),   'kappa-tilde_Z','kapt(2)','IC26'       
        WRITE(NOUT,BXL2C) lamt(2),  'lambda-tilde_Z','lamt(2)','IC27'          
        WRITE(NOUT,BXTXT)'                    for WWg vertex (gamma)'  
        WRITE(NOUT,BXL2C) g1(1),             'g_1^g','g1(1)  ','IC21'
        WRITE(NOUT,BXL2C) kap(1),          'kappa_g','kap(1) ','IC22'
        WRITE(NOUT,BXL2C) lam(1),         'lambda_g','lam(1) ','IC23' 
        WRITE(NOUT,BXL2C) g4(1),             'g_4^g','g4(1)  ','IC24'
        WRITE(NOUT,BXL2C) g5(1),             'g_5^g','g5(1)  ','IC25'     
        WRITE(NOUT,BXL2C) kapt(1),   'kappa-tilde_g','kapt(1)','IC26'       
        WRITE(NOUT,BXL2C) lamt(1),  'lambda-tilde_g','lamt(1)','IC27'          
        WRITE(NOUT,BXTXT)' '
      ENDIF

      WRITE(NOUT,BXTXT) '                              DECAY LIBRARIES'
      WRITE(NOUT,BXL1I) JAK1,         'TAUOLA for W+' ,'JAK1','IL1'
      WRITE(NOUT,BXL1I) JAK2,         'TAUOLA for W-' ,'JAK2','IL2'
      WRITE(NOUT,BXL1I) ITDKRC,   'TAUOLA Ord(alpha)' ,'ITDKRC','IL3'
      WRITE(NOUT,BXL1I) IFPHOT,              'PHOTOS' ,'IFPHOT','IL4'
      WRITE(NOUT,BXL1I) IFHADM,       'JETSET for W-' ,'IFHADM','IL5'
      WRITE(NOUT,BXL1I) IFHADP,       'JETSET for W+' ,'IFHADP','IL6'
      WRITE(NOUT,BXCLO)         


*///////////////////////////////////////////////////////////////
*//        Compulsory Initialization of GLIBK                 //
*///////////////////////////////////////////////////////////////
      CALL glimit(50000)
      CALL goutpu(nout)
*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
* identificator for this generator
      m_IdGen = 7
* important histo which remembers total x-section
      CALL gmonit(-1, m_IdGen,1d0,1d0,1d0) ! m_IdGen=7
*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
* this is "pointer" for internal monitoring histograms/averages
      idyfs = 0
      m_svar=4*ene**2

      DO i=1,100
        wtset(i)=0
      ENDDO
*!!!!!!!!!! this should go out to tests [[[[[[[[[[[[[[[
* ============================================================
* let us keep for KORALW the glibk id-ent range from 2 to 1000
* ============================================================
* Principal weight 
      CALL gmonit(-1,idyfs+80,0d0,1d0,1d0)  ! total xs
      CALL gmonit(-1,idyfs+81,0d0,1d0,1d0)  ! xs for wt<0
      CALL gmonit(-1,idyfs+82,0d0,1d0,1d0)  ! xs for wt>wtmax
*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!]]]]]]]]]]]]]]]
      WRITE(6,*) 'KORALW  <-3>'
*-- initialization of qed part
      CALL karlud(-1,m_Xcrude,dum2,dum3)
      ievacc=0
      nevtot=0
      m_NevTru=0
*///////////////////////////////////////////////////////////////
*//     initialization tauola photos etc.                     //
*///////////////////////////////////////////////////////////////
         WRITE(6,*) '>>>>>>>> initialization tauola photos etc.'
         CALL  inietc(Jak1, Jak2, itdkrc, ifphot)
         CALL  inimas
         CALL  iniphx(0.01d0)   !<--What is this 0.01d0? should go to data!
         CALL  initdk
         CALL  phoini
*///////////////////////////////////////////////////////////////
*//  initialization of 4fermion matrix el. libraries          //
*///////////////////////////////////////////////////////////////
      IF( m_Key4f .NE. 0 ) THEN
        CALL ampinw(m_Xpar,m_Npar)
      ENDIF
*///////////////////////////////////////////////////////////////
      wtu=0d0
*-- activates 4fermion tests
      m_i_4f=1
*-- beta functions tests, activated with i_beta=1
      m_i_beta=1
      IF( m_KeyIsr .NE. 0 ) THEN
*-- initialize tests of beta functions
         IF(m_i_beta. EQ. 1) 
     $        CALL KW_beta_tests(-1,idyfs,m_Xcrude,wtkarl,wtset)
      ENDIF
*///////////////////////////////////////////////////////////////
*//               4fermion monitoring                         //
*///////////////////////////////////////////////////////////////
      IF(m_i_4f .EQ. 1) 
     $  CALL KW_f4_tests(-1,idyfs,m_Xcrude,wttot,wtboww,wtbo4f,wt4f) 
* monitoring xsections in different decay channels
      CALL decay_monit(-1,wtmod,m_Xcrude,m_svar,label_dumm,nout)
* counter for z_ libraries reset
      CALL z_counter(-1,idum)
      DO i=58,60
        CALL gmonit(-1,idyfs+i,0d0,1d0,1d0)
      ENDDO
      END       



      SUBROUTINE KW_Make
*//////////////////////////////////////////////////////////////////
*//                                                              //
*//   Generation of single Monte Carlo event                     //
*//                                                              //
*//                                                              //
*//////////////////////////////////////////////////////////////////
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      SAVE
      INCLUDE   'KW.inc'
*//////////////////////////////////////////////////////////////////
*//    Common blocks sorted in alphabetic order!!!!              //
*//////////////////////////////////////////////////////////////////
      COMMON / decays / IFlav(4), amdec(4) 
      COMMON / decdat / amafin(20), br(20)
      COMMON / cms_eff_momdec /
     $      effbeam1(4),effbeam2(4),effp1(4),effp2(4),effp3(4),effp4(4)
      COMMON / inout  / ninp,nout
* tauola, photos and jetset overall switches
      COMMON / libra  / jak1,jak2,itdkrc,ifphot,ifhadm,ifhadp 
!<-- only ifhadm,ifhadp used
      COMMON / momset / qeff1(4),qeff2(4),sphum(4),sphot(100,4),nphot
      COMMON / momdec / q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      COMMON / phypar / alfinv,gpicob
      COMMON / weking / ene,amaz,gammz,amel,amfin,xk0,sinw2,ide,idf
      COMMON / wekin2 / amaw,gammw,gmu,alphaw
      COMMON / wgtgen / wtves,wtyfs,wtborn !<-- only wtborn is used
      COMMON / wgtall / wtcrud,wtmod,wtset(100)
      COMMON / wt_max / wtmax,wtmax_cc03     
      COMMON / vvrec  / vvmin,vvmax,vv,beti                   
*//////////////////////////////////////////////////////////////////
*//    Other variables and data statements                       //
*//////////////////////////////////////////////////////////////////
      DIMENSION wt4f(9)         !<-- 4fermion weights
      DIMENSION drvec(100)      !<-- vector of random numbers
      DIMENSION bsp(4)
      DATA kardmp /1/ !<-- dipswitch kardmp: printout on wt over wtmax: 1-on, 0-off
*//////////////////////////////////////////////////////////////////
*//    End of declarations UFFF!!!                               //
*//////////////////////////////////////////////////////////////////
      m_NevTru=m_NevTru+1
! fill the counter (it counts m_NevTru)
      CALL z_counter(0,idum)
 200  CONTINUE
      nevtot=nevtot+1
      CALL karlud( 0,xcborn,wtkarl,wtdumm)
! find out the label
      CALL store_label(1,label)
c these states added to Born, 10/7/98 ms
c! check if requested final state is a CC03 + MIX + doubly CKM suppresses 
c! if so it is 'NC03'  and we suppress it
c! equivalently it is ZZ with CC03
c      CALL linear_to_WZ_label(1,label,icwm,icwp,if_z,if_w)
c      IF(m_Key4f.EQ.0 .AND. if_z.EQ.1) THEN
c        wtkarl=0d0
c        wtborn=0d0
c      ENDIF
      wtcrud = wtkarl
*     ============================
      IF( m_KeyWgt .EQ. 2 ) THEN
*     ============================
*********************************************************
*   Constant weight (wt=1) events for internal CC03     *
*********************************************************
         IF (wtcrud .NE. 0d0) THEN
           wtcc03 = wwborn(effp1,effp2,effp3,effp4,m_KeyAcc)
           DO i=1,4
             bsp(i) = effp1(i)+effp2(i)+effp3(i)+effp4(i)
           ENDDO
           sp = dmas2(bsp)
*-- Born level flux factor 1/2s'
           fluxf = 1d0/(2d0*sp)
           wtcc03 = wtcc03*fluxf*gpicob
!---------------------------
!-- pseudo-rejection loop for internal CC03
           CALL varran(drvec,1)
           rn = drvec(1)
           IF (wtcrud*wtcc03 .LT. rn*wtmax_cc03) THEN
             wtcrud = 0d0
           ELSE
             wtcrud = wtmax_cc03/wtcc03
           ENDIF
!---------------------------
         ENDIF
*      ===========================
       ENDIF
*      ===========================
********************************************************
*-- (CC03,4-fermions) + Coulomb + ACC + nQCD weights --*
********************************************************
      CALL KW_model_4f(wtcrud,wtboww,wtbo4f,wt4f,br,m_alpha_s,
     $              effp1,effp2,effp3,effp4,label,m_Key4f,m_KeyAcc)
!-- Total Born weight (4f) 
      wtborn = wtbo4f*gpicob

      IF( wtborn.NE.0d0 .AND. wtcrud.NE.0d0) THEN
!-- tohep sets into HEPEVT all generated particles.
!   it decays taus and generates bremsstrahlung
!   in tau and W decays.
!-- it must be called here since eexx_wt_cor needs PHOTOS (for now)
        CALL tohep
!-- If ISR switched ON, calculate betas and corrective weights
        IF( m_KeyIsr.NE.0 ) THEN
!-- Correcting "bad" QED t-channel weights for eexx final states 
          CALL eexx_wt_cor(wtcrud,m_svar,amel,iflav,vvmin,wtcort)
!-- QED ISR model weight
          CALL betar(alfinv,wtborn*wtcort,m_svar,amel,nphot,sphot,wtset)
        ELSE
          DO i=1,100
            wtset(i)=0d0
          ENDDO
        ENDIF
        wtset(40)=wtbo4f
        DO i4f=1,9
          wtset(40+i4f)=wt4f(i4f)
        ENDDO
      ELSE
        wtcrud=0d0
        wtborn=0d0
        DO i=1,100
          wtset(i)=0d0
        ENDDO
      ENDIF



* **********************************
*       Total (principal) weight
* **********************************
! add the option of downgrading the ISR part of principal weight
!     i_principal_weight=2       ! First  order ISR
!     i_principal_weight=3       ! Second order ISR
!     i_principal_weight=4       ! Third  order ISR
! by default we use the best one !

      i_principal_weight=4       ! Third  order ISR

      IF( m_KeyIsr .EQ. 0 ) THEN
        wttot    =wtcrud*wtborn
        wtset(1) =wtborn
      ELSE
        wttot    =wtcrud*wtset(i_principal_weight)
      ENDIF

!==============================
!== weights monitoring begin ==
!==
!-- phase space volume (crude weight, no Born) 
      CALL gmonit(0,idyfs+59,wtcrud, 0d0,0d0)
!-- CC03 Born, no betas
      wtbww = wtcrud*wtboww*gpicob
      CALL gmonit(0,idyfs+58,wtbww, wtmax_cc03,0d0)
!-- cc03 born OVER wtmax, no betas
      wtovr = MAX(0d0,wtbww - wtmax_cc03)
      CALL gmonit(0,idyfs+60,wtovr, 0d0,0d0)
!-- total weight monitoring
      CALL gmonit(0,idyfs+80,wttot,  wtmax,0d0)
!-- events with wt<0
      wtneg = MIN(wttot,0d0)
      CALL gmonit(0,idyfs+81,wtneg,  0d0,0d0)
!-- events with wt>wtmax
      wtovr = MAX(0d0,wttot-wtmax)
      CALL gmonit(0,idyfs+82,wtovr,  0d0,0d0)

!-- monitoring xsections in different decay channels
      CALL decay_monit(0,wttot,m_Xcrude,m_svar,label,nout)

!-- 4fermion monitoring
      IF(m_i_4f .EQ. 1) 
     $     CALL KW_f4_tests(0,idyfs,m_Xcrude,wttot,wtboww,wtbo4f,wt4f)

!-- tests of beta functions
      IF(m_KeyIsr.NE.0 .AND. m_i_beta. EQ. 1) 
     $       CALL KW_beta_tests(0,idyfs,m_Xcrude,wtcrud,wtset) 
!==
!== weights monitoring end   ==
!==============================

*     ==============================
      IF( m_KeyWgt .EQ. 0 ) THEN
*     ==============================
*     Constant weight (wt=1) events

!-- principal rejection loop
         CALL varran(drvec,1)
         rn = drvec(1)
         IF (wttot .LT. rn*wtmax ) GOTO 200

! ms 10/10/98         DO i=1,100
! ms 10/10/98            wtset(i)=0
! ms 10/10/98         ENDDO
         wtcrud = 1d0
! Principal event weight =1 now!
         wtmod  = 1d0
*     ==============================
      ELSEIF( m_KeyWgt .EQ. 1 ) THEN
*     ==============================
*        Variable weight events

! Principal event weight
         wtmod = wttot
! Remembers crude x-section and total number of events 
         CALL gmonit(  0, m_IdGen,      m_Xcrude, wtmax,0d0)
*     ==============================
      ELSEIF( m_KeyWgt .NE. 2) THEN
*     ==============================
         WRITE(6,*)'KORALW==>wrong Keywgt=',m_KeyWgt
         STOP
*     ==============================
      ENDIF
*     ==============================

*-------------------
* dump for debugging
*-------------------
      IF(m_KeySmp .EQ. 2  .AND.  kardmp .EQ. 1
     @   .AND.  wttot/wtmax .GE. 1d0) THEN
        wtu=max(wttot/wtmax,wtu)
        CALL mm_dumper(12,6,m_NevTru,wttot,wtu,wtmax,wtmod,wtbo4f,iflav)
        CALL ww_dumper(6,m_svar,amel,wtcort) 
      ELSEIF( m_KeySmp .EQ. 1  .AND.  kardmp .EQ. 1
     @   .AND. (wttot/wtmax .GT. 1d0  .OR.  m_NevTru .EQ. -3320) ) THEN
        wtu=max(wttot/wtmax,wtu)
        CALL zz_dumper(6,m_NevTru,wttot,wtu,wtmax,wtmod,wtbo4f,iflav)
        CALL ww_dumper(6,m_svar,amel,wtcort) 
      ELSEIF( m_KeySmp .EQ. 3  .AND.  kardmp .EQ. 1
     @    .AND.  wttot/wtmax .GT. 1d0 ) THEN
        wtu=max(wttot/wtmax,wtu)
        CALL mm_dumper(12,6,m_NevTru,wttot,wtu,wtmax,wtmod,wtbo4f,iflav)
        CALL ww_dumper(6,m_svar,amel,wtcort) 
        CALL zz_dumper_short(6)
      ENDIF
*-------------------
* END dump for debugging
*-------------------

* ccccccccccccccccccccccccccccccccc
      IF( wttot  .NE.  0d0) THEN
* ccccccccccccccccccccccccccccccccc
* tohep sets into hepevt all generated particles.
*       it decays taus and generates bremsstrahlung
*       in tau and W decays.
!WP (now called earlier)         CALL tohep
* and tohad moves to lund FORMAT.
* it hadronizes whatever requires.
         CALL tohad(ifhadm,ifhadp,PReco)
      ELSE
* some routine to set hepevt to 0 should be here <<<<<============
        CONTINUE
* ccccccccccccccccccccccccccccccccc
      ENDIF
* ccccccccccccccccccccccccccccccccc
      IF(  (m_NevTru .LE. 1 .OR. m_NevTru .EQ. 2000000) 
     $     .AND. wtkarl .GT. 0d0       ) THEN
         CALL dumpl(6,p1,p2,p3,p4,qeff1,qeff2,sphot,nphot)
         CALL dumpw(nout)
      ENDIF
*-- presampler channels monitoring
      IF( m_KeySmp  .EQ.  2 ) CALL pres_monit(0,wtcrud,wtmod,wtset)
!!! temporary, to monitor progress of the accuracy with statistics
      IF(mod(nevtot,200000).EQ.0) THEN
        CALL decay_monit(1,wtmod,m_Xcrude,m_svar,label_dumm,6)
      ENDIF
      END


      SUBROUTINE KW_Finalize
*//////////////////////////////////////////////////////////////////
*//                                                              //
*//   Final printouts and calculation of the total Xsection      //
*//                                                              //
*//   Final Xsections and statistics available from getters:     //
*//   KW_GetXSecMC(XSecMC,XErrMC)                                //
*//   KW_GetXSecNR(XSecNR,XErrNR)                                //
*//   KW_GetNevMC(NevMC)                                         //
*//                                                              //
*//////////////////////////////////////////////////////////////////
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      SAVE
      INCLUDE   'KW.inc'

      COMMON / bxfmts / bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g
      CHARACTER*80      bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g
      COMMON / inout  / ninp,nout
      COMMON / wt_max / wtmax,wtmax_cc03     
      DIMENSION wt4f(9)
      DIMENSION wtset(100)
      REAL pol(4)               !<-- single precision parameter for tauola

*--------------------------------------------------------------------------------
      CALL karlud(1,m_Xcrude,xcvesk,dumm1)
*-- presampler channels monitoring
      IF( m_KeySmp  .EQ.  2 ) CALL pres_monit(1,wtcrud,wtmod,wtset)
* ccccccccccccccccccccccccccccccccc
* final printouts of tauola
      IF( m_KeyWgt  .EQ.  0) THEN
         CALL dexay(100,pol)
      ENDIF
* ccccccccccccccccccccccccccccccccc
!---- stuff moved from karlud beg ----
*-- crude xs. no born
      CALL gmonit(1,idyfs+59,wtkacr,erkacr,parm3)
      CALL gmonit(2,idyfs+59,evacc,evneg,evove)
      nevneg = evneg
      nevtot = parm3
      WRITE(nout,bxope)
      WRITE(nout,bxtxt) '         KORALW  final  report '
      WRITE(nout,bxtxt) '               Window A        '
      WRITE(nout,bxtxt) '            WEIGHTED evts.     '
      WRITE(nout,bxtxt) '                               '
      WRITE(nout,bxtxt) ' ccru matrix element means:    '
      WRITE(nout,bxtxt) ' a) Born matrix element for CC03 processes  '
      WRITE(nout,bxtxt) ' b) technical crude m.e. for nc processes or'
      WRITE(nout,bxtxt) '    for keysmp .NE. 0                       '
      WRITE(nout,bxtxt) '                               '
      WRITE(nout,bxtxt) '     xsect with no matrix element   '
      WRITE(nout,bxl1i) nevtot,'total no of events      ','nevtot','a0'
      WRITE(nout,bxl1i) nevneg,'wtcrud < 0 evts         ','nevneg','a1'
      xskr   = m_Xcrude*wtkacr
      erkr   = xskr*erkacr
      WRITE(nout,bxl1g) m_Xcrude,'sigma_crude           ','Xcrude','a2'
      WRITE(nout,bxl2g) 
     $           wtkacr,erkacr,'<wtcrud>, rel err       ','wtkacr','a3'
      WRITE(nout,bxl2g)
     $              xskr,erkr,'phsp. vol, no beta-0     ','xskr  ','a4'
      WRITE(nout,bxtxt) ' '

*-- born xsection, total
      CALL gmonit(1,idyfs+58,wtkabo,erkabo,parm3)
      CALL gmonit(2,idyfs+58,evacc,evneg,evove)
      nevneg = evneg
      nevove = evove
      nevtot = parm3
      xskb0  = m_Xcrude*wtkabo
      erkb0  = xskb0*erkabo
      WRITE(nout,bxtxt) '                               '
      WRITE(nout,bxtxt) ' xsect with ccru matrix el. only, no betas'
      WRITE(nout,bxl1i) nevtot,'total no of events      ','nevtot','a5'
      WRITE(nout,bxl1i) nevneg,'wtcrud*wtborn <0 evts   ','nevneg','a6'
      WRITE(nout,bxl2g)
     $           wtkabo,erkabo,'<wtcrud*wtborn>, rel err','wtkabo','a7'
      WRITE(nout,bxl2g)
     $           xskb0,erkb0,  'sigma (born m.el.)      ','xska0','a8'

*-- born xsection from above wtmax
      CALL gmonit(1,idyfs+60,wtkabo,erkabo,parm3)
      xskb   = m_Xcrude*wtkabo
      erkb   = xskb*erkabo
      IF (xskb.NE.0d0) THEN
        xx=xskb/xskb0
        ee=erkb/xskb0
      ELSE
         xx=0d0
         ee=0d0
      ENDIF
      WRITE(nout,bxtxt) '                               '
      WRITE(nout,bxtxt) '     xsect over wtmax_cc03     '
      WRITE(nout,bxtxt) ' ccru matrix el. only, no betas'
      WRITE(nout,bxl1i) nevove,   'evts: wt>wtmax_cc03  ','nevove','a9'
      WRITE(nout,bxl2g) xskb,erkb,'sigma: wt>wtmax_cc03 ','xskabo','a10'
      WRITE(nout,bxl2g) xx,ee,    'relat sigma: wt>wtmax','xskabo','a11'
      WRITE(nout,bxclo)
!---- stuff moved from karludw end ----
      IF( m_KeyIsr  .NE.  0 ) THEN
*-- tests of beta functions
        IF(m_i_beta. EQ. 1) 
     $        CALL KW_beta_tests(1,idyfs,m_Xcrude,wtkarl,wtset)
      ENDIF
*-- best xsection printout, total and over
      CALL gmonit(1,idyfs+80,averwt,errela,evtot)
      m_XSecMC  = m_Xcrude*averwt
      m_XErrMC  = m_XSecMC*errela
      CALL gmonit(2,idyfs+80,evacc,evneg,evove)
      nevacc = evacc
      nevneg = evneg
      nevove = evove
      CALL gmonit(1,idyfs+81,averwn,erreln,evtot)
      xsneg   = averwn/averwt
      erneg   = xsneg*erreln
      CALL gmonit(1,idyfs+82,averwo,errelo,evtot)
      xsove   = averwo/averwt
      erove   = xsove*errelo
      WRITE(nout,bxope)
      WRITE(nout,bxtxt) '         KORALW  final  report '
      WRITE(nout,bxtxt) '               Window C        '
      WRITE(nout,bxtxt) '                               '
      WRITE(nout,bxtxt) '     BEST order total xsect.   '
      WRITE(nout,bxl1i)nevtot,       'total no of events ','nevtot','c1'
      WRITE(nout,bxl1i)m_NevTru,   'accepted events      ','NevTru','c2'
      WRITE(nout,bxl2g)
     $             m_XSecMC,m_XErrMC,'sigma_tot [pb]     ','xskabo','c3'
      WRITE(nout,bxl1f)errela,       'relative error     ','errela','c4'
      WRITE(nout,bxl1i)nevneg,       'events: wt<0       ','nevneg','c5'
      WRITE(nout,bxl2g)xsneg,erneg,  'xsec/xtot: wt<0    ','xsneg ','c6'
      WRITE(nout,bxl1i)nevove,       'events: wt>wtmax   ','nevove','c7'
      WRITE(nout,bxl2g)xsove,erove,  'xsec/xtot: wt>wtmax','xsove ','c8'
      WRITE(nout,bxclo)
      IF( m_Key4f  .NE.  0 ) THEN
!-- 4fermion monitoring
         IF(m_i_4f .EQ. 1) 
     $     CALL KW_f4_tests( 1,idyfs,m_Xcrude,wttot,wtboww,wtbo4f,wt4f)
      ENDIF
! monitoring xsections in different decay channels
      CALL decay_monit(1,wtmod,m_Xcrude,m_svar,label_dumm,nout)
**********
* on request also the printout for the photonic pre-tabulation
*********      CALL decay_monit(2,wtmod,m_Xcrude,m_svar,label_dumm,nout)
*-----------------------------------------------------------------------
      IF( m_KeyWgt .EQ. 0 ) THEN
        m_XSecNR      = m_XSecMC
        m_XErrNR      = m_XErrMC
      ELSE
        m_XSecNR      = m_Xcrude
        m_XErrNR      = 0d0
      ENDIF
      END


      SUBROUTINE KW_model_4f(wtcrud,wtboww,wtbo4f,wt4f,br,alpha_s,
     $                    effp1,effp2,effp3,effp4,label,Key4f,Keyacc)
!     ****************************************************************
! external weight connected with 4fermions
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      COMMON / MATPAR / PI,CEULER    
      SAVE / MATPAR /

      DIMENSION wt4f(9),br(20)
      DIMENSION effp1(4),effp2(4),effp3(4),effp4(4)
      DIMENSION bq1(4),bq2(4),bsp(4)

      DIMENSION ipdg(4)
      CHARACTER*3 chuman(4)

      IF (wtcrud .EQ. 0d0) THEN
        wtboww = 0d0
        wtbo4f = 0d0
        DO i4f=1,9
          wt4f(i4f)=0d0
        ENDDO
        RETURN
      ENDIF

! from now on WTCRUD <> 0 !

! common part
      DO i=1,4
        bq1(i) = effp1(i)+effp2(i)
        bq2(i) = effp3(i)+effp4(i)
        bsp(i) = bq1(i)+bq2(i)
      ENDDO
      s1 = dmas2(bq1)
      s2 = dmas2(bq2)
      sp = dmas2(bsp)
*-- Born level flux factor 1/2s'
      fluxf = 1d0/(2d0*sp)
*-- identify WW type final states
      CALL linear_to_WZ_label(1,label,iwm,iwp,if_z,if_w)
*-- Internal WW Born matrix element, not necesarily used later on
      wtborn = wwborn(effp1,effp2,effp3,effp4,Keyacc)
*-- Coulomb corr.
      IF( if_w .NE. 0 ) THEN
        cc = CulMC(sp,s1,s2)
      ELSE 
        cc = 1
      ENDIF
      wtboww = fluxf*wtborn*cc
      wtbo4f = wtboww
      DO i4f=1,9
        wt4f(i4f)=0d0
      ENDDO
        
      IF (Key4f.GT.0) THEN
!    1====================1
*-- External 4fermion matrix el.
        CALL ampext(wtmod4f,wt4f )
        IF( if_w .NE. 0 ) THEN
!      2======================2
*-- WW-extras
*-- SM CC03 Born
          IF (Keyacc .EQ. 0) THEN
            wtborn_sm = wtborn
          ELSE
            wtborn_sm = wwborn(effp1,effp2,effp3,effp4,0)
          ENDIF
*-- Divide off naive QCD correction from wtborn_sm if included
          CALL KW_Naive_QCD(label,br,wt_qcd)
*-- ACC & Coulomb corrections to CC03 Born (divide off naive QCD)
          dif_bc = (wtborn*cc - wtborn_sm)/wt_qcd
*-- 4fermion "improved" Born (naive QCD correction - multiplicative) 
          wtbo4f = (wtmod4f + dif_bc)*wt_qcd*fluxf
!--------- ms tests beg
!          rat_bc = wtborn*cc / wtborn_sm
!          wtbo4f = (wtmod4f*rat_bc)*fluxf
!--------- ms tests end
          DO i4f=1,9
             wt4f(i4f) = (wt4f(i4f) + dif_bc)*wt_qcd*fluxf
          ENDDO
        ELSE
!      2====2
*-- ZZ-extras
*-- alpha_s/pi for naive QCD corrections
          aspi = alpha_s/pi
*-- Naive QCD correction - multiplicative
          wt_qcd = 1d0
          CALL linear_to_pdg_label(1,label,ipdg,chuman)
          IF (abs(ipdg(1)) .LT. 10) wt_qcd = wt_qcd*(1 + aspi)
          IF (abs(ipdg(3)) .LT. 10) wt_qcd = wt_qcd*(1 + aspi)
*-- 4fermion "improved" Born 
          wtbo4f = wtmod4f*wt_qcd*fluxf
          DO i4f=1,9
             wt4f(i4f) = wt4f(i4f)*wt_qcd*fluxf
          ENDDO
          wtboww = 0d0
        ENDIF
!      2=====2
      ELSE
!    1====1 
        wtbo4f = wtboww
        DO i4f=1,9
          wt4f(i4f)=0d0
        ENDDO
      ENDIF
!    1=====1

      END

      SUBROUTINE KW_beta_tests(mode,idyfs,Xcrude,wtkarl,wtset)
!     ***********************************************
! beta functions related tests
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)  
      COMMON / matpar / pi,ceuler
      COMMON / phypar / alfinv,gpicob
 
      COMMON / bxfmts / bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g
      CHARACTER*80      bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g

      COMMON / inout  / ninp,nout

      DIMENSION wtset(*)

      IF(mode .EQ. -1) THEN
* Totals O(alf0-alf3)
         DO k=10,14
            CALL gmonit(-1,idyfs+k,0d0,1d0,1d0) ! 10-14
         ENDDO
* Betas O(alf0-alf3) and differences
         DO k=20,38
            CALL gmonit(-1,idyfs+k,0d0,1d0,1d0) ! 20-38
         ENDDO
      ELSEIF(mode .EQ. 0) THEN
*     Totals O(alf0-alf3)
         DO k=1,4
            CALL gmonit(0,idyfs+10+k,wtkarl*wtset(k),1d0,1d0)
         ENDDO
* Betas O(alf0-alf3)
         DO k=10,19
            CALL gmonit(0,idyfs+10+k,wtkarl*wtset(k),1d0,1d0)
         ENDDO
* Differences total O(alf0-alf3)
         CALL gmonit(0,idyfs+30,wtkarl*(wtset(2)-wtset(1)),1d0,1d0)
         CALL gmonit(0,idyfs+31,wtkarl*(wtset(3)-wtset(2)),1d0,1d0)
         CALL gmonit(0,idyfs+32,wtkarl*(wtset(4)-wtset(3)),1d0,1d0)
* bt01-bt00, bt10
         CALL gmonit(0,idyfs+33,wtkarl*(wtset(11)-wtset(10)),1d0,1d0)
* bt02-bt01, bt11-bt10, bt20
         CALL gmonit(0,idyfs+34,wtkarl*(wtset(13)-wtset(11)),1d0,1d0)
         CALL gmonit(0,idyfs+35,wtkarl*(wtset(14)-wtset(12)),1d0,1d0)
* bt03-bt02, bt12-bt11, bt21-bt20,bt30
         CALL gmonit(0,idyfs+36,wtkarl*(wtset(16)-wtset(13)),1d0,1d0)
         CALL gmonit(0,idyfs+37,wtkarl*(wtset(17)-wtset(14)),1d0,1d0)
         CALL gmonit(0,idyfs+38,wtkarl*(wtset(18)-wtset(15)),1d0,1d0)
      ELSEIF(mode .EQ. 1) THEN
        WRITE(nout,bxope)
        WRITE(nout,bxtxt) '         KORALW  final  report '
        WRITE(nout,bxtxt) '               Window B        '
        WRITE(nout,bxtxt) '            Xsec-s in [pb]     '
        WRITE(nout,bxtxt) '                               '
*****************************************************************
*****************************************************************
        CALL gmonit(1,idyfs+11,averwt,errela,evtot)
        xstot0   = Xcrude*averwt
        ertot0   = xstot0*errela
        WRITE(nout,bxl2g) xstot0,ertot0,'xsec total    ','O(alf0)','b3'
*****************************************************************
        CALL gmonit(1,idyfs+12,averwt,errela,evtot)
        xstot1   = Xcrude*averwt
        ertot1   = xstot1*errela
        WRITE(nout,bxl2g) xstot1,ertot1,'xsec total    ','O(alf1)','b4'
*****************************************************************
        CALL gmonit(1,idyfs+13,averwt,errela,evtot)
        xstot2   = Xcrude*averwt
        ertot2   = xstot2*errela
        WRITE(nout,bxl2g) xstot2,ertot2,'xsec total    ','O(alf2)','b5'
*****************************************************************
        CALL gmonit(1,idyfs+14,averwt,errela,evtot)
        xstot3   = Xcrude*averwt
        ertot3   = xstot3*errela
        WRITE(nout,bxl2g) xstot3,ertot3,'xsec total    ','O(alf3)','b6'
*****************************************************************
        CALL gmonit(2,idyfs+11,evacc1,evneg1,evove1)
        CALL gmonit(2,idyfs+12,evacc2,evneg2,evove2)
        CALL gmonit(2,idyfs+13,evacc3,evneg3,evove3)
        CALL gmonit(2,idyfs+14,evacc4,evneg4,evove4)
        neg0=evneg1
        neg1=evneg2
        neg2=evneg3
        neg3=evneg4
        WRITE(nout,bxl1i) neg0,         'wt<0  events  ','O(alf0)',' '
        WRITE(nout,bxl1i) neg1,         'wt<0  events  ','O(alf1)',' '
        WRITE(nout,bxl1i) neg2,         'wt<0  events  ','O(alf2)',' '
        WRITE(nout,bxl1i) neg2,         'wt<0  events  ','O(alf3)',' '
*****************************************************************
        CALL gmonit(1,idyfs+20,averwt,errela,evtot)
        xsbt00   = Xcrude*averwt
        erbt00   = xsbt00*errela
        WRITE(nout,bxl2g) xsbt00,erbt00,'xsec(beta00)  ','O(alf0)','b7'
*****************************************************************
        CALL gmonit(1,idyfs+21,averwt,errela,evtot)
        xsbt01   = Xcrude*averwt
        erbt01   = xsbt01*errela
        WRITE(nout,bxl2g) xsbt01,erbt01,'xsec(beta01)  ','O(alf1)','b8'
*****************************************************************
        CALL gmonit(1,idyfs+22,averwt,errela,evtot)
        xsbt10   = Xcrude*averwt
        erbt10   = xsbt10*errela
        WRITE(nout,bxl2g) xsbt10,erbt10,'xsec(beta10)  ','O(alf1)','b9'
*****************************************************************
        CALL gmonit(1,idyfs+23,averwt,errela,evtot)
        xsbt02   = Xcrude*averwt
        erbt02   = xsbt02*errela
        WRITE(nout,bxl2g) xsbt02,erbt02,'xsec(beta02)  ','O(alf2)','b10'
*****************************************************************
        CALL gmonit(1,idyfs+24,averwt,errela,evtot)
        xsbt11   = Xcrude*averwt
        erbt11   = xsbt11*errela
        WRITE(nout,bxl2g) xsbt11,erbt11,'xsec(beta11)  ','O(alf2)','b11'
*****************************************************************
        CALL gmonit(1,idyfs+25,averwt,errela,evtot)
        xsbt20   = Xcrude*averwt
        erbt20   = xsbt20*errela
        WRITE(nout,bxl2g) xsbt20,erbt20,'xsec(beta20)  ','O(alf2)','b12'
*****************************************************************
*****************************************************************
        CALL gmonit(1,idyfs+26,averwt,errela,evtot)
        xsbt03   = Xcrude*averwt
        erbt03   = xsbt03*errela
        WRITE(nout,bxl2g) xsbt03,erbt03,'xsec(beta03)  ','O(alf3)','b13'
*****************************************************************
        CALL gmonit(1,idyfs+27,averwt,errela,evtot)
        xsbt12   = Xcrude*averwt
        erbt12   = xsbt12*errela
        WRITE(nout,bxl2g) xsbt12,erbt12,'xsec(beta12)  ','O(alf3)','b14'
*****************************************************************
        CALL gmonit(1,idyfs+28,averwt,errela,evtot)
        xsbt21   = Xcrude*averwt
        erbt21   = xsbt21*errela
        WRITE(nout,bxl2g) xsbt21,erbt21,'xsec(beta21)  ','O(alf3)','b15'
*****************************************************************
        CALL gmonit(1,idyfs+29,averwt,errela,evtot)
        xsbt30   = Xcrude*averwt
        erbt30   = xsbt30*errela
        WRITE(nout,bxl2g) xsbt30,erbt30,'xsec(beta30)  ','O(alf3)','b16'
*****************************************************************
        WRITE(nout,bxtxt) ' xsec_tot differences '
*****************************************************************
        CALL gmonit(1,idyfs+30,averwt,errela,evtot)
        xsdel1   = Xcrude*averwt
        erdel1   = xsdel1*errela
        CALL gmonit(1,idyfs+31,averwt,errela,evtot)
        xsdel2   = Xcrude*averwt
        erdel2   = xsdel2*errela
        CALL gmonit(1,idyfs+32,averwt,errela,evtot)
        xsdel3   = Xcrude*averwt
        erdel3   = xsdel3*errela
        WRITE(nout,bxl2g) xsdel1,erdel1,'xstot(alf1-0)','O(alf1)','b17'
        WRITE(nout,bxl2g) xsdel2,erdel2,'xstot(alf2-1)','O(alf2)','b18'
        WRITE(nout,bxl2g) xsdel3,erdel3,'xstot(alf3-2)','O(alf3)','b19'
*****************************************************************
        WRITE(nout,bxtxt) ' betas differences '
*****************************************************************
        CALL gmonit(1,idyfs+33,averwt,errela,evtot)
        xsdt01   = Xcrude*averwt
        erdt01   = xsdt01*errela
        WRITE(nout,bxl2g) xsdt01,erdt01,'xs(beta01-00)','O(alf1)','b20'
        WRITE(nout,bxl2g) xsbt10,erbt10,'xs(beta10)   ','O(alf1)','b21'
*****************************************************************
        CALL gmonit(1,idyfs+34,averwt,errela,evtot)
        xsdt02   = Xcrude*averwt
        erdt02   = xsdt02*errela
        CALL gmonit(1,idyfs+35,averwt,errela,evtot)
        xsdt11   = Xcrude*averwt
        erdt11   = xsdt11*errela
        WRITE(nout,bxl2g) xsdt02,erdt02,'xs(beta02-01)','O(alf2)','b19'
        WRITE(nout,bxl2g) xsdt11,erdt11,'xs(beta11-10)','O(alf2)','b20'
        WRITE(nout,bxl2g) xsbt20,erbt20,'xs(beta20)   ','O(alf2)','b21'
*****************************************************************
        CALL gmonit(1,idyfs+36,averwt,errela,evtot)
        xsdt03   = Xcrude*averwt
        erdt03   = xsdt03*errela
        CALL gmonit(1,idyfs+37,averwt,errela,evtot)
        xsdt12   = Xcrude*averwt
        erdt12   = xsdt12*errela
        CALL gmonit(1,idyfs+38,averwt,errela,evtot)
        xsdt21   = Xcrude*averwt
        erdt21   = xsdt21*errela
        WRITE(nout,bxl2g) xsdt03,erdt03,'xs(beta03-02)','O(alf3)','b22'
        WRITE(nout,bxl2g) xsdt12,erdt12,'xs(beta12-11)','O(alf3)','b23'
        WRITE(nout,bxl2g) xsdt21,erdt21,'xs(beta21-20)','O(alf3)','b24'
        WRITE(nout,bxl2g) xsbt30,erbt30,'xs(beta30)   ','O(alf3)','b25'
*****************************************************************
        WRITE(nout,bxclo)
      ENDIF
      END

      SUBROUTINE KW_f4_tests(mode,idyfs,Xcrude,wttot,wtboww,wtbo4f,wt4f)
!     ***************************************************************
! 4fermion related tests
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)  
      COMMON / matpar / pi,ceuler
      COMMON / phypar / alfinv,gpicob
 
      COMMON / bxfmts / bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g
      CHARACTER*80      bxope,bxclo,bxtxt,bxl1i,bxl1f,bxl2f,bxl1g,bxl2g

      COMMON / inout  / ninp,nout

      DIMENSION wt4f(*)

      IF(mode .EQ. -1) THEN
*-- all 4-fermion
        DO k=91,96
          CALL gmonit(-1,idyfs+k,0d0,1d0,1d0) ! 91-96
        ENDDO
      ELSEIF(mode .EQ. 0) THEN
*-- 4-fermion monitoring
         IF (ABS(wtbo4f).GT.1d-50) THEN
           wttww = wtboww/wtbo4f*wttot
         ELSE
           wttww = 0d0
         ENDIF
         CALL gmonit(0,idyfs+91,wtbo4f,1d0,0d0)
         CALL gmonit(0,idyfs+92,wttot,1d0,0d0)
         CALL gmonit(0,idyfs+93,wttww,1d0,0d0)
         CALL gmonit(0,idyfs+94,wt4f(1),1d0,0d0)
         CALL gmonit(0,idyfs+95,wt4f(2),1d0,0d0)
         CALL gmonit(0,idyfs+96,wttot-wttww,1d0,0d0)
*-- END 4-fermion monitoring
      ELSEIF(mode .EQ. 1) THEN
*-- 4-fermion monitoring
        WRITE(nout,bxope)
        WRITE(nout,bxtxt) '         KORALW  final  report '
        WRITE(nout,bxtxt) '               Window D        '
        WRITE(nout,bxtxt) '                               '
        WRITE(nout,bxtxt) '     Complete 4-fermion process'
        WRITE(nout,bxtxt) '                               '
        WRITE(nout,bxtxt) '   I. Best ord. W-pair total xsect.    '
*****************************************************************
        CALL gmonit(1,idyfs+93,averwt,errela,evtot)
        WRITE(nout,bxl2g)
     $           averwt,errela,      '<wttww>: WW weight ','averwt','d1'
        xskbs   = Xcrude*averwt
        erkbs   = xskbs*errela
        WRITE(nout,bxl2g)xskbs,erkbs,'sigma_WW, best [pb]','xskabo','d2'
        WRITE(nout,bxtxt) '                               '
        WRITE(nout,bxtxt) '   II. Best ord. 4-fermion total xsect.'
*****************************************************************
        CALL gmonit(1,idyfs+91,averwt,errela,evtot)
        WRITE(nout,bxl2g)
     $          averwt,errela,       '<wtbo4f>, rel err  ','averwt','d3'
*****************************************************************
        CALL gmonit(1,idyfs+92,averwt,errela,evtot)
        WRITE(nout,bxl2g)
     $          averwt,errela,       '<wttot>,rel err    ','averwt','d4'
        xskbb   = Xcrude*averwt
        erkbb   = xskbb*errela
        WRITE(nout,bxl2g)xskbb,erkbb,'sigma_4f, best [pb]','xskabo','d5'
        stob = 1- xskbs/xskbb
        stober = dsqrt( (erkbs*xskbb)**2 +(erkbb*xskbs)**2 ) / xskbb**2
        WRITE(nout,bxl2g)stob,stober,'sigma 1-Wpair/4ferm','1-d2/5','d6'
*****************************************************************
        CALL gmonit(1,idyfs+96,averwt,errela,evtot)
        xskbd   = Xcrude*averwt
        erkbd   = xskbd*errela
        stob = xskbd/xskbb
        stober = dsqrt( (erkbd*xskbb)**2 +(erkbb*xskbd)**2 ) / xskbb**2
        WRITE(nout,bxl2g)stob,stober,'sigma 1-Wpair/4ferm','wtbgr ','d7'
        WRITE(nout,bxclo)
      ENDIF
*****************************************************************
      END


      SUBROUTINE KW_Naive_QCD(label,br,wt_qcd)
!******************************************
! This routine adds the naive QCD correction to the external matrix el.
! It is done only for the WW final states, and is justified for CC03.
! Also the effective nontrivial CKM is introduced on request.
! Everything is done based solely on the deviation of branching ratios
! from the 1/3, 1/9 settings.

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION br(20),br0(20)
      DATA init /0/
      SAVE
 
      IF(init .EQ. 0) THEN
         init=1

         br0(1)=(1d0/3d0) !  <== ud
         br0(2)=0d0       !  <== cd
         br0(3)=0d0       !  <== us
         br0(4)=(1d0/3d0) !  <== cs
         br0(5)=0d0       !  <== ub
         br0(6)=0d0       !  <== cb
         br0(7)=(1d0/9d0) !  <== e
         br0(8)=(1d0/9d0) !  <== mu
         br0(9)=(1d0/9d0) !  <== tau
      ENDIF
! do not redefine off-diagonal states, they are corrected already in
! Born
! off-diagonal are also SINGLE off-diag.  m.s. 3/13/98
         
      CALL linear_to_WZ_label(1,label,iwm,iwp,if_z,if_w)
      IF( if_w .NE. 0 ) THEN
         IF(br0(iwm) .NE. 0d0 .AND. br0(iwp) .NE. 0d0) THEN
           wm = (br(iwm)/br(7)) / (br0(iwm)/br0(7))
           wp = (br(iwp)/br(7)) / (br0(iwp)/br0(7))
         ELSE
           wm = 1d0
           wp = 1d0
         ENDIF
         wt_q= wm*wp
      ELSE
         wt_q=1d0
      ENDIF
           
      wt_qcd=wt_q
          
      END

      SUBROUTINE KW_ReadMask(DiskFile,iTalk,xpar)
*///////////////////////////////////////////////////////////////////
*//                                                               //
*//   DiskFile  = input file to read                              //
*///////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      CHARACTER*250 DiskFile
      REAL*8 xpar(*)
      INTEGER iTalk
*
* User mask on final states 
      INTEGER i_umask(202)
      CHARACTER*1 chdum
      CHARACTER*60 comm60
      CHARACTER*6  beg6
      INTEGER ninp,i,j,line
*--------------------------------------------
      IF(iTalk .EQ. 1) THEN
         WRITE(  *,*) '**************************'
         WRITE(  *,*) '*   KW_ReadMask starts   *'
         WRITE(  *,*) '**************************'
      ENDIF
* read user umask from file only here 
      ninp=13
      OPEN(unit=ninp, file=DiskFile, status='unknown')

* Search for 'BeginM'
      DO line =1,10000
         READ(ninp,'(a6,a)') beg6,comm60
         IF(beg6 .EQ. 'BeginM') THEN
            IF(iTalk .EQ. 1)   WRITE( *,'(a6,a)') beg6,comm60
            GOTO 200
         ENDIF
      ENDDO
 200  CONTINUE
*
      READ(ninp,'(a1)') chdum
      READ(ninp,'(a1)') chdum
      DO i=1,9
         READ(ninp,*) (i_umask((i-1)*9+j),j=1,9)
         IF(iTalk .EQ. 1) WRITE( *,'(20i5)') 
     $                (i_umask((i-1)*9+j),j=1,9)
      ENDDO
      READ(ninp,'(a1)') chdum
      READ(ninp,'(a1)') chdum
      DO i=1,11
         READ(ninp,*) (i_umask(81+(i-1)*11+j),j=1,11)
         IF(iTalk .EQ. 1) WRITE( *,'(20i5)') 
     $                (i_umask(81+(i-1)*11+j),j=1,11)
      ENDDO
*
      DO i=1,202
         xpar(1100+i)=i_umask(i)
      ENDDO
      CLOSE(ninp)
      IF(iTalk .EQ. 1) THEN
         WRITE(  *,*) '**************************'
         WRITE(  *,*) '*    KW_ReadMask Ends    *'
         WRITE(  *,*) '**************************'
      ENDIF
      END

      SUBROUTINE KW_ReaDataX(DiskFileByte,Length,iReset,imax,xpar)
*///////////////////////////////////////////////////////////////////
*//                                                               //
*//   DiskFile  = input file to read                              //
*//   imax   = maximum index in xpar                              //
*//   iReset = 1, resets xpar to 0d0                              //
*//   iTalk=1,     prints echo into standard input                //
*//                                                               //
*//   Single data card is:    (a1,i4,d15.0,a60)                   //
*//   First data card: BeginX                                     //
*//   Last  data card: EndX                                       //
*//   First character * defines comment card!                     //
*//                                                               //
*///////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      BYTE           DiskFileByte(250)
      INTEGER        Length, a
      CHARACTER*250  DiskFile
      REAL*8 xpar(*)
      CHARACTER*6 beg6
      CHARACTER*4 end4
      CHARACTER*1 mark1
      CHARACTER*60 comm60
      CHARACTER*80 comm80
      INTEGER  imax,iReset,iTalk
*
      INTEGER   ninp,i,line,index
      REAL*8   value

      WRITE (DiskFile,'(250a)') (DiskFileByte(a),a=1,Length)
*
*////////////////////////////////////////
*//  Clear xpar and read default Umask //
*////////////////////////////////////////
      iTalk = 1
      IF(iReset .EQ. 1 ) THEN
         iTalk = 0
         DO i=1,imax
            xpar(i)=0d0
         ENDDO
         CALL KW_ReadMask(DiskFile,iTalk,xpar)
      ENDIF
      ninp = 13
      OPEN(ninp,file=DiskFile)
      IF(iTalk .EQ. 1) THEN
         WRITE(  *,*) '**************************'
         WRITE(  *,*) '*   KW_ReaDataX starts   *'
         WRITE(  *,*) '**************************'
      ENDIF
* Search for 'BeginX'
      DO line =1,10000
         READ(ninp,'(a6,a)') beg6,comm60
         IF(beg6 .EQ. 'BeginX') THEN
            IF(iTalk .EQ. 1)   WRITE( *,'(a6,a)') beg6,comm60
            GOTO 200
         ENDIF
      ENDDO
 200  CONTINUE
* Read data, 'EndX' terminates data, '*' marks comment
      DO line =1,1000
         READ(ninp,'(a)') mark1
         IF(mark1 .EQ. ' ') THEN
            BACKSPACE(ninp)
            READ(ninp,'(a1,i4,d15.0,a60)') mark1,index,value,comm60
            IF(iTalk .EQ. 1) 
     $           WRITE( *,'(a1,i4,g15.6,a60)') mark1,index,value,comm60
            IF( (index .LE. 0) .OR. (index .GE. imax)) GOTO 990
            xpar(index) = value
         ELSEIF(mark1 .EQ. 'E') THEN
            BACKSPACE(ninp)
            READ(  ninp,'(a4,a)') end4,comm60
            IF(iTalk .EQ. 1)   WRITE( *,'(a4,a)') end4,comm60
            IF(end4 .EQ. 'EndX') GOTO 300
            GOTO 991
         ELSEIF(mark1 .EQ. '*') THEN
            BACKSPACE(ninp)
            READ(  ninp,'(a)') comm80
            IF(iTalk .EQ. 1)    WRITE( *,'(a)') comm80
         ENDIF
      ENDDO
 300  CONTINUE
      IF(iTalk .EQ. 1)  THEN
         WRITE(  *,*) '************************'
         WRITE(  *,*) '*  KW_ReaDataX ends    *'
         WRITE(  *,*) '************************'
      ENDIF
      CLOSE(ninp)
      RETURN
*-----------
 990  WRITE(    *,*) '+++ KW_ReaDataX: wrong index= ',index
      STOP
      RETURN
 991  WRITE(    *,*) '+++ KW_ReaDataX: wrong end of data '
      STOP
      END

      SUBROUTINE KW_VecPrint(nunit,word,pp)
*/////////////////////////////////////////////////////////////////////////
*//                                                                     //
*//   prints single momentum "pp" on unit "nunit" with comment "word"   //
*//                                                                     //
*/////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER      nunit
      CHARACTER*8  word
      REAL*8       pp(4),ams
      INTEGER      i
*----
      ams = pp(4)**2 -pp(3)**2 -pp(2)**2 -pp(1)**2
      IF(ams .GT. 0.0) ams = SQRT(ams)
      WRITE(nunit,'(a8,5(1x,f20.13))') word,(pp(i),i=1,4),ams
      END

      SUBROUTINE KW_GetWtMain(WtMain)
*//////////////////////////////////////////////////////////////////
*//   Main weights  WtMain                                       //
*//////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'KW.inc'
      REAL*8      WtMain
*
      COMMON / wgtall / m_WtCrud,m_WtMod,m_WtSet(100)
      REAL*8            m_WtCrud,m_WtMod,m_WtSet
*--------------------------------------------------------------
      WtMain = m_WtMod   ! the best total weight
*
      END                       !!! KW_GetWt


      SUBROUTINE KW_GetWtAll(WtMain,WtCrud,WtSet)
*///////////////////////////////////////////////////////////////////
*//   Weights ALL                                                 //
*///////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'KW.inc'
*
      COMMON / wgtall / m_WtCrud,m_WtMod,m_WtSet(100)
      REAL*8            m_WtCrud,m_WtMod,m_WtSet
*
      INTEGER  j
      REAL*8   WtMain,WtCrud,WtSet(*)
*--------------------------------------------------------------
      WtMain = m_WtMod   ! the best total weight
      WtCrud = m_WtCrud  ! Crude weight (helps to avoid bad events)
      DO j=1,100
         WtSet(j) = m_WtSet(j)
      ENDDO
      END                       !!! KW_GetWtAll

      SUBROUTINE KW_GetMomDec(p1,p2,p3,p4)
*//////////////////////////////////////////////////////////////////////
*//   Final parton momenta                                           //
*//////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'KW.inc'
      REAL*8          p1(4), p2(4), p3(4), p4(4)   
*
      COMMON /momdec/ m_q1(4),m_q2(4),m_p1(4),m_p2(4),m_p3(4),m_p4(4)
      REAL*8          m_q1   ,m_q2   ,m_p1   ,m_p2   ,m_p3   ,m_p4   
      INTEGER  j
*--------------------------------------------------------------
      DO j=1,4
         p1(j) = m_p1(j)
         p2(j) = m_p2(j)
         p3(j) = m_p3(j)
         p4(j) = m_p4(j)
      ENDDO
      END                       !!! KW_GetMomDec

      SUBROUTINE KW_GetBeams(q1,q2)
*///////////////////////////////////////////////////////////////////////
*//   Four-momenta of beams                                           //
*///////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'KW.inc'
      REAL*8  q1(4),q2(4)
*
      COMMON /momdec/ m_q1(4),m_q2(4),m_p1(4),m_p2(4),m_p3(4),m_p4(4)
      REAL*8          m_q1   ,m_q2   ,m_p1   ,m_p2   ,m_p3   ,m_p4   
      INTEGER k
*--------------------------------------------------------------
      DO k=1,4
         q1(k) = m_q1(k)
         q2(k) = m_q2(k)
      ENDDO
      END                       !!! KW_GetBeams

      SUBROUTINE KW_GetPhotAll(NphAll,PhoAll)
*///////////////////////////////////////////////////////////////////////
*//                                                                   //
*//   Get all photons, note that they are ordered in energy           //
*//                                                                   //
*///////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'KW.inc'
*
      INTEGER  NphAll
      REAL*8   PhoAll(100,4)
*
      COMMON / momset / 
     $     m_qeff1(4),m_qeff2(4),m_sphum(4),m_sphot(100,4),m_nphot
      REAL*8      m_qeff1, m_qeff2, m_sphum, m_sphot
      INTEGER     m_nphot
*
      INTEGER  j,k
*------------------
      NphAll = m_nphot
      DO j=1,100
         DO k=1,4
            PhoAll(j,k) = m_sphot(j,k)
         ENDDO
      ENDDO
      END                       !!! KW_GetPhotAll !!!

      SUBROUTINE KW_GetNevMC(NevMC)
*/////////////////////////////////////////////////////////////////////////
*//   Number of generated events from MC generator                      //
*//   CALL KW_Finalize before !!!                                       //
*/////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'KW.inc'
      INTEGER       NevMC
*--------------------------------------------------------------
      NevMC = M_NevTru
      END                       !!! KW_GetNevTot

      SUBROUTINE KW_GetXSecNR(XSecNR,XErrNR)
*////////////////////////////////////////////////////////////////////////
*//   Normalization   Monte Carlo Cross Section [pb] and its error     //
*//   To be used for normalization of histograms                       //
*//   CALL KW_Finalize before using this !!!!                          //
*//   For KeyWgt = 0    XSecNR =    XSecMC                             //
*//   For KeyWgt = 1    XSecNR =    XCruMC                             //
*////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'KW.inc'
      REAL*8        XSecNR,XErrNR
*--------------------------------------------------------------
      XSecNR = m_XSecNR
      XErrNR = m_XErrNR
      END                       !!! KW_GetXSecNR


      SUBROUTINE KW_GetXSecMC(XSecMC,XErrMC)
*////////////////////////////////////////////////////////////////////////
*//   Final Main Best Monte Carlo Cross Section [pb] and its error     //
*//   CALL KW_Finalize before !!!                                      //
*////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'KW.inc'
      REAL*8        XSecMC,XErrMC
*--------------------------------------------------------------
      XSecMC = m_XSecMC
      XErrMC = m_XErrMC
      END                       !!! KW_GetXSecMC

*////////////////////////////////////////////////////////////////////////
*//                                                                    //
*//   End of Class KW                                                  //
*//                                                                    //
*////////////////////////////////////////////////////////////////////////


