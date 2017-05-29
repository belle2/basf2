!-----------------------------------------------------------------------
! utility functions
! M. Staric, feb-2000, sept-2008, sept-211, 
!
! mar-2017: change in some commons
!-----------------------------------------------------------------------

**** line

      SUBROUTINE SMER(THETA,FI,DIR)

! calculates unit vector from angles theta and phi

      REAL THETA, FI, DIR(3)

      DIR(1)=COS(FI)*SIN(THETA)
      DIR(2)=SIN(FI)*SIN(THETA)
      DIR(3)=COS(THETA)
      RETURN
      END


      SUBROUTINE PREMICA(RINIT,DIR,RFINL,PATH,IZB)

! to calculate the second point or the distance between two points
! of line given by:
! RINIT   initial point (input)
! DIR     unit vector of line direction (input)
! RFINL   final point (input/output)
! PATH    distance between initial and final points (input/output)
! depending on parameter
! IZB =0  PATH given
!     =1  RFINL(1) given
!     =2  RFINL(2) given
!     =3  RFINL(3) given
! calculates three other unknowns

      REAL RINIT(3), DIR(3), RFINL(3), PATH
      INTEGER IZB

      IF(IZB.NE.0) THEN
         IF(DIR(IZB).NE.0) THEN
	    PATH=(RFINL(IZB)-RINIT(IZB))/DIR(IZB)
         ELSE
            PATH=0.
         ENDIF
      ENDIF
      DO I=1,3
         RFINL(I)=RINIT(I)+PATH*DIR(I)
      ENDDO
      RETURN
      END


      SUBROUTINE SET_PREMICA(R1,R2,DIR)

! set line thru points R1(), R2()
! output is unit vector DIR()
! points must be different

      REAL R1(3), R2(3), DIR(3)

      DIRN=0.
      DO I=1,3
         DIR(I)=R2(I)-R1(I)
         DIRN=DIRN+DIR(I)**2
      ENDDO
      DIRN=SQRT(DIRN)
      IF(DIRN.EQ.0) RETURN
      DO I=1,3
         DIR(I)=DIR(I)/DIRN
      ENDDO
      RETURN
      END


**** rotations

      SUBROUTINE ROTATE_TO_FIXED(SMER_Z,VIN,VOUT)

! transformation of a vector from current system S' to fixed system S 
! SMER_Z() is unit vector of z-axis of S' given in fixed system S 
! VIN()  are the coordinates of a vector in S'
! VOUT() are the coordinates of a vector in S

      REAL SMER_Z(3), VIN(3), VOUT(3)

      IF(SMER_Z(3).EQ.1) THEN
         VOUT(1)=VIN(1)
         VOUT(2)=VIN(2)
         VOUT(3)=VIN(3)
      ELSE IF(SMER_Z(3).EQ.-1) THEN
         VOUT(1)=-VIN(1)
         VOUT(2)= VIN(2)
         VOUT(3)=-VIN(3)
      ELSE
         CTH=SMER_Z(3)
         STH=SQRT(1.-SMER_Z(3)**2)
         CFI=SMER_Z(1)/STH
         SFI=SMER_Z(2)/STH
! rotation by Theta around y axis
         X=CTH*VIN(1)+STH*VIN(3)
         Y=VIN(2)
         Z=-STH*VIN(1)+CTH*VIN(3)
! rotation by Fi around z axis
         VOUT(1)=CFI*X-SFI*Y
         VOUT(2)=SFI*X+CFI*Y
         VOUT(3)=Z
      ENDIF
      RETURN
      END


      SUBROUTINE ROTATE_FROM_FIXED(SMER_Z,VIN,VOUT)

! transformation of a vector from fixed system S to current system S'
! SMER_Z() is unit vector of z-axis of S' given in fixed system S 
! VIN()  are the coordinates of a vector in S
! VOUT() are the coordinates of a vector in S'

      REAL SMER_Z(3), VIN(3), VOUT(3)

      IF(SMER_Z(3).EQ.1) THEN
         VOUT(1)=VIN(1)
         VOUT(2)=VIN(2)
         VOUT(3)=VIN(3)
      ELSE IF(SMER_Z(3).EQ.-1) THEN
         VOUT(1)=-VIN(1)
         VOUT(2)= VIN(2)
         VOUT(3)=-VIN(3)
      ELSE
         CTH=SMER_Z(3)
         STH=SQRT(1.-SMER_Z(3)**2)
         CFI=SMER_Z(1)/STH
         SFI=SMER_Z(2)/STH
! rotation by -Fi around z axis
         X=CFI*VIN(1)+SFI*VIN(2)
         Y=-SFI*VIN(1)+CFI*VIN(2)
         Z=VIN(3)
! rotation by -Theta around y axis
         VOUT(1)=CTH*X-STH*Z
         VOUT(2)=Y
         VOUT(3)=STH*X+CTH*Z
      ENDIF
      RETURN
      END


      SUBROUTINE TRANSFORM_FROM_FIXED(SMER_Z,VIN,VOUT)

