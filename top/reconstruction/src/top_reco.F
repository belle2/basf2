!-----------------------------------------------------------------------
! TOP counter for Belle: Reconstruction
! M. Staric, oct-2008, may-2009, feb-2010, oct-2011
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
!-----------------------------------------------------------------------


      SUBROUTINE TOP_RECO

      IMPLICIT NONE
#include "TOP_RTRA.fi"     
#include "TOP_TRACK.fi"     
#include "TOP_GEO.fi"
#include "TOP_PMT.fi"
#include "TOP_PIK.fi"

      INTEGER ITRA, K, I
      REAL HLX(10)
      LOGICAL OK

      CALL CLEAR_PULLS
      DO ITRA=1,N_RTRA
         DO K=1,NHYP
            RTRA_PLKH(K,ITRA)=0.
            RTRA_SFOT(K,ITRA)=0.
         ENDDO
         RTRA_BFOT(ITRA)=0.
         RTRA_NFOT(ITRA)=0
         RTRA_FLAG(ITRA)=0
      ENDDO

      IF(.NOT.TOP_INI) THEN
         PRINT*,'ERROR *** TOP not initialized ***'
         RETURN
      ENDIF

      DO ITRA=1,N_RTRA
         I_RTRA=ITRA
         CALL TRESHOLD_TEST(RTRA_P(ITRA),NHYP,RTRA_MASS,OK)
         IF(.NOT.OK) CYCLE
         PMOM=RTRA_P(ITRA)
         CALL SET_HELIX(RTRA_R0(1,ITRA),RTRA_DIR(1,ITRA),RTRA_P(ITRA),
     &        RTRA_CHARGE(ITRA),HLX)
         CALL RECO_TRACK_TO_TOP(HLX,RTRA_R0(1,ITRA),RTRA_FLAG(ITRA),OK)
         IF(.NOT.OK) CYCLE
         RTRA_MOD(ITRA)=MODUL
         DO I=1,3
            RTRA_TR(I,ITRA)=T_R0(I)
            RTRA_TDIR(I,ITRA)=T_DIR(I)
         ENDDO
         RTRA_TLEN(ITRA)=T_LEN
         RTRA_TOFC0(ITRA)=TOF_C0
         CALL LIKELIHOOD(NHYP,RTRA_MASS,RTRA_HYPID,
     &        RTRA_T0(ITRA),RTRA_HYP(ITRA),
     &        RTRA_PLKH(1,ITRA),RTRA_NFOT(ITRA),RTRA_SFOT(1,ITRA),
     &        RTRA_BFOT(ITRA))
      ENDDO

      END


      SUBROUTINE RECO_TRACK_TO_TOP(HLX0,R0,FLAG,OK)

      IMPLICIT NONE
      REAL HLX0(10)
      REAL R0(3)
      INTEGER FLAG
      LOGICAL OK
#include "TOP_GEO.fi"     
#include "TOP_TRACK.fi"     
#include "TOP_CONS.fi"     
      INTEGER I
      REAL RR, HLX(10)
      REAL HLX_LOC(10), Y, T0, T1, T2, T3, T4, TMI, FI, R(3), DIR(3)
      LOGICAL OK1
      LOGICAL INSIDE_QBAR
      
      FLAG=0
      DO I=1,10
         HLX(I)=HLX0(I)
      ENDDO
!     move starting point to TOP_R1
      RR=R0(1)**2+R0(2)**2
      IF(RR.NE.TOP_R1**2) THEN
         CALL XSEC_CYL(HLX,TOP_R1,T1,T2,OK)
         IF(.NOT.OK) RETURN
         IF(RR.LT.TOP_R1**2) THEN
            T0=T1
         ELSE
            T0=T2-HLX(10)
         ENDIF
         CALL MOVE_PIVOT(HLX,T0)
      ENDIF

!     find, which TOP module is intersected 
      TMI=1.E10
      DO I=0,NUM_MOD-1
         CALL HLX_TO_LOCAL(HLX,I,HLX_LOC)
         Y=0.
         CALL XSEC_PLANE_Y(HLX_LOC,Y,T1,T2,OK)
         IF(.NOT.OK) CYCLE
         TMI=MIN(TMI,T1)
         CALL GET_HLX_RDIR(HLX_LOC,T1,T_R0,T_DIR)
         OK=ABS(T_R0(1)).LT.QBAR_A(I)/2.
         IF(.NOT.OK) CYCLE
         OK=ABS(T_R0(3)).LT.QBAR_C(I)/2.
         IF(.NOT.OK) CYCLE
         OK=INSIDE_QBAR(T_R0(1),T_R0(2),T_R0(3),I)
         IF(.NOT.OK) CYCLE
         MODUL=I
         EXIT
      ENDDO

      IF(.NOT.OK) THEN
         IF(TMI.LT.(1.E10)) THEN
            CALL GET_HLX_RDIR(HLX,TMI,R,DIR)
            IF(R(3).GT.TOP_Z1.AND.R(3).LT.TOP_Z2) FLAG=-1
         ENDIF
         RETURN
      ENDIF

