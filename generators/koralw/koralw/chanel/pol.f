************************************************************************
C
C      ************************************************************
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *             ============================                 *
C      *                CHANEL   (Version. 1.1)                   *
C      *             ============================                 *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                   Written by H. Tanaka                   *
C      *                                                          *
C      *                                                          *
C      *                        ---------                         *
C      *                        Reference                         *
C      *                        ---------                         *
C      *    ' Numerical Calculation of Helicity Amplitudes        *
C      *      for Processes Involving Massive Fermions'           *
C      *                                                          *
C      *      H. Tanaka, Hiroshima University preprint            *
C      *         Accepted to Comput. Phys. Commun.                *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      *                                                          *
C      ************************************************************
C
C
C
C
C
C ******************************************************************
C *                                                                *
C *  SUBROUTINE POL(I:I*4, AM:R*8, P:R*8(4), EP:R*8(4), EM:R*8(4)) *
C *                                                                *
C *     Purpose: To set components of polarization vectors.        *
C *                                                                *
C *     I= : Polarization states   I=1,2 : Transverse              *
C *                                I=3   : Longitudinal            *
C *                                I=4   : Scalar                  *
C *                                                                *
C *     AM= : Mass of vector boson                                 *
C *     P = : Momentum of vector boson                             *
C *     EP= : Polarization vector for state I                      *
C *     EM= : Weight factors                                       *
C *                                                                *
C *                                                                *
C *                                                                *
C *     Note                                                       *
C *     ----                                                       *
C *                                                                *
C *     For summation of the polarization vectors I=1 to 4,        *
C *     following gauges are chosen :                              *
C *                                                                *
C *     Unitary gauge : for massive vector boson                   *
C *     Feynman gauge : for massless vector boson                  *
C *                                                                *
C *                                                                *
C *                                    written by H. Tanaka        *
C ******************************************************************
C
C
C              ============================
               SUBROUTINE POL(I,AM,P,EP,EM)
C              ============================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        PARAMETER (ONE = 1.0)
        DIMENSION P(4),EP(4),EM(4)
        DATA DD/0.D0/
 
        PT2 = P(1)**2 + P(2)**2
        PP  = (P(4) - P(3))*(P(4) + P(3)) - PT2
        IF (PT2.LE.DD) THEN
          PT  = 0.0D0
          RTY = 1.0D0
          RTX = 0.0D0
        ELSE
          PT  = SQRT(PT2)
          PTI = 1.0D0/PT
          RTY = P(2)*PTI
          RTX = P(1)*PTI
        END IF
        PN2 = PT2 + P(3)**2
        IF (PN2.LE.DD) THEN
          PN  = 0.0D0
          RNX = 0.0D0
          RNY = 0.0D0
          RNZ = 1.0D0
          RTN = 0.0D0
        ELSE IF (PN2.GT.DD) THEN
          PN  = SQRT(PN2)
          PNI = 1.0D0/PN
          RNX = P(1)*PNI
          RNY = P(2)*PNI
          RNZ = P(3)*PNI
          RTN = PT*PNI
        END IF
        IF (I.EQ.1) THEN
          EP(1) =  RTX*RNZ
          EP(2) =  RTY*RNZ
          EP(3) = -RTN
          EP(4) =  0.0D0
          EM(1) =  1.0D0
        ELSE IF (I.EQ.2) THEN
          EP(1) = -RTY
          EP(2) =  RTX
          EP(3) =  0.0D0
          EP(4) =  0.0D0
          EM(2) =  1.0D0
        ELSE IF (I.EQ.3) THEN
          APP=ABS(PP)
          IF (APP.LE.DD) THEN
            EP(1) = 0.0D0
            EP(2) = 0.0D0
            EP(3) = 0.0D0
            EP(4) = 0.0D0
            EM(3) = 0.0D0
          ELSE
            RPPI  = 1.0D0/SQRT(APP)
            EP(1) = RNX*P(4)*RPPI
            EP(2) = RNY*P(4)*RPPI
            EP(3) = RNZ*P(4)*RPPI
            EP(4) = PN*RPPI
            EM(3) = SIGN(1.0D0, PP)
          ENDIF
        ELSE IF (I.EQ.4) THEN
          APP = ABS(PP)
          IF (APP.LE.DD) THEN
            EP(1) = 0.D0
            EP(2) = 0.D0
            EP(3) = 0.D0
            EP(4) = 0.D0
            EM(4) = 0.0D0
          ELSE
            RPPI  = 1.0D0/SQRT(APP)
            EP(1) = P(1)*RPPI
            EP(2) = P(2)*RPPI
            EP(3) = P(3)*RPPI
            EP(4) = P(4)*RPPI
            IF (AM.GT.0.D0) THEN
              EM(4)=  SIGN(ONE, PP)*(PP/AM**2-1.0D0)
            ELSE
              EM(4)= -SIGN(ONE, PP)
            END IF
          ENDIF
        END IF
        RETURN
      END
