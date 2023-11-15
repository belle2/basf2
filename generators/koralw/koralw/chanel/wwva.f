C ********************************************************************
C *                                                                  *
C *          3 W(-) P3 EP3                                           *
C *          !                                                       *
C *          !                                                       *
C *  1 -->---!---<-- 2    SUBROUTINE WWVA                            *
C *  V              W(+)                                             *
C *  P1             P2                                               *
C *  EP1            EP2                                              *
C *                                                                  *
C *     Purpose: to calculate vertex amplitudes for three vector-    *
C *              boson vertex including anomalous couplings.         *
C *                                                                  *
C *     Comment : The momenta of particles with vertices are         *
C *               taken to flow in.                                  *
C *                                                                  *
C *     The constructed amplitude has the following form:            *
C *                                                                  *
C *     AALL=GG*( G1*(   (EP3.(P1-P2))*(EP1.EP2)                     *
C *                    + (EP1.(P2-P3))*(EP2.EP3)                     *
C *                    + (EP2.(P3-P1))*(EP1.EP3) )                   *
C *                                                                  *
C *         + AE*((EP1.EP3)*(EP2.P1) - (EP1.EP2)*(EP3.P1))           *
C *         + AL/WM2*((EP2.EP3)*((EP1.P3)*(P1.P2)-(EP1.P2)*(P1.P3))  *
C *                  +(EP1.EP3)*((EP2.P1)*(P2.P3)-(EP2.P3)*(P1.P2))  *
C *                  +(EP1.EP2)*((EP3.P2)*(P1.P3)-(EP3.P1)*(P2.P3))  *
C *                  +(EP1.P2)*(EP2.P3)*(EP3.P1)                     *
C *                  -(EP1.P3)*(EP2.P1)*(EP3.P2))                    *
C *                                                                  *
C *                                                                  *
C *     GG : coupling constant for the standard model.               *
C *     G1 :                                                         *
C *     AE : G1 - K                                                  *
C *     AL :                                                         *
C *     WM2: W boson mass squared.                                   *
C *     P1,P2,P3=: momenta of the vector bosons.                     *
C *     EP1,EP2,EP3=: polarization vectors of vector bosons.         *
C *     AALL=: calculated results of vertex amplitudes for given     *
C *            plarization states.                                   *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *                                                                  *
C *                                      written by H. Tanaka        *
C ********************************************************************
C
C
C
C        GG : COUPLING FOR STANDARD MODEL
C        G1 :
C        AE : G1 - K
C        AL :
C        WM2: W BOSON MASS SQUARED
C
C        ==========================================================
         SUBROUTINE WWVA(GG,G1,AE,AL,WM2,P1,P2,P3,EP1,EP2,EP3,AALL)
C        ==========================================================
C
         IMPLICIT DOUBLE PRECISION(A-H,O-Z)
!         DIMENSION P1(4),P2(4),P3(3),EP1(4),EP2(4),EP3(4)
! this bug was corrected by M. Skrzypek Jun 8 1986. 
         DIMENSION P1(4),P2(4),P3(4),EP1(4),EP2(4),EP3(4)
         GG0 = 1.D0
         CALL VVV(GG0,P1,P2,P3,EP1,EP2,EP3,AALL0)
 
        E1E2=EP1(4)*EP2(4)-EP1(1)*EP2(1)-EP1(2)*EP2(2)-EP1(3)*EP2(3)
        E1E3=EP1(4)*EP3(4)-EP1(1)*EP3(1)-EP1(2)*EP3(2)-EP1(3)*EP3(3)
        E2E3=EP2(4)*EP3(4)-EP2(1)*EP3(1)-EP2(2)*EP3(2)-EP2(3)*EP3(3)
 
        P1P2=P1(4)*P2(4)-P1(1)*P2(1)-P1(2)*P2(2)-P1(3)*P2(3)
        P1P3=P1(4)*P3(4)-P1(1)*P3(1)-P1(2)*P3(2)-P1(3)*P3(3)
        P2P3=P2(4)*P3(4)-P2(1)*P3(1)-P2(2)*P3(2)-P2(3)*P3(3)
 
        E1P2=EP1(4)*P2(4)-EP1(1)*P2(1)-EP1(2)*P2(2)-EP1(3)*P2(3)
        E1P3=EP1(4)*P3(4)-EP1(1)*P3(1)-EP1(2)*P3(2)-EP1(3)*P3(3)
        E2P3=EP2(4)*P3(4)-EP2(1)*P3(1)-EP2(2)*P3(2)-EP2(3)*P3(3)
 
        E2P1=EP2(4)*P1(4)-EP2(1)*P1(1)-EP2(2)*P1(2)-EP2(3)*P1(3)
        E3P1=EP3(4)*P1(4)-EP3(1)*P1(1)-EP3(2)*P1(2)-EP3(3)*P1(3)
        E3P2=EP3(4)*P2(4)-EP3(1)*P2(1)-EP3(2)*P2(2)-EP3(3)*P2(3)
 
        AALL1 = E1E3*E2P1 - E1E2*E3P1
        AALL2 = E2E3*(E1P3*P1P2 - E1P2*P1P3)
        AALL3 = E1E3*(E2P1*P2P3 - E2P3*P1P2)
        AALL4 = E1E2*(E3P2*P1P3 - E3P1*P2P3)
        AALL5 = E1P2*E2P3*E3P1 - E1P3*E2P1*E3P2
 
        AALL = GG*(G1*AALL0+AE*AALL1
     &       + AL/WM2*(AALL2+AALL3+AALL4+AALL5))
 
        RETURN
        END
C
C ********************************************************************
C *          V4 EP4                                                  *
C *          |                                                       *
C *          |                                                       *
C * W- -->---|---<-- V3   SUBROUTINE WWVVA                           *
C * EP1      |      EP3                                              *
C *          |                                                       *
C *         W+ EP2                                                   *
C *                                                                  *
C *                                                                  *
C *     Purpose: to calculate vertex amplitudes for three vector-    *
C *              boson vertex including anomalous couplings.         *
C *                                                                  *
C *                                                                  *
C *     Comment : the constructed amplitude has the following form:  *
C *                                                                  *
C *    AALL=GG*(EP1.EP3*EP2.EP4+EP1.EP4*EP2.EP3-2.*EP1.EP2*EP3.EP4). *
C *                                                                  *
C *     GG=: coupling constant for vertex.                           *
C *     EP1,EP2,EP3,EP4=: polarization vectors for vector bosons.    *
C *     AALL=: calculated result of vertex amplitude for given       *
C *            plarization states.                                   *
C *                                                                  *
C *                                      written by H. Tanaka        *
C ********************************************************************
C
C
C       ==============================================================
        SUBROUTINE WWVVA(GG,G1,AE,AL,WM2,P1,P2,P3,P4,
     &                                         EP1,EP2,EP3,EP4,AALL)
C       ==============================================================
C
C
        IMPLICIT DOUBLE PRECISION(A-H,O-Z)
        DIMENSION EP1(4),EP2(4),EP3(4),EP4(4)
        DIMENSION P1(4),P2(4),P3(4),P4(4) ,PS(4)
 
         PS(1) = P1(1)+P2(1)
         PS(2) = P1(2)+P2(2)
         PS(3) = P1(3)+P2(3)
         PS(4) = P1(4)+P2(4)
 
         GG0 = 1.D0
         CALL VVVV(GG0,EP1,EP2,EP3,EP4,AALL0)
CX       WRITE(6,*) ' GG G1 AE AL ',GG,G1,AE,AL
CX       WRITE(6,*) ' AALL0 ',AALL0
 
        E1E2=EP1(4)*EP2(4)-EP1(1)*EP2(1)-EP1(2)*EP2(2)-EP1(3)*EP2(3)
        E3E4=EP3(4)*EP4(4)-EP3(1)*EP4(1)-EP3(2)*EP4(2)-EP3(3)*EP4(3)
        E1E3=EP1(4)*EP3(4)-EP1(1)*EP3(1)-EP1(2)*EP3(2)-EP1(3)*EP3(3)
        E2E4=EP2(4)*EP4(4)-EP2(1)*EP4(1)-EP2(2)*EP4(2)-EP2(3)*EP4(3)
        E1E4=EP1(4)*EP4(4)-EP1(1)*EP4(1)-EP1(2)*EP4(2)-EP1(3)*EP4(3)
        E2E3=EP2(4)*EP3(4)-EP2(1)*EP3(1)-EP2(2)*EP3(2)-EP2(3)*EP3(3)
 
        P1P2=P1(4)*P2(4)-P1(1)*P2(1)-P1(2)*P2(2)-P1(3)*P2(3)
        P3P4=P3(4)*P4(4)-P3(1)*P4(1)-P3(2)*P4(2)-P3(3)*P4(3)
        P1P3=P1(4)*P3(4)-P1(1)*P3(1)-P1(2)*P3(2)-P1(3)*P3(3)
        P2P4=P2(4)*P4(4)-P2(1)*P4(1)-P2(2)*P4(2)-P2(3)*P4(3)
        P1P4=P1(4)*P4(4)-P1(1)*P4(1)-P1(2)*P4(2)-P1(3)*P4(3)
        P2P3=P2(4)*P3(4)-P2(1)*P3(1)-P2(2)*P3(2)-P2(3)*P3(3)
        PSPS=PS(4)*PS(4)-PS(1)*PS(1)-PS(2)*PS(2)-PS(3)*PS(3)
 
        E1P2=EP1(4)*P2(4)-EP1(1)*P2(1)-EP1(2)*P2(2)-EP1(3)*P2(3)
        E3P4=EP3(4)*P4(4)-EP3(1)*P4(1)-EP3(2)*P4(2)-EP3(3)*P4(3)
        E1P3=EP1(4)*P3(4)-EP1(1)*P3(1)-EP1(2)*P3(2)-EP1(3)*P3(3)
        E2P4=EP2(4)*P4(4)-EP2(1)*P4(1)-EP2(2)*P4(2)-EP2(3)*P4(3)
        E1P4=EP1(4)*P4(4)-EP1(1)*P4(1)-EP1(2)*P4(2)-EP1(3)*P4(3)
        E2P3=EP2(4)*P3(4)-EP2(1)*P3(1)-EP2(2)*P3(2)-EP2(3)*P3(3)
        E1PS=EP1(4)*PS(4)-EP1(1)*PS(1)-EP1(2)*PS(2)-EP1(3)*PS(3)
        E2PS=EP2(4)*PS(4)-EP2(1)*PS(1)-EP2(2)*PS(2)-EP2(3)*PS(3)
        E3PS=EP3(4)*PS(4)-EP3(1)*PS(1)-EP3(2)*PS(2)-EP3(3)*PS(3)
        E4PS=EP4(4)*PS(4)-EP4(1)*PS(1)-EP4(2)*PS(2)-EP4(3)*PS(3)
 
        E2P1=EP2(4)*P1(4)-EP2(1)*P1(1)-EP2(2)*P1(2)-EP2(3)*P1(3)
        E4P3=EP4(4)*P3(4)-EP4(1)*P3(1)-EP4(2)*P3(2)-EP4(3)*P3(3)
        E3P1=EP3(4)*P1(4)-EP3(1)*P1(1)-EP3(2)*P1(2)-EP3(3)*P1(3)
        E4P2=EP4(4)*P2(4)-EP4(1)*P2(1)-EP4(2)*P2(2)-EP4(3)*P2(3)
        E4P1=EP4(4)*P1(4)-EP4(1)*P1(1)-EP4(2)*P1(2)-EP4(3)*P1(3)
        E3P2=EP3(4)*P2(4)-EP3(1)*P2(1)-EP3(2)*P2(2)-EP3(3)*P2(3)
 
        AALL1 = E3E4*PSPS
        AALL2 = E1E3*E2E4*(P2P3+P1P4)+E1E4*E2E3*(P1P3+P2P4)
        AALL3 = E1E2*(E4P3*E3PS+E3P4*E4PS)
        AALL4 = E3E4*(E2P1*E1PS+E1P2*E2PS)
        AALL5 = E2E4*(E1P3*E3P2+E1P4*E3P1-E1P4*E3P2+E3P4*E1P2)
        AALL6 = E1E4*(-E2P4*E3P1+E3P4*E2P1+E2P3*E3P1+E2P4*E3P2)
        AALL7 = E2E3*(-E4P2*E1P3+E1P2*E4P3+E4P1*E1P3+E4P2*E1P4)
        AALL8 = E1E3*(-E4P1*E2P3+E2P1*E4P3+E2P3*E4P2+E2P4*E4P1)
 
CX       WRITE(6,*) ' AALL0 ',AALL0
        AALL = GG*( G1*AALL0
     &              + AL/WM2*(AALL1+AALL2+AALL3
CX   &              - AL/WM2*(AALL1+AALL2+AALL3 ???
     &                          - AALL4-AALL5-AALL6-AALL7-AALL8))
CX       WRITE(6,*) ' AALL  ',AALL
        RETURN
        END
