C ******************************************************************
C *                                                                *
C *  SUBROUTINE POLP(I:I*4, A:R*8 , AM:R*8, P:R*8(4),              *
C *                                       EP:R*8(4), EM:R*8(4))    *
C *                                                                *
C *     Purpose: To set components of polarization vectors         *
C *              for QED vertex with on-shell fermions.            *
C *                                                                *
C *              -                                                 *
C *              U(P2)(EP)U(P1) vertex with P1**2=P2**2=m**2       *
C *                                                                *
C *                                                                *
C *     I= : Polarization states   I=1,2 : Transverse              *
C *                                I=3   : Longitudinal            *
C *                                I=4   : Scalar                  *
C *                                                                *
C *     (*) I=5: EP(P)=P for massless gauge bosons.                *
C *         ( For check of gouge invarience for external           *
C *           massless gouge boson legs.)                          *
C *                                                                *
C * X   A = : Gauge parametor                                      *
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
C *     Use only to QED vertex.  Gauge invarience can not          *
C *     be cheched in this subroutine.                             *
C *                                    written by H. Tanaka        *
C ******************************************************************
C
C
C              ===================================
               SUBROUTINE POLP(I,A,AM,P,EP,EM)
C              ===================================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
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
CX        EM(1) =  PROP
        ELSE IF (I.EQ.2) THEN
          EP(1) = -RTY
          EP(2) =  RTX
          EP(3) =  0.0D0
          EP(4) =  0.0D0
          EM(2) =  1.0D0
CX        EM(2) =  PROP
        ELSE IF (I.EQ.3) THEN
          APP=ABS(PP)
          IF (APP.LE.DD) THEN
            EP(1) = 0.0D0
            EP(2) = 0.0D0
            EP(3) = 0.0D0
            EP(4) = 0.0D0
            EM(3) = 0.0D0
          ELSE
            EP(1) = 0.D0
            EP(2) = 0.D0
            EP(3) = 0.D0
            EP(4) = -SQRT(APP)/PN
            EM(3) = SIGN(1.0D0, PP)
CX          EM(3) = SIGN(1.0D0, PP)*PROP
          ENDIF
        ELSE IF (I.EQ.4) THEN
            EP(1) = 0.D0
            EP(2) = 0.D0
            EP(3) = 0.D0
            EP(4) = 0.D0
            EM(4) = 0.0D0
        ELSE IF (I.EQ.5) THEN
          EP(1) = P(1)
          EP(2) = P(2)
          EP(3) = P(3)
          EP(4) = P(4)
        END IF

        RETURN
        END
