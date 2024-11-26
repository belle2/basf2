************************************************************************
      SUBROUTINE SMCONF(LT1,LT2,LP1,LP2,EW,AV1,AV2,LT,AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0)
*   * DUMMY ARRAY SIZE.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      DOUBLE COMPLEX AV1(0:LASIZE), AV2(0:LASIZE), AV(0:LASIZE)
      INTEGER    LT1(0:LTSIZE), LT2(0:LTSIZE), LT(0:LTSIZE)
      INTEGER    LP1, LP2
      DIMENSION  EW(2)
*
*   CONNECT TWO PARTIAL AMPLITUDES BY A FERMIONIC PROPAGATOR.
*   DENOMINATOR OF THE PROPAGATOR SHOULD BE MULTIPLIED ELSEWHERE.
*
*      EW       : INPUT  : WEIGHT FOR PROPAGATOR
*      AV1, AV2 : INPUT  : TABLES OF AMPLITUDES
*      LT1, LT2 : INPUT  : TABLES OF SIZES.
*      AV       : OUTPUT : TABLES OF AMPLITUDES
*      LT       : OUTPUT : TABLES OF SIZES.
*
*      DECLARATION OF TABLE OF AMPLITUDES
*
*         DIMENTION AV(LAV)
*         LAV = (FOR I = 1 TO N PRODUCT LT(I)),  N = LT(0)
*
*      IS EQUIVALENT TO
*
*         DIMENSION AV(0:LT(1)-1, 0:LT(2)-1, ... , 0:LT(N)-1)
*
*      AN ELEMENT OF THE TABLE
*
*         AV(I_1, I_2, ... , I_N),    0 <= I_J < LT(J)
*
*      IS ACCESSIBLE BY
*
*         AV(I)
*
*      WHERE
*
*         I = (FOR J = 1 TO N  SUM I_J*BASE_J)
*         BASE_J = (FOR K = 1 TO J - 1 PRODUCT LT(K))
*
*      THIS SUBROUTINE CALCULATES.
*
*        AV( I_1, ..., I_(LP1-1), I_(LP1+1), ..., I_(LT1(0)),
*            J_1, ..., J_(LP2-1), J_(LP2+1), ..., J_(LT2(0)) )
*
*        = (FOR K1 = 0 TO 1, K2 = 0 TO 1  SUM
*             AV1(I_1,...,I_(LP1-1),K1,K2,I_(LP1+1),...,I_(LT1(0)))
*            *AV2(J_1,...,J_(LP2-1),K1,K2,J_(LP2+1),...,J_(LT2(0)))
*            *EW(K2) )
*
*       WHERE, K1 IS THE SPIN INDEX AND K2 IS THE INDEX CORRESPONDING
*       TO DECOMPOSED MOMENTA L1 AND L2.
*
      DIMENSION EM(0:3)

*      COMPLEX*32 AVK
      DOUBLE COMPLEX AVK

*-----------------------------------------------------------------------
      EM(0) = EW(1)
      EM(1) = EW(1)
      EM(2) = EW(2)
      EM(3) = EW(2)
      LP = LT1(LP1)
      IF(LP.NE.LT2(LP2)) THEN
        WRITE(6,*) '*** SMCONF:INCONSISTENT TABLE SIZE OF AMPLITUDE.'
        WRITE(6,*) 'LT1(',LP1,') = ', LT1(LP1)
        WRITE(6,*) 'LT2(',LP2,') = ', LT2(LP2)
        WRITE(6,*) 'LT1 =', LT1
        WRITE(6,*) 'LT2 =', LT2
        STOP
      ENDIF
      J = 0
      ILOW1 = 1
      DO 10 I = 1, LP1 - 1
        J = J + 1
        LT(J) = LT1(I)
        ILOW1  = ILOW1 * LT1(I)
   10 CONTINUE
      IHIGH1 = 1
      DO 20 I = LP1 + 1, LT1(0)
        J = J + 1
        LT(J) = LT1(I)
        IHIGH1 = IHIGH1 * LT1(I)
   20 CONTINUE
      ILOW2 = 1
      DO 30 I = 1, LP2 - 1
        J = J + 1
        LT(J) = LT2(I)
        ILOW2  = ILOW2 * LT2(I)
   30 CONTINUE
      IHIGH2 = 1
      DO 40 I = LP2 + 1, LT2(0)
        J = J + 1
        LT(J) = LT2(I)
        IHIGH2 = IHIGH2 * LT2(I)
   40 CONTINUE
      LT(0) = J
      ILOW   = ILOW1 * IHIGH1
      IHIGH  = ILOW2 * IHIGH2
*
      DO 100 IL1 = 0, ILOW1  - 1
      DO 100 IH1 = 0, IHIGH1 - 1
        IL  = IH1 * ILOW1 + IL1
        KK1 = IH1 * ILOW1 * LP + IL1
        DO 100 IL2 = 0, ILOW2  - 1
        DO 100 IH2 = 0, IHIGH2 - 1
          IH  = IH2 * ILOW2 + IL2
          KK2 = IH2 * ILOW2 * LP + IL2
          K   = IH  * ILOW  + IL
          AVK = 0.0D0
          DO 200 IPOL = 0, LP - 1
            K1 = KK1 + IPOL * ILOW1
            K2 = KK2 + IPOL * ILOW2
            AVK = AVK + AV1(K1)*AV2(K2)*EM(IPOL)
  200     CONTINUE
          AV(K) = AVK
  100 CONTINUE
*     CALL CTIME('SMCONF')
      RETURN
      END
