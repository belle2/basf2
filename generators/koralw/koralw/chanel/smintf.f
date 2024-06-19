************************************************************************
      SUBROUTINE SMINTF(AM, PI, VM, EW, PS, CE)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0)
*     DOUBLE PRECISION     AM
      DIMENSION  PI(4), EW(2), PS(4,3)
      DOUBLE COMPLEX CE(2,4)
*
*   Decompose momentum of internal fermion.
*
*            PI = L1 + L2          off-shell momentum
*            L1**2  = 0            massless on-shell
*            L2**2  = AM**2        massive on-shell
*            L2 = L21 + L22
*
*            L21**2 = 0            massless on-shell
*            L22**2 = 0            massless on-shell
*            PS(*, 1) = L21
*            PS(*, 2) = L22
*            PS(*, 3) = L1
*
*     AM : input  : Mass of the fermion
*     PI : input  : Momentum of the fermion (off-shell)
*     EW : output : Weight for L2 and L1
*     PS : output : Decomposed massless momenta
*     CE : output : Phase factor for L2 and L1.
*
*     1994/12/17 : fug fix for pi(j) = 0, j <> 3     T.Kaneko
*     1994/12/20 :                                   Y.Kurihara
*
      DIMENSION PM(4)
*     DATA DD/1.0D-7/
      DATA DD/0.0D0/
*-----------------------------------------------------------------------
      PSN  = SQRT(PI(1)**2 + PI(2)**2 + PI(3)**2)
      IF(ABS(PI(4)).LE.DD) THEN
        IF(PSN.LE.DD) THEN
            P0  = 0.0D0
        ELSE
            A   = 1.0D0/SQRT(3.0D0)
            B   = A
            C   = A
            DEN = PI(4)-A*PI(1)-B*PI(2)-C*PI(3)
            P0  = (VM - AM**2)/(2.0D0*DEN)
        ENDIF
      ELSE 
        IF(PSN.LE.DD) THEN
            A = 1.0D0
            B = 0.0D0
            C = 0.0D0
            DEN = PI(4)-PI(1)
        ELSE IF(PI(4) .LT. 0.0D0) THEN
            PSNI = 1.0D0/PSN
            A = - PI(1)*PSNI
            B = - PI(2)*PSNI
            C = - PI(3)*PSNI
            den=    pi(4)**2
     .-         max(pi(1)**2,pi(2)**2,pi(3)**2)
     .-         max(min(max(pi(1)**2,pi(2)**2),pi(3)**2)
     .,                 min(pi(1)**2,pi(2)**2)          )
     .-         min(pi(1)**2,pi(2)**2,pi(3)**2)
            den=den/(pi(4)-psn)
        ELSE
            PSNI = 1.0D0/PSN
            A = PI(1)*PSNI
            B = PI(2)*PSNI
            C = PI(3)*PSNI
            den=    pi(4)**2
     .-         max(pi(1)**2,pi(2)**2,pi(3)**2)
     .-         max(min(max(pi(1)**2,pi(2)**2),pi(3)**2)
     .,                 min(pi(1)**2,pi(2)**2)          )
     .-         min(pi(1)**2,pi(2)**2,pi(3)**2)
            den=den/(pi(4)+psn)
        ENDIF
c       DEN = PI(4)-A*PI(1)-B*PI(2)-C*PI(3)
        P0  = (VM - AM**2)/(2.0D0*DEN)
      ENDIF

      P10  = ABS(P0)
      IF (P10.LE.0.0D0) THEN
        EW(2) = 0.0D0
      ELSE
        EW(2) = SIGN(1.0D0, P0)
      END IF
      PS(1, 3) = A*P10
      PS(2, 3) = B*P10
      PS(3, 3) = C*P10
      PS(4, 3) =   P10
 
      P00=PI(4)-EW(2)*PS(4, 3)
      P20=ABS(P00)
      IF (P20.LE.0.0D0) THEN
        EW(1) = 0.0D0
      ELSE
        EW(1) = SIGN(1.0D0, P00)
      END IF
 
      PM(1) = EW(1)*(PI(1) - EW(2)*PS(1, 3))
      PM(2) = EW(1)*(PI(2) - EW(2)*PS(2, 3))
      PM(3) = EW(1)*(PI(3) - EW(2)*PS(3, 3))
      PM(4) = EW(1)*(PI(4) - EW(2)*PS(4, 3))
 
      CALL SPLTQ(AM, PM, PS(1,2), PS(1,1))
      CALL PHASEQ(1,      PM, CE(1,1))
      CALL PHASEQ(2,      PM, CE(1,2))
      CALL PHASEQ(1, PS(1,3), CE(1,3))
      CALL PHASEQ(2, PS(1,3), CE(1,4))
 
*     write(*,*) 'pi = ', pi(1),pi(2),pi(3),pi(4),' mass=',am
*     write(*,*) 'p1 = ', ps(1,1),ps(2,1),ps(3,1),ps(4,1),
*    &           ps(4,1)**2-ps(1,1)**2-ps(2,1)**2-ps(3,1)**2
*     write(*,*) 'p2 = ', ps(1,2),ps(2,2),ps(3,2),ps(4,2),
*    &           ps(4,2)**2-ps(1,2)**2-ps(2,2)**2-ps(3,2)**2
*     write(*,*) 'p3 = ', ps(1,3),ps(2,3),ps(3,3),ps(4,3),
*    &           ps(4,3)**2-ps(1,3)**2-ps(2,3)**2-ps(3,3)**2
*     write(*,*) 'pm = ', pm(1),pm(2),pm(3),pm(4),
*    &           pm(4)**2-pm(1)**2-pm(2)**2-pm(3)**2,am**2

*     CALL CTIME('SMINTF')
      RETURN
      END
