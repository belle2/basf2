************************************************************************
      SUBROUTINE SMCONS(LT1,LT2,LP1,LP2,AV1,AV2,LT,AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
*   * dummy array size.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      DOUBLE COMPLEX AV1(0:LASIZE), AV2(0:LASIZE), AV(0:LASIZE)
      INTEGER    LP1, LP2
      INTEGER    LT1(0:LTSIZE), LT2(0:LTSIZE), LT(0:LTSIZE)
*
*   Connect two partial amplitudes by a propagator of vector boson.
*   Denominator of the propagator should be multiplied elsewhere.
*
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
*        AV( i_1, ..., i_(LT1(0)), j_1, ..., j_(LT2(0)) )
*
*        =    AV1( i_1, ..., i_(LT1(0)) ) * AV2( j_1, ..., j_(LT2(0)) )
*
*-----------------------------------------------------------------------
      IF(LT1(LP1).NE.1 .OR. LT2(LP2).NE.1) THEN
        WRITE(6,*) '*** SMCONS:INCONSISTENT TABLE SIZE OF AMPLITUDE.'
        WRITE(6,*) 'LT1(',LP1,') = ', LT1(LP1)
        WRITE(6,*) 'LT2(',LP2,') = ', LT2(LP2)
        WRITE(6,*) 'LT1 =', LT1
        WRITE(6,*) 'LT2 =', LT2
        STOP
      ENDIF
      LT(0) = LT1(0) + LT2(0) - 2
      IT = 1
      IL1 = 1
      DO 10 I = 1, LT1(0)
        IF(LP1.NE.I) THEN
          IL1    = IL1*LT1(I)
          LT(IT) = LT1(I)
          IT     = IT + 1
        ENDIF
   10 CONTINUE
      IL2 = 1
      DO 20 I = 1, LT2(0)
        IF(LP2.NE.I) THEN
          IL2 = IL2*LT2(I)
          LT(IT) = LT2(I)
          IT     = IT + 1
        ENDIF
   20 CONTINUE
      K = 0
      DO 100 K2 = 0, IL2 - 1
      DO 100 K1 = 0, IL1 - 1
        AV(K) = AV1(K1)*AV2(K2)
        K = K + 1
  100 CONTINUE
*     CALL CTIME('SMCONS')
      RETURN
      END
