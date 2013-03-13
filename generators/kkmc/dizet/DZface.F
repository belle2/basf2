*/////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                     //
*//                     Pseudo-CLASS  DZface                                            //
*//   BornV.h  header is included in all routines                                       //
*//   (Some kind of 'inheritance' from class BornV class)                               //
*//   Initialization of BornV class is not necessary, however.                          //
*//                                                                                     //
*//   Purpose: Calculates Electroweak formfactor from Dizet                             //
*//            Writes the on the disk                                                   //
*//                                                                                     //
*//  General philosophy is that DZface is not linked with the MC like other classes     //
*//  but produces look-up tables on the disk which are read by BornV class.             //
*//  The tables can be produced using TabMain program which is run separately for       //
*//  each flavor, producing one table per flavour.                                      //
*//  In such a case there is no worry whether EW  library can be reinitialized          //
*//  properly (although Dizet apparently can reinitialize propoerly).                   //
*//  There is special testing program TabCheck in BornV/tabtest/ which tests            //
*//  whether interpolation is done correctly.                                           //
*//  Present version is for sub-permille precision.                                     //
*//  there is only one misteriuos discontinutity at W=40GeV of order 1%, to be checked. //
*//                                                                                     //
*// Notes:                                                                              //
*//  cyy(i+1,5 )=123.456 not used                                                       //
*//  ibox = 1 for all energies??? requires costheta in tables!                          //
*//  Is QCDcor smaller for b-quark then for other quarks??                              //
*//  remember to check signs of s,t,u of rokanc!!!                                      //
*//                                                                                     //
*/////////////////////////////////////////////////////////////////////////////////////////
      SUBROUTINE DZface_Initialize( KFfin, xpar)
*/////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                     //
*//     Class initializator                                                             //
*//  Interface to DIZET of the Dubna-Zeuthen EWRC group                                 //
*//  Based on that in KORALZ 4.x                                                        //
*//  Notes:                                                                             //
*//    QED alfinv is separate from alfinv used bremsstrahlung part of KK2f.             //
*//    Note that fermion masses in Dizet are isolated from these in KK2f.               //
*//                                                                                     //
*/////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE '../bornv/BornV.fi'
      INCLUDE 'DZface.fi'
      INCLUDE 'BXformat.fi'
      DOUBLE PRECISION  xpar(*)
      INTEGER           KFfin
*
      DOUBLE PRECISION  partz(0:11),partw(3)
      DOUBLE PRECISION  zpard(30)
      INTEGER           Npar(30)
      INTEGER           ihvp,iamt4,Iqcd,imoms,iscre,ialem,imask
      INTEGER           iscal,ibarb,iftjr,ifacr,ifact,ihigs,iafmt
      INTEGER           imass,ii,i,kdumm,kolor
      INTEGER           iewlc,iczak,ihig2,iale2,igfer,iddzz
      DOUBLE PRECISION  amfin,wmass_start,qe,aizor,xolor,qf
      DOUBLE PRECISION  AlStrZ,AlQedZ,DAL5H,AlStrT
*/////////////////////////////////////////////////////////////////////////////
* Translation table KF-->IV
      INTEGER IV(-16:16)
      DATA IV / -1, -2, -1, -2, -1, -2, 4*0, -3, -4, -3, -4, -3, -4,  0,  
     $           4,  3,  4,  3,  4,  3, 4*0,  2,  1,  2,  1,  2,  1    /
*/////////////////////////////////////////////////////////////////////////////
      m_out   = 16
      m_KFfin = KFfin
      m_KFini = xpar(400) !!!
      m_IVini = IV(m_KFini)
      m_IVfin = IV(m_KFfin)
*
      m_MZ      = xpar(502) !!!
*
      m_ibox    = xpar(801) !!!
      m_amh     = xpar(805) !!!
      m_amtop   = xpar(806) !!!
