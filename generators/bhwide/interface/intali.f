      SUBROUTINE sprogz(s,PROPG,PROPZ)
!---------------------------------------------------------------!
! This routine comes originally from ALIBABA (see below).       !
! Propagators factor in the s-channel.                          !
! Modified by: Wieslaw Placzek           Knoxville, Oct. 1995   !
! Last update: 09.02.1996     by: W.P.                          !
!---------------------------------------------------------------!
!WP   SUBROUTINE GZPROP(QSQR,PROPG,PROPZ,MIXING)
*     -----------------
* The gamma-Z propagators and their mixing, up to one loop corrections,
* but for the imaginary part of the Z propagator, which includes
* second order corrections.
* QSQR is input: the momentum transfer squared through the progagators.
* PROPG, PROPZ and MIXING are complex*16 output.
      IMPLICIT REAL*8(A-H,O-Z)
      REAL*8 IMSIGG,IMSGGZ,IMSIGZ,IMZ2
      COMPLEX*16 Z1,Z2,Z3, PROPG,PROPZ,MIXING
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
!WP
      SAVE  / BOSONS /, / OPTION /
      QSQR = s
      IF(IWEAK .EQ. 1) THEN
        Z1 = DCMPLX( SIGG (QSQR) , IMSIGG(QSQR) )
        Z2 = DCMPLX( SIGZ (QSQR) , IMSIGZ(QSQR) + IMZ2(QSQR) )
!WP     Z3 = DCMPLX( SIGGZ(QSQR) , IMSGGZ(QSQR) )
        PROPG = 1D0/( QSQR + Z1 )
        PROPZ = 1D0/( QSQR - RMZ**2 + Z2 )
!WP     MIXING= - Z3/( QSQR*(QSQR-RMZ**2+Z2) )
      ELSE
        PROPG  = 1D0/QSQR
!WP     IF(QSQR .GT. 0D0) THEN
          PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , RMZ*ZWID )
!WP     ELSE
!WP       PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , 0D0 )
!WP     ENDIF
!WP      MIXING = DCMPLX( 0D0 , 0D0 )
      ENDIF
      END

      SUBROUTINE tprogz(t,gprof,zprof)
*     ********************************
!---------------------------------------------------------------!
! This routine comes originally from ALIBABA (see below).       !
! Propagators factor in  t-channel (no imaginary parts!).       !
! Modified by: Wieslaw Placzek           Knoxville, Oct. 1995   !
! Last update: 09.02.1996     by: W.P.                          !
!---------------------------------------------------------------!
!WP   SUBROUTINE GZPROP(QSQR,PROPG,PROPZ,MIXING)
*     -----------------
* The gamma-Z propagators and their mixing, up to one loop corrections,
* but for the imaginary part of the Z propagator, which includes
* second order corrections.
* QSQR is input: the momentum transfer squared through the progagators.
* PROPG, PROPZ and MIXING are complex*16 output.
      IMPLICIT REAL*8(A-H,O-Z)
!WP   REAL*8 IMSIGG,IMSGGZ,IMSIGZ,IMZ2
!WP   COMPLEX*16 Z1,Z2,Z3, PROPG,PROPZ,MIXING
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
!WP
      SAVE  / BOSONS /, / OPTION /
      COMPLEX*16 HADRQQ
      IF(IWEAK .EQ. 1) THEN
!WP     Z1 = DCMPLX( SIGG (QSQR) , IMSIGG(QSQR) )
!WP     Z2 = DCMPLX( SIGZ (QSQR) , IMSIGZ(QSQR) + IMZ2(QSQR) )
!WP     Z3 = DCMPLX( SIGGZ(QSQR) , IMSGGZ(QSQR) )
!WP     PROPG = 1D0/( QSQR + Z1 )
!WP     PROPZ = 1D0/( QSQR - RMZ**2 + Z2 )
!WP     MIXING= - Z3/( QSQR*(QSQR-RMZ**2+Z2) )
!WP: No imaginary parts in t-channel
        hadcor = HADRQQ(t)
        gprof = 1/( t + SIGG(t) )/( 1 - hadcor - PHADPI(t) )
        zprof = 1/( t - RMZ**2 + SIGZ(t) )
      ELSE
