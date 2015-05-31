
CDECK  ID>, ZMART.  

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C    CALCULATION OF ALL THE Z FUNCTIONS FOR GAMMA EXCHANGE
C
C                               C.MANA & M.MARTINEZ   DESY-86
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      FUNCTION ZMART(P1,L1,P2,L2,P3,L3,P4,L4)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      COMPLEX*16 ZMART,SP,SM
      INTEGER P1,P2,P3,P4,P5,P6
      DIMENSION SP(6,6),SM(6,6),D(6,6),E(6),U(6)
      COMMON / PRODUX / SP,SM,U,E,D
      LZ=9-4*L1-2*L2-L3-(L4+1)/2
      GOTO(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16),LZ
    1 ZMART= -2.D0*( SP(P1,P3)*SM(P2,P4)
     .        - U(P1)*U(P2)*E(P3)*E(P4)
     .        - U(P3)*U(P4)*E(P1)*E(P2) )
      GOTO 17
    2 ZMART= -2.D0*U(P2)*(    SP(P1,P3)*E(P4)
     .                 - SP(P1,P4)*E(P3) )
      GOTO 17
    3 ZMART= -2.D0*U(P1)*(    SM(P2,P3)*E(P4)
     .                 - SM(P2,P4)*E(P3) )
      GOTO 17
    4 ZMART= -2.D0*( SP(P1,P4)*SM(P2,P3)
     .        - U(P1)*U(P2)*E(P3)*E(P4)
     .        - U(P3)*U(P4)*E(P1)*E(P2) )
      GOTO 17
    5 ZMART= -2.D0*U(P4)*(    SP(P3,P1)*E(P2)
     .                 - SP(P3,P2)*E(P1) )
      GOTO 17
    6 ZMART=(0.D0,0.D0)
      GOTO 17
    7 ZMART=  2.D0*( E(P2)*U(P1) - E(P1)*U(P2) )
     .       *( E(P4)*U(P3) - E(P3)*U(P4) )
      GOTO 17
    8 ZMART=  2.D0*U(P3)*(    SP(P1,P4)*E(P2)
     .                 - SP(P2,P4)*E(P1) )
      GOTO 17
    9 ZMART=  2.D0*U(P3)*(    SM(P1,P4)*E(P2)
     .                 - SM(P2,P4)*E(P1) )
      GOTO 17
   10 ZMART=  2.D0*( E(P2)*U(P1) - E(P1)*U(P2) )
     .       *( E(P4)*U(P3) - E(P3)*U(P4) )
      GOTO 17
   11 ZMART=(0.D0,0.D0)
      GOTO 17
   12 ZMART=  2.D0*U(P4)*(    SM(P1,P3)*E(P2)
     .                 - SM(P2,P3)*E(P1) )
      GOTO 17
   13 ZMART= -2.D0*( SP(P2,P3)*SM(P1,P4)
     .        - U(P1)*U(P2)*E(P3)*E(P4)
     .        - U(P3)*U(P4)*E(P1)*E(P2) )
      GOTO 17
   14 ZMART= -2.D0*U(P1)*(    SP(P2,P3)*E(P4)
     .                 - SP(P2,P4)*E(P3) )
      GOTO 17
   15 ZMART= -2.D0*U(P2)*(    SM(P1,P3)*E(P4)
     .                 - SM(P1,P4)*E(P3) )
      GOTO 17
   16 ZMART= -2.D0*( SP(P2,P4)*SM(P1,P3)
     .        - U(P1)*U(P2)*E(P3)*E(P4)
     .        - U(P3)*U(P4)*E(P1)*E(P2) )
   17 CONTINUE
      RETURN
      END
