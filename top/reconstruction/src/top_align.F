!-----------------------------------------------------------------------
! TOP counter for Belle II: Alignment
! M. Staric, sep-2015
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE TOP_ALIGNMENT(NP,PAR,STEP,UM,DPAR,COV,IER)

c--   alignment iteration
      
      IMPLICIT NONE
      INTEGER NP                ! number of alignment parameters [in]
      REAL PAR(NP)              ! initial (current) alignment parameters [in]
      REAL STEP(NP)             ! step sizes [in]
      REAL*8 UM(NP,NP)          ! matrix (neg. sum of second derivatives) [in/out]
      REAL DPAR(NP)             ! delta par [out] if IER=0, otherwise unchaged 
      REAL COV(NP,NP)           ! covariance matrix [out] if IER=0, otherwise unchanged
      INTEGER IER               ! error status [out]
#include "TOP_RTRA.fi"
      REAL FIRST(NP)
      REAL SECND(NP,NP)
      REAL*8 UUU(NP,NP)
      REAL*8 VM(NP,NP)
      LOGICAL OK
      INTEGER I, K
      EXTERNAL TOP_LOGL
      REAL TOP_LOGL
      
      IER=-2
      IF(N_RTRA.NE.1) RETURN
      CALL SET_PDF_OPT(0)

      IER=-1
      CALL CALC_DERIV(TOP_LOGL,NP,PAR,STEP,FIRST,SECND,OK)
      IF(.NOT.OK) RETURN

      UM=UM-SECND
      CALL TOP_MDECO(UM,UUU,NP,IER)
      IF(IER.EQ.0) THEN
         CALL TOP_MINVERS(UUU,VM,NP)
         DO I=1,NP
            DPAR(I)=0
            DO K=1,NP
               DPAR(I)=DPAR(I)+VM(I,K)*FIRST(K)
            ENDDO
         ENDDO
         COV=VM
      ENDIF
      
      END


      REAL FUNCTION TOP_LOGL(PAR,NPAR,OK)

c--   returns log likelihood for given alignment parameters 
      
      IMPLICIT NONE
      INTEGER NPAR              ! number of parameters [in]
      REAL PAR(NPAR)            ! parameters [in]
      LOGICAL OK                ! status [out]
#include "TOP_RTRA.fi"
#include "TOP_PIK.fi"
      INTEGER MDN
      REAL DX, DY, DZ, RX, RY, RZ, T0
      REAL GET_LOGL

      IF(NPAR.LT.7) THEN
         TOP_LOGL=0
         OK=.FALSE.
         RETURN
      ENDIF
      
      MDN=RTRA_MOD(1)
      DX=PAR(1)
      DY=PAR(2)
      DZ=PAR(3)
      RX=PAR(4)
      RY=PAR(5)
      RZ=PAR(6)
      T0=PAR(7)

      CALL SET_ALIGNMENT(MDN,DX,DY,DZ,RX,RY,RZ)
      CALL TOP_RECO
      TOP_LOGL=GET_LOGL(T0,T_MIN,T_MAX,0.0)
      OK=RTRA_FLAG(1).EQ.1
      
      END
      

      SUBROUTINE CALC_DERIV(FUN,NVAR,VAR0,STEP,FIRST,SECND,OK)

