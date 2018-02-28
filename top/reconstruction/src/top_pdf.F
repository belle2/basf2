!-----------------------------------------------------------------------
! TOP counter for Belle: Reconstruction - construct TOP PDF
! M. Staric, nov-2008, jun-2009, feb-2010, jun-2012, feb-2013, dec-2013,
! mar-2017
!
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
! avg-2017: change in some commons
! dec-2017: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE TOP_SET_PDF(MASS,T0)

      IMPLICIT NONE
      REAL MASS                 ! mass of particle hypothesis
      REAL T0                   ! start time for beta=1
#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_PMT.fi"
#include "TOP_TDC.fi"
#include "TOP_PAR.fi"
#include "TOP_PIK.fi"
#include "TOP_TOPF.fi"
#include "TOP_TRACK.fi"
#include "TOP_MASK.fi"
      DATA NCH /3*0/
      DATA BETA_RQ /0./
      DATA STORE_OPT /.TRUE./
      DATA PDF_OPT /2/

      INTEGER NUMCH
      INTEGER NN, ICH, LR, L, K, I
      REAL ZW(0:1), ZD, ZB, ZM, DZEXT
      REAL NORM, FOT, RIND(-1:1), SIGE_SIP
      REAL PHASE_INDEX
      INTEGER MAXPIK

!     reset & set some parameters
      LR_FLAG(0)=QBAR_LSIDE(MODUL)
      LR_FLAG(1)=QBAR_RSIDE(MODUL)
      ZW(0)=-QBAR_C(MODUL)/2.
      ZW(1)=QBAR_C(MODUL)/2.
      NCH(0)=PMT_NX*PMT_NY*N_PMT(0,MODUL)
      NCH(1)=PMT_NX*PMT_NY*N_PMT(1,MODUL)
      NUMCH=NCH(0)+NCH(1)
      IF(NUMCH.GT.NUM_CHA) THEN
         PRINT*,'top_set_pdf_: NUMCH>NUM_CHA, NUMCH=',NUMCH
         RETURN
      ENDIF
      CALL SET_XYCH(MODUL)
      CALL SET_BACKG(MODUL)
      DO ICH=0,NUMCH-1
         NUM_PIK(ICH)=0
         PIK_OVFL(ICH)=0
      ENDDO
      STFS=0.
      IF(BETA_RQ.GT.0) THEN
         MASS_HYP=0.
         BETA=BETA_RQ           ! for beta resolution studies
      ELSE
         IF(MASS.LE.0) RETURN
         MASS_HYP=MASS
         BETA=PMOM/MASS/SQRT(1+(PMOM/MASS)**2)
      ENDIF
      TOF=(TOF_C0+T0)/BETA
      RIND(-1)=PHASE_INDEX(E_MEAN-D_E)
      RIND(0)=PHASE_INDEX(E_MEAN) ! =REF_IND
      RIND(1)=PHASE_INDEX(E_MEAN+D_E) ! =REF_IND1
      DO I=-1,1
         C_THC(I)=1./BETA/RIND(I)
         IF(C_THC(I).GE.1) RETURN
         S_THC(I)=SQRT(1.-C_THC(I)**2)
      ENDDO
      FOT0=N_ZERO*T_LEN/2./PI*S_THC(0)**2
      PTOT=SQRT(1.-1./REF_IND**2)
      IF(QBAR_RADL.GT.0) THEN
         TH0_SIP=13.6E-3/BETA/PMOM*SQRT(T_LEN/2./QBAR_RADL)
      ELSE
         TH0_SIP=0.
      ENDIF
      D_THC=(S_THC(1)-S_THC(0))/C_THC(0)
      SIGE_SIP=D_E/D_THC*TH0_SIP
      CALL QE_SET_XQE(SIGE_SIP)