*
      m_alfinvMZ  = xpar(808) !!! (128.86674175d0)
      m_alfQCDMZ  = xpar(809) !!! (0.125d0)

      m_MW   = 80d0      ! input, rededined by Dizet

      WRITE(m_out,bxope)
      WRITE(m_out,bxtxt) 'DZface_Initialize, Interface to Dizet 6.xx   '
      WRITE(m_out,bxl1f) m_MZ    ,   'Z mass             ','amz   ','a1'
      WRITE(m_out,bxl1f) m_amh   ,   'Higgs mass         ','amh   ','a2'
      WRITE(m_out,bxl1f) m_amtop ,   'Top mass           ','amtop ','a3'
      WRITE(m_out,bxl1i) m_KFini ,   'KF code of beam    ','KFini ','a5'
      WRITE(m_out,bxl1i) m_KFfin ,   'KF of final fermion','KFfin ','a6'
      WRITE(m_out,bxl1i) m_IVini ,   'IV code of beam    ','IVini ','a7'
      WRITE(m_out,bxl1i) m_IVfin ,   'IV of final fermion','IVfin ','a8'
      WRITE(m_out,bxl1i) m_ibox  ,   'EW box switch      ','ibox  ','a9'
      WRITE(m_out,bxl1f) m_alfinvMZ, 'QED alfa inv. at Z ','alfinv','a1'
      WRITE(m_out,bxl1f) m_alfQCDMZ, 'QCD alfa at Z mass ','alfQCD','a2'
      WRITE(m_out,bxclo)

      CALL DZface_GivIzo( m_IVini, 1,aizor,qe,kdumm)
      CALL DZface_GivIzo( m_IVfin, 1,aizor,qf,kolor)

*  Default values
*      Ihvp  =  1  ! =1,2,3  (Jegerlehner/Eidelman, Jegerlehner(1988), Burkhardt etal.)
*      Iamt4 =  4  ! =0,1,2,3,4 (=4 the best, Degrassi/Gambino)
*      Iqcd  =  3  ! =1,2,3  (approx/fast/lep1, exact/Slow!/Bardin/, exact/fast/Kniehl)
*      Imoms =  1  ! =0,1    (=1 W mass recalculated)
*      Imass =  0  ! =0,1    (=1 test only, effective quark masses)
*      Iscre =  0  ! =0,1,2  ( Remainder terms, 
*      Ialem =  3  ! =1,3 or 0,2, (for 1,3 DALH5 not input)
*      Imask =  0  ! =0,1 (=0: Quark masses everywhere; =1 Phys. threshold in the ph.sp.)
*      Iscal =  0  ! =0,1,2,3  ( Kniehl=1,2,3, Sirlin=4)
*      Ibarb =  2  ! =-1,0,1,2 ( Barbieri???)
*      Iftjr =  1  ! =0,1      ( FTJR corrections)
*      Ifacr =  0  ! =0,1,2,3  ( Expansion of delta_r; =0 none; =3 fully, unrecommed.)
*      Ifact =  0  ! =0,1,2,3,4,5 (Expansion of kappa; =0 none )
*      Ihigs =  0  ! =0,1      ( Leading Higgs contribution resummation)
*      Iafmt =  1  ! =0,1      (=0 for old ZF)
* new parameters of 6.x version
*      Iewlc =  1  ! =0,1   (???)
*      Iczak =  1  ! =0,1   (Czarnecki/Kuehn corrections)
*      Ihig2 =  1  ! =0,1   (Two-loop higgs  corrections off,on) 
*      Iale2 =  3  ! =1,2,3 (Two-loop constant corrections in delta_alpha)
*      Igfer =  2  ! =0,1,2 (QED corrections for fermi constant)
*      Iddzz =  1  ! =0,1   (??? DD-ZZ game, internal flag)
*
* Input flags in NPAR
      DO i=1,21
         Npar(i) = xpar(900+i)
      ENDDO
      WRITE(m_out,'(a/(a8,i2,a8,i2))')
     $ ' DIZET flags, see routine Dizet for explanation:',
     $ '  Ihvp =',Npar( 1),  ' Iamt4 =',Npar( 2),
     $ '  Iqcd =',Npar( 3),  ' Imoms =',Npar( 4),
     $ ' Imass =',Npar( 5),  ' Iscre =',Npar( 6),
     $ ' Ialem =',Npar( 7),  ' Imask =',Npar( 8),
     $ ' Iscal =',Npar( 9),  ' Ibarb =',Npar(10),
     $ ' IFtjr =',Npar(11),  ' Ifacr =',Npar(12),
     $ ' IFact =',Npar(13),  ' Ihigs =',Npar(14),
     $ ' Iafmt =',Npar(15),  ' Iewlc =',Npar(16),
     $ ' Iczak =',Npar(17),  ' Ihig2 =',Npar(18),
     $ ' Iale2 =',Npar(19),  ' Igfer =',Npar(20),
     $ ' Iddzz =',Npar(21)
*     =================================================================
* Input which is not in Npar
      AlStrZ      =  m_alfQCDMZ          ! input at MZ
      AlQedZ      =  1d0/m_alfinvMZ      ! will be redefibed by dizet6
*     =================================================================
******      !!!!! Dizet 5.x, obsolete !!!!!
******      CALL dizet( Npar, m_MW, m_MZ, m_amtop, m_amh,
******     $            AlQedZ, AlStrZ, 
******     $            zpard, partz, partw)
*     =================================================================
*  Dizet 6.x
      CALL DIZET( 
     $   Npar,   ! Inp. Integer switches
     $  m_MW,   ! I/O. W mass (Out. for Imoms=Npar(4)=1,3)
     $  m_MZ,    ! I/O. Z mass (Out. for Imoms=Npar(4)=2,4)
     $  m_amtop, ! Inp. t-quark mass 
     $  m_amh,   ! Inp. Higgs boson mass
     $  DAL5H,   ! I/O. \Delta_Alpha^5_{had}(MZ), (Inp. Ialem=0,2)(Out. Ialem=1,3)
     $  AlQedZ,  ! Out. Alpha_QED
     $  AlStrZ,  ! Inp. Alpha_strong(MZ)
     $  AlStrT,  ! Out. Alpha_strong(MT)
     $  zpard,   ! Out. zpar(1) = del_r, zpar(2) = del_r_rem, zpar(3) = sw2, ... etc
     $  partz,   ! Out. Z partial decay widths
     $  partw)   ! Out. W partial decay widths
*     =================================================================
*(((((((((( m_QCDcor is for debug only
      IF (    ABS(m_KFfin) .GT. 10)  THEN
         m_QCDcor = 0d0             ! leptons
      ELSEIF (ABS(m_KFfin) .EQ. 5)  THEN
         m_QCDcor = zpard(20)-1.0   ! b-quark
      ELSE
         m_QCDcor = zpard(25)-1.0   ! light-quarks
      ENDIF
c))))))))))))))))))))))))))))))))))))))
      WRITE(m_out,*) '   '
      WRITE(m_out,'(a,  f8.6)') ' Alpha-QED   (MZ)  =',AlQedZ
      WRITE(m_out,'(a,  f8.4)') ' Alfa strong (MZ)  =',AlStrZ
      WRITE(m_out,'(a,  f8.4)') ' Alfa strong (Mt)  =',AlStrT
      WRITE(m_out,'(a,f15.10)') 'zpard(20): QCD corr.fact. to Z-width (no b)  =',zpard(20)
      WRITE(m_out,'(a,f15.10)') 'zpard(25): QCD corr.fact. to Z-width (into b)=',zpard(25)
      WRITE(m_out,*) '   '
      WRITE(m_out,*) 'zpar-matrix: standard output of dizet:'
      WRITE(m_out,'(a,i2,a,f12.8)') ('    zpar(',ii,')=',zpard(ii),ii=1,30)

      WRITE(m_out,bxope)
      WRITE(m_out,bxtxt) 'DZface_Initializion ended  '
      WRITE(m_out,bxclo)
      END


      SUBROUTINE DZface_ReaDataX(DiskFile,iReset,imax,xpar)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Clone of KK2f_ReaDataX                                                        //
