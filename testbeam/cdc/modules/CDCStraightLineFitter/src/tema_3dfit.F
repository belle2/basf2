!-----------------------------------------------------------------------
! fitting routines: straight line in 3D
! M. Staric, mar-2001
!
!-----------------------------------------------------------------------


      SUBROUTINE TEMA_FIT_3D

      IMPLICIT NONE
#include "TEMA_HITS.fi"
#include "TEMA_RSEG.fi"
#include "TEMA_PAR.fi"
#include "TEMA_ONE.fi"

      INTEGER ISEG, I, K

      DO ISEG=1,NSEG
         IF(FIT_SEG(ISEG)) THEN
            CALL TEMA_PRO3DLIN(SEG_R(1,ISEG),SEG_S(1,ISEG),Z_CENTER,
     &           SEG_R(1,ISEG))
            CALL TEMA_FIT_SEG(ISEG)

cc            IF(FIT_SEG(ISEG)) THEN
cc               DO I=1,NT
cc                  IF(USED(I)) THEN
cc                     K=KPT(I)
cc                     IK(K)=ISEG
cc                     QHIT(K)=SIGN(QHIT(K),XM(I)-XH(I))
cc                     CALL TEMA_PRO3DLIN(SEG_R(1,ISEG),SEG_S(1,ISEG),
cc     &                    ZH(I),RPHIT(1,K))
cc                  ENDIF
cc               ENDDO
cc               DO K=1,3
cc                  NL_SEG(K,ISEG)=0
cc               ENDDO
cc            ENDIF

         ENDIF
      ENDDO

      RETURN
      END



      SUBROUTINE TEMA_ONE_SEGM(ISEG,K0)

! collect hits of one segment, store in /TEMA_ONE/
      IMPLICIT NONE
      INTEGER ISEG, K0

#include "TEMA_HITS.fi"
#include "TEMA_RSEG.fi"
#include "TEMA_ONE.fi"
      REAL Z0, DD, WW(3), F, X(4)
      INTEGER I, K, II
      REAL TEMA_DOT_PROD
      
      NT=0
      Z0=SEG_R(3,ISEG)
      DO II=IP_SEG(ISEG-1)+1,IP_SEG(ISEG)
         K=INDIK(II)
         IK(K)=0
         IF(SHIT(1,K).LE.0.OR.SHIT(2,K).LE.0) GOTO 1
         NT=NT+1
         IF(NT.GT.NSIZ) GOTO 2
         SIGMA(NT)=SHIT(K0+1,K)

         USED(NT)=.FALSE.
         CALL TEMA_VECT_PROD(WIRE_S(1,K),SEG_S(1,ISEG),ROT(1,1,NT))
         CALL TEMA_UNIT_VECT(ROT(1,1,NT),DD)
         IF(DD.EQ.0) THEN
            PRINT*,'cudno??'
            CYCLE
         ENDIF
         DO I=1,3
            ROT(I,2,NT)=WIRE_S(I,K)
         ENDDO
         CALL TEMA_VECT_PROD(ROT(1,1,NT),ROT(1,2,NT),ROT(1,3,NT))

         DO I=1,3
            WW(I)=SEG_R(I,ISEG)-WIRE_R0(I,K)
         ENDDO
         DD=TEMA_DOT_PROD(SEG_S(1,ISEG),ROT(1,3,NT))
         IF(DD.EQ.0) CYCLE
         ZH(NT)=Z0-TEMA_DOT_PROD(WW,ROT(1,3,NT))/DD

         CD(1,NT)=ROT(1,1,NT)
         CD(2,NT)=ROT(2,1,NT)
         CD(3,NT)=(ZH(NT)-Z0)*CD(1,NT)
         CD(4,NT)=(ZH(NT)-Z0)*CD(2,NT)

         WW(1)=WW(1)+SEG_S(1,ISEG)*(ZH(NT)-Z0)
         WW(2)=WW(2)+SEG_S(2,ISEG)*(ZH(NT)-Z0)
         WW(3)=ZH(NT)-WIRE_R0(3,K)
         F=TEMA_DOT_PROD(WW,ROT(1,1,NT))

         WW(1)=WIRE_R0(1,K)
         WW(2)=WIRE_R0(2,K)
         WW(3)=-WW(3)
         DD=TEMA_DOT_PROD(WW,ROT(1,1,NT))
         IF(K0.EQ.0) THEN
            XM(NT)=DD
         ELSE
            IF(F.GT.0) THEN
               XM(NT)=DD+QHIT(K)
            ELSE
               XM(NT)=DD-QHIT(K)
            ENDIF
         ENDIF
         KPT(NT)=K
         HI2(NT)=0.
         USED(NT)=.TRUE.
 1       CONTINUE
      ENDDO
 2    CONTINUE
      IF(NT.GT.NSIZ) THEN
         PRINT*,'TEMA: tema_fit_3d_, NT > NSIZ'
         NT=NSIZ
      ENDIF

      RETURN
      END



      SUBROUTINE TEMA_FIT_SEG(ISEG)

      IMPLICIT NONE
      INTEGER ISEG

