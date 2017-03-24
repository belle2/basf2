!-----------------------------------------------------------------------
! TOP counter for Belle: interface to TOP_RTRA
! M. Staric, mar-2009, mar-2010, 
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE RTRA_CLEAR

      IMPLICIT NONE
#include "TOP_RTRA.fi"
      DATA N_RTRA /0/
      DATA NHYP /0/

      N_RTRA=0
      END


      SUBROUTINE RTRA_SET_HYPO(NUMHYP,MASSES)

      IMPLICIT NONE
      INTEGER NUMHYP            ! number of mass hypotheses
      REAL MASSES(*)            ! masses
#include "TOP_RTRA.fi"
      INTEGER I

      NHYP=MIN(NUMHYP,NHYP_SIZ)
      DO I=1,NHYP
         RTRA_MASS(I)=MASSES(I)
         RTRA_HYPID(I)=I
      ENDDO

      END


      SUBROUTINE RTRA_SET_HYPID(NUMHYP,HYPID)

      IMPLICIT NONE
      INTEGER NUMHYP            ! number of mass hypotheses
      INTEGER HYPID(*)          ! hypotheses ID (1=e, 2=mu, 3=pi, 4=K, 5=p)
#include "TOP_RTRA.fi"
      INTEGER I

      NHYP=MIN(NUMHYP,NHYP_SIZ)
      DO I=1,NHYP
         RTRA_HYPID(I)=HYPID(I)
      ENDDO

      END


      SUBROUTINE RTRA_GET_HYPID(NUMHYP,HYPID)

      IMPLICIT NONE
      INTEGER NUMHYP            ! number of mass hypotheses
      INTEGER HYPID(*)          ! hypotheses ID (1=e, 2=mu, 3=pi, 4=K, 5=p)
#include "TOP_RTRA.fi"
      INTEGER I

      NHYP=MIN(NUMHYP,NHYP_SIZ)
      DO I=1,NHYP
         HYPID(I)=RTRA_HYPID(I)
      ENDDO

      END


      SUBROUTINE RTRA_PUT(X0,Y0,Z0,T0,PX,PY,PZ,Q,HYP,REF,MDN)

      IMPLICIT NONE
      REAL X0, Y0, Z0           ! track: point [cm] 
      REAL T0                   ! track: TOF for beta=1 from IP to point [ns]
      REAL PX, PY, PZ           ! track: momentum [GeV/c]
      INTEGER Q                 ! track: charge (+/-1)
      INTEGER HYP               ! hypothesis to measure pulls
      INTEGER REF               ! reference
      INTEGER MDN               ! module number
#include "TOP_RTRA.fi"
      INTEGER K, I
      REAL P

      K=N_RTRA+1
      IF(K.GT.NSIZ_RTRA) THEN
         PRINT*,'TOP_RTRA: no space for new track'
         RETURN
      ENDIF
      RTRA_R0(1,K)=X0
      RTRA_R0(2,K)=Y0
      RTRA_R0(3,K)=Z0
      RTRA_T0(K)=T0
      P=SQRT(PX**2+PY**2+PZ**2)
      RTRA_P(K)=P
      IF(P.EQ.0) P=1.
      RTRA_DIR(1,K)=PX/P
      RTRA_DIR(2,K)=PY/P
      RTRA_DIR(3,K)=PZ/P
      RTRA_CHARGE(K)=Q
      DO I=1,NHYP
         RTRA_PLKH(I,K)=0.
         RTRA_SFOT(I,K)=0.
      ENDDO
      RTRA_BFOT(K)=0
      RTRA_NFOT(K)=0
      RTRA_FLAG(K)=0
      RTRA_MOD(K)=MDN
      DO I=1,3
         RTRA_TR(I,K)=0.
         RTRA_TDIR(I,K)=0.
      ENDDO
      RTRA_TLEN(K)=0.
      RTRA_TOFC0(K)=0.
      RTRA_MTRA(K)=0
      RTRA_REF(K)=REF
      RTRA_HYP(K)=HYP
      N_RTRA=K

      END


      INTEGER FUNCTION RTRA_GETNUM()

      IMPLICIT NONE
#include "TOP_RTRA.fi"
      
      RTRA_GETNUM=N_RTRA
      END


      REAL FUNCTION RTRA_GET_PLKH(K,I)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_RTRA [in]
      INTEGER I                 ! hypothesis index