*//                                                                                 //
*//   DiskFile  = input file to read                                                //
*//   imax   = maximum index in xpar                                                //
*//   iReset = 1, resets xpar to 0d0                                                //
*//   iTalk=1,     prints echo into standard input                                  //
*//                                                                                 //
*//   Single data card is:    (a1,i4,d15.0,a60)                                     //
*//   First data card: BeginX                                                       //
*//   Last  data card: EndX                                                         //
*//   First character * defines comment card!                                       //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      CHARACTER*(*)     DiskFile
      DOUBLE PRECISION  xpar(*)
      CHARACTER*6       beg6
      CHARACTER*4       end4
      CHARACTER*1       mark1
      CHARACTER*60      comm60
      CHARACTER*80      comm80
      INTEGER           imax,iReset,iTalk
      INTEGER           ninp,i,line,index
      DOUBLE PRECISION  value
*////////////////////////////////////////
*//  Clear xpar and read default Umask //
*////////////////////////////////////////
      iTalk = 1
      IF(iReset .EQ. 1 ) THEN
         iTalk = 0
         DO i=1,imax
            xpar(i)=0d0
         ENDDO
      ENDIF
      ninp = 13
      OPEN(ninp,file=DiskFile)
      IF(iTalk .EQ. 1) THEN
         WRITE(  *,*) '****************************'
         WRITE(  *,*) '*  DZface_ReaDataX Starts  *'
         WRITE(  *,*) '****************************'
      ENDIF
* Search for 'BeginX'
      DO line =1,100000
         READ(ninp,'(a6,a)') beg6,comm60
         IF(beg6 .EQ. 'BeginX') THEN
            IF(iTalk .EQ. 1)   WRITE( *,'(a6,a)') beg6,comm60
            GOTO 200
         ENDIF
      ENDDO
 200  CONTINUE
* Read data, 'EndX' terminates data, '*' marks comment
      DO line =1,100000
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
         WRITE(  *,*) '**************************'
         WRITE(  *,*) '*   DZface_ReaDataX Ends   *'
         WRITE(  *,*) '**************************'
      ENDIF
      CLOSE(ninp)
      RETURN
