C
C ********************************************************************
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *   SUBROUTINE FFV(L:I*4, II:I*4, I:I*4, AAM:R*8, AM:R*8,          *
C *  &               AL:R*8, AR:R*8, CC:C*16(2), C:C*16(2),          *
C *  &               Q1:R*8(4), Q2:R*8(4), P1:R*8(4), P2:R*8(4),     *
C *  &               Q:R*8(4), AALL:C*16(4,2,2))                     *
C *                                                                  *
C *     Purpose: To calculate vertex amplitudes for vector boson-    *
C *              massive fermions vertex.                            *
C *                                                                  *
C *     L=: Polarization state of vector boson.                      *
C *     I,II=: Indices to specify fermion(I,II=3) or antifermion     *
C *            (I,II=1) state.                                       *
C *     AM,AAM=: Masses of fermions.                                 *
C *     AL,AR=: Coupling constants for vertex.                       *
C *     C,CC=: Phase factors for massive fermions.                   *
C *     P1,P2,Q1,Q2=: Light-like vectors decomposed by subroutine    *
C *                   SPLTQ.                                         *
C *     Q=: Polarization vector of vector boson.                     *
C *     AALL=: Calculated results of vertex amplitudes for all       *
C *            possible helicity states.                             *
C *                                                                  *
C *                                                                  *
C *                                      written by H. Tanaka        *
C *                                                                  *
C *    Q1, Q2 : FOR UB OR VB                                         *
C *    P1, P2 : FOR U OR V                                           *
C *                                                                  *
C *                                                                  *
C ********************************************************************
C
C
C       ===========================================================
        SUBROUTINE FFV(L,II,I,AAM,AM,AL,AR,CC,C,Q1,Q2,P1,P2,Q,AALL)
C       ===========================================================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        DOUBLE COMPLEX AALL
        DOUBLE COMPLEX C,CC,AALL11,AALL22,AALL12,AALL21
        DIMENSION Q(4),P1(4),P2(4),Q1(4),Q2(4),C(2),CC(2)
        DIMENSION AALL(4,2,2),AALL11(2),AALL22(2),AALL12(2),AALL21(2)
 
        CALL FFV0(Q1,P1,Q,AALL11)
        CALL FFV0(Q2,P2,Q,AALL22)
        CALL FFV0(Q1,P2,Q,AALL12)
        CALL FFV0(Q2,P1,Q,AALL21)
        R  = DFLOAT(I  - 2)
        RR = DFLOAT(II - 2)
        IF (AM.LE.0.0D0) THEN
          R = 0.0D0
        ENDIF
        IF (AAM.LE.0.0D0) THEN
          RR = 0.0D0
        ENDIF
 
        J1 = (5-II)/2
        J2 = 3 - J1
        K1 = (5-I)/2
        K2 = 3 - K1
 
        AALL(L,J1,K1)=(AL*AALL11(1))
     &               +(AR*R*RR*C(2)*CC(2)*AALL22(2))
        AALL(L,J2,K2)=(AR*AALL11(2))
     &               +(AL*R*RR*C(1)*CC(1)*AALL22(1))
        AALL(L,J1,K2)=-(AR*RR*CC(2)*AALL21(2))
     &                -(AL*R*C(1)*AALL12(1))
        AALL(L,J2,K1)=-(AL*RR*CC(1)*AALL21(1))
     &                -(AR*R*C(2)*AALL12(2))
 
 
        RETURN
      END
