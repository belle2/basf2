      SUBROUTINE babewc(s,t,epsCMS,dsig0,dsig)
*     ****************************************
!---------------------------------------------------------------!
! This routine comes originally from BABAMC (see below).        !
! Modified by: Wieslaw Placzek           Knoxville, Oct. 1995   !
! Last update: 08.02.1996     by: W.P.                          !
!---------------------------------------------------------------!
!WP   SUBROUTINE BABCOR(S,T,DEL, DSIG0,DSIG)
C
C THE 'WORKING HORSE' THAT CALCULATES THE CORRECTED D(SIGMA)/D(OMEGA)
C
C   S,T ARE THE MANDELSTAM VARIABLES
C   DEL IS MAXIMUM PHOTON ENERGY / BEAM ENERGY
C   DSIG0: DIFFERENTIAL BORN CROSS SECTION (IN PBARN)
C   DSIG : DIFFERENTIAL CROSS SECTION WITH RAD. CORRECTIONS (PBARN)
C
C  MZ, MW, MH ARE THE BOSON MASSES, ME,...MT THE FERMION MASSES IN GEV.
C  SW = SIN**2 THETA-W, CW = COS**2 THETA-W, THETA-W = WEINBERG ANGLE.
C  V,A ARE THE LEPTONIC VECTOR AND AXIALVECTOR COUPLING CONSTANTS;
C  VU,AU THOSE OF I=1/2 QUARKS; VD,AD THOSE OF I=-1/2 QUARKS
C  (ALL NORMALIZED TO E=SQRT(4*PI*ALFA)).
C  GZ IS THE WIDTH OF THE Z IN GEV.
C  MF(I,J) IS THE ARRAY OF FERMION MASSES, WHERE I =1,..6 AND J=1,2;
C  LEPTONS(I=1,2,3) AND QUARKS(I=4,5,6);  J=1: UP,  J=2: DOWN MEMBERS.
C  VF IS THE CORRESPONDING ARRAY OF THE FERMION VECTOR COUPLINGS,
C  VF2, AF2 ARE THE CORRESPONDING ARRAYS OF THE VECTOR AND AXIALVECTOR
C  COUPLINGS SQUARED.
C  M IS THE ARRAY OF THE FERMION MASSES SQUARED.
C  ALFA = 1/137.036, ALPHA = ALFA/4*PI, AL = ALFA/PI
C*****************************************************************
      IMPLICIT REAL*8(A-Z)
      COMPLEX*16   CIR1,PIGS,PIGT,PIG,X1C,X2C,SP1,SP2,SPENCE,CHIS,
     1             TGZS,TGZTS,TZZTS,TZBZS,TZBZTS,TWZS,TWZTS,TGVZS,
     2             TGZ5S,TZZZ5S,TGGZS,TGGZTS,TGZZST,TGZZTS,TGGVT,
     3             TGGVS,TGVZST,TGVZTS,TGVZT,TGZVS,TGZVST,TGZVTS,
     4             TGZVT,TZVZS,TZVZST,TZVZTS,TZVZT,TGGVST,TGGVTS,
     5             FZVS,FZAS,FZVT,FZAT,FGVS,FGAS,FGVT,FGAT,
     6             CFZVS,CFZAS,CFGVS,CFGAS,FS1,FS2,FT1,FT2,
     7             ZS,PROP,PIZS,CHITC,TGZST,TGZT
      DIMENSION MF(6,2),VF(6,2),M(6,2),VF2(6,2),AF2(6,2)
      COMMON /BOS/MZ,MW,MH/LEPT/ME,MMU,MTAU
     1       /HAD/MU,MD,MS,MC,MB,MT
     2       /COUP/SW,CW,V,A,VU,AU,VD,AD
     3       /WIDTH/GZ
     4       /ALF/AL,ALPHA,ALFA
     5       /FERMI/MF,VF,M,VF2,AF2
     6       /CONV/CST
!WP: Common block from BHWIDE
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      INTEGER           KEYRAD,KEYOPT,KeyEWC
      SAVE  
C
!WP: Switch for weak corrections
      KeyEWC = MOD(KeyRad,10000)/1000
!WP: Soft photon cut-off
      DEL = epsCMS
C  LEPTONIC COUPLING CONSTANTS
      SW1=DSQRT(SW)
      CW1=DSQRT(CW)
      A2=A*A
      V2=V*V
      W=V2+A2
      U=2.D0*V*A
