!---------------------------------------------------------------!
!     This is the original ALIBABA program with a few minor     !
!     modifications by W. Placzek (last update: 10.12.1998).    !
!---------------------------------------------------------------!
      SUBROUTINE BHABHA(S,THETA1,THETA2,THETA3,THETA4,
     +                    BIGEP0,BIGEM0,ACOL,RESULT,ERROR)
*     -----------------
* Calculates the cross section for large angle Bhabha scattering at
* a total CoM energy squared of S, where the positron comes out
* at angles between THETA1 and THETA2 (defined with respect to its
* original direction) whereas the electron is required to come out
* at angles between THETA3 and THETA4 (defined with respect to its
* original direction). The positron is required to have an energy
* larger than BIGEP0 (GeV), the electron to have more that BIGEM0.
* Furthermore the acollinearity is restricted to be less than ACOL.
* S, THETA1,2,3,4, BIGEP0, BIGEM0 and ACOL are input variables.
* The result is returned as the output variable RESULT (picobarn). The
* estimated numerical error is returned as the output variable ERROR.
* (ERROR is only of practical importance if the flag NONLOG=1)
* !!! Before the first call to BHABHA, initialization has to be done
* by calling INITBH !!!
* For flag settings etc., see the comments in INITBH.
*
* THETA2 should be larger than THETA1.
* THETA1 should be larger than 0 for t channel processes.
*        It is recommended that this program is not used below 10 degr.
*        for t channel scattering.
* THETA4 should be larger than THETA3.
* THETA3 should be larger than 0 for t channel processes.
*        The program has the following built-in assumptions:
*        1) that either: theta1 .le. theta3  .and.  theta2 .ge. theta4
*                    or: theta1 .ge. theta3  .and.  theta2 .le. theta4
*           If the input does not satisfy this, it is reset to
*           theta1=theta3 and theta2=theta4.
*        2) that theta1.le.90.le.theta2  .and.  theta3.le.90.le.theta4
*           If the input does not satisfy this, it is reset to
*           theta2=180-theta1 and theta4=180-theta3.
*
* BIGEP0 and BIGEM0 should be less than .5*sqrt(s) (=kinematical upper
*                      bound on the energy of the outcoming particles)
* ACOL  should be between 0 and 180 degrees (180 means: no cut)
*
* This subroutine is not meant to be changed by the user, except
* possibly for the number of points that is required in the
* integrations by VEGAS. This number is NCALL and is set several times
* for the integration of several contributions.
*
* After this subroutine two subroutines have been added that
* calculate the forward-backward asymmetry (according two two
* different definitions), using the subroutine BHABHA. These
* subroutines AFBASY and AFBSYM are meant as examples; one can
* write other subroutines calculating the asymmetry according to
* ones own favourite definition.
*
* W. Beenakker, F.A. Berends and S.C. van der Marck, August 1990.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BOOKKP / IBH1,IBH2,IBH3,IBH4,IBH5,IBH6
      PARAMETER( NX = 10 )
      COMMON/BVEG1/XL(NX),XU(NX),ACC,NDIM,NCALL,ITMX,NPRN
      COMMON/ EVENT  / PP(0:3),PM(0:3),QP(0:3),QM(0:3),SP,QK(0:3)
      EXTERNAL SIGFLX,TWOHRD,RNONLG,EEEEVS,EEEEW
* First set parameters, use input to set integration boundaries.
* Technicalities and such.
      IF( S .LT. 4D0*RMASS2(IFERM) ) THEN
        WRITE(*,'(/,A,/)')' BHABHA: Below production threshold !'
        RESULT = 0D0
        ERROR  = 0D0
      ENDIF
      SCM = S
      C1  =   COS( THETA1 *PI /180D0 )
      C2  =   COS( THETA2 *PI /180D0 )
      C3  = - COS( THETA3 *PI /180D0 )
      C4  = - COS( THETA4 *PI /180D0 )
      TH1 = THETA1
      TH2 = THETA2
      IF( ( THETA3.LE.THETA1 .AND. THETA4.GE.THETA2 ) .OR.
     +    ( THETA3.GE.THETA1 .AND. THETA4.LE.THETA2 ) ) THEN
        IRESET = 0
        TH3 = THETA3
        TH4 = THETA4
      ELSE
        IRESET = 1
        C3  = - C1
        C4  = - C2
        TH3 = THETA1
        TH4 = THETA2
      ENDIF
      IF( TH1 .GT. 90D0 .OR. TH2 .LT. 90D0 ) THEN
        IRESET = 1
        TH2 = 180D0 - TH1
        C2  = - C1
      ENDIF
      IF( TH3 .GT. 90D0 .OR. TH4 .LT. 90D0 ) THEN
        IRESET = 1
        TH4 = 180D0 - TH3
        C4  = - C3
      ENDIF
      EBEAM = .5D0*SQRT(S)
      EP0   = BIGEP0 / EBEAM
      EM0   = BIGEM0 / EBEAM
      EP0   = MAX( EP0 , RMASS(IFERM)/EBEAM )
      EM0   = MAX( EM0 , RMASS(IFERM)/EBEAM )
      IF ( ABS(1D0-C2).GT.1D-10 .AND. ABS(1D0-C3).GT.1D-10 ) THEN
        BOUND1 = SQRT( (1D0+C2)/(1D0-C2)*(1D0+C3)/(1D0-C3) )
      ELSE
        BOUND1 = 1D0
      ENDIF
      IF ( ABS(1D0+C1).GT.1D-10 .AND. ABS(1D0+C4).GT.1D-10 ) THEN
        BOUND2 = SQRT( (1D0-C1)/(1D0+C1)*(1D0-C4)/(1D0+C4) )
      ELSE
        BOUND2 = 1D0
      ENDIF
      X0    = MIN( BOUND1 , BOUND2 )
      X0    = MAX( X0 , EP0+EM0 - 1D0 )
      ACOLMX= ACOL *PI/180D0
      IF ( ACOL.LE.0D0 .OR. ACOL.GT.180D0 ) ACOLMX = PI
      BETALL= 2D0*ALFA/PI*( LOG(SCM/RMASS2(1)) - 1D0 )
      IF(IFERM.NE.1) THEN
        IF ( IFERM .NE. 0 ) THEN
          BETALF = QF(IFERM)**2*2D0*ALFA/PI*(LOG(S/RMASS2(IFERM))-1D0)
        ELSE
          BETALF = 0D0
        ENDIF
        ISAV3  = ICHANN
        ICHANN = 1
      ELSE
        BETALF = BETALL
      ENDIF
      X0 = MAX( X0 , 4D0*RMASS2(IFERM)/SCM )
      X1MAX = 1D0 - X0
*
*     See to it that the user that asks for nonsense doesn't get it.
      IF((THETA1.LT.10D0.OR.THETA3.LT.10D0) .AND. ICHANN.NE.1) THEN
        WRITE(*,'(A,A)') ' BHABHA: This program is not fit for',
     +                   ' low angle t channel scattering !'
        RESULT = 0D0
        ERROR  = 0D0
        RETURN
      ENDIF
      IF(BIGEP0 .GE. EBEAM .OR. BIGEM0 .GE. EBEAM) THEN
        WRITE(*,'(A,A)') ' BHABHA: Nonsensical energy cut entered.',
     +                   ' RESULT = 0 !'
        RESULT = 0D0
        ERROR  = 0D0
        RETURN
      ENDIF
      IF(IFERM .LT. 0 .OR. IFERM .GT. NRMASS) THEN
        WRITE(*,'(A,I3,A)') ' BHABHA: IFERM =',IFERM,
     +                      ', which is not valid. RESULT = 0 !'
        RESULT = 0D0
        ERROR  = 0D0
        RETURN
      ENDIF
      IF ( IFERM .EQ. 0 ) THEN
        WRITE(*,'(/,A,A,/)')' Neutrino production is calculated',
     +    ' without the t-channel (1 species only) !'
      ENDIF
      IF (IFERM.EQ.NRMASS+1.OR.IFERM.EQ.8) THEN
        WRITE(*,'(/,A,A,I3)')' Terribly sorry, but this program is',
     +      ' not adapted for the choice IFERM =',IFERM
        WRITE(*,'(A,/)')' RESULT = 0 !'
        RESULT = 0D0
        ERROR  = 0D0
        RETURN
      ENDIF
      IF ( THETA1 .GE. THETA2 ) THEN
        WRITE(*,'(/,A,/)')' Empty angular region! RESULT = 0 !'
        RESULT = 0D0
        ERROR  = 0D0
        RETURN
      ENDIF
      IF ( ACOLMX .LE. 1D-3 ) THEN
        WRITE(*,'(/,A,/)')' Acollinearity cut too tight! RESULT = 0 !'
        RESULT = 0D0
        ERROR  = 0D0
        RETURN
      ENDIF
      IWRONG = 0
      IF(IWEAK .LT.0 .OR. IWEAK .GT.1) THEN
        IWEAK  = 1
        IWRONG = 1
      ENDIF
      IF(IORDER.LT.0 .OR. IORDER.GT.4) THEN
        IORDER = 4
        IWRONG = 1
      ENDIF
      IF(IFINAL.LT.0 .OR. IFINAL.GT.2) THEN
        IFINAL = 2
        IWRONG = 1
      ENDIF
      IF(NONLOG.LT.0 .OR. NONLOG.GT.1) THEN
        NONLOG = 0
        IWRONG = 1
      ENDIF
      IF(IWRONG.EQ.1) WRITE(IOUT,20)
   20 FORMAT(/,' =======> The flag setting has been changed, for',
     + ' non-valid values occurred.',/,10X,'Please check the new',
     + ' setting.',/)
      IF(IRESET.EQ.1) WRITE(IOUT,25)
   25 FORMAT(/,' =======> The angular range has been altered, in ',
     + 'order to satisfy ',/,10X,'a built-in assumption.',/,10X,
     + 'Please check the new setting and see the comment in ',
     + 'routine BHABHA.',/)
*
*     Write options, if first call since INITBH or if options changed.
      IWRITE=ABS(IBH1-IWEAK )+ABS(IBH2-ICHANN)+ABS(IBH3-IORDER)+
     +       ABS(IBH4-IFINAL)+ABS(IBH5-NONLOG)+ABS(IBH6-IFERM )+IWRONG
      IF (IWRITE.GT.0) THEN
        IBH1 = IWEAK
        IBH2 = ICHANN
        IBH3 = IORDER
        IBH4 = IFINAL
        IBH5 = NONLOG
        IBH6 = IFERM
        WRITE(IOUT,30)IWEAK,IORDER,IFINAL,NONLOG,ICHANN,IFERM
   30   FORMAT(/,' The following options have been chosen:',/,
     +         ' iweak =',I2,',   iorder =',I2,',   ifinal =',I2,
     +         ',   nonlog =',I2,',   ichann =',I2,',   iferm =',I2)
        IF(IFERM.EQ.9) WRITE(IOUT,'(/,1X,A,/,1X,A,/)')
     +  ' Bottom pair production has been chosen. Please be aware',
     +  ' that masses have been neglected.'
      ENDIF
      EPS   = 1D-5
      RLOG  = LOG( EPS )
      IF(IORDER .LT. 3) THEN
        X1MIN = RLOG
        X1MAX = LOG(X1MAX)
      ELSE
        X1MIN = 0D0
        X1MAX = X1MAX**(BETALL)
      ENDIF
*
*     Let's be nice and confirm the input to the user.
      WRITE(IOUT,40) EBEAM,2D0*EBEAM,BETALL,ACOLMX*180D0/PI,
     + EP0*EBEAM,EP0,EM0*EBEAM,EM0,TH1,C1,TH2,C2,TH3,-C3,TH4,-C4,X0
   40 FORMAT(/,'   Beam energy =',F9.4,' GeV,   <-->  sqrt(s) =',
     1 F11.6,' GeV',/,'          beta = ',F10.6,/,' Acollinearity <',
     2 F9.4,' deg.',/,'  Energy f-bar >',F9.4,' GeV  -->   cut/Ebeam ='
     3 ,F11.6,/,'  Energy   f   >',F9.4,' GeV  -->   cut/Ebeam ='
     4 ,F11.6,/,'   Angle f-bar >',F9.4,' deg. --> cos(theta1) = ',
     5  F10.6,/,'   Angle f-bar <',F9.4,' deg. --> cos(theta2) = ',
     6  F10.6,/,'   Angle   f   >',F9.4,' deg. --> cos(theta3) = ',
     7  F10.6,/,'   Angle   f   <',F9.4,' deg. --> cos(theta4) = ',
     8  F10.6,/,9X,'--> a priori cut on the x integration = ',F10.6,/)
*
* Now the actual calculation starts ...
* There are a lot of branches, different contributions, ...
* First Born and the most 'elementary' weak and QED corrections.
      BETAL1 = BETALL + 2D0*ALFA/PI
      BORN   = SIGHAT(0D0,0D0)
      ACC1   = 1D-5
      ISAVFI = IFINAL
*     If no higher than 1st order is asked for, avoid ini*fin corr.
      IF(IORDER.EQ.1.AND.IFINAL.EQ.1)  THEN
        IFINAL = 0
        FINEXT = BORN - SIGHAT(0D0,0D0)
      ENDIF
      IF(IORDER .GT. 0) HARD = DGAUSS(SIGFLX,X1MIN,X1MAX,ACC1)
      IF(IORDER .LT. 3 .AND. IORDER.GT.0) THEN
*       Non-exponentiated; hard has been done; need to do V/S part.
        FLUX = 1D0 + .5D0*BETALL*( 2D0*RLOG ) + .5D0*BETAL1*( 1.5D0 )
        IF(IORDER.EQ.2) FLUX = FLUX + .25D0*BETALL**2*( 2D0*RLOG*RLOG
     +          + 3D0*RLOG ) + .25D0*BETAL1**2*( 9D0/8D0 - PI*PI/3D0 )
        SOFT = FLUX*BORN
      ELSEIF(IORDER .GE. 3) THEN
*       Exponentiated; we did the 'soft distribution' so far, so now
*       we need to do the pure hard part.
        X1MIN  = RLOG
        X1MAX  = LOG( 1D0 - X0 )
        IORDER = IORDER + 2
        SOFT   = HARD
        HARD   = DGAUSS(SIGFLX,X1MIN,X1MAX,ACC1)
        IORDER = IORDER - 2
      ELSE
        SOFT = BORN
        HARD = 0D0
      ENDIF
      IF(IFINAL.NE.ISAVFI) THEN
        IFINAL = ISAVFI
        SOFT = SOFT + FINEXT
      ENDIF
*
      WEAKT = 0D0
      IF(IWEAK.EQ.1) THEN
*       The t dependence of the weak corrections to Born that we
*       didn't calculate exactly so far - do it now.
        IHELP  = IFINAL
        IFINAL = 0
        HELP   = SIGHAT(0D0,0D0)
        COS1   = C1
        COS2   = C2
        IF(-C3.LT.C1) COS1 = - C3
        IF(-C4.GT.C2) COS2 = - C4
        WEAKT  = DGAUSS(EEEEW,COS2,COS1,ACC1) - HELP
        IFINAL = IHELP
      ENDIF
*
      FIN2 = 0D0
      IF(IFINAL.EQ.2.AND.(EP0.GT.1D-3.OR.EM0.GT.1D-3)) THEN
*       Second order LL final state corrections.
        IHELP  = IFINAL
        IFINAL = 0
        HELP   = SIGHAT(0D0,0D0)
        FIN2   = HELP * FINAL2()
        IFINAL = IHELP
      ENDIF
*
      IF ( IORDER .EQ. 2 .OR. IORDER .EQ. 4 ) THEN
*       The part that does not exponentiate, and where both x1 and x2
*       are different from 1. Only non zero at the second and higher
*       order QED. This part is rather small, therefore somewhat less
*       fractional accuracy is needed.
*       VEGAS is used to do the 2-dimensional integration.
        ISAV1 = IWEAK
        ISAV2 = IFINAL
        IWEAK = 0
        IFINAL= 0
        ACC   = 1D-3
        XL(1) = EPS
        X0    = RMASS(IFERM)/EBEAM
        XU(1) = 1D0 - X0
        XL(2) = XL(1)
        XU(2) = XU(1)
        NDIM  = 2
        ITMX  = 5
        NCALL = 5000*4
        NPRN  = 0
        CALL VEGAS(TWOHRD,RES,ERRMC,CHI1)
        IF(ERRMC.GT.ACC*ABS(RES)) THEN
          ITMX  = 15
          NCALL = 10000
          CALL VEGAS1(TWOHRD,RES,ERRMC,CHI1)
        ENDIF
        HARD2 = RES
        IWEAK = ISAV1
        IFINAL= ISAV2
      ELSE
        HARD2 = 0D0
        ERRMC = 0D0
        CHI1  = 0D0
      ENDIF
*
      IF( NONLOG .EQ. 1 ) THEN
*       Now take care of the first order non-log terms by explicitly
*       integrating over the exact matrix element minus the collinear
*       approximation. Integration by VEGAS again, this time 5 dim.
        ISAV1 = IWEAK
        IWEAK = 0
        ACC   = 1D-3
        EPS2  = 1D-3
*       First however calculate the V/S non log contrs. (1 dim integr)
        COS1  = C1
        COS2  = C2
        IF(-C3.LT.C1) COS1 = - C3
        IF(-C4.GT.C2) COS2 = - C4
        RNLLVS= DGAUSS(EEEEVS,COS2,COS1,ACC)
*       Now the hard part (5 dimensional integral):
*       This part is a 5 dim. Monte Carlo integral (but negative
*       weights are possible, so events with unity weight can not be
*       constructed). As this is a Monte Carlo calculation, do expect
*       this to be very slow indeed.
*       Initialize incoming beam momenta:
        PP(0) = EBEAM
        PP(1) = 0D0
        PP(2) = 0D0
        PP(3) = SQRT( PP(0)**2 - RMASS2(1) )
        PM(0) =   PP(0)
        PM(1) = - PP(1)
        PM(2) = - PP(2)
        PM(3) = - PP(3)
*       Upper and lower boundaries, the variables are k0,
*       Omega(k w.r.t. p+) and Omega(q+ w.r.t. k).
        XL(1) = LOG( EPS2 * EBEAM )
        XU(1) = LOG(        EBEAM*( 1D0 - RMASS2(IFERM)/EBEAM**2 ) )
        XL(2) = - 1D0
        XU(2) =   1D0
        XL(3) =   0D0
        XU(3) =   2D0*PI
        XL(4) = - 1D0
        XU(4) =   1D0
        XL(5) =   0D0
        XU(5) =   2D0*PI
*       And now for the integration parameters and the actual integral.
        NPRN  = 0
        NDIM  = 5
        ITMX  = 5
        NCALL = 5000*4
        RES1  = 0D0
        ERR21 = 0D0
        CHI2  = 0D0
        CALL HISTO(1,0,0D0,0D0,0D0,0D0,1,' ',IOUT,1)
        IF ( ICHANN.NE.1 .OR. (IFINAL+IORDER).NE.0 ) THEN
          CALL VEGAS(RNONLG,RES1,ERR21,CHI2)
          IF(ERR21.GT.ACC*ABS(RES1)) THEN
            NPRN  = 0
            ITMX  = 5
            NCALL = 30000*4
            CALL HISTO(1,0,0D0,0D0,0D0,0D0,1,' ',IOUT,1)
            CALL VEGAS1(RNONLG,RES1,ERR21,CHI2)
          ENDIF
        ENDIF
*
*       Printing of histograms to see a few distributions for the
*       non log contributions. Can be turned on/off by the user,
*       but only together with the calls to HISTO in RNONLG.
*        X = FLOAT(NCALL)
*        CALL HISTO( 1,7, X ,0D0,0D0, 0D0,1,' ',6,1)
*        CALL HISTO( 1,2,0D0,0D0,0D0,10D0,1,' k(0)       ',IOUT,10)
*        CALL HISTO( 2,2,0D0,0D0,0D0,10D0,1,' cos q+     ',IOUT,10)
*        CALL HISTO( 3,2,0D0,0D0,0D0,10D0,1,' cos q-     ',IOUT,10)
*        CALL HISTO( 4,2,0D0,0D0,0D0,10D0,1,' cos k      ',IOUT,10)
*        CALL HISTO( 5,2,0D0,0D0,0D0,10D0,1,' cos(q+,k)  ',IOUT,10)
*        CALL HISTO( 6,2,0D0,0D0,0D0,10D0,1,' cos(q-,k)  ',IOUT,10)
*        CALL HISTO( 7,2,0D0,0D0,0D0,10D0,1,' cos(q+,q-) ',IOUT,10)
*        CALL HISTO( 8,2,0D0,0D0,0D0,10D0,1,' q+(0)      ',IOUT,10)
*        CALL HISTO( 9,2,0D0,0D0,0D0,10D0,1,' q-(0)      ',IOUT,10)
*        CALL HISTO(10,2,0D0,0D0,0D0,10D0,1,' event check',IOUT,10)
*
        RNLL  = RES1
        ERR2  = ERR21
        IWEAK = ISAV1
      ELSE
        RNLL = 0D0
        ERR2 = 0D0
        CHI2 = 0D0
        EPS2 = 1D-3
        RNLLVS = 0D0
      ENDIF
*
      IF(IFERM.NE.1) ICHANN = ISAV3
      IF(IFERM.GT.3) THEN
*       For quarks: multiply with the number of colours and with a
*       QCD correction factor due to final state QCD corr.
        COLOUR = 3D0*( 1D0 + FACQCD )
        BORN   = COLOUR * BORN
        SOFT   = COLOUR * SOFT
        HARD   = COLOUR * HARD
        HARD2  = COLOUR * HARD2
        RNLL   = COLOUR * RNLL
        RNLLVS = COLOUR * RNLLVS
        WEAKT  = COLOUR * WEAKT
        FIN2   = COLOUR * FIN2
        ERRMC  = COLOUR * ERRMC
        ERR2   = COLOUR * ERR2
      ENDIF
*
* The result!
      RESULT = SOFT + HARD + HARD2 + RNLL + RNLLVS + WEAKT + FIN2
      ERROR  = ACC1*RESULT + ERRMC + ERR2
*
* Print the results. Can also be turned off if boring.
      RESLL = SOFT + HARD + HARD2 + WEAKT + FIN2
      WRITE(IOUT,100) BORN,SOFT-BORN,HARD,RNLLVS,RNLL,ERR2,CHI2,
     + HARD2,ERRMC,CHI1,WEAKT,FIN2,RESLL,RNLLVS+RNLL,ERR2,RESULT
  100 FORMAT(' The ''Born'' contribution:      ',F15.3,/,
     1 ' The  V+S LL contribution:     ',F15.3,/,
     2 ' The hard LL contribution:     ',F15.3,/,
     3 ' The  V+S non log contribution:',F15.3,/,' The hard non log',
     4 ' contribution:',F15.3,' +- ',F15.3,' (',G11.4,')',/,
     5 ' The x1,x2 part:',15X,F15.3,' +- ',F15.3,' (',G11.4,')',/,
     6 ' The t dependence of weak corr:',F15.3,/,
     7 ' The 2nd order final st. corr.:',F15.3,/,
     8 ' The total cross section in LLA',F15.3,/,
     9 ' The total non log contribution',F15.3,' +- ',F15.3,//,
     1 ' The total cross section =     ',F15.3,' picobarn')
*
      IF(ABS(RNLLVS+RNLL).GT.2D-1*ABS(BORN)) WRITE(IOUT,110)
  110 FORMAT(' The non-log terms are quite large !',/,' Be careful',
     +       ' with the interpretation of this result.',/,
     +       ' Contact the authors ?')
      END
 
      SUBROUTINE AFBASY(S,THETA,BIGEP0,BIGEM0,ACOL,RESULT,ERROR)
*     -----------------
* Example 1 of a subroutine to calculate the forward backward
* asymmetry: the integral from THETA to 90 minus the integral
* from 90 to 180-THETA.
* NOTE: in the integral from THETA to 90  ONLY  the outgoing
*       positron is required to scatter between THETA and 90 off
*       its original direction. Analogously for 90 -> 180-THETA.
* For the meaning of the other parameters in the argument list, see
* the comments in subroutine BHABHA. For the meaning of the
* flags that govern which corrections are taken into account,
* see the comments in subroutine INITBH.
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      CALL BHABHA(S,THETA,   90D0    ,0D0,180D0,BIGEP0,BIGEM0,ACOL,
     +                                                   RESF,ERRF)
      CALL BHABHA(S, 90D0,180D0-THETA,0D0,180D0,BIGEP0,BIGEM0,ACOL,
     +                                                   RESB,ERRB)
      RESULT = 0D0
      ERROR  = 0D0
      TOTAL = RESF + RESB
      IF( ABS(TOTAL) .GT. 0D0 ) THEN
        RESULT = ( RESF - RESB )/TOTAL
        ERROR  = ( ERRF + ERRB )/TOTAL
      ENDIF
*     QCD correction factor for quarks:
      IF(IFERM.GT.3) RESULT = RESULT*( 1D0 - FACQCD )
      WRITE(IOUT,100) RESULT,ERROR
  100 FORMAT(/,' Afb (definition 1) = ',F15.5,' +- ',F15.5,/)
      END
 
      SUBROUTINE AFBSYM(S,THETA,BIGEP0,BIGEM0,ACOL,RESULT,ERROR)
*     -----------------
* Example 2 of a subroutine to calculate the forward backward
* asymmetry: the integral from THETA to 90 minus the integral
* from 90 to 180-THETA.
* NOTE: in the integral from THETA to 90  BOTH  outgoing fermions
*       are required to scatter between THETA and 90 degrees
*       off their original direction. Analogously for 90 -> 180-THETA.
* For the meaning of the other parameters in the argument list, see
* the comments in subroutine BHABHA. For the meaning of the
* flags that govern which corrections are taken into account,
* see the comments in subroutine INITBH.
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      CALL BHABHA(S,THETA,90D0,THETA,90D0,BIGEP0,BIGEM0,ACOL,
     +                                             RESF,ERRF)
      CALL BHABHA(S,90D0,180D0-THETA,90D0,180D0-THETA,BIGEP0,BIGEM0,
     +                                               ACOL,RESB,ERRB)
      RESULT = 0D0
      ERROR  = 0D0
      TOTAL = RESF + RESB
      IF( ABS(TOTAL) .GT. 0D0 ) THEN
        RESULT = ( RESF - RESB )/TOTAL
        ERROR  = ( ERRF + ERRB )/TOTAL
      ENDIF
*     QCD correction factor for quarks:
      IF(IFERM.GT.3) RESULT = RESULT*( 1D0 - FACQCD )
      WRITE(IOUT,100) RESULT,ERROR
  100 FORMAT(/,' Afb (definition 2) = ',F15.5,' +- ',F15.5,/)
      END
 
      SUBROUTINE INITBH(XMZ,XMH,XMT)
