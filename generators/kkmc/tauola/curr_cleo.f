


      subroutine testresu
C this routine calculates Gamma_X/Gamma_e for tau decay into five
C masless pions via narrow a_2 and later omega +rho resonances
C (also narrow). Karlsruhe 16 Feb 2005. Test worked down to 1 % level
C to go beyond, rho presampler must be implemented in phase space.
C otherwise one can not get rid off the tails of distribution tails
C which remain with sizable effect, 
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      COMMON / DECPAR / GFERMI,GV,GA,CCABIB,SCABIB,GAMEL                
      REAL*4            GFERMI,GV,GA,CCABIB,SCABIB,GAMEL                
 
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.0*Y*Z))
      DATA PI /3.141592653589793238462643/
      AMOM=.782                                                         
      GAMOM= 0.0085         
      ama2=1.260      ! for the test to work ama2> AMOM+ AMRO
      gama2=0.001 !0.400                                             

      
      CARB=3000
      Gropp=6
      FOMEGA=0.07
      wyni= 1.0D0/4.0D0/AMTAU**2*(1-ama2**2/AMTAU**2)**2
      wyni=wyni*(1+2*ama2**2/AMTAU**2)*CARB**2*AMA2**4/AMA2/GAMA2*PI
      dwpynd=XLAM(ama2**2,amro**2,amom**2)/ama2
      eff=dwpynd/ama2*(0.5*dwpynd**2*(1d0/amro**2+1d0/amom**2)+6)
      wyni=wyni*eff
      GAM3PI= 1D0/3.0D0/128D0/(2*PI)**3*AMOM**7*
     $        (fomega*gropp/amro**2)**2/120D0
      GAM2PI= gropp**2/48D0/PI*AMRO
      wyni=wyni* GAM3pi/GAMOM
      wyni=wyni* GAM2pi/GAMRO
      wyni=wyni*ccabib**2
      write(*,*) 'testresu=',wyni
      end

      SUBROUTINE CURR5(MNUM,PIM1,PIM2,PIM3,PIM4,PIM5,HADCUR)
      REAL  PIM1(4),PIM2(4),PIM3(4),PIM4(4),PIM5(4)
      COMPLEX HADCUR(4), HADCU(4)
      IF (MNUM.EQ.2) THEN  ! --+00
       CALL CURR5X(MNUM,PIM1,PIM2,PIM3,PIM4,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCU(K)
       ENDDO
      ELSEIF (MNUM.EQ.4) THEN  ! +--00
       CALL CURR5X(MNUM,PIM2,PIM3,PIM1,PIM4,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM4,PIM5,PIM2,PIM3,PIM1,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM4,PIM5,PIM3,PIM2,PIM1,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
C --
       CALL CURR5X(2,PIM2,PIM3,PIM1,PIM4,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCU(K)  +HADCUR(K)
       ENDDO
       CALL CURR5X(2,PIM2,PIM3,PIM1,PIM5,PIM4,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(2,PIM3,PIM2,PIM1,PIM4,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(2,PIM3,PIM2,PIM1,PIM5,PIM4,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
C --
       DO K=1,4
        HADCUR(K)=HADCUR(K)*SQRT(0.25)  ! statistical factor
       ENDDO
      ELSEIF (MNUM.EQ.5) THEN   ! -0000
       CALL CURR5X(MNUM,PIM2,PIM3,PIM1,PIM4,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM2,PIM4,PIM1,PIM3,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM2,PIM5,PIM1,PIM3,PIM4,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM4,PIM3,PIM1,PIM2,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM5,PIM3,PIM1,PIM4,PIM2,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM5,PIM4,PIM1,PIM3,PIM2,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       DO K=1,4
        HADCUR(K)=HADCUR(K)*SQRT(1.0/24.0)  ! statistical factor
       ENDDO
      ELSEIF (MNUM.EQ.6) THEN  ! -++--
       CALL CURR5X(MNUM,PIM4,PIM5,PIM2,PIM3,PIM1,HADCU)
       DO K=1,4
        HADCUR(K)=HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM1,PIM5,PIM2,PIM3,PIM4,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM1,PIM4,PIM2,PIM3,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM4,PIM5,PIM3,PIM2,PIM1,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM1,PIM5,PIM3,PIM2,PIM4,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       CALL CURR5X(MNUM,PIM1,PIM4,PIM3,PIM2,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCUR(K)+HADCU(K)
       ENDDO
       DO K=1,4
        HADCUR(K)=HADCUR(K)*SQRT(1.0/12.0)  ! statistical factor
       ENDDO

      ELSE
       CALL CURR5X(MNUM,PIM1,PIM2,PIM3,PIM4,PIM5,HADCU)
       DO K=1,4
        HADCUR(K)=HADCU(K)
       ENDDO
      ENDIF
      END
*AJW 1 version of CURR from KORALB.
      SUBROUTINE CURR5X(MNUM,PIM1,PIM2,PIM3,PIM4,PIM5,HADCUR)
C     ==================================================================
C ZBW, 02/2005 - prototype current for 5 pione, several options.
C     ==================================================================
 
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL  PIM1(4),PIM2(4),PIM3(4),PIM4(4),PIM5(4)
      COMPLEX HADCUR(4)

      INTEGER K,L,MNUM,K1,K2,IRO,I,J,KK
      REAL PA(4),PB(4),PAA(4),PC(4),PD(4)
      REAL AA(4,4),PP(4,4)
      REAL A,XM,XG,G1,G2,G,AMRO2,GAMRO2,AMRO3,GAMRO3,AMOM,GAMOM
      REAL FRO,COEF1,FPI,COEF2,QQ,SK,DENOM,SIG,QQA,SS23,SS24,SS34,QP1P2
      REAL QP1P3,QP1P4,P1P2,P1P3,P1P4,SIGN
      REAL PKORB,AMPA
      COMPLEX ALF0,ALF1,ALF2,ALF3
      COMPLEX LAM0,LAM1,LAM2,LAM3
      COMPLEX BET1,BET2,BET3
      COMPLEX FORM1,FORM2,FORM3,FORM4,FORM2PI
      COMPLEX BWIGM,WIGFOR,FPIKM,FPIKMD
      COMPLEX AMPL(7),AMPR
      COMPLEX BWIGN
C
      DATA PI /3.141592653589793238462643/
      BWIGN(A,XM,XG)=XM**2/CMPLX(A-XM**2,XM*XG)
C*******************************************************************************
      sa1=(PIM1(4)+PIM2(4)+PIM3(4)+PIM4(4)+PIM5(4))**2
     $   -(PIM1(3)+PIM2(3)+PIM3(3)+PIM4(3)+PIM5(3))**2
     $   -(PIM1(2)+PIM2(2)+PIM3(2)+PIM4(2)+PIM5(2))**2
     $   -(PIM1(1)+PIM2(1)+PIM3(1)+PIM4(1)+PIM5(1))**2

C                              
      IF (MNUM.EQ.2) THEN ! simple, semi realistic current for 
                           ! pi+pi+pi-pi0pi0, saturated with a2 --> rho omega

      somega=(PIM2(4)+PIM3(4)+PIM4(4))**2-(PIM2(3)+PIM3(3)+PIM4(3))**2
     $      -(PIM2(2)+PIM3(2)+PIM4(2))**2-(PIM2(1)+PIM3(1)+PIM4(1))**2
      sp=    (PIM2(4)+PIM4(4))**2-(PIM2(3)+PIM4(3))**2
     $      -(PIM2(2)+PIM4(2))**2-(PIM2(1)+PIM4(1))**2
      sm=    (PIM3(4)+PIM4(4))**2-(PIM3(3)+PIM4(3))**2
     $      -(PIM3(2)+PIM4(2))**2-(PIM3(1)+PIM4(1))**2
      s0=    (PIM2(4)+PIM3(4))**2-(PIM2(3)+PIM3(3))**2
     $      -(PIM2(2)+PIM3(2))**2-(PIM2(1)+PIM3(1))**2
 
 

      srho=(PIM1(4)+PIM5(4))**2-(PIM1(3)+PIM5(3))**2
     $    -(PIM1(2)+PIM5(2))**2-(PIM1(1)+PIM5(1))**2

      DO  K=1,4
       HADCUR(K)=CMPLX(0.0)
       PAA(K)=PIM1(K)+PIM2(K)+PIM3(K)+PIM4(K)+PIM5(K)
       PA(K) =PIM2(K)+PIM3(K)+PIM4(K)
       PB(K)=PIM1(K)-PIM5(K)
      ENDDO
      CALL LEVICI(PC,PIM2,PIM3,PIM4)
      CALL LEVICI(PD,PB,PC,PAA)
!      write(*,*) sqrt(somega),
      AMOM=.782                                                 
      GAMOM= 0.0085         
      ama2=1.260
      gama2=0.400                                          

      CARB=3000
      Gropp=6
      FOMEGA=0.07
      COEF1=CARB/amro**2/amom**2* Gropp*  (fomega*Gropp/AMRO**2)
      DO K=1,4
       HADCUR(K)=COEF1*PD(K)
       HADCUR(K)=HADCUR(K)*BWIGN(SOMEGA,AMOM,GAMOM)
     $          *BWIGN(SRHO,AMRO,GAMRO)*BWIGN(SA1,AMA2,GAMA2)
!     $ *(BWIGN(SP,AMRO,GAMRO)+BWIGN(SM,AMRO,GAMRO)+BWIGN(S0,AMRO,GAMRO))
!       write(*,*) sqrt(somega)-amom,amom

      ENDDO
      ELSEIF (MNUM.EQ.4.OR.MNUM.EQ.5.OR.MNUM.EQ.6) THEN ! simple, semi realistic current for 
                           ! pi-pi-pi+pi0pi0, saturated with a2 --> f0 a2

      DO  K=1,4
       HADCUR(K)=CMPLX(0.0)
      ENDDO

      sa2=(PIM2(4)+PIM3(4)+PIM1(4))**2-(PIM2(3)+PIM3(3)+PIM1(3))**2
     $      -(PIM2(2)+PIM3(2)+PIM1(2))**2-(PIM2(1)+PIM3(1)+PIM1(1))**2
      s2=    (PIM1(4)+PIM3(4))**2-(PIM1(3)+PIM3(3))**2
     $      -(PIM1(2)+PIM3(2))**2-(PIM1(1)+PIM3(1))**2
      s1=    (PIM2(4)+PIM3(4))**2-(PIM2(3)+PIM3(3))**2
     $      -(PIM2(2)+PIM3(2))**2-(PIM2(1)+PIM3(1))**2
      s2x13=PIM2(4)*(PIM1(4)-PIM3(4))-PIM2(3)*(PIM1(3)-PIM3(3))
     $     -PIM2(2)*(PIM1(2)-PIM3(2))-PIM2(1)*(PIM1(1)-PIM3(1))
      s1x23=PIM1(4)*(PIM2(4)-PIM3(4))-PIM1(3)*(PIM2(3)-PIM3(3))
     $     -PIM1(2)*(PIM2(2)-PIM3(2))-PIM1(1)*(PIM2(1)-PIM3(1))
 
 

      sf=(PIM4(4)+PIM5(4))**2-(PIM4(3)+PIM5(3))**2
     $  -(PIM4(2)+PIM5(2))**2-(PIM4(1)+PIM5(1))**2

      DO  K=1,4
       PAA(K)=PIM1(K)+PIM2(K)+PIM3(K)+PIM4(K)+PIM5(K)
       PA(K) =PIM1(K)+PIM2(K)+PIM3(K)
       PB(K)=PA(K)*s2x13/sa2-(PIM1(K)-PIM3(K))
       PC(K)=PA(K)*s1x23/sa2-(PIM2(K)-PIM3(K))
      ENDDO
      PAPB=PAA(4)*PB(4)-PAA(3)*PB(3)-PAA(2)*PB(2)-PAA(1)*PB(1)
      PAPC=PAA(4)*PC(4)-PAA(3)*PC(3)-PAA(2)*PC(2)-PAA(1)*PC(1)
      DO  K=1,4
       HADCUR(K)=HADCUR(K)+(PAA(K)*PAPB/sa1-PB(K))*BWIGN(s2,amro,gamro)
       HADCUR(K)=HADCUR(K)+(PAA(K)*PAPC/sa1-PC(K))*BWIGN(s1,amro,gamro)
      ENDDO
!      write(*,*) sqrt(somega),

      AMf2=.800                                    
      GAMF2=.600          
      ama2=1.260
      gama2=.400                                            
      
      CARB=4.
      faaf=4.
      fpp=5.
      Grorop=6.
      Gropp=6.
      COEF1=CARB/ama2**4/amf2**2/amro**2*faaf*fpp* Grorop* Gropp
      DO K=1,4
       HADCUR(K)=COEF1*HADCUR(K)
       HADCUR(K)=HADCUR(K)*BWIGN(SF,AMF2,GAMF2)
     $          *BWIGN(SA2,AMA2,GAMA2)*BWIGN(SA1,AMA2,GAMA2)
      ENDDO

      ELSE !MNUM! not realistic current, for tests only !

    !  write(*,*) coef1
      FPI=93.3E-3
      COEF1=2*2.0*SQRT(3.0)/FPI**3
      COEF1= 1D0/AMTAU**3 *(4*3*2*1)* (4*pi)**3* SQRT(20.0D0) ! this
      ! normalization gives Gamma/Gamma_e=ccabib**2 in masless limit
      ! Benchmark current !1
    !  write(*,*) coef1
 !     stop
      DO  K=1,4
       HADCUR(K)=CMPLX(0.0)
       PAA(K)=PIM1(K)+PIM2(K)+PIM3(K)+PIM4(K)+PIM5(K)
       HADCUR(K)=COEF1*PAA(K)
      ENDDO
      ENDIF  !MNUM!
      

      END

      SUBROUTINE LEVICI(P,A,B,C)
      REAL P(4),A(4),B(4),C(4)
     
      P(1)=A(2)*B(3)*C(4)+A(3)*B(4)*C(2)+A(4)*B(2)*C(3)
     $    -A(2)*B(4)*C(3)-A(4)*B(3)*C(2)-A(3)*B(2)*C(4)

      P(2)=A(1)*B(4)*C(3)+A(3)*B(1)*C(4)+A(4)*B(3)*C(1)
     $    -A(1)*B(3)*C(4)-A(4)*B(1)*C(3)-A(3)*B(4)*C(1)

      P(3)=A(1)*B(2)*C(4)+A(4)*B(1)*C(2)+A(2)*B(4)*C(1)
     $    -A(1)*B(4)*C(2)-A(2)*B(1)*C(4)-A(4)*B(2)*C(1)

      P(4)=A(1)*B(3)*C(2)+A(2)*B(1)*C(3)+A(3)*B(2)*C(1)
     $    -A(1)*B(2)*C(3)-A(3)*B(1)*C(2)-A(2)*B(3)*C(1)

      P(1)=-P(1)
      P(2)=-P(2)
      P(3)=-P(3)
      END

C*AJW 1 version of CURR from KORALB.
      SUBROUTINE CURR_CLEO(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
C     ==================================================================
C AJW, 11/97 - based on original CURR from TAUOLA:
C     hadronic current for 4 pi final state
C     R. Fisher, J. Wess and F. Wagner Z. Phys C3 (1980) 313
C     R. Decker Z. Phys C36 (1987) 487.
C     M. Gell-Mann, D. Sharp, W. Wagner Phys. Rev. Lett 8 (1962) 261.
C BUT, rewritten to be more general and less "theoretical",
C  using parameters tuned by Vasia and DSC.
C     ==================================================================
 
      COMMON / PARMAS / AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL*4            AMTAU,AMNUTA,AMEL,AMNUE,AMMU,AMNUMU
     *                 ,AMPIZ,AMPI,AMRO,GAMRO,AMA1,GAMA1
     *                 ,AMK,AMKZ,AMKST,GAMKST
C
      REAL  PIM1(4),PIM2(4),PIM3(4),PIM4(4)
      COMPLEX HADCUR(4)

      INTEGER K,L,MNUM,K1,K2,IRO,I,J,KK
      REAL PA(4),PB(4),PAA(4)
      REAL AA(4,4),PP(4,4)
      REAL A,XM,XG,G1,G2,G,AMRO2,GAMRO2,AMRO3,GAMRO3,AMOM,GAMOM
      REAL FRO,COEF1,FPI,COEF2,QQ,SK,DENOM,SIG,QQA,SS23,SS24,SS34,QP1P2
      REAL QP1P3,QP1P4,P1P2,P1P3,P1P4,SIGN
      REAL PKORB,AMPA
      COMPLEX ALF0,ALF1,ALF2,ALF3
      COMPLEX LAM0,LAM1,LAM2,LAM3
      COMPLEX BET1,BET2,BET3
      COMPLEX FORM1,FORM2,FORM3,FORM4,FORM2PI
      COMPLEX BWIGM,WIGFOR,FPIKM,FPIKMD
      COMPLEX AMPL(7),AMPR
      COMPLEX BWIGN
C
      BWIGN(A,XM,XG)=1.0/CMPLX(A-XM**2,XM*XG)
C*******************************************************************************
C
C --- masses and constants
      IF (G1.NE.12.924) THEN
      G1=12.924
      G2=1475.98
      FPI=93.3E-3
      G =G1*G2
      FRO=0.266*AMRO**2
      COEF1=2.0*SQRT(3.0)/FPI**2
      COEF2=FRO*G ! overall constant for the omega current
      COEF2= COEF2*0.56  ! factor 0.56 reduces contribution of omega from 68% to 40 %

C masses and widths for for rho-prim and rho-bis:
      AMRO2 = 1.465
      GAMRO2= 0.310
      AMRO3=1.700
      GAMRO3=0.235
C
      AMOM  = PKORB(1,14)
      GAMOM = PKORB(2,14)
      AMRO2 = PKORB(1,21)
      GAMRO2= PKORB(2,21)
      AMRO3 = PKORB(1,22)
      GAMRO3= PKORB(2,22)
C
C Amplitudes for (pi-pi-pi0pi+) -> PS, rho0, rho-, rho+, omega.
      AMPL(1) = CMPLX(PKORB(3,31)*COEF1,0.)
      AMPL(2) = CMPLX(PKORB(3,32)*COEF1,0.)*CEXP(CMPLX(0.,PKORB(3,42)))
      AMPL(3) = CMPLX(PKORB(3,33)*COEF1,0.)*CEXP(CMPLX(0.,PKORB(3,43)))
      AMPL(4) = CMPLX(PKORB(3,34)*COEF1,0.)*CEXP(CMPLX(0.,PKORB(3,44)))
      AMPL(5) = CMPLX(PKORB(3,35)*COEF2,0.)*CEXP(CMPLX(0.,PKORB(3,45)))
C Amplitudes for (pi0pi0pi0pi-) -> PS, rho-.
      AMPL(6) = CMPLX(PKORB(3,36)*COEF1)
      AMPL(7) = CMPLX(PKORB(3,37)*COEF1)
C
C rho' contributions to rho' -> pi-omega:
      ALF0 = CMPLX(PKORB(3,51),0.0)
      ALF1 = CMPLX(PKORB(3,52)*AMRO**2,0.0)
      ALF2 = CMPLX(PKORB(3,53)*AMRO2**2,0.0)
      ALF3 = CMPLX(PKORB(3,54)*AMRO3**2,0.0)
C rho' contribtions to rho' -> rhopipi:
      LAM0 = CMPLX(PKORB(3,55),0.0)
      LAM1 = CMPLX(PKORB(3,56)*AMRO**2,0.0)
      LAM2 = CMPLX(PKORB(3,57)*AMRO2**2,0.0)
      LAM3 = CMPLX(PKORB(3,58)*AMRO3**2,0.0)
C rho contributions to rhopipi, rho -> 2pi:
      BET1 = CMPLX(PKORB(3,59)*AMRO**2,0.0)
      BET2 = CMPLX(PKORB(3,60)*AMRO2**2,0.0)
      BET3 = CMPLX(PKORB(3,61)*AMRO3**2,0.0)
C
      END IF
C**************************************************
C
C --- initialization of four vectors
      DO 7 K=1,4
      DO 8 L=1,4
 8    AA(K,L)=0.0
      HADCUR(K)=CMPLX(0.0)
      PAA(K)=PIM1(K)+PIM2(K)+PIM3(K)+PIM4(K)
      PP(1,K)=PIM1(K)
      PP(2,K)=PIM2(K)
      PP(3,K)=PIM3(K)
 7    PP(4,K)=PIM4(K)
C
!      IF (mnum.gt.2) write(*,*) 'curr cleo mnum=',mnum
!      IF (MNUM.gt.2) goto 389
      IF (MNUM.EQ.1) THEN
C ===================================================================
C pi- pi- p0 pi+ case                                            ====
C ===================================================================
       QQ=PAA(4)**2-PAA(3)**2-PAA(2)**2-PAA(1)**2

C  Add M(4pi)-dependence to rhopipi channels:
       FORM4= LAM0+LAM1*BWIGN(QQ,AMRO,GAMRO)
     *            +LAM2*BWIGN(QQ,AMRO2,GAMRO2)
     *            +LAM3*BWIGN(QQ,AMRO3,GAMRO3)

C --- loop over five contributions of the rho-pi-pi
       DO 201 K1=1,3
       DO 201 K2=3,4
C
         IF (K2.EQ.K1) THEN
           GOTO 201
         ELSEIF (K2.EQ.3) THEN
C rho-
            AMPR = AMPL(3)
            AMPA = AMPIZ
         ELSEIF (K1.EQ.3) THEN
C rho+
            AMPR = AMPL(4)
            AMPA = AMPIZ
         ELSE
C rho0
            AMPR = AMPL(2)
            AMPA = AMPI
         END IF
C
         SK=(PP(K1,4)+PP(K2,4))**2-(PP(K1,3)+PP(K2,3))**2
     $     -(PP(K1,2)+PP(K2,2))**2-(PP(K1,1)+PP(K2,1))**2

C -- definition of AA matrix
C -- cronecker delta
        DO 202 I=1,4
         DO 203 J=1,4
 203     AA(I,J)=0.0
 202    AA(I,I)=1.0
C ... and the rest ...
        DO 204 L=1,4
         IF (L.NE.K1.AND.L.NE.K2) THEN
          DENOM=(PAA(4)-PP(L,4))**2-(PAA(3)-PP(L,3))**2
     $         -(PAA(2)-PP(L,2))**2-(PAA(1)-PP(L,1))**2
          DO 205 I=1,4
          DO 205 J=1,4
                      SIG= 1.0
           IF(J.NE.4) SIG=-SIG
           AA(I,J)=AA(I,J)
     $            -SIG*(PAA(I)-2.0*PP(L,I))*(PAA(J)-PP(L,J))/DENOM
 205      CONTINUE
         ENDIF
 204    CONTINUE
C
C --- lets add something to HADCURR
C        FORM1= FPIKM(SQRT(SK),AMPI,AMPI) *FPIKM(SQRT(QQ),AMPI,AMPI)
C        FORM1= AMPL(1)+AMPR*FPIKM(SQRT(SK),AMPI,AMPI)

        FORM2PI= BET1*BWIGM(SK,AMRO,GAMRO,AMPA,AMPI)
     1          +BET2*BWIGM(SK,AMRO2,GAMRO2,AMPA,AMPI)
     2          +BET3*BWIGM(SK,AMRO3,GAMRO3,AMPA,AMPI)
        FORM1= AMPL(1)+AMPR*FORM2PI
C
       DO 206 I=1,4
       DO 206 J=1,4
        HADCUR(I)=HADCUR(I)+FORM1*FORM4*AA(I,J)*(PP(K1,J)-PP(K2,J))
 206   CONTINUE
C --- end of the rho-pi-pi current (5 possibilities)
 201   CONTINUE
C
C ===================================================================
C Now modify the coefficient for the omega-pi current:              =
C ===================================================================
       IF (AMPL(5).EQ.CMPLX(0.,0.)) GOTO 311

C Overall rho+rhoprime for the 4pi system:
C       FORM2=AMPL(5)*(BWIGN(QQ,AMRO,GAMRO)+ELPHA*BWIGN(QQ,AMROP,GAMROP))
C Modified M(4pi)-dependence:
       FORM2=AMPL(5)*(ALF0+ALF1*BWIGN(QQ,AMRO,GAMRO)
     *                    +ALF2*BWIGN(QQ,AMRO2,GAMRO2)
     *                    +ALF3*BWIGN(QQ,AMRO3,GAMRO3))
C
C --- there are two possibilities for omega current
C --- PA PB are corresponding first and second pi-s
       DO 301 KK=1,2
        DO 302 I=1,4
         PA(I)=PP(KK,I)
         PB(I)=PP(3-KK,I)
 302    CONTINUE
C --- lorentz invariants
         QQA=0.0
         SS23=0.0
         SS24=0.0
         SS34=0.0
         QP1P2=0.0
         QP1P3=0.0
         QP1P4=0.0
         P1P2 =0.0
         P1P3 =0.0
         P1P4 =0.0
        DO 303 K=1,4
                     SIGN=-1.0
         IF (K.EQ.4) SIGN= 1.0
         QQA=QQA+SIGN*(PAA(K)-PA(K))**2
         SS23=SS23+SIGN*(PB(K)  +PIM3(K))**2
         SS24=SS24+SIGN*(PB(K)  +PIM4(K))**2
         SS34=SS34+SIGN*(PIM3(K)+PIM4(K))**2
         QP1P2=QP1P2+SIGN*(PAA(K)-PA(K))*PB(K)
         QP1P3=QP1P3+SIGN*(PAA(K)-PA(K))*PIM3(K)
         QP1P4=QP1P4+SIGN*(PAA(K)-PA(K))*PIM4(K)
         P1P2=P1P2+SIGN*PA(K)*PB(K)
         P1P3=P1P3+SIGN*PA(K)*PIM3(K)
         P1P4=P1P4+SIGN*PA(K)*PIM4(K)
 303    CONTINUE
C
C omega -> rho pi for the 3pi system:
C       FORM3=BWIGN(QQA,AMOM,GAMOM)*(BWIGN(SS23,AMRO,GAMRO)+
C     $        BWIGN(SS24,AMRO,GAMRO)+BWIGN(SS34,AMRO,GAMRO))
C No omega -> rho pi; just straight omega:
        FORM3=BWIGN(QQA,AMOM,GAMOM)
C
        DO 304 K=1,4
         HADCUR(K)=HADCUR(K)+FORM2*FORM3*(
     $             PB  (K)*(QP1P3*P1P4-QP1P4*P1P3)
     $            +PIM3(K)*(QP1P4*P1P2-QP1P2*P1P4)
     $            +PIM4(K)*(QP1P2*P1P3-QP1P3*P1P2) )
 304    CONTINUE
 301   CONTINUE
 311   CONTINUE
C
      ELSE
C ===================================================================
C pi0 pi0 p0 pi- case                                            ====
C ===================================================================
! 389     continue  ! temporary solution for `new' 4-pi modes as well.
       QQ=PAA(4)**2-PAA(3)**2-PAA(2)**2-PAA(1)**2

C --- loop over three contribution of the non-omega current
       DO 101 K=1,3
        SK=(PP(K,4)+PIM4(4))**2-(PP(K,3)+PIM4(3))**2
     $    -(PP(K,2)+PIM4(2))**2-(PP(K,1)+PIM4(1))**2

C -- definition of AA matrix
C -- cronecker delta
        DO 102 I=1,4
         DO 103 J=1,4
 103     AA(I,J)=0.0
 102    AA(I,I)=1.0
C
C ... and the rest ...
        DO 104 L=1,3
         IF (L.NE.K) THEN
          DENOM=(PAA(4)-PP(L,4))**2-(PAA(3)-PP(L,3))**2
     $         -(PAA(2)-PP(L,2))**2-(PAA(1)-PP(L,1))**2
          DO 105 I=1,4
          DO 105 J=1,4
                      SIG=1.0
           IF(J.NE.4) SIG=-SIG
           AA(I,J)=AA(I,J)
     $            -SIG*(PAA(I)-2.0*PP(L,I))*(PAA(J)-PP(L,J))/DENOM
 105      CONTINUE
         ENDIF
 104    CONTINUE

C --- lets add something to HADCURR
C       FORM1= FPIKM(SQRT(SK),AMPI,AMPI) *FPIKMD(SQRT(QQ),AMPI,AMPI)
CCCCCCCCCCCCC       FORM1=WIGFOR(SK,AMRO,GAMRO)        (tests)
C       FORM1= FPIKM(SQRT(SK),AMPI,AMPI) *FPIKM(SQRT(QQ),AMPI,AMPI)
       FORM1 = AMPL(6)+AMPL(7)*FPIKM(SQRT(SK),AMPI,AMPI)

        DO 106 I=1,4
        DO 106 J=1,4
         HADCUR(I)=HADCUR(I)+FORM1*AA(I,J)*(PP(K,J)-PP(4,J))
 106    CONTINUE
C --- end of the non omega current (3 possibilities)
 101   CONTINUE

      ENDIF
      END
 