#include "TEMA_RSEG.fi"
      INTEGER K0, ITE
      LOGICAL OK

      K0=0
      CALL TEMA_ONE_SEGM(ISEG,K0)
      CALL TEMA_FIT3DLINE(ISEG,OK)
      FIT_SEG(ISEG)=OK.AND.NDF_SEG(ISEG).GT.0
      IF(.NOT.FIT_SEG(ISEG)) RETURN
      K0=1
      DO ITE=1,10
         CALL TEMA_ONE_SEGM(ISEG,K0)
         CALL TEMA_FIT3DLINE(ISEG,OK)
         FIT_SEG(ISEG)=OK.AND.NDF_SEG(ISEG).GT.0
         IF(.NOT.FIT_SEG(ISEG)) RETURN
      ENDDO
      CALL TEMA_OSTALO(ISEG)

      RETURN
      END



      SUBROUTINE TEMA_FIT3DLINE(ISEG,OK)

      IMPLICIT NONE
      INTEGER ISEG              ! segment number
      LOGICAL OK                ! fit status

#include "TEMA_RSEG.fi"
#include "TEMA_ONE.fi"
      REAL BM(4)                ! right side of equ. to be filled
      REAL X(4)                 ! solution of Ax=b
      REAL UM(4,4)
      INTEGER N
      INTEGER I,K,KK,IER
      REAL F

! construct covariance matrix
      CALL TEMA_MMAKE(NT,XM,SIGMA,USED,CD,AM,BM,N)
! matrix inversion
      CALL TEMA_MDECO(AM,UM,4,IER)
      OK=IER.EQ.0
      IF(.NOT.OK) RETURN
      CALL TEMA_MINVERS(UM,AM,4)
! store covariance matrix
      KK=0
      DO K=1,4
         DO I=1,K
            KK=KK+1
            SEG_CV(KK,ISEG)=AM(I,K)
         ENDDO
      ENDDO
! new fit parameters
      DO K=1,4
         X(K)=0.
         DO I=1,4
            X(K)=X(K)+AM(I,K)*BM(I)
         ENDDO
      ENDDO
      SEG_R(1,ISEG)=X(1)
      SEG_R(2,ISEG)=X(2)
      SEG_S(1,ISEG)=X(3)
      SEG_S(2,ISEG)=X(4)
! chi2, n.d.f
      NDF_SEG(ISEG)=N-4
      SEG_HI2(ISEG)=0.
      DO K=1,NT
         F=0.
         DO I=1,4
            F=F+X(I)*CD(I,K)
         ENDDO
         HI2(K)=((XM(K)-F)/SIGMA(K))**2
         IF(USED(K)) THEN
            SEG_HI2(ISEG)=SEG_HI2(ISEG)+HI2(K)
         ENDIF
      ENDDO

      RETURN
      END



      SUBROUTINE TEMA_MMAKE(NT,XM,SIG,USED,CD,AM,BM,NN)