!     calculate pattern
      DO L=0,1
         DZEXT=0.
         IF(LR_FLAG(L).EQ.0) THEN
            CYCLE
         ELSE IF(LR_FLAG(L).EQ.1) THEN
            LR=L
            IF(QBAR_EXT(LR,MODUL).GT.0) DZEXT=EXT_DZ(QBAR_EXT(LR,MODUL))
            ZM=0
            ZB=ZW(L)
            ZD=ZB+(2*L-1)*(DZEXT+QBAR_WIN(MODUL))
            CALL TOP_PATT_WIN(LR,L,0,ZD,ZB,ZM)
         ELSE IF(LR_FLAG(L).EQ.2) THEN
            LR=1-L
            IF(QBAR_EXT(LR,MODUL).GT.0) DZEXT=EXT_DZ(QBAR_EXT(LR,MODUL))
            ZM=ZW(L)
            ZB=ZM+(2*L-1)*QBAR_C(MODUL)
            ZD=ZB+(2*L-1)*(DZEXT+QBAR_WIN(MODUL))
            CALL TOP_PATT_WIN(LR,L,0,ZD,ZB,ZM)
         ELSE IF(LR_FLAG(L).EQ.3) THEN
            LR=1-L
            IF(QBAR_EXT(LR,MODUL).GT.0) DZEXT=EXT_DZ(QBAR_EXT(LR,MODUL))
            ZM=ZW(L)
            ZB=ZM+(2*L-1)*QBAR_C(MODUL)
            ZD=ZB+(2*L-1)*(DZEXT+QBAR_WIN(MODUL))
            CALL TOP_PATT_WIN(LR,L,1,ZD,ZB,ZM)
         ELSE IF(LR_FLAG(L).EQ.4) THEN
            LR=1-L
            CALL PDF_SPHERICAL(LR) ! new PDF for spherical mirror
         ELSE
            PRINT*,'top_reco: unknown exit window type ',LR_FLAG(L)
            CYCLE
         ENDIF
      ENDDO

      MAXPIK=0
      NN=0
      DO ICH=0,NUMCH-1
         MAXPIK=MAX(MAXPIK,NUM_PIK(ICH)+PIK_OVFL(ICH))
         IF(PIK_OVFL(ICH).GT.0) THEN
            NN=NN+1
            NUM_PIK(ICH)=MAX_PIK
         ENDIF
      ENDDO      
      IF(NN.GT.0) PRINT*,'num_pik>MAX_PIK---->',NN, MAXPIK

!     normalize 
      STFS=0.
      NORM=0.
      DO ICH=0,NUMCH-1
         IF(.NOT.CH_MASK(ICH,MODUL)) THEN
            NUM_PIK(ICH)=0
            CYCLE
         ENDIF
         FOT=0.                 ! kaj pa ozadje?
         DO K=1,NUM_PIK(ICH)
            PIK_NPH(K,ICH)=PIK_NPH(K,ICH)*CH_EFFI(ICH,MODUL)
            NORM=NORM+PIK_NPH(K,ICH)
            FOT=FOT+PIK_NPH(K,ICH)
         ENDDO
         STFS=STFS+(1.-EXP(-FOT))
      ENDDO
      IF(NORM.EQ.0) RETURN

      DO ICH=0,NUMCH-1
         DO K=1,NUM_PIK(ICH)
            PIK_NPH(K,ICH)=PIK_NPH(K,ICH)/NORM
         ENDDO
      ENDDO

      END


      SUBROUTINE TOP_PATT_WIN(LR,L,TYP,ZD,ZB,ZM)

      IMPLICIT NONE
      INTEGER LR, L             ! exit window: 0/1=left/right
      INTEGER TYP               ! focusing type: 0=non, 1=cyl, 2=sph
      REAL ZD                   ! z coordinate of detector plane
      REAL ZB                   ! z coordinate of qbar-end
      REAL ZM                   ! z coordinate of mirror for focusing type
