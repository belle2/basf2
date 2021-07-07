
CDECK  ID>, ATRAN7. 
*.
*...ATRAN7   interface to RAND routine.
*.
*. INPUT     : NUM    number of elements in ARRAY to fill
*. OUTPUT    : ARRAY  array filled with random numbers
*.
*. CALLS     : RAND
*. CALLED    : RNDGEN
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
C Interface to RAND routine
C
      SUBROUTINE ATRAN7(ARRAY,NUM)
      REAL ARRAY(1)
      INTEGER IARRAY(1)

      DO 1 I=1,NUM
 1    ARRAY(I)=RAND(IX)
      RETURN

      ENTRY ITRAN7(IARRAY,NUM)
      DO 2 I=1,NUM
      DUM=RAND(IX)
 2    IARRAY(I)=IX
      RETURN

      ENTRY TRAN7A(IY)
      IX=IY
      RETURN
      END
