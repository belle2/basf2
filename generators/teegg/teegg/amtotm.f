
CDECK  ID>, AMTOTM. 

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C    THIS FUNCTION ADDS THE CONTRIBUTION OF ALL THE POLARIZATION
C    CONFIGURATIONS BY THE ADEQUATE PERMUTATIONS OF THE ONE
C    CALCULATED IN AMPLI
C
C                           M.MARTINEZ & R.MIQUEL  BARCELONA-87
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      FUNCTION AMTOTM(L)
      IMPLICIT DOUBLE PRECISION(A-H,M,O-Z)
      COMPLEX*16 AMT,AMPLIM,SP,SM
      DIMENSION SP(6,6),SM(6,6),D(6,6),E(6),U(6),B(6)
      DIMENSION L(6)
      COMMON / PRODUX / SP,SM,U,E,D
      COMMON /MCONST/ PI,DR,SR2,PB
C
C  SR2 IS THE SQUARED ROOT OF 2
      COMMON /MARQED/ALF,ALF2,ALDPI
      DATA INIT/0/
      IF(INIT.NE.0)GO TO 100
      INIT=1
C
C  OVERALL FACTOR
      AMFAC = (4.D0*PI*ALF)**4
100   CONTINUE
C
      AMT =    AMPLIM (1,L(1),2,L(2),3,L(3),4,L(4),5, L(5),6, L(6),1)
     .+        AMPLIM (4,L(4),3,L(3),2,L(2),1,L(1),6, L(6),5, L(5),1)
     .+ DCONJG(AMPLIM (3,L(3),4,L(4),1,L(1),2,L(2),6,-L(6),5,-L(5),1)
     .    +    AMPLIM (2,L(2),1,L(1),4,L(4),3,L(3),5,-L(5),6,-L(6),1))
     .+        AMPLIM (1,L(1),2,L(2),3,L(3),4,L(4),5, L(5),6, L(6),2)
     .+        AMPLIM (4,L(4),3,L(3),2,L(2),1,L(1),6, L(6),5, L(5),2)
     .+        AMPLIM (1,L(1),2,L(2),3,L(3),4,L(4),5, L(5),6, L(6),3)
     .+ DCONJG(AMPLIM (3,L(3),4,L(4),1,L(1),2,L(2),5,-L(5),6,-L(6),3))
     .+        AMPLIM (1,L(1),2,L(2),3,L(3),4,L(4),5, L(5),6, L(6),4)
     .+ DCONJG(AMPLIM (3,L(3),4,L(4),1,L(1),2,L(2),5,-L(5),6,-L(6),4))
C
      AMT =    AMT
     .+        AMPLIM (1,L(1),2,L(2),3,L(3),4,L(4),6, L(6),5, L(5),1)
     .+        AMPLIM (4,L(4),3,L(3),2,L(2),1,L(1),5, L(5),6, L(6),1)
     .+ DCONJG(AMPLIM (3,L(3),4,L(4),1,L(1),2,L(2),5,-L(5),6,-L(6),1)
     .    +    AMPLIM (2,L(2),1,L(1),4,L(4),3,L(3),6,-L(6),5,-L(5),1))
     .+        AMPLIM (1,L(1),2,L(2),3,L(3),4,L(4),6, L(6),5, L(5),2)
     .+        AMPLIM (4,L(4),3,L(3),2,L(2),1,L(1),5, L(5),6, L(6),2)
     .+        AMPLIM (1,L(1),2,L(2),3,L(3),4,L(4),6, L(6),5, L(5),3)
     .+ DCONJG(AMPLIM (3,L(3),4,L(4),1,L(1),2,L(2),6,-L(6),5,-L(5),3))
     .+        AMPLIM (1,L(1),2,L(2),3,L(3),4,L(4),6, L(6),5, L(5),4)
     .+ DCONJG(AMPLIM (3,L(3),4,L(4),1,L(1),2,L(2),6,-L(6),5,-L(5),4))
C
      AMTOTM =  AMFAC*AMT*DCONJG(AMT)
      RETURN
      END