#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
#include "TOP_PAR.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_TOPF.fi"
      REAL XMI, XMA, XCH, DXCH, X
      INTEGER KMI, KMA, IPMTX, IX, ICH, K, ICHAN
      LOGICAL NOT_WINDOW, OK

      IF(NOT_WINDOW(ZD,C_THC(0),S_THC(0))) RETURN
      XMI=T_R0(1)-(T_MAX-TOF)/GRU_IND*C0*PTOT
      XMA=T_R0(1)+(T_MAX-TOF)/GRU_IND*C0*PTOT
      CALL XMINMAX(XMI,XMA,ZD,C_THC(0),S_THC(0),OK)
      IF(.NOT.OK) RETURN
      KMI=NINT(XMI/QBAR_A(MODUL))
      KMA=NINT(XMA/QBAR_A(MODUL))

      DO IPMTX=0,N_PMTX(LR,MODUL)-1
         DO IX=0,PMT_NX-1
            ICH=ICHAN(IX,0,IPMTX,0,LR,MODUL)
            XCH=PIK_XCH(ICH)
            DXCH=PIK_DXCH(ICH)
            IF(DXCH.EQ.0) CYCLE
            DO K=KMI,KMA
               IF(MOD(ABS(K),2).EQ.0) THEN
                  X=K*QBAR_A(MODUL)+XCH
               ELSE
                  X=K*QBAR_A(MODUL)-XCH
               ENDIF
               IF(X.LT.XMI-DXCH.OR.X.GT.XMA+DXCH) CYCLE
               IF(TYP.EQ.0) THEN
                  CALL TOP_PATT_PMT(X,ZD,ZB,DXCH,LR,L,IPMTX,IX,K)
               ELSE IF(TYP.EQ.1) THEN
                  CALL TOP_PATT_CYL(X,ZD,ZB,ZM,DXCH,LR,IPMTX,IX,K)
               ENDIF                  
            ENDDO
         ENDDO
      ENDDO

      END


      SUBROUTINE TOP_PATT_PMT(XD,ZD,ZB,DXCH,LR,L,IPMTX,IX,NX1)

      IMPLICIT NONE
      REAL XD                   ! unfolded channel x-coordinate
      REAL ZD                   ! PMT plane z-position 
      REAL ZB                   ! bar-to-wedge z-position 
      REAL DXCH                 ! channel width
      INTEGER LR                ! position of PMT's (0=left, 1=right)
      INTEGER L                 ! if different from LR: reflection from plane mirror
      INTEGER IPMTX, IX         ! PMT number and PMT channel number in x 
      INTEGER NX1               ! x-reflections
#include "TOP_CONS.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_PAR.fi"
#include "TOP_TOPF.fi"
      INTEGER TOPF_PMT
      INTEGER I, K0, K1, K2, K3, TYP
      REAL H, TIM

c--   clear TOP_TOPF
      NT=0
c--   solve equations
      H=0.
      K0=TOPF_PMT(XD,ZD,ZB,0,0,H)
      IF(K0.EQ.0) RETURN
c--   for channel width
      H=0.1
      K1=TOPF_PMT(XD+H,ZD,ZB,0,0,H)
      IF(K1.EQ.0) RETURN
c--   for parallax
      H=T_H
      K2=TOPF_PMT(XD,ZD,ZB,1,0,H)
      IF(K2.EQ.0) RETURN
c--   for cromatic
      H=D_E
      K3=TOPF_PMT(XD,ZD,ZB,0,1,H)
      IF(K3.EQ.0) RETURN

      TYP=0
      IF(LR.NE.L) TYP=1