! transformation of a vector from fixed system S to current system S'
! such that x' is in plane (x,z) -  horizontal in HERA-B 
! SMER_Z() is unit vector of z-axis of S' given in fixed system S 
! VIN()  are the coordinates of a vector in S
! VOUT() are the coordinates of a vector in S'

      REAL SMER_Z(3), VIN(3), VOUT(3)
      REAL SMER_X(3), SMER_Y(3)

      SS=1.-SMER_Z(2)**2
      IF(SS.LE.0) THEN
         VOUT(1)=VIN(1)
         VOUT(2)=VIN(3)*SMER_Z(2)
         VOUT(3)=-VIN(2)*SMER_Z(2)
      ELSE
         SS=SQRT(SS)
         SMER_X(1)=SMER_Z(3)/SS
         SMER_X(2)=0.
         SMER_X(3)=-SMER_Z(1)/SS
         CALL VEKT_PROD(SMER_Z,SMER_X,SMER_Y)
         CALL SKAL_PROD(VIN,SMER_X,VOUT(1))
         CALL SKAL_PROD(VIN,SMER_Y,VOUT(2))
         CALL SKAL_PROD(VIN,SMER_Z,VOUT(3))
      ENDIF

      RETURN
      END


**** 3D vector

      SUBROUTINE VEKT_PROD(A,B,C)

! vector product: C = A x B

      REAL A(3), B(3), C(3)

      C(1)=A(2)*B(3)-A(3)*B(2)
      C(2)=A(3)*B(1)-A(1)*B(3)
      C(3)=A(1)*B(2)-A(2)*B(1)
      RETURN
      END		


      SUBROUTINE SKAL_PROD(A,B,S)

! scalar product: s=A.B

      REAL A(3), B(3), S

      S=A(1)*B(1)+A(2)*B(2)+A(3)*B(3)
      RETURN
      END


      REAL FUNCTION S_PROD(A,B)

! scalar product: s=A.B

      REAL A(3), B(3)

      S_PROD=A(1)*B(1)+A(2)*B(2)+A(3)*B(3)
      RETURN
      END


      SUBROUTINE NORMA_VEKT(A,D)

! norm of a vector A

      REAL A(3), D
      
      D=SQRT(A(1)**2+A(2)**2+A(3)**2)
      RETURN
      END


      SUBROUTINE ENOTNI_VEKT(A)

! unit vector from vector A

      REAL A(3)

      CALL NORMA_VEKT(A,D)
      IF(D.EQ.0) RETURN
      DO I=1,3
         A(I)=A(I)/D
      ENDDO
      RETURN
      END


      SUBROUTINE UNIT_VECT(A,B,D)

!     from vector A: unit vector B and norm D

      REAL A(3), B(3), D

      D=SQRT(A(1)**2+A(2)**2+A(3)**2)
      IF(D.EQ.0) RETURN
      DO I=1,3
         B(I)=A(I)/D
      ENDDO
      RETURN
      END


      SUBROUTINE V_SFERNE_K(R,RR,TH,FI)

! to spherical koordinates

      REAL R(3)                 ! vector (x,y,z)
      REAL RR, TH, FI
      
      RR=SQRT(R(1)**2+R(2)**2+R(3)**2)
      IF(RR.EQ.0) THEN
         TH=0.
         FI=0.
      ELSE
         FI=ATAN2(R(2),R(1))
         TH=ACOS(R(3)/RR)
      ENDIF      
      RETURN
      END


****  misc

      SUBROUTINE PHOTON_DIR(DIR,THC,FIC,DIRF,POL)

      IMPLICIT NONE
      REAL DIR(3), THC, FIC
      REAL DIRF(3), POL(3)

      REAL DIR1(3), CFI, SFI, CTH, STH

      CTH=COS(THC)
      STH=SIN(THC)
      CFI=COS(FIC)
      SFI=SIN(FIC)

      DIR1(1)=CFI*STH
      DIR1(2)=SFI*STH
      DIR1(3)=CTH
      CALL ROTATE_TO_FIXED(DIR,DIR1,DIRF)
      DIR1(1)=CFI*CTH
      DIR1(2)=SFI*CTH
      DIR1(3)=-STH
      CALL ROTATE_TO_FIXED(DIR,DIR1,POL)

      RETURN
      END


      SUBROUTINE TO_FIRST_PERIOD(FI)

      IMPLICIT NONE
      REAL FI
      REAL TWOPI
      DATA TWOPI /0./
      SAVE TWOPI

      IF(TWOPI.EQ.0) TWOPI=8.*ATAN(1.)
      DO WHILE(FI.GE.TWOPI)
         FI=FI-TWOPI
      ENDDO
      DO WHILE(FI.LT.0)
         FI=FI+TWOPI
      ENDDO

      RETURN
      END


