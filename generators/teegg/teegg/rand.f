
CDECK  ID>, RAND.   
*.
*...RAND     the RAN7 random number generator
*.
*. INPUT     : IX     seed
*.
*. CALLED    : ATRAN7
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.1
*. CREATED   : 29-Jun-87
*. LAST MOD  : 29-Jun-87
*.
*. Modification Log.
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C This is the code for the RAN7 random number generator:
C
C Reference : P.A.W. Lewis, A.S. Goodman, and J.M. Miller,
C             "Pseudo-Random Number Generator for the System/360",
C             IBM Syst. J. 8,2 (1969) 136-146
C
                  FUNCTION RAND(IX)
C PORTABLE RANDOM NUMBER GENERATOR USING MULTIPLICATIVE
C CONGRUENTIAL METHOD, MULTIPLIER=16807, MODULUS=2**31-1.
C NOTE - IX MUST NOT BE CHANGED BETWEEN CALLS.
C
C SOME COMPILERS, E.G. THE HP3000, REQUIRE THE FOLLOWING
C DECLARATION TO BE INTEGER*4
      INTEGER A,P,IX,B15,B16,XHI,XALO,LEFTLO,FHI,K
C
C CONSTANTS   7**5,     2**15,     2**16,      2**31-1
      DATA A/16807/,B15/32768/,B16/65536/,P/2147483647/
C
C GET 15 HIGH-ORDER BITS OF IX
      XHI = IX / B16
C GET 16 LOW-ORDER BITS OF IX AND FORM LOW-ORDER PRODUCT
      XALO = (IX - XHI * B16) * A
C GET 15 HIGH-ORDER BITS OF LOW-ORDER PRODUCT
      LEFTLO = XALO / B16
C FORM THE 31 HIGHEST BITS OF THE FULL PRODUCT
      FHI = XHI * A + LEFTLO
C GET OVERFLOW PAST 31-ST BIT OF FULL PRODUCT
      K = FHI / B15
C ASSEMBLE ALL PARTS, PRESUBTRACT P (THE PARENS ARE ESSENTIAL)
      IX = (((XALO-LEFTLO*B16)-P) + (FHI-K*B15) * B16) + K
C ADD P BACK IN IF NECESSARY
      IF (IX .LT. 0) IX = IX + P
C MULTIPLY BY 1/(2**31-1)
      RAND = FLOAT(IX) * 4.656612875E-10
      RETURN
      END
