!-----------------------------------------------------------------------
! TOP counter for Belle: Reconstruction - construct PDF in y-slices
! M. Staric, nov-2008, jun-2009, feb-2010, dec-2013, apr-2014, avg-2017
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE PDF_YSKEN(I0,K0,K1,K2,K3,XD,DXCH,ZB,ZD,IX,IPMTX,LR,TYP)
      
      IMPLICIT NONE
      INTEGER I0, K0, K1, K2, K3
      REAL XD, DXCH, ZB, ZD
      INTEGER IX, IPMTX, LR
      INTEGER TYP               ! 0=not reflected, 1=PLM, 2=CYL, 3=SPH
#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_PMT.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_PAR.fi"
#include "TOP_TOPF.fi"
#include "TOP_YBUF.fi"
#include "TOP_QEFF.fi"
      REAL TOPF_DTL, TOPF_DYD, TOPF_DYB, TOPF_DFIC
      INTEGER TOPF_RAYTR

      INTEGER K4, K, NX(3), NY(3)
      REAL TL0, YD0, YB0, FIC0
      REAL KY
      
      REAL DTLDX, DTLDL, DTLDE, DTDTHC
      REAL DYDDX, DYDDL, DYDDE, DYDZ
      REAL DYBDX, DYBDL, DYBDE
      REAL DFICDX, DFICDE

      REAL DELY, DELY_E
      REAL DFI, DTDE, DTDL, DTDX, WID0, DEL_E, YD
      REAL TL, TIM, WID, FOT, FIC, CTH
      REAL GROUP_INDEX, GET_LOSSES

      LOGICAL SCAN_FLAG, IS_SCAN

      DFICDX=TOPF_DFIC(I0,K0,K1)
      K4=TOPF_RAYTR(I0,K0,DFICDX)
      IF(K4.EQ.0) RETURN

      TL0=F_TL(I0,K4)
      TIM=TL0*GRU_IND/C0+TOF
      IF(TIM.GT.T_MAX) RETURN

      DYDDX=TOPF_DYD(I0,K0,K1)
      DYDDL=TOPF_DYD(I0,K0,K2)
      DYDDE=TOPF_DYD(I0,K0,K3)
      DELY_E=ABS(DYDDE*(EMAX-EMIN))
      DELY=DELY_E+ABS(DYDDX*DXCH)+ABS(DYDDL*T_LEN)
      IF(DELY.EQ.0) RETURN

      DTLDE=TOPF_DTL(I0,K0,K3)
      DTDE=(DTLDE+TL0*REL_DNGDE)*GRU_IND/C0 ! dispersion (cromatic)
      DTLDX=TOPF_DTL(I0,K0,K1)
      DTDX=DTLDX*GRU_IND/C0     ! channel x-size
      DTLDL=TOPF_DTL(I0,K0,K2)
      DTDL=(DTLDL*GRU_IND+1/BETA)/C0 ! parallax
      DTDTHC=DTLDE*D_E/D_THC*GRU_IND/C0 ! multiple scattering in Q-bar
      WID0=(DTDX*DXCH)**2/12.+(DTDL*SIG_TLEN)**2+(DTDTHC*TH0_SIP)**2
      WID=WID0+(SIG_E*DTDE)**2

      IF(PDF_OPT.EQ.0) THEN
         SCAN_FLAG=.FALSE.
      ELSE IF(PDF_OPT.EQ.1) THEN
         SCAN_FLAG=.TRUE.
      ELSE
         SCAN_FLAG=DELY_E/DELY.GT.0.1.AND.DELY.LT.100*QBAR_B(MODUL)
         IF(SCAN_FLAG) SCAN_FLAG=IS_SCAN(TIM,WID,IX,IPMTX)
      ENDIF

      IF(SCAN_FLAG) THEN
         YD0=F_YD(I0,K4)
         IF(ZB.EQ.ZD) THEN
            CALL YSKEN(IX,IPMTX,LR,MODUL,
     &           YD0,DYDDE,DYDDL,DYDDX,
     &           NBUF,SUM,E0,SIGE2,ICHA,ECPP,NP)
         ELSE
            YB0=F_YB(I0,K4)
            DYBDX=TOPF_DYB(I0,K0,K1)
            DYBDL=TOPF_DYB(I0,K0,K2)
            DYBDE=TOPF_DYB(I0,K0,K3)
            IF(EXT_SHAPE(QBAR_EXT(LR,MODUL)).EQ.2) THEN
               KY=SIGN(F_DIRB(2),F_KY(I0,K4)) ! unfold in y
               DYDZ=KY/F_DIRB(3) ! at entrance to prism
               CALL YSKENX(IX,IPMTX,LR,MODUL,
     &              YD0,DYDDE,DYDDL,DYDDX,DYDZ,
     &              YB0,DYBDE,DYBDL,DYBDX,
     &              NBUF,SUM,E0,SIGE2,ICHA,ECPP,NP)
            ELSE
               CALL YSKENE(IX,IPMTX,LR,MODUL,
     &              YD0,DYDDE,DYDDL,DYDDX,
     &              YB0,DYBDE,DYBDL,DYBDX,
     &              NBUF,SUM,E0,SIGE2,ICHA,ECPP,NP)
            ENDIF
         ENDIF
      ELSE
         CALL NO_YSKEN(IX,IPMTX,LR,MODUL,
     &        NBUF,SUM,E0,SIGE2,ICHA,ECPP,NP)
      ENDIF

      NX(1)=F_NX(I0,K4)
      NX(2)=F_NXM(I0,K4)
      NX(3)=F_NXE(I0,K4)
      NY(1)=F_NY(I0,K4)
      NY(2)=F_NYM(I0,K4)
      NY(3)=F_NYE(I0,K4)

      FIC0=ATAN2(F_SFIC(I0,K4),F_CFIC(I0,K4))
      IF(FIC0.LT.0) FIC0=FIC0+2*PI

      DFICDE=TOPF_DFIC(I0,K0,K3)
      DFICDX=TOPF_DFIC(I0,K0,K1)
      DFI=ABS(DFICDX)*DXCH

      CTH=ABS(F_DIR(3))

      DO K=1,NP
         DEL_E=E0(K)-E_MEAN
         TL=TL0+DTLDE*DEL_E
         TIM=TL*GROUP_INDEX(E0(K))/C0+TOF
         WID=WID0+SIGE2(K)*DTDE**2
         FOT=FOT0*DFI*SUM(K)*GET_LOSSES(E0(K),TL,CTH,NX(1),NY(1),TYP)
         FIC=FIC0+DFICDE*DEL_E
         CALL TO_FIRST_PERIOD(FIC)
         YD=YD0+DYDDE*DEL_E
         CALL PATT_PUSH(ICHA(K),TIM,WID,FOT,FIC,E0(K),SIGE2(K),TYP,
     &        NX,NY,XD,YD)
      ENDDO

      END


      SUBROUTINE PATT_PUSH(ICH,TIM,WID,FOT,FIC,E,SIGE2,TYP,NX,NY,XD,YD)

      IMPLICIT NONE
      INTEGER ICH
      REAL TIM, WID, FOT
      REAL FIC, E, SIGE2
      INTEGER TYP
      INTEGER NX(3), NY(3)
      REAL XD, YD
