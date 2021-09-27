      SUBROUTINE Filexp_Obsolete(Xpar,Npar)   
*////////////////////////////////////////////////////////////////////////////////
*//                                                                            //
*//  Filexp transfers input params form Xpar to common blocks                  //
*//  Actualy it does part of the job, KoralW(-1) does another part.            //
*//  Some parameters are modified according to values of Key's (filtering)     //
*//  Obsolete Npar is also defined, it should disappear in next version.       //
*//                                                                            //
*//  Xpar should be ONLY and ONLY an input object,                             //
*//  just image of the input data from the disk                                //
*//  it should never transfer any values between program units, because        //
*//  this doubles the role of common blocks, i.e. the same variables are       //
*//  communicated between routines through static commons and (pointer) xpar!! //
*//                                                                            //
*//  In actual version xpar re-defined in filexp is used in only in            //
*//         ampli4f.grc.sgl/grc4f_init/setmas_koralw.f:                        //
*//         ampli4f.grc.all/grc4f_init/setmas_koralw.f:                        //
*//  through  CALL ampinw(xpar,npar) in KoralW                                 //
*//  This cross-talk should be eliminated, for example filtering from xpar to  //
*//  common blocks in setmas_koralw.f should be done independently,            //
*//  or it should be eliminated,ie.  xpar replaced by getters from KW class.   //
*//                                                                            //
*////////////////////////////////////////////////////////////////////////////////
      IMPLICIT DOUBLE PRECISION  (A-H,O-Z)  
      DIMENSION  XPAR( *),NPAR( *)          
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF  
      COMMON / WEKIN2 / AMAW,GAMMW,GMU,ALPHAW
