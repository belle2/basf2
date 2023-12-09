C ********************************************************************
C *          3                                                       *
C *          V                                                       *
C *          |                                                       *
C *          |                                                       *
C *  S --<---|---<-- S    SUBROUTINE SSV                             *
C *  1               2                                               *
C *                                                                  *
C *                                                                  *
C *     Purpose: To calculate vertex amplitudes for vector boson     *
C *              coupled to scalor boson line.                       *
C *                                                                  *
C *     GG=: Coupling constant for vertex.                           *
C *     L=: Polarization state of vector boson.                      *
C *     EP=: Polarization vectors for vector bosons.                 *
C *     P1,P2=: Momenta for scalor bosons.                           *
C *     AALL=: Calculated results of vertex amplitudes for given     *
C *            plarization states.                                   *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *                                      written by H. Tanaka        *
C ********************************************************************
C
C
C            =====================================
CX           SUBROUTINE SSV(GG,L,P1,P2,EP,AALL)
             SUBROUTINE SSV(GG,P1,P2,EP,AALL)
C            =====================================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
CXX     DOUBLE COMPLEX AALL
        DIMENSION EP(4),P1(4),P2(4)
        EP1P2=EP(4)*(P1(4)-P2(4))-EP(1)*(P1(1)-P2(1))
     *       -EP(2)*(P1(2)-P2(2))-EP(3)*(P1(3)-P2(3))
        AALL=GG*EP1P2
CXX     AALL=DCMPLX(AALL,0.D0)
        RETURN
      END