#include "TOP_PIK.fi"
#include "TOP_TDC.fi"
      INTEGER N
      
      IF(TIM.LT.0.OR.TIM.GT.T_MAX) RETURN
      IF(ICH.LT.0.OR.ICH.GE.NUM_CHA) RETURN
      IF(WID.LE.0) RETURN
      IF(FOT.LE.0) RETURN

      N=NUM_PIK(ICH)+1
      IF(N.GT.MAX_PIK) THEN
         PIK_OVFL(ICH)=PIK_OVFL(ICH)+1
         RETURN
      ENDIF
      PIK_POZ(N,ICH)=TIM
      PIK_WID(N,ICH)=WID+TDC_DT**2/12.
      PIK_NPH(N,ICH)=FOT
      PIK_FIC(N,ICH)=FIC
      NUM_PIK(ICH)=N

      IF(STORE_OPT) RETURN

      PIK_E(N,ICH)=E
      PIK_SIGE(N,ICH)=SIGE2
      PIK_NX(N,ICH)=NX(1)
      PIK_NY(N,ICH)=NY(1)
      PIK_NXM(N,ICH)=NX(2)
      PIK_NYM(N,ICH)=NY(2)
      PIK_NXE(N,ICH)=NX(3)
      PIK_NYE(N,ICH)=NY(3)
      PIK_XD(N,ICH)=XD
      PIK_YD(N,ICH)=YD
      PIK_TYP(N,ICH)=TYP

      END


      REAL FUNCTION GET_LOSSES(E,X,CTH,NX,NY,TYP)

c--   get losses due to:
c--   mirror reflectivity, total internal reflectivity, bulk absorption

      IMPLICIT NONE
      REAL E                    ! photon energy [eV]
      REAL X                    ! propagation length [cm]
      REAL CTH                  ! cos of impact angle to PMT
      INTEGER NX, NY            ! number of reflections
      INTEGER TYP               ! 0=not reflected, 1=PLM, 2=CYL, 3=SPH
#include "TOP_PAR.fi"
#include "TOP_CONS.fi"
      REAL LOST

      REAL MIRROR_REFL, ABSPT_LENG
      REAL TOTREFL
      DATA TOTREFL /0.9994/

      LOST=TOTREFL**(ABS(NX)+ABS(NY))
      IF(TYP.NE.0) LOST=LOST*MIRROR_REFL(E)
      LOST=LOST*EXP(-X/ABSPT_LENG(E))

      GET_LOSSES=LOST

      END


      LOGICAL FUNCTION IS_SCAN(TIM,WID,IX,IPMTX)

      IMPLICIT NONE
      REAL TIM, WID
      INTEGER IX, IPMTX

#include "TOP_PHTIM.fi"
#include "TOP_PMT.fi"
      INTEGER ICHX, K, I
      REAL SIG, X

      IS_SCAN=.TRUE.
      IF(FORCE_SCAN) RETURN

      ICHX=IPMTX*PMT_NX+IX
      IF(ICHX.GE.NUM_CHX) RETURN
      IF(NUM_PHOT(ICHX).GT.MAX_PHOT) RETURN

      DO K=1,NUM_PHOT(ICHX)
         DO I=1,TTS_NG
            SIG=WID+TTS_SIG(I)**2+PH_TERRS(K,ICHX)**2
            X=(PH_TIMES(K,ICHX)-TIM-TTS_T0(I))**2/SIG
            IF(X.LT.10) RETURN
         ENDDO
      ENDDO

      IS_SCAN=.FALSE.

      END


      SUBROUTINE NO_YSKEN(IX,IPMTX,LR,M,NB,SUM,E0,SIG2,ICHA,ECPP,NT)
      
      IMPLICIT NONE
      INTEGER IX, IPMTX, LR, M
      INTEGER NB
      REAL SUM(NB), E0(NB), SIG2(NB)
      INTEGER ICHA(NB), NT
      REAL ECPP(NB)