*-----------
 990  WRITE(    *,*) '+++ DZface_ReaDataX: wrong index= ',index
      STOP
      RETURN
 991  WRITE(    *,*) '+++ DZface_ReaDataX: wrong end of data '
      STOP
      END


      SUBROUTINE DZface_Tabluj
*/////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                     //
*//  Routine for evaluation and pretabulation of electroweak formfactors                //
*//                                                                                     //
*/////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE '../bornv/BornV.fi'
      INCLUDE 'DZface.fi'
      DOUBLE PRECISION  ww,x,cosi
      DOUBLE PRECISION  QCDcorR(20)
      DOUBLE PRECISION  amzD,gammz1,gammw1
      DOUBLE COMPLEX    GSW(100)
      INTEGER           i,j,k,kk,ndisk
*----------------------------------------------------------------------
*/////////////////////////////////////
*//    some initialisations         //
*/////////////////////////////////////
* Note that m_MZ m_amh, m_amtop are already defined.
* We get other params out of dizet using DZface_GetPrm.
      CALL DZface_GetPrm( amzD,m_gammz,gammz1,m_MW,m_GammW,gammw1,m_swsq) !
*/////////////////////////////////////////////////////////////////
*//   basic s range LEP1 and below        m_cyy(m_poin1+1, 7)   //
*/////////////////////////////////////////////////////////////////
      WRITE(m_out,*) 'DZface_Tabluj: pretabulation, basic LEP1 range'
      WRITE(6    ,*) 'DZface_Tabluj: pretabulation, basic LEP1 range'
      CALL DZface_MakeGSW(-1, ww, 0d0, GSW, QCDcorR) !initialize QCDcor calculation.
      DO i=0,m_poin1
         ww = m_WminLEP1 *(m_WmaxLEP1/m_WminLEP1)**(DFLOAT(i)/DFLOAT(m_poin1)) !
         IF(MOD(i,10)  .EQ.  0) WRITE(    6,*) 'a: i,ww= ',i,ww
         IF(MOD(i,10)  .EQ.  0) WRITE(m_out,*) 'a: i,ww= ',i,ww
         CALL DZface_MakeGSW( 0, ww, 0d0, GSW, QCDcorR) ! at theta=pi, ibox=0
*--------------------
         DO kk=1,m_poinG
            m_cyy(i+1,kk,m_KFfin) = GSW(kk)
         ENDDO
         DO kk=1,m_poinQ
            m_syy(i+1,kk,m_KFfin) = QCDcorR(kk)
         ENDDO
***      WRITE(6,*) 'KFf=',m_KFfin,'sqrt(s)=',ww,
***  $              'old=',m_QCDcor,'new=',QCDcorR
      ENDDO
*/////////////////////////////////////////////////////////////////
*/              near Z0 resonance    m_czz(m_poin2+1, 7)        //
*/////////////////////////////////////////////////////////////////
      WRITE(m_out,*) 'DZface_Tabluj: pretabulation, near Z0:'
      WRITE(    6,*) 'DZface_Tabluj: pretabulation, near Z0:'
      CALL DZface_MakeGSW( -1, ww, cosi, GSW, QCDcorR) !initialize QCDcor calculation.
      m_WminZ =m_MZ-m_WdelZ
      m_WmaxZ =m_MZ+m_WdelZ
      DO i=0,m_poin2
         ww = m_WminZ +(m_WmaxZ-m_WminZ)*(DFLOAT(i)/DFLOAT(m_poin2))
         IF(MOD(i,10)  .EQ.  0) WRITE(    6,*) 'b: i,ww= ',i,ww
         IF(MOD(i,10)  .EQ.  0) WRITE(m_out,*) 'b: i,ww= ',i,ww
         IF(m_poTh2.EQ.0) THEN
            cosi=0d0
            CALL DZface_MakeGSW( 0, ww, cosi, GSW, QCDcorR) ! at theta=pi, ibox=0
            DO kk=1,m_poinG
               m_czz(i+1,1,kk,m_KFfin)=GSW(kk)
            ENDDO
         ELSE
            DO  j=0,m_poTh2
               cosi = ( -1d0+2d0*DFLOAT(j)/DFLOAT(m_poTh2) )*(1d0-1d-8)
               IF(j  .EQ.        0) cosi=cosi+.3d0/DFLOAT(m_poTh2)
               IF(j  .EQ.  m_poTh2) cosi=cosi-.3d0/DFLOAT(m_poTh2)
               CALL DZface_MakeGSW( m_ibox, ww, cosi, GSW, QCDcorR) ! ibox from input
               DO kk=1,m_poinG
                  m_czz(i+1,j+1,kk,m_KFfin)=GSW(kk)
               ENDDO
            ENDDO
         ENDIF
         DO kk=1,m_poinQ
            m_szz(i+1,kk,m_KFfin) = QCDcorR(kk)
         ENDDO
      ENDDO