! This common contains parameters of non-established particles such as higgs
! Which need to be used by pre-sampler (to be activated by dipswitch IHIG
      COMMON / WEKIN3 / AMHIG,GAMHIG,IHIG
! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
! TAUOLA, PHOTOS and JETSET overall switches
      COMMON / LIBRA  / JAK1,JAK2,ITDKRC,IFPHOT,IFHADM,IFHADP

      COMMON / VVREC  / VVMIN,VVMAX,VV,BETI                   
      COMMON / WT_MAX / WTMAX,WTMAX_CC03     
      COMMON / DECDAT / AMAFIN(20), BR(20)
      COMMON / INOUT  / NINP,NOUT     
      COMMON / MATPAR / PI,CEULER     
      COMMON / PHYPAR / ALFINV,GPICOB     
!   -- cuts for selecto
      COMMON /articut/ arbitr,arbitr1,themin,arbitr2  
!   --Anomalous WWV Couplings, for WWamgc only
      COMMON / ancoco / g1(2),kap(2),lam(2),g4(2),g5(2),kapt(2),lamt(2)
      DOUBLE COMPLEX g1,kap,lam,g4,g5,kapt,lamt
! user mask on final states
      COMMON /cumask/ user_umask(202)
!   --Formats for anyone
      COMMON / BXFMTS / BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G 
      CHARACTER*80      BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G       
      CHARACTER*80      BXL2C      
      COMMON / RANPAR / KEYRND
!-- The CKM mixing matrix and VV+ which should be =1 (V - unitary) 
      DIMENSION VCKM(3,3),VVH(3,3)
!   -- upspeeding
      SAVE / WEKING /,/ KEYkey /,/ VVREC  /
      SAVE / INOUT  /,/ BXFMTS /,/ RANPAR /,/ WEKIN2 /,/WEKIN3/
      SAVE  / ancoco /, / decdat /
      SAVE

!-- Data
      CHARACTER*64 Logo(44)
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
     $'*  ###   ###           version 1.42.1             ##   ##     *',
     $'***************************************************************',
     $'********************** December  1998 *************************',
     $'***************************************************************',
     $'               Last modification: 16.10.1998                   ',
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
*//   Math. constants, should go to PARAMETER !!!
      PI=3.1415926535897932D0
      CEULER = 0.57721566D0                  
      ZET2= PI**2/6D0  
      ZET3= 1.2020569031595942854D0
*///////////////////////////////////////////////////////////
*//                                                       //
*//  Npar is now pure internal object (obsolete)          //
*//  below we translate xpar--> npar fot internal use     //
*//                                                       //
*///////////////////////////////////////////////////////////
*     KeyRad = 1000*KeyCul +100*KeyNLL +10*KeyFSR +KeyISR
      npar(1)= 
     $           +NINT(xpar(1011))  ! KeyISR
     $        +10*NINT(xpar(1012))  ! KeyFSR
     $       +100*NINT(xpar(1013))  ! KeyNLL
     $      +1000*NINT(xpar(1014))  ! KeyCul
*
*     KeyPhy = 100000*KeyWu +10000*KeyRed +1000*KeySpn+100*KeyZet+10*KeyMas+KeyBra 
      npar(2)= 
     $           +NINT(xpar(1021))  ! KeyBra 
     $        +10*NINT(xpar(1022))  ! KeyMas
     $       +100*NINT(xpar(1023))  ! KeyZet
     $      +1000*NINT(xpar(1024))  ! KeySpn
     $     +10000*NINT(xpar(1025))  ! KeyRed
     $    +100000*NINT(xpar(1026))  ! KeyWu
*     KeyTek = 100*KeySmp +10*KeyRnd +KeyWgt
      npar(3)= 
     $           +NINT(xpar(1031))  ! KeyWgt
     $        +10*NINT(xpar(1032))  ! KeyRnd
     $       +100*NINT(xpar(1033))  ! KeySmp
*     KeyMis = 10000*KeyWon +1000*KeyZon+100*KeyAcc+10*Key4f +KeyMix
      npar(4)= 
     $           +NINT(xpar(1041))  ! KeyMix
     $        +10*NINT(xpar(1042))  ! Key4f
     $       +100*NINT(xpar(1043))  ! KeyAcc
     $      +1000*NINT(xpar(1044))  ! KeyZon
     $     +10000*NINT(xpar(1045))  ! KeyWon
*
      npar( 5)= NINT(xpar(1055))  ! KeyDWM
      npar( 6)= NINT(xpar(1056))  ! KeyDWP
      npar( 7)= NINT(xpar(1057))  ! Nout
*
      npar(21)= NINT(xpar(1071))  ! JAK1
      npar(22)= NINT(xpar(1072))  ! JAK2
      npar(23)= NINT(xpar(1073))  ! ITDKRC
      npar(24)= NINT(xpar(1074))  ! IFPHOT
      npar(25)= NINT(xpar(1075))  ! IFHADM
      npar(26)= NINT(xpar(1076))  ! IFHADP
* anomalki
      DO i=101,302
         npar(i)=NINT(xpar(i+1000))
      ENDDO
*///////////////////////////////////////////////////////////
*//         end of translation xpar-->npar                //
*///////////////////////////////////////////////////////////
*  Some phys. constants
      amel    =xpar(100)
      AlfInv  =xpar(101)
      gpicob  =xpar(102)
!-----------------------------------------------------------------------
! Physics switches 
! KeyRad =  1000*KeyCul+100*KeyNLL+10*KeyFSR+KeyISR
      KeyRad = NPAR(1)
      KeyISR = MOD(KeyRad,10)
      KeyFSR = MOD(KeyRad,100)/10
      KeyNLL = MOD(KeyRad,1000)/100
      KeyCul = MOD(KeyRad,10000)/1000
!
! KeyPhy = 100000*KeyWu +10000*KeyRed +1000*KeySpn 
!                       +100*KeyZet +10*KeyMas +KeyBra
      KeyPhy = NPAR(2)
      KeyBra = MOD(KeyPhy,10)
      KeyMas = MOD(KeyPhy,100)/10
      KeyZet = MOD(KeyPhy,1000)/100
      KeySpn = MOD(KeyPhy,10000)/1000
      KeyRed = MOD(KeyPhy,100000)/10000
      KeyWu  = MOD(KeyPhy,1000000)/100000
!-----------------------------------------------------------------------
! Technical switches
! KeyTek = 100*KeySmp +10*KeyRnd +KeyWgt
      KeyTek = NPAR(3)
      KeyWgt = MOD(KeyTek,10)
      KeyRnd = MOD(KeyTek,100)/10
      KeySmp = MOD(KeyTek,1000)/100 
!-----------------------------------------------------------------------
! Miscelaneous, for future develoment
! KeyMis = 10000*KeyWon +1000*KeyZon+100*KeyAcc +10*Key4f +KeyMix
      KeyMis = NPAR(4)
      KeyMix = MOD(KeyMis,10)
      Key4f  = MOD(KeyMis,100)/10
      KeyAcc = MOD(KeyMis,1000)/100
      KeyZon = MOD(KeyMis,10000)/1000
      KeyWon = MOD(KeyMis,100000)/10000
! Higgs pre-sampler dipswitch
      IHIG=0
!-----------------------------------------------------------------------
      KEYDWM = NPAR(5)     
      KEYDWP = NPAR(6)     
      IF((KeyWon*KeyZon*(KEYDWP+KEYDWM) .NE. 0) .OR.
     $   (KeyWon .EQ. 0 .AND. KeyZon.eq.0       )     ) THEN
          WRITE(6,*) 'FILEXP==> inconsistent input: '
          WRITE(6,*) 'KeyWon=',KeyWon,'KeyZon=',KeyZon
          WRITE(6,*) 'KeyDWP=',KeyDWP,'KeyDWM=',KeyDWM
          STOP
      ENDIF
      nout = npar(7)
      IF(nout. LE. 0) nout=16     
      jak1 = npar(21)
      jak2 = npar(22)
      itdkrc = npar(23)
      ifphot = npar(24)
      ifhadm = npar(25)
      ifhadp = npar(26)
      cmsene = xpar(1)      
      gmu    = xpar(2)   
      alfwin = xpar(3)
      amaz   = xpar(4)
      gammz  = xpar(5)
      amaw   = xpar(6)
      gammw  = xpar(7)
      vvmin  = xpar(8)
      vvmax  = xpar(9)
      wtmax  = xpar(10)
      amhig  = xpar(11)
      gamhig = xpar(12)
      alpha_s= xpar(13)
      arbitr = xpar(14)
      arbitr1= xpar(15)
      themin = xpar(16)
      arbitr2= xpar(17)
      wtmax_cc03= xpar(18)
      PReco  = xpar(19)
      ene    = CMSene/2d0      
* ?????????????  too small for yfs3 !!!!!!!!!!!!!!!!!!!!!
      vvmax  = MIN( vvmax, 1d0-(amel/ene)**2 )                   
c?????????????????????????????????????????????
c re-used in KoralW
      XPAR(9) =VVMAX ! send it back !!!
c?????????????????????????????????????????????

*/////////////////////////////////////////////////////////////////////////
*//               If arbitr2=<0 reset it to the maximum value           //
      IF (arbitr2 .LE. 0d0) THEN
        arbitr2 = cmsene**2
c?????????????????????????????????????????????
c seems to be unused
        xpar(17) = arbitr2
c?????????????????????????????????????????????
      ENDIF
      IDE=2               
      IDF=2               
      XK0=3.D-3         
*/////////////////////////////////////////////////////////////////////////
*//                       users mask                                    //
      DO i=1,202
         user_umask(i)=npar(100+i)
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
         xpar(3) = alfwin
c??????????????????????????????????????
         ALPHAW = 1D0/ ALFWIN
      ELSEIF(KeyMix .EQ. 1) THEN
!.. this option is turned into G_mu scheme, 
!.. so, we recalculate ALFWIN
         SINW2 = 1D0 -AMAW**2/AMAZ**2
         ALFWIN = pi/( sqrt(2d0)*gmu*amaw**2*sinw2 )
c??????????????????????????????????????
c re-used in setmas_koralw.f
         xpar(3) = alfwin
c??????????????????????????????????????
         ALPHAW = 1D0/ ALFWIN
      ELSE 
* LEP2 workshop definition
         sinw2 = pi * alphaw /( sqrt(2d0) * amaw**2 * gmu )
      ENDIF
*/////////////////////////////////////////////////////////////////////////
*//            cuts for selecto removed for CC03                        //
*/////////////////////////////////////////////////////////////////////////
      IF (Key4f .EQ. 0) THEN
* no cuts for CC03 presampler
        arbitr = 0d0  !  min. vis p_t**2 
        arbitr1= 0d0  !  add. cut for e+e-ch+ 
        themin = 0d0  !  min. theta [rad] with beam   
        arbitr2= cmsene**2  !  max p_t**2 of photons for e+e-ch+ 
c????????????????????????????????????????
c seems to be unused
        xpar(14)=arbitr    
        xpar(15)=arbitr1   
        xpar(16)=themin    
        xpar(17)=arbitr2  
c????????????????????????????????????????
      ENDIF
*/////////////////////////////////////////////////////////////////////////
*//             alpha_s/pi for naive QCD corrections                    //
*/////////////////////////////////////////////////////////////////////////
      aspi = alpha_s/pi
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
         ALPHA_S = 0.12d0  ! make sure alpha_s is consistent
c??????????????????????????????????????????????
c re-used in setmas_koralw.f and KoralW
         xpar(13)=alpha_s  ! <== send it back
c??????????????????????????????????????????????
         aspi = alpha_s/pi
         gammw=-1d0        ! make sure W width will be recalculated
         DO i=1,9
            BR(i) = xpar(130 +i)
         ENDDO
      ELSEIF(  KeyBra .EQ. 2 )THEN
*/////////////////////////////////////////////////////////////////////////
*//              Default values of CKM and BRanchings                   //
*// Recalculate br. ratios from the CKM matrix and alpha_s according to //
*// theoretical formula of A. Denner, Fortschr. Phys. 41 (1993) 307.    //
*// Values of the CKM matrix elements from 1996 PDG Review:             //
*//  http://www-pdg.lbl.gov/pdg.html (mean values of given ranges)      //
*/////////////////////////////////////////////////////////////////////////
         VCKM(1,1) =xpar(111)   ! V_ud  real part
         VCKM(1,2) =xpar(112)   ! V_us  real part
         VCKM(1,3) =xpar(113)   ! V_ub  real part
         VCKM(2,1) =xpar(114)   ! V_cd  real part
         VCKM(2,2) =xpar(115)   ! V_cs  real part
         VCKM(2,3) =xpar(116)   ! V_cb  real part
         VCKM(3,1) =xpar(117)   ! V_td  real part
         VCKM(3,2) =xpar(118)   ! V_ts  real part
         VCKM(3,3) =xpar(119)   ! V_tb  real part
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
        WRITE(6,*)'filexp=> wrong KeyBra: ',keybra
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
         XPAR(7) = GAMMW  ! send it back !!!
c????????????????????????????????????????????
      ENDIF
*///////////////////////////////////////////////////////////////////
*//               final fermions masses                           //
*///////////////////////////////////////////////////////////////////
      DO i = 1,6
         amafin(   i) = xpar(500+i*10 +6) ! leptons
         amafin(10+i) = xpar(600+i*10 +6) ! quarks
      ENDDO
      amel   = amafin(11)       ! <---now probably not necessary ?????
      IF(  KeyMas .EQ. 0 ) THEN
         DO i = 1,6
            amafin(   i) = 0d0
            amafin(10+i) = 0d0
         ENDDO
      ENDIF
cccc*///////////////////////////////////////////////////////////////////
cccc*// Not necessary because default wtmax is in data_DEFAUTS anyway //
cccc*///////////////////////////////////////////////////////////////////
!!!!!!   NECESSARY this is a part of algorithm, not data   !!!!!!!  m.s.
      IF(wtmax.LE.0d0) THEN
        wtmax=2d0
      ENDIF
ccccc??????????????????????????????????????????????
ccccc seems to be unused
ccccc        xpar(10) = wtmax   ! send it back !!!
ccccc???????????????????????????????????????????????
      IF(wtmax_cc03 .LE. 0d0) THEN
        wtmax_cc03 = xpar(151)
        IF(cmsene.GT.162) wtmax_cc03 = xpar(152)
        IF(cmsene.GT.175) wtmax_cc03 = xpar(153)
        IF(cmsene.GT.200) wtmax_cc03 = xpar(154)
        IF(cmsene.GT.250) wtmax_cc03 = xpar(155)
        IF(cmsene.GT.350) wtmax_cc03 = xpar(156)
        IF(cmsene.GT.700) wtmax_cc03 = xpar(157)
c?????????????????????????????????????????????
c seems to be unused
        xpar(18) = wtmax_cc03   ! send it back !!!
c?????????????????????????????????????????????
      ENDIF
!-- if WW-CC03 matrix el. is requested, use wtmax_cc03 instead of wtmax
      IF(key4f .EQ. 0) THEN
        wtmax=wtmax_cc03
c?????????????????????????????????????????????
c seems to be unused
        xpar(10) = wtmax   ! send it back !!!
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
      WRITE(NOUT,BXL1I) KeyISR,     'Init. state Rad.   ','KeyISR','IQ2'
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
      WRITE(NOUT,BXL1I) KeySmp,     'presampler type    ','KeySmp','IT2'
      WRITE(NOUT,BXL1I) KeyRnd,     'rand Numb type     ','KeyRnd','IT3'
      WRITE(NOUT,BXL1I) KeyWgt,     'weighting  switch  ','KeyWgt','IT4'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1I) KeyMis,     'Miscelaneous       ','KeyMis','IM1'
      WRITE(NOUT,BXL1I) KeyMix,     'sinW2 input type   ','KeyMix','IM2'
      WRITE(NOUT,BXL1I) Key4f,      '4 fermion matr el  ','Key4f ','IM3'
      WRITE(NOUT,BXL1I) KeyAcc,     'Anomalous couplings','KeyAcc','IM4'
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
      WRITE(NOUT,BXL1F) alpha_s,    'alpha_s: QCD coupl.','ALPHAS','I11'
      WRITE(NOUT,BXL1F) PReco  ,    'Color Re-Con. Prob.','PReco ','I12'
      WRITE(NOUT,BXTXT)'***********************************************'
      WRITE(NOUT,BXL1F) SINW2,      'sin(theta_W)**2    ','SINW2 ','I13'
      WRITE(NOUT,BXTXT)'***********************************************'
