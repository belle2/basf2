************************************************************************
      SUBROUTINE SMEXTF(IO, AM, PE, PS, CE)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0)
      INTEGER    IO
*     DOUBLE PRECISION     AM
      DIMENSION  PE(4), PS(4,2)
      DOUBLE COMPLEX CE(2, 2)
*
*   Decompose momentum of internal fermion.
*
*         PE = L1 + L2
*         L1**2 = 0
*         L2**2 = 0
*         PS(*,1) = L1
*         PS(*,2) = L2
*
*     IO : input  : 1: imcoming, u or v    2: outgoing, ub or vb
*     AM : input  : mass of the fermion.
*     PE : input  : momentum of the fermion.
*     PS : output : decomposed momenta.
*     CE : output : phase factor, only CE(*, IO) is used.
*
*-----------------------------------------------------------------------
      CALL SPLTQ(AM, PE, PS(1,2), PS(1,1))
      CALL PHASEQ(IO, PE, CE(1,IO))
*     CALL CTIME('SMEXTF')
      RETURN
      END