!WP     PROPG  = 1D0/QSQR
!WP     IF(QSQR .GT. 0D0) THEN
!WP       PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , RMZ*ZWID )
!WP     ELSE
!WP       PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , 0D0 )
!WP     ENDIF
!WP     MIXING = DCMPLX( 0D0 , 0D0 )
!WP: No imaginary parts in t-channel
        gprof = 1/t 
        zprof = 1/(t - RMZ**2)        
      ENDIF
      END

      SUBROUTINE FprogZ(QSQR,PROPG,PROPZ,MIXING)
!---------------------------------------------------------------!
! This routine comes originally from ALIBABA (see below).       !
! Gamma and Z propagators and their mixing.                     !
! New vacuum polarization added!                                !
! Modified by: Wieslaw Placzek           Knoxville, Sept. 1996  !
! Last update: 25.09.1996     by: W.P.                          !
!---------------------------------------------------------------!
!WP   SUBROUTINE GZPROP(QSQR,PROPG,PROPZ,MIXING)
*     -----------------
* The gamma-Z propagators and their mixing, up to one loop corrections,
* but for the imaginary part of the Z propagator, which includes
* second order corrections.
* QSQR is input: the momentum transfer squared through the progagators.
* PROPG, PROPZ and MIXING are complex*16 output.
      IMPLICIT REAL*8(A-H,O-Z)
      REAL*8 IMSIGG,IMSGGZ,IMSIGZ,IMZ2
      COMPLEX*16 Z1,Z2,Z3, PROPG,PROPZ,MIXING
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
!WP
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHPARZ / AMAZ,GAMMZ,SINW2,GV,GA
      SAVE  / BOSONS /, / OPTION /, / BHPAR3 /, / BHPARZ /
!
      KeyPia = MOD(KeyRad,10)
! Vacuum polarization
      CALL vacpol(KeyPia,QSQR,SINW2,RePiq,dRePiq)
      IF(IWEAK .EQ. 1) THEN
!WP        Z1 = DCMPLX( SIGG (QSQR) , IMSIGG(QSQR) )
        Z1 = DCMPLX( QSQR*RePiq , IMSIGG(QSQR) )
        Z2 = DCMPLX( SIGZ (QSQR) , IMSIGZ(QSQR) + IMZ2(QSQR) )
        Z3 = DCMPLX( SIGGZ(QSQR) , IMSGGZ(QSQR) )
        PROPG = 1D0/( QSQR + Z1 )
        PROPZ = 1D0/( QSQR - RMZ**2 + Z2 )
        MIXING= - Z3/( QSQR*(QSQR-RMZ**2+Z2) )
      ELSE
!WP        PROPG  = 1D0/QSQR
        PROPG  = 1D0/QSQR/(1+RePiq)
        IF(QSQR .GT. 0D0) THEN
! Fixed width
          PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , RMZ*ZWID ) /(1+RePiq)
! Running width
cc          PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , QSQR/RMZ*ZWID ) /(1+RePiq)
        ELSE
          PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , 0D0 ) /(1+RePiq)
        ENDIF
        MIXING = DCMPLX( 0D0 , 0D0 )
      ENDIF
      END
 
      FUNCTION xmatvs(s,t,epsCMS)
*     ***************************
!---------------------------------------------------------------!
! This routine comes originally from ALIBABA (see below).       !
! Modified by: Wieslaw Placzek           Knoxville, Oct. 1995   !
! Last update: 25.09.1996     by: W.P.                          !
!---------------------------------------------------------------!
!WP   FUNCTION EEEEVS(COSTH)
*     ---------------
* Calculation of the non-log terms of the virtual and soft corrections
* on the Born Bhabha cross section. Included are the corrections due
* to final state and initial state photons and their interference,
* and hence also box diagrams. COSTH is input and is to be integrated
* over.
* W. Beenakker and S.C. van der Marck, April 1990.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( OFFSET = 1D-10 )
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BHSTAT / ISIGH,ISIGF,ITWOH,IZBR,IRNON,IEVS,IEEW
      REAL*8 MATRIX(1:6), RESEFF(1:4)
      COMPLEX*16 LABDAS,LABDAT,LABDFS,LABDFT,LE,LL,G,A, SPENCE,MZ2
      COMPLEX*16 GS,GT,ZS,ZT,MIX,GZS(-1:1,-1:1),GZT(-1:1,-1:1)
      COMPLEX*16 AGZS,VGZS,AGGS,VGGS,AGZT,VGZT,AGGT,VGGT,GDINT,HULP
      SAVE 