!     module is found
      TOF_C0=T1
      Y=-QBAR_B(MODUL)/2.
      CALL XSEC_PLANE_Y(HLX_LOC,Y,T1,T2,OK)
      IF(.NOT.OK) RETURN
      Y=-Y
      CALL XSEC_PLANE_Y(HLX_LOC,Y,T3,T4,OK1)
      IF(OK1) T2=T3
      T_LEN=ABS(T2-T1)*C0
      SIG_TLEN=T_LEN/SQRT(12.)

!     additional info
      T_CTH(0)=T_DIR(3)
      T_STH(0)=SQRT(1-T_CTH(0)**2)
      FI=ATAN2(T_DIR(2),T_DIR(1))
      T_CFI(0)=COS(FI)
      T_SFI(0)=SIN(FI)

      T_H=0.1                   ! [cm]
      T1=TOF_C0+T_H/C0
      CALL GET_HLX_RDIR(HLX_LOC,T1,T_R1,DIR)
      OK=INSIDE_QBAR(T_R1(1),T_R1(2),T_R1(3),MODUL)
      IF(.NOT.OK) RETURN
      T_CTH(1)=DIR(3)
      T_STH(1)=SQRT(1-T_CTH(1)**2)
      FI=ATAN2(DIR(2),DIR(1))
      T_CFI(1)=COS(FI)
      T_SFI(1)=SIN(FI)

      T1=TOF_C0-T_H/C0
      CALL GET_HLX_RDIR(HLX_LOC,T1,T_R2,DIR)
      OK=INSIDE_QBAR(T_R2(1),T_R2(2),T_R2(3),MODUL)
      IF(.NOT.OK) RETURN
      T_CTH(-1)=DIR(3)
      T_STH(-1)=SQRT(1-T_CTH(1)**2)
      FI=ATAN2(DIR(2),DIR(1))
      T_CFI(-1)=COS(FI)
      T_SFI(-1)=SIN(FI)

      TOF_C0=TOF_C0+T0
      FLAG=1

      END


      SUBROUTINE TRESHOLD_TEST(P,NHYP,MASS,OK)

      IMPLICIT NONE
      REAL P
      INTEGER NHYP
      REAL MASS(*)
      LOGICAL OK

#include "TOP_PAR.fi"
      REAL MIN_MASS, CHER_ANGLE
      INTEGER I

      MIN_MASS=1.e10
      DO I=1,NHYP
         IF(MASS(I).GT.0) MIN_MASS=MIN(MIN_MASS,MASS(I))
      ENDDO
      OK=CHER_ANGLE(P,MIN_MASS,REF_IND).GT.0

      END


      REAL FUNCTION CHER_ANGLE(P,MASS,REF_IND)

      IMPLICIT NONE
      REAL P, MASS, REF_IND
      REAL A, B
      
      IF(MASS.LE.0.OR.P.LT.0) THEN
         CHER_ANGLE=0.
         RETURN
      ENDIF
      A=P/MASS
      B=A/SQRT(1.+A**2)*REF_IND
      IF(B.GT.1) THEN
         CHER_ANGLE=ACOS(1./B)
      ELSE
         CHER_ANGLE=0.
      ENDIF

      END
     
      
      SUBROUTINE LIKELIHOOD(NHYP,MASS,HYPID,T0,HYP,PLKH,NFOT,SFOT,BFOT)

      IMPLICIT NONE
      INTEGER NHYP              ! number of hyphotheses [in]
      REAL MASS(NHYP)           ! mass [in]
      INTEGER HYPID(NHYP)       ! hypotheses ID [in]
      REAL T0                   ! start time for beta=1 [in]
      INTEGER HYP               ! hypothesis to histogram pulls
      REAL PLKH(NHYP)           ! log likelihoods [out]
      INTEGER NFOT              ! number of photons [out]
      REAL SFOT(NHYP)           ! expected num of all photons [out]
      REAL BFOT                 ! expected num of bg. photons [out]

      INTEGER I
      
      CALL SET_PHTIMES(.FALSE.)

      DO I=1,NHYP
         CALL TOP_SET_PDF(MASS(I),T0)
         CALL FILL_LKH(PLKH(I),NFOT,SFOT(I),BFOT,HYPID(I).EQ.HYP)
      ENDDO

      END


      SUBROUTINE FILL_LKH(PLKH,NFOT,SFOT,BFOT,FLAG)

      IMPLICIT NONE
      REAL PLKH
      INTEGER NFOT
      REAL SFOT
      REAL BFOT
      LOGICAL FLAG