C  Z PROPAGATOR
      MZ2=MZ**2
      G2=GZ**2
      SM=S-MZ2
      TM=T-MZ2
      DS=SM**2+MZ2*G2
!WP   DT=TM**2+MZ2*G2
      DT=TM**2
      CHISR=S*SM/DS
      CHITR=T*TM/DT
      CHISI=-MZ*GZ/DS*S
      CHITI=0.D0
      CHITC=DCMPLX(CHITR,CHITI)
      CHIT=T/TM
      CHIS=DCMPLX(CHISR,CHISI)
      CHIS2=S*S/DS
      CHIT2=(T/TM)**2
C   COMBINATION OF ANGLES
      C=1.D0+2.D0*T/S
      C2=1.D0+C*C
      C1=1.D0-C
      C0=1.D0+C
      C3=C0**2/C1
      C4=(C0**2+4.D0)/C1**2
      C5=(C0**2-4.D0)/C1**2
C
      PI=3.1415926536D0
C
C  NOW THE TIJ... ARE SPECIFIED        ********************
C
C  1) BORN TERMS:
      TGZS=(V2*C2+2.D0*A2*C)*CHIS
      TGZST=-W*C3*CHITC
      TGZTS=-W*C3*CHIS
      TGZT=(V2*C4+A2*C5)*2.D0*CHITC
C
      TZZS=(W*W*C2+U*U*2.D0*C)*CHIS2
      TZZTS=-(W*W+U*U)*C3*CHIS*CHITC
      TZZT=(W*W*C4+U*U*C5)*2.D0*CHIT2
C
C  2) TERMS APPEARING IN THE PHOTON-Z-MIXING PART
C
      TGGZS=2.D0*V*C2*CHIS
      TGGZST=-2.D0*V*C3*CHIT
      TGGZTS=-2.D0*V*C3*CHIS
      TGGZT=4.D0*V*C4*CHIT
C
      TGZZS=2.D0*(V*W*C2+A*U*2.D0*C)*CHIS2
      TGZZST=-2.D0*(V*W+A*U)*DCONJG(CHIS)*CHIT*C3
      TGZZTS=-2.D0*(V*W+A*U)*CHIS*CHIT*C3
      TGZZT=4.D0*(V*W*C4+A*U*C5)*CHIT2
C
C  3) TERMS WITH WEAK VERTEX CORRECTIONS
C
!WP: Including weak corrections
      IF (KeyEWC.EQ.1) THEN
        CALL FGAM(S,FGVS,FGAS)
        CALL FGAM(T,FGVT,FGAT)
        CALL FZ(S,FZVS,FZAS)
        CALL FZ(T,FZVT,FZAT)
      ELSE
        FGVS = 0
        FGAS = 0
        FGVT = 0
        FGAT = 0
        FZVS = 0
        FZAS = 0
        FZVT = 0
        FZAT = 0
      ENDIF
      CFGVS=DCONJG(FGVS)
      CFGAS=DCONJG(FGAS)
      CFZVS=DCONJG(FZVS)
      CFZAS=DCONJG(FZAS)
C
      TGGVS=2.D0*FGVS*C2
      TGGVST=-2.D0*FGVT*C3
      TGGVTS=-2.D0*FGVS*C3
      TGGVT=4.D0*FGVT*C4
C
      TGVZS=2.D0*(V*(V*CFGVS+A*CFGAS)*C2+A*(V*CFGAS+A*CFGVS)*2.D0*C)
     1      *CHIS
      TGVZST=-2.D0*(W*CFGVS+U*CFGAS)*C3*CHIT
      TGVZTS=-2.D0*(W*FGVT+U*FGAT)*C3*CHIS
      TGVZT=4.D0*(V*(V*FGVT+A*FGAT)*C4+A*(V*FGAT+A*FGVT)*C5)*CHIT
C
      TGZVS=2.D0*(V*FZVS*C2+A*FZAS*2.D0*C)*CHIS
      TGZVST=-2.D0*(V*FZVT+A*FZAT)*C3*CHIT
      TGZVTS=-2.D0*(V*FZVS+A*FZAS)*C3*CHIS
      TGZVT=  (V*FZVT*C4+   A*FZAT*C5)*4.D0*CHIT