* Statement functions for the box corrections
      G(SL,TL) = SL/2D0/(SL+TL)*LOG(TL/CMPLX(SL,OFFSET)) -
     +           SL*(SL+2D0*TL)/4D0/(SL+TL)**2*(
     +             LOG(TL/CMPLX(SL,OFFSET))**2 + PI*PI )
      A(SL,TL) = (SL-MZ2)/(SL+TL)*( LOG(TL/(SL-MZ2)) +
     +             MZ2/SL*LOG(1D0-SL/MZ2) + (SL+2D0*TL+MZ2)/(SL+TL)*(
     +               LOG(-TL/MZ2)*LOG((MZ2-SL)/(MZ2+TL)) +
     +               SPENCE(SL/MZ2) - SPENCE(-TL/MZ2) ) )
      IEVS=IEVS+1
!
!WP: COSTH from BHWIDE
      COSTH = 1 +2*t/s
!WP: Soft photon cut-off from BHWIDE
      EPS2  = epsCMS      
!WP   S = SCM
      PPQP = .25D0*S*( 1D0 - COSTH )
      PPQM = .25D0*S*( 1D0 + COSTH )
      PPPM =  .5D0*S
!WP   T = - 2D0*PPQP
* Define propagators.
      E2   = EE**2
      QF2  = QF(1)*QF(IFERM)
      QF3  = QF(1)**2
!WP   CALL GZPROP(S ,GS ,ZS ,MIX)
!WP   CALL GZPROP(T ,GT ,ZT ,MIX)
      CALL FprogZ(S ,GS ,ZS ,MIX)
      CALL FprogZ(T ,GT ,ZT ,MIX)
      IF(ICHANN .EQ. 1) THEN
        ZT = (0D0,0D0)
        GT = (0D0,0D0)
      ELSEIF(ICHANN .EQ. 2) THEN
        ZS = (0D0,0D0)
        GS = (0D0,0D0)
      ENDIF
      I=IFERM
      DO 20 L1 = - 1 , 1 , 2
        DO 10 L2 = - 1 , 1 , 2
          GZS(L1,L2)=E2*(QF2*GS+(VF(1)-L1*AF(1))*(VF(I)-L2*AF(I))*ZS)
          GZT(L1,L2)=E2*(QF3*GT+(VF(1)-L1*AF(1))*(VF(1)-L2*AF(1))*ZT)
   10   CONTINUE
   20 CONTINUE
* Start calculating corrections
      SK = S
      U  = - S - T
      MZ2 = CMPLX( RMZ**2 , - RMZ*ZWID )
* The photonic vertex correction: initial state -------------------
      LE =   LOG( -CMPLX(S,OFFSET)/RMASS2(1) )
      LL = - LOG( -CMPLX(S,OFFSET)/SK )
      LABDAS= ALFA/2D0/PI*QF(1)**2*( 2D0*LL*( LE-1D0 ) + LE*(LE-1D0)
     +                           + 2D0*LE + 4D0*( PI**2/12D0 - 1D0 ) )
      LE =   LOG( -CMPLX(T,OFFSET)/RMASS2(1) )
      LL = - LOG( -CMPLX(T,OFFSET)/SK )
      LABDAT= ALFA/2D0/PI*QF(1)**2*( 2D0*LL*( LE-1D0 ) + LE*(LE-1D0)
     +                           + 2D0*LE + 4D0*( PI**2/12D0 - 1D0 ) )
* and final state (more precise: the other fermion current)
      LE =   LOG( -CMPLX(S,OFFSET)/RMASS2(IFERM) )
      LL = - LOG( -CMPLX(S,OFFSET)/SK )
      LABDFS= ALFA/2D0/PI*QF(IFERM)**2*( 2D0*LL*(LE-1D0) + LE*(LE-1D0)
     +                           + 2D0*LE + 4D0*( PI**2/12D0 - 1D0 ) )
      LE =   LOG( -CMPLX(T,OFFSET)/RMASS2(IFERM) )
      LL = - LOG( -CMPLX(T,OFFSET)/SK )
      LABDFT= ALFA/2D0/PI*QF(IFERM)**2*( 2D0*LL*(LE-1D0) + LE*(LE-1D0)
     +                           + 2D0*LE + 4D0*( PI**2/12D0 - 1D0 ) )
