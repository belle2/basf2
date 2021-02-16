#define JAKER  JAKER_BBB
#define DEKAY  DEKAY_BBB
#define DEKAY1 DEKAY1_BBB
#define DEKAY2 DEKAY2_BBB
#define DEXAY  DEXAY_BBB
#define DEXAY1 DEXAY1_BBB
#define DAMPRY DAMPRY_BBB
#define DADNEW DADNEW_BBB
#define DAM4PI DAM4PI_BBB
#define DPH4PI DPH4PI_BBB
#define DPH5PI DPH5PI_BBB
#define DPHNPI DPHNPI_BBB
#define DPHTRE DPHTRE_BBB
#define DWLNEW DWLNEW_BBB
#define TAURDF TAURDF_BBB
#define TAUBRA TAUBRA_BBB
#define TAUDCD TAUDCD_BBB
#define TAUBMC TAUBMC_BBB
#define DADMEL DADMEL_BBB
#define DADMMU DADMMU_BBB      
      SUBROUTINE JAKER(JAK)
C     *********************
C
C **********************************************************************
C                                                                      *
C           ********TAUOLA LIBRARY: VERSION 3.1 *********              *
C           ************** Sep     2016******************              *
C           **      AUTHORS: S.JADACH, Z.WAS        *****              *
C           **  R. DECKER, M. JEZABEK, J.H.KUEHN,   *****              *
C           ********AVAILABLE FROM: WASM AT CERNVM ******              *
C           *******PUBLISHED IN COMP. PHYS. COMM.********              *
C           *** PREPRINT CERN-TH-5856 SEPTEMBER 1990 ****              *
C           *** PREPRINT CERN-TH-6195 OCTOBER   1991 ****              *
C           *** PREPRINT CERN-TH-6793 NOVEMBER  1992 ****              *
C           ********** IFJ-PAN-IV-2016-24 ***************              *
C **********************************************************************
C 
C ----------------------------------------------------------------------
c SUBROUTINE JAKER,
C CHOOSES DECAY MODE ACCORDING TO LIST OF BRANCHING RATIOS
C JAK=1 ELECTRON MODE
C JAK=2 MUON MODE
C JAK=3+ nPI  MODE
C
C     called by : DEXAY
C ----------------------------------------------------------------------
      include 'TAUDCD_BBB_size.inc'
      REAL*8            GAMPRT,SUM,CUMUL(NBMC)
      COMMON / TAUBRA / GAMPRT(NBMC),JLIST(NBMC),NCHAN
      REAL              RRR(1)
C
      IF(NCHAN.LE.0.OR.NCHAN.GT.NBMC) GOTO 902
      CALL KKMC_RANMAR(RRR,1)
      SUM=0
      DO 20 I=1,NCHAN
      SUM=SUM+GAMPRT(I)
  20  CUMUL(I)=SUM
      DO 25 I=NCHAN,1,-1
      IF(RRR(1).LT.CUMUL(I)/CUMUL(NCHAN)) JI=I
  25  CONTINUE
      JAK=JLIST(JI)
      RETURN
 902  PRINT 9020
 9020 FORMAT(' ----- JAKER: WRONG NCHAN')
      STOP
      END
      SUBROUTINE JAKEREQ(ISKIP,JAK)
      include 'TAUDCD_BBB_size.inc'
      INTEGER ISKIP,JAK
      INTEGER KSUM,ACTIVE
      COMMON /KSUM/ KSUM,ACTIVE(NBMC)
      INTEGER ICOUNT,ANSWER
      DATA ICOUNT/0/
      SAVE ICOUNT,ANSWER
      IF (ISKIP.EQ.0) THEN
         JAK=ANSWER
      ELSE
         ICOUNT=ICOUNT+1
         JAK=ACTIVE(MOD(ICOUNT-1,KSUM)+1)
C        PRINT *, 'ICOUNT, JAK, KSUM = ',ICOUNT, JAK, KSUM
         ANSWER=JAK
      ENDIF
      RETURN
      END
      SUBROUTINE DEKAY(KTO,HX)
C     ***********************
C THIS DEKAY IS IN SPIRIT OF THE 'DECAY' WHICH
C WAS INCLUDED IN KORAL-B PROGRAM, COMP. PHYS. COMMUN.
C VOL. 36 (1985) 191, SEE COMMENTS  ON GENERAL PHILOSOPHY THERE.
C KTO=0 INITIALISATION (OBLIGATORY)
C KTO=1,11 DENOTES TAU+ AND KTO=2,12 TAU-
C DEKAY(1,H) AND DEKAY(2,H) IS CALLED INTERNALLY BY MC GENERATOR.
C H DENOTES THE POLARIMETRIC VECTOR, USED BY THE HOST PROGRAM FOR
C CALCULATION OF THE SPIN WEIGHT.
C USER MAY OPTIONALLY CALL DEKAY(11,H) DEKAY(12,H) IN ORDER
C TO TRANSFORM DECAY PRODUCTS TO CMS AND WRITE LUND RECORD IN /LUJETS/.
C KTO=100, PRINT FINAL REPORT  (OPTIONAL).
C DECAY MODES:
C JAK=1 ELECTRON DECAY
C JAK=2 MU  DECAY

C JAK=0 INCLUSIVE:  JAK=1,2,3,4,5,6,7,8,...

      include 'TAUDCD_BBB_size.inc'
      REAL  H(4)
      REAL*8 HX(4)
      COMMON / JAKI   /  JAK1,JAK2,JAKP,JAKM,KTOM

      COMMON / IDFC  / IDF

      COMMON /TAUPOS/ NP1,NP2                
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER
      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
      COMMON / INOUT / INUT,IOUT
      REAL  PDUM1(4),PDUM2(4),PDUM3(4),PDUM4(4),PDUM5(4),HDUM(4)
      REAL  PDUMX(4,9)
      DATA IWARM/0/
      KTOM=KTO

      IF(KTO.EQ.-1) THEN
