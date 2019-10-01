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
      COMMON / TAUBRA / GAMPRT(NBMC),JLIST(NBMC),NCHAN

C      REAL   CUMUL(20)

      REAL   CUMUL(NBMC),RRR(1)
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
C     *******************************
C THIS ROUTINE  SIMULATES TAU+  DECAY
      include 'TAUDCD_BBB_size.inc'
      COMMON / DECP4 / PP1(4),PP2(4),KF1,KF2
      COMMON / JAKI   /  JAK1,JAK2,JAKP,JAKM,KTOM
      COMMON / TAUBMC / GAMPMC(NBMC),GAMPER(NBMC),NEVDEC(NBMC)
      REAL*4            GAMPMC    ,GAMPER
      
      REAL  HH(4)
      REAL  HV(4),PNU(4),PPI(4)
      REAL  PWB(4),PMU(4),PNM(4)
      REAL  PRHO(4),PIC(4),PIZ(4)
      REAL  PAA(4),PIM1(4),PIM2(4),PIPL(4)
      REAL  PKK(4),PKS(4)
      REAL  PNPI(4,9)
      REAL  PHOT(4)
      REAL  PDUM(4)
      DATA NEV,NPRIN/0,10/
      KTO=1
      IF(JAK1.EQ.-1) RETURN
      IMD=IMOD
      IF(IMD.EQ.0) THEN
C     =================
      JAK=JAK1
      IF(JAK1.EQ.0) CALL JAKER(JAK)
      IF(JAK.EQ.1) THEN
        CALL DADMEL(0, ISGN,HV,PNU,PWB,PMU,PNM,PHOT)
      ELSEIF(JAK.EQ.2) THEN
        CALL DADMMU(0, ISGN,HV,PNU,PWB,PMU,PNM,PHOT)
      ELSE
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
        CALL DWLNEW(1,ISGN,PNU,PWB,PNPI,JAK)
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

      REAL  HH(4)
      REAL  HV(4),PNU(4),PPI(4)
      REAL  PWB(4),PMU(4),PNM(4)
      REAL  PRHO(4),PIC(4),PIZ(4)
      REAL  PAA(4),PIM1(4),PIM2(4),PIPL(4)
      REAL  PKK(4),PKS(4)
      REAL  PNPI(4,9)
      REAL  PHOT(4)
      REAL  PDUM(4)
      DATA NEV,NPRIN/0,10/
      KTO=2
      IF(JAK2.EQ.-1) RETURN
      IMD=IMOD
      IF(IMD.EQ.0) THEN
C     =================
      JAK=JAK2
      IF(JAK2.EQ.0) CALL JAKER(JAK)
      IF(JAK.EQ.1) THEN
        CALL DADMEL(0, ISGN,HV,PNU,PWB,PMU,PNM,PHOT)
      ELSEIF(JAK.EQ.2) THEN
        CALL DADMMU(0, ISGN,HV,PNU,PWB,PMU,PNM,PHOT)
      ELSE
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
        CALL DWLNEW(2,ISGN,PNU,PWB,PNPI,JAK)
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
C
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
        CALL DWLNEW(KTO,ISGN,PNU,PWB,PNPI,JAK)
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
       ELSEIF (MNUM.EQ.2) THEN
         CALL CURR_PIK0(PIM1,PIM2,HADCUR)
       ELSEIF (MNUM.EQ.3) THEN
         CALL CURR_KPI0(PIM1,PIM2,HADCUR)
       ELSEIF (MNUM.EQ.4) THEN
         CALL CURR_KK0(PIM1,PIM2,HADCUR)
       ELSE
         write(*,*) 'DAM2PI: wrong MNUM= ',MNUM
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
        AMPLIT= GFERMI**2       

      ELSEIF (IME.EQ.1) THEN ! flat phase space
C        FLAT PHASE SPACE ONLY;
        DO  I=1,3
         HV(I)=0.0
        ENDDO
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
      COMMON /SETINI/ IFBABAR
      INTEGER        IFBABAR
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

      IF (IFBABAR.EQ.2) THEN 
       CALL GETFF2PIRHO(FF2PIRHO)

       IF (FF2PIRHO.EQ.2) THEN ! Belle, 
C                                  ! all fit parameters, par(1...11), are free
        if(sqrt(pksd).le.2*ampi) pksd=4*ampi**2 ! phase space edge protection
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
      ELSEIF (IFBABAR.eq.1.or.IFBABAR.eq.0) THEN ! BaBar/cleo
        DO K=1,4 
         HADCUR(K)=QQ(k)* sqrt(FPIRHO(sqrt(pksd)))
        ENDDO

      ELSE
        write(*,*) 'subroutine CURR_PIPI0: problem in 2-scalars current IFBABAR=',IFBABAR
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
C IFBABAR=0  TAUOLA cleo COEF(I,J) =  COEFc(I,J)
C IIBABAR=2  TAUOLA RChL COEF(I,J) =  COEFr(I,J)
      COMMON /SETINI/ IFBABAR
      INTEGER         IFBABAR
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

       COEFc(1,0)= 2.0*SQRT(2.)/3.0
       COEFc(2,0)=-2.0*SQRT(2.)/3.0
C AJW 2/98: Add in the D-wave and I=0 3pi substructure:
       COEFc(3,0)= 2.0*SQRT(2.)/3.0
       COEFc(4,0)= FPIc
       COEFc(5,0)= 0.0
C
       COEFc(1,1)=-SQRT(2.)/3.0
       COEFc(2,1)= SQRT(2.)/3.0
       COEFc(3,1)= 0.0
       COEFc(4,1)= FPIc
       COEFc(5,1)= SQRT(2.)

C
       COEFc(1,2)=-SQRT(2.)/3.0
       COEFc(2,2)= SQRT(2.)/3.0
       COEFc(3,2)= 0.0
       COEFc(4,2)= 0.0
       COEFc(5,2)=-SQRT(2.)


C AJW 11/97: Add in the K*-prim-s, ala Finkemeier&Mirkes
      IF (IFBABAR.eq.1) then ! BaBar
       COEFc(1,3)= 0.0
       COEFc(2,3)= -1.0
       COEFc(3,3)= 0.0
      ELSE  ! CLEO
       COEFc(1,3)= 1./3.
       COEFc(2,3)= -2./3.
       COEFc(3,3)= 2./3.
      endif
       COEFc(4,3)= 0.0
       COEFc(5,3)= 0.0
C
       COEFc(1,4)= 1.0/SQRT(2.)/3.0
       COEFc(2,4)=-1.0/SQRT(2.)/3.0
       COEFc(3,4)= 0.0
       COEFc(4,4)= 0.0
       COEFc(5,4)= 0.0
C
       COEFc(1,5)=-SQRT(2.)/3.0
       COEFc(2,5)= SQRT(2.)/3.0
       COEFc(3,5)= 0.0
       COEFc(4,5)= 0.0
       COEFc(5,5)=-SQRT(2.)
C
C AJW 11/97: Add in the K*-prim-s, ala Finkemeier&Mirkes
      IF (IFBABAR.eq.1) then ! BaBar
       COEFc(1,6)= 0.0
       COEFc(2,6)= -1.0
       COEFc(3,6)= 0.0
      ELSE  ! CLEO
       COEFc(1,6)= 1./3.
       COEFc(2,6)= -2./3.
       COEFc(3,6)= 2./3.
      endif

       COEFc(4,6)= 0.0
       COEFc(5,6)=-2.0
