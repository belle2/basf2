!-----------------------------------------------------------------------
! TOP counter for Belle: Helix
! M. Staric, oct-2008, jan-2010 (added B=0 case), avg-2015, 
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
!
! Helix along z-axis; t'=t*beta is running parameter
!         --------- B.ne.0 -----------------------    --- B.eq.0--------
! HLX(1)  R      radius [cm]                          Z0  point [cm]
! HLX(2)  XC     x-center [cm]                        X0  point [cm]
! HLX(3)  YC     y-center [cm]                        Y0  point [cm]
! HLX(4)  OMEGA  omega/beta [1/ns] - angular speed    =0
! HLX(5)  VZ     vz/beta [cm/ns]   - speed in z       VZ  vz/beta [cm/ns]
! HLX(6)  PHI0   phi0              - angle at t'=0    VX  vx/beta [cm/ns]
! HLX(7)  Z0     z0 [cm]           - z at t'=0        VY  vy/beta [cm/ns]
! HLX(8)  S_T    pT/p                                 S_T pT/p
! HLX(9)  S_Z    pz/p                                 S_Z pz/p
! HLX(10) T0     2*pi/OMEGA        - t' period        =0
! mar-2017: change in some commons
!-----------------------------------------------------------------------


      SUBROUTINE SET_HELIX(R0,DIR,P,CHARGE,HLX)

      IMPLICIT NONE
      REAL R0(3)                ! track point at t=t0 [in]
      REAL DIR(3)               ! track direction at t=t0 [in]
      REAL P                    ! track momentum [in]
      REAL CHARGE               ! track charge in units of e0 (+/-1) [in]
      REAL HLX(10)              ! helix [out]
#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
      REAL S_T, R, OMEGA, VZ, FI0, XC, YC

      S_T=SQRT(DIR(1)**2+DIR(2)**2)
      IF(ABS(B_FIELD).GT.S_T*P/C0) THEN ! R < 100 m
         R=10000./C0*S_T*P/ABS(B_FIELD)
         OMEGA=C0**2/10000.*CHARGE*B_FIELD/P
         VZ=C0*DIR(3)
         FI0=ATAN2(DIR(2),DIR(1))-SIGN(PI/2.,OMEGA)
         IF(FI0.LT.-PI) FI0=FI0+2*PI
         IF(FI0.GT.PI) FI0=FI0-2*PI
         XC=R0(1)-R*COS(FI0)
         YC=R0(2)-R*SIN(FI0)
         HLX(1)=R
         HLX(2)=XC
         HLX(3)=YC
         HLX(4)=OMEGA
         HLX(5)=VZ
         HLX(6)=FI0
         HLX(7)=R0(3)
         HLX(8)=SIGN(S_T,OMEGA)
         HLX(9)=DIR(3)
         HLX(10)=2.*PI/ABS(OMEGA)
      ELSE
         HLX(1)=R0(3)
         HLX(2)=R0(1)
         HLX(3)=R0(2)
         HLX(4)=0.
         HLX(5)=C0*DIR(3)
         HLX(6)=C0*DIR(1)
         HLX(7)=C0*DIR(2)
         HLX(8)=S_T
         HLX(9)=DIR(3)
         HLX(10)=0.
      ENDIF

      END


      SUBROUTINE MOVE_PIVOT(HLX,T)

      IMPLICIT NONE
      REAL HLX(10)              ! helix [in/out]
      REAL T                    ! t' [in]

      IF(HLX(4).NE.0) THEN
         HLX(6)=HLX(6)+HLX(4)*T
         HLX(7)=HLX(7)+HLX(5)*T
      ELSE
         HLX(1)=HLX(1)+HLX(5)*T
         HLX(2)=HLX(2)+HLX(6)*T
         HLX(3)=HLX(3)+HLX(7)*T
      ENDIF

      END


      SUBROUTINE GET_HLX_RDIR(HLX,T,R0,DIR)

      IMPLICIT NONE
      REAL HLX(10)              ! helix [in]
      REAL T                    ! t' [in]
      REAL R0(3), DIR(3)        ! track parameters at t' [out]