C     ==================
C       INITIALISATION OR REINITIALISATION
C       first or second tau positions in HEPEVT as in KORALB/Z
        NP1=3
        NP2=4
        KTOM=1
        IF (IWARM.EQ.1) X=5/(IWARM-1)
        IWARM=1
        WRITE(IOUT,7001) JAK1,JAK2
        NEVTOT=0
        NEV1=0
        NEV2=0
        IF(JAK1.NE.-1.OR.JAK2.NE.-1) THEN
          CALL DADMEL(-1,IDUM,HDUM,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
          CALL DADMMU(-1,IDUM,HDUM,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
          CALL DADNEW(-1,IDUM,HDUM,PDUM1,PDUM2,PDUMX,JDUM)
        ENDIF
        DO 21 I=1,NBMC
        NEVDEC(I)=0
        GAMPMC(I)=0
 21     GAMPER(I)=0
      ELSEIF(KTO.EQ.1) THEN
C     =====================
C DECAY OF TAU+ IN THE TAU REST FRAME
        NEVTOT=NEVTOT+1
        IF(IWARM.EQ.0) GOTO 902
        ISGN= IDF/IABS(IDF)

C AJWMOD to change BRs depending on sign:
        CALL TAURDF(KTO)

        CALL DEKAY1(0,H,ISGN)
      ELSEIF(KTO.EQ.2) THEN
C     =================================
C DECAY OF TAU- IN THE TAU REST FRAME
        NEVTOT=NEVTOT+1
        IF(IWARM.EQ.0) GOTO 902
        ISGN=-IDF/IABS(IDF)

C AJWMOD to change BRs depending on sign:
        CALL TAURDF(KTO)

        CALL DEKAY2(0,H,ISGN)
      ELSEIF(KTO.EQ.11) THEN
C     ======================
C REST OF DECAY PROCEDURE FOR ACCEPTED TAU+ DECAY
        NEV1=NEV1+1
        ISGN= IDF/IABS(IDF)
        CALL DEKAY1(1,H,ISGN)
      ELSEIF(KTO.EQ.12) THEN
C     ======================
C REST OF DECAY PROCEDURE FOR ACCEPTED TAU- DECAY
        NEV2=NEV2+1
        ISGN=-IDF/IABS(IDF)
        CALL DEKAY2(1,H,ISGN)
      ELSEIF(KTO.EQ.100) THEN
C     =======================
        IF(JAK1.NE.-1.OR.JAK2.NE.-1) THEN
          CALL DADMEL( 1,IDUM,HDUM,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
          CALL DADMMU( 1,IDUM,HDUM,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
          CALL DADNEW( 1,IDUM,HDUM,PDUM1,PDUM2,PDUMX,JDUM)
          WRITE(IOUT,7010) NEV1,NEV2,NEVTOT
          WRITE(IOUT,7011) (I,NEVDEC(I),GAMPMC(I),GAMPER(I),I= 1,NLT)
          WRITE(IOUT,7012) 
     $         (I,NEVDEC(I),GAMPMC(I),GAMPER(I),NAMES(I-NLT),I=NLT+1,NLT+NMODE)
          WRITE(IOUT,7013) 
        ENDIF
      ELSE
C     ====
        GOTO 910
      ENDIF
C     =====
        DO 78 K=1,4
 78     HX(K)=H(K)
      RETURN
 7001 FORMAT(///1X,15(5H*****)

     $ /,' *',     25X,'*****TAUOLA LIBRARY: VERSION 3.1 ******',9X,1H*,
     $ /,' *',     25X,'***********Sep      2016***************',9X,1H*,
     $ /,' *',     25X,'**AUTHORS: S.JADACH, Z.WAS*************',9X,1H*,
     $ /,' *',     25X,'**R. DECKER, M. JEZABEK, J.H.KUEHN*****',9X,1H*,
     $ /,' *',     25X,'**AVAILABLE FROM: WASM AT CERNVM ******',9X,1H*,
     $ /,' *',     25X,'***** PUBLISHED IN COMP. PHYS. COMM.***',9X,1H*,
     $ /,' *',     25X,'*******CERN TH-6793 NOVEMBER  1992*****',9X,1H*,
     $ /,' *',     25X,'**5 or more pi dec.: precision limited ',9X,1H*,
     $ /,' *',     25X,'******* IFJ-PAN-IV-2016-24 ************',9X,1H*,

     $ /,' *',     25X,'****DEKAY ROUTINE: INITIALIZATION******',9X,1H*,
     $ /,' *',I20  ,5X,'JAK1   = DECAY MODE TAU+               ',9X,1H*,
     $ /,' *',I20  ,5X,'JAK2   = DECAY MODE TAU-               ',9X,1H*,
     $  /,1X,15(5H*****)/)
 7010 FORMAT(///1X,15(5H*****)

     $ /,' *',     25X,'*****TAUOLA LIBRARY: VERSION 3.1 ******',9X,1H*,
     $ /,' *',     25X,'***********Sep      2016***************',9X,1H*,

     $ /,' *',     25X,'**AUTHORS: S.JADACH, Z.WAS*************',9X,1H*,
     $ /,' *',     25X,'**R. DECKER, M. JEZABEK, J.H.KUEHN*****',9X,1H*,
     $ /,' *',     25X,'**AVAILABLE FROM: WASM AT CERNVM ******',9X,1H*,
     $ /,' *',     25X,'***** PUBLISHED IN COMP. PHYS. COMM.***',9X,1H*,
     $ /,' *',     25X,'*******CERN-TH-5856 SEPTEMBER 1990*****',9X,1H*,
     $ /,' *',     25X,'*******CERN-TH-6195 SEPTEMBER 1991*****',9X,1H*,
     $ /,' *',     25X,'*******CERN TH-6793 NOVEMBER  1992*****',9X,1H*,
     $ /,' *',     25X,'******* IFJ-PAN-IV-2016-24 ************',9X,1H*,
     $ /,' *',     25X,'*****DEKAY ROUTINE: FINAL REPORT*******',9X,1H*,
     $ /,' *',I20  ,5X,'NEV1   = NO. OF TAU+ DECS. ACCEPTED    ',9X,1H*,
     $ /,' *',I20  ,5X,'NEV2   = NO. OF TAU- DECS. ACCEPTED    ',9X,1H*,
     $ /,' *',I20  ,5X,'NEVTOT = SUM                           ',9X,1H*,
     $ /,' *','NCHAN    NOEVTS ',
     $   ' PART.WIDTH     ERROR       ROUTINE    DECAY MODE    ',4X,1H*)
 7011 FORMAT(1X,'*',
     $        I4,'*',I10,2F12.7       ,'     DADMEL     ELECTRON      ',4X,1H*
     $ /,' *',I4,'*',I10,2F12.7       ,'     DADMMU     MUON          ',4X,1H*)
 7012 FORMAT(1X,'*',I4,'*'
     $       ,I10,2F12.7,A31                                    ,3X,1H*)
 7013 FORMAT(1X,'*'
     $       ,20X,'THE ERROR IS RELATIVE AND  PART.WIDTH      ',10X,1H*
     $ /,' *',20X,'IN UNITS GFERMI**2*MASS**5/192/PI**3       ',10X,1H*
     $  /,1X,15(5H*****)/)
 902  PRINT 9020
 9020 FORMAT(' ----- DEKAY: LACK OF INITIALISATION')
      STOP
 910  PRINT 9100
 9100 FORMAT(' ----- DEKAY: WRONG VALUE OF KTO ')
      STOP
      END
      SUBROUTINE DEKAY1(IMOD,HH,ISGN)
C     ******************************
C THIS ROUTINE  SIMULATES TAU+  DECAY
      include 'TAUDCD_BBB_size.inc'
      COMMON / DECP4 / PP1(4),PP2(4),KF1,KF2
      COMMON / JAKI   /  JAK1,JAK2,JAKP,JAKM,KTOM
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER
      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31      
      REAL  HH(4)
      REAL  HV(4),PNU(4),PPI(4)
      REAL  PWB(4),PMU(4),PNM(4)
      REAL  PRHO(4),PIC(4),PIZ(4)
      REAL  PAA(4),PIM1(4),PIM2(4),PIPL(4)
      REAL  PKK(4),PKS(4)
      REAL  PNPI(4,9)
      REAL  PHOT(4)
      REAL  PDUM(4)
      INTEGER IFEQUALBR
      COMMON /IFEQUALBR/ IFEQUALBR
      DATA NEV,NPRIN/0,0/
      KTO=1
      IF(JAK1.EQ.-1) RETURN
      IMD=IMOD
      IF(IMD.EQ.0) THEN
C     =================
      JAK=JAK1
      IF(JAK1.EQ.0) CALL JAKER(JAK)
      IF (IFEQUALBR.GT.0) CALL JAKEREQ(1,JAK)
      IF(JAK.EQ.1) THEN
         IF (NEV.LT.NPRIN.OR.NPRIN.EQ.-1)
     C   PRINT *, 'DEKAY1(0): tau- JAK = ', JAK,
     C            'TAU-  -->   E- ANTI-NU_E NU_TAU'
         CALL DADMEL(0, ISGN,HV,PNU,PWB,PMU,PNM,PHOT)
      ELSEIF(JAK.EQ.2) THEN
         IF (NEV.LT.NPRIN.OR.NPRIN.EQ.-1)
     C   PRINT *, 'DEKAY1(0): tau- JAK = ', JAK,
     C            'TAU-  -->   MU- ANTI-NU_MU NU_TAU'
         CALL DADMMU(0, ISGN,HV,PNU,PWB,PMU,PNM,PHOT)
      ELSE
         IF (NEV.LT.NPRIN.OR.NPRIN.EQ.-1)
     C   PRINT *, 'DEKAY1(0): tau- JAK = ', JAK,' ',NAMES(JAK-NLT),' '         
         CALL DADNEW(0, ISGN,HV,PNU,PWB,PNPI,JAK-NLT)
      ENDIF
      DO 33 I=1,3
 33   HH(I)=HV(I)
      HH(4)=1.0
 
      ELSEIF(IMD.EQ.1) THEN
C     =====================
      NEV=NEV+1
        IF (JAK.LT.NBMC+1) THEN
           NEVDEC(JAK)=NEVDEC(JAK)+1
         ENDIF
      DO 34 I=1,4
 34   PDUM(I)=.0
      IF(JAK.EQ.1) THEN
        CALL DWLUEL(1,ISGN,PNU,PWB,PMU,PNM)
        CALL DWRPH(KTOM,PHOT)
        DO 10 I=1,4
 10     PP1(I)=PMU(I)
 
      ELSEIF(JAK.EQ.2) THEN
        CALL DWLUMU(1,ISGN,PNU,PWB,PMU,PNM)
        CALL DWRPH(KTOM,PHOT)
        DO 20 I=1,4
 20     PP1(I)=PMU(I)
 
      ELSE
CAM     MULTIPION DECAY
        CALL DWLNEW(1,ISGN,PNU,PWB,PNPI,JAK,NEV)
        DO 80 I=1,4
 80     PP1(I)=PWB(I)
      ENDIF
 
      ENDIF
C     =====
      END
      SUBROUTINE DEKAY2(IMOD,HH,ISGN)
C     *******************************
C THIS ROUTINE  SIMULATES TAU-  DECAY
      include 'TAUDCD_BBB_size.inc'
      COMMON / DECP4 / PP1(4),PP2(4),KF1,KF2
      COMMON / JAKI   /  JAK1,JAK2,JAKP,JAKM,KTOM
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER
      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
      REAL  HH(4)
      REAL  HV(4),PNU(4),PPI(4)
      REAL  PWB(4),PMU(4),PNM(4)
      REAL  PRHO(4),PIC(4),PIZ(4)
      REAL  PAA(4),PIM1(4),PIM2(4),PIPL(4)
      REAL  PKK(4),PKS(4)
      REAL  PNPI(4,9)
      REAL  PHOT(4)
      REAL  PDUM(4)
      INTEGER IFEQUALBR
      COMMON /IFEQUALBR/ IFEQUALBR
      DATA NEV,NPRIN/0,0/
      KTO=2
      IF(JAK2.EQ.-1) RETURN
      IMD=IMOD
      IF(IMD.EQ.0) THEN
C     =================
      JAK=JAK2
      IF(JAK2.EQ.0) CALL JAKER(JAK)
      IF (IFEQUALBR.GT.0) CALL JAKEREQ(0,JAK)
      IF(JAK.EQ.1) THEN
         IF (NEV.LT.NPRIN.OR.NPRIN.EQ.-1)
     C   PRINT *, 'DEKAY2(0): tau+ JAK = ', JAK,
     C            'TAU+  -->   E+ NU_E ANTI-NU_TAU'
         CALL DADMEL(0, ISGN,HV,PNU,PWB,PMU,PNM,PHOT)
      ELSEIF(JAK.EQ.2) THEN
         IF (NEV.LT.NPRIN.OR.NPRIN.EQ.-1)
     C   PRINT *, 'DEKAY2(0): tau+ JAK = ', JAK,
     C            'TAU+  -->   MU+ NU_MU ANTI-NU_TAU'
         CALL DADMMU(0, ISGN,HV,PNU,PWB,PMU,PNM,PHOT)
      ELSE
         IF (NEV.LT.NPRIN.OR.NPRIN.EQ.-1)
     C   PRINT *, 'DEKAY2(0): tau+ JAK = ',JAK,'~',NAMES(JAK-NLT),'~'
         CALL DADNEW(0, ISGN,HV,PNU,PWB,PNPI,JAK-NLT)
      ENDIF
      DO 33 I=1,3
 33   HH(I)=HV(I)
      HH(4)=1.0
      ELSEIF(IMD.EQ.1) THEN
C     =====================
      NEV=NEV+1
        IF (JAK.LT.NBMC+1) THEN
           NEVDEC(JAK)=NEVDEC(JAK)+1
         ENDIF
      DO 34 I=1,4
 34   PDUM(I)=.0
      IF(JAK.EQ.1) THEN
        CALL DWLUEL(2,ISGN,PNU,PWB,PMU,PNM)
        CALL DWRPH(KTOM,PHOT)
        DO 10 I=1,4
 10     PP2(I)=PMU(I)
 
      ELSEIF(JAK.EQ.2) THEN
        CALL DWLUMU(2,ISGN,PNU,PWB,PMU,PNM)
        CALL DWRPH(KTOM,PHOT)
        DO 20 I=1,4
 20     PP2(I)=PMU(I)
 
      ELSE
CAM     MULTIPION DECAY
        CALL DWLNEW(2,ISGN,PNU,PWB,PNPI,JAK,NEV)
        DO 80 I=1,4
 80     PP1(I)=PWB(I)
      ENDIF
C 
      ENDIF
C     =====
      END
      SUBROUTINE DEXAY(KTO,POL)
C ----------------------------------------------------------------------
C THIS 'DEXAY' IS A ROUTINE WHICH GENERATES DECAY OF THE SINGLE
C POLARIZED TAU,  POL IS A POLARIZATION VECTOR (NOT A POLARIMETER
C VECTOR AS IN DEKAY) OF THE TAU AND IT IS AN INPUT PARAMETER.
C KTO=0 INITIALISATION (OBLIGATORY)
C KTO=1 DENOTES TAU+ AND KTO=2 TAU-
C DEXAY(1,POL) AND DEXAY(2,POL) ARE CALLED INTERNALLY BY MC GENERATOR.
C DECAY PRODUCTS ARE TRANSFORMED READILY
C TO CMS AND WRITEN IN THE  LUND RECORD IN /LUJETS/
C KTO=100, PRINT FINAL REPORT (OPTIONAL).
C
C     called by : KORALZ
C ----------------------------------------------------------------------
      include 'TAUDCD_BBB_size.inc'
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER
      COMMON / JAKI   /  JAK1,JAK2,JAKP,JAKM,KTOM
      COMMON / IDFC  / IDFF
      COMMON /TAUPOS/ NP1,NP2                

      COMMON / TAUDCD /IDFFIN(9,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
      COMMON / INOUT / INUT,IOUT
      REAL  POL(4)
      REAL  PDUM1(4),PDUM2(4),PDUM3(4),PDUM4(4),PDUM5(4)
      REAL  PDUM(4)
      REAL  PDUMI(4,9)
      DATA IWARM/0/
      KTOM=KTO
C
      IF(KTO.EQ.-1) THEN
C     ==================

C       INITIALISATION OR REINITIALISATION
C       first or second tau positions in HEPEVT as in KORALB/Z
        NP1=3
        NP2=4
        IWARM=1
        WRITE(IOUT, 7001) JAK1,JAK2
        NEVTOT=0
        NEV1=0
        NEV2=0
        IF(JAK1.NE.-1.OR.JAK2.NE.-1) THEN
          CALL DEXEL(-1,IDUM,PDUM,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
          CALL DEXMU(-1,IDUM,PDUM,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
          CALL DEXNEW(-1,IDUM,PDUM,PDUM1,PDUM2,PDUMI,IDUM)
        ENDIF
        DO 21 I=1,NBMC
        NEVDEC(I)=0
        GAMPMC(I)=0
 21     GAMPER(I)=0
      ELSEIF(KTO.EQ.1) THEN
C     =====================
C DECAY OF TAU+ IN THE TAU REST FRAME
        NEVTOT=NEVTOT+1
        NEV1=NEV1+1
        IF(IWARM.EQ.0) GOTO 902
        ISGN=IDFF/IABS(IDFF)
CAM     CALL DEXAY1(POL,ISGN)
        CALL DEXAY1(KTO,JAK1,JAKP,POL,ISGN)
      ELSEIF(KTO.EQ.2) THEN
C     =================================
C DECAY OF TAU- IN THE TAU REST FRAME
        NEVTOT=NEVTOT+1
        NEV2=NEV2+1
        IF(IWARM.EQ.0) GOTO 902
        ISGN=-IDFF/IABS(IDFF)
CAM     CALL DEXAY2(POL,ISGN)
        CALL DEXAY1(KTO,JAK2,JAKM,POL,ISGN)
      ELSEIF(KTO.EQ.100) THEN
C     =======================
        IF(JAK1.NE.-1.OR.JAK2.NE.-1) THEN
          CALL DEXEL( 1,IDUM,PDUM,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
          CALL DEXMU( 1,IDUM,PDUM,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
          CALL DEXNEW( 1,IDUM,PDUM,PDUM1,PDUM2,PDUMI,IDUM)
          WRITE(IOUT,7010) NEV1,NEV2,NEVTOT
          WRITE(IOUT,7011) (I,NEVDEC(I),GAMPMC(I),GAMPER(I),I= 1,NLT)
          WRITE(IOUT,7012) 
     $         (I,NEVDEC(I),GAMPMC(I),GAMPER(I),NAMES(I-NLT),I=1+NLT,NLT+NMODE)
          WRITE(IOUT,7013) 
        ENDIF
      ELSE
        GOTO 910
      ENDIF
      RETURN
 7001 FORMAT(///1X,15(5H*****)

     $ /,' *',     25X,'*****TAUOLA LIBRARY: VERSION 3.1 ******',9X,1H*,
     $ /,' *',     25X,'*********** Sep     2016***************',9X,1H*,
     $ /,' *',     25X,'**AUTHORS: S.JADACH, Z.WAS*************',9X,1H*,
     $ /,' *',     25X,'**R. DECKER, M. JEZABEK, J.H.KUEHN*****',9X,1H*,
     $ /,' *',     25X,'**AVAILABLE FROM: WASM AT CERNVM ******',9X,1H*,
     $ /,' *',     25X,'***** PUBLISHED IN COMP. PHYS. COMM.***',9X,1H*,
     $ /,' *',     25X,'*******CERN TH-6793 NOVEMBER  1992*****',9X,1H*,
     $ /,' *',     25X,'**5 or more pi dec.: precision limited ',9X,1H*,
     $ /,' *',     25X,'******* IFJ-PAN-IV-2016-24 ************',9X,1H*,
     $ /,' *',     25X,'******DEXAY ROUTINE: INITIALIZATION****',9X,1H*
     $ /,' *',I20  ,5X,'JAK1   = DECAY MODE FERMION1 (TAU+)    ',9X,1H*
     $ /,' *',I20  ,5X,'JAK2   = DECAY MODE FERMION2 (TAU-)    ',9X,1H*
     $  /,1X,15(5H*****)/)
CHBU  format 7010 had more than 19 continuation lines
CHBU  split into two
 7010 FORMAT(///1X,15(5H*****)

     $ /,' *',     25X,'*****TAUOLA LIBRARY: VERSION 3.1 ******',9X,1H*,
     $ /,' *',     25X,'***********Sep      2016***************',9X,1H*,
     $ /,' *',     25X,'**AUTHORS: S.JADACH, Z.WAS*************',9X,1H*,
     $ /,' *',     25X,'**R. DECKER, M. JEZABEK, J.H.KUEHN*****',9X,1H*,
     $ /,' *',     25X,'**AVAILABLE FROM: WASM AT CERNVM ******',9X,1H*,
     $ /,' *',     25X,'***** PUBLISHED IN COMP. PHYS. COMM.***',9X,1H*,
     $ /,' *',     25X,'*******CERN-TH-5856 SEPTEMBER 1990*****',9X,1H*,
     $ /,' *',     25X,'*******CERN-TH-6195 SEPTEMBER 1991*****',9X,1H*,
     $ /,' *',     25X,'*******CERN-TH-6793 NOVEMBER  1992*****',9X,1H*,
     $ /,' *',     25X,'******* IFJ-PAN-IV-2016-24 ************',9X,1H*,
     $ /,' *',     25X,'******DEXAY ROUTINE: FINAL REPORT******',9X,1H*
     $ /,' *',I20  ,5X,'NEV1   = NO. OF TAU+ DECS. ACCEPTED    ',9X,1H*
     $ /,' *',I20  ,5X,'NEV2   = NO. OF TAU- DECS. ACCEPTED    ',9X,1H*
     $ /,' *',I20  ,5X,'NEVTOT = SUM                           ',9X,1H*
     $ /,' *','NCHAN    NOEVTS ',
     $   ' PART.WIDTH     ERROR       ROUTINE    DECAY MODE    ',4X,1H*)
 7011 FORMAT(1X,'*',
     $        I4,'*',I10,2F12.7       ,'     DADMEL     ELECTRON      ',4X,1H*
     $ /,' *',I4,'*',I10,2F12.7       ,'     DADMMU     MUON          ',4X,1H*)
 7012 FORMAT(1X,'*',I4,'*'
     $       ,I10,2F12.7,A31                                    ,3X,1H*)
 7013 FORMAT(1X,'*'
     $       ,20X,'THE ERROR IS RELATIVE AND  PART.WIDTH      ',10X,1H*
     $ /,' *',20X,'IN UNITS GFERMI**2*MASS**5/192/PI**3       ',10X,1H*
     $  /,1X,15(5H*****)/)
 902  WRITE(IOUT, 9020)
 9020 FORMAT(' ----- DEXAY: LACK OF INITIALISATION')
      STOP
 910  WRITE(IOUT, 9100)
 9100 FORMAT(' ----- DEXAY: WRONG VALUE OF KTO ')
      STOP
      END
      SUBROUTINE DEXAY1(KTO,JAKIN,JAK,POL,ISGN)
C ---------------------------------------------------------------------
C THIS ROUTINE  SIMULATES TAU+-  DECAY
C
C     called by : DEXAY
C ---------------------------------------------------------------------
      include 'TAUDCD_BBB_size.inc'
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER
      COMMON / INOUT / INUT,IOUT
      REAL  POL(4),POLAR(4)
      REAL  PNU(4),PPI(4)
      REAL  PRHO(4),PIC(4),PIZ(4)
      REAL  PWB(4),PMU(4),PNM(4)
      REAL  PAA(4),PIM1(4),PIM2(4),PIPL(4)
      REAL  PKK(4),PKS(4)
      REAL  PNPI(4,9)
      REAL PHOT(4)
      REAL PDUM(4)
      INTEGER NEV
C
      NEV=NEV+1
      IF(JAKIN.EQ.-1) RETURN
      DO 33 I=1,3
 33   POLAR(I)=POL(I)
      POLAR(4)=0.
      DO 34 I=1,4
 34   PDUM(I)=.0
      JAK=JAKIN
      IF(JAK.EQ.0) CALL JAKER(JAK)
CAM
      IF(JAK.EQ.1) THEN
        CALL DEXEL(0, ISGN,POLAR,PNU,PWB,PMU,PNM,PHOT)
        CALL DWLUEL(KTO,ISGN,PNU,PWB,PMU,PNM)
        CALL DWRPH(KTO,PHOT )
      ELSEIF(JAK.EQ.2) THEN
        CALL DEXMU(0, ISGN,POLAR,PNU,PWB,PMU,PNM,PHOT)
        CALL DWLUMU(KTO,ISGN,PNU,PWB,PMU,PNM)
        CALL DWRPH(KTO,PHOT )
      ELSE
        JNPI=JAK-NLT
        CALL DEXNEW(0, ISGN,POLAR,PNU,PWB,PNPI,JNPI)
        CALL DWLNEW(KTO,ISGN,PNU,PWB,PNPI,JAK,NEV)
      ENDIF
      NEVDEC(JAK)=NEVDEC(JAK)+1
      END
      SUBROUTINE DAMPRY(ITDKRC,XK0DEC,XK,XA,QP,XN,AMPLIT,HV)
      IMPLICIT REAL*8 (A-H,O-Z)
C ----------------------------------------------------------------------
C IT CALCULATES MATRIX ELEMENT FOR THE
C TAU --> MU(E) NU NUBAR DECAY MODE
C INCLUDING COMPLETE ORDER ALPHA QED CORRECTIONS.
C ----------------------------------------------------------------------
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL

      REAL*8  HV(4),QP(4),XN(4),XA(4),XK(4)
      REAL*8  MASS
C
      HV(4)=1.D0
      MASS = DSQRT( QP(4)**2 - QP(3)**2 - QP(2)**2 - QP(1)**2 )
      IF(MASS.LE.0.1) THEN
        IME = IMEGET(0,1)
      ELSE
        IME = IMEGET(0,2)
      ENDIF

      IF (IME.EQ.2) THEN
       AK0=XK0DEC*AMTAU
       IF(XK(4).LT.0.1D0*AK0) THEN
         AMPLIT=THB(ITDKRC,QP,XN,XA,AK0,HV)
       ELSE
         AMPLIT=SQM2(ITDKRC,QP,XN,XA,XK,AK0,HV)
       ENDIF
      ELSEIF (IME.EQ.5) THEN
        CALL DAMPRY_wrap(ITDKRC,XK0DEC,XK,XA,QP,XN,AMPLIT,HV)
      ELSEIF (IME.EQ.4) THEN
       WRITE(*,*) 'STOP from DAMPRY of TAUOLA FORTRAN.'
       WRITE(*,*) 'For leptonic decays IME=4 is not allowed.'
       WRITE(*,*) 'No hadronic current in leptonic decays !'
       STOP
      ELSEIF (IME.EQ.1) THEN
        DO  I=1,3
         HV(I)=0.0
        ENDDO
        AMPLIT= GFERMI**2       

      ELSE 
        DO  I=1,3
         HV(I)=0.0
        ENDDO
        AMPLIT= GFERMI**2       

      ENDIF
      RETURN
      END
C ----------------------------------------------------------------------
C ----------------------------------------------------------------------
C ----------------------------------------------------------------------
C RCHL UPDATE - NEW FUNCTIONS
C ----------------------------------------------------------------------
C ----------------------------------------------------------------------
C ----------------------------------------------------------------------

      
      SUBROUTINE DAM2PI(MNUM,PT,PN,PIM1,PIM2,AMPLIT,HV)
C ----------------------------------------------------------------------
* CALCULATES DIFFERENTIAL CROSS SECTION AND POLARIMETER VECTOR
* FOR TAU DECAY INTO 2 scalar MODES
* ALL SPIN EFFECTS IN THE FULL DECAY CHAIN ARE TAKEN INTO ACCOUNT.
* CALCULATIONS DONE IN TAU REST FRAME WITH Z-AXIS ALONG NEUTRINO MOMENT
C MNUM DECAY MODE IDENTIFIER.
C
C     called by : DPHSAA
C ----------------------------------------------------------------------
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4)
      REAL  PIVEC(4),PIAKS(4),HVM(4)
      COMPLEX HADCUR(4)
      DATA PI /3.141592653589793238462643/
      DATA ICONT /0/
      
      IME=IMEGET(2,MNUM)   ! type of matrix element to be used
C
      IF (IME.EQ.2) THEN
       IF     (MNUM.EQ.1) THEN
        CALL CURR_PIPI0(PIM1,PIM2,HADCUR)
       ELSEIF (MNUM.EQ.2.OR.MNUM.EQ.64.OR.MNUM.EQ.65) THEN 
         CALL CURR_PIK0(PIM1,PIM2,HADCUR)
       ELSEIF (MNUM.EQ.3) THEN
         CALL CURR_KPI0(PIM1,PIM2,HADCUR)
       ELSEIF (MNUM.EQ.4.OR.MNUM.EQ.66.OR.MNUM.EQ.67) THEN 
         CALL CURR_KK0(PIM1,PIM2,HADCUR)
      ELSE
         write(*,*) 'DAM2PI: IME = ', IME, ' wrong MNUM= ',MNUM
         STOP
       ENDIF
      ELSEIF (IME.EQ.4) THEN
!        WRITE(*,*) "(F77) Communication check. Input:"
!        WRITE(*,*) "   PIM1: ",PIM1
!        WRITE(*,*) "   PIM2: ",PIM2
        CALL CURR2_wrap(MNUM,PIM1,PIM2,HADCUR)
!        WRITE(*,*) "(F77) Communication check. Output:"
!        WRITE(*,*) " HADCUR: ",HADCUR
      ELSEIF (IME.EQ.5) THEN
!        WRITE(*,*) "(F77) Communication check. Input:"
!        WRITE(*,*) "     PT: ",PT
!        WRITE(*,*) "     PN: ",PN
!        WRITE(*,*) "   PIM1: ",PIM1
!        WRITE(*,*) "   PIM2: ",PIM2
        CALL DAM2PI_wrap(MNUM,PT,PN,PIM1,PIM2,AMPLIT,HV)
!        WRITE(*,*) "(F77) Communication check. Output:"
!        WRITE(*,*) " AMPLIT: ",AMPLIT
!        WRITE(*,*) "     HV: ",HV
        RETURN
      ENDIF

      IF (IME.EQ.2.OR.IME.EQ.4) THEN
C
* CALCULATE PI-VECTORS: VECTOR AND AXIAL
       CALL CLVEC(HADCUR,PN,PIVEC)
       CALL CLAXI(HADCUR,PN,PIAKS)
       CALL CLNUT(HADCUR,BRAKM,HVM)
* SPIN INDEPENDENT PART OF DECAY DIFF-CROSS-SECT. IN TAU REST  FRAME
       BRAK= (GV**2+GA**2)*PT(4)*PIVEC(4) +2.*GV*GA*PT(4)*PIAKS(4)
     &      +2.*(GV**2-GA**2)*AMNUTA*AMTAU*BRAKM
       IF (MNUM.EQ.1.OR.MNUM.EQ.4) THEN
         AMPLIT=(CCABIB*GFERMI)**2*BRAK
       ELSE
         AMPLIT=(SCABIB*GFERMI)**2*BRAK
       ENDIF
C POLARIMETER VECTOR IN TAU REST FRAME
       DO 90 I=1,3
       HV(I)=-(AMTAU*((GV**2+GA**2)*PIAKS(I)+2.*GV*GA*PIVEC(I)))
     &       +(GV**2-GA**2)*AMNUTA*AMTAU*HVM(I)
C HV IS DEFINED FOR TAU-    WITH GAMMA=B+HV*POL
       HV(I)=-HV(I)/BRAK
 90    CONTINUE

      ELSEIF (IME.EQ.0) THEN  ! not initialized
        DO  I=1,3
         HV(I)=0.0
        ENDDO
        HV(4)=1.0
        AMPLIT= GFERMI**2       

      ELSEIF (IME.EQ.1) THEN ! flat phase space
C        FLAT PHASE SPACE ONLY;
        DO  I=1,3
         HV(I)=0.0
        ENDDO
        HV(4)=1.0
        AMPLIT=GFERMI**2 
        RETURN
      ELSE
       write(*,*) 'DAM2PI: wrong IME= ',IME
        STOP
      ENDIF

      END


      SUBROUTINE CURR_PIPI0(PC,PN,HADCUR)
C standard TAUOLA current for tau to pi pi0 nu decay 
C now it has universal form eg. it is straighforward to add 
C scalar part
C NOTE:
C       PC 4-momentum of pi
C       PN 4-momentum of pi0
C       06.08.2011  
      IMPLICIT NONE
      COMPLEX BWIGS,HADCUR(4),FKPIPL,FRHO_PI
      COMPLEX*16              FPIBEL
      REAL  PC(4),PN(4),QQ(4),PKS(4),FPIRHO
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      INTEGER FF2PIRHO
       
      REAL PKSD,QQPKS
      INTEGER IK,K
      DO IK=1,4
       PKS(IK)=PC(IK)+ PN(IK)
        QQ(IK)=PC(IK)- PN(IK)
      ENDDO
C QQ transverse to PKS
      PKSD =PKS(4)*PKS(4)-PKS(3)*PKS(3)-PKS(2)*PKS(2)-PKS(1)*PKS(1)
      QQPKS=PKS(4)* QQ(4)-PKS(3)* QQ(3)-PKS(2)* QQ(2)-PKS(1)* QQ(1)
      DO 31 IK=1,4
 31      QQ(IK)=QQ(IK)-PKS(IK)*QQPKS/PKSD

      if(sqrt(pksd).le.2*ampi) pksd=4*ampi**2 ! phase space edge protection

      CALL GETFF2PIRHO(FF2PIRHO)
C     PRINT *, 'FF2PIRHO = ', FF2PIRHO
      IF (FF2PIRHO.EQ.0.OR.FF2PIRHO.EQ.1) THEN
C Historically supposed to use RChL package or not, but now RChL has been integrated  
         DO K=1,4 
          HADCUR(K)=QQ(k)* sqrt(FPIRHO(sqrt(pksd)))
         ENDDO
          
      ELSEIF (FF2PIRHO.EQ.2) THEN ! Belle, 
C                                  ! all fit parameters, par(1...11), are free
        DO K=1,4
         HADCUR(K)=QQ(k)* fpibel(sqrt(pksd),0)
        ENDDO
      ELSEIF (FF2PIRHO.EQ.3) THEN ! Belle
c                             ! all fit parameter free except for 
c                             !  par(1)=F_pi(0)=1-fixed

        DO K=1,4
         HADCUR(K)=QQ(k)* fpibel(sqrt(pksd),1)
        ENDDO
      ELSE
        write(*,*) 'problem in 2-scalars current FF2PIRHO=',FF2PIRHO
        stop
      ENDIF
      END


      SUBROUTINE CURR_PIK0(PC,PN,HADCUR)
C standard TAUOLA current for tau to pi K0 nu decay 
C now it has universal form eg. it is straighforward to add 
C scalar part
C NOTE:
C       PC 4-momentum of pi
C       PN 4-momentum of K0
C       06.08.2011  
      implicit none
      COMPLEX BWIGS,HADCUR(4),FKPIPL
      REAL  PC(4),PN(4),QQ(4),PKS(4),FKPISC,PKSD,QQPKS
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     &                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     &                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     &                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     &                 ,AMK,AMKZ,AMKST,GAMKST,FACT_K0PI
      COMMON / TAUKLE / BRA1,BRK0,BRK0B,BRKS
      REAL*4            BRA1,BRK0,BRK0B,BRKS 
      Integer           I,K
      
        DO I=1,4
         PKS(I)=PC(I)+ PN(I)
          QQ(I)=PC(I)- PN(I)
        ENDDO
 
C QQ transverse to PKS
        PKSD =PKS(4)*PKS(4)-PKS(3)*PKS(3)-PKS(2)*PKS(2)-PKS(1)*PKS(1)
        QQPKS=PKS(4)* QQ(4)-PKS(3)* QQ(3)-PKS(2)* QQ(2)-PKS(1)* QQ(1)
        DO 31 I=1,4
 31      QQ(I)=QQ(I)-PKS(I)*QQPKS/PKSD


        DO K=1,4
         HADCUR(K)=QQ(k)*BWIGS(pksd,AMKST,GAMKST)
c          24.03.2014 OSh: clebsh/normalization vs. PI-PI0
         HADCUR(K) = HADCUR(K)/SQRT(2.)
        ENDDO

      END


      SUBROUTINE CURR_KPI0(PC,PN,HADCUR)
C standard TAUOLA current for tau to pi pi0 nu decay 
C now it has universal form eg. it is straighforward to add 
C scalar part
C NOTE:
C       PC 4-momentum of K
C       PN 4-momentum of pi0
C       06.08.2011  
      implicit none
      COMPLEX BWIGS,HADCUR(4),FKPIPL
      REAL  PC(4),PN(4),QQ(4),PKS(4),FKPISC,PKSD,QQPKS
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST,FACT_KPI0
      COMMON / TAUKLE / BRA1,BRK0,BRK0B,BRKS
      REAL*4            BRA1,BRK0,BRK0B,BRKS
      INTEGER        I,K

        DO 30 I=1,4
         PKS(I)=PC(I)+ PN(I)
 30       QQ(I)=PC(I)- PN(I)
C QQ transverse to PKS
        PKSD =PKS(4)*PKS(4)-PKS(3)*PKS(3)-PKS(2)*PKS(2)-PKS(1)*PKS(1)
        QQPKS=PKS(4)* QQ(4)-PKS(3)* QQ(3)-PKS(2)* QQ(2)-PKS(1)* QQ(1)
        DO 31 I=1,4
 31      QQ(I)=QQ(I)-PKS(I)*QQPKS/PKSD
        DO K=1,4
         HADCUR(K)=QQ(k)*BWIGS(pksd,AMKST,GAMKST)
C          24.03.2014 OSh: clebsh/normalization vs. PI-PI0
         HADCUR(K) = HADCUR(K)/2.
        ENDDO

      END   


      SUBROUTINE CURR_KK0(PC,PN,HADCUR)
C standard TAUOLA current for tau to K K0 nu decay 
C now it has universal form eg. it is straighforward to add 
C scalar part
C NOTE:
C       PC 4-momentum of K
C       PN 4-momentum of K0
C       06.08.2011  
      IMPLICIT NONE
      COMPLEX BWIGS,HADCUR(4),FKK0_RCHT
      REAL  PC(4),PN(4),QQ(4),PKS(4),PKSD,QQPKS,FPIRK
      INTEGER I,K
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
        DO I=1,4
         PKS(I)=PC(I)+ PN(I)
          QQ(I)=PC(I)- PN(I)
        ENDDO
C QQ transverse to PKS
        PKSD =PKS(4)*PKS(4)-PKS(3)*PKS(3)-PKS(2)*PKS(2)-PKS(1)*PKS(1)
        QQPKS=PKS(4)* QQ(4)-PKS(3)* QQ(3)-PKS(2)* QQ(2)-PKS(1)* QQ(1)
        DO 31 I=1,4
 31      QQ(I)=QQ(I)-PKS(I)*QQPKS/PKSD

        DO K=1,4
         HADCUR(K)=QQ(k)*sqrt(fpirk(sqrt(pksd)))
C          24.03.2014 OSh: clebsh/normalization vs. PI-PI0
         HADCUR(K) = HADCUR(K)/sqrt(2.)
        ENDDO
 
      END


      FUNCTION COEF(I,J)
C clebsh gordan (or so ...)  coefs for 3 scalar final states
      implicit none
C IRCHL3PI=0  TAUOLA cleo COEF(I,J) =  COEFc(I,J)
C IRCHL3PI=1  TAUOLA RChL COEF(I,J) =  COEFr(I,J)
      INTEGER IRCHL3PI
      COMMON /SETRCHL/ IRCHL3PI
      INTEGER IFKPIPI
      COMMON /IFKPIPI/ IFKPIPI
      REAL COEFc(1:5,0:7)
      REAL COEFr(1:5,0:7)
      REAL COEF,COEFrr
      DATA PI /3.141592653589793238462643/
      REAL PI
      DATA ICONT /0/
      INTEGER ICONT
      INTEGER I,J,JJ
      REAL FPIc,FPIr

C initialization of FPI matrix defined in ...
C FPIc is to be used with cleo initialization

C actual choice is made in ???

      DATA  FPIc /93.3E-3/


C initialization of COEF matrix defined in ...
C COEFc is to be used with cleo initialization

      IF (ICONT.EQ.0) THEN
       ICONT=1
C
C*****COEFc(I,J)
C TAU-  -->  PI- PI- PI+
       COEFc(1,0)= 2.0*SQRT(2.)/3.0
       COEFc(2,0)=-2.0*SQRT(2.)/3.0
C AJW 2/98: Add in the D-wave and I=0 3pi substructure:
       COEFc(3,0)= 2.0*SQRT(2.)/3.0
       COEFc(4,0)= FPIc
       COEFc(5,0)= 0.0
C      TAU-  -->  K-  PI- K+
       COEFc(1,1)=-SQRT(2.)/3.0
       COEFc(2,1)= SQRT(2.)/3.0
       COEFc(3,1)= 0.0
       COEFc(4,1)= FPIc
       COEFc(5,1)= SQRT(2.)

C      TAU-  -->  K0  PI- K0B
       COEFc(1,2)=-SQRT(2.)/3.0
       COEFc(2,2)= SQRT(2.)/3.0
       COEFc(3,2)= 0.0
       COEFc(4,2)= 0.0
       COEFc(5,2)=-SQRT(2.)

C      TAU-  -->  K-  PI0 K0
C AJW 11/97: Add in the K*-prim-s, ala Finkemeier&Mirkes
       COEFc(1,3)= 1./3.
       COEFc(2,3)= -2./3.
       COEFc(3,3)= 2./3.
       COEFc(4,3)= 0.0
       COEFc(5,3)= 0.0
C      TAU-  -->  PI0 PI0 K-
       COEFc(1,4)= 1.0/SQRT(2.)/3.0
       COEFc(2,4)=-1.0/SQRT(2.)/3.0
       COEFc(3,4)= 0.0
       COEFc(4,4)= 0.0
       COEFc(5,4)= 0.0
C      TAU-  -->  K-  PI- PI+
       COEFc(1,5)=-SQRT(2.)/3.0
       COEFc(2,5)= SQRT(2.)/3.0
       COEFc(3,5)= 0.0
       COEFc(4,5)= 0.0
       COEFc(5,5)=-SQRT(2.)
C      TAU-  -->  PI- K0B PI0
C AJW 11/97: Add in the K*-prim-s, ala Finkemeier&Mirkes
       COEFc(1,6)= 1./3.
       COEFc(2,6)=-2./3.
       COEFc(3,6)= 2./3.
       COEFc(4,6)= 0.0
       COEFc(5,6)=-2.0
C      TAU-  --> ETA  PI-  PI0
       COEFc(1,7)= 0.0
       COEFc(2,7)= 0.0
       COEFc(3,7)= 0.0
       COEFc(4,7)= 0.0
       COEFc(5,7)=-SQRT(2.0/3.0)

      ENDIF
         
      IF (J.GE.11.AND.J.NE.13.AND.J.NE.14) THEN 
         COEF=COEFc(I,0)        ! these modes are not initialized
      ELSEIF ((J.EQ.9.OR.J.EQ.0.OR.J.EQ.10).AND.(IRCHL3PI.EQ.1)) THEN ! so far rchl only for 3pi modes
       COEF=COEFrr(I,J)
      ELSE 
       JJ=J
       IF(J.EQ.13.OR.J.EQ.14) JJ=7
       IF(J.GE.9) JJ=0
       COEF=COEFc(I,JJ)
      ENDIF
      END


      SUBROUTINE INIRChL(IVERI)
C routine to set version no for the currents physics initialization
C IVER=0  TAUOLA cleo
C IVER=1  TAUOLA RChL

      implicit none
      INTEGER IVERI

      COMMON /IPChT/ IVER
      INTEGER        IVER
      IVER=IVERI

      IF (IVER.EQ.1) THEN
        CALL RCHL_PARAMETERS(1)
      ENDIF
      end


      SUBROUTINE INIRChLget(I)
C routine to get version no for the currents physics initialization
C IVER=0  TAUOLA cleo
C IVER=1  TAUOLA RChL
      COMMON /IPChT/ IVER
      INTEGER        IVER

      I=IVER
      end

C ----------------------------------------------------------------------
C ----------------------------------------------------------------------
C ----------------------------------------------------------------------
C RCHL UPDATE - END OF NEW FUNCTIONS
C ----------------------------------------------------------------------
C ----------------------------------------------------------------------
C ----------------------------------------------------------------------


      SUBROUTINE DPHNPI(DGAMT,HVX,PNX,PRX,PPIX,JNPI)

C ----------------------------------------------------------------------
C IT SIMULATES MULTIPI DECAY IN TAU REST FRAME WITH
C Z-AXIS OPPOSITE TO NEUTRINO MOMENTUM
C ----------------------------------------------------------------------
      IMPLICIT NONE
      COMMON / PARMASD / AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
C
      REAL*8            AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      include 'TAUDCD_BBB_size.inc'

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
      REAL*8 WETMAX(NBMC)
C
      REAL*8  PN(4),PR(4),PPI(4,9),HV(4)
      REAL*8  PV(5,9),PT(4),UE(3),BE(3)
      REAL*8  AMX,AMS1,AMS2,PA,PHS,PHSMAX,PMIN,PMAX
      REAL*8  AMW,AMX2,BRAK,PS,PXN,PXQ,QXN,PHSPAC
      
      REAL*4  RRR(9),RRX(2),RN(1),RR2(1) ! RANMAR generates floats
      REAL*8  GAM,BEP,PHI,PAWT,A,B,C
      REAL*8  AMPIK,PI,XNPI,RR1
      
C --- Variables below are arguments of this or other routines
      REAL*4  PNX(4),PRX(4),PPIX(4,9),HVX(4)
      REAL*4  AMPLIT,DGAMT,DCDMAS,AMX2R,SIGEE,SIGEE_wrap

      INTEGER  JNPI,IDFFIN,MULPIK,NCONT,MNUM,IMEGET
      INTEGER  I,J,K,L,IL,JL,JN,KK,ND,IME

      DATA PI /3.141592653589793238462643/
      DATA WETMAX /500*1D-15/
C
CC--      PAWT(A,B,C)=SQRT((A**2-(B+C)**2)*(A**2-(B-C)**2))/(2.*A)
C
      PAWT(A,B,C)=
     $  SQRT(MAX(0.D0,(A**2-(B+C)**2)*(A**2-(B-C)**2)))/(2.D0*A)

C
      AMPIK(I,J)=DCDMAS(IDFFIN(I,J),0)
C
C
      jn=JNPI-nm4-nm5+3  ! for Q^2 spectrum we use sigee from ancient times. First 3 options of sigee are skipped.
      MNUM=JNPI-nm4-nm5
      IME=IMEGET(6,MNUM)
      IF ((JNPI.LE.0).OR.JNPI.GT.100) THEN
       WRITE(6,*) 'JNPI OUTSIDE RANGE DEFINED BY WETMAX; JNPI=',JNPI
       STOP
      ENDIF


C TAU MOMENTUM
      PT(1)=0.
      PT(2)=0.
      PT(3)=0.
      PT(4)=AMTAUD
C

 500  CONTINUE

C MASS OF VIRTUAL W
      ND=MULPIK(JNPI)
      PS=0.
      PHSPAC = 1./2.**5 /PI**2
      DO 4 I=1,ND
4     PS  =PS+AMPIK(I,JNPI)

      CALL KKMC_RANMAR(RR2,1)

      AMS1=PS**2
      AMS2=(AMTAUD-AMNUTAD)**2
C
C

      AMX2=AMS1+   RR2(1)*(AMS2-AMS1)

      AMX =SQRT(AMX2)
      AMW =AMX
      PHSPAC=PHSPAC * (AMS2-AMS1)
C
C TAU-NEUTRINO MOMENTUM
      PN(1)=0
      PN(2)=0
      PN(4)=1./(2*AMTAUD)*(AMTAUD**2+AMNUTAD**2-AMX2)
      PN(3)=-SQRT(ABS((PN(4)-AMNUTAD)*(PN(4)+AMNUTAD)))
C W MOMENTUM
      PR(1)=0
      PR(2)=0
      PR(4)=1./(2*AMTAUD)*(AMTAUD**2-AMNUTAD**2+AMX2)
      PR(3)=-PN(3)
      PHSPAC=PHSPAC * (4.*PI) * (2.*PR(3)/AMTAUD)
C
C AMPLITUDE  (cf YS.Tsai Phys.Rev.D4,2821(1971)
C    or F.Gilman SH.Rhie Phys.Rev.D31,1066(1985)
C
      PXQ=AMTAUD*PR(4)
      PXN=AMTAUD*PN(4)
      QXN=PR(4)*PN(4)-PR(1)*PN(1)-PR(2)*PN(2)-PR(3)*PN(3)

C HERE WAS AN ERROR. 20.10.91 (ZW)
C       BRAK=2*(GV**2+GA**2)*(2*PXQ*PXN+AMX2*QXN)

        BRAK=2*(GV**2+GA**2)*(2*PXQ*QXN+AMX2*PXN)
     &      -6*(GV**2-GA**2)*AMTAUD*AMNUTAD*AMX2
CAM     Assume neutrino mass=0. and sum over final polarisation
C     BRAK= 2*(AMTAUD**2-AMX2) * (AMTAUD**2+2.*AMX2)

!       if(jn.le.6) write(*,*) 'sigeje=',jn,amx2,jn,SIGEE(AMX2,JN)
!       if(jn.eq.7) stop
      AMX2R=AMX2
      IF (IME.EQ.2) THEN
       AMPLIT=CCABIB**2*GFERMI**2/2.* BRAK*AMX2*SIGEE(AMX2R,JN)
      ELSEIF (IME.EQ.4) THEN
       AMPLIT=CCABIB**2*GFERMI**2/2.* BRAK*AMX2*SIGEE_wrap(AMX2R,JN)
      ELSEIF (IME.EQ.5) THEN
       WRITE(*,*) 'STOP from DPHNPI of TAUOLA FORTRAN.'
       WRITE(*,*) 'For multipion modes IME=5 is not allowed.'
       WRITE(*,*) 'No matrix element in this cases !'
       STOP

      ELSE
       AMPLIT=CCABIB**2*GFERMI**2/2.*     AMX2*SIGEE(AMX2R,JN)
      ENDIF
      DGAMT=1./(2.*AMTAUD)*AMPLIT*PHSPAC
C
C   ISOTROPIC W DECAY IN W REST FRAME

      PHSMAX = 1.

      DO 200 I=1,4
  200 PV(I,1)=PR(I)
      PV(5,1)=AMW
      PV(5,ND)=AMPIK(ND,JNPI)
C    COMPUTE MAX. PHASE SPACE FACTOR
      PMAX=AMW-PS+AMPIK(ND,JNPI)
      PMIN=.0
      DO 220 IL=ND-1,1,-1
      PMAX=PMAX+AMPIK(IL,JNPI)
      PMIN=PMIN+AMPIK(IL+1,JNPI)

  220 PHSMAX=PHSMAX*PAWT(PMAX,PMIN,AMPIK(IL,JNPI))/PMAX

C --- 2.02.94 ZW  9 lines
      AMX=AMW
      DO 222 IL=1,ND-2
      AMS1=.0
      DO 223 JL=IL+1,ND
 223  AMS1=AMS1+AMPIK(JL,JNPI)
      AMS1=AMS1**2
      AMX =(AMX-AMPIK(IL,JNPI))
      AMS2=(AMX)**2
      PHSMAX=PHSMAX * (AMS2-AMS1)
 222  CONTINUE
      NCONT=0
  100 CONTINUE
      NCONT=NCONT+1
CAM  GENERATE ND-2 EFFECTIVE MASSES
      PHS=1.D0
      PHSPAC = 1./2.**(6*ND-7) /PI**(3*ND-4)
      AMX=AMW
      CALL KKMC_RANMAR(RRR,ND-2)
      DO 230 IL=1,ND-2
      AMS1=.0D0
      DO 231 JL=IL+1,ND
  231 AMS1=AMS1+AMPIK(JL,JNPI)
      AMS1=AMS1**2
      AMS2=(AMX-AMPIK(IL,JNPI))**2
      RR1=RRR(IL)
      AMX2=AMS1+  RR1*(AMS2-AMS1)
      AMX=SQRT(AMX2)
      PV(5,IL+1)=AMX
      PHSPAC=PHSPAC * (AMS2-AMS1)
C ---  2.02.94 ZW 1 line 
      PHS=PHS* (AMS2-AMS1)
      PA=PAWT(PV(5,IL),PV(5,IL+1),AMPIK(IL,JNPI))
      PHS   =PHS    *PA/PV(5,IL)
  230 CONTINUE
      PA=PAWT(PV(5,ND-1),AMPIK(ND-1,JNPI),AMPIK(ND,JNPI))
      PHS   =PHS    *PA/PV(5,ND-1)
      CALL KKMC_RANMAR(RN,1)
      IF(PHSMAX.NE.0.0) THEN    ! TP 5.10.2011 due to rounding errs.
                             ! PHSMAX may be zero, protect div. by it
        WETMAX(JNPI)=1.2D0*MAX(WETMAX(JNPI)/1.2D0,PHS/PHSMAX)
      ELSE
        WETMAX(JNPI)=1.2D0*WETMAX(JNPI)/1.2D0
      ENDIF

      IF (NCONT.EQ.500 000) THEN
          XNPI=0.0
          DO KK=1,ND
            XNPI=XNPI+AMPIK(KK,JNPI)
          ENDDO
       WRITE(6,*) 'ROUNDING INSTABILITY IN DPHNPI ?'
       WRITE(6,*) 'AMW=',AMW,'XNPI=',XNPI
       WRITE(6,*) 'IF =AMW= IS NEARLY EQUAL =XNPI= THAT IS IT' 
       WRITE(6,*) 'PHS=',PHS,'PHSMAX=',PHSMAX 
       GOTO 500
      ENDIF
      IF(RN(1)*PHSMAX*WETMAX(JNPI).GT.PHS) GO TO 100

C...PERFORM SUCCESSIVE TWO-PARTICLE DECAYS IN RESPECTIVE CM FRAME
  280 DO 300 IL=1,ND-1
      PA=PAWT(PV(5,IL),PV(5,IL+1),AMPIK(IL,JNPI))

      CALL KKMC_RANMAR(RRX,2)
      UE(3)=2.*RRX(1)-1.
      PHI=2.*PI*RRX(2)
      UE(1)=SQRT(1.D0-UE(3)**2)*COS(PHI)
      UE(2)=SQRT(1.D0-UE(3)**2)*SIN(PHI)

      DO 290 J=1,3
      PPI(J,IL)=PA*UE(J)
  290 PV(J,IL+1)=-PA*UE(J)
      PPI(4,IL)=SQRT(PA**2+AMPIK(IL,JNPI)**2)
      PV(4,IL+1)=SQRT(PA**2+PV(5,IL+1)**2)
      PHSPAC=PHSPAC *(4.*PI)*(2.*PA/PV(5,IL))
  300 CONTINUE
C...LORENTZ TRANSFORM DECAY PRODUCTS TO TAU FRAME
      DO 310 J=1,4
  310 PPI(J,ND)=PV(J,ND)
      DO 340 IL=ND-1,1,-1
      DO 320 J=1,3
  320 BE(J)=PV(J,IL)/PV(4,IL)
      GAM=PV(4,IL)/PV(5,IL)
      DO 340 I=IL,ND
      BEP=BE(1)*PPI(1,I)+BE(2)*PPI(2,I)+BE(3)*PPI(3,I)
      DO 330 J=1,3

  330 PPI(J,I)=PPI(J,I)+GAM*(GAM*BEP/(1.D0+GAM)+PPI(4,I))*BE(J)

      PPI(4,I)=GAM*(PPI(4,I)+BEP)
  340 CONTINUE
C
            HV(4)=1.
            HV(3)=0.
            HV(2)=0.
            HV(1)=0.

      DO K=1,4
        PNX(K)=PN(K)
        PRX(K)=PR(K)
        HVX(K)=HV(K)
        DO L=1,ND
          PPIX(K,L)=PPI(K,L)
        ENDDO
      ENDDO

      RETURN
      END

      SUBROUTINE DPH3PI(DGAMT,HV,PN,PAA,PNPI,JNPI)
C ----------------------------------------------------------------------
* IT SIMULATES THREE PI (K) DECAY IN THE TAU REST FRAME
* Z-AXIS ALONG HADRONIC SYSTEM
C ----------------------------------------------------------------------
      include 'TAUDCD_BBB_size.inc'

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
      REAL*4  RR1(1)    ! RANMAR generates floats
      REAL  HV(4),PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4),PNPI(4,9)
C --- MASSES OF THE DECAY PRODUCTS
       AMP1=DCDMAS(IDFFIN(1,JNPI),0)
       AMP2=DCDMAS(IDFFIN(2,JNPI),0)
       AMP3=DCDMAS(IDFFIN(3,JNPI),0)

C MATRIX ELEMENT NUMBER:
      MNUM=JNPI-NM4-NM5-NM6
      IF(JNPI.EQ.0) then
       write(*,*) 'problem JNPI=',JNPI
       STOP
      endif
      IF(JNPI.EQ.0) MNUM=0

      CALL DPHTRE(DGAMT,HV,PN,PAA,PIM1,AMP1,PIM2,AMP2,PIPL,AMP3,MNUM)
      DO I=1,4
         PNPI(I,1)=PIM1(I)
         PNPI(I,2)=PIM2(I)
         PNPI(I,3)=PIPL(I)
      ENDDO
      IF (MNUM.EQ.21) THEN ! symmetrize 
         CALL KKMC_RANMAR(RR1,1)
         IF (RR1(1).GT.0.5) THEN
            DO I=1,4
               PNPI(I,1)=PIPL(I)
               PNPI(I,2)=PIM2(I)
               PNPI(I,3)=PIM1(I)
            ENDDO    
         ENDIF
      ENDIF
      END


      SUBROUTINE CHOICE3(MNUM,RR,ICHAN,xPROB1,xPROB2,xPROB3,
     $            xAMRX,xGAMRX,xAMRA,xGAMRA,xAMRB,xGAMRB)
      include 'TAUDCD_BBB_size.inc'
      COMMON /SAMPL3/ PROB1(NM3),PROB2(NM3),AMRX(NM3),GAMRX(NM3),AMRA(NM3),GAMRA(NM3),AMRB(NM3),GAMRB(NM3)

       xPROB1=PROB1(MNUM)
       xPROB2=PROB2(MNUM)
       xAMRX =AMRX(MNUM)
       xGAMRX=GAMRX(MNUM)
       xAMRA =AMRA(MNUM)
       xGAMRA=GAMRA(MNUM)
       xAMRB =AMRB(MNUM)
       xGAMRB=GAMRB(MNUM)

      IF    (RR.LE.xPROB1) THEN
       ICHAN=1
      ELSEIF(RR.LE.(xPROB1+xPROB2)) THEN
       ICHAN=2
        AX    =xAMRA
        GX    =xGAMRA
        xAMRA =xAMRB
        xGAMRA=xGAMRB
        xAMRB =AX
        xGAMRB=GX
        PX    =xPROB1
        xPROB1=xPROB2
        xPROB2=PX
      ELSE
       ICHAN=3
      ENDIF
C
      xPROB3=1.0-xPROB1-xPROB2
      END


      SUBROUTINE DPHTRE(DGAMT,HV,PNR,PAAR,PIM1R,AMPA,PIM2R,AMPB,PIPLR,AMP3R,MNUM)
C ----------------------------------------------------------------------
* IT SIMULATES A1  DECAY IN TAU REST FRAME WITH
* Z-AXIS ALONG A1  MOMENTUM
* it can be also used to generate K K pi and K pi pi tau decays.
* INPUT PARAMETERS
* AMP1 - mass of first pi, etc. (1-3)
* MNUM - matrix element type
*  0   - a1 matrix element
* 1-6  - matrix element for K pi pi, K K pi decay modes
*  7   - pi- pi0 gamma matrix element
C ----------------------------------------------------------------------
      IMPLICIT NONE
      COMMON / PARMASD / AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
C
      REAL*8            AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL

      REAL*8 PR(4),PT(4),PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4)
      REAL*8 AMP1,AMP2,AMP3,AMS1,AMS2,AM2,AM3,AM2SQ,AM3SQ,PHSPAC
      REAL*8 FF1,FF2,GG1,GG2,PHF0,PHF1,PHF2,A1,A2,A3
      REAL*8 XPRO,XJAJE,XJADW,XJAC1,XJAC2,XJAC3
      REAL*8 ENQ1,ENQ2,EXE,THET,PHI,PPI,PPPI,ALP,ALP1,ALP2

      REAL*4 RRR(5) ! RANMAR generates floats
      REAL*8 RR1,RR2,RR3,RR4
      REAL*8 X,Y,Z,XLAM,PI

C --- Variables below are arguments of this or other routines
      REAL*4 HV(4),PTR(4),PNR(4),PAAR(4),PIM1R(4),PIM2R(4),PIPLR(4)
      REAL*4 AMPLIT,DGAMT,DCDMAS,AMPA,AMPB,AMP3R
      REAL*4 AMRA,AMRB,AMRX,GAMRA,GAMRB,GAMRX,PROB1,PROB2,PROB3,RR

      INTEGER  MNUM,IDFFIN,MULPIK,ICONT,I,ICHAN

      DATA PI /3.141592653589793238462643/
      DATA ICONT /0/
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.d0*Y*Z))
C AMROD, GAMROD IS ONLY A PARAMETER FOR GETTING HIGH EFFICIENCY
C
C THREE BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)
      PHSPAC=1.d0/2**17/PI**8

C     TAU MOMENTUM
      PT(1)=0.d0
      PT(2)=0.d0
      PT(3)=0.d0
      PT(4)=AMTAUD
C
      CALL KKMC_RANMAR(RRR,5)
      RR=RRR(5)
C
      CALL CHOICE3(MNUM,RR,ICHAN,PROB1,PROB2,PROB3,
     $            AMRX,GAMRX,AMRA,GAMRA,AMRB,GAMRB)
      IF     (ICHAN.EQ.1) THEN
        AMP1=AMPB
        AMP2=AMPA
      ELSEIF (ICHAN.EQ.2) THEN
        AMP1=AMPA
        AMP2=AMPB
      ELSE
        AMP1=AMPB
        AMP2=AMPA
      ENDIF
        AMP3=AMP3R
      
CAM
        RR1=RRR(1)
        AMS1=(AMP1+AMP2+AMP3)**2
        AMS2=(AMTAUD-AMNUTAD)**2

* PHASE SPACE WITH SAMPLING FOR A1  RESONANCE

        ALP1=ATAN((AMS1-AMRX**2)/AMRX/GAMRX)
        ALP2=ATAN((AMS2-AMRX**2)/AMRX/GAMRX)
        ALP=ALP1+RR1*(ALP2-ALP1)
        AM3SQ =AMRX**2+AMRX*GAMRX*TAN(ALP)
        AM3 =SQRT(AM3SQ)
        PHSPAC=PHSPAC*((AM3SQ-AMRX**2)**2+(AMRX*GAMRX)**2)/(AMRX*GAMRX)
        PHSPAC=PHSPAC*(ALP2-ALP1)
C MASS OF (REAL/VIRTUAL) RHO -
        RR2=RRR(2)
        AMS1=(AMP2+AMP3)**2
        AMS2=(AM3-AMP1)**2
        
      IF (ICHAN.LE.2) THEN

* PHASE SPACE WITH SAMPLING FOR RHO RESONANCE,

        ALP1=ATAN((AMS1-AMRA**2)/AMRA/GAMRA)
        ALP2=ATAN((AMS2-AMRA**2)/AMRA/GAMRA)
        ALP=ALP1+RR2*(ALP2-ALP1)
        AM2SQ =AMRA**2+AMRA*GAMRA*TAN(ALP)
        AM2 =SQRT(AM2SQ)
C --- THIS PART OF THE JACOBIAN WILL BE RECOVERED LATER ---------------
C     PHSPAC=PHSPAC*(ALP2-ALP1)
C     PHSPAC=PHSPAC*((AM2SQ-AMRA**2)**2+(AMRA*GAMRA)**2)/(AMRA*GAMRA)
C----------------------------------------------------------------------
      ELSE

* FLAT PHASE SPACE;

        AM2SQ=AMS1+   RR2*(AMS2-AMS1)
        AM2 =SQRT(AM2SQ)
C --- this part of jacobian will be recovered later       
        PHF0=(AMS2-AMS1)
      ENDIF

* RHO RESTFRAME, DEFINE PIPL AND PIM1

        ENQ1=(AM2SQ-AMP2**2+AMP3**2)/(2*AM2)
        ENQ2=(AM2SQ+AMP2**2-AMP3**2)/(2*AM2)
        PPI=         ENQ1**2-AMP3**2
        PPPI=SQRT(ABS(ENQ1**2-AMP3**2))
C --- this part of jacobian will be recovered later
        PHF1=(4.d0*PI)*(2.d0*PPPI/AM2)

* PI MINUS MOMENTUM IN RHO REST FRAME

        CALL SPHERD(PPPI,PIPL)
        PIPL(4)=ENQ1

* PI0 1 MOMENTUM IN RHO REST FRAME

        DO 30 I=1,3
 30     PIM1(I)=-PIPL(I)
        PIM1(4)=ENQ2

* A1 REST FRAME, DEFINE PIM2

*       RHO  MOMENTUM
        PR(1)=0.d0
        PR(2)=0.d0
        PR(4)=1.d0/(2.d0*AM3)*(AM3**2+AM2**2-AMP1**2)
        PR(3)= SQRT(ABS(PR(4)**2-AM2**2))
        PPI  =          PR(4)**2-AM2**2
*       PI0 2 MOMENTUM
        PIM2(1)=0.d0
        PIM2(2)=0.d0
        PIM2(4)=1.d0/(2.d0*AM3)*(AM3**2-AM2**2+AMP1**2)
        PIM2(3)=-PR(3)
C --- this part of jacobian will be recovered later     
        PHF2=(4*PI)*(2*PR(3)/AM3)

* OLD PIONS BOOSTED FROM RHO REST FRAME TO A1 REST FRAME

      EXE=(PR(4)+PR(3))/AM2
      CALL BOSTD3(EXE,PIPL,PIPL)
      CALL BOSTD3(EXE,PIM1,PIM1)
      RR3=RRR(3)
      RR4=RRR(4)

CAM   THET =PI*RR3

      THET =ACOS(-1.d0+2.d0*RR3)
      PHI = 2.d0*PI*RR4
      CALL ROTPOX(THET,PHI,PIPL)
      CALL ROTPOX(THET,PHI,PIM1)
      CALL ROTPOX(THET,PHI,PIM2)
      CALL ROTPOX(THET,PHI,PR)
C
* NOW TO THE TAU REST FRAME, DEFINE A1 AND NEUTRINO MOMENTA
* A1  MOMENTUM
      PAA(1)=0.d0
      PAA(2)=0.d0
      PAA(4)=1.d0/(2.d0*AMTAUD)*(AMTAUD**2-AMNUTAD**2+AM3**2)
      PAA(3)= SQRT(ABS(PAA(4)**2-AM3**2))
      PPI   =          PAA(4)**2-AM3**2
      PHSPAC=PHSPAC*(4.d0*PI)*(2.d0*PAA(3)/AMTAUD)
* TAU-NEUTRINO MOMENTUM
      PN(1)=0.d0
      PN(2)=0.d0
      PN(4)=1.d0/(2.d0*AMTAUD)*(AMTAUD**2+AMNUTAD**2-AM3**2)
      PN(3)=-PAA(3)
C     HERE WE CORRECT FOR THE JACOBIANS OF THE TWO CHAINS
         !    write(*,*) 'amnuta=',amnuta,' ',amnuta**2,' ',sqrt(pn(4)**2-pn(3)**2)
       ! stop
C ---FIRST CHANNEL ------- PIM1+PIPL
        AMS1=(AMP2+AMP3)**2
        AMS2=(AM3-AMP1)**2
        ALP1=ATAN((AMS1-AMRA**2)/AMRA/GAMRA)
        ALP2=ATAN((AMS2-AMRA**2)/AMRA/GAMRA)
       XPRO =      (PIM1(3)+PIPL(3))**2
     $            +(PIM1(2)+PIPL(2))**2+(PIM1(1)+PIPL(1))**2
       AM2SQ=-XPRO+(PIM1(4)+PIPL(4))**2
C JACOBIAN OF SPEEDING
       FF1   =       ((AM2SQ-AMRA**2)**2+(AMRA*GAMRA)**2)/(AMRA*GAMRA)
       FF1   =FF1     *(ALP2-ALP1)
C LAMBDA OF RHO DECAY
       GG1   =       (4.d0*PI)*(XLAM(AM2SQ,AMP2**2,AMP3**2)/AM2SQ)
C LAMBDA OF A1 DECAY
       GG1   =GG1   *(4.d0*PI)*SQRT(4.d0*XPRO/AM3SQ)
       XJAJE=GG1*(AMS2-AMS1)
C ---SECOND CHANNEL ------ PIM2+PIPL
       AMS1=(AMP1+AMP3)**2
       AMS2=(AM3-AMP2)**2
        ALP1=ATAN((AMS1-AMRB**2)/AMRB/GAMRB)
        ALP2=ATAN((AMS2-AMRB**2)/AMRB/GAMRB)
       XPRO =      (PIM2(3)+PIPL(3))**2
     $            +(PIM2(2)+PIPL(2))**2+(PIM2(1)+PIPL(1))**2
       AM2SQ=-XPRO+(PIM2(4)+PIPL(4))**2
       FF2   =       ((AM2SQ-AMRB**2)**2+(AMRB*GAMRB)**2)/(AMRB*GAMRB)
       FF2   =FF2     *(ALP2-ALP1)
       GG2   =       (4.d0*PI)*(XLAM(AM2SQ,AMP1**2,AMP3**2)/AM2SQ)
       GG2   =GG2   *(4.d0*PI)*SQRT(4.d0*XPRO/AM3SQ)
       XJADW=GG2*(AMS2-AMS1)
C
       A1=0.d0
       A2=0.d0
       A3=0.d0
       XJAC1=FF1*GG1
       XJAC2=FF2*GG2
       IF (ICHAN.EQ.2) THEN
         XJAC3=XJADW
       ELSE
         XJAC3=XJAJE
       ENDIF
       IF (XJAC1.NE.0.d0) A1=PROB1/XJAC1
       IF (XJAC2.NE.0.d0) A2=PROB2/XJAC2
       IF (XJAC3.NE.0.d0) A3=PROB3/XJAC3
C
       IF (A1+A2+A3.NE.0.d0) THEN
         PHSPAC=PHSPAC/(A1+A2+A3)
       ELSE
         PHSPAC=0.d0
       ENDIF
       IF(ICHAN.EQ.2) THEN
        DO 70 I=1,4
        X=PIM1(I)
        PIM1(I)=PIM2(I)
 70     PIM2(I)=X
       ENDIF
* ALL PIONS BOOSTED FROM A1  REST FRAME TO TAU REST FRAME
* Z-AXIS ANTIPARALLEL TO NEUTRINO MOMENTUM
      EXE=(PAA(4)+PAA(3))/AM3
      CALL BOSTD3(EXE,PIPL,PIPL)
      CALL BOSTD3(EXE,PIM1,PIM1)
      CALL BOSTD3(EXE,PIM2,PIM2)
      CALL BOSTD3(EXE,PR,PR)

      IF(MNUM.EQ.9) THEN
C THE STATISTICAL FACTOR FOR IDENTICAL PI-S
        PHSPAC=PHSPAC/2.d0
        CALL CH3PISET(2) ! information that it's pi- pi0 pi0 passed for further use
      ELSEIF(MNUM.EQ.0.OR.MNUM.EQ.10) THEN   ! MNUM=0 probably never hap
C THE STATISTICAL FACTOR FOR IDENTICAL PI-S
        PHSPAC=PHSPAC/2.d0
        CALL CH3PISET(1) ! information that it's pi- pi- pi+ passed for further use
      ENDIF

      DO I=1,4           ! DAM3PI requires floats
        PNR(I)=PN(I)     ! PN and PAA arearguments of this routine
        PAAR(I)=PAA(I)   ! and have to remain float
        PTR(I)=PT(I)
        PIM1R(I)=PIM1(I)
        PIM2R(I)=PIM2(I)
        PIPLR(I)=PIPL(I)
      ENDDO

C PARTIAL WIDTH CONSISTS OF PHASE SPACE AND AMPLITUDE
       CALL DAM3PI(MNUM,PTR,PNR,PIM1R,PIM2R,PIPLR,AMPLIT,HV)
!        if (mnum.eq.9) write(*,*) 'mnum=',mnum,amplit,phspac
!        if (amplit.eq.0.0) stop
!      if (mnum.gt.7) write(*,*) 'mnumy=',mnum

      DGAMT=1/(2.*AMTAUD)*AMPLIT*PHSPAC
      END
 
      SUBROUTINE TAUGETSIGN(SIGN)
C calculation of sign for CLAXI: version of C++
      COMMON / JAKI   /  JAK1,JAK2,JAKP,JAKM,KTOM
      COMMON / IDFC  / IDFF
      IF     (KTOM.EQ.1.OR.KTOM.EQ.-1) THEN
        SIGN= IDFF/ABS(IDFF)
      ELSEIF (KTOM.EQ.2) THEN
        SIGN=-IDFF/ABS(IDFF)
      ELSE
        PRINT *, 'TAUOLA: STOP IN calculation of sign for CLAXI: KTOM=',KTOM
        STOP
      ENDIF
      END

      SUBROUTINE CURR3PI(MNU,PIM1,PIM2,PIM3,HADCUR)

      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL

      INTEGER IRCHL3PI
      COMMON /SETRCHL/ IRCHL3PI
      
      REAL  PIM1(4),PIM2(4),PIM3(4)
      REAL  PAA(4),VEC1(4),VEC2(4),VEC3(4),VEC4(4),VEC5(4)

      REAL FNORM(0:18)
      DOUBLE PRECISION GETFPIRPT

      COMPLEX HADCUR(4),FORM1,FORM2,FORM3,FORM4,FORM5,UROJ

      COMPLEX F1,F2,F3,F4,F5

      EXTERNAL FORM1,FORM2,FORM3,FORM4,FORM5
      DATA PI /3.141592653589793238462643/
      DATA ICONT /0/
C
      DATA  FPIc /93.3E-3/
      IF (ICONT.EQ.0) THEN
       ICONT=1
       UROJ=CMPLX(0.0,1.0)
       DWAPI0=SQRT(2.0)

      ENDIF
      MNUM=MNU
      IF(MNUM.eq.10) MNUM=0  ! we shift position 10 to 0 (temporarily) 
                             ! to have the flexibility for FORM1, ... 
                             ! of the past
      IF (MNUM.GE.19.AND.MNUM.LE.21) MNUM=2 
      IF (MNUM.GE.22.AND.MNUM.LE.23) MNUM=3
      IF (MNUM.GE.24.AND.MNUM.LE.25) MNUM=6
      IF (MNUM.GE.26.AND.MNUM.LE.27) MNUM=14
      IF (MNUM.GE.31.AND.MNUM.LE.34) MNUM=7 
      IF (MNUM.GE.35.AND.MNUM.LE.38) MNUM=13 
      IF (MNUM.GE.39.AND.MNUM.LE.46) MNUM=14 
      IF ((MNUM.EQ.9.OR.MNUM.EQ.0).AND.IRCHL3PI.EQ.1) THEN ! so far rchl only for 3pi modes
         FPI=GETFPIRPT(1) ! GET  defined in in ffwid3pi.f  of RChL-currents
       ELSE
         FPI=FPIc
       ENDIF       
       FNORM(0)=CCABIB/FPI
       FNORM(1)=CCABIB/FPI
       FNORM(2)=CCABIB/FPI
       FNORM(3)=CCABIB/FPI
       FNORM(4)=SCABIB/FPI/DWAPI0
       FNORM(5)=SCABIB/FPI
       FNORM(6)=SCABIB/FPI
       FNORM(7)=CCABIB/FPI
       FNORM(8)=0.0  ! this chanel is dead
       FNORM(9)=CCABIB/FPI
       DO K=10,18
         FNORM(K)=FNORM(9) ! these chanells are not initialized (except 13,14)
       ENDDO
       FNORM(13)=SCABIB/FPI
       FNORM(14)=SCABIB/FPI
C     
      DO 10 I=1,4
   10 PAA(I)=PIM1(I)+PIM2(I)+PIM3(I)
      XMAA   =SQRT(ABS(PAA(4)**2-PAA(3)**2-PAA(2)**2-PAA(1)**2))
      XMRO1  =SQRT(ABS((PIM3(4)+PIM2(4))**2-(PIM3(1)+PIM2(1))**2
     $                -(PIM3(2)+PIM2(2))**2-(PIM3(3)+PIM2(3))**2))
      XMRO2  =SQRT(ABS((PIM3(4)+PIM1(4))**2-(PIM3(1)+PIM1(1))**2
     $                -(PIM3(2)+PIM1(2))**2-(PIM3(3)+PIM1(3))**2))
      XMRO3  =SQRT(ABS((PIM1(4)+PIM2(4))**2-(PIM1(1)+PIM2(1))**2
     $                -(PIM1(2)+PIM2(2))**2-(PIM1(3)+PIM2(3))**2))
* ELEMENTS OF HADRON CURRENT
      PROD1  =PAA(4)*(PIM2(4)-PIM3(4))-PAA(1)*(PIM2(1)-PIM3(1))
     $       -PAA(2)*(PIM2(2)-PIM3(2))-PAA(3)*(PIM2(3)-PIM3(3))
      PROD2  =PAA(4)*(PIM3(4)-PIM1(4))-PAA(1)*(PIM3(1)-PIM1(1))
     $       -PAA(2)*(PIM3(2)-PIM1(2))-PAA(3)*(PIM3(3)-PIM1(3))
      PROD3  =PAA(4)*(PIM1(4)-PIM2(4))-PAA(1)*(PIM1(1)-PIM2(1))
     $       -PAA(2)*(PIM1(2)-PIM2(2))-PAA(3)*(PIM1(3)-PIM2(3))
      DO 40 I=1,4
         
      VEC1(I)= PIM2(I)-PIM3(I) -PAA(I)*PROD1/XMAA**2
      VEC2(I)= PIM3(I)-PIM1(I) -PAA(I)*PROD2/XMAA**2
      VEC3(I)= PIM1(I)-PIM2(I) -PAA(I)*PROD3/XMAA**2
 40   VEC4(I)= PIM1(I)+PIM2(I)+PIM3(I)
      CALL PROD5(PIM1,PIM2,PIM3,VEC5)
* HADRON CURRENT
C be aware that sign of vec2 is opposite to sign of vec1 in a1 case
C Rationalize this code:
      F1 = CMPLX(COEF(1,MNUM))*FORM1(MNUM,XMAA**2,XMRO1**2,XMRO2**2)
      F2 = CMPLX(COEF(2,MNUM))*FORM2(MNUM,XMAA**2,XMRO2**2,XMRO1**2)
      F3 = CMPLX(COEF(3,MNUM))*FORM3(MNUM,XMAA**2,XMRO3**2,XMRO1**2)
      F4 = (-1.0*UROJ)*
     $CMPLX(COEF(4,MNUM))*FORM4(MNUM,XMAA**2,XMRO1**2,XMRO2**2,XMRO3**2)
      F5 = (-1.0)*UROJ/4.0/PI**2/FPI**2*
     $     CMPLX(COEF(5,MNUM))*FORM5(MNUM,XMAA**2,XMRO1**2,XMRO2**2)
!      if (mnum.eq.9) write(*,*) 'effy=', mnum,'>>',f1,f2,f3,f4,f5
!      if (mnum.eq.9) write(*,*) 'coef=', mnum,'>>',COEF(1,MNUM),COEF(2,MNUM),COEF(3,MNUM),COEF(4,MNUM),COEF(5,MNUM)

      DO 45 I=1,4
      HADCUR(I)= CMPLX(FNORM(MNUM)) * (
     $  CMPLX(VEC1(I))*F1+CMPLX(VEC2(I))*F2+CMPLX(VEC3(I))*F3+
     $  CMPLX(VEC4(I))*F4+CMPLX(VEC5(I))*F5)
 45   CONTINUE

      END

      SUBROUTINE DAM3PI(MNUM,PT,PN,PIM1,PIM2,PIM3,AMPLIT,HV)
C ----------------------------------------------------------------------
* CALCULATES DIFFERENTIAL CROSS SECTION AND POLARIMETER VECTOR
* FOR TAU DECAY INTO K K pi, K pi pi.
* ALL SPIN EFFECTS IN THE FULL DECAY CHAIN ARE TAKEN INTO ACCOUNT.
* CALCULATIONS DONE IN TAU REST FRAME WITH Z-AXIS ALONG NEUTRINO MOMENT
C MNUM DECAY MODE IDENTIFIER.
C

C     called by : DPHSAA

C ----------------------------------------------------------------------
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4)
      REAL  PIVEC(4),PIAKS(4),HVM(4)
      COMPLEX HADCUR(4)
      IME=IMEGET(3,MNUM)   ! type of matrix element to be used
      IF (IME.EQ.0) THEN
         AMPLIT=GFERMI**2
         DO  I=1,3
          HV(I)=0.0
         ENDDO
      ELSEIF (IME.EQ.1) THEN
         AMPLIT=GFERMI**2
         DO  I=1,3
          HV(I)=0.0
         ENDDO
      ELSEIF (IME.EQ.3) THEN
C omega is a special case because of sum over photon spin states
        CALL DAMPOG(PT,PN,PIM1,PIM2,PIM3,AMPLIT,HV)
        RETURN
       ELSEIF (IME.EQ.5) THEN
C wrapper
        CALL DAM3PI_wrap(MNUM,PT,PN,PIM1,PIM2,PIM3,AMPLIT,HV)
        RETURN
      ELSEIF(IME.EQ.2.OR.IME.EQ.4) THEN
       IF(IME.EQ.4 ) THEN
        CALL CURR3PI_wrap(MNUM,PIM1,PIM2,PIM3,HADCUR)
       ELSE
        CALL CURR3PI(MNUM,PIM1,PIM2,PIM3,HADCUR)
       ENDIF
C
* CALCULATE PI-VECTORS: VECTOR AND AXIAL
       CALL CLVEC(HADCUR,PN,PIVEC)
       CALL CLAXI(HADCUR,PN,PIAKS)
       CALL CLNUT(HADCUR,BRAKM,HVM)
* SPIN INDEPENDENT PART OF DECAY DIFF-CROSS-SECT. IN TAU REST  FRAME
       BRAK= (GV**2+GA**2)*PT(4)*PIVEC(4) +2.*GV*GA*PT(4)*PIAKS(4)
     &      +2.*(GV**2-GA**2)*AMNUTA*AMTAU*BRAKM
       AMPLIT=(GFERMI)**2*BRAK/2.
       IF (MNUM.GE.49) THEN   ! dead code under this if ! 20->31->47 (May 2020) -> 49 (Oct 2020) [SwB]
         PRINT *, 'MNUM=',MNUM
         ZNAK=-1.0
         XM1=0.0
         XM2=0.0
         XM3=0.0
         DO 77 K=1,4
         IF (K.EQ.4) ZNAK=1.0
         XM1=ZNAK*PIM1(K)**2+XM1
         XM2=ZNAK*PIM2(K)**2+XM2
         XM3=ZNAK*PIM3(K)**2+XM3
 77      PRINT *, 'PIM1=',PIM1(K),'PIM2=',PIM2(K),'PIM3=',PIM3(K)
         PRINT *, 'XM1=',SQRT(XM1),'XM2=',SQRT(XM2),'XM3=',SQRT(XM3)
         PRINT *, '************************************************'
       ENDIF
C POLARIMETER VECTOR IN TAU REST FRAME
       DO 90 I=1,3
       HV(I)=-(AMTAU*((GV**2+GA**2)*PIAKS(I)+2.*GV*GA*PIVEC(I)))
     &      +(GV**2-GA**2)*AMNUTA*AMTAU*HVM(I)
C HV IS DEFINED FOR TAU-    WITH GAMMA=B+HV*POL
       HV(I)=-HV(I)/BRAK
 90    CONTINUE
      ENDIF
      END
      SUBROUTINE DADNEW(MODE,ISGN,HV,PNU,PWB,PNPI,JNPI)
C ----------------------------------------------------------------------
      include 'TAUDCD_BBB_size.inc'
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER

      COMMON / INOUT / INUT,IOUT

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
 
      REAL*4 PNU(4),PWB(4),PNPI(4,9),HV(4),HHV(4)
      REAL*4 PDUM1(4),PDUM2(4),PDUMI(4,9)
      REAL*4 RRR(3)
      REAL*4 WTMAX(NMODE)
      REAL*8              SWT(NMODE),SSWT(NMODE)
      INTEGER*8 NEVRAW(NMODE),NEVOVR(NMODE),NEVACC(NMODE)
C
      DATA PI /3.141592653589793238462643/
      DATA IWARM/0/
C
      IF(MODE.EQ.-1) THEN
C     ===================
C -- AT THE MOMENT ONLY TWO DECAY MODES OF MULTIPIONS HAVE M. ELEM
        NMOD=NMODE
        IWARM=1
C       PRINT 7003
        DO 1 JNPI=1,NMOD
        NEVRAW(JNPI)=0
        NEVACC(JNPI)=0
        NEVOVR(JNPI)=0
        SWT(JNPI)=0
        SSWT(JNPI)=0
        WTMAX(JNPI)=-1.

C for 4pi phase space, need lots more trials at initialization,
C or use the WTMAX determined with many trials for default model:
        NTRIALS = 5000
C cleo knew nothing about channels recently introduced
        IF (JNPI.LE.4) THEN
C         11.Oct.11: fix for BINP and KARLSRUHE currents added
          WTMAX(JNPI) = PKORB(3,37+JNPI)
          NTRIALS = 20000
        END IF
!       write(*,*) 'jnpi=',jnpi, ntrials, a
        DO  I=1,NTRIALS

          IF    (JNPI.LE.0) THEN
            GOTO 903 
          ELSEIF(JNPI.LE.NM4) THEN 
            CALL DPH4PI(WT,HV,PDUM1,PDUM2,PDUMI,JNPI)
!            IF (I.eq.1) write(*,*) '4 pi jnpi=',jnpi
          ELSEIF(JNPI.LE.NM4+NM5) THEN
             CALL DPH5PI(WT,HV,PDUM1,PDUM2,PDUMI,JNPI)
          ELSEIF(JNPI.LE.NM4+NM5+NM6) THEN
            CALL DPHNPI(WT,HV,PDUM1,PDUM2,PDUMI,JNPI)
          ELSEIF(JNPI.LE.NM4+NM5+NM6+NM3) THEN
            CALL DPH3PI(WT,HV,PDUM1,PDUM2,PDUMI,JNPI)
          ELSEIF(JNPI.LE.NM4+NM5+NM6+NM3+NM2) THEN
            CALL DPH2PI(WT,HV,PDUM1,PDUM2,PDUMI,JNPI)
          ELSEIF(JNPI.LE.NM4+NM5+NM6+NM3+NM2+NM1) THEN
            CALL DPH1PI(WT,HV,PDUM1,PDUM2,PDUMI,JNPI)
          ELSE
           GOTO 903
          ENDIF   
        IF(WT.GT.WTMAX(JNPI)/1.2) WTMAX(JNPI)=WT*1.2
        ENDDO

C       PRINT *,' DADNEW JNPI,NTRIALS,WTMAX =',JNPI,NTRIALS,WTMAX(JNPI)

C       CALL HBOOK1(801,'WEIGHT DISTRIBUTION  DADNPI    $',100,0.,2.,.0)
C       PRINT 7004,WTMAX(JNPI)
1       CONTINUE
        WRITE(IOUT,7005)
C
      ELSEIF(MODE.EQ. 0) THEN
C     =======================
        IF(IWARM.EQ.0) GOTO 902
C
300     CONTINUE
          IF    (JNPI.LE.0) THEN
            GOTO 903 
          ELSEIF(JNPI.LE.NM4) THEN
             CALL DPH4PI(WT,HHV,PNU,PWB,PNPI,JNPI)
          ELSEIF(JNPI.LE.NM4+NM5) THEN
             CALL DPH5PI(WT,HHV,PNU,PWB,PNPI,JNPI)
          ELSEIF(JNPI.LE.NM4+NM5+NM6) THEN
            CALL DPHNPI(WT,HHV,PNU,PWB,PNPI,JNPI) 
          ELSEIF(JNPI.LE.NM4+NM5+NM6+NM3) THEN
            CALL DPH3PI(WT,HHV,PNU,PWB,PNPI,JNPI)
          ELSEIF(JNPI.LE.NM4+NM5+NM6+NM3+NM2) THEN
            CALL DPH2PI(WT,HHV,PNU,PWB,PNPI,JNPI)
          ELSEIF(JNPI.LE.NM4+NM5+NM6+NM3+NM2+NM1) THEN
            CALL DPH1PI(WT,HHV,PNU,PWB,PNPI,JNPI)
           ELSE
           GOTO 903
          ENDIF   
            DO I=1,4
              HV(I)=-ISGN*HHV(I)
            ENDDO
C       CALL HFILL(801,WT/WTMAX(JNPI))
        NEVRAW(JNPI)=NEVRAW(JNPI)+1
        SWT(JNPI)=SWT(JNPI)+WT

cccM.S.>>>>>>
cc        SSWT(JNPI)=SSWT(JNPI)+WT**2
        SSWT(JNPI)=SSWT(JNPI)+dble(WT)**2
cccM.S.<<<<<<

        CALL KKMC_RANMAR(RRR,3)
        RN=RRR(1)
        IF(WT.GT.WTMAX(JNPI)) NEVOVR(JNPI)=NEVOVR(JNPI)+1
        IF(RN*WTMAX(JNPI).GT.WT) GOTO 300
C ROTATIONS TO BASIC TAU REST FRAME
        COSTHE=-1.+2.*RRR(2)
        THET=ACOS(COSTHE)
        PHI =2*PI*RRR(3)
        CALL ROTOR2(THET,PNU,PNU)
        CALL ROTOR3( PHI,PNU,PNU)
        CALL ROTOR2(THET,PWB,PWB)
        CALL ROTOR3( PHI,PWB,PWB)
        CALL ROTOR2(THET,HV,HV)
        CALL ROTOR3( PHI,HV,HV)
        ND=MULPIK(JNPI)
        DO 301 I=1,ND
        CALL ROTOR2(THET,PNPI(1,I),PNPI(1,I))
        CALL ROTOR3( PHI,PNPI(1,I),PNPI(1,I))
301     CONTINUE
        NEVACC(JNPI)=NEVACC(JNPI)+1
C
      ELSEIF(MODE.EQ. 1) THEN
C     =======================
        DO 500 JNPI=1,NMOD
          IF(NEVRAW(JNPI).EQ.0) GOTO 500
          PARGAM=SWT(JNPI)/FLOAT(NEVRAW(JNPI)+1)
          ERROR=0
          IF(NEVRAW(JNPI).NE.0)
     &    ERROR=SQRT(ABS(SSWT(JNPI)/SWT(JNPI)**2-1./FLOAT(NEVRAW(JNPI))))
          RAT=PARGAM/GAMEL
          WRITE(IOUT, 7010) NLT+JNPI,NAMES(JNPI),
     &     NEVRAW(JNPI),NEVACC(JNPI),NEVOVR(JNPI),PARGAM,RAT,ERROR
CC        CALL HPRINT(801)
          GAMPMC(NLT+JNPI)=RAT
          GAMPER(NLT+JNPI)=ERROR
CAM       NEVDEC(NLT+JNPI)=NEVACC(JNPI)
  500     CONTINUE
      ENDIF
C     =====
      RETURN
 7003 FORMAT(///1X,15(5H*****)
     $ /,' *',     25X,'******** DADNEW INITIALISATION ********',9X,1H*
     $ )
 7004 FORMAT(' *',E20.5,5X,'WTMAX  = MAXIMUM WEIGHT  ',9X,1H*/)
 7005 FORMAT(
     $  /,1X,15(5H*****)/)
 7010 FORMAT(///1X,15(5H*****)
     $ /,' *',     25X,'******** DADNEW FINAL REPORT  ******** ',9X,1H*
     $ /,' *',     25X,'CHANNEL',I4,': ',A31                    ,4X,1H*
     $ /,' *',I20  ,5X,'NEVRAW = NO. OF DECAYS TOTAL           ',9X,1H*
     $ /,' *',I20  ,5X,'NEVACC = NO. OF DECAYS ACCEPTED        ',9X,1H*
     $ /,' *',I20  ,5X,'NEVOVR = NO. OF OVERWEIGHTED EVENTS    ',9X,1H*
     $ /,' *',E20.5,5X,'PARTIAL WTDTH IN GEV UNITS             ',9X,1H*
     $ /,' *',F20.9,5X,'IN UNITS GFERMI**2*MASS**5/192/PI**3   ',9X,1H*
     $ /,' *',F20.8,5X,'RELATIVE ERROR OF PARTIAL WIDTH        ',9X,1H*
     $  /,1X,15(5H*****)/)
 902  WRITE(IOUT, 9020)
 9020 FORMAT(' ----- DADNEW: LACK OF INITIALISATION')
      STOP
 903  WRITE(IOUT, 9030) JNPI,MODE
 9030 FORMAT(' ----- DADNEW: WRONG JNPI',2I5)
      STOP
      END

      SUBROUTINE DPH1PI(WT,HV,PNU,PWB,PNPI,JNPI)
C ----------------------------------------------------------------------
C FZ
      IMPLICIT NONE
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL

      COMMON / PARMASD / AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
C
      REAL*8            AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
      include 'TAUDCD_BBB_size.inc'
      
      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
      
      REAL*8  ENU,XKK,EKK,PKK(4),PI
      
C --- Variables below are arguments of this or other routines
      REAL*4  PNU(4),HV(4),PNPI(4,9),PWB(4),WT
      REAL*4  PKKR(4),GAMM,AMF1,AMF0,DCDMAS
      
      INTEGER  JNPI,IDFFIN,MULPIK,INUM,I
    
      DATA PI /3.141592653589793238462643/
      INUM=JNPI-NM4-NM5-NM6-NM3-NM2

      AMF1= DCDMAS(IDFFIN(1,JNPI),4)
      AMF0=AMNUTAD
      IF(INUM.GE.3.AND.IDFFIN(3,JNPI).NE.0) AMF0= DCDMAS(IDFFIN(3,JNPI),4)
      IF(INUM.GE.3.AND.IDFFIN(4,JNPI).NE.0) AMF0= DCDMAS(IDFFIN(4,JNPI),4)
        EKK= (AMTAUD**2+AMF1**2-AMF0**2)/(2*AMTAUD)
        ENU= (AMTAUD**2-AMF1**2+AMF0**2)/(2*AMTAUD)
        XKK= SQRT(EKK**2-AMF1**2)
C K MOMENTUM
        CALL SPHERD(XKK,PKK)
        PKK(4)=EKK
C TAU-NEUTRINO MOMENTUM
        DO I=1,4
        PNU(I)=-PKK(I)
        PKKR(I)=PKK(I)
        ENDDO
        PNU(4)=ENU

 
       CALL DAM1PI(INUM,PNU,AMF0,PKKR,AMF1,GAMM,HV)
       WT=GAMM
       DO I=1,4
         PNPI(I,1)=PKK(I)
       ENDDO

      END

      SUBROUTINE DAM1PI(INUM,PNU,AMF0,PKK,AMF1,GAMM,HV)

      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST

      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL  PKK(4),PNU(4),HV(4)
      DATA PI /3.141592653589793238462643/

       IME=IMEGET(1,INUM) 

       IF (IME.EQ.2.OR.IME.EQ.4) THEN
        EKK=PKK(4)
        ENU=PNU(4)
        PXQ=AMTAU*EKK
        PXN=AMTAU*ENU
        QXN=PKK(4)*PNU(4)-PKK(1)*PNU(1)-PKK(2)*PNU(2)-PKK(3)*PNU(3)
        BRAK=(GV**2+GA**2)*(2*PXQ*QXN-AMF1**2*PXN)
     &      +(GV**2-GA**2)*AMTAU*AMF0*AMF1**2
        DO 40 I=1,3
40      HV(I)=2*GA*GV*AMTAU*(2*PKK(I)*QXN-PNU(I)*AMF1**2)/BRAK
        HV(4)=1.0

C WARNING: 2-BODY PHASE SPACE FACTOR IS INCLUDED !

       IF (IME.EQ.2) THEN
        IF (INUM.EQ.1) THEN  ! pi nu 
         FPI=0.1284
         GAMM=(GFERMI*FPI)**2/(16.*PI)*AMTAU**3*
     $        (BRAK/AMTAU**4)*
     $        SQRT((AMTAU**2-AMF1**2-AMF0**2)**2
     $             -4*AMF1**2*AMF0**2           )/AMTAU**2

        ELSEIF (INUM.EQ.2) THEN  ! K nu
         FKK=0.0354
         GAMM=(GFERMI*FKK)**2/(16.*PI)*AMTAU**3*
     $        (BRAK/AMTAU**4)*
     $        SQRT((AMTAU**2-AMF1**2-AMF0**2)**2
     $             -4*AMF1**2*AMF0**2           )/AMTAU**2
        ELSE 
C optional non-sm ME or dalitz plot enhancements etc. 
C may be  be installed here for some values of MNUM.
C CALL ALTERN1(INUM,PNU,PKK,GAMM,HV)

         GAMM=GFERMI**2
         DO  I=1,3
          HV(I)=0.0
         ENDDO
         HV(4)=1.0
        ENDIF

       ELSEIF (IME.EQ.4) THEN
        FKK=FCONST_wrap(INUM) ! to be filled in by C++ 
         GAMM=(GFERMI*FKK)**2/(16.*PI)*AMTAU**3*
     $        (BRAK/AMTAU**4)*
     $        SQRT((AMTAU**2-AMF1**2-AMF0**2)**2
     $             -4*AMF1**2*AMF0**2           )/AMTAU**2
       ENDIF
       ELSEIF (IME.EQ.5) THEN       
          CALL DAM1PI_wrap(INUM,PNU,AMF0,PKK,AMF1,GAMM,HV)
          GAMM=1./(16.*PI)*AMTAU**3*
     $        (GAMM/AMTAU**4)*
     $        SQRT((AMTAU**2-AMF1**2-AMF0**2)**2
     $             -4*AMF1**2*AMF0**2           )/AMTAU**2
         RETURN
       ELSEIF (IME.EQ.0) THEN    ! not initialized
         GAMM=GFERMI**2
         DO  I=1,3
          HV(I)=0.0
         ENDDO
         HV(4)=1.0
       ELSE    ! any other; flat phase space
         GAMM=GFERMI**2
         DO  I=1,3
          HV(I)=0.0
         ENDDO
         HV(4)=1.0
       ENDIF

      END
 

      SUBROUTINE CHOICE4(MNUM,xPROB1,xPROB2,xAMRX,xGAMRX,xAMRA,xGAMRA)
      include 'TAUDCD_BBB_size.inc'
      COMMON /SAMPL4/ PROB1(NM4),PROB2(NM4),AMRX(NM4),GAMRX(NM4),AMRA(NM4),GAMRA(NM4)
       xPROB1=PROB1(MNUM)
       xPROB2=PROB2(MNUM)
       xAMRX =AMRX (MNUM)
       xGAMRX=GAMRX(MNUM)
       xAMRA =AMRA (MNUM)
       xGAMRA=GAMRA(MNUM)

      END

      SUBROUTINE DPH4PI(DGAMT,HV,PNR,PAAR,PMULT,JNPI)
C ----------------------------------------------------------------------

* IT SIMULATES A1  DECAY IN TAU REST FRAME WITH
* Z-AXIS ALONG A1  MOMENTUM

C ----------------------------------------------------------------------
      IMPLICIT NONE
      COMMON / PARMASD / AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
C
      REAL*8            AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      include 'TAUDCD_BBB_size.inc'

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31


      REAL*8 PIM1(4),PIM2(4),PIPL(4),PIZ(4),PR(4),PN(4),PAA(4)
      REAL*8 PHSPAC,PHSP,EXE,THET,PHI,PPI,PPPI
      REAL*8 ENQ1,ENQ2,AMS1,AMS2,ALP1,ALP2,ALP
      REAL*8 AM1,AM2,AM3,AM4,AM4SQ,AM3SQ,AM2SQ
      REAL*8 AMP1,AMP2,AMP3,AMP4
      REAL*8 UU,FF,GG,FF1,FF2,FF3,FF4,GG1,GG2,GG3,GG4,RR ! RR no longer used for jacobian
      REAL*8 EE,UE,FE,GE,A1,A2,A3,A4,A5,A6

      REAL*4 RRR(11) ! RANMAR generates floats
      REAL*8 RR1,RR2,RR3,RR4,RR5,PREZ
      REAL*8 X,Y,Z,XLAM,PI
      REAL*8 PRC2, AMKUBA, GKUBA  ! locally defined presampling resonance 
      REAL*8 PRFIR,PRSEC ! probabilities of first and second channel in the mass of 3

C --- Variables below are arguments of this or other routines
      REAL*4 HV(4),PNR(4),PAAR(4),PMULT(4,9)
      REAL*4 PIZR(4),PIM1R(4),PIM2R(4),PIPLR(4),PT(4)
      REAL*4 AMPLIT, DGAMT, DCDMAS
      REAL*4 AMRX,GAMRX,AMROP,GAMROP,PROB1,PROB2

      INTEGER  JNPI,IDFFIN,MULPIK,ICONT,I
      
      DATA PI /3.141592653589793238462643/
      DATA ICONT /0/
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.0D0*Y*Z))
      PRC2=0.0d0     ! Probability for two scalars resonance presampler
      AMKUBA=0.001d0 ! Its mass
      GKUBA=0.001d0  ! and width
      
      PRFIR=0.5d0
      PRSEC=0.5d0
C --- Correction to generation channels for decays of different 1st and 2nd particle
      IF(IDFFIN(1,JNPI).ne.IDFFIN(2,JNPI)) THEN
       PRFIR=1.0d0 ! if first and second particles are different
       PRSEC=0.0d0 ! we use only channel using (2,3,4) in the mass of three
      ENDIF

C AMROD, GAMROD IS ONLY A PARAMETER FOR GETING HIGHT EFFICIENCY
C
C THREE BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)
      PHSPAC=1.d0/2**23/PI**11
      PHSP=1.d0/2**5/PI**2
      AMP1=DCDMAS(IDFFIN(1,JNPI),0)    ! mass for  PIM1
      AMP2=DCDMAS(IDFFIN(2,JNPI),0)    ! mass for  PIM2
      AMP3=DCDMAS(IDFFIN(3,JNPI),0)    ! mass for  PIPL
      AMP4=DCDMAS(IDFFIN(4,JNPI),0)    ! mass for  PIZ

      IF (AMP3+AMP4.LE.0.002d0) PRC2=0.8d0 ! only for electron pair in final state
 
      CALL CHOICE4(JNPI,PROB1,PROB2,AMROP,GAMROP,AMRX,GAMRX)

      PREZ=PROB1+PROB2
