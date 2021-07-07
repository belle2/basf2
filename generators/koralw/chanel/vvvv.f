C
C ********************************************************************
C *          V4                                                      *
C *          |                                                       *
C *          |                                                       *
C * V1 -->---|---<-- V3   SUBROUTINE VVVV                            *
C *          |                                                       *
C *          |                                                       *
C *         V2                                                       *
C *                                                                  *
C *     V1,V2=: Charged gauge bosons ( W+ OR W- ).                   *
C *     V3,V4=: Neutral gauge bosons ( photon or Z0 ) or             *
C *             charged gouge bosons with oposite charges to         *
C *             V1 and V2.                                           *
C *                                                                  *
C *     Purpose: To calculate vertex amplitudes for four vector-     *
C *              boson vertex.                                       *
C *                                                                  *
C *     GG=: Coupling constant for vertex.                           *
C *     L1,L2,L3,L4=: Polarization state of vector boson.            *
C *     P1,P2,P3,P4=: Momenta for the vector bosons.                 *
C *     EP1,EP2,EP3,EP4=: Polarization vectors for vector bosons.    *
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
C       ==============================================================
CXXX    SUBROUTINE VVVV(GG,L1,L2,L3,L4,
CXXX *             P1,P2,P3,P4,EP1,EP2,EP3,EP4,AALL)
        SUBROUTINE VVVV(GG,EP1,EP2,EP3,EP4,AALL)
C       ==============================================================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
CXX     DOUBLE COMPLEX AALL
CXX     DIMENSION P1(4),P2(4),P3(4),P4(4),EP1(4),EP2(4),EP3(4),EP4(4)
        DIMENSION EP1(4),EP2(4),EP3(4),EP4(4)
        E1E2=EP1(4)*EP2(4)-EP1(1)*EP2(1)-EP1(2)*EP2(2)-EP1(3)*EP2(3)
        E3E4=EP3(4)*EP4(4)-EP3(1)*EP4(1)-EP3(2)*EP4(2)-EP3(3)*EP4(3)
        E1E3=EP1(4)*EP3(4)-EP1(1)*EP3(1)-EP1(2)*EP3(2)-EP1(3)*EP3(3)
        E2E4=EP2(4)*EP4(4)-EP2(1)*EP4(1)-EP2(2)*EP4(2)-EP2(3)*EP4(3)
        E1E4=EP1(4)*EP4(4)-EP1(1)*EP4(1)-EP1(2)*EP4(2)-EP1(3)*EP4(3)
        E2E3=EP2(4)*EP3(4)-EP2(1)*EP3(1)-EP2(2)*EP3(2)-EP2(3)*EP3(3)
        AALL=GG*(E1E3*E2E4+E1E4*E2E3-2.D0*E1E2*E3E4)
CXX     AALL=DCMPLX(AALL,0.D0)
        RETURN
      END