C
       COEFc(1,7)= 0.0
       COEFc(2,7)= 0.0
       COEFc(3,7)= 0.0
       COEFc(4,7)= 0.0
       COEFc(5,7)=-SQRT(2.0/3.0)

      ENDIF
         
      IF (J.GE.11) THEN 
       COEF=COEFc(I,0)   ! these modes are not initialized
      ELSEIF (IFBABAR.EQ.1.or.IFBABAR.eq.0.OR.(.not.(J.EQ.9.OR.J.EQ.0.OR.J.EQ.10))) THEN   ! so far rchl only for 3pi modes
       JJ=J
       IF(J.GE.9) JJ=0
       COEF=COEFc(I,JJ)
      ELSEIF (IFBABAR.EQ.2) THEN
       COEF=COEFrr(I,J)
      ELSE
       write(*,*) 'function COEF: wrong IFBABAR=',IFBABAR
       stop
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
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      include 'TAUDCD_BBB_size.inc'
      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
      REAL*8 WETMAX(NBMC)
C


      REAL*8  PN(4),PR(4),PPI(4,9),HV(4)
      REAL*4  PNX(4),PRX(4),PPIX(4,9),HVX(4)
      REAL*8  PV(5,9),PT(4),UE(3),BE(3)
      REAL*8  PAWT,AMX,AMS1,AMS2,PA,PHS,PHSMAX,PMIN,PMAX
!!! M.S. to fix underflow >>>
      REAL*8  PHSPAC
!!! M.S. to fix underflow <<<
      REAL*8  GAM,BEP,PHI,A,B,C
      REAL*8  AMPIK
      REAL*4 RRR(9),RRX(2),RN(1),RR2(1)
C
      DATA PI /3.141592653589793238462643/
      DATA WETMAX /500*1D-15/
C
CC--      PAWT(A,B,C)=SQRT((A**2-(B+C)**2)*(A**2-(B-C)**2))/(2.*A)
C
      PAWT(A,B,C)=
     $  SQRT(MAX(0.D0,(A**2-(B+C)**2)*(A**2-(B-C)**2)))/(2.D0*A)

C
      AMPIK(I,J)=DCDMAS(IDFFIN(I,J))
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
      PT(4)=AMTAU
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
      AMS2=(AMTAU-AMNUTA)**2
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
      PN(4)=1./(2*AMTAU)*(AMTAU**2+AMNUTA**2-AMX2)
      PN(3)=-SQRT(ABS((PN(4)-AMNUTA)*(PN(4)+AMNUTA)))
C W MOMENTUM
      PR(1)=0
      PR(2)=0
      PR(4)=1./(2*AMTAU)*(AMTAU**2-AMNUTA**2+AMX2)
      PR(3)=-PN(3)
      PHSPAC=PHSPAC * (4.*PI) * (2.*PR(3)/AMTAU)
C
C AMPLITUDE  (cf YS.Tsai Phys.Rev.D4,2821(1971)
C    or F.Gilman SH.Rhie Phys.Rev.D31,1066(1985)
C
      PXQ=AMTAU*PR(4)
      PXN=AMTAU*PN(4)
      QXN=PR(4)*PN(4)-PR(1)*PN(1)-PR(2)*PN(2)-PR(3)*PN(3)

C HERE WAS AN ERROR. 20.10.91 (ZW)
C       BRAK=2*(GV**2+GA**2)*(2*PXQ*PXN+AMX2*QXN)

        BRAK=2*(GV**2+GA**2)*(2*PXQ*QXN+AMX2*PXN)
     &      -6*(GV**2-GA**2)*AMTAU*AMNUTA*AMX2
CAM     Assume neutrino mass=0. and sum over final polarisation
C     BRAK= 2*(AMTAU**2-AMX2) * (AMTAU**2+2.*AMX2)

!       if(jn.le.6) write(*,*) 'sigeje=',jn,amx2,jn,SIGEE(AMX2,JN)
!       if(jn.eq.7) stop
      IF (IME.EQ.2) THEN
       AMPLIT=CCABIB**2*GFERMI**2/2.* BRAK*AMX2*SIGEE(AMX2,JN)
      ELSEIF (IME.EQ.4) THEN
       AMPLIT=CCABIB**2*GFERMI**2/2.* BRAK*AMX2*SIGEE_wrap(AMX2,JN)
      ELSEIF (IME.EQ.5) THEN
       WRITE(*,*) 'STOP from DPHNPI of TAUOLA FORTRAN.'
       WRITE(*,*) 'For multipion modes IME=5 is not allowed.'
       WRITE(*,*) 'No matrix element in this cases !'
       STOP

      ELSE
       AMPLIT=CCABIB**2*GFERMI**2/2.*     AMX2*SIGEE(AMX2,JN)
      ENDIF
      DGAMT=1./(2.*AMTAU)*AMPLIT*PHSPAC
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

      REAL  HV(4),PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4),PNPI(4,9)
C --- MASSES OF THE DECAY PRODUCTS
       AMP1=DCDMAS(IDFFIN(1,JNPI))
       AMP2=DCDMAS(IDFFIN(2,JNPI))
       AMP3=DCDMAS(IDFFIN(3,JNPI))

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


      SUBROUTINE DPHTRE(DGAMT,HV,PN,PAA,PIM1,AMPA,PIM2,AMPB,PIPL,AMP3,MNUM)
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
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL  HV(4),PT(4),PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4)
      REAL  PR(4)
      REAL*4 RRR(5)
      DATA PI /3.141592653589793238462643/
      DATA ICONT /0/
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.0*Y*Z))
C AMRO, GAMRO IS ONLY A PARAMETER FOR GETING HIGHT EFFICIENCY
C
C THREE BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)
      PHSPAC=1./2**17/PI**8
C TAU MOMENTUM
      PT(1)=0.
      PT(2)=0.
      PT(3)=0.
      PT(4)=AMTAU
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
CAM
        RR1=RRR(1)
        AMS1=(AMP1+AMP2+AMP3)**2
        AMS2=(AMTAU-AMNUTA)**2

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
        PHF0=(AMS2-AMS1)
      ENDIF

* RHO RESTFRAME, DEFINE PIPL AND PIM1

        ENQ1=(AM2SQ-AMP2**2+AMP3**2)/(2*AM2)
        ENQ2=(AM2SQ+AMP2**2-AMP3**2)/(2*AM2)
        PPI=         ENQ1**2-AMP3**2
        PPPI=SQRT(ABS(ENQ1**2-AMP3**2))
C --- this part of jacobian will be recovered later
        PHF1=(4*PI)*(2*PPPI/AM2)

* PI MINUS MOMENTUM IN RHO REST FRAME

        CALL SPHERA(PPPI,PIPL)
        PIPL(4)=ENQ1

* PI0 1 MOMENTUM IN RHO REST FRAME

        DO 30 I=1,3
 30     PIM1(I)=-PIPL(I)
        PIM1(4)=ENQ2

* A1 REST FRAME, DEFINE PIM2