C TAU MOMENTUM
      PT(1)=0.
      PT(2)=0.
      PT(3)=0.
      PT(4)=AMTAUD
C
      CALL KKMC_RANMAR(RRR,11)
C
* MASSES OF 4, 3 AND 2 PI SYSTEMS
C 3 PI WITH SAMPLING FOR RESONANCE
CAM
      RR1=RRR(6)
      AMS1=(AMP1+AMP2+AMP3+AMP4)**2
      AMS2=(AMTAUD-AMNUTAD)**2
      ALP1=ATAN((AMS1-AMROP**2)/AMROP/GAMROP)
      ALP2=ATAN((AMS2-AMROP**2)/AMROP/GAMROP)
      ALP=ALP1+RR1*(ALP2-ALP1)
      AM4SQ =AMROP**2+AMROP*GAMROP*TAN(ALP)
      AM4 =SQRT(AM4SQ)
      PHSPAC=PHSPAC*
     $       ((AM4SQ-AMROP**2)**2+(AMROP*GAMROP)**2)/(AMROP*GAMROP)
      PHSPAC=PHSPAC*(ALP2-ALP1)
 
C
      RR1=RRR(1)
      AMS1=(AMP2+AMP3+AMP4)**2
      AMS2=(AM4-AMP1)**2
      IF (RRR(9).GT.PREZ) THEN
        AM3SQ=AMS1+   RR1*(AMS2-AMS1)
        AM3 =SQRT(AM3SQ)