#include "TOP_PAR.fi" 
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_PMT.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_QEFF.fi" 
      INTEGER ICHAN
      INTEGER IPMTY, IY, ICH, ID
      REAL DY, WINDIM
      
      NT=0
      WINDIM=QBAR_B(M)
      ID=QBAR_EXT(LR,M)
      IF(ID.NE.0) WINDIM=EXT_YUP(ID)-EXT_YDN(ID)
      DO IPMTY=0,N_PMTY(LR,M)-1
         DO IY=0,PMT_NY-1
            ICH=ICHAN(IX,IY,IPMTX,IPMTY,LR,M)
            DY=PIK_DYCH(ICH)
            IF(DY.LE.0) CYCLE
            IF(NT.GE.NB) GOTO 999
            NT=NT+1
            SUM(NT)=DY/WINDIM
            E0(NT)=E_MEAN
            SIG2(NT)=SIG_E**2
            ICHA(NT)=ICH
            ECPP(NT)=E_MEAN
         ENDDO
      ENDDO
      RETURN

 999  CONTINUE
      PRINT*,'NO_YSKEN: NT>NB ',NT,NB

      END


c--   Qbar without expansion volume at side LR

      SUBROUTINE YSKEN(IX,IPMTX,LR,M,Y0,DYDE,DYDL,DYDX,
     &     NB,SUM,E0,SIG2,ICHA,ECPP,NT)
      
      IMPLICIT NONE
      INTEGER IX, IPMTX, LR, M
      REAL Y0, DYDE, DYDL, DYDX
      INTEGER NB
      REAL SUM(NB), E0(NB), SIG2(NB)
      INTEGER ICHA(NB), NT
      REAL ECPP(NB)
#include "TOP_PAR.fi" 
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_QEFF.fi" 
      REAL DELY, Y, Y1, Y2, YCH, DX, DY, WTAG, DX_PREV, DY_PREV
      REAL ECP, S, S_E, S_E2
      INTEGER ICHAN, ICH
      INTEGER I, IPT
      INTEGER J, J1, J2, IY, IPMTY
      integer NSIZ
      PARAMETER (NSIZ=MAX_NT*2)
      REAL MASK(0:NSIZ-1), WID
      INTEGER MM
      LOGICAL TAG

      NT=0
      IF(DYDE.EQ.0) RETURN

      ICH=ICHAN(IX,0,IPMTX,0,LR,M)
      DX=PIK_DXCH(ICH)
      DELY=(ABS(DYDL)*T_LEN+ABS(DYDX)*DX)/2
      IF(DYDE.GT.0) THEN
         Y1=Y0-DELY+DYDE*(EMIN-E_MEAN)
         Y2=Y0+DELY+DYDE*(EMAX-E_MEAN)
      ELSE
         Y1=Y0-DELY+DYDE*(EMAX-E_MEAN)
         Y2=Y0+DELY+DYDE*(EMIN-E_MEAN)
      ENDIF
      J1=NINT(Y1/QBAR_B(M))
      J2=NINT(Y2/QBAR_B(M))

      DX_PREV=0.
      DY_PREV=0.
      DO IPMTY=0,N_PMTY(LR,M)-1
         DO IY=0,PMT_NY-1
            ICH=ICHAN(IX,IY,IPMTX,IPMTY,LR,M)
            DX=PIK_DXCH(ICH)
            IF(DX.LE.0) CYCLE
            DY=PIK_DYCH(ICH)
            IF(DY.LE.0) CYCLE
            IF(XQE_FLAG) THEN
               YCH=PIK_YCH(ICH)
               IF(BTEST(ABS(J1),0)) YCH=-YCH
               IF(DX.NE.DX_PREV.OR.DY.NE.DY_PREV) THEN
                  DX_PREV=DX
                  DY_PREV=DY
                  WTAG=(QBAR_B(M)+2*ABS(YCH))/ABS(DYDE)
                  TAG=XQE_MAXSEP*WTAG.GT.1
                  IF(TAG) THEN
                     WTAG=MIN(WTAG,EMAX-EMIN)
                     CALL SET_EMASK(DYDE,DYDL,DYDX,DY,T_LEN,DX,WTAG,
     &                    MASK,MM,NSIZ,TAG,WID)
                  ENDIF
               ENDIF
            ELSE
               TAG=.FALSE.
            ENDIF
            IF(TAG) THEN
               IPT=NT
               DO J=J1,J2
                  Y=YCH+J*QBAR_B(M)
                  ECP=(Y-Y0)/DYDE+E_MEAN
                  CALL QE_INTEGRATE(MASK,MM,NSIZ,ECP,WID,S,S_E,S_E2)
                  YCH=-YCH
                  IF(S.EQ.0) CYCLE
                  DO I=IPT+1,NT
                     IF(ABS(ECP-ECPP(I)).LT.WID) EXIT
                  ENDDO
                  IF(I.GT.NT) THEN
                     IF(I.GT.NB) GOTO 999
                     NT=I
                     ECPP(I)=ECP
                     SUM(I)=S
                     E0(I)=S_E
                     SIG2(I)=S_E2
                     ICHA(I)=ICH
                  ELSE
                     ECPP(I)=ECP
                     SUM(I)=SUM(I)+S
                     E0(I)=E0(I)+S_E
                     SIG2(I)=SIG2(I)+S_E2
                  ENDIF
               ENDDO
               DO I=IPT+1,NT
                  E0(I)=E0(I)/SUM(I)
                  SIG2(I)=SIG2(I)/SUM(I)-E0(I)**2
               ENDDO
            ELSE
               IF(NT.GE.NB) GOTO 999
               NT=NT+1
               SUM(NT)=DY/QBAR_B(M)
               E0(NT)=E_MEAN
               SIG2(NT)=SIG_E**2
               ICHA(NT)=ICH
            ENDIF
         ENDDO
      ENDDO
      RETURN

 999  CONTINUE
      PRINT*,'YSKEN: NT>NB ',NT,NB

      END


c--   Qbar with expansion volume at side LR, type Box

      SUBROUTINE YSKENE(IX,IPMTX,LR,M,Y0,DYDE,DYDL,DYDX,
     &     YB0,DYBDE,DYBDL,DYBDX,
     &     NB,SUM,E0,SIG2,ICHA,ECPP,NT)
      
      IMPLICIT NONE
      INTEGER IX, IPMTX, LR, M
      REAL Y0, DYDE, DYDL, DYDX
      REAL YB0, DYBDE, DYBDL, DYBDX
      INTEGER NB
      REAL SUM(NB), E0(NB), SIG2(NB)
      INTEGER ICHA(NB), NT
      REAL ECPP(NB)
