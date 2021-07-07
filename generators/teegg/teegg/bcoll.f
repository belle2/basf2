
CDECK  ID>, BCOLL.  
*.
*...BCOLL    determines if collinear photon calcuation should be used.
*.
*. INPUT     : P      electron 4-momentum
*. INPUT     : K      photon 4-momentum
*.
*. CALLED    : BEEGGC
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
C DOUBLE PRECISION FUNCTION BCOLL: Determines if photon is collinear
C with a fermion. Does not need to be very accurate.
C-----------------------------------------------------------------------

      DOUBLE PRECISION FUNCTION BCOLL(P,K)
      IMPLICIT NONE

      DOUBLE PRECISION P(4),K(4)
      DOUBLE PRECISION COST,SINT

      BCOLL=999.D0
      COST=(P(1)*K(1)+P(2)*K(2)+P(3)*K(3))
     >     /SQRT(P(1)**2+P(2)**2+P(3)**2)/K(4)
      SINT=SQRT(1.D0-COST**2)
      IF(COST.GT.0.D0)BCOLL=K(4)*SINT

      RETURN
      END