*!-----------------------------------------------------------------------
*! Feynman rules and their printout, LEP200 style
*!-----------------------------------------------------------------------
*c      QE =-1
*c      VEI= 1-4*SINW2
*c      AEI= 1
*c      EEW = SQRT(4D0*PI*ALPHAW)
*c      GAE =-EEW/sqrt(16D0*SinW2*(1d0-SinW2))
*c      GVE = GAE*VEI
*c      GWF = EEW/(2D0*sqrt(2d0)*sqrt(SinW2))
*c      GWWG= EEW
*c      GWWZ= EEW * sqrt(1d0-SinW2) /sqrt(SinW2)
*c      WRITE(NOUT,BXL1F) GVE,        'LEP200 workshop      ','GVE ','***'
*c      WRITE(NOUT,BXL1F) GAE,        'LEP200 workshop      ','GAE ','***'
*c      WRITE(NOUT,BXL1F) GWF,        'LEP200 workshop      ','GWF ','***'
*c      WRITE(NOUT,BXL1F) GWWG,       'LEP200 workshop      ','GWWG','***'
*c      WRITE(NOUT,BXL1F) GWWZ,       'LEP200 workshop      ','GWWZ','***'
*!-----------------------------------------------------------------------
      WRITE(NOUT,BXTXT)'***********************************************'