*       RHO  MOMENTUM
        PR(1)=0
        PR(2)=0
        PR(4)=1./(2*AM3)*(AM3**2+AM2**2-AMP1**2)
        PR(3)= SQRT(ABS(PR(4)**2-AM2**2))
        PPI  =          PR(4)**2-AM2**2
*       PI0 2 MOMENTUM
        PIM2(1)=0
        PIM2(2)=0
        PIM2(4)=1./(2*AM3)*(AM3**2-AM2**2+AMP1**2)
        PIM2(3)=-PR(3)
      PHF2=(4*PI)*(2*PR(3)/AM3)

* OLD PIONS BOOSTED FROM RHO REST FRAME TO A1 REST FRAME

      EXE=(PR(4)+PR(3))/AM2
      CALL BOSTR3(EXE,PIPL,PIPL)
      CALL BOSTR3(EXE,PIM1,PIM1)
      RR3=RRR(3)
      RR4=RRR(4)

CAM   THET =PI*RR3

      THET =ACOS(-1.+2*RR3)
      PHI = 2*PI*RR4
      CALL ROTPOL(THET,PHI,PIPL)
      CALL ROTPOL(THET,PHI,PIM1)
      CALL ROTPOL(THET,PHI,PIM2)
      CALL ROTPOL(THET,PHI,PR)
C
* NOW TO THE TAU REST FRAME, DEFINE A1 AND NEUTRINO MOMENTA
* A1  MOMENTUM
      PAA(1)=0
      PAA(2)=0
      PAA(4)=1./(2*AMTAU)*(AMTAU**2-AMNUTA**2+AM3**2)
      PAA(3)= SQRT(ABS(PAA(4)**2-AM3**2))
      PPI   =          PAA(4)**2-AM3**2
      PHSPAC=PHSPAC*(4*PI)*(2*PAA(3)/AMTAU)
* TAU-NEUTRINO MOMENTUM
      PN(1)=0
      PN(2)=0
      PN(4)=1./(2*AMTAU)*(AMTAU**2+AMNUTA**2-AM3**2)
      PN(3)=-PAA(3)
C HERE WE CORRECT FOR THE JACOBIANS OF THE TWO CHAINS
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
       GG1   =       (4*PI)*(XLAM(AM2SQ,AMP2**2,AMP3**2)/AM2SQ)
C LAMBDA OF A1 DECAY
       GG1   =GG1   *(4*PI)*SQRT(4*XPRO/AM3SQ)
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
       GG2   =       (4*PI)*(XLAM(AM2SQ,AMP1**2,AMP3**2)/AM2SQ)
       GG2   =GG2   *(4*PI)*SQRT(4*XPRO/AM3SQ)
       XJADW=GG2*(AMS2-AMS1)
C
       A1=0.0
       A2=0.0
       A3=0.0
       XJAC1=FF1*GG1
       XJAC2=FF2*GG2
       IF (ICHAN.EQ.2) THEN
         XJAC3=XJADW
       ELSE
         XJAC3=XJAJE
       ENDIF
       IF (XJAC1.NE.0.0) A1=PROB1/XJAC1
       IF (XJAC2.NE.0.0) A2=PROB2/XJAC2
       IF (XJAC3.NE.0.0) A3=PROB3/XJAC3
C
       IF (A1+A2+A3.NE.0.0) THEN
         PHSPAC=PHSPAC/(A1+A2+A3)
       ELSE
         PHSPAC=0.0
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
      CALL BOSTR3(EXE,PIPL,PIPL)
      CALL BOSTR3(EXE,PIM1,PIM1)
      CALL BOSTR3(EXE,PIM2,PIM2)
      CALL BOSTR3(EXE,PR,PR)

      IF(MNUM.EQ.9) THEN
C THE STATISTICAL FACTOR FOR IDENTICAL PI-S
        PHSPAC=PHSPAC/2.
        CALL CH3PISET(2) ! information that it's pi- pi0 pi0 passed for further use
      ELSEIF(MNUM.EQ.0.OR.MNUM.EQ.10) THEN   ! MNUM=0 probably never hap
C THE STATISTICAL FACTOR FOR IDENTICAL PI-S
        PHSPAC=PHSPAC/2.
        CALL CH3PISET(1) ! information that it's pi- pi- pi+ passed for further use
      ENDIF

C PARTIAL WIDTH CONSISTS OF PHASE SPACE AND AMPLITUDE
       CALL DAM3PI(MNUM,PT,PN,PIM1,PIM2,PIPL,AMPLIT,HV)
!        if (mnum.eq.9) write(*,*) 'mnum=',mnum,amplit,phspac
!        if (amplit.eq.0.0) stop
!      if (mnum.gt.7) write(*,*) 'mnumy=',mnum

      DGAMT=1/(2.*AMTAU)*AMPLIT*PHSPAC
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

      COMMON /SETINI/ IFBABAR
      INTEGER         IFBABAR

      REAL  PIM1(4),PIM2(4),PIM3(4)
      REAL  PAA(4),VEC1(4),VEC2(4),VEC3(4),VEC4(4),VEC5(4)

      REAL FNORM(0:19)
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
 
       IF (IFBABAR.EQ.1.or.IFBABAR.eq.0.OR.(.not.(MNUM.EQ.9.OR.MNUM.EQ.0))) THEN ! so far rchl only for 3pi modes
         FPI=FPIc
        ELSEIF (IFBABAR.EQ.2) THEN
         FPI=GETFPIRPT(1) ! GET  defined in in ffwid3pi.f  of RChL-currents
        ELSE
         write(*,*) 'subroutine CURR3PI: wrong IFBABAR=',IFBABAR
         stop
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
       DO K=10,19
         FNORM(K)=FNORM(9) ! these chanells are not initialized
       ENDDO

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
       IF (MNUM.GE.20) THEN   ! dead code under this if
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

      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL

      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
      include 'TAUDCD_BBB_size.inc'
      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31


      REAL  PKK(4),PNU(4),HV(4),PNPI(4,9),PWB(4)
      DATA PI /3.141592653589793238462643/
      INUM=JNPI-NM4-NM5-NM6-NM3-NM2

      AMF1= DCDMAS(IDFFIN(1,JNPI))
      AMF0=AMNUTA
      IF(INUM.GE.3.AND.IDFFIN(3,JNPI).NE.0) AMF0= DCDMAS(IDFFIN(3,JNPI))
      IF(INUM.GE.3.AND.IDFFIN(4,JNPI).NE.0) AMF0= DCDMAS(IDFFIN(4,JNPI))
        EKK= (AMTAU**2+AMF1**2-AMF0**2)/(2*AMTAU)
        ENU= (AMTAU**2-AMF1**2+AMF0**2)/(2*AMTAU)
        XKK= SQRT(EKK**2-AMF1**2)
C K MOMENTUM
        CALL SPHERA(XKK,PKK)
        PKK(4)=EKK
C TAU-NEUTRINO MOMENTUM
        DO 30 I=1,3
