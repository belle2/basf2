
C End of non-APOLLO specific code

CDECK  ID>, TPRD.   
*.
*...TPRD     calcuates the invariant product of two 4-vectors
*.
*. INPUT     : A      a 4-vector
*. INPUT     : B      another 4-vector
*.
*. CALLED    : T4BODY BEEGGC
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
C DOUBLE PRECISION FUNCTION TPRD: Calculates the invariant product.
C-----------------------------------------------------------------------

      DOUBLE PRECISION FUNCTION TPRD(A,B)
      IMPLICIT NONE
      DOUBLE PRECISION A(4),B(4)
      TPRD = A(4)*B(4) - A(1)*B(1) - A(2)*B(2) - A(3)*B(3)
      RETURN
      END