#include "TOP_DATA.fi"
#include "TOP_TRACK.fi"
#include "TOP_TDC.fi"
#include "TOP_PIK.fi"
      INTEGER I, ICH, M, NUMCH
      REAL P, T, F
      REAL TOP_SIGNAL, TOP_BGR, POIS_LOG

      PLKH=0.
      NFOT=0
      SFOT=STFS+STFB
      BFOT=STFB
      IF(SFOT.EQ.0) RETURN
      NUMCH=NCH(0)+NCH(1)
      P=STFS/SFOT
      DO I=1,N_DATA
         M=DATA_MOD(I)
         IF(M.EQ.MODUL) THEN
            ICH=DATA_ICH(I)
            IF(ICH.LT.0.OR.ICH.GE.NUMCH) CYCLE
            T=DATA_TIME(I)
            IF(T.LT.0.OR.T.GT.T_MAX) CYCLE
            NFOT=NFOT+1
            F=P*TOP_SIGNAL(T,ICH)+(1-P)*TOP_BGR(T,ICH)
            PLKH=PLKH+LOG(F)
            IF(FLAG) CALL STORE_PULLS(T,ICH)
         ENDIF
      ENDDO
      PLKH=PLKH+POIS_LOG(SFOT,FLOAT(NFOT))

      END


      REAL FUNCTION GET_LOGL(T0,TWIN)

      IMPLICIT NONE
      REAL T0                   ! time shift
      REAL TWIN                 ! time window
#include "TOP_DATA.fi"
#include "TOP_RTRA.fi"
#include "TOP_TRACK.fi"
#include "TOP_TDC.fi"
#include "TOP_PIK.fi"
      INTEGER I, ICH, M, NUMCH, NFOT
      REAL P, T, F, SFOT, PLKH, PS, PB
      REAL TOP_SIGNAL, TOP_BGR, POIS_LOG

      GET_LOGL=0.
      IF(N_RTRA.NE.1) RETURN
      IF(RTRA_FLAG(1).NE.1) RETURN

      CALL  GET_NEW_NORM(T0,TWIN,PS,PB)
      SFOT=STFS*PS+STFB*PB
      IF(SFOT.EQ.0) RETURN

      PLKH=0.
      NFOT=0
      NUMCH=NCH(0)+NCH(1)
      P=STFS*PS/SFOT
      IF(PS.EQ.0) PS=1.         ! to prevent NaN's 
      DO I=1,N_DATA
         M=DATA_MOD(I)
         IF(M.EQ.MODUL) THEN
            ICH=DATA_ICH(I)
            IF(ICH.LT.0.OR.ICH.GE.NUMCH) CYCLE
            T=DATA_TIME(I)
            IF(T.LT.0.OR.T.GT.TWIN) CYCLE
            NFOT=NFOT+1
            F=P*TOP_SIGNAL(T+T0,ICH)/PS+(1-P)*TOP_BGR(T+T0,ICH)/PB
            PLKH=PLKH+LOG(F)
         ENDIF
      ENDDO
      GET_LOGL=PLKH+POIS_LOG(SFOT,FLOAT(NFOT))

      END


      SUBROUTINE GET_NEW_NORM(T0,TWIN,PS,PB)
      
      IMPLICIT NONE
      REAL T0                   ! time shift
      REAL TWIN                 ! time window
      REAL PS, PB               ! new normalizations for signal and background
#include "TOP_PIK.fi"
      INTEGER ICH, NUMCH, K

      PS=0.
      NUMCH=NCH(0)+NCH(1)
      DO ICH=0,NUMCH-1
         DO K=1,NUM_PIK(ICH)
            IF(PIK_POZ(K,ICH).LT.T0) CYCLE
            IF(PIK_POZ(K,ICH).GT.T0+TWIN) CYCLE
            PS=PS+PIK_NPH(K,ICH)
         ENDDO
      ENDDO

      PB=TWIN/T_MAX

      END


      REAL FUNCTION TOP_SIGNAL(T,ICH)

      IMPLICIT NONE
      REAL T
      INTEGER ICH