* Subtract the leading log terms, to end up with non-log terms only
      BETAL1 = BETALL + 2D0*ALFA/PI
      BETAF1 = BETALF + 2D0*ALFA/PI*QF(IFERM)**2
!WP   LABDAS = LABDAS + LABDFS - .75D0*( BETAL1 + BETALF )
!WP   LABDAT = LABDAT + LABDFT - .75D0*( BETAL1 + BETALF )
      LABDAS = LABDAS + LABDFS
      LABDAT = LABDAT + LABDFT
* The soft photon corrections ----------------------------
      XF    = - RMASS2(IFERM)/S
      XFLOG = LOG(-XF)
      XFLI2 = PI*PI/6D0
      BE    = LOG(S/RMASS2(1)) - 1D0
      BFIN  = BE
      BINT  = 0D0
      IF(ABS(ABS(COSTH)-1D0).GT.0D0) BINT=2D0*LOG(T/U)
      IF (IFERM .GT. 1) BFIN = LOG(S/RMASS2(IFERM)) - 1D0
      IF (IFERM .EQ. 0) BFIN = 0D0
      GIR = - ALFA/PI*LOG(SK/S)*( QF(1)**2*BE + QF(IFERM)**2*BFIN
     +                          + QF(1)*QF(IFERM)*BINT )
      GD = ALFA/PI*2D0*LOG(EPS2)*( QF(1)**2*BE + QF(IFERM)**2*BFIN
     +         + QF(1)*QF(IFERM)*BINT )
      GFIN =-ALFA/PI*( QF(1)**2*( PI**2/3D0 - .5D0 + .5D0*BE**2 )
     +        + QF(IFERM)**2*( XFLOG + 2D0*XFLI2 + .5D0*XFLOG**2 ) )
      IF(ABS(ABS(COSTH)-1D0).GT.0D0)
     +  GFIN = GFIN - ALFA/PI*( + 2D0*QF(1)*QF(IFERM)*(
     +              + DILOG(1D0+S/T) - DILOG(1D0+S/U) ) )
      DELSOF = GIR + GD + GFIN
* Subtract the leading log terms, to end ...
!WP   DELSOF = DELSOF - LOG(EPS2)*( BETALL + BETALF )
      DELSOF = DELSOF
* Resonance effects when soft photons are not all that soft ...
      HULP = (S-MZ2)/(S*(1D0-EPS)-MZ2)
      GDINT = ALFA/PI*2D0*( QF(1)**2*BE*LOG(HULP) +
     +        .5D0*QF(1)*QF(IFERM)*BINT*LOG(HULP) )
      GDRES = ALFA/PI*2D0*( QF(1)**2*BE*LOG(ABS(HULP)) +
     +             QF(1)*QF(IFERM)*BINT*LOG(ABS(HULP)) +
     +             QF(1)**2*BE*(S-RMZ**2)/RMZ/ZWID*(
     +          ATAN((RMZ**2-S*(1D0-EPS))/(RMZ*ZWID)) -
     +          ATAN((RMZ**2-S          )/(RMZ*ZWID)) )  )
      K = 0
      E4 = E2 * E2
      DO 40 L1 = - 1 , 1 , 2
        DO 30 LF = - 1 , 1 , 2
          K = K + 1
          RESEFF(K) = E4*
     +              DBLE((VF(1)-L1*AF(1))*(VF(IFERM)-LF*AF(IFERM))*ZS*
     +      CONJG((VF(1)-L1*AF(1))*(VF(IFERM)-LF*AF(IFERM))*ZS))*GDRES
     +           + 2D0*DBLE( QF(1)*QF(IFERM)*GS*CONJG(
     +            (VF(1)-L1*AF(1))*(VF(IFERM)-LF*AF(IFERM))*ZS))*GDINT
   30   CONTINUE
   40 CONTINUE
      RESEFF(1) = RESEFF(1) + E4*
     +           2D0*DBLE( (QF(1)*QF(1)*GT+(VF(1)+AF(1))**2*ZT)*
     +           CONJG( (VF(1)+AF(1))*(VF(1)+AF(1))*ZS ) )*GDINT
      RESEFF(4) = RESEFF(4) + E4*
     +           2D0*DBLE( (QF(1)*QF(1)*GT+(VF(1)-AF(1))**2*ZT)*
     +           CONJG( (VF(1)-AF(1))*(VF(1)-AF(1))*ZS ) )*GDINT
      ALPI = ALFA/PI * EE**2
      IF ( ICHANN .EQ. 1 ) THEN
        IF( IFINAL+IORDER .EQ. 0 ) THEN
          LABDAS = (0D0,0D0)
          DELSOF = 0D0
          ALPI   = 0D0
        ELSEIF ( IFINAL.EQ.0 ) THEN
          LABDAT = (0D0,0D0)
          LABDAS = LABDAS - ( LABDFS - .75D0*BETAF1 )
          DELSOF =-ALFA/PI*QF(1)**2*( PI**2/3D0 -.5D0 +.5D0*BE**2 )
          ALPI   = 0D0
        ELSEIF ( IORDER.EQ.0 .AND. ICHANN.EQ.1 ) THEN
          LABDAT = (0D0,0D0)
          LABDAS = LABDFS - .75D0*BETAF1
          DELSOF = - ALFA/PI*QF(IFERM)**2*
     +                  (XFLOG+2D0*XFLI2+.5D0*XFLOG**2)
          ALPI   = 0D0
        ENDIF
      ENDIF