! construct matrix of system of equations - no multiple scattering

      IMPLICIT NONE

      INTEGER NT                ! number of measurements
      REAL XM(*)                ! measurements
      REAL SIG(*)               ! measurement errors
      LOGICAL USED(*)           ! flag
      REAL CD(4,*)              ! derivatives of fit funct (d/dx,d/dy,d/dtx,d/dty)
      REAL AM(4,4)              ! matrix
      REAL BM(4)                ! right side
      INTEGER NN                ! number of entries

      INTEGER I,K,J
      REAL SIG2

      NN=0
      DO K=1,4
         DO I=1,4
            AM(I,K)=0.
         ENDDO
         BM(K)=0.
      ENDDO

      DO J=1,NT
         IF(USED(J)) THEN
            NN=NN+1
            SIG2=SIG(J)**2
            DO K=1,4
               DO I=1,K
                  AM(I,K)=AM(I,K)+CD(I,J)*CD(K,J)/SIG2
               ENDDO
               BM(K)=BM(K)+XM(J)*CD(K,J)/SIG2
            ENDDO
         ENDIF
      ENDDO
      DO K=2,4
         DO I=1,K-1
            AM(K,I)=AM(I,K)
         ENDDO
      ENDDO

      RETURN
      END



      SUBROUTINE TEMA_MDECO(A,U,N,IER)

! symmetrical matrix decomposition (Choletski)

      IMPLICIT NONE

      INTEGER N                 ! dimension of matrix [in]
      REAL A(N,N)		! symmetric matrix [in]
      REAL U(N,N)		! upper triangular matrix [out]
      INTEGER IER               ! 0=OK, on error IER=row where error [out]

      REAL S
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
      RETURN
      END

      

      SUBROUTINE TEMA_MINVERS(U,AI,N)

! matrix inversion after decomposition

      IMPLICIT NONE

      INTEGER N                 ! dimension [in]
      REAL U(N,N)		! upper triangular matrix (from decomposition) [in]
      REAL AI(N,N)              ! inverse matrix [out]

      REAL S
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
      RETURN
      END	
      


      SUBROUTINE TEMA_OSTALO(ISEG)

      IMPLICIT NONE
      INTEGER ISEG
#include "TEMA_RSEG.fi"
#include "TEMA_ONE.fi"
      REAL AMM(4,4), QMS(4,4)
      REAL ZMI, ZMA, TX, TY, PF, Z0
      INTEGER I,K,KK

! inverse momentum Q/p (target constrain) 
      CALL TEMA_RSEG_PF(ISEG)
! begin, end of track
      ZMI=10000.
      ZMA=-10000.
      DO I=1,NT
         IF(USED(I)) THEN
            ZMI=MIN(ZMI,ZH(I))
            ZMA=MAX(ZMA,ZH(I))
         ENDIF
      ENDDO
      SEG_ZMI(ISEG)=ZMI
      SEG_ZMA(ISEG)=ZMA
      CALL TEMA_PRO3DLIN(SEG_R(1,ISEG),SEG_S(1,ISEG),ZMI,SEG_RF(1,ISEG))
      CALL TEMA_PRO3DLIN(SEG_R(1,ISEG),SEG_S(1,ISEG),ZMA,SEG_RE(1,ISEG))
! multiple scattering contribution to covariance matrix
      TX=SEG_S(1,ISEG)
      TY=SEG_S(2,ISEG)
      PF=SEG_PF(ISEG)
      CALL TEMA_QMS(SEG_R(1,ISEG),TX,TY,PF,ZMI,ZMA,QMS)
! covariance matrix at begin
      Z0=SEG_R(3,ISEG)
      CALL TEMA_PRO3DCOV(AM,Z0,AMM,ZMI)
      KK=0
      DO K=1,4
         DO I=1,K
            KK=KK+1
            SEG_CVF(KK,ISEG)=AMM(I,K)+QMS(K,I)
         ENDDO
      ENDDO