#include "TOP_PAR.fi" 
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_PMT.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_QEFF.fi" 
      REAL DELY, Y, Y1, Y2, YCH, DX, DY, YB, DX_PREV, DY_PREV
      REAL ECP, S, S_E, S_E2
      INTEGER ICHAN, ICH
      INTEGER I, IPT
      INTEGER J, J1, J2, IY, IPMTY, K, K1, K2
      integer NSIZ
      PARAMETER (NSIZ=MAX_NT*4)
      REAL MASK(0:NSIZ-1,10), WID, WIDI(10)
      INTEGER MM(10)
      LOGICAL TAG(10)
      REAL DY1, DYBH, DYBDY, YBAR, BH, Y0EXT, Y0_EXT, DY_EXT, YCHE
      REAL DELY1, DELY2, WYB
      LOGICAL TYPE_BOX

      NT=0
      IF(DYDE.EQ.0.OR.DYBDE.EQ.0) RETURN

      ICH=ICHAN(IX,0,IPMTX,0,LR,M)
      DX=PIK_DXCH(ICH)
      DELY=(ABS(DYBDL)*T_LEN+ABS(DYBDX)*DX)/2
      IF(DYBDE.GT.0) THEN
         Y1=YB0-DELY+DYBDE*(EMIN-E_MEAN)
         Y2=YB0+DELY+DYBDE*(EMAX-E_MEAN)
      ELSE
         Y1=YB0-DELY+DYBDE*(EMAX-E_MEAN)
         Y2=YB0+DELY+DYBDE*(EMIN-E_MEAN)
      ENDIF
      J1=NINT(Y1/QBAR_B(M))
      J2=NINT(Y2/QBAR_B(M))
      BH=QBAR_B(M)/2
      DYBDY=DYBDE/DYDE

      K=QBAR_EXT(LR,M)
      TYPE_BOX=EXT_SHAPE(K).EQ.1
      IF(TYPE_BOX) THEN
         Y0_EXT=(EXT_YUP(K)+EXT_YDN(K))/2
         DY_EXT=EXT_YUP(K)-EXT_YDN(K)
         DELY=(ABS(DYDL)*T_LEN+ABS(DYDX)*DX)/2
         IF(DYDE.GT.0) THEN
            DELY1=DELY+DYDE*(E_MEAN-EMIN)
            DELY2=DELY+DYDE*(EMAX-E_MEAN)
         ELSE
            DELY1=DELY-DYDE*(EMAX-E_MEAN)
            DELY2=DELY-DYDE*(E_MEAN-EMIN)
         ENDIF
      ELSE
         Y0_EXT=0.
         DY_EXT=0.
      ENDIF 

      DX_PREV=0.
      DY_PREV=0.
      DO IPMTY=0,N_PMTY(LR,M)-1
         DO IY=0,PMT_NY-1
            ICH=ICHAN(IX,IY,IPMTX,IPMTY,LR,M)
            DX=PIK_DXCH(ICH)
            IF(DX.LE.0) CYCLE
            DY=PIK_DYCH(ICH)
            IF(DY.LE.0) CYCLE
            YCH=PIK_YCH(ICH)
            Y0EXT=Y0_EXT
            IF(BTEST(ABS(J1),0)) THEN
               YCH=-YCH
               Y0EXT=-Y0EXT
            ENDIF
            IF(DX.NE.DX_PREV.OR.DY.NE.DY_PREV) THEN
               DX_PREV=DX
               DY_PREV=DY
               DYBH=DYBDY*DY/2
               WYB=BH+DYBH
               DO I=1,10
                  TAG(I)=.FALSE.
               ENDDO
               CALL SET_EMASK(DYDE,DYDL,DYDX,DY,T_LEN,DX,0.,
     &              MASK(0,10),MM(10),NSIZ,TAG(10),WIDI(10))
            ENDIF
            IPT=NT
            DO J=J1,J2
               YBAR=J*QBAR_B(M)
               IF(TYPE_BOX) THEN
                  Y1=Y0-YBAR-DELY1
                  Y2=Y0-YBAR+DELY2
                  K1=NINT((Y1-Y0EXT)/DY_EXT)
                  K2=NINT((Y2-Y0EXT)/DY_EXT)
                  YCHE=YCH
                  IF(BTEST(ABS(K1),0)) YCHE=2*Y0EXT-YCHE
               ELSE
                  K1=0
                  K2=0
                  YCHE=YCH
               ENDIF
               DO K=K1,K2
                  Y=YCHE+YBAR+K*DY_EXT-Y0
                  YB=YB0+Y*DYBDY-YBAR
                  YCHE=2*Y0EXT-YCHE
                  IF(ABS(YB).GT.WYB) CYCLE
                  Y1=MAX(YB-DYBH,-BH)
                  Y2=MIN(YB+DYBH, BH)
                  DY1=(Y2-Y1)/DYBDY
                  I=MIN(IFIX(DY1/DY*10.),10)
                  IF(I.LE.0) CYCLE
                  IF(.NOT.TAG(I)) THEN
                     CALL SET_EMASK(DYDE,DYDL,DYDX,DY1,T_LEN,DX,0.,
     &                    MASK(0,I),MM(I),NSIZ,TAG(I),WIDI(I))
                  ENDIF
                  ECP=((Y1+Y2)/2+YBAR-YB0)/DYBDE+E_MEAN
                  CALL QE_INTEGRATE(MASK(0,I),MM(I),NSIZ,ECP,WIDI(I),
     &                 S,S_E,S_E2)
                  IF(S.LE.0) CYCLE
                  ECP=S_E/S
                  WID=(S_E2/S-ECP**2)*4
                  DO I=IPT+1,NT
                     IF((ECP-ECPP(I))**2.LT.WID) EXIT
                  ENDDO
                  IF(I.GT.NT) THEN
                     IF(I.GT.NB) GOTO 999
                     NT=NT+1
                     ECPP(NT)=ECP
                     SUM(NT)=S
                     E0(NT)=S_E
                     SIG2(NT)=S_E2
                     ICHA(NT)=ICH
                  ELSE
                     ECPP(I)=ECP
                     SUM(I)=SUM(I)+S
                     E0(I)=E0(I)+S_E
                     SIG2(I)=SIG2(I)+S_E2
                  ENDIF
               ENDDO
               YCH=-YCH
               Y0EXT=-Y0EXT
            ENDDO
         ENDDO
      ENDDO
      DO I=1,NT
         E0(I)=E0(I)/SUM(I)
         SIG2(I)=SIG2(I)/SUM(I)-E0(I)**2
      ENDDO
      RETURN

 999  CONTINUE
      PRINT*,'YSKENE: NT>NB ',NT,NB

      END