#include "TOP_CONS.fi"
      REAL FI, CFI, SFI

      IF(HLX(4).NE.0) THEN
         FI=HLX(4)*T+HLX(6)
         CFI=COS(FI)
         SFI=SIN(FI)
         R0(1)=HLX(2)+HLX(1)*CFI
         R0(2)=HLX(3)+HLX(1)*SFI
         R0(3)=HLX(5)*T+HLX(7)
         DIR(1)=-HLX(8)*SFI
         DIR(2)=HLX(8)*CFI
         DIR(3)=HLX(9)
      ELSE
         R0(1)=HLX(2)+HLX(6)*T
         R0(2)=HLX(3)+HLX(7)*T
         R0(3)=HLX(1)+HLX(5)*T
         DIR(1)=HLX(6)/C0
         DIR(2)=HLX(7)/C0
         DIR(3)=HLX(5)/C0
      ENDIF

      END


      SUBROUTINE XSEC_PLANE(HLX,RP,NORM,T,OK)

!     cross-section of helix and plane nearly paralel to z-axis (norm(3) << 1)

      IMPLICIT NONE
      REAL HLX(10)              ! helix [in]
      REAL RP(3), NORM(3)       ! plane: point, normal [in]
      REAL T                    ! t' of nearest x-section in the flight direction [out]
      LOGICAL OK                ! existance of x-sections [out]
#include "TOP_CONS.fi"

      REAL R(3), V(3), S, ALFA, AA, T1, T2, T_PREV, DT, DT_PREV
      REAL S_PROD, FIRST_PERIOD
      INTEGER I
      
      T=0
      IF(HLX(4).NE.0) THEN
         R(1)=RP(1)-HLX(2)
         R(2)=RP(2)-HLX(3)
         R(3)=RP(3)-HLX(7)
         ALFA=ATAN2(NORM(2), NORM(1))
         S=S_PROD(R,NORM)
         DT=0
         DO I=1,100
            T_PREV=T
            DT_PREV=DT
            AA=(S-HLX(5)*T*NORM(3))/HLX(1)
            OK=ABS(AA).LE.1
            IF(.NOT.OK) RETURN
            AA=ACOS(AA)
            T1=FIRST_PERIOD((ALFA-HLX(6)-AA)/HLX(4),HLX(10))
            T2=FIRST_PERIOD((ALFA-HLX(6)+AA)/HLX(4),HLX(10))
            T=MIN(T1,T2)
            DT=T-T_PREV
            IF(DT.EQ.0) RETURN
            IF(DT+DT_PREV.eq.0) RETURN
         ENDDO
         OK=.FALSE.
         PRINT*,'*** Error in TOP reconstruction (xsec_plane): ',
     &        'not converged'
      ELSE
         R(1)=RP(1)-HLX(2)
         R(2)=RP(2)-HLX(3)
         R(3)=RP(3)-HLX(1)
         V(1)=HLX(6)
         V(2)=HLX(7)
         V(3)=HLX(5)
         S=S_PROD(V,NORM)
         OK=ABS(S).GT.1E-10
         IF(.NOT.OK) RETURN
         T=S_PROD(R,NORM)/S
         OK=T.GE.0
      ENDIF

      END


      REAL FUNCTION FIRST_PERIOD(T,T0)

      IMPLICIT NONE
      REAL T, T0
      REAL TT

      TT=T
      DO WHILE(TT.GE.T0)
         TT=TT-T0
      ENDDO
      DO WHILE(TT.LT.0)
         TT=TT+T0
      ENDDO
      FIRST_PERIOD=TT
      
      END

      
      SUBROUTINE XSEC_PLANE_X(HLX,X0,T1,T2,OK)

