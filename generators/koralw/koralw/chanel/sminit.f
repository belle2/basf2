************************************************************************
      SUBROUTINE SMINIT(NV, NS)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (LVER=1, LSUBV=1)
*
*   INITIALIZE RUN-TIME RUOTINES
*
*   INPUT
*     NV, NS : VERSION NUMBER
*
*-----------------------------------------------------------------------
      IF(NV.NE.LVER .OR. NS.NE.LSUBV) THEN
        WRITE(6,100) NV, NS
  100   FORMAT(' ****** VERSION ',I4,'.',I2.2,' IS NOT SUPPORTED *****')
        STOP
      ENDIF
      RETURN
      END
