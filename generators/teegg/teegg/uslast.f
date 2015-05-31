
CDECK  ID>, USLAST. 
*.
*...USLAST   GOPAL interface for TEEGG - print routine
*.
*. CALLS     : TEEGGC TEEGGP
*. CALLED    : UGLAST
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 1.0
*. CREATED   : 28-Sep-88
*. LAST MOD  : 28-Sep-88
*.
*. Modification Log.
*.
*.**********************************************************************
      SUBROUTINE USLAST
      CALL TEEGGC
      CALL TEEGGP(6)
      RETURN
      END