C --- this part of jacobian will be recovered later
        FF1=AMS2-AMS1
      ELSE
* PHASE SPACE WITH SAMPLING FOR OMEGA RESONANCE,
        ALP1=ATAN((AMS1-AMRX**2)/AMRX/GAMRX)
        ALP2=ATAN((AMS2-AMRX**2)/AMRX/GAMRX)
        ALP=ALP1+RR1*(ALP2-ALP1)
        AM3SQ =AMRX**2+AMRX*GAMRX*TAN(ALP)
        AM3 =SQRT(AM3SQ)
C --- THIS PART OF THE JACOBIAN WILL BE RECOVERED LATER ---------------
        FF1=((AM3SQ-AMRX**2)**2+(AMRX*GAMRX)**2)/(AMRX*GAMRX)
        FF1=FF1*(ALP2-ALP1)
      ENDIF
C MASS OF 2
      RR2=RRR(2)
      AMS1=(AMP3+AMP4)**2
      AMS2=(AM3-AMP2)**2
      
      IF (RRR(10).GT.PRC2) THEN
* FLAT PHASE SPACE;
        AM2SQ=AMS1+   RR2*(AMS2-AMS1)
        AM2 =SQRT(AM2SQ)
C --- this part of jacobian will be recovered later
        FF2=(AMS2-AMS1)
      ELSE
