
CDECK  ID>, FQPEG.  
*.
*...FQPEG    calculates the maximum energy of the `missing' electron.
*.
*.  FQPEG is a double precision function that returns the minimum
*.  Eb-qp0, given the detector acceptance, for the e-gamma configuration
*.  A value of -1 is returned if the configuration is kinematically
*.  impossible.
*.
*. INPUT     : TEMIN  minimum theta of observed electron
*. INPUT     : TGMIN  minimum theta of observed photon
*. INPUT     : EEMIN  minimum energy of observed electron
*. INPUT     : EGMIN  minimum energy of observed photon
*. INPUT     : D      maximum theta of missing electron
*. INPUT     : EB     beam energy
*.
*. CALLED    : TEEGGL
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
C DOUBLE PRECISION FUNCTION FQPEG:
C Returns the minimum Eb-qp0, given the detector acceptance (e-gamma).
C Returns -1 if configuration is kinematically impossible.
C-----------------------------------------------------------------------

      DOUBLE PRECISION FUNCTION FQPEG(TEMIN,TGMIN,EEMIN,EGMIN,D,EB)
      IMPLICIT NONE

      DOUBLE PRECISION TEMIN,TGMIN,EEMIN,EGMIN,D,EB
      DOUBLE PRECISION SQRTS,DELT,FQPTST,EETST,EGTST,SINED,SINGD,CON1
      DOUBLE PRECISION SIGND(2)/1.D0,-1.D0/
      INTEGER   I,J

      SQRTS=2.*EB
      FQPEG=2.*EB

C Go through each endpoint given by two of (TEMIN,TGMIN,EEMIN,EGMIN)
C with TEVETO= DELT and -DELT

C 1) TEMIN,TGMIN

      DO 1 I=1,2
         DELT=D*SIGND(I)
         FQPTST = EB*TAN((TEMIN+DELT)/2.D0)*TAN((TGMIN-DELT)/2.D0)
         EETST = (EB+FQPTST)*SIN(TGMIN-DELT)/
     >           ( SIN(TEMIN+DELT)+SIN(TGMIN-DELT) )
         EGTST = (EB+FQPTST-EETST)
         IF(EETST.GE.EEMIN .AND. EGTST.GE.EGMIN .AND.
     >      FQPTST.GE.0.D0 )FQPEG=MIN(FQPEG,FQPTST)
 1    CONTINUE

C 2) EGMIN,TGMIN

      DO 2 I=1,2
         DELT=D*SIGND(I)
         FQPTST =  EB * EGMIN * SIN( (TGMIN-DELT)/2.D0 )**2/
     >            (EB - EGMIN * COS( (TGMIN-DELT)/2.D0 )**2)
         EETST = EB+FQPTST-EGMIN
         SINED = EGMIN/EETST * SIN(TGMIN-DELT)
         IF(EETST.GE.EEMIN .AND. SINED .GE. SIN(TEMIN+DELT) .AND.
     >      FQPTST.GE.0.D0 .AND. SINED .LE. 1.D0)FQPEG=MIN(FQPEG,FQPTST)
 2    CONTINUE


C 3) EEMIN,TEMIN

      DO 3 I=1,2
         DELT=D*SIGND(I)
         FQPTST =  EB * EEMIN * SIN( (TEMIN+DELT)/2.D0 )**2/
     >            (EB - EEMIN * COS( (TEMIN+DELT)/2.D0 )**2)
         EGTST = EB+FQPTST-EEMIN
         SINGD = EEMIN/EGTST * SIN(TEMIN+DELT)
         IF(EGTST.GE.EGMIN .AND. SINGD .GE. SIN(TGMIN-DELT) .AND.
     >      FQPTST.GE.0.D0 .AND. SINGD .LE. 1.D0)FQPEG=MIN(FQPEG,FQPTST)
 3    CONTINUE

C 4) EGMIN,TEMIN

      DO 4 I=1,2
         DELT=D*SIGND(I)
         CON1= (EGMIN/2.D0)**2 - EB*(EB-EGMIN)*TAN((TEMIN+DELT)/2.D0)**2
         IF(CON1.GE.0.D0)THEN
            DO 41 J=1,2
               FQPTST = EGMIN/2.D0 + SIGND(J)*SQRT(CON1)
               EETST=EB+FQPTST-EGMIN
               SINGD = EETST/EGMIN * SIN (TEMIN+DELT)
               IF(EETST.GE.EEMIN .AND. SINGD .GE. SIN(TGMIN-DELT) .AND.
     >           FQPTST.GE.0.D0  .AND. SINGD .LE. 1.D0
     >           )FQPEG=MIN(FQPEG,FQPTST)
 41         CONTINUE
         ENDIF
 4    CONTINUE

C 5) EEMIN,TGMIN

      DO 5 I=1,2
         DELT=D*SIGND(I)
         CON1= (EEMIN/2.D0)**2 - EB*(EB-EEMIN)*TAN((TGMIN-DELT)/2.D0)**2
         IF(CON1.GE.0.D0)THEN
            DO 51 J=1,2
               FQPTST = EEMIN/2.D0 + SIGND(J)*SQRT(CON1)
               EGTST=EB+FQPTST-EEMIN
               SINED = EGTST/EEMIN * SIN (TGMIN-DELT)
               IF(EGTST.GE.EGMIN .AND. SINED .GE. SIN(TEMIN+DELT) .AND.
     >           FQPTST.GE.0.D0  .AND. SINED .LE. 1.D0
     >           )FQPEG=MIN(FQPEG,FQPTST)
 51         CONTINUE
         ENDIF
 5    CONTINUE

C 6) EEMIN,EGMIN

      DO 6 I=1,2
         DELT=D*SIGND(I)
         FQPTST = EEMIN + EGMIN - EB
         SINED = (EGMIN**2-(EB-FQPTST-EEMIN)**2)/4.D0/(EB-FQPTST)/EEMIN
         SINGD = (EEMIN**2-(EB-FQPTST-EGMIN)**2)/4.D0/(EB-FQPTST)/EGMIN
         IF(SINED.GE.SIN((TEMIN+DELT)/2.D0)**2 .AND. SINED.LE. 1.D0.AND.
     >      SINGD.GE.SIN((TGMIN-DELT)/2.D0)**2 .AND. SINGD.LE. 1.D0
     >     )FQPEG=MIN(FQPEG,FQPTST)
 6    CONTINUE

      IF(FQPEG.GT.EB)FQPEG=-1.D0

      RETURN
      END
