************************************************************************
      SUBROUTINE SMINTP(LP, AM, PI, EP, EW, VM, IGAUG)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0, ANOGAU=200.0D0)
      INTEGER LP
*     DOUBLE PRECISION    AM, VM
      DIMENSION PI(4), EP(4,LP), EW(LP)
*
*   Caluculate polarization vectors for vector boson.
*   QED vertex with on-shell fermions.
* Gauge invarience can not be checked by using this subroutine.
*
*
*     LP    : input  : degree of freedom
*                      2: ext. photon
*                      3: int. photon or ext. weak-boson
*                      4: int. weak-boson
*     NP    : OUTPUT : EFFECTIVE VALUE OF LP
*     AM    : input  : mass of the vector boson
*     PI    : input  : momentum of the vector boson
*     EP    : output : set of polarization vectors
*     EW    : output : weights for the polariztion vectors
*     VM    : input  : PI.PI
*     IGAUG : input  : selctor of gauge parameter
*
* Common for Gauge Parameters 1:A, 2:W, 3:Z, 4:QCD
      COMMON /SMGAUG/AGAUGE(0:4)

      DATA DD/0.0D0/
*-----------------------------------------------------------------------
      DO 40 I = 1, LP
        EW(I) = 0.0D0
   40 CONTINUE

CX    WRITE(6,*) ' ------  SMINTP --------- '
      IF(LP.EQ.1) THEN
        DO 30 I = 1, 4
          EP(I,1) = PI(I)
   30   CONTINUE
        EW(1) = 1.0D0
        NP    = 1
      END IF

        APP=ABS(VM)
        RAPP = SQRT(APP)

*     COVARIANT GAUGE
        PT2 = PI(1)**2 + PI(2)**2
        IF(PT2.LE.DD) THEN
          PT  = 0.0D0
          RTY = 1.0D0
          RTX = 0.0D0
        ELSE
          PT  = SQRT(PT2)
          PTI = 1.0D0/PT
          RTY = PI(2)*PTI
          RTX = PI(1)*PTI
        ENDIF
        PN2 = PT2 + PI(3)**2
CX      PP = (PI(4)-PI(3))*(PI(4)+PI(3))-PT2
        IF(PN2.LE.DD) THEN
          PN  = 0.0D0
          RNX = 0.0D0
          RNY = 0.0D0
          RNZ = 1.0D0
          RTN = 0.0D0
        ELSE IF(PN2.GT.DD) THEN
          PN  = SQRT(PN2)
          PNI = 1.0D0/PN
          RNX = PI(1)*PNI
          RNY = PI(2)*PNI
          RNZ = PI(3)*PNI
          RTN = PT*PNI
        ENDIF

        EP(1,1) =  RTX*RNZ
        EP(2,1) =  RTY*RNZ
        EP(3,1) = -RTN
        EP(4,1) =  0.0D0
        EW(1) =  1.0D0

        EP(1,2) = -RTY
        EP(2,2) =  RTX
        EP(3,2) =  0.0D0
        EP(4,2) =  0.0D0
        EW(2) =  1.0D0
        NP = 2

c       WRITE(6,*) 'PI(1) PI(2) ',PI(1),PI(2)
c       WRITE(6,*) 'PT PTI RTY  ',PT,PTI,RTY
c       WRITE(6,*) 'EP          ',EP(1,2)

        IF(LP.GE.3) THEN
          IF (APP.GT.DD) THEN
            NP = NP + 1
            RPPI  = 1.0D0/SQRT(APP)
            EP(1,NP) = 0.D0
            EP(2,NP) = 0.D0
            EP(3,NP) = 0.D0
            EP(4,NP) = -RAPP/PN
CX          EP(4,NP) = -1.D0/PN
            EW(NP) = SIGN(1.0D0, VM)
c           WRITE(6,*) ' NP APP EP ',NP,APP,EP(4,NP)
          ENDIF
        ENDIF

        IF(LP.GE.4) THEN
          IF (APP.GT.DD) THEN
            NP = NP + 1
            RPPI  = 1.0D0/SQRT(APP)
            EP(1,NP) = 0.D0
            EP(2,NP) = 0.D0
            EP(3,NP) = 0.D0
            EP(4,NP) = 0.D0
            EW(NP) = 0.D0
            AM2 = AM*AM
         END IF
       END IF

c       WRITE(6,*) 'NP EP  ',NP,EP(1,2)
*     CALL CTIME('SMINTV')
      RETURN
      END