c>    WRITE(NOUT,BXTXT) '       sin(th_W) from G_mu, alpha_w and M_Z: '
c>    WRITE(NOUT,BXTXT) '        A2 = PI / ( ALFWIN*SQRT(2D0)*GMU )   '
c>    WRITE(NOUT,BXTXT) '     SINW2 = ( 1-SQRT( 1-(4*A2/AMAZ**2) ) )/2'
c>    WRITE(NOUT,BXL1F) SINW2,      'sin(theta_W)**2    ','SINW2 ','A6'
c>    WRITE(NOUT,BXTXT)'***********************************************'
      IF(keyzet.eq.0) THEN 
        WRITE(NOUT,BXTXT) '  Z width in Z propagator: s/M_Z *GAMM_Z '
      ELSEIF(keyzet.eq.1) THEN 
        WRITE(NOUT,BXTXT) '  Z width in Z propagator:   M_Z *GAMM_Z '
      ELSEIF(keyzet.eq.2) THEN 
        WRITE(NOUT,BXTXT) '  Z width in Z propagator:   0           '
      ELSE
        WRITE(NOUT,BXTXT) '  FILEXP ==> wrong KEYZET =',keyzet
        STOP
      ENDIF
      WRITE(NOUT,BXTXT)'***********************************************'
      IF(keyspn.ne.1) THEN 
        WRITE(NOUT,BXTXT) '         WARNING!  spin in decays is OFF: '
        WRITE(NOUT,BXL1I) KEYSPN, 'spin in decays switch','KEYSPN','A13'
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
      IF (KeySmp.NE.0) THEN
        WRITE(NOUT,BXTXT) ' Predefined cuts on final state fermions'
        WRITE(NOUT,BXL1F)arbitr, 'min. vis p_t**2    ','GeV^2','X2'
        WRITE(NOUT,BXL1F)arbitr1,'add. cut for e+e-ch+ch-','GeV^2','X3'
        WRITE(NOUT,BXL1G)themin, 'min. theta with beam','rad ','X6'
        WRITE(NOUT,BXL1F)arbitr2,'max. p_t**2 phot eexx','GeV^2','X3'
      ENDIF

      IF( KeyAcc .NE.0 ) THEN 
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
         IF( KeyAcc .EQ. 1) THEN 
