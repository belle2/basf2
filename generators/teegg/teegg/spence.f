
CDECK  ID>, SPENCE. 
*.
*...SPENCE   calculates the spence function for x < 1.
*.
*. INPUT     : X
*.
*. CALLED    : DMORK
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.2
*. CREATED   : 29-Jun-87
*. LAST MOD  : 10-Apr-89
*.
*. Modification Log.
*. 28-Sep-88   Dean Karlen   Change to conform to OPAL standards.
*. 10-Apr-89   Dean Karlen   Replace code by CERNLIB DILOG function.
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C DOUBLE PRECISION FUNCTION SPENCE: Calculates the Spence function.(X<1)
C-----------------------------------------------------------------------

C  DOUBLE PRECISION VERSION OF DILOGARITHM FUNCTION FROM CERNLIB.

      DOUBLE PRECISION FUNCTION SPENCE(X)
      IMPLICIT NONE

      DOUBLE PRECISION X,Y,Z,A,B,S,T

      Z=-1.644934066848226 D0
      IF(X .LT.-1.0 D0) GO TO 1
      IF(X .LE. 0.5 D0) GO TO 2
      IF(X .EQ. 1.0 D0) GO TO 3
      IF(X .LE. 2.0 D0) GO TO 4
C
      Z=3.289868133696453 D0
    1 T=1.0 D0/X
      S=-0.5 D0
      Z=Z-0.5 D0*DLOG(DABS(X))**2
      GO TO 5
C
    2 T=X
      S=0.5 D0
      Z=0. D0
      GO TO 5
C
    3 SPENCE=1.644934066848226 D0
      RETURN
C
    4 T=1.0 D0-X
      S=-0.5 D0
      Z=1.644934066848226 D0-DLOG(X)*DLOG(DABS(T))
C
    5 Y=2.666666666666667 D0*T+0.666666666666667 D0
      B=      0.00000 00000 00001 D0
      A=Y*B  +0.00000 00000 00004 D0
      B=Y*A-B+0.00000 00000 00011 D0
      A=Y*B-A+0.00000 00000 00037 D0
      B=Y*A-B+0.00000 00000 00121 D0
      A=Y*B-A+0.00000 00000 00398 D0
      B=Y*A-B+0.00000 00000 01312 D0
      A=Y*B-A+0.00000 00000 04342 D0
      B=Y*A-B+0.00000 00000 14437 D0
      A=Y*B-A+0.00000 00000 48274 D0
      B=Y*A-B+0.00000 00001 62421 D0
      A=Y*B-A+0.00000 00005 50291 D0
      B=Y*A-B+0.00000 00018 79117 D0
      A=Y*B-A+0.00000 00064 74338 D0
      B=Y*A-B+0.00000 00225 36705 D0
      A=Y*B-A+0.00000 00793 87055 D0
      B=Y*A-B+0.00000 02835 75385 D0
      A=Y*B-A+0.00000 10299 04264 D0
      B=Y*A-B+0.00000 38163 29463 D0
      A=Y*B-A+0.00001 44963 00557 D0
      B=Y*A-B+0.00005 68178 22718 D0
      A=Y*B-A+0.00023 20021 96094 D0
      B=Y*A-B+0.00100 16274 96164 D0
      A=Y*B-A+0.00468 63619 59447 D0
      B=Y*A-B+0.02487 93229 24228 D0
      A=Y*B-A+0.16607 30329 27855 D0
      A=Y*A-B+1.93506 43008 69969 D0
      SPENCE=S*T*(A-B)+Z
      RETURN
      END