30      PNU(I)=-PKK(I)
        PNU(4)=ENU

 
       CALL DAM1PI(INUM,PNU,AMF0,PKK,AMF1,GAMM,HV)
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
        HV(4)=1

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
         RETURN
       ELSEIF (IME.EQ.0) THEN    ! not initialized
         GAMM=GFERMI**2
         DO  I=1,3
          HV(I)=0.0
         ENDDO
       ELSE    ! any other; flat phase space
         GAMM=GFERMI**2
         DO  I=1,3
          HV(I)=0.0
         ENDDO
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


      SUBROUTINE DPH4PI(DGAMT,HV,PN,PAA,PMULT,JNPI)
C ----------------------------------------------------------------------

* IT SIMULATES A1  DECAY IN TAU REST FRAME WITH
* Z-AXIS ALONG A1  MOMENTUM

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
      include 'TAUDCD_BBB_size.inc'
      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31


      REAL  HV(4),PT(4),PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4),PMULT(4,9)
      REAL  PR(4),PIZ(4)
      REAL*4 RRR(9)
      REAL*8 UU,FF,FF1,FF2,FF3,FF4,GG1,GG2,GG3,GG4,RR
      DATA PI /3.141592653589793238462643/
      DATA ICONT /0/
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.0*Y*Z))
C AMRO, GAMRO IS ONLY A PARAMETER FOR GETING HIGHT EFFICIENCY
C
C THREE BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)
      PHSPAC=1./2**23/PI**11
      PHSP=1./2**5/PI**2
      AMP1=DCDMAS(IDFFIN(1,JNPI))    ! mass for  PIM2
      AMP2=DCDMAS(IDFFIN(2,JNPI))    ! mass for  PIM1
      AMP3=DCDMAS(IDFFIN(3,JNPI))    ! mass for  PIPL
      AMP4=DCDMAS(IDFFIN(4,JNPI))    ! mass for  PIZ

 
      CALL CHOICE4(JNPI,PROB1,PROB2,AMROP,GAMROP,AMRX,GAMRX)
      PREZ=PROB1+PROB2
C TAU MOMENTUM
      PT(1)=0.
      PT(2)=0.
      PT(3)=0.
      PT(4)=AMTAU
C
      CALL KKMC_RANMAR(RRR,9)
C
* MASSES OF 4, 3 AND 2 PI SYSTEMS
C 3 PI WITH SAMPLING FOR RESONANCE
CAM
        RR1=RRR(6)
        AMS1=(AMP1+AMP2+AMP3+AMP4)**2
        AMS2=(AMTAU-AMNUTA)**2
        ALP1=ATAN((AMS1-AMROP**2)/AMROP/GAMROP)
        ALP2=ATAN((AMS2-AMROP**2)/AMROP/GAMROP)
        ALP=ALP1+RR1*(ALP2-ALP1)
        AM4SQ =AMROP**2+AMROP*GAMROP*TAN(ALP)
        AM4 =SQRT(AM4SQ)
        PHSPAC=PHSPAC*
     $         ((AM4SQ-AMROP**2)**2+(AMROP*GAMROP)**2)/(AMROP*GAMROP)
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
* FLAT PHASE SPACE;
        AM2SQ=AMS1+   RR2*(AMS2-AMS1)
        AM2 =SQRT(AM2SQ)
C --- this part of jacobian will be recovered later
        FF2=(AMS2-AMS1)
*  2 RESTFRAME, DEFINE PIZ AND PIPL

        ENQ1=(AM2SQ-AMP3**2+AMP4**2)/(2*AM2)
        ENQ2=(AM2SQ+AMP3**2-AMP4**2)/(2*AM2)
        PPI=         ENQ1**2-AMP4**2
        PPPI=SQRT(ABS(ENQ1**2-AMP4**2))

        PHSPAC=PHSPAC*(4*PI)*(2*PPPI/AM2)

* PIZ   MOMENTUM IN 2 REST FRAME

        CALL SPHERA(PPPI,PIZ)
        PIZ(4)=ENQ1

* PIPL  MOMENTUM IN 2 REST FRAME

        DO 30 I=1,3
 30     PIPL(I)=-PIZ(I)
        PIPL(4)=ENQ2
* 3 REST FRAME, DEFINE PIM1

*       PR   MOMENTUM

        PR(1)=0
        PR(2)=0
        PR(4)=1./(2*AM3)*(AM3**2+AM2**2-AMP2**2)
        PR(3)= SQRT(ABS(PR(4)**2-AM2**2))
        PPI  =          PR(4)**2-AM2**2

*       PIM1  MOMENTUM

        PIM1(1)=0
        PIM1(2)=0
        PIM1(4)=1./(2*AM3)*(AM3**2-AM2**2+AMP2**2)
        PIM1(3)=-PR(3)
C --- this part of jacobian will be recovered later
        FF3=(4*PI)*(2*PR(3)/AM3)
* OLD PIONS BOOSTED FROM 2 REST FRAME TO 3 REST FRAME
      EXE=(PR(4)+PR(3))/AM2
      CALL BOSTR3(EXE,PIZ,PIZ)
      CALL BOSTR3(EXE,PIPL,PIPL)
      RR3=RRR(3)
      RR4=RRR(4)
      THET =ACOS(-1.+2*RR3)
      PHI = 2*PI*RR4
      CALL ROTPOL(THET,PHI,PIPL)
      CALL ROTPOL(THET,PHI,PIM1)
      CALL ROTPOL(THET,PHI,PIZ)
      CALL ROTPOL(THET,PHI,PR)

* 4  REST FRAME, DEFINE PIM2
*       PR   MOMENTUM

        PR(1)=0
        PR(2)=0
        PR(4)=1./(2*AM4)*(AM4**2+AM3**2-AMP1**2)
        PR(3)= SQRT(ABS(PR(4)**2-AM3**2))
        PPI  =          PR(4)**2-AM3**2

*       PIM2 MOMENTUM

        PIM2(1)=0
        PIM2(2)=0
        PIM2(4)=1./(2*AM4)*(AM4**2-AM3**2+AMP1**2)
        PIM2(3)=-PR(3)
C --- this part of jacobian will be recovered later
        FF4=(4*PI)*(2*PR(3)/AM4)
* OLD PIONS BOOSTED FROM 3 REST FRAME TO 4 REST FRAME
      EXE=(PR(4)+PR(3))/AM3
      CALL BOSTR3(EXE,PIZ,PIZ)
      CALL BOSTR3(EXE,PIPL,PIPL)
      CALL BOSTR3(EXE,PIM1,PIM1)
      RR3=RRR(7)
      RR4=RRR(8)
      THET =ACOS(-1.+2*RR3)
      PHI = 2*PI*RR4
      CALL ROTPOL(THET,PHI,PIPL)
      CALL ROTPOL(THET,PHI,PIM1)
      CALL ROTPOL(THET,PHI,PIM2)
      CALL ROTPOL(THET,PHI,PIZ)
      CALL ROTPOL(THET,PHI,PR)
C
* NOW TO THE TAU REST FRAME, DEFINE PAA AND NEUTRINO MOMENTA
* PAA  MOMENTUM
      PAA(1)=0
      PAA(2)=0
      PAA(4)=1./(2*AMTAU)*(AMTAU**2-AMNUTA**2+AM4**2)
      PAA(3)= SQRT(ABS(PAA(4)**2-AM4**2))
      PPI   =          PAA(4)**2-AM4**2
      PHSPAC=PHSPAC*(4*PI)*(2*PAA(3)/AMTAU)
      PHSP=PHSP*(4*PI)*(2*PAA(3)/AMTAU)
