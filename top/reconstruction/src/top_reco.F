!-----------------------------------------------------------------------
! TOP counter for Belle: Reconstruction
! M. Staric, oct-2008, may-2009, feb-2010, oct-2011
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
! dec-2017: change in some commons
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
         CALL TRESHOLD_TEST(RTRA_P(ITRA),NHYP,RTRA_MASS,OK)
         IF(.NOT.OK) CYCLE
         I_RTRA=ITRA
         MODUL=RTRA_MOD(ITRA)
         PMOM=RTRA_P(ITRA)
         CALL SET_HELIX(RTRA_R0(1,ITRA),RTRA_DIR(1,ITRA),RTRA_P(ITRA),
     &        RTRA_CHARGE(ITRA),HLX)
         CALL SET_TRACK_PAR(HLX,OK)
         IF(.NOT.OK) CYCLE
         RTRA_FLAG(ITRA)=1
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


      SUBROUTINE SET_TRACK_PAR(HLX,OK)

      IMPLICIT NONE
      REAL HLX(10)
      LOGICAL OK
#include "TOP_GEO.fi"     
#include "TOP_TRACK.fi"     
#include "TOP_CONS.fi"     
      REAL T1, T2, T, FI
      REAL HLX_LOC(10), R(3), DIR(3)
      LOGICAL INSIDE_QBAR
      INTEGER I, K
      
      OK=MODUL.GE.0.AND.MODUL.LT.NUM_MOD
      IF(.NOT.OK) RETURN
      
      CALL XSEC_QBAR(HLX,MODUL,T1,T2,OK)
      IF(.NOT.OK) RETURN
      
      TOF_C0=(T1+T2)/2
      T_LEN=(T2-T1)*C0
      OK=T_LEN.GT.QBAR_B(MODUL)/2
      IF(.NOT.OK) RETURN
      SIG_TLEN=T_LEN/SQRT(12.)
      
      CALL HLX_TO_LOCAL(HLX,MODUL,HLX_LOC) ! helix is in nominal frame

      T_H=0.1                   ! [cm]
      DO I=-1,1
         T=TOF_C0+T_H/C0*I
         CALL GET_HLX_RDIR(HLX_LOC,T,R,DIR) ! get in nominal frame
         CALL NOMINAL_TO_LOCAL(R,DIR,MODUL) ! transform to local (bar) frame
         OK=INSIDE_QBAR(R(1),R(2),R(3),MODUL)
         IF(.NOT.OK) RETURN

         T_CTH(I)=DIR(3)
         T_STH(I)=SQRT(1-DIR(3)**2)
         FI=ATAN2(DIR(2),DIR(1))
         T_CFI(I)=COS(FI)
         T_SFI(I)=SIN(FI)
         DO K=1,3
            T_R(K,I)=R(K)
         ENDDO
         IF(I.EQ.0) T_DIR=DIR
      ENDDO
      
      END
      

      SUBROUTINE XSEC_QBAR(HLX,MDN,T1,T2,OK)

      REAL HLX(10)              ! helix in global frame [in]
      INTEGER MDN               ! valid bar ID [in]
      REAL T1, T2               ! entrance and exit times (for beta=1) [out]
      LOGICAL OK                ! true if x-section exists