#include "TOP_RTRA.fi"
      
      IF(K.GT.0.AND.K.LE.N_RTRA.AND.I.GT.0.AND.I.LE.NHYP) THEN
         RTRA_GET_PLKH=RTRA_PLKH(I,K)
      ELSE
         RTRA_GET_PLKH=0.
      ENDIF
      END


      REAL FUNCTION RTRA_GET_SFOT(K,I)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_RTRA [in]
      INTEGER I                 ! hypothesis index
#include "TOP_RTRA.fi"
      
      IF(K.GT.0.AND.K.LE.N_RTRA.AND.I.GT.0.AND.I.LE.NHYP) THEN
         RTRA_GET_SFOT=RTRA_SFOT(I,K)
      ELSE
         RTRA_GET_SFOT=0.
      ENDIF
      END


      REAL FUNCTION RTRA_GET_BFOT(K)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_RTRA [in]
#include "TOP_RTRA.fi"
      
      IF(K.GT.0.AND.K.LE.N_RTRA) THEN
         RTRA_GET_BFOT=RTRA_BFOT(K)
      ELSE
         RTRA_GET_BFOT=0.
      ENDIF
      END


      INTEGER FUNCTION RTRA_GET_NFOT(K)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_RTRA [in]
#include "TOP_RTRA.fi"
      
      IF(K.GT.0.AND.K.LE.N_RTRA) THEN
         RTRA_GET_NFOT=RTRA_NFOT(K)
      ELSE
         RTRA_GET_NFOT=0
      ENDIF
      END


      INTEGER FUNCTION RTRA_GET_FLAG(K)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_RTRA [in]
#include "TOP_RTRA.fi"
      
      IF(K.GT.0.AND.K.LE.N_RTRA) THEN
         RTRA_GET_FLAG=RTRA_FLAG(K)
      ELSE
         RTRA_GET_FLAG=0
      ENDIF
      END


      SUBROUTINE RTRA_GET(K,PLKH,SFOT,NB,NFOT,FLAG,MTRA,REF)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_RTRA [in]
      REAL PLKH(*)              ! log likelihoods
      REAL SFOT(*)              ! expected number of photons
      INTEGER NB                ! size of output arrays
      INTEGER NFOT              ! number of detected photons
      INTEGER FLAG              ! flag
      INTEGER MTRA              ! pointer to MC particle
      INTEGER REF               ! reference
#include "TOP_RTRA.fi"
      INTEGER I

      DO I=1,NB
         PLKH(I)=0.
         SFOT(I)=0.
      ENDDO
      IF(K.GT.0.AND.K.LE.N_RTRA) THEN
         DO I=1,MIN(NHYP,NB)
            PLKH(I)=RTRA_PLKH(I,K)
            SFOT(I)=RTRA_SFOT(I,K)
         ENDDO
         NFOT=RTRA_NFOT(K)
         FLAG=RTRA_FLAG(K)
         MTRA=RTRA_MTRA(K)
         REF=RTRA_REF(K)
      ELSE
         NFOT=0
         FLAG=0
         MTRA=0
         REF=0
      ENDIF
      
      END


      SUBROUTINE RTRA_GETHIT(K,LOC,R,DIR,TLEN,TOF,P,MDN)

      IMPLICIT NONE
      INTEGER K                 ! index (row#) in TOP_RTRA [in]
      INTEGER LOC               ! 0/1 in local/global frame [in]
      REAL R(3)                 ! track point at TOP (local/global) [cm]
      REAL DIR(3)               ! track direction at TOP (local/global)
      REAL TLEN                 ! track length in Q-bar [cm]
      REAL TOF                  ! time-of-flight for beta=1 [ns]
      REAL P                    ! track momentum [GeV/c]
      INTEGER MDN               ! Q-bar ID
#include "TOP_RTRA.fi"
      INTEGER I

      IF(K.GT.0.AND.K.LE.N_RTRA) THEN
         DO I=1,3
            R(I)=RTRA_TR(I,K)
            DIR(I)=RTRA_TDIR(I,K)
         ENDDO
         TLEN=RTRA_TLEN(K)
         TOF=RTRA_T0(K)+RTRA_TOFC0(K)
         P=RTRA_P(K)
         MDN=RTRA_MOD(K)
         IF(LOC.NE.0) CALL TO_GLOBAL(R,DIR,MDN)
      ELSE
         DO I=1,3
            R(I)=0.
            DIR(I)=0.
         ENDDO
         TLEN=0.
         TOF=0.
         P=0.
         MDN=-1
      ENDIF
      
      END



