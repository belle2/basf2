!-----------------------------------------------------------------------
! TOP counter for Belle: initialization of/interface to TOP_GEO
!                        quartz data, mirror reflectivity
! M. Staric, mar-2009, feb-2010, sept-2011, jan-2012
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm 
! mar-2017: change in some commons
! dec-2017: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE SET_TOPVOL(R1,R2,Z1,Z2)

      IMPLICIT NONE
      REAL R1, R2               ! inner, outer radius of TOP
      REAL Z1, Z2               ! left-end, right-end of TOP
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
      DATA TOP_INI /.FALSE./
      DATA B_FIELD /1.5/        ! magnetic field [T]
      DATA SIG_BEAM /0/         ! MC: time spread of interactions [ns]
      DATA TOP_R1 /115./        ! inner radius of TOP [cm]
      DATA TOP_R2 /125./        ! outer radius of TOP [cm]
      DATA TOP_Z1 /-78./        ! left-end of TOP [cm]
      DATA TOP_Z2 /183./        ! right-end of TOP [cm]
      DATA QBAR_RADL /12.3/     ! MC: radiation length [cm]
      DATA BGR_PER_QBAR /0./    ! MC: number of bgr hits per Q-bar exit window
      DATA QBAR_REDG /0./       ! MC: Q-bar edge sharpness (radius of) [cm]
      DATA LIN_OPT /.FALSE./    ! MC: linear or exact optics simulated
      DATA NUM_MOD /0/

      TOP_R1=R1
      TOP_R2=R2
      TOP_Z1=Z1
      TOP_Z2=Z2
      NUM_MOD=0
      NUM_EXT=0

      END


      SUBROUTINE SET_BFIELD(B)

      IMPLICIT NONE
      REAL B               ! magnetic field [T]
#include "TOP_GEO.fi"

      B_FIELD=B

      END

      
      SUBROUTINE SET_SIMU_PAR(SIGMA_BEAM,BKG)
      
      IMPLICIT NONE
      REAL SIGMA_BEAM, BKG
#include "TOP_GEO.fi"

      IF(SIGMA_BEAM.NE.SIG_BEAM) THEN
         SIG_BEAM=SIGMA_BEAM
         PRINT*,'TOPsimu: T0 jitter = ', SIG_BEAM*1000.,' ps'
      ENDIF
      IF(BKG.NE.BGR_PER_QBAR) THEN
         BGR_PER_QBAR=BKG
         PRINT*,'TOPsimu: Bkg./Q-bar exit window = ', BGR_PER_QBAR
      ENDIF

      END


      SUBROUTINE SET_QBAR_REDG(R)

      IMPLICIT NONE
      REAL R
#include "TOP_GEO.fi"

      QBAR_REDG=R

      END


      INTEGER FUNCTION SET_QBAR(A,B,Z1,Z2,R,DX,FI,LSIDE,RSIDE)

c--   returns module ID or -1 (=failure)

      IMPLICIT NONE
      REAL A, B                 ! Q-bar width and thickness [cm]
      REAL Z1, Z2               ! Q-bar z-position
      REAL R                    ! Q-bar radius [cm]
      REAL DX                   ! shift in local x [cm]
      REAL FI                   ! Q-bar angle [rad]
      INTEGER LSIDE, RSIDE      ! type of left exit, right exit
#include "TOP_GEO.fi"
#include "TOP_DGEO.fi"
      INTEGER K, LR
      REAL C

      IF(NUM_MOD.GT.NSIZ_MOD) THEN
         PRINT*,'*** no space for new Qbar'
         SET_QBAR=-1
         RETURN
      ENDIF
      K=NUM_MOD
      NUM_MOD=NUM_MOD+1
      C=ABS(Z2-Z1)
      QBAR_A(K)=A
      QBAR_B(K)=B
      QBAR_C(K)=C
      QBAR_R(K)=R+B/2
      QBAR_DX(K)=DX
      QBAR_DZ(K)=(Z1+Z2)/2
      QBAR_FI(K)=FI
      QBAR_CFI(K)=COS(FI)
      QBAR_SFI(K)=SIN(FI)
      QBAR_RMI(K)=0.
      QBAR_FOCL(K)=0.
      QBAR_XCMI(K)=0.
      QBAR_YCMI(K)=0.
      QBAR_ZCMI(K)=0.
      QBAR_MIPOS(K)=0.
      QBAR_ZL(K)=-C/2
      QBAR_ZR(K)= C/2
      QBAR_WIN(K)=0.
      IF(LSIDE.EQ.1.OR.RSIDE.EQ.1) THEN
         QBAR_LSIDE(K)=LSIDE
         QBAR_RSIDE(K)=RSIDE
         IF(LSIDE.GE.3.OR.RSIDE.GE.3) THEN
            QBAR_RMI(K)=2*C
            QBAR_FOCL(K)=C
            QBAR_YCMI(K)=B/2
            CALL SET_ZCMI(K)
         ENDIF
         IF(LSIDE.GT.1) QBAR_MIPOS(K)=-1.
         IF(RSIDE.GT.1) QBAR_MIPOS(K)=1.
      ELSE
         QBAR_LSIDE(K)=0
         QBAR_RSIDE(K)=0
         PRINT*,'*** Q-bar has no PMT, ID=',K
      ENDIF
      DO LR=0,1
         QBAR_EXT(LR,K)=0
         CALL ARRANGE_PMT(K,LR,A,B,0.,0.)
      ENDDO
      DGEO_FLAG(K)=.FALSE.
      TOP_Z1=MIN(TOP_Z1,MIN(Z1,Z2))
      TOP_Z2=MAX(TOP_Z2,MAX(Z1,Z2))
      SET_QBAR=K

      END


      SUBROUTINE SET_ALIGNMENT(ID,DX,DY,DZ,ALFA,BETA,GAMA)

      IMPLICIT NONE
      INTEGER ID                ! module ID
      REAL DX, DY, DZ           ! translations
      REAL ALFA, BETA, GAMA     ! rotation angles around x, y, z
      