* PHASE SPACE WITH SAMPLING FOR e-e+ PEAK;
        ALP1=ATAN((AMS1-AMKUBA**2)/AMKUBA/GKUBA)
        ALP2=ATAN((AMS2-AMKUBA**2)/AMKUBA/GKUBA)
        ALP=ALP1+RR2*(ALP2-ALP1)
        AM2SQ=AMKUBA**2+AMKUBA*GKUBA*TAN(ALP)
        AM2 =SQRT(AM2SQ)
C --- THIS PART OF THE JACOBIAN WILL BE RECOVERED LATER ---------------
        FF2=((AM2SQ-AMKUBA**2)**2+(AMKUBA*GKUBA)**2)/(AMKUBA*GKUBA)
        FF2=FF2*(ALP2-ALP1)
      ENDIF
*  2 RESTFRAME, DEFINE PIZ AND PIPL

      ENQ1=(AM2SQ-AMP3**2+AMP4**2)/(2*AM2)
      ENQ2=(AM2SQ+AMP3**2-AMP4**2)/(2*AM2)
      PPI=         ENQ1**2-AMP4**2
      PPPI=SQRT(ABS(ENQ1**2-AMP4**2))

      PHSPAC=PHSPAC*(4.d0*PI)*(2.d0*PPPI/AM2)

* PIZ   MOMENTUM IN 2 REST FRAME

      CALL SPHERD(PPPI,PIZ)
      PIZ(4)=ENQ1

* PIPL  MOMENTUM IN 2 REST FRAME

      DO 30 I=1,3
 30   PIPL(I)=-PIZ(I)
      PIPL(4)=ENQ2
* 3 REST FRAME, DEFINE PIM2

*       PR   MOMENTUM

      PR(1)=0.d0
      PR(2)=0.d0
      PR(4)=1.d0/(2.d0*AM3)*(AM3**2+AM2**2-AMP2**2)
      PR(3)= SQRT(ABS(PR(4)**2-AM2**2))
      PPI  =          PR(4)**2-AM2**2

*       PIM2  MOMENTUM

      PIM2(1)=0.d0
      PIM2(2)=0.d0
      PIM2(4)=1.d0/(2.d0*AM3)*(AM3**2-AM2**2+AMP2**2)
      PIM2(3)=-PR(3)
C --- this part of jacobian will be recovered later
      FF3=(4.d0*PI)*(2.d0*PR(3)/AM3)
* OLD PIONS BOOSTED FROM 2 REST FRAME TO 3 REST FRAME
      EXE=(PR(4)+PR(3))/AM2
      CALL BOSTD3(EXE,PIZ,PIZ)
      CALL BOSTD3(EXE,PIPL,PIPL)
      RR3=RRR(3)
      RR4=RRR(4)
      THET =ACOS(-1.d0+2.d0*RR3)
      PHI = 2.d0*PI*RR4
      CALL ROTPOX(THET,PHI,PIPL)
      CALL ROTPOX(THET,PHI,PIM2)
      CALL ROTPOX(THET,PHI,PIZ)
c      CALL ROTPOX(THET,PHI,PR) 

* 4  REST FRAME, DEFINE PIM1
*       PR   MOMENTUM

      PR(1)=0.d0
      PR(2)=0.d0
      PR(4)=1.d0/(2.d0*AM4)*(AM4**2+AM3**2-AMP1**2)
      PR(3)= SQRT(ABS(PR(4)**2-AM3**2))
      PPI  =          PR(4)**2-AM3**2

*       PIM1 MOMENTUM 

      PIM1(1)=0.d0
      PIM1(2)=0.d0
      PIM1(4)=1.d0/(2.d0*AM4)*(AM4**2-AM3**2+AMP1**2)
      PIM1(3)=-PR(3)
C --- this part of jacobian will be recovered later
      FF4=(4.d0*PI)*(2.d0*PR(3)/AM4)
* OLD PIONS BOOSTED FROM 3 REST FRAME TO 4 REST FRAME
      EXE=(PR(4)+PR(3))/AM3
      CALL BOSTD3(EXE,PIZ,PIZ)
      CALL BOSTD3(EXE,PIPL,PIPL)
      CALL BOSTD3(EXE,PIM2,PIM2)
      RR3=RRR(7)
      RR4=RRR(8)
      THET =ACOS(-1.d0+2.d0*RR3)
      PHI = 2.d0*PI*RR4
      CALL ROTPOX(THET,PHI,PIPL)
      CALL ROTPOX(THET,PHI,PIM1)
      CALL ROTPOX(THET,PHI,PIM2)
      CALL ROTPOX(THET,PHI,PIZ)
c      CALL ROTPOX(THET,PHI,PR)
C
* NOW TO THE TAU REST FRAME, DEFINE PAA AND NEUTRINO MOMENTA
* PAA  MOMENTUM
      PAA(1)=0.d0
      PAA(2)=0.d0
      PAA(4)=1.d0/(2.d0*AMTAUD)*(AMTAUD**2-AMNUTAD**2+AM4**2)
      PAA(3)= SQRT(ABS(PAA(4)**2-AM4**2))
      PPI    =          PAA(4)**2-AM4**2
      PHSPAC =PHSPAC*(4.d0*PI)*(2.d0*PAA(3)/AMTAUD)
      PHSP   =PHSP*(4.d0*PI)*(2.d0*PAA(3)/AMTAUD)    

* TAU-NEUTRINO MOMENTUM
      PN(1)=0.d0
      PN(2)=0.d0
      PN(4)=1.d0/(2.d0*AMTAUD)*(AMTAUD**2+AMNUTAD**2-AM4**2)
      PN(3)=-PAA(3)

C ZBW 20.12.2002 bug fix
C JZ  17.07.2019 symmetrization does not apply for particles of different mass
C JZ  27.08.2019 IF(AMP1.eq.AMP2) THEN  !  early version improved. 
      IF(IDFFIN(1,JNPI).eq.IDFFIN(2,JNPI)) THEN
        IF(RRR(9).LE.0.5*PREZ) THEN
          DO 72 I=1,4
          X=PIM1(I)
          PIM1(I)=PIM2(I)
 72       PIM2(I)=X
        ENDIF
      ENDIF
C end of bug fix
C WE INCLUDE REMAINING PART OF THE JACOBIAN
C --- FLAT CHANNEL of 3
      AM3SQ=(PIM2(4)+PIZ(4)+PIPL(4))**2-(PIM2(3)+PIZ(3)+PIPL(3))**2
     $     -(PIM2(2)+PIZ(2)+PIPL(2))**2-(PIM2(1)+PIZ(1)+PIPL(1))**2
      AMS2=(AM4-AMP1)**2
      AMS1=(AMP2+AMP3+AMP4)**2
      FF1=(AMS2-AMS1)

      AMS1=(AMP3+AMP4)**2
      AMS2=(SQRT(AM3SQ)-AMP2)**2
      FF2=AMS2-AMS1
      ALP1=ATAN((AMS1-AMKUBA**2)/AMKUBA/GKUBA)
      ALP2=ATAN((AMS2-AMKUBA**2)/AMKUBA/GKUBA)
      AM2SQ=(PIZ(4)+PIPL(4))**2-(PIZ(3)+PIPL(3))**2
     $     -(PIZ(2)+PIPL(2))**2-(PIZ(1)+PIPL(1))**2

      EE=((AM2SQ-AMKUBA**2)**2+(AMKUBA*GKUBA)**2)/(AMKUBA*GKUBA) ! Jacobian for pair
      EE=EE*(ALP2-ALP1)

      FF3=(4.d0*PI)*(XLAM(AM2SQ,AMP2**2,AM3SQ)/AM3SQ)
      FF4=(4.d0*PI)*(XLAM(AM3SQ,AMP1**2,AM4**2)/AM4**2)
      UU=FF1*FF2*FF3*FF4

      UE=FF1*EE*FF3*FF4 ! jacobian for 3 and 2: end of ``FLAT CHANNEL of 3''

C --- FIRST CHANNEL of 3
      AM3SQ=(PIM2(4)+PIZ(4)+PIPL(4))**2-(PIM2(3)+PIZ(3)+PIPL(3))**2
     $     -(PIM2(2)+PIZ(2)+PIPL(2))**2-(PIM2(1)+PIZ(1)+PIPL(1))**2
      AMS2=(AM4-AMP1)**2
      AMS1=(AMP2+AMP3+AMP4)**2
      ALP1=ATAN((AMS1-AMRX**2)/AMRX/GAMRX)
      ALP2=ATAN((AMS2-AMRX**2)/AMRX/GAMRX)
      FF1=((AM3SQ-AMRX**2)**2+(AMRX*GAMRX)**2)/(AMRX*GAMRX)
      FF1=FF1*(ALP2-ALP1)

      AMS1=(AMP3+AMP4)**2
      AMS2=(SQRT(AM3SQ)-AMP2)**2
      FF2=AMS2-AMS1
      ALP1=ATAN((AMS1-AMKUBA**2)/AMKUBA/GKUBA)
      ALP2=ATAN((AMS2-AMKUBA**2)/AMKUBA/GKUBA)
      AM2SQ=(PIZ(4)+PIPL(4))**2-(PIZ(3)+PIPL(3))**2
     $     -(PIZ(2)+PIPL(2))**2-(PIZ(1)+PIPL(1))**2
      EE=((AM2SQ-AMKUBA**2)**2+(AMKUBA*GKUBA)**2)/(AMKUBA*GKUBA) ! Jacobian for pair
      EE=EE*(ALP2-ALP1)

      FF3=(4.d0*PI)*(XLAM(AM2SQ,AMP2**2,AM3SQ)/AM3SQ)
      FF4=(4.d0*PI)*(XLAM(AM3SQ,AMP1**2,AM4**2)/AM4**2)
      FF=FF1*FF2*FF3*FF4

      FE=FF1*EE*FF3*FF4 ! jacobian for 3 and 2: end of ``FIRST CHANNEL of 3"
C --- SECOND CHANNEL
      AM3SQ=(PIM1(4)+PIZ(4)+PIPL(4))**2-(PIM1(3)+PIZ(3)+PIPL(3))**2
     $     -(PIM1(2)+PIZ(2)+PIPL(2))**2-(PIM1(1)+PIZ(1)+PIPL(1))**2
      AMS2=(AM4-AMP2)**2
      AMS1=(AMP1+AMP3+AMP4)**2
      ALP1=ATAN((AMS1-AMRX**2)/AMRX/GAMRX)
      ALP2=ATAN((AMS2-AMRX**2)/AMRX/GAMRX)
      GG1=((AM3SQ-AMRX**2)**2+(AMRX*GAMRX)**2)/(AMRX*GAMRX)
      GG1=GG1*(ALP2-ALP1)
      AMS1=(AMP3+AMP4)**2
      AMS2=(SQRT(AM3SQ)-AMP1)**2
      GG2=AMS2-AMS1

      ALP1=ATAN((AMS1-AMKUBA**2)/AMKUBA/GKUBA)
      ALP2=ATAN((AMS2-AMKUBA**2)/AMKUBA/GKUBA)
      AM2SQ=(PIZ(4)+PIPL(4))**2-(PIZ(3)+PIPL(3))**2
     $     -(PIZ(2)+PIPL(2))**2-(PIZ(1)+PIPL(1))**2
      EE=((AM2SQ-AMKUBA**2)**2+(AMKUBA*GKUBA)**2)/(AMKUBA*GKUBA)
      EE=EE*(ALP2-ALP1)

      GG3=(4.d0*PI)*(XLAM(AM2SQ,AMP1**2,AM3SQ)/AM3SQ)
      GG4=(4.d0*PI)*(XLAM(AM3SQ,AMP2**2,AM4**2)/AM4**2)
      GG=GG1*GG2*GG3*GG4

      GE=GG1*EE*GG3*GG4  ! jacobian for 3 and 2: end of ``SECOND CHANNEL"
CC
CC      averaging over channels
CC        
C --- JACOBIAN AVERAGED OVER THE TWO
C     Old code. removed:   IF ( ( (FF+GG)*UU+FF*GG ).GT.0.0D0) THEN

      A1=0.0D0  ! now we have cascades so probablilites
      A2=0.0D0  ! for channels are products of 1-variable 
      A3=0.0D0  ! presampl. probablilities
      A4=0.0D0
      A5=0.0D0
      A6=0.0D0

C  --- Probabilities of decay channels divided by jacobians, ready for
C  --- harmonic average calculation. Eq. 20 from Tauola 2.4 documentation        
      IF (FF.NE.0.d0) A1=PRFIR*PREZ* (1.d0-PRC2)/FF
      IF (GG.NE.0.d0) A2=PRSEC*PREZ* (1.d0-PRC2)/GG
      IF (UU.NE.0.d0) A3=(1.d0-PREZ)*(1.d0-PRC2)/UU
      IF (FE.NE.0.d0) A4=PRFIR*PREZ* PRC2   /FE
      IF (GE.NE.0.d0) A5=PRSEC*PREZ* PRC2   /GE
      IF (UE.NE.0.d0) A6=(1.d0-PREZ)*PRC2   /UE

C  --- If channel is NOT used corresponding jacobians
C  --- may be prone to numerical instabilities
      IF (PREZ.eq.0.d0) THEN
        A1=0.0d0
        A2=0.0d0
        A4=0.0d0
        A5=0.0d0
      ENDIF
      IF (PRC2.eq.0.d0) THEN
        A4=0.0d0
        A5=0.0d0
        A6=0.0d0
      ENDIF
      IF (PREZ.eq.1.d0) THEN
        A3=0.0d0
        A6=0.0d0
      ENDIF
      IF (PRC2.eq.1.d0) THEN
        A1=0.0d0
        A2=0.0d0
        A3=0.0d0
      ENDIF
 
      IF (A1+A2+A3+A4+A5+A6.NE.0.0) THEN
        PHSPAC=PHSPAC/(A1+A2+A3+A4+A5+A6)
c          IF ( (0.5*PREZ*(FF+GG)*UU+(1.0-PREZ)*FF*GG).GT.0.0D0) THEN
c           RR=FF*GG*UU/(0.5*PREZ*(FF+GG)*UU+(1.0-PREZ)*FF*GG)
c           PHSPAC=PHSPAC*RR          
C           print *, JNPI, RR, 1/(A1+A2+A3+A4+A5+A6) !for prc2=0.0 RR=1/(A1+...+A6)
      ELSE
        PHSPAC=0.0d0
      ENDIF

* MOMENTA OF THE TWO PI-MINUS ARE RANDOMLY SYMMETRISED
      IF (JNPI.EQ.1) THEN
        RR5= RRR(5)
        IF(RR5.GT.0.5) THEN
          DO 70 I=1,4
          X=PIM1(I)
          PIM1(I)=PIM2(I)
 70       PIM2(I)=X
        ENDIF
        PHSPAC=PHSPAC/2.d0
      ELSEIF (JNPI.EQ.2) THEN
C MOMENTA OF PI0-S ARE GENERATED UNIFORMLY ONLY IF PREZ=0.0
        RR5= RRR(5)
        IF(RR5.GT.0.5) THEN
          DO 71 I=1,4
          X=PIM1(I)
          PIM1(I)=PIM2(I)
 71       PIM2(I)=X
        ENDIF
        PHSPAC=PHSPAC/6.d0
      ELSE
C        do nothing  ! .OR.JNPI.EQ.3.OR.JNPI.EQ.4
C        case of e-e-e+ nunu  will require new presampler because
C        there are two regions where photon is near real.
C        case e-e+ mu- nunu can be easily treated if order
C        mu- nu e-e+ nu is adopted.
      ENDIF
* ALL PIONS BOOSTED FROM  4  REST FRAME TO TAU REST FRAME
* Z-AXIS ANTIPARALLEL TO NEUTRINO MOMENTUM
      EXE=(PAA(4)+PAA(3))/AM4
      CALL BOSTD3(EXE,PIZ,PIZ)
      CALL BOSTD3(EXE,PIPL,PIPL)
      CALL BOSTD3(EXE,PIM1,PIM1)
      CALL BOSTD3(EXE,PIM2,PIM2)
c      CALL BOSTD3(EXE,PR,PR)
C PARTIAL WIDTH CONSISTS OF PHASE SPACE AND AMPLITUDE
C CHECK ON CONSISTENCY WITH DADNPI, THEN, CODE BREAKES UNIFORM PION
C DISTRIBUTION IN HADRONIC SYSTEM

CAM     Assume neutrino mass=0. and sum over final polarisation
C      AMX2=AM4**2
C      BRAK= 2*(AMTAUD**2-AMX2) * (AMTAUD**2+2.*AMX2)
C      AMPLIT=CCABIB**2*GFERMI**2/2. * BRAK * AMX2*SIGEE(AMX2,1)

      DO I=1,4           ! DAM4PI requires floats
        PNR(I)=PN(I)     ! PN and PAA arearguments of this routine
        PAAR(I)=PAA(I)   ! and have to remain float
        PIZR(I)=PIZ(I)
        PIM1R(I)=PIM1(I)
        PIM2R(I)=PIM2(I)
        PIPLR(I)=PIPL(I)
      ENDDO

      IF     (JNPI.EQ.1) THEN
        CALL DAM4PI(JNPI,PT,PNR,PIM1R,PIM2R,PIZR,PIPLR,AMPLIT,HV)
      ELSEIF (JNPI.EQ.2) THEN
        CALL DAM4PI(JNPI,PT,PNR,PIM1R,PIM2R,PIPLR,PIZR,AMPLIT,HV)
      ELSE
        CALL DAM4PI(JNPI,PT,PNR,PIM1R,PIM2R,PIPLR,PIZR,AMPLIT,HV) ! temporarily
      ENDIF

      DGAMT=1/(2.*AMTAUD)*AMPLIT*PHSPAC
C PHASE SPACE CHECK
C      DGAMT=PHSPAC

      DO I=1,4
         IF (JNPI.EQ.3.OR.JNPI.EQ.4) THEN ! eee or mumumu
            PMULT(I,1)=PIM1(I)
            PMULT(I,3)=PIPL(I)
            IF (RRR(11).GT.0.5) THEN
               PMULT(I,2)=PIM2(I)
               PMULT(I,4)=PIZ (I)
            ELSE
               PMULT(I,2)=PIZ (I)
               PMULT(I,4)=PIM2(I)
            ENDIF
         ELSE
            PMULT(I,1)=PIM1(I)
            PMULT(I,2)=PIM2(I)
            PMULT(I,3)=PIPL(I)
            PMULT(I,4)=PIZ (I)
         ENDIF
      ENDDO
      
 77   CONTINUE

      RETURN

      END

      SUBROUTINE DAM4PI(MNUM,PT,PN,PIM1,PIM2,PIM3,PIM4,AMPLIT,HV)
C ----------------------------------------------------------------------
* CALCULATES DIFFERENTIAL CROSS SECTION AND POLARIMETER VECTOR
* FOR TAU DECAY INTO 4 PI MODES
* ALL SPIN EFFECTS IN THE FULL DECAY CHAIN ARE TAKEN INTO ACCOUNT.
* CALCULATIONS DONE IN TAU REST FRAME WITH Z-AXIS ALONG NEUTRINO MOMENT
C MNUM DECAY MODE IDENTIFIER.
C

C     called by : DPHSAA

C ----------------------------------------------------------------------
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4),PIM4(4)
      REAL  PIVEC(4),PIAKS(4),HVM(4)
      COMPLEX HADCUR(4),HADCUR1(4),FORM1,FORM2,FORM3,FORM4,FORM5
      EXTERNAL FORM1,FORM2,FORM3,FORM4,FORM5
      DATA PI /3.141592653589793238462643/
      DATA ICONT /0/
      INTEGER IFCURR4PI
      COMMON /IFCURR4PI/ IFCURR4PI
      IME=IMEGET(4,MNUM)   ! type of matrix element to be used