C
      FS1=V*CFZVS+A*CFZAS
      FS2=V*CFZAS+A*CFZVS
      FT1=V*FZVT+A*FZAT
      FT2=V*FZAT+A*FZVT
C
      TZVZS=2.D0*(W*FS1*C2+U*FS2*2.D0*C)*CHIS2
      TZVZST=-2.D0*(W*FS1+U*FS2)*C3*CHIT*DCONJG(CHIS)
      TZVZTS=-2.D0*(W*FT1+U*FT2)*C3*CHIT*CHIS
      TZVZT=4.D0*(W*FT1*C4+U*FT2*C5)*CHIT2
C
C
C  4) TERMS WHICH APPEAR WITH BOX DIAGRAMS
C
      TGZB=W*W*C2+U*U*2.D0*C
      TGZBST=-(W**2+U**2)*C3
      TGZBT=2.D0*(W*W*C4+U*U*C5)
C
      S16=16.D0*SW**2
      TGWS=C0*C0/S16
      TGWST=-2.D0*C3/S16
      TGWT=4.D0*(C0/C1)**2/S16
C
      W3=V2+3.D0*A2
      U3=3.D0*V2+A2
      TZBZS=(V2*W3*W3*C2+A2*U3*U3*2.D0*C)*CHIS
      TZBZST=-(V2*W3*W3+A2*U3*U3)*CHIT
      TZBZTS=TZBZST/CHIT*CHIS
      TZBZT=2.D0*(V2*W3*W3*C4+A2*U3*U3*C5)*CHIT
C
      VA2=(V+A)**2/S16
      TWZS=VA2*C0*C0*CHIS
      TWZST=-2.D0*VA2*C3*CHIT
      TWZTS=TWZST/CHIT*CHIS
      TWZT=4.D0*VA2*(C0/C1)**2*CHIT
C
      TGZ5S=(A2*C2+V2*2.D0*C)*CHIS
      TGZ5T =2.D0 *(A2*C4+V2*C5)*CHIT
      TZZ5S=(U*U*C2+W*W*2.D0*C)*CHIS2
      TZZ5T=2.D0*(U*U*C4+W*W*C5)*CHIT2
C
      TGZZ5S=U*U*C2+W*W*C*2.
      TGZZ5T=2.D0*(U*U*C4+W*W*C5)
      TZZZ5S=(A2*U3*U3*C2+V2*W3*W3*2.D0*C)*CHIS
      TZZZ5T=2.D0*(A2*U3*U3*C4+V2*W3*W3*C5)*CHIT
C
C END OF DEFINITION OF THE TIJ... TERMS      **************
C
C  NOW THE INFRARED CORRECTIONS ARE CALLED:
C  CIR: NON RESONANT
C  CIR1: INTERFERENCE RESONANT - NON RESONANT
C  CIR2: RESONANT
      CALL INFRA(DEL,S,T,CIR,CIR1,CIR2)
C  DEL: MAX. PHOTONENERGY/BEAM ENERGY
C  CIR1 COMPLEX, OTHERS REAL
C
C  SPECIFICATION OF THE FINITE QED CORRECTIONS:
      ME2=ME*ME
      BE=   DLOG(S/ME2)-1.D0
      X1=C1/2.D0
      X2=C0/2.D0
      DX1=DLOG(X1)
      DX2=DLOG(X2)
      X1C=DCMPLX(X1,0.D0)
      X2C=DCMPLX(X2,0.D0)
      SP1=SPENCE(X1C)
      SP2=SPENCE(X2C)
      X=DX1**2-DX2**2-2.D0*DBLE(SP1)+2.D0*DBLE(SP2)
      Z= 3.D0*BE-1.D0+2.D0*PI**2/3.D0
      Y=1.5D0*DX1-.5D0*DX1**2-PI**2/2.D0
C  TWO PHOTON BOXES
      CALL GBOX(S,T,V1S,V2S,A1S,A2S,V1T,V2T,A1T,A2T)
C  PHOTON-Z BOXES
      CALL GZBOX(S,T,V1ZS,V2ZS,A1ZS,A2ZS,V1ZT,V2ZT,A1ZT,A2ZT)