#include "TOP_GEO.fi"
#include "TOP_DGEO.fi"
      REAL RX(3,3), RY(3,3), RZ(3,3), RR(3,3), S
      REAL CFI, SFI
      INTEGER I,J,K
      
      IF(ID.LT.0.OR.ID.GE.NUM_MOD) RETURN

      DGEO_FLAG(ID)=.TRUE.
      
      QBAR_TRS(1,ID)=DX
      QBAR_TRS(2,ID)=DY
      QBAR_TRS(3,ID)=DZ
      
      RX=0
      CFI=COS(ALFA)
      SFI=SIN(ALFA)
      RX(1,1)=1
      RX(2,2)=CFI
      RX(2,3)=-SFI
      RX(3,2)=SFI
      RX(3,3)=CFI
      
      RY=0
      CFI=COS(BETA)
      SFI=SIN(BETA)
      RY(1,1)=CFI
      RY(1,3)=SFI
      RY(2,2)=1
      RY(3,1)=-SFI
      RY(3,3)=CFI
      
      RZ=0
      CFI=COS(GAMA)
      SFI=SIN(GAMA)
      RZ(1,1)=CFI
      RZ(1,2)=-SFI
      RZ(2,1)=SFI
      RZ(2,2)=CFI
      RZ(3,3)=1
      
      DO K=1,3
         DO I=1,3
            S=0
            DO J=1,3
               S=S+RY(I,J)*RX(J,K)
            ENDDO
            RR(I,K)=S
         ENDDO
      ENDDO

      DO K=1,3
         DO I=1,3
            S=0
            DO J=1,3
               S=S+RZ(I,J)*RR(J,K)
            ENDDO
            QBAR_ROT(I,K,ID)=S
         ENDDO
      ENDDO
      
      END


      SUBROUTINE SET_BBWIN(ID,D)

      IMPLICIT NONE
      INTEGER ID                ! module ID
      REAL D                    ! thickness

#include "TOP_GEO.fi"

      IF(ID.LT.0.OR.ID.GE.NUM_MOD) RETURN
      IF(D>0) QBAR_WIN(ID)=D

      END


      SUBROUTINE SET_EXTVOL(ID,LR,SHAPE,DZE,YUPE,YDNE,XSIZ,
     &     YUPI,YDNI,DZREFL,REFL)

      IMPLICIT NONE
      INTEGER ID                ! module ID
      INTEGER LR                ! 0/1=left/right
      INTEGER SHAPE             ! shape, 0/1/2=noRefl/Box/Prism
      REAL DZE, YUPE, YDNE      ! exp. volume geo [cm]
      REAL XSIZ                 ! width if >0 else bar width used
      REAL YUPI, YDNI           ! input window if>0 else bar used
      REAL DZREFL, REFL         ! reflective part