C
!     write(*,*) 'falanti',mnum
      IF (IME.EQ.2) THEN
         IF     (MNUM.EQ.1.OR.MNUM.EQ.2) THEN ! MNUM
            IF(IFCURR4PI.EQ.0) THEN ! current choice between Karlsruhe (1), Novosibirsk(0)[default]
               IF (MNUM.EQ.1) THEN    !  2PI-,  PI+,  PI0 
                  CALL CURR_BINP(MNUM,1,PIM1,PIM4,PIM2,PIM3,HADCUR)
                  CALL CURR_BINP(MNUM,7,PIM1,PIM4,PIM2,PIM3,HADCUR1)
                  DO I=1,4
                     HADCUR(I) = HADCUR(I) + HADCUR1(I)
                  ENDDO
               ELSEIF (MNUM.EQ.2) THEN ! 3PI0,        PI-   
                  CALL CURR_BINP(MNUM,-1,PIM4,PIM3,PIM1,PIM2,HADCUR)
               ENDIF
            ELSE IF (IFCURR4PI.eq.1) THEN
               CALL CURR_KARLS(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
            ELSE IF (IFCURR4PI.eq.2) THEN
               CALL CURR(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
            ELSE IF (IFCURR4PI.EQ.3) THEN
               CALL CURR_CLEO(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
            ENDIF
         ELSE                   ! MNUM
            CALL CURR(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
         ENDIF                  ! MNUM
      ELSEIF (IME.EQ.4) THEN
         CALL CURR4_wrap(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
      ELSEIF (IME.EQ.5) THEN
         CALL DAM4PI_wrap(MNUM,PT,PN,PIM1,PIM2,PIM3,PIM4,AMPLIT,HV)
         RETURN
      ELSE
!       do nothing
      ENDIF  ! IME

      IF (IME.EQ.2.OR.IME.EQ.4) THEN   ! we use standard ME calc. also for IME=4 (user current)
C
* CALCULATE PI-VECTORS: VECTOR AND AXIAL
       CALL CLVEC(HADCUR,PN,PIVEC)
       CALL CLAXI(HADCUR,PN,PIAKS)
       CALL CLNUT(HADCUR,BRAKM,HVM)
* SPIN INDEPENDENT PART OF DECAY DIFF-CROSS-SECT. IN TAU REST  FRAME
       BRAK= (GV**2+GA**2)*PT(4)*PIVEC(4) +2.*GV*GA*PT(4)*PIAKS(4)
     &      +2.*(GV**2-GA**2)*AMNUTA*AMTAU*BRAKM
       AMPLIT=(CCABIB*GFERMI)**2*BRAK/2.
C POLARIMETER VECTOR IN TAU REST FRAME
       DO 90 I=1,3
       HV(I)=-(AMTAU*((GV**2+GA**2)*PIAKS(I)+2.*GV*GA*PIVEC(I)))
     &       +(GV**2-GA**2)*AMNUTA*AMTAU*HVM(I)
C HV IS DEFINED FOR TAU-    WITH GAMMA=B+HV*POL
       IF (BRAK.NE.0.0)
     & HV(I)=-HV(I)/BRAK
 90    CONTINUE

      ELSEIF (IME.EQ.0) THEN  ! not initialized
        DO  I=1,3
         HV(I)=0.0
        ENDDO
        AMPLIT= GFERMI**2       

      ELSEIF (IME.EQ.1) THEN ! flat phase space
C        FLAT PHASE SPACE ONLY;
        DO  I=1,3
         HV(I)=0.0
        ENDDO
        AMPLIT=GFERMI**2 
        RETURN
      ELSE
       write(*,*) 'DAM4PI: wrong IME= ',IME
        STOP
      ENDIF

      END
      SUBROUTINE DAM5PI(MNUM,PT,PN,PIM1,PIM2,PIM3,PIM4,PIM5,AMPLIT,HV)
C ----------------------------------------------------------------------
* CALCULATES DIFFERENTIAL CROSS SECTION AND POLARIMETER VECTOR
* FOR TAU DECAY INTO 4 PI MODES
* ALL SPIN EFFECTS IN THE FULL DECAY CHAIN ARE TAKEN INTO ACCOUNT.
* CALCULATIONS DONE IN TAU REST FRAME WITH Z-AXIS ALONG NEUTRINO MOMENT
C MNUM DECAY MODE IDENTIFIER.
C

C     called by : DPHSAA

C ----------------------------------------------------------------------
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4),PIM4(4),PIM5(4)
      REAL  PIVEC(4),PIAKS(4),HVM(4)
      COMPLEX HADCUR(4),FORM1,FORM2,FORM3,FORM4,FORM5
      EXTERNAL FORM1,FORM2,FORM3,FORM4,FORM5
      include 'TAUDCD_BBB_size.inc'
      DATA PI /3.141592653589793238462643/
      DATA ICONT /0/

      
      IME=IMEGET(5,MNUM)   ! type of matrix element to be used

C
!      write(*,*) 'falanti',mnum
      IF (IME.EQ.2) THEN
       IF     (MNUM.EQ.9.OR.MNUM.LE.6) THEN
        CALL CURR5(MNUM,PIM1,PIM2,PIM3,PIM4,PIM5,HADCUR)
       ELSE
         write(*,*) 'DAM5PI: wrong MNUM= ',MNUM
         STOP
       ENDIF
      ELSEIF (IME.EQ.4) THEN
        CALL CURR5_wrap(MNUM,PIM1,PIM2,PIM3,PIM4,PIM5,HADCUR)
      ELSEIF (IME.EQ.5) THEN
        CALL DAM5PI_wrap(MNUM,PT,PN,PIM1,PIM2,PIM3,PIM4,PIM5,AMPLIT,HV)
        RETURN
      ENDIF

      IF (IME.EQ.2.OR.IME.EQ.4) THEN
C
* CALCULATE PI-VECTORS: VECTOR AND AXIAL
       CALL CLVEC(HADCUR,PN,PIVEC)
       CALL CLAXI(HADCUR,PN,PIAKS)
       CALL CLNUT(HADCUR,BRAKM,HVM)
* SPIN INDEPENDENT PART OF DECAY DIFF-CROSS-SECT. IN TAU REST  FRAME
       BRAK= (GV**2+GA**2)*PT(4)*PIVEC(4) +2.*GV*GA*PT(4)*PIAKS(4)
     &      +2.*(GV**2-GA**2)*AMNUTA*AMTAU*BRAKM
       AMPLIT=(CCABIB*GFERMI)**2*BRAK/2.
C POLARIMETER VECTOR IN TAU REST FRAME
       DO 90 I=1,3
       HV(I)=-(AMTAU*((GV**2+GA**2)*PIAKS(I)+2.*GV*GA*PIVEC(I)))
     &       +(GV**2-GA**2)*AMNUTA*AMTAU*HVM(I)
C HV IS DEFINED FOR TAU-    WITH GAMMA=B+HV*POL
       IF (BRAK.NE.0.0)
     & HV(I)=-HV(I)/BRAK
 90    CONTINUE
      ELSEIF (IME.EQ.0) THEN  ! not initialized
        DO  I=1,3
         HV(I)=0.0
        ENDDO
        AMPLIT= GFERMI**2       

      ELSEIF (IME.EQ.1) THEN ! flat phase space
C        FLAT PHASE SPACE ONLY;
        DO  I=1,3
         HV(I)=0.0
        ENDDO
        AMPLIT=GFERMI**2 
        RETURN
      ELSE
       write(*,*) 'DAM5PI: wrong IME= ',IME
        STOP
      ENDIF

      END


      SUBROUTINE CHOICE5(INUM,xPROBa2,xPROBOM,xama2,xgama2,xAMOM,xGAMOM)
      include 'TAUDCD_BBB_size.inc'
      REAL*8          xAMOM,xGAMOM
      REAL*8           AMOM, GAMOM
      COMMON /SAMPL5/ PROBa2(NM5),PROBOM(NM5),ama2(NM5),gama2(NM5),AMOM(NM5),GAMOM(NM5)

      xPROBa2=PROBa2(INUM)
      xPROBOM=PROBOM(INUM)
      xama2=ama2(INUM)
      xgama2=gama2(INUM)
      xAMOM=AMOM(INUM)
      xGAMOM=GAMOM(INUM)

      END

      SUBROUTINE DPH5PI(DGAMT,HV,PNR,PAAR,PMULT,JNPI)                    
C ----------------------------------------------------------------------
* IT SIMULATES 5pi DECAY IN TAU REST FRAME WITH                         
* Z-AXIS ALONG 5pi MOMENTUM                                             
C ----------------------------------------------------------------------
      IMPLICIT NONE
      COMMON / PARMASD / AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD             
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D                
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD                           
C                                                                       
      REAL*8            AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD             
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D                
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD                           
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL                
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL                
      include 'TAUDCD_BBB_size.inc'

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31

      REAL*8 PT(4),PAA(4),PN(4),PPI,PPPI
      REAL*8 PR(4),PI1(4),PI2(4),PI3(4),PI4(4),PI5(4)
      REAL*8 AMP1,AMP2,AMP3,AMP4,AMP5,ams1,ams2
      REAL*8 AM5SQ,AM4SQ,AM3SQ,AM2SQ,AM5,AM4,AM3
      REAL*8 gg1,gg2,gg3,gg5,ff1,ff2,ff3,ff4,alp,alp1,alp2
      REAL*8 AM2,BRAK,ENQ1,ENQ2,PHSPAC
      REAL*8 EXE,THET,PHI,FOMPP,FNORM,PXN,PXR,QXN,PXQ
      
      REAL*4 RRR(12) ! RANMAR generates floats
      REAL*8 RR1,RR2,RR3,RR4
      REAL*8 XM,AM,GAMMA,PI,FPI

C --- Variables below are arguments of this or other routines
      REAL*4 HV(4),PNR(4),PAAR(4),PMULT(4,9) 
      REAL*4 PI1R(4),PI2R(4),PI3R(4),PI4R(4),PI5R(4),PTR(4)
      REAL*4 AMPLIT, DGAMT, DCDMAS
      REAL*4 ama2,gama2,PROBa2,PROBOM
      REAL*8 amom,gamom ! declared as REAL*8 in choice5
      
      INTEGER  JNPI,IDFFIN,MULPIK,ICONT,I,K,INUM
      
      DATA PI /3.141592653589793238462643/                              
      DATA ICONT /0/                                                    
      data fpi /93.3e-3/                                                
c                                                                       
      COMPLEX BWIGN                                                     
C                                                                     

      BWIGN(XM,AM,GAMMA)=XM**2/CMPLX(XM**2-AM**2,GAMMA*AM)            

 
      INUM=JNPI-NM4
 
C get parameters for presampler                                                 
      call choice5(INUM,PROBa2,PROBOM,ama2,gama2,AMOM,GAMOM)
c                                                                       
C 6 BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL                     
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)                            
      PHSPAC=1./2**29/PI**14                                            
c     PHSPAC=1./2**5/PI**2                                              
C init 5pi decay mode (JNPI)    
      AMP1=DCDMAS(IDFFIN(1,JNPI),0)
      AMP2=DCDMAS(IDFFIN(2,JNPI),0)
      AMP3=DCDMAS(IDFFIN(3,JNPI),0)
      AMP4=DCDMAS(IDFFIN(4,JNPI),0)
      AMP5=DCDMAS(IDFFIN(5,JNPI),0)
c                                                                       
C TAU MOMENTUM                                                          
      PT(1)=0.                                                          
      PT(2)=0.                                                          
      PT(3)=0.                                                          
      PT(4)=AMTAUD                                                                                                                             
      CALL KKMC_RANMAR(RRR,12)                                               
C                                                                       
c masses of 5, 4, 3 and 2 pi systems                                    
c 3 pi with sampling for omega resonance                                
cam                                                                     
c mass of 5   (12345)                     
       IF (RRR(11).GT.PROBa2) THEN                              
c  flat phase space:
        rr1=rrr(10)                                                       
        ams1=(amp1+amp2+amp3+amp4+amp5)**2                                
        ams2=(amtaud-amnutad)**2 
        alp1=atan((ams1-ama2**2)/ama2/gama2)                              
        alp2=atan((ams2-ama2**2)/ama2/gama2)                     
        am5sq=ams1+   rr1*(ams2-ams1)                                     
        am5 =sqrt(am5sq)                                                  
C      phspac=phspac*(ams2-ams1)  
c or peaked phase space  for a1(?) resonance: 
       ELSE
        rr1=rrr(10)                                                       
        ams1=(amp1+amp2+amp3+amp4+amp5)**2                                
        ams2=(amtaud-amnutad)**2    
        alp1=atan((ams1-ama2**2)/ama2/gama2)                              
        alp2=atan((ams2-ama2**2)/ama2/gama2)                              
        alp=alp1+rr1*(alp2-alp1)                                          
        am5sq =ama2**2+ama2*gama2*tan(alp)                                
        am5 =sqrt(am5sq)
       ENDIF                                                  
c --- these are two parts of jacobian, plugged here --------------- 
      gg5=((am5sq-ama2**2)**2+(ama2*gama2)**2)/(ama2*gama2)             
      gg5=gg5*(alp2-alp1)                          
      phspac=phspac/(PROBa2/gg5+(1D0-PROBa2)/(ams2-ams1) )               
c                                                                       
c mass of 4   (2345)                                                    
c flat phase space                                                      
      rr1=rrr(9)                                                        
      ams1=(amp2+amp3+amp4+amp5)**2                                     
      ams2=(am5-amp1)**2                                                
      am4sq=ams1+   rr1*(ams2-ams1)                                     
      am4 =sqrt(am4sq)                                                  
      gg1=ams2-ams1                   
c                                                                       
c mass of 3   (234)                                                     

       IF (RRR(12).LT.PROBom) THEN                    
C phase space with sampling for omega resonance     
        rr1=rrr(1)                                                        
        ams1=(amp2+amp3+amp4)**2                                          
        ams2=(am4-amp5)**2                                                
        alp1=atan((ams1-amom**2)/amom/gamom)                              
        alp2=atan((ams2-amom**2)/amom/gamom)                              
        alp=alp1+rr1*(alp2-alp1)                                          
        am3sq =amom**2+amom*gamom*tan(alp)                                
        am3 =sqrt(am3sq)                                                  
       ELSE                             
c flat phase space; 
        rr1=rrr(1)                                                        
        ams1=(amp2+amp3+amp4)**2                                          
        ams2=(am4-amp5)**2                                                
        alp1=atan((ams1-amom**2)/amom/gamom)                              
        alp2=atan((ams2-amom**2)/amom/gamom)                              
                                                   
        am3sq=ams1+   rr1*(ams2-ams1)                                     
        am3 =sqrt(am3sq)                                                  
c --- this part of jacobian will be recovered later                     
       ENDIF
c --- this part of the jacobian will be recovered later --------------- 
       gg2=((am3sq-amom**2)**2+(amom*gamom)**2)/(amom*gamom)             
       gg2=gg2*(alp2-alp1)   
       gg2=1D0/(PROBOM/gg2+(1D0-PROBOM)/(ams2-ams1))
c                                                                       
C mass of 2  (34)                                                       
      rr2=rrr(2)                                                        
      ams1=(amp3+amp4)**2                                               
      ams2=(am3-amp2)**2                                                
c flat phase space;                                                     
      am2sq=ams1+   rr2*(ams2-ams1)                                     
      am2 =sqrt(am2sq)                                                  
c --- this part of jacobian will be recovered later                     
      gg3=ams2-ams1                            
c                                                                       
c (34) restframe, define pi3 and pi4                                    
      enq1=(am2sq+amp3**2-amp4**2)/(2*am2)                              
      enq2=(am2sq-amp3**2+amp4**2)/(2*am2)                              
      ppi=          enq1**2-amp3**2                                     
      pppi=sqrt(abs(enq1**2-amp3**2))                                   
      ff1=(4*pi)*(2*pppi/am2)                                           
c pi3   momentum in (34) rest frame                                     
      call spherd(pppi,pi3)                                             
      pi3(4)=enq1                                                       
c pi4   momentum in (34) rest frame                                     
      do 30 i=1,3                                                       
 30   pi4(i)=-pi3(i)                                                    
      pi4(4)=enq2                                                       
c                                                                       
c (234) rest frame, define pi2                                          
c pr   momentum                                                         
      pr(1)=0.d0                                                        
      pr(2)=0.d0                                                        
      pr(4)=1.d0/(2*am3)*(am3**2+am2**2-amp2**2)                        
      pr(3)= sqrt(abs(pr(4)**2-am2**2))                                 
      ppi  =          pr(4)**2-am2**2                                   
c pi2   momentum                                                        
      pi2(1)=0.d0                                                       
      pi2(2)=0.d0                                                       
      pi2(4)=1.d0/(2.d0*am3)*(am3**2-am2**2+amp2**2)                    
      pi2(3)=-pr(3)                                                     
c --- this part of jacobian will be recovered later                     
      ff2=(4.d0*pi)*(2.d0*pr(3)/am3)                                    
c old pions boosted from 2 rest frame to 3 rest frame                   
      exe=(pr(4)+pr(3))/am2                                             
      call bostd3(exe,pi3,pi3)                                          
      call bostd3(exe,pi4,pi4)                                          
      rr3=rrr(3)                                                        
      rr4=rrr(4)                                                        
      thet =acos(-1.d0+2.d0*rr3)                                        
      phi = 2.d0*pi*rr4                                                 
      call rotpox(thet,phi,pi2)                                         
      call rotpox(thet,phi,pi3)                                         
      call rotpox(thet,phi,pi4)                                         
C                                                                       
C (2345)  rest frame, define pi5                                        
c pr   momentum                                                         
      pr(1)=0.d0                                                        
      pr(2)=0.d0                                                        
      pr(4)=1.d0/(2.d0*am4)*(am4**2+am3**2-amp5**2)                     
      pr(3)= sqrt(abs(pr(4)**2-am3**2))                                 
      ppi  =          pr(4)**2-am3**2                                   
c pi5  momentum                                                         
      pi5(1)=0.d0                                                       
      pi5(2)=0.d0                                                       
      pi5(4)=1.d0/(2.d0*am4)*(am4**2-am3**2+amp5**2)                    
      pi5(3)=-pr(3)                                                     
c --- this part of jacobian will be recovered later                     
      ff3=(4.d0*pi)*(2.d0*pr(3)/am4)                                    
c old pions boosted from 3 rest frame to 4 rest frame                   
      exe=(pr(4)+pr(3))/am3                                             
      call bostd3(exe,pi2,pi2)                                          
      call bostd3(exe,pi3,pi3)                                          
      call bostd3(exe,pi4,pi4)                                          
      rr3=rrr(5)                                                        
      rr4=rrr(6)                                                        
      thet =acos(-1.d0+2.d0*rr3)                                        
      phi = 2.d0*pi*rr4                                                 
      call rotpox(thet,phi,pi2)                                         
      call rotpox(thet,phi,pi3)                                         
      call rotpox(thet,phi,pi4)                                         
      call rotpox(thet,phi,pi5)                                         
C                                                                       
C (12345)  rest frame, define pi1                                       
c pr   momentum                                                         
      pr(1)=0.d0                                                        
      pr(2)=0.d0                                                        
      pr(4)=1.d0/(2.d0*am5)*(am5**2+am4**2-amp1**2)                     
      pr(3)= sqrt(abs(pr(4)**2-am4**2))                                 
      ppi  =          pr(4)**2-am4**2                                   
c pi1  momentum                                                         
      pi1(1)=0.d0                                                       
      pi1(2)=0.d0                                                       
      pi1(4)=1.d0/(2.d0*am5)*(am5**2-am4**2+amp1**2)                    
      pi1(3)=-pr(3)                                                     
c --- this part of jacobian will be recovered later                     
      ff4=(4.d0*pi)*(2.d0*pr(3)/am5)                                    
c old pions boosted from 4 rest frame to 5 rest frame                   
      exe=(pr(4)+pr(3))/am4                                             
      call bostd3(exe,pi2,pi2)                                          
      call bostd3(exe,pi3,pi3)                                          
      call bostd3(exe,pi4,pi4)                                          
      call bostd3(exe,pi5,pi5)                                          
      rr3=rrr(7)                                                        
      rr4=rrr(8)                                                        
      thet =acos(-1.d0+2.d0*rr3)                                        
      phi = 2.d0*pi*rr4                                                 
      call rotpox(thet,phi,pi1)                                         
      call rotpox(thet,phi,pi2)                                         
      call rotpox(thet,phi,pi3)                                         
      call rotpox(thet,phi,pi4)                                         
      call rotpox(thet,phi,pi5)                                         
c                                                                       
* now to the tau rest frame, define paa and neutrino momenta            
* paa  momentum                                                         
      paa(1)=0                                                          
      paa(2)=0                                                          
c     paa(4)=1./(2*amtaud)*(amtaud**2-amnutad**2+am5**2)                   
c     paa(3)= sqrt(abs(paa(4)**2-am5**2))                               
c     ppi   =          paa(4)**2-am5**2                                 
      paa(4)=1.d0/(2.d0*amtaud)*(amtaud**2-amnutad**2+am5sq)               
      paa(3)= sqrt(abs(paa(4)**2-am5sq))                                
      ppi   =          paa(4)**2-am5sq                                  
      phspac=phspac*(4.d0*pi)*(2*paa(3)/amtaud)                          
* tau-neutrino momentum                                                 
      pn(1)=0.d0                                                        
      pn(2)=0.d0                                                        
      pn(4)=1.d0/(2.d0*amtaud)*(amtaud**2+amnutad**2-am5**2)               
      pn(3)=-paa(3)                                                     
c                                                                       
      phspac=phspac * gg1*gg2*gg3*ff1*ff2*ff3*ff4                       
c                                                                       
C all pions boosted from  5  rest frame to tau rest frame               
C z-axis antiparallel to neutrino momentum                              
      exe=(paa(4)+paa(3))/am5                                           
      call bostd3(exe,pi1,pi1)                                          
      call bostd3(exe,pi2,pi2)                                          
      call bostd3(exe,pi3,pi3)                                          
      call bostd3(exe,pi4,pi4)                                          
      call bostd3(exe,pi5,pi5)                                          
c                                                                       
C partial width consists of phase space and amplitude                   
C AMPLITUDE  (cf YS.Tsai Phys.Rev.D4,2821(1971)                         
C    or F.Gilman SH.Rhie Phys.Rev.D31,1066(1985)                        
C                                                                       
      PXQ=AMTAUD*PAA(4)                                                  
      PXN=AMTAUD*PN(4)                                                   
      QXN=PAA(4)*PN(4)-PAA(1)*PN(1)-PAA(2)*PN(2)-PAA(3)*PN(3)           
      BRAK=2.d0*(GV**2+GA**2)*(2.d0*PXQ*QXN+AM5SQ*PXN)                  
     &    -6.d0*(GV**2-GA**2)*AMTAUD*AMNUTAD*AM5SQ                        
      fompp = cabs(bwign(am3,amom,gamom))**2                            
c normalisation factor (to some numerical undimensioned factor;         
c cf R.Fischer et al ZPhys C3, 313 (1980))                              
      fnorm = 1.d0/fpi**6                                               
c     AMPLIT=CCABIB**2*GFERMI**2/2. * BRAK * AM5SQ*SIGEE(AM5SQ,JNPI)    
      AMPLIT=CCABIB**2*GFERMI**2/2.d0 * BRAK !* (1D0*(jnpi-12))         
      amplit = amplit * fompp * fnorm                                   
c phase space test                                                      
c     amplit = amplit * fnorm                                           

!      write(*,*) '5pi jnpi=',jnpi                                  
c ignore spin terms                                                     
      DO 40 I=1,3                                                       
 40   HV(I)=0.       
                             
!      write(*,*) jnpi
!      stop

      DO I=1,4
        PTR(I)=PT(I)
        PNR(I)=PN(I)
        PAAR(I)=PAA(I)
        PI1R(I)=PI1(I)
        PI2R(I)=PI2(I)
        PI3R(I)=PI3(I)
        PI4R(I)=PI4(I)
        PI5R(I)=PI5(I)
      ENDDO

      if (INUM.gt.1) ! for the time being we want to keep old wrong m.e.
     $ CALL DAM5PI(INUM,PTR,PNR,PI1R,PI2R,PI3R,PI4R,PI5R,AMPLIT,HV)
      DGAMT=1/(2.*AMTAUD)*AMPLIT*PHSPAC
c                                                                       
      do 77 k=1,4                                                       
        pmult(k,1)=pi1(k)                                               
        pmult(k,2)=pi2(k)                                               
        pmult(k,3)=pi3(k)                                               
        pmult(k,4)=pi4(k)                                               
        pmult(k,5)=pi5(k)                                               
 77   continue                                                          
      return

C missing: transposition of identical particles, startistical factors 
C for identical matrices, polarimetric vector. Matrix element rather naive.

C flat phase space in pion system + with breit wigner for omega
C anyway it is better than nothing, and code is improvable.                                                  
      end         

      SUBROUTINE CHOICE2(INUM,xPROB1,xPROB2,xAM2,xGAM2,xAM3,xGAM3)
      include 'TAUDCD_BBB_size.inc'
      COMMON /SAMPL2/ PROB1(NM2),PROB2(NM2),AM2(NM2),GAM2(NM2),AM3(NM2),GAM3(NM2)


!        PROB(1) flat
!        PROB(2) K*
!        PROB(3) rho
        xAM2  = AM2  (INUM)
        xGAM2 = GAM2 (INUM)
        xAM3  = AM3  (INUM)
        xGAM3 = GAM3 (INUM)
        xPROB1= PROB1(INUM)
        xPROB2= PROB2(INUM)

      END
                                                    
      SUBROUTINE DPH2PI(DGAMT,HV,PNR,PRR,PMULT,JNPI)
C ----------------------------------------------------------------------
C IT SIMULATES RHO DECAY IN TAU REST FRAME WITH                         
C Z-AXIS ALONG RHO MOMENTUM                                             
C Rho decays to K Kbar       
C WARNING: DPH2PI routine is missing 2 scalar ME calculation                
C ----------------------------------------------------------------------
      IMPLICIT NONE
      COMMON / PARMASD / AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD             
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D                
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD                           
C                                                                       
      REAL*8            AMTAUD,AMNUTAD,AMELD,AMNUED,AMMUD,AMNUMUD             
     *                 ,AMPIZD,AMPID,AMROD,GAMROD,AMA1D,GAMA1D                
     *                 ,AMKD,AMKZD,AMKSTD,GAMKSTD                           
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL                
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL 
      include 'TAUDCD_BBB_size.inc'

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
    
      REAL*8  PKC(4),PKZ(4),PN(4),PR(4),QQ(4),A1,A2,A3
      REAL*8  AMF0,AMF1,AMF2
      REAL*8  ALP,ALP1,ALP1R,ALP2,ALP2R
      REAL*8  AMS1,AMS2,AMX,AMX2,ENQ1,ENQ2,EXE
      REAL*8  PHSPAC,PHSPAC1,PHSPAC2,PHSPAC3,PPPI
      
      REAL*4  RR1(1),RR2(1) ! RANMAR generates floats
      REAL*8  PI
      
C --- Variables below are arguments of this or other routines
      REAL*4  HV(4),PNR(4),PRR(4),PMULT(4,9)
      REAL*4  PT(4),PKCR(4),PKZR(4)
      REAL*4  AMPLIT,DGAMT,DCDMAS
      REAL*4  PROB1,PROB2,AM2,AM3,GAM2,GAM3

      INTEGER INUM,JNPI,IDFFIN,MULPIK,ICONT,I,K

      DATA PI /3.141592653589793238462643/                              
      DATA ICONT /0/                                                    
C       
      INUM=JNPI-NM3-NM4- NM5- NM6

      AMF1 = DCDMAS(IDFFIN(1,JNPI),0)
      AMF2 = DCDMAS(IDFFIN(2,JNPI),4)  
      AMF0 = AMNUTAD
!      IF(INUM.GT.4) THEN
!       write(*,*) inum,':',IDFFIN(1,IADDR),IDFFIN(2,IADDR),IDFFIN(3,IADDR),DCDMAS(IDFFIN(3,IADDR))
!      ENDIF
      IF(INUM.GT.3.AND.IDFFIN(3,JNPI).NE.0) AMF0= DCDMAS(IDFFIN(3,JNPI),4)
                   
C THREE BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL                 
      PHSPAC=1.d0/2**11/PI**5      
C TAU MOMENTUM                                                          
      PT(1)=0.                                                          
      PT(2)=0.                                                          
      PT(3)=0.                                                          
      PT(4)=AMTAUD                                                       
C MASS OF (REAL/VIRTUAL) RHO                                            
      AMS1=(AMF1+AMF2)**2                                                
      AMS2=(AMTAUD-AMF0)**2                                            
C FLAT PHASE SPACE                                                      
 100  CONTINUE                                                          
      CALL KKMC_RANMAR(RR1,1)                                                
      CALL CHOICE2(INUM,PROB1,PROB2,AM2,GAM2,AM3,GAM3)
      
      ALP1 =ATAN((AMS1-AM2**2)/AM2/GAM2)
      ALP2 =ATAN((AMS2-AM2**2)/AM2/GAM2)
      ALP1R=ATAN((AMS1-AM3**2)/AM3/GAM3)                              
      ALP2R=ATAN((AMS2-AM3**2)/AM3/GAM3)                              

      CALL KKMC_RANMAR(RR2,1)

      IF (RR2(1).LT.PROB1) THEN
C FLAT PHASE SPACE
        AMX2=AMS1+   RR1(1)*(AMS2-AMS1)
        AMX=SQRT(AMX2)
      ELSEIF(RR2(1).LT.PROB1+PROB2) THEN
C PHASE SPACE WITH SAMPLING FOR K* RESONANCE
        ALP=ALP1+RR1(1)*(ALP2-ALP1)
        AMX2=AM2**2+AM2*GAM2*TAN(ALP)
        AMX=SQRT(AMX2)
      ELSE
        ALP=ALP1R+RR1(1)*(ALP2R-ALP1R)                                          
        AMX2=AM3**2+AM3*GAM3*TAN(ALP)                                  
        AMX=SQRT(AMX2)  
      ENDIF
      IF(AMX.LE.(AMF1+AMF2)) GO TO 100 
      IF(AMX.GE.(AMTAUD-AMF0)) GO TO 100
C merging of the three channels
      PHSPAC1=(AMS2-AMS1)

      PHSPAC2=((AMX2-AM2**2)**2+(AM2*GAM2)**2)/(AM2*GAM2)
      PHSPAC2=PHSPAC2*(ALP2-ALP1)

      PHSPAC3=((AMX2-AM3**2)**2+(AM3*GAM3)**2)/(AM3*GAM3)    
      PHSPAC3=PHSPAC3*(ALP2R-ALP1R)                                         

      A1=0.d0
      A2=0.d0
      A3=0.d0
      IF (PHSPAC1.NE.0.d0) A1=PROB1          /PHSPAC1
      IF (PHSPAC2.NE.0.d0) A2=PROB2          /PHSPAC2
      IF (PHSPAC3.NE.0.d0) A3=(1-PROB1-PROB2)/PHSPAC3
      IF (A1+A2+A3.NE.0.d0) THEN
        PHSPAC=PHSPAC/(A1+A2+A3)
      ELSE
        PHSPAC=0.d0
      ENDIF
                               
      PN(1)=0.d0                                                           
      PN(2)=0.d0                                                           
      PN(4)=1.d0/(2.d0*AMTAUD)*(AMTAUD**2+AMF0**2-AMX**2)                    
      PN(3)=-SQRT((PN(4)-AMF0)*(PN(4)+AMF0))                        
C RHO MOMENTUM                                                          
      PR(1)=0.d0                                                           
      PR(2)=0.d0                                                           
      PR(4)=1.d0/(2.d0*AMTAUD)*(AMTAUD**2-AMF0**2+AMX**2)                    
      PR(3)=-PN(3)                                                      
      PHSPAC=PHSPAC*(4.d0*PI)*(2.d0*PR(3)/AMTAUD)                              
C                                                                       
CAM                                                                     
      ENQ1=(AMX2+AMF1**2-AMF2**2)/(2.d0*AMX)                               
      ENQ2=(AMX2-AMF1**2+AMF2**2)/(2.d0*AMX)                               
      PPPI=SQRT((ENQ1-AMF1)*(ENQ1+AMF1))                                  
      PHSPAC=PHSPAC*(4.d0*PI)*(2.d0*PPPI/AMX)                                 
C CHARGED PI MOMENTUM IN RHO REST FRAME  
      CALL SPHERD(PPPI,PKC)                                             

      PKC(4)=ENQ1                                                       
C NEUTRAL PI MOMENTUM IN RHO REST FRAME                                 
      DO 20 I=1,3                                                       
20      PKZ(I)=-PKC(I)                                                    
      PKZ(4)=ENQ2                                                       
      EXE=(PR(4)+PR(3))/AMX                                             
C PIONS BOOSTED FROM RHO REST FRAME TO TAU REST FRAME   
      CALL BOSTD3(EXE,PKC,PKC)                                          
      CALL BOSTD3(EXE,PKZ,PKZ)                                          

      DO I=1,4           ! DAM2PI requires floats
        PNR(I)=PN(I)     ! arearguments of this routine
        PKCR(I)=PKC(I)   ! and have to remain float
        PKZR(I)=PKZ(I)
        PRR(I)=PR(I)
      ENDDO
!      if (inn.gt.3) inn=3   ! for higher inn channels flat phase space
      CALL DAM2PI(INUM,PT,PNR,PKCR,PKZR,AMPLIT,HV)
      DGAMT=1/(2.*AMTAUD)*AMPLIT*PHSPAC 
      DO K=1,4                                                       
        PMULT(K,1)=PKCR(K)
        PMULT(K,2)=PKZR(K)
      ENDDO           
      RETURN             
      END                
      SUBROUTINE DWLNEW(KTO,ISGN,PNU,PWB,PNPI,MODE,NEV)
C ----------------------------------------------------------------------
C Lorentz transformation to CMsystem and
C Updating of HEPEVT record
C
C ISGN = 1/-1 for tau-/tau+
C
C     called by : DEXAY,(DEKAY1,DEKAY2)
C ----------------------------------------------------------------------
C
      include 'TAUDCD_BBB_size.inc'

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      COMMON /TAUPOS/ NP1,NP2
      CHARACTER NAMES(NMODE)*31
      REAL  PNU(4),PWB(4),PNPI(4,9)
      REAL  PPI(4)
      INTEGER IS, NDAU, IDECAY
      INTEGER NEV, NPRIN
      DATA NPRIN/0/
      INTEGER LAMDEC
      COMMON /LAMDEC/ LAMDEC
C     
      JNPI=MODE-NLT
C position of decaying particle
      IF(KTO.EQ. 1) THEN
        NPS=NP1
      ELSE
        NPS=NP2
      ENDIF
      IS=0
C
C tau neutrino (nu_tau is 16)
      CALL TRALO4(KTO,PNU,PNU,AM)
      ND=MULPIK(JNPI)
      IF(ND.EQ.2.AND.IDFFIN(3,JNPI).NE.0) THEN
        IS=1
        CALL FILHEP(0,1,-IDFFIN(3,JNPI)*ISGN,NPS,NPS,0,0,PNU,AM,.TRUE.)
      ELSEIF(ND.EQ.2.AND.IDFFIN(4,JNPI).NE.0) THEN
        IS=1
        CALL FILHEP(0,1, IDFFIN(4,JNPI)     ,NPS,NPS,0,0,PNU,AM,.TRUE.)
      ELSEIF(ND.EQ.1.AND.IDFFIN(3,JNPI).NE.0) THEN
        IS=1
        CALL FILHEP(0,1,-IDFFIN(3,JNPI)*ISGN,NPS,NPS,0,0,PNU,AM,.TRUE.)
      ELSEIF(ND.EQ.1.AND.IDFFIN(4,JNPI).NE.0) THEN
        IS=1
        CALL FILHEP(0,1, IDFFIN(4,JNPI)     ,NPS,NPS,0,0,PNU,AM,.TRUE.)
c sigle scalar has no W
      ELSEIF(ND.EQ.1) THEN
        IS=1
        CALL FILHEP(0,1,16*ISGN,NPS,NPS,0,0,PNU,AM,.TRUE.)

      ELSE
       ! IS=0
        CALL FILHEP(0,1,16*ISGN,NPS,NPS,0,0,PNU,AM,.TRUE.)
C
C W boson (W+ is 24)
        CALL TRALO4(KTO,PWB,PWB,AM)
        CALL FILHEP(0,1,-24*ISGN,NPS,NPS,0,0,PWB,AM,.TRUE.)
      ENDIF
C
C multi pi mode JNPI
C 
C get multiplicity of mode JNPI
      ND=MULPIK(JNPI)
      NDAU=0
      DO I=1,ND
        KFPI=LUNPIK(IDFFIN(I,JNPI),-ISGN)
C       PRINT *, 'DWLNEW: I, JNPI, -ISGN, KFPI = ',I,JNPI,-ISGN,KFPI
        IDECAY=0
        IF ((KFPI.EQ.221).AND.  !eta
     c       (MODE.EQ.37.OR.MODE.EQ.38.OR.MODE.EQ.39.OR.MODE.EQ.40.OR.
     c        MODE.EQ.41.OR.MODE.EQ.42.OR.MODE.EQ.43.OR.MODE.EQ.44.OR.
     c        MODE.EQ.133.OR.MODE.EQ.134.OR.MODE.EQ.135.OR.MODE.EQ.136.OR.
     c        MODE.EQ.137.OR.MODE.EQ.138.OR.MODE.EQ.139.OR.MODE.EQ.140.OR.
     c        MODE.EQ.141.OR.MODE.EQ.142.OR.MODE.EQ.143.OR.MODE.EQ.144.OR.
     c        MODE.EQ.145.OR.MODE.EQ.146.OR.MODE.EQ.147.OR.MODE.EQ.148.OR.
     c        MODE.EQ.242.OR.MODE.EQ.243.OR.MODE.EQ.244.OR.MODE.EQ.245.OR.
     c        MODE.EQ.246.OR.MODE.EQ.247.OR.MODE.EQ.248.OR.MODE.EQ.249.OR.
     c        MODE.EQ.350.OR.MODE.EQ.351.OR.MODE.EQ.352.OR.MODE.EQ.353.OR.
     c        MODE.EQ.354.OR.MODE.EQ.355.OR.MODE.EQ.356.OR.MODE.EQ.357))
     c        IDECAY=1    
        IF ((KFPI.EQ.223).AND. ! omega
     c       (MODE.EQ.45.OR.MODE.EQ.46.OR.MODE.EQ.47.OR.
     c        MODE.EQ.48.OR.MODE.EQ.49.OR.MODE.EQ.50.OR.
     c        MODE.EQ.130.OR.MODE.EQ.131.OR.MODE.EQ.132.OR.
     c        MODE.EQ.236.OR.MODE.EQ.237.OR.MODE.EQ.238.OR.
     c        MODE.EQ.239.OR.MODE.EQ.240.OR.MODE.EQ.241.OR.
     c        MODE.EQ.250.OR.MODE.EQ.251.OR.MODE.EQ.252.OR.
     c        MODE.EQ.358.OR.MODE.EQ.359.OR.MODE.EQ.360.OR.
     c        MODE.EQ.361.OR.MODE.EQ.362.OR.MODE.EQ.363))
     c        IDECAY=1
        IF ((KFPI.EQ.333).AND. ! phi
     c       (MODE.EQ.230.OR.MODE.EQ.231.OR.MODE.EQ.232)) IDECAY=1
        IF ((KFPI.EQ.20223).AND. ! f1
     c       (MODE.EQ.233.OR.MODE.EQ.234.OR.MODE.EQ.235.OR.
     c        MODE.EQ.253.OR.MODE.EQ.254.OR.MODE.EQ.255.OR.MODE.EQ.256.OR.
     c        MODE.EQ.257.OR.MODE.EQ.258.OR.MODE.EQ.259.OR.MODE.EQ.260))
     c       IDECAY=1
        IF ((ABS(KFPI).EQ.20213).AND.(MODE.EQ.341)) IDECAY=1 ! a1+
        IF ((ABS(KFPI).EQ.213).AND.(MODE.EQ.342)) IDECAY=1 ! rho+
        IF ((ABS(KFPI).EQ.9000211).AND.(MODE.GE.343.AND.MODE.LE.346)) IDECAY=1 !a0+
        IF ((ABS(KFPI).EQ.10213).AND.(MODE.GE.347.AND.MODE.LE.349)) IDECAY=1 !b1+
        IF ((ABS(KFPI).EQ.3122).AND.(MODE.GE.329.AND.MODE.LE.332).AND.(LAMDEC.EQ.1)) IDECAY=1 ! Lambda(bar)
        IF ((ABS(KFPI).EQ.9010221).AND.(MODE.GE.339.AND.MODE.LE.340)) IDECAY=1 ! f0
        IF ((ABS(KFPI).EQ.9000111).AND.(MODE.GE.364.AND.MODE.LE.371)) IDECAY=1 ! a0
C     
C for charged conjugate case, change charged pions only
C        IF(KFPI.NE.111)KFPI=KFPI*ISGN
        DO J=1,4
          PPI(J)=PNPI(J,I)
        END DO
        CALL TRALO4(KTO,PPI,PPI,AM)
        IF(IS.EQ.0) NPS = -I
        IF (IDECAY.EQ.1) THEN
           CALL FILHEP(0,2,KFPI,NPS,NPS,0,0,PPI,AM,.TRUE.)
           CALL TDECAY(KFPI,MODE,PPI,NDAU)
        ELSE
           CALL FILHEP(0,1,KFPI,NPS-NDAU,NPS-NDAU,0,0,PPI,AM,.TRUE.)
        ENDIF
      END DO
      IF (NEV.LT.NPRIN.OR.NPRIN.EQ.-1) THEN
C        PRINT *, 'DWLNEW: DUMP '
         CALL KKDUMPHEP(NEV,0,0)
      ENDIF
      RETURN
      END
      SUBROUTINE TDECAY(KFPI,MODE,PINPUT,NDAU)
      IMPLICIT NONE
      INTEGER KFPI,MODE,NDAU,ISIGN,ID1,ID2,ID3
      REAL PINPUT(4),XM1,XM2,XM3,PDAU1(4),PDAU2(4),PDAU3(4)
      INTEGER IDECAY2,ID2_1,ID2_2,ID2_3
      REAL XM2_1,XM2_2,XM2_3,PDAU2_1(4),PDAU2_2(4),PDAU2_3(4)
      INTEGER IDECAY22,ID22_1,ID22_2,ID22_3
      REAL XM22_1,XM22_2,XM22_3,PDAU22_1(4),PDAU22_2(4),PDAU22_3(4)
      INTEGER IDECAY3,ID3_1,ID3_2
      REAL XM3_1,XM3_2,PDAU3_1(4),PDAU3_2(4)
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      REAL     DCDMAS
      EXTERNAL DCDMAS
C
      ISIGN = KFPI/ABS(KFPI)
      IF (MODE.EQ.37.OR.
     c     MODE.EQ.41.OR.
     c     MODE.EQ.133.OR.
     c     MODE.EQ.137.OR.
     c     MODE.EQ.141.OR.
     c     MODE.EQ.145.OR.
     c     MODE.EQ.242.OR.
     c     MODE.EQ.246.OR.
     c     MODE.EQ.350.OR.
     c     MODE.EQ.354) THEN                    ! eta->gammagamma
         ID1=22
         ID2=22
         ID3=0
      ELSE IF (MODE.EQ.38.OR.
     c        MODE.EQ.42.OR.
     c        MODE.EQ.134.OR.
     c        MODE.EQ.138.OR.
     c        MODE.EQ.142.OR.
     c        MODE.EQ.146.OR.
     c        MODE.EQ.243.OR.
     c        MODE.EQ.247.OR.
     c        MODE.EQ.351.OR.
     c        MODE.EQ.355) THEN                 ! eta->pi0pi0pi0
         ID1= 111
         ID2= 111
         ID3= 111
      ELSE IF (MODE.EQ.39.OR.
     c        MODE.EQ.43.OR.
     c        MODE.EQ.135.OR.
     c        MODE.EQ.139.OR.
     c        MODE.EQ.143.OR.
     c        MODE.EQ.147.OR.
     c        MODE.EQ.244.OR.
     c        MODE.EQ.248.OR.
     c        MODE.EQ.352.OR.
     c        MODE.EQ.356) THEN                 ! eta->pi+pi-pi0
         ID1= 211
         ID2=-211
         ID3= 111
      ELSE IF (MODE.EQ.40.OR.
     c        MODE.EQ.44.OR.
     c        MODE.EQ.136.OR.
     c        MODE.EQ.140.OR.
     c        MODE.EQ.144.OR.
     c        MODE.EQ.148.OR.
     c        MODE.EQ.245.OR.
     c        MODE.EQ.249.OR.
     c        MODE.EQ.353.OR.
     c        MODE.EQ.357) THEN                 ! eta->pi+pi-gamma
         ID1= 211
         ID2=-211
         ID3= 22
      ELSE IF (MODE.EQ.45.OR.
     c        MODE.EQ.48.OR.
     c        MODE.EQ.130.OR.
     c        MODE.EQ.236.OR.
     c        MODE.EQ.239.OR.
     c        MODE.EQ.250.OR.
     c        MODE.EQ.358.OR.
     c        MODE.EQ.361) THEN                 ! omega->pi+pi-pi0
         ID1= 211
         ID2=-211
         ID3= 111
      ELSE IF (MODE.EQ.46.OR.
     c         MODE.EQ.49.OR.
     c         MODE.EQ.131.OR.
     c         MODE.EQ.237.OR.
     c         MODE.EQ.240.OR.
     c         MODE.EQ.251.OR.
     c         MODE.EQ.359.OR.
     c         MODE.EQ.362) THEN                 ! omega->pi+pi-
         ID1= 211
         ID2=-211
         ID3=0
      ELSE IF (MODE.EQ.47.OR.
     c         MODE.EQ.50.OR.
     c         MODE.EQ.132.OR.
     c         MODE.EQ.238.OR.
     c         MODE.EQ.241.OR.
     c         MODE.EQ.252.OR.
     c         MODE.EQ.360.OR.
     c         MODE.EQ.363) THEN                 ! omega->pi0gamma
         ID1=111
         ID2=22
         ID3=0
      ELSE IF (MODE.EQ.230) THEN     ! phi->K+K-
         ID1= 321
         ID2=-321
         ID3=0
      ELSE IF (MODE.EQ.231) THEN     ! phi->K0SK0L
         ID1= 310
         ID2= 130
         ID3=0
      ELSE IF (MODE.EQ.232) THEN     ! phi->pi+pi-pi0
         ID1= 211
         ID2=-211
         ID3= 111
      ELSE IF (MODE.EQ.233) THEN     ! f1->pi+pi-rho0(->pi+pi-)
         ID1= 211
         ID2=-211
         ID3= 113
      ELSE IF (MODE.EQ.234.OR.(MODE.GE.253.AND.MODE.LE.256)) THEN ! f1->pi+a0-(->pi-eta)
         ID1= 211
         ID2=-9000211
         ID3=0
      ELSE IF (MODE.EQ.235.OR.(MODE.EQ.257.AND.MODE.LE.260)) THEN ! f1->pi0a00(->pi0eta)
         ID1= 111
         ID2= 9000111
         ID3=0
      ELSE IF (MODE.EQ.341) THEN !   a1- -> pi-gamma
         ID1=211*ISIGN
         ID2=22
         ID3=0
      ELSE IF (MODE.EQ.342) THEN ! rho- -> pi-gamma
         ID1=211*ISIGN
         ID2=22
         ID3=0
      ELSE IF (MODE.GE.343.AND.MODE.LE.346) THEN ! a0- -> pi-eta
         ID1=211*ISIGN
         ID2=221
         ID3=0
      ELSE IF (MODE.GE.347.AND.MODE.LE.349) THEN ! b1- -> pi-omega
         ID1=211*ISIGN
         ID2=223
         ID3=0
      ELSE IF (MODE.GE.329.AND.MODE.LE.332) THEN ! Lambda -> p pi- OR Lambdabar -> pbar pi+
         ID1=2212*ISIGN
         ID2=-211*ISIGN
         ID3=0
      ELSE IF (MODE.GE.339.AND.MODE.LE.340) THEN ! f0(980)->pi+pi-
         ID1= 211
         ID2=-211
         ID3=0
      ELSE IF (MODE.GE.364.AND.MODE.LE.371) THEN ! a00(->pi0eta)
         ID1=111
         ID2=221
         ID3=0
      ENDIF
C
      XM1=DCDMAS(ID1,4)         ! masses
      XM2=DCDMAS(ID2,4)
      XM3=DCDMAS(ID3,4)
C
      IF (ID3.EQ.0) THEN ! 2 body channels
         CALL TWODECAY(PINPUT,XM1,XM2,PDAU1,PDAU2)
C     to hepevt
         IDECAY2=0
         IF ((ID2.EQ.-9000211).OR. ! a0- -> pi-eta
     $       (ID2.EQ.9000111)) THEN ! a00 -> pi0eta
            IDECAY2=1
         ENDIF
         IF (ID2.EQ.221) IDECAY2=1 ! eta decays
         IF (ID2.EQ.223) IDECAY2=1 ! omega decays
         IF (IDECAY2.EQ.0) THEN
            NDAU=NDAU+2
            CALL FILHEP(0,1,ID1,-1,-1,0,0,PDAU1,XM1,.TRUE.)
            CALL FILHEP(0,1,ID2,-2,-2,0,0,PDAU2,XM2,.TRUE.)
         ELSE
            NDAU=NDAU+1
            CALL FILHEP(0,1,ID1,-1,-1,0,0,PDAU1,XM1,.TRUE.)
            CALL FILHEP(0,2,ID2,-2,-2,0,0,PDAU2,XM2,.TRUE.)
C           CALL TDECAY(ID2,-MODE,PDAU2,NDAU) ! Alas: Fortran 77 does not have recursive function
            IF (ID2.EQ.-9000211) THEN ! a0- -> pi-eta
               ID2_1=-211
               ID2_2= 221
               ID2_3= 0
            ELSE IF (ID2.EQ.9000111) THEN ! a00 -> pi0eta
               ID2_1= 111
               ID2_2= 221
               ID2_3= 0
            ELSE IF (ID2.EQ.221.AND.(MODE.EQ.343.OR.MODE.EQ.364.OR.MODE.EQ.368)) THEN ! eta -> gamma gamma
               ID2_1=22
               ID2_2=22
               ID2_3=0
            ELSE IF (ID2.EQ.221.AND.(MODE.EQ.344.OR.MODE.EQ.365.OR.MODE.EQ.369)) THEN ! eta -> pi0 pi0 pi0
               ID2_1=111
               ID2_2=111
               ID2_3=111
            ELSE IF (ID2.EQ.221.AND.(MODE.EQ.345.OR.MODE.EQ.366.OR.MODE.EQ.370)) THEN ! eta -> pi+ pi- pi0
               ID2_1= 211
               ID2_2=-211
               ID2_3= 111
            ELSE IF (ID2.EQ.221.AND.(MODE.EQ.346.OR.MODE.EQ.367.OR.MODE.EQ.371)) THEN ! eta -> pi+ pi- gammma
               ID2_1= 211
               ID2_2=-211
               ID2_3= 22
            ELSE IF (ID2.EQ.223.AND.MODE.EQ.347) THEN ! omega -> pi+ pi- pi0
               ID2_1= 211
               ID2_2=-211
               ID2_3= 111
            ELSE IF (ID2.EQ.223.AND.MODE.EQ.348) THEN ! omega -> pi+ pi-
               ID2_1= 211
               ID2_2=-211
               ID2_3= 0
            ELSE IF (ID2.EQ.223.AND.MODE.EQ.349) THEN ! omega -> pi0 gamma
               ID2_1=111
               ID2_2=22
               ID2_3= 0
            ENDIF
            XM2_1=DCDMAS(ID2_1,4)
            XM2_2=DCDMAS(ID2_2,4)
            XM2_3=DCDMAS(ID2_3,4)
            IF (ID2_3.EQ.0) THEN
               CALL TWODECAY(PDAU2,XM2_1,XM2_2,PDAU2_1,PDAU2_2)
               IDECAY22=0
               IF (ID2_2.EQ.221.AND.
     $              ((MODE.GE.253.AND.MODE.LE.256).OR.
     $               (MODE.GE.257.AND.MODE.LE.260))) IDECAY22=1 ! decay eta
               IF (IDECAY22.EQ.0) THEN
                  NDAU=NDAU+2
                  CALL FILHEP(0,1,ID2_1,-1,-1,0,0,PDAU2_1,XM2_1,.TRUE.)
                  CALL FILHEP(0,1,ID2_2,-2,-2,0,0,PDAU2_2,XM2_2,.TRUE.)
               ELSE
                  NDAU=NDAU+1
                  CALL FILHEP(0,1,ID2_1,-1,-1,0,0,PDAU2_1,XM2_1,.TRUE.)
                  CALL FILHEP(0,2,ID2_2,-2,-2,0,0,PDAU2_2,XM2_2,.TRUE.)
C                 CALL TDECAY(ID2_2,-MODE,PDAU2_2,NDAU) ! Alas: Fortran 77 does not have recursive function
                  IF (MODE.EQ.253.OR.MODE.EQ.257) THEN !eta->gamma gamma
                     ID22_1=22
                     ID22_2=22
                     ID22_3=0
                     XM22_1=DCDMAS(ID22_1,4) ! masses
                     XM22_2=DCDMAS(ID22_2,4)
                     XM22_3=DCDMAS(ID22_3,4)
                  ELSE IF (MODE.EQ.254.OR.MODE.EQ.258) THEN ! eta->pi0 pi0 pi0
                     ID22_1=111
                     ID22_2=111
                     ID22_3=111
                     XM22_1=DCDMAS(ID22_1,4) ! masses
                     XM22_2=DCDMAS(ID22_2,4)
                     XM22_3=DCDMAS(ID22_3,4)
                  ELSE IF (MODE.EQ.255.OR.MODE.EQ.259) THEN ! eta->pi+pi-pi0
                     ID22_1= 211
                     ID22_2=-211
                     ID22_3= 111
                     XM22_1=DCDMAS(ID22_1,4) ! masses
                     XM22_2=DCDMAS(ID22_2,4)
                     XM22_3=DCDMAS(ID22_3,4)
                  ELSE IF (MODE.EQ.256.OR.MODE.EQ.260) THEN ! eta->pi+pi-gamma
                     ID22_1= 211
                     ID22_2=-211
                     ID22_3= 22
                     XM22_1=DCDMAS(ID22_1,4) ! masses
                     XM22_2=DCDMAS(ID22_2,4)
                     XM22_3=DCDMAS(ID22_3,4)
                  ENDIF
                  IF (ID22_3.EQ.0) THEN ! 2 body channels of eta
                     CALL TWODECAY(PDAU2_2,XM22_1,XM22_2,PDAU22_1,PDAU22_2)
                     NDAU=NDAU+2
                     CALL FILHEP(0,1,ID22_1,-1,-1,0,0,PDAU22_1,XM22_1,.TRUE.)
                     CALL FILHEP(0,1,ID22_2,-2,-2,0,0,PDAU22_2,XM22_2,.TRUE.)
                  ELSE          ! 3 body channels of eta
                     CALL THRDECAY(PDAU2_2,XM22_1,XM22_2,XM22_3,PDAU22_1,PDAU22_2,PDAU22_3)
                     NDAU=NDAU+3
                     CALL FILHEP(0,1,ID22_1,-1,-1,0,0,PDAU22_1,XM22_1,.TRUE.)
                     CALL FILHEP(0,1,ID22_2,-2,-2,0,0,PDAU22_2,XM22_2,.TRUE.)
                     CALL FILHEP(0,1,ID22_3,-3,-3,0,0,PDAU22_3,XM22_3,.TRUE.)
                  ENDIF
               ENDIF
            ELSE           ! body channels of eta and omega
               CALL THRDECAY(PDAU2,XM2_1,XM2_2,XM2_3,PDAU2_1,PDAU2_2,PDAU2_3)
               NDAU=NDAU+3
               CALL FILHEP(0,1,ID2_1,-1,-1,0,0,PDAU2_1,XM2_1,.TRUE.)
               CALL FILHEP(0,1,ID2_2,-2,-2,0,0,PDAU2_2,XM2_2,.TRUE.)
               CALL FILHEP(0,1,ID2_3,-3,-3,0,0,PDAU2_3,XM2_3,.TRUE.)
            ENDIF
         ENDIF
      ELSE                 ! 3 body channels    
         CALL THRDECAY(PINPUT,XM1,XM2,XM3,PDAU1,PDAU2,PDAU3)
C     to hepevt
         IDECAY3=0
         IF (ID3.EQ.113) IDECAY3=1 ! rho0->pi+pi-
         IF (IDECAY3.EQ.0) THEN
            NDAU=NDAU+3
            CALL FILHEP(0,1,ID1,-1,-1,0,0,PDAU1,XM1,.TRUE.)
            CALL FILHEP(0,1,ID2,-2,-2,0,0,PDAU2,XM2,.TRUE.)
            CALL FILHEP(0,1,ID3,-3,-3,0,0,PDAU3,XM3,.TRUE.)
         ELSE
            NDAU=NDAU+2
            CALL FILHEP(0,1,ID1,-1,-1,0,0,PDAU1,XM1,.TRUE.)
            CALL FILHEP(0,1,ID2,-2,-2,0,0,PDAU2,XM2,.TRUE.)
            CALL FILHEP(0,2,ID3,-3,-3,0,0,PDAU3,XM3,.TRUE.)
C           CALL TDECAY(ID3,-MODE,PDAU3,NDAU) ! Alas: Fortran 77 does not have recursive function
            ID3_1= 211
            ID3_2=-211
            XM3_1=DCDMAS(ID3_1,4)
            XM3_2=DCDMAS(ID3_2,4)
            CALL TWODECAY(PDAU3,XM3_1,XM3_2,PDAU3_1,PDAU3_2)
            NDAU=NDAU+2
            CALL FILHEP(0,1,ID3_1,-1,-1,0,0,PDAU3_1,XM3_1,.TRUE.)
            CALL FILHEP(0,1,ID3_2,-2,-2,0,0,PDAU3_2,XM3_2,.TRUE.)
         ENDIF
      ENDIF
C
      END
      SUBROUTINE TWODECAY(PINPUT,XM1,XM2,PDAU1,PDAU2)
      INTEGER I
      REAL PINPUT(4),XM1,XM2,PDAU1(4),PDAU2(4)
      DOUBLE PRECISION PIN(4),X(4),Y(4),YM1,YM2,R,RU
      DOUBLE PRECISION XLAM,a,b,c
      XLAM(a,b,c)=SQRT(ABS((a-b-c)**2-4.0*b*c))
      DO I=1,4
         PIN(I)= PINPUT(I)     ! mother 4 momentum
      ENDDO
      YM1=XM1
      YM2=XM2
      R=SQRT(PIN(4)**2-PIN(3)**2-PIN(2)**2-PIN(1)**2)
      RU=XLAM(R**2,YM1**2,YM2**2)/R/2 ! momenta of the two products
      CALL SPHERD(RU,X)         !     random 3 vector on the sphere   
      X(4)=SQRT(RU**2+YM1**2)
      Y(4)=SQRT(RU**2+YM2**2)
C      
      Y(1)=-X(1)
      Y(2)=-X(2)
      Y(3)=-X(3)
!     boost to lab and to real*4
      CALL bostdq(-1,PIN,X,X)  
      CALL bostdq(-1,PIN,Y,Y)
      DO I=1,4
         PDAU1(I)=X(I)
         PDAU2(I)=Y(I)
      ENDDO
      END
      SUBROUTINE THRDECAY(PINPUT,XM1,XM2,XM3,PDAU1,PDAU2,PDAU3)
      INTEGER I
      REAL PINPUT(4),XM1,XM2,XM3,PDAU1(4),PDAU2(4),PDAU3(4),RR(2)
      DOUBLE PRECISION AMIN,AMAX,AM2,WT
      DOUBLE PRECISION PIN(4),X(4),Y(4),Z(4),YM1,YM2,YM3,R,RU
      DOUBLE PRECISION XLAM,a,b,c
      XLAM(a,b,c)=SQRT(ABS((a-b-c)**2-4.0*b*c))
      DO I=1,4
         PIN(I)= PINPUT(I)     ! mother 4 momentum
      ENDDO
      YM1=XM1
      YM2=XM2
      YM3=XM3
 7    CONTINUE                  ! we generate mass of the first pair:
      CALL KKMC_RANMAR(RR,2)
      R=SQRT(PIN(4)**2-PIN(3)**2-PIN(2)**2-PIN(1)**2)
      AMIN=YM1+YM2
      AMAX=R-YM3
      AM2=SQRT(AMIN**2+RR(1)*(AMAX**2-AMIN**2))
C     weight for flat phase space
      WT=XLAM(R**2,AM2**2,YM3**2)*XLAM(AM2**2,YM1**2,YM2**2)
     &        /R**2                   /AM2**2
      IF (RR(2).GT.WT) GOTO 7
C
      RU=XLAM(AM2**2,YM1**2,YM2**2)/AM2/2 ! momenta of the first two products in the rest frame of that pair
      CALL SPHERD(RU,X)
      X(4)=SQRT(RU**2+YM1**2)
      Y(4)=SQRT(RU**2+YM2**2)
C         
      Y(1)=-X(1)
      Y(2)=-X(2)
      Y(3)=-X(3)
C     generate momentum of that pair in rest frame of mother:
      RU=XLAM(R**2,AM2**2,YM3**2)/R/2
      CALL SPHERD(RU,Z)
      Z(4)=SQRT(RU**2+AM2**2)
C     and boost first two decay products to rest frame of other.
      CALL bostdq(-1,Z,X,X)
      CALL bostdq(-1,Z,Y,Y)
C     redefine Z(4) to 4-momentum of the last decay product: 
      Z(1)=-Z(1)
      Z(2)=-Z(2)
      Z(3)=-Z(3)
      Z(4)=SQRT(RU**2+YM3**2)
C     boost all to lab and move to real*4; also masses
      CALL bostdq(-1,PIN,X,X)
      CALL bostdq(-1,PIN,Y,Y)
      CALL bostdq(-1,PIN,Z,Z)
      DO I=1,4
         PDAU1(I)=X(I)
         PDAU2(I)=Y(I)
         PDAU3(I)=Z(I)
      ENDDO
      END
      subroutine bostdq(idir,vv,pp,q)
*     *******************************
c Boost along arbitrary vector v (see eg. J.D. Jacson, Classical 
c Electrodynamics).
c Four-vector pp is boosted from an actual frame to the rest frame 
c of the four-vector v (for idir=1) or back (for idir=-1). 
c q is a resulting four-vector.
c Note: v must be time-like, pp may be arbitrary.
c
c Written by: Wieslaw Placzek            date: 22.07.1994
c Last update: 3/29/95                     by: M.S.
c 
      implicit DOUBLE PRECISION (a-h,o-z)
      parameter (nout=6)
      DOUBLE PRECISION v(4),p(4),q(4),pp(4),vv(4)  
      save
!
      do 1 i=1,4
      v(i)=vv(i)
 1    p(i)=pp(i)
      amv=(v(4)**2-v(1)**2-v(2)**2-v(3)**2)
      if (amv.le.0d0) then
        write(6,*) 'bosstv: warning amv**2=',amv
      endif
      amv=sqrt(abs(amv))
      if (idir.eq.-1) then
        q(4)=( p(1)*v(1)+p(2)*v(2)+p(3)*v(3)+p(4)*v(4))/amv
        wsp =(q(4)+p(4))/(v(4)+amv)
      elseif (idir.eq.1) then
        q(4)=(-p(1)*v(1)-p(2)*v(2)-p(3)*v(3)+p(4)*v(4))/amv
        wsp =-(q(4)+p(4))/(v(4)+amv)
      else
        write(nout,*)' >>> boostv: wrong value of idir = ',idir
      endif
      q(1)=p(1)+wsp*v(1)
      q(2)=p(2)+wsp*v(2)
      q(3)=p(3)+wsp*v(3)
      end
      SUBROUTINE KKDumpHEP (IEV, IFL,PRNTNU)
*
************************************************************************
*
*        SUBR. KKDumpHEP (IEV, IFL)
*
*   Dumps HEPEVT of event (CM frame) IEV depending on flag
*    IFL = 0 (all particles)
*    IFL > 0 (particles with ISTHEP = IFL)
*
*   Called by User
*
*   Version 1, 4/25/2002 Swagato Banerjee
*   Version 2, 5/26/2005 Swagato Banerjee (CALL PYNAME)
*
************************************************************************
**
      Implicit none
*..Include:
*#include "stdhep/stdhep.inc"
      INCLUDE '../KK2f/HepEvt.fi'
*
      INTEGER          IEV, IFL, I, J, NP,PRNTNU
      REAL*8              PX,PY,PZ,EN, MS
      CHARACTER CHAP*16
*    -------------------------------------------------------------------
      WRITE(6,'(''KKDumpHEP>CM> Event, NHEP : '',2I6)') IEV, NHEP
      WRITE(6,
     +  '(''     I   ISTHEP   ID                 M1 M2 D1 D2 PHEP(J),'',
     +    ''J=1,5 m(PHEP(1-4))'')')
      NP = 0
      PX = 0.
      PY = 0.
      PZ = 0.
      EN = 0.
      MS = 0.
      DO I= 1,NHEP
        MS = PHEP(4,I)**2 - PHEP(3,I)**2 - PHEP(2,I)**2 - PHEP(1,I)**2
        IF(MS.NE.0.0) MS=MS/SQRT(ABS(MS))
        IF (PRNTNU.EQ.1.AND.ABS(IDHEP(I)).eq.16)
     +   WRITE (6,'(''Mass(P4(nutau)) = '',F14.7,'' MeV'')') MS*1000.0
        CALL PYNAME(IDHEP(I),CHAP)
        IF (IFL.EQ.0) THEN
          WRITE (6,'(2I6,I8,1X,A10,1X,4I6,6(1X,F10.5))')
     +      I, ISTHEP(I), IDHEP(I), CHAP(1:10),
     +      JMOHEP(1,I),JMOHEP(2,I),
     +      JDAHEP(1,I),JDAHEP(2,I),
     +      (PHEP(J,I),J=1,5),MS
        ELSEIF (IFL.Eq.ISTHEP(I)) THEN
          WRITE (6,'(2I6,I8,1X,A10,1X,4I6,6(1X,F10.5))')
     +      I, ISTHEP(I), IDHEP(I), CHAP(1:10),
     +      JMOHEP(1,I),JMOHEP(2,I),
     +      JDAHEP(1,I),JDAHEP(2,I),
     +     (PHEP(J,I),J=1,5),MS
        ENDIF
        IF (ISTHEP(I).EQ.1) THEN
          NP=NP+1
          PX=PX+PHEP(1,I)
          PY=PY+PHEP(2,I)
          PZ=PZ+PHEP(3,I)
          EN=EN+PHEP(4,I)
        ENDIF
      ENDDO
      WRITE(6,
     +'(''KKDumpHEP>CM>Final State::NP,PX,PY,PZ,EN = '',I6,4(1X,F10.5))')
     +                          NP,PX,PY,PZ,EN
      RETURN
*                                                            END KKDumpHEP
      END
      FUNCTION IMEGET(IMULT,MNUM)
      IMPLICIT NONE  
      include 'TAUDCD_BBB_size.inc'
      INTEGER imeget,imult,mnum
      integer KEY0,KEY1,KEY2,KEY3,KEY4,KEY5,KEY6
      COMMON /METYP/ KEY0(2),KEY1(NM1),KEY2(NM2),KEY3(NM3),
     $               KEY4(NM4),KEY5(NM5),KEY6(NM6)
C this function provides access to the list of decay products to pass info
C on the type of matrix element.

C INPUT:
C    IMULT: multiplicity of the channel (e.g. IMULT=2 denotes 2 products plus tau_nu) 
C    MNUM:  position on the list of decay channels for the given multiplicity   
C OUTPUT:
C    IMEGET; 0- channel not initialized,  1- constant ME flat phase space
C            2- default ME,               3- default ME, but one stable spin>0
C            4- default ME wrapped curr., 5- wrapped ME  
       IF(IMULT.LT.0.OR.IMULT.GT.6) THEN
        WRITE(*,*) 'stop in IMEGET IMULT=',IMULT
        STOP
       ENDIF

      IF (IMULT.EQ.0) THEN
       IF(MNUM.LE.0.OR.MNUM.GT.2) THEN
        WRITE(*,*) 'stop in IMEGET IMULT=',IMULT,' but MNUM=',MNUM
        STOP
       ENDIF
       IMEGET=KEY0(MNUM)
      ELSEIF (IMULT.EQ.1) THEN
       IF(MNUM.LE.0.OR.MNUM.GT.NM1) THEN
        WRITE(*,*) 'stop in IMEGET IMULT=',IMULT,' but MNUM=',MNUM
        STOP
       ENDIF
       IMEGET=KEY1(MNUM)
      ELSEIF (IMULT.EQ.2) THEN
       IF(MNUM.LE.0.OR.MNUM.GT.NM2) THEN
        WRITE(*,*) 'stop in IMEGET IMULT=',IMULT,' but MNUM=',MNUM
        STOP
       ENDIF
       IMEGET=KEY2(MNUM)
      ELSEIF (IMULT.EQ.3) THEN
       IF(MNUM.LT.0.OR.MNUM.GT.NM3) THEN
        WRITE(*,*) 'stop in IMEGET IMULT=',IMULT,' but MNUM=',MNUM
        STOP
       ENDIF
       IMEGET=KEY3(MNUM)
      ELSEIF (IMULT.EQ.4) THEN
       IF(MNUM.LT.0.OR.MNUM.GT.NM4) THEN
        WRITE(*,*) 'stop in IMEGET IMULT=',IMULT,' but MNUM=',MNUM
        STOP
       ENDIF
       IMEGET=KEY4(MNUM)
      ELSEIF (IMULT.EQ.5) THEN
       IF(MNUM.LE.0.OR.MNUM.GT.NM5) THEN
        WRITE(*,*) 'stop in IMEGET IMULT=',IMULT,' but MNUM=',MNUM
        STOP
       ENDIF
       IMEGET=KEY5(MNUM)   
      ELSEIF (IMULT.EQ.6) THEN
       IF(MNUM.LE.0.OR.MNUM.GT.NM6) THEN
        WRITE(*,*) 'stop in IMEGET IMULT=',IMULT,' but MNUM=',MNUM
        STOP
       ENDIF
       IMEGET=KEY6(MNUM)
      ENDIF
      END
      subroutine iniofc
      return
      end

      FUNCTION SIGEE_WRAP(Q2,JNP)  
      SIGEE_WRAP=0
      RETURN
      END


      SUBROUTINE DAM1PI_wrap(INUM,PNU,AMF0,PKK,AMF1,GAMM,HV)
      REAL  PKK(4),PNU(4),HV(4)
C
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
C
      REAL    AMINVI            ! mass
      INTEGER ISPNVI            ! spin
      REAL    VCINVI,ACINVI     ! coupling co-efficients in (A+ gamma5 B) term or (A + gamma_5 B) gamma-mu terms for Spin0/Spin1
      COMMON /AMINVI/ AMINVI,ISPNVI,VCINVI,ACINVI
C
      REAL  QN
C
      IF (INUM.NE.70.AND.INUM.NE.71) RETURN
C
      QN = PKK(4) * PNU(4)
     $   - PKK(1) * PNU(1) - PKK(2) * PNU(2)- PKK(3) * PNU(3)

!     Following is written for covariant vectors HV(k) (namely h_i, likewise 3.25 Comp. Phys. Commun. 64 (1991) 275).
!     It is assumed that HV(4) corresponds to h_0

      IF (ISPNVI.EQ.0) THEN      ! scalar case
         GAMM = 2 * (VCINVI**2 + ACINVI**2) * AMTAU * PNU(4)
     $        + 2 * (VCINVI**2 - ACINVI**2) * AMF0 * AMTAU
         DO K=1,3
            HV(K)= 4 * VCINVI * ACINVI * AMTAU * PNU(K)/GAMM
         ENDDO
         HV(4)=1
         GAMM= GAMM*0.5*AMTAU**2
      ELSEIF (ISPNVI.EQ.1) THEN  ! vector case
         GAMM = 2 * (VCINVI**2 + ACINVI**2)
     $            * (2 * AMTAU * PKK(4) * QN + AMTAU * PNU(4) * AMF1**2)
     $        - 6 * (VCINVI**2 - ACINVI**2) * AMTAU * AMF0 * AMF1**2
         DO K=1,3
            HV(K) = 4 * VCINVI * ACINVI * AMTAU * PNU(K) * (AMF1**2 - 2 * QN)/GAMM
         ENDDO
         HV(4)=1
         GAMM= GAMM*0.5*AMTAU**2/AMF1**2
      ENDIF
      GAMM=GAMM*GFERMI**2
      RETURN
      END

      FUNCTION FCONST_wrap(INUM)
      FCONST_WRAP=1
      RETURN
      END

      SUBROUTINE CURR2_wrap(MNUM,PIM1,PIM2,HADCUR)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4)
      COMPLEX HADCUR(4)
      RETURN
      END

      SUBROUTINE DAM2PI_wrap(MNUM,PT,PN,PIM1,PIM2,AMPLIT,HV)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4)
      RETURN
      END

      SUBROUTINE CURR3PI_wrap(MNUM,PIM1,PIM2,PIM3,HADCUR)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4)
      COMPLEX HADCUR(4)
      RETURN
      END

      SUBROUTINE DAM3PI_wrap(MNUM,PT,PN,PIM1,PIM2,PIM3,AMPLIT1,HV)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4), FPI, FPIc, VUD, CE
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      EXTERNAL eepi, mumupi
      DATA  FPIc /93.3E-3/ 
      IF (MNUM.EQ.47) THEN
         CALL eepi(AMEL,PT,PN,PIM1,PIM2,PIM3,AMPLIT,HV)
      ELSE IF (MNUM.EQ.48) THEN
         CALL mumupi(AMMU,PT,PN,PIM1,PIM2,PIM3,AMPLIT,HV)
      ENDIF
      FPI = FPIc
      VUD = CCABIB 
      CE = 16.*GFERMI * GFERMI * AMTAU * AMTAU * FPI * FPI * VUD * VUD * (16.* ACOS(-1.) * ACOS(-1.) / 137. / 137.)
      AMPLIT1 = AMPLIT * CE
      RETURN
      END

      SUBROUTINE CURR4_wrap(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4),PIM4(4)
      COMPLEX HADCUR(4)
      RETURN
      END

      SUBROUTINE DAM4PI_wrap(MNUM,PT,PN,PIM1,PIM2,PIM3,PIM4,AMPLIT1,HV)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4),PIM4(4)
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      EXTERNAL eemu
      IF (MNUM.GE.3.AND.MNUM.LE.6) THEN
         CALL eemu(AMTAU,PT,PN,PIM1,PIM2,PIM3,PIM4,AMPLIT,HV)
         AMPLIT1=AMPLIT* 32.*(GFERMI * GFERMI / 2.0)*4.*(16.*ACOS(-1.) * ACOS(-1.) / 137. / 137.);
      ENDIF
      RETURN
      END

      SUBROUTINE CURR5_wrap(MNUM,PIM1,PIM2,PIM3,PIM4,PIM5,HADCUR)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4),PIM4(4),PIM5(4)
      COMPLEX HADCUR(4)
      RETURN
      END

      SUBROUTINE DAM5PI_wrap(MNUM,PT,PN,PIM1,PIM2,PIM3,PIM4,PIM5,AMPLIT,HV)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4),PIM4(4),PIM5(4)
      RETURN
      END

      SUBROUTINE DAMPRY_wrap(ITDKRC,XK0DEC,XK,XA,QP,XN,AMPLIT,HV)
      IMPLICIT REAL*8 (A-H,O-Z)
      REAL*8  HV(4),QP(4),XN(4),XA(4),XK(4)
      RETURN
      END
      SUBROUTINE DADMEL(MODE,ISGN,HHV,PNU,PWB,Q1,Q2,PHX)