#include "TOP_GEO.fi"     
#include "TOP_CONS.fi"     
      REAL HLX_LOC(10), T0
      integer NPL
      PARAMETER (NPL=4)         ! number of bar surfaces (planes)
      REAL RP(3,NPL)            ! point defining plane
      REAL NORM(3,NPL)          ! normal defining plane
      INTEGER I, K
      REAL AH, BH, R(3), DIR(3), H, T
      REAL S_PROD
      LOGICAL FL1, FL2
      
      T1=0
      T2=0
      AH=QBAR_A(MDN)/2
      BH=QBAR_B(MDN)/2
      
      RP=0
      RP(2,1)=-BH
      RP(2,2)=BH
      RP(1,3)=-AH
      RP(1,4)=AH
      NORM=0
      NORM(2,1)=-1.
      NORM(2,2)=1.
      NORM(1,3)=-1.
      NORM(1,4)=1.

      CALL HLX_TO_LOCAL(HLX,MDN,HLX_LOC) ! helix is in nominal frame
      T0=-10.0/C0
      CALL MOVE_PIVOT(HLX_LOC,T0)   ! ensure that bar surfaces are infront of track

      FL1=.FALSE.
      FL2=.FALSE.
      DO I=1,NPL
         CALL LOCAL_TO_NOMINAL(RP(1,I),NORM(1,I),MDN) ! planes in nominal frame
         CALL XSEC_PLANE(HLX_LOC,RP(1,I),NORM(1,I),T,OK)
         IF(.NOT.OK) CYCLE
         CALL GET_HLX_RDIR(HLX_LOC,T,R,DIR)
         T=T+T0
         IF(S_PROD(DIR,NORM(1,I)).LT.0) THEN
            IF(.NOT.FL1) THEN
               T1=T
               FL1=.TRUE.
            ELSE
               T1=MAX(T1,T)
            ENDIF
         ELSE
            IF(.NOT.FL2) THEN
               T2=T
               FL2=.TRUE.
            ELSE
               T2=MIN(T2,T)
            ENDIF
         ENDIF
      ENDDO
      OK=FL1.AND.FL2.AND.T2.GT.T1
      
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
      REAL P, T, TERR, F
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
            TERR=DATA_TERR(I)
            IF(T.LT.T_MIN.OR.T.GT.T_MAX) CYCLE
            NFOT=NFOT+1
            F=P*TOP_SIGNAL(T,TERR,ICH)+(1-P)*TOP_BGR(T,ICH)
            PLKH=PLKH+LOG(F)
            IF(FLAG) CALL STORE_PULLS(T,TERR,ICH)
         ENDIF
      ENDDO
      PLKH=PLKH+POIS_LOG(SFOT,FLOAT(NFOT))
      
      END


      REAL FUNCTION GET_LOGL(T0,TMIN,TMAX,SIGT)

      IMPLICIT NONE
      REAL T0                   ! time shift
      REAL TMIN                 ! time window lower edge
      REAL TMAX                 ! time window upper edge
      REAL SIGT                 ! additional time smearing
#include "TOP_DATA.fi"
#include "TOP_RTRA.fi"
#include "TOP_TRACK.fi"
#include "TOP_TDC.fi"
#include "TOP_PIK.fi"
      INTEGER I, ICH, M, NUMCH, NFOT
      REAL P, T, TERR, F, SFOT, PLKH, PS, PB
      REAL TOP_SIGNAL1, TOP_BGR, POIS_LOG

      GET_LOGL=0.
      IF(N_RTRA.NE.1) RETURN
      IF(RTRA_FLAG(1).NE.1) RETURN

      IF(TMAX.LE.TMIN) THEN
         TMAX=T_MAX
         TMIN=T_MIN
      ENDIF
      
      CALL  GET_NEW_NORM(T0,TMIN,TMAX,PS,PB)
      SFOT=STFS*PS+STFB*PB
      IF(SFOT.EQ.0) RETURN

      PLKH=0.
      NFOT=0
      NUMCH=NCH(0)+NCH(1)
      P=STFS*PS/SFOT
      DO I=1,N_DATA
         M=DATA_MOD(I)
         IF(M.EQ.MODUL) THEN
            ICH=DATA_ICH(I)
            IF(ICH.LT.0.OR.ICH.GE.NUMCH) CYCLE
            T=DATA_TIME(I)
            TERR=DATA_TERR(I)
            IF(T.LT.TMIN.OR.T.GT.TMAX) CYCLE
            NFOT=NFOT+1
            F=P*TOP_SIGNAL1(T-T0,TERR,ICH,SIGT) +
     &           (1-P)*TOP_BGR(T-T0,ICH)
            PLKH=PLKH+LOG(F)
         ENDIF
      ENDDO
      GET_LOGL=PLKH+POIS_LOG(SFOT,FLOAT(NFOT))
      RTRA_NFOT(1)=NFOT
      
      END


      SUBROUTINE GET_NEW_NORM(T0,TMIN,TMAX,PS,PB)
      
      IMPLICIT NONE
      REAL T0                   ! time shift
      REAL TMIN                 ! time window lower edge
      REAL TMAX                 ! time window upper edge
      REAL PS, PB               ! new normalizations for signal and background