*/////////////////////////////////////////////////////////////////////
*//   the region of boxes, LEP2,   m_ctt(m_poin3+1, m_poTh3+1, 7)   //
*/////////////////////////////////////////////////////////////////////
      WRITE(m_out,*) 'DZface_Tabluj: LEP2 energy zone: pretabulation starts' !
      WRITE(    *,*) 'DZface_Tabluj: LEP2 energy zone: pretabulation starts: it will take a bit time' !
      write(    *,*) 'DZface_Tabluj: m_poTh3=', m_poTh3
      CALL DZface_MakeGSW( -1, ww, cosi, GSW, QCDcorR)
      DO  i=0,m_poin3
         ww = m_WmaxLEP1 +(m_WmaxLEP2-m_WmaxLEP1)*(DFLOAT(i)/DFLOAT(m_poin3)) !
         IF(MOD(i,10)  .EQ.  0) WRITE(    6,*) 'c: i,ww= ',i,ww
         IF(MOD(i,10)  .EQ.  0) WRITE(m_out,*) 'c: i,ww= ',i,ww
         DO  j=0,m_poTh3
            cosi = ( -1d0+2d0*DFLOAT(j)/DFLOAT(m_poTh3) )*(1d0-1d-8)
            IF(j  .EQ.        0) cosi=cosi+.3d0/DFLOAT(m_poTh3)
            IF(j  .EQ.  m_poTh3) cosi=cosi-.3d0/DFLOAT(m_poTh3)
            CALL DZface_MakeGSW( m_ibox, ww, cosi, GSW, QCDcorR) ! ibox from input
            DO  kk=1,m_poinG
               m_ctt(i+1,j+1,kk,m_KFfin)=GSW(kk)
            ENDDO
         ENDDO
         DO kk=1,m_poinQ
            m_stt(i+1,kk,m_KFfin) = QCDcorR(kk)
         ENDDO
      ENDDO
*/////////////////////////////////////////////////////////////////////
*//   the region of boxes, NLC,    m_clc(m_poin4+1, m_poTh4+1, 7)   //
*/////////////////////////////////////////////////////////////////////
      WRITE(m_out,*) 'DZface_Tabluj: NLC energy range: pretabulation starts' !
      WRITE(    *,*) 'DZface_Tabluj: NLC energy range: pretabulation starts: it will take a bit time' !
      CALL DZface_MakeGSW(-1,ww,cosi,GSW,QCDcorR)
      DO  i=0,m_poin4
         ww  = m_WmaxLEP2+(m_WmaxNLC-m_WmaxLEP2)*(DFLOAT(i)/DFLOAT(m_poin4)) !
         IF(MOD(i,10)  .EQ.  0) WRITE(    6,*) 'd: i,ww= ',i,ww
         IF(MOD(i,10)  .EQ.  0) WRITE(m_out,*) 'd: i,ww= ',i,ww
         DO  j=0,m_poTh4
            cosi = ( -1d0+2d0*DFLOAT(j)/DFLOAT(m_poTh4) )*(1d0-1d-8)
            IF(j  .EQ.        0) cosi=cosi+.3d0/DFLOAT(m_poTh4)
            IF(j  .EQ.  m_poTh4) cosi=cosi-.3d0/DFLOAT(m_poTh4)
            CALL DZface_MakeGSW( m_ibox, ww, cosi, GSW, QCDcorR) ! ibox from input
*---------------
            DO  kk=1,m_poinG
               m_clc(i+1,j+1,kk,m_KFfin) = GSW(kk)
            ENDDO
         ENDDO
         DO kk=1,m_poinQ
            m_slc(i+1,kk,m_KFfin) = QCDcorR(kk)
         ENDDO
      ENDDO
      WRITE(m_out,*) 'DZface_Tabluj: pretabulatin finished  now !'
      WRITE(6    ,*) 'DZface_Tabluj: pretabulatin finished  now !'
      END                       ! DZface_Tabluj


      SUBROUTINE DZface_WriteFile(DiskFile)
*///////////////////////////////////////////////////////////////////
*//       Write tables into DiskFile                              //
*///////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE '../bornv/BornV.fi'
      INCLUDE 'DZface.fi'
      CHARACTER*(*) DiskFile
      INTEGER KFfin
*--------------------------------------------------------------------
      DOUBLE PRECISION  ww,cosi,gammz1,gammw1,amzD
      INTEGER           KFf,i,j,k,ndisk
