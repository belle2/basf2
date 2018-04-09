!-------------------------------------------------------------------------
! TOP counter for Belle: Reconstruction - basic functions to construct PDF 
! M. Staric, nov-2008, jun-2009, jan-2010, oct-2011, feb-2013, apr-2014
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm 
! mar-2017: change in some commons
! feb-2018: change in some commons
!-------------------------------------------------------------------------

      INTEGER FUNCTION TOPF_PMT(XD,ZD,ZB,IEP,ICA,H)

      IMPLICIT NONE
      REAL XD, ZD               ! unfolded detector coordinates [in]
      REAL ZB                   ! bar-end
      INTEGER IEP, ICA          ! index of emission point and Cer. angle [in]
      REAL H                    ! step for numerical calc. of derivatives
#include "TOP_GEO.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_TOPF.fi"
      INTEGER I, K, TOPSOLU
      REAL X0, Y0, Z0
      REAL DX, DZ, DXDZ, DYDZ, AH, BH
      LOGICAL OK, TOTX, TOTY

      TOPF_PMT=0

      X0=T_R(1,IEP)
      Y0=T_R(2,IEP)
      Z0=T_R(3,IEP)
      DX=XD-X0
      DZ=ZD-Z0
      IF(DZ.EQ.0) RETURN
      DXDZ=DX/DZ
      K=TOPSOLU(DXDZ,IEP,ICA)
      IF(K.EQ.0) RETURN

      F_H(K)=H
      DO I=1,2
         F_TL(I,K)=DZ/F_KZ(I,K)
         F_XD(I,K)=XD
         DYDZ=F_KY(I,K)/F_KZ(I,K)
         F_YD(I,K)=Y0+DYDZ*(ZD-Z0)
         F_YB(I,K)=Y0+DYDZ*(ZB-Z0)
         F_YM(I,K)=0.
         F_NXM(I,K)=0
         F_NYM(I,K)=0
         IF(K.EQ.1) THEN
            AH=QBAR_A(MODUL)/2
            BH=QBAR_B(MODUL)/2
            TOTX=ABS(F_KX(I,K)).LT.PTOT.OR.ABS(XD).LT.AH
            TOTY=ABS(F_KY(I,K)).LT.PTOT.OR.ABS(F_YB(I,K)).LT.BH
            F_OK(I)=F_TL(I,K).GT.0.AND.TOTX.AND.TOTY
         ENDIF
      ENDDO
      OK=F_OK(1).OR.F_OK(2)
      IF(.NOT.OK) RETURN

      TOPF_PMT=K

      END


      INTEGER FUNCTION TOPF_CYL(XD,ZD,ZB,ZM,IEP,ICA,H)

      IMPLICIT NONE
      REAL XD, ZD               ! unfolded detector coordinates [in]
      REAL ZB, ZM               ! bar-end, mirror position
      INTEGER IEP, ICA          ! index of emission point and Cer. angle [in]
      REAL H                    ! step for numerical calc. of derivatives