*
* And finally the box corrections ------------------------
      AGGS = ALPI*( G(S,T) + G(S,U) )
      VGGS = ALPI*( G(S,T) - G(S,U) + 2D0*LOG(SK/CMPLX(-S,-OFFSET))*
     +                                    LOG(T/U) )
      AGZS = ALPI*( A(S,T) + A(S,U) )
      VGZS = ALPI*( A(S,T) - A(S,U) + 2D0*SPENCE(1D0+MZ2/T) -
     +                                2D0*SPENCE(1D0+MZ2/U) +
     +                        4D0*LOG(SQRT(MZ2*SK)/(MZ2-S))*LOG(T/U) )
      AGGT = ALPI*( G(T,S) + G(T,U) )
      VGGT = ALPI*( G(T,S) - G(T,U) + 2D0*LOG(SK/CMPLX(-T,-OFFSET))*
     +                                    LOG(CMPLX(S,OFFSET)/U) )
      AGZT = ALPI*( A(T,S) + A(T,U) )
      VGZT = ALPI*( A(T,S) - A(T,U) + 2D0*SPENCE(1D0+MZ2/S) -
     +                                2D0*SPENCE(1D0+MZ2/U) +
     +          4D0*LOG(SQRT(MZ2*SK)/(MZ2-T))*LOG(CMPLX(S,OFFSET)/U) )