*--------------------------------------------------------------------
      KFf = ABS(m_KFfin)
      IF( KFf.LT.1 .OR. KFf.GT.16 ) THEN
         WRITE(m_out,*) '+++ DZface_WriteFile: wrong KFf= ', KFf
         WRITE(    6,*) '+++ DZface_WriteFile: wrong KFf= ', KFf
         STOP
      ENDIF
      WRITE(6 ,*) 'DZface_WriteFile: Tables are Written to DiskFile ',DiskFile !
* Note that m_MZ m_amh, m_amtop are already defined.
* We get other params out of dizet using DZface_GetPrm.
      CALL DZface_GetPrm( amzD,m_gammz,gammz1,m_MW,m_GammW,gammw1,m_swsq) !
      ndisk=21
      OPEN(ndisk,FILE=DiskFile)
      WRITE(ndisk,m_fmt0) m_MZ, m_amh, m_amtop, m_swsq, m_gammz, m_MW, m_GammW ! header
*/////////////////////////////////////////////////////////////////
*//   basic s range LEP1 and below        m_cyy(m_poin1+1, 7)   //
*/////////////////////////////////////////////////////////////////
      DO i=0, m_poin1
         ww = m_WminLEP1 *(m_WmaxLEP1/m_WminLEP1)**(DFLOAT(i)/DFLOAT(m_poin1)) !
         WRITE(ndisk,m_fmt1) 'a',i,ww
         WRITE(ndisk,m_fmt2) (m_cyy(i+1,k,KFf),k=1,m_poinG) ! EW
         WRITE(ndisk,m_fmt2) (m_syy(i+1,k,KFf),k=1,m_poinQ) ! QCD
      ENDDO
*/////////////////////////////////////////////////////////////////
*/              near Z0 resonance    m_czz(m_poin2+1, 7)        //
*/////////////////////////////////////////////////////////////////
      m_WminZ =m_MZ-m_WdelZ
      m_WmaxZ =m_MZ+m_WdelZ
      DO i=0,m_poin2
         ww = m_WminZ +(m_WmaxZ-m_WminZ)*(DFLOAT(i)/DFLOAT(m_poin2))
         DO  j=0,m_poTh2
            WRITE(ndisk,m_fmt1) 'b',i,ww,j
            WRITE(ndisk,m_fmt2) (m_czz(i+1,j+1,k,KFf),k=1,m_poinG) ! EW
         ENDDO
         WRITE(ndisk,m_fmt2) (m_szz(i+1,k,KFf),k=1,m_poinQ) ! QCD
      ENDDO
*/////////////////////////////////////////////////////////////////////
*//   the region of boxes, LEP2,   m_ctt(m_poin3+1, m_poTh3+1, 7)   //
*/////////////////////////////////////////////////////////////////////
      write(*,*) 'DZface_WriteFile: m_poTh3=', m_poTh3
      DO  i=0,m_poin3
         ww = m_WmaxLEP1 +(m_WmaxLEP2-m_WmaxLEP1)*(DFLOAT(i)/DFLOAT(m_poin3)) !
         DO  j=0,m_poTh3
            cosi = ( -1d0+2d0*DFLOAT(j)/DFLOAT(m_poTh3) )*(1d0-1d-8)
            IF(j  .EQ.        0) cosi=cosi+.3d0/DFLOAT(m_poTh3)
            IF(j  .EQ.  m_poTh3) cosi=cosi-.3d0/DFLOAT(m_poTh3)
            WRITE(ndisk,m_fmt1) 'c',i,ww,j,cosi
            WRITE(ndisk,m_fmt2) (m_ctt(i+1,j+1,k,KFf),k=1,m_poinG) ! EW
         ENDDO
         WRITE(   ndisk,m_fmt2) (m_stt(i+1,k,KFf),k=1,m_poinQ) ! QCD
      ENDDO
*/////////////////////////////////////////////////////////////////////
*//   the region of boxes, NLC,    m_clc(m_poin4+1, m_poTh4+1, 7)   //
*/////////////////////////////////////////////////////////////////////
      DO  i=0,m_poin4
         ww  = m_WmaxLEP2+(m_WmaxNLC-m_WmaxLEP2)*(DFLOAT(i)/DFLOAT(m_poin4)) !
         DO  j=0,m_poTh4
            cosi = ( -1d0+2d0*DFLOAT(j)/DFLOAT(m_poTh4) )*(1d0-1d-8)
            IF(j  .EQ.        0) cosi=cosi+.3d0/DFLOAT(m_poTh4)
            IF(j  .EQ.  m_poTh4) cosi=cosi-.3d0/DFLOAT(m_poTh4)
            WRITE(ndisk,m_fmt1) 'd',i,ww,j,cosi
            WRITE(ndisk,m_fmt2) (m_clc(i+1,j+1,k,KFf),k=1,m_poinG) ! EW
         ENDDO
         WRITE(   ndisk,m_fmt2) (m_slc(i+1,k,KFf),k=1,m_poinQ) ! QCD
      ENDDO
      CLOSE(ndisk)
      WRITE(m_out,*) 'DZface_WriteFile: finished  now !!!'
      WRITE(6    ,*) 'DZface_WriteFile: finished  now !!!'
      END                       ! DZface_WriteFile

      SUBROUTINE DZface_MakeGSW(iBox,ww,cosi,GSW,QCDcorR)