#include "TOP_GEO.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_TOPF.fi"
      INTEGER I, K, TOPSOLU
      REAL X0, Y0, Z0, XM
      REAL DX, DZ, DXDZ, DYDZ, AH, BH
      LOGICAL OK, TOTX, TOTY
      REAL YY
      
      TOPF_CYL=0

      X0=T_R(1,IEP)
      Y0=T_R(2,IEP)
      Z0=T_R(3,IEP)
      DX=XD-X0
      DZ=ZD-Z0
      IF(DZ.EQ.0) RETURN
      DXDZ=DX/DZ
      K=TOPSOLU(DXDZ,IEP,ICA)
      IF(K.EQ.0) RETURN

      F_H(K)=H
      DO I=1,2
         F_TL(I,K)=DZ/F_KZ(I,K)
         F_XD(I,K)=XD
         DYDZ=F_KY(I,K)/F_KZ(I,K)
         XM=X0+DXDZ*(ZM-Z0)
         F_YM(I,K)=Y0+DYDZ*(ZM-Z0)
         F_NXM(I,K)=NINT(XM/QBAR_A(MODUL))
         F_NYM(I,K)=NINT(F_YM(I,K)/QBAR_B(MODUL))
         YY=Y0+DYDZ*(ZB-Z0)
         F_NY(I,K)=NINT(YY/QBAR_B(MODUL))
         IF(MOD(F_NYM(I,1),2).NE.0) DYDZ=-DYDZ
         F_YD(I,K)=QBAR_YCMI(MODUL)+DYDZ*(ZD-ZM)
         F_YB(I,K)=QBAR_YCMI(MODUL)+DYDZ*(ZB-ZM)
         IF(K.EQ.1) THEN
            AH=QBAR_A(MODUL)/2
            BH=QBAR_B(MODUL)/2
            TOTX=ABS(F_KX(I,K)).LT.PTOT.OR.ABS(XD).LT.AH
            TOTY=ABS(F_KY(I,K)).LT.PTOT.OR.ABS(YY).LT.BH
            F_OK(I)=F_TL(I,K).GT.0.AND.TOTX.AND.TOTY
         ENDIF
      ENDDO
      OK=F_OK(1).OR.F_OK(2)
      IF(.NOT.OK) RETURN

      TOPF_CYL=K

      END


      INTEGER FUNCTION TOPF_SPHER(XD,ZD,ZB,NXM,IEP,ICA,H)

      IMPLICIT NONE
      REAL XD, ZD               ! unfolded detector coordinates [in]
      REAL ZB                   ! bar-end, mirror position
      INTEGER NXM               ! x-reflections before mirror
      INTEGER IEP, ICA          ! index of emission point and Cer. angle [in]
      REAL H                    ! step for numerical calc. of derivatives
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"
#include "TOP_PIK.fi"
#include "TOP_TOPF.fi"
#include "TOP_CONS.fi"
#include "TOP_PAR.fi"
      INTEGER I, K, TOPSOLU
      REAL R0(3), DIR(3), RD(3), YM, YB, YD, TL, AH, BH, TIM
      REAL RM(3), NORM(3), DXDZ
      INTEGER NYM, NY2, NY
      LOGICAL OK, TOTX1, TOTY1, TOTX2, TOTY2

      TOPF_SPHER=0

      R0(1)=T_R(1,IEP)
      R0(2)=T_R(2,IEP)
      R0(3)=T_R(3,IEP)
      CALL GET_RM(XD,ZD,R0(1),R0(3),NXM,RM,DXDZ,NORM,OK)
      IF(.NOT.OK) RETURN

      K=TOPSOLU(DXDZ,IEP,ICA)
      IF(K.EQ.0) RETURN

      F_H(K)=H
      DO I=1,2
         IF(K.EQ.1) THEN
            F_OK(I)=.FALSE.
         ELSE
            IF(.NOT.F_OK(I)) CYCLE
         ENDIF
         DIR(1)=F_KX(I,K)
         DIR(2)=F_KY(I,K)
         DIR(3)=F_KZ(I,K)
         CALL AT_NEW_Z(R0,DIR,RM(3),TL,YM,NYM)
         IF(K.EQ.1.AND.TL.LT.0) CYCLE
         F_TL(I,K)=TL
         F_YM(I,K)=YM
         F_NXM(I,K)=NXM
         F_NYM(I,K)=NYM
         RM(2)=QBAR_YCMI(MODUL)
         IF(MOD(F_NYM(I,1),2).NE.0) DIR(2)=-DIR(2)
         CALL REFLECTION(DIR,NORM,DIR)
         CALL AT_NEW_Z(RM,DIR,ZB,TL,YB,NY2)
         F_YB(I,K)=YB
         IF(MOD(NYM,2).NE.0) NY2=-NY2
         F_NY(I,K)=NYM+NY2
         CALL AT_NEW_Z(RM,DIR,ZD,TL,YD,NY)
         IF(K.EQ.1.AND.TL.LT.0) CYCLE
         F_TL(I,K)=F_TL(I,K)+TL
         IF(K.EQ.1) THEN
            AH=QBAR_A(MODUL)/2
            BH=QBAR_B(MODUL)/2
            TOTX1=ABS(F_KX(I,K)).LT.PTOT.OR.NXM.EQ.0
            TOTX2=ABS(DIR(1)).LT.PTOT.OR.ABS(XD).LT.AH
            TOTY1=ABS(F_KY(I,K)).LT.PTOT.OR.NYM.EQ.0
            TOTY2=ABS(DIR(2)).LT.PTOT.OR.ABS(YB).LT.BH
            TIM=F_TL(I,K)*GRU_IND/C0+TOF
            F_OK(I)=TOTX1.AND.TOTY1.AND.TOTX2.AND.TOTY2.AND.TIM.LE.T_MAX
         ENDIF
         F_XD(I,K)=XD
         F_YD(I,K)=YD
         F_NYE(I,K)=0
      ENDDO
      OK=F_OK(1).OR.F_OK(2)
      IF(.NOT.OK) RETURN

      TOPF_SPHER=K

      END


      INTEGER FUNCTION TOPF_RAYTR(I0,K0,DFICDX)

      IMPLICIT NONE
      INTEGER I0, K0
      REAL DFICDX
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"
#include "TOP_PIK.fi"
#include "TOP_TOPF.fi"
      INTEGER I, K
      REAL XD, X, Y, X1, Y1, X2, Y2, DELX, Y0
      REAL TL1, TL2
      REAL GET_XD_RAYTR
      LOGICAL OK, TOT
      REAL Y_PREC /0.01/
      SAVE Y_PREC

      TOPF_RAYTR=0
      K=NT+1
      IF(K.GT.NB) THEN
         PRINT*,'topf_raytr: TOP_TOPF is full'
         RETURN
      ENDIF
      F_X0(K)=F_X0(K0)
      F_Y0(K)=F_Y0(K0)
      F_Z0(K)=F_Z0(K0)
      F_H(K)=0

      XD=F_XD(I0,K0)
      X=ATAN2(F_SFIC(I0,K0),F_CFIC(I0,K0))
      Y=GET_XD_RAYTR(X,I0,K,OK,TOT)-XD
      IF(.NOT.OK) RETURN
      IF(ABS(Y).LT.Y_PREC) THEN
         IF(TOT) TOPF_RAYTR=K
         RETURN
      ENDIF         
      X1=X
      Y1=Y
      TL1=F_TL(I0,K)

      I=0
      DELX=DFICDX*Y1
      DO WHILE(.TRUE.)
         X=X-DELX
         Y=GET_XD_RAYTR(X,I0,K,OK,TOT)-XD
         IF(.NOT.OK) RETURN
         IF(ABS(Y).LT.Y_PREC) THEN
            IF(TOT) TOPF_RAYTR=K
            RETURN
         ENDIF         
         IF(Y1*Y.LT.0) EXIT
         I=I+1
         IF(I.GT.20) RETURN
         X1=X
         Y1=Y
         TL1=F_TL(I0,K)
      ENDDO
      X2=X
      Y2=Y
      TL2=F_TL(I0,K)

      DO I=1,40
         Y0=Y
         X=X1-(X2-X1)/(Y2-Y1)*Y1
         Y=GET_XD_RAYTR(X,I0,K,OK,TOT)-XD
         IF(.NOT.OK) RETURN
         IF(ABS(Y).LT.Y_PREC) THEN
            IF(TOT) TOPF_RAYTR=K
            RETURN
         ENDIF         
         IF(Y.EQ.Y0) EXIT
         IF(Y*Y1.LT.0) THEN
            X2=X
            Y2=Y
            TL2=F_TL(I0,K)
         ELSE
            X1=X
            Y1=Y
            TL1=F_TL(I0,K)
         ENDIF
      ENDDO
      IF(ABS(Y).LT.5.AND.TOT) THEN
         F_TL(I0,K)=TL1+(TL2-TL1)/(Y2-Y1)*(0-Y1)
         TOPF_RAYTR=K
      ENDIF

      END



      REAL FUNCTION GET_XD_RAYTR(FIC,I0,K,OK,TOT)

      IMPLICIT NONE
      REAL FIC
      INTEGER I0, K
      LOGICAL OK, TOT