* TAU-NEUTRINO MOMENTUM
      PN(1)=0
      PN(2)=0
      PN(4)=1./(2*AMTAU)*(AMTAU**2+AMNUTA**2-AM4**2)
      PN(3)=-PAA(3)
C ZBW 20.12.2002 bug fix
        IF(RRR(9).LE.0.5*PREZ) THEN
         DO 72 I=1,4
         X=PIM1(I)
         PIM1(I)=PIM2(I)
 72      PIM2(I)=X
        ENDIF           
C end of bug fix
C WE INCLUDE REMAINING PART OF THE JACOBIAN
C --- FLAT CHANNEL
        AM3SQ=(PIM1(4)+PIZ(4)+PIPL(4))**2-(PIM1(3)+PIZ(3)+PIPL(3))**2
     $       -(PIM1(2)+PIZ(2)+PIPL(2))**2-(PIM1(1)+PIZ(1)+PIPL(1))**2
        AMS2=(AM4-AMP1)**2
        AMS1=(AMP2+AMP3+AMP4)**2
        FF1=(AMS2-AMS1)
        AMS1=(AMP3+AMP4)**2
        AMS2=(SQRT(AM3SQ)-AMP2)**2
        FF2=AMS2-AMS1
        FF3=(4*PI)*(XLAM(AM2**2,AMP2**2,AM3SQ)/AM3SQ)
        FF4=(4*PI)*(XLAM(AM3SQ,AMP1**2,AM4**2)/AM4**2)
        UU=FF1*FF2*FF3*FF4
C --- FIRST CHANNEL
        AM3SQ=(PIM1(4)+PIZ(4)+PIPL(4))**2-(PIM1(3)+PIZ(3)+PIPL(3))**2
     $       -(PIM1(2)+PIZ(2)+PIPL(2))**2-(PIM1(1)+PIZ(1)+PIPL(1))**2
        AMS2=(AM4-AMP1)**2
        AMS1=(AMP2+AMP3+AMP4)**2
        ALP1=ATAN((AMS1-AMRX**2)/AMRX/GAMRX)
        ALP2=ATAN((AMS2-AMRX**2)/AMRX/GAMRX)
        FF1=((AM3SQ-AMRX**2)**2+(AMRX*GAMRX)**2)/(AMRX*GAMRX)
        FF1=FF1*(ALP2-ALP1)
        AMS1=(AMP3+AMP4)**2
        AMS2=(SQRT(AM3SQ)-AMP2)**2
        FF2=AMS2-AMS1
        FF3=(4*PI)*(XLAM(AM2**2,AMP2**2,AM3SQ)/AM3SQ)
        FF4=(4*PI)*(XLAM(AM3SQ,AMP1**2,AM4**2)/AM4**2)
        FF=FF1*FF2*FF3*FF4
C --- SECOND CHANNEL
        AM3SQ=(PIM2(4)+PIZ(4)+PIPL(4))**2-(PIM2(3)+PIZ(3)+PIPL(3))**2
     $       -(PIM2(2)+PIZ(2)+PIPL(2))**2-(PIM2(1)+PIZ(1)+PIPL(1))**2
        AMS2=(AM4-AMP2)**2
        AMS1=(AMP1+AMP3+AMP4)**2
        ALP1=ATAN((AMS1-AMRX**2)/AMRX/GAMRX)
        ALP2=ATAN((AMS2-AMRX**2)/AMRX/GAMRX)
        GG1=((AM3SQ-AMRX**2)**2+(AMRX*GAMRX)**2)/(AMRX*GAMRX)
        GG1=GG1*(ALP2-ALP1)
        AMS1=(AMP3+AMP4)**2
        AMS2=(SQRT(AM3SQ)-AMP1)**2
        GG2=AMS2-AMS1
        GG3=(4*PI)*(XLAM(AM2**2,AMP1**2,AM3SQ)/AM3SQ)
        GG4=(4*PI)*(XLAM(AM3SQ,AMP2**2,AM4**2)/AM4**2)
        GG=GG1*GG2*GG3*GG4
C --- JACOBIAN AVERAGED OVER THE TWO
C        IF ( ( (FF+GG)*UU+FF*GG ).GT.0.0D0) THEN
        IF ( (0.5*PREZ*(FF+GG)*UU+(1.0-PREZ)*FF*GG).GT.0.0D0) THEN
           RR=FF*GG*UU/(0.5*PREZ*(FF+GG)*UU+(1.0-PREZ)*FF*GG)
           PHSPAC=PHSPAC*RR
        ELSE
          PHSPAC=0.0
        ENDIF
* MOMENTA OF THE TWO PI-MINUS ARE RANDOMLY SYMMETRISED
       IF (JNPI.EQ.1) THEN
        RR5= RRR(5)
        IF(RR5.LE.0.5) THEN
         DO 70 I=1,4
         X=PIM1(I)
         PIM1(I)=PIM2(I)
 70      PIM2(I)=X
        ENDIF
        PHSPAC=PHSPAC/2.
       ELSEIF (JNPI.EQ.2) THEN
C MOMENTA OF PI0-S ARE GENERATED UNIFORMLY ONLY IF PREZ=0.0
        RR5= RRR(5)
        IF(RR5.LE.0.5) THEN
         DO 71 I=1,4
         X=PIM1(I)
         PIM1(I)=PIM2(I)
 71      PIM2(I)=X
        ENDIF
        PHSPAC=PHSPAC/6.
       ELSE
C note that         PIM1(4)=1./(2*AM3)*(AM3**2-AM2**2+AMP2**2)
C in this case it matters PIM1 PIM2 are not particles of identical mass 
         DO  I=1,4
          X=PIM1(I)
          PIM1(I)=PIM2(I)
          PIM2(I)=X
         ENDDO
C        do nothing  ! .OR.JNPI.EQ.3.OR.JNPI.EQ.4
C        case of e-e-e+ nunu  will require new presampler because
C        there are two regions where photon is near real.
C        case e-e+ mu- nunu can be easily treated if order
C        mu- nu e-e+ nu is adopted.
       ENDIF
* ALL PIONS BOOSTED FROM  4  REST FRAME TO TAU REST FRAME
* Z-AXIS ANTIPARALLEL TO NEUTRINO MOMENTUM
      EXE=(PAA(4)+PAA(3))/AM4
      CALL BOSTR3(EXE,PIZ,PIZ)
      CALL BOSTR3(EXE,PIPL,PIPL)
      CALL BOSTR3(EXE,PIM1,PIM1)
      CALL BOSTR3(EXE,PIM2,PIM2)
      CALL BOSTR3(EXE,PR,PR)
C PARTIAL WIDTH CONSISTS OF PHASE SPACE AND AMPLITUDE
C CHECK ON CONSISTENCY WITH DADNPI, THEN, CODE BREAKES UNIFORM PION
C DISTRIBUTION IN HADRONIC SYSTEM

