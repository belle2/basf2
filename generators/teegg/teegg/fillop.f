
CDECK  ID>, FILLOP. 
*.
*...FILLOP   fills the LUJET common with 4-momenta from TEEGG
*.
*. INPUT     : PVEC   4-momenta for the final state electrons/photons
*. INPUT     : EB     electron/positron beam energy
*. INPUT     : M      electron mass
*.
*. SEQUENCE  : LUJETS
*. CALLS     : GOLINT GOLIFE
*. CALLED    : KIUSER
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 1.0
*. CREATED   : 28-Sep-88
*. LAST MOD  : 01-May-89
*.
*. Modification Log.
*. 01-May-89  Dean Karlen  Change the initial electron direction to +z
*.
*.**********************************************************************

      SUBROUTINE FILLOP(PVEC,EB,M)

      IMPLICIT NONE

      COMMON/LUJETS/N,K(4000,5),P(4000,5),V(4000,5)
      INTEGER N,K
      REAL P,V
      SAVE /LUJETS/
*

      DOUBLE PRECISION PVEC(4,4),EB,M
      INTEGER  ELCTRN,PHOTON,I,J
      PARAMETER  (ELCTRN=7,PHOTON=1)

      N = 5
      IF (PVEC(4,4).NE.0.) N = 6

C Set four momenta

      P(1,1)=0.
      P(1,2)=0.
      P(1,3)=SQRT(EB**2-M**2)
      P(1,4)=EB
      P(2,1)=0.
      P(2,2)=0.
      P(2,3)=-SQRT(EB**2-M**2)
      P(2,4)=EB
      DO 2 J=1,4
         DO 1 I=1,4
            P(J+2,I) = PVEC(I,J)
 1       CONTINUE
 2    CONTINUE

C Set masses

      DO 3 I=1,4
         P(I,5) = M
 3    CONTINUE
      P(5,5)=0.
      P(6,5)=0.

C Status and history:

      K(1,1) = 40000
      K(2,1) = 40000
      K(3,1) = 20001
      K(4,1) = 20002
      K(5,1) = 20001
      K(6,1) = 20001

C Particle ID

      K (1,2) =   ELCTRN
      K (2,2) = - ELCTRN
      K (3,2) =   ELCTRN
      K (4,2) = - ELCTRN
      K (5,2) =   PHOTON
      K (6,2) =   PHOTON

      CALL GOLINT
      CALL GOLIFE

      RETURN
      END
