C
C ********************************************************************
C *          3                                                       *
C *          S                                                       *
C *          |                                                       *
C *          |                                                       *
C *  V -->---|---<-- V    SUBROUTINE VVS                             *
C *  1               2                                               *
C *                                                                  *
C *                                                                  *
C *     Purpose: To calculate vertex amplitudes for vector boson-    *
C *              scalor boson vertex.                                *
C *                                                                  *
C *     GG=: Coupling constant for vertex.                           *
C *     L1,L2=: Polarization state of vector boson.                  *
C *     EP1,EP2=: Polarization vectors for vector bosons.            *
C *     AALL=: Calculated results of vertex amplitudes for given     *
C *            plarization states.                                   *
C *                                                                  *
C *                                                                  *
C *     Comment : The momenta of particles with vertices are         *
C *               taken to flow in.                                  *
C *                                                                  *
C *      (*) This subroutine can be also used for VVSS vertex.       *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *                                      written by H. Tanaka        *
C ********************************************************************
C
C
C            =====================================
CXX          SUBROUTINE VVS(GG,L1,L2,EP1,EP2,AALL)
             SUBROUTINE VVS(GG,EP1,EP2,AALL)
C            =====================================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
CXX     DOUBLE COMPLEX AALL
        DIMENSION EP1(4),EP2(4)
        E1E2=EP1(4)*EP2(4)-EP1(1)*EP2(1)-EP1(2)*EP2(2)-EP1(3)*EP2(3)
        AALL=GG*E1E2
CXX     AALL=DCMPLX(AALL,0.D0)
        RETURN
      END