#include "TOP_PIK.fi"
      INTEGER ICH, NUMCH, K

      PS=0.
      NUMCH=NCH(0)+NCH(1)
      DO ICH=0,NUMCH-1
         DO K=1,NUM_PIK(ICH)
            IF(PIK_POZ(K,ICH).LT.TMIN-T0) CYCLE
            IF(PIK_POZ(K,ICH).GT.TMAX-T0) CYCLE
            PS=PS+PIK_NPH(K,ICH)
         ENDDO
      ENDDO

      PB=(TMAX-TMIN)/(T_MAX-T_MIN)
      IF(PB.LT.0) PB=0.
      
      END


      REAL FUNCTION TOP_SIGNAL(T,TERR,ICH)

      IMPLICIT NONE
      REAL T, TERR
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
            SIG=PIK_WID(K,ICH)+TTS_SIG(I)**2+TERR**2
            X=(T-PIK_POZ(K,ICH)-TTS_T0(I))**2/SIG
            IF(X.GT.10) CYCLE
            SIG=SQRT(SIG)
            F=PIK_NPH(K,ICH)*TTS_FRAC(I)/SQPI/SIG*EXP(-X/2)
            TOP_SIGNAL=TOP_SIGNAL+F
         ENDDO
      ENDDO

      END
     

      REAL FUNCTION TOP_SIGNAL1(T,TERR,ICH,SIGT)

      IMPLICIT NONE
      REAL T, TERR
      INTEGER ICH
      REAL SIGT
#include "TOP_PIK.fi"
#include "TOP_PMT.fi"
      INTEGER K, I
      REAL X, F, SIG
      REAL SQPI
      PARAMETER (SQPI=2.50662827463)

      TOP_SIGNAL1=0.
      DO K=1,NUM_PIK(ICH)
         DO I=1,TTS_NG
            SIG=PIK_WID(K,ICH)+TTS_SIG(I)**2+TERR**2+SIGT**2
            X=(T-PIK_POZ(K,ICH)-TTS_T0(I))**2/SIG
            IF(X.GT.10) CYCLE
            SIG=SQRT(SIG)
            F=PIK_NPH(K,ICH)*TTS_FRAC(I)/SQPI/SIG*EXP(-X/2)
            TOP_SIGNAL1=TOP_SIGNAL1+F
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
            IF(T.LT.T_MIN.OR.T.GT.T_MAX) CYCLE
            CALL ICHIII(ICH,M,IX,IY,IPMTX,IPMTY,LR)
            IF(LR.LT.0) CYCLE
            ICHX=IPMTX*PMT_NX+IX
            IF(ICHX.GE.NUM_CHX) CYCLE
            NUM_PHOT(ICHX)=NUM_PHOT(ICHX)+1
            IF(NUM_PHOT(ICHX).GT.MAX_PHOT) CYCLE
            PH_TIMES(NUM_PHOT(ICHX),ICHX)=T
            PH_TERRS(NUM_PHOT(ICHX),ICHX)=DATA_TERR(I)
         ENDIF
      ENDDO

      END


      SUBROUTINE STORE_PULLS(T,TERR,ICH)

      IMPLICIT NONE
      REAL T, TERR
      INTEGER ICH
      INTEGER N, KK
      REAL TOP, T_0, WIDTH, PHIC, UT
      INTEGER ICHID
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
      INTEGER CHID(NB)
      SAVE NT, TIME, T0, WID, FIC, WT, CHID

      SFOT=STFS+STFB
      IF(SFOT.EQ.0) RETURN
      P=STFS/SFOT

      S=0.
      DO K=1,NUM_PIK(ICH)
         DO I=1,TTS_NG
            SIG=PIK_WID(K,ICH)+TTS_SIG(I)**2+TERR**2
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
            CHID(NT)=ICH
         ENDDO
      ENDDO

      IF(NT.GE.NB) RETURN
      NT=NT+1
      TIME(NT)=T
      T0(NT)=T0_MIN
      WID(NT)=0.
      FIC(NT)=0.
      WT(NT)=BGR/S
      CHID(NT)=ICH
      RETURN

      ENTRY CLEAR_PULLS
      NT=0
      RETURN

      ENTRY GETNUM_PULLS(N)
      N=NT
      RETURN

      ENTRY GET_PULLS(KK,TOP,T_0,WIDTH,PHIC,UT,ICHID)
      UT=0.
      IF(KK.LT.1.OR.KK.GT.NT) RETURN
      TOP=TIME(KK)
      T_0=T0(KK)
      WIDTH=WID(KK)
      PHIC=FIC(KK)
      UT=WT(KK)
      ICHID=CHID(KK)
      RETURN

      END



      REAL FUNCTION GET_PDF(ICH,T,TERR,MASS)

      IMPLICIT NONE
      INTEGER ICH
      REAL T, TERR, MASS

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
cc            PRINT*,'construct PDF for Beta=',BETA_RQ
            CALL SET_PHTIMES(.TRUE.)
            CALL TOP_SET_PDF(MASS,RTRA_T0(1))
         ENDIF
      ELSE
         IF(MASS.NE.MASS_HYP.OR..NOT.FORCE_SCAN) THEN