c--   calculate PDF parameters
      DO I=1,2
         IF(.NOT.F_OK(I)) CYCLE
         TIM=F_TL(I,K0)*GRU_IND/C0+TOF
         IF(TIM.GT.T_MAX) CYCLE

         CALL PDF_YSKEN(I,K0,K1,K2,K3,XD,DXCH,ZB,ZD,IX,IPMTX,LR,TYP)

      ENDDO

      END


      SUBROUTINE TOP_PATT_CYL(XD,ZD,ZB,ZM,DXCH,LR,IPMTX,IX,NX1)

      IMPLICIT NONE
      REAL XD                   ! unfolded channel x-coordinate
      REAL ZD                   ! PMT plane z-position 
      REAL ZB                   ! bar-to-wedge z-position 
      REAL ZM                   ! mirror z-position 
      REAL DXCH                 ! channel width
      INTEGER LR                ! position of PMT's (0=left, 1=right)
      INTEGER IPMTX, IX         ! PMT number and PMT channel number in x 
      INTEGER NX1               ! x-reflections
#include "TOP_CONS.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_PAR.fi"
#include "TOP_TOPF.fi"
      INTEGER TOPF_CYL
      INTEGER I, K0, K1, K2, K3, TYP
      REAL H, TIM

c--   clear TOP_TOPF
      NT=0
c--   solve equations
      H=0.
      K0=TOPF_CYL(XD,ZD,ZB,ZM,0,0,H)
      IF(K0.EQ.0) RETURN
c--   for channel width
      H=0.1
      K1=TOPF_CYL(XD+H,ZD,ZB,ZM,0,0,H)
      IF(K1.EQ.0) RETURN
c--   for parallax
      H=T_H
      K2=TOPF_CYL(XD,ZD,ZB,ZM,1,0,H)
      IF(K2.EQ.0) RETURN
c--   for cromatic
      H=D_E
      K3=TOPF_CYL(XD,ZD,ZB,ZM,0,1,H)
      IF(K3.EQ.0) RETURN

      TYP=2
c--   calculate PDF parameters
      DO I=1,2
         IF(.NOT.F_OK(I)) CYCLE
         TIM=F_TL(I,K0)*GRU_IND/C0+TOF
         IF(TIM.GT.T_MAX) CYCLE

         CALL PDF_YSKEN(I,K0,K1,K2,K3,XD,DXCH,ZB,ZD,IX,IPMTX,LR,TYP)

      ENDDO

      END


      SUBROUTINE PDF_SPHERICAL(LR)

      IMPLICIT NONE
      INTEGER LR                ! position of PMT's (0=left, 1=right)

#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_PAR.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"

      REAL ZM, ZB, ZD
      REAL XMI, XMA
      INTEGER KMI, KMA, K
      LOGICAL NOT_WINDOW, OK

c--   initial 

      IF(LR.EQ.0) THEN
         ZM=QBAR_ZR(MODUL)
         ZB=-QBAR_C(MODUL)/2
         ZD=ZB-QBAR_WIN(MODUL)
         IF(QBAR_EXT(LR,MODUL).GT.0) ZD=ZD-EXT_DZ(QBAR_EXT(LR,MODUL))
      ELSE
         ZM=QBAR_ZL(MODUL)
         ZB=QBAR_C(MODUL)/2
         ZD=ZB+QBAR_WIN(MODUL)
         IF(QBAR_EXT(LR,MODUL).GT.0) ZD=ZD+EXT_DZ(QBAR_EXT(LR,MODUL))
      ENDIF

c--   number of x-reflections before mirror (range)

      IF(NOT_WINDOW(ZM,C_THC(0),S_THC(0))) RETURN
      XMI=T_R0(1)-(T_MAX-TOF)/GRU_IND*C0*PTOT
      XMA=T_R0(1)+(T_MAX-TOF)/GRU_IND*C0*PTOT
      CALL XMINMAX(XMI,XMA,ZM,C_THC(0),S_THC(0),OK)
      IF(.NOT.OK) RETURN
      KMI=NINT(XMI/QBAR_A(MODUL))
      KMA=NINT(XMA/QBAR_A(MODUL))