#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
      REAL DZ, YUP, YDN, ZZ, A
      REAL YUPIN, YDNIN
      INTEGER TYP(0:1)
      INTEGER K

      IF(SHAPE.LT.0.OR.SHAPE.GT.2) THEN
         PRINT*,'addExpansionVolume: unknown shape ',SHAPE
         RETURN
      ENDIF

      IF(ID.LT.0.OR.ID.GE.NUM_MOD) RETURN
      TYP(0)=QBAR_LSIDE(ID)
      TYP(1)=QBAR_RSIDE(ID)
      IF(TYP(LR).NE.1) RETURN

      DZ=ABS(DZE)
      A=XSIZ
      IF(A.LE.0) A=QBAR_A(ID)
      YUPIN=YUPI
      IF(YUPIN.EQ.0) YUPIN=QBAR_B(ID)/2
      YDNIN=YDNI
      IF(YDNIN.EQ.0) YDNIN=-QBAR_B(ID)/2
      IF(YUPE.GT.YDNE) THEN
         YUP=YUPE
         YDN=YDNE
      ELSE
         YUP=YDNE
         YDN=YUPE
      ENDIF
      IF(SHAPE.EQ.1) THEN
         YUPIN=YUP
         YDNIN=YDN
      ENDIF

      DO K=1,NUM_EXT
         IF(EXT_DZ(K).EQ.DZ.AND.EXT_YUP(K).EQ.YUP.AND.EXT_YDN(K).EQ.YDN
     &        .AND.EXT_YUPIN(K).EQ.YUPIN.AND.EXT_YDNIN(K).EQ.YDNIN
     &        .AND.EXT_A(K).EQ.A.AND.EXT_DZREFL(K).EQ.DZREFL
     &        .AND.EXT_REFL(K).EQ.REFL
     &        .AND.EXT_LR(K).EQ.LR.AND.EXT_SHAPE(K).EQ.SHAPE) THEN 
            EXIT
         ENDIF
      ENDDO      
      IF(K.GT.MAX_EXT) RETURN

      IF(K.GT.NUM_EXT) THEN
         EXT_DZ(K)=DZ
         EXT_YUP(K)=YUP
         EXT_YDN(K)=YDN
         EXT_YUPIN(K)=YUPIN
         EXT_YDNIN(K)=YDNIN
         EXT_A(K)=A
         EXT_DZREFL(K)=DZREFL
         EXT_REFL(K)=REFL
         EXT_SHAPE(K)=SHAPE
         EXT_LR(K)=LR
         NUM_EXT=K
      ENDIF
      QBAR_EXT(LR,ID)=K
      CALL ARRANGE_PMT(ID,LR,A,YUP-YDN,0.,0.)
      QBAR_FOCL(ID)=QBAR_C(ID)+EXT_DZ(K)
      IF(LR.EQ.0) THEN
         ZZ=QBAR_ZL(ID)-EXT_DZ(K)+QBAR_DZ(ID)
         TOP_Z1=MIN(TOP_Z1,ZZ)
      ELSE
         ZZ=QBAR_ZR(ID)+EXT_DZ(K)+QBAR_DZ(ID)
         TOP_Z2=MAX(TOP_Z2,ZZ)
      ENDIF

      END


      SUBROUTINE ARRANGE_PMT(ID,LR,SIZX,SIZY,DX,DY)

      IMPLICIT NONE
      INTEGER ID                ! module ID
      INTEGER LR                ! 0/1=left/right
      REAL SIZX, SIZY, DX, DY   ! window size, window shift relative to exit window [cm]
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_PMT.fi"
      INTEGER TYP(0:1), II
      REAL DXW, DYW
      
      IF(ID.LT.0.OR.ID.GE.NUM_MOD) RETURN
      N_PMTX(LR,ID)=0
      N_PMTY(LR,ID)=0
      PMT_DX(LR,ID)=0.
      PMT_X0(LR,ID)=0.
      PMT_DY(LR,ID)=0.
      PMT_Y0(LR,ID)=0.
      IF(.NOT.PMT_INI) THEN
         PRINT*,'*** PMT data not loaded ID=',ID
         RETURN
      ENDIF
      TYP(0)=QBAR_LSIDE(ID)
      TYP(1)=QBAR_RSIDE(ID)
      IF(TYP(LR).NE.1) RETURN

      N_PMTX(LR,ID)=MAX(IFIX(SIZX/PMT_A),1)
      N_PMTY(LR,ID)=MAX(IFIX(SIZY/PMT_B),1)
      
      II=QBAR_EXT(LR,ID)
      IF(II.GT.0) THEN
         DXW=DX
         DYW=DY+(EXT_YUP(II)+EXT_YDN(II))/2.
      ELSE
         DXW=DX
         DYW=DY
      ENDIF

      CALL SET_X0DX(SIZX,DXW,PMT_A,N_PMTX(LR,ID),
     &     PMT_X0(LR,ID),PMT_DX(LR,ID))
      CALL SET_X0DX(SIZY,DYW,PMT_B,N_PMTY(LR,ID),
     &     PMT_Y0(LR,ID),PMT_DY(LR,ID))

      END


      SUBROUTINE SET_RMI(ID,RMI)

      IMPLICIT NONE
      INTEGER ID                ! module ID
      REAL RMI                  ! mirror: radius of curvature
#include "TOP_GEO.fi"

      IF(ID.LT.0.OR.ID.GE.NUM_MOD) RETURN
      IF(QBAR_LSIDE(ID).GE.3.OR.QBAR_RSIDE(ID).GE.3) THEN
         QBAR_RMI(ID)=RMI
         CALL SET_ZCMI(ID)
      ENDIF
      END


      SUBROUTINE SET_XYC(ID,XC,YC)

      IMPLICIT NONE
      INTEGER ID                ! module ID
      REAL XC, YC               ! mirror: center of curvature
#include "TOP_GEO.fi"

      IF(ID.LT.0.OR.ID.GE.NUM_MOD) RETURN
      IF(QBAR_LSIDE(ID).GE.3.OR.QBAR_RSIDE(ID).GE.3) THEN
         QBAR_XCMI(ID)=XC
         QBAR_YCMI(ID)=YC
         CALL SET_ZCMI(ID)
      ENDIF
      END


      LOGICAL FUNCTION TOP_GEO_FINALIZE(FLAG)

      IMPLICIT NONE
      INTEGER FLAG
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
#include "TOP_QEFF.fi"
#include "TOP_TDC.fi"
#include "TOP_PAR.fi"
#include "TOP_PIK.fi"
#include "TOP_PHTIM.fi"
      INTEGER I, MAXCH, MAXCHX, LR

      NMA_PMT=0
      MAXCHX=0
      DO I=0,NUM_MOD-1
         N_PMT(0,I)=N_PMTX(0,I)*N_PMTY(0,I)
         N_PMT(1,I)=N_PMTX(1,I)*N_PMTY(1,I)
         NMA_PMT=MAX(NMA_PMT,N_PMT(0,I)+N_PMT(1,I))
         QBAR_NCH0(I)=PMT_NX*PMT_NY*N_PMT(0,I)
         DO LR=0,1
            MAXCHX=MAX(MAXCHX,N_PMTX(LR,I)*PMT_NX)
         ENDDO
      ENDDO

      IF(.NOT.PAR_INI) CALL SET_TOP_PAR(0.,1.)

      TOP_INI=PMT_INI.AND.QE_INIT.AND.TDC_INI.AND.PAR_INI
      TOP_INI=TOP_INI.AND.NUM_MOD.GT.0
      DO I=0,NUM_MOD-1
         TOP_INI=TOP_INI.AND.N_PMT(0,I)+N_PMT(1,I).GT.0
      ENDDO

      MAXCH=PMT_NX*PMT_NY*NMA_PMT ! max # of Q-bar channels
      TOP_INI=TOP_INI.AND.MAXCH.LE.NUM_CHA
      
      IF(MAXCH.GT.NUM_CHA) THEN
         PRINT*,'*** Error (top_reco): MAXCH>NUM_CHA, MAXCH=',MAXCH
         PRINT*,'--> Increase parameter value ',
     &        'TOP_PIK.fi::NUM_CHA and recompile!'
      ENDIF

      TOP_INI=TOP_INI.AND.MAXCHX.LE.NUM_CHX

      IF(MAXCHX.GT.NUM_CHX) THEN
         PRINT*,'*** Error (top_reco): MAXCHX>NUM_CHX, MAXCHX=',MAXCHX
         PRINT*,'--> Increase parameter value ',
     &        'TOP_PHTIM.fi::NUM_CHX and recompile!'
      ENDIF

      IF(TOP_INI) THEN
         IF(FLAG.NE.0) CALL TOP_GEO_DUMP
         PRINT*,'TOP configured with ',NUM_MOD,' Q-bars.'
         PRINT*
      ELSE
         PRINT*,'**** TOP configuration failed.'
      ENDIF

      TOP_GEO_FINALIZE=TOP_INI

      END