C ----------------------------------------------------------------------
C
C     called by : DEXEL,(DEKAY,DEKAY1)
C ----------------------------------------------------------------------
      include 'TAUDCD_BBB_size.inc'
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER

      REAL*4         PHX(4)

      COMMON / INOUT / INUT,IOUT


      REAL  HHV(4),HV(4),PWB(4),PNU(4),Q1(4),Q2(4)
      REAL  PDUM1(4),PDUM2(4),PDUM3(4),PDUM4(4),PDUM5(4)
      REAL*4 RRR(3)
      REAL*8 SWT, SSWT
      DATA PI /3.141592653589793238462643/
      DATA IWARM/0/
C
C     PRINT *, 'DADMEL: NBMC = ', NBMC
      IF(MODE.EQ.-1) THEN
C     ===================
        IWARM=1
        NEVRAW=0
        NEVACC=0
        NEVOVR=0
        SWT=0
        SSWT=0
        WTMAX=1E-20
        DO 15 I=1,500
        CALL DPHSEL(WT,HV,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
        IF(WT.GT.WTMAX/1.2) WTMAX=WT*1.2
15      CONTINUE
CC      CALL HBOOK1(803,'WEIGHT DISTRIBUTION  DADMEL    $',100,0,2)
C
      ELSEIF(MODE.EQ. 0) THEN
C     =======================
300     CONTINUE
        IF(IWARM.EQ.0) GOTO 902
        NEVRAW=NEVRAW+1
        CALL DPHSEL(WT,HV,PNU,PWB,Q1,Q2,PHX)
CC      CALL HFILL(803,WT/WTMAX)
        SWT=SWT+WT
        SSWT=SSWT+WT**2
        CALL KKMC_RANMAR(RRR,3)
        RN=RRR(1)
        IF(WT.GT.WTMAX) NEVOVR=NEVOVR+1
        IF(RN*WTMAX.GT.WT) GOTO 300
C ROTATIONS TO BASIC TAU REST FRAME
        RR2=RRR(2)
        COSTHE=-1.+2.*RR2
        THET=ACOS(COSTHE)
        RR3=RRR(3)
        PHI =2*PI*RR3
        CALL ROTOR2(THET,PNU,PNU)
        CALL ROTOR3( PHI,PNU,PNU)
        CALL ROTOR2(THET,PWB,PWB)
        CALL ROTOR3( PHI,PWB,PWB)
        CALL ROTOR2(THET,Q1,Q1)
        CALL ROTOR3( PHI,Q1,Q1)
        CALL ROTOR2(THET,Q2,Q2)
        CALL ROTOR3( PHI,Q2,Q2)
        CALL ROTOR2(THET,HV,HV)
        CALL ROTOR3( PHI,HV,HV)
        CALL ROTOR2(THET,PHX,PHX)
        CALL ROTOR3( PHI,PHX,PHX)
        DO 44,I=1,3
 44     HHV(I)=-ISGN*HV(I)
        NEVACC=NEVACC+1
C
      ELSEIF(MODE.EQ. 1) THEN
C     =======================
        IF(NEVRAW.EQ.0) RETURN
        PARGAM=SWT/FLOAT(NEVRAW+1)
        ERROR=0
        IF(NEVRAW.NE.0) ERROR=SQRT(SSWT/SWT**2-1./FLOAT(NEVRAW))
        RAT=PARGAM/GAMEL
        WRITE(IOUT, 7010) NEVRAW,NEVACC,NEVOVR,PARGAM,RAT,ERROR
CC      CALL HPRINT(803)
        GAMPMC(1)=RAT
        GAMPER(1)=ERROR
CAM     NEVDEC(1)=NEVACC
      ENDIF
C     =====
      RETURN
 7010 FORMAT(///1X,15(5H*****)
     $ /,' *',     25X,'******** DADMEL FINAL REPORT  ******** ',9X,1H*
     $ /,' *',I20  ,5X,'NEVRAW = NO. OF EL  DECAYS TOTAL       ',9X,1H*
     $ /,' *',I20  ,5X,'NEVACC = NO. OF EL   DECS. ACCEPTED    ',9X,1H*
     $ /,' *',I20  ,5X,'NEVOVR = NO. OF OVERWEIGHTED EVENTS    ',9X,1H*
     $ /,' *',E20.5,5X,'PARTIAL WTDTH ( ELECTRON) IN GEV UNITS ',9X,1H*
     $ /,' *',F20.9,5X,'IN UNITS GFERMI**2*MASS**5/192/PI**3   ',9X,1H*
     $ /,' *',F20.9,5X,'RELATIVE ERROR OF PARTIAL WIDTH        ',9X,1H*
     $ /,' *',25X,     'COMPLETE QED CORRECTIONS INCLUDED      ',9X,1H*
     $ /,' *',25X,     'BUT ONLY V-A CUPLINGS                  ',9X,1H*
     $  /,1X,15(5H*****)/)
 902  WRITE(IOUT, 9020)
 9020 FORMAT(' ----- DADMEL: LACK OF INITIALISATION')
      STOP
      END
      SUBROUTINE DADMMU(MODE,ISGN,HHV,PNU,PWB,Q1,Q2,PHX)
C ----------------------------------------------------------------------
      include 'TAUDCD_BBB_size.inc'
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER
      COMMON / INOUT / INUT,IOUT
      REAL*4         PHX(4)
      REAL  HHV(4),HV(4),PNU(4),PWB(4),Q1(4),Q2(4)
      REAL  PDUM1(4),PDUM2(4),PDUM3(4),PDUM4(4),PDUM5(4)
      REAL*4 RRR(3)
      REAL*8 SWT, SSWT
      DATA PI /3.141592653589793238462643/
      DATA IWARM /0/
C
C     PRINT *, 'DADMMU: NBMC = ', NBMC
      IF(MODE.EQ.-1) THEN
C     ===================
        IWARM=1
        NEVRAW=0
        NEVACC=0
        NEVOVR=0
        SWT=0
        SSWT=0
        WTMAX=1E-20
        DO 15 I=1,500
        CALL DPHSMU(WT,HV,PDUM1,PDUM2,PDUM3,PDUM4,PDUM5)
        IF(WT.GT.WTMAX/1.2) WTMAX=WT*1.2
15      CONTINUE
CC      CALL HBOOK1(802,'WEIGHT DISTRIBUTION  DADMMU    $',100,0,2)
C
      ELSEIF(MODE.EQ. 0) THEN
C     =======================
300     CONTINUE
        IF(IWARM.EQ.0) GOTO 902
        NEVRAW=NEVRAW+1
        CALL DPHSMU(WT,HV,PNU,PWB,Q1,Q2,PHX)
CC      CALL HFILL(802,WT/WTMAX)
        SWT=SWT+WT
        SSWT=SSWT+WT**2
        CALL KKMC_RANMAR(RRR,3)
        RN=RRR(1)
        IF(WT.GT.WTMAX) NEVOVR=NEVOVR+1
        IF(RN*WTMAX.GT.WT) GOTO 300
C ROTATIONS TO BASIC TAU REST FRAME
        COSTHE=-1.+2.*RRR(2)
        THET=ACOS(COSTHE)
        PHI =2*PI*RRR(3)
        CALL ROTOR2(THET,PNU,PNU)
        CALL ROTOR3( PHI,PNU,PNU)
        CALL ROTOR2(THET,PWB,PWB)
        CALL ROTOR3( PHI,PWB,PWB)
        CALL ROTOR2(THET,Q1,Q1)
        CALL ROTOR3( PHI,Q1,Q1)
        CALL ROTOR2(THET,Q2,Q2)
        CALL ROTOR3( PHI,Q2,Q2)
        CALL ROTOR2(THET,HV,HV)
        CALL ROTOR3( PHI,HV,HV)
        CALL ROTOR2(THET,PHX,PHX)
        CALL ROTOR3( PHI,PHX,PHX)
        DO 44,I=1,3
 44     HHV(I)=-ISGN*HV(I)
        NEVACC=NEVACC+1
C
      ELSEIF(MODE.EQ. 1) THEN
C     =======================
        IF(NEVRAW.EQ.0) RETURN
        PARGAM=SWT/FLOAT(NEVRAW+1)
        ERROR=0
        IF(NEVRAW.NE.0) ERROR=SQRT(SSWT/SWT**2-1./FLOAT(NEVRAW))
        RAT=PARGAM/GAMEL
        WRITE(IOUT, 7010) NEVRAW,NEVACC,NEVOVR,PARGAM,RAT,ERROR
CC      CALL HPRINT(802)
        GAMPMC(2)=RAT
        GAMPER(2)=ERROR
CAM     NEVDEC(2)=NEVACC
      ENDIF
C     =====
      RETURN
 7010 FORMAT(///1X,15(5H*****)
     $ /,' *',     25X,'******** DADMMU FINAL REPORT  ******** ',9X,1H*
     $ /,' *',I20  ,5X,'NEVRAW = NO. OF MU  DECAYS TOTAL       ',9X,1H*
     $ /,' *',I20  ,5X,'NEVACC = NO. OF MU   DECS. ACCEPTED    ',9X,1H*
     $ /,' *',I20  ,5X,'NEVOVR = NO. OF OVERWEIGHTED EVENTS    ',9X,1H*
     $ /,' *',E20.5,5X,'PARTIAL WTDTH (MU  DECAY) IN GEV UNITS ',9X,1H*
     $ /,' *',F20.9,5X,'IN UNITS GFERMI**2*MASS**5/192/PI**3   ',9X,1H*
     $ /,' *',F20.9,5X,'RELATIVE ERROR OF PARTIAL WIDTH        ',9X,1H*
     $ /,' *',25X,     'COMPLETE QED CORRECTIONS INCLUDED      ',9X,1H*
     $ /,' *',25X,     'BUT ONLY V-A CUPLINGS                  ',9X,1H*
     $  /,1X,15(5H*****)/)
 902  WRITE(IOUT, 9020)
 9020 FORMAT(' ----- DADMMU: LACK OF INITIALISATION')
      STOP
      END
