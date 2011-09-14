!-------------------------------------------------------------------------
! TOP counter for Belle: Reconstruction - basic functions to construct PDF 
! M. Staric, nov-2008, jun-2009, jan-2010
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
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
         F_YY(I,K)=F_YB(I,K)
         F_YM(I,K)=0.
         F_NXM(I,K)=0
         F_NYM(I,K)=0
         IF(K.EQ.1) THEN
            AH=QBAR_A(MODUL)/2
            BH=QBAR_B(MODUL)/2
            TOTX=ABS(F_KX(I,K)).LT.PTOT.OR.ABS(XD).LT.AH
            TOTY=ABS(F_KY(I,K)).LT.PTOT.OR.ABS(F_YY(I,K)).LT.BH
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
         F_YD(I,K)=QBAR_YCMI(MODUL)+DYDZ*(ZD-ZM)
         F_YB(I,K)=QBAR_YCMI(MODUL)+DYDZ*(ZB-ZM)
         F_YY(I,K)=Y0+DYDZ*(ZB-Z0)
         XM=X0+DXDZ*(ZM-Z0)
         F_YM(I,K)=Y0+DYDZ*(ZM-Z0)
         F_NXM(I,K)=NINT(XM/QBAR_A(MODUL))
         F_NYM(I,K)=NINT(F_YM(I,K)/QBAR_B(MODUL))
         IF(K.EQ.1) THEN
            AH=QBAR_A(MODUL)/2
            BH=QBAR_B(MODUL)/2
            TOTX=ABS(F_KX(I,K)).LT.PTOT.OR.ABS(XD).LT.AH
            TOTY=ABS(F_KY(I,K)).LT.PTOT.OR.ABS(F_YY(I,K)).LT.BH
            F_OK(I)=F_TL(I,K).GT.0.AND.TOTX.AND.TOTY
         ENDIF
      ENDDO
      OK=F_OK(1).OR.F_OK(2)
      IF(.NOT.OK) RETURN

      TOPF_CYL=K

      END


      INTEGER FUNCTION TOPF_SPH(XD,ZD,ZB,ZM,IEP,ICA,H)

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
      REAL X0, Y0, Z0
      REAL DX, F, DXDZ, DYDZ, AH, BH, XM
      REAL KX, KY, KZ, TLEN, S
      LOGICAL OK, TOTX, TOTY
      INTEGER NX, NITER /2/, NX1, NY1
      SAVE NX, NITER

      TOPF_SPH=0

      X0=T_R(1,IEP)
      Y0=T_R(2,IEP)
      Z0=T_R(3,IEP)
      DX=XD-QBAR_XCMI(MODUL)
      F=ZD-ZM
      DXDZ=DX/F
      IF(.NOT.LIN_OPT) THEN
         DO I=1,NITER
            XM=X0+DXDZ*(ZM-Z0)
            IF(NT.EQ.0) NX=NINT(XM/QBAR_A(MODUL))
            XM=XM-NX*QBAR_A(MODUL)-QBAR_XCMI(MODUL)
            DXDZ=(DX-(1-2*ABS(F)/QBAR_RMI(MODUL)*(1+DXDZ**2))*XM)/F
         ENDDO
      ENDIF

      K=TOPSOLU(DXDZ,IEP,ICA)
      IF(K.EQ.0) RETURN

      F_H(K)=H
      DO I=1,2
         F_XD(I,K)=XD
         DYDZ=F_KY(I,K)/F_KZ(I,K)
         F_YM(I,K)=Y0+DYDZ*(ZM-Z0)
         NY1=NINT(F_YM(I,K)/QBAR_B(MODUL))
         F_YD(I,K)=QBAR_YCMI(MODUL)+DYDZ*(ZD-ZM)
         F_YB(I,K)=QBAR_YCMI(MODUL)+DYDZ*(ZB-ZM)
         F_YY(I,K)=Y0+DYDZ*(ZB-Z0)
         KX=F_KX(I,K)
         KY=F_KY(I,K)
         KZ=F_KZ(I,K)
         TLEN=(ZM-Z0)/KZ
         XM=X0+TLEN*KX
         NX1=NINT(XM/QBAR_A(MODUL))
         XM=XM-QBAR_A(MODUL)*NX1-QBAR_XCMI(MODUL)
         IF(LIN_OPT) THEN
            KX=KX-XM/F*KZ
         ELSE
            KX=KX-2*XM/QBAR_RMI(MODUL)*(1+(KX/KZ)**2)*KZ
         ENDIF
         S=SQRT(KX**2+KY**2+KZ**2)
         KX=KX/S
         KY=KY/S
         KZ=KZ/S
         F_TL(I,K)=TLEN+F/KZ
         F_NXM(I,K)=NX1
         F_NYM(I,K)=NY1
         IF(K.EQ.1) THEN
            AH=QBAR_A(MODUL)/2
            BH=QBAR_B(MODUL)/2
            TOTX=ABS(F_KX(I,K)).LT.PTOT.AND.ABS(KX).LT.PTOT
            TOTX=TOTX.OR.ABS(XD).LT.AH
            TOTY=ABS(F_KY(I,K)).LT.PTOT.AND.ABS(KY).LT.PTOT
            TOTY=TOTY.OR.ABS(F_YY(I,K)).LT.BH
            F_OK(I)=F_TL(I,K).GT.0.AND.TOTX.AND.TOTY
         ENDIF
         IF(.NOT.LIN_OPT) F_OK(I)=F_OK(I).AND.NX1.EQ.NX
      ENDDO
      OK=F_OK(1).OR.F_OK(2)
      IF(.NOT.OK) RETURN

      TOPF_SPH=K

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


      REAL FUNCTION TOPF_DYY(I,K1,K2)

      IMPLICIT NONE
      INTEGER I, K1, K2