c-- internal functions --------------------------------------------------

      SUBROUTINE TOP_GEO_DUMP

      IMPLICIT NONE
#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
#include "TOP_TDC.fi"
#include "TOP_PAR.fi"
#include "TOP_EXT.fi"

      INTEGER I
      LOGICAL FOCTOP

      FOCTOP=.FALSE.
      DO I=0,NUM_MOD-1
         IF(QBAR_LSIDE(I).EQ.3) FOCTOP=.TRUE.
         IF(QBAR_LSIDE(I).EQ.4) FOCTOP=.TRUE.
         IF(QBAR_RSIDE(I).EQ.3) FOCTOP=.TRUE.
         IF(QBAR_RSIDE(I).EQ.4) FOCTOP=.TRUE.
      ENDDO

      PRINT*,'Magnetic field [T]:       ', B_FIELD
      PRINT*,'inner/outer radius [cm]:  ', TOP_R1, TOP_R2
      PRINT*,'left/right ends [cm]:     ', TOP_Z1, TOP_Z2
      PRINT*,'Q-bar radiation len. [cm]:', QBAR_RADL
      PRINT*,'Q-bar edge roughness [um]:', QBAR_REDG*1.e4
      PRINT*,'Number of modules:        ', NUM_MOD
      PRINT*,'PMT dimensions [cm]:      ', PMT_A, PMT_B
      PRINT*,'PMT active area [cm]:     ', PMT_AA, PMT_BB
      PRINT*,'PMT number of pads:       ', PMT_NX, PMT_NY
      PRINT*,'PMT pad size [cm]:        ', PMT_DXCH, PMT_DYCH
      PRINT*,'TTS number of Gaussians:  ', TTS_NG
      PRINT*,'          I  fraction [%]     mean [ps]      sigma [ps]'
      DO I=1,TTS_NG
         PRINT*,I,TTS_FRAC(I)*100.,TTS_T0(I)*1000.,TTS_SIG(I)*1000.
      ENDDO
      PRINT*,'TDC range [channels]:     ', TDC_RANGE
      PRINT*,'TDC channel width [ps]:   ', TDC_DT*1000.
      PRINT*,'TDC offset [ns]:          ', TDC_T0
      PRINT*,'time range [ns]:          ', -TDC_T0,TIME_RANGE-TDC_T0
      PRINT*,'CFD pile-up time [ns]:    ', CFD_DT
      PRINT*,'CFD double hit res. [ns]: ', CFD_DELT
      PRINT*,'mean photon energy [eV]:  ', E_MEAN
      PRINT*,'sigma e [eV]:             ', SIG_E
      PRINT*,'step for d/de [eV]:       ', D_E
      PRINT*,'mean phase & group index: ', REF_IND, GRU_IND
      PRINT*,'phase & group dispersion: ', REL_DNDE, REL_DNGDE
      PRINT*,'N0 [cm^-1]:               ', N_ZERO
      IF(FOCTOP) THEN
         IF(LIN_OPT) THEN
            PRINT*,'Focusing mirror: linear optics approximation'
         ELSE
            PRINT*,'Focusing mirror: exact optics'
         ENDIF
      ENDIF
      PRINT*

      PRINT*,'ID     A      B      C    Rxy     Dx     Dz     PHI  L  R
     &   Rmi     Xc     Yc     Zc    Zl     Zr  NpmtL NpmtR ExtL ExtR bb
     &win'

      DO I=0,NUM_MOD-1
         PRINT 100,I,QBAR_A(I),QBAR_B(I),QBAR_C(I),QBAR_R(I),QBAR_DX(I),
     &        QBAR_DZ(I),QBAR_FI(I)/PI*180.,QBAR_LSIDE(I),QBAR_RSIDE(I),
     &        QBAR_RMI(I),QBAR_XCMI(I),QBAR_YCMI(I),QBAR_ZCMI(I),
     &        QBAR_ZL(I),QBAR_ZR(I),N_PMT(0,I),N_PMT(1,I),QBAR_EXT(0,I),
     &        QBAR_EXT(1,I),QBAR_WIN(I)
      ENDDO
 100  FORMAT(I3,7F7.1,2I3,6F7.1,4I5,F6.2)
      IF(NUM_EXT.GT.0) THEN
         PRINT*
         PRINT*,'Expansion volume(s):'
         PRINT*,' I Shape LR    Dz      Yup     Ydn     A      YupIn   Y
     &dnIn   DzRefl  Refl'
         DO I=1,NUM_EXT
            PRINT 101,I,EXT_SHAPE(I),EXT_LR(I),EXT_DZ(I),EXT_YUP(I),
     &           EXT_YDN(I),EXT_A(I),EXT_YUPIN(I),EXT_YDNIN(I),
     &           EXT_DZREFL(I),EXT_REFL(I)
         ENDDO
      ENDIF
 101  FORMAT(I3,2I4,8F8.2)
      PRINT*

