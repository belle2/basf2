
CDECK  ID>, ELEMAT. 

C ----------------------------------------------------------------------
C MARTINEZ/MIQUEL SUBROUTINES:
C The routines below were provided by Martinez & Miquel.
C Reference: preprint UAB-LFAE 87-01 (Barcelona)
C ----------------------------------------------------------------------

C The following external references have had their names changed to
C protect the innocent (more obscure names so that conflicts are
C less likely).

C Original subprogram name        New name
C ------------------------        --------
C ELEMAT                           (same)
C AMTOT                            AMTOTM
C AMPLI                            AMPLIM
C Z                                ZMART
C SPININ                           (same)

C Original common block name      New name
C --------------------------      --------
C PRODUX                           (same)
C CONST                            MCONST
C QED                              MARQED
C LEPT1                            MLEPT1
C MOMENZ                           (same)

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C    THIS ROUTINE CALCULATES THE MATRIX ELEMENT SQUARED
C    FOR THE PROCESS
C    E-(P1) + E+(P2) ----> E-(P3) + E+(P4) + G(P5) + G(P6)
C    THE OUTPUT CONTAINS ALL FACTORS RELATED WITH MAT. ELEMENT
C    BUT NOT CONVERSION TO PB.
C
C    (THIS HELICITY AMPLITUDES METHOD IS DESCRIBED, FOR INSTANCE,
C     IN DESY 86-062 AND 86-114 REPORTS)
C
C                          M.MARTINEZ & R.MIQUEL   BARCELONA-87
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      SUBROUTINE ELEMAT(WT)
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DIMENSION NCONF1(6,16),NCONF2(6,16),NCONF3(6,16),NCONF4(6,16)
      DATA NCONF1/
     . 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1,-1,
     . 1, 1, 1, 1,-1,-1,   1, 1, 1, 1,-1, 1,
     . 1, 1,-1,-1, 1, 1,   1, 1,-1,-1, 1,-1,
     . 1, 1,-1,-1,-1,-1,   1, 1,-1,-1,-1, 1,
     .-1,-1,-1,-1, 1, 1,  -1,-1,-1,-1, 1,-1,
     .-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1, 1,
     .-1,-1, 1, 1, 1, 1,  -1,-1, 1, 1, 1,-1,
     .-1,-1, 1, 1,-1,-1,  -1,-1, 1, 1,-1, 1/
      DATA NCONF2/
     . 1, 1, 1,-1, 1, 1,   1, 1, 1,-1, 1,-1,
     . 1, 1, 1,-1,-1,-1,   1, 1, 1,-1,-1, 1,
     . 1, 1,-1, 1, 1, 1,   1, 1,-1, 1, 1,-1,
     . 1, 1,-1, 1,-1,-1,   1, 1,-1, 1,-1, 1,
     .-1,-1,-1, 1, 1, 1,  -1,-1,-1, 1, 1,-1,
     .-1,-1,-1, 1,-1,-1,  -1,-1,-1, 1,-1, 1,
     .-1,-1, 1,-1, 1, 1,  -1,-1, 1,-1, 1,-1,
     .-1,-1, 1,-1,-1,-1,  -1,-1, 1,-1,-1, 1/
      DATA NCONF3/
     . 1,-1, 1, 1, 1, 1,   1,-1, 1, 1, 1,-1,
     . 1,-1, 1, 1,-1,-1,   1,-1, 1, 1,-1, 1,
     . 1,-1,-1,-1, 1, 1,   1,-1,-1,-1, 1,-1,
     . 1,-1,-1,-1,-1,-1,   1,-1,-1,-1,-1, 1,
     .-1, 1,-1,-1, 1, 1,  -1, 1,-1,-1, 1,-1,
     .-1, 1,-1,-1,-1,-1,  -1, 1,-1,-1,-1, 1,
     .-1, 1, 1, 1, 1, 1,  -1, 1, 1, 1, 1,-1,
     .-1, 1, 1, 1,-1,-1,  -1, 1, 1, 1,-1, 1/
      DATA NCONF4/
     . 1,-1, 1,-1, 1, 1,   1,-1, 1,-1, 1,-1,
     . 1,-1, 1,-1,-1,-1,   1,-1, 1,-1,-1, 1,
     . 1,-1,-1, 1, 1, 1,   1,-1,-1, 1, 1,-1,
     . 1,-1,-1, 1,-1,-1,   1,-1,-1, 1,-1, 1,
     .-1, 1,-1, 1, 1, 1,  -1, 1,-1, 1, 1,-1,
     .-1, 1,-1, 1,-1,-1,  -1, 1,-1, 1,-1, 1,
     .-1, 1, 1,-1, 1, 1,  -1, 1, 1,-1, 1,-1,
     .-1, 1, 1,-1,-1,-1,  -1, 1, 1,-1,-1, 1/
C
      NSPIN = 16
      CALL SPININ(0)
C
      WT = 0.D0
      DO 100 I=1,NSPIN
      PCONF = AMTOTM(NCONF1(1,I))
      WT  = WT + PCONF
 100  CONTINUE
      DO 200 I=1,NSPIN
      PCONF = AMTOTM(NCONF2(1,I))
      WT  = WT + PCONF
 200  CONTINUE
      DO 300 I=1,NSPIN
      PCONF = AMTOTM(NCONF3(1,I))
      WT  = WT + PCONF
 300  CONTINUE
      DO 400 I=1,NSPIN
      PCONF = AMTOTM(NCONF4(1,I))
      WT  = WT + PCONF
 400  CONTINUE
C
C   FACTOR 8 STANDS FOR AVERAGE OVER INITIAL POLS AND PHOT SYMM FACTOR
      WT = WT/8.D0
      RETURN
      END