#include "TOP_TOPF.fi"
      REAL DELH, DELF

      DELF=F_YY(I,K2)-F_YY(I,K1)
      DELH=F_H(K2)-F_H(K1)
      IF(DELH.NE.0) THEN
         TOPF_DYY=DELF/DELH
      ELSE
         TOPF_DYY=0.
         PRINT*,'topf_dyy: DELH=0',K1,K2
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
         DELF=(F_CFIC(I,K2)-F_CFIC(I,K1))/F_SFIC(I,K1)
      ENDIF
      DELH=F_H(K2)-F_H(K1)
      IF(DELH.NE.0) THEN
         TOPF_DFIC=DELF/DELH
      ELSE
         TOPF_DFIC=0.
         PRINT*,'topf_dfic: DELH=0',K1,K2
      ENDIF

      END


      INTEGER FUNCTION TOPF_RAYTRACE(K0,I0,DFICDX)

c--   fast raytracing 

      IMPLICIT NONE
      INTEGER K0, I0
      REAL DFICDX
#include "TOP_TRACK.fi"
#include "TOP_PIK.fi"
#include "TOP_TOPF.fi"
      REAL R(3), DIR(3), TL, XD, YD, YB
      INTEGER LR, NX(3), NY(3)
      INTEGER K, I
      LOGICAL OK
      REAL FIC, D_XD, AA, BB, KX, KY, KZ, CFIC, SFIC, AD_XD, H
      REAL FI0, FI1, XD0, XD1, DFIDX

      TOPF_RAYTRACE=0
      K=NT+1
      IF(K.GT.NB) THEN
         PRINT*,'topf_raytrace: TOP_TOPF is full'
         RETURN
      ENDIF
      F_X0(K)=F_X0(K0)
      F_Y0(K)=F_Y0(K0)
      F_Z0(K)=F_Z0(K0)
      F_YY(I0,K)=F_YY(I0,K0)
      F_YM(I0,K)=F_YM(I0,K0)
      KX=F_KX(I0,K0)
      KY=F_KY(I0,K0)
      KZ=F_KZ(I0,K0)
      CFIC=F_CFIC(I0,K0)
      SFIC=F_SFIC(I0,K0)
      AD_XD=1e10
      
      DO I=0,10
         R(1)=F_X0(K)
         R(2)=F_Y0(K)
         R(3)=F_Z0(K)
         DIR(1)=KX
         DIR(2)=KY
         DIR(3)=KZ
         CALL TOPF_TRACE(R,DIR,MODUL,TL,LR,XD,YD,YB,NX,NY,OK)
         IF(.NOT.OK) RETURN
         D_XD=XD-F_XD(I0,K0)
         IF(ABS(D_XD).LT.AD_XD.OR.I.EQ.0) THEN
            AD_XD=ABS(D_XD)
            F_CFIC(I0,K)=CFIC
            F_SFIC(I0,K)=SFIC
            F_KX(I0,K)=KX
            F_KY(I0,K)=KY
            F_KZ(I0,K)=KZ
            F_TL(I0,K)=TL
            F_XH(I0,K)=R(1)
            F_YH(I0,K)=R(2)
            F_XD(I0,K)=XD
            F_YD(I0,K)=YD
            F_YB(I0,K)=YB
            F_NX(I0,K)=NX(1)
            F_NY(I0,K)=NY(1)
            F_NXM(I0,K)=NX(2)
            F_NYM(I0,K)=NY(2)
            F_NXE(I0,K)=NX(3)
            F_NYE(I0,K)=NY(3)
         ELSE
            IF(I.GT.1) EXIT
         ENDIF
         IF(ABS(D_XD).LT.0.25) EXIT

         FI0=FI1
         FI1=FIC
         XD0=XD1
         XD1=XD
         IF(I.EQ.0) THEN
            DFIDX=DFICDX
            FIC=ATAN2(SFIC,CFIC)
            FI1=FIC
         ELSE IF(I.EQ.1) THEN
            H=XD1-XD0
            IF(ABS(H).LT.1E-6) EXIT
            DFIDX=(FI1-FI0)/H
         ENDIF
         FIC=FIC-DFIDX*D_XD
         CFIC=COS(FIC)
         SFIC=SIN(FIC)
         AA=T_CTH(0)*S_THC(0)*CFIC+T_STH(0)*C_THC(0)
         BB=S_THC(0)*SFIC
         KX=AA*T_CFI(0)-BB*T_SFI(0)
         KY=AA*T_SFI(0)+BB*T_CFI(0)
         KZ=T_CTH(0)*C_THC(0)-T_STH(0)*S_THC(0)*CFIC
      ENDDO

      NT=K
      TOPF_RAYTRACE=K

      END


      SUBROUTINE TOPF_PHTRACE(FIC,IEP,ICA,RH,TL,LR,XD,YD,YB,NX,NY,OK)

      IMPLICIT NONE
      REAL FIC
      INTEGER IEP, ICA
      REAL RH(3), TL
      INTEGER LR
      REAL XD, YD, YB
      INTEGER NX(3), NY(3)
      LOGICAL OK
#include "TOP_TRACK.fi"
#include "TOP_PIK.fi"
      REAL DIR(3), CFIC, SFIC, AA, BB
      INTEGER I

      DO I=1,3
         RH(I)=T_R(I,IEP)
      ENDDO
      CFIC=COS(FIC)
      SFIC=SIN(FIC)
      AA=T_CTH(IEP)*S_THC(ICA)*CFIC+T_STH(IEP)*C_THC(ICA)
      BB=S_THC(ICA)*SFIC
      DIR(1)=AA*T_CFI(IEP)-BB*T_SFI(IEP)
      DIR(2)=AA*T_SFI(IEP)+BB*T_CFI(IEP)
      DIR(3)=T_CTH(IEP)*C_THC(ICA)-T_STH(IEP)*S_THC(ICA)*CFIC
      OK=ABS(DIR(3)).GT.1E-5
      IF(.NOT.OK) RETURN
      OK=ABS(DIR(1)).LT.PTOT
      IF(.NOT.OK) RETURN
      OK=ABS(DIR(2)).LT.PTOT
      IF(.NOT.OK) RETURN

      CALL TOPF_TRACE(RH,DIR,MODUL,TL,LR,XD,YD,YB,NX,NY,OK)

      END