c      call print_refind(1.4, 4.0, 0.01) ! xml format

      END


      SUBROUTINE SET_X0DX(SIZX,SHIFTX,A,NX,X0,DX)

      IMPLICIT NONE
      REAL SIZX, SHIFTX,A
      INTEGER NX
      REAL X0, DX

      IF(NX.GT.1) THEN
         X0=-(SIZX-A)/2+SHIFTX
         DX=(SIZX-A)/FLOAT(NX-1)
      ELSE
         X0=SHIFTX
         DX=A
      ENDIF

      END


      SUBROUTINE SET_ZCMI(I)

      IMPLICIT NONE
      INTEGER I
#include "TOP_GEO.fi"
      REAL ZMI, ZMA

      IF(QBAR_LSIDE(I).EQ.3) THEN
         CALL ZMIMA_CYL(I,ZMI,ZMA)
         QBAR_ZCMI(I)=-(QBAR_C(I)/2-ZMA)
         QBAR_ZL(I)=  -(QBAR_C(I)/2-ZMA+ZMI)
      ELSE IF(QBAR_LSIDE(I).EQ.4) THEN
         CALL ZMIMA_SPH(I,ZMI,ZMA)
         QBAR_ZCMI(I)=-(QBAR_C(I)/2-ZMA)
         QBAR_ZL(I)=  -(QBAR_C(I)/2-ZMA+ZMI)
      ELSE IF(QBAR_RSIDE(I).EQ.3) THEN
         CALL ZMIMA_CYL(I,ZMI,ZMA)
         QBAR_ZCMI(I)=QBAR_C(I)/2-ZMA
         QBAR_ZR(I)=  QBAR_C(I)/2-ZMA+ZMI
      ELSE IF(QBAR_RSIDE(I).EQ.4) THEN
         CALL ZMIMA_SPH(I,ZMI,ZMA)
         QBAR_ZCMI(I)=QBAR_C(I)/2-ZMA
         QBAR_ZR(I)=  QBAR_C(I)/2-ZMA+ZMI
      ENDIF

      END


      SUBROUTINE ZMIMA_CYL(I,ZMI,ZMA)

      IMPLICIT NONE
      INTEGER I
      REAL ZMI, ZMA
#include "TOP_GEO.fi"
      REAL YC, RMI, BH

      YC=QBAR_YCMI(I)
      BH=QBAR_B(I)/2.
      RMI=QBAR_RMI(I)

      ZMI=SQRT(RMI**2-(BH+ABS(YC))**2)
      IF(ABS(YC).LE.BH) THEN
         ZMA=RMI
      ELSE
         ZMA=SQRT(RMI**2-(BH-ABS(YC))**2)
      ENDIF

      END


      SUBROUTINE ZMIMA_SPH(I,ZMI,ZMA)

      IMPLICIT NONE
      INTEGER I
      REAL ZMI, ZMA
#include "TOP_GEO.fi"
      REAL XC, YC, RMI, AH, BH, R

      XC=QBAR_XCMI(I)
      YC=QBAR_YCMI(I)
      AH=QBAR_A(I)/2.
      BH=QBAR_B(I)/2.
      RMI=QBAR_RMI(I)

      ZMI=SQRT(RMI**2-(AH+ABS(XC))**2-(BH+ABS(YC))**2)
      IF(ABS(YC).LE.BH) THEN
         R=RMI
      ELSE
         R=SQRT(RMI**2-(BH-ABS(YC))**2)
      ENDIF
      IF(ABS(XC).LE.AH) THEN
         ZMA=R
      ELSE
         ZMA=SQRT(R**2-(AH-ABS(XC))**2)
      ENDIF

      END



c-- functions with quartz & mirror data -------------------------------------

      SUBROUTINE PRINT_REFIND(EMIN, EMAX, STEP)

c     print refractive index in xml format

      IMPLICIT NONE
      REAL EMIN, EMAX, STEP     ! range and step [eV]
      
      REAL E, PHASE_INDEX

      E=EMIN
      DO WHILE(E.LE.EMAX)
         PRINT 100,' <value energy="',E,'">',PHASE_INDEX(E),'</value>'
         E=E+STEP
      ENDDO
      PRINT*
 100  FORMAT(A,F10.8,A,F10.8,A)

      END

***** quartz refractive index, absorption length

      REAL FUNCTION PHIND_LAMBDA(LAM)

      IMPLICIT NONE
      REAL LAM                  ! photon wavelength (nm)

      INTEGER I
      REAL X, B(3), C(3)
      DATA B /0.6961663, 0.4079426, 0.8974794/
      DATA C /0.0046791, 0.0135121, 97.9340025/
      SAVE B, C

C      PHIND_LAMBDA=1.44+8.20/(LAM-126.) ! T. Ijima (6-jul-2004)
      
      X=(LAM*0.001)**2
      PHIND_LAMBDA=1.
      DO I=1,3
         PHIND_LAMBDA=PHIND_LAMBDA+B(I)*X/(X-C(I))
      ENDDO
      PHIND_LAMBDA=SQRT(PHIND_LAMBDA) ! K. Inami (avg-2013)

      END


      REAL FUNCTION PHASE_INDEX(E)

      IMPLICIT NONE
      REAL E                    ! photon energy (eV)

      REAL LAM, PHIND_LAMBDA

      LAM=1240./E
      PHASE_INDEX=PHIND_LAMBDA(LAM)

      END


      REAL FUNCTION GROUP_INDEX(E)

      IMPLICIT NONE
      REAL E                    ! photon energy (eV)

      REAL LAM, DL, DNDL, PHIND, PHIND_LAMBDA
      DATA DL /1./              ! (nm)
      SAVE DL

      LAM=1240./E
      PHIND=PHIND_LAMBDA(LAM)
      DNDL=(PHIND_LAMBDA(LAM+DL/2)-PHIND_LAMBDA(LAM-DL/2))/DL

      GROUP_INDEX=PHIND/(1.+LAM/PHIND*DNDL)

      END


      REAL FUNCTION ABSPT_LENG(E)

      IMPLICIT NONE
      REAL E                    ! photon energy (eV)

      REAL LAM

      LAM=1240/E