c--   loop over x-reflections before mirror

      DO K=KMI,KMA
         CALL PDF_SPHER_XREFL(LR,K,XMI,XMA,ZB,ZD)
      ENDDO

      END


      SUBROUTINE PDF_SPHER_XREFL(LR,NXM,XMI,XMA,ZB,ZD)

      IMPLICIT NONE
      INTEGER LR                ! position of PMT's (0=left, 1=right)
      INTEGER NXM               ! x-reflections before mirror
      REAL XMI, XMA
      REAL ZB                   ! bar-to-wedge z-position 
      REAL ZD                   ! PMT plane z-position 
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"
#include "TOP_PMT.fi"
#include "TOP_PIK.fi"
#include "TOP_PAR.fi"
#include "TOP_CONS.fi"
#include "TOP_TOPF.fi"
      REAL X0, Z0, XD1, XD2, AH, XD, TL_MAX, TL1, TL2
      REAL UNFOLD
      REAL XMMI, XMMA
      INTEGER K, KMI, KMA
      INTEGER IPMTX, IX, ICH, ICHAN
      REAL XCH, DXCH
      LOGICAL ODD, OK

c--   number of x-reflections after mirror (range)

      ODD=MOD(NXM,2).NE.0
      X0=T_R0(1)
      Z0=T_R0(3)
      AH=QBAR_A(MODUL)/2
      XMMI=MAX(XMI,(2*NXM-1)*AH)
      XMMA=MIN(XMA,(2*NXM+1)*AH)
      TL_MAX=(T_MAX-TOF)/GRU_IND*C0

      CALL XMIRR_REFLECT_3D(X0,Z0,NXM,XMMI,ZD,XD1,TL1,OK) 
      IF(.NOT.OK) RETURN
      CALL XMIRR_REFLECT_3D(X0,Z0,NXM,XMMA,ZD,XD2,TL2,OK) 
      IF(.NOT.OK) RETURN
      IF(MIN(TL1,TL2).GT.TL_MAX) RETURN

      IF(XD1.GT.XD2) THEN
         XD=XD2
         XD2=XD1
         XD1=XD
      ENDIF

      KMI=NINT(XD1/QBAR_A(MODUL))
      KMA=NINT(XD2/QBAR_A(MODUL))
      IF(KMI.GT.KMA) RETURN

c--   loop over x-reflections after mirror and over channels

      DO IPMTX=0,N_PMTX(LR,MODUL)-1
         DO IX=0,PMT_NX-1
            ICH=ICHAN(IX,0,IPMTX,0,LR,MODUL)
            XCH=PIK_XCH(ICH)
            IF(ODD) XCH=-XCH
            DXCH=PIK_DXCH(ICH)
            IF(DXCH.EQ.0) CYCLE
            DO K=KMI,KMA
               XD=UNFOLD(XCH,K,QBAR_A(MODUL))
               IF(XD.LT.XD1-DXCH.OR.XD.GT.XD2+DXCH) CYCLE
               CALL TOP_PATT_SPHER(XD,ZD,ZB,DXCH,LR,IPMTX,IX,NXM,K)
            ENDDO
         ENDDO
      ENDDO

      END


      SUBROUTINE TOP_PATT_SPHER(XD,ZD,ZB,DXCH,LR,IPMTX,IX,NX1,NX2)

      IMPLICIT NONE
      REAL XD                   ! unfolded channel x-coordinate
      REAL ZD                   ! PMT plane z-position 
      REAL ZB                   ! bar-to-wedge z-position 
      REAL DXCH                 ! channel width
      INTEGER LR                ! position of PMT's (0=left, 1=right)
      INTEGER IPMTX, IX         ! PMT number and PMT channel number in x 
      INTEGER NX1               ! x-reflections before mirror
      INTEGER NX2               ! x-reflections after mirror
#include "TOP_CONS.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_PAR.fi"
#include "TOP_TOPF.fi"
      INTEGER TOPF_SPHER
      INTEGER I, K0, K1, K2, K3, TYP
      REAL H, TIM