*!-- Set 1:
*!       --Set up constants OTHER than SM:
*!       --for WWgamma vertex
          g1(1)   = DCMPLX(xpar(21),xpar(31))
          kap(1)  = DCMPLX(xpar(22),xpar(32))
          lam(1)  = DCMPLX(xpar(23),xpar(33))
          g4(1)   = DCMPLX(xpar(24),xpar(34))
          g5(1)   = DCMPLX(xpar(25),xpar(35))
          kapt(1) = DCMPLX(xpar(26),xpar(36))
          lamt(1) = DCMPLX(xpar(27),xpar(37))
*!       --WWZ vertex
          g1(2)   = DCMPLX(xpar(41),xpar(51))
          kap(2)  = DCMPLX(xpar(42),xpar(52))
          lam(2)  = DCMPLX(xpar(43),xpar(53))
          g4(2)   = DCMPLX(xpar(44),xpar(54))
          g5(2)   = DCMPLX(xpar(45),xpar(55))
          kapt(2) = DCMPLX(xpar(46),xpar(56))
          lamt(2) = DCMPLX(xpar(47),xpar(57))
*!======================================================
*!====== Other TGC parametrizations disussed in: ======= 
*!== YR CERN-96-01, "Physics at LEP2" Vol. 1, p. 525. ==
*!======================================================
        ELSEIF (KeyAcc.EQ.2) THEN