cc      ABSPT_LENG=50000.*(LAM/442)**4 ! J. Va'vra (7-jul-2004)
      ABSPT_LENG=15100.*(LAM/405)**4 ! Alan, 2013

      END

***** mirror reflectivity

      REAL FUNCTION MIRROR_REFL(E)
      
      IMPLICIT NONE
      REAL E                    ! photon energy (eV)

      MIRROR_REFL=0.90

      END


c-- utility functions (moved from top_mcgen.F + new) -------------------

      SUBROUTINE TO_MODULE(RF,DIR,MODULE,OK)
      
      IMPLICIT NONE
      REAL RF(3), DIR(3)
      INTEGER MODULE
      LOGICAL OK
#include "TOP_GEO.fi"
      REAL X, Y, Z
      INTEGER I, K
      LOGICAL INSIDE_QBAR, INSIDE_EXTVOL
      INTEGER MDN /0/
      SAVE MDN

      OK=.FALSE.
      DO K=0,NUM_MOD-1
         I=MOD(K+MDN,NUM_MOD)
         X=RF(1)*QBAR_CFI(I)+RF(2)*QBAR_SFI(I)-QBAR_DX(I)
         IF(ABS(X).GT.QBAR_A(I)/2) CYCLE
         Y=RF(2)*QBAR_CFI(I)-RF(1)*QBAR_SFI(I)-QBAR_R(I)
         Z=RF(3)-QBAR_DZ(I)
         OK=INSIDE_QBAR(X,Y,Z,I).OR.INSIDE_EXTVOL(X,Y,Z,I)
         IF(.NOT.OK) CYCLE
         MODULE=I
         MDN=I
         RF(1)=X
         RF(2)=Y
         RF(3)=Z
         X=DIR(1)*QBAR_CFI(I)+DIR(2)*QBAR_SFI(I)
         Y=DIR(2)*QBAR_CFI(I)-DIR(1)*QBAR_SFI(I)
         DIR(1)=X
         DIR(2)=Y
         RETURN
      ENDDO

      END


      LOGICAL FUNCTION INSIDE_QBAR(XP,YP,ZP,M)

      IMPLICIT NONE
      REAL XP,YP,ZP             ! point
      INTEGER M                 ! module number

#include "TOP_GEO.fi"
      REAL RR, XC, YC, X, Y

      INSIDE_QBAR=.FALSE.
      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN
      IF(ABS(XP).GT.QBAR_A(M)/2) RETURN
      IF(ABS(YP).GT.QBAR_B(M)/2) RETURN
      IF(ABS(ZP).GT.QBAR_C(M)/2) RETURN
      IF(QBAR_REDG.GT.0) THEN
         XC=QBAR_A(M)/2-QBAR_REDG
         YC=QBAR_B(M)/2-QBAR_REDG
         X=ABS(XP)-XC
         Y=ABS(YP)-YC
         IF(X.GT.0.AND.Y.GT.0) THEN
            RR=X**2+Y**2
            IF(RR.GT.QBAR_REDG**2) RETURN
         ENDIF
      ENDIF
      IF(ZP.LT.QBAR_ZL(M)) THEN
         IF(QBAR_LSIDE(M).EQ.3) THEN
            RR=(YP-QBAR_YCMI(M))**2+(ZP-QBAR_ZCMI(M))**2
            IF(RR.GT.QBAR_RMI(M)**2) RETURN
         ELSE IF(QBAR_LSIDE(M).EQ.4) THEN
            RR=(XP-QBAR_XCMI(M))**2+(YP-QBAR_YCMI(M))**2+
     &           (ZP-QBAR_ZCMI(M))**2
            IF(RR.GT.QBAR_RMI(M)**2) RETURN
         ENDIF
      ELSE IF(ZP.GT.QBAR_ZR(M)) THEN
         IF(QBAR_RSIDE(M).EQ.3) THEN
            RR=(YP-QBAR_YCMI(M))**2+(ZP-QBAR_ZCMI(M))**2
            IF(RR.GT.QBAR_RMI(M)**2) RETURN
         ELSE IF(QBAR_RSIDE(M).EQ.4) THEN
            RR=(XP-QBAR_XCMI(M))**2+(YP-QBAR_YCMI(M))**2+
     &           (ZP-QBAR_ZCMI(M))**2
            IF(RR.GT.QBAR_RMI(M)**2) RETURN
         ENDIF
      ENDIF

      INSIDE_QBAR=.TRUE.

      END


      LOGICAL FUNCTION INSIDE_EXTVOL(XP,YP,ZP,M)

      IMPLICIT NONE
      REAL XP,YP,ZP             ! point
      INTEGER M                 ! module number

