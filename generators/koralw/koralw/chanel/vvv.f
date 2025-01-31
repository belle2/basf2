C
C ********************************************************************
C *          3                                                       *
C *          |                                                       *
C *          |                                                       *
C *  1 -->---|---<-- 2    SUBROUTINE VVV                             *
C *                                                                  *
C *     Purpose: To calculate vertex amplitudes for three vector-    *
C *              boson vertex.                                       *
C *                                                                  *
C *     GG=: Coupling constant for vertex.                           *
C *     L1,L2,L3=: Polarization state of vector boson.               *
C *     P1,P2,P3=: Momenta for the vector bosons.                    *
C *     EP1,EP2,EP3=: Polarization vectors for vector bosons.        *
C *     AALL=: Calculated results of vertex amplitudes for given     *
C *            plarization states.                                   *
C *                                                                  *
C *                                                                  *
C *     Comment : The momenta of particles with vertices are         *
C *               taken to flow in.                                  *
C *                                                                  *
C *                                                                  *
C *                                      written by H. Tanaka        *
C ********************************************************************
C
C
C       ===================================================
CXXX    SUBROUTINE VVV(GG,L1,L2,L3,P1,P2,P3,EP1,EP2,EP3,AALL)
        SUBROUTINE VVV(GG,P1,P2,P3,EP1,EP2,EP3,AALL)
C       ===================================================
C
C
CX      IMPLICIT REAL*16(A-H,O-Z)
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
CXX     DOUBLE PRECISION GG,P1,P2,P3,EP1,EP2,EP3,AALL
CXX     DOUBLE COMPLEX AALL
        DIMENSION P1(4),P2(4),P3(4),EP1(4),EP2(4),EP3(4)
        E3P1P2=EP3(4)*(P1(4)-P2(4))-EP3(1)*(P1(1)-P2(1))
     *        -EP3(2)*(P1(2)-P2(2))-EP3(3)*(P1(3)-P2(3))
        E1P2P3=EP1(4)*(P2(4)-P3(4))-EP1(1)*(P2(1)-P3(1))
     *        -EP1(2)*(P2(2)-P3(2))-EP1(3)*(P2(3)-P3(3))
        E2P3P1=EP2(4)*(P3(4)-P1(4))-EP2(1)*(P3(1)-P1(1))
     *        -EP2(2)*(P3(2)-P1(2))-EP2(3)*(P3(3)-P1(3))
        E1E2=EP1(4)*EP2(4)-EP1(1)*EP2(1)-EP1(2)*EP2(2)-EP1(3)*EP2(3)
        E2E3=EP2(4)*EP3(4)-EP2(1)*EP3(1)-EP2(2)*EP3(2)-EP2(3)*EP3(3)
        E1E3=EP1(4)*EP3(4)-EP1(1)*EP3(1)-EP1(2)*EP3(2)-EP1(3)*EP3(3)
        VVVV=GG*(E3P1P2*E1E2+E1P2P3*E2E3+E2P3P1*E1E3)
        AALL=VVVV
CXX     AALL=DCMPLX(AALL,0.D0)
        RETURN
      END