C  PHOTON VACUUM POLARIZATION
!WP   PIGS=PIG(S)
!WP   PIGT=PIG(T)
!WP   RPIGS=DBLE(PIGS)
!WP   IPIGS=DIMAG(PIGS)
!WP   RPIGT=DBLE(PIGT)
!WP: Vacuum polarization included in BHWIDE routines 
      IF (KeyEWC.EQ.1) THEN
        PIGS  = PIG(S)
      ELSE
        PIGS = 0
      ENDIF
      PIGT  = 0
      RPIGS = 0
      IPIGS = DIMAG(PIGS)
      RPIGT = 0
C  SPECIFICATION OF THE WEAK CORRECTIONS:
C  Z BOSON SELF ENERGY
!WP: Obsolete!
!WP   RZS=RESZ(S)
!WP   IZS=IMSZ(S)
!WP   RZT=RESZ(T)
      RZS = 0
      IZS = 0
      RZT = 0
      ZS=DCMPLX(RZS,IZS)
      GM= MZ*GZ
      PROP= DCMPLX(SM,GM)
!WP   PIZS= PROP/(SM+ZS)-1.D0
!WP   PIZT= TM/(TM+RZT)-1.D0
      PIZS = 0
      PIZT = 0
      RPIZT=PIZT
      RPIZS=DBLE(PIZS)
      IPIZS=DIMAG(PIZS)
!WP: Including weak corrections
      IF (KeyEWC.EQ.1) THEN
C  PHOTON-Z MIXING ENERGY
        RPIGZS=-RESGZ(S)/S
        IPIGZS=-IMSGZ(S)/S
        RPIGZT=-RESGZ(T)/T
C  HEAVY BOX DIAGRAMS
        CALL BOX(S,T,V1ZZS,V2ZZS,A1ZZS,A2ZZS,
     1             V1ZZT,V2ZZT,A1ZZT,A2ZZT,
     2             V1WS,V2WS,V1WT,V2WT)
      ELSE
        RPIGZS = 0
        IPIGZS = 0
        RPIGZT = 0
        V1ZZS = 0
        V2ZZS = 0
        A1ZZS = 0
        A2ZZS = 0
        V1ZZT = 0
        V2ZZT = 0
        A1ZZT = 0
        A2ZZT = 0
        V1WS  = 0
        V2WS  = 0
        V1WT  = 0
        V2WT  = 0
      ENDIF
C  COMPOSITION OF THE "REDUCED CROSS SECTIONS"     ***********
C  PHOTON-PHOTON
      DEL1=CIR+2.D0*RPIGS+AL*(X+Z+V1S+A1S*2.D0*C/C2)
      DEL2=CIR+RPIGS+RPIGT+AL*(X+Y+Z+.5D0*(V1S+V1T+A1S+A1T))
      DEL3=CIR+2.D0*RPIGT+AL*(X+2.D0*Y+Z+V1T+A1T*C5/C4)
      SGGS=C2*(1.D0+DEL1)+2.D0*DBLE(TGGVS)
     1    +AL*(TGZB*V1ZZS+TGZZ5S*A1ZZS+TGWS*V1WS)
      SGGST=-2.D0*C3*(1.D0+DEL2)
     1    +2.D0*DBLE(TGGVTS+TGGVST)
     2     +AL*(TGZBST*(V1ZZS+A1ZZS+V1ZZT+A1ZZT)
     3          +TGWST*(V1WS+V1WT))
      SGGT=2.D0*C4*(1.D0+DEL3)
     1     +2.D0*DBLE(TGGVT)
     2     +AL*(TGZBT*V1ZZT+TGZZ5T*A1ZZT+TGWT*V1WT)
C  PHOTON-Z-INTERFERENCE
      RCIR=DBLE(CIR1)
      ICIR=DIMAG(CIR1)
      DEL11=RCIR+RPIGS+AL*(X+Z+.5D0*(V1S+V1ZS))+RPIZS
      DEL12=ICIR-IPIGS+ALFA*(V2ZS-V2S)+IPIZS
      SGZS=2.D0*DBLE(TGZS)*(1.+DEL11)-DIMAG(TGZS)*DEL12*2.D0
     1    +AL*DBLE(TGZ5S)*(A1S+A1ZS)-2.D0*ALFA*DIMAG(TGZ5S)
     &                              *(A2ZS-A2S)
     2    +2.D0*DBLE(TGGZS)*RPIGZS -2.D0*DIMAG(TGGZS)*IPIGZS
     3    +2.D0*DBLE(TGVZS+TGZVS)
     4    +AL*(DBLE(TZBZS)*V1ZZS+DBLE(TZZZ5S)*A1ZZS
     5         +DBLE(TWZS)*V1WS)
     6    +2.D0*ALFA*(DIMAG(TZBZS)*V2ZZS+DIMAG(TZZZ5S)*A2ZZS
     7              +DIMAG(TWZS)*V2WS)