c--   Qbar with expansion volume at side LR, type Prism

      SUBROUTINE YSKENX(IX,IPMTX,LR,M,
     &     YD0,DYDE,DYDL,DYDX,DYDZ,
     &     YB0,DYBDE,DYBDL,DYBDX,
     &     NB,SUM,E0,SIG2,ICHA,ECPP,NT)
      
      IMPLICIT NONE
      INTEGER IX, IPMTX, LR, M
      REAL YD0, DYDE, DYDL, DYDX, DYDZ
      REAL YB0, DYBDE, DYBDL, DYBDX
      INTEGER NB
      REAL SUM(NB), E0(NB), SIG2(NB)
      INTEGER ICHA(NB), NT
      REAL ECPP(NB)
#include "TOP_PAR.fi" 
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_PMT.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_QEFF.fi" 
      REAL YCH, DXCH, DYCH, DX_PREV
      REAL DELY, Y1, Y2, YB, DYB, DY, YBAR, BH, DYDZ_TMP
      REAL ECP, S, S_E, S_E2
      INTEGER ICHAN, ICH
      INTEGER I, IPT
      INTEGER J, J1, J2, IY, IPMTY, K, KMI, KMA, K1, K2
      
      integer NSIZ
      PARAMETER (NSIZ=MAX_NT*4)
      integer NMASKS
      PARAMETER (NMASKS=100)
      REAL MASK(0:NSIZ-1,NMASKS), WID, WIDI(NMASKS)
      INTEGER MM(NMASKS)
      LOGICAL TAG(NMASKS), OK
      LOGICAL EVEN
      
      
      NT=0
      IF(DYDE.EQ.0.OR.DYBDE.EQ.0) RETURN
      
      ICH=ICHAN(IX,0,IPMTX,0,LR,M)
      DXCH=PIK_DXCH(ICH)
      DELY=(ABS(DYBDL)*T_LEN+ABS(DYBDX)*DXCH)/2
      IF(DYBDE.GT.0) THEN
         Y1=YB0-DELY+DYBDE*(EMIN-E_MEAN)
         Y2=YB0+DELY+DYBDE*(EMAX-E_MEAN)
      ELSE
         Y1=YB0-DELY+DYBDE*(EMAX-E_MEAN)
         Y2=YB0+DELY+DYBDE*(EMIN-E_MEAN)
      ENDIF
      J1=NINT(Y1/QBAR_B(M))
      J2=NINT(Y2/QBAR_B(M))
      BH=QBAR_B(M)/2

      CALL UNFOLD_EXTVOL(M,LR,KMI,KMA)
      
      DX_PREV=0.
      DO IPMTY=0,N_PMTY(LR,M)-1
         DO IY=0,PMT_NY-1
            ICH=ICHAN(IX,IY,IPMTX,IPMTY,LR,M)
            DXCH=PIK_DXCH(ICH)
            IF(DXCH.LE.0) CYCLE
            DYCH=PIK_DYCH(ICH)
            IF(DYCH.LE.0) CYCLE
            YCH=PIK_YCH(ICH)
            IF(DXCH.NE.DX_PREV) THEN
               DX_PREV=DXCH
               DO I=1,NMASKS
                  TAG(I)=.FALSE.
               ENDDO
            ENDIF
            IPT=NT

            DO J=J1,J2
               YBAR=J*QBAR_B(M)
               EVEN=MOD(ABS(J),2).EQ.0
               IF(EVEN) THEN
                  DYDZ_TMP=DYDZ
               ELSE
                  DYDZ_TMP=-DYDZ
               ENDIF
               IF(DYDZ_TMP*(1+2*(LR-1)).GT.0) THEN
                  K1=0
                  K2=KMA
               ELSE
                  K1=KMI
                  K2=0
               ENDIF
               DO K=K1,K2
                  CALL TO_BAR_EXIT(YCH,DYCH,BH,K,DYDZ_TMP,YB,DYB)
                  IF(.NOT.EVEN) YB=-YB
                  I=MIN(NINT(DYB/2/BH*NMASKS),NMASKS)
                  IF(I.LT.1) CYCLE
                  IF(.NOT.TAG(I)) THEN
                     DYB=2*BH/FLOAT(NMASKS)*FLOAT(I)
                     DY=DYB/DYBDE*DYDE
                     CALL SET_EMASK(DYDE,DYDL,DYDX,DY,T_LEN,DXCH,0.,
     &                    MASK(0,I),MM(I),NSIZ,TAG(I),WIDI(I))
                  ENDIF
                  ECP=(YB+YBAR-YB0)/DYBDE+E_MEAN
                  CALL QE_INTEGRATE(MASK(0,I),MM(I),NSIZ,ECP,WIDI(I),
     &                 S,S_E,S_E2)
                  IF(S.LE.0) CYCLE
                  ECP=S_E/S
                  WID=(S_E2/S-ECP**2)*4
                  DO I=IPT+1,NT
                     IF((ECP-ECPP(I))**2.LT.WID) EXIT
                  ENDDO
                  IF(I.GT.NT) THEN
                     IF(I.GT.NB) GOTO 999
                     NT=NT+1
                     ECPP(NT)=ECP
                     SUM(NT)=S
                     E0(NT)=S_E
                     SIG2(NT)=S_E2
                     ICHA(NT)=ICH
                 ELSE
                     ECPP(I)=ECP
                     SUM(I)=SUM(I)+S
                     E0(I)=E0(I)+S_E
                     SIG2(I)=SIG2(I)+S_E2
                  ENDIF
               ENDDO
            ENDDO
         ENDDO
      ENDDO
      DO I=1,NT
         E0(I)=E0(I)/SUM(I)
         SIG2(I)=SIG2(I)/SUM(I)-E0(I)**2
      ENDDO

      RETURN

 999  CONTINUE
      PRINT*,'YSKENX: NT>NB ',NT,NB

      END



      SUBROUTINE SET_EMASK(DYDE,DYDL,DYDX,DY,DL,DX,WTAG,
     &     P,NT,NSIZ,TAG,WID)

      IMPLICIT NONE
      REAL DYDE, DYDL, DYDX
      REAL DY, DL, DX, WTAG
      REAL P(*)                 ! mask
      INTEGER NT, NSIZ          ! size of mask, size of array P
      LOGICAL TAG
      REAL WID