c--   clear TOP_TOPF
      NT=0
c--   solve equations
      H=0.
      K0=TOPF_SPHER(XD,ZD,ZB,NX1,0,0,H)
      IF(K0.EQ.0) RETURN
c--   for channel width
      H=0.1
      K1=TOPF_SPHER(XD+H,ZD,ZB,NX1,0,0,H)
      IF(K1.EQ.0) RETURN
c--   for parallax
      H=T_H
      K2=TOPF_SPHER(XD,ZD,ZB,NX1,1,0,H)
      IF(K2.EQ.0) RETURN
c--   for cromatic
      H=D_E
      K3=TOPF_SPHER(XD,ZD,ZB,NX1,0,1,H)
      IF(K3.EQ.0) RETURN

      TYP=3
c--   calculate PDF parameters
      DO I=1,2
         IF(.NOT.F_OK(I)) CYCLE
         TIM=F_TL(I,K0)*GRU_IND/C0+TOF
         IF(TIM.GT.T_MAX) CYCLE

         CALL PDF_YSKEN(I,K0,K1,K2,K3,XD,DXCH,ZB,ZD,IX,IPMTX,LR,TYP)

      ENDDO

      END


      SUBROUTINE SET_XYCH(M)

      IMPLICIT NONE
      INTEGER M                 ! module number
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_PMT.fi"
#include "TOP_PIK.fi"
      INTEGER LR, I, ICH, II
      REAL X, Y, Z, AH, BH, XMI, XMA, YMI, YMA, DXH, DYH
      REAL X1, X2, Y1, Y2, DX, DY
      INTEGER M_OLD /-1/
      SAVE M_OLD

      IF(M.EQ.M_OLD) RETURN
      M_OLD=M

      AH=QBAR_A(M)/2
      BH=QBAR_B(M)/2
      DXH=PMT_DXCH/2
      DYH=PMT_DYCH/2

      XMI=-AH
      XMA=AH
      DO LR=0,1
         II=QBAR_EXT(LR,M)
         IF(II.GT.0) THEN
            YMI=EXT_YDN(II)
            YMA=EXT_YUP(II)
         ELSE
            YMI=-BH
            YMA=BH
         ENDIF
         DO I=0,NCH(LR)-1
            ICH=I+LR*NCH(LR-1)
            CALL ICHXYZ(ICH,M,X,Y,Z)
            DX=PMT_DXCH
            DY=PMT_DYCH
            X1=X-DXH
            X2=X+DXH
            IF(X1.LT.XMI.OR.X2.GT.XMA) THEN
               X1=MAX(X-DXH,XMI)
               X2=MIN(X+DXH,XMA)
               DX=MAX(X2-X1,0.)
               IF(DX.GT.0) X=(X1+X2)/2
            ENDIF
            Y1=Y-DYH
            Y2=Y+DYH
            IF(Y1.LT.YMI.OR.Y2.GT.YMA) THEN
               Y1=MAX(Y-DYH,YMI)
               Y2=MIN(Y+DYH,YMA)
               DY=MAX(Y2-Y1,0.)
               IF(DY.GT.0) Y=(Y1+Y2)/2
            ENDIF
            PIK_XCH(ICH)=X
            PIK_YCH(ICH)=Y
            PIK_DXCH(ICH)=DX
            PIK_DYCH(ICH)=DY
         ENDDO
      ENDDO

      END



      SUBROUTINE SET_BACKG(M)

      IMPLICIT NONE
      INTEGER M                 ! module number
