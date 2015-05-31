
CDECK  ID>, TBOORT. 
*.
*...TBOORT   boost 4-momentum from gamma-electron center of mass to lab
*.
*. INPUT     : IN     4-momentum in gamma-electron center of mass
*. INPUT     : SINTR  sin(theta rotation between systems)
*. INPUT     : COSTR  cos(theta rotation between systems)
*. INPUT     : GAMA   gamma between two systems
*. INPUT     : BETA   beta between two systems
*. INPUT     : SINQP  sin(theta of g-e system wrt lab system)
*. INPUT     : COSQP1 1 - cos(theta of g-e system wrt lab system)
*. INPUT     : PHIP   pi-phi of g-e system
*. OUTPUT    : OUT    4-momentum in lab system
*.
*. CALLED    : T4BODY
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.1
*. CREATED   : 29-Jun-87
*. LAST MOD  : 28-Sep-88
*.
*. Modification Log.
*. 28-Sep-88   Dean Karlen   Change to conform to OPAL standards.
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C SUBROUTINE TBOORT: Boosts 4-momentum from g-e center of mass to lab.
C-----------------------------------------------------------------------

      SUBROUTINE TBOORT(IN,SINTR,COSTR,GAMA,BETA,SINQP,COSQP1,PHIP,OUT)
      IMPLICIT NONE

      DOUBLE PRECISION
     >       IN(4),SINTR,COSTR,GAMA,BETA,SINQP,COSQP1,PHIP,OUT(4)
      DOUBLE PRECISION
     >       INROT(4),ROTBOO(4),CONST,COSQP

      INROT(1)=IN(1)*COSTR - IN(3)*SINTR
      INROT(2)=IN(2)
      INROT(3)=IN(3)*COSTR + IN(1)*SINTR
      INROT(4)=IN(4)

      COSQP = 1.D0-COSQP1
      CONST = (GAMA-1.D0)*INROT(1)*SINQP + (GAMA-1.D0)*INROT(3)*COSQP
     >        - GAMA*BETA*INROT(4)

      ROTBOO(1) = INROT(1) + CONST*SINQP
      ROTBOO(2) = INROT(2)
      ROTBOO(3) = INROT(3) + CONST*COSQP
      ROTBOO(4) = GAMA * (INROT(4) - BETA*INROT(1)*SINQP
     >                              -BETA*INROT(3)*COSQP)

      OUT(1) = ROTBOO(1)*COS(PHIP) - ROTBOO(2)*SIN(PHIP)
      OUT(2) = ROTBOO(2)*COS(PHIP) + ROTBOO(1)*SIN(PHIP)
      OUT(3) = ROTBOO(3)
      OUT(4) = ROTBOO(4)

      RETURN
      END