#include "TOP_TRACK.fi"
#include "TOP_TOPF.fi"
#include "TOP_PIK.fi"

      REAL RF(3), CFIC, SFIC, AA, BB
      REAL TL, XD, YD, YB, YM
      INTEGER NX(3), NY(3)

      RF(1)=F_X0(K)
      RF(2)=F_Y0(K)
      RF(3)=F_Z0(K)
      CFIC=COS(FIC)
      SFIC=SIN(FIC)
      F_CFIC(I0,K)=CFIC
      F_SFIC(I0,K)=SFIC
      AA=T_CTH(0)*S_THC(0)*CFIC+T_STH(0)*C_THC(0)
      BB=S_THC(0)*SFIC
      F_DIR(1)=AA*T_CFI(0)-BB*T_SFI(0)
      F_DIR(2)=AA*T_SFI(0)+BB*T_CFI(0)
      F_DIR(3)=T_CTH(0)*C_THC(0)-T_STH(0)*S_THC(0)*CFIC
      F_KX(I0,K)=F_DIR(1)
      F_KY(I0,K)=F_DIR(2)
      F_KZ(I0,K)=F_DIR(3)
      CALL FAST_RAYTR(RF,F_DIR,MODUL,TL,NX,NY,XD,YD,YB,YM,OK,TOT)
      F_TL(I0,K)=TL
      F_XH(I0,K)=RF(1)
      F_YH(I0,K)=RF(2)
      F_XD(I0,K)=XD
      F_YD(I0,K)=YD
      F_YB(I0,K)=YB
      F_YM(I0,K)=YM
      F_NX(I0,K)=NX(1)
      F_NY(I0,K)=NY(1)
      F_NXM(I0,K)=NX(2)
      F_NYM(I0,K)=NY(2)
      F_NXE(I0,K)=NX(3)
      F_NYE(I0,K)=NY(3)
      
      GET_XD_RAYTR=XD

      END


      REAL FUNCTION TOPF_DTL(I,K1,K2)

      IMPLICIT NONE
      INTEGER I, K1, K2
#include "TOP_TOPF.fi"
      REAL DELH, DELF

      DELF=F_TL(I,K2)-F_TL(I,K1)
      DELH=F_H(K2)-F_H(K1)
      IF(DELH.NE.0) THEN
         TOPF_DTL=DELF/DELH
      ELSE
         TOPF_DTL=0.
         PRINT*,'topf_dtl: DELH=0',K1,K2
      ENDIF

      END


      REAL FUNCTION TOPF_DYD(I,K1,K2)

      IMPLICIT NONE
      INTEGER I, K1, K2
#include "TOP_TOPF.fi"
      REAL DELH, DELF

      DELF=F_YD(I,K2)-F_YD(I,K1)
      DELH=F_H(K2)-F_H(K1)
      IF(DELH.NE.0) THEN
         TOPF_DYD=DELF/DELH
      ELSE
         TOPF_DYD=0.
         PRINT*,'topf_dyd: DELH=0',K1,K2
      ENDIF

      END


      REAL FUNCTION TOPF_DYB(I,K1,K2)

      IMPLICIT NONE
      INTEGER I, K1, K2
#include "TOP_TOPF.fi"
      REAL DELH, DELF

      DELF=F_YB(I,K2)-F_YB(I,K1)
      DELH=F_H(K2)-F_H(K1)
      IF(DELH.NE.0) THEN
         TOPF_DYB=DELF/DELH
      ELSE
         TOPF_DYB=0.
         PRINT*,'topf_dyd: DELH=0',K1,K2
      ENDIF

      END


      REAL FUNCTION TOPF_DYM(I,K1,K2)

      IMPLICIT NONE
      INTEGER I, K1, K2
#include "TOP_TOPF.fi"
      REAL DELH, DELF

      DELF=F_YM(I,K2)-F_YM(I,K1)
      DELH=F_H(K2)-F_H(K1)
      IF(DELH.NE.0) THEN
         TOPF_DYM=DELF/DELH
      ELSE
         TOPF_DYM=0.
         PRINT*,'topf_dyd: DELH=0',K1,K2
      ENDIF

      END


      REAL FUNCTION TOPF_DFIC(I,K1,K2)

      IMPLICIT NONE
      INTEGER I, K1, K2
#include "TOP_TOPF.fi"
      REAL DELH, DELF

      IF(ABS(F_CFIC(I,K1)).GT.ABS(F_SFIC(I,K1))) THEN
         DELF=(F_SFIC(I,K2)-F_SFIC(I,K1))/F_CFIC(I,K1)
      ELSE
         DELF=-(F_CFIC(I,K2)-F_CFIC(I,K1))/F_SFIC(I,K1)
      ENDIF
      DELH=F_H(K2)-F_H(K1)
      IF(DELH.NE.0) THEN
         TOPF_DFIC=DELF/DELH
      ELSE
         TOPF_DFIC=0.
         PRINT*,'topf_dfic: DELH=0',K1,K2
      ENDIF

      END


      INTEGER FUNCTION TOPSOLU(DXDZ,IEP,ICA)

c--   solve for cos(fic), sin(fic), kx, ky, kz; if OK, increment nt; store @ nt

      IMPLICIT NONE
      REAL DXDZ                 ! dx/dz [in]
      INTEGER IEP, ICA          ! index of emission point and Cer. angle [in]