*
* Combine the corrections with right helicity combinations with the
* different matrix element structures.
* The six helicity combinations are (p+ p- q+ q-):
* + + + +, - - - -, + + - -, - - + +, + - + -, - + - +
      MATRIX(1) = 16D0*PPQM**2*( (GZS( 1, 1)+GZT( 1, 1))*
     1                    DCONJG( GZS( 1, 1)*(LABDAS+DELSOF) +
     2                            GZT( 1, 1)*(LABDAT+DELSOF) +
     3             QF(1)**2*QF(IFERM)**2*GS*( VGGS + AGGS ) +
     4                 QF(1)**2*QF(1)**2*GT*( VGGT + AGGT ) +
     5     QF(1)*QF(IFERM)*(VF(1)-AF(1))*(VF(IFERM)-AF(IFERM))*
     6                                   ZS*( VGZS + AGZS ) +
     7     QF(1)*QF(1)*(VF(1)-AF(1))*(VF(1)-AF(1))*
     8                                   ZT*( VGZT + AGZT ) )
     +     + RESEFF(4) )
      MATRIX(2) = 16D0*PPQM**2*( (GZS(-1,-1)+GZT(-1,-1))*
     1                    DCONJG( GZS(-1,-1)*(LABDAS+DELSOF) +
     2                            GZT(-1,-1)*(LABDAT+DELSOF) +
     3             QF(1)**2*QF(IFERM)**2*GS*( VGGS + AGGS ) +
     4                 QF(1)**2*QF(1)**2*GT*( VGGT + AGGT ) +
     5     QF(1)*QF(IFERM)*(VF(1)+AF(1))*(VF(IFERM)+AF(IFERM))*
     6                                   ZS*( VGZS + AGZS ) +
     7     QF(1)*QF(1)*(VF(1)+AF(1))*(VF(1)+AF(1))*
     8                                   ZT*( VGZT + AGZT ) )
     +     + RESEFF(1) )
      MATRIX(3) = 16D0*PPQP**2*( GZS( 1,-1)*
     1                   DCONJG( GZS( 1,-1)*(LABDAS+DELSOF) +
     2              QF(1)**2*QF(IFERM)**2*GS*( VGGS - AGGS ) +
     3     QF(1)*QF(IFERM)*(VF(1)-AF(1))*(VF(IFERM)+AF(IFERM))*
     4                                    ZS*( VGZS - AGZS ) )
     +     + RESEFF(3) )
      MATRIX(4) = 16D0*PPQP**2*( GZS(-1, 1)*
     1                   DCONJG( GZS(-1, 1)*(LABDAS+DELSOF) +
     2              QF(1)**2*QF(IFERM)**2*GS*( VGGS - AGGS ) +
     3     QF(1)*QF(IFERM)*(VF(1)+AF(1))*(VF(IFERM)-AF(IFERM))*
     4                                    ZS*( VGZS - AGZS ) )
     +     + RESEFF(2) )
      MATRIX(5) = 16D0*PPPM**2* GZT( 1,-1)*
     1                  DCONJG( GZT( 1,-1)*(LABDAT+DELSOF) +
     2            QF(1)**2*QF(IFERM)**2*GT*( VGGT - AGGT ) +
     3     QF(1)*QF(1)*(VF(1)-AF(1))*(VF(1)+AF(1))*
     4                                  ZT*( VGZT - AGZT ) )
      MATRIX(6) = 16D0*PPPM**2* GZT(-1, 1)*
     1                  DCONJG( GZT(-1, 1)*(LABDAT+DELSOF) +
     2             QF(1)**2*QF(IFERM)**2*GT*( VGGT - AGGT ) +
     3     QF(1)*QF(1)*(VF(1)+AF(1))*(VF(1)-AF(1))*
     4                                   ZT*( VGZT - AGZT ) )
*
      SUM = MATRIX(1) + MATRIX(2) + MATRIX(3) +
     +      MATRIX(4) + MATRIX(5) + MATRIX(6)
* conversion to picobarn, 2pi from azimuthal angle, 1/8/(2pi)**2 from
* phase space, 1/(2s) from flux factor, 1/4 from spin averaging.
!WP   EEEEVS = HBARC2/8D0/2D0/PI/2D0/S/4D0 * SUM
      xmatvs = SUM/4
      END

      FUNCTION xmatwc(s,t)
*     ********************
!---------------------------------------------------------------!
! This routine comes originally from ALIBABA (see below).       !
! Modified by: Wieslaw Placzek           Knoxville, Oct. 1995   !
! Last update: 25.09.1996     by: W.P.                          !
!---------------------------------------------------------------!
!WP   FUNCTION EEEEW(COSTH)
*     ---------------
* The Born e+e- --> e+e- matrix element squared, including both gamma
* and Z in both s and t channel, and including WEAK corrections.
* Summing/averaging over spins is performed.
* COSTH is input and is to integrated over.
* W. Beenakker and S.C. van der Marck, April 1990.
* Heavy boxes (ZZ and WW) added: July 1990.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ FORMFA /FZV(0:NRMASS),FZA(0:NRMASS),
     +                FGV(0:NRMASS),FGA(0:NRMASS)
      COMPLEX*16 FZV,FZA,FGV,FGA
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BHSTAT / ISIGH,ISIGF,ITWOH,IZBR,IRNON,IEVS,IEEW
      REAL*8 MATRIX(1:6),ZCOP(-1:1,-1:1)
      COMPLEX*16 GS,GT,ZS,ZT,MIXS,MIXT,GZS(-1:1,-1:1), GZT(-1:1,-1:1)
      COMPLEX*16 HADRQQ,VZZS,AZZS,VWWS,AWWS,VZZT,AZZT,VWWT,AWWT
      SAVE 
      IEEW=IEEW+1
!
!WP: COSTH from BHWIDE
      COSTH = 1 +2*t/s
!WP   S = SCM
      PPQP = .25D0*S*( 1D0 - COSTH )
      PPQM = .25D0*S*( 1D0 + COSTH )
      PPPM =  .5D0*S