#include "TOP_QEFF.fi" 
      REAL W, A, B, C, WY, E
      REAL CONV_SQUARES
      INTEGER I

      NT=0
      IF(DY.LE.0) RETURN

      CALL SET_MASK_PAR(DYDE,DYDL,DYDX,DY,DL,DX,A,B,C,WY,WID)

      TAG=A.LT.WTAG.OR.WTAG.EQ.0
      IF(.NOT.TAG) RETURN

      CALL SET_MASK(A,B,C,WY,P,NT,NSIZ)

      END


      SUBROUTINE SET_MASK_PAR(DYDE,DYDL,DYDX,DY,DL,DX,A,B,C,WY,WID)

      IMPLICIT NONE
      REAL DYDE, DYDL, DYDX     ! input
      REAL DY, DL, DX           ! input
      REAL A, B, C, WY, WID     ! output
#include "TOP_QEFF.fi" 
      REAL W
      REAL CONV_SQUARES

      IF(DY.LE.0) THEN
         A=0.
         B=0.
         C=0.
         WY=0.
         WID=E_STEP*4
         RETURN
      ENDIF

      WY=DY/ABS(DYDE)
      A=WY
      W=ABS(DL*DYDL/DYDE)
      IF(W.GT.A) THEN
         B=A
         A=W
      ELSE
         B=W
      ENDIF
      W=ABS(DX*DYDX/DYDE)
      IF(W.GT.A) THEN
         C=B
         B=A
         A=W
      ELSE IF(W.GT.B) THEN
         C=B
         B=W
      ELSE
         C=W
      ENDIF

      W=(A+B+C)/2
      WID=2*MAX(W,E_STEP*2)

      END


      SUBROUTINE SET_MASK(A,B,C,WY,P,NT,NSIZ)

      IMPLICIT NONE
      REAL A, B, C, WY          ! input, must be sorted: A>B>C
      REAL P(*)                 ! mask
      INTEGER NT, NSIZ          ! size of mask, size of array P
#include "TOP_QEFF.fi" 
      REAL W, E
      REAL CONV_SQUARES
      INTEGER I

      W=(A+B+C)/2
      IF(W/E_STEP.LT.NSIZ) THEN
         NT=IFIX(W/E_STEP)+1
         E=0.
         DO I=1,NT
            P(I)=CONV_SQUARES(E,A,B,C)*WY
            E=E+E_STEP
         ENDDO
      ELSE
         NT=NSIZ+1
         P(1)=A
         P(2)=B
         P(3)=C
         P(4)=WY
      ENDIF

      END


      REAL FUNCTION CONV_SQUARES(X,A,B,C)

c--   normalized convolution of three square functions of full width A, B and C
c--   sorting required: A>B>C
      
      IMPLICIT NONE
      REAL X, A, B, C
      REAL CONV_2SQUARE, CONV_3SQUARE

      IF(A.GT.0) THEN
         IF(C/A.GT.0.001) THEN
            CONV_SQUARES=CONV_3SQUARE(A,B,C,X)
         ELSE
            CONV_SQUARES=CONV_2SQUARE(A,B,X)
         ENDIF
      ELSE
         CONV_SQUARES=0.
      ENDIF
      END


      REAL FUNCTION CONV_2SQUARE(A,B,X)

c--   normalized convolution of two square functions of full width A and B
c--   sorting required: A>B

      IMPLICIT NONE
      REAL A, B, X
      REAL X1, X2, ABSX

      X1=(A-B)/2
      X2=(A+B)/2
      ABSX=ABS(X)

      IF(ABSX.LT.X1) THEN
         CONV_2SQUARE=1./A
      ELSE IF(ABSX.LT.X2) THEN
         CONV_2SQUARE=(X2-ABSX)/(X2-X1)/A
      ELSE
         CONV_2SQUARE=0.
      ENDIF
      END


      REAL FUNCTION CONV_3SQUARE(A,B,C,X)

