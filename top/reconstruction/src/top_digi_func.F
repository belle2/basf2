!-----------------------------------------------------------------------
! TOP counter for Belle: Digitization - basic functions and inverse functions
! M. Staric, sept-2008, sept-2011, avg-2014
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
!-----------------------------------------------------------------------

      INTEGER FUNCTION ICH_DIGI(XH,YH,LR,M) ! chID from x,y,lr,m

      IMPLICIT NONE
      REAL XH, YH               ! photon hit coordinates (Q-bar local frame)
      INTEGER LR                ! left/right = 0/1
      INTEGER M                 ! module ID number
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
      REAL X, Y
      INTEGER IX, IY, IPMTX, IPMTY
      INTEGER ICHAN

      ICH_DIGI=-1
      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN
      IF(N_PMT(LR,M).EQ.0) RETURN

      X=XH-PMT_X0(LR,M)
      IPMTX=NINT(X/PMT_DX(LR,M))
      IF(IPMTX.LT.0.OR.IPMTX.GE.N_PMTX(LR,M)) RETURN
      X=X-IPMTX*PMT_DX(LR,M)
      IF(ABS(X).GE.PMT_DELX) RETURN
      IX=IFIX((X+PMT_DELX)/PMT_DXCH)

      Y=YH-PMT_Y0(LR,M)
      IPMTY=NINT(Y/PMT_DY(LR,M))
      IF(IPMTY.LT.0.OR.IPMTY.GE.N_PMTY(LR,M)) RETURN
      Y=Y-IPMTY*PMT_DY(LR,M)
      IF(ABS(Y).GE.PMT_DELY) RETURN
      IY=IFIX((Y+PMT_DELY)/PMT_DYCH)

      IX=PMT_NX-1-IX ! increase as phi, e.g. opposite to x
      IPMTX=N_PMTX(LR,M)-1-IPMTX ! increase as phi, e.g. opposite to x
      ICH_DIGI=ICHAN(IX,IY,IPMTX,IPMTY,LR,M)

      END


      INTEGER FUNCTION ICH_DIGIZ(XH,YH,ZH,M) ! chID from x,y,z,m
      
      IMPLICIT NONE
      REAL XH, YH, ZH           ! photon hit coordinates (Q-bar local frame)
      INTEGER M                 ! module ID number
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
      INTEGER LR, ICH_DIGI

      IF(ZH.LT.0) THEN
         LR=0
      ELSE
         LR=1
      ENDIF
      ICH_DIGIZ=ICH_DIGI(XH,YH,LR,M)
      END


      SUBROUTINE ICHXYZ(ICH,M,XH,YH,ZH) ! inverse of ICH_DIGIZ

      IMPLICIT NONE
      INTEGER ICH, M            ! channel and module ID [in]
      REAL XH, YH, ZH           ! photon hit coordinates (Q-bar local frame)
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
      INTEGER IX, IY, IPMTX, IPMTY, LR
      REAL X, Y

      XH=0.
      YH=0.
      ZH=0.
      IF(ICH.LT.0) RETURN
      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN

      CALL ICHIII(ICH,M,IX,IY,IPMTX,IPMTY,LR)
      IF(LR.LT.0) RETURN

      IX=PMT_NX-1-IX ! increase as phi, e.g. opposite to x
      IPMTX=N_PMTX(LR,M)-1-IPMTX ! increase as phi, e.g. opposite to x

      X=(2*IX+1-PMT_NX)*PMT_DXCH/2.
      Y=(2*IY+1-PMT_NY)*PMT_DYCH/2.
      XH=X+IPMTX*PMT_DX(LR,M)+PMT_X0(LR,M)
      YH=Y+IPMTY*PMT_DY(LR,M)+PMT_Y0(LR,M)
      ZH=(LR-0.5)*QBAR_C(M)

      END


      INTEGER FUNCTION ICHAN(IX,IY,IPMTX,IPMTY,LR,M) ! chID from ix,iy,ipmt,lr,m

      IMPLICIT NONE
      INTEGER IX, IY            ! PMT channel number (2D)
      INTEGER IPMTX, IPMTY      ! PMT ID number (2D)
      INTEGER LR                ! exit window ID: left/right = 0/1
      INTEGER M                 ! module ID
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
      INTEGER I, J, NX

      I=IX+IPMTX*PMT_NX
      J=IY+IPMTY*PMT_NY
      NX=N_PMTX(LR,M)*PMT_NX
      ICHAN=I+J*NX
      IF(LR.EQ.0) RETURN
      ICHAN=ICHAN+QBAR_NCH0(M)

      END


      SUBROUTINE ICHIII(ICH,M,IX,IY,IPMTX,IPMTY,LR) ! inverse of ICHAN

      IMPLICIT NONE
      INTEGER ICH, M            ! channel and module ID [in]
      INTEGER IX, IY            ! PMT channel number [out]
      INTEGER IPMTX, IPMTY      ! PMT ID number [out]
      INTEGER LR                ! exit window ID (left/right = 0/1, ERR=-1) [out]
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
      INTEGER K, I, J, NX

      IX=-1
      IY=-1
      IPMTX=-1
      IPMTY=-1
      LR=-1
      IF(ICH.LT.0) RETURN
      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN

      IF(ICH.LT.QBAR_NCH0(M)) THEN
         LR=0
         K=ICH
      ELSE
         LR=1
         K=ICH-QBAR_NCH0(M)
      ENDIF
      IF(N_PMT(LR,M).EQ.0) RETURN

      NX=N_PMTX(LR,M)*PMT_NX
      I=MOD(K,NX)
      J=K/NX
      IX=MOD(I,PMT_NX)
      IPMTX=I/PMT_NX
      IY=MOD(J,PMT_NY)
      IPMTY=J/PMT_NY
      IF(IPMTY.GE.N_PMTY(LR,M)) LR=-1

      END


      INTEGER FUNCTION TDC_DIGI(T) ! TDC from T

      IMPLICIT NONE
      REAL T                    ! time
#include "TOP_TDC.fi"

      TDC_DIGI=TDC_RANGE
      IF(T.LT.0.OR.T.GT.TIME_RANGE) RETURN
      TDC_DIGI=IFIX(T/TDC_DT)
      END

      
      REAL FUNCTION TDC_TO_T(TDC) ! inverse of TDC_DIGI

      IMPLICIT NONE
      INTEGER TDC               ! TDC time (digitized)
#include "TOP_TDC.fi"
      
      TDC_TO_T=(TDC+0.5)*TDC_DT-TDC_T0
      END