C
      DEL21=CIR+RPIGS+AL*(X+Y+Z+.5D0*(V1S+A1S+V1ZT+A1ZT))+RPIZT
      DEL22=IPIGS-ALFA*(1.5D0-V2S-A2S+V2ZT+A2ZT)
      SGZST=  DBLE(TGZST)*(1.D0+DEL21)*2.D0
     1      +2.D0*TGGZST*RPIGZT +2.D0*DBLE(TGVZST+TGZVST)
     2      +AL*(TZBZST*(V1ZZS+A1ZZS)+TWZST*V1WS)
      DEL31=RCIR+RPIGT+AL*(X+Y+Z+.5D0*(V1T+A1T+V1ZS+A1ZS))+RPIZS
      DEL32=ICIR-ALFA *(1.5D0 +A2T-V2ZS-A2ZS+V2T)+IPIZS
      SGZTS=2.D0*DBLE(TGZTS)*(1.D0+DEL31)-2.D0*DIMAG(TGZTS)*DEL32
     1     +2.D0*DBLE(TGGZTS)*RPIGZS-2.D0*DIMAG(TGGZTS)*IPIGZS
     2     +2.D0 *DBLE(TGVZTS+TGZVTS)
     3     +AL*(DBLE(TZBZTS)*(V1ZZT+A1ZZT)+DBLE(TWZTS)*V1WT)
     4     +2.D0*ALFA*(DIMAG(TZBZTS)*(V2ZZT+A2ZZT)
     5               +DIMAG(TWZTS)*V2WT)
      DEL41=CIR+RPIGT+AL*(X+2.D0*Y+Z+.5D0*(V1T+V1ZT))+RPIZT
      SGZT=2.D0*DBLE(TGZT)*(1.D0+DEL41)+    TGZ5T *AL*(A1T+A1ZT)
     1     +2.D0*TGGZT*RPIGZT+2.D0*DBLE(TGZVT+TGVZT)
     2     +AL*(TZBZT*V1ZZT+TZZZ5T*A1ZZT+TWZT*V1WT)
C  Z-Z TERMS
      DEL51=CIR2+AL*(X+Z+V1ZS)+2.D0*RPIZS
      SZZS=TZZS*(1.D0+DEL51)+TZZ5S*AL*A1ZS
     1    +2.D0*TGZZS*RPIGZS+2.D0*DBLE(TZVZS)
      DEL61=RCIR+AL*(X+Y+Z+.5D0*(V1ZS+V1ZT+A1ZS+A1ZT))+RPIZS+PIZT
      DEL62=ICIR-ALFA*(1.5D0+V2ZT-V2ZS+A2ZT-A2ZS)+IPIZS
      SZZST=2.D0*DBLE(TZZTS)*(1.+DEL61)-2.*DIMAG(TZZTS)*DEL62
     1     +2.D0*DBLE(TGZZTS)*(RPIGZS+RPIGZT)
     2     +2.D0*DBLE(TZVZTS+TZVZST)-2.D0*DIMAG(TGZZTS) *IPIGZS
      DEL71=CIR+AL*(X+2.D0*Y+Z +V1ZT )+2.D0*PIZT
      SZZT=TZZT*(1.D0+DEL71)+TZZ5T*AL*A1ZT
     1    +2.D0*TGZZT*RPIGZT+2.D0*DBLE(TZVZT)
C  RADIATIVELY CORRECTED CROSS SECTION
      DSIG=SGGS+SGZS+SZZS
     1    +SGGST+SGZST+SGZTS+SZZST
     3    +SGGT+SGZT+SZZT
      DSIG=DSIG*CST
C  CROSS SECTION IN LOWEST ORDER IN NBARN
      DSIG0=C2-2.D0*C3+2.D0*C4
     1     +2.D0*DBLE(TGZS+TGZTS)+(TGZT+TGZST)*2.D0
     2     +TZZS+2.D0*DBLE(TZZTS)+TZZT
      DSIG0=DSIG0*CST
      END