c--   normalized convolution of three square functions of full width A, B and C
c--   sorting required: A>B>C

      IMPLICIT NONE
      REAL A, B, C, X
      REAL T1, T2, T3, T4, P, ABC

      ABC=(A+B+C)/2
      IF(ABS(X).LT.ABC) THEN
         T1=ABC-X
         T2=ABC+X
         P=T1**2+T2**2
         T3=T1-A
         T4=T2-A
         P=P-(SIGN(T3**2,T3)+SIGN(T4**2,T4))
         T3=T1-B
         T4=T2-B
         P=P-(SIGN(T3**2,T3)+SIGN(T4**2,T4))
         T3=T1-C
         T4=T2-C
         P=P-(SIGN(T3**2,T3)+SIGN(T4**2,T4))
         CONV_3SQUARE=MAX(P/(4*A*B*C),0.)
      ELSE
         CONV_3SQUARE=0.
      ENDIF

      END
      

      SUBROUTINE QE_INTEGRATE(P,N,NSIZ,ECP,WID,S,SE,SEE)

      IMPLICIT NONE
      INTEGER NSIZ
      REAL P(0:NSIZ-1)
      INTEGER N
      REAL ECP, WID
      REAL S, SE, SEE
#include "TOP_QEFF.fi" 
      INTEGER I0, I, K, K1, K2, II
      REAL PP, A, B, C, WY, W, E, WH
      REAL CONV_SQUARES
      
      S=0.
      SE=0.
      SEE=0.
      WH=WID/2
      IF(ECP+WH.LT.EMIN) RETURN
      IF(ECP-WH.GT.EMAX) RETURN

      IF(N.LE.NSIZ) THEN
         I0=NINT((ECP-EMIN)/E_STEP)+1
         K1=MAX(-(N-1),1-I0)
         K2=MIN(N-1,NT_XQE-I0)
         DO K=K1,K2
            I=K+I0
            II=ABS(K)
            S=S+XQE_S(I)*P(II)
            SE=SE+XQE_SE(I)*P(II)
            SEE=SEE+XQE_SEE(I)*P(II)
         ENDDO
      ELSE
         A=P(0)
         IF(A.EQ.0) RETURN
         B=P(1)
         C=P(2)
         WY=P(3)
         W=(A+B+C)/2
         DO I=1,NT_XQE
            E=XQE_E(I)-ECP
            IF(ABS(E).LT.W) THEN
               PP=CONV_SQUARES(E,A,B,C)*WY
               S=S+XQE_S(I)*PP
               SE=SE+XQE_SE(I)*PP
               SEE=SEE+XQE_SEE(I)*PP
            ENDIF
         ENDDO         
      ENDIF

      END


      SUBROUTINE QE_INT_MASK(A,B,C,WY,ECP,S,SE,SEE)

      IMPLICIT NONE
      REAL A, B, C, WY
      REAL ECP
      REAL S, SE, SEE
#include "TOP_QEFF.fi" 
      INTEGER I
      REAL PP, E, W
      REAL CONV_SQUARES

      S=0.
      SE=0.
      SEE=0.
      IF(A.EQ.0) RETURN

      W=(A+B+C)/2
      DO I=1,NT_XQE
         E=XQE_E(I)-ECP
         IF(ABS(E).LT.W) THEN
            PP=CONV_SQUARES(E,A,B,C)*WY
            S=S+XQE_S(I)*PP
            SE=SE+XQE_SE(I)*PP
            SEE=SEE+XQE_SEE(I)*PP
         ENDIF
      ENDDO         

      END


      SUBROUTINE UNFOLD_EXTVOL(MDN,LR,KMIN,KMAX)

      IMPLICIT NONE
      INTEGER MDN, LR
      INTEGER KMIN, KMAX

      INTEGER KK
      REAL SMER_Y, SMER_Z
      REAL ORIG_Y, ORIG_Z
      REAL YCH, DYCH, BH, DYDZ, Y, DY
      INTEGER LUN
      LOGICAL OK