!WP   T    = - 2D0*PPQP
* Define propagators, and include vertex form factors.
      E2   = 4D0*PI*ALFA
!WP   CALL GZPROP(S,GS,ZS,MIXS)
      CALL FprogZ(S,GS,ZS,MIXS)
      CALL FORMFS(S,IFERM)
      IF(ICHANN .EQ. 2) THEN
        GS = (0D0,0D0)
        ZS = (0D0,0D0)
        MIXS=(0D0,0D0)
      ENDIF
      I=IFERM
      DO 20 L1 = - 1 , 1 , 2
        DO 10 L2 = - 1 , 1 , 2
          GZS(L1,L2)=E2*( ( -QF(1)-FGV(1)-L1*(      -FGA(1) ) )*
     +                    ( -QF(I)-FGV(I)-L2*(      -FGA(I) ) )*GS +
     +                    (  VF(1)+FZV(1)-L1*( AF(1)+FZA(1) ) )*
     +                    (  VF(I)+FZV(I)-L2*( AF(I)+FZA(I) ) )*ZS -
     +      ( QF(1)*(VF(I)-L2*AF(I)) + QF(I)*(VF(1)-L1*AF(1)) )*MIXS )
          ZCOP(L1,L2) = ((VF(1)-L1*AF(1))*(VF(IFERM)-L2*AF(IFERM)))**2
   10   CONTINUE
   20 CONTINUE
*     Heavy boxes !
      IF(ICHANN .EQ. 2) THEN
        VZZS = (0D0,0D0)
        AZZS = (0D0,0D0)
        VWWS = (0D0,0D0)
        AWWS = (0D0,0D0)
      ELSE
        CALL HEAVYB(S,T,VZZS,AZZS,VWWS,AWWS)
      ENDIF
*     Now everything for the t channel
!WP   CALL GZPROP(T,GT ,ZT ,MIXT)
      CALL FprogZ(T,GT ,ZT ,MIXT)
      CALL FORMFS(T,1)
*     Incorporate the Burkhardt fit for the light quark loops.
!WP   GT = GT/( 1D0 - HADRQQ(T) - PHADPI(T) )
      IF(ICHANN .EQ. 1) THEN
        GT = (0D0,0D0)
        ZT = (0D0,0D0)
        MIXT=(0D0,0D0)
      ENDIF
      DO 40 L1 = - 1 , 1 , 2
        DO 30 L2 = - 1 , 1 , 2
          GZT(L1,L2)=E2*(
     +         ( -QF(1)-FGV(1)-L1*(      -FGA(1) ) )*
     +         ( -QF(1)-FGV(1)-L2*(      -FGA(1) ) )*GT +
     +         (  VF(1)+FZV(1)-L1*( AF(1)+FZA(1) ) )*
     +         (  VF(1)+FZV(1)-L2*( AF(1)+FZA(1) ) )*ZT -
     +       (QF(1)*(VF(1)-L2*AF(1))+QF(1)*(VF(1)-L1*AF(1)))*MIXT )
   30   CONTINUE
   40 CONTINUE
*     Heavy boxes !
      IF(ICHANN .EQ. 1) THEN
        VZZT = (0D0,0D0)
        AZZT = (0D0,0D0)
        VWWT = (0D0,0D0)
        AWWT = (0D0,0D0)
      ELSE
        CALL HEAVYB(T,S,VZZT,AZZT,VWWT,AWWT)
      ENDIF
!!!
c        VZZS = (0D0,0D0)
c        AZZS = (0D0,0D0)
c        VWWS = (0D0,0D0)
c        AWWS = (0D0,0D0)
c        VZZT = (0D0,0D0)
c        AZZT = (0D0,0D0)
c        VWWT = (0D0,0D0)
c        AWWT = (0D0,0D0)
      
* There are 6 different helicity combinations (see EEEEVS).
      IF ( ICHANN .NE. 3 ) THEN
        MATRIX(1) = 16D0*PPQM**2*(GZS( 1, 1)+GZT( 1, 1))*
     +                      CONJG(GZS( 1, 1)+GZT( 1, 1))
        MATRIX(2) = 16D0*PPQM**2*(GZS(-1,-1)+GZT(-1,-1))*
     +                      CONJG(GZS(-1,-1)+GZT(-1,-1))
        MATRIX(3) = 16D0*PPQP**2* GZS( 1,-1)*CONJG(GZS( 1,-1))
        MATRIX(4) = 16D0*PPQP**2* GZS(-1, 1)*CONJG(GZS(-1, 1))
        MATRIX(5) = 16D0*PPPM**2* GZT( 1,-1)*CONJG(GZT( 1,-1))
        MATRIX(6) = 16D0*PPPM**2* GZT(-1, 1)*CONJG(GZT(-1, 1))