!     cross-section of helix and plane at x=X0

      IMPLICIT NONE
      REAL HLX(10)              ! helix [in]
      REAL X0                   ! plane coordinate [in]
      REAL T1, T2               ! t' of first and second x-sections (0<T1<T2) [out]
      LOGICAL OK                ! existance of x-sections [out]
#include "TOP_CONS.fi"

      REAL X, Y, FI1, FI2, DFI1, DFI2, TT

      IF(HLX(4).NE.0) THEN
         X=X0-HLX(2)
         Y=HLX(1)**2-X**2
         OK=Y.GE.0
         IF(.NOT.OK) RETURN
         Y=SQRT(Y)
         FI1=ATAN2(Y,X)
         FI2=-FI1
         DFI1=FI1-HLX(6)
         DFI2=FI2-HLX(6)
         IF(HLX(4).LT.0) THEN
            DFI1=-DFI1
            DFI2=-DFI2
         ENDIF
         CALL TO_FIRST_PERIOD(DFI1)
         CALL TO_FIRST_PERIOD(DFI2)
         T1=DFI1/ABS(HLX(4))
         T2=DFI2/ABS(HLX(4))
         IF(T2.LT.T1) THEN
            TT=T1
            T1=T2
            T2=TT
         ENDIF
      ELSE
         OK=ABS(HLX(6)).GT.1E-10
         IF(.NOT.OK) RETURN
         T1=(X0-HLX(2))/HLX(6)
         T2=T1
         OK=T1.GE.0
      ENDIF

      END


      SUBROUTINE XSEC_PLANE_Y(HLX,Y0,T1,T2,OK)

!     cross-section of helix and plane at Y=Y0

      IMPLICIT NONE
      REAL HLX(10)              ! helix [in]
      REAL Y0                   ! plane coordinate [in]
      REAL T1, T2               ! t' of first and second x-sections (0<T1<T2) [out]
      LOGICAL OK                ! existance of x-sections [out]
#include "TOP_CONS.fi"

      REAL X, Y, FI1, FI2, DFI1, DFI2, TT

      IF(HLX(4).NE.0) THEN
         Y=Y0-HLX(3)
         X=HLX(1)**2-Y**2
         OK=X.GE.0
         IF(.NOT.OK) RETURN
         X=SQRT(X)
         FI1=ATAN2(Y,X)
         FI2=SIGN(PI,FI1)-FI1
         DFI1=FI1-HLX(6)
         DFI2=FI2-HLX(6)
         IF(HLX(4).LT.0) THEN
            DFI1=-DFI1
            DFI2=-DFI2
         ENDIF
         CALL TO_FIRST_PERIOD(DFI1)
         CALL TO_FIRST_PERIOD(DFI2)
         T1=DFI1/ABS(HLX(4))
         T2=DFI2/ABS(HLX(4))
         IF(T2.LT.T1) THEN
            TT=T1
            T1=T2
            T2=TT
         ENDIF
      ELSE
         OK=ABS(HLX(7)).GT.1E-10
         IF(.NOT.OK) RETURN
         T1=(Y0-HLX(3))/HLX(7)
         T2=T1
         OK=T1.GE.0
      ENDIF

      END


      SUBROUTINE XSEC_PLANE_Z(HLX,Z0,T,OK)

!     cross-section of helix and plane at Z=Z0

      IMPLICIT NONE
      REAL HLX(10)              ! helix [in]
      REAL Z0                   ! plane coordinate [in]
      REAL T                    ! t' of x-section [out]
      LOGICAL OK                ! existance of x-sections [out]

      OK=ABS(HLX(5)).GT.1E-10
      IF(.NOT.OK) RETURN

      IF(HLX(4).NE.0) THEN
         T=(Z0-HLX(7))/HLX(5)
      ELSE
         T=(Z0-HLX(1))/HLX(5)
      ENDIF

      END


      SUBROUTINE XSEC_CYL(HLX,R,T1,T2,OK)