CAM     Assume neutrino mass=0. and sum over final polarisation
C      AMX2=AM4**2
C      BRAK= 2*(AMTAU**2-AMX2) * (AMTAU**2+2.*AMX2)
C      AMPLIT=CCABIB**2*GFERMI**2/2. * BRAK * AMX2*SIGEE(AMX2,1)
      IF     (JNPI.EQ.1) THEN
        CALL DAM4PI(JNPI,PT,PN,PIM1,PIM2,PIZ,PIPL,AMPLIT,HV)
      ELSEIF (JNPI.EQ.2) THEN
        CALL DAM4PI(JNPI,PT,PN,PIM1,PIM2,PIPL,PIZ,AMPLIT,HV)
      ELSE
        CALL DAM4PI(JNPI,PT,PN,PIM1,PIM2,PIPL,PIZ,AMPLIT,HV) ! temporarily
      ENDIF

      DGAMT=1/(2.*AMTAU)*AMPLIT*PHSPAC
C PHASE SPACE CHECK
C      DGAMT=PHSPAC
      DO 77 K=1,4
        PMULT(K,1)=PIM1(K)
        PMULT(K,2)=PIM2(K)

        PMULT(K,3)=PIPL(K)
        PMULT(K,4)=PIZ (K)

 77   CONTINUE
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

      COMMON /SETINI/ IFBABAR
      INTEGER        IFBABAR
      INTEGER        IFKARL
      IME=IMEGET(4,MNUM)   ! type of matrix element to be used