*/////////////////////////////////////////////////////////////////////////
*//   gets EWK FFactors and QCD corr. out of Dizet, at ww and theta     //
*//   Prior Dizet initialization is necesary!!!                         //
*//   Used in Tabluj and also in program testing tables                 //
*/////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE '../bornv/BornV.fi'
      INCLUDE 'DZface.fi'
      INTEGER           iBox
      DOUBLE COMPLEX    GSW(*)
      DOUBLE PRECISION  ww,cosi,QCDcorR(*),ROW
      DOUBLE PRECISION  QCDcorD(0:14)
      DOUBLE COMPLEX    xff(4),xfem,xfota
      INTEGER           i,j,kk,k,kdumm,kolor,ibfla,KFf
      DOUBLE PRECISION  ss,uu,tt,aizor,qe,qf,dum1,dum2
      SAVE
*     ---------------------------------------------------
      IF(    iBox.EQ.-1) THEN
         IF ( ABS(m_KFfin)  .EQ.  5) THEN
            ibfla=1
         ELSE
            ibfla=0
         ENDIF
         CALL DZface_GivIzo( m_IVini, 1,aizor,qe,kdumm)
         CALL DZface_GivIzo( m_IVfin, 1,aizor,qf,kolor)
         DO k=1,m_poinG
            m_GSW(k) =0d0
         ENDDO
      ELSEIF( iBox.EQ.0 .OR. iBox.EQ.1) THEN
         ss  = ww*ww
         tt  = ss*(1d0-cosi)/2d0
         uu  = ss*(1d0+cosi)/2d0
         CALL rokanc( iBox,ibfla,-uu,-ss,-tt,qe,qf,xff,xfem,xfota) ! check signs of s,u,t!!!
         IF (ABS(m_KFfin).EQ.12) THEN
c[[[[[[[[[[[[
c   CALL RHOCC(-s-t,-t,S,0D0,-1D0,-1D0,0D0,ROW)
c   ROW=ROW+1D0/m_AlfInv/m_pi/2*(-3D0/2*LOG(s/m_MW**2)+1D0/2*(LOG(-t/s))**2-m_pi**2/6+2D0)
c]]]]]]]]]]]]
ccc            CALL rhocc(ss,tt,-ss+tt,-qe,qe,0D0,0D0,ROW) ! this is for nunu electron chanel
            CALL RHOCC(-ss+tt,+tt,ss,   0D0,-1D0,-1D0,0D0,ROW)
         ELSE
            row=123.456         ! UNUSED!!!!
         ENDIF
         CALL DZface_MakeQCDcor(ww,QCDcorD)
         DO  kk=1,4
            GSW(kk)=xff(kk)
         ENDDO
         GSW(5 )=ROW  !!  123.456d0 ! UNUSED!!!!
         GSW(6 )=xfem
         GSW(7 )=xfota
         GSW(8 )=QCDcorD(1)-1d0     !  obsolete!!! test of pretabulation,may be replace with alpha_s???
