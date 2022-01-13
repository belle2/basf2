************************************************************************
      SUBROUTINE SMINTV(LP, AM, PI, EP, EW, VM, IGAUG)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0, ANOGAU=200.0D0)
      INTEGER LP
*     DOUBLE PRECISION    AM, VM
      DIMENSION PI(4), EP(4,LP), EW(LP)
*
*   Caluculate polarization vectors for vector boson.
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
 
      IF(LP.EQ.1) THEN
        DO 30 I = 1, 4
          EP(I,1) = PI(I)
   30   CONTINUE
        EW(1) = 1.0D0
        NP    = 1
 
*     AXIAL GAUGE
      ELSE IF(IGAUG.EQ.-1) THEN
        VNK = PI(4) - PI(1)
        IF (VNK.NE.0.D0) THEN
          I = 1
          J = 3
        ELSE
          VNK = PI(4) - PI(3)
          I = 3
          J = 1
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
        NP      = 2
        IF(LP.GT.2) THEN
          IF(AKK.GT.0) THEN
            SIGNK = 1.0
          ELSE
            SIGNK =-1.0
          ENDIF
          AKK   = SQRT(ABS(AKK))
          EP(4,3) = VM/VNK
          EP(I,3) = VM/VNK
          EP(2,3) = 0
          EP(J,3) = 0
          EW(3)   = SIGNK
          NP      = 4
        ENDIF
 
*     COVARIANT GAUGE
      ELSE
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
        NP    =  2
 
        APP=ABS(VM)
        IF(LP.GE.3) THEN
          IF(APP.GT.DD) THEN
            NP = NP + 1
            RPPI  = 1.0D0/SQRT(APP)
            EP(1,NP) = RNX*PI(4)*RPPI
            EP(2,NP) = RNY*PI(4)*RPPI
            EP(3,NP) = RNZ*PI(4)*RPPI
            EP(4,NP) = PN*RPPI
            EW(NP) = SIGN(1.0D0, VM)
          ENDIF
        ENDIF
 
        IF(LP.GE.4) THEN
          IF (APP.GT.DD) THEN
            NP = NP + 1
            RPPI  = 1.0D0/SQRT(APP)
            EP(1,NP) = PI(1)*RPPI
            EP(2,NP) = PI(2)*RPPI
            EP(3,NP) = PI(3)*RPPI
            EP(4,NP) = PI(4)*RPPI
            AM2 = AM*AM
 
*           COVARIANT GAUGE
            IF(IGAUG.NE.0) THEN
              A = AGAUGE(IGAUG)
              AVM = VM - A*AM2
              IF (ABS(AVM).GT.DD) THEN
                EW(NP) =  SIGN(1.0D0, VM)*(AM2-VM)*A/AVM
              ELSE
                EW(NP) = -SIGN(1.0D0, VM)
                WRITE(6,*) ' Caution : p*p = a*m**2 '
                WRITE(6,*) '  a = 1 was taken. '
              END IF
 
*           UNITARY GAUGE
            ELSE
              IF (AM.GT.0.D0) THEN
                EW(NP) =   SIGN(1.0D0, VM)*(VM/AM**2-1.0D0)
              ELSE
                EW(NP) = -SIGN(1.0D0, VM)*ANOGAU
              END IF
            END IF
          ENDIF
        END IF
      END IF
 
*     CALL CTIME('SMINTV')
      RETURN
      END
