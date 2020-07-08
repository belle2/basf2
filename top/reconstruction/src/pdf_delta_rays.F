!-----------------------------------------------------------------------
! TOP counter for Belle: Reconstruction - PDF of photons from delta rays
! M. Staric, may-2020
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
!-----------------------------------------------------------------------

      SUBROUTINE SET_TOP_DELTA

c--   sets variables in /TOP_DELTA/ to be shared between functions
c--   to be called per each track
      
      IMPLICIT NONE
#include "TOP_PAR.fi"
#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
#include "TOP_TRACK.fi"
      REAL F_D                  ! fraction of direct photons
      REAL T0_D                 ! minimal propagation time of direct photons
      REAL T0_R                 ! minimal propagation time of reflected photons
      COMMON /TOP_DELTA/ F_D, T0_D, T0_R

      REAL FRACT_DELTA
      REAL Z, ZD(0:1), ZW(0:1), LR_FLAG(0:1), DZEXT
      INTEGER L, LR
      
      Z=T_R0(3)
      F_D=FRACT_DELTA(Z)

      LR_FLAG(0)=QBAR_LSIDE(MODUL)
      LR_FLAG(1)=QBAR_RSIDE(MODUL)
      ZW(0)=-QBAR_C(MODUL)/2.
      ZW(1)=QBAR_C(MODUL)/2.
      DO L=0,1
         DZEXT=0.
         IF(LR_FLAG(L).EQ.1) THEN
            LR=L
            IF(QBAR_EXT(LR,MODUL).GT.0) DZEXT=EXT_DZ(QBAR_EXT(LR,MODUL))
            ZD(L)=ZW(L)+(2*L-1)*(DZEXT+QBAR_WIN(MODUL))
         ELSE
            LR=1-L
            IF(QBAR_EXT(LR,MODUL).GT.0) DZEXT=EXT_DZ(QBAR_EXT(LR,MODUL))
            ZD(L)=ZW(L)+(2*L-1)*(QBAR_C(MODUL)+DZEXT+QBAR_WIN(MODUL))
         ENDIF
      ENDDO      

      ! note: times are correct only for prism on -z and mirror on +z
      T0_D=ABS(ZD(0)-Z)*GRU_IND/C0
      T0_R=ABS(ZD(1)-Z)*GRU_IND/C0
      
      END

      
      REAL FUNCTION NFOT_DELTA()

c--   number of photons from delta-electrons for nominal detection efficiency
      
      IMPLICIT NONE
#include "TOP_TRACK.fi"
#include "TOP_PIK.fi"
      REAL F, NORM_DELTA
      REAL AP(3) /13.5859, -28.0625, 17.2684/ ! pol2 parameters for anti-protons
      REAL P(5) /-8.15871, 10.0082, -1.25140, -120.225, 120.210/ ! parameters for others
      REAL AVRG_RQE /1.06404889/ ! average RQE (relative to nominal PDE)
      SAVE AP, P, AVRG_RQE

      IF(ABS(PDG_HYP).EQ.11) THEN   ! electrons and positrons
         F=5.30
      ELSE IF(PDG_HYP.EQ.-2212) THEN ! anti-protons
         F=AP(1)+AP(2)*BETA+AP(3)*BETA**2
      ELSE                      ! others
         F=EXP(P(1)+P(2)*BETA+P(3)*BETA**2) + EXP(P(4)+P(5)*BETA)
      ENDIF
      NFOT_DELTA=F*T_LEN/AVRG_RQE*NORM_DELTA(T_MIN,T_MAX)
      
      END

      
      REAL FUNCTION NORM_DELTA(TMIN,TMAX)

c--   normalization fraction within time interval

      IMPLICIT NONE
      REAL TMIN, TMAX           ! time interval in which to normalize
#include "TOP_PIK.fi"
      REAL F_D                  ! fraction of direct photons
      REAL T0_D                 ! minimal propagation time of direct photons
      REAL T0_R                 ! minimal propagation time of reflected photons
      COMMON /TOP_DELTA/ F_D, T0_D, T0_R

      REAL N_D, N_R
      REAL NORM_DELTA_PIK

      N_R=NORM_DELTA_PIK(TMIN-TOF,TMAX-TOF,T0_R)
      N_D=NORM_DELTA_PIK(TMIN-TOF,TMAX-TOF,T0_D)

      NORM_DELTA=F_D*N_D+(1-F_D)*N_R
      
      END

      
      REAL FUNCTION PDF_DELTA(TIME,ICH)

c--   PDF of photons from delta-electrons
      
      IMPLICIT NONE
      REAL TIME                 ! photon time
      INTEGER ICH               ! channel (=pixel-1)
#include "TOP_PIK.fi"
      REAL F_D                  ! fraction of direct photons
      REAL T0_D                 ! minimal propagation time of direct photons
      REAL T0_R                 ! minimal propagation time of reflected photons
      COMMON /TOP_DELTA/ F_D, T0_D, T0_R

      REAL T, PDF_D, PDF_R
      REAL SMEARED_TIME_DELTA
      
      T=TIME-TOF
      PDF_D=SMEARED_TIME_DELTA(T,T0_D)
      PDF_R=SMEARED_TIME_DELTA(T,T0_R)
      
      PDF_DELTA=(F_D*PDF_D+(1-F_D)*PDF_R)*PIK_DELTA(ICH)
      
      END
      

      REAL FUNCTION FRACT_DELTA(Z)