#include "TOP_TRACK.fi"
#include "TOP_PIK.fi"
#include "TOP_TOPF.fi"
      LOGICAL OK
      REAL CTH, STH, CFI, SFI
      REAL CTHC, STHC
      REAL A, B, D, BB, DD, AB, AA
      INTEGER I, K
      real KZ_MIN
      PARAMETER (KZ_MIN=1.E-5)

      TOPSOLU=0

      CTH=T_CTH(IEP)
      STH=T_STH(IEP)
      CFI=T_CFI(IEP)
      SFI=T_SFI(IEP)
      CTHC=C_THC(ICA)
      STHC=S_THC(ICA)

      A=(DXDZ*CTH-CFI*STH)*CTHC
      B=(DXDZ*STH+CFI*CTH)*STHC
      D=SFI*STHC
      BB=B**2+D**2
      OK=BB.NE.0
      IF(.NOT.OK) RETURN

      K=NT+1
      OK=K.LE.NB
      IF(.NOT.OK) THEN
         PRINT*,'topsolu: TOP_TOPF is full'
         RETURN
      ENDIF
      
      IF(D.EQ.0) THEN
         F_CFIC(1,K)=A/B
         OK=ABS(F_CFIC(1,K)).LE.1
         IF(.NOT.OK) RETURN
         F_SFIC(1,K)=SQRT(1.-F_CFIC(1,K)**2)
         F_CFIC(2,K)=F_CFIC(1,K)
         F_SFIC(2,K)=-F_SFIC(1,K)
      ELSE
         DD=BB-A**2
         OK=DD.GE.0
         IF(.NOT.OK) RETURN
         DD=D*SQRT(DD)
         AB=A*B
         F_CFIC(1,K)=(AB+DD)/BB
         F_CFIC(2,K)=(AB-DD)/BB
         DO I=1,2
            F_SFIC(I,K)=(B*F_CFIC(I,K)-A)/D
         ENDDO
      ENDIF

      DO I=1,2
         AA=CTH*STHC*F_CFIC(I,K)+STH*CTHC
         BB=STHC*F_SFIC(I,K)
         F_KX(I,K)=AA*CFI-BB*SFI
         F_KY(I,K)=AA*SFI+BB*CFI
         F_KZ(I,K)=CTH*CTHC-STH*STHC*F_CFIC(I,K)
         IF(ABS(F_KZ(I,K)).LT.KZ_MIN) F_KZ(I,K)=SIGN(KZ_MIN,F_KZ(I,K))
      ENDDO
      F_X0(K)=T_R(1,IEP)
      F_Y0(K)=T_R(2,IEP)
      F_Z0(K)=T_R(3,IEP)
      NT=K
      TOPSOLU=NT

      END


      SUBROUTINE AT_NEW_Z(R0,DIR,Z,TL,Y,NY)

      IMPLICIT NONE
      REAL R0(3), DIR(3)        ! 3D line [in]
      REAL Z                    ! z [in]
      REAL TL                   ! path length to z [out]
      REAL Y                    ! y at z [out]
      INTEGER NY                ! num of reflections in y [out]
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"

      TL=(Z-R0(3))/DIR(3)
      Y=R0(2)+TL*DIR(2)
      NY=NINT(Y/QBAR_B(MODUL))

      END


      SUBROUTINE GET_RM(XD,ZD,X0,Z0,NXM,RM,DXDZ,NORM,OK)

      IMPLICIT NONE
      REAL XD, ZD               ! detection point (unfolded) [in]
      REAL X0, Z0               ! emission point [in]
      INTEGER NXM               ! x-reflections before mirror
      REAL RM(3)                ! point on mirror [out]
      REAL DXDZ                 ! slope dx/dz [out]
      REAL NORM(3)              ! mirror normal at RM
      LOGICAL OK                ! status [out]
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"
      REAL XE, ZE, XM, ZM
      REAL UNFOLD

      IF(MOD(NXM,2).EQ.0) THEN
         XE=UNFOLD(X0,-NXM,QBAR_A(MODUL))
         ZE=Z0
         CALL FIND_RM(XE,ZE,XD,ZD,XM,ZM,OK)
         DXDZ=(XM-XE)/(ZM-ZE)
         RM(1)=XM
         RM(2)=0
         RM(3)=ZM
         NORM(1)=(XM-QBAR_XCMI(MODUL))/QBAR_RMI(MODUL)
         NORM(2)=0
         NORM(3)=QBAR_MIPOS(MODUL)*SQRT(1-NORM(1)**2)
      ELSE
         XE=UNFOLD(X0,NXM,QBAR_A(MODUL))
         ZE=Z0
         CALL FIND_RM(XE,ZE,-XD,ZD,XM,ZM,OK)
         DXDZ=-(XM-XE)/(ZM-ZE)
         RM(1)=-XM
         RM(2)=0
         RM(3)=ZM
         NORM(1)=-(XM-QBAR_XCMI(MODUL))/QBAR_RMI(MODUL)
         NORM(2)=0
         NORM(3)=QBAR_MIPOS(MODUL)*SQRT(1-NORM(1)**2)
      ENDIF

      END


      SUBROUTINE FIND_RM(XE,ZE,XD,ZD,XM,ZM,OK)

      IMPLICIT NONE
      REAL XE, ZE               ! emission point (unfolded) [in]
      REAL XD, ZD               ! detection point (unfolded) [in]
      REAL XM, ZM               ! point at mirror [out]
      LOGICAL OK                ! status [out]
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"
      REAL AH, X, X1, X2 
      REAL Y, Y0, Y1, Y2
      REAL XMIRR_REFLECT_2D
      INTEGER I

      XM=0
      ZM=QBAR_ZCMI(MODUL)+QBAR_MIPOS(MODUL)*QBAR_RMI(MODUL)
      AH=QBAR_A(MODUL)/2
      X1=-AH
      X2=AH
      Y1=XMIRR_REFLECT_2D(XE,ZE,X1,ZD,OK)-XD
      IF(.NOT.OK) RETURN
      Y2=XMIRR_REFLECT_2D(XE,ZE,X2,ZD,OK)-XD
      IF(.NOT.OK) RETURN
      OK=Y1*Y2.LT.0
      IF(.NOT.OK) RETURN
      Y=Y1
      DO I=1,40
         Y0=Y
         X=X1-(X2-X1)/(Y2-Y1)*Y1
         Y=XMIRR_REFLECT_2D(XE,ZE,X,ZD,OK)-XD
         IF(.NOT.OK) RETURN
         IF(Y.EQ.0) EXIT
         IF(Y.EQ.Y0) EXIT
         IF(Y*Y1.LT.0) THEN
            X2=X
            Y2=Y
         ELSE
            X1=X
            Y1=Y
            IF(Y1.EQ.Y2) EXIT
         ENDIF
      ENDDO
      XM=X
      ZM=QBAR_ZCMI(MODUL)+QBAR_MIPOS(MODUL)*
     &     SQRT(QBAR_RMI(MODUL)**2-(XM-QBAR_XCMI(MODUL))**2)

      END


      REAL FUNCTION XMIRR_REFLECT_2D(X0,Z0,XM,Z,OK) 

