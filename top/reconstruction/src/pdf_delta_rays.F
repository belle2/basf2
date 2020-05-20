!-----------------------------------------------------------------------
! TOP counter for Belle: Reconstruction - PDF of photons from delta rays
! M. Staric, may-2020
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
!-----------------------------------------------------------------------

      REAL FUNCTION NFOT_DELTA(BETA,PDG)

c--   number of photons from delta-electrons for nominal detection efficiency
      
      IMPLICIT NONE
      REAL BETA                 ! particle beta
      INTEGER PDG               ! PDG code
#include "TOP_TRACK.fi"
      REAL F
      REAL AP(3) /13.5859, -28.0625, 17.2684/ ! pol2 parameters for anti-protons
      REAL P(5) /-8.15871, 10.0082, -1.25140, -120.225, 120.210/ ! parameters for others
      REAL AVRG_RQE /1.06404889/ ! average RQE (relative to nominal PDE)
      SAVE AP, P, AVRG_RQE

      IF(ABS(PDG).EQ.11) THEN   ! electrons and positrons
         F=5.30
      ELSE IF(PDG.EQ.-2212) THEN ! anti-protons
         F=AP(1)+AP(2)*BETA+AP(3)*BETA**2
      ELSE                      ! others
         F=EXP(P(1)+P(2)*BETA+P(3)*BETA**2) + EXP(P(4)+P(5)*BETA)
      ENDIF
      NFOT_DELTA=F*T_LEN/AVRG_RQE
      
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


      REAL FUNCTION PDF_DELTA(TIME,ICH)

c--   PDF of photons from delta-electrons
      
      IMPLICIT NONE
      REAL TIME                 ! photon time
      INTEGER ICH               ! channel
#include "TOP_PIK.fi"
#include "TOP_TRACK.fi"
      REAL T, F, Z
      REAL FRACT_DELTA, SMEARED_TIME_DELTA
      
      T=TIME-TOF
      Z=T_R0(3)
      F=FRACT_DELTA(Z)

      PDF_DELTA=(F*SMEARED_TIME_DELTA(T,Z,0) +
     &     (1-F)*SMEARED_TIME_DELTA(T,Z,1))*PIK_DELTA(ICH)
      
      END
      

      REAL FUNCTION SMEARED_TIME_DELTA(T,Z,LR)

c--   time distribution of photons from delta electrons smeared by resolution

      IMPLICIT NONE
      REAL T                    ! propagation time
      REAL Z                    ! local z position of track at TOP
      INTEGER LR                ! 0 direct, 1 reflected
#include "TOP_PAR.fi"
#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
#include "TOP_TRACK.fi"

      REAL TIME_DELTA
      REAL T0, SIGMA, DX, X, S, F
      REAL SIG0 /0.15/
      integer NP
      PARAMETER (NP=101)
      REAL XP(NP), GAUS(NP)
      INTEGER I
      LOGICAL START /.TRUE./
      SAVE SIG0, XP, GAUS, START
      
      IF(LR.EQ.0) THEN
         T0=(140.6+Z)*GRU_IND/C0 ! TODO
      ELSE
         T0=(400.6-Z)*GRU_IND/C0 ! TODO
      ENDIF

      SIGMA=SQRT(SIG0**2+(REL_DNGDE*SIG_E*T)**2)
      IF(SIGMA.LT.0.001) THEN
         SMEARED_TIME_DELTA=TIME_DELTA(T,T0)
         RETURN
      ENDIF

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
      REAL T0                   ! propagation time of photon propagating along z axis (first coming one)

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
         print*,'norm=',norm
      ENDIF

      IF(T.LT.T0) THEN
         TIME_DELTA=0
      ELSE
         TIME_DELTA=KZ_DELTA(T0/T)/NORM*T0/T**2
      ENDIF
      
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