#include "TOP_GEO.fi"
#include "TOP_PIK.fi"
#include "TOP_PAR.fi"
#include "TOP_DATA.fi"
#include "TOP_MASK.fi"
      
      INTEGER N_BGR, I
      INTEGER ICH, NUMCH
      REAL S, T
      
      IF(NUM_MOD.GT.1) THEN
         N_BGR=0
         DO I=1,N_DATA
            IF(DATA_MOD(I).NE.M) THEN
               T=DATA_TIME(I)
               IF(T.LT.T_MIN.OR.T.GT.T_MAX) CYCLE
               N_BGR=N_BGR+1
            ENDIF
         ENDDO
         STFB=FLOAT(N_BGR)/FLOAT(NUM_MOD-1)
         STFB=MAX(NUM_BGR,STFB)
      ELSE
         STFB=NUM_BGR
      ENDIF

      NUMCH=NCH(0)+NCH(1)
      S=0.
      DO ICH=0,NUMCH-1
         IF(CH_MASK(ICH,M)) THEN
            PIK_BGR(ICH)=PIK_DXCH(ICH)*PIK_DYCH(ICH)*CH_EFFI(ICH,M)
            S=S+PIK_BGR(ICH)
         ELSE
            PIK_BGR(ICH)=0.
         ENDIF
      ENDDO
      IF(S.EQ.0) RETURN
      DO ICH=0,NUMCH-1
         PIK_BGR(ICH)=PIK_BGR(ICH)/S/(T_MAX-T_MIN)
      ENDDO

      END


      LOGICAL FUNCTION NOT_WINDOW(Z,CTHC,STHC)

      IMPLICIT NONE
      REAL Z                    ! window coordinate
      REAL CTHC, STHC           ! cos(theta_cer), sin(theta_cer)
#include "TOP_TRACK.fi"
      
      IF(Z-T_R0(3).GT.0) THEN
         NOT_WINDOW=T_CTH(0)*CTHC.LT.-T_STH(0)*STHC
      ELSE
         NOT_WINDOW=T_CTH(0)*CTHC.GT.T_STH(0)*STHC
      ENDIF

      END


      SUBROUTINE XMINMAX(XMI,XMA,Z,CTHC,STHC,OK)


      IMPLICIT NONE
      REAL XMI, XMA             ! min, max unfolded x-coordinate [in-out]
      REAL Z                    ! window coordinate
      REAL CTHC, STHC           ! cos(theta_cer), sin(theta_cer)
      LOGICAL OK                ! flag