c--   returns X at Z after reflection

      IMPLICIT NONE
      REAL X0, Z0               ! start point
      REAL XM                   ! x-coordinate at mirror
      REAL Z                    ! end point z-coordinate
      LOGICAL OK                ! status
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"
      REAL R0(2), DIR(2), RM(2), NORM(2), RC(2), RMI, S, X
      INTEGER I

      R0(1)=X0
      R0(2)=Z0

      RC(1)=QBAR_XCMI(MODUL)
      RC(2)=QBAR_ZCMI(MODUL)
      RMI=QBAR_RMI(MODUL)

      NORM(1)=(XM-RC(1))/RMI
      NORM(2)=QBAR_MIPOS(MODUL)*SQRT(1-NORM(1)**2)
      DO I=1,2
         RM(I)=RC(I)+RMI*NORM(I)
         DIR(I)=RM(I)-R0(I)
      ENDDO
      S=2*(DIR(1)*NORM(1)+DIR(2)*NORM(2))
      DO I=1,2
         DIR(I)=DIR(I)-S*NORM(I)
      ENDDO
      OK=DIR(2).NE.0
      IF(.NOT.OK) RETURN
      S=(Z-RM(2))/DIR(2)
      OK=S.GT.0
      IF(.NOT.OK) RETURN
      X=RM(1)+S*DIR(1)
      XMIRR_REFLECT_2D=X
      
      END


c--   routines for fast ray-tracing of a photon

      SUBROUTINE FAST_RAYTR(RF,DIR,MDN,TLEN,NX,NY,XD,YD,YB,YM,OK,TOT)

c--   propagate photon from emission point to detection plane

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! photon [in/out]
      INTEGER MDN               ! bar ID [in]
      REAL TLEN                 ! propagation length [out]
      INTEGER NX(3), NY(3)      ! number of reflections [out]
      REAL XD, YD, YB, YM       ! unfolded hit coordinates [out]
      LOGICAL OK                ! status [out]
      LOGICAL TOT               ! total reflection [out]
#include "TOP_GEO.fi" 
#include "TOP_PIK.fi" 
#include "TOP_TOPF.fi"
      INTEGER TYP, I, ISNX, ISNY
      REAL XM, XB, XE, YE, TL
      INTEGER LR, NX1, NY1
      LOGICAL TOTX, TOTY
      
      TLEN=0
      DO I=1,3
         NX(I)=0
         NY(I)=0
      ENDDO
      YM=0
      ISNX=1
      IF(DIR(1).LT.0) ISNX=-1
      ISNY=1
      IF(DIR(2).LT.0) ISNY=-1
      OK=.FALSE.
      TOT=.TRUE.

      IF(DIR(3).GT.0) THEN
         TYP=QBAR_RSIDE(MDN)
         LR=1
      ELSE
         TYP=QBAR_LSIDE(MDN)
         LR=0
      ENDIF
      IF(TYP.EQ.0) RETURN

      CALL UNFOLDED_RESET
      IF(TYP.GT.1) THEN
         TOTX=ABS(DIR(1)).LT.PTOT
         TOTY=ABS(DIR(2)).LT.PTOT
         CALL MIRR_RAYTR(RF,DIR,MDN,TL,NX1,NY1,XM,YM,OK)
         IF(.NOT.OK) RETURN
         TOT=TOT.AND.(TOTX.OR.NX1.EQ.0).AND.(TOTY.OR.NY1.EQ.0)
         LR=1-LR
         CALL UNFOLDED_PUSH(2,RF,NX1,NY1,MDN,LR,TYP)
         TLEN=TLEN+TL
         NX(2)=SIGN(NX1,ISNX)
         NY(2)=SIGN(NY1,ISNY)
         NX(1)=NX(2)
         NY(1)=NY(2)
      ENDIF

      TOTX=ABS(DIR(1)).LT.PTOT
      TOTY=ABS(DIR(2)).LT.PTOT
      CALL BAR_RAYTR(RF,DIR,MDN,TL,NX1,NY1,XB,YB,OK)
      IF(.NOT.OK) RETURN
      TOT=TOT.AND.(TOTX.OR.NX1.EQ.0).AND.(TOTY.OR.NY1.EQ.0)
      CALL UNFOLDED_PUSH(1,RF,NX1,NY1,MDN,LR,0)
      TLEN=TLEN+TL
      NX(1)=NX(1)+SIGN(NX1,ISNX)
      NY(1)=NY(1)+SIGN(NY1,ISNY)

      DO I=1,3
         F_DIRB(I)=DIR(I)
      ENDDO
      
      IF(QBAR_EXT(LR,MDN).GT.0) THEN
         TOTX=ABS(DIR(1)).LT.PTOT
         TOTY=ABS(DIR(2)).LT.PTOT
         CALL EVOL_RAYTR(RF,DIR,MDN,TL,NX1,NY1,XE,YE,OK)
         IF(.NOT.OK) RETURN
         TOT=TOT.AND.(TOTX.OR.NX1.EQ.0).AND.(TOTY.OR.NY1.EQ.0)
         CALL UNFOLDED_PUSH(3,RF,NX1,NY1,MDN,LR,0)
         TLEN=TLEN+TL
         NX(3)=SIGN(NX1,ISNX)
         NY(3)=SIGN(NY1,ISNY)
         NX(1)=NX(1)+NX(3)
         NY(1)=NY(1)+NY(3)
      ENDIF

      IF(QBAR_WIN(MDN).GT.0) THEN
         TL=ABS(QBAR_WIN(MDN)/DIR(3))
         DO I=1,3
            RF(I)=RF(I)+TL*DIR(I)
         ENDDO
         TLEN=TLEN+TL
         CALL UNFOLDED_REPLACE(RF)
      ENDIF      

      CALL UNFOLDED_GET(XD,YD,YB)

      END


      SUBROUTINE BAR_RAYTR(RF,DIR,MDN,TL,NX,NY,XD,YD,OK)

