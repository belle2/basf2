************************************************************************
      SUBROUTINE SMCONV(LT1,LT2,LP1,LP2,EW,AV1,AV2,LT,AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
*   * dummy array size.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      DOUBLE COMPLEX AV1(0:LASIZE), AV2(0:LASIZE), AV(0:LASIZE)
      INTEGER    LT1(0:LTSIZE), LT2(0:LTSIZE), LT(0:LTSIZE)
      INTEGER    LP1, LP2
      DIMENSION  EW(0:3)
*
*   Connect two partial amplitudes by a propagator of vector boson.
*   Denominator of the propagator should be multiplied elsewhere.
*
*      EW       : input  : Weight for propagator
*      AV1, AV2 : input  : Tables of amplitudes
*      LT1, LT2 : input  : Tables of sizes.
*      AV       : output : resulting table of amplitudes
*      LT       : output : resulting table of sizes
*
*      Declaration of table of amplitudes
*
*         DIMENTION AV(lav)
*         lav = (for i = 1 to n product LT(i)),  n = LT(0)
*
*      is equivalent to
*
*         DIMENSION AV(0:LT(1)-1, 0:LT(2)-1, ... , 0:LT(n)-1)
*
*      An element of the table
*
*         AV(i_1, i_2, ... , i_n),    0 <= i_j < LT(j)
*
*      is accessible by
*
*         AV(i)
*
*      where
*
*         i = (for j = 1 to n  sum i_j*base_j)
*         base_j = (for k = 1 to j - 1 product LT(k))
*
*      This subroutine calculates.
*
*        AV( i_1, ..., i_(LP1-1), i_(LP1+1), ..., i_(LT1(0)),
*            j_1, ..., j_(LP2-1), j_(LP2+1), ..., j_(LT2(0)) )
*
*        = (for k = 0 to LP-1  sum
*             AV1( i_1, ..., i_(LP1-1), k, i_(LP1+1), ..., i_(LT1(0)) )
*            *AV2( j_1, ..., j_(LP2-1), k, j_(LP2+1), ..., j_(LT2(0)) )
*            *EW(k) )
*
*       where, LP = LT1(LP1) = LT2(LP2) is a degree of freedom for
*       vector boson poralization.
*

*      COMPLEX*32 AVK
      DOUBLE COMPLEX AVK

*-----------------------------------------------------------------------
      LP = LT1(LP1)
      IF(LP.NE.LT2(LP2)) THEN
        WRITE(6,*) '*** SMCONV:INCONSISTENT TABLE SIZE OF AMPLITUDE.'
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
CC    IHIGH  = ILOW2 * IHIGH2
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
            AVK = AVK + AV1(K1)*AV2(K2)*EW(IPOL)
  200     CONTINUE
          AV(K) = AVK
  100 CONTINUE
*     CALL CTIME('SMCONV')
      RETURN
      END
