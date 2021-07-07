

      SUBROUTINE AMPINW(XPAR,NPAR)
c **********************
C Initialization of the external 4fermion matrix el. codes
C For the moment only GRACE will be interfaced
c **********************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION  XPAR ( *),NPAR ( *)   
      DIMENSION  XPARY(100),NPARY(100)   
      COMMON / INOUT  / NINP,NOUT 
      COMMON / BXFMTS / BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G
      CHARACTER*80      BXOPE,BXCLO,BXTXT,BXL1I,BXL1F,BXL2F,BXL1G,BXL2G
! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      Key4f  = MOD(KeyMis,100)/10
      if (key4f.ne.0) then
        WRITE(NOUT,BXOPE) 
        WRITE(NOUT,BXTXT) '      Window H used only by  Grace 2.0  '
        WRITE(NOUT,BXTXT) '          Higgs  boson parameters       '
        WRITE(NOUT,BXL1F)xpar(11),'xpar(11)= higgs mass ','amh','H1'
        WRITE(NOUT,BXL1F)xpar(12),'xpar(12)= higgs width','agh','H2'
        WRITE(NOUT,BXTXT) '                                        '
        WRITE(NOUT,BXCLO)
      endif
      DO I=1,100
        XPARY(I)=XPAR(I)
        NPARY(I)=NPAR(I)
      ENDDO

      CALL AMPINI(XPARY,NPARY)
      END
C
      SUBROUTINE AMPEXT(WTMOD4F,WT4F )
! *****************************************************
! external 4fermion matrix elements calculations
! BUFFOR routine
! OUTPUTS
!   wtmod4f        - principal weight for rejection
!   wt4f(9)        - auxiliary weights wector
! *****************************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)  
      COMMON / MOMSET / QEFF1(4),QEFF2(4),SPHUM(4),SPHOT(100,4),NPHOT
      COMMON / MOMDEC / Q1(4),Q2(4),P1(4),P2(4),P3(4),P4(4)
      COMMON / DECAYS / IFLAV(4), AMDEC(4)
!!!      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF
      COMMON / cms_eff_MOMDEC / 
     $      effbeam1(4),effbeam2(4),effP1(4),effP2(4),effP3(4),effP4(4)
      save   /cms_eff_momdec/   
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      SAVE   / DECAYS /,/ MOMDEC /,/ KeyKey/
      DIMENSION XQ1(4),XQ2(4),XP1(4),XP2(4),XP3(4),XP4(4)      
      DIMENSION WT4F(9),WT(9)
      I1 = 11
      I2 =-11
      IF1=IFLAV(1)
      IF2=IFLAV(2)
      IF3=IFLAV(3)
      IF4=IFLAV(4)

      key_cms_eff = 1

      IF(key_cms_eff.EQ.0) THEN
! Ext. Matr. el. calculated in CMS 
! function elmatr will break down in this case !!!!!
       DO I=1,4
          XQ1(I)=QEFF1(I)
          XQ2(I)=QEFF2(I)
          XP1(I)=P1(I)
          XP2(I)=P2(I)
          XP3(I)=P3(I)
          XP4(I)=P4(I)
        ENDDO
      ELSE
! Ext. Matr. el. calculated in EFFECTIVE CMS 
        DO I=1,4
          XQ1(I)=EFFbeam1(I)
          XQ2(I)=EFFbeam2(I)
          XP1(I)=effP1(I)
          XP2(I)=effP2(I)
          XP3(I)=effP3(I)
          XP4(I)=effP4(I)
        ENDDO
      ENDIF

      CALL AMP4F(XQ1,I1,XQ2,I2,XP1,IF1,XP2,IF2,XP3,IF3,XP4,IF4
     $          , WTMOD,WT )
C
      DO I4F=1,9
        WT4F(I4F) = WT(I4F)
      ENDDO

      WTMOD4F = WTMOD
      END


      FUNCTION ELMATRunsd(imode,p1,p2,p3,p4,iflav1,iflav2,iflav3,iflav4)
!     ***************************************************************
!  plain matrix el. as of koralw BUT with modified normalisation 
!  according to the IMODE parameter
!  imode = 1  : GRACE
! NOTE,
! p-i momenta MUST be in their CMS frame with e- beam into z+ direction

      implicit double precision (a-h,o-z)   

      DIMENSION P1(4),P2(4),P3(4),P4(4)

      elmatr=wwborn(p1,p2,p3,p4,keyac)
ccc      elmatr=wwborn_massive(p1,p2,p3,p4)
ccc      elmatr=bornkd(p1,p2,p3,p4)
      ELMATRunsd=elmatr
      end

      SUBROUTINE MASOW(SIN2W,GPICB,AMAF)
C     **********************************
C buffor routine: transmits to the external library 
C information from KORALW commons. 
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      COMMON / PHYPAR / ALFINV,GPICOB  
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF
      COMMON / DECDAT / AMAFIN(20), BR(20)
      SAVE   / DECDAT /
      DIMENSION AMAF(20)
      SIN2W=SINW2
      GPICB = GPICOB
      DO I=1,20
       AMAF(I)=AMAFIN(I)
      ENDDO
      END

      SUBROUTINE 
     $   KWPAR2(XAMAW,XAMAZ,XGAMMW,XGAMMZ,XSINW2)
************************************************************************
! buffor routine, like MASOW
************************************************************************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
!************ KORALW stuff ******************
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF  
      COMMON / WEKIN2 / AMAW,GAMMW,GMU,ALPHAW
! This common can be everywhere, contains various switches
      COMMON / KeyKey /  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
! TAUOLA, PHOTOS and JETSET overall switches
      COMMON / LIBRA  / JAK1,JAK2,ITDKRC,IFPHOT,IFHADM,IFHADP

      COMMON / DECDAT / AMAFIN(20), BR(20)
      COMMON / INOUT  / NINP,NOUT     
      COMMON / MATPAR / PI,CEULER     
      COMMON / PHYPAR / ALFINV,GPICOB  

!************ end KORALW stuff ******************

      XAMAW = AMAW
      XAMAZ = AMAZ
      XGAMMW = GAMMW
      XGAMMZ = GAMMZ
      XSINW2 = SINW2

      END