c--   propagate photon to the bar end 

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! photon [in/out]
      INTEGER MDN               ! bar ID [in]
      REAL TL                   ! propagation length [out]
      INTEGER NX, NY            ! number of reflections [out]
      REAL XD, YD               ! unfolded hit coordinates [out]
      LOGICAL OK                ! status [out]
#include "TOP_GEO.fi" 
      REAL ZD, X, Y

      OK=.FALSE.
      IF(ABS(DIR(3)).LT.1E-5) RETURN

      IF(DIR(3).GT.0) THEN
         ZD=QBAR_C(MDN)/2
      ELSE
         ZD=-QBAR_C(MDN)/2
      ENDIF

      TL=(ZD-RF(3))/DIR(3)
      IF(TL.LT.0) RETURN
      IF(TL.GT.10000) RETURN

      XD=RF(1)+TL*DIR(1)
      YD=RF(2)+TL*DIR(2)
      NX=NINT(XD/QBAR_A(MDN))
      NY=NINT(YD/QBAR_B(MDN))

      X=XD-QBAR_A(MDN)*NX
      IF(MOD(NX,2).NE.0) THEN
         X=-X
         DIR(1)=-DIR(1)
      ENDIF
      Y=YD-QBAR_B(MDN)*NY
      IF(MOD(NY,2).NE.0) THEN
         Y=-Y
         DIR(2)=-DIR(2)
      ENDIF
      RF(1)=X
      RF(2)=Y
      RF(3)=ZD
      OK=.TRUE.
      
      END


      SUBROUTINE MIRR_RAYTR(RF,DIR,MDN,TL,NX,NY,XM,YM,OK)

c--   propagate photon to mirror and do reflection

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! photon [in/out]
      INTEGER MDN               ! bar ID [in]
      REAL TL                   ! propagation length [out]
      INTEGER NX, NY            ! number of reflections [out]
      REAL XM, YM               ! unfolded hit coordinates [out]
      LOGICAL OK                ! status [out]
#include "TOP_GEO.fi" 
      REAL ZM, X, Y, NORM(3)
      INTEGER TYP

      OK=.FALSE.
      IF(ABS(DIR(3)).LT.1E-5) RETURN

      IF(DIR(3).GT.0) THEN
         TYP=QBAR_RSIDE(MDN)
         ZM=QBAR_C(MDN)/2
      ELSE
         TYP=QBAR_LSIDE(MDN)
         ZM=-QBAR_C(MDN)/2
      ENDIF
      IF(TYP.EQ.4) THEN
         CALL ZM_MIRROR(RF,DIR,MDN,ZM,NORM)
         IF(ZM.EQ.0) RETURN
      ENDIF

      TL=(ZM-RF(3))/DIR(3)
      IF(TL.LT.0) RETURN
      IF(TL.GT.10000) RETURN

      XM=RF(1)+TL*DIR(1)
      YM=RF(2)+TL*DIR(2)
      NX=NINT(XM/QBAR_A(MDN))
      NY=NINT(YM/QBAR_B(MDN))

      X=XM-QBAR_A(MDN)*NX
      IF(MOD(NX,2).NE.0) THEN
         X=-X
         DIR(1)=-DIR(1)
      ENDIF
      Y=YM-QBAR_B(MDN)*NY
      IF(MOD(NY,2).NE.0) THEN
         Y=-Y
         DIR(2)=-DIR(2)
      ENDIF

      IF(TYP.EQ.2) THEN
         DIR(3)=-DIR(3)
      ELSE IF(TYP.EQ.3) THEN
         Y=QBAR_YCMI(MDN)
         DIR(3)=-DIR(3)
      ELSE IF(TYP.EQ.4) THEN
         Y=QBAR_YCMI(MDN)
         CALL REFLECTION(DIR,NORM,DIR)
      ENDIF
      RF(1)=X
      RF(2)=Y
      RF(3)=ZM
      OK=.TRUE.
      
      END


      SUBROUTINE ZM_MIRROR(RF,DIR,MDN,ZM,NORM)

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! photon [in/out]
      INTEGER MDN               ! bar ID [in]
      REAL ZM                   ! z coordinate [out]
      REAL NORM(3)              ! normal [out]