! covariance matrix at end
      CALL TEMA_PRO3DCOV(AM,Z0,AMM,ZMA)
      KK=0
      DO K=1,4
         DO I=1,K
            KK=KK+1
            SEG_CVE(KK,ISEG)=AMM(I,K)+QMS(I,K)
         ENDDO
      ENDDO

      RETURN
      END



      SUBROUTINE TEMA_QMS(R0,TX,TY,PF,ZF,ZE,QMS)

      IMPLICIT NONE

      REAL R0(3), TX, TY, PF    ! segment with Q/p
      REAL ZF, ZE               ! begin, end of segment
      REAL QMS(4,4)             ! scattering contribution to cov. matrix
                                ! QMS(K,I) K=1,4 I=1,K : begin of segment      
                                ! QMS(I,K) K=1,4 I=1,K : end of segment      

      REAL DZ(8), RADL(8)
      REAL THICK, CS, COSI, CMS, CXX, CXY, CYY, DL
      INTEGER I,K

      DO I=1,4
         DO K=1,4
            QMS(I,K)=0
         ENDDO
      ENDDO
      RETURN                    ! ne upostevam, treba popraviti spodaj!


      CALL TEMA_CROS_PC(R0,TX,TY,ZF,ZE,DZ,RADL)
      THICK=0.
      DO I=1,8
         THICK=THICK+DZ(I)/RADL(I)
      ENDDO
      CS=1+TX**2+TY**2
      COSI=SQRT(CS)
      THICK=THICK*COSI
      CMS=(THICK*CS*(13.6E-3*PF)**2)/3.
      CXX=(1+TX**2)*CMS
      CYY=(1+TY**2)*CMS
      CXY=TX*TY*CMS
      DL=(ZE-ZF)*COSI/4.

      QMS(1,1)=CXX*(DL**2)/3.
      QMS(1,2)=CXY*(DL**2)/3.
      QMS(1,3)=CXX*DL/2.
      QMS(1,4)=CXY*DL/2.

      QMS(2,1)=QMS(1,2)
      QMS(2,2)=CYY*(DL**2)/3.
      QMS(2,3)=CXY*DL/2.
      QMS(2,4)=CYY*DL/2.

      QMS(3,1)=-QMS(1,3)
      QMS(3,2)=-QMS(2,3)
      QMS(3,3)=CXX
      QMS(3,4)=CXY

      QMS(4,1)=-QMS(1,4)
      QMS(4,2)=-QMS(2,4)
      QMS(4,3)=QMS(3,4)
      QMS(4,4)=CYY

      RETURN
      END



      SUBROUTINE TEMA_CROS_PC(R0,TX,TY,ZF,ZE,DZ,RADL)

! returns approx. lengths of a segment projected to z for each PC superlayer
! order is (pc01,pc02,pc03,pc04,ms10,ms11,ms12,ms13)

      IMPLICIT NONE
      REAL R0(3), TX, TY        ! segment [in]
      REAL ZF, ZE               ! begin z, end z of a segment [in]
      REAL DZ(8)                ! lengths 
      REAL RADL(8)              ! rad. lengths

      LOGICAL OK
      REAL X,Y,Z
      INTEGER K

      REAL Z1(8), Z2(8), X0(8), Y0(8), XR(8), YR(8), RDL(8)
      SAVE Z1,Z2,X0,Y0,XR,YR,RDL
      DATA Z1 /704.3, 744.8, 780.4, 825.1, 684.1, 733.0, 768.6, 804.6/
      DATA Z2 /728.0, 764.0, 799.6, 848.9, 701.0, 741.1, 776.7, 821.5/
      DATA X0 /205., 205., 205., 205., 25., 25., 25., 25./
      DATA Y0 /140., 140., 140., 140., 50., 50., 50., 50./
      DATA XR /20., 20., 20., 20., 0., 0., 0., 0./
      DATA YR /20., 20., 20., 20., 0., 0., 0., 0./
      DATA RDL /593., 533., 533., 593., 155., 153., 153., 155./ 

      DO K=1,8
         RADL(K)=RDL(K)
         DZ(K)=0.
         IF(ZF.LT.Z2(K).AND.ZE.GT.Z1(K)) THEN
            Z=(Z1(K)+Z2(K))/2.
            X=ABS(R0(1)+TX*(Z-R0(3)))
            Y=ABS(R0(2)+TY*(Z-R0(3)))
            OK=X.LT.X0(K).AND.Y.LT.Y0(K)
            OK=OK.AND..NOT.(X.LT.XR(K).AND.Y.LT.YR(K))
            IF(OK) THEN
               DZ(K)=MIN(Z2(K),ZE)-MAX(Z1(K),ZF)
            ENDIF
         ENDIF
      ENDDO
      
      RETURN
      END



      SUBROUTINE TEMA_RSEG_PF(ISEG)

      IMPLICIT NONE
      INTEGER ISEG