c--   calculate numerically first and second derivatives of an external function
      
      IMPLICIT NONE
      REAL FUN                  ! function: fun(var,nvar,ok) [in]
      INTEGER NVAR              ! number of variables [in]
      REAL VAR0(NVAR)           ! variable values at which to calculate derivatives [in]
      REAL STEP(NVAR)           ! steps [in]
      REAL FIRST(NVAR)          ! vector of first derivatives [out]
      REAL SECND(NVAR,NVAR)     ! matrix of second derivatives (symmetric) [out]
      LOGICAL OK                ! status [out]

      INTEGER J, K
      REAL F0, FP, FM, FPP, FPM, FMP, FMM
      REAL VAR(NVAR)

      F0=FUN(VAR0,NVAR,OK)
      IF(.NOT.OK) RETURN
      
      VAR=VAR0
      DO K=1,NVAR
         VAR(K)=VAR0(K)+STEP(K)
         FP=FUN(VAR,NVAR,OK)
         IF(.NOT.OK) RETURN
         
         VAR(K)=VAR0(K)-STEP(K)
         FM=FUN(VAR,NVAR,OK)
         IF(.NOT.OK) RETURN

         FIRST(K)=(FP-FM)/2./STEP(K)
         SECND(K,K)=(FP-2*F0+FM)/STEP(K)**2

         VAR(K)=VAR0(K)
      ENDDO

      DO K=1,NVAR
         DO J=K+1,NVAR
            VAR(K)=VAR0(K)+STEP(K)
            VAR(J)=VAR0(J)+STEP(J)
            FPP=FUN(VAR,NVAR,OK)
            IF(.NOT.OK) RETURN

            VAR(J)=VAR0(J)-STEP(J)
            FPM=FUN(VAR,NVAR,OK)
            IF(.NOT.OK) RETURN

            VAR(K)=VAR0(K)-STEP(K)
            VAR(J)=VAR0(J)+STEP(J)
            FMP=FUN(VAR,NVAR,OK)
            IF(.NOT.OK) RETURN

            VAR(J)=VAR0(J)-STEP(J)
            FMM=FUN(VAR,NVAR,OK)
            IF(.NOT.OK) RETURN

            SECND(J,K)=(FPP-FMP-FPM+FMM)/4./STEP(K)/STEP(J)
            SECND(K,J)=SECND(J,K)
            
            VAR(J)=VAR0(J)
         ENDDO
         VAR(K)=VAR0(K)
      ENDDO

      END

      
      SUBROUTINE TOP_MDECO(A,U,N,IER)

c--   symmetrical matrix decomposition (Choletski)

      IMPLICIT NONE

      INTEGER N                 ! dimension of matrix [in]
      REAL*8 A(N,N)             ! symmetric matrix [in]
      REAL*8 U(N,N)             ! upper triangular matrix [out]
      INTEGER IER               ! 0=OK, on error IER=row where error [out]

      REAL*8 S
      INTEGER I,J,K

      IER=0
      DO I=1,N
         S=0.
         DO J=1,I-1
            S=S+U(J,I)**2
         ENDDO
         S=A(I,I)-S
         IF(S.LE.0) THEN
            IER=I
            RETURN
         ELSE
            U(I,I)=SQRT(S)
         ENDIF
         DO K=I+1,N
            S=0.
            DO J=1,I-1
               S=S+U(J,I)*U(J,K)
            ENDDO
            U(I,K)=(A(I,K)-S)/U(I,I)
         ENDDO
      ENDDO

      END


      SUBROUTINE TOP_MINVERS(U,AI,N)

c--   matrix inversion after decomposition

      IMPLICIT NONE

      INTEGER N                 ! dimension [in]
      REAL*8 U(N,N)             ! upper triangular matrix (from decomposition) [in]
      REAL*8 AI(N,N)            ! inverse matrix [out]

      REAL*8 S
      INTEGER I,K,J

      DO J=1,N
         DO I=1,J-1
            AI(I,J)=0.
         ENDDO
         I=J
         S=0.
         DO K=1,I-1
            S=S+U(K,I)*AI(K,J)
         ENDDO
         AI(I,J)=(1.-S)/U(I,I)
         DO I=J+1,N
            S=0.
            DO K=1,I-1
               S=S+U(K,I)*AI(K,J)
            ENDDO
            AI(I,J)=(-S)/U(I,I)
         ENDDO
         DO I=N,1,-1
            S=0.
            DO K=I+1,N
               S=S+U(I,K)*AI(K,J)
            ENDDO
            AI(I,J)=(AI(I,J)-S)/U(I,I)
         ENDDO
      ENDDO

      END


      