#include "TOP_PIK.fi"
#include "TOP_PMT.fi"
      INTEGER K, I
      REAL X, F, SIG
      REAL SQPI
      PARAMETER (SQPI=2.50662827463)

      TOP_SIGNAL=0.
      DO K=1,NUM_PIK(ICH)
         DO I=1,TTS_NG
            SIG=PIK_WID(K,ICH)+TTS_SIG(I)**2
            X=(T-PIK_POZ(K,ICH)-TTS_T0(I))**2/SIG
            IF(X.GT.10) CYCLE
            SIG=SQRT(SIG)
            F=PIK_NPH(K,ICH)*TTS_FRAC(I)/SQPI/SIG*EXP(-X/2)
            TOP_SIGNAL=TOP_SIGNAL+F
         ENDDO
      ENDDO

      END
     

      REAL FUNCTION TOP_BGR(T,ICH)

      IMPLICIT NONE
      REAL T
      INTEGER ICH
#include "TOP_PIK.fi"

      TOP_BGR=PIK_BGR(ICH)

      END


      REAL FUNCTION POIS_LOG(MEAN,N) ! log(Poisson(mean,n)) w/o log(gamma(n+1))

      IMPLICIT NONE
      REAL MEAN
      REAL N

      IF(MEAN.EQ.0) THEN
         IF(N.EQ.0) THEN
            POIS_LOG=0.
         ELSE
            POIS_LOG=-1.E10
         ENDIF
      ELSE
         POIS_LOG=N*LOG(MEAN)-MEAN
      ENDIF

      END


      SUBROUTINE SET_PHTIMES(FORCE)

      IMPLICIT NONE
      LOGICAL FORCE             ! true to force y-scan
#include "TOP_DATA.fi"
#include "TOP_TRACK.fi"
#include "TOP_PIK.fi"
#include "TOP_TDC.fi"
#include "TOP_PMT.fi"
#include "TOP_PHTIM.fi"
      DATA FORCE_SCAN /.TRUE./

      INTEGER ICHX, NUMCH, I, M, ICH
      INTEGER IX, IY, IPMTX, IPMTY, LR
      REAL T

      FORCE_SCAN=FORCE
      IF(FORCE) RETURN

      DO ICHX=0,NUM_CHX
         NUM_PHOT(ICHX)=0
      ENDDO

      NUMCH=NCH(0)+NCH(1)
      DO I=1,N_DATA
         M=DATA_MOD(I)
         IF(M.EQ.MODUL) THEN
            ICH=DATA_ICH(I)
            IF(ICH.LT.0.OR.ICH.GE.NUMCH) CYCLE
            T=DATA_TIME(I)
            IF(T.LT.0.OR.T.GT.T_MAX) CYCLE
            CALL ICHIII(ICH,M,IX,IY,IPMTX,IPMTY,LR)
            IF(LR.LT.0) CYCLE
            ICHX=IPMTX*PMT_NX+IX
            IF(ICHX.GE.NUM_CHX) CYCLE
            NUM_PHOT(ICHX)=NUM_PHOT(ICHX)+1
            IF(NUM_PHOT(ICHX).GT.MAX_PHOT) CYCLE
            PH_TIMES(NUM_PHOT(ICHX),ICHX)=T
         ENDIF
      ENDDO

      END


      SUBROUTINE STORE_PULLS(T,ICH)

      IMPLICIT NONE
      REAL T
      INTEGER ICH
      INTEGER N, KK
      REAL TOP, T_0, WIDTH, PHIC, UT
      INTEGER IRTRA
