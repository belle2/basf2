
CDECK  ID>, DMORK.  
*.
*...DMORK    calculates the soft and virtual correction.
*.
*.  DMORK is a double precision function that returns the soft and
*.  virtual radiative correction. This routine contains the corrections
*.  to the misprints as communicated by K.J. Mork.
*.  Reference: Phys. Rev. A4 (1971) 917.
*.
*. INPUT     : T      kinematical quantity
*. INPUT     : Y      kinematical quantity
*. INPUT     : U      kinematical quantity
*. INPUT     : E      kinematical quantity
*. INPUT     : DE     kinematical quantity
*.
*. CALLS     : SPENCE
*. CALLED    : T3BODY
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.1
*. CREATED   : 29-Jun-87
*. LAST MOD  : 28-Sep-88
*.
*. Modification Log.
*. 28-Sep-88   Dean Karlen   Change to conform to OPAL standards.
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C DOUBLE PRECISION FUNCTION DMORK: Calculates the soft and virtual corr.
C This routine contains the corrections to the misprints as communicated
C by K.J. Mork. Reference: Phys. Rev. A4 (1971) 917.
C-----------------------------------------------------------------------

      DOUBLE PRECISION FUNCTION DMORK(T,Y,U,E,DE)
      IMPLICIT NONE

      DOUBLE PRECISION PI,ALPHA
      PARAMETER (PI=3.14159265358979D0 , ALPHA=1.D0/137.036D0)

      DOUBLE PRECISION T,Y,U,E,DE,TERM(10),SUM
      INTEGER   I

      DOUBLE PRECISION SPENCE
      EXTERNAL SPENCE

      TERM(1) = 2.D0*(1.D0-2.D0*Y)*U*LOG(2.D0*DE)
      TERM(2) = PI**2/6.D0*(4.D0-3.D0*T-1.D0/T-2.D0/E**4/T**3)
      TERM(3) = 4.D0*(2.D0-U)*Y**2
      TERM(4) = -4.D0*Y+1.5D0*U+2.D0/E**2/T**2
      TERM(5) = 4.D0*(1.D0-.5D0/T)*LOG(E)**2
      TERM(6) = (2.D0*T+1.D0/T-2.D0+2.D0/E**4/T**3)*SPENCE(1.D0-E**2*T)
      TERM(7) = (2.D0-5.D0*T-2.D0/T+4.D0*Y*(2.D0/T+T-2.D0))*LOG(E)
      TERM(8) = -.5D0*U*LOG(1-T)**2
      TERM(9) = -U*SPENCE(T)
      TERM(10)= (1.D0-2.D0/T-2.D0/E**2/T**2-.5D0*E**2/(1.D0-E**2*T)
     >           +4.D0*Y*(T-1.D0+.5D0/T))*LOG(E**2*T)

      SUM=0.D0
      DO 1 I=1,10
 1    SUM=SUM+TERM(I)

      DMORK=-ALPHA/PI/U * SUM

      RETURN
      END