!     cross-section of helix and cylinder oriented in z and centered at 0,0

      IMPLICIT NONE
      REAL HLX(10)              ! helix [in]
      REAL R                    ! radius of cylinder [in]
      REAL T1, T2               ! t' of first and second x-sections (0<T1<T2) [out]
      LOGICAL OK                ! existance of x-sections [out]
#include "TOP_CONS.fi"
      REAL RC2, CC, ALPHA, DELTA, DFI1, DFI2, TT
      REAL A, B, C, D

      IF(HLX(4).NE.0) THEN
         OK=HLX(1).GT.0
         IF(.NOT.OK) RETURN
         RC2=HLX(2)**2+HLX(3)**2
         OK=RC2.GT.0
         IF(.NOT.OK) RETURN
         CC=(R**2-HLX(1)**2-RC2)/2./HLX(1)/SQRT(RC2)
         OK=ABS(CC).LE.1
         IF(.NOT.OK) RETURN
         DELTA=ACOS(CC)
         ALPHA=ATAN2(HLX(3),HLX(2))
         DFI1=ALPHA+DELTA-HLX(6)
         DFI2=ALPHA-DELTA-HLX(6)
         IF(HLX(4).LT.0) THEN
            DFI1=-DFI1
            DFI2=-DFI2
         ENDIF
         CALL TO_FIRST_PERIOD(DFI1)
         CALL TO_FIRST_PERIOD(DFI2)
         T1=DFI1/ABS(HLX(4))
         T2=DFI2/ABS(HLX(4))
         IF(T2.LT.T1) THEN
            TT=T1
            T1=T2
            T2=TT
         ENDIF
      ELSE
         A=HLX(6)**2+HLX(7)**2
         OK=A.GT.1E-10
         IF(.NOT.OK) RETURN
         B=2*(HLX(6)*HLX(2)+HLX(7)*HLX(3))
         C=HLX(2)**2+HLX(3)**2-R**2
         D=B**2-4*A*C
         OK=D.GE.0
         IF(.NOT.OK) RETURN
         D=SQRT(D)
         T1=(-B+D)/2/A
         T2=(-B-D)/2/A
         IF(T1.LT.0) THEN
            TT=T1
            T1=T2
            T2=TT
         ENDIF
      ENDIF

      END


      SUBROUTINE HLX_TO_LOCAL(HLX,M,HLXL)

!     transformation of helix to local frame of Q-bar

      IMPLICIT NONE
      REAL HLX(10)              ! helix in global frame [in]
      INTEGER M                 ! Q-bar module number [in]
      REAL HLXL(10)             ! helix in local frame [out]
#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
      INTEGER I

      DO I=1,10
         HLXL(I)=HLX(I)
      ENDDO
      IF(M.LT.0.OR.M.GE.NUM_MOD) RETURN

      IF(HLX(4).NE.0) THEN
         HLXL(2)=HLX(2)*QBAR_CFI(M)+HLX(3)*QBAR_SFI(M)-QBAR_DX(M)
         HLXL(3)=HLX(3)*QBAR_CFI(M)-HLX(2)*QBAR_SFI(M)-QBAR_R(M)
         HLXL(7)=HLX(7)-QBAR_DZ(M)
         HLXL(6)=HLX(6)-QBAR_FI(M)
      ELSE
         HLXL(1)=HLX(1)-QBAR_DZ(M)
         HLXL(2)=HLX(2)*QBAR_CFI(M)+HLX(3)*QBAR_SFI(M)-QBAR_DX(M)
         HLXL(3)=HLX(3)*QBAR_CFI(M)-HLX(2)*QBAR_SFI(M)-QBAR_R(M)
         HLXL(6)=HLX(6)*QBAR_CFI(M)+HLX(7)*QBAR_SFI(M)
         HLXL(7)=HLX(7)*QBAR_CFI(M)-HLX(6)*QBAR_SFI(M)
      ENDIF

      END