#include "TOP_TRACK.fi"
#include "TOP_CONS.fi"
      real XINF
      PARAMETER (XINF=1.E10)
      REAL X(2), X1D, X2D, DZ, TH, THC, A, B, C, D, CFIC, KZ
      INTEGER I

      DZ=Z-T_R0(3)
      OK=DZ.NE.0
      IF(.NOT.OK) RETURN

      TH=ACOS(T_CTH(0))
      THC=ACOS(CTHC)
      IF(DZ.LT.0) TH=PI-TH      ! rotation around x for pi (phi=-phi; z=-z)
      DZ=ABS(DZ)
      OK=TH-THC.LT.PI/2
      IF(.NOT.OK) RETURN

      IF(T_SFI(0).EQ.0) THEN
         IF(TH-THC.LT.PI/2) THEN
            X(1)=TAN(TH-THC)*T_CFI(0)
         ELSE
            X(1)=XINF*T_CFI(0)
         ENDIF
         IF(TH+THC.LT.PI/2) THEN
            X(2)=TAN(TH+THC)*T_CFI(0)
         ELSE
            X(2)=XINF*T_CFI(0)
         ENDIF
      ELSE
         A=-COS(TH+THC)*COS(TH-THC)
         B=SIN(2*TH)*T_CFI(0)
         C=(T_SFI(0)*STHC)**2-T_CFI(0)**2*SIN(TH+THC)*SIN(TH-THC)
         IF(A.EQ.0) THEN
            IF(B.EQ.0) RETURN
            X(1)=-C/B
            X(2)=SIGN(XINF,B)
         ELSE
            D=B**2-4*A*C
            IF(D.LT.0) RETURN
            D=SQRT(D)
            X(1)=(-B+D)/2/A
            X(2)=(-B-D)/2/A
            DO I=1,2
               A=(X(I)*COS(TH)-T_CFI(0)*SIN(TH))*CTHC
               B=(X(I)*SIN(TH)+T_CFI(0)*COS(TH))*STHC
               D=T_SFI(0)*STHC
               CFIC=A*B/(B**2+D**2)
               KZ=COS(TH)*CTHC-SIN(TH)*STHC*CFIC
               IF(KZ.LT.0) X(I)=SIGN(XINF,X(3-I)-X(I))
            ENDDO
         ENDIF
      ENDIF

      X1D=T_R0(1)+DZ*X(1)
      X2D=T_R0(1)+DZ*X(2)
      IF(X1D.LT.X2D) THEN
         XMI=MAX(X1D,XMI)
         XMA=MIN(X2D,XMA)
      ELSE
         XMI=MAX(X2D,XMI)
         XMA=MIN(X1D,XMA)
      ENDIF
      OK=XMI.LT.XMA

      END


      REAL FUNCTION UNFOLD(X,NX,A)

      IMPLICIT NONE
      REAL X, A
      INTEGER NX

      IF(MOD(NX,2).EQ.0) THEN
         UNFOLD=NX*A+X
      ELSE
         UNFOLD=NX*A-X
      ENDIF

      END


      SUBROUTINE XMIRR_REFLECT_3D(X0,Z0,NXM,XM,ZD,XD,TLEN,OK) 

      IMPLICIT NONE
      REAL X0, Z0               ! start point [in]
      INTEGER NXM               ! x-reflections before mirror
      REAL XM                   ! x-coordinate at mirror [in]
      REAL ZD                   ! end point z-coordinate [in]
      REAL XD, TLEN             ! end point x-coord., prop. length [out]
      LOGICAL OK                ! status [out]
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"
#include "TOP_TOPF.fi"
      REAL RC(3), RMI, NORM(3), RM(3), DIR(3)
      REAL DZ, DXDZ, TL(2), XH(2), TLA
      REAL UNFOLD
      INTEGER I, K
      INTEGER TOPSOLU

      OK=.FALSE.

      RC(1)=UNFOLD(QBAR_XCMI(MODUL),NXM,QBAR_A(MODUL))
      RC(2)=QBAR_YCMI(MODUL)
      RC(3)=QBAR_ZCMI(MODUL)
      RMI=QBAR_RMI(MODUL)

      IF((X0-RC(1))**2+(Z0-RC(3))**2.GE.RMI**2) RETURN

      NORM(1)=(XM-RC(1))/RMI
      NORM(2)=0.
      NORM(3)=QBAR_MIPOS(MODUL)*SQRT(1-NORM(1)**2)
      DO I=1,3
         RM(I)=RC(I)+RMI*NORM(I)
      ENDDO

      DZ=RM(3)-Z0
      IF(ABS(DZ).LT.1E-10) RETURN
      DXDZ=(RM(1)-X0)/DZ
      NT=0
      K=TOPSOLU(DXDZ,0,0)
      IF(K.EQ.0) RETURN

      DO I=1,2
         DIR(1)=F_KX(I,K)
         DIR(2)=F_KY(I,K)
         DIR(3)=F_KZ(I,K)
         TL(I)=DZ/DIR(3)
         XH(I)=0
         IF(TL(I).LT.0) CYCLE
         CALL REFLECTION(DIR,NORM,DIR)
         TLA=(ZD-RM(3))/DIR(3)
         TL(I)=TL(I)+TLA
         XH(I)=RM(1)+TLA*DIR(1)
      ENDDO

      I=1
      IF(TL(2).LT.TL(1)) I=2
      IF(TL(I).LT.0) I=3-I
      TLEN=TL(I)
      XD=XH(I)-NXM*QBAR_A(MODUL)

      OK=TLEN.GT.0

      END



      