#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
      INTEGER LR, K
      REAL Z, ZZ, Y


      INSIDE_EXTVOL=.FALSE.
      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN
      ZZ=ZP
      DO LR=0,1
         K=QBAR_EXT(LR,M)
         IF(K.EQ.0) CYCLE
         IF(ZZ.GT.0) CYCLE
         ZZ=-ZZ
         Z=ABS(ZP)-QBAR_C(M)/2
         IF(Z.LT.0.OR.Z.GT.EXT_DZ(K)) CYCLE
         IF(ABS(XP).GT.EXT_A(K)/2) RETURN
         IF(YP.LT.MIN(EXT_YDN(K),EXT_YDNIN(K))) CYCLE
         IF(YP.GT.MAX(EXT_YUP(K),EXT_YUPIN(K))) CYCLE
         IF(EXT_SHAPE(K).EQ.2) THEN
            Y=(EXT_YUP(K)-EXT_YUPIN(K))/EXT_DZ(K)*Z+EXT_YUPIN(K)
            IF(YP.GT.Y) CYCLE
            Y=(EXT_YDN(K)-EXT_YDNIN(K))/EXT_DZ(K)*Z+EXT_YDNIN(K)
            IF(YP.LT.Y) CYCLE
         ENDIF
         INSIDE_EXTVOL=.TRUE.
         RETURN
      ENDDO

      END


      SUBROUTINE TO_GLOBAL(R,DIR,M)

c--   transformation from local (bar) to global (Belle II) frame
      
      IMPLICIT NONE
      REAL R(3), DIR(3)         ! point, direction (or momentum vector)
      INTEGER M                 ! bar ID
#include "TOP_GEO.fi"
      REAL X, Y

      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN

      CALL LOCAL_TO_NOMINAL(R,DIR,M)

!     nominal to global: translation by (dx,r,dz) then rotation around z by phi
      
      X=R(1)+QBAR_DX(M)
      Y=R(2)+QBAR_R(M)
      R(1)=X*QBAR_CFI(M)-Y*QBAR_SFI(M)
      R(2)=Y*QBAR_CFI(M)+X*QBAR_SFI(M)
      R(3)=R(3)+QBAR_DZ(M)
      X=DIR(1)
      Y=DIR(2)
      DIR(1)=X*QBAR_CFI(M)-Y*QBAR_SFI(M)
      DIR(2)=Y*QBAR_CFI(M)+X*QBAR_SFI(M)

      END


      SUBROUTINE TO_LOCAL(R,DIR,M)

c--   transformation from global (Belle II) to local (bar) frame

      IMPLICIT NONE
      REAL R(3), DIR(3)         ! point, direction (or momentum vector)
      INTEGER M                 ! bar ID
#include "TOP_GEO.fi"
      REAL X, Y

      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN

!     global to nominal: rotation around z by -phi then translation by (-dx,-r,-dz)

      X=R(1)*QBAR_CFI(M)+R(2)*QBAR_SFI(M)
      Y=R(2)*QBAR_CFI(M)-R(1)*QBAR_SFI(M)
      R(1)=X-QBAR_DX(M)
      R(2)=Y-QBAR_R(M)
      R(3)=R(3)-QBAR_DZ(M)
      X=DIR(1)
      Y=DIR(2)
      DIR(1)=X*QBAR_CFI(M)+Y*QBAR_SFI(M)
      DIR(2)=Y*QBAR_CFI(M)-X*QBAR_SFI(M)

      CALL NOMINAL_TO_LOCAL(R,DIR,M)

      END


      SUBROUTINE LOCAL_TO_NOMINAL(R,DIR,M)

c--   transformation from local to nominal bar frame
c--   rotation first then translation
      
      IMPLICIT NONE
      REAL R(3), DIR(3)         ! point, direction (or momentum vector)
      INTEGER M                 ! bar ID
#include "TOP_GEO.fi"
#include "TOP_DGEO.fi"
      REAL RR(3), S
      INTEGER I,J
      
      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN
      IF(.NOT.DGEO_FLAG(M)) RETURN

      DO I=1,3
         S=0
         DO J=1,3
            S=S+QBAR_ROT(I,J,M)*R(J)
         ENDDO      
         RR(I)=S+QBAR_TRS(I,M)
      ENDDO
      R=RR

      DO I=1,3
         S=0
         DO J=1,3
            S=S+QBAR_ROT(I,J,M)*DIR(J)
         ENDDO      
         RR(I)=S
      ENDDO
      DIR=RR

      END
      

      SUBROUTINE NOMINAL_TO_LOCAL(R,DIR,M)

c--   transformation from nominal to local bar frame

      IMPLICIT NONE
      REAL R(3), DIR(3)         ! point, direction (or momentum vector)
      INTEGER M                 ! bar ID
#include "TOP_GEO.fi"
#include "TOP_DGEO.fi"
      REAL RR(3), S
      INTEGER I,J
      
      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN
      IF(.NOT.DGEO_FLAG(M)) RETURN

      DO I=1,3
         R(I)=R(I)-QBAR_TRS(I,M)
      ENDDO
      DO I=1,3
         S=0
         DO J=1,3
            S=S+QBAR_ROT(J,I,M)*R(J)
         ENDDO      
         RR(I)=S
      ENDDO
      R=RR
      
      DO I=1,3
         S=0
         DO J=1,3
            S=S+QBAR_ROT(J,I,M)*DIR(J)
         ENDDO      
         RR(I)=S
      ENDDO
      DIR=RR
      
      END
      

      SUBROUTINE PRESEK_HELIX_TOP(HLX,RHO,T1,T2,OK)

!     crossing of helix and TOP volume

      IMPLICIT NONE
      REAL HLX(10)              ! helix
      REAL RHO                  ! radius particle is
      REAL T1, T2               ! crossing times
      LOGICAL OK                ! T, if helix crosses TOP volume
#include "TOP_GEO.fi"
      REAL T3, T4, Z1, Z2
      LOGICAL OK1

      OK=.FALSE.
      IF(RHO.GE.TOP_R2) RETURN

      CALL XSEC_CYL(HLX,TOP_R1,T1,T2,OK)
      IF(.NOT.OK) RETURN
      IF(RHO.GE.TOP_R1) THEN
         T2=T1
         T1=0.
      ENDIF
      CALL XSEC_CYL(HLX,TOP_R2,T3,T4,OK1)
      IF(OK1) T2=T3