*       Heavy boxes (factor 2 from 2*M0*M1)
        MATRIX(1) = MATRIX(1)+32D0*PPQM**2*(GZS( 1, 1)+GZT( 1, 1))*
     +                        CONJG((VZZS+AZZS+VZZT+AZZT)*ZCOP( 1, 1))
        MATRIX(2) = MATRIX(2)+32D0*PPQM**2*(GZS(-1,-1)+GZT(-1,-1))*
     +    CONJG((VZZS+AZZS+VZZT+AZZT)*ZCOP(-1,-1)+VWWS+AWWS+VWWT+AWWT)
        MATRIX(3) = MATRIX(3) +     32D0*PPQP**2*GZS( 1,-1)*
     +                         CONJG(VZZS-AZZS)*ZCOP( 1,-1)
        MATRIX(4) = MATRIX(4) +     32D0*PPQP**2*GZS(-1, 1)*
     +                         CONJG(VZZS-AZZS)*ZCOP(-1, 1)
        MATRIX(5) = MATRIX(5) +     32D0*PPPM**2*GZT( 1,-1)*
     +                         CONJG(VZZT-AZZT)*ZCOP( 1,-1)
        MATRIX(6) = MATRIX(6) +     32D0*PPPM**2*GZT(-1, 1)*
     +                         CONJG(VZZT-AZZT)*ZCOP(-1, 1)
      ELSE
        MATRIX(1) = 16D0*PPQM**2*2D0*GZS( 1, 1)*CONJG(GZT( 1, 1))
        MATRIX(2) = 16D0*PPQM**2*2D0*GZS(-1,-1)*CONJG(GZT(-1,-1))
*       Heavy boxes
        MATRIX(1) = MATRIX(1)+32D0*PPQM**2*GZS( 1, 1)*
     +                             CONJG((VZZT+AZZT)*ZCOP( 1, 1))
     +                       +32D0*PPQM**2*GZT( 1, 1)*
     +                             CONJG((VZZS+AZZS)*ZCOP( 1, 1))
        MATRIX(2) = MATRIX(2)+32D0*PPQM**2*GZS(-1,-1)*
     +                  CONJG((VZZT+AZZT)*ZCOP(-1,-1)+VWWT+AWWT)
     +                       +32D0*PPQM**2*GZT(-1,-1)*
     +                  CONJG((VZZS+AZZS)*ZCOP(-1,-1)+VWWS+AWWS)
        MATRIX(3) = 0D0
        MATRIX(4) = 0D0
        MATRIX(5) = 0D0
        MATRIX(6) = 0D0
      ENDIF
      SUM = MATRIX(1) + MATRIX(2) + MATRIX(3) +
     +      MATRIX(4) + MATRIX(5) + MATRIX(6)
!WP   EEEEW = HBARC2/8D0/2D0/PI/2D0/S/4D0 * SUM
      xmatwc = SUM/4
      END

      SUBROUTINE ZbcOFF
*     *****************
!---------------------------------------------------------------!
! Switch OFF the Z-boson contribution in the ALIBABA part.      !     
! Written by: Wieslaw Placzek                 CERN, Dec. 1998   !
! Last update: 09.12.1998     by: W.P.                          !
!---------------------------------------------------------------!
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      SAVE
      DO I = 0 , NRMASS
        VF(I) = 0d0
        AF(I) = 0d0
      ENDDO
      END

      SUBROUTINE SETCHA(Ichann)
*     *************************
!---------------------------------------------------------------!
! Set up a process channel in ALIBABA:                          !
! Ichann = 0/1/2: s+t/s-only/t-only channel.                    !
! Written by: Wieslaw Placzek                 CERN, Dec. 1998   !
! Last update: 10.12.1998     by: W.P.                          !
!---------------------------------------------------------------!
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      SAVE   / BHPAR3 /
!
      KeyCha = MOD(KeyOpt,1000)/100
      Ichann = KeyCha
      END