#include "TOP_CONS.fi"
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
#include "TOP_PMT.fi"
      INTEGER K, I
      REAL W(MAX_G,MAX_PIK), SIGMA(MAX_G,MAX_PIK)
      REAL SFOT, P
      REAL S, BGR, XX, SIG, T0_MIN
      REAL TOP_BGR
      real SQPI
      PARAMETER (SQPI=2.50662827463)
      
      integer NB
      PARAMETER (NB=10000)
      INTEGER NT
      REAL TIME(NB), T0(NB), WID(NB), FIC(NB), WT(NB)
      INTEGER RTRA(NB)
      SAVE NT, TIME, T0, WID, FIC, WT, RTRA

      SFOT=STFS+STFB
      IF(SFOT.EQ.0) RETURN
      P=STFS/SFOT

      S=0.
      DO K=1,NUM_PIK(ICH)
         DO I=1,TTS_NG
            SIG=PIK_WID(K,ICH)+TTS_SIG(I)**2
            XX=(T-PIK_POZ(K,ICH)-TTS_T0(I))**2/SIG
            W(I,K)=0
            IF(XX.GT.100) CYCLE
            SIG=SQRT(SIG)
            SIGMA(I,K)=SIG
            W(I,K)=P*PIK_NPH(K,ICH)*TTS_FRAC(I)/SQPI/SIG*EXP(-XX/2)
            S=S+W(I,K)
         ENDDO
      ENDDO
      BGR=(1-P)*TOP_BGR(T,ICH)
      S=S+BGR
      IF(S.EQ.0) RETURN

      T0_MIN=T_MAX
      DO K=1,NUM_PIK(ICH)
         T0_MIN=MIN(T0_MIN,PIK_POZ(K,ICH))
         DO I=1,TTS_NG
            IF(W(I,K).EQ.0) CYCLE
            IF(NT.GE.NB) RETURN
            NT=NT+1
            TIME(NT)=T
            T0(NT)=PIK_POZ(K,ICH)+TTS_T0(I)
            WID(NT)=SIGMA(I,K)
            FIC(NT)=PIK_FIC(K,ICH)-PI
            WT(NT)=W(I,K)/S
            RTRA(NT)=I_RTRA
         ENDDO
      ENDDO

      IF(NT.GE.NB) RETURN
      NT=NT+1
      TIME(NT)=T
      T0(NT)=T0_MIN
      WID(NT)=0.
      FIC(NT)=0.
      WT(NT)=BGR/S
      RTRA(NT)=I_RTRA
      RETURN

      ENTRY CLEAR_PULLS
      NT=0
      RETURN

      ENTRY GETNUM_PULLS(N)
      N=NT
      RETURN

      ENTRY GET_PULLS(KK,TOP,T_0,WIDTH,PHIC,UT,IRTRA)
      UT=0.
      IF(KK.LT.1.OR.KK.GT.NT) RETURN
      TOP=TIME(KK)
      T_0=T0(KK)
      WIDTH=WID(KK)
      PHIC=FIC(KK)
      UT=WT(KK)
      IRTRA=RTRA(KK)
      RETURN

      END



      REAL FUNCTION GET_PDF(ICH,T,MASS)

      IMPLICIT NONE
      INTEGER ICH
      REAL T, MASS

#include "TOP_RTRA.fi"     
#include "TOP_PIK.fi"
#include "TOP_PHTIM.fi"
      INTEGER NUMCH
      REAL SFOT, P
      REAL TOP_SIGNAL, TOP_BGR

      GET_PDF=0.
      IF(N_RTRA.NE.1) RETURN
      IF(RTRA_FLAG(1).NE.1) RETURN
      NUMCH=NCH(0)+NCH(1)
      IF(ICH.LT.0.OR.ICH.GE.NUMCH) RETURN

      IF(BETA_RQ.GT.0) THEN
         IF(BETA_RQ.NE.BETA.OR..NOT.FORCE_SCAN) THEN
            PRINT*,'construct PDF for Beta=',BETA_RQ
            CALL SET_PHTIMES(.TRUE.)
            CALL TOP_SET_PDF(MASS,RTRA_T0(1))
         ENDIF
      ELSE
         IF(MASS.NE.MASS_HYP.OR..NOT.FORCE_SCAN) THEN
            PRINT*,'construct PDF for Mass=',MASS
            CALL SET_PHTIMES(.TRUE.)
            CALL TOP_SET_PDF(MASS,RTRA_T0(1))
         ENDIF
      ENDIF
      SFOT=STFS+STFB
      P=STFS/SFOT
      GET_PDF=P*TOP_SIGNAL(T,ICH)+(1-P)*TOP_BGR(T,ICH)

      END


      SUBROUTINE SET_BETA_RQ(BT)

      IMPLICIT NONE
      REAL BT
#include "TOP_PIK.fi"

      BETA_RQ=BT

      END


      SUBROUTINE SET_PDF_OPT(OPT)

      IMPLICIT NONE
      INTEGER OPT
#include "TOP_PIK.fi"

      PDF_OPT=OPT

      END