*     -----------------
* Initialization routine, needed when using the routine BHABHA.
* The parameters are the masses of the Z, Higgs and top in GeV.
* All variables are input.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ FORMFA /FZV(0:NRMASS),FZA(0:NRMASS),
     +                FGV(0:NRMASS),FGA(0:NRMASS)
      COMPLEX*16 FZV,FZA,FGV,FGA
      COMMON/ FORMMZ /FZVMZ(0:NRMASS),FZAMZ(0:NRMASS),
     +                FGVMZ(0:NRMASS),FGAMZ(0:NRMASS)
      COMPLEX*16 FZVMZ,FZAMZ,FGVMZ,FGAMZ
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BOOKKP / IBH1,IBH2,IBH3,IBH4,IBH5,IBH6
      COMMON/ BHSTAT / ISIGH,ISIGF,ITWOH,IZBR,IRNON,IEVS,IEEW
      REAL*8 IMSIGZ,IMZ2
      CHARACTER*13 NAMES(0:NRMASS+1)
      DATA NAMES /'     neutrino','     electron','         muon',
     +            '          tau','     up quark','   down quark',
     +            '  charm quark','strange quark',
     +            '    top quark',' bottom quark','      hadrons'/
      RMZ    = XMZ
      RMH    = XMH
      RMT    = XMT
*
* A few options that can be set by the user. These determine what is
* calculated and what is not.
* IWEAK  : 0/1 switches weak (non-QED) corrections off/on.
* IORDER : the order of the initial state LL QED corrections. Valid are
*          0: no initial state LL corrections
*          1: first  order initial state LL corrections
*          2: second order initial state LL corrections
*          3: first  order initial state LL corrections+exponentiation
*          4: second order initial state LL corrections+exponentiation
*             For s-channel processes IORDER=0 also switches off the
*             non-log terms from the initial state (when t-channel is
*             included 'initial state radiation' is not a gauge
*             invariant subset any more).
* IFINAL : the order of the final state LL QED corrections. Valid are
*          0: no final state LL corrections
*          1: first  order final state LL corrections
*          2: second order final state LL corrections
*              For s-channel processes IFINAL=0 also switches off the
*              non-log terms from the final state (when t-channel is
*              included 'final state radiation' is not a gauge
*              invariant subset any more).
* NONLOG : 0/1 switches off/on: the calculation of the first order
*              non-log correction terms (SLOW! [ = 5 dim. int. ! ]).
*              It is NOT possible to switch off parts of the
*              non log terms (like final state e.g.) in the case of
*              Bhabha scattering.
* ( "Best     choice": iweak=1, iorder=4, ifinal=2, nonlog=1 (SLOW!),
*   "Best LLA choice": iweak=1, iorder=4, ifinal=2, nonlog=0        )
*
* These options can be altered after calling INITBH, by picking
* up the common block OPTION and change them elsewhere; another call
* to INITBH is NOT needed then. Upon changing Z, Higgs or top mass,
* however, INITBH has to be called again!
*
      IWEAK  = 1
      IORDER = 4
      IFINAL = 2
      NONLOG = 1
*
* IOUT  : the unit nr where the output goes to (6=screen).
* ICHANN: enables calculating only certain channels:
*         =0 calculates s + t channels plus interference,
*         =1 calculates s channel only,
*         =2 calculates t channel only,
*         The option =3 calculates only s-t interference for the
*         LL part but the s+t total for the non log part. To put
*         it simply: this option is not implemented very well.
* IFERM : final state fermion label. IFERM=1 means Bhabha scattering,
*         for the other ones see the data statement. The choices
*         8 (top) and 10 (hadrons) are not allowed. Neutrino
*         production can be calculated in s channel only, for one
*         species only.
*
      IOUT   = 6
!WP      ICHANN = 0
!WP: ICHANN set up by BHWIDE (from input parameters)
      CALL SETCHA(ICHANN)

      IFERM  = 1
*
* From here on: NOT user settable
*
!WP   ALFAS  = .12D0
      ALFAS = 0.119d0
* ALFA and HBARC2 from Particle Data Group Publ. 1990.
      ALFA   = 1D0 / 137.0359895D0
      HBARC2 = 3.8937966D8
      PI = 4D0*DATAN(1D0)
      EE = SQRT( 4D0*PI*ALFA )
* QCD and QED correction factors
      FACQCD = ALFAS/PI + (ALFAS/PI)**2*( 1.98D0 - 5D0*.115D0 )
      FACQCB = 0.045D0
      IF (ALFAS .LE. 0D0) THEN
        FACQCB = 0D0
      END IF
      FACQED = 3D0*ALFA/4D0/PI
* Starting value for sin**2(theta-w)
      SIN2TH = .2310D0
      RMW = RMZ*DSQRT( 1D0 - SIN2TH )
* Iterate to find the value for sin**2(theta-w) and Mw
* After this all couplings and renormalization constants are defined.
      NITER = 20
      CALL COUPLS(SIN2TH,RMT)
      DO 110 I = 1 , NITER
        RMWOLD = RMW
        CALL RNORM()
        CALL COUPLS(SIN2TH,RMT)
        IF(DABS(RMWOLD-RMW)/RMW .LT. 1D-6) GOTO 130
  110 CONTINUE
      WRITE(*,120) NITER
  120 FORMAT(' The calculation of MW does not converge in',I4,' steps')
      STOP' We stop right here !'
  130 CONTINUE
*
* Echo the given input and write all calculated parameters.
      WRITE(IOUT,135)
  135 FORMAT(/,' Thank you for flying .... choosing A L I B A B A',/,
     + ' ',35X,'=============',//,' A (semi) Analytical Leading log',
     + ' Improved BhABhA scattering calculation.',/,' This program is',
     + ' meant for large angle Bhabha scattering [and for other',/,
     + ' fermion pair production (but then only in s channel)].')
      WRITE(IOUT,140)
  140 FORMAT(/,' ',67('*'),/,
     1 ' * Authors: W.J.P. Beenakker, F.A. Berends and S.C.',
     2 ' van der Marck. *',/,' * Address: Instituut-Lorentz, ',
     3 'University of Leiden',16X,'*',/,' * ',9X,
     4 'P.o.b. 9506, 2300 RA Leiden, The Netherlands',11X,'*',/,
     5 ' * Bitnet addresses: BEENAKKER @ HLERUL59',26X,'*',/,
     6 ' * ',18X,'BERENDS @ HLERUL5 or BERENDS @ HLERUL59',7X,'*',/,
     7 ' * ',18X,'VANDERMARCK @ HLERUL59',24X,'*',/,
     8 ' *',19X,'joint address:',' LORENTZ @ HLERUL5',14X,'*',/,
     9 ' ',67('*'))
      WRITE(IOUT,145)
  145 FORMAT(' * References:',53X,'*',/,' * [1] W. Beenakker, F.A.',
     1 ' Berends and S.C. van der Marck,',10X,'*',/,
     2 ' *',5X,'"Large angle Bhabha scattering" and "Higher order',
     3 11X,'*',/,' *',5X,'corrections to the forward-backward',
     4 ' asymmetry,"',13X,'*',/
     5 ' *',5X,'Leiden preprints 1990, for the treatment of the',
     6 ' purely',6X,'*',/,' *',5X,'QED corrections and the',
     7 ' incorporation of cuts on energy and *',/,' *',5X,'angle',
     8 ' of both outgoing particles and their acollinearity.   *',/,
     9 ' * [2] W. Beenakker and W. Hollik, ECFA workshop on LEP 200,',
     1 7X,'*',/,' *',8X,'CERN 87-08 p.185, ed. by A. Boehm and W.',
     2 ' Hoogland;',7X,'*',/,' *',5X,'W. Hollik,',
     3 ' DESY preprint 88-188, both for the treatment',5X,'*',/,
     4 ' *',5X,'of the weak (non-QED) corrections.',26X,'*',/,
     5 ' ',67('*'),/,' Version 2.0, August 1990')
*
* Differences with version 1.0:
*  1) Integration from theta1 to theta2 (and theta3->theta4)
*     instead of theta->180-theta. Therefore Afb calculation
*     is possible with version 2.0.
*  2) Different energy cuts for e+ and e- available.
*  3) ZZ and WW boxes added.
*  4) Technicalities: Labda2 'stable' for x<1d-6.
*                     EEEEW s-t separation fixed.
*                     Erroneous check on acol cut left out.
*
      WRITE(IOUT,150) (I,NAMES(I),RMASS(I),PWIDTH(I),I=0,NRMASS)
  150 FORMAT(/,' The properties of the fermions:',/,3X,
     +       ' label',7X,'name',4X,' mass (GeV)',
     +       '  partial width of the Z (GeV)',/,
     +       (' ',I6,1X,A13,1X,F12.7,8X,F12.7))
      WRITE(IOUT,'(1X,I6,1X,A13,21X,F12.7)')NRMASS+1,'      hadrons',
     +              PWIDTH(4)+PWIDTH(5)+PWIDTH(6)+PWIDTH(7)+PWIDTH(9)
      Z = RMZ**2
      CALL FORMFS(Z,9)
      DO 160 I = 0 , NRMASS
         FZVMZ(I) = FZV(I)
         FZAMZ(I) = FZA(I)
         FGVMZ(I) = FGV(I)
         FGAMZ(I) = FGA(I)
  160 CONTINUE
      ZWID = (IMSIGZ(Z)+IMZ2(Z))/RMZ/(1D0+BIGPIZ(Z))
      WRITE(IOUT,170) RMZ,ZWID,RMW,SIN2TH,RMH
  170 FORMAT(/,' For the bosons we have (everything in GeV):',/,
     + '  mass of the   Z   =',F10.4,
     + '    total width of the Z = ',F10.7,/,
     + '  mass of the   W   =',F10.4,
     + '    <==> sin**2(theta-w) = ',F10.7,/,
     + '  mass of the Higgs =',F10.4,/)
      WRITE(IOUT,180) 1D0/ALFA,1D0+FACQED,ALFAS,1D0+FACQCD
  180 FORMAT(' Some coupling strengths:',/,
     +       '                    1/alfa = ',F10.3,/,
     +       ' the QED correction factor = ',F14.7,/,
     +       '               alfa-strong = ',F10.3,/,
     +       ' the QCD correction factor = ',F14.7,/)
* Don't forget to initialize the random number generator.
!WP   CALL RMARIN(2125,3106,IOUT)
* Bookkeeping
      IBH1 = IWEAK+10
      IBH2 = ICHANN
      IBH3 = IORDER
      IBH4 = IFINAL
      IBH5 = NONLOG
      IBH6 = IFERM
      ISIGH = 0
      ISIGF = 0
      ITWOH = 0
      IZBR  = 0
      IRNON = 0
      IEVS  = 0
      IEEW  = 0
      END
 
      SUBROUTINE ENDBH()
*     ----------------
* Print a few statistics. May give a clue on bottlenecks in the program
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BHSTAT / ISIGH,ISIGF,ITWOH,IZBR,IRNON,IEVS,IEEW
      WRITE(IOUT,10)   ISIGH,IEVS,ISIGF,IEEW,ITWOH,IZBR,IRNON
   10 FORMAT(/,' The following subroutines have been called ',
     +  'the following number of times:',/,'      SIGMAH  ',I15,
     +  10X,' EEEEVS  ',I15,/,'      SIGFLX  ',I15,
     +  10X,' EEEEW   ',I15,/,'      TWOHRD  ',I15,
     +  10X,' ZBRENT  ',I15,/,'      RNONLG  ',I15)
      END
 
      FUNCTION SIGHAT(X11,X21)
*     ---------------
* The part of SIGMAH that is symmetric in x1,x2. Under the condition
* that the same cuts are applied for both particles, SIGMAH already is
* symmetric.
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      IF ( ABS(C1+C3).LT.1D-6 .AND. ABS(C2+C4).LT.1D-6 .AND.
     +     ABS(EP0-EM0).LT.1D-3 ) THEN
        SIGHAT = SIGMAH(X11,X21)
      ELSE
        SIGHAT = .5D0*( SIGMAH(X11,X21) + SIGMAH(X21,X11) )
      ENDIF
      END
 
      FUNCTION SIGMAH(X11,X21)
*     ---------------
* The Born cross section for Bhabha scattering at reduced beam
* energies: p+ --> (1-x11)p+  ;  p- --> (1-x21)p- .
* This function includes weak corrections (but not the t dependence
* of them) if asked for. It also includes final state corrections
* if asked for.
* W. Beenakker and S.C. van der Marck, June, 1990.
* Adapted for version 2.0 (four angular cuts): July 1990.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ SAVEX  / X1SAVE, X2SAVE
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ FORMFA /FZV(0:NRMASS),FZA(0:NRMASS),
     +                FGV(0:NRMASS),FGA(0:NRMASS)
      COMPLEX*16 FZV,FZA,FGV,FGA
      COMPLEX*16 PROPG,PROPZ,MIXING,PROPGT,PROPZT,MIXINT,HADRQQ
      COMPLEX*16 PPS,PMS,ZPS,ZMS,PPSC,PMSC,ZPSC,ZMSC
      COMPLEX*16 PPT,PMT,ZPT,ZMT,PPTC,PMTC,ZPTC,ZMTC
      COMPLEX*16 PPS2,PMS2,ZPS2,ZMS2
      COMPLEX*16 PPS1,PMS1,ZPS1,ZMS1,PPSC1,PMSC1,ZPSC1,ZMSC1,ZFERM
      REAL*8 TMIN(1:2),TMAX(1:2),TINT(0:8),IMSGGZ
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BHSTAT / ISIGH,ISIGF,ITWOH,IZBR,IRNON,IEVS,IEEW
      EXTERNAL ACOLLI
      INTEGER TLOOP
      ISIGH=ISIGH+1
* preliminaries ...
      X1 = 1D0 - X11
      X2 = 1D0 - X21
      X1SAVE = X1
      X2SAVE = X2
      Z = X1 * X2
      ETA1 = 0D0
      ETA2 = 0D0
      ETA3 = 0D0
      ETA4 = 0D0
      IF(ABS(1D0-C1).GT.0D0) ETA1 = ( 1D0 + C1 )/( 1D0 - C1 )
      IF(ABS(1D0-C2).GT.0D0) ETA2 = ( 1D0 + C2 )/( 1D0 - C2 )
      IF(ABS(1D0+C3).GT.0D0) ETA3 = ( 1D0 - C3 )/( 1D0 + C3 )
      IF(ABS(1D0+C4).GT.0D0) ETA4 = ( 1D0 - C4 )/( 1D0 + C4 )
      SHAT = Z*SCM
      RMZ2 = RMZ**2
* The minimum and maximum value allowed for t, using E and c cuts.
      IF(C1*C2.GT.1D-6)C2=C1
      IF(C3*C4.GT.1D-6)C4=C3
      IF(ABS(1D0-C2).GT.0D0) THEN
        TMIN(1) = - X1*SHAT/( X1 + X2*ETA2 )
      ELSE
        TMIN(1) = 0D0
      ENDIF
      IF(ABS(1D0-C1).GT.0D0) THEN
        TMAX(1) = - X1*SHAT/( X1 + X2*ETA1 )
      ELSE
        TMAX(1) = 0D0
      ENDIF
      IF(ABS(1D0+C4).GT.0D0) THEN
        TMIN(1) = MAX( TMIN(1) , - X2*SHAT/( X2 + X1*ETA4 ) )
      ELSE
        TMIN(1) = 0D0
      ENDIF
      IF(ABS(1D0+C3).GT.0D0) THEN
        TMAX(1) = MIN( TMAX(1) , - X2*SHAT/( X2 + X1*ETA3 ) )
      ELSE
        TMAX(1) = MIN( TMAX(1) , 0D0 )
      ENDIF
      IF(X1 .GT. X2) THEN
        TMIN(1) = MAX( TMIN(1) ,  SHAT*(X1-EP0)/(X2-X1) )
        TMAX(1) = MIN( TMAX(1) , -SHAT*(X2-EM0)/(X2-X1) )
      ELSEIF(X2 .GT. X1) THEN
        TMIN(1) = MAX( TMIN(1) , -SHAT*(X2-EM0)/(X2-X1) )
        TMAX(1) = MIN( TMAX(1) ,  SHAT*(X1-EP0)/(X2-X1) )
      ENDIF
      TMIN(2) = 0D0
      TMAX(2) = 0D0
*
*     Now implement the acollinearity cut. Possibly get two t regimes.
      IT = 1
      IF(TMIN(1).LT.TMAX(1).AND.ABS(X1-X2).GT.0D0.AND.
     +   ABS(ACOLMX-PI).GT.1D-10) THEN
        TMID = - .5D0*X1*X2*SCM
        ACCUCY = 1D-5
        IF ( TMID.GT.TMIN(1) .AND. TMID.LT.TMAX(1) ) THEN
*         The symmetric point lies in the interval. Therefore
*         the acollinearity has its maximum there.
          F2 = ACOLLI(TMID)
*         If f2 is negative, the acollinearity cut is always satisfied.
          IF ( F2 .GT. 0D0 ) THEN
            F1 = ACOLLI(TMIN(1))
            F3 = ACOLLI(TMAX(1))
            IF ( F1 .GT. 0D0 .AND. F3 .GT. 0D0 ) THEN
*             No possibility to satisfy the acollinearity cut.
              IT = - 1
            ELSE
*             Two distinct t regions to be integrated over.
              IT = 2
              TMIN(2) = TMIN(1)
              TMAX(2) = TMAX(1)
              IF ( F1 .GT. 0D0 ) THEN
*               The acollinearity cut cannot be satisfied.
                TMIN(1) = TMAX(1) + 1D0
              ELSE
*               Find t value with the max. allowed acollinearity.
                IZBR    = IZBR+1
                TMAX(1) = ZBRENT(ACOLLI,TMIN(1),TMID,ACCUCY)
              ENDIF
              IF ( F3 .GT. 0D0 ) THEN
*               The acollinearity cut cannot be satisfied.
                TMIN(2) = TMAX(2) + 1D0
              ELSE
*               Find t value with the max. allowed acollinearity.
                IZBR    = IZBR+1
                TMIN(2) = ZBRENT(ACOLLI,TMID,TMAX(2),ACCUCY)
              ENDIF
            ENDIF
          ENDIF
        ELSEIF ( TMID.LT.TMIN(1) ) THEN
*         acollinearity decreases from TMIN to TMAX
          F1 = ACOLLI(TMIN(1))
          F2 = ACOLLI(TMAX(1))
          IF( F2 .LT. 0D0 ) THEN
            IF ( F1 .GT. 0D0 ) THEN
              IZBR    = IZBR+1
              TMIN(1) = ZBRENT(ACOLLI,TMIN(1),TMAX(1),ACCUCY)
            ENDIF
          ELSE
            TMIN(1) = TMAX(1) + 1D0
          ENDIF
        ELSE
*         acollinearity increases from TMIN to TMAX
          F1 = ACOLLI(TMIN(1))
          F2 = ACOLLI(TMAX(1))
          IF( F1 .LT. 0D0 ) THEN
            IF ( F2 .GT. 0D0 ) THEN
              IZBR    = IZBR+1
              TMAX(1) = ZBRENT(ACOLLI,TMIN(1),TMAX(1),ACCUCY)
            ENDIF
          ELSE
            TMIN(1) = TMAX(1) + 1D0
          ENDIF
        ENDIF
      ENDIF
*
      TOTAL = 0D0
      DO 100 TLOOP = 1 , IT
*       If the minimum is larger than the maximum, the integral
*       over t is defined as 0 !
        IF ( TMIN(TLOOP) .GT. TMAX(TLOOP) ) GOTO 100
*       Define coupling constants that incorporate vertex corrections
*       and propagator corrections. First the s channel consts.
        I=IFERM
        IF(ICHANN .NE. 2) THEN
*         First calculate propagators and vertex corrections.
          CALL GZPROP(SHAT,PROPG ,PROPZ ,MIXING)
          IF(IWEAK .NE. 0) THEN
            CALL FORMFS(SHAT,1)
            MIXING = CMPLX( SIGGZ(SHAT) , IMSGGZ(SHAT) ) / SHAT
            FZV(1) = FZV(1) + QF(1)*MIXING
            IF(I.NE.1) FZV(I) = FZV(I) + QF(I)*MIXING
            PPS = - QF(1) - FGV(1) - (       - FGA(1) )
            PMS = - QF(1) - FGV(1) + (       - FGA(1) )
            ZPS =   VF(1) + FZV(1) - ( AF(1) + FZA(1) )
            ZMS =   VF(1) + FZV(1) + ( AF(1) + FZA(1) )
            PPS2= - QF(I) - FGV(I) - (       - FGA(I) )
            PMS2= - QF(I) - FGV(I) + (       - FGA(I) )
            ZPS2=   VF(I) + FZV(I) - ( AF(I) + FZA(I) )
            ZMS2=   VF(I) + FZV(I) + ( AF(I) + FZA(I) )
          ELSE
            PPS = - QF(1)
            PMS = - QF(1)
            ZPS =   VF(1) - AF(1)
            ZMS =   VF(1) + AF(1)
            PPS2= - QF(I)
            PMS2= - QF(I)
            ZPS2=   VF(I) - AF(I)
            ZMS2=   VF(I) + AF(I)
          ENDIF
          ZINT=.5D0*(PPS*PMS2*SHAT*PROPG*CONJG( ZPS*ZMS2*SHAT*PROPZ )
     +            +  PMS*PPS2*SHAT*PROPG*CONJG( ZMS*ZPS2*SHAT*PROPZ ))
          IF(IFERM .NE. 1) THEN
            PPS1 = PPS * PMS2 * SHAT*PROPG
            PMS1 = PMS * PPS2 * SHAT*PROPG
            ZPS1 = ZPS * ZMS2 * SHAT*PROPZ
            ZMS1 = ZMS * ZPS2 * SHAT*PROPZ
            PPSC1= CONJG( PPS1 )
            PMSC1= CONJG( PMS1 )
            ZPSC1= CONJG( ZPS1 )
            ZMSC1= CONJG( ZMS1 )
            ZFERM= PPS*PPS2*SHAT*PROPG * CONJG( ZPS*ZPS2*SHAT*PROPZ )
     +           + PMS*PMS2*SHAT*PROPG * CONJG( ZMS*ZMS2*SHAT*PROPZ )
          ENDIF
          PPS = PPS * PPS2 * SHAT*PROPG
          PMS = PMS * PMS2 * SHAT*PROPG
          ZPS = ZPS * ZPS2 * SHAT*PROPZ
          ZMS = ZMS * ZMS2 * SHAT*PROPZ
        ELSE
*         Only t channel wanted.
          PPS = (0D0,0D0)
          PMS = (0D0,0D0)
          ZPS = (0D0,0D0)
          ZMS = (0D0,0D0)
          ZINT= (0D0,0D0)
        ENDIF
        PPSC = CONJG( PPS )
        PMSC = CONJG( PMS )
        ZPSC = CONJG( ZPS )
        ZMSC = CONJG( ZMS )
*       Now the t channel consts
*       Start by choosing a value for T to calculate these things at.
        T = - 2D0*SHAT*( 1D0 - C1 )/( 1D0 + Z + C1*( Z - 1D0 ) )
        IF( T.LT.TMIN(TLOOP) .OR. T.GT.TMAX(TLOOP) )
     +      T = ( TMIN(TLOOP) + TMAX(TLOOP) )/2D0
        IF(ICHANN .NE. 1) THEN
*         The t channel propagators and vertex corrections ...
          CALL GZPROP(  T ,PROPGT,PROPZT,MIXINT)
*         Incorporate the Burkhardt fit to the dispersion integral to
*         have the correct t channel photon propagator corrections
*         (Watch the minus sign: difference in definition of PI-gamma.)
          IF(IWEAK.EQ.1) PROPGT=PROPGT/( 1D0 - HADRQQ(T) - PHADPI(T) )
          IF(ABS(IWEAK) .EQ. 1) THEN
            CALL FORMFS(T,1)
            MIXINT = DCMPLX( SIGGZ(T) , IMSGGZ(T) ) /T
            FZV(1) = FZV(1) + QF(1)*MIXINT/T
            PPT = - QF(1) - FGV(1) - (       - FGA(1) )
            PMT = - QF(1) - FGV(1) + (       - FGA(1) )
            ZPT =   VF(1) + FZV(1) - ( AF(1) + FZA(1) )
            ZMT =   VF(1) + FZV(1) + ( AF(1) + FZA(1) )
          ELSE
            PPT = - QF(1)
            PMT = - QF(1)
            ZPT =   VF(1) - AF(1)
            ZMT =   VF(1) + AF(1)
          ENDIF
          PROPGT =   T  * PROPGT
          PROPZT = ( T - RMZ**2 ) * PROPZT
          Z100 =(PPT*ZPT+PMT*ZMT)*PROPGT*CONJG((PPT*ZPT+PMT*ZMT)*PROPZT)
          PPT = PPT * PPT * PROPGT
          PMT = PMT * PMT * PROPGT
          ZPT = ZPT * ZPT * PROPZT
          ZMT = ZMT * ZMT * PROPZT
        ELSE
*         Only s channel wanted.
          PPT = (0D0,0D0)
          PMT = (0D0,0D0)
          ZPT = (0D0,0D0)
          ZMT = (0D0,0D0)
          Z100= (0D0,0D0)
        ENDIF
        PPTC = CONJG( PPT )
        PMTC = CONJG( PMT )
        ZPTC = CONJG( ZPT )
        ZMTC = CONJG( ZMT )
