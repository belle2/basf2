************************************************************************
      SUBROUTINE SMFFS(L2,L1,EW2,EW1,AM2,AM1,CPL,CE2,CE1,
     &                 PS2,PS1,LT,AV)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      PARAMETER (ZERO = 0.0D0, ONE = 1.0D0)
*   * dummy array size.
      PARAMETER (LTSIZE = 20, LASIZE = 1024)
      INTEGER    L1, L2
      DIMENSION  EW1(1), EW2(1)
*     DOUBLE PRECISION     AM1, AM2
      DOUBLE COMPLEX CPL(2)
      DOUBLE COMPLEX CE1(2,L1), CE2(2,L2)
      DIMENSION  PS1(4,3), PS2(4,3)
*     DIMENSION  PS1(4,L1/2+1), PS2(4,L2/2+1)
      DOUBLE COMPLEX AV(0:LASIZE)
*     DOUBLE COMPLEX AV(0:L2*L1-1)
      INTEGER    LT(0:LTSIZE)
*
*    Calculate fermion-fermion-scalar vertex.
*
*           ! 3                 _      _
*           !                1  u  or  v
*      -->--+-->---          2  u  or  v
*        2     1             3  scalar boson
*
*     L1, L2   : input  : If external then 2 else 4, for fermion
*     EW1, EW2 : input  : If >=0 then particle else anti-particle
*     AM1, AM2 : input  : masses of the fermions
*     CPL      : input  : coupling constants (L, R)
*     CE1, CE2 : input  : phase factor, calculated by SMINTF or SMEXTF
*     PS1, PS2 : input  : decomposed momenta (L21, L22 , L1) or
*                         (L21, L22) calculated by SMINTF or SMEXTF
*     AV       : output : table of amplitudes
*     LT       : output : table of sizes in AV
*
      DOUBLE COMPLEX AVT(2,2,2,2), CPN
      DIMENSION  P0(4), CPR(2)
      DATA P0/4*ZERO/, AM0/ZERO/
*-----------------------------------------------------------------------
      IF(CPL(1).NE.ZERO) THEN
        CPN = CPL(1)
        CPR(1) = ONE
        CPR(2) = CPL(2)/CPL(1)
      ELSE IF(CPL(2).NE.ZERO) THEN
        CPN = CPL(2)
        CPR(2) = ONE
        CPR(1) = CPL(1)/CPL(2)
      ELSE
CC      WRITE(6,*) '*** SMFFS:COUPLING CONSTANT FOR FFS VERTEX IS 0.'
CC      WRITE(6,*) 'L2=',L2,' L1=',L1,' EW2=',EW2,' EW1=',EW1
CC      WRITE(6,*) 'AM2=',AM2,' AM1=',AM1,' CPL=',CPL
CC      FOR FACOM
CC      CALL SDFDMP(1, 0)
CC      STOP
        CPN = ZERO
        CPR(1) = ONE
        CPR(2) = ONE
      ENDIF
      IF(EW1(1).GE.ZERO) THEN
        K1 = 3
      ELSE
        K1 = 1
      ENDIF
      IF(EW2(1).GE.ZERO) THEN
        K2 = 3
      ELSE
        K2 = 1
      ENDIF
      LT(0) = 3
      LT(1) = L2
      LT(2) = L1
      LT(3) = 1
      CALL FFS(K1,K2,AM1,AM2,CPR(1),CPR(2),CE1(1,2),CE2(1,1),
     &         PS1(1,1),PS1(1,2),PS2(1,1),PS2(1,2), AVT(1,1,1,1))
      IF(L1.EQ.4) THEN
          CALL FFS( 3,K2,AM0,AM2,CPR(1),CPR(2),CE1(1,4),CE2(1,1),
     &             PS1(1,3),P0,      PS2(1,1),PS2(1,2), AVT(1,1,2,1))
      ENDIF
      IF(L2.EQ.4) THEN
          CALL FFS(K1, 3,AM1,AM0,CPR(1),CPR(2),CE1(1,2),CE2(1,3),
     &             PS1(1,1),PS1(1,2),PS2(1,3),P0,       AVT(1,1,1,2))
  300   CONTINUE
      ENDIF
      IF(L1.EQ.4 .AND. L2.EQ.4) THEN
          CALL FFS( 3, 3,AM0,AM0,CPR(1),CPR(2),CE1(1,4),CE2(1,3),
     &             PS1(1,3),P0,      PS2(1,3),P0,       AVT(1,1,2,2))
  400   CONTINUE
      ENDIF
      IA = 0
      DO 500 IP1 = 1, L1/2
      DO 500 IL1 = 1, 2
      DO 500 IP2 = 1, L2/2
      DO 500 IL2 = 1, 2
        AV(IA) = CPN*AVT(IL1, IL2, IP1, IP2)
        IA = IA + 1
  500 CONTINUE
*     CALL CTIME('SMFFS ')
      RETURN
      END