#include "TOP_GEO.fi" 
      REAL XC, ZC, RMI, A, XCN
      REAL XM, TL
      REAL X, Z, RDIR, RR, D, SS
      INTEGER I, NX, NXX
      INTEGER MDN_OLD /-1/
      REAL Z_B
      SAVE MDN_OLD, Z_B

      ZM=0

      XC=QBAR_XCMI(MDN)
      ZC=QBAR_ZCMI(MDN)
      RMI=QBAR_RMI(MDN)
      A=QBAR_A(MDN)

      IF(MDN.NE.MDN_OLD) THEN
         MDN_OLD=MDN
         Z_B=MIN(RMI**2-(A/2-XC)**2, RMI**2-(A/2+XC)**2)
         IF(DIR(3).GT.0) THEN
            Z_B=ZC+SQRT(Z_B)
         ELSE
            Z_B=ZC-SQRT(Z_B)
         ENDIF
      ENDIF

      TL=(Z_B-RF(3))/DIR(3)
      IF(TL.LT.0) RETURN
      IF(TL.GT.10000) RETURN
      XM=RF(1)+TL*DIR(1)
      NX=NINT(XM/A)
      DO I=1,2
         IF(MOD(NX,2).EQ.0) THEN
            XCN=A*NX+XC
         ELSE
            XCN=A*NX-XC
         ENDIF
         SS=DIR(1)**2+DIR(3)**2
         IF(SS.EQ.0) RETURN
         X=RF(1)-XCN
         Z=RF(3)-ZC
         RDIR=X*DIR(1)+Z*DIR(3)
         RR=X**2+Z**2
         D=RDIR**2+(RMI**2-RR)*SS
         IF(D.LT.0) RETURN
         D=SQRT(D)
         TL=(D-RDIR)/SS
         IF(TL.LT.0) RETURN
         XM=RF(1)+TL*DIR(1)
         NXX=NINT(XM/A)
         IF(NXX.EQ.NX) EXIT
         NX=NXX
      ENDDO
      ZM=RF(3)+TL*DIR(3)
      NORM(1)=(XM-XCN)/RMI
      NORM(2)=0
      NORM(3)=(ZM-ZC)/RMI
      IF(MOD(NX,2).NE.0) NORM(1)=-NORM(1)

      END


      SUBROUTINE REFLECTION(DIR,NORM,RDIR)

      IMPLICIT NONE
      REAL DIR(3)               ! direction [in]
      REAL NORM(3)              ! surface normal [in]
      REAL RDIR(3)              ! reflected direction [out]
      
      REAL S, S_PROD
      INTEGER I

      S=2*S_PROD(DIR,NORM)
      DO I=1,3
         RDIR(I)=DIR(I)-S*NORM(I)
      ENDDO
      END


      SUBROUTINE EVOL_RAYTR(RF,DIR,MDN,TL,NX,NY,XD,YD,OK)

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! photon [in/out]
      INTEGER MDN               ! bar ID [in]
      REAL TL                   ! propagation length [out]
      INTEGER NX, NY            ! number of reflections [out]
      REAL XD, YD               ! unfolded hit coordinates [out]
      LOGICAL OK                ! status [out]
#include "TOP_GEO.fi" 
#include "TOP_EXT.fi" 
      INTEGER LR, ID, SHAPE
      REAL ZB, ZD, X, Y, YSIZ, Y0, KY, KZ
      INTEGER KMIN, KMAX, K, K2, STEP
      
      OK=.FALSE.
      IF(ABS(DIR(3)).LT.1E-5) RETURN

      IF(DIR(3).GT.0) THEN
         LR=1
         ZB=QBAR_C(MDN)/2
      ELSE
         LR=0
         ZB=-QBAR_C(MDN)/2
      ENDIF
      ID=QBAR_EXT(LR,MDN)
      IF(ID.EQ.0) RETURN
      IF(ABS(RF(1)).GT.EXT_A(ID)/2) RETURN
      IF(RF(2).LT.EXT_YDNIN(ID)) RETURN
      IF(RF(2).GT.EXT_YUPIN(ID)) RETURN

      ZD=EXT_DZ(ID)
      IF(LR.EQ.0) ZD=-ZD
      ZD=ZD+ZB

      TL=(ZD-RF(3))/DIR(3)
      IF(TL.LT.0) RETURN
      IF(TL.GT.10000) RETURN

      XD=RF(1)+TL*DIR(1)
      YD=RF(2)+TL*DIR(2)

      SHAPE=EXT_SHAPE(ID)
      IF(SHAPE.EQ.0) THEN         ! no reflection on +/-y surface
         IF(YD.GT.EXT_YUP(ID)) RETURN
         IF(YD.LT.EXT_YDN(ID)) RETURN
         NX=NINT(XD/EXT_A(ID))
         NY=0
         X=XD-EXT_A(ID)*NX
         IF(MOD(NX,2).NE.0) THEN
            X=-X
            DIR(1)=-DIR(1)
         ENDIF
         Y=YD
      ELSE IF(SHAPE.EQ.1) THEN    ! box shaped
         NX=NINT(XD/EXT_A(ID))
         X=XD-EXT_A(ID)*NX
         IF(MOD(NX,2).NE.0) THEN
            X=-X
            DIR(1)=-DIR(1)
         ENDIF
         YSIZ=EXT_YUP(ID)-EXT_YDN(ID)
         Y0=(EXT_YUP(ID)+EXT_YDN(ID))/2
         Y=YD-Y0
         NY=NINT(Y/YSIZ)
         Y=Y-YSIZ*NY
         IF(MOD(NY,2).NE.0) THEN
            Y=-Y
            DIR(2)=-DIR(2)
         ENDIF
         Y=Y+Y0
      ELSE                      ! prism shaped
         CALL UNFOLD_EXTVOL(MDN,LR,KMIN,KMAX)
         Y0=(EXT_YUP(ID)+EXT_YDN(ID))/2
         IF(YD.GT.Y0) THEN
            K2=KMAX
            STEP=1
         ELSE
            K2=KMIN
            STEP=-1
         ENDIF
         DO K=0,K2,STEP
            CALL EVOL_HIT(RF,DIR,ZD,K,TL,XD,Y,KY,KZ,OK)
            OK=OK.AND.Y.GE.EXT_YDN(ID).AND.Y.LE.EXT_YUP(ID)
            IF(OK) EXIT
         ENDDO
         IF(.NOT.OK) RETURN
         NX=NINT(XD/EXT_A(ID))
         X=XD-EXT_A(ID)*NX
         IF(MOD(NX,2).NE.0) THEN
            X=-X
            DIR(1)=-DIR(1)
         ENDIF
         DIR(2)=KY
         DIR(3)=KZ
         NY=K
         IF(MOD(NY,2).EQ.0) THEN
            YD=NY*YSIZ+Y
         ELSE
            YD=NY*YSIZ-Y
            DIR(3)=-DIR(3)
         ENDIF
      ENDIF

      RF(1)=X
      RF(2)=Y
      RF(3)=ZD
      OK=.TRUE.

      END


      SUBROUTINE EVOL_HIT(RF,DIR,ZD,K,TL,XD,Y,KY,KZ,OK)

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! photon [in/out]
      REAL ZD                   ! exit window z-position
      INTEGER K                 ! reflection number in y [in]
      REAL TL                   ! propagation length [out]
      REAL XD                   ! unfolded x-coordinate
      REAL Y                    ! hit y-coordinate at exit window [out]
      REAL KY, KZ               ! photon direction at exit window [out]
      LOGICAL OK                ! status [out]
      
      REAL SY, SZ, Y0, Z0
      REAL RP(3), NORM(3), R(3), S
      REAL S_PROD
      INTEGER I

      CALL GET_UNFOLD_EXT(K,SY,SZ,Y0,Z0)
      RP(1)=0
      RP(2)=Y0
      RP(3)=Z0+ZD
      NORM(1)=0
      NORM(2)=-SZ
      NORM(3)=SY
      DO I=1,3
         R(I)=RF(I)-RP(I)
      ENDDO
      S=S_PROD(DIR,NORM)
      OK=S.NE.0
      IF(.NOT.OK) RETURN
      TL=-S_PROD(R,NORM)/S
      OK=TL.GT.0
      IF(.NOT.OK) RETURN
      DO I=1,3
         R(I)=RF(I)+TL*DIR(I)-RP(I)
      ENDDO
      XD=R(1)
      Y=R(2)*SY+R(3)*SZ
      KY=DIR(2)*SY+DIR(3)*SZ
      KZ=DIR(2)*(-SZ)+DIR(3)*SY
      
      END


      SUBROUTINE UNFOLDED_PUSH(KJE,RF,NX,NY,MDN,LR,ITIP)

      IMPLICIT NONE
      INTEGER KJE
      REAL RF(3)
      INTEGER NX, NY, MDN, LR, ITIP
      REAL XD, YD, YB

