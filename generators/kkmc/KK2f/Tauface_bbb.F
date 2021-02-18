#define INITDK INITDK_BBB
#define TAURDF TAURDF_BBB
#define TAUBRA TAUBRA_BBB
#define TAUDCD TAUDCD_BBB      
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


      SUBROUTINE INISAMPL
C Initialization of parameters used in optimalization of phase space generation
C
C only efficiency/speed of generation depend on the actual choice, 
C unless e.g. for a given channel probablilities (used for parallel presamplers) are negative 
C or their sum is bigger than 1.
C `widths and masses '  of presampling resonances which are unphysical and used to parametrize change
C of generation variables, have to be chosen reasonably as well.
      CALL INISAMPL2
      CALL INISAMPL3
      CALL INISAMPL4
      CALL INISAMPL5
      END

      SUBROUTINE INISAMPL2
      include '../tauola/TAUDCD_BBB_size.inc'
      COMMON /SAMPL2/ PROB1(NM2),PROB2(NM2),AM2(NM2),GAM2(NM2),AM3(NM2),GAM3(NM2)
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU             
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1                
     *                 ,AMK,AMKZ,AMKST,GAMKST                           
C                                                                       
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU             
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1                
     *                 ,AMK,AMKZ,AMKST,GAMKST                           


C initialization for 2 scalars
      DO INUM=1,NM2
        AM2(INUM) =AMKST
        GAM2(INUM)=GAMKST
        AM3(INUM) =AMRO
        GAM3(INUM)=GAMRO

!         PROB(1) flat
!         PROB(2) K*
!         PROB(3) rho
        IF    (INUM.EQ.1) THEN
          PROB1(INUM)=0.0 !0.2
          PROB2(INUM)=0.0
        ELSEIF(INUM.EQ.2.OR.INUM.EQ.64.OR.INUM.EQ.65) THEN
          PROB1(INUM)=0.0 !0.2
          PROB2(INUM)=1.0
        ELSEIF(INUM.EQ.3) THEN
          PROB1(INUM)=0.0 !0.2
          PROB2(INUM)=1.0
        ELSEIF(INUM.EQ.4.OR.INUM.EQ.66.OR.INUM.EQ.67) THEN
          PROB1(INUM)=1.0 !0.2
          PROB2(INUM)=0.0
        ELSE
          PROB1(INUM)=1.0 !0.2
          PROB2(INUM)=0.0

        ENDIF
      ENDDO
      END

      SUBROUTINE INISAMPL3
      include '../tauola/TAUDCD_BBB_size.inc'
      COMMON /SAMPL3/ PROB1(NM3),PROB2(NM3),AMRX(NM3),GAMRX(NM3),AMRA(NM3),GAMRA(NM3),AMRB(NM3),GAMRB(NM3)
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU             
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1                
     *                 ,AMK,AMKZ,AMKST,GAMKST                           
C                                                                       
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU             
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1                
     *                 ,AMK,AMKZ,AMKST,GAMKST                           

      REAL*8  AMDARK, GMDARK, EPDARK
      COMMON /CMDARK/ AMDARK, GMDARK, EPDARK

C     initialization for 3 scalars
      AMROP=1.1
      GAMROP=0.36
      AMOM=.782
      GAMOM=0.0084


      DO MNUM=1,NM3
C     XXXXA CORRESPOND TO S2 CHANNEL !
       IF(MNUM.EQ.10) THEN
        PROB1(MNUM)=0.5
        PROB2(MNUM)=0.5
        AMRX(MNUM) =AMA1
        GAMRX(MNUM)=GAMA1
        AMRA(MNUM) =AMRO
        GAMRA(MNUM)=GAMRO
        AMRB(MNUM) =AMRO
        GAMRB(MNUM)=GAMRO
       ELSEIF(MNUM.EQ.1) THEN
        PROB1(MNUM)=0.5
        PROB2(MNUM)=0.5
        AMRX(MNUM) =1.57
        GAMRX(MNUM)=0.9
        AMRB(MNUM) =AMKST
        GAMRB(MNUM)=GAMKST
        AMRA(MNUM) =AMRO
        GAMRA(MNUM)=GAMRO
       ELSEIF(MNUM.EQ.2.OR.(MNUM.GE.19.AND.MNUM.LE.21)) THEN
        PROB1(MNUM)=0.5
        PROB2(MNUM)=0.5
        AMRX(MNUM) =1.57
        GAMRX(MNUM)=0.9
        AMRB(MNUM) =AMKST
        GAMRB(MNUM)=GAMKST
        AMRA(MNUM) =AMRO
        GAMRA(MNUM)=GAMRO
       ELSEIF(MNUM.EQ.3.OR.(MNUM.GE.22.AND.MNUM.LE.23)) THEN
        PROB1(MNUM)=0.5
        PROB2(MNUM)=0.5
        AMRX(MNUM) =1.27
        GAMRX(MNUM)=0.3
        AMRA(MNUM) =AMKST
        GAMRA(MNUM)=GAMKST
        AMRB(MNUM) =AMKST
        GAMRB(MNUM)=GAMKST
       ELSEIF(MNUM.EQ.4) THEN
        PROB1(MNUM)=0.5
        PROB2(MNUM)=0.5
        AMRX(MNUM) =1.27
        GAMRX(MNUM)=0.3
        AMRA(MNUM) =AMKST
        GAMRA(MNUM)=GAMKST
        AMRB (MNUM)=AMKST
        GAMRB(MNUM)=GAMKST
       ELSEIF(MNUM.EQ.5) THEN
        PROB1(MNUM)=0.5
        PROB2(MNUM)=0.5
        AMRX(MNUM) =1.27
        GAMRX(MNUM)=0.3
        AMRA(MNUM) =AMKST
        GAMRA(MNUM)=GAMKST
        AMRB(MNUM) =AMRO
        GAMRB(MNUM)=GAMRO
       ELSEIF(MNUM.EQ.6.OR.(MNUM.GE.24.AND.MNUM.LE.25)) THEN
        PROB1(MNUM)=0.4
        PROB2(MNUM)=0.4
        AMRX(MNUM) =1.27
        GAMRX(MNUM)=0.3
        AMRA(MNUM) =AMRO
        GAMRA(MNUM)=GAMRO
        AMRB(MNUM) =AMKST
        GAMRB(MNUM)=GAMKST
       ELSEIF(MNUM.EQ.7.OR.(MNUM.GE.31.AND.MNUM.LE.34)) THEN
        PROB1(MNUM)=0.0
        PROB2(MNUM)=1.0
        AMRX (MNUM)=1.27
        GAMRX(MNUM)=0.9
        AMRA(MNUM) =AMRO
        GAMRA(MNUM)=GAMRO
        AMRB(MNUM) =AMRO
        GAMRB(MNUM)=GAMRO
       ELSEIF(MNUM.EQ.8) THEN
        PROB1(MNUM)=0.0
        PROB2(MNUM)=1.0
        AMRX(MNUM) =AMROP
        GAMRX(MNUM)=GAMROP
        AMRB(MNUM) =AMOM
        GAMRB(MNUM)=GAMOM
        AMRA(MNUM) =AMRO
        GAMRA(MNUM)=GAMRO
       ELSEIF(MNUM.EQ.9) THEN
        PROB1(MNUM)=0.5
        PROB2(MNUM)=0.5
        AMRX(MNUM) =AMA1
        GAMRX(MNUM)=GAMA1
        AMRA(MNUM) =AMRO
        GAMRA(MNUM)=GAMRO
        AMRB(MNUM) =AMRO
        GAMRB(MNUM)=GAMRO
       ELSEIF((MNUM.EQ.13.OR.MNUM.EQ.14).OR.(MNUM.GE.26.AND.MNUM.LE.27).OR.(MNUM.GE.31.AND.MNUM.EQ.46)) THEN
        PROB1(MNUM)=0.5
        PROB2(MNUM)=0.5
        AMRX (MNUM)=1.57
        GAMRX(MNUM)=0.9
        AMRA(MNUM) =AMKST
        GAMRA(MNUM)=GAMKST
        AMRB(MNUM) =1.43
        GAMRB(MNUM)=0.9
       ELSEIF (MNUM.EQ.47) THEN  ! piee, 0.0, 0.8, 0.16, 0.1, 0.7759, 0.1479, 0.001, 0.005
        PROB1(MNUM)=0.0
        PROB2(MNUM)=0.8
        AMRX (MNUM)=0.16
        GAMRX(MNUM)=0.1
        AMRA(MNUM) =0.7759
        GAMRA(MNUM)=0.1479
        AMRB(MNUM) =0.001
        GAMRB(MNUM)=0.005
        IF (AMDARK.GT.1.0D-6) THEN
           AMRX (MNUM) = 2*AMDARK
           GAMRX(MNUM) = AMDARK
           AMRA (MNUM) = AMDARK
           GAMRA(MNUM) = GMDARK
           AMRB (MNUM) = AMDARK
           GAMRB(MNUM) = GMDARK
        ENDIF
      ELSEIF (MNUM.EQ.48) THEN  ! pimumu, 0.0, 0.8, 0.7, 0.4, 0.7759, 0.1479, 0.27, 0.2
        PROB1(MNUM)=0.0
        PROB2(MNUM)=0.8
        AMRX (MNUM)=0.7
        GAMRX(MNUM)=0.4
        AMRA(MNUM) =0.7759
        GAMRA(MNUM)=0.1479
        AMRB(MNUM) =0.27
        GAMRB(MNUM)=0.2
        IF (AMDARK.GT.1.0D-6) THEN
           AMRX (MNUM) = 2*AMDARK
           GAMRX(MNUM) = AMDARK
           AMRA (MNUM) = AMDARK
           GAMRA(MNUM) = GMDARK
           AMRB (MNUM) = AMDARK
           GAMRB(MNUM) = GMDARK
        ENDIF
       ELSE
        PROB1(MNUM)=0.0
        PROB2(MNUM)=0.0
        AMRX(MNUM) =AMA1
        GAMRX(MNUM)=GAMA1
        AMRA(MNUM) =AMRO
        GAMRA(MNUM)=GAMRO
        AMRB(MNUM) =AMRO
        GAMRB(MNUM)=GAMRO
       ENDIF

      ENDDO
      END

      SUBROUTINE INISAMPL4
      include '../tauola/TAUDCD_BBB_size.inc'
      COMMON /SAMPL4/ PROB1(NM4),PROB2(NM4),AMRX(NM4),GAMRX(NM4),AMRA(NM4),GAMRA(NM4)

      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU             
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1                
     *                 ,AMK,AMKZ,AMKST,GAMKST                           
C                                                                       
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU             
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1                
     *                 ,AMK,AMKZ,AMKST,GAMKST                           

      REAL*8  AMDARK, GMDARK, EPDARK
      COMMON /CMDARK/ AMDARK, GMDARK, EPDARK

C initialization for 4 scalars
      DO INUM=1,NM4
       AMOM=.782
       GAMOM=0.0084
       IF(INUM.EQ.1) THEN
        PROB1(INUM)=.35
        PROB2(INUM)=.35
        AMRX(INUM) =1.2
        GAMRX(INUM)=.46
        AMRA(INUM) =AMOM
        GAMRA(INUM)=GAMOM
       ELSEIF(INUM.EQ.2) THEN
        PROB1(INUM)=0.0
        PROB2(INUM)=0.0
        AMRX(INUM) =1.4
        GAMRX(INUM)=.6
        AMRA(INUM) =AMOM
        GAMRA(INUM)=GAMOM
       ELSE IF (INUM.EQ.3) THEN ! eee, 0.0, 0.8, 1.0, 0.9, 0.03, 0.07
        PROB1(INUM)=0.0
        PROB2(INUM)=0.8
        AMRX(INUM) =1.0
        GAMRX(INUM)=0.9
        AMRA(INUM) =0.03
        GAMRA(INUM)=0.07
       ELSE IF (INUM.EQ.4) THEN ! mumumu, 0.0, 0.8, 1.0, 0.9, 0.15, 0.1
        PROB1(INUM)=0.0
        PROB2(INUM)=0.8
        AMRX(INUM) =1.0
        GAMRX(INUM)=0.9
        AMRA(INUM) =0.15
        GAMRA(INUM)=0.1
       ELSE IF (INUM.EQ.5) THEN ! emumu, 0.0, 0.8, 1.0, 0.9, 0.15, 0.1
        PROB1(INUM)=0.0
        PROB2(INUM)=0.8
        AMRX(INUM) =1.0
        GAMRX(INUM)=0.9
        AMRA(INUM) =0.15
        GAMRA(INUM)=0.1
       ELSE IF (INUM.EQ.6) THEN ! muee, 0.0, 0.8, 1.0, 0.9, 0.03, 0.07
        PROB1(INUM)=0.0
        PROB2(INUM)=0.8
        AMRX(INUM) =1.0
        GAMRX(INUM)=0.9
        AMRA(INUM) =0.03
        GAMRA(INUM)=0.07
       ELSEIF((INUM.GT.6.AND.INUM.LE.12).OR.(INUM.GE.28.AND.INUM.LE.31)) THEN
        PROB1(INUM)=0.0
        PROB2(INUM)=0.0
        AMRX(INUM) =1.4
        GAMRX(INUM)=.6
        AMRA(INUM) =AMOM
        GAMRA(INUM)=GAMOM
       ELSE
        PROB1(INUM)=0.0
        PROB2(INUM)=0.0
        AMRX(INUM) =AMA1
        GAMRX(INUM)=GAMA1
        AMRA(INUM) =AMRO
        GAMRA(INUM)=GAMRO
      ENDIF
      IF (INUM.GE.3.AND.INUM.LE.6) THEN
         IF (AMDARK.GT.1.0D-6) THEN
            AMRA(INUM) = AMDARK
            GAMRA(INUM) = 2.0D0*GMDARK
         ENDIF
      ENDIF
      ENDDO
      END

      SUBROUTINE INISAMPL5
      include '../tauola/TAUDCD_BBB_size.inc'
      REAL*8          AMOM,GAMOM
      COMMON /SAMPL5/ PROBa2(NM5),PROBOM(NM5),ama2(NM5),gama2(NM5),AMOM(NM5),GAMOM(NM5)

      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU             
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1                
     *                 ,AMK,AMKZ,AMKST,GAMKST                           
C                                                                       
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU             
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1                
     *                 ,AMK,AMKZ,AMKST,GAMKST                           


C initialization for 5 scalars
      DO INUM=1,NM5
         PROBa2(INUM)=0.7
         PROBOM(INUM)=0.7
         ama2(INUM)=1.260
         gama2(INUM)=0.400
         AMOM(INUM)=.78257
         GAMOM(INUM)=.7
         IF (INUM.EQ.1.OR.INUM.EQ.2) GAMOM(INUM)= 0.00844
      ENDDO
      END

      SUBROUTINE INITDK(ITAUXPAR,xpar)
* ----------------------------------------------------------------------
*     INITIALISATION OF TAU DECAY PARAMETERS  and routines
*
*     called by : KORALZ
* ----------------------------------------------------------------------
      include '../tauola/TAUDCD_BBB_size.inc'
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
      integer KEY0,KEY1,KEY2,KEY3,KEY4,KEY5,KEY6
      COMMON /METYP/ KEY0(2),KEY1(NM1),KEY2(NM2),KEY3(NM3),
     $               KEY4(NM4),KEY5(NM5),KEY6(NM6)
      INTEGER k
      INTEGER KSUM,ACTIVE
      COMMON  /KSUM /KSUM,ACTIVE(NBMC)
C     
      CHARACTER OLDNAMES(NLT)*31
      CHARACTER*80 bxINIT
      PARAMETER (
     $  bxINIT ='(1x,1h*,g17.8,            16x, a31,a4,a4, 1x,1h*)'
     $ )
      REAL*4 PI,POL1(4)

C SWITCHES FOR OPTIONS IN INITIALIZATION:
C =======================================================================
C switch to initialize that all modes are represented equally.
      INTEGER IFEQUALBR
      COMMON /IFEQUALBR/ IFEQUALBR
C==========================================================================

C      PARAMETER (NMODE=196,NM1=40,NM2=71,NM3=19,NM4=32,NM5=21,NM6=13)
*
*
* LIST OF BRANCHING RATIOS (from old times)
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
*                 ET,P-,P0 , P-,P0,GM  , P-,P0,P0
*                  9, 1, 2  , 1, 2, 8  ,  1, 2, 2
*

C
      DIMENSION NPIK(NMODE),NPIK6(NM6)
      DIMENSION NOPIK1(JMAX,NM1),NOPIK2(JMAX,NM2),NOPIK3(JMAX,NM3)
      DIMENSION NOPIK6(JMAX,NM6),NOPIK5(JMAX,NM5),NOPIK4(JMAX,NM4)
      DIMENSION KEYstrt0(2),KEYstrt1(NM1),KEYstrt2(NM2),KEYstrt3(NM3)
      DIMENSION             KEYstrt4(NM4),KEYstrt5(NM5),KEYstrt6(NM6)

*AM   outgoing multiplicity and flavors of multi-pion /multi-K modes    
      DATA   NPIK6  /                
     x                                                    5,    ! old npi starts here
     2                              6,                    6,
     a                              7,                    7,    ! new (may 2004)
     b                              8,                    6,    ! new (may 2004)
     c                              6,                    6,    ! new (may 2004)
     d                              6,                    6,    ! new (may 2004)
     e                              6,                    6,    ! new (may 2004)
     o                              6,                    6,    ! new (may 2020)          
     o                              6,                    6,    ! new (may 2020)          
     o                              6,                    6,    ! new (may 2020)          
     o                              6/                          ! new (may 2020)          
C MatrEl key 0- channel not initialized,  1- constant ME flat phase space
C            2- default ME,               3- default ME, but one stable spin>0
C            4- default ME wrapped curr., 5- wrapped ME  
      DATA KEYstrt0 / 2, 2/
      DATA KEYstrt1 / 2, 2, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 0, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 0,   0, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0/
      DATA KEYstrt2 / 2, 2, 2, 2, 1,   1, 1, 1, 1, 1, 
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 2, 2,   2, 2, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 0, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0/
      DATA KEYstrt3 / 2, 2, 2, 2, 2,   2, 2, 3, 2, 2, 
     a                1, 1, 2, 2, 1,   1, 1, 1, 2, 2,
     a                2, 2, 2, 2, 2,   2, 2, 1, 1, 1,
     a                2, 2, 2, 2, 2,   2, 2, 2, 2, 2,
     a                2, 2, 2, 2, 2,   2, 5, 5, 0, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0/
      DATA KEYstrt4 / 2, 2, 5, 5, 5,   5, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 0, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 1, 1,
     a                1, 1, 1, 1, 1,   1, 1, 1, 0, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0/
      DATA KEYstrt5 / 2, 2, 0, 2, 2,   2, 1, 1, 2, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0/
      DATA KEYstrt6 / 2, 2, 2, 2, 2,   2, 2, 0, 0, 0,
     a                0, 0, 0, 0, 0,   0, 0, 0, 0, 0/

      DATA NOPIK4 / -1,-1, 1, 2, 0, 0,3*0,     2, 2, 2,-1, 0, 0,3*0,                      ! 2
     a              12,-11,11,-11,0,0,3*0,    14,-13,13,-13,0,0,3*0,     ! new (may 2004) ! 4
     b              12,-11,13,-13,0,0,3*0,    14,-13,11,-11,0,0,3*0,     ! new (may 2004) ! 6
     c              -3, 2, 2, 2, 0, 0,3*0,     2, 2, 9,-3, 0, 0,3*0,     ! new (may 2004) ! 8
     d               2, 2, 4,-1, 0, 0,3*0,     2, 4, 9,-1, 0, 0,3*0,     ! new (may 2004) ! 10
     e               2,-1, 1,-3, 0, 0,3*0,     4,-1, 1,-1, 0, 0,3*0,     ! new (may 2004) ! 12
     a               2, 2, 9,-1, 0, 0,3*0,     4,-4, 9,-1, 0, 0,3*0,     ! new (sep 2004) ! 14
     b               4,-4, 2,-1, 0, 0,3*0,     4,-4, 4,-1, 0, 0,3*0,     ! new (sep 2004) ! 16
     c               4, 2, 2,-3, 0, 0,3*0,     4,-4, 2,-3, 0, 0,3*0,     ! new (sep 2004) ! 18
     d               2, 4, 9,-3, 0, 0,3*0,    -1, 1,-1, 9, 0, 0,3*0,     ! new (sep 2004) ! 20
     e              -1, 3,-3, 2, 0, 0,3*0,    -3, 3,-3, 2, 0, 0,3*0,     ! new (sep 2004) ! 22
     a              -3, 3,-3, 4, 0, 0,3*0,    -3, 1,-1, 4, 0, 0,3*0,     ! new (sep 2004) ! 24
     b              -3, 3,-1, 4, 0, 0,3*0,    -1, 1,-1, 223, 0, 0,3*0,   ! new (sep 2004) ! 26
     c              -1, 2, 2,223,0, 0,3*0,     2, 2,310, -1, 0, 0,3*0,   ! new (may 2020) ! 28
     d               2, 2,130,-1, 0,0,3*0,     310,-1, 1,-1, 0, 0,3*0,   ! new (may 2020) ! 30
     e               130,-1, 1,-1,0,0,3*0,     310,310,2,-1, 0, 0,3*0,   ! new (may 2020) ! 32
     a               130,130,2,-1,0,0,3*0,     310,130,2,-1, 0, 0,3*0,   ! new (may 2020) ! 34
     b               2, 2, 9,-1, 0, 0,3*0,     2, 2, 9,-1, 0, 0,3*0,     ! new (may 2020) ! 36
     c               2, 2, 9,-1, 0, 0,3*0,     2, 2, 9,-1, 0, 0,3*0,     ! new (may 2020) ! 38
     d              -1, 1,-1, 9, 0, 0,3*0,    -1, 1,-1, 9, 0, 0,3*0,     ! new (may 2020) ! 40
     e              -1, 1,-1, 9, 0, 0,3*0,    -1, 1,-1, 9, 0, 0,3*0,     ! new (may 2020) ! 42
     a              -1, 1,-1,223,0, 0,3*0,    -1, 1,-1,223,0, 0,3*0,     ! new (may 2020) ! 44
     b              -1, 1,-1,223,0, 0,3*0,    -1, 2, 2,223,0, 0,3*0,     ! new (may 2020) ! 46
     c              -1, 2, 2,223,0, 0,3*0,    -1, 2, 2,223,0, 0,3*0,     ! new (may 2020) ! 48
     d               4, 2, 2,-1, 0, 0,3*0,     4, 2, 2,-1, 0, 0,3*0,     ! new (may 2020) ! 50
     e               4, 2, 2,-1, 0, 0,3*0,     4, 2, 2,-1, 0, 0,3*0,     ! new (may 2020) ! 52
     a               4, 2, 2,-1, 0, 0,3*0,     4, 2, 2,-1, 0, 0,3*0,     ! new (may 2020) ! 54
     b               4, 2, 2,-1, 0, 0,3*0,     4, 2, 2,-1, 0, 0,3*0,     ! new (may 2020) ! 56
     c               4, 2, 2,-1, 0, 0,3*0,     4, 2, 2,-1, 0, 0,3*0,     ! new (may 2020) ! 58
     d               4, 2, 2,-1, 0, 0,3*0,     4, 2, 2,-1, 0, 0,3*0/     ! new (may 2020) ! 60

      DATA NOPIK5 /
     1              -1,-1, 1, 2, 2, 0,3*0,  
     a              -1,-1, 1, 2, 2, 0,3*0,     2, 2, 2, 2, 2, 0,3*0,     ! new (may 2004)
     a               1,-1,-1, 2, 2, 0,3*0,    -1, 2, 2, 2, 2, 0,3*0,     ! new (may 2004)
     a              -1, 1, 1,-1,-1, 0,3*0,    -1,-1, 1, 1,-3, 0,3*0,     ! new (may 2004)
     a              -1,-1, 1, 2, 4, 0,3*0,    -1, 2, 2, 2, 2, 0,3*0,     ! new (may 2004)
     a              -1,-1, 1, 2, 4, 0,3*0,    -1,-1, 1, 2, 4, 0,3*0,     ! new (may 2004)
     a              -1,-1, 1, 2, 4, 0,3*0,    -1,-1, 1, 2, 4, 0,3*0,     ! new (sep 2004)
     a              -1,-1, 1, 2, 4, 0,3*0,    -1,-1, 1, 2, 4, 0,3*0,     ! new (sep 2004)
     a              -1,-1, 1, 2, 4, 0,3*0,    -1,-1, 1, 2, 4, 0,3*0,     ! new (sep 2004)
     a              -1,-1, 1, 2, 4, 0,3*0,    -1,-1, 1, 2, 4, 0,3*0,     ! new (sep 2004)
     a              -1,-1, 1, 2, 4, 0,3*0/                               ! new (sep 2004)

      DATA NOPIK6 /
     x                                        -1,-1,-1, 1, 1, 0,3*0,     ! old npi starts here
     2              -1,-1,-1, 1, 1, 2,3*0,    -1,-1, 1, 2, 2, 2,3*0,
     a              -1,-1,-1, 1, 1,2*2,2*0,   -1,-1,-1,-1, 1,2*1,2*0,    ! new (may 2004)  7PI
     b              -1,-1,-1,-1,3*1, 2, 0,    -1,-1, 1, 1,-3, 2,3*0,     ! new (may 2004)
     c              -1,-1,-1, 1, 1, 1,3*0,    -1,-1, 1, 2, 2, 1,3*0,     ! new (may 2004)
     d              -1,-1,-1, 1, 1, 1,3*0,    -1,-1, 1, 2, 2, 1,3*0,     ! new (may 2004)
     e              -1,-1,-1, 1, 1, 1,3*0,    -1,-1, 1, 2, 2, 1,3*0,     ! new (may 2004)
     e              -1,-1,-1, 1, 1, 1,3*0,    -1,-1, 1, 2, 2, 1,3*0,     ! new (may 2020)
     e              -1,-1,-1, 1, 1, 1,3*0,    -1,-1, 1, 2, 2, 1,3*0,     ! new (may 2020)
     e              -1,-1,-1, 1, 1, 1,3*0,    -1,-1, 1, 2, 2, 1,3*0,     ! new (may 2020)
     e              -1,-1,-1, 1, 1, 1,3*0/                               ! new (may 2020)

      DATA NOPIK3 /
     3              -3,-1, 3, 0, 0, 0,3*0,    -4,-1, 4, 0, 0, 0,3*0,  
     4              -3, 2,-4, 0, 0, 0,3*0,     2, 2,-3, 0, 0, 0,3*0,  
     5              -3,-1, 1, 0, 0, 0,3*0,    -1, 4, 2, 0, 0, 0,3*0,  
     6               9,-1, 2, 0, 0, 0,3*0,    -1, 2, 8, 0, 0, 0,3*0,
C AJWMOD fix sign bug, 2/22/99
     7               2, 2,-1, 0, 0, 0,3*0,                           ! new (may 2004) but useful
     7              -1,-1, 1, 0, 0, 0,3*0,    -3,-3, 3, 0, 0, 0,3*0, ! new (may 2004)  ! 11
     7              -3, 4, 4, 0, 0, 0,3*0,    -3, 9, 2, 0, 0, 0,3*0, ! new (may 2004)  ! 13
     7               4, 9,-1, 0, 0, 0,3*0,    -3, 4,113, 0, 0, 0,3*0, ! new (may 2004) ! 15
     7              -1,333, 2, 0, 0, 0,3*0,   -3,333, 2, 0, 0, 0,3*0, ! new (may 2004) ! 17
     7               4, 9,-3, 0, 0, 0,3*0,   310,-1,310, 0, 0, 0,3*0, ! new (may 2020) ! 19
     7              130,-1,130, 0, 0, 0,3*0, 310,-1,130, 0, 0, 0,3*0, ! new (may 2020) ! 21
     7              -3, 2,310, 0, 0, 0,3*0,   -3, 2,130, 0, 0, 0,3*0, ! new (may 2020) ! 23 
     7              -1, 310,2, 0, 0, 0,3*0,   -1,130, 2, 0, 0, 0,3*0, ! new (may 2020) ! 25  
     7              310, 9,-1, 0, 0, 0,3*0,  130, 9,-1, 0, 0, 0,3*0,  ! new (may 2020) ! 27
     7              -1, 2,223, 0, 0, 0,3*0,                           ! new (may 2020) ! 28  
     7              -1, 2,223, 0, 0, 0,3*0,  -1, 2, 223, 0, 0, 0,3*0, ! new (may 2020) ! 30
     7               9,-1,  2, 0, 0, 0,3*0,   9,-1,   2, 0, 0, 0,3*0, ! new (may 2020) ! 32 
     7               9,-1,  2, 0, 0, 0,3*0,   9,-1,   2, 0, 0, 0,3*0, ! new (may 2020) ! 34
     7              -3, 9,  2, 0, 0, 0,3*0,  -3, 9,   2, 0, 0, 0,3*0, ! new (may 2020) ! 36
     7              -3, 9,  2, 0, 0, 0,3*0,  -3, 9,   2, 0, 0, 0,3*0, ! new (may 2020) ! 38
     7              310, 9,-1, 0, 0, 0,3*0,   310, 9,-1, 0, 0, 0,3*0, ! new (may 2020) ! 40
     7              310, 9,-1, 0, 0, 0,3*0,   310, 9,-1, 0, 0, 0,3*0, ! new (may 2020) ! 42
     7              130, 9,-1, 0, 0, 0,3*0,   130, 9,-1, 0, 0, 0,3*0, ! new (may 2020) ! 44
     7              130, 9,-1, 0, 0, 0,3*0,   130, 9,-1, 0, 0, 0,3*0, ! new (may 2020) ! 46
     7              -1,11,-11, 0, 0, 0,3*0,  -1, 13,-13, 0, 0, 0,3*0, ! new (may 2020) ! 48
     7               2, 2,  2, 0, 0, 0,3*0,   2,  2,  2, 0, 0, 0,3*0, ! new (may 2020) 
     7               2, 2,  2, 0, 0, 0,3*0,   2,  2,  2, 0, 0, 0,3*0, ! new (may 2020) 
     7               2, 2,  2, 0, 0, 0,3*0,   2,  2,  2, 0, 0, 0,3*0, ! new (may 2020) 
     7               2, 2,  2, 0, 0, 0,3*0,   2,  2,  2, 0, 0, 0,3*0, ! new (may 2020) 
     7               2, 2,  2, 0, 0, 0,3*0,   2,  2,  2, 0, 0, 0,3*0, ! new (may 2020) 
     7               2, 2,  2, 0, 0, 0,3*0,   2,  2,  2, 0, 0, 0,3*0/ ! new (may 2020) ostatnie 3

      DATA NOPIK2 /                            -1,  2,   0, 0, 0, 0,3*0, ! new (may 2004) !  1
     8             -1,   4,   0, 0, 0, 0,3*0,  -3,  2,   0, 0, 0, 0,3*0, ! new (may 2004) !  3
     8             -3,  -4,   0, 0, 0, 0,3*0, -13,-13,  13, 0, 0, 0,3*0, ! new (may 2004) !  5
     8             -13,-13,  11, 0, 0, 0,3*0, -13,-11,  13, 0, 0, 0,3*0, ! new (may 2004) !  7
     8             -13,-11,  11, 0, 0, 0,3*0,  13,-11, -11, 0, 0, 0,3*0, ! new (may 2004) !  9
     8             -11,-11,  11, 0, 0, 0,3*0,  -1,  1, -11, 0, 0, 0,3*0, ! new (may 2004) ! 11
     8             -1,   1, -13, 0, 0, 0,3*0,   1, -3, -11, 0, 0, 0,3*0, ! new (may 2004) ! 13
     8              1,  -3, -13, 0, 0, 0,3*0,  -1,  3, -11, 0, 0, 0,3*0, ! new (may 2004) ! 15
     8             -1,   3, -13, 0, 0, 0,3*0,  -3,  3, -11, 0, 0, 0,3*0, ! new (may 2004) ! 17
     8             -3,   3, -13, 0, 0, 0,3*0,   4,  4, -11, 0, 0, 0,3*0, ! new (may 2004) ! 19
     8              4,   4, -13, 0, 0, 0,3*0,  -1, -1,  11, 0, 0, 0,3*0, ! new (may 2004) ! 21
     8             -1,  -1,  13, 0, 0, 0,3*0,  -1, -3,  11, 0, 0, 0,3*0, ! new (may 2004) ! 23
     8             -1,  -3,  13, 0, 0, 0,3*0,  -3, -3,  11, 0, 0, 0,3*0, ! new (may 2004) ! 25
     8             -3,  -3,  13, 0, 0, 0,3*0, -13,-13,-2212,0, 0, 0,3*0, ! new (may 2004) ! 27
     8             -13, 13,2212, 0, 0, 0,3*0, -11,-11,-2212,0, 0, 0,3*0, ! new (may 2004) ! 29
     8             -11, 11,2212, 0, 0, 0,3*0,   9, -3,   0, 0, 0, 0,3*0, ! new (may 2004) ! 31
     8              9,  -1,   0, 0, 0, 0,3*0,  -1,333,   0, 0, 0, 0,3*0, ! new (may 2004) ! 33
     8             -3, 333,   0, 0, 0, 0,3*0,  -1,223,   0, 0, 0, 0,3*0, ! new (may 2004) ! 35
     8             -3, 223,   0, 0, 0, 0,3*0,  -1,331,   0, 0, 0, 0,3*0, ! new (may 2004) ! 37
     8             -3, 331,   0, 0, 0, 0,3*0, -11, 13, 2212,0, 0, 0,3*0, ! new (may 2004) ! 39
     8             11, -13,2212, 0, 0, 0,3*0, -11,-13,-2212,0, 0, 0,3*0, ! new (may 2004) ! 41
     8              2,    2,-11, 0, 0, 0,3*0,   2,  2, -13, 0, 0, 0,3*0, ! new (may 2004) ! 43
     8              2,    9,-11, 0, 0, 0,3*0,   2,  9, -13, 0, 0, 0,3*0, ! new (may 2004) ! 45
     8              2,  331,-11, 0, 0, 0,3*0,   2,331, -13, 0, 0, 0,3*0, ! new (may 2004) ! 47
     8              9,    9,-11, 0, 0, 0,3*0,   9,  9, -13, 0, 0, 0,3*0, ! new (may 2004) ! 49
     8              9,  331,-11, 0, 0, 0,3*0,   9,331, -13, 0, 0, 0,3*0, ! new (may 2004) ! 51
     8              2,  310,-11, 0, 0, 0,3*0,   2,310, -13, 0, 0, 0,3*0, ! new (may 2004) ! 53
     8              9,  310,-11, 0, 0, 0,3*0,   9,310, -13, 0, 0, 0,3*0, ! new (may 2004) ! 55
     8             331,310, -11, 0, 0, 0,3*0, 331,310, -13, 0, 0, 0,3*0, ! new (may 2004) ! 57
     8              1,   -3,2212,0, 0, 0,3*0,  -1, -3,-2212,0, 0, 0,3*0, ! new (may 2004) ! 59
     8              3,   -1,2212,0, 0, 0,3*0,   2,  2, 2212,0, 0, 0,3*0, ! new (may 2004) ! 61
     8              2,    9,2212,0, 0, 0,3*0,   2,310, 2212,0, 0, 0,3*0, ! new (may 2004) ! 63
     8             -1, 310,   0, 0, 0, 0,3*0,  -1,130,   0, 0, 0, 0,3*0, ! new (may 2020) ! 65
     8             -3, 310,   0, 0, 0, 0,3*0,  -3,130,   0, 0, 0, 0,3*0, ! new (may 2020) ! 67
     8             -3, 333,   0, 0, 0, 0,3*0,  -3,333,   0, 0, 0, 0,3*0, ! new (may 2020) ! 69
     8             -3, 333,   0, 0, 0, 0,3*0,  -1,20223, 0, 0, 0, 0,3*0, ! new (may 2020) ! 71
     8             -1,20223,  0, 0, 0, 0,3*0,  -1,20223, 0, 0, 0, 0,3*0, ! new (may 2020) ! 73
     8             -1, 223,   0, 0, 0, 0,3*0,                            ! new (may 2020) ! 74
     8             -1, 223,   0, 0, 0, 0,3*0,  -1,223,   0, 0, 0, 0,3*0, ! new (may 2020) ! 76
     8             213,223,   0, 0, 0, 0,3*0, 213,223,   0, 0, 0, 0,3*0, ! new (may 2020) ! 78
     8             213,223,   0, 0, 0, 0,3*0,  -3,  9,   0, 0, 0, 0,3*0, ! new (may 2020) ! 80
     8             -3,   9,   0, 0, 0, 0,3*0,  -3,  9,   0, 0, 0, 0,3*0, ! new (may 2020) ! 82
     8             -3,   9,   0, 0, 0, 0,3*0,  -1,  9,   0, 0, 0, 0,3*0, ! new (may 2020) ! 84
     8             -1,   9,   0, 0, 0, 0,3*0,  -1,  9,   0, 0, 0, 0,3*0, ! new (may 2020) ! 86
     8             -1,   9,   0, 0, 0, 0,3*0,  -3,223,   0, 0, 0, 0,3*0, ! new (may 2020) ! 88
     8             -3, 223,   0, 0, 0, 0,3*0,  -3,223,   0, 0, 0, 0,3*0, ! new (may 2020) ! 90
     8             -1,20223,  0, 0, 0, 0,3*0,  -1,20223, 0, 0, 0, 0,3*0, ! new (may 2020) ! 92
     8             -1,20223,  0, 0, 0, 0,3*0,  -1,20223, 0, 0, 0, 0,3*0, ! new (may 2020) ! 94
     8             -1,20223,  0, 0, 0, 0,3*0,  -1,20223, 0, 0, 0, 0,3*0, ! new (may 2020) ! 96
     8             -1,20223,  0, 0, 0, 0,3*0,  -1,20223, 0, 0, 0, 0,3*0, ! new (may 2020) ! 98
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 100
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 102
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 104
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 106
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 108
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 110
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 112
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 114
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 116
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 118
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 120
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 122
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 124
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 126
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 128
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 130
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 132
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 134
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 136
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0, ! new (may 2020) ! 138
     8             -3,   -3,  0, 0, 0, 0,3*0,  -3, -3,   0, 0, 0, 0,3*0/ ! new (may 2020)  ostattnie 2

      DATA NOPIK1 /
     o               -1,0,  0,   0,0,0,3*0,     -3,0,  0,    0,0,0,3*0, ! new (may 2004) ! 2
     o                8,0,-11,   0,0,0,3*0,      8,0,-13,    0,0,0,3*0, ! new (may 2004) ! 4
     o                2,0,-11,   0,0,0,3*0,      2,0,-13,    0,0,0,3*0, ! new (may 2004) ! 6
     o                9,0,-11,   0,0,0,3*0,      9,0,-13,    0,0,0,3*0, ! new (may 2004) ! 8
     o              -11,0,130,   0,0,0,3*0,    -13,0,130,    0,0,0,3*0, ! new (may 2004) !10
     o              -11,0,  0, 223,0,0,3*0,    -13,0,  0,  223,0,0,3*0, ! new (may 2004) !12
     o              -11,0,  0, 333,0,0,3*0,    -13,0,  0,  333,0,0,3*0, ! new (may 2004) !14
     o              -11,0,  0, 113,0,0,3*0,    -13,0,  0,  113,0,0,3*0, ! new (may 2004) !16
     o          9000211,0,  0,   0,0,0,3*0,  10213,0,  0,    0,0,0,3*0, ! new (may 2004) !18
     o              -11,0,  0, 310,0,0,3*0,    -13,0,  0,  310,0,0,3*0, ! new (may 2004) !20
     o             2212,0,  0,  22,0,0,3*0,   2212,0,  0,  111,0,0,3*0, ! new (may 2004) !22
     o             2212,0,  0, 221,0,0,3*0,   2212,0,  0,    4,0,0,3*0, ! new (may 2004) !24
     o              -11,0,  0, 331,0,0,3*0,    -13,0,  0,  331,0,0,3*0, ! new (may 2004) !26
     o            -3122,0,211,   0,0,0,3*0,   3122,0,211,    0,0,0,3*0, ! new (may 2020) !28
     o            -3122,0,321,   0,0,0,3*0,   3122,0,321,    0,0,0,3*0, ! new (may 2020) !30
     o             -313,0,-11,   0,0,0,3*0,    313,0,-11,    0,0,0,3*0, ! new (may 2020) !32
     o             -313,0,-13,   0,0,0,3*0,    313,0,-13,    0,0,0,3*0, ! new (may 2020) !34
     o          9000111,0,-11,   0,0,0,3*0,9000111,0,-11,    0,0,0,3*0, ! new (may 2020) !36
     o          9010221,0,-11,   0,0,0,3*0,9010221,0,-13,    0,0,0,3*0, ! new (may 2020) !38
     o            20213,0,  0,   0,0,0,3*0,    213,0,  0,    0,0,0,3*0, ! new (may 2020) !40
     o          9000211,0,  0,   0,0,0,3*0,9000211,0,  0,    0,0,0,3*0, ! new (may 2020) !42  
     o          9000211,0,  0,   0,0,0,3*0,9000211,0,  0,    0,0,0,3*0, ! new (may 2020) !44     
     o            10213,0,  0,   0,0,0,3*0,  10213,0,  0,    0,0,0,3*0, ! new (may 2020) !46     
     o            10213,0,  0,   0,0,0,3*0,                             ! new (may 2020) !47     
     o              221,0,-11,   0,0,0,3*0,    221,0,-11,    0,0,0,3*0, ! new (may 2020) !49
     o              221,0,-11,   0,0,0,3*0,    221,0,-11,    0,0,0,3*0, ! new (may 2020) !51
     o              221,0,-13,   0,0,0,3*0,    221,0,-13,    0,0,0,3*0, ! new (may 2020) !53
     o              221,0,-13,   0,0,0,3*0,    221,0,-13,    0,0,0,3*0, ! new (may 2020) !55
     o              223,0,-11,   0,0,0,3*0,    223,0,-11,    0,0,0,3*0, ! new (may 2020) !57     
     o              223,0,-11,   0,0,0,3*0,                             ! new (may 2020) !58     
     o              223,0,-13,   0,0,0,3*0,    223,0,-13,    0,0,0,3*0, ! new (may 2020) !60     
     o              223,0,-13,   0,0,0,3*0,                             ! new (may 2020) !61
     o          9000111,0,-11,   0,0,0,3*0, 9000111,0,-11,   0,0,0,3*0, ! new (may 2020) !63
     o          9000111,0,-11,   0,0,0,3*0, 9000111,0,-11,   0,0,0,3*0, ! new (may 2020) !65
     o          9000111,0,-13,   0,0,0,3*0, 9000111,0,-13,   0,0,0,3*0, ! new (may 2020) !67
     o          9000111,0,-13,   0,0,0,3*0, 9000111,0,-13,   0,0,0,3*0, ! new (may 2020) !69
     o               -3, 0, 0, 0, 0, 0,3*0,     - 3, 0, 0, 0, 0, 0,3*0, ! new (may 2020) !71
     o               -3, 0, 0, 0, 0, 0,3*0,     - 3, 0, 0, 0, 0, 0,3*0, ! new (may 2020) !73
     o               -3, 0, 0, 0, 0, 0,3*0,     - 3, 0, 0, 0, 0, 0,3*0, ! new (may 2020) !75
     o               -3, 0, 0, 0, 0, 0,3*0,     - 3, 0, 0, 0, 0, 0,3*0, ! new (may 2020) !77
     o               -3, 0, 0, 0, 0, 0,3*0,     - 3, 0, 0, 0, 0, 0,3*0, ! new (may 2020) !79
     o               -3, 0, 0, 0, 0, 0,3*0/                             ! new (may 2020) !ostattnie 1

      NCHAN = NMODE + NLT



C Initialization of matrix elements, BRANCHING RATIOS, channel names and final state flavours
C it is grouped into segments accordingly to multiplicity for final state scalars

C=================================================
C LEPTONIC CHANNELS plus channels to be removed
C=================================================
      DO I=1,2
       KEY0(I)=KEYstrt0(I)
      ENDDO

      DO  I = 1,NLT
        JLIST(I) = I

        IF(I.EQ. 1) OLDNAMES(I)='  TAU-  -->   E-               ' ! TauDecayModeBBB = 1 ! explicitly included
        IF(I.EQ. 2) OLDNAMES(I)='  TAU-  -->  MU-               ' ! TauDecayModeBBB = 2 ! explicitly included

      ENDDO


C=================================================
C 4-scalar CHANNELS 
C=================================================

      DO II = 1,NM4
          I = II+NLT  ! position on the whole list of decay channels

        KEY4(II)=KEYstrt4(II)

        JLIST(I) = I
        NPIK(I-NLT)=4

        IF(II.EQ. 1) NAMES(I-NLT)='  TAU-  --> 2PI-,  PI+,  PI0   ' ! TauDecayModeBBB =  3 ! explicitly included
        IF(II.EQ. 2) NAMES(I-NLT)='  TAU-  --> 3PI0,        PI-   ' ! TauDecayModeBBB =  4 ! explicitly included
        IF(II.EQ. 3) NAMES(I-NLT)='  TAU-  --> nu_e e- e- e+      ' ! TauDecayModeBBB =  5 ! explicitly included
        IF(II.EQ. 4) NAMES(I-NLT)='  TAU-  --> nu_mu mu- mu- mu+  ' ! TauDecayModeBBB =  6
        IF(II.EQ. 5) NAMES(I-NLT)='  TAU-  --> nu_e e- mu- mu+    ' ! TauDecayModeBBB =  7
        IF(II.EQ. 6) NAMES(I-NLT)='  TAU-  --> nu_mu mu- e- e+    ' ! TauDecayModeBBB =  8
        IF(II.EQ. 7) NAMES(I-NLT)='  TAU-  --> K- 3PI0            ' ! TauDecayModeBBB =  9 ! explicitly included
        IF(II.EQ. 8) NAMES(I-NLT)='  TAU-  --> 2PI0 ETA K-        ' ! TauDecayModeBBB = 10
        IF(II.EQ. 9) NAMES(I-NLT)='  TAU-  --> 2PI0 K0B PI-       ' ! TauDecayModeBBB = 11 ! included after being split into K0S, K0L (TauDecayModeBBB = 30, 31)
        IF(II.EQ.10) NAMES(I-NLT)='  TAU-  --> PI0  K0  ETA PI-   ' ! TauDecayModeBBB = 12
        IF(II.EQ.11) NAMES(I-NLT)='  TAU-  --> PI0  PI- PI+ K-    ' ! TauDecayModeBBB = 13 ! explicitly included 
        IF(II.EQ.12) NAMES(I-NLT)='  TAU-  --> K0B  PI- PI+ PI-   ' ! TauDecayModeBBB = 14 ! included after being split into K0S, K0L (TauDecayModeBBB = 32, 33)
        IF(II.EQ.13) NAMES(I-NLT)='  TAU-  --> 2PI0 ETA PI-       ' ! TauDecayModeBBB = 15 ! included after being split into TauDecayModeBBB = 37-40
        IF(II.EQ.14) NAMES(I-NLT)='  TAU-  --> K0 K0B ETA PI-     ' ! TauDecayModeBBB = 16
        IF(II.EQ.15) NAMES(I-NLT)='  TAU-  --> K0 K0B PI0 PI-     ' ! TauDecayModeBBB = 17 ! included after being split into K0S, K0L (TauDecayModeBBB = 34, 35, 36)
        IF(II.EQ.16) NAMES(I-NLT)='  TAU-  --> K0 K0B K0  PI-     ' ! TauDecayModeBBB = 18
        IF(II.EQ.17) NAMES(I-NLT)='  TAU-  --> K0 PI0 PI0 K-      ' ! TauDecayModeBBB = 19
        IF(II.EQ.18) NAMES(I-NLT)='  TAU-  --> K0 K0B PI0 K-      ' ! TauDecayModeBBB = 20
        IF(II.EQ.19) NAMES(I-NLT)='  TAU-  --> PI0  K0  ETA K-    ' ! TauDecayModeBBB = 21
        IF(II.EQ.20) NAMES(I-NLT)='  TAU-  --> PI-PI+PI-  ETA     ' ! TauDecayModeBBB = 22 ! included after being split into TauDecayModeBBB = 41-44
        IF(II.EQ.21) NAMES(I-NLT)='  TAU-  --> PI-K+ K-   PI0     ' ! TauDecayModeBBB = 23 ! explicitly included
        IF(II.EQ.22) NAMES(I-NLT)='  TAU-  --> K- K+ K-   PI0     ' ! TauDecayModeBBB = 24
        IF(II.EQ.23) NAMES(I-NLT)='  TAU-  --> K- K+ K-   K0      ' ! TauDecayModeBBB = 25
        IF(II.EQ.24) NAMES(I-NLT)='  TAU-  --> K- PI+PI-  K0      ' ! TauDecayModeBBB = 26
        IF(II.EQ.25) NAMES(I-NLT)='  TAU-  --> K- K+ PI-  K0      ' ! TauDecayModeBBB = 27
        IF(II.EQ.26) NAMES(I-NLT)='  TAU-  --> PI-PI+PI-  OMEGA   ' ! TauDecayModeBBB = 28 ! included after being split into TauDecayModeBBB= 45-47
        IF(II.EQ.27) NAMES(I-NLT)='  TAU-  --> PI-PI0PI0  OMEGA   ' ! TauDecayModeBBB = 29 ! included after being split into TauDecayModeBBB= 48-50
        IF(II.EQ.28) NAMES(I-NLT)='  TAU-  --> 2PI0 K0S PI-       ' ! TauDecayModeBBB = 30 ! explicitly included 
        IF(II.EQ.29) NAMES(I-NLT)='  TAU-  --> 2PI0 K0L PI-       ' ! TauDecayModeBBB = 31 ! explicitly included 
        IF(II.EQ.30) NAMES(I-NLT)='  TAU-  --> K0S  PI- PI+ PI-   ' ! TauDecayModeBBB = 32 ! explicitly included 
        IF(II.EQ.31) NAMES(I-NLT)='  TAU-  --> K0L  PI- PI+ PI-   ' ! TauDecayModeBBB = 33 ! explicitly included 
        IF(II.EQ.32) NAMES(I-NLT)='  TAU-  --> K0S K0S PI0 PI-    ' ! TauDecayModeBBB = 34 ! explicitly included 
        IF(II.EQ.33) NAMES(I-NLT)='  TAU-  --> K0L K0L PI0 PI-    ' ! TauDecayModeBBB = 35 ! explicitly included 
        IF(II.EQ.34) NAMES(I-NLT)='  TAU-  --> K0S K0L PI0 PI-    ' ! TauDecayModeBBB = 36 ! explicitly included 
        IF(II.EQ.35) NAMES(I-NLT)='  TAU-  --> PI-2PI0ETA->GamGam ' ! TauDecayModeBBB = 37 ! explicitly included 
        IF(II.EQ.36) NAMES(I-NLT)='  TAU-  --> PI-2PI0ETA->P0P0P0 ' ! TauDecayModeBBB = 38 ! explicitly included 
        IF(II.EQ.37) NAMES(I-NLT)='  TAU-  --> PI-2PI0ETA->P+P-P0 ' ! TauDecayModeBBB = 39 ! explicitly included 
        IF(II.EQ.38) NAMES(I-NLT)='  TAU-  --> PI-2PI0ETA->P+P-Gam' ! TauDecayModeBBB = 40 ! explicitly included 
        IF(II.EQ.39) NAMES(I-NLT)='  TAU-  --> 2PI-PI+ETA->GamGam ' ! TauDecayModeBBB = 41 ! explicitly included 
        IF(II.EQ.40) NAMES(I-NLT)='  TAU-  --> 2PI-PI+ETA->P0P0P0 ' ! TauDecayModeBBB = 42 ! explicitly included 
        IF(II.EQ.41) NAMES(I-NLT)='  TAU-  --> 2PI-PI+ETA->P+P-P0 ' ! TauDecayModeBBB = 43 ! explicitly included 
        IF(II.EQ.42) NAMES(I-NLT)='  TAU-  --> 2PI-PI+ETA->P+P-Gam' ! TauDecayModeBBB = 44 ! explicitly included 
        IF(II.EQ.43) NAMES(I-NLT)='  TAU-  --> 2PI-PI+OM(PI+PI-P0)' ! TauDecayModeBBB = 45 ! explicitly included
        IF(II.EQ.44) NAMES(I-NLT)='  TAU-  --> 2PI-PI+OM(PI+PI-)  ' ! TauDecayModeBBB = 46 ! explicitly included
        IF(II.EQ.45) NAMES(I-NLT)='  TAU-  --> 2PI-PI+OM(PI0Gam)  ' ! TauDecayModeBBB = 47 ! explicitly included
        IF(II.EQ.46) NAMES(I-NLT)='  TAU-  --> PI-2PI0OM(PI+PI-P0)' ! TauDecayModeBBB = 48 ! explicitly included
        IF(II.EQ.47) NAMES(I-NLT)='  TAU-  --> PI-2PI0OM(PI+PI-)  ' ! TauDecayModeBBB = 49 ! explicitly included
        IF(II.EQ.48) NAMES(I-NLT)='  TAU-  --> PI-2PI0OM(PI0Gam)  ' ! TauDecayModeBBB = 50 ! explicitly included
        IF(II.EQ.49) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 51
        IF(II.EQ.50) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 52
        IF(II.EQ.51) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 53
        IF(II.EQ.52) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 54
        IF(II.EQ.53) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 55
        IF(II.EQ.54) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 56
        IF(II.EQ.55) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 57
        IF(II.EQ.56) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 58
        IF(II.EQ.57) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 59
        IF(II.EQ.58) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 60
        IF(II.EQ.59) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 61
        IF(II.EQ.60) NAMES(I-NLT)='  TAU-  --> xxxxxxx4xxxxxxxx   ' ! TauDecayModeBBB = 62

        MULPIK(I-NLT)=NPIK(I-NLT)
        DO J=1,JMAX
         IDFFIN(J,I-NLT)=NOPIK4(J,I-NLT)
        ENDDO

      ENDDO


C=================================================
C 5-scalar CHANNELS 
C=================================================

      DO II = 1,NM5
          I = II+NLT+NM4  ! position on the whole list of decay channels

        KEY5(II)=KEYstrt5(II)

        JLIST(I) = I
        NPIK(I-NLT)=5

        IF(II.EQ. 1) NAMES(I-NLT)='  TAU-  --> 2PI-, PI+, 2PI0 old' ! TauDecayModeBBB = 63
        IF(II.EQ. 2) NAMES(I-NLT)='  TAU-  --> a1 --> rho omega   ' ! TauDecayModeBBB = 64 ! implicitly included in TauDecayModeBBB = 3 (om->pi+pi-, rho om -> 2pi-pi+pi0) and 66 (om->pi+pi-pi0, rho om -> 2pi-pi+2pi0) and 111 (om->pi0gamma, rho om -> pi- pi0 pi0 )
        IF(II.EQ. 3) NAMES(I-NLT)='  TAU-  --> benchmark curr     ' ! TauDecayModeBBB = 65 
        IF(II.EQ. 4) NAMES(I-NLT)='  TAU-  --> 2PI- PI+ 2PI0 app08' ! TauDecayModeBBB = 66 ! explicitly included
        IF(II.EQ. 5) NAMES(I-NLT)='  TAU-  --> PI- 4PI0  app08    ' ! TauDecayModeBBB = 67 ! explicitly included
        IF(II.EQ. 6) NAMES(I-NLT)='  TAU-  --> 3PI- 2PI+ app08    ' ! TauDecayModeBBB = 68 ! explicitly included
        IF(II.EQ. 7) NAMES(I-NLT)='  TAU-  --> 2PI- 2PI+  K-      ' ! TauDecayModeBBB = 69 ! explicitly included
        IF(II.EQ. 8) NAMES(I-NLT)='  TAU-  --> 2PI- PI+ PI0 K0    ' ! TauDecayModeBBB = 70
        IF(II.EQ. 9) NAMES(I-NLT)='  TAU-  --> PI- 4PI0        old' ! TauDecayModeBBB = 71
        IF(II.EQ.10) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 72
        IF(II.EQ.11) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 73
        IF(II.EQ.12) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 74
        IF(II.EQ.13) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 75
        IF(II.EQ.14) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 76
        IF(II.EQ.15) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 77
        IF(II.EQ.16) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 78
        IF(II.EQ.17) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 79
        IF(II.EQ.18) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 80
        IF(II.EQ.19) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 81
        IF(II.EQ.20) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx5xxxxxx   ' ! TauDecayModeBBB = 82

        MULPIK(I-NLT)=NPIK(I-NLT)
        DO J=1,JMAX
         IDFFIN(J,I-NLT)=NOPIK5(J,I-NLT-NM4)
        ENDDO

      ENDDO


C=================================================
C multiple-scalar CHANNELS 
C=================================================

      DO II = 1,NM6
          I = II+NLT+NM4+NM5  ! position on the whole list of decay channels

        KEY6(II)=KEYstrt6(II)

        JLIST(I) = I
        NPIK(I-NLT)=NPIK6(I-NLT-NM4-NM5)

        IF(II.EQ. 1) NAMES(I-NLT)='  TAU-  --> 3PI-, 2PI+ old     ' ! TauDecayModeBBB = 83
        IF(II.EQ. 2) NAMES(I-NLT)='  TAU-  --> 3PI-, 2PI+,  PI0   ' ! TauDecayModeBBB = 84 ! explicitly included
        IF(II.EQ. 3) NAMES(I-NLT)='  TAU-  --> 2PI-,  PI+, 3PI0   ' ! TauDecayModeBBB = 85 ! explicitly included
        IF(II.EQ. 4) NAMES(I-NLT)='  TAU-  --> 3pi- 2pi+ 2pi0     ' ! TauDecayModeBBB = 86
        IF(II.EQ. 5) NAMES(I-NLT)='  TAU-  --> 4PI- 3PI+          ' ! TauDecayModeBBB = 87
        IF(II.EQ. 6) NAMES(I-NLT)='  TAU-  --> 4PI- 3PI+  PI0     ' ! TauDecayModeBBB = 88
        IF(II.EQ. 7) NAMES(I-NLT)='  TAU-  --> 2PI- 2PI+ K- PI0   ' ! TauDecayModeBBB = 89 ! explicitly included
        IF(II.EQ. 8) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 90
        IF(II.EQ. 9) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 91
        IF(II.EQ.10) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 92
        IF(II.EQ.11) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 93
        IF(II.EQ.12) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 94
        IF(II.EQ.13) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 95
        IF(II.EQ.14) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 96
        IF(II.EQ.15) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 97
        IF(II.EQ.16) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 98
        IF(II.EQ.17) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 99
        IF(II.EQ.18) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 100
        IF(II.EQ.19) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 101
        IF(II.EQ.20) NAMES(I-NLT)='  TAU-  --> xxxxxxxxxnxxxxxx   ' ! TauDecayModeBBB = 102

        MULPIK(I-NLT)=NPIK(I-NLT)
        DO J=1,JMAX
         IDFFIN(J,I-NLT)=NOPIK6(J,I-NLT-NM4-NM5)
        ENDDO

      ENDDO


C=================================================
C 3-scalar CHANNELS 
C=================================================


      DO II = 1,NM3
          I = II+NLT+NM4+NM5+NM6  ! position on the whole list of decay channels

        KEY3(II)=KEYstrt3(II)

        JLIST(I) = I
        NPIK(I-NLT)=3

        IF(II.EQ. 1) NAMES(I-NLT)='  TAU-  -->  K-, PI-,  K+      ' ! TauDecayModeBBB = 103 ! explicitly included 
        IF(II.EQ. 2) NAMES(I-NLT)='  TAU-  -->  K0, PI-, K0B      ' ! TauDecayModeBBB = 104 ! included after being split into K0S, K0L (TauDecayModeBBB = 121, 122, 123) 
        IF(II.EQ. 3) NAMES(I-NLT)='  TAU-  -->  K-,  PI0, K0      ' ! TauDecayModeBBB = 105 ! included after being split into K0S, K0L (TauDecayModeBBB = 124, 125)
        IF(II.EQ. 4) NAMES(I-NLT)='  TAU-  --> PI0  PI0   K-      ' ! TauDecayModeBBB = 106 ! explicitly included
        IF(II.EQ. 5) NAMES(I-NLT)='  TAU-  -->  K-  PI-  PI+      ' ! TauDecayModeBBB = 107 ! explicitly included
        IF(II.EQ. 6) NAMES(I-NLT)='  TAU-  --> PI-  K0B  PI0      ' ! TauDecayModeBBB = 108 ! included after being split into K0S, K0L (TauDecayModeBBB = 126, 127)
        IF(II.EQ. 7) NAMES(I-NLT)='  TAU-  --> ETA  PI-  PI0      ' ! TauDecayModeBBB = 109 ! included after being split into TauDecayModeBBB = 133-136
        IF(II.EQ. 8) NAMES(I-NLT)='  TAU-  --> PI-  PI0  GAM      ' ! TauDecayModeBBB = 110 ! explicitly included
        IF(II.EQ. 9) NAMES(I-NLT)='  TAU-  --> PI0  PI0  PI-      ' ! TauDecayModeBBB = 111 ! explicitly included
        IF(II.EQ.10) NAMES(I-NLT)='  TAU-  --> PI-  PI-  PI+      ' ! TauDecayModeBBB = 112 ! explicitly included
        IF(II.EQ.11) NAMES(I-NLT)='  TAU-  --> K-    K-   K+      ' ! TauDecayModeBBB = 113 ! (PS) this is not used: it comes from K-PHI instead: see TauDecayModeBBB = 196 
        IF(II.EQ.12) NAMES(I-NLT)='  TAU-  --> K-    K0   K0      ' ! TauDecayModeBBB = 114 ! (PS) this is not used: it comes from K-PHI instead: see TauDecayModeBBB = 196
        IF(II.EQ.13) NAMES(I-NLT)='  TAU-  --> K-   ETA  PI0      ' ! TauDecayModeBBB = 115 ! included after being split into TauDecayModeBBB = 137-140
        IF(II.EQ.14) NAMES(I-NLT)='  TAU-  --> K0B  ETA  PI-      ' ! TauDecayModeBBB = 116 ! included after being split into K0S, K0L (TauDecayModeBBB = 128->141-144, 129->145-148)
        IF(II.EQ.15) NAMES(I-NLT)='  TAU-  --> K-   K0   RHO0     ' ! TauDecayModeBBB = 117 ! (PS=PhaseSpace)
        IF(II.EQ.16) NAMES(I-NLT)='  TAU-  --> PI-  PHI  PI0      ' ! TauDecayModeBBB = 118 ! (PS=PhaseSpace)
        IF(II.EQ.17) NAMES(I-NLT)='  TAU-  --> K-   PHI  PI0      ' ! TauDecayModeBBB = 119 ! (PS=PhaseSpace)
        IF(II.EQ.18) NAMES(I-NLT)='  TAU-  --> K0   ETA  K-       ' ! TauDecayModeBBB = 120 ! (PS=PhaseSpace)
        IF(II.EQ.19) NAMES(I-NLT)='  TAU-  -->  K0S, PI-, K0S     ' ! TauDecayModeBBB = 121 ! (ME=2) explicitly included
        IF(II.EQ.20) NAMES(I-NLT)='  TAU-  -->  K0L, PI-, K0L     ' ! TauDecayModeBBB = 122 ! (ME=2) explicitly included
        IF(II.EQ.21) NAMES(I-NLT)='  TAU-  -->  K0S, PI-, K0L     ' ! TauDecayModeBBB = 123 ! (ME=2) explicitly included
        IF(II.EQ.22) NAMES(I-NLT)='  TAU-  -->  K-,  PI0, K0S     ' ! TauDecayModeBBB = 124 ! (ME=3) explicitly included
        IF(II.EQ.23) NAMES(I-NLT)='  TAU-  -->  K-,  PI0, K0L     ' ! TauDecayModeBBB = 125 ! (ME=3) explicitly included
        IF(II.EQ.24) NAMES(I-NLT)='  TAU-  --> PI-  K0S  PI0      ' ! TauDecayModeBBB = 126 ! (ME=6) explicitly included 
        IF(II.EQ.25) NAMES(I-NLT)='  TAU-  --> PI-  K0L  PI0      ' ! TauDecayModeBBB = 127 ! (ME=6) explicitly included 
        IF(II.EQ.26) NAMES(I-NLT)='  TAU-  --> K0S  ETA  PI-      ' ! TauDecayModeBBB = 128 ! included after being split into TauDecayModeBBB = 141-144
        IF(II.EQ.27) NAMES(I-NLT)='  TAU-  --> K0L  ETA  PI-      ' ! TauDecayModeBBB = 129 ! included after being split into TauDecayModeBBB = 145-148
        IF(II.EQ.28) NAMES(I-NLT)='  TAU-  --> Pi-PizOm->Pi+Pi-Piz' ! TauDecayModeBBB = 130 ! (PS) included in TauDecayModeBBB=66
        IF(II.EQ.29) NAMES(I-NLT)='  TAU-  --> Pi-PizOm->Pi+Pi-   ' ! TauDecayModeBBB = 131 ! (PS) explicitly included
        IF(II.EQ.30) NAMES(I-NLT)='  TAU-  --> Pi-PizOm->PizGamma ' ! TauDecayModeBBB = 132 ! (PS) explicitly included
        IF(II.EQ.31) NAMES(I-NLT)='  TAU-  --> Pi-PizETA->Gamgam  ' ! TauDecayModeBBB = 133 ! (ME=7)  explicitly included 
        IF(II.EQ.32) NAMES(I-NLT)='  TAU-  --> Pi-PizETA->PzPzPz  ' ! TauDecayModeBBB = 134 ! (ME=7)  explicitly included 
        IF(II.EQ.33) NAMES(I-NLT)='  TAU-  --> Pi-PizETA->Pi+Pi-Pz' ! TauDecayModeBBB = 135 ! (ME=7)  explicitly included 
        IF(II.EQ.34) NAMES(I-NLT)='  TAU-  --> Pi-PizETA->Pi+Pi-Ga' ! TauDecayModeBBB = 136 ! (ME=7)  explicitly included 
        IF(II.EQ.35) NAMES(I-NLT)='  TAU-  --> K-PI0 ETA->Gamgam  ' ! TauDecayModeBBB = 137 ! (ME=13) explicitly included 
        IF(II.EQ.36) NAMES(I-NLT)='  TAU-  --> K-PI0 ETA->P0P0P0  ' ! TauDecayModeBBB = 138 ! (ME=13) explicitly included
        IF(II.EQ.37) NAMES(I-NLT)='  TAU-  --> K-PI0 ETA->P+P-P0  ' ! TauDecayModeBBB = 139 ! (ME=13) explicitly included
        IF(II.EQ.38) NAMES(I-NLT)='  TAU-  --> K-PI0 ETA->P+P-Gam ' ! TauDecayModeBBB = 140 ! (ME=13) explicitly included
        IF(II.EQ.39) NAMES(I-NLT)='  TAU-  --> PI-K0S ETA->Gamgam ' ! TauDecayModeBBB = 141 ! (ME=14) explicitly included
        IF(II.EQ.40) NAMES(I-NLT)='  TAU-  --> PI-K0S ETA->P0P0P0 ' ! TauDecayModeBBB = 142 ! (ME=14) explicitly included
        IF(II.EQ.41) NAMES(I-NLT)='  TAU-  --> PI-K0S ETA->P+P-P0 ' ! TauDecayModeBBB = 143 ! (ME=14) explicitly included
        IF(II.EQ.42) NAMES(I-NLT)='  TAU-  --> PI-K0S ETA->P+P-Gam' ! TauDecayModeBBB = 144 ! (ME=14) explicitly included
        IF(II.EQ.43) NAMES(I-NLT)='  TAU-  --> PI-K0L ETA->Gamgam ' ! TauDecayModeBBB = 145 ! (ME=14) explicitly included
        IF(II.EQ.44) NAMES(I-NLT)='  TAU-  --> PI-K0L ETA->P0P0P0 ' ! TauDecayModeBBB = 146 ! (ME=14) explicitly included
        IF(II.EQ.45) NAMES(I-NLT)='  TAU-  --> PI-K0L ETA->P+P-P0 ' ! TauDecayModeBBB = 147 ! (ME=14) explicitly included
        IF(II.EQ.46) NAMES(I-NLT)='  TAU-  --> PI-K0L ETA->P+P-Gam' ! TauDecayModeBBB = 148 ! (ME=14) explicitly included
        IF(II.EQ.47) NAMES(I-NLT)='  TAU-  --> PI- E+ E-          ' ! TauDecayModeBBB = 149
        IF(II.EQ.48) NAMES(I-NLT)='  TAU-  --> PI- MU+MU-         ' ! TauDecayModeBBB = 150
        IF(II.EQ.49) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 151
        IF(II.EQ.50) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 152
        IF(II.EQ.51) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 153
        IF(II.EQ.52) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 154
        IF(II.EQ.53) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 155
        IF(II.EQ.54) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 156
        IF(II.EQ.55) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 157
        IF(II.EQ.56) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 158
        IF(II.EQ.57) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 159
        IF(II.EQ.58) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 160
        IF(II.EQ.59) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 161
        IF(II.EQ.60) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx3xxxxxx   ' ! TauDecayModeBBB = 162

        MULPIK(I-NLT)=NPIK(I-NLT)
        DO J=1,JMAX
         IDFFIN(J,I-NLT)=NOPIK3(J,I-NLT-NM4-NM5-NM6)
        ENDDO
      
      ENDDO


C=================================================
C 2-scalar or anomalous CHANNELS 
C=================================================
 
      DO II = 1,NM2
          I = II+NLT+NM4+NM5+NM6+NM3  ! position on the whole list of decay channels

        KEY2(II)=KEYstrt2(II)

        JLIST(I) = I
        NPIK(I-NLT)=2

        IF(II.EQ. 1) NAMES(I-NLT)='  TAU-  -->  PI- PI0           ' ! TauDecayModeBBB = 163 ! explicitly included
        IF(II.EQ. 2) NAMES(I-NLT)='  TAU-  -->  PI- K0B           ' ! TauDecayModeBBB = 164 ! included after being split into K0S, K0L (TauDecayModeBBB = 226, 227)
        IF(II.EQ. 3) NAMES(I-NLT)='  TAU-  -->  K-  PI0           ' ! TauDecayModeBBB = 165 ! explicitly included
        IF(II.EQ. 4) NAMES(I-NLT)='  TAU-  -->  K-  K0            ' ! TauDecayModeBBB = 166 ! included after being split into K0S, K0L (TauDecayModeBBB = 228, 229)
        IF(II.EQ. 5) NAMES(I-NLT)='  TAU-  -->  mu-mu-mu+ !nu_tau ' ! TauDecayModeBBB = 167 
        IF(II.EQ. 6) NAMES(I-NLT)='  TAU-  --> mu- mu- e+ !nu_tau ' ! TauDecayModeBBB = 168 
        IF(II.EQ. 7) NAMES(I-NLT)='  TAU-  --> mu- e- mu+ !nu_tau ' ! TauDecayModeBBB = 169 
        IF(II.EQ. 8) NAMES(I-NLT)='  TAU-  --> mu- e- e+  !nu_tau ' ! TauDecayModeBBB = 170 
        IF(II.EQ. 9) NAMES(I-NLT)='  TAU-  --> mu+ e- e-  !nu_tau ' ! TauDecayModeBBB = 171 
        IF(II.EQ.10) NAMES(I-NLT)='  TAU-  --> e- e- e+   !nu_tau ' ! TauDecayModeBBB = 172 
        IF(II.EQ.11) NAMES(I-NLT)='  TAU-  --> e-pi+pi-  !nu_tau  ' ! TauDecayModeBBB = 173 
        IF(II.EQ.12) NAMES(I-NLT)='  TAU-  --> mu-pi+pi-  !nu_tau ' ! TauDecayModeBBB = 174 
        IF(II.EQ.13) NAMES(I-NLT)='  TAU-  --> e-pi+K-  !nu_tau   ' ! TauDecayModeBBB = 175
        IF(II.EQ.14) NAMES(I-NLT)='  TAU-  --> mu-pi+K-  !nu_tau  ' ! TauDecayModeBBB = 176 
        IF(II.EQ.15) NAMES(I-NLT)='  TAU-  --> e-pi-K+  !nu_tau   ' ! TauDecayModeBBB = 177 
        IF(II.EQ.16) NAMES(I-NLT)='  TAU-  --> mu-pi-K+  !nu_tau  ' ! TauDecayModeBBB = 178 
        IF(II.EQ.17) NAMES(I-NLT)='  TAU-  --> e-K-K+  !nu_tau    ' ! TauDecayModeBBB = 179 
        IF(II.EQ.18) NAMES(I-NLT)='  TAU-  --> mu-K-K+  !nu_tau   ' ! TauDecayModeBBB = 180 
        IF(II.EQ.19) NAMES(I-NLT)='  TAU-  --> e-K0K0  !nu_tau    ' ! TauDecayModeBBB = 181 
        IF(II.EQ.20) NAMES(I-NLT)='  TAU-  --> mu-K0K0  !nu_tau   ' ! TauDecayModeBBB = 182 
        IF(II.EQ.21) NAMES(I-NLT)='  TAU-  --> e+pi-pi-  !nu_tau  ' ! TauDecayModeBBB = 183 
        IF(II.EQ.22) NAMES(I-NLT)='  TAU-  --> mu+pi-pi-  !nu_tau ' ! TauDecayModeBBB = 184 
        IF(II.EQ.23) NAMES(I-NLT)='  TAU-  --> e+pi-K-  !nu_tau   ' ! TauDecayModeBBB = 185 
        IF(II.EQ.24) NAMES(I-NLT)='  TAU-  --> mu+pi-K-  !nu_tau  ' ! TauDecayModeBBB = 186 
        IF(II.EQ.25) NAMES(I-NLT)='  TAU-  --> e+K-K-  !nu_tau    ' ! TauDecayModeBBB = 187 
        IF(II.EQ.26) NAMES(I-NLT)='  TAU-  --> mu+K-K-  !nu_tau   ' ! TauDecayModeBBB = 188 
        IF(II.EQ.27) NAMES(I-NLT)='  TAU-  --> mu-mu- p+  !nu_tau ' ! TauDecayModeBBB = 189 
        IF(II.EQ.28) NAMES(I-NLT)='  TAU-  --> mu-mu+ p-  !nu_tau ' ! TauDecayModeBBB = 190 
        IF(II.EQ.29) NAMES(I-NLT)='  TAU-  -->  e - e- p+ !nu_tau ' ! TauDecayModeBBB = 191 
        IF(II.EQ.30) NAMES(I-NLT)='  TAU-  -->  e - e+ p- !nu_tau ' ! TauDecayModeBBB = 192 
        IF(II.EQ.31) NAMES(I-NLT)='  TAU-  --> eta k-             ' ! TauDecayModeBBB = 193 ! included after being split into TauDecayModeBBB = 242-245
        IF(II.EQ.32) NAMES(I-NLT)='  TAU-  --> eta pi-            ' ! TauDecayModeBBB = 194 ! split into TauDecayModeBBB = 246-249
        IF(II.EQ.33) NAMES(I-NLT)='  TAU-  --> PI-  PHI           ' ! TauDecayModeBBB = 195 
        IF(II.EQ.34) NAMES(I-NLT)='  TAU-  -->  K-  PHI           ' ! TauDecayModeBBB = 196 ! included after being split into K+K-, K0SK0L (TauDecayModeBBB = 230, 231)
        IF(II.EQ.35) NAMES(I-NLT)='  TAU-  -->  PI- OMEGA         ' ! TauDecayModeBBB = 197 ! in TauDecayModeBBB = 237 (om->pi+pi-) and 3 (om->pi+pi-pi0) and 110 (om->pi0gamma)
        IF(II.EQ.36) NAMES(I-NLT)='  TAU-  -->  K-  OMEGA         ' ! TauDecayModeBBB = 198 ! included after being split into TauDecayModeBBB = 250-252
        IF(II.EQ.37) NAMES(I-NLT)='  TAU-  -->  PI- ETAprm        ' ! TauDecayModeBBB = 199 
        IF(II.EQ.38) NAMES(I-NLT)='  TAU-  -->  K-  ETAprm        ' ! TauDecayModeBBB = 200 
        IF(II.EQ.39) NAMES(I-NLT)='  TAU-  -->  e- mu+ p- !nu_tau ' ! TauDecayModeBBB = 201                 
        IF(II.EQ.40) NAMES(I-NLT)='  TAU-  -->  e+ mu- p- !nu_tau ' ! TauDecayModeBBB = 202                
        IF(II.EQ.41) NAMES(I-NLT)='  TAU-  -->  e- mu- p+ !nu_tau ' ! TauDecayModeBBB = 203                       
        IF(II.EQ.42) NAMES(I-NLT)='  TAU-  --> e- PI0 PI0  !nu_tau' ! TauDecayModeBBB = 204                     
        IF(II.EQ.43) NAMES(I-NLT)='  TAU-  --> mu- PI0 PI0 !nu_tau' ! TauDecayModeBBB = 205                       
        IF(II.EQ.44) NAMES(I-NLT)='  TAU-  --> e- PI0 eta !nu_tau ' ! TauDecayModeBBB = 206      
        IF(II.EQ.45) NAMES(I-NLT)='  TAU-  --> mu- PI0 eta !nu_tau' ! TauDecayModeBBB = 207 
        IF(II.EQ.46) NAMES(I-NLT)='  TAU-  -->  e- PI0 eta_p !nu_t' ! TauDecayModeBBB = 208 
        IF(II.EQ.47) NAMES(I-NLT)='  TAU-  --> mu- PI0 eta_p !nu_t' ! TauDecayModeBBB = 209 
        IF(II.EQ.48) NAMES(I-NLT)='  TAU-  --> e- eta eta  !nu_tau' ! TauDecayModeBBB = 210 
        IF(II.EQ.49) NAMES(I-NLT)='  TAU-  --> mu- eta eta !nu_tau' ! TauDecayModeBBB = 211 
        IF(II.EQ.50) NAMES(I-NLT)='  TAU-  --> e- eta eta_p !nu_t ' ! TauDecayModeBBB = 212 
        IF(II.EQ.51) NAMES(I-NLT)='  TAU-  --> mu- eta eta_p !nu_t' ! TauDecayModeBBB = 213 
        IF(II.EQ.52) NAMES(I-NLT)='  TAU-  --> e- PI0 Ks  !nu_tau ' ! TauDecayModeBBB = 214 
        IF(II.EQ.53) NAMES(I-NLT)='  TAU-  --> mu- PI0 Ks !nu_tau ' ! TauDecayModeBBB = 215 
        IF(II.EQ.54) NAMES(I-NLT)='  TAU-  --> e- eta  Ks !nu_tau ' ! TauDecayModeBBB = 216 
        IF(II.EQ.55) NAMES(I-NLT)='  TAU-  --> mu- eta Ks !nu_tau ' ! TauDecayModeBBB = 217 
        IF(II.EQ.56) NAMES(I-NLT)='  TAU-  --> e- eta_p Ks !nu_tau' ! TauDecayModeBBB = 218 
        IF(II.EQ.57) NAMES(I-NLT)='  TAU-  --> mu- eta_p Ks !nu_t ' ! TauDecayModeBBB = 219 
        IF(II.EQ.58) NAMES(I-NLT)='  TAU-  --> p- pi+ K- !nu_tau  ' ! TauDecayModeBBB = 220 
        IF(II.EQ.59) NAMES(I-NLT)='  TAU-  --> p+ pi- K-  !nu_tau ' ! TauDecayModeBBB = 221 
        IF(II.EQ.60) NAMES(I-NLT)='  TAU-  --> p- K+ pi- !nu_tau  ' ! TauDecayModeBBB = 222 
        IF(II.EQ.61) NAMES(I-NLT)='  TAU-  --> p- pi0 pi0 !nu_tau ' ! TauDecayModeBBB = 223 
        IF(II.EQ.62) NAMES(I-NLT)='  TAU-  --> p- pi0 eta !nu_tau ' ! TauDecayModeBBB = 224 
        IF(II.EQ.63) NAMES(I-NLT)='  TAU-  --> p- pi0 Ks !nu_tau  ' ! TauDecayModeBBB = 225 
        IF(II.EQ.64) NAMES(I-NLT)='  TAU-  -->  PI- K0S           ' ! TauDecayModeBBB = 226 ! explicitly included
        IF(II.EQ.65) NAMES(I-NLT)='  TAU-  -->  PI- K0L           ' ! TauDecayModeBBB = 227 ! explicitly included
        IF(II.EQ.66) NAMES(I-NLT)='  TAU-  -->  K-  K0S           ' ! TauDecayModeBBB = 228 ! explicitly included 
        IF(II.EQ.67) NAMES(I-NLT)='  TAU-  -->  K-  K0L           ' ! TauDecayModeBBB = 229 ! explicitly included 
        IF(II.EQ.68) NAMES(I-NLT)='  TAU-  -->  K-  PHI (K+K-)    ' ! TauDecayModeBBB = 230 ! explicitly included
        IF(II.EQ.69) NAMES(I-NLT)='  TAU-  -->  K-  PHI (K0S,K0L) ' ! TauDecayModeBBB = 231 ! explicitly included
        IF(II.EQ.70) NAMES(I-NLT)='  TAU-  -->  K-  PHI(PI+PI-PI0)' ! TauDecayModeBBB = 232 
        IF(II.EQ.71) NAMES(I-NLT)='  TAU-  --> pi- f1 (2pi-2pi+)  ' ! TauDecayModeBBB = 233 ! explicitly included 
        IF(II.EQ.72) NAMES(I-NLT)='  TAU-  --> pi- f1 (pi-pi+eta) ' ! TauDecayModeBBB = 234 ! split into TauDecayModeBBB=253-256
        IF(II.EQ.73) NAMES(I-NLT)='  TAU-  --> pi- f1 (pi0pi0eta) ' ! TauDecayModeBBB = 235 ! split into TauDecayModeBBB=257-260
        IF(II.EQ.74) NAMES(I-NLT)='  TAU-  -->  PI- OM(Pi+Pi-Pi0) ' ! TauDecayModeBBB = 236 ! included in TauDecayModeBBB=3
        IF(II.EQ.75) NAMES(I-NLT)='  TAU-  -->  PI- OM(Pi+Pi-)    ' ! TauDecayModeBBB = 237 ! explicitly included 
        IF(II.EQ.76) NAMES(I-NLT)='  TAU-  -->  PI- OM(Pi0Gamma)  ' ! TauDecayModeBBB = 238 ! included in TauDecayModeBBB=110
        IF(II.EQ.77) NAMES(I-NLT)='  TAU-  -->  RHO- OM(Pi+Pi-Pi0)' ! TauDecayModeBBB = 239 ! 
        IF(II.EQ.78) NAMES(I-NLT)='  TAU-  -->  RHO- OM(Pi+Pi-)   ' ! TauDecayModeBBB = 240 ! 
        IF(II.EQ.79) NAMES(I-NLT)='  TAU-  -->  RHO- OM(Pi0Gamma) ' ! TauDecayModeBBB = 241 ! 
        IF(II.EQ.80) NAMES(I-NLT)='  TAU-  --> K-eta(GamGam)      ' ! TauDecayModeBBB = 242 ! explicitly included
        IF(II.EQ.81) NAMES(I-NLT)='  TAU-  --> K-eta(Pi0Pi0Pi0)   ' ! TauDecayModeBBB = 243 ! explicitly included
        IF(II.EQ.82) NAMES(I-NLT)='  TAU-  --> K-eta(Pi+Pi-Pi0)   ' ! TauDecayModeBBB = 244 ! explicitly included
        IF(II.EQ.83) NAMES(I-NLT)='  TAU-  --> K-eta(Pi+Pi-Gam)   ' ! TauDecayModeBBB = 245 ! explicitly included
        IF(II.EQ.84) NAMES(I-NLT)='  TAU-  --> pi-eta(GamGam)     ' ! TauDecayModeBBB = 246
        IF(II.EQ.85) NAMES(I-NLT)='  TAU-  --> pi-eta(Pi0Pi0Pi0)  ' ! TauDecayModeBBB = 247
        IF(II.EQ.86) NAMES(I-NLT)='  TAU-  --> pi-eta(Pi+Pi-Pi0)  ' ! TauDecayModeBBB = 248
        IF(II.EQ.87) NAMES(I-NLT)='  TAU-  --> pi-eta(Pi+Pi-Gam)  ' ! TauDecayModeBBB = 249
        IF(II.EQ.88) NAMES(I-NLT)='  TAU-  --> K-OM(Pi+Pi-Pi0)    ' ! TauDecayModeBBB = 250 ! explicitly included
        IF(II.EQ.89) NAMES(I-NLT)='  TAU-  --> K-OM(Pi+Pi-)       ' ! TauDecayModeBBB = 251 ! explicitly included
        IF(II.EQ.90) NAMES(I-NLT)='  TAU-  --> K-OM(Pi0Gamma)     ' ! TauDecayModeBBB = 252 ! explicitly included
        IF(II.EQ.91) NAMES(I-NLT)='  TAU-  -->pi-f1(pi-pi+eta:GG) ' ! TauDecayModeBBB = 253 ! explicitly included
        IF(II.EQ.92) NAMES(I-NLT)='  TAU-  -->pi-f1(pi-pi+eta:3P0)' ! TauDecayModeBBB = 254 ! explicitly included
        IF(II.EQ.93) NAMES(I-NLT)='  TAU-  -->pi-f1(pi-pi+eta:PPP)' ! TauDecayModeBBB = 255 ! explicitly included
        IF(II.EQ.94) NAMES(I-NLT)='  TAU-  -->pi-f1(pi-pi+eta:PPG)' ! TauDecayModeBBB = 256 ! explicitly included
        IF(II.EQ.95) NAMES(I-NLT)='  TAU-  -->pi-f1(2pi0eta:GG)   ' ! TauDecayModeBBB = 257 
        IF(II.EQ.96) NAMES(I-NLT)='  TAU-  -->pi-f1(2pi0eta:3P0)  ' ! TauDecayModeBBB = 258 
        IF(II.EQ.97) NAMES(I-NLT)='  TAU-  -->pi-f1(2pi0eta:PPP)  ' ! TauDecayModeBBB = 259 
        IF(II.EQ.98) NAMES(I-NLT)='  TAU-  -->pi-f1(2pi0eta:PPG)  ' ! TauDecayModeBBB = 260 
        IF(II.EQ.99) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 261
        IF(II.EQ.100)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 262
        IF(II.EQ.101)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 263
        IF(II.EQ.102)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 264
        IF(II.EQ.103)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 265
        IF(II.EQ.104)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 266
        IF(II.EQ.105)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 267
        IF(II.EQ.106)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 268
        IF(II.EQ.107)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 269
        IF(II.EQ.108)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 270
        IF(II.EQ.109)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 271
        IF(II.EQ.110)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 272
        IF(II.EQ.111)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 273
        IF(II.EQ.112)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 274
        IF(II.EQ.113)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 275
        IF(II.EQ.114)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 276
        IF(II.EQ.115)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 277
        IF(II.EQ.116)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 278
        IF(II.EQ.117)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 279
        IF(II.EQ.118)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 280
        IF(II.EQ.119)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 281
        IF(II.EQ.120)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 282
        IF(II.EQ.121)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 283
        IF(II.EQ.122)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 284
        IF(II.EQ.123)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 285
        IF(II.EQ.124)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 286
        IF(II.EQ.125)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 287
        IF(II.EQ.126)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 288
        IF(II.EQ.127)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 289
        IF(II.EQ.128)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 290
        IF(II.EQ.129)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 291
        IF(II.EQ.130)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 292
        IF(II.EQ.131)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 293
        IF(II.EQ.132)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 294
        IF(II.EQ.133)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 295
        IF(II.EQ.134)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 296
        IF(II.EQ.135)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 297
        IF(II.EQ.136)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 298
        IF(II.EQ.137)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 299
        IF(II.EQ.138)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 300
        IF(II.EQ.139)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 301
        IF(II.EQ.140)NAMES(I-NLT)='  TAU-  --> xxxxxxxxx2xxxxxx   ' ! TauDecayModeBBB = 302

        MULPIK(I-NLT)=NPIK(I-NLT)
        DO J=1,JMAX
         IDFFIN(J,I-NLT)=NOPIK2(J,I-NLT-NM4-NM5-NM6-NM3)
        ENDDO

      ENDDO


C=================================================
C 1-scalar or anomalous CHANNELS  
C=================================================

      DO II = 1,NM1
          I = II+NLT+NM4+NM5+NM6+NM3+NM2  ! position on the whole list of decay channels

        KEY1(II)=KEYstrt1(II)

        JLIST(I) = I
        NPIK(I-NLT)=1

        IF(II.EQ. 1) NAMES(I-NLT)='  TAU-  --> PI-                ' ! TauDecayModeBBB = 303 ! explicitly included
        IF(II.EQ. 2) NAMES(I-NLT)='  TAU-  --> K-                 ' ! TauDecayModeBBB = 304 ! explicitly included
        IF(II.EQ. 3) NAMES(I-NLT)='  TAU-  --> gamma e-   !nu_tau ' ! TauDecayModeBBB = 305 
        IF(II.EQ. 4) NAMES(I-NLT)='  TAU-  --> gamma mu-  !nu_tau ' ! TauDecayModeBBB = 306 
        IF(II.EQ. 5) NAMES(I-NLT)='  TAU-  --> PI0 e-     !nu_tau ' ! TauDecayModeBBB = 307 
        IF(II.EQ. 6) NAMES(I-NLT)='  TAU-  --> PI0 mu-    !nu_tau ' ! TauDecayModeBBB = 308 
        IF(II.EQ. 7) NAMES(I-NLT)='  TAU-  --> eta e-     !nu_tau ' ! TauDecayModeBBB = 309 ! split into 350-353
        IF(II.EQ. 8) NAMES(I-NLT)='  TAU-  --> eta mu-    !nu_tau ' ! TauDecayModeBBB = 310 ! split into 354-357
        IF(II.EQ. 9) NAMES(I-NLT)='  TAU-  --> e-  K0L    !nu_tau ' ! TauDecayModeBBB = 311 
        IF(II.EQ.10) NAMES(I-NLT)='  TAU-  --> mu- K0L    !nu_tau ' ! TauDecayModeBBB = 312 
        IF(II.EQ.11) NAMES(I-NLT)='  TAU-  --> e-  omega  !nu_tau ' ! TauDecayModeBBB = 313 ! split into 358-360
        IF(II.EQ.12) NAMES(I-NLT)='  TAU-  --> mu- omega  !nu_tau ' ! TauDecayModeBBB = 314 ! split into 361-363
        IF(II.EQ.13) NAMES(I-NLT)='  TAU-  --> e-  phi    !nu_tau ' ! TauDecayModeBBB = 315 
        IF(II.EQ.14) NAMES(I-NLT)='  TAU-  --> mu- phi    !nu_tau ' ! TauDecayModeBBB = 316 
        IF(II.EQ.15) NAMES(I-NLT)='  TAU-  --> e-  rho0   !nu_tau ' ! TauDecayModeBBB = 317 
        IF(II.EQ.16) NAMES(I-NLT)='  TAU-  --> mu- rho0   !nu_tau ' ! TauDecayModeBBB = 318 
        IF(II.EQ.17) NAMES(I-NLT)='  TAU-  --> A0-                ' ! TauDecayModeBBB = 319 
        IF(II.EQ.18) NAMES(I-NLT)='  TAU-  --> B1-                ' ! TauDecayModeBBB = 320 
        IF(II.EQ.19) NAMES(I-NLT)='  TAU-  --> e-  K0S    !nu_tau ' ! TauDecayModeBBB = 321 
        IF(II.EQ.20) NAMES(I-NLT)='  TAU-  --> mu- K0S    !nu_tau ' ! TauDecayModeBBB = 322 
        IF(II.EQ.21) NAMES(I-NLT)='  TAU-  --> p gamma    !nu_tau ' ! TauDecayModeBBB = 323 
        IF(II.EQ.22) NAMES(I-NLT)='  TAU-  --> p pi0      !nu_tau ' ! TauDecayModeBBB = 324 
        IF(II.EQ.23) NAMES(I-NLT)='  TAU-  --> p eta      !nu_tau ' ! TauDecayModeBBB = 325 
        IF(II.EQ.24) NAMES(I-NLT)='  TAU-  --> p K0       !nu_tau ' ! TauDecayModeBBB = 326 
        IF(II.EQ.25) NAMES(I-NLT)='  TAU-  --> e- eta_p   !nu_tau ' ! TauDecayModeBBB = 327 
        IF(II.EQ.26) NAMES(I-NLT)='  TAU-  --> mu- eta_p  !nu_tau ' ! TauDecayModeBBB = 328 
        IF(II.EQ.27) NAMES(I-NLT)='  TAU-  --> pi- lambda !nu_tau ' ! TauDecayModeBBB = 329 
        IF(II.EQ.28) NAMES(I-NLT)='  TAU-  --> pi- lmb_br !nu_tau ' ! TauDecayModeBBB = 330 
        IF(II.EQ.29) NAMES(I-NLT)='  TAU-  --> K- lambda  !nu_tau ' ! TauDecayModeBBB = 331 
        IF(II.EQ.30) NAMES(I-NLT)='  TAU-  --> K- lmb_bar !nu_tau ' ! TauDecayModeBBB = 332 
        IF(II.EQ.31) NAMES(I-NLT)='  TAU-  --> e-  K*     !nu_tau ' ! TauDecayModeBBB = 333 
        IF(II.EQ.32) NAMES(I-NLT)='  TAU-  --> e-  K*_bar !nu_tau ' ! TauDecayModeBBB = 334 
        IF(II.EQ.33) NAMES(I-NLT)='  TAU-  --> mu- K*     !nu_tau ' ! TauDecayModeBBB = 335 
        IF(II.EQ.34) NAMES(I-NLT)='  TAU-  --> mu- K*_bar !nu_tau ' ! TauDecayModeBBB = 336 
        IF(II.EQ.35) NAMES(I-NLT)='  TAU-  --> e-  a0(980)!nu_tau ' ! TauDecayModeBBB = 337 ! split into 364-367
        IF(II.EQ.36) NAMES(I-NLT)='  TAU-  --> mu- a0(980)!nu_tau ' ! TauDecayModeBBB = 338 ! split into 368-371
        IF(II.EQ.37) NAMES(I-NLT)='  TAU-  --> e-  f0(980)!nu_tau ' ! TauDecayModeBBB = 339 ! f0(980)->pi+pi- 
        IF(II.EQ.38) NAMES(I-NLT)='  TAU-  --> mu- f0(980)!nu_tau ' ! TauDecayModeBBB = 340 ! f0(980)->pi+pi- 
        IF(II.EQ.39) NAMES(I-NLT)='  TAU-  --> a1-  ->pi- gamma   ' ! TauDecayModeBBB = 341 ! explicitly included 
        IF(II.EQ.40) NAMES(I-NLT)='  TAU-  --> rho- ->pi- gamma   ' ! TauDecayModeBBB = 342 ! explicitly included 
        IF(II.EQ.41) NAMES(I-NLT)='  TAU-  --> a0-:pi-eta:GamGam  ' ! TauDecayModeBBB = 343
        IF(II.EQ.42) NAMES(I-NLT)='  TAU-  --> a0-:pi-eta:PzPzPz  ' ! TauDecayModeBBB = 344
        IF(II.EQ.43) NAMES(I-NLT)='  TAU-  --> a0-:pi-eta:Pi+Pi-Pz' ! TauDecayModeBBB = 345
        IF(II.EQ.44) NAMES(I-NLT)='  TAU-  --> a0-:pi-eta:Pi+Pi-G ' ! TauDecayModeBBB = 346
        IF(II.EQ.45) NAMES(I-NLT)='  TAU-  --> b1-:pi-OM:Pi+Pi-Pi0' ! TauDecayModeBBB = 347 
        IF(II.EQ.46) NAMES(I-NLT)='  TAU-  --> b1-:pi-OM:Pi+Pi-   ' ! TauDecayModeBBB = 348 
        IF(II.EQ.47) NAMES(I-NLT)='  TAU-  --> b1-:pi-OM:Pi0Gamma ' ! TauDecayModeBBB = 349
        IF(II.EQ.48) NAMES(I-NLT)='  TAU-  --> e-  eta    !nu_tau ' ! TauDecayModeBBB = 350 ! eta->gammagamma
        IF(II.EQ.49) NAMES(I-NLT)='  TAU-  --> e-  eta    !nu_tau ' ! TauDecayModeBBB = 351 ! eta->pi0pi0pi0
        IF(II.EQ.50) NAMES(I-NLT)='  TAU-  --> e-  eta    !nu_tau ' ! TauDecayModeBBB = 352 ! eta->pi+pi-pi0
        IF(II.EQ.51) NAMES(I-NLT)='  TAU-  --> e-  eta    !nu_tau ' ! TauDecayModeBBB = 353 ! eta->pi+pi-gamma
        IF(II.EQ.52) NAMES(I-NLT)='  TAU-  --> mu- eta    !nu_tau ' ! TauDecayModeBBB = 354 ! eta->gammagamma
        IF(II.EQ.53) NAMES(I-NLT)='  TAU-  --> mu- eta    !nu_tau ' ! TauDecayModeBBB = 355 ! eta->pi0pi0pi0
        IF(II.EQ.54) NAMES(I-NLT)='  TAU-  --> mu- eta    !nu_tau ' ! TauDecayModeBBB = 356 ! eta->pi+pi-pi0
        IF(II.EQ.55) NAMES(I-NLT)='  TAU-  --> mu- eta    !nu_tau ' ! TauDecayModeBBB = 357 ! eta->pi+pi-gamma
        IF(II.EQ.56) NAMES(I-NLT)='  TAU-  --> e-  omega  !nu_tau ' ! TauDecayModeBBB = 358 ! omega->pi+pi-pi0 
        IF(II.EQ.57) NAMES(I-NLT)='  TAU-  --> e-  omega  !nu_tau ' ! TauDecayModeBBB = 359 ! omega->pi+pi- 
        IF(II.EQ.58) NAMES(I-NLT)='  TAU-  --> e-  omega  !nu_tau ' ! TauDecayModeBBB = 360 ! omega->pi0gamma 
        IF(II.EQ.59) NAMES(I-NLT)='  TAU-  --> mu- omega  !nu_tau ' ! TauDecayModeBBB = 361 ! omega->pi+pi-pi0 
        IF(II.EQ.60) NAMES(I-NLT)='  TAU-  --> mu- omega  !nu_tau ' ! TauDecayModeBBB = 362 ! omega->pi+pi- 
        IF(II.EQ.61) NAMES(I-NLT)='  TAU-  --> mu- omega  !nu_tau ' ! TauDecayModeBBB = 363 ! omega->pi0gamma 
        IF(II.EQ.62) NAMES(I-NLT)='  TAU-  --> e-  a0(980) !nu_tau' ! TauDecayModeBBB = 364 ! a0(980)->pi0eta(gammagamma) 
        IF(II.EQ.63) NAMES(I-NLT)='  TAU-  --> e-  a0(980) !nu_tau' ! TauDecayModeBBB = 365 ! a0(980)->pi0eta(pi0pi0pi0) 
        IF(II.EQ.64) NAMES(I-NLT)='  TAU-  --> e-  a0(980) !nu_tau' ! TauDecayModeBBB = 366 ! a0(980)->pi0eta(pi+pi-pi0) 
        IF(II.EQ.65) NAMES(I-NLT)='  TAU-  --> e-  a0(980) !nu_tau' ! TauDecayModeBBB = 367 ! a0(980)->pi0eta(pi+pi-gamma) 
        IF(II.EQ.66) NAMES(I-NLT)='  TAU-  --> mu- a0(980) !nu_tau' ! TauDecayModeBBB = 368 ! a0(980)->pi0eta(gammagamma) 
        IF(II.EQ.67) NAMES(I-NLT)='  TAU-  --> mu- a0(980) !nu_tau' ! TauDecayModeBBB = 369 ! a0(980)->pi0eta(pi0pi0pi0) 
        IF(II.EQ.68) NAMES(I-NLT)='  TAU-  --> mu- a0(980) !nu_tau' ! TauDecayModeBBB = 370 ! a0(980)->pi0eta(pi+pi-pi0) 
        IF(II.EQ.69) NAMES(I-NLT)='  TAU-  --> mu- a0(980) !nu_tau' ! TauDecayModeBBB = 371 ! a0(980)->pi0eta(pi+pi-gamma) 
        IF(II.EQ.70) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 372
        IF(II.EQ.71) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 373
        IF(II.EQ.72) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 374
        IF(II.EQ.73) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 375
        IF(II.EQ.74) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 376
        IF(II.EQ.75) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 377
        IF(II.EQ.76) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 378
        IF(II.EQ.77) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 379
        IF(II.EQ.78) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 380
        IF(II.EQ.79) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 381
        IF(II.EQ.80) NAMES(I-NLT)='  TAU-  --> xxxxxxxxx1xxxxxx   ' ! TauDecayModeBBB = 382

        MULPIK(I-NLT)=NPIK(I-NLT)
        DO J=1,JMAX
         IDFFIN(J,I-NLT)=NOPIK1(J,I-NLT-NM4-NM5-NM6-NM3-NM2)
        ENDDO

      ENDDO


      DO I = NLT+NM4+NM5+NM6+NM3+NM2+NM1+1,NBMC  ! initialization of memory not allocated even to multiplicity.
        JLIST(I) = 0
      ENDDO

c
c Initialization of default values for pre-samplers
c
      WRITE(iout,bxope)
      WRITE(iout,bxtxt) ' TAUOLA pre-sampler init SUBROUTINE INISAMPL '
      WRITE(iout,bxclo)
      CALL INISAMPL
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
      
      BRA1=1D0                  ! 0.5  ! PI+PI-PI- and PI-PI0PI0 are now 2 separate channels
      BRK0=0.5                  ! although K0  for active channels have been split into K0S, K0L, this is left = 0.5 for backward compatibility for other channels
      BRK0B=0.5                 ! although K0B for active channels have been split into K0S, K0L, this is left = 0.5 for backward compatibility for other channels
      BRKS=1D0                  !0.6667!  K+PI0 and PI+K0 are now 2 separate channels
*
      GFERMI = 1.16637E-5
      CCABIB = 0.975
      GV     = 1.0
      GA     =-1.0

      GFERMI = xpar(32)
      
C initialization form KK [obsolete]
C     IF (XPAR(ITAUXPAR+100+1).GT.-1D0) THEN
C        CCABIB = XPAR(ITAUXPAR+7)
C        GV     = XPAR(ITAUXPAR+8)
C        GA     = XPAR(ITAUXPAR+9)
C
C        BRA1   = 1.0 !XPAR(ITAUXPAR+10)
C        BRKS   = 1.0 !XPAR(ITAUXPAR+11)
C        BRK0   = XPAR(ITAUXPAR+12)
C        BRK0B  = XPAR(ITAUXPAR+13)
C     ENDIF

     
*     
*     http://pdg.lbl.gov/2020/reviews/rpp2020-rev-tau-branching-fractions.pdf
*     
      DO K=1,NCHAN
         GAMPRT(K)=0.0D0           ! XPAR(ITAUXPAR+100+K)
      ENDDO
      
C                    New BF              ! Old BF   !TauDecayMarker! Channel
       GAMPRT(1  ) = 0.178175 !± 0.000399! 0.178500 ! 1            ! e-
       GAMPRT(2  ) = 0.173937 !± 0.000384! 0.173600 ! 2            ! mu-
       GAMPRT(303) = 0.108164 !± 0.000512! 0.109100 ! 3            ! pi-
       GAMPRT(304) = 0.006964 !± 0.000096! 0.006960 ! 6            ! K-
       GAMPRT(163) = 0.254941 !± 0.000893! 0.255100 ! 4            ! pi-pi0
       GAMPRT(165) = 0.004328 !± 0.000148! 0.004230 ! 7            ! K-pi0
       GAMPRT(111) = 0.092595 !± 0.000964! 0.091600 ! 5            ! pi-2pi0 (ex K0) 
       GAMPRT(132) = 0.000374 !± 0.000038!          ! -1           ! pipi0omega(->pi0gamma): 0.004085*(1-0.893- 0.0153)=0.0003745945, 0.000419*(1-0.893-0.0153)=0.0000384
       GAMPRT(106) = 0.000647 !± 0.000218! 0.000650 ! 17           ! K-2pi0 (ex K0)
       GAMPRT(4  ) = 0.010429 !± 0.000707! 0.010400 ! 9            ! pi-3pi0 (ex K0)
       GAMPRT(9  ) = 0.000478 !± 0.000212! 0.000490 ! 30           ! K-3pi0 (ex K0, eta)
       GAMPRT(67 ) = 0.001118 !± 0.000391! 0.001100 ! 23           ! pi-4pi0 (ex K0, eta)
       GAMPRT(226) = 0.004192 !± 0.000069!0.008460/2! 7            ! pi-K0B, K0B = K0S; 0.008384/2 = 0.004192; 0.000138/2 = 0.000069
       GAMPRT(227) = 0.004192 !± 0.000069!0.008460/2! 7            ! pi-K0B, K0B = K0L; 0.008384/2 = 0.004192; 0.000138/2 = 0.000069
       GAMPRT(228) = 0.000743 !± 0.000017!0.001590/2! 22           ! K-K0, K0 = K0S; 0.001486/2 = 0.000743 ; 0.000034/2 = 0.000017
       GAMPRT(229) = 0.000743 !± 0.000017!0.001590/2! 22           ! K-K0, K0 = K0L; 0.001486/2 = 0.000743 ; 0.000034/2 = 0.000017
       GAMPRT(126) = 0.001909 !± 0.000065! 0.004000 ! 19           ! pi-K0Bpi0; K0B = K0S ; 0.003817/2 = 0.0019085 ; 0.000129/2 = 0.0000645
       GAMPRT(127) = 0.001909 !± 0.000065! 0.004000 ! 19           ! pi-K0Bpi0; K0B = K0L ; 0.003817/2 = 0.0019085 ; 0.000129/2 = 0.0000645
       GAMPRT(124) = 0.000750 !± 0.000035! 0.001590 ! 16           ! K-pi0K0; K0B = K0S; 0.001500/2 = 0.00075 ; 0.000070/2 = 0.000035
       GAMPRT(125) = 0.000750 !± 0.000035! 0.001590 ! 16           ! K-pi0K0; K0B = K0L; 0.001500/2 = 0.00075 ; 0.000070/2 = 0.000035
       GAMPRT(30 ) = 0.000131 !± 0.000113!0.000260/2! 32           ! pi-K0B2pi0 (ex K0), K0B = K0S; 0.000263/2 = 0.0001315 ; 0.000226/2 = 0.000113
       GAMPRT(31 ) = 0.000131 !± 0.000113!0.000260/2! 32           ! pi-K0B2pi0 (ex K0), K0B = K0L; 0.000263/2 = 0.0001315 ; 0.000226/2 = 0.000113
       GAMPRT(121) = 0.000235 !± 0.000006! 0.001700 ! 15           ! pi-K0K0B =  2*(pi- KS0 KS0) + (pi- KS0 KL0) = 2*0.000235 + 0.001081 = 0.001551 ; sqrt (2*0.000006*0.000006 + 0.000241*0.000241) = 0.000241
       GAMPRT(122) = 0.000235 !± 0.000006! 0.001700 ! 15           ! pi-K0K0B =  2*(pi- KS0 KS0) + (pi- KS0 KL0) = 2*0.000235 + 0.001081 = 0.001551 ; sqrt (2*0.000006*0.000006 + 0.000241*0.000241) = 0.000241
       GAMPRT(123) = 0.001081 !± 0.000241! 0.001700 ! 15           ! pi-K0K0B =  2*(pi- KS0 KS0) + (pi- KS0 KL0) = 2*0.000235 + 0.001081 = 0.001551 ; sqrt (2*0.000006*0.000006 + 0.000241*0.000241) = 0.000241
       GAMPRT(34 ) = 0.000018 !± 0.000002! 0.000310 ! 34           ! pi-pi0K0K0B = 2*(pi-pi0KS0KS0) + (pi-pi0KS0KL0) = 2 * 0.000018 + 0.000325 = 0.000361 ; sqrt(2*0.000002*0.000002 + 0.000119*0.000119) = 0.000119
       GAMPRT(35 ) = 0.000018 !± 0.000002! 0.000310 ! 34           ! pi-pi0K0K0B = 2*(pi-pi0KS0KS0) + (pi-pi0KS0KL0) = 2 * 0.000018 + 0.000325 = 0.000361 ; sqrt(2*0.000002*0.000002 + 0.000119*0.000119) = 0.000119
       GAMPRT(36 ) = 0.000325 !± 0.000119! 0.000310 ! 34           ! pi-pi0K0K0B = 2*(pi-pi0KS0KS0) + (pi-pi0KS0KL0) = 2 * 0.000018 + 0.000325 = 0.000361 ; sqrt(2*0.000002*0.000002 + 0.000119*0.000119) = 0.000119
       GAMPRT(32 ) = 0.000123 !± 0.000099!0.000230/2! 31           ! K0B 2pi-pi+, K0B = K0S; 0.000247/2 = 0.0001235; 0.000199/2 = 0.0000995
       GAMPRT(33 ) = 0.000123 !± 0.000099!0.000230/2! 31           ! K0B 2pi-pi+, K0B = K0L; 0.000247/2 = 0.0001235; 0.000199/2 = 0.0000995
       GAMPRT(112) = 0.089868 !± 0.000513! 0.091600 ! 5            ! 2pi-pi+ (ex K0,omega)                                                      
       GAMPRT(237) = 0.000298 !± 0.000010!          ! -1           ! piomega(->pi+pi-): 0.019494 * 0.0153 = 0.0002982582 ;  0.000645 * 0.0153 = 0.0000098685
       GAMPRT(3  ) = 0.044812 !± 0.000936! 0.044800 ! 8            ! 2pi-pi+pi0 (ex K0,omega) + piomega(->pi+pi-pi0) : 0.027404+0.019494*0.893 = 0.044812142 ; sqrt (0.000710*0.000710 + 0.000645*0.000645* 0.893) = 0.0009357
       GAMPRT(131) = 0.000063 !± 0.000006! 0.044800 ! -1           ! pipi0omega(->pi+pi-): 0.004085 * 0.0153 = 0.0000625005 ; 0.000419 * 0.0153 = 0.0000064
       GAMPRT(66 ) = 0.004629 !± 0.000532! 0.005100 ! 10           ! 2pi-pi+2pi0 (ex K0,omega,eta) + pi-pi0omega(->pipipi0) =  2pi-pi+2pi0 (ex K0,eta) : 0.000981 +  0.004085 * 0.893 = 0.004628905  ; sqrt( 0.000356*0.000356 + 0.000419*0.000419 * 0.893 ) = 0.000532 
       GAMPRT(103) = 0.001435 !± 0.000027! 0.001400 ! 14           ! pi-K-K+
       GAMPRT(23 ) = 0.000061 !± 0.000018! 0.000061 ! 33           ! pi-K-K+pi0
C      GAMPRT(109) = 0.001389 !± 0.000072! 0.001390 ! 20           ! pi-pi0eta
       GAMPRT(133) = 0.000547 !± 0.000028! 0.001390 ! 20           ! pi-pi0eta, eta->gammagamma : 0.001389*0.3941 = 0.0005474049, 0.000072*0.3941 = 0.0000283752
       GAMPRT(134) = 0.000454 !± 0.000024! 0.001390 ! 20           ! pi-pi0eta, eta->pi0pi0pi0  : 0.001389*0.3268 = 0.0004539252, 0.000072*0.3268 = 0.0000235296
       GAMPRT(135) = 0.000318 !± 0.000017! 0.001390 ! 20           ! pi-pi0eta, eta->pi+pi-pi0  : 0.001389*0.2292 = 0.0003183588, 0.000072*0.2292 = 0.0000165024
       GAMPRT(136) = 0.000069 !± 0.000004! 0.001390 ! 20           ! pi-pi0eta, eta->pi+pi-gamma: 0.001389*0.0499 = 0.0000693111, 0.000072*0.0499 = 0.0000035928
C      GAMPRT(193) = 0.000155 !± 0.000008! 0.000161 ! 27           ! K-eta
       GAMPRT(242) = 0.000061 !± 0.000003! 0.000161 ! 27           ! K-eta, eta->gammagamma : 0.000155*0.3941 = 0.0000610855, 0.000008*0.3941 = 0.0000031528
       GAMPRT(243) = 0.000050 !± 0.000003! 0.000161 ! 27           ! K-eta, eta->pi0pi0pi0  : 0.000155*0.3268 = 0.000050654,  0.000008*0.3268 = 0.0000026144
       GAMPRT(244) = 0.000036 !± 0.000002! 0.000161 ! 27           ! K-eta, eta->pi+pi-pi0  : 0.000155*0.2292 = 0.000035526,  0.000008*0.2292 = 0.0000018336
       GAMPRT(245) = 0.000008 !± 0.000001! 0.000161 ! 27           ! K-eta, eta->pi+pi-gamma: 0.000155*0.0499 = 0.0000077345, 0.000008*0.0499 = 0.0000003992
C      GAMPRT(115) = 0.000048 !± 0.000012! 0.000046 ! 28           ! K-pi0eta
       GAMPRT(137) = 0.000019 !± 0.000005! 0.000046 ! 28           ! K-pi0eta, eta->gammagamma : 0.000048*0.3941 = 0.0000189168, 0.000012*0.3941 = 0.0000047292
       GAMPRT(138) = 0.000016 !± 0.000004! 0.000046 ! 28           ! K-pi0eta, eta->pi0pi0pi0  : 0.000048*0.3268 = 0.0000156864, 0.000012*0.3268 = 0.0000039216
       GAMPRT(139) = 0.000011 !± 0.000003! 0.000046 ! 28           ! K-pi0eta, eta->pi+pi-pi0  : 0.000048*0.2292 = 0.0000110016, 0.000012*0.2292 = 0.0000027504
       GAMPRT(140) = 0.000002 !± 0.000001! 0.000046 ! 28           ! K-pi0eta, eta->pi+pi-gamma: 0.000048*0.0499 = 0.0000023952, 0.000012*0.0499 = 0.0000005988
C      GAMPRT(128) = 0.000047 !± 0.000008! 0.000092 ! 28           ! pi-K0Beta, K0B = K0S; 0.000094/2 = 0.000047; 0.000015/2 = 0.0000075
       GAMPRT(141) = 0.000019 !± 0.000003! 0.000092 ! 28           ! pi-K0Seta, eta->gammagamma : 0.000047*0.3941 = 0.0000185227, 0.000008*0.3941 = 0.0000031528
       GAMPRT(142) = 0.000015 !± 0.000003! 0.000092 ! 28           ! pi-K0Seta, eta->pi0pi0pi0  : 0.000047*0.3268 = 0.0000153596, 0.000008*0.3268 = 0.0000026144
       GAMPRT(143) = 0.000011 !± 0.000002! 0.000092 ! 28           ! pi-K0Seta, eta->pi+pi-pi0  : 0.000047*0.2292 = 0.0000107724, 0.000008*0.2292 = 0.0000018336
       GAMPRT(144) = 0.000002 !± 0.000001! 0.000092 ! 28           ! pi-K0Seta, eta->pi+pi-gamma: 0.000047*0.0499 = 0.0000023453, 0.000008*0.0499 = 0.0000003992
C      GAMPRT(129) = 0.000047 !± 0.000008! 0.000092 ! 28           ! pi-K0Beta, K0B = K0L; 0.000094/2 = 0.000047; 0.000015/2 = 0.0000075
       GAMPRT(145) = 0.000019 !± 0.000003! 0.000092 ! 28           ! pi-K0Leta, eta->gammagamma : 0.000047*0.3941 = 0.0000185227, 0.000008*0.3941 = 0.0000031528
       GAMPRT(146) = 0.000015 !± 0.000003! 0.000092 ! 28           ! pi-K0Leta, eta->pi0pi0pi0  : 0.000047*0.3268 = 0.0000153596, 0.000008*0.3268 = 0.0000026144
       GAMPRT(147) = 0.000011 !± 0.000002! 0.000092 ! 28           ! pi-K0Leta, eta->pi+pi-pi0  : 0.000047*0.2292 = 0.0000107724, 0.000008*0.2292 = 0.0000018336
       GAMPRT(148) = 0.000002 !± 0.000001! 0.000092 ! 28           ! pi-K0Leta, eta->pi+pi-gamma: 0.000047*0.0499 = 0.0000023453, 0.000008*0.0499 = 0.0000003992
C      GAMPRT(22 ) = 0.000094 !± 0.000015! 0.000164 ! 25           ! 2pi-pi+eta (ex K0,f1) [0.000220 - 0.000126 = 0.000094, sqrt(0.000013* 0.000013+0.000008*0.000008)=0.000015]
       GAMPRT(41 ) = 0.000037 !± 0.000006! 0.000164 ! 25           ! 2pi-pi+eta (ex K0,f1), eta->gammagamma  : 0.000094 * 0.3941 = 0.0000370454, 0.000015 * 0.3941 = 0.0000059115
       GAMPRT(42 ) = 0.000031 !± 0.000005! 0.000164 ! 25           ! 2pi-pi+eta (ex K0,f1), eta->pi0pi0pi0   : 0.000094 * 0.3268 = 0.0000307192, 0.000015 * 0.3268 = 0.000004902
       GAMPRT(43 ) = 0.000022 !± 0.000003! 0.000164 ! 25           ! 2pi-pi+eta (ex K0,f1), eta->pi+pi-pi0   : 0.000094 * 0.2292 = 0.0000215448, 0.000015 * 0.2292 = 0.000003438
       GAMPRT(44 ) = 0.000005 !± 0.000001! 0.000164 ! 25           ! 2pi-pi+eta (ex K0,f1), eta->pi+pi-gamma : 0.000094 * 0.0499 = 0.0000046906, 0.000015 * 0.0499 = 0.0000007485
C      GAMPRT(198) = 0.000410 !± 0.000092! 0.000410 ! 39           ! K-omega
       GAMPRT(250) = 0.000366 !± 0.000082! 0.000410 ! 39           ! K-omega, omega -> pi+pi-pi0, 0.000410 * 0.893  = 0.00036613,  0.000092 * 0.893  = 0.000082156
       GAMPRT(251) = 0.000006 !± 0.000001! 0.000410 ! 39           ! K-omega  omega -> pi+pi,     0.000410 * 0.0153 = 0.000006273, 0.000092 * 0.0153 = 0.0000014076
       GAMPRT(252) = 0.000038 !± 0.000008! 0.000410 ! 39           ! K-omega  omega -> pi0gamma,  0.000410 * 0.0917 = 0.000037597, 0.000092 * 0.0917 = 0.0000084364
       GAMPRT(230) = 0.000022 !± 0.000013! 0.000000 ! -1           ! K-Phi = K-K+K- + K-K0SK0L = 0.000022 + 0.000015 = 0.000037;  (0.000037/0.000044)*0.000016 = 0.000013
       GAMPRT(231) = 0.000015 !± 0.000013! 0.000000 ! -1           ! K-Phi = K-K+K- + K-K0SK0L = 0.000022 + 0.000015 = 0.000037;  (0.000037/0.000044)*0.000016 = 0.000013
       GAMPRT(107) = 0.002927 !± 0.000068! 0.003420 ! 18           ! K-pi-pi+ (ex K0, omega)
       GAMPRT(13 ) = 0.000394 !± 0.000142! 0.000370 ! 29           ! K-pi-pi+pi0 (ex K0, omega,eta)
C      GAMPRT(29 ) = 0.000072 !± 0.000016! 0.000140 ! 36           ! pi-2pi0omega (ex K0)
       GAMPRT(48 ) = 0.000064 !± 0.000014! 0.000140 ! 36           ! pi-2pi0omega (ex K0), omega -> pi+pi-pi0, 0.000072 * 0.893  = 0.000064296,  0.000016 * 0.893  = 0.000014288
       GAMPRT(49 ) = 0.000001 !± 0.000001! 0.000140 ! 36           ! pi-2pi0omega (ex K0), omega -> pi+pi,     0.000072 * 0.0153 = 0.0000011016, 0.000016 * 0.0153 = 0.0000002448
       GAMPRT(50 ) = 0.000007 !± 0.000001! 0.000140 ! 36           ! pi-2pi0omega (ex K0), omega -> pi0gamma,  0.000072 * 0.0917 = 0.0000066024, 0.000016 * 0.0917 = 0.0000014672
       GAMPRT(85 ) = 0.000014 !± 0.000027! 0.000230 ! 13           ! 2pi-pi+3pi0 (ex K0, eta, omega, f1)
       GAMPRT(68 ) = 0.000775 !± 0.000030! 0.000839 ! 11           ! 3pi-2pi+ (ex K0, omega, f1)
       GAMPRT(69 ) = 0.000001 !± 0.000001! 0.000000 ! -1           ! K-2pi-2pi+ (ex K0)
C      GAMPRT(28 ) = 0.000084 !± 0.000006! 0.000120 ! 35           ! 2pi-pi+omega (ex K0)
       GAMPRT(45 ) = 0.000075 !± 0.000005! 0.000120 ! 35           ! 2pi-pi+omega (ex K0), omega -> pi+pi-pi0, 0.000084 * 0.893  = 0.000075012,  0.000006 * 0.893  = 0.000005358
       GAMPRT(46 ) = 0.000001 !± 0.000001! 0.000120 ! 35           ! 2pi-pi+omega (ex K0), omega -> pi+pi,     0.000084 * 0.0153 = 0.0000012852, 0.000006 * 0.0153 = 0.0000000918
       GAMPRT(47 ) = 0.000008 !± 0.000001! 0.000120 ! 35           ! 2pi-pi+omega (ex K0), omega -> pi0gamma,  0.000084 * 0.0917 = 0.0000077028, 0.000006 * 0.0917 = 0.0000005502
       GAMPRT(84 ) = 0.000038 !± 0.000009! 0.000178 ! 12           ! 3pi-2pi+pi0 (ex K0, eta, omega, f1) 
       GAMPRT(89 ) = 0.000001 !± 0.000001! 0.000000 ! -1           ! K-2pi-2pi+pi0 (ex K0)
       GAMPRT(233) = 0.000052 !± 0.000004! 0.000360 ! 38           ! pi-f1 (f1 -> 2pi-2pi+)
C      GAMPRT(234) = 0.000126 !± 0.000008! 0.000360 ! 38           ! pi-f1 (f1 -> pi-pi+eta)
       GAMPRT(253) = 0.000050 !± 0.000003! 0.000360 ! 38           ! pi-f1 (f1 -> pi-pi+eta->gammagamma), 0.000126*0.3941 = 0.0000496566, 0.000008*0.3941=0.0000031528
       GAMPRT(254) = 0.000041 !± 0.000003! 0.000360 ! 38           ! pi-f1 (f1 -> pi-pi+eta->pi+pi-pi0)   0.000126*0.3268 = 0.0000411768, 0.000008*0.3268=0.0000026144
       GAMPRT(255) = 0.000029 !± 0.000002! 0.000360 ! 38           ! pi-f1 (f1 -> pi-pi+eta->pi+pi-pi0)   0.000126*0.2292 = 0.0000288792, 0.000008*0.2292=0.0000018336
       GAMPRT(256) = 0.000006 !± 0.000001! 0.000360 ! 38           ! pi-f1 (f1 -> pi-pi+eta->pi+pi-gamma) 0.000126*0.0499 = 0.0000062874, 0.000008*0.0499=0.0000003992
C      GAMPRT(15 ) = 0.000195 !± 0.000038! 0.000150 ! 26           ! pi-2pi0eta (ex K0)
       GAMPRT(37 ) = 0.000077 !± 0.000015! 0.000150 ! 26           ! pi-2pi0eta (ex K0), eta->gammagamma :  0.000195 * 0.3941 = 0.0000768495; 0.000038 * 0.3941 = 0.000014972
       GAMPRT(38 ) = 0.000064 !± 0.000012! 0.000150 ! 26           ! pi-2pi0eta (ex K0), eta->pi0pi0pi0  :  0.000195 * 0.3268 = 0.000063726;  0.000038 * 0.3268 = 0.0000124184 
       GAMPRT(39 ) = 0.000044 !± 0.000009! 0.000150 ! 26           ! pi-2pi0eta (ex K0), eta->pi+pi-pi0  :  0.000195 * 0.2292 = 0.000044694;  0.000038 * 0.2292 = 0.0000087096
       GAMPRT(40 ) = 0.000010 !± 0.000002! 0.000150 ! 26           ! pi-2pi0eta (ex K0), eta->pi+pi-gamma:  0.000195 * 0.0499 = 0.0000097305; 0.000038 * 0.0499 = 0.0000018962
       GAMPRT(5  ) = 0.000028 !± 0.000015! 0.000028 ! 37           ! e-e-e+
       GAMPRT(110) = 0.001789 !± 0.005914! 0.001300 ! 21           ! pi-omega(->pi0gam)=pi-pi0gamma; 0.019494 * (1-0.893- 0.0153)= 0.0017875998 ; 0.0645 * (1-0.893- 0.0153) = 0.005914
       GAMPRT(341) = 0.000238 !± 0.000000 ! -1          ! pi-gamma (a1-  ->pi-gamma) (0.092595 + 0.089868) * 0.0021 - 0.00011472345 - 0.000028 = 0.0003831723-0.00011472345-0.000028=0.00026844885-0.000028 = 0.000240 -0.000002 [rounding issues] = 0.000238
       GAMPRT(342) = 0.000115 !± 0.000000 ! -1          ! pi-gamma (rho- ->pi-gamma) .254941*4.5e-4 = 0.00011472345
       
C Reading of Branching Fractions turned off fow now ... 
!       DO K=1,NCHAN
!         GAMPRT(K)=XPAR(ITAUXPAR+100+K)
!        ENDDO

* ZW 13.04.89 HERE WAS AN ERROR
      SCABIB = SQRT(1.-CCABIB**2)
      PI =4.*ATAN(1.)
      GAMEL  = GFERMI**2*AMTAU**5/(192*PI**3)
*
*      CALL DEXAY(-1,pol1)
*

* PRINTOUTS FOR KK version
C      CALL INIOFC ! dummy for now anyway
      SUM=0.0D0
      KSUM=0
      DO K=1,NCHAN
         IF (GAMPRT(K).GT.0.0D0) THEN
            SUM=SUM+GAMPRT(K)
            KSUM=KSUM+1
         ENDIF
      ENDDO
      WRITE(IOUT,'(''NCHAN, KSUM, SUM = '',2I4,1X,F10.7)') NCHAN, KSUM, SUM

      IF(IFEQUALBR.EQ.1) THEN   ! uniform  initialization useful for some tests only
         DO K=1,NCHAN
            ACTIVE(K)=0
         ENDDO
         KSUM=0
         DO K=1,NCHAN
            IF (GAMPRT(K).GT.0.0D0) THEN
               KSUM=KSUM+1
               ACTIVE(KSUM)=K
               IF (K.LE.NLT) THEN
                  PRINT*, '0 KSUM = ',KSUM, ' TauDecayModeBBB = ',K,OLDNAMES(K),' GAMPRT = ',GAMPRT(K)
               ELSE
                  PRINT*, 'N KSUM = ',KSUM, ' TauDecayModeBBB = ',K,NAMES(K-NLT),' GAMPRT = ',GAMPRT(K)
               ENDIF
            ENDIF
         ENDDO
         WRITE(IOUT,'(''IFEQUALBR, KSUM, GAMPRT = '',2I4,1X,F10.7)') IFEQUALBR, KSUM, GAMPRT(1)
      ENDIF

      IF(IFEQUALBR.EQ.2) THEN   ! uniform  initialization useful for some tests only
         DO K=1,NCHAN
            ACTIVE(K)=0
         ENDDO
         KSUM=0
         DO K=1,NCHAN
            GAMPRT(K) = 0
            IF (K.LE.NLT) THEN
               IF (KEY0(K).GT.0) THEN
                  KSUM=KSUM+1
                  ACTIVE(KSUM)=K
                  WRITE (IOUT,*) '0 KSUM = ',KSUM, ' TauDecayModeBBB = ',K,OLDNAMES(K)
                  GAMPRT(K)=1
               ENDIF
            ELSE IF (K.LE.NLT+NM4) THEN
               IF (KEY4(K-NLT).GT.0) THEN
                  KSUM=KSUM+1
                  ACTIVE(KSUM)=K
                  WRITE(IOUT,*) '4 KSUM = ',KSUM, ' TauDecayModeBBB = ',K,NAMES(K-NLT)
                  GAMPRT(K)=1
               ENDIF
            ELSE IF (K.LE.NLT+NM4+NM5) THEN
               IF (KEY5(K-NLT-NM4).GT.0) THEN
                  KSUM=KSUM+1
                  ACTIVE(KSUM)=K
                  WRITE(IOUT,*) '5 KSUM = ',KSUM, ' TauDecayModeBBB = ',K,NAMES(K-NLT)
                  GAMPRT(K)=1
               ENDIF
            ELSE IF (K.LE.NLT+NM4+NM5+NM6) THEN
               IF (KEY6(K-NLT-NM4-NM5).GT.0) THEN
                  KSUM=KSUM+1
                  ACTIVE(KSUM)=K
                  WRITE(IOUT,*) '6 KSUM = ',KSUM, ' TauDecayModeBBB = ',K,NAMES(K-NLT)
                  GAMPRT(K)=1
               ENDIF
            ELSE IF (K.LE.NLT+NM4+NM5+NM6+NM3) THEN
               IF (KEY3(K-NLT-NM4-NM5-NM6).GT.0) THEN
                  KSUM=KSUM+1
                  ACTIVE(KSUM)=K
                  WRITE(IOUT,*) '3 KSUM = ',KSUM, ' TauDecayModeBBB = ',K,NAMES(K-NLT)
                  GAMPRT(K)=1
               ENDIF
            ELSE IF (K.LE.NLT+NM4+NM5+NM6+NM3+NM2) THEN
               IF (KEY2(K-NLT-NM4-NM5-NM6-NM3).GT.0) THEN
                  KSUM=KSUM+1
                  ACTIVE(KSUM)=K
                  WRITE(IOUT,*) '2 KSUM = ',KSUM, ' TauDecayModeBBB = ',K,NAMES(K-NLT)
                  GAMPRT(K)=1
               ENDIF
            ELSE IF (K.LE.NLT+NM4+NM5+NM6+NM3+NM2+NM1) THEN
               IF (KEY1(K-NLT-NM4-NM5-NM6-NM3-NM2).GT.0) THEN
                  KSUM=KSUM+1
                  ACTIVE(KSUM)=K
                  WRITE(IOUT,*) '1 KSUM = ',KSUM, ' TauDecayModeBBB = ',K,NAMES(K-NLT)
                  GAMPRT(K)=1
               ENDIF
            ENDIF
         ENDDO
         DO K=1,NCHAN
            IF (GAMPRT(K).GT.0.0D0) THEN
               GAMPRT(K) = 1.0D0/KSUM
            ENDIF
         ENDDO
         WRITE(IOUT,'(''IFEQUALBR, KSUM, GAMPRT = '',2I4,1X,F10.7)') IFEQUALBR, KSUM, GAMPRT(1)
      ENDIF
      
      WRITE(iout,bxope)
      WRITE(iout,bxtxt) ' TAUOLA Initialization SUBROUTINE INITDK:    '
      WRITE(iout,bxtxt) ' Adopted to read from KK                     '
      WRITE(iout,bxtxt) '                      '
      WRITE(iout,bxtxt) ' Choice Probability      --     Decay Channel'
      DO K=1,NLT      
      WRITE(iout,bxINIT) GAMPRT(K)/SUM,    OLDNAMES(K),'****','***'
      ENDDO
      DO K=NLT+1,NLT+NMODE      
      WRITE(iout,bxINIT) GAMPRT(K)/SUM,     NAMES(K-NLT),'****','***'
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

      SUBROUTINE TAURDF(KTO)
* THIS ROUTINE CAN BE CALLED BEFORE ANY TAU+ OR TAU- EVENT IS GENERATED
* IT CAN BE USED TO GENERATE TAU+ AND TAU- SAMPLES OF DIFFERENT
* CONTENTS
      include '../tauola/TAUDCD_BBB_size.inc'
      COMMON / TAUKLE / BRA1,BRK0,BRK0B,BRKS
      REAL*4            BRA1,BRK0,BRK0B,BRKS
      REAL*8            GAMPRT
      COMMON / TAUBRA / GAMPRT(NBMC),JLIST(NBMC),NCHAN
C this is a prototype, at present it is inactive, blocked by RETURN
      RETURN
     
      IF (KTO.EQ.1) THEN
*     ==================
* LIST OF BRANCHING RATIOS
C      NCHAN = 19
      DO 1 I = 1,30
      IF (I.LE.NCHAN) THEN
        JLIST(I) = I
        IF(I.EQ. 1) GAMPRT(I) = .0000
        IF(I.EQ. 2) GAMPRT(I) = .0000
        IF(I.EQ. 3) GAMPRT(I) = .0000
        IF(I.EQ. 4) GAMPRT(I) = .0000
        IF(I.EQ. 5) GAMPRT(I) = .0000
        IF(I.EQ. 6) GAMPRT(I) = .0000
        IF(I.EQ. 7) GAMPRT(I) = .0000
        IF(I.EQ. 8) GAMPRT(I) = 1.0000
        IF(I.EQ. 9) GAMPRT(I) = 1.0000
        IF(I.EQ.10) GAMPRT(I) = 1.0000
        IF(I.EQ.11) GAMPRT(I) = 1.0000
        IF(I.EQ.12) GAMPRT(I) = 1.0000
        IF(I.EQ.13) GAMPRT(I) = 1.0000
        IF(I.EQ.14) GAMPRT(I) = 1.0000
        IF(I.EQ.15) GAMPRT(I) = 1.0000
        IF(I.EQ.16) GAMPRT(I) = 1.0000
        IF(I.EQ.17) GAMPRT(I) = 1.0000
        IF(I.EQ.18) GAMPRT(I) = 1.0000
        IF(I.EQ.19) GAMPRT(I) = 1.0000
      ELSE
        JLIST(I) = 0
        GAMPRT(I) = 0.
      ENDIF
   1  CONTINUE
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
      ELSE
*     ====
* LIST OF BRANCHING RATIOS
C      NCHAN = 19
      DO 2 I = 1,30
      IF (I.LE.NCHAN) THEN
        JLIST(I) = I
        IF(I.EQ. 1) GAMPRT(I) = .0000
        IF(I.EQ. 2) GAMPRT(I) = .0000
        IF(I.EQ. 3) GAMPRT(I) = .0000
        IF(I.EQ. 4) GAMPRT(I) = .0000
        IF(I.EQ. 5) GAMPRT(I) = .0000
        IF(I.EQ. 6) GAMPRT(I) = .0000
        IF(I.EQ. 7) GAMPRT(I) = .0000
        IF(I.EQ. 8) GAMPRT(I) = 1.0000
        IF(I.EQ. 9) GAMPRT(I) = 1.0000
        IF(I.EQ.10) GAMPRT(I) = 1.0000
        IF(I.EQ.11) GAMPRT(I) = 1.0000
        IF(I.EQ.12) GAMPRT(I) = 1.0000
        IF(I.EQ.13) GAMPRT(I) = 1.0000
        IF(I.EQ.14) GAMPRT(I) = 1.0000
        IF(I.EQ.15) GAMPRT(I) = 1.0000
        IF(I.EQ.16) GAMPRT(I) = 1.0000
        IF(I.EQ.17) GAMPRT(I) = 1.0000
        IF(I.EQ.18) GAMPRT(I) = 1.0000
        IF(I.EQ.19) GAMPRT(I) = 1.0000
      ELSE
        JLIST(I) = 0
        GAMPRT(I) = 0.
      ENDIF
   2  CONTINUE
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
      ENDIF
*     =====
      END