cc            PRINT*,'construct PDF for Mass=',MASS
            CALL SET_PHTIMES(.TRUE.)
            CALL TOP_SET_PDF(MASS,RTRA_T0(1))
         ENDIF
      ENDIF
      SFOT=STFS+STFB
      P=STFS/SFOT
      GET_PDF=P*TOP_SIGNAL(T,TERR,ICH)+(1-P)*TOP_BGR(T,ICH)

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


      SUBROUTINE REDO_PDF(MASS)
      
      IMPLICIT NONE
      REAL MASS
#include "TOP_RTRA.fi"     

      CALL SET_PHTIMES(.TRUE.)
      CALL TOP_SET_PDF(MASS,RTRA_T0(1))

      END

      
      INTEGER FUNCTION GET_NUM_PEAKS(ICH)

      IMPLICIT NONE
      INTEGER ICH               ! channel (e.g. pixelID-1)
#include "TOP_PIK.fi"
      INTEGER NUMCH
      
      GET_NUM_PEAKS = 0

      NUMCH=NCH(0)+NCH(1)
      IF(ICH.LT.0.OR.ICH.GE.NUMCH) RETURN
      
      GET_NUM_PEAKS = NUM_PIK(ICH)

      END


      SUBROUTINE GET_PEAK(ICH, K, POZ, WID, NPH)

      IMPLICIT NONE
      INTEGER ICH               ! channel (e.g. pixelID-1)
      INTEGER K                 ! peak count
      REAL POZ                  ! position
      REAL WID                  ! width
      REAL NPH                  ! number of photons
#include "TOP_PIK.fi"
      INTEGER NUMCH

      POZ = 0
      WID = 0
      NPH = 0
      NUMCH=NCH(0)+NCH(1)
      IF(ICH.LT.0.OR.ICH.GE.NUMCH) RETURN
      IF(K.LT.1.OR.K.GT.NUM_PIK(ICH)) RETURN

      POZ = PIK_POZ(K,ICH)
      WID = SQRT(PIK_WID(K,ICH))
      NPH = STFS*PIK_NPH(K,ICH)

      END