#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
      INTEGER ID, I, K
      REAL XX(2)

      integer NB
      PARAMETER (NB=10)
      INTEGER NT
      REAL X(2,NB), X0(2,NB), A(2,NB)
      INTEGER N(2,NB), TIP(NB)
      SAVE NT, X, X0, A, N, TIP

      NT=NT+1
      IF(NT.GT.NB) RETURN
      X(1,NT)=RF(1)
      X(2,NT)=RF(2)
      N(1,NT)=NX
      N(2,NT)=NY
      TIP(NT)=KJE
      IF(KJE.EQ.1) THEN
         A(1,NT)=QBAR_A(MDN)
         A(2,NT)=QBAR_B(MDN)
         X0(1,NT)=0.
         X0(2,NT)=0.
      ELSE IF(KJE.EQ.2) THEN
         IF(ITIP.EQ.4) THEN
            DO I=1,NT
               A(1,I)=0.
               A(2,I)=0.
               X0(1,I)=0.
               X0(2,I)=0.
               N(2,I)=0
            ENDDO
         ELSE IF(ITIP.EQ.3) THEN
            A(1,NT)=QBAR_A(MDN)
            X0(1,NT)=0.
            DO I=1,NT
               A(2,I)=0.
               X0(2,I)=0.
               N(2,I)=0
            ENDDO
         ELSE
            A(1,NT)=QBAR_A(MDN)
            A(2,NT)=QBAR_B(MDN)
            X0(1,NT)=0.
            X0(2,NT)=0.
         ENDIF
      ELSE IF(KJE.EQ.3) THEN
         ID=QBAR_EXT(LR,MDN)
         IF(ID.EQ.0) RETURN
         A(1,NT)=EXT_A(ID)
         A(2,NT)=EXT_YUP(ID)-EXT_YDN(ID)
         X0(1,NT)=0.
         X0(2,NT)=(EXT_YUP(ID)+EXT_YDN(ID))/2
      ENDIF
      RETURN

      ENTRY UNFOLDED_REPLACE(RF)
      X(1,NT)=RF(1)
      X(2,NT)=RF(2)
      RETURN

      ENTRY UNFOLDED_RESET
      NT=0
      RETURN

      ENTRY UNFOLDED_GET(XD,YD,YB)

      DO I=1,2
         XX(I)=X(I,NT)
         DO K=NT,1,-1
            XX(I)=XX(I)-X0(I,K)
            IF(MOD(N(I,K),2).EQ.0) THEN
               XX(I)=A(I,K)*N(I,K)+XX(I)
            ELSE
               XX(I)=A(I,K)*N(I,K)-XX(I)
            ENDIF
            XX(I)=XX(I)+X0(I,K)
         ENDDO
      ENDDO
      XD=XX(1)
      YD=XX(2)
      YB=YD
      IF(TIP(NT).NE.3) RETURN

      YB=X(2,NT-1)
      DO K=NT-1,1,-1
         YB=YB-X0(2,K)
         IF(MOD(N(2,K),2).EQ.0) THEN
            YB=A(2,K)*N(2,K)+YB
         ELSE
            YB=A(2,K)*N(2,K)-YB
         ENDIF
         YB=YB+X0(2,K)
      ENDDO

      END