* translate QCD R-factors and corrections
         QCDcorR(1) = 1d0
         QCDcorR(2) = 1d0
         QCDcorR(3) = 0d0
         QCDcorR(4) = 0d0
         KFf = ABS(m_KFfin)
         IF(     KFf .EQ. 1 ) THEN
            QCDcorR(1) = QCDcorD(3)  ! D quark, R Vector
            QCDcorR(2) = QCDcorD(4)  ! D quark, R Axial
            QCDcorR(3) = QCDcorD(13) ! singlet
            QCDcorR(4) = QCDcorD(14) ! f_1
         ELSEIF( KFf .EQ. 2 ) THEN
            QCDcorR(1) = QCDcorD(1)  ! U quark, R Vector
            QCDcorR(2) = QCDcorD(2)  ! U quark, R Axial
            QCDcorR(3) = QCDcorD(13) ! singlet
            QCDcorR(4) = QCDcorD(14) ! f_1
         ELSEIF( KFf .EQ. 3 ) THEN
            QCDcorR(1) = QCDcorD(7)  ! S quark, R Vector
            QCDcorR(2) = QCDcorD(8)  ! S quark, R Axial
            QCDcorR(3) = QCDcorD(13) ! singlet
            QCDcorR(4) = QCDcorD(14) ! f_1
         ELSEIF( KFf .EQ. 4 ) THEN
            QCDcorR(1) = QCDcorD(5)  ! C quark, R Vector
            QCDcorR(2) = QCDcorD(6)  ! C quark, R Axial
            QCDcorR(3) = QCDcorD(13) ! singlet
            QCDcorR(4) = QCDcorD(14) ! f_1
         ELSEIF( KFf .EQ. 5 ) THEN
            QCDcorR(1) = QCDcorD(11) ! B quark, R Vector
            QCDcorR(2) = QCDcorD(12) ! B quark, R Axial
            QCDcorR(3) = QCDcorD(13) ! singlet
            QCDcorR(4) = QCDcorD(14) ! f_1
         ENDIF
      ELSE
         WRITE(*,*) '++++ STOP in DZface_MakeGSW, wrong iBox =',iBox
         STOP
      ENDIF
      END

      SUBROUTINE DZface_GivIzo(idferm,ihelic,sizo3,charge,kolor)
* ----------------------------------------------------------------------
* Provides electric charge and weak izospin of a family fermion where
* idferm =           1,        2,        3,         4,
* denotes:    neutrino,   lepton,       up,      down   (quark)
* negative idferm=-1,-2,-3,-4, denotes corresponding antiparticle
* ihelic =     +1,  -1   denotes  right and left handednes ( chirality)
* sizo3 is third projection of weak izospin (plus minus half)
* and charge is electric charge in units of electron charge
* kolor is a qcd colour, 1 for lepton, 3 for quarks
* ----------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
*
      IF(idferm.EQ.0  .OR.  iabs(idferm).GT.4 ) GOTO 901
      IF(iabs(ihelic).NE.1)                     GOTO 901
      ih  =ihelic
      idtype =iabs(idferm)
      ic  =idferm/idtype
      lepqua=int(idtype*0.4999999d0)
      iupdow=idtype-2*lepqua-1
      charge  =(-iupdow+2d0/3d0*lepqua)*ic
      sizo3   =0.25d0*(ic-ih)*(1-2*iupdow)
      kolor=1+2*lepqua
* Note that conventionaly Z0 coupling is
* xoupz=(sizo3-charge*swsq)/sqrt(swsq*(1-swsq))
      RETURN
 901  print *,' STOP in DZface_GivIzo: wrong params.'
      STOP
      END


      SUBROUTINE DZface_GetPrm( zmass,gamz0,gamzf,wmass,gamw0,gamwf,sin2w)
*/////////////////////////////////////////////////////////////////////////
*//     Gets out params from Dizet common blocks
*/////////////////////////////////////////////////////////////////////////
      IMPLICIT DOUBLE PRECISION  (a-h,o-z)
      COMMON /cdzzwg/camz,camh,gmu,a0,gamz,gamw,calsz,calst,calxi,calqed
      COMMON /cdzwsm/amw2,amz2,r,r1,r12,r2,amh2,rw,rw1,rw12,rw2,rz,rz1,
     $               rz12,rz2,alr,alrw,alrz,sw2m,cw2m,aksx,r1w,r1w2
*     -------------------------------------------------------------
      sin2w = r1
      wmass = SQRT(amw2)
      zmass = wmass/SQRT(1d0-sin2w)
      gamw0 = gamw
      gamz0 = gamz
      END

      SUBROUTINE DZface_MakeQCDcor(ww,QCDcor)
*/////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE '../bornv/BornV.fi'
      INCLUDE 'DZface.fi'
      DOUBLE PRECISION ww, ene, AlfQED             ! Input
      DOUBLE PRECISION ALPHTT,ALPHXI, QCDcor(0:14) ! Output
      INTEGER  i,j,k
*     -------------------------------------------------------------
      ene     = MAX(ww,20d0)
      AlfQED  = 1d0/m_alfinvMZ
      CALL qcdcof(ene, m_amtop, m_swsq, AlfQED, m_alfQCDMZ, ALPHTT, ALPHXI, QCDcor) !
c[[[[[[[[[[[[[[
c      IF( ABS(ww-189d0).LT.0.1) 
c     $   WRITE(*,'(a,f15.6, 20f8.5)') '|||| ene,QCDcor= ',ene,(QCDcor(k),k=0,14) !
c]]]]]]]]]]]]]]
      END
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  DZface                                //
*//////////////////////////////////////////////////////////////////////////////
