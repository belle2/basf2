C ********************************************************************
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *   SUBROUTINE PHASEQ(I:I*4, P:R*8(4), C:C*16(2))                  *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *   Purpose: To calculate phase factors for massive fermion.       *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *         I=: I=1 for complex phase C                              *
C *             I=2 for conjugate of phase C                         *
C *         P=: Momuntum of massive fermion.                         *
C *         C=: Calculated phase factors.                            *
C *                                            written by H. Tanaka  *
C ********************************************************************
C
C
C               ========================
                SUBROUTINE PHASEQ(I,P,C)
C               ========================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        PARAMETER (ONE = 1.0)
 
        DOUBLE COMPLEX C
        DIMENSION P(4),C(2)
 
        PTN=DSQRT(P(2)**2+P(3)**2)
        IF (PTN.LE.0.D0) THEN
          PYN=SIGN(ONE, P(1))
          PZN=0.0D0
        ELSE
          PYN=P(2)/PTN
          PZN=P(3)/PTN
        ENDIF
        IF (I.EQ.2) PZN=-PZN
        C(1)=DCMPLX(-PYN,PZN)
        C(2)=DCMPLX( PYN,PZN)
        RETURN
      END