*!-- Set 2:  
	  delta_Z = xpar(61)
	  x_gamma = xpar(62)
	  x_Z     = xpar(63)
	  y_gamma = xpar(64)
	  y_Z     = xpar(65)
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
        ELSEIF (KeyAcc.EQ.3) THEN
*!-- Set 3:  
	  alpha_Wphi = xpar(71)
	  alpha_Bphi = xpar(72)
	  alpha_W    = xpar(73)
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
          write(6,*)'FILEXP==> Wrong KeyAcc: ',keyacc
          STOP
        ENDIF
*!
        WRITE(NOUT,BXTXT)' '
	IF (KeyAcc.EQ.2) THEN
          WRITE(NOUT,BXTXT)'Anomalous Couplings - set 2; YR CERN 96-01'
          WRITE(NOUT,BXTXT)'******************************************'
          WRITE(NOUT,BXL1F) delta_Z,'delta_Z','delta_Z','IA21'
          WRITE(NOUT,BXL1F) x_gamma,'x_gamma','x_gamma','IA22'
          WRITE(NOUT,BXL1F) x_Z    ,'x_Z    ','x_Z    ','IA23'
          WRITE(NOUT,BXL1F) y_gamma,'y_gamma','y_gamma','IA24'
          WRITE(NOUT,BXL1F) y_Z    ,'y_Z    ','y_Z    ','IA25'
	ELSEIF (KeyAcc.EQ.3) THEN
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

      END       
