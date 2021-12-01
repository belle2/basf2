************************************************************************
      SUBROUTINE SMEXTV(LP, AM, PI, EP, EW, IGAUG)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0, ANOGAU=200.0D0)
      INTEGER LP
*     DOUBLE PRECISION    AM
      DIMENSION PI(4), EP(4,LP), EW(LP)
*
*   Caluculate polarization vectors for vector boson.
*
*     LP    : input  : degree of freedom
*                      2: ext. photon
*                      3: int. photon or ext. weak-boson
*                      4: int. weak-boson
*     AM    : input  : mass of the vector boson
*     PI    : input  : momentum of the vector boson
*     EP    : output : set of polarization vectors
*     EW    : output : weights for the polariztion vectors
*     IGAUG : input  : selctor of gauge parameter
*
* Common for Gauge Parameters 1:A, 2:W, 3:Z, 4:QCD
      COMMON /SMGAUG/AGAUGE(0:4)
*-----------------------------------------------------------------------
*TI
*      write(6,*) '<<SMEXTV>> LP ',LP
*TI
      DO 10 I = 1, LP
        EW(I) = 0.0
   10 CONTINUE
 
      IF(LP.EQ.1) THEN
        DO 20 I = 1, 4
          EP(I,1) = PI(I)
   20   CONTINUE
        EW(1) = 1.0D0
 
*     AXIAL GAUGE
      ELSE IF(IGAUG.EQ.-1) THEN
        VNK = PI(4) - PI(1)
        IF (VNK.NE.0.D0) THEN
          I   = 1
          J   = 3
        ELSE
          VNK = PI(4) - PI(3)
          I   = 3
          J   = 1
        ENDIF
        EP(4,1) = PI(2)/VNK
        EP(I,1) = PI(2)/VNK
        EP(2,1) = 1
        EP(J,1) = 0
        EP(4,2) = PI(J)/VNK
        EP(I,2) = PI(J)/VNK
        EP(2,2) = 0
        EP(J,2) = 1
        EW(1)   = 1
        EW(2)   = 1
        IF(LP.GT.2) THEN
          AKK = PI(4)**2 - PI(1)**2 - PI(2)**2 - PI(3)**2
          IF(AKK.GT.0) THEN
            SIGNK = 1.0
          ELSE
            SIGNK =-1.0
          ENDIF
          AKK   = SQRT(ABS(AKK))
          EP(4,3) = AKK/VNK
          EP(I,3) = AKK/VNK
          EP(2,3) = 0
          EP(J,3) = 0
          EW(3)   = SIGNK
        ENDIF
 
*     UNITARY GAUGE
      ELSE IF(IGAUG.EQ.0) THEN
        DO 30 I = 1, LP
          CALL POLA(I, ANOGAU, AM, PI, EP(1,I), EW)
   30   CONTINUE
 
*     COVARIANT GAUGE
      ELSE
        DO 40 I = 1, LP
          CALL POLA(I, AGAUGE(IGAUG), AM, PI, EP(1,I), EW)
   40   CONTINUE
      ENDIF
 
*     CALL CTIME('SMEXTV')
      RETURN
      END