c--   fraction of direct photons from delta-electrons, e.g direct/(direct+reflected)

      IMPLICIT NONE
      REAL Z                    ! local z position of track at TOP
      
      REAL X, F
      INTEGER I
      REAL P(6) /0.332741, -0.00331502, 2.0801e-05, -3.43689e-09,
     &     -6.35849e-10, 3.54556e-12/
      SAVE P

      X=1
      F=0
      DO I=1,6
         F=F+P(I)*X
         X=X*Z
      ENDDO
      FRACT_DELTA=F
      
      END


      REAL FUNCTION SMEARED_TIME_DELTA(T,T0)

c--   time distribution of photons from delta electrons smeared by resolution

      IMPLICIT NONE
      REAL T                    ! propagation time
      REAL T0                   ! minimal propagation time
#include "TOP_PAR.fi"
      REAL TIME_DELTA
      REAL SIGMA, DX, X, S, F
      REAL SIG0 /0.15/
      integer NP
      PARAMETER (NP=101)
      REAL XP(NP), GAUS(NP)
      INTEGER I
      LOGICAL START /.TRUE./
      SAVE SIG0, XP, GAUS, START
      
      IF(START) THEN
         START=.FALSE.
         DX=6.0/(NP-1)
         X=-3.0
         DO I=1,NP
            XP(I)=X
            GAUS(I)=EXP(-0.5*X**2)
            X=X+DX
         ENDDO
         S=0
         DO I=1,NP
            S=S+GAUS(I)
         ENDDO
         DO I=1,NP
            GAUS(I)=GAUS(I)/S
         ENDDO
      ENDIF
      
      SIGMA=SQRT(SIG0**2+(REL_DNGDE*SIG_E*T)**2)
      IF(SIGMA.LT.0.001) THEN
         SMEARED_TIME_DELTA=TIME_DELTA(T,T0)
         RETURN
      ENDIF

      F=0
      DO I=1,NP
         F=F+TIME_DELTA(T-XP(I)*SIGMA,T0)*GAUS(I)
      ENDDO
      SMEARED_TIME_DELTA=F
      
      END
      

      REAL FUNCTION TIME_DELTA(T,T0)
      
c--   time distribution of photons from delta electrons

      IMPLICIT NONE
      REAL T                    ! propagation time
      REAL T0                   ! minimal propagation time

      REAL X, DX, KZ_DELTA
      INTEGER I
      integer N
      PARAMETER (N=1000)
      REAL NORM /0/
      SAVE NORM

      IF(NORM.EQ.0) THEN
         DX=1.0/N
         X=DX/2
         DO I=1,N
            NORM=NORM+KZ_DELTA(X)
            X=X+DX
         ENDDO
         NORM=NORM*DX
      ENDIF

      IF(T.LT.T0) THEN
         TIME_DELTA=0
      ELSE
         TIME_DELTA=KZ_DELTA(T0/T)/NORM*T0/T**2
      ENDIF
      
      END

      
      REAL FUNCTION NORM_DELTA_PIK(TMIN,TMAX,T0)

c--   normalization fraction of single peak within time interval
      
      IMPLICIT NONE
      REAL TMIN, TMAX           ! propagation time interval in which to normalize
      REAL T0                   ! minimal propagation time

      REAL X, KZ_DELTA
      INTEGER I, I1, I2
      integer NT
      PARAMETER (NT=1000)
      REAL NORM(0:NT), DX /0./
      SAVE NORM, DX

      IF(DX.EQ.0) THEN
         DX=1.0/NT
         X=DX/2
         NORM(0)=0.0
         DO I=1,NT
            NORM(I)=NORM(I-1)+KZ_DELTA(X)*DX
            X=X+DX
         ENDDO
         DO I=1,NT
            NORM(I)=NORM(I)/NORM(NT)
         ENDDO
      ENDIF

      IF(TMAX.LE.T0) THEN
         I1=NT
      ELSE
         X=T0/TMAX
         I1=INT(X/DX)
      ENDIF
      IF(TMIN.LE.T0) THEN
         I2=NT
      ELSE
         X=T0/TMIN
         I2=INT(X/DX)
      ENDIF
      
      NORM_DELTA_PIK=ABS(NORM(I2)-NORM(I1))
      
      END
      
      
      REAL FUNCTION KZ_DELTA(KZ)

c--   angular distribution of photons from delta electrons w/ total reflection requirement
c--   distribution is not normalized
      
      IMPLICIT NONE
      REAL KZ                   ! kz = cos(theta)
#include "TOP_PAR.fi"
#include "TOP_CONS.fi"
      REAL X
      
      IF(ABS(KZ).GE.1./REF_IND) THEN
         KZ_DELTA=1
      ELSE
         X= SQRT((1.-1./REF_IND**2)/(1.-KZ**2))
         KZ_DELTA=1.-ACOS(X)*4/PI
      ENDIF
      
      END