#include "TEMA_RSEG.fi"
      REAL Z1, X1, ALFA, BETA, FI

CC      Z1=450.
CC      X1=SEG_R(1,ISEG)+SEG_S(1,ISEG)*(Z1-SEG_R(3,ISEG))
CC      ALFA=ATAN(X1/Z1)
CC      BETA=ATAN(SEG_S(1,ISEG))
CC      FI=BETA-ALFA
CC      SEG_PF(ISEG)=FI/.644/SQRT(1.+SEG_S(2,ISEG)**2)
      SEG_PF(ISEG)=1.0
      
      RETURN
      END



      SUBROUTINE TEMA_PRO3DLIN(R0,SV,Z,R)

! 3D line: new point at z
      IMPLICIT NONE

      REAL R0(3)                ! point on line (x0,y0,z0)
      REAL SV(3)                ! direction vector (tx,ty,1)
      REAL Z                    ! z of new point
      REAL R(3)                 ! new point on line (x,y,z)

      R(1)=R0(1)+SV(1)*(Z-R0(3))
      R(2)=R0(2)+SV(2)*(Z-R0(3))
      R(3)=Z

      RETURN
      END



      SUBROUTINE TEMA_PRO3DCOV(CV,Z0,COV,Z)

! propagation of covariance matrix of parameters to new z
! state vector is (x,y,tx,ty)
      IMPLICIT NONE

      REAL CV(4,4)              ! initial covariant matrix at position Z0 [in]
      REAL Z0                   ! z position [in]
      REAL COV(4,4)             ! covariant matrix at new position Z [out]
      REAL Z                    ! z position [in]

      REAL F(4,4), C(4,4)
      DATA F /
     &     1.,0.,0.,0.,
     &     0.,1.,0.,0.,
     &     0.,0.,1.,0.,
     &     0.,0.,0.,1./
      SAVE F

      REAL DZ
      INTEGER K, I, J, L

      DZ=Z-Z0
      F(1,3)=DZ
      F(2,4)=DZ

      DO K=1,4
         DO I=1,4
            C(I,K)=0.
            DO J=1,4
               C(I,K)=C(I,K)+F(I,J)*CV(J,K)
            ENDDO
         ENDDO
      ENDDO
      DO L=1,4
         DO I=1,4
            COV(I,L)=0.
            DO K=1,4
               COV(I,L)=COV(I,L)+C(I,K)*F(L,K)
            ENDDO
         ENDDO
      ENDDO

      RETURN
      END



      SUBROUTINE TEMA_VECT_PROD(A,B,C)

! vector product: C = A x B

      REAL A(3), B(3), C(3)

      C(1)=A(2)*B(3)-A(3)*B(2)
      C(2)=A(3)*B(1)-A(1)*B(3)
      C(3)=A(1)*B(2)-A(2)*B(1)
      END               


      REAL FUNCTION TEMA_DOT_PROD(A,B)

! scalar product: s=A.B

      REAL A(3), B(3)

      TEMA_DOT_PROD=A(1)*B(1)+A(2)*B(2)+A(3)*B(3)
      END


      SUBROUTINE TEMA_UNIT_VECT(A,D)

! unit vector from vector A

      REAL A(3), D

      D=SQRT(A(1)**2+A(2)**2+A(3)**2)
      IF(D.EQ.0) RETURN
      DO I=1,3
         A(I)=A(I)/D
      ENDDO
      RETURN
      END