*
*       Calculate the structures that multiply the distinct t-integrals
        IF(ICHANN .NE. 3) THEN
          IF(IFERM .EQ. 1) THEN
            W1 = ( PPS+PMS )*( PPSC+PMSC ) + ( ZPS+ZMS )*( ZPSC+ZMSC )
     +         + 2D0*( PPS*ZPSC + PMS*ZMSC + 2D0*ZINT )
          ELSE
            W1 = ZPS1*ZPSC1 + ZMS1*ZMSC1 + ZPS*ZPSC + ZMS*ZMSC +
     +           PPS1*PPSC1 + PMS1*PMSC1 + PPS*PPSC + PMS*PMSC +
     +           2D0*( ZFERM + 2D0*ZINT )
          ENDIF
          W2 = 2D0*( PPS*PPSC + PMS*PMSC ) +2D0*( ZPS*ZPSC + ZMS*ZMSC )
     +       + 4D0*( PPS*ZPSC + PMS*ZMSC )
          WA = .5D0 * W2
          W3 = 2D0*( PPT*ZPSC + PMT*ZMSC ) +2D0*( ZPS*ZPTC + ZMS*ZMTC )
     +       + 2D0*( PPS*PPTC + PMS*PMTC ) +2D0*( PPS*ZPTC + PMS*ZMTC )
          W4 = PPT*PPTC + PMT*PMTC + 2D0*( PPT*ZPTC + PMT*ZMTC )
     1       + ZPT*ZPTC + ZMT*ZMTC + 4D0*( PPS*PPTC + PMS*PMTC ) +
     2       2D0*( 2D0 + RMZ2/SHAT )*( PPS*ZPTC + PMS*ZMTC ) +
     3     ( 4D0*( PPT*ZPSC + PMT*ZMSC ) +
     4       2D0*( 2D0 + RMZ2/SHAT )*( ZPS*ZPTC + ZMS*ZMTC ) )
          W5 = - 2D0*SHAT/RMZ2*Z100        +2D0*( PPS*PPTC + PMS*PMTC )
     +       + 2D0*( PPT*PPTC + PMT*PMTC ) +2D0*( PPT*ZPSC + PMT*ZMSC )
          W6 = 2D0*SHAT/RMZ2*Z100 + 2D0*( PPS*ZPTC + PMS*ZMTC ) +
     1          ( 2D0 + RMZ2/SHAT )*2D0*( PPT*ZPTC + PMT*ZMTC ) +
     2          ( 1D0 + RMZ2/SHAT )*2D0*( ZPT*ZPTC + ZMT*ZMTC ) +
     3       (2D0+RMZ2/SHAT)*2D0*( PPS*ZPTC + PMS*ZMTC )*RMZ2/SHAT +
     4       2D0*( ZPS*ZPTC + ZMS*ZMTC )*( 1D0 + RMZ2/SHAT )**2
          W7 = ( PPT + PMT )*( PPTC + PMTC )
          W8 = ( ZPT + ZMT )*( ZPTC + ZMTC ) +
     +         ( ZPT*ZPTC + ZMT*ZMTC )*RMZ2/SHAT*( 2D0 + RMZ2/SHAT )
        ELSE
*         Only s-t interference wanted.
          W1 = 0D0
          W2 = 0D0
          WA = 0D0
          W3 = 2D0*( PPT*ZPSC + PMT*ZMSC ) +2D0*( ZPS*ZPTC + ZMS*ZMTC )
     +       + 2D0*( PPS*PPTC + PMS*PMTC ) +2D0*( PPS*ZPTC + PMS*ZMTC )
          W4 = 4D0*( PPS*PPTC + PMS*PMTC ) +
     +       2D0*( 2D0 + RMZ2/SHAT )*( PPS*ZPTC + PMS*ZMTC ) +
     +     ( 4D0*( PPT*ZPSC + PMT*ZMSC ) +
     +       2D0*( 2D0 + RMZ2/SHAT )*( ZPS*ZPTC + ZMS*ZMTC ) )
          W5 = 2D0*( PPS*PPTC + PMS*PMTC ) +2D0*( PPT*ZPSC + PMT*ZMSC )
          W6 = 2D0*( PPS*ZPTC + PMS*ZMTC ) +
     +      (2D0+RMZ2/SHAT)*2D0*( PPS*ZPTC + PMS*ZMTC )*RMZ2/SHAT +
     +       2D0*( ZPS*ZPTC + ZMS*ZMTC )*( 1D0 + RMZ2/SHAT )**2
          W7 = 0D0
          W8 = 0D0
        ENDIF
*       Calculate the T integrals,
        CALL CALINT(TINT,SHAT,TMIN(TLOOP),TMAX(TLOOP),X1,X2)
*       and add all the contributions:
        SUM = W1*TINT(0) + W2*TINT(1) + WA*TINT(2) + W3*TINT(3) +
     +        W4*TINT(4) + W5*TINT(5) + W6*TINT(6) + W7*TINT(7) +
     +        W8*TINT(8)
        SUM = 1D0/SHAT/SHAT * PI*ALFA*ALFA * SUM
        TOTAL = TOTAL + SUM
  100 CONTINUE
      SIGMAH = HBARC2 * TOTAL
      END
 
      FUNCTION ACOLLI(T)
*     ---------------
* Calculates the acollinearity corresponding to a " X1,X2,T event ",
* minus the maximum acollinearity ACOLMX that has been asked for.
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ SAVEX  / X1SAVE, X2SAVE
      IF(ABS(T) .LE. 0D0) THEN
        ACOLLI = - ACOLMX
      ELSE
        COS3 =   1D0 - 2D0/( 1D0-X1SAVE/X2SAVE - X1SAVE*X1SAVE*SCM/T )
        COS4 = - 1D0 + 2D0/( 1D0-X2SAVE/X1SAVE - X2SAVE*X2SAVE*SCM/T )
        IF(COS3 .LT. -1D0) COS3 = -1D0
        IF(COS3 .GT.  1D0) COS3 =  1D0
        IF(COS4 .LT. -1D0) COS4 = -1D0
        IF(COS4 .GT.  1D0) COS4 =  1D0
        THETA3 = ACOS( COS3 )
        THETA4 = ACOS( COS4 )
        ACOLLI = ABS( PI - THETA3 - THETA4 ) - ACOLMX
      ENDIF
      END
 
      SUBROUTINE CALINT(TINT,SHAT,TMIN,TMAX,X1,X2)
*     -----------------
* The result of the integration over T from tmin to tmax is put
* into the array TINT. The integration incorporates first order final
* state QED corrections, when the flag IFINAL is set accordingly.
* W. Beenakker and S.C. van der Marck, February, 1990.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      REAL*8 TINT(0:8),LOG1,LOG2,LOG3,LOG4,LOG5
* Statement functions for the no final state corrections case
      R1(T) = 1D0/3D0*T**3/SHAT**2
      R2(T) =    .5D0*T**2/SHAT
      RA(T) = T
      R3(T) = R2(T)
      R4(T) = RA(T)
      R5(T) = SHAT*LOG( - T )
      R6(T) = SHAT*LOG( - T + RMZ2 )
      R7(T) = - SHAT**2/T
      R8(T) = - SHAT**2/( T - RMZ2 )
*
      RMZ2 = RMZ**2
      TINT(0) = R1(TMAX) - R1(TMIN)
      TINT(1) = R2(TMAX) - R2(TMIN)
      TINT(2) = RA(TMAX) - RA(TMIN)
      IF(ICHANN .NE. 1) THEN
        TINT(3) = R3(TMAX) - R3(TMIN)
        TINT(4) = R4(TMAX) - R4(TMIN)
        TINT(5) = R5(TMAX) - R5(TMIN)
        TINT(6) = R6(TMAX) - R6(TMIN)
        TINT(7) = R7(TMAX) - R7(TMIN)
        TINT(8) = R8(TMAX) - R8(TMIN)
      ELSE
*       No t channel wanted; by not calculating these integrals
*       we open up the possibility to have the angular cut go to zero.
        TINT(3) = 0D0
        TINT(4) = 0D0
        TINT(5) = 0D0
        TINT(6) = 0D0
        TINT(7) = 0D0
        TINT(8) = 0D0
      ENDIF
*
      EBEAM = .5D0*SQRT(SCM)
      IF(IFINAL.GE.1.AND.((EP0.GT.1D-3.AND.EP0.GT.RMASS(IFERM)/EBEAM)
     +  .OR.(EM0.GT.1D-3.AND.EM0.GT.RMASS(IFERM)/EBEAM))) THEN
*       Final state corrections (=0 if energy cuts EP0 = EM0 = 0)
*       i=1,2: x3 part, where (i=1 <--> tmax) and (i=2 <--> tmin)
*       i=3,4: x4 part, where (i=3 <--> tmax) and (i=4 <--> tmin)
        BETA = BETALF + 2D0*ALFA/PI*QF(IFERM)**2
        DO 10 I = 1 , 4
          IF(I .LE. 2) THEN
            IF(EP0.LE.RMASS(IFERM)/EBEAM.OR.EP0.LT.1D-3) GOTO 10
            A = X1/EP0
            B = - 1D0/EP0/SCM * (1D0-X1/X2) /X1
          ELSE
            IF(EM0.LE.RMASS(IFERM)/EBEAM.OR.EM0.LT.1D-3) GOTO 10
            A = X2/EM0
            B =   1D0/EM0/SCM * (1D0-X1/X2) /X1
          ENDIF
          IF(I .EQ. 1 .OR. I .EQ. 3) THEN
            T    = TMAX
            SIGN =   1D0
          ELSE
            T    = TMIN
            SIGN = - 1D0
          ENDIF
          S3 = 0D0
          S4 = 0D0
          S5 = 0D0
          S6 = 0D0
          S7 = 0D0
          S8 = 0D0
          T2 = T**2
          T3 = T*T2
          LOG1 = LOG( CMPLX( A + B*T           ,0D0) )
          LOG4 = (0D0,0D0)
          IF(ICHANN.NE.1)  LOG4 = LOG( CMPLX( T,0D0) )
          LOG5 = LOG( CMPLX( T - RMZ2        ,0D0) )
*         If b too small we have numerical problems, so in that case
*         calculate a form where we have expanded around b=0.
          IF(ABS(B*TMIN).GT.1D-2.OR.ABS(B*TMAX).GT.1D-2 .OR.
     +       ( ABS(A-1D0) .LT. 1D-10 .AND. ABS(B).GT.1D-10 ) ) THEN
            S1 = BETALF*( - T2/6D0/B - T/3D0/B/B*(-2D0*A+1D0) ) +
     +           BETA  *( .5D0/B*( .5D0*
     +            T2 - A/B*T + A*A/B/B*LOG1 ) + .25D0/B/B*( T -
     +            2D0*A/B*LOG1 - A*A/B/(A+B*T) ) )
            S2 = BETALF*( -T/2D0/B ) + BETA*( .5D0/B*( T - A/B*LOG1 )
     +                   +.25D0/B/B*( LOG1 + A/(A+B*T) ) )
            SA = BETA*( .5D0/B*LOG1 - .25D0/B/(A+B*T) )
            S3 = S2
            S4 = SA
            IF(ICHANN .NE. 1) THEN
              IF( ABS(A-1D0) .LT. 1D-10 ) THEN
                S5 = BETALF*(.5D0*LOG(CMPLX(B*T,0D0))**2+DILOG(-B*T))+
     +               BETA*(-.75D0*(LOG1-LOG4) + .25D0/(1D0+B*T) )
                S7 = - (BETALF+BETA)*B*LOG(CMPLX(T/(1D0/B+T),0D0)) +
     +                BETA*( -.75D0/T - .25D0*B/(1D0+B*T) )
              ELSEIF( ABS(A) .LT. .05D0 ) THEN
                S5 = BETALF*( -DILOG(B*T)-.5D0*LOG(ABS(B*T))**2 ) +
     +               BETA*( -.5D0/B/T - 1D0/8D0/B/B/T/T )
                S7 = BETA*( 1D0/T - .25D0/B/T2 - 1D0/12D0/B/B/T3 )
              ELSE
                S5 = BETALF*( LOG(CMPLX(1D0-1D0/A,0D0))*LOG4 -
     +                        DILOG(-B*T/(A-1D0)) + DILOG(-B*T/A) ) +
     +               BETA*( - .5D0/A*(1D0+.5D0/A)*( LOG1-LOG4 ) +
     +                     .25D0/A/(A+B*T) )
                S7 = ( BETA*B/A/A/2D0*(1D0+1D0/A)-BETALF*B/A/(A-1D0) )*
     +               (LOG1-LOG4) + BETA*( - .5D0/A*(1D0+.5D0/A)/T
     +                                    - B/4D0/A/A/(A+B*T) )
              ENDIF
              IF( ABS(A+B*RMZ2) .GT. 1D-2) THEN
                S6 = BETALF*(LOG(CMPLX(1D0-1D0/(A+B*RMZ2),0D0))*LOG5 -
     +              DILOG(-(T-RMZ2)/((A-1D0)/B+RMZ2)) +
     +              DILOG(-(T-RMZ2)/( A     /B+RMZ2)) ) +
     +             BETA*( .5D0/(A+B*RMZ2)*(1D0+.5D0/(A+B*RMZ2))*
     +                   (LOG5-LOG1) + .25D0/(A+B*RMZ2)/(A+B*T) )
                S8 = ( BETA*B/2D0/(A+B*RMZ2)**2*(1D0+1D0/(A+B*RMZ2))
     +               -BETALF*B/(A+B*RMZ2)/(A-1D0+B*RMZ2) )*
     +               ( LOG1 - LOG5 ) + BETA*(
     +             -.5D0/(A+B*RMZ2)*(1D0+.5D0/(A+B*RMZ2))/(T-RMZ2)-
     +              B/4D0/(A+B*RMZ2)**2/(A+B*T) )
              ELSE
                S6 = -DILOG(B*(T-RMZ2))-.5D0*LOG(ABS(B*(T-RMZ2)))**2-
     +               .5D0/B/(T-RMZ2) - 1D0/8D0/B/B/(T-RMZ2)**2
                S8 = -(1D0/(T-RMZ2)-B)*LOG1 -1D0/(T-RMZ2)
     +                -.25D0/B/(T-RMZ2)**2-1D0/12D0/B/B/(T-RMZ2)**3
              ENDIF
            ENDIF
            IF( ABS( (A+B*T)-1D0 ) .GE. 1D-6 ) THEN
*             Parts that shouldn't be evaluated at a+b*t=1 (0*log(0))
              LOG2 = LOG(CMPLX( 1D0 - 1D0/(A+B*T) ,0D0) )
              LOG3 = LOG(CMPLX( (A-1D0) + B*T     ,0D0) )
              S1 = S1 + BETALF*( 1D0/3D0*( T3 + (A/B)**3 )*LOG2 +
     +                   1D0/3D0/B**3*(-3D0*A*A+3D0*A-1D0)*LOG3 )
              S2E = BETALF*( + .5D0*( T2 - A*A/B/B )*LOG2 -
     +                    .5D0/B/B*( - 2D0*A + 1D0 )*LOG3 )
              SAE = BETALF*( + ( A/B + T )*LOG2 - 1D0/B*LOG3 )
              IF(ICHANN .NE. 1) THEN
                IF( ABS(A-1D0) .GT. 1D-10 ) THEN
                  S7 = S7 - BETALF*( ( 1D0/T + B/(A-1D0) )*LOG2 )
                ELSE
                  S7 = S7 - BETALF/T*(1D0+LOG4-LOG(CMPLX(1D0/B+T,0D0)))
                ENDIF
                IF( ABS(A+B*RMZ2) .GT. 1D-2)
     +            S8 = S8 - BETALF*(1D0/(T-RMZ2)+B/(A-1D0+B*RMZ2))*LOG2
              ENDIF
            ENDIF
            S2 = S2 + S2E
            SA = SA + SAE
            S3 = S3 + S2E
            S4 = S4 + SAE
            IF(ABS(A-1D0) .GT. 1D-10) THEN
              IF(ABS(B*TMIN/(A-1D0)) .LT. 1D-1 .AND.
     +           ABS(B*TMAX/(A-1D0)) .LT. 1D-1      ) THEN
                S1=T3/3D0*(BETALF*LOG(ABS((A-1D0)/A)) + BETA*
     +              .5D0/A*(1D0+.5D0/A)) + BETA*(
     +            - B*T2*T2*( (1D0+1D0/A)/8D0/A/A-1D0/4D0/A/(A-1D0) ) )
              ENDIF
            ELSE
              LOG1 = LOG(CMPLX(1D0/B+T,0D0))
              S1=BETALF*(T3/3D0*( LOG4 - LOG1 ) - 1D0/3D0/B**3*LOG1) +
     +           BETA*( ( T2 + T/B )/12D0/B - .25D0/B**3/(1D0+B*T) )
            ENDIF
          ELSE
            IF( ABS(A-1D0)        .GT. 1D-10 .AND.
     +          ABS(A+B*TMIN-1D0) .GT. 1D-10 .AND.
     +          ABS(A+B*TMAX-1D0) .GT. 1D-10      ) THEN
*             For S1,S2,SA we have expanded around b=0, the others
*             do not need that.
              S1 = T3/3D0*(BETALF*LOG(ABS((A-1D0)/A))+BETA*.5D0/A*
     +             (1D0+.5D0/A)) + BETA*(
     +         - B*T**4*( (1D0+1D0/A)/8D0/A/A - 1D0/4D0/A/(A-1D0) ) )
              S2 = T2/2D0*(BETALF*LOG(ABS((A-1D0)/A))+BETA*.5D0/A*
     +             (1D0+.5D0/A)) + BETA*(
     +           - B*T3*( (1D0+1D0/A)/6D0/A/A - 1D0/3D0/A/(A-1D0) ) )
              SA = T     *(BETALF*LOG(ABS((A-1D0)/A))+BETA*.5D0/A*
     +             (1D0+.5D0/A)) + BETA*(
     +           - B*T2*( (1D0+1D0/A)/4D0/A/A - 1D0/2D0/A/(A-1D0) ) )
              S3 = S2
              S4 = SA
              IF(ICHANN .NE. 1) THEN
                S5 = BETALF*( LOG(CMPLX(1D0-1D0/A,0D0))*LOG4-
     +                        DILOG(-B*T/(A-1D0)) + DILOG(-B*T/A) ) +
     +               BETA*( -.5D0/A*(1D0+.5D0/A)*( LOG1 - LOG4 ) +
     +                     .25D0/A/(A+B*T) )
                S6 = BETALF*( LOG(CMPLX(1D0-1D0/(A+B*RMZ2),0D0))*LOG5 -
     +              DILOG(-B*(T-RMZ2)/((A-1D0)+B*RMZ2)) +
     +              DILOG(-B*(T-RMZ2)/( A     +B*RMZ2)) ) + BETA*(
     +             .5D0/(A+B*RMZ2)*(1D0+.5D0/(A+B*RMZ2))*(LOG5-LOG1)
     +            + .25D0/(A+B*RMZ2)/(A+B*T) )
                S7 = ( BETA*B/A/A/2D0*(1D0+1D0/A)-BETALF*B/A/(A-1D0) )*
     +               (LOG1-LOG4) + BETA*(
     +           - .5D0/A*(1D0+.5D0/A)/T - B/4D0/A/A/(A+B*T) )
                S8 = ( BETA*B/2D0/(A+B*RMZ2)**2*(1D0+1D0/(A+B*RMZ2))
     +             -BETALF*B/(A+B*RMZ2)/(A-1D0+B*RMZ2) )*( LOG1-LOG5 )
     +             + BETA*( -.5D0/(A+B*RMZ2)*(1D0+.5D0/(A+B*RMZ2))/
     +                   (T-RMZ2)-B/4D0/(A+B*RMZ2)**2/(A+B*T) )
                LOG2 = LOG(CMPLX(1D0-1D0/(A+B*T),0D0) )
                S7 = S7 - BETALF*( 1D0/T + B/(A-1D0) )*LOG2
                S8 = S8 - BETALF*(1D0/(T-RMZ2)+B/(A-1D0+B*RMZ2))*LOG2
              ENDIF
            ELSE
              S1 = 0D0
              S2 = 0D0
              SA = 0D0
              S3 = 0D0
              S4 = 0D0
              S5 = 0D0
              S6 = 0D0
              S7 = 0D0
              S8 = 0D0
            ENDIF
          ENDIF
          S1 = S1/SHAT/SHAT
          S2 = S2/SHAT
          SA = SA
          S3 = S3/SHAT
          S4 = S4
          S5 = S5*SHAT
          S6 = S6*SHAT
          S7 = S7*SHAT*SHAT
          S8 = S8*SHAT*SHAT
          FAC = .5D0 * SIGN
          TINT(0) = TINT(0) + FAC * S1
          TINT(1) = TINT(1) + FAC * S2
          TINT(2) = TINT(2) + FAC * SA
          TINT(3) = TINT(3) + FAC * S3
          TINT(4) = TINT(4) + FAC * S4
          TINT(5) = TINT(5) + FAC * S5
          TINT(6) = TINT(6) + FAC * S6
          TINT(7) = TINT(7) + FAC * S7
          TINT(8) = TINT(8) + FAC * S8
   10   CONTINUE
      ENDIF
      END
 
      FUNCTION SIGFLX(Y1)
*     ---------------
* The hard part of the structure functions times the reduced cross
* section. Of the latter only the part with x1=z,x2=1 is taken.
* Here we calculate the structure function part, the reduced cross
* section is given by SIGHAT. The form of the structure functions
* depends on the required QED order at which one wants to calculate
* the corrections. This is governed by IORDER.
* This function is to be integrated over numerically over Y1
* (Caution: Y1 is not simply the energy fraction x. It differs
* due to a mapping to make the integral more smooth.)
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BHSTAT / ISIGH,ISIGF,ITWOH,IZBR,IRNON,IEVS,IEEW
      ISIGF=ISIGF+1
*
      BETAL1 = BETALL + 2D0*ALFA/PI
      IF(IORDER .LT. 3) THEN
        Z1  = EXP( Y1 )
        Z   = 1D0 - Z1
        PEE = BETALL*2D0 - BETAL1*( 1D0 - Z*Z )
        IF(IORDER .EQ. 2) THEN
          PEEPEE = ( 1D0 + Z*Z )*( 2D0*LOG(Z1) - LOG(Z) + 3D0/2D0 )
     +                + Z1*( + .5D0*(1D0+Z)*LOG(Z) - (1D0-Z) )
        ELSE
          PEEPEE = 0D0
        ENDIF
        FLUX = .5D0*PEE + .25D0*BETALL**2*PEEPEE
      ELSEIF(IORDER .EQ. 3 .OR. IORDER .EQ. 4) THEN
        Z1 = Y1**(1D0/BETALL)
        FLUX = 1D0 +.75D0*BETAL1
        IF(IORDER.EQ.4) FLUX=FLUX+.25D0*(9D0/8D0-PI**2/3D0)*BETAL1**2
      ELSE
        Z1  = EXP( Y1 )
        Z   = 1D0 - Z1
        PEE = Z1*( - ( 1D0 + Z ) )
        PEEPEE = Z1*( -(1D0+Z**2)/Z1*LOG(Z) + (1D0+Z)*(
     +               - 2D0*LOG(Z1) + .5D0*LOG(Z) ) - Z/2D0 - 2.5D0 )
        FLUX = .5D0*BETAL1*PEE
        IF(IORDER.EQ.6) FLUX = FLUX + .25D0*BETAL1**2*PEEPEE
      ENDIF
*
      SIGFLX = FLUX * SIGHAT(Z1,0D0)
      END
 
      FUNCTION TWOHRD(X,WEIGHT)
*     -------------------------
* The hard part of the structure functions times the reduced cross
* section, the part where both x1 and x2 are not equal to 1.
* Two dimensional integral needed (in this form to be done by VEGAS).
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ BHSTAT / ISIGH,ISIGF,ITWOH,IZBR,IRNON,IEVS,IEEW
      REAL*8 X(1:2)
      ITWOH=ITWOH+1
      Z1 = X(1)
      Y1 = X(2)
      Z  = 1D0 - Z1
      Y  = 1D0 - Y1
      PEE1 = ( 1D0 + Z*Z )/Z1
      PEE2 = ( 1D0 + Y*Y )/Y1
      FLUX = 1D0/16D0 * (BETALL+2D0*ALFA/PI)**2 * PEE1*PEE2
      TWOHRD = FLUX*( SIGHAT(Z1,Y1) - SIGHAT(Z1+Y1-Z1*Y1,0D0) )
      END
 
      FUNCTION FINAL2()
*     ---------------
* Second order LL final state corrections as a factor times Born.
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
* This was the formula for EP0=EM0
*      RLOG1 = LOG( 1D0 - E0 )
*      RLOG2 = LOG( E0 )
*      DILO  = DILOG( 1D0 - E0 )
*      FAC = ( 2D0*RLOG1 + E0 + .5D0*E0**2 )**2 + 2D0*(
*     +      - PI**2/3D0 + 2D0*DILO + 2D0*RLOG1**2 - 4D0*(1D0-E0)*RLOG1
*     +      + 4D0*(1D0-E0) + (1D0-E0)**2*( RLOG1 - .5D0 )
*     +      - 1.5D0*E0*RLOG2 + 1.5D0*E0 - .75D0*E0**2*( RLOG2-.5D0 )
*     +      + 3D0*RLOG1 + 2.5D0*E0 + .25D0*E0**2 - 3.5D0   )
* Now for the new one:
      RLOGP1 = LOG( 1D0 - EP0 )
      RLOGM1 = LOG( 1D0 - EM0 )
      RLOGP2 = 0D0
      RLOGM2 = 0D0
      IF(EP0.GT.0D0) RLOGP2 = LOG( EP0 )
      IF(EM0.GT.0D0) RLOGM2 = LOG( EM0 )
      FAC = 2D0*DILOG(1D0-EP0) + 2D0*DILOG(1D0-EM0) - PI*PI*2D0/3D0 +
     1      2D0*( RLOGP1 + RLOGM1 )**2 + 2D0*( RLOGP1 + RLOGM1 )*
     2      ( EP0*(1D0+.5D0*EP0) + EM0*(1D0+.5D0*EM0) ) + EP0 + EM0 +
     3      1D0/8D0*( EP0**2 + EM0**2 ) + EP0*EM0*(1D0+.5D0*EP0)*
     4      (1D0+.5D0*EM0) - 1.5D0*EP0*(1D0+.5D0*EP0)*RLOGP2 -
     5                       1.5D0*EM0*(1D0+.5D0*EM0)*RLOGM2
      FINAL2 = FAC * (BETALF/4D0)**2
      END
 
      SUBROUTINE GZPROP(QSQR,PROPG,PROPZ,MIXING)
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
      IF(IWEAK .EQ. 1) THEN
        Z1 = DCMPLX( SIGG (QSQR) , IMSIGG(QSQR) )
        Z2 = DCMPLX( SIGZ (QSQR) , IMSIGZ(QSQR) + IMZ2(QSQR) )
        Z3 = DCMPLX( SIGGZ(QSQR) , IMSGGZ(QSQR) )
        PROPG = 1D0/( QSQR + Z1 )
        PROPZ = 1D0/( QSQR - RMZ**2 + Z2 )
        MIXING= - Z3/( QSQR*(QSQR-RMZ**2+Z2) )
      ELSE
        PROPG  = 1D0/QSQR
        IF(QSQR .GT. 0D0) THEN
          PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , RMZ*ZWID )
        ELSE
          PROPZ  = 1D0/DCMPLX( QSQR-RMZ**2 , 0D0 )
        ENDIF
        MIXING = DCMPLX( 0D0 , 0D0 )
      ENDIF
      END
 
      FUNCTION EEEEVS(COSTH)
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
      COMPLEX*16 LABDAS,LABDAT,LABDFS,LABDFT,LE,LL,G,A, SPENCF,MZ2
      COMPLEX*16 GS,GT,ZS,ZT,MIX,GZS(-1:1,-1:1),GZT(-1:1,-1:1)
      COMPLEX*16 AGZS,VGZS,AGGS,VGGS,AGZT,VGZT,AGGT,VGGT,GDINT,HULP