#include "TOP_GEO.fi" 
#include "TOP_EXT.fi" 
      INTEGER ID, II, K, I
      REAL PHI0, PHI1, YSIZ, AA, Z, Y1, Y2, Z1, Z2
      REAL R_PL(3,0:1), NORM(3,0:1)
      integer nsiz
      PARAMETER (NSIZ=20)
      INTEGER KMI, KMA
      REAL R0(3,-NSIZ:NSIZ), SMER(3,-NSIZ:NSIZ), ORIG(3,-NSIZ:NSIZ)
      REAL DY0(-NSIZ:NSIZ), DZ0(-NSIZ:NSIZ)
      REAL Z_MAX
      INTEGER MDN_OLD /-1/, LR_OLD /0/
      SAVE KMI, KMA, R0, SMER, ORIG, DY0, DZ0, MDN_OLD, LR_OLD, Z_MAX

      IF(MDN.EQ.MDN_OLD.AND.LR.EQ.LR_OLD) THEN
         KMIN=KMI
         KMAX=KMA
         RETURN
      ENDIF
      MDN_OLD=MDN
      LR_OLD=LR
      
      SMER(1,0)=0.
      SMER(2,0)=1.
      SMER(3,0)=0.
      KMI=0
      KMA=0

      ID=QBAR_EXT(LR,MDN)
      IF(ID.EQ.0) RETURN
      IF(EXT_SHAPE(ID).EQ.2) THEN
         PHI0=ATAN2(EXT_YDN(ID)-EXT_YDNIN(ID),EXT_DZ(ID))
         PHI1=ATAN2(EXT_YUP(ID)-EXT_YUPIN(ID),EXT_DZ(ID))
      ELSE
         PHI0=0.
         PHI1=0.
      ENDIF
      R_PL(1,0)=0.
      R_PL(2,0)=EXT_YDN(ID)
      R_PL(3,0)=EXT_DZ(ID)
      R_PL(1,1)=0.
      R_PL(2,1)=EXT_YUP(ID)
      R_PL(3,1)=EXT_DZ(ID)
      NORM(1,0)=0.
      NORM(2,0)=COS(PHI0)
      NORM(3,0)=-SIN(PHI0)
      NORM(1,1)=0.
      NORM(2,1)=-COS(PHI1)
      NORM(3,1)=SIN(PHI1)
      II=0
      K=II
      DO I=1,3
         R0(I,K)=R_PL(I,II)
      ENDDO
      DO WHILE(K.GT.-NSIZ)
         CALL REFLECT_PLANE(R_PL,NORM,II)
         K=K-1
         DO I=1,3
            R0(I,K)=R_PL(I,II)
         ENDDO
         IF(R_PL(3,II).LT.0) EXIT
      ENDDO
      KMI=K

      R_PL(1,0)=0.
      R_PL(2,0)=EXT_YDN(ID)
      R_PL(3,0)=EXT_DZ(ID)
      R_PL(1,1)=0.
      R_PL(2,1)=EXT_YUP(ID)
      R_PL(3,1)=EXT_DZ(ID)
      NORM(1,0)=0.
      NORM(2,0)=COS(PHI0)
      NORM(3,0)=-SIN(PHI0)
      NORM(1,1)=0.
      NORM(2,1)=-COS(PHI1)
      NORM(3,1)=SIN(PHI1)
      II=1
      K=II
      DO I=1,3
         R0(I,K)=R_PL(I,II)
      ENDDO
      DO WHILE(K.LT.NSIZ)
         CALL REFLECT_PLANE(R_PL,NORM,II)
         K=K+1
         DO I=1,3
            R0(I,K)=R_PL(I,II)
         ENDDO
         IF(R_PL(3,II).LT.0) EXIT
      ENDDO
      KMA=K-1

      DO K=KMI,KMA+1
         R0(3,K)=R0(3,K)-EXT_DZ(ID)
         IF(LR.EQ.0) R0(3,K)=-R0(3,K)
      ENDDO

      YSIZ=EXT_YUP(ID)-EXT_YDN(ID)
      DO K=KMI,KMA
         IF(MOD(ABS(K),2).EQ.0) THEN
            AA=-EXT_YDN(ID)
            DO I=1,3
               SMER(I,K)=(R0(I,K+1)-R0(I,K))/YSIZ
               ORIG(I,K)=R0(I,K)+AA*SMER(I,K)
            ENDDO
            IF(LR.EQ.0) THEN
               DY0(K)=SMER(3,K)
               DZ0(K)=-SMER(2,K)
            ELSE
               DY0(K)=-SMER(3,K)
               DZ0(K)=SMER(2,K)
            ENDIF
         ELSE
            AA=-EXT_YUP(ID)
            DO I=1,3
               SMER(I,K)=-(R0(I,K+1)-R0(I,K))/YSIZ
               ORIG(I,K)=R0(I,K)+AA*SMER(I,K)
            ENDDO
            IF(LR.EQ.0) THEN
               DY0(K)=-SMER(3,K)
               DZ0(K)=SMER(2,K)
            ELSE
               DY0(K)=SMER(3,K)
               DZ0(K)=-SMER(2,K)
            ENDIF
         ENDIF
         DY0(K)=DY0(K)*QBAR_WIN(MDN)
         DZ0(K)=DZ0(K)*QBAR_WIN(MDN)
      ENDDO
      Z_MAX=EXT_DZ(ID)
      KMIN=KMI
      KMAX=KMA

      RETURN

      ENTRY GET_UNFOLD_EXT(KK,SMER_Y,SMER_Z, ORIG_Y, ORIG_Z)

      SMER_Y=SMER(2,KK)
      SMER_Z=SMER(3,KK)
      ORIG_Y=ORIG(2,KK)
      ORIG_Z=ORIG(3,KK)
      RETURN

      ENTRY TO_BAR_EXIT(YCH,DYCH,BH,KK,DYDZ,Y,DY) ! note: valid only for LR=0

      Y=0
      DY=0
      
      Z1=(YCH+DYCH/2)*SMER(3,KK)+ORIG(3,KK)+DZ0(KK)
      IF(Z1.GT.Z_MAX) RETURN
      Y1=(YCH+DYCH/2)*SMER(2,KK)+ORIG(2,KK)+DY0(KK)+DYDZ*(Z_MAX-Z1)
      
      Z2=(YCH-DYCH/2)*SMER(3,KK)+ORIG(3,KK)+DZ0(KK)
      IF(Z2.GT.Z_MAX) RETURN
      Y2=(YCH-DYCH/2)*SMER(2,KK)+ORIG(2,KK)+DY0(KK)+DYDZ*(Z_MAX-Z2)

      IF(Y1.GT.Y2) THEN
         Y=Y1
         Y1=Y2
         Y2=Y
      ENDIF
      Y1=MAX(Y1,-BH)
      Y2=MIN(Y2,BH)
      Y=(Y1+Y2)/2
      DY=Y2-Y1
      RETURN

      ENTRY DUMP_UNFOLD(LUN)

      DO K=KMI,KMA+1
         WRITE(LUN,*) K,R0(3,K),R0(2,K),ORIG(3,K),ORIG(2,K),
     &        SMER(3,K),SMER(2,K)
      ENDDO
      RETURN

      END


      SUBROUTINE REFLECT_PLANE(R_PL,NORM,K)

      IMPLICIT NONE
      REAL R_PL(3,0:1)          ! planes (point)
      REAL NORM(3,0:1)          ! planes (normal)
      INTEGER K                 ! mirror plane [in], reflected plane [out]

      INTEGER K0, K1, I
      REAL R(3), A, S_PROD

      K0=MOD(K,2)               ! mirror plane
      K1=MOD(K+1,2)             ! plane to be refected

      DO I=1,3
         R(I)=R_PL(I,K1)-R_PL(I,K0)
      ENDDO
      A=2.*S_PROD(R,NORM(1,K0))
      DO I=1,3
         R_PL(I,K1)=R_PL(I,K1)-A*NORM(I,K0)
      ENDDO
      A=2.*S_PROD(NORM(1,K1),NORM(1,K0))
      DO I=1,3
         NORM(I,K1)=NORM(I,K1)-A*NORM(I,K0)
      ENDDO
      K=K1

      END