C
!      write(*,*) 'falanti',mnum
      IF (IME.EQ.2) THEN
       IF     (MNUM.EQ.1.OR.MNUM.EQ.2) THEN ! MNUM
        if (IFBABAR.eq.1) then
         CALL CURR(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
        else if (IFBABAR.eq.2) then
          IFKARL=0 ! current choice between Karlsruhe (1), Novosibirsk(0)
          if(IFKARL.eq.1) then 
           CALL CURR_KARLS(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
          else
             if (MNUM.EQ.1) then
              CALL CURR_BINP(MNUM,1,PIM1,PIM4,PIM2,PIM3,HADCUR)
              CALL CURR_BINP(MNUM,7,PIM1,PIM4,PIM2,PIM3,HADCUR1)
              do I=1,4
              HADCUR(I) = HADCUR(I) + HADCUR1(I)
              enddo
             elseif (MNUM.EQ.2) then
              CALL CURR_BINP(MNUM,-1,PIM4,PIM3,PIM1,PIM2,HADCUR)
             endif
          endif
        else  ! IFBABAR
         CALL CURR_CLEO(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
        endif ! IFBABAR
       ELSE   ! MNUM
        CALL CURR(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
       ENDIF  ! MNUM
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

      SUBROUTINE DPH5PI(DGAMT,HV,PN,PAA,PMULT,JNPI)                    
C ----------------------------------------------------------------------
* IT SIMULATES 5pi DECAY IN TAU REST FRAME WITH                         
* Z-AXIS ALONG 5pi MOMENTUM                                             
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
      include 'TAUDCD_BBB_size.inc'

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)

     &                ,NAMES
      CHARACTER NAMES(NMODE)*31
      REAL  HV(4),PT(4),PN(4),PAA(4),PMULT(4,9) 
      REAL*4 PR(4),PI1(4),PI2(4),PI3(4),PI4(4),PI5(4)                   
      REAL*8 AMP1,AMP2,AMP3,AMP4,AMP5,ams1,ams2,amom,gamom
      REAL*8 AM5SQ,AM4SQ,AM3SQ,AM2SQ,AM5,AM4,AM3
      REAL*4 RRR(12)                                                    
      REAL*8 gg1,gg2,gg3,ff1,ff2,ff3,ff4,alp,alp1,alp2

      REAL*8 XM,AM,GAMMA
ccM.S.>>>>>>
      real*8 phspac
ccM.S.<<<<<<

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
      AMP1=DCDMAS(IDFFIN(1,JNPI))
      AMP2=DCDMAS(IDFFIN(2,JNPI))
      AMP3=DCDMAS(IDFFIN(3,JNPI))
      AMP4=DCDMAS(IDFFIN(4,JNPI))
      AMP5=DCDMAS(IDFFIN(5,JNPI))
c                                                                       
C TAU MOMENTUM                                                          
      PT(1)=0.                                                          
      PT(2)=0.                                                          
      PT(3)=0.                                                          
      PT(4)=AMTAU                                                                                                                             
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
        ams2=(amtau-amnuta)**2 
        alp1=atan((ams1-ama2**2)/ama2/gama2)                              
        alp2=atan((ams2-ama2**2)/ama2/gama2)                     
        am5sq=ams1+   rr1*(ams2-ams1)                                     
        am5 =sqrt(am5sq)                                                  
C      phspac=phspac*(ams2-ams1)  
c or peaked phase space  for a1(?) resonance: 
       ELSE
        rr1=rrr(10)                                                       
        ams1=(amp1+amp2+amp3+amp4+amp5)**2                                
        ams2=(amtau-amnuta)**2    
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
      call sphera(pppi,pi3)                                             
      pi3(4)=enq1                                                       
c pi4   momentum in (34) rest frame                                     
      do 30 i=1,3                                                       
 30   pi4(i)=-pi3(i)                                                    
      pi4(4)=enq2                                                       
c                                                                       
c (234) rest frame, define pi2                                          
c pr   momentum                                                         
      pr(1)=0                                                           
      pr(2)=0                                                           
      pr(4)=1./(2*am3)*(am3**2+am2**2-amp2**2)                          
      pr(3)= sqrt(abs(pr(4)**2-am2**2))                                 
      ppi  =          pr(4)**2-am2**2                                   
c pi2   momentum                                                        
      pi2(1)=0                                                          
      pi2(2)=0                                                          
      pi2(4)=1./(2*am3)*(am3**2-am2**2+amp2**2)                         
      pi2(3)=-pr(3)                                                     
c --- this part of jacobian will be recovered later                     
      ff2=(4*pi)*(2*pr(3)/am3)                                          
c old pions boosted from 2 rest frame to 3 rest frame                   
      exe=(pr(4)+pr(3))/am2                                             
      call bostr3(exe,pi3,pi3)                                          
      call bostr3(exe,pi4,pi4)                                          
      rr3=rrr(3)                                                        
      rr4=rrr(4)                                                        
      thet =acos(-1.+2*rr3)                                             
      phi = 2*pi*rr4                                                    
      call rotpol(thet,phi,pi2)                                         
      call rotpol(thet,phi,pi3)                                         
      call rotpol(thet,phi,pi4)                                         
C                                                                       
C (2345)  rest frame, define pi5                                        
c pr   momentum                                                         
      pr(1)=0                                                           
      pr(2)=0                                                           
      pr(4)=1./(2*am4)*(am4**2+am3**2-amp5**2)                          
      pr(3)= sqrt(abs(pr(4)**2-am3**2))                                 
      ppi  =          pr(4)**2-am3**2                                   
c pi5  momentum                                                         
      pi5(1)=0                                                          
      pi5(2)=0                                                          
      pi5(4)=1./(2*am4)*(am4**2-am3**2+amp5**2)                         
      pi5(3)=-pr(3)                                                     
c --- this part of jacobian will be recovered later                     
      ff3=(4*pi)*(2*pr(3)/am4)                                          
c old pions boosted from 3 rest frame to 4 rest frame                   
      exe=(pr(4)+pr(3))/am3                                             
      call bostr3(exe,pi2,pi2)                                          
      call bostr3(exe,pi3,pi3)                                          
      call bostr3(exe,pi4,pi4)                                          
      rr3=rrr(5)                                                        
      rr4=rrr(6)                                                        
      thet =acos(-1.+2*rr3)                                             
      phi = 2*pi*rr4                                                    
      call rotpol(thet,phi,pi2)                                         
      call rotpol(thet,phi,pi3)                                         
      call rotpol(thet,phi,pi4)                                         
      call rotpol(thet,phi,pi5)                                         
C                                                                       
C (12345)  rest frame, define pi1                                       
c pr   momentum                                                         
      pr(1)=0                                                           
      pr(2)=0                                                           
      pr(4)=1./(2*am5)*(am5**2+am4**2-amp1**2)                          
      pr(3)= sqrt(abs(pr(4)**2-am4**2))                                 
      ppi  =          pr(4)**2-am4**2                                   
c pi1  momentum                                                         
      pi1(1)=0                                                          
      pi1(2)=0                                                          
      pi1(4)=1./(2*am5)*(am5**2-am4**2+amp1**2)                         
      pi1(3)=-pr(3)                                                     
c --- this part of jacobian will be recovered later                     
      ff4=(4*pi)*(2*pr(3)/am5)                                          
c old pions boosted from 4 rest frame to 5 rest frame                   
      exe=(pr(4)+pr(3))/am4                                             
      call bostr3(exe,pi2,pi2)                                          
      call bostr3(exe,pi3,pi3)                                          
      call bostr3(exe,pi4,pi4)                                          
      call bostr3(exe,pi5,pi5)                                          
      rr3=rrr(7)                                                        
      rr4=rrr(8)                                                        
      thet =acos(-1.+2*rr3)                                             
      phi = 2*pi*rr4                                                    
      call rotpol(thet,phi,pi1)                                         
      call rotpol(thet,phi,pi2)                                         
      call rotpol(thet,phi,pi3)                                         
      call rotpol(thet,phi,pi4)                                         
      call rotpol(thet,phi,pi5)                                         
c                                                                       
* now to the tau rest frame, define paa and neutrino momenta            
* paa  momentum                                                         
      paa(1)=0                                                          
      paa(2)=0                                                          
c     paa(4)=1./(2*amtau)*(amtau**2-amnuta**2+am5**2)                   
c     paa(3)= sqrt(abs(paa(4)**2-am5**2))                               
c     ppi   =          paa(4)**2-am5**2                                 
      paa(4)=1./(2*amtau)*(amtau**2-amnuta**2+am5sq)                    
      paa(3)= sqrt(abs(paa(4)**2-am5sq))                                
      ppi   =          paa(4)**2-am5sq                                  
      phspac=phspac*(4*pi)*(2*paa(3)/amtau)                             
* tau-neutrino momentum                                                 
      pn(1)=0                                                           
      pn(2)=0                                                           
      pn(4)=1./(2*amtau)*(amtau**2+amnuta**2-am5**2)                    
      pn(3)=-paa(3)                                                     
c                                                                       
      phspac=phspac * gg1*gg2*gg3*ff1*ff2*ff3*ff4                       
c                                                                       
C all pions boosted from  5  rest frame to tau rest frame               
C z-axis antiparallel to neutrino momentum                              
      exe=(paa(4)+paa(3))/am5                                           
      call bostr3(exe,pi1,pi1)                                          
      call bostr3(exe,pi2,pi2)                                          
      call bostr3(exe,pi3,pi3)                                          
      call bostr3(exe,pi4,pi4)                                          
      call bostr3(exe,pi5,pi5)                                          
c                                                                       
C partial width consists of phase space and amplitude                   
C AMPLITUDE  (cf YS.Tsai Phys.Rev.D4,2821(1971)                         
C    or F.Gilman SH.Rhie Phys.Rev.D31,1066(1985)                        
C                                                                       
      PXQ=AMTAU*PAA(4)                                                  
      PXN=AMTAU*PN(4)                                                   
      QXN=PAA(4)*PN(4)-PAA(1)*PN(1)-PAA(2)*PN(2)-PAA(3)*PN(3)           
      BRAK=2*(GV**2+GA**2)*(2*PXQ*QXN+AM5SQ*PXN)                        
     &    -6*(GV**2-GA**2)*AMTAU*AMNUTA*AM5SQ                           
      fompp = cabs(bwign(am3,amom,gamom))**2                            
c normalisation factor (to some numerical undimensioned factor;         
c cf R.Fischer et al ZPhys C3, 313 (1980))                              
      fnorm = 1/fpi**6                                                  
c     AMPLIT=CCABIB**2*GFERMI**2/2. * BRAK * AM5SQ*SIGEE(AM5SQ,JNPI)    
      AMPLIT=CCABIB**2*GFERMI**2/2. * BRAK !* (1D0*(jnpi-12))                             
      amplit = amplit * fompp * fnorm                                   
c phase space test                                                      
c     amplit = amplit * fnorm                                           

!      write(*,*) '5pi jnpi=',jnpi                                  
c ignore spin terms                                                     
      DO 40 I=1,3                                                       
 40   HV(I)=0.       
                             
!      write(*,*) jnpi
!      stop

      if (INUM.gt.1) ! for the time being we want to keep old wrong m.e.
     $ CALL DAM5PI(INUM,PT,PN,PI1,PI2,PI3,PI4,PI5,AMPLIT,HV)
      DGAMT=1/(2.*AMTAU)*AMPLIT*PHSPAC
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
                                                    
      SUBROUTINE DPH2PI(DGAMT,HV,PN,PR,PMULT,JNPI)
C ----------------------------------------------------------------------
C IT SIMULATES RHO DECAY IN TAU REST FRAME WITH                         
C Z-AXIS ALONG RHO MOMENTUM                                             
C Rho decays to K Kbar       
C WARNING: DPH2PI routine is missing 2 scalar ME calculation                
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
      include 'TAUDCD_BBB_size.inc'

      COMMON / TAUDCD /IDFFIN(JMAX,NMODE),MULPIK(NMODE)
     &                ,NAMES
      CHARACTER NAMES(NMODE)*31

      REAL  HV(4),PT(4),PN(4),PR(4),PKC(4),PKZ(4),QQ(4),PMULT(4,9)

      REAL RR1(1),RR2(1)

      DATA PI /3.141592653589793238462643/                              
      DATA ICONT /0/                                                    
C       
      INUM=JNPI-NM3-NM4- NM5- NM6

      AMF1= DCDMAS(IDFFIN(1,JNPI))
      AMF2  = DCDMAS(IDFFIN(2,JNPI))  
      AMF0=AMNUTA
!      IF(INUM.GT.4) THEN
!       write(*,*) inum,':',IDFFIN(1,IADDR),IDFFIN(2,IADDR),IDFFIN(3,IADDR),DCDMAS(IDFFIN(3,IADDR))
!      ENDIF
      IF(INUM.GT.3.AND.IDFFIN(3,JNPI).NE.0) AMF0= DCDMAS(IDFFIN(3,JNPI))

!      write(*,*) 'pajacna', INUM,  IDFFIN(1,IADDR),  IDFFIN(2,IADDR),  IDFFIN(3,IADDR)   
!      write(*,*) 'pajacna', INUM,  DCDMAS(IDFFIN(1,IADDR)),  DCDMAS(IDFFIN(2,IADDR)),  IDFFIN(3,IADDR)
!       write(*,*) 'pajacna', AMK,AMKZ  
!       write(*,*) 'pajacna',AMF1,AMF2
!      STOP                        
C THREE BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL                 
      PHSPAC=1./2**11/PI**5      
C TAU MOMENTUM                                                          
      PT(1)=0.                                                          
      PT(2)=0.                                                          
      PT(3)=0.                                                          
      PT(4)=AMTAU                                                       
C MASS OF (REAL/VIRTUAL) RHO                                            
      AMS1=(AMF1+AMF2)**2                                                
      AMS2=(AMTAU-AMF0)**2                                            
C FLAT PHASE SPACE                                                      
 100  CONTINUE                                                          
      CALL KKMC_RANMAR(RR1,1)                                                
      CALL CHOICE2(INUM,PROB1,PROB2,AM2,GAM2,AM3,GAM3)
        ALP1 =ATAN((AMS1-AM2**2)/AM2/GAM2)
        ALP2 =ATAN((AMS2-AM2**2)/AM2/GAM2)
        ALP1R=ATAN((AMS1-AM3**2)/AM3/GAM3)                              
        ALP2R=ATAN((AMS2-AM3**2)/AM3/GAM3)                              

        CALL KKMC_RANMAR(RR2,1)
!        RR2(1)=0.03
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
        IF(AMX.GE.(AMTAU-AMF0)) GO TO 100
C merging of the three channels
        PHSPAC1=(AMS2-AMS1)

        PHSPAC2=((AMX2-AM2**2)**2+(AM2*GAM2)**2)
     &                /(AM2*GAM2)
        PHSPAC2=PHSPAC2*(ALP2-ALP1)

        PHSPAC3=((AMX2-AM3**2)**2+(AM3*GAM3)**2)/(AM3*GAM3)    
        PHSPAC3=PHSPAC3*(ALP2R-ALP1R)                                         

        A1=0.0
        A2=0.0
        A3=0.0
        IF (PHSPAC1.NE.0.0) A1=PROB1          /PHSPAC1
        IF (PHSPAC2.NE.0.0) A2=PROB2          /PHSPAC2
        IF (PHSPAC3.NE.0.0) A3=(1-PROB1-PROB2)/PHSPAC3
        IF (A1+A2+A3.NE.0.0) THEN
         PHSPAC=PHSPAC/(A1+A2+A3)
        ELSE
         PHSPAC=0
        ENDIF
                               
      PN(1)=0                                                           
      PN(2)=0                                                           
      PN(4)=1./(2*AMTAU)*(AMTAU**2+AMF0**2-AMX**2)                    
      PN(3)=-SQRT((PN(4)-AMF0)*(PN(4)+AMF0))                        
C RHO MOMENTUM                                                          
      PR(1)=0                                                           
      PR(2)=0                                                           
      PR(4)=1./(2*AMTAU)*(AMTAU**2-AMF0**2+AMX**2)                    
      PR(3)=-PN(3)                                                      
      PHSPAC=PHSPAC*(4*PI)*(2*PR(3)/AMTAU)                              
C                                                                       
CAM                                                                     
      ENQ1=(AMX2+AMF1**2-AMF2**2)/(2.*AMX)                               
      ENQ2=(AMX2-AMF1**2+AMF2**2)/(2.*AMX)                               
      PPPI=SQRT((ENQ1-AMF1)*(ENQ1+AMF1))                                  
      PHSPAC=PHSPAC*(4*PI)*(2*PPPI/AMX)                                 
C CHARGED PI MOMENTUM IN RHO REST FRAME                                 
      CALL SPHERA(PPPI,PKC)                                             
      PKC(4)=ENQ1                                                       
C NEUTRAL PI MOMENTUM IN RHO REST FRAME                                 
      DO 20 I=1,3                                                       
20    PKZ(I)=-PKC(I)                                                    
      PKZ(4)=ENQ2                                                       
      EXE=(PR(4)+PR(3))/AMX                                             
C PIONS BOOSTED FROM RHO REST FRAME TO TAU REST FRAME                   
      CALL BOSTR3(EXE,PKC,PKC)                                          
      CALL BOSTR3(EXE,PKZ,PKZ)                                          
!      write(*,*) 'inum=',inum

!      if (inn.gt.3) inn=3   ! for higher inn channels flat phase space
      CALL DAM2PI(INUM,PT,PN,PKC,PKZ,AMPLIT,HV)
      DGAMT=1/(2.*AMTAU)*AMPLIT*PHSPAC 

      do 77 k=1,4                                                       
        pmult(k,1)=pkc(k)
        pmult(k,2)=pkz(k)
 77   continue           
      RETURN             
      END                
      SUBROUTINE DWLNEW(KTO,ISGN,PNU,PWB,PNPI,MODE)
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
        CALL FILHEP(0,1,-IDFFIN(3,JNPI)*ISGN,-IS,-IS,0,0,PNU,AM,.TRUE.)
      ELSEIF(ND.EQ.2.AND.IDFFIN(4,JNPI).NE.0) THEN
        IS=1
        CALL FILHEP(0,1, IDFFIN(4,JNPI)     ,-IS,-IS,0,0,PNU,AM,.TRUE.)
      ELSEIF(ND.EQ.1.AND.IDFFIN(3,JNPI).NE.0) THEN
        IS=1
        CALL FILHEP(0,1,-IDFFIN(3,JNPI)*ISGN,-IS,-IS,0,0,PNU,AM,.TRUE.)
      ELSEIF(ND.EQ.1.AND.IDFFIN(4,JNPI).NE.0) THEN
        IS=1
        CALL FILHEP(0,1, IDFFIN(4,JNPI)     ,-IS,-IS,0,0,PNU,AM,.TRUE.)
c sigle scalar has no W
      ELSEIF(ND.EQ.1) THEN
        IS=1
        CALL FILHEP(0,1,16*ISGN,-IS,-IS,0,0,PNU,AM,.TRUE.)

      ELSE
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

      DO I=1,ND

        KFPI=LUNPIK(IDFFIN(I,JNPI),-ISGN)

C for charged conjugate case, change charged pions only
C        IF(KFPI.NE.111)KFPI=KFPI*ISGN
        DO J=1,4
          PPI(J)=PNPI(J,I)
        END DO
        CALL TRALO4(KTO,PPI,PPI,AM)
        CALL FILHEP(0,1,KFPI,-I-IS,-I-IS,0,0,PPI,AM,.TRUE.)
      END DO
C
      RETURN
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

      SUBROUTINE DAM3PI_wrap(MNUM,PT,PN,PIM1,PIM2,PIM3,AMPLIT,HV)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4)
      RETURN
      END

      SUBROUTINE CURR4_wrap(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4),PIM4(4)
      COMPLEX HADCUR(4)
      RETURN
      END

      SUBROUTINE DAM4PI_wrap(MNUM,PT,PN,PIM1,PIM2,PIM3,PIM4,AMPLIT,HV)
      REAL  HV(4),PT(4),PN(4),PIM1(4),PIM2(4),PIM3(4),PIM4(4)
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
