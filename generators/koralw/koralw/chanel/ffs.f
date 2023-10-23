C
C ********************************************************************
C *         S                                                        *
C *         |                                                        *
C *         |                                                        *
C * F1--<---|---<---F2    SUBROUTINE FFS                             *
C *                                                                  *
C *     Purpose: To calculate vertex amplitudes for scalor boson-    *
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
C *     AALL=: Calculated results of vertex amplitudes for all       *
C *            possible helicity states.                             *
C *                                                                  *
C *                                                                  *
C *                                      written by H. Tanaka        *
C ********************************************************************
C
C
C       ===========================================================
        SUBROUTINE FFS(II,I,AAM,AM,AL,AR,CC,C,Q1,Q2,P1,P2,AALL)
C       ===========================================================
C
C
C
C   FFS              CALCULATION OF FFS   MASSIVE CASE
C
C   I,II=1:V    I,II=3:U
C      J,JJ=1:HS=-   J,JJ=2:HS=+
C
CDD     SUBROUTINE FFS(L,II,I,AAM,AM,AL,AR,PP,P,AALL)
CXX     SUBROUTINE FFS(L,II,I,AAM,AM,AL,AR,CC,C,Q1,Q2,P1,P2,AALL)
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        DOUBLE COMPLEX AALL
        DOUBLE COMPLEX C,CC,AALL11,AALL22,AALL12,AALL21
        DIMENSION P1(4),P2(4),Q1(4),Q2(4),C(2),CC(2)
        DIMENSION AALL(2,2),AALL11(2),AALL22(2),AALL12(2),AALL21(2)
        CALL FFS0(Q1,P1,AALL11)
        CALL FFS0(Q2,P2,AALL22)
        CALL FFS0(Q1,P2,AALL12)
        CALL FFS0(Q2,P1,AALL21)
 
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
 
        AALL(J1,K2)=(AR*AALL11(1))
     &               +(AL*R*RR*C(1)*CC(2)*AALL22(2))
        AALL(J2,K1)=(AL*AALL11(2))
     &               +(AR*R*RR*C(2)*CC(1)*AALL22(1))
        AALL(J2,K2)=-(AR*RR*CC(1)*AALL21(1))
     &                -(AL*R*C(1)*AALL12(2))
        AALL(J1,K1)=-(AL*RR*CC(2)*AALL21(2))
     &                -(AR*R*C(2)*AALL12(1))
 
 
CX      R=DFLOAT(I)-2.D0
CX      RR=DFLOAT(II)-2.D0
CX      IF (AM.LE.0.D0) R=0.D0
CX      IF (AAM.LE.0.D0) RR=0.D0
CX      IF (AAM.LE.0.D0) RR=0.D0
CX      IF (AM.LE.0.D0) R=0.D0
CX      IF (II.EQ.3.AND.I.EQ.3) THEN
CX      AALL(1,2)=AR*AALL11(1)+AL*R*RR*C(1)*CC(2)*AALL22(2)
CX      AALL(2,1)=AL*AALL11(2)+AR*R*RR*C(2)*CC(1)*AALL22(1)
CX      AALL(2,2)=-AR*RR*CC(1)*AALL21(1)-AL*R*C(1)*AALL12(2)
CX      AALL(1,1)=-AL*RR*CC(2)*AALL21(2)-AR*R*C(2)*AALL12(1)
CX      ELSE IF (II.EQ.3.AND.I.EQ.1) THEN
CX      AALL(1,1)=AR*AALL11(1)+AL*R*RR*C(1)*CC(2)*AALL22(2)
CX      AALL(2,2)=AL*AALL11(2)+AR*R*RR*C(2)*CC(1)*AALL22(1)
CX      AALL(2,1)=-AR*RR*CC(1)*AALL21(1)-AL*R*C(1)*AALL12(2)
CX      AALL(1,2)=-AL*RR*CC(2)*AALL21(2)-AR*R*C(2)*AALL12(1)
CX      ELSE IF (II.EQ.1.AND.I.EQ.3) THEN
CX      AALL(2,2)=AR*AALL11(1)+AL*R*RR*C(1)*CC(2)*AALL22(2)
CX      AALL(1,1)=AL*AALL11(2)+AR*R*RR*C(2)*CC(1)*AALL22(1)
CX      AALL(1,2)=-AR*RR*CC(1)*AALL21(1)-AL*R*C(1)*AALL12(2)
CX      AALL(2,1)=-AL*RR*CC(2)*AALL21(2)-AR*R*C(2)*AALL12(1)
CX      ELSE IF (II.EQ.1.AND.I.EQ.1) THEN
CX      AALL(2,1)=AR*AALL11(1)+AL*R*RR*C(1)*CC(2)*AALL22(2)
CX      AALL(1,2)=AL*AALL11(2)+AR*R*RR*C(2)*CC(1)*AALL22(1)
CX      AALL(1,1)=-AR*RR*CC(1)*AALL21(1)-AL*R*C(1)*AALL12(2)
CX      AALL(2,2)=-AL*RR*CC(2)*AALL21(2)-AR*R*C(2)*AALL12(1)
CX      END IF
C       WRITE(6,100) Q1(4),P1(4),Q(4)
CX
CX      WRITE(6,*) 'II,I,AAM,AM',II,I,AAM,AM
C100    FORMAT(2X,3D15.5)
 
        RETURN
      END