* Statement functions for the box corrections
      G(SL,TL) = SL/2D0/(SL+TL)*LOG(TL/CMPLX(SL,OFFSET)) -
     +           SL*(SL+2D0*TL)/4D0/(SL+TL)**2*(
     +             LOG(TL/CMPLX(SL,OFFSET))**2 + PI*PI )
      A(SL,TL) = (SL-MZ2)/(SL+TL)*( LOG(TL/(SL-MZ2)) +
     +             MZ2/SL*LOG(1D0-SL/MZ2) + (SL+2D0*TL+MZ2)/(SL+TL)*(
     +               LOG(-TL/MZ2)*LOG((MZ2-SL)/(MZ2+TL)) +
     +               SPENCF(SL/MZ2) - SPENCF(-TL/MZ2) ) )
      IEVS=IEVS+1
*
      S = SCM
      PPQP = .25D0*S*( 1D0 - COSTH )
      PPQM = .25D0*S*( 1D0 + COSTH )
      PPPM =  .5D0*S
      T = - 2D0*PPQP
* Define propagators.
      E2   = EE**2
      QF2  = QF(1)*QF(IFERM)
      QF3  = QF(1)**2
      CALL GZPROP(S ,GS ,ZS ,MIX)
      CALL GZPROP(T ,GT ,ZT ,MIX)
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
      LABDAS = LABDAS + LABDFS - .75D0*( BETAL1 + BETALF )
      LABDAT = LABDAT + LABDFT - .75D0*( BETAL1 + BETALF )
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
      DELSOF = DELSOF - LOG(EPS2)*( BETALL + BETALF )
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
      VGZS = ALPI*( A(S,T) - A(S,U) + 2D0*SPENCF(1D0+MZ2/T) -
     +                                2D0*SPENCF(1D0+MZ2/U) +
     +                        4D0*LOG(SQRT(MZ2*SK)/(MZ2-S))*LOG(T/U) )
      AGGT = ALPI*( G(T,S) + G(T,U) )
      VGGT = ALPI*( G(T,S) - G(T,U) + 2D0*LOG(SK/CMPLX(-T,-OFFSET))*
     +                                    LOG(CMPLX(S,OFFSET)/U) )
      AGZT = ALPI*( A(T,S) + A(T,U) )
      VGZT = ALPI*( A(T,S) - A(T,U) + 2D0*SPENCF(1D0+MZ2/S) -
     +                                2D0*SPENCF(1D0+MZ2/U) +
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
      EEEEVS = HBARC2/8D0/2D0/PI/2D0/S/4D0 * SUM
      END
 
      FUNCTION RNONLG(X,WEIGHT)
*     ---------------
* Calculate the single photon bremsstrahlung matrix element minus
* the 4 collinear approximations of it.
* First we have to set up the momenta of all the particles involved,
* then check whether the resulting 'event' lies within our cuts, and
* if so, the matrix element can be computed. Next we call COLLIN
* four times to calculate the 4 collinear approximations.
* The integral over this combination should give us the hard photon
* non log corrections.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ EVENT  / PP(0:3),PM(0:3),QP(0:3),QM(0:3),SP,QK(0:3)
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ BHSTAT / ISIGH,ISIGF,ITWOH,IZBR,IRNON,IEVS,IEEW
      REAL*8 X(1:5)
      IRNON=IRNON+1
*     Construct the event.
      QK(0) = EXP( X(1) )
      COSK  = X(2)
      FIK   = X(3)
      COSQP = X(4)
      FIQP  = X(5)
      SINQP = SQRT(ABS( 1D0 - COSQP**2 ))
      SINK  = SQRT(ABS( 1D0 - COSK **2 ))
      SINFK = SIN( FIK  )
      COSFK = COS( FIK  )
      SINFP = SIN( FIQP )
      COSFP = COS( FIQP )
      SP    = 4D0*PP(0)*( PP(0) - QK(0) )
      QK(1) = QK(0)*SINK *COSFK
      QK(2) = QK(0)*SINK *SINFK
      QK(3) = QK(0)*COSK
      CQK   = COSQP
      I=1
      QP(0) = SP/( (4D0*PP(0)-2D0*QK(0))**2 - 4D0*QK(0)**2*CQK**2 )*
     +    ( 4D0*PP(0)-2D0*QK(0)*( 1D0+CQK*SQRT(ABS(1D0-RMASS2(I)/SP**2*
     +        ( (4D0*PP(0)-2D0*QK(0))**2 - 4D0*QK(0)**2*CQK**2 ) )) ) )
      IF(QP(0) .LT. RMASS(IFERM) .OR. QP(0).GT.PP(0)) THEN
        RNONLG = 0D0
        RETURN
      ENDIF
      QV    = SQRT(ABS( QP(0)*QP(0) - RMASS2(I) ))
      QP(1) = QV*SINQP*COSFP
      QP(2) = QV*SINQP*SINFP
      QP(3) = QV*COSQP
*     Have to rotate q+, for the angles were defined w.r.t. k
      THETA = ACOS( COSK )
      CALL ROTATE(QP,FIK,THETA)
*     q- from momentum conservation
      DO 10 J = 0 , 3
        QM(J) = PP(J) + PM(J) - QP(J) - QK(J)
   10 CONTINUE
*     so the mass of q- is a genuine check
      CHK = ABS(QM(0)**2-QM(1)**2-QM(2)**2-QM(3)**2-RMASS2(I))/PP(0)**2
*      CALL HISTO(10,1,CHK,1D-20,1D-10,WEIGHT,2,' ',6,10)
      IF(CHK.GT.1D-10.OR.QM(0).LT.RMASS(IFERM).OR.QM(0).GT.PP(0)) THEN
        RNONLG = 0D0
        RETURN
      ENDIF
*
*     We actually have an event now. Dot products:
      PPK = ABS( DOT(PP,QK) )
      PMK = ABS( DOT(PM,QK) )
      QPK = ABS( DOT(QP,QK) )
      QMK = ABS( DOT(QM,QK) )
*
*     Logs to recognise collinear situations.
      IF( PPK .LE. 0D0) THEN
        RLOG1 = 100D0
      ELSEIF( PMK .LE. 0D0) THEN
        RLOG1 = - 100D0
      ELSE
        RLOG1 = - LOG( PPK/PMK )
      ENDIF
      IF( QPK .LE. 0D0) THEN
        RLOG2 = 100D0
      ELSEIF( QMK .LE. 0D0) THEN
        RLOG2 = - 100D0
      ELSE
        RLOG2 = - LOG( QPK/QMK )
      ENDIF
      RMAX1 = 15D0
      RMAX2 = RMAX1
      COSP = QP(3)/QP(0)
      COSM = QM(3)/QM(0)
      COSPM= ( QP(1)*QM(1) + QP(2)*QM(2) + QP(3)*QM(3) )/QP(0)/QM(0)
      IF(COSPM .GT. 1D0) COSPM = 1D0
      IF(COSPM .LT.-1D0) COSPM =-1D0
      ACOL = ABS( PI - ACOS(COSPM) )
*
*     The actual calls to calculate matrix elements.
*     Check on cuts to calculate the matrix element. For the collinear
*     approximations the check on cuts is done separately, as the
*     event will be modified! In a collinear situation, leave out the
*     matrix element and the appropriate approximation, as they should
*     cancel exactly.
      RNON = 0D0
      IF(COSP.LT.C1.AND.COSP.GT.C2 .AND. QP(0).GT.EP0*PP(0) .AND.
     +   COSM.LT.C4.AND.COSM.GT.C3 .AND. QM(0).GT.EM0*PP(0) .AND.
     +   ACOL.LT.ACOLMX .AND.
     +   ABS(RLOG1).LT.RMAX1 .AND. ABS(RLOG2).LT.RMAX2 ) THEN
         RJACOB = QP(0)/ABS(4D0*PP(0)-2D0*QK(0)*(1D0-CQK))*QK(0)**2
         RNON = RNON + EEEEG(PPK,PMK,QPK,QMK) * RJACOB
      ENDIF
      IF( IORDER.NE.0 .OR. ICHANN.NE.1 ) THEN
        IF(RLOG1.LT. RMAX1) RNON = RNON - COLLIN(PPK,1)
        IF(RLOG1.GT.-RMAX1) RNON = RNON - COLLIN(PMK,2)
      ENDIF
      IF( IFINAL.NE.0 .OR. ICHANN.NE.1 ) THEN
        IF(RLOG2.LT. RMAX2) RNON = RNON - COLLIN(QPK,3)
        IF(RLOG2.GT.-RMAX2) RNON = RNON - COLLIN(QMK,4)
      ENDIF
*
*     Include flux and phase space factors and conversion to picobarn
      RNON = HBARC2 * RNON /4D0/( 2D0*SCM*(2D0*PI)**5 )
*
*     Fill histo's. Can be left out.
*      COSQPK = 1D0 - QPK/QP(0)/QK(0)
*      COSQMK = 1D0 - QMK/QM(0)/QK(0)
*      COSQPM = 1D0 - DOT(QP,QM)/QM(0)/QP(0)
*      W = WEIGHT*RNON
*      CALL HISTO(1,1,QK(0)/PP(0),EPS2,1D0,W,2,' ',6,12)
*      CALL HISTO(2,1,QP(3)/QP(0),  C2, C1,W,1,' ',6,10)
*      CALL HISTO(3,1,QM(3)/QM(0), -C1,-C2,W,1,' ',6,10)
*      CALL HISTO(4,1,QK(3)/QK(0),-1D0,1D0,W,1,' ',6,10)
*      CALL HISTO(5,1,  COSQPK   ,-1D0,1D0,W,1,' ',6,10)
*      CALL HISTO(6,1,  COSQMK   ,-1D0,1D0,W,1,' ',6,10)
*      CALL HISTO(7,1,  COSQPM   ,-1D0,1D0,W,1,' ',6,10)
*      CALL HISTO(8,1,QP(0),RMASS(1),PP(0),W,1,' ',6,10)
*      CALL HISTO(9,1,QM(0),RMASS(1),PP(0),W,1,' ',6,10)
*
      RNONLG = RNON
      END
 
      FUNCTION DOT(P,Q)
*     ------------
      IMPLICIT REAL*8(A-H,O-Z)
      REAL*8 P(0:3),Q(0:3)
      DOT = P(0)*Q(0) - P(1)*Q(1) - P(2)*Q(2) - P(3)*Q(3)
      END
 
      FUNCTION EEEE(P1,P2,Q1,Q2)
*     -------------
* The Born e+(p1) e-(p2) --> e+(q1) e-(q2) matrix element squared,
* including both gamma and Z in both s and t channel. Summing/averaging
* over spins is performed. The four momenta p1,...,q2 are input.
* W. Beenakker and S.C. van der Marck, April 1990.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      REAL*8 P1(0:3),P2(0:3),Q1(0:3),Q2(0:3),MATRIX(1:6)
      COMPLEX*16 GS,GT,ZS,ZT,MIX,GZS(-1:1,-1:1),GZT(-1:1,-1:1)
      PPQP = DOT(P1,Q1)
      PPQM = DOT(P1,Q2)
      PPPM = DOT(P1,P2)
      S    =   2D0*PPPM
      T    = - 2D0*PPQP
      E2   = EE**2
      QF2  = QF(1)*QF(IFERM)
      QF3  = QF(1)**2
      CALL GZPROP(S ,GS ,ZS ,MIX)
      CALL GZPROP(T ,GT ,ZT ,MIX)
      IF(ICHANN .EQ. 1) THEN
        ZT = (0D0,0D0)
        GT = (0D0,0D0)
      ELSEIF(ICHANN .EQ. 2) THEN
        ZS = (0D0,0D0)
        GS = (0D0,0D0)
      ENDIF
      I=IFERM
      DO 50 LE = - 1 , 1 , 2
        DO 40 LF = - 1 , 1 , 2
          GZS(LE,LF)=E2*(QF2*GS+(VF(1)-LE*AF(1))*(VF(I)-LF*AF(I))*ZS)
          GZT(LE,LF)=E2*(QF3*GT+(VF(1)-LE*AF(1))*(VF(1)-LF*AF(1))*ZT)
   40   CONTINUE
   50 CONTINUE
      MATRIX(1) = 16D0*ABS(GZS( 1, 1)+GZT( 1, 1))**2*PPQM**2
      MATRIX(2) = 16D0*ABS(GZS(-1,-1)+GZT(-1,-1))**2*PPQM**2
      MATRIX(3) = 16D0*ABS(GZS( 1,-1))**2*PPQP**2
      MATRIX(4) = 16D0*ABS(GZS(-1, 1))**2*PPQP**2
      MATRIX(5) = 16D0*ABS(GZT( 1,-1))**2*PPPM**2
      MATRIX(6) = 16D0*ABS(GZT(-1, 1))**2*PPPM**2
      EEEE = .25D0*( MATRIX(1) + MATRIX(2) + MATRIX(3) +
     +               MATRIX(4) + MATRIX(5) + MATRIX(6) )
      END
 
      FUNCTION EEEEW(COSTH)
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
      IEEW=IEEW+1
      S = SCM
      PPQP = .25D0*S*( 1D0 - COSTH )
      PPQM = .25D0*S*( 1D0 + COSTH )
      PPPM =  .5D0*S
      T    = - 2D0*PPQP
* Define propagators, and include vertex form factors.
      E2   = 4D0*PI*ALFA
      CALL GZPROP(S,GS,ZS,MIXS)
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
      CALL GZPROP(T,GT ,ZT ,MIXT)
      CALL FORMFS(T,1)
*     Incorporate the Burkhardt fit for the light quark loops.
      GT = GT/( 1D0 - HADRQQ(T) - PHADPI(T) )
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
      EEEEW = HBARC2/8D0/2D0/PI/2D0/S/4D0 * SUM
      END
 
      SUBROUTINE HEAVYB(S,T,VZZ,AZZ,VWW,AWW)
*     -----------------
* Subroutine giving the 'couplings' with which to contract the
* ZZ and WW boxes with the Born matrix element.
* S,T are input and VZZ,AZZ,VWW,AWW are complex*16 output.
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMPLEX*16 VZZ,AZZ,VWW,AWW,RI5ST,RIST,RI5SU,RISU
      U = - S - T
      CALL HEAVYI(S,T,RMZ,RIST,RI5ST)
      CALL HEAVYI(S,U,RMZ,RISU,RI5SU)
      VZZ = ALFA/2D0/PI*( RIST  - RISU  )
      AZZ = ALFA/2D0/PI*( RI5ST + RI5SU )
*     WW boxes depend strongly on the isospin of the produced fermion
      IF(IFERM.EQ.0.OR.IFERM.EQ.4.OR.IFERM.EQ.6.OR.IFERM.EQ.8) THEN
*       isospin = + 1/2
        CALL HEAVYI(S,U,RMW,RISU,RI5SU)
        VWW = ALFA/2D0/PI*( - RISU  )
        AWW = ALFA/2D0/PI*( + RI5SU )
      ELSE
*       isospin = - 1/2
        CALL HEAVYI(S,T,RMW,RIST,RI5ST)
        VWW = ALFA/2D0/PI*( + RIST  )
        AWW = ALFA/2D0/PI*( + RI5ST )
      ENDIF
* To get the normalization right
      E2 = EE**2
      VZZ = VZZ * E2/S
      AZZ = AZZ * E2/S
      VWW = VWW * E2/S/4D0/SIN2TH**2
      AWW = AWW * E2/S/4D0/SIN2TH**2
      END
 
      SUBROUTINE HEAVYI(S,T,RM,RI,RI5)
*     -----------------
* Function needed to calculate ZZ or WW boxes.
* S,T,RM are input, RI,RI5 are complex*16 output.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( EPS = 1D-10 )
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMPLEX*16 RI,RI5,SPENCF,ROOT1,ROOT2,X1,X2,Y1,Y2,FOURSP,RLOG12
      COMPLEX*16 SHELP,THELP,I,X1X2
      IF ( S.GT.0D0 . AND. T.GT.0D0 ) THEN
        WRITE(*,'(A)')' HEAVYI: both S and T > 0.  This is not valid!'
        RI  = (0D0,0D0)
        RI5 = (0D0,0D0)
        RETURN
      ENDIF
      RM2 = RM**2
      IF( S .GT. 0D0 ) THEN
        SHELP = 4D0*RM2/CMPLX(S,EPS)
      ELSE
        SHELP = 4D0*RM2/S
      ENDIF
      IF( T .GT. 0D0 ) THEN
        THELP = RM2/CMPLX(T,EPS)
      ELSE
        THELP = RM2/T
      ENDIF
      ROOT1 = SQRT( (1D0,0D0)-SHELP )
      IF(S.LT.0D0.AND.T.LT.0D0.AND.4D0*RM2/S*(1D0+RM2/T).GT.1D0) THEN
        I = (0D0,1D0)
        ROOT2 = I*SQRT( -( (1D0,0D0)-SHELP*( (1D0,0D0) + THELP ) ) )
      ELSE
        ROOT2 =   SQRT(    (1D0,0D0)-SHELP*( (1D0,0D0) + THELP )   )
      ENDIF
      Y1 = .5D0*( 1D0 + ROOT1 )
      Y2 = .5D0*( 1D0 - ROOT1 )
      X1 = .5D0*( 1D0 + ROOT2 )
      X2 = .5D0*( 1D0 - ROOT2 )
      X1X2 = ROOT2
      FOURSP = SPENCF(X1/(X1-Y1)) + SPENCF(X1/(X1-Y2)) -
     +         SPENCF(X2/(X2-Y2)) - SPENCF(X2/(X2-Y1))
      RLOG12 = LOG(-Y1/Y2)
      IF( ABS(X1X2) .LT. 10D0*EPS ) THEN
        X1X2 = (1D0,0D0)
        FOURSP = 1D0/(Y1-Y2)*( - 4D0*Y1*LOG(2D0*Y1/(Y1-Y2))
     +                         + 4D0*Y2*LOG(2D0*Y2/(Y2-Y1)) )
      ENDIF
      RI5 = (2D0*T+S+2D0*RM2)/(2D0*(S+T))*(
     +      SPENCF( 1D0+CMPLX(T,EPS)/RM2 ) - PI*PI/6D0 - RLOG12**2 ) +
     +      .5D0*LOG(-CMPLX(T,EPS)/RM2) + (Y2-Y1)/2D0*RLOG12 +
     +      ( S+2D0*T - 4D0*T*RM2/S + 2D0*RM2**2/T - 2D0*RM2**2/S )/
     +      ( 2D0*( S + T )*(-X1X2) ) * FOURSP
      RI5 = S/( S + T ) * RI5
      RI  = RI5 + 2D0*RLOG12**2 + 2D0/X1X2*FOURSP
      END
 
      FUNCTION COLLIN(DOTPK,I1)
*     ---------------
* Compute the collinear approximation to the process
* e+e- --> e+e-gamma. The matrix element squared for e+e- --> e+e-
* is supposed to be given by EEEE. I1 indicates which particle the
* photon is supposed to be 'collinear' with: 1=p+, 2=p-, 3=q+, 4=q-.
* DOTPK is the dot product of this particle and the photon.
* I1 is input, DOTPK is input, but gets changed.     March 1990.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      COMMON/ CONTRL / C1,C2,C3,C4,EP0,EM0,ACOLMX,SCM,BETALL,BETALF,
     +                 EPS,EPS2
      COMMON/ EVENT  / PP(0:3),PM(0:3),QP(0:3),QM(0:3),SP,QK(0:3)
      REAL*8 PMINK(0:3),Q1(0:3),Q2(0:3)
* Project the photon onto the particle momentum it is supposed to
* be collinear with. Adjust the rest of the event to balance
* energy and momentum.
      ICUT = 0
      IF(I1 .LE. 2) THEN
        X = QK(0)/PP(0)
        PDOTK = DOTPK
        QV2   = SQRT(QP(1)*QP(1)+QP(2)*QP(2)+QP(3)*QP(3))
        CQK   = QP(3)/QV2
        IF(I1 .EQ. 2) CQK = - CQK
        Q1(0) = SP/( 4D0*PP(0)-2D0*QK(0)*( 1D0 - CQK ) )
        QV1   = Q1(0)
        Q1(1) = QV1/QV2*QP(1)
        Q1(2) = QV1/QV2*QP(2)
        Q1(3) = QV1/QV2*QP(3)
        IF (I1 .EQ. 1) THEN
          DO 20 I = 0 , 3
            PMINK(I) = ( 1D0 - X )*PP(I)
            Q2(I) = PMINK(I) + PM(I) - Q1(I)
   20     CONTINUE
        ELSE
          DO 30 I = 0 , 3
            PMINK(I) = ( 1D0 - X )*PM(I)
            Q2(I) = PP(I) + PMINK(I) - Q1(I)
   30     CONTINUE
        ENDIF
        COSQP = Q1(3)/Q1(0)
        COSQM = Q2(3)/Q2(0)
        COSPM = ( Q1(1)*Q2(1) + Q1(2)*Q2(2) + Q1(3)*Q2(3) )/Q1(0)/Q2(0)
        IF(COSPM .GT. 1D0) COSPM = 1D0
        IF(COSPM .LT.-1D0) COSPM =-1D0
        ACOL = ABS( PI - ACOS(COSPM) )
        IF(COSQP.LT.C1.AND.COSQP.GT.C2 .AND. Q1(0).GT.EP0*PP(0) .AND.
     +     COSQM.LT.C4.AND.COSQM.GT.C3 .AND. Q2(0).GT.EM0*PP(0) .AND.
     +     ACOL .LT. ACOLMX ) ICUT = 1
        ROOT = Q1(3)/Q1(0)
        IF(I1.EQ.2) ROOT = - ROOT
        RJACOB = ABS(Q1(0)/(4D0*PP(0)-2D0*QK(0)*(1D0-ROOT))) *QK(0)**2
      ELSEIF(I1 .EQ. 3) THEN
        X = QK(0)/(QP(0)+QK(0))
        IF( (1D0-X)*PP(0) .LT. RMASS(IFERM) .OR.X.LT.EPS2) THEN
          COLLIN = 0D0
          RETURN
        ENDIF
        PMINK(0) = PP(0)
        FAC = PP(3)/PP(0)
        QV2 = SQRT(QP(1)*QP(1)+QP(2)*QP(2)+QP(3)*QP(3))
        Q2(0) = PP(0)
        DO 40 I = 1 , 3
          PMINK (I) = PP(0)/QV2*QP(I)
          Q2    (I) = - PMINK(I)
   40   CONTINUE
        COSTH = ( QP(1)*QK(1) + QP(2)*QK(2) + QP(3)*QK(3) )/QK(0)/QV2
        QV2 = SQRT(QP(0)**2-RMASS2(IFERM))
        PDOTK = X*(1D0-X)*PP(0)**2*( 1D0 - COSTH*FAC )
        DOTPK = X*(1D0-X)*PP(0)**2*( 1D0 - COSTH*QV2/QP(0) )
        COSQP = QP(3)/QP(0)
        COSQM = - COSQP
        IF( COSQP.LT.C1.AND.COSQP.GT.C2 .AND.
     +      COSQM.LT.C4.AND.COSQM.GT.C3 .AND. 1D0-X.GT.EP0 ) ICUT = 1
        RJACOB = (1D0-X)/4D0 * X**2*PP(0)**2
      ELSEIF(I1 .EQ. 4) THEN
        X = QK(0)/(QM(0)+QK(0))
        IF( (1D0-X)*PP(0) .LT. RMASS(IFERM) .OR.X.LT.EPS2) THEN
          COLLIN = 0D0
          RETURN
        ENDIF
        PMINK(0) = PP(0)
        FAC = PP(3)/PP(0)
        QV1 = SQRT(QM(1)*QM(1)+QM(2)*QM(2)+QM(3)*QM(3))
        QV2 = SQRT(QP(1)*QP(1)+QP(2)*QP(2)+QP(3)*QP(3))
        Q1(0) = PP(0)
        DO 50 I = 1 , 3
          Q1   (I) = PP(0)/QV2*QP(I)
          PMINK(I) = - Q1(I)
   50   CONTINUE
        COSTH = ( QP(1)*QK(1) + QP(2)*QK(2) + QP(3)*QK(3) )/QK(0)/QV2
        COST1 = ( QM(1)*QK(1) + QM(2)*QK(2) + QM(3)*QK(3) )/QK(0)/QV1
        QV2 = SQRT(QM(0)**2-RMASS2(IFERM))
        PDOTK = X*(1D0-X)*PP(0)**2*( 1D0 + COSTH*FAC )
        DOTPK = X*(1D0-X)*PP(0)**2*( 1D0 - COST1*QV2/QM(0) )
        COSQP = QP(3)/QP(0)
        COSQM = - COSQP
        IF( COSQP.LT.C1.AND.COSQP.GT.C2 .AND.
     +      COSQM.LT.C4.AND.COSQM.GT.C3 .AND. 1D0-X.GT.EM0 ) ICUT = 1
        RJACOB = (1D0-X)/4D0 * X**2*PP(0)**2
      ELSE
        STOP' COLLIN: I1 error! Fatal!'
      ENDIF
*
      IF(ICUT .EQ. 1) THEN
*       Compute the matrix element e+e- ---> e+e-
        IF(I1 .EQ. 1) THEN
          RM4 = EEEE(PMINK,PM,Q1,Q2)
        ELSEIF(I1 .EQ. 2) THEN
          RM4 = EEEE(PP,PMINK,Q1,Q2)
        ELSEIF(I1 .EQ. 3) THEN
          RM4 = EEEE(PP,PM,PMINK,Q2) *(1D0-X)*QF(IFERM)**2
        ELSE
          RM4 = EEEE(PP,PM,Q1,PMINK) *(1D0-X)*QF(IFERM)**2
        ENDIF
        RM4 = RM4*RJACOB
      ELSE
*       The modified momenta didn't pass the cuts.
        RM4 = 0D0
      ENDIF
*     Now relate this to the result for e+e- ---> e+e-gamma
      COLFAC = EE**2*( ( 1D0 + (1D0-X)**2 )/X/(1D0-X)/PDOTK )
      IF(I1.LT.3.AND.IFERM.NE.1) THEN
        COLFAC = COLFAC + EE**2*( RMASS2(1)*X/DOTPK**2 )
      ELSEIF(IFERM.NE.1) THEN
        COLFAC = COLFAC + EE**2*(RMASS2(IFERM)*X/DOTPK**2)
      ENDIF
      COLLIN = COLFAC*RM4
      END
 
      FUNCTION EEEEG(PPK,PMK,QPK,QMK)
*     --------------
* Matrix element squared for e+e- --> e+e-gamma, including
* initial and final state bremsstrahlung on the s and t channel.
* As input is needed the common EVENT and the dotproducts PPK,etc.
* Output is EEEEG = spin averaged and summed matrix element squared.
      IMPLICIT REAL*8(A-H,O-Y),COMPLEX*16(Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ EVENT  / PP(0:3),PM(0:3),QP(0:3),QM(0:3),SP,QK(0:3)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ OPTION / IWEAK,ICHANN,IOUT,IORDER,IFINAL,NONLOG,IFERM
      PARAMETER( IPP=1, IPM=2, IQ1=3, IP=4, IM=5, IBIGQ1=5, IBIGQ2=1 )
      REAL*8 QQ(0:3,5), DD(5,5)
      COMPLEX*16 Z(-1:1,5,5),GS,GSP,GT,GTP,ZS,ZSP,ZT,ZTP,MIX
      COMPLEX*16 GZS(-1:1,-1:1),GZSP(-1:1,-1:1)
      COMPLEX*16 GZT(-1:1,-1:1),GZTP(-1:1,-1:1)
* Statement functions
      ZINI(IQP,IQM,IK,IQ,L)=- EE*ROOT2/DFLOAT(L)/Z(-L,IQ1,IBIGQ1)*(
     1  Z(1,IPP,IQM)*(  Z(-1,IQP,IPM)*Z(1,IPM, IK)
     2                - Z(-1,IQP,IQ1)*Z(1,IQ1, IK) )*Z(-1, IQ,IPM)/PMK
     3 +Z(1,IPP, IK)*(- Z(-1, IQ,IPP)*Z(1,IPP,IQM)
     4                + Z(-1, IQ,IQ1)*Z(1,IQ1,IQM) )*Z(-1,IQP,IPM)/PPK)
      ZFIN(IEP,IEM,IK,IQ,L)=- QF(IFERM)*EE*ROOT2/DFLOAT(L)/
     1  Z(-L,IQ1,IBIGQ2)*(Z(1, IM,IEP)*(- Z(-1,IEM, IP)*Z(1, IP, IK)
     2                - Z(-1,IEM,IQ1)*Z(1,IQ1, IK) )*Z(-1, IQ, IP)/QPK
     3 +Z(1, IM, IK)*(  Z(-1, IQ, IM)*Z(1, IM,IEP)
     4                + Z(-1, IQ,IQ1)*Z(1,IQ1,IEP) )*Z(-1,IEM, IP)/QMK)
      ZUPP(IFP,IFM,IK,IQ,L)= EE*ROOT2/DFLOAT(L)/Z(-L,IQ1,IBIGQ1)*(
     1  Z(1,IPP,IFM)*(  Z(-1,IFP, IP)*Z(1, IP, IK)
     2                + Z(-1,IFP,IQ1)*Z(1,IQ1, IK) )*Z(-1, IQ, IP)/QPK
     3 +Z(1,IPP, IK)*(- Z(-1, IQ,IPP)*Z(1,IPP,IFM)
     4                + Z(-1, IQ,IQ1)*Z(1,IQ1,IFM) )*Z(-1,IFP, IP)/PPK)
      ZLOW(IEP,IEM,IK,IQ,L)=- EE*ROOT2/DFLOAT(L)/Z(-L,IQ1,IBIGQ2)*(
     1  Z(1, IM,IEP)*(- Z(-1,IEM,IPM)*Z(1,IPM, IK)
     2                + Z(-1,IEM,IQ1)*Z(1,IQ1, IK) )*Z(-1, IQ,IPM)/PMK
     3 +Z(1, IM, IK)*(  Z(-1, IQ, IM)*Z(1, IM,IEP)
     4                + Z(-1, IQ,IQ1)*Z(1,IQ1,IEP) )*Z(-1,IEM,IPM)/QMK)
*
* Define spinor products and dot products
      DO 10 I=0,3
        QQ(I,1)=PP(I)
        QQ(I,2)=PM(I)
        QQ(I,3)=QK(I)
        QQ(I,4)=QP(I)
        QQ(I,5)=QM(I)
   10 CONTINUE
      DO 30   I=  1,5
        DO 20 J=I+1,5
          ROOT = SQRT( ( QQ(0,J)-QQ(1,J) )/( QQ(0,I)-QQ(1,I) ) )
          Z( 1,I,J) = CMPLX( QQ(2,I) , QQ(3,I) )*ROOT
     +              - CMPLX( QQ(2,J) , QQ(3,J) )/ROOT
          Z(-1,I,J) = - CONJG( Z(1,I,J) )
          Z( 1,J,I) = - Z( 1,I,J)
          Z(-1,J,I) = - Z(-1,I,J)
          DD(  I,J) = - Z( 1,I,J)*Z(-1,I,J)/2D0
          DD(  J,I) =   DD(I,J)
   20   CONTINUE
        Z( 1,I,I) = CMPLX(0D0,0D0)
        Z(-1,I,I) = CMPLX(0D0,0D0)
        DD(  I,I) = 0D0
   30 CONTINUE
      ROOT2 = SQRT( 2D0 )
* Calculate gamma,Z propagators and appropriate couplings.
      S = 4D0*PP(0)**2
      T = - 2D0*DD(1,4)
      TP= - 2D0*DD(2,5)
      E2= EE**2
      QF2 = QF(1)*QF(IFERM)
      QF3 = QF(1)**2
      CALL GZPROP(S ,GS ,ZS ,MIX)
      CALL GZPROP(SP,GSP,ZSP,MIX)
      CALL GZPROP(T ,GT ,ZT ,MIX)
      CALL GZPROP(TP,GTP,ZTP,MIX)
      IF(ICHANN .EQ. 1) THEN
        GT  = (0D0,0D0)
        GTP = (0D0,0D0)
        ZT  = (0D0,0D0)
        ZTP = (0D0,0D0)
      ELSEIF(ICHANN .EQ. 2) THEN
        GS  = (0D0,0D0)
        GSP = (0D0,0D0)
        ZS  = (0D0,0D0)
        ZSP = (0D0,0D0)
      ENDIF
      I=IFERM
      DO 50 LE = - 1 , 1 , 2
       DO 40 LF = - 1 , 1 , 2
         GZS (LE,LF)=E2*(QF2*GS +(VF(1)-LE*AF(1))*(VF(I)-LF*AF(I))*ZS )
         GZSP(LE,LF)=E2*(QF2*GSP+(VF(1)-LE*AF(1))*(VF(I)-LF*AF(I))*ZSP)
         GZT (LE,LF)=E2*(QF3*GT +(VF(1)-LE*AF(1))*(VF(1)-LF*AF(1))*ZT )
         GZTP(LE,LF)=E2*(QF3*GTP+(VF(1)-LE*AF(1))*(VF(1)-LF*AF(1))*ZTP)
   40  CONTINUE
   50 CONTINUE
* Calculate the fermion currents we need
      IF ( IORDER.EQ.0 .AND. ICHANN.EQ.1 ) THEN
        ZINI1 = (0D0,0D0)
        ZINI2 = (0D0,0D0)
        ZINI3 = (0D0,0D0)
        ZINI4 = (0D0,0D0)
      ELSE
        ZINI1 = ZINI( IP, IM,IQ1,IBIGQ1, 1)
        ZINI2 = ZINI( IP, IM,IBIGQ1,IQ1,-1)
        ZINI3 = ZINI( IM, IP,IQ1,IBIGQ1, 1)
        ZINI4 = ZINI( IM, IP,IBIGQ1,IQ1,-1)
      ENDIF
      IF ( IFINAL.EQ.0 .AND. ICHANN.EQ.1 ) THEN
        ZFIN1 = (0D0,0D0)
        ZFIN2 = (0D0,0D0)
        ZFIN3 = (0D0,0D0)
        ZFIN4 = (0D0,0D0)
      ELSE
        ZFIN1 = ZFIN(IPP,IPM,IQ1,IBIGQ2, 1)
        ZFIN2 = ZFIN(IPP,IPM,IBIGQ2,IQ1,-1)
        ZFIN3 = ZFIN(IPM,IPP,IBIGQ2,IQ1,-1)
        ZFIN4 = ZFIN(IPM,IPP,IQ1,IBIGQ2, 1)
      ENDIF
      IF ( ICHANN .NE. 1 ) THEN
        ZUPP1 = ZUPP(IPM, IM,IQ1,IBIGQ1, 1)
        ZUPP2 = ZUPP(IPM, IM,IBIGQ1,IQ1,-1)
        ZUPP3 = ZUPP( IM,IPM,IQ1,IBIGQ1, 1)
        ZUPP4 = ZUPP( IM,IPM,IBIGQ1,IQ1,-1)
        ZLOW1 = ZLOW(IPP, IP,IQ1,IBIGQ2, 1)
        ZLOW2 = ZLOW(IPP, IP,IBIGQ2,IQ1,-1)
        ZLOW3 = ZLOW( IP,IPP,IBIGQ2,IQ1,-1)
        ZLOW4 = ZLOW( IP,IPP,IQ1,IBIGQ2, 1)
      ELSE
        ZUPP1 = (0D0,0D0)
        ZUPP2 = (0D0,0D0)
        ZUPP3 = (0D0,0D0)
        ZUPP4 = (0D0,0D0)
        ZLOW1 = (0D0,0D0)
        ZLOW2 = (0D0,0D0)
        ZLOW3 = (0D0,0D0)
        ZLOW4 = (0D0,0D0)
      ENDIF
* and construct helicity amplitudes
* First s channel contributions
      ZPPP =       ZINI1 *GZSP( 1, 1)+      ZFIN1 *GZS( 1, 1)
      ZPPM =       ZINI2 *GZSP( 1, 1)+      ZFIN2 *GZS( 1, 1)
      ZPMP =       ZINI3 *GZSP( 1,-1)+CONJG(ZFIN3)*GZS( 1,-1)
      ZPMM =       ZINI4 *GZSP( 1,-1)+CONJG(ZFIN4)*GZS( 1,-1)
      ZMMM = CONJG(ZINI1)*GZSP(-1,-1)+CONJG(ZFIN1)*GZS(-1,-1)
      ZMMP = CONJG(ZINI2)*GZSP(-1,-1)+CONJG(ZFIN2)*GZS(-1,-1)
      ZMPM = CONJG(ZINI3)*GZSP(-1, 1)+      ZFIN3 *GZS(-1, 1)
      ZMPP = CONJG(ZINI4)*GZSP(-1, 1)+      ZFIN4 *GZS(-1, 1)
* and then t channel ones
      ZTPPP=       ZUPP1 *GZTP( 1, 1)+      ZLOW1 *GZT( 1, 1)
      ZTPPM=       ZUPP2 *GZTP( 1, 1)+      ZLOW2 *GZT( 1, 1)
      ZTPMP=       ZUPP3 *GZTP( 1,-1)+CONJG(ZLOW3)*GZT( 1,-1)
      ZTPMM=       ZUPP4 *GZTP( 1,-1)+CONJG(ZLOW4)*GZT( 1,-1)
      ZTMMM= CONJG(ZUPP1)*GZTP(-1,-1)+CONJG(ZLOW1)*GZT(-1,-1)
      ZTMMP= CONJG(ZUPP2)*GZTP(-1,-1)+CONJG(ZLOW2)*GZT(-1,-1)
      ZTMPM= CONJG(ZUPP3)*GZTP(-1, 1)+      ZLOW3 *GZT(-1, 1)
      ZTMPP= CONJG(ZUPP4)*GZTP(-1, 1)+      ZLOW4 *GZT(-1, 1)
* Some of these have the same helicity combination: those where all
* fermions have equal helicity.
      ZPPP = ZPPP + ZTPPP
      ZPPM = ZPPM + ZTPPM
      ZMMP = ZMMP + ZTMMP
      ZMMM = ZMMM + ZTMMM
* Summing and averaging over helicities.
      EEEEG = .25D0*(  ZPPP*CONJG( ZPPP) +  ZPPM*CONJG( ZPPM)
     +              +  ZPMP*CONJG( ZPMP) +  ZPMM*CONJG( ZPMM)
     +              +  ZMMP*CONJG( ZMMP) +  ZMMM*CONJG( ZMMM)
     +              +  ZMPP*CONJG( ZMPP) +  ZMPM*CONJG( ZMPM)
     +              + ZTPMP*CONJG(ZTPMP) + ZTMPM*CONJG(ZTMPM)
     +              + ZTPMM*CONJG(ZTPMM) + ZTMPP*CONJG(ZTMPP) )
      IF(EEEEG .LT. 0D0) STOP'EEEEG less than zero !! Alarm !!'
      END
 
      SUBROUTINE ROTATE(VECTOR,ANGLE1,ANGLE2)
*     -----------------
      IMPLICIT REAL*8(A-H,O-Z)
      REAL*8 VECTOR(0:3),NEW(0:3)
*      Rotate VECTOR around the y-axis with ANGLE2
* Then rotate VECTOR around the z-axis with ANGLE1
      C1 = COS(ANGLE1)
      S1 = SIN(ANGLE1)
      C2 = COS(ANGLE2)
      S2 = SIN(ANGLE2)
      NEW(1) = C1*C2*VECTOR(1) - S1*VECTOR(2) + C1*S2*VECTOR(3)
      NEW(2) = S1*C2*VECTOR(1) + C1*VECTOR(2) + S1*S2*VECTOR(3)
      NEW(3) =-   S2*VECTOR(1)                +    C2*VECTOR(3)
      DO 1 I = 1 , 3
         VECTOR(I) = NEW(I)
    1 CONTINUE
      END
 
      SUBROUTINE HISTO(I,ISTAT,X,X0,X1,WEIGHT,LINLOG,TITLE,IUNIT,NX)
*     ----------------
* Steven van der Marck, April 2nd, 1990.
* I      = number of this particular histogram (must be >0, <N(=50))
* ISTAT  = 0   : clear the arrays for all  histo's
*          1   : fill  the arrays for histo nr. I
*          2   : print histogram nr. I to unit nr. IUNIT
*          3   : output the data to HISTO.GRA - to be used for a
*                'home made' graphics program.
*          4   : same as 3, but the whole histogram is divided by
*                the number of points.
*          5   : save all relevant information to a file HISTO.DAT
*          6   : read all relevant information from file HISTO.DAT
*          ELSE: rescale all  histo's by a factor X
* X      = x-value to be placed in a bin of histogram nr. I
*          If ISTAT=2 and LINLOG<>1, x = the number of decades (def=3).
* X0     = the minimum for x in histogram nr. I
* X1     = the maximum for x in histogram nr. I
* WEIGHT = If ISTAT=1: the weight assigned to this value of x.
*          If ISTAT=2: the number of divisions on the y-axis (def=20),
*                     should not be <5 or >80 (because of screenwidth).
* LINLOG = determines the scale of both axes of the histogram.
*          If ISTAT=1 it goes for the x axis, for ISTAT=2 the y axis.
*          For LINLOG=1 the axis is linear, ELSE logarithmic.
*          If a linear histo has only one peak that is too sharp,
*          this routine will automatically switch to a log. y axis.
* TITLE  = title of this particular histogram ( character*(*) )
* IUNIT  = unit number to write this histogram to
* NX     = the number of divisions on the x-axis for this histogram
*          NX should not be greater than NXMAX (=50 in this version).
*
* When ISTAT = 0   : IUNIT is used.
*            = 1   : I, X, X0, X1, WEIGHT, LINLOG and NX are used.
*            = 2   : I, X, WEIGHT, LINLOG, TITLE and IUNIT are used.
*            = 3,4 : I, LINLOG and TITLE are used. The user should not
*             be using unit nr 11 when invoking HISTO with this option!
*            = 5,6 : no other variables are used.
*            = ELSE: only X is used.
      IMPLICIT LOGICAL(A-Z)
      INTEGER N,NX,NXMAX,I,ISTAT,LINLOG,IUNIT
* N = the maximum number of histo's allowed; can be changed on its own.
* NXMAX = the maximum allowed for NX(= nr. of divisions on the x-axis)
      PARAMETER( N = 50 , NXMAX = 50 )
      INTEGER J,J1,J2,IX,IY,JUNIT,NYDIV, LINLOX(N), IWARN
      REAL*8 X,X0,X1,WEIGHT, Z,WEISUM,WEISU2,FACTOR
      REAL*8 Y(N,NXMAX), YMAX(N), BOUND0(N), BOUND1(N),
     +                 XMIN(N), XMAX(N), YSQUAR(N,NXMAX), YOUT(N)
      INTEGER IUNDER(N), IIN(N), IOVER(N), NRBINS(N), IBIN(N,NXMAX)
      CHARACTER TITLE*(*),LINE(132),BLANK,STAR,TEXT*12
      CHARACTER FORM1*80,F*80,STRP*1,FH*30,F2(1:3)*12
      SAVE
      DATA STRP/'-'/F2/'(1X,4I10)','(1X,10I10)','(1X,10D12.5)'/
      DATA BLANK/' '/STAR/'*'/FH/'(A),''I'',3X,G11.4,2X,G11.4,I12)'/
*     Is this a valid histo nr.? (Not necessary if ISTAT=0.)
      IF( (I.LT.1.OR.I.GT.N) .AND. (ISTAT.NE.0) ) GOTO 910
*     ISTAT decides on several MUTUALLY EXCLUSIVE branches.
      IF(ISTAT .EQ. 0) THEN
*       Zero everything you've got.
        DO 20 J1 = 1 , N
          BOUND0(J1) = 0D0
          BOUND1(J1) = 0D0
          XMIN  (J1) = 0D0
          XMAX  (J1) = 0D0
          YMAX  (J1) = 0D0
          YOUT  (J1) = 0D0
          IUNDER(J1) = 0
          IIN   (J1) = 0
          IOVER (J1) = 0
          NRBINS(J1) = 0
          DO 10 J2 = 1 , NXMAX
            Y     (J1,J2) = 0D0
            YSQUAR(J1,J2) = 0D0
            IBIN  (J1,J2) = 0
   10     CONTINUE
   20   CONTINUE
        IWARN = 0
*       Finished ... - exit !
      ELSEIF(ISTAT.EQ.1) THEN
*       Fill arrays with this x value
        IF(NRBINS(I) .EQ. 0) THEN
*         First time around: remember boundaries, lin or log x-scale,
*         and the number of divisions on the x axis.
          BOUND0(I) = X0
          BOUND1(I) = X1
          LINLOX(I) = LINLOG
          IF(LINLOX(I) .NE. 1) THEN
            IF(BOUND0(I)*BOUND1(I) .GT. 0D0) THEN
              BOUND0(I) = LOG(ABS( BOUND0(I) ))
              BOUND1(I) = LOG(ABS( BOUND1(I) ))
            ELSE
              LINLOX(I) = 1
            ENDIF
          ENDIF
          NRBINS(I) = NXMAX
          IF(NX.GT.0 .AND. NX.LE.NXMAX) NRBINS(I) = NX
        ENDIF
        IF(LINLOX(I) .NE. 1) THEN
          IF(ABS(X) .GT. 0D0) THEN
            Z = LOG(ABS(X))
          ELSE
            Z = BOUND0(I) - 1D-10
          ENDIF
        ELSE
          Z = X
        ENDIF
*       Does this x value lie within the boundaries? Update statistics!
        IF(Z.LT.BOUND0(I))THEN
          IUNDER(I) = IUNDER(I) + 1
          YOUT  (I) = YOUT  (I) + WEIGHT
          IF(Z.LT.XMIN(I).OR.IUNDER(I).EQ.1) XMIN(I) = Z
        ELSEIF(Z.GT.BOUND1(I))THEN
          IOVER(I) = IOVER(I) + 1
          YOUT (I) = YOUT (I) + WEIGHT
          IF(Z.GT.XMAX(I).OR. IOVER(I).EQ.1) XMAX(I) = Z
        ELSE
          IIN(I) = IIN(I) + 1
          IX = INT((Z-BOUND0(I))/(BOUND1(I)-BOUND0(I))*NRBINS(I))+1
          IF(IX.EQ.NRBINS(I)+1) IX = NRBINS(I)
          IBIN  (I,IX) = IBIN  (I,IX) + 1
          Y     (I,IX) = Y     (I,IX) + WEIGHT
          YSQUAR(I,IX) = YSQUAR(I,IX) + WEIGHT**2
          IF(Y(I,IX).GT.YMAX(I)) YMAX(I) = Y(I,IX)
        ENDIF
*       Finished ... - exit !
      ELSEIF(ISTAT .EQ. 2) THEN
*       Print histogram. First a header.
        WRITE(IUNIT,'(//,A,I2,A,I10,A,I8,A,I8)')' Histogram nr.',I,
     +  '  Points in:',IIN(I),'  under:',IUNDER(I),'  over:',IOVER(I)
        WRITE(IUNIT,*)' ',TITLE
*       Leave if all entries have equal 0 weight.
        IF(ABS(YMAX(I)).LE.0D0) GOTO 920
*       Determine the number of divisions on the y axis.
        NYDIV = INT(WEIGHT)
        IF(NYDIV .LT. 5 .OR. NYDIV .GT. 80) NYDIV = 20
*       Determine lin/log scale y axis.
        IF(LINLOG .EQ. 1) THEN
*         Count the number of entries that will show up in a lin scale,
*         for if they are too few, make it a log one.
          IX = 0
          DO 30 J1 = 1 , NRBINS(I)
            IF(Y(I,J1)/YMAX(I)*NYDIV .GT. 1D0) IX = IX + 1
   30     CONTINUE
        ENDIF
        IF(IX .LE. 2 .OR. LINLOG .NE. 1) THEN
          IX = 2
          FACTOR = 1D3/YMAX(I)
          IF(X.GE.1D0 .AND. X.LE.10D0) FACTOR = 10D0**X/YMAX(I)
        ELSE
          IX = 1
        ENDIF
*       Prepare the formats (they depend on #divisions in y)
        WRITE(FORM1,'(A,I3,A)') '('' '',G11.4,1X,',NYDIV,'(A),A)'
        WRITE(F,'(A,I3,A)') '('' '',A12,',NYDIV,FH
        Z = BOUND0(I)
        IF(LINLOX(I) .NE. 1) Z = EXP(Z)
        WRITE(IUNIT,FORM1) Z,(STRP,J1=1,NYDIV),
     +    '   bin boundary   bin ''area''    # points'
        WEISUM = 0D0
        WEISU2 = 0D0
*       Loop over the divisions on the x axis. Print a line per div.
        DO 50 J1 = 1 , NRBINS(I)
*         First determine the height of this entry on the y axis.
          IY=1
          IF(IX.EQ.1) THEN
            IF(Y(I,J1).GT.0D0) IY=INT(Y(I,J1)/YMAX(I)*FLOAT(NYDIV))+1
          ELSE
            IF(FACTOR*Y(I,J1).GT.1D0) IY=INT(LOG(FACTOR*Y(I,J1))/
     +                            LOG(FACTOR*YMAX(I))*FLOAT(NYDIV))+1
          ENDIF
*         Fill the character array LINE that will be printed.
          IF(IY .EQ. NYDIV+1) IY = NYDIV
          DO 40 J2 = 1 , NYDIV
            LINE(J2)=BLANK
            IF(J2.EQ.IY) LINE(J2)=STAR
   40     CONTINUE
*         Prepare surrounding text and numbers
          Z = BOUND0(I) + J1/FLOAT(NRBINS(I))*(BOUND1(I)-BOUND0(I))
          IF(LINLOX(I) .NE. 1) Z = EXP(Z)
          WEISUM = WEISUM + Y(I,J1)
          WEISU2 = WEISU2 + YSQUAR(I,J1)
          IF(J1.EQ.INT(FLOAT(NRBINS(I))/2D0))THEN
            TEXT = ' (x,y) =   I'
          ELSEIF(J1.EQ.INT(FLOAT(NRBINS(I))/2D0)+1)THEN
            TEXT = ' (lin,lin) I'
            IF(IX.NE.1.AND.LINLOX(I).EQ.1) TEXT = ' (lin,log) I'
            IF(IX.EQ.1.AND.LINLOX(I).NE.1) TEXT = ' (log,lin) I'
            IF(IX.NE.1.AND.LINLOX(I).NE.1) TEXT = ' (log,log) I'
          ELSE
            TEXT = '           I'
          ENDIF
*         The actual WRITE !
          WRITE(IUNIT,F)TEXT,(LINE(J2),J2=1,NYDIV),Z,Y(I,J1),IBIN(I,J1)
   50   CONTINUE
        Z = BOUND1(I)
        IF(LINLOX(I) .NE. 1) THEN
          Z       = EXP(Z)
          XMIN(I) = EXP(XMIN(I))
          XMAX(I) = EXP(XMAX(I))
        ENDIF
*       End with information. Then we're through.
        WRITE(IUNIT,FORM1) Z,(STRP,J1=1,NYDIV),' '
        Z=SQRT(ABS(WEISU2-WEISUM**2/FLOAT(IIN(I))))/FLOAT(IIN(I))
        WRITE(IUNIT,'(12X,''The average of the entries amounts to '',
     +    G11.4,'' +- '',G11.4,/,12X,
     +    ''The fraction inside the histo bounds: '',G11.4)')WEISUM/
     +    FLOAT(IIN(I)+IUNDER(I)+IOVER(I)),Z,WEISUM/(WEISUM+YOUT(I))
        IF(IUNDER(I).GE.1) WRITE(IUNIT,60)'minimum',XMIN(I)
        IF( IOVER(I).GE.1) WRITE(IUNIT,60)'maximum',XMAX(I)
   60   FORMAT(12X,'The ',A,' value that occurred was   ',G11.4)
*       Finished ... - exit !
      ELSEIF(ISTAT .EQ. 3 .OR. ISTAT .EQ. 4) THEN
        IF(YMAX(I) .LE. 0D0) GOTO 930
        JUNIT = 11
        OPEN(UNIT=JUNIT,FILE='HISTO',STATUS='NEW')
        FACTOR = NRBINS(I)/(BOUND1(I)-BOUND0(I))
        IF(ISTAT .EQ. 4) FACTOR=FACTOR/FLOAT(IIN(I)+IUNDER(I)+IOVER(I))
        IF(LINLOG .EQ. 1) THEN
          IF(LINLOX(I) .EQ. 1) THEN
            WRITE(JUNIT,110) BOUND0(I),BOUND1(I),1.1D0*YMAX(I)*FACTOR
  110       FORMAT('*B',/,'VH 3.0',/,'LX 14.0',/,'LY 14.0',/,
     +       'XM ',D12.4,2X,D12.4,' 10',/,'YM 0. ',D12.4,' 10',/,'//')
          ELSE
            WRITE(JUNIT,120) 1.1D0*YMAX(I)*FACTOR
  120       FORMAT('*B',/,'VH 3.0',/,'LX 14.0',/,'LY 14.0',/,
     +       'XL',/,'YM 0. ',D12.4,' 10',/,'//')
          ENDIF
        ELSE
          Z = YMAX(I)*FACTOR
          DO 130 J1 = 2 , NRBINS(I)
            IF(FACTOR*Y(I,J1).LT.Z.AND.Y(I,J1).GT.0D0) Z=Y(I,J1)*FACTOR
  130     CONTINUE
          WEISUM = .8D0*Z
          IF(LINLOX(I) .EQ. 1) THEN
            WRITE(JUNIT,140) BOUND0(I),BOUND1(I)
          ELSE
            WRITE(JUNIT,150)
          ENDIF
  140     FORMAT('*B',/,'VH 3.0',/,'LX 14.0',/,'LY 14.0',/,'XM ',D12.4,
     +      2X,D12.4,' 10',/,'YL ',/,'//')
  150     FORMAT('*B',/,'VH 3.0',/,'LX 14.0',/,'LY 14.0',/,'XL',
     +      /,'YL',/,'//')
        ENDIF
        WRITE(JUNIT,*)' ',TITLE
        WRITE(JUNIT,'(///,''//'',/,''*P'',/,''SN -1'',/,''CL'')')
        Z = BOUND0(I)
        IF(LINLOX(I) .NE. 1) Z = EXP(Z)
        IF(Y(I,1).GT.0D0 .OR. LINLOG.EQ.1)
     +    WRITE(JUNIT,170) Z,Y(I,1)*FACTOR
        DO 160 J1 = 1 , NRBINS(I)-1
          Z = BOUND0(I) + J1/FLOAT(NRBINS(I))*(BOUND1(I)-BOUND0(I))
          IF(LINLOX(I) .NE. 1) Z = EXP(Z)
          IF((Y(I,J1).GT.0D0.AND.Y(I,J1+1).GT.0D0).OR.LINLOG.EQ.1)THEN
            WRITE(JUNIT,170) Z,Y(I,J1  )*FACTOR
            WRITE(JUNIT,170) Z,Y(I,J1+1)*FACTOR
          ELSEIF(Y(I,J1).GT.0D0) THEN
            WRITE(JUNIT,170) Z,Y(I,J1)*FACTOR
            WRITE(JUNIT,170) Z, WEISUM
            WRITE(JUNIT,'(''/'')')
          ELSEIF(Y(I,J1+1).GT.0D0) THEN
            WRITE(JUNIT,170) Z, WEISUM
            WRITE(JUNIT,170) Z,Y(I,J1+1)*FACTOR
          ENDIF
  160   CONTINUE
        J1 = NRBINS(I)
        Z  = BOUND1(I)
        IF(LINLOX(I) .NE. 1) Z = EXP(Z)
        IF(Y(I,J1).GT.0D0 .OR. LINLOG.EQ.1)
     +    WRITE(JUNIT,170) Z,Y(I,J1)*FACTOR
  170   FORMAT(' ',2D15.7)
        WRITE(JUNIT,'(''//'',/,''*E'')')
        CLOSE(UNIT=JUNIT)
      ELSEIF(ISTAT .EQ. 5) THEN
        JUNIT = 11
        OPEN(UNIT=JUNIT,FILE='HISTO',STATUS='NEW')
        WRITE(JUNIT,F2(1))(IUNDER(J),IIN(J),IOVER(J),NRBINS(J),J=1,N)
        WRITE(JUNIT,F2(2)) ((IBIN(J1,J2),J2=1,NXMAX),J1=1,N)
        WRITE(JUNIT,F2(3)) (YMAX(J1),BOUND0(J1),BOUND1(J1),
     +                     XMIN(J1),XMAX  (J1),YOUT  (J1),J1=1,N)
        WRITE(JUNIT,F2(3))((Y(J1,J2),YSQUAR(J1,J2),J2=1,NXMAX),J1=1,N)
        CLOSE(UNIT=JUNIT)
      ELSEIF(ISTAT .EQ. 6) THEN
        JUNIT = 11
        OPEN(UNIT=JUNIT,FILE='HISTO',STATUS='OLD')
        READ(JUNIT,F2(1))(IUNDER(J),IIN(J),IOVER(J),NRBINS(J),J=1,N)
        READ(JUNIT,F2(2)) ((IBIN(J1,J2),J2=1,NXMAX),J1=1,N)
        READ(JUNIT,F2(3)) (YMAX(J1),BOUND0(J1),BOUND1(J1),
     +                    XMIN(J1),XMAX  (J1),YOUT  (J1),J1=1,N)
        READ(JUNIT,F2(3))((Y(J1,J2),YSQUAR(J1,J2),J2=1,NXMAX),J1=1,N)
        CLOSE(UNIT=JUNIT)
      ELSE
*       Scaling of the y axis with factor X. Useful for normalization.
        DO 200 J1 = 1 , N
          YMAX(J1) = X * YMAX(J1)
          DO 190 J2 = 1 , NXMAX
            Y     (J1,J2) = X    * Y     (J1,J2)
            YSQUAR(J1,J2) = X**2 * YSQUAR(J1,J2)
  190     CONTINUE
  200   CONTINUE
      ENDIF
      RETURN
*     A few error returns.
  910 IWARN = IWARN + 1
      IF(IWARN .LE. 5) WRITE(*,'(A,I10,A,I2)')' HISTO called with I =',
     +                       I,',     Warning nr.',IWARN
      RETURN
  920 WRITE(IUNIT,'(A)')' All bins have weights <=0. No histo printed.'
      RETURN
  930 WRITE(  *  ,'(A)')' All bins have weights <=0. No histo printed.'
      RETURN
      END
 
      SUBROUTINE COUPLS(SIN2TH,RMT)
*     -----------------
* Define the fermion masses and their couplings to the bosons.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      RMASS(0) = 0D0
* Charged lepton masses from Particle Data Group Publ. 1990.
!WP      RMASS(1) = .51099906D-3
!WP      RMASS(2) = .105658387D0
!WP      RMASS(3) = 1.7841D0
!WP      RMASS(4) = .04145D0
!WP      RMASS(5) = .04146D0
!WP      RMASS(6) = 1.5D0
!WP      RMASS(7) = .15D0
!WP      RMASS(8) = RMT
!WP      RMASS(9) = 4.5D0
!WP: Charged lepton masses from Particle Data Group Publ. 1998.
      RMASS(1) = 0.51099907d-3
      RMASS(2) = 0.105658389d0
      RMASS(3) = 1.77705d0
      RMASS(4) = 0.00325d0
      RMASS(5) = 0.006d0
      RMASS(6) = 1.250d0
      RMASS(7) = 0.115d0
      RMASS(8) = RMT
      RMASS(9) = 4.250d0
      SW    = DSQRT( SIN2TH )
      CW    = DSQRT( 1D0 - SIN2TH )
      DO 10 I = 0 , NRMASS
         RMASS2(I) = RMASS(I)**2
         IF(I .EQ. 0) THEN
            Q  =  0D0
            T3 = .5D0
         ELSEIF(I .LE. 3) THEN
            Q  = -  1D0
            T3 = - .5D0
         ELSEIF(I.EQ.4 .OR. I.EQ.6 .OR. I.EQ.8) THEN
            Q  =   2D0/3D0
            T3 =  .5D0
         ELSE
            Q  = -  1D0/3D0
            T3 = - .5D0
         ENDIF
         VF(I) = ( T3 - 2D0*Q*SIN2TH ) /2D0/CW/SW
         AF(I) = T3 /2D0/SW/CW
         QF(I) =   Q
   10 CONTINUE
      END
 
      SUBROUTINE RNORM()
*     ----------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Calculate all quantities that have to do with weak corrections on
* boson propagators.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ RENORM / PIGAM0,SIGGZ0,DELMZ,DELMW,DELZ2Z,DELZ2G,DELZ2W,
     +                 SUMQ1,SUMQ2
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      SW  = DSQRT( SIN2TH )
      CW2 =  1D0 - SIN2TH
      CW  = DSQRT( CW2 )
      Z   = RMZ**2
      W   = RMW**2
      SIGGZ0 = USIGGZ( 0D0 )
      PIGAM0 = DUSIGG( 0D0 )
*
* Renormalization constants eq. (3.16) and (3.17) of ref.2b
      DELMZ  =   USIGZ( Z )
      DELMW  =   USIGW( W )
      DELZ2G = - PIGAM0
      DELZ2Z = - PIGAM0 - 2D0*(CW2-SIN2TH)/SW/CW*SIGGZ0/Z +
     +                    (CW2-SIN2TH)/SIN2TH*( DELMZ/Z - DELMW/W )
      DELZ2W = - PIGAM0 - 2D0*CW/SW*SIGGZ0/Z +
     +                    CW2/SIN2TH*( DELMZ/Z - DELMW/W )
*
* Contributions from the DELTA-i terms
      SUMQ1 = ALFA/4D0/PI/2D0/SIN2TH/W*(
     +      + ( RMASS2(4)-RMASS2(5) )*DLOG(RMASS(4)/RMASS(5))
     +      + ( RMASS2(6)-RMASS2(7) )*DLOG(RMASS(6)/RMASS(7))
     +      + ( RMASS2(8)-RMASS2(9) )*DLOG(RMASS(8)/RMASS(9)) )
      SUMQ2 = ALFA/2D0/PI*( + DLOG(RMASS(4)/RMASS(5))
     +                      + DLOG(RMASS(6)/RMASS(7))
     +                      + DLOG(RMASS(8)/RMASS(9)) )
*
* Calculate delta-r and update the values for sin(theta-w) and MW.
      DR = DELTAR()
      BIGA0  = 37.281D0**2
      SIN2TH = .5D0*(  1D0 - DSQRT( 1D0-4D0*BIGA0/Z/(1D0-DR) )  )
      RMW = DSQRT( Z*( 1D0 - SIN2TH ) )
      END
 
      SUBROUTINE FORMFS(QSQR,IFERM)
*     -----------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Calculate the vector and axial vector formfactors for the Z-ff and
* the gamma-ff couplings.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      COMMON/ FORMFA /FZV(0:NRMASS),FZA(0:NRMASS),
     +                FGV(0:NRMASS),FGA(0:NRMASS)
      COMPLEX*16 FZV,FZA,FGV,FGA,FL1,FL2,FL3,FL4,FL5
      COMPLEX*16 RL2Z,RL2W,RL3W,RL3Z
*
      CALL LABDAS(QSQR,RMZ,RL2Z,RL3Z)
      CALL LABDAS(QSQR,RMW,RL2W,RL3W)
      CW2  = 1D0 - SIN2TH
      SW   = DSQRT( SIN2TH )
      CW   = DSQRT( CW2 )
      ALF4PI = ALFA/4D0/PI
* eq. (C.4) ref 2b.
      FL1 = RL2W/8D0/SW**3/CW - 3D0*CW/4D0/SW**3*RL3W
      FL2 = -(1D0-2D0/3D0*SIN2TH)/8D0/SW**3/CW*RL2W +
     +         3D0*CW/4D0/SW**3*RL3W
      FL3 =  (1D0-4D0/3D0*SIN2TH)/8D0/SW**3/CW*RL2W -
     +         3D0*CW/4D0/SW**3*RL3W
      IF(IFERM .EQ. 9) THEN
         CALL FLBOT(QSQR,FL4,FL5)
      ELSE
         FL4 = FL3
         FL5 = 1D0/6D0/SIN2TH*RL2W - 3D0/4D0/SIN2TH*RL3W
      ENDIF
* eq. (C.3) of ref 2b.
      FZV(0) = ALFA/4D0/PI/4D0/SW/CW*( RL2Z/4D0/CW2/SIN2TH +
     +           (1D0-1D0/2D0/SIN2TH)*RL2W + 3D0*CW2/SIN2TH*RL3W )
      FZA(0) = FZV(0)
      FZV(1) = ALF4PI*( VF(1)*(VF(1)**2+3D0*AF(1)**2)*RL2Z + FL1 )
      FZA(1) = ALF4PI*( AF(1)*(3D0*VF(1)**2+AF(1)**2)*RL2Z + FL1 )
      FZV(4) = ALF4PI*( VF(4)*(VF(4)**2+3D0*AF(4)**2)*RL2Z + FL2 )
      FZA(4) = ALF4PI*( AF(4)*(3D0*VF(4)**2+AF(4)**2)*RL2Z + FL2 )
      FZV(5) = ALF4PI*( VF(5)*(VF(5)**2+3D0*AF(5)**2)*RL2Z + FL3 )
      FZA(5) = ALF4PI*( AF(5)*(3D0*VF(5)**2+AF(5)**2)*RL2Z + FL3 )
      FZV(9) = ALF4PI*( VF(9)*(VF(9)**2+3D0*AF(9)**2)*RL2Z + FL4 )
      FZA(9) = ALF4PI*( AF(9)*(3D0*VF(9)**2+AF(9)**2)*RL2Z + FL4 )
* eq. (C.12) ref 2b.
      FL1 = -3D0/ 4D0/SIN2TH*RL3W
      FL2 = -1D0/12D0/SIN2TH*RL2W + 3D0/4D0/SIN2TH*RL3W
      FL3 =  1D0/ 6D0/SIN2TH*RL2W - 3D0/4D0/SIN2TH*RL3W
      FGV(0) = CMPLX(0D0,0D0)
      FGA(0) = FGV(0)
* eq. (C.11) ref 2b.
      FGV(1) = ALF4PI*( QF(1)*(VF(1)**2+AF(1)**2)*RL2Z + FL1 )
      FGA(1) = ALF4PI*( QF(1)*( 2D0*VF(1)*AF(1) )*RL2Z + FL1 )
      FGV(4) = ALF4PI*( QF(4)*(VF(4)**2+AF(4)**2)*RL2Z + FL2 )
      FGA(4) = ALF4PI*( QF(4)*( 2D0*VF(4)*AF(4) )*RL2Z + FL2 )
      FGV(5) = ALF4PI*( QF(5)*(VF(5)**2+AF(5)**2)*RL2Z + FL3 )
      FGA(5) = ALF4PI*( QF(5)*( 2D0*VF(5)*AF(5) )*RL2Z + FL3 )
      FGV(9) = ALF4PI*( QF(9)*(VF(9)**2+AF(9)**2)*RL2Z + FL5 )
      FGA(9) = ALF4PI*( QF(9)*( 2D0*VF(9)*AF(9) )*RL2Z + FL5 )
* all others are related to the previous ones.
      DO 10 I = 0 , NRMASS
         IF(I.EQ.2 .OR. I.EQ.3) THEN
            FZV(I) = FZV(1)
            FZA(I) = FZA(1)
            FGV(I) = FGV(1)
            FGA(I) = FGA(1)
         ELSEIF(I.EQ.6 .OR. I.EQ.8) THEN
            FZV(I) = FZV(4)
            FZA(I) = FZA(4)
            FGV(I) = FGV(4)
            FGA(I) = FGA(4)
         ELSEIF(I.EQ.7) THEN
            FZV(I) = FZV(5)
            FZA(I) = FZA(5)
            FGV(I) = FGV(5)
            FGA(I) = FGA(5)
         ENDIF
   10 CONTINUE
      END
 
      SUBROUTINE LABDAS(QSQR,RM,LABDA2,LABDA3)
*     -----------------
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 LABDA2,LABDA3,W,X,BIGK,SPENCF
      PARAMETER( EPS = 1D-10 )
      W = RM**2/DCMPLX( QSQR , EPS )
      X = LOG( - BIGK(QSQR,RM) )
* This way of writing was not very stable for qsqr << rm**2. The
* second way is somewhat better, but still has to be cut off at some
* low qsqr value, in which case it should yield zero.
*      LABDA2 = -3.5D0 - 2D0*W - (2D0*W+3D0)*LOG(-W)+
*     +   2D0*(1D0+W)**2*( SPENCF(1D0+1D0/W) - PI**2/6D0 )
      LABDA2 = -3.5D0 - 2D0*W - (2D0*W+3D0)*LOG(-W)+
     +   2D0*(1D0+W)**2*( -SPENCF(-1D0/W)+LOG(-W)*LOG(1D0+1D0/W) )
      IF(DBLE(W).GT.1D6) LABDA2 = (0D0,0D0)
      LABDA3 =  5D0/6D0 - 2D0*W/3D0 - 1D0/3D0*(2D0*W+1)*
     +   CDSQRT(1D0-4D0*W)*X + 2D0/3D0*W*(W+2D0)*X**2
      END
 
      FUNCTION BIGK(QSQR,RM)
*     -------------
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 BIGK,W
      W = RM**2/DCMPLX( QSQR , 1D-10 )
      BIGK = - ( CDSQRT(1D0-4D0*W) - 1D0 )/( CDSQRT(1D0-4D0*W) + 1D0 )
      END
 
      FUNCTION SIGG(QSQR)
*     -------------
* Real part of the renormalized weakly corrected photon propagator
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ RENORM / PIGAM0,SIGGZ0,DELMZ,DELMW,DELZ2Z,DELZ2G,DELZ2W,
     +                 SUMQ1,SUMQ2
* eq. (3.23) ref 2b.
      SIGG = USIGG(QSQR) - PIGAM0 * QSQR
      END
 
      FUNCTION SIGGZ(QSQR)
*     --------------
* Real part of the renormalized weakly corrected photon-Z mixing
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ RENORM / PIGAM0,SIGGZ0,DELMZ,DELMW,DELZ2Z,DELZ2G,DELZ2W,
     +                 SUMQ1,SUMQ2
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      CW2 = 1D0 - SIN2TH
      CW  = DSQRT( CW2 )
      SW  = DSQRT( SIN2TH )
* eq. (3.23) ref 2b.
      SIGGZ = USIGGZ(QSQR) - SIGGZ0 - QSQR* CW*SW/(CW2-SIN2TH)*
     +        ( DELZ2Z - DELZ2G ) +
     +        QSQR*( - CW/SW*SUMQ1 - SUMQ2/6D0/CW/SW )
      END
 
      FUNCTION SIGZ(QSQR)
*     -------------
* Real part of the renormalized weakly corrected Z propagator
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ RENORM / PIGAM0,SIGGZ0,DELMZ,DELMW,DELZ2Z,DELZ2G,DELZ2W,
     +                 SUMQ1,SUMQ2
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      CW2 = 1D0 - SIN2TH
* eq. (3.23) ref 2b.
      SIGZ = USIGZ(QSQR) - DELMZ + DELZ2Z*( QSQR - RMZ**2 ) +
     +       (QSQR-RMZ**2)*((CW2-SIN2TH)/SIN2TH*SUMQ1+SUMQ2/3D0/SIN2TH)
      END
 
      FUNCTION SIGW(QSQR)
*     -------------
* Real part of the renormalized weakly corrected W propagator
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ RENORM / PIGAM0,SIGGZ0,DELMZ,DELMW,DELZ2Z,DELZ2G,DELZ2W,
     +                 SUMQ1,SUMQ2
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      CW2 = 1D0 - SIN2TH
* eq. (3.23) ref 2b.
      SIGW = USIGW(QSQR) - DELMW + DELZ2W*( QSQR - RMW**2 ) +
     +       (QSQR-RMW**2)*( CW2/SIN2TH*SUMQ1 + SUMQ2/3D0/SIN2TH )
      END
 
      FUNCTION DELTAR()
*     ---------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* The weak correction factor delta-r
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
* eq. (4.18) ref 2b.
      DELTAR = SIGW(0D0)/RMW**2 + ALFA/4D0/PI/SIN2TH*
     +        ( 6D0 + (7D0-4D0*SIN2TH)/2D0/SIN2TH*DLOG(1D0-SIN2TH) )
      END
 
      FUNCTION USIGG(QSQR)
*     --------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Real part of the unrenormalized weakly corrected photon prop
* eq. (B.2) ref 2b with errata, a minus sign and a bracket.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      S   = QSQR
      TOT = 0D0
      FAC = 1D0
      DO 10 I = 1 , NRMASS
         IF(I .EQ. 4) FAC = 3D0
         RM = RMASS(I)
         TOT = TOT + 4D0/3D0*QF(I)**2*(
     +         ( S+2D0*RMASS2(I) )*FREAL(S,RM,RM) - S/3D0 )  *  FAC
   10 CONTINUE
      TOT = TOT - ( 3D0*S + 4D0*RMW**2 )*FREAL(S,RMW,RMW)
      USIGG = ALFA/4D0/PI * TOT
      END
 
      FUNCTION DUSIGG(QSQR)
*     ---------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Derivative of the real part of the unrenormalized
* weakly corrected photon propagator
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      S   = QSQR
      TOT = 0D0
      FAC = 1D0
      DO 10 I = 1 , NRMASS
         IF(I .EQ. 4) FAC = 3D0
         RM = RMASS(I)
         TOT = TOT + 4D0/3D0*QF(I)**2*( + FREAL(S,RM,RM) +
     +         ( S+2D0*RMASS2(I) )*DFREAL(S,RM,RM) - 1D0/3D0 )  *  FAC
   10 CONTINUE
      TOT = TOT - 3D0*FREAL(S,RMW,RMW) -
     +            ( 3D0*S + 4D0*RMW**2 )*DFREAL(S,RMW,RMW)
      DUSIGG = ALFA/4D0/PI * TOT
      END
 
      FUNCTION PHADPI(QSQR)
*     ---------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Real part of the hadronic part of sigma-g(qsqr) / qsqr = pi-hadronic,
* calculated perturbatively.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      S   = QSQR
      TOT = 0D0
      FAC = 3D0
      DO 10 I = 4 , NRMASS
         RM = RMASS(I)
         IF(I.NE.8) TOT = TOT + 4D0/3D0*QF(I)**2*(
     +         ( S+2D0*RMASS2(I) )*FREAL(S,RM,RM) - S/3D0 ) * FAC/S
   10 CONTINUE
      PHADPI = ALFA/4D0/PI * TOT
      END
 
      FUNCTION USIGGZ(QSQR)
*     ---------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Real part of the unrenormalized photon-Z mixing propagator
* eq. (B.3) ref 2b.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      S   = QSQR
      SW  = DSQRT(       SIN2TH )
      CW  = DSQRT( 1D0 - SIN2TH )
      TOT = 0D0
      FAC = 1D0
      DO 10 I = 1 , NRMASS
         IF(I .EQ. 4) FAC = 3D0
         RM = RMASS(I)
         TOT = TOT - 4D0/3D0*QF(I)*VF(I)*(
     +         ( S+2D0*RMASS2(I) )*FREAL(S,RM,RM) - S/3D0 )  *  FAC
   10 CONTINUE
      TOT = TOT + 1D0/CW/SW*( ( 3D0*CW**2 + 1D0/6D0 )*S
     +                      + ( 4D0*CW**2 + 4D0/3D0 )*RMW**2 )*
     +                       FREAL(S,RMW,RMW)  +  S/9D0/CW/SW
      USIGGZ = ALFA/4D0/PI * TOT
      END
 
      FUNCTION USIGZ(QSQR)
*     --------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Real part of the unrenormalized weakly corrected Z propagator,
* for QSQR > 0.
* eq. (B.4) ref 2b. 1 erratum in the pole part, not apparent here.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      S   = QSQR
      CW2 =  1D0 - SIN2TH
      TOT = 0D0
      FAC = 1D0
      DO 10 I = 1 , NRMASS
         IF(I .EQ. 4) FAC = 3D0
         IF(I .LE. 3) TOT = TOT + 4D0/3D0* 2D0*AF(I)**2*S*(
     +                    + 5D0/3D0 - DLOG(ABS( S/RMASS2(I) )) ) * FAC
         RM = RMASS(I)
         F = FREAL(S,RM,RM)
         TOT = TOT + 4D0/3D0*(  ( VF(I)**2+AF(I)**2 )*(
     +                         + ( S+2D0*RMASS2(I) )*F - S/3D0 )
     +             - 3D0/8D0/SIN2TH/CW2*RMASS2(I)*F ) * FAC
   10 CONTINUE
      W = RMW**2
      Z = RMZ**2
      H = RMH**2
      TOT = TOT + ( ( -CW2**2*(40D0*S+80D0*W) + 12D0*W +
     1              (CW2-SIN2TH)**2*( 8D0*W+S ) )*FREAL(S,RMW,RMW) +
     2            ( 10D0*Z - 2D0*H + S + (H-Z)**2/S )*FREAL(S,RMH,RMZ)-
     3            2D0*H*DLOG(H/W) - 2D0*Z*DLOG(Z/W) +
     4            ( 10D0*Z - 2D0*H + S )*( 1D0 - (H+Z)/(H-Z)*
     5              DLOG(RMH/RMZ) - DLOG(RMH*RMZ/W) ) +
     6            2D0/3D0*S*( 1D0 + (CW2-SIN2TH)**2 - 4D0*CW2**2 )
     7  )/12D0/CW2/SIN2TH
      USIGZ = ALFA/4D0/PI * TOT
      END
 
      FUNCTION USIGW(QSQR)
*     --------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Real part of the unrenormalized weakly corrected W propagator,
* for QSQR >= 0.
* eq. (B.5) ref 2b with errata: a factor 3 for the last 7 lines,
*                               one factor s, one factor 1/s and a sign
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      REAL*8 MP,MM
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      S   = QSQR
      CW2 =  1D0 - SIN2TH
      W   = RMW**2
      Z   = RMZ**2
      H   = RMH**2
      TOT = 0D0
      IF(DABS(S) .GT. 1D-10) THEN
         DO 10 I = 1 , NRMASS
            RM  = RMASS (I)
            RM2 = RMASS2(I)
            IF(I .LE. 3) THEN
              TOT = TOT + ( S-RM2/2D0-RM2**2/2D0/S )*FREAL(S,0D0,RM) +
     +                     2D0/3D0*S - RM2/2D0
            ELSE
               FAC = 3D0
               IF(MOD(I,2) .EQ. 0) THEN
                  MP  = RMASS(I)
                  MM  = RMASS(I+1)
                  SUM = RMASS2(I) + RMASS2(I+1)
                  DIF = RMASS2(I) - RMASS2(I+1)
                  TOT = TOT + (
     +               (S-SUM/2D0-DIF**2/2D0/S)*FREAL(S,MP,MM)+
     +               (S-SUM/2D0)*(1D0-SUM/DIF*DLOG(MP/MM))-S/3D0) * FAC
               ENDIF
            ENDIF
   10    CONTINUE
         TOT = TOT + 3D0*(  ( SIN2TH**2*Z - CW2/3D0*( 7D0*Z + 7D0*W +
     1                  10D0*S - 2D0*(Z-W)**2/S ) - 1D0/6D0*(W+Z-S/2D0-
     2              (Z-W)**2/2D0/S ) )*FREAL(S,RMZ,RMW) +
     3       SIN2TH/3D0*( -4D0*W-10D0*S+2D0*W**2/S )*FREAL(S,0D0,RMW) +
     4       1D0/6D0*( 5D0*W-H+S/2D0+(H-W)**2/2D0/S )*FREAL(S,RMH,RMW)+
     5       ( CW2/3D0*( 7D0*Z+7D0*W+10D0*S-4D0*(Z-W) ) - SIN2TH**2*Z +
     6         1D0/6D0*( 2D0*W - S/2D0 ) ) * Z/(Z-W)*DLOG(Z/W) -
     7       ( 2D0/3D0*W + S/12D0 ) * H/(H-W)*DLOG(H/W) -
     8       CW2/3D0*( 7D0*Z + 7D0*W + 32D0/3D0*S ) + SIN2TH**2*Z +
     9       1D0/6D0*( 5D0/3D0*S + 4D0*W - Z - H ) -
     1       SIN2TH/3D0*( 4D0*W + 32D0/3D0*S )  )
      ELSE
         DO 20 I = 1 , NRMASS
            RM  = RMASS (I)
            RM2 = RMASS2(I)
            IF(I .LE. 3) THEN
              TOT = TOT - 3D0/4D0 * RM2
            ELSE
               FAC = 3D0
               IF(MOD(I,2) .EQ. 0) THEN
                  MP  = RMASS(I)
                  MM  = RMASS(I+1)
                  SUM = RMASS2(I) + RMASS2(I+1)
                  DIF = RMASS2(I) - RMASS2(I+1)
                  TOT = TOT -.5D0*(
     +                      3D0/2D0*SUM - DLOG(MP/MM)/DIF*
     +                      (SUM**2+2D0*RMASS2(I)*RMASS2(I+1)) ) * FAC
               ENDIF
            ENDIF
   20    CONTINUE
         TOT = TOT +3D0*( (2D0/3D0*CW2+1D0/12D0)*(.5D0*(W+Z)-W*Z/(W-Z)*
     1               DLOG(W/Z) ) + W/3D0*SIN2TH + 1D0/12D0*( .5D0*
     2               (H+W)-H*W/(W-H)*DLOG(W/H) ) + ( CW2/3D0*( 3D0*Z+
     3               11D0*W ) - SIN2TH**2*Z + 1D0/3D0*W )*Z/(Z-W)*
     4               DLOG(Z/W) - 2D0/3D0*W*H/(H-W)*DLOG(H/W) -
     5               1D0/3D0*CW2*(7D0*Z+7D0*W) + SIN2TH**2*Z + 1D0/6D0*
     6              (4D0*W-Z-H) - SIN2TH/3D0*4D0*W  )
      ENDIF
      USIGW = ALFA/4D0/PI /3D0/SIN2TH  * TOT
      END
 
      FUNCTION BIGPIZ(QSQR)
*     ---------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Derivative of the real part of the renormalized
* weakly corrected Z propagator, for QSQR > 0.
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/ RENORM / PIGAM0,SIGGZ0,DELMZ,DELMW,DELZ2Z,DELZ2G,DELZ2W,
     +                 SUMQ1,SUMQ2
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      S   = QSQR
      CW2 = 1D0 - SIN2TH
      TOT = 0D0
      FAC = 1D0
      DO 10 I = 1 , NRMASS
         IF(I .EQ. 4) FAC = 3D0
         IF(I .LE. 3) TOT = TOT + 4D0/3D0* 2D0*AF(I)**2*(
     +                    + 2D0/3D0 - DLOG(ABS( S/RMASS2(I) )) ) * FAC
         RM = RMASS(I)
         F  =  FREAL(S,RM,RM)
         DF = DFREAL(S,RM,RM)
         TOT = TOT + 4D0/3D0*(  ( VF(I)**2+AF(I)**2 )*(
     +                   F  + ( S+2D0*RMASS2(I) )*DF - 1D0/3D0 )
     +             - 3D0/8D0/SIN2TH/CW2*RMASS2(I)*DF ) * FAC
   10 CONTINUE
      W = RMW**2
      Z = RMZ**2
      H = RMH**2
      TOT = TOT + ( ( -CW2**2*(40D0*S+80D0*W) + 12D0*W +
     1                (CW2-SIN2TH)**2*( 8D0*W+S ) )*DFREAL(S,RMW,RMW) +
     2               (-40D0*CW2**2+(CW2-SIN2TH)**2 )*FREAL(S,RMW,RMW) +
     3            ( 10D0*Z-2D0*H+S+(H-Z)**2/S )*DFREAL(S,RMH,RMZ) +
     4            (        1D0 - (H-Z)**2/S**2 )*FREAL(S,RMH,RMZ) +
     5            ( 1D0-(H+Z)/(H-Z)*DLOG(RMH/RMZ)-DLOG(RMH*RMZ/W) ) +
     6            2D0/3D0*( 1D0 + (CW2-SIN2TH)**2 - 4D0*CW2**2 )
     7  )/12D0/CW2/SIN2TH
      BIGPIZ = TOT * ALFA/4D0/PI + DELZ2Z +
     +                    (CW2-SIN2TH)/SIN2TH*SUMQ1 + SUMQ2/3D0/SIN2TH
      END
 
      FUNCTION FREAL(S,RM1,RM2)
*     --------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Real part of the function F(s,ma,mb), eq. (B.6) ref 2b.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( EPS = 1D-10 )
      IF(RM1 .LT. RM2) THEN
        RMA = RM1
        RMB = RM2
      ELSE
        RMA = RM2
        RMB = RM1
      ENDIF
      RMA2 = RMA**2
      RMB2 = RMB**2
      IF(DABS(S) .LT. EPS) THEN
        F = 0D0
      ELSEIF(S .GT. 0D0) THEN
        IF(RMA2 .LT. EPS .AND. RMB2 .LT. EPS) THEN
          WRITE(*,*)'FREAL: ma = mb = 0 cannot be handled !'
          F = 0D0
        ELSEIF(RMA2 .LT. EPS) THEN
          F = - 1D0 + DLOG( RMB2 )
        ELSEIF(DABS(RMA2-RMB2) .LT. EPS) THEN
          F =         DLOG( RMB2 )
        ELSE
          F = - 1D0 + ( RMA2*DLOG(RMA2)-RMB2*DLOG(RMB2) )/(RMA2-RMB2)
        ENDIF
        F = - F
        IF(RMA2 .LT. EPS) THEN
          IF(DABS(S-RMB2) .LT. EPS) THEN
            F = F + DLOG(S) - 2D0
          ELSE
            F = F + DLOG(S) - 1D0 + RMB2/S*DLOG(RMB2/S) - RMB2/S -
     +        ( (RMB2/S-1D0)*DLOG(DABS(RMB2/S-1D0)) - (RMB2/S-1D0) )
          ENDIF
        ELSE
          S0   = - .5D0*( 1D0 + RMA2/S - RMB2/S )
          S1   =   .5D0*( 1D0 - RMA2/S + RMB2/S )
          DISCR= ( (S+RMA2-RMB2)**2 - 4D0*RMA2*S ) /4D0/S**2
          ROOTD= DSQRT( DABS( DISCR ) )
          F = F + DLOG(S) + S1*DLOG( RMB2/S ) - 2D0*S1 -
     +                      S0*DLOG( RMA2/S ) + 2D0*S0
          IF(DISCR .GE. 0D0) THEN
            IF(S.LT.RMA2 .OR. S.LT.RMB2) THEN
              F = F + ROOTD*( DLOG( (S1+ROOTD)**2*S/RMB2 )
     +                      - DLOG( (S0+ROOTD)**2*S/RMA2 ) )
            ELSE
              F = F + ROOTD*( DLOG( (S1+ROOTD)**2*S/RMB2 )
     +                      - DLOG( RMA2/S/(S0-ROOTD)**2 ) )
            ENDIF
          ELSE
            F = F + 2D0*ROOTD*( DATAN(S1/ROOTD) - DATAN(S0/ROOTD) )
          ENDIF
        ENDIF
      ELSE
        IF(RMA2 .LT. EPS .AND. RMB2 .LT. EPS) THEN
          WRITE(*,*)'FREAL: ma = mb = 0 cannot be handled !'
          F = 0D0
        ELSEIF(RMA2 .LT. EPS) THEN
          F = - 1D0 - ( 1D0-RMB2/S )*LOG( RMB2/(RMB2-S) )
        ELSE
          IF(ABS(RMA2-RMB2) .LT. EPS) THEN
            F = - 2D0
          ELSE
            F = - 1D0 - ( (RMA2-RMB2)/S - (RMA2+RMB2)/(RMA2-RMB2) )*
     +                  .5D0*LOG(RMB2/RMA2)
          ENDIF
          ROOTA = SQRT( (RMA+RMB)**2 - S )
          ROOTB = SQRT( (RMA-RMB)**2 - S )
          F = F - ROOTA*ROOTB/S*LOG( (ROOTA+ROOTB)**2/4D0/RMA/RMB )
        ENDIF
      ENDIF
      FREAL = - F
      END
 
      FUNCTION FIMAG(S,RMA,RMB)
*     --------
* Imaginary part of the function F(s,ma,mb)
      IMPLICIT REAL*8 (A-H,O-Z)
      PARAMETER (PI=3.1415926535897932D0)
      FIMAG = 0D0
      IF(S.GT.(RMA+RMB)**2) FIMAG=PI*DSQRT((S-(RMA+RMB)**2)*
     +                                     (S-(RMA-RMB)**2))/S
      END
 
      FUNCTION DFREAL(S,RM1,RM2)
*     ---------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Derivative of the real part of the function F(s,ma,mb).
      IMPLICIT REAL*8(A-H,O-Z)
      EPS = 1D-10
      IF(RM1 .LT. RM2) THEN
         RMA = RM1
         RMB = RM2
      ELSE
         RMA = RM2
         RMB = RM1
      ENDIF
      RMA2 = RMA**2
      RMB2 = RMB**2
      DIFF = RMB2 - RMA2
      IF(S.LT.EPS .AND. RMA2.LT.EPS .AND. RMB2.LT.EPS) THEN
         WRITE(*,*)'DFREAL: S = Ma = Mb = 0 cannot be handled !'
         F = 0D0
      ELSEIF(RMA2.LT.EPS .AND. RMB2.LT.EPS) THEN
         F = 1D0/S
      ELSEIF(ABS(S).LT.EPS .AND. DABS(RMA2-RMB2).LT.EPS) THEN
         F = - 1D0/6D0/RMA2
      ELSEIF(RMA2 .LT. EPS) THEN
         F = 1D0/S*( 1D0 - RMB2/S*DLOG( RMB2/DABS(RMB2-S) ) )
      ELSEIF(ABS(S) .LT. EPS) THEN
         F = 1D0/DIFF*( .5D0 - RMB2/DIFF -
     +                  RMA2*RMB2/DIFF**2*DLOG(RMB2/RMA2) )
      ELSEIF(S .LT. 0D0) THEN
         A = (RMA+RMB)**2 - S
         B = (RMA-RMB)**2 - S
         ROOTA = SQRT( A )
         ROOTB = SQRT( B )
         F = .5D0*(RMA2-RMB2)/S**2*LOG(RMB2/RMA2) + ROOTA*ROOTB/S*( (
     +       1D0/2D0/A + 1D0/2D0/B + 1D0/S )*
     +       LOG( (ROOTA+ROOTB)**2/4D0/RMA/RMB ) + 1D0/ROOTA/ROOTB )
      ELSE
         DISCR = - ( ( S + RMA2 - RMB2 )**2 - 4D0*RMA2*S )/4D0/S**2
         ROOTD = DSQRT( DABS( DISCR ) )
         SP = (   S - RMA2 + RMB2 )/2D0/S
         SM = ( - S - RMA2 + RMB2 )/2D0/S
         IF(ROOTD .LT. EPS) THEN
            F = - ( 1D0/SP - 1D0/SM ) / S
         ELSEIF(DISCR .LT. 0D0) THEN
            IF(S.LT.RMA2 .OR. S.LT.RMB2) THEN
               F =-.5D0/S/ROOTD*DLOG( (ROOTD+SP)**2/RMB2
     +                               /(ROOTD+SM)**2*RMA2 )
            ELSE
               F =-.5D0/S/ROOTD*( DLOG( (ROOTD+SP)**2*S/RMB2 )
     +                          - DLOG( RMA2/S/(SM-ROOTD)**2 ) )
            ENDIF
         ELSE
            F = 1D0/S/ROOTD*( DATAN(SP/ROOTD) - DATAN(SM/ROOTD) )
         ENDIF
         F = F * ( - S*(RMA2+RMB2) + DIFF**2 )/2D0/S**2 +
     +         1D0/S - DIFF/2D0/S**2*DLOG(RMB2/RMA2)
      ENDIF
      DFREAL = - F
      END
 
      FUNCTION PWIDTH(I)
*     ---------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* The partial width of the Z due to fermion i.
* Fermionic 2 loop effects have been taken into account using
* eq. (5.18) ref 2b and simple QED and QCD correction factors.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ FORMFA /FZV(0:NRMASS),FZA(0:NRMASS),
     +                FGV(0:NRMASS),FGA(0:NRMASS)
      COMPLEX*16 FZV,FZA,FGV,FGA
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      Z = RMZ**2
      GZMIX  = SIGGZ(Z) / Z
      CALL FORMFS(Z,I)
      TOT = 0D0
      IF(I .EQ. 0) THEN
         FAC = 1D0
      ELSEIF(I .LE. 3) THEN
         FAC = 1D0*( 1D0 + FACQED*QF(I)**2 )
      ELSEIF(I .EQ. 9) THEN
         FAC = 3D0*( 1D0 + FACQED*QF(I)**2 )*( 1D0 + FACQCB )
      ELSE
         FAC = 3D0*( 1D0 + FACQED*QF(I)**2 )*( 1D0 + FACQCD )
      ENDIF
      IF(Z .GT. 4.D0*RMASS2(I)) THEN
         TOT = ALFA*DSQRT(1D0-4D0*RMASS2(I)/Z)*
     +      ( ( VF(I)**2 + AF(I)**2 )*( Z + 2D0*RMASS2(I) ) -
     +       6D0*RMASS2(I)*AF(I)**2 )/3.D0 * FAC
*
         TOT = TOT + FAC * 2D0/3D0*ALFA*Z*(
     +         VF(I)*( DBLE(FZV(I)) + QF(I)*GZMIX )
     +       + AF(I)*  DBLE(FZA(I)) )
      ENDIF
      PWIDTH = TOT/( 1D0 + BIGPIZ(Z) )/ RMZ
      END
 
      FUNCTION IMSIGZ(S)
*     ---------------
* Imaginary part of the 1-loop Z self-energy
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      REAL*8 IMSIGZ
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      CW2 = 1D0 - SIN2TH
      TOT = 0D0
      DO 10 I = 0 , NRMASS
         IF(I .EQ. 0) FAC = 3D0
         IF(I .EQ. 1) FAC = 1D0
         IF(I .EQ. 4) FAC = 3D0
         IF(S .GT. 4.D0*RMASS2(I)) THEN
            TOT = TOT + DSQRT(1D0-4D0*RMASS2(I)/S)*
     +         ( ( VF(I)**2 + AF(I)**2 )*( S + 2D0*RMASS2(I) ) -
     +          6D0*RMASS2(I)*AF(I)**2 )/3.D0 * FAC
         ENDIF
   10 CONTINUE
      IF(S .GT. 4D0*RMW**2) TOT = TOT + DSQRT(1D0-4D0*RMW**2/S)*
     +       ((-10D0*S-20D0*RMW**2)*CW2**2+(2D0*RMW**2+S/4D0)*
     +             (CW2-SIN2TH)**2+3D0*RMW**2)*4.D0*AF(1)**2/3D0
      IF(S .GT. (RMH+RMZ)**2) TOT = TOT +
     +        (10D0*RMZ**2-2D0*RMH**2+S+(RMH**2-RMZ**2)**2/S)*AF(1)**2*
     +        DSQRT((1D0-(RMZ-RMH)**2/S)*(1D0-(RMZ+RMH)**2/S))/3D0
      IMSIGZ = TOT * ALFA
      END
 
      FUNCTION IMZ2(S)
*     -------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* Imaginary part of the fermionic 2-loop Z self-energy.
* eq. (5.18) ref 2b and simple QED and QCD correction factors.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      REAL*8 IMZ2
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ FORMMZ /FZVMZ(0:NRMASS),FZAMZ(0:NRMASS),
     +                FGVMZ(0:NRMASS),FGAMZ(0:NRMASS)
      COMPLEX*16 FZVMZ,FZAMZ,FGVMZ,FGAMZ
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      Z = RMZ**2
      GZMIX  = SIGGZ(Z) / Z
      TOT = 0D0
      DO 10 I = 0 , NRMASS
*!!!=============================================
*!!! WP: Here was a BUG - fixed 30.05.2001 (WP)
*!!!         IF(S .GT. 4D0*RMASS2(I)) THEN
         IF(Z .GT. 4D0*RMASS2(I)) THEN
*!!!=============================================
            IF(I .EQ. 0) THEN
               FAC1 = 3D0*FACQED*QF(I)**2
               FAC2 = 3D0 + FAC1
            ELSEIF(I .LE. 3) THEN
               FAC1 = 1D0*FACQED*QF(I)**2
               FAC2 = 1D0 + FAC1
            ELSEIF(I .EQ. 9) THEN
               FAC1 = 3D0*( FACQED*QF(I)**2*(1D0+FACQCB) + FACQCB )
               FAC2 = 3D0 + FAC1
            ELSE
               FAC1 = 3D0*( FACQED*QF(I)**2*(1D0+FACQCD) + FACQCD )
               FAC2 = 3D0 + FAC1
            ENDIF
            TOT = TOT +
     +           RMZ**2*ALFA*DSQRT(1D0-4D0*RMASS2(I)/Z)*
     +         ( ( VF(I)**2 + AF(I)**2 )*( 1D0 + 2D0*RMASS2(I)/Z ) -
     +          6D0*RMASS2(I)/Z*AF(I)**2 )/3.D0 * FAC1
            TOT = TOT + FAC2 * 2D0/3D0*ALFA*RMZ**2*(
     +               VF(I)*( DBLE(FZVMZ(I)) + QF(I)*GZMIX )
     +             + AF(I)*  DBLE(FZAMZ(I)) )
         ENDIF
   10 CONTINUE
      TOT  = TOT * S/RMZ**2
      IMZ2 = TOT
      END
 
      FUNCTION IMSIGG(S)
*     ---------------
* Imaginary part of the 1-loop QED vacuumpolarization
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      REAL*8 IMSIGG
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      TOT = 0D0
      FAC = 1D0
      DO 10 I = 1 , NRMASS
         IF(I .EQ. 4) FAC = 3D0
         IF(S .GT. 4D0*RMASS2(I)) TOT = TOT + FAC * QF(I)**2*
     +        DSQRT(1D0-4D0*RMASS2(I)/S)*(1D0+2D0*RMASS2(I)/S)/3D0
   10 CONTINUE
      IF(S .GT. 4D0*RMW**2) TOT = TOT -
     +         DSQRT(1D0-4D0*RMW**2/S)*(3D0/4D0+RMW**2/S)
      IMSIGG = TOT * ALFA * S
      END
 
      FUNCTION IMSGGZ(S)
*     ---------------
* Imaginary part of the 1-loop Z-gamma mixing
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      REAL*8 IMSGGZ
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMMON/ ADHOC  / ALFA, PI, FACQCB, FACQCD,FACQED,HBARC2,EE
      CW2 = 1D0 - SIN2TH
      TOT = 0D0
      FAC = 1D0
      DO 10 I = 1 , NRMASS
         IF(I .EQ. 4) FAC = 3D0
         IF(S .GT. 4D0*RMASS2(I)) TOT = TOT - S*QF(I)*VF(I)/3D0* FAC*
     +             DSQRT(1D0-4D0*RMASS2(I)/S)*(1D0+2D0*RMASS2(I)/S)
   10 CONTINUE
      IF(S .GT. 4D0*RMW**2) TOT = TOT-S*AF(1)*DSQRT(1D0-4D0*RMW**2/S)*
     +                ( (3D0*CW2+1D0/6D0)+RMW**2/S*(4D0*CW2+4D0/3D0) )
      IMSGGZ = TOT * ALFA
      END
 
      SUBROUTINE FLBOT(QSQR,FZL9,FGL9)
*     ----------------
* W.J.P. Beenakker, F.A.Berends and S.C. van der Marck, June 14th 1989
* The left handed Z-bb and gamma-bb form factors
* see eqs (C.8),(C.9), (C.14) ff. of ref 2b.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( NRMASS = 9 )
      COMMON/ MASSES / RMASS(0:NRMASS), RMASS2(0:NRMASS),
     +            VF(0:NRMASS),AF(0:NRMASS),QF(0:NRMASS)
      COMMON/ BOSONS / RMZ,RMW,RMH,RMT,SIN2TH,ZWID
      COMPLEX*16 FI(1:7),GI(1:7),ZLFIN,TOT1,TOT2,FZL9,FGL9,HELP
      COMPLEX*16 C1,C2,C3,C4,C5,C6,C7,C8,C9,C10
      COMPLEX*16 B1BAR,C0SCAL,C1PLUS,C2ZERO,C2MIN,C2PLUS
      SW = DSQRT( SIN2TH )
      CW2 = 1D0 - SIN2TH
      CW  = DSQRT( CW2 )
      RT = RMASS(8)
      RB = RMASS(9)
      C1 = C0SCAL(QSQR,RT,RMW,RB)
      C2 = C1PLUS(QSQR,RT,RMW,RB)
      C3 = C2MIN (QSQR,RT,RMW,RB)
      C4 = C2ZERO(QSQR,RT,RMW,RB)
      C5 = C2PLUS(QSQR,RT,RMW,RB)
      C6 = C0SCAL(QSQR,RMW,RT,RB)
      C7 = C1PLUS(QSQR,RMW,RT,RB)
      C8 = C2MIN (QSQR,RMW,RT,RB)
      C9 = C2ZERO(QSQR,RMW,RT,RB)
      C10= C2PLUS(QSQR,RMW,RT,RB)
      ZLFIN = 1D0/2D0/SIN2TH*( 2D0 + RT**2/RMW**2 )*(
     +         B1BAR(RB**2,RT,RMW) )
      FI(1) = (2D0/3D0*SIN2TH-1D0)/4D0/CW/SW * ZLFIN
      GI(1) = -1D0/6D0*ZLFIN
      HELP  = -1.5D0 + 2D0*DLOG(RMW/RT) + 4D0*C4 - 2D0*QSQR*(C5 - C3)
     +        + 4D0*QSQR*(C2-.5D0*C1)
      FI(2) = (VF(8)+AF(8))/4D0/SIN2TH*( HELP )
     +      - (VF(8)-AF(8))/4D0/SIN2TH*2D0*RT**2*C1
      GI(2) =  1D0/6D0/SIN2TH*( HELP - 2D0*RT**2*C1 )
      HELP = -1.5D0 + 12D0*C9 - 2D0*QSQR*( C10 - C8 ) + 4D0*QSQR*C7
      FI(3) = -  CW/4D0/SIN2TH/SW * HELP
      GI(3) = - 1D0/4D0/SIN2TH    * HELP
      HELP = RT**2/RMW**2*( -.75D0 + DLOG(RMW/RT) + 2D0*C4 -
     +       QSQR*( C5 - C3 ) )
      FI(4) = (VF(8)-AF(8))/4D0/SIN2TH*HELP -
     +        (VF(8)+AF(8))/4D0/SIN2TH*RT**4/RMW**2*C1
      GI(4) = 1D0/6D0/SIN2TH*( HELP - RT**4/RMW**2*C1 )
      HELP  = RT**2/RMW**2*( - .25D0 + 2D0*C9 )
      FI(5) = (SIN2TH-CW2)/8D0/SIN2TH/SW/CW*HELP
      GI(5) = - 1D0/4D0/SIN2TH*HELP
      FI(6) = - RT**2/4D0/SW/CW *C6
      GI(6) =   RT**2/4D0/SIN2TH*C6
      FI(7) = FI(6)
      GI(7) = GI(6)
      TOT1 = 0D0
      TOT2 = 0D0
      DO 10 I = 1 , 7
         TOT1 = TOT1 + FI(I)
         TOT2 = TOT2 + GI(I)
   10 CONTINUE
      FZL9 = TOT1
      FGL9 = TOT2
      END
 
      FUNCTION B0BAR(QSQR,RM1,RM2)
*     --------------
* eq. (C.15)
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 DCMPLX,B0BAR
      IF(DABS(RM1-RM2) .GT. 1D-10) THEN
         B0BAR = 1D0 - (RM1**2+RM2**2)/(RM1**2-RM2**2)*DLOG(RM1/RM2) +
     +           DCMPLX( FREAL(QSQR,RM1,RM2) , FIMAG(QSQR,RM1,RM2) )
      ELSE
         B0BAR = DCMPLX( FREAL(QSQR,RM1,RM2) , FIMAG(QSQR,RM1,RM2) )
      ENDIF
      END
 
      FUNCTION B1BAR(QSQR,RM1,RM2)
*     --------------
* eq. (C.16)
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 DCMPLX,B1BAR
      IF(DABS(RM1-RM2) .GT. 1D-10) THEN
         B1BAR = -.25D0 + RM1**2/(RM1**2-RM2**2)*DLOG(RM1/RM2) +
     +           ( RM2**2-RM1**2-QSQR )/2D0/QSQR*
     +            DCMPLX( FREAL(QSQR,RM1,RM2) , FIMAG(QSQR,RM1,RM2) )
      ELSE
         B1BAR = -.25D0 + .5D0 +
     +           ( RM2**2-RM1**2-QSQR )/2D0/QSQR*
     +            DCMPLX( FREAL(QSQR,RM1,RM2) , FIMAG(QSQR,RM1,RM2) )
      ENDIF
      END
 
      FUNCTION C0SCAL(QSQR,RM1,RM2,RMF)
*     ---------------
* The scalar 3 point function with equal external masses.
* eq. (5.10), (C.17)
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 X(1:3),Y(1:3,1:2),HELP,AA,BB,CC,DD,FF,TOT,C0SCAL
      COMPLEX*16 SPENCF,DCMPLX
      AA = DCMPLX(RMF**2,0D0)
      BB = DCMPLX(QSQR,0D0)
      CC = - BB
      DD = DCMPLX(RM1**2 - RM2**2 - RMF**2,0D0)
      FF = DCMPLX(RM2**2,-1D-15)
      ALPHA = 2D0*RMF**2/QSQR/( 1D0 + DSQRT(1D0-4D0*RMF**2/QSQR) )
      X(1) = - ( DD + 2D0*AA + CC*ALPHA )/(CC+2D0*ALPHA*BB)
      X(2) = - DD/( (1D0-ALPHA)*(CC+2D0*ALPHA*BB) )
      X(3) = DD/ALPHA/(CC+2D0*ALPHA*BB)
      HELP = CDSQRT( CC**2 - 4D0*BB*( AA + DD + FF ) )
      IF(DBLE(CC) .GE. 0D0) THEN
         Y(1,1) = ( - CC - HELP )/2D0/BB
         Y(1,2) = 4D0*BB*( AA + DD + FF )/(-CC-HELP)/2D0/BB
      ELSE
         Y(1,1) = 4D0*BB*( AA + DD + FF )/(-CC+HELP)/2D0/BB
         Y(1,2) = ( - CC + HELP )/2D0/BB
      ENDIF
      HELP = CDSQRT( DD**2 - 4D0*FF*( AA + BB + CC ) )
      IF(DBLE(DD) .GE. 0D0) THEN
         Y(2,1) = ( - DD - HELP )/2D0/AA
         Y(2,2) = 4D0*FF*( AA + BB + CC )/(-DD-HELP)/2D0/AA
      ELSE
         Y(2,1) = 4D0*FF*( AA + BB + CC )/(-DD+HELP)/2D0/AA
         Y(2,2) = ( - DD + HELP )/2D0/AA
      ENDIF
      Y(3,1) = Y(2,1)
      Y(3,2) = Y(2,2)
      TOT = 0D0
      DO 20 J = 1 , 2
         DO 10 L = 1 , 3
            TOT = TOT + (-1D0)**L*(SPENCF(  X(L)     /(X(L)-Y(L,J)) )
     +                            -SPENCF( (X(L)-1D0)/(X(L)-Y(L,J)) ) )
   10    CONTINUE
   20 CONTINUE
      C0SCAL = TOT / ( CC + 2D0*ALPHA*BB )
      END
 
      FUNCTION C1PLUS(QSQR,RM1,RM2,RMF)
*     ---------------
* eq. (C.9)
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 C0SCAL,HELP,B0BAR,C1PLUS
      HELP = DLOG(RM2/RM1) + B0BAR(QSQR,RM1,RM1) -
     +       B0BAR(RMF**2,RM1,RM2) + (RM2**2-RM1**2+RMF**2)*
     +       C0SCAL(QSQR,RM1,RM2,RMF)
      C1PLUS = HELP / ( 4D0*RMF**2 - QSQR )
      END
 
      FUNCTION C2ZERO(QSQR,RM1,RM2,RMF)
*     ---------------
* eq. (C.9)
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 B0BAR,C0SCAL,C1PLUS,C2ZERO
      C2ZERO = .25D0*( B0BAR(QSQR,RM1,RM1) + 1D0 ) +
     +         .5D0*( RM1**2 - RM2**2 - RMF**2 )*
     +         C1PLUS(QSQR,RM1,RM2,RMF) + .5D0*RM2**2*
     +         C0SCAL(QSQR,RM1,RM2,RMF)
      END
 
      FUNCTION C2PLUS(QSQR,RM1,RM2,RMF)
*     ---------------
* eq. (C.9)
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 B0BAR,B1BAR,C1PLUS,C2ZERO,HELP,C2PLUS
      HELP = .5D0*B0BAR(QSQR,RM1,RM1) + .5D0*( B1BAR(RMF**2,RM2,RM1)
     +       - .25D0 ) + ( RM2**2-RM1**2+RMF**2 )*
     +       C1PLUS(QSQR,RM1,RM2,RMF) - C2ZERO(QSQR,RM1,RM2,RMF)
      C2PLUS = HELP / ( 4D0*RMF**2 - QSQR )
      END
 
      FUNCTION C2MIN(QSQR,RM1,RM2,RMF)
*     --------------
* eq. (C.9)
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 HELP,B1BAR,C2ZERO,C2MIN
      HELP = -.5D0*( B1BAR(RMF**2,RM2,RM1) - .25D0 ) -
     +         C2ZERO(QSQR,RM1,RM2,RMF)
      C2MIN = HELP / QSQR
      END
 
      FUNCTION SPENCF(X)
*     ---------------
* Hans Kuijf, 1988
* SPENCF(X) calcs the complex spence-function, through mapping on
* the area where there is a quickly convergent series.
      REAL*8 PI
      COMPLEX*16 X, SPENC, SPENCF
      PI=4D0*DATAN(1D0)
* Map the x on the unit circle.
* But so that x is not in the neighbourhood of (1,0)
* ABS(Z)=-CDLOG(1D0-X) is always smaller than 1.10
* But (1.10)^19/(19!)*bernoulli(19)=2.7D-15
      IF (CDABS(1D0-X).LT.1D-13) THEN
        SPENCF=PI*PI/6D0
      ELSE IF (CDABS(1D0-X).LT.0.5D0) THEN
        SPENCF=PI*PI/6D0-CDLOG(1D0-X)*CDLOG(X)-SPENC(1D0-X)
      ELSE IF (CDABS(X).GT.1D0) THEN
        SPENCF=-PI*PI/6D0-0.5D0*CDLOG(-X)*CDLOG(-X)-SPENC(1D0/X)
      ELSE
        SPENCF = SPENC(X)
      END IF
      END
 
      FUNCTION SPENC(X)
      COMPLEX*16 X,SUM,Z,Z2,SPENC
      Z=-CDLOG(1D0-X)
      Z2=Z*Z
* Horner's rule for the powers z^3 through z^19
      SUM=43867D0/798D0
      SUM=SUM*Z2/342D0-3617D0/510D0
      SUM=SUM*Z2/272D0+7D0/6D0
      SUM=SUM*Z2/210D0-691D0/2730D0
      SUM=SUM*Z2/156D0+5D0/66D0
      SUM=SUM*Z2/110D0-1D0/30D0
      SUM=SUM*Z2/ 72D0+1D0/42D0
      SUM=SUM*Z2/ 42D0-1D0/30D0
      SUM=SUM*Z2/ 20D0+1D0/6D0
* The first three terms of the power series
      SUM=Z2*Z*SUM/6D0-0.25D0*Z2+Z
      SPENC=SUM
      END
 
* --- From here on: general purpose routines from other authors.
 
      FUNCTION DILOG(X)
*     --------------
      IMPLICIT REAL*8(A-H,O-Z)
      Z=-1.644934066848226D0
      IF(DABS(X-1.D0) .LE. 1.D-17) THEN
         DILOG=1.644934066848226D0
      ELSE
         IF(X.LE.-1.D0 .OR. X.GT.2.D0)THEN
            IF(X.GT.2.D0) Z=3.289868133696453D0
            T=1.D0/X
            S=-0.5D0
            Z=Z-0.5D0*DLOG(DABS(X))**2
         ELSEIF(X .LE. 0.5D0)THEN
            T=X
            S=0.5D0
            Z=0.D0
         ELSEIF(X .LE. 2.D0)THEN
            T=1.D0-X
            S=-0.5D0
            Z=1.644934066848226D0-DLOG(X)*DLOG(DABS(T))
         ENDIF
         Y=2.666666666666667D0*T+0.666666666666667D0
         B=      0.000000000000001D0
         A=Y*B  +0.000000000000004D0
         B=Y*A-B+0.000000000000011D0
         A=Y*B-A+0.000000000000037D0
         B=Y*A-B+0.000000000000121D0
         A=Y*B-A+0.000000000000398D0
         B=Y*A-B+0.000000000001312D0
         A=Y*B-A+0.000000000004342D0
         B=Y*A-B+0.000000000014437D0
         A=Y*B-A+0.000000000048274D0
         B=Y*A-B+0.000000000162421D0
         A=Y*B-A+0.000000000550291D0
         B=Y*A-B+0.000000001879117D0
         A=Y*B-A+0.000000006474338D0
         B=Y*A-B+0.000000022536705D0
         A=Y*B-A+0.000000079387055D0
         B=Y*A-B+0.000000283575385D0
         A=Y*B-A+0.000001029904264D0
         B=Y*A-B+0.000003816329463D0
         A=Y*B-A+0.000014496300557D0
         B=Y*A-B+0.000056817822718D0
         A=Y*B-A+0.000232002196094D0
         B=Y*A-B+0.001001627496164D0
         A=Y*B-A+0.004686361959447D0
         B=Y*A-B+0.024879322924228D0
         A=Y*B-A+0.166073032927855D0
         A=Y*A-B+1.935064300869969D0
         DILOG=S*T*(A-B)+Z
      ENDIF
      END
 
      FUNCTION HADRQQ(S)
C  HADRONIC IRREDUCIBLE QQ SELF-ENERGY: TRANSVERSE
C     parametrize the real part of the photon self energy function
C     by  a + b ln(1+C*|S|) , as in my 1981 TASSO note but using
C     updated values, extended using RQCD up to 100 TeV
C     for details see:
C     H.Burkhardt, F.Jegerlehner, G.Penso and C.Verzegnassi
C     in CERN Yellow Report on "Polarization at LEP" 1988
C               H.BURKHARDT, CERN/ALEPH, AUGUST 1988
C
      IMPLICIT REAL*8(A-H,O-Z)
      COMPLEX*16 HADRQQ
C
      DATA A1,B1,C1/   0.0   ,   0.00835,  1.0   /
      DATA A2,B2,C2/   0.0   ,   0.00238,  3.927 /
      DATA A3,B3,C3/ 0.00165 ,   0.00300,  1.0   /
      DATA A4,B4,C4/ 0.00221 ,   0.00293,  1.0   /
C
      DATA PI/3.141592653589793/,ALFAIN/137.0359895D0/,INIT/0/
C
      IF(INIT.EQ.0) THEN
        INIT=1
        ALFA=1./ALFAIN
      ENDIF
      T=ABS(S)
      IF(T.LT.0.3**2) THEN
        REPIAA=A1+B1*LOG(1.+C1*T)
      ELSEIF(T.LT.3.**2) THEN
        REPIAA=A2+B2*LOG(1.+C2*T)
      ELSEIF(T.LT.100.**2) THEN
        REPIAA=A3+B3*LOG(1.+C3*T)
      ELSE
        REPIAA=A4+B4*LOG(1.+C4*T)
      ENDIF
C     as imaginary part take -i alfa/3 Rexp
      HADRQQ=REPIAA-(0.,1.)*ALFA/3.*REXP(S)
      END
 
      FUNCTION REXP(S)
C  HADRONIC IRREDUCIBLE QQ SELF-ENERGY: IMAGINARY
      IMPLICIT REAL*8(A-H,O-Z)
C     continuum R = Ai+Bi W ,  this + resonances was used to calculate
C     the dispersion integral. Used in the imag part of HADRQQ
      PARAMETER (NDIM=18)
      DIMENSION WW(NDIM),RR(NDIM),AA(NDIM),BB(NDIM)
      DATA WW/1.,1.5,2.0,2.3,3.73,4.0,4.5,5.0,7.0,8.0,9.,10.55,
     .  12.,50.,100.,1000.,10 000.,100 000./
      DATA RR/0.,2.3,1.5,2.7,2.7,3.6,3.6,4.0,4.0,3.66,3.66,3.66,
     .   4.,3.87,3.84, 3.79, 3.76,    3.75/
      DATA INIT/0/
      IF(INIT.EQ.0) THEN
        INIT=1
C       calculate A,B from straight lines between R measurements
        BB(NDIM)=0.
        DO 4 I=1,NDIM
          IF(I.LT.NDIM) BB(I)=(RR(I)-RR(I+1))/(WW(I)-WW(I+1))
          AA(I)=RR(I)-BB(I)*WW(I)
    4   CONTINUE
      ENDIF
      REXP=0.D0
      IF(S.GT.0.D0) THEN
        W=DBLE(SQRT(S))
        IF(W.GT.WW(1)) THEN
          DO 2 I=1,NDIM
C           find out between which points of the RR array W is
            K=I
            IF(I.LT.NDIM) THEN
              IF(W.LT.WW(I+1)) GOTO 3
            ENDIF
    2     CONTINUE
    3     CONTINUE
          REXP=AA(K)+BB(K)*W
        ENDIF
      ENDIF
      END
 
      FUNCTION ZBRENT(FUNC,X1,X2,TOL)
*     ---------------
* Ref.: William H. Press et.al., Numerical Recipes,
*       Cambridge Univ. Press, 1987.
* Using Brent's method,  find the root of a function FUNC known to
* lie between X1 and X2. The root, returned as ZBRENT, will be refined
* untill its accuracy is TOL.
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER ( ITMAX=100 , EPS=3D-8 )
      EXTERNAL FUNC
      A  = X1
      B  = X2
      FA = FUNC(A)
      FB = FUNC(B)
      IF(FB*FA.GT.0D0) THEN
*       Root is not bracketed. Return the value closest to 0.
        WRITE(*,'(A)')' ZBRENT: the root is not bracketed.'
        ZBRENT = A
        IF ( ABS(FB) .LT. ABS(FA) ) ZBRENT = B
        RETURN
      ENDIF
      FC = FB
      DO 10 ITER=1,ITMAX
        IF(FB*FC.GT.0D0) THEN
*         Rename A,B,C and adjust bounding interval D.
          C = A
          FC= FA
          D = B-A
          E = D
        ENDIF
        IF(ABS(FC).LT.ABS(FB)) THEN
          A =B
          B =C
          C =A
          FA=FB
          FB=FC
          FC=FA
        ENDIF
        TOL1=2D0*EPS*ABS(B)+0.5D0*TOL
        XM=.5D0*(C-B)
*       Convergence check:
        IF(ABS(XM).LE.TOL1 .OR. DABS(FB).LT.1D-20) GOTO 999
        IF(ABS( E).GE.TOL1 .AND. ABS(FA).GT.ABS(FB)) THEN
*         Attempt inverse quadratic interpolation
          S=FB/FA
          IF(DABS(A-C).LT.1D-20) THEN
            P=2.*XM*S
            Q=1D0-S
          ELSE
            Q=FA/FC
            R=FB/FC
            P=S*(2.*XM*Q*(Q-R)-(B-A)*(R-1D0))
            Q=(Q-1D0)*(R-1D0)*(S-1D0)
          ENDIF
          IF(P.GT.0D0) Q=-Q
*         Check whether in bounds
          P=ABS(P)
          IF(2D0*P .LT. MIN(3D0*XM*Q-ABS(TOL1*Q),ABS(E*Q))) THEN
*           Accept interpolation
            E=D
            D=P/Q
          ELSE
*           Interpolation failed, use bisection.
            D=XM
            E=D
          ENDIF
        ELSE
*         Bounds decreasing too slowly, use bisection.
          D=XM
          E=D
        ENDIF
*       Move last guess to A
        A=B
        FA=FB
        IF(ABS(D) .GT. TOL1) THEN
*         Evaluate new trial root
          B=B+D
        ELSE
          B=B+SIGN(TOL1,XM)
        ENDIF
        FB=FUNC(B)
   10 CONTINUE
      WRITE(*,'(A,2G18.8)')' ZBRENT exceeding maximum iterations.',A,B
  999 ZBRENT=B
      END
 
      FUNCTION DGAUSS(F,A,B,EPS)
C.----------------------------------------------------------------------
C.
C.    GAUSS INTEGRAL OF THE FUNCTION F IN INTERVAL A,B
C.    LAST UPDATE: 12/03/87
C.
C.----------------------------------------------------------------------
      IMPLICIT REAL*8 (A-H,O-Z)
      DIMENSION W(12),X(12)
      EXTERNAL F
      DATA CONST/1.E-12/
      DATA W
     &/0.101228536290376, 0.222381034453374, 0.313706645877887,
     & 0.362683783378362, 0.027152459411754, 0.062253523938648,
     & 0.095158511682493, 0.124628971255534, 0.149595988816577,
     & 0.169156519395003, 0.182603415044924, 0.189450610455069/
      DATA X
     &/0.960289856497536, 0.796666477413627, 0.525532409916329,
     & 0.183434642495650, 0.989400934991650, 0.944575023073233,
     & 0.865631202387832, 0.755404408355003, 0.617876244402644,
     & 0.458016777657227, 0.281603550779259, 0.095012509837637/
C--
C--   INITIALISE
      DELTA=CONST*ABS(A-B)
      DGAUSS=0.
      AA=A
C--
C--   ITERATION LOOP
   10 Y=B-AA
C--
C--   EPSILON REACHED ??
      IF (ABS(Y).LE.DELTA) RETURN
   20 BB=AA+Y
      C1=0.5*(AA+BB)
      C2=C1-AA
      S8=0.
      S16=0.
      DO 30 I=1,4
         U=X(I)*C2
   30 S8=S8+W(I)*(F(C1+U)+F(C1-U))
      DO 40 I=5,12
         U=X(I)*C2
   40 S16=S16+W(I)*(F(C1+U)+F(C1-U))
      S8=S8*C2
      S16=S16*C2
      IF (ABS(S16-S8).GT.EPS*(1.0+ABS(S16))) GOTO 50
      DGAUSS=DGAUSS+S16
      AA=BB
      GOTO 10
   50 Y=0.5*Y
      IF (ABS(Y).GT.DELTA) GOTO 20
      WRITE (6,9000)
      DGAUSS=0.
      RETURN
 9000 FORMAT(1H ,'****** DGAUSS... TOO HIGH ACCURACY REQUIRED ******')
      END
 
      SUBROUTINE VEGAS(FXN,AVGI,SD,CHI2A)
C
C   SUBROUTINE PERFORMS N-DIMENSIONAL MONTE CARLO INTEG'N
C      - BY G.P. LEPAGE   SEPT 1976/(REV)APR 1978
C
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/BVEG1/XL(10),XU(10),ACC,NDIM,NCALL,ITMX,NPRN
      COMMON/BVEG2/XI(50,10),SI,SI2,SWGT,SCHI,NDO,IT
      DIMENSION D(50,10),DI(50,10),XIN(50),R(50),DX(10),DT(10),X(10)
     1   ,KG(10),IA(10)
      REAL*8 QRAN(10)
      DATA NDMX/50/,ALPH/1.5D0/,ONE/1D0/,MDS/1/
C
      NDO=1
      DO 1 J=1,NDIM
1     XI(1,J)=ONE
C
      ENTRY VEGAS1(FXN,AVGI,SD,CHI2A)
C         - INITIALIZES CUMMULATIVE VARIABLES, BUT NOT GRID
      IT=0
      SI=0.
      SI2=SI
      SWGT=SI
      SCHI=SI
C
      ENTRY VEGAS2(FXN,AVGI,SD,CHI2A)
C         - NO INITIALIZATION
      ND=NDMX
      NG=1
      IF(MDS.EQ.0) GO TO 2
      NG=(NCALL/2.)**(1./NDIM)
      MDS=1
      IF((2*NG-NDMX).LT.0) GO TO 2
      MDS=-1
      NPG=NG/NDMX+1
      ND=NG/NPG
      NG=NPG*ND
2     K=NG**NDIM
      NPG=NCALL/K
      IF(NPG.LT.2) NPG=2
      CALLS=NPG*K
      DXG=ONE/NG
      DV2G=(CALLS*DXG**NDIM)**2/NPG/NPG/(NPG-ONE)
      XND=ND
      NDM=ND-1
      DXG=DXG*XND
      XJAC=ONE/CALLS
      DO 3 J=1,NDIM
      DX(J)=XU(J)-XL(J)
3     XJAC=XJAC*DX(J)
C
C   REBIN PRESERVING BIN DENSITY
C
      IF(ND.EQ.NDO) GO TO 8
      RC=NDO/XND
      DO 7 J=1,NDIM
      K=0
      XN=0.
      DR=XN
      I=K
4     K=K+1
      DR=DR+ONE
      XO=XN
      XN=XI(K,J)
5     IF(RC.GT.DR) GO TO 4
      I=I+1
      DR=DR-RC
      XIN(I)=XN-(XN-XO)*DR
      IF(I.LT.NDM) GO TO 5
      DO 6 I=1,NDM
6     XI(I,J)=XIN(I)
7     XI(ND,J)=ONE
      NDO=ND
C
8     IF(NPRN.NE.0) WRITE(6,200) NDIM,CALLS,IT,ITMX,ACC,MDS,ND
     1                           ,(XL(J),XU(J),J=1,NDIM)
C
      ENTRY VEGAS3(FXN,AVGI,SD,CHI2A)
C         - MAIN INTEGRATION LOOP
9     IT=IT+1
      TI=0.
      TSI=TI
      DO 10 J=1,NDIM
      KG(J)=1
      DO 10 I=1,ND
      D(I,J)=TI
10    DI(I,J)=TI
C
11    FB=0.
      F2B=FB
      K=0
12    K=K+1
      CALL ARAN9(QRAN,NDIM)
      WGT=XJAC
      DO 15 J=1,NDIM
      XN=(KG(J)-QRAN(J))*DXG+ONE
      IA(J)=XN
      IF(IA(J).GT.1) GO TO 13
      XO=XI(IA(J),J)
      RC=(XN-IA(J))*XO
      GO TO 14
13    XO=XI(IA(J),J)-XI(IA(J)-1,J)
      RC=XI(IA(J)-1,J)+(XN-IA(J))*XO
14    X(J)=XL(J)+RC*DX(J)
15    WGT=WGT*XO*XND
C
      F=WGT
      F=F*FXN(X,WGT)
      F2=F*F
      FB=FB+F
      F2B=F2B+F2
      DO 16 J=1,NDIM
      DI(IA(J),J)=DI(IA(J),J)+F
16    IF(MDS.GE.0) D(IA(J),J)=D(IA(J),J)+F2
      IF(K.LT.NPG) GO TO 12
C
      F2B=DSQRT(F2B*NPG)
      F2B=(F2B-FB)*(F2B+FB)
      TI=TI+FB
      TSI=TSI+F2B
      IF(MDS.GE.0) GO TO 18
      DO 17 J=1,NDIM
17    D(IA(J),J)=D(IA(J),J)+F2B
18    KNEW=NDIM
19    KG(KNEW)=MOD(KG(KNEW),NG)+1
      IF(KG(KNEW).NE.1) GO TO 11
      KNEW=KNEW-1
      IF(KNEW.GT.0) GO TO 19
C
C   FINAL RESULTS FOR THIS ITERATION
C
      TSI=TSI*DV2G
      TI2=TI*TI
      WGT=TI2/TSI
      SI=SI+TI*WGT
      SI2=SI2+TI2
      SWGT=SWGT+WGT
      SCHI=SCHI+TI2*WGT
      AVGI=SI/SWGT
      SD=SWGT*IT/SI2
      CHI2A=SD*(SCHI/SWGT-AVGI*AVGI)/(IT-.999)
      SD=DSQRT(ONE/SD)
C
      IF(NPRN.EQ.0) GO TO 21
      TSI=DSQRT(TSI)
      WRITE(6,201) IT,TI,TSI,AVGI,SD,CHI2A
      IF(NPRN.GE.0) GO TO 21
      DO 20 J=1,NDIM
20    WRITE(6,202) J,(XI(I,J),DI(I,J),D(I,J),I=1,ND)
C
C   REFINE GRID
C
21    DO 23 J=1,NDIM
      XO=D(1,J)
      XN=D(2,J)
      D(1,J)=(XO+XN)/2.
      DT(J)=D(1,J)
      DO 22 I=2,NDM
      D(I,J)=XO+XN
      XO=XN
      XN=D(I+1,J)
      D(I,J)=(D(I,J)+XN)/3.
22    DT(J)=DT(J)+D(I,J)
      D(ND,J)=(XN+XO)/2.
23    DT(J)=DT(J)+D(ND,J)
C
      DO 28 J=1,NDIM
      RC=0.
      DO 24 I=1,ND
      R(I)=0.
      IF(D(I,J).LE.0.) GO TO 24
      XO=DT(J)/D(I,J)
      R(I)=((XO-ONE)/XO/DLOG(XO))**ALPH
24    RC=RC+R(I)
      RC=RC/XND
      K=0
      XN=0.
      DR=XN
      I=K
25    K=K+1
      DR=DR+R(K)
      XO=XN
      XN=XI(K,J)
26    IF(RC.GT.DR) GO TO 25
      I=I+1
      DR=DR-RC
      XIN(I)=XN-(XN-XO)*DR/R(K)
      IF(I.LT.NDM) GO TO 26
      DO 27 I=1,NDM
27    XI(I,J)=XIN(I)
28    XI(ND,J)=ONE
C
      IF(IT.LT.ITMX.AND.ACC*DABS(AVGI).LT.SD) GO TO 9
200   FORMAT('0INPUT PARAMETERS FOR VEGAS:  NDIM=',I3,'  NCALL=',F8.0
     1    /28X,'  IT=',I5,'  ITMX=',I5/28X,'  ACC=',G9.3
     2    /28X,'  MDS=',I3,'   ND=',I4/28X,'  (XL,XU)=',
     3    (T40,'( ',G12.6,' , ',G12.6,' )'))
201   FORMAT(///' INTEGRATION BY VEGAS' / '0ITERATION NO.',I3,
     1    ':   INTEGRAL =',G14.8/21X,'STD DEV  =',G10.4 /
     2    ' ACCUMULATED RESULTS:   INTEGRAL =',G14.8 /
     3    24X,'STD DEV  =',G10.4 / 24X,'CHI**2 PER IT''N =',G10.4)
202   FORMAT('0DATA FOR AXIS',I2 / ' ',6X,'X',7X,'  DELT I  ',
     1    2X,' CONV''CE  ',11X,'X',7X,'  DELT I  ',2X,' CONV''CE  '
     2   ,11X,'X',7X,'  DELT I  ',2X,' CONV''CE  ' /
     2    (' ',3G12.4,5X,3G12.4,5X,3G12.4))
      RETURN
      END
      SUBROUTINE SAVE(NDIM)
      IMPLICIT REAL*8(A-H,O-Z)
      COMMON/BVEG2/XI(50,10),SI,SI2,SWGT,SCHI,NDO,IT
C
C   STORES VEGAS DATA (UNIT 7) FOR LATER RE-INITIALIZATION
C
      WRITE(7,200) NDO,IT,SI,SI2,SWGT,SCHI,
     1             ((XI(I,J),I=1,NDO),J=1,NDIM)
      RETURN
      ENTRY RESTR(NDIM)
C
C   ENTERS INITIALIZATION DATA FOR VEGAS
C
      READ(7,200) NDO,IT,SI,SI2,SWGT,SCHI,
     1            ((XI(I,J),I=1,NDO),J=1,NDIM)
200   FORMAT(2I8,4Z16/(5Z16))
      RETURN
      END
 
      SUBROUTINE ARAN9(QRAN,NDIM)
      REAL*8 QRAN(10)
      DO 1 I=1,NDIM
         CALL RANMAR(QRAN(I))
    1 CONTINUE
      RETURN
      END
 
      BLOCK DATA VEGASS
      IMPLICIT REAL*8(A-H,O-Z)
      PARAMETER( IXDIM = 10 )
      COMMON/BVEG1/XL(10),XU(10),ACC,NDIM,NCALL,ITMX,NPRN
      COMMON/BVEG2/XI(50,10),SI,SI2,SWGT,SCHI,NDO,IT
      DATA NCALL/5000/ITMX/5/NPRN/5/ACC/-1D0/XL/IXDIM*0D0/XU/IXDIM*1D0/
      END