!     z-clip
      IF(HLX(4).NE.0) THEN
         Z1=HLX(5)*T1+HLX(7)
         Z2=HLX(5)*T2+HLX(7)
      ELSE
         Z1=HLX(5)*T1+HLX(1)
         Z2=HLX(5)*T2+HLX(1)
      ENDIF
      OK=MAX(Z1,Z2).GT.TOP_Z1
      IF(.NOT.OK) RETURN
      OK=MIN(Z1,Z2).LT.TOP_Z2
      IF(.NOT.OK) RETURN

      RETURN
      END


      SUBROUTINE TRACK2TOP(R0,PV,Q,TOF,MDN)

      IMPLICIT NONE
      REAL R0(3)                ! start point [in], at TOP [out]
      REAL PV(3)                ! momentum vector [in/out]
      REAL Q                    ! charge (+/-1) [in]
      REAL TOF                  ! time-of-flight for beta=1 from R0 to TOP [out]
      INTEGER MDN               ! Q-bar ID or -1
#include "TOP_GEO.fi"
      REAL HLX(10), HLX_LOC(10)
      REAL R(3), DIR(3), P, T1, T2, RHO
      INTEGER I, K
      LOGICAL OK

      MDN=-1
      P=SQRT(PV(1)**2+PV(2)**2+PV(3)**2)
      IF(P.LT.1E-4) RETURN
      DO I=1,3
         DIR(I)=PV(I)/P
      ENDDO
      CALL SET_HELIX(R0,DIR,P,Q,HLX)
      RHO=SQRT(R0(1)**2+R0(2)**2)
      TOF=0
      IF(RHO.NE.TOP_R1) THEN
         CALL XSEC_CYL(HLX,TOP_R1,T1,T2,OK)
         IF(.NOT.OK) RETURN
         IF(RHO.LT.TOP_R1) THEN
            TOF=T1
         ELSE
            TOF=T2-HLX(10)
         ENDIF
         CALL GET_HLX_RDIR(HLX,TOF,R0,DIR)
         DO I=1,3
            PV(I)=DIR(I)*P
         ENDDO
      ENDIF

      DO K=0,NUM_MOD-1
         CALL HLX_TO_LOCAL(HLX,K,HLX_LOC)
         CALL XSEC_PLANE_Y(HLX_LOC,0.,T1,T2,OK)
         IF(.NOT.OK) CYCLE
         CALL GET_HLX_RDIR(HLX_LOC,T1,R,DIR)
         OK=ABS(R(1)).LT.QBAR_A(K)/2.
         IF(.NOT.OK) CYCLE
         OK=ABS(R(3)).LT.QBAR_C(K)/2.
         IF(.NOT.OK) CYCLE
         MDN=K
         TOF=T1
         CALL GET_HLX_RDIR(HLX,T1,R0,DIR)
         DO I=1,3
            PV(I)=DIR(I)*P
         ENDDO
         RETURN
      ENDDO

      END


      SUBROUTINE GET_QBAR(ID,A,B,C,LSIDE,RSIDE,LEXPV,REXPV)

      IMPLICIT NONE
      INTEGER ID
      REAL A, B, C
      INTEGER LSIDE, RSIDE
      INTEGER LEXPV, REXPV
#include "TOP_GEO.fi"

      A=0.
      B=0.
      C=0.
      LSIDE=0
      RSIDE=0
      LEXPV=0
      REXPV=0
      IF(ID.LT.0.OR.ID.GE.NUM_MOD) RETURN

      A=QBAR_A(ID)
      B=QBAR_B(ID)
      C=QBAR_C(ID)
      LSIDE=QBAR_LSIDE(ID)
      RSIDE=QBAR_RSIDE(ID)
      LEXPV=QBAR_EXT(0,ID)
      REXPV=QBAR_EXT(1,ID)
      END


      SUBROUTINE GET_MIRROR(ID,R,XC,YC,ZC,F)

      IMPLICIT NONE
      INTEGER ID
      REAL R, XC, YC, ZC, F
#include "TOP_GEO.fi"

      R=0
      XC=0
      YC=0
      ZC=0
      F=0
      IF(ID.LT.0.OR.ID.GE.NUM_MOD) RETURN

      R=QBAR_RMI(ID)
      XC=QBAR_XCMI(ID)
      YC=QBAR_YCMI(ID)
      ZC=QBAR_ZCMI(ID)
      F=QBAR_FOCL(ID)
      END


      SUBROUTINE GET_EXPVOL(ID,DZ,YUP,YDN,YUPIN,YDNIN,A,SHAPE,LR)
      
      IMPLICIT NONE
      INTEGER ID
      REAL DZ, YUP, YDN, YUPIN, YDNIN, A
      INTEGER SHAPE, LR
#include "TOP_EXT.fi"

      DZ=0
      YUP=0
      YDN=0
      SHAPE=-1
      LR=-1
      IF(ID.LT.1.OR.ID.GT.NUM_EXT) RETURN

      DZ=EXT_DZ(ID)
      YUP=EXT_YUP(ID)
      YDN=EXT_YDN(ID)
      YUPIN=EXT_YUPIN(ID)
      YDNIN=EXT_YDNIN(ID)
      A=EXT_A(ID)
      SHAPE=EXT_SHAPE(ID)
      LR=EXT_LR(ID)
      END



      REAL FUNCTION GET_BBWIN(ID)

      IMPLICIT NONE
      INTEGER ID                ! module ID

#include "TOP_GEO.fi"

      IF(ID.GE.0.AND.ID.LT.NUM_MOD) THEN
         GET_BBWIN=QBAR_WIN(ID)
      ELSE
         GET_BBWIN=0.
      ENDIF

      END



