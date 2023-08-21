      subroutine kinchce(IGCL,MODE,AMAW,GAMMW,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      dimension bp1(4),bp2(4),bp3(4),bp4(4)
      IF (IGCL.eq.10) then
        IF(MODE.EQ.0) THEN      
         CALL KINbre(1,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
        elseif(MODE.EQ.1) THEN
         CALL rKINbre(1,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
        else
         write(*,*) 'kinche; wrong mode=',mode
         stop
        endif
      elseIF (IGCL.eq.9) then
        IF(MODE.EQ.0) THEN      
         CALL KINT(1,AMAW,GAMMW,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
        elseif(MODE.EQ.1) THEN
         CALL rKINT(1,AMAW,GAMMW,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
        else
         write(*,*) 'kinche; wrong mode=',mode
         stop
        endif
      elseIF (IGCL.eq.8) then
        IF(MODE.EQ.0) THEN      
         CALL KINT(0,AMAW,GAMMW,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
        elseif(MODE.EQ.1) THEN
         CALL rKINT(0,AMAW,GAMMW,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
        else
         write(*,*) 'kinche; wrong mode=',mode
         stop
        endif
      elseIF (IGCL.eq.7) then
        IF(MODE.EQ.0) THEN      
         CALL KINbre(0,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
        elseif(MODE.EQ.1) THEN
         CALL rKINbre(0,pr,SPRIM,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
        else
         write(*,*) 'kinche; wrong mode=',mode
         stop
        endif
      elseIF (IGCL.lt.7) then
        svar=pr
        call spaceold(mode,igcl,AMAW,GAMMW,svar,sprim,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
      else
       write(*,*) 'kinche; wrong igcl=',igcl 
       stop
      endif
      end

      subroutine spaceold(mode,itype,amx,gamx,svar,sprim,fakir,
     $     bP1,amd1,bP2,amd2,bP3,amd3,bP4,amd4)
********************************************************
! ================================================================
! mode=0                                                         =
!        generates 4-momenta accordingly to itype of generation, =
!        writes them down into / bormom/.                        =
!        calculates jacobian (out from 4-momenta from / bormom/) =
! mode=1                                                         =
!        calculates jacobian (out from 4-momentafrom / bormom/)  =
!        for itype generation branch                             =
! ================================================================
      implicit DOUBLE PRECISION (a-h,o-z)
! we take just amel from this common
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF 
      COMMON / MATPAR / PI,CEULER
!!! / angles / output SOLELY for external tests (tests102.f) !!!
      COMMON / ANGLES / COSTHE,PHI,COSDE1,PHI1,COSDE2,PHI2

      save  / WEKING /,/ MATPAR /
      save  / ANGLES /
      dimension amdet(4)
      dimension bq1(4),bq2(4),bp1(4),bp2(4),bp3(4),bp4(4)
      common /nevik/ nevtru,ifprint
      save /nevik/
      SAVE
!!!
!     itype=1 WW kinematics with appropriate breit wigners to be set,
!     itype=2    kinematics with flatened breit wigners to be set, obsolete
!     itype=3    kinematics with appropriate breit wigners to be set, ini pairs
!     itype=4-6  kinematics with approp. b-wigners to set, t-chan. Obsolete?
! 
!
!#############################################################
!    GENERAL INITIALIZATION:                                 #
!    SETTING PRESAMPLER PARAMETERS AND MASSES IN ORDER       #
!    FOR PARTICULAR >ITYPE< SLOT-SPACE                       #
!    most of ITYPES differ by order of 4-vectors only        #
!#############################################################
       amdet(1)=amd1
       amdet(2)=amd2
       amdet(3)=amd3
       amdet(4)=amd4


      IF (MODE.EQ.1) THEN 
!##############################################
! INITIALIZATION FOR RECALCULATION MODE:      #
!  ANGLES AND S-i's FROM FOUR VECTORS         #
!##############################################
      if     (itype.le.3 )then
          call invkin(ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $                  amwmn,amwpn,  bp1,bp2,bp3,bp4)
      elseif (itype.le.6 )then
        call invkin(ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $                  amwmn,amwpn,  bp1,bp2,bp3,bp4)
      endif
!
      s1=amwmn**2
      s2=amwpn**2
      ENDIF

!##############################################
! BASIC PART:                                 #
! MODE=0 GENERATION AND JACOBIAN CALCULATION  #
! MODE=1 JACOBIAN CALCULATION ONLY            #
!##############################################
!... s1,s2 subspace
      if (itype.eq.1) then
       CALL RES2GN(mode,SVAR,SPRIM,AMx,GAMx,amdet,S1,S2,SSCRU)
      else
       CALL RES3GN(mode,itype,SVAR,SPRIM,AMx,GAMx,amdet,S1,S2,SSCRU)
      endif
      if (ifprint.eq.1) then
      write(*,*) 'invariants old-t ',s1,s2,sscru
      endif
      if (mode.eq.10.and.itype.eq.3) then
        write(*,*) 'itype=',itype
        write(*,*) 'invariants old-t ',s1,s2,sscru
      endif 
!
! rejection
      if(mode.eq.0.and.sscru.eq.0d0)  then
!-- short-out ... 
         fakir=0D0
         return
      endif

! lambda factors, can be moved to res2/3-s or even out.
      x1=s1/sprim
      x2=s2/sprim
      bmain=sqrt( (1-x1-x2)**2 - 4*x1*x2 )
      xwm1=amdet(1)**2/s1
      xwm2=amdet(2)**2/s1
      bwm=sqrt( (1-xwm1-xwm2)**2 - 4*xwm1*xwm2 )
      xwp1=amdet(3)**2/s2
      xwp2=amdet(4)**2/s2
      bwp=sqrt( (1-xwp1-xwp2)**2 - 4*xwp1*xwp2 )
      wjac=bmain*bwp*bwm

!... production angles
      if (itype.eq.1) then
       CALL cospro(mode,sprim,s1,s2,ctn,fin,xccos)
      else
       CALL cosprozz(mode,sprim,s1,s2,ctn,fin,xccos)
      endif
      if (ifprint.eq.1) then
      write(*,*) 'prod',ctn,xccos
      endif

! decay angles
      if (itype.le.3 )then
       ifl=1
       if (itype.eq.3 ) ifl=0
       CALL cosdecc(mode,ifl,sprim,ct1n,fi1n,xccos1)
       CALL cosdecc(mode,ifl,sprim,ct2n,fi2n,xccos2)

      else
       if (s1.gt.s2) then
        ifak=1
        if (ctn.lt.0d0) ifak=-1

        CALL cosdec_t(mode, 1,svar,sprim,s1,s2,ctn,fin,amel,
     @              amdet(1),amdet(2),ct1n,fi1n,xccos1)
        CALL cosdecc(mode,0,sprim,ct2n,fi2n,xccos2)
       else
 2        ifak=1
        if (ctn.lt.0d0) ifak=-1
        CALL cosdecc(mode,ifak,sprim,ct1n,fi1n,xccos1)
        CALL cosdec_t(mode,-1,svar,sprim,s2,s1,ctn,fin,amel,
     @              amdet(3),amdet(4),ct2n,fi2n,xccos2)
       endif
      endif
      if (ifprint.eq.1) then
      write(*,*) 'dec1 ',ct1n,xccos1
      write(*,*) 'dec2 ',ct2n,xccos2
      endif

!... WE CALCULATE OVERALL JACOBIAN ...
      fak= 1D0/32D0*xccos1*xccos2*xccos*sscru*wjac
!...  EN_pi=(2*pi)**4/(2*(2*PI)**3)**(r;r=4) 
      ENPI=(2*PI)**4/(2*(2*PI)**3)**4
      fakir=fak*ENPI
      if (ifprint.eq.1) then
      write(*,*) 'fakir ',fakir
      endif



      if (mode.eq.0) then 
!##############################################
!  KONSTRUCT FINAL 4-VECTORS etc:             #
!  MODE=0 ONLY                                #
!##############################################
!
      if(itype.le.3) then
       CALL kineww(sprim,ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $           sqrt(s1),sqrt(s2),amdet,bq1,bq2,bp1,bp2,bp3,bp4)
!
      elseif(itype.le.6) then
       CALL kineww(sprim,ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $           sqrt(s1),sqrt(s2),amdet,bq1,bq2,bp1,bp2,bp3,bp4)
      else
       write(6,*) 'spacegen: wrong itype=',itype
       stop
      endif        
      endif   

! security check
      if (fak.eq.0d0) then
       write(*,*) 'spacegen: we have troubles; fakir=',
     $  xccos1,xccos2,xccos,sscru,wjac
       write(*,*) 'please contact Z.Was '
       write(*,*) 'this effect is irrelevant or serious ...'
       write(*,*) 'spacegen: itype=',itype,' mode=',mode
       write(*,*) 'division by zero will be protected'
       write(*,*) 'bp1=',bp1
       write(*,*) 'bp2=',bp2
       write(*,*) 'bp3=',bp3
       write(*,*) 'bp4=',bp4
       write(*,*) amdet
       write(*,*) sqrt(s1),sqrt(s2)
!... WE Re-CALCULATE OVERALL JACOBIAN ...
!... this event was outside phase space, the only justifiable config.
!... is if s1 or s2 is just under threshold defined by electron and neutrino
!... masses
      sscru=1d-40
      fak= 1D0/32D0*xccos1*xccos2*xccos*sscru*wjac
!...  EN_pi=(2*pi)**4/(2*(2*PI)**3)**(r;r=4) 
      ENPI=(2*PI)**4/(2*(2*PI)**3)**4
      fakir=fak*ENPI
      endif

      end

      SUBROUTINE res3gn(mode,itype,svar,sprim,rmas,rgam,amdec,s1,s2,wt)
!     ***************************************************************
! Generation of ds_1ds_2 distribution within phase space boundaries
! using weighted (pre-sampled) events
!---------------------
! note:
! so far generation is within theta_crude and fine tuning is added at the
! end. For non-acceptable events weight is set to zero.
!---------------------
! breit-wigners pre-samplers in both s_1 and s_2 channels are set.
! total volume 'prnorm' ( S(s') defined in formula 31 of koralw 1.02 manual) 
! is calculated including additional W(s_1)*W(s_2) factor 
! (see koralw 1.02 manual). To obtain proper ds_1ds_2 distribution
! weight wt=prnorm/W(s_1)/W(s_2) must be included, and this will help later
! cancelling singularities of matrix element
! 
! note: both resonances have the same mass distribution function
!         svar    - max sprim
!         sprim   - actual s
!         rmas    - central value of a resonance mass distribution
!         rgam    - width of a resonance
! OUTPUT: s1, s2  - svar's of two resonances
!         wt      - weight
! for mode=1
! INPUT:  s1, s2  - no generation,  just calculation of weight. 
!
! Written by: M. Skrzypek            date: 2/16/95
! Last update: 5/5/96                  by: Z. Was
!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / matpar / pi,ceuler
! This common contains parameters of non-established particles such as higgs
! Which need to be used by pre-sampler (to be activated by dipswitch IHIG
      COMMON / WEKIN3 / AMHIG,GAMHIG,IHIG
      SAVE / matpar /,/ WEKIN3 /     
      DOUBLE PRECISION AMDEC(4),amd(4)
      DOUBLE PRECISION drvec(100)
      SAVE
      do k=1,4
       amd(k)=amdec(k)
       if (amd(k).lt.0.0005d0) amd(k)=0.000511d0
      enddo
      POLD=0.3
!      if (itype.eq.3) pold=.7
      proa=0.5
      if (itype.eq.5) proa=0.2d0
      if (itype.eq.6) proa=0.8d0
      ALP2=ATAN((sprim-rmas**2)/rmas/rgam)
      ALP1=ATAN(((amd(1)+amd(2))**2-rmas**2)/rmas/rgam)
      BLP2=ATAN((sprim-rmas**2)/rmas/rgam)
      BLP1=ATAN(((amd(3)+amd(4))**2-rmas**2)/rmas/rgam)
      IF (IHIG.EQ.1) THEN
       CLP2=ATAN((sprim-AMHIG**2)/AMHIG/GAMHIG)
       CLP1=ATAN(((amd(1)+amd(2))**2-AMHIG**2)/AMHIG/GAMHIG)
       DLP2=ATAN((sprim-AMHIG**2)/AMHIG/GAMHIG)
       DLP1=ATAN(((amd(3)+amd(4))**2-AMHIG**2)/AMHIG/GAMHIG)
       PROB1=1D0/3D0
       PROB2=2D0/3D0
       PROB3=2D0/3D0
       PROB4=1D0
      ELSE
       CLP2=ATAN((sprim-rmas**2)/rmas/rgam)
       CLP1=ATAN(((amd(1)+amd(2))**2-rmas**2)/rmas/rgam)
       DLP2=ATAN((sprim-rmas**2)/rmas/rgam)
       DLP1=ATAN(((amd(3)+amd(4))**2-rmas**2)/rmas/rgam)
       PROB1=1D0/2D0
       PROB2=1D0/2D0
       PROB3=1D0/2D0
       PROB4=1D0
      ENDIF
      biglog1=log(sprim/(amd(1)+amd(2))**2)
      biglog2=log(sprim/(amd(3)+amd(4))**2)
!
!     ====================
      if (mode.ne.1) then 
!     ====================
!
 10   call varran(drvec,7)
      r1=drvec(1)
      r2=drvec(2)
      r3=drvec(3)
      r4=drvec(4)
      r5=drvec(5)
      r6=drvec(6)
      r7=drvec(7)
      IF(r6.lt.POLD) THEN
!! ########################################
      if(r3.lt.PROB1) then      
        ALP=ALP1+R1*(ALP2-ALP1)
        s1=rmas**2+rmas*rgam*TAN(ALP)
      elseif(r3.lt.PROB2) then  
        CLP=CLP1+R1*(CLP2-CLP1)
        s1=AMHIG**2+AMHIG*GAMHIG*TAN(DLP)    
      elseif(r3.lt.PROB3) then      
        s1=(sprim-(amd(1)+amd(2))**2)*r1+(amd(1)+amd(2))**2 
      else
        s1=(amd(1)+amd(2))**2*exp(r1*biglog1)
      endif
      if(r4.lt.PROB1) then   
        ALP=BLP1+R2*(BLP2-BLP1)
        s2=rmas**2+rmas*rgam*TAN(ALP)
      elseif(r4.lt.PROB2) then  
        DLP=DLP1+R2*(DLP2-DLP1)
        s2=AMHIG**2+AMHIG*GAMHIG*TAN(DLP) 
      elseif(r4.lt.PROB3) then   
        s2=(sprim-(amd(3)+amd(4))**2)*r2+(amd(3)+amd(4))**2
      else
        s2=(amd(3)+amd(4))**2*exp(r2*biglog2)
      endif
      ELSE ! PNEW !!!
      if(r7.lt.proa) then
      xx=4*(amd(1)+amd(2))**2/sprim
      beta=sqrt(1d0-4*(amd(1)+amd(2))**2/sprim)
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
        u=(log((1D0+beta)/4D0))**2 +xlog*xlog1*r1
        costhe=-1D0/beta*(4D0*EXP(-SQRT(u))-1)
        s1=(sprim-(amd(1)+amd(2))**2)*(1d0-costhe)/2
     $    +(amd(1)+amd(2))**2
!        write(*,*) s1
!        biglog1=log(sprim/(amd(1)+amd(2))**2)
!        s1=(amd(1)+amd(2))**2*exp(r1*biglog1)
        spri1=(sqrt(sprim)-sqrt(s1))**2
        biglo1=log(spri1/(amd(3)+amd(4))**2)
        if(r7.lt.proa/2) then
         s2=(amd(3)+amd(4))**2*exp(r2*biglo1)
         s2=spri1+(amd(3)+amd(4))**2-s2
        else
         s2=(spri1-(amd(3)+amd(4))**2)*r2+(amd(3)+amd(4))**2
        endif

      else
      xx=4*(amd(3)+amd(4))**2/sprim
      beta=sqrt(1d0-4*(amd(3)+amd(4))**2/sprim)
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
        u=(log((1D0+beta)/4D0))**2 +xlog*xlog1*r1
        costhe=-1D0/beta*(4D0*EXP(-SQRT(u))-1)
        s2=(sprim-(amd(3)+amd(4))**2)*(1d0-costhe)/2
     $    +(amd(3)+amd(4))**2

!        biglog2=log(sprim/(amd(3)+amd(4))**2)
!        s2=(amd(3)+amd(4))**2*exp(r1*biglog2)
        spri2=(sqrt(sprim)-sqrt(s2))**2
        biglo2=log(spri2/(amd(1)+amd(2))**2)
        if(r7.gt.proa+(1-proa)/2) then
         s1=(amd(1)+amd(2))**2*exp(r2*biglo2)
         s1=spri2+(amd(1)+amd(2))**2-s1
        else
         s1=(spri2-(amd(1)+amd(2))**2)*r2+(amd(1)+amd(2))**2
        endif

      endif
      ENDIF
!     =====
      endif
!     =====
      ph1c=(sprim-(amdec(1)+amdec(2))**2)
      ph2c=(sprim-(amdec(3)+amdec(4))**2)
!
      PH1a=((s1-rmas**2)**2+(rmas*rgam)**2)/(rmas*rgam)
      PH1a=PH1a*(ALP2-ALP1)
      PH2a=((s2-rmas**2)**2+(rmas*rgam)**2)/(rmas*rgam)
      PH2a=PH2a*(BLP2-BLP1)
!
      IF (IHIG.EQ.1) THEN
       PH1b=((s1-AMHIG**2)**2+(AMHIG*GAMHIG)**2)/(AMHIG*GAMHIG)
       PH1b=PH1b*(CLP2-CLP1)
       PH2b=((s2-AMHIG**2)**2+(AMHIG*GAMHIG)**2)/(AMHIG*GAMHIG)
       PH2b=PH2b*(DLP2-DLP1)
      ELSE
       PH1b=((s1-rmas**2)**2+(rmas*rgam)**2)/(rmas*rgam)
       PH1b=PH1b*(CLP2-CLP1)
       PH2b=((s2-rmas**2)**2+(rmas*rgam)**2)/(rmas*rgam)
       PH2b=PH2b*(DLP2-DLP1)
      ENDIF
!      
      ph1d=s1*biglog1
      ph2d=s2*biglog2
!!!
      ph1=1/( PROB1       /ph1a+(PROB2-PROB1)/ph1b
     $      +(PROB3-PROB2)/ph1c+(PROB4-PROB3)/ph1d)
      ph2=1/( PROB1       /ph2a+(PROB2-PROB1)/ph2b
     $      +(PROB3-PROB2)/ph2c+(PROB4-PROB3)/ph2d)
!      ph1=3/(1d0/ph1a+1d0/ph1b+1d0/ph1c)
!      ph2=3/(1d0/ph2a+1d0/ph2b+1d0/ph2c)
      prn=ph1*ph2

!####################################################
       biglog1=log(sprim/(amd(1)+amd(2))**2)
       ph1cc=s1*biglog1                           ! s1 -->
c to jest zlosliwe swinstwo !! rozklad musi byc 1/E*ln(s1/E**2)
c co powoduje, ze rozbieznosc jest nieco `przydymiona'.
      xx=4*(amd(1)+amd(2))**2/sprim
      beta=sqrt(1d0-4*(amd(1)+amd(2))**2/sprim)
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
      cost=1-2*(s1-(amd(1)+amd(2))**2)/(sprim-(amd(1)+amd(2))**2)
      xccos=1d0/2d0*beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-cost)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-cost))))
       ph1cc=(sprim-(amd(1)+amd(2))**2)/xccos/2
       spri1=(sqrt(sprim)-sqrt(s1))**2
       biglo1=log(spri1/(amd(3)+amd(4))**2)
       ph2cc=1/(0.5/((spri1+(amd(3)+amd(4))**2-s2)*biglo1)
     $         +0.5/(spri1-(amd(3)+amd(4))**2))           !    --> s2 

       biglog2=log(sprim/(amd(3)+amd(4))**2)
       ph1cd=s2*biglog2                           ! s2 --> 
      xx=4*(amd(3)+amd(4))**2/sprim
      beta=sqrt(1d0-4*(amd(3)+amd(4))**2/sprim)
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
      cost=1-2*(s2-(amd(3)+amd(4))**2)/(sprim-(amd(3)+amd(4))**2)
      xccos=1d0/2d0*beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-cost)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-cost))))
       ph1cd=(sprim-(amd(3)+amd(4))**2)/xccos/2

       spri2=(sqrt(sprim)-sqrt(s2))**2
       biglo2=log(spri2/(amd(1)+amd(2))**2)
       ph2cd=1/(0.5/((spri2+(amd(1)+amd(2))**2-s1)*biglo2) 
     $         +0.5/(spri2-(amd(1)+amd(2))**2))           !    --> s1 

       prx=1d0/(proa/(ph1cc*ph2cc)+(1d0-proa)/(ph1cd*ph2cd))
      prnorm=1d0/(pold/prn+(1d0-pold)/prx)
      wt=prnorm
!####################################################
! thresholds
      IF(sqrt(s1)+sqrt(s2).gt.sqrt(sprim)) THEN
        wt=0d0
      ENDIF
!-- check thresholds on decays
      IF(amdec(1)+amdec(2).gt.sqrt(s1)) THEN
        wt=0D0
      ENDIF
      IF(amdec(3)+amdec(4).gt.sqrt(s2)) THEN
        wt=0D0
      ENDIF
      if(mode.eq.1.and.wt.eq.0d0) then
      write(*,*) 'RES3-mode=1 vol=',ph1,ph2
      write(*,*) sqrt(s1),'+',sqrt(s2),'.gt.',sqrt(sprim)
      write(*,*) amdec
      wt=prnorm
      endif
      END

      subroutine invx(s,sprim,ct,fi,ct1,fi1,ct2,fi2,
     $                  amwm,amwp,amdec,  q1,q2,p1,p2,p3,p4)
      implicit DOUBLE PRECISION (a-h,o-z)
! ... this routine is for tests of invkin only. It is nat called at all
      COMMON / cms_eff_momdec /
     $      effbeam1(4),effbeam2(4),bp1(4),bp2(4),bp3(4),bp4(4)

      dimension amdec(4),q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      dimension rp1(4),rp2(4),rp3(4),rp4(4)
c to activate dumps KINDMP=1
      KINDMP=0

      IF(KINDMP.EQ.1) THEN
      write(*,*) '================================================='
      write(*,*) '============     begin     ======================'
      write(*,*) 'we want:  ct2=', ct2,'  fi2=', fi2
      write(*,*) 'we want:  ct1=', ct1,'  fi1=', fi1
      write(*,*) 'we want:   ct=',  ct,'   fi=', fi
      write(*,*) 'we want: amwm=',amwm,' amwp=',amwp
      write(*,*) '-------------------------------------------------'
      endif
!!!
      do k=1,4
      rp1(k)=bp1(k)
      rp2(k)=bp4(k)
      rp3(k)=bp3(k)
      rp4(k)=bp2(k)
      enddo
      call invkintt(ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $                  amwmn,amwpn,  rp1,rp2,rp3,rp4)
!
      IF(KINDMP.EQ.1) THEN
      write(*,*) 'we  get:  ct2=', ct2n,'  fi2=', fi2n
      write(*,*) 'we  get:  ct1=', ct1n,'  fi1=', fi1n
      write(*,*) 'we  get:   ct=',  ctn,'   fi=', fin
      write(*,*) 'we  get: amwm=',amwmn,' amwp=',amwpn
      write(*,*) '============       end     ======================'
      write(*,*) '================================================='
      endif
      end

      SUBROUTINE RKINBRE(ift,ple,
     $ SVAR,DGAMT,PN,amnuta,PIM2,AMP1,PIM1,AMP2,PIPL,AMP3)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION  PT(4),PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4)
      DIMENSION  PR(4),PBST(4)
      common /nevik/ nevtru,ifprint
      save /nevik/
      SAVE
      DATA PI /3.141592653589793238462643D0/
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.0D0*Y*Z))


C
C FOUR BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)
      PHSPAC=1.D0/2**17/PI**8
      amtau=sqrt(svar)
      amro=100
      gamro=50
C
      DO K=1,4
        PAA(K)=PIM1(K)+PIM2(K)+PIPL(K)
        PR(K) =PIM1(K)+PIPL(K)
        pt(k) =PIM1(K)-PIM2(K)+PIPL(K)
      ENDDO
C
C.. MASS OF two
        AMS1=(AMP2+AMP3)**2
        AMS2=(AMTAU-AMNUTA-amp1)**2
        AM2SQ=dmas2(PR)
        AM2 =SQRT(AM2SQ)
!       PHSPAC=PHSPAC*(AMS2-AMS1)
        B=LOG(AMS1)
        A=LOG(AMS2)
        AM2 =SQRT(AM2SQ)
!        PHSPAC=PHSPAC*AM2SQ*(A-B)
C.. MASS OF two
        prob1=.3
        prob2=.3
        PROB3=.0
        PROB4=.4
        AMS1=(AMP2+AMP3)**2
        AMS2=(AMTAU-AMNUTA-amp1)**2
        AM2SQ=dmas2(PR)
        AM2 =SQRT(AM2SQ)
        XJ1=(AMS2-AMS1)
C PHASE SPACE WITH SAMPLING FOR RHO RESONANCE
         B=LOG(AMS1)
         A=LOG(AMS2)
        xj2=AM2SQ*(A-B)

      ALP1=ATAN((AMS1-AMRO**2)/AMRO/GAMRO)
      ALP2=ATAN((AMS2-AMRO**2)/AMRO/GAMRO)
      xj3=((AM2SQ-AMRO**2)**2+(AMRO*GAMRO)**2)/(AMRO*GAMRO)
      xj3=xj3*(ALP2-ALP1)

        n=1
        xj4=am2SQ**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)
!      write(*,*) 't',phspac,xj1,xj2,xj3,xj4,prob1,prob2,prob3,prob4
        PHSPAC=PHSPAC/(PROB1/XJ1+PROB2/XJ2+PROB3/XJ3+PROB4/XJ4)
C.. mass of three
        AMS1=(AMP1+am2)**2
        AMS2=(AMTAU-AMNUTA)**2
        AM3SQ =dmas2(PAA)
        AM3 =SQRT(AM3SQ)
!        PHSPAC=PHSPAC*(AMS2-AMS1)
        B=LOG(AMS1)
        A=LOG(AMS2)
!!        PHSPAC=PHSPAC*AM3SQ*(A-B)
        prbam3=1d0
        AM3SQb =ams2-am3sq+ams1
        PHSPAC=PHSPAC/(prbam3/(AM3SQ*(A-B))+(1-prbam3)/(AM3SQb*(A-B)))
        ENQ1=(AM2SQ-AMP2**2+AMP3**2)/(2*AM2)
        PPPI=SQRT(ABS(ENQ1**2-AMP3**2))
        PHSPAC=PHSPAC*(4*PI)*(2*PPPI/AM2)
C
        PR4=1.D0/(2*AM3)*(AM3**2+AM2**2-AMP1**2)
        PR3= SQRT(ABS(PR4**2-AM2**2))
        PHSPAC=PHSPAC*(2*PR3/AM3)

        PAA4=1.D0/(2*AMTAU)*(AMTAU**2-AMNUTA**2+AM3**2)
        PAA3= SQRT(ABS(PAA(4)**2-AM3**2))
        PHSPAC=PHSPAC*(4*PI)*(2*PAA3/AMTAU)
        pim24=1.D0/(2*AM3)*(AM3**2-AM2**2+AMP1**2)
!... versor of two in three restframe
      PNPAA=dot(pn,paa)
      DO K=1,4
         pt(k) =( (PIM1(K)-PIM2(K)+PIPL(K))
     $           -PAA(K)*(pr4-pim24)/am3   )/2/PR3
         PBST(K)=PN(K)-PAA(K)*PNPAA/am3**2
      ENDDO
        cost=dot(pt,pbst)/sqrt(-dmas2(pbst))
cc ms 03.07.97        if(sqrt(-dmas2(pt)).gt.1d0) cost=cost/sqrt(-dmas2(pt))
        if(sqrt(dabs(-dmas2(pt))).gt.1d0) 
     $       cost=cost/sqrt(dabs(-dmas2(pt)))
! m.s. 8/24/98 beg
        IF(cost.LT.-1d0) cost=-1d0
        IF(cost.GT. 1d0) cost= 1d0
! m.s. 8/24/98 end
        thet=acos(cost)
        costhe=cost
        EPS=(AM2/AM3)**2
        XL1=LOG((2+EPS)/EPS)
        XL0=LOG(EPS)
        etaa=1+eps+cost
        etab=1+eps-cost
        prb=1d0
        PHSPAC=PHSPAC/(prb/(XL1/2*ETAa)+(1d0-prb)/(XL1/2*ETAb))
      PHSPAC=PHSPAC*(4*PI)
!        write(*,*) 'thet inv=',thet,eps,xl1,eta,cost
      cthx=paa(3)/sqrt(paa(1)**2+ paa(2)**2+ paa(3)**2)
      thet=acos(cthx)

        EPS=(AM2/AMtau)**2
        EPS=(AMnuta/Pn(4))**2
        EPS=(AMnuta/AMtau)**2*max((am3/amtau)**2,1d-4)
        EPS=(am3/amtau)**2
        XL1=LOG((2+EPS)/EPS)
        XL0=LOG(EPS)
      eta1=1+eps+cthx
      eta2=1+eps-cthx
      if (ift.eq.0) then
      ff=1d0
      else
      FF=1d0/3d0/1d0+2*ple/3d0/(XL1/2*ETA1)+2*(1d0-ple)/3d0/(XL1/2*ETA2)
      pleft=ple
        prob1=.2
        prob2=Pleft*.4d0
        prob3=(1D0-PLEFT)*.4D0
        prob4=Pleft*.4d0
        prob5=(1D0-PLEFT)*.4D0

      FF=0d0/3d0/1d0+pleft/(XL1/2*ETA1)+(1d0-pleft)/(XL1/2*ETA2)
      beta=sqrt(1d0-eps)
      xx=eps
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
      ct=-cthx

      xccos1=beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))
      ct=cthx
      xccos2=beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))

        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
        n=1
        AM2SQX= CT+2D0-sqrt(1d0-eps)
        xj4=am2SQX**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)/2
        n=1
        AM2SQX=-CT+2D0-sqrt(1d0-eps)
        xj5=am2SQX**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)/2

 !     FF=0d0/3d0/1d0+pleft*xccos1+(1d0-pleft)*xccos2
      FF=PROB1/1d0+PROB2*xccos1+PROB3*xccos2+PROB4/XJ4+PROB5/XJ5
      endif
!      write(*,*) 'inv  ff=',ff,xl1,eta1,eta2,eps,cthx
!      write(*,*) '     thet=',thet,cthx
      PHSPAC=PHSPAC/FF
      DGAMT=PHSPAC
       if (ifprint.eq.1) write(*,*) 'rkinbre dgamt=',dgamt
      END

      SUBROUTINE KINBRE(ift,ple,
     $ SVAR,DGAMT,PN,amnuta,PIM2,AMP1,PIM1,AMP2,PIPL,AMP3)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION  PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4)
      DIMENSION  PR(4)
c ms 12/12/98      REAL*4 RRR(14)
      DOUBLE PRECISION RRR(14)
      common /nevik/ nevtru,ifprint
      save /nevik/
      real*4 rrx
      common /erery/ rrx(14)
      save /erery/
      SAVE
      DATA PI /3.141592653589793238462643D0/
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.0D0*Y*Z))

C
C FOUR BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)
      PHSPAC=1.D0/2**17/PI**8
      amtau=sqrt(svar)
C
!      nn=1000000
!      sum=0
!      sum2=0
!      do ll=1,nn
c ms 12/12/98      CALL RANMAR(RRR,14)
      CALL varran(RRR,14)
      do k=1,14
      rrx(k)=rrr(k)
      enddo
C
      amro=100
      gamro=50
C.. MASS OF two
        RR2=RRR(2)
        AMS1=(AMP2+AMP3)**2
        AMS2=(AMTAU-AMNUTA-amp1)**2
!        AM2SQ=AMS1+   RR2*(AMS2-AMS1)
!        AM2 =SQRT(AM2SQ)
!        PHSPAC=PHSPAC*(AMS2-AMS1)
        B=LOG(AMS1)
        A=LOG(AMS2)
        AM2SQ=AMS2*EXP((B-A)*RR2)
        AM2 =SQRT(AM2SQ)
        prob1=.3
        prob2=.3
        PROB3=.0
        PROB4=.4
        RR2=RRR(2)
        AMS1=(AMP2+AMP3)**2      
        AMS2=(AMTAU-AMNUTA-amp1)**2
        ALP1=ATAN((AMS1-AMRO**2)/AMRO/GAMRO)
        ALP2=ATAN((AMS2-AMRO**2)/AMRO/GAMRO)
        IF (RRR(12).LT.PROB1) THEN
         AM2SQ=AMS1+   RR2*(AMS2-AMS1)
         AM2 =SQRT(AM2SQ)
        elseIF (RRR(12).LT.(PROB1+PROB2)) THEN  
         B=LOG(AMS1)
         A=LOG(AMS2)
         AM2SQ=AMS2*EXP((B-A)*RR2)
         AM2 =SQRT(AM2SQ)     
        ELSEIF (RRR(12).LT.(PROB1+PROB2+PROB3)) THEN
         ALP=ALP1+RR2*(ALP2-ALP1)
         AM2SQ=AMRO**2+AMRO*GAMRO*TAN(ALP)
         AM2 =SQRT(AM2SQ)
        ELSE
         n=1
          if(n.eq.1) then
         AM2SQ=AMS1/(1D0-RR2*(1-(ams1/ams2)**n))
          elseif(n.eq.2) then
         AM2SQ=AMS1/sqrt(1D0-RR2*(1-(ams1/ams2)**n))
          else
         AM2SQ=AMS1*(1D0-RR2*(1-(ams1/ams2)**n))**(-1d0/n)
          endif
         AM2 =SQRT(AM2SQ)
         if (am2sq.gt.ams2) WRITE(*,*) 'am2sq',am2sq,ams1,ams2,rr2
         if (am2sq.gt.ams2) stop
         if (am2sq.lt.ams1) WRITE(*,*) 'am2sq',am2sq,ams1,ams2,rr2
         if (am2sq.lt.ams1) stop

        ENDIF
        XJ1=(AMS2-AMS1)
         B=LOG(AMS1)
         A=LOG(AMS2)
        xj2=AM2SQ*(A-B)
        xj3=((AM2SQ-AMRO**2)**2+(AMRO*GAMRO)**2)/(AMRO*GAMRO)
        xj3=xj3*(ALP2-ALP1)
        n=1
        xj4=am2SQ**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)
!        sum=Sum+1d0/(PROB1/XJ1+PROB2/XJ2+PROB3/XJ3+PROB4/XJ4)
!        sum2=Sum2+1d0/(PROB1/XJ1+PROB2/XJ2+PROB3/XJ3+PROB4/XJ4)**2
!        enddo
!        sum=sum/nn
!        sum2=sum2/nn
!        err=sqrt((sum2-sum**2)/nn)
!        write(*,*) sum,'+-',err
!        write(*,*) '28761.2547837270613 +- 0'
!        stop
        PHSPAC=PHSPAC/(PROB1/XJ1+PROB2/XJ2+PROB3/XJ3+PROB4/XJ4)

!        PHSPAC=PHSPAC*AM2SQ*(A-B)
C.. mass of three
        RR1=RRR(1)
        AMS1=(AMP1+am2)**2
        AMS2=(AMTAU-AMNUTA)**2
!        AM3SQ =AMS1+   RR1*(AMS2-AMS1)
!        AM3 =SQRT(AM3SQ)
!        PHSPAC=PHSPAC*(AMS2-AMS1)
        B=LOG(AMS1)
        A=LOG(AMS2)
        prbam3=1d0
        if(rrr(10).lt.prbam3) then
         AM3SQ=AMS2*EXP((B-A)*RR1)
         AM3SQb=ams2-am3sq+ams1
        else
         AM3SQ=AMS2*EXP((B-A)*RR1)
         am3sqb=am3sq
         AM3SQ =ams2-am3sq+ams1
        endif
        AM3 =SQRT(AM3SQ)
        PHSPAC=PHSPAC/(prbam3/(AM3SQ*(A-B))+(1-prbam3)/(AM3SQb*(A-B)))
* two RESTFRAME, DEFINE PIPL AND PIM1
        ENQ1=(AM2SQ-AMP2**2+AMP3**2)/(2*AM2)
        ENQ2=(AM2SQ+AMP2**2-AMP3**2)/(2*AM2)
        PPI=         ENQ1**2-AMP3**2
        PPPI=SQRT(ABS(ENQ1**2-AMP3**2))
        PHSPAC=PHSPAC*(4*PI)*(2*PPPI/AM2)
* PIPL  MOMENTUM IN TWO REST FRAME,RRR(7),RRR(8)
        THET =ACOS(-1.D0+2*RRR(7))
        PHI = 2*PI*RRR(8)
!ms 03.sept.97
c        PIPL(1)=D0
c        PIPL(2)=D0
        PIPL(1)=0D0
        PIPL(2)=0D0
!ms 03.sept.97
        PIPL(3)=PPPI
        PIPL(4)=ENQ1
        CALL ROTPOD(THET,PHI,PIPL)
* PIM1 MOMENTUM IN TWO REST FRAME
        DO 30 I=1,3
 30     PIM1(I)=-PIPL(I)
        PIM1(4)=ENQ2
* three REST FRAME, DEFINE momentum of two
*       two  MOMENTUM
        PR(1)=0
        PR(2)=0
        PR(4)=1.D0/(2*AM3)*(AM3**2+AM2**2-AMP1**2)
        PR(3)= SQRT(ABS(PR(4)**2-AM2**2))
        PPI  =          PR(4)**2-AM2**2
*       PIM2 MOMENTUM
        PIM2(1)=0
        PIM2(2)=0
        PIM2(4)=1.D0/(2*AM3)*(AM3**2-AM2**2+AMP1**2)
        PIM2(3)=-PR(3)
        PHSPAC=PHSPAC*(2*PR(3)/AM3)
* PIPL PIM1 BOOSTED FROM two REST FRAME TO three REST FRAME
      EXE=(PR(4)+PR(3))/AM2
      CALL BOSTd3(EXE,PIPL,PIPL)
      CALL BOSTd3(EXE,PIM1,PIM1)
      rr3=rrr(3)
      rr4=rrr(4)
        EPS=(AM2/AM3)**2
        XL1=LOG((2+EPS)/EPS)
        XL0=LOG(EPS)

        ETA  =EXP(XL1*RR3+XL0)
        prb=1d0
        rrr10=0.1d0 ! branch frozen 
        if (rrr(11).lt.prb) then
         CTHET=-(1+EPS-ETA)
         etaa=eta
         etab=1+eps-CTHET

        else
         CTHET=(1+EPS-ETA)
         etaa=1+eps+CTHET
         etab=eta

        endif
        THET =ACOS(CTHET)
        PHSPAC=PHSPAC/(prb/(XL1/2*ETAa)+(1d0-prb)/(XL1/2*ETAb))

      PHI = 2*PI*RR4
      PHSPAC=PHSPAC*(4*PI)
      CALL ROTPOd(THET,PHI,PIPL)
      CALL ROTPOd(THET,PHI,PIM1)
      CALL ROTPOd(THET,PHI,PIM2)
      CALL ROTPOd(THET,PHI,PR)
!      write(*,*) 'thet gen=',thet,eps,xl1,eta,cthet
C
* NOW TO THE  REST FRAME, DEFINE three AND PN MOMENTA
* three  MOMENTUM
      PAA(1)=0
      PAA(2)=0
      PAA(4)=1.D0/(2*AMTAU)*(AMTAU**2-AMNUTA**2+AM3**2)
      PAA(3)= SQRT(ABS(PAA(4)**2-AM3**2))
      PPI   =          PAA(4)**2-AM3**2
      PHSPAC=PHSPAC*(4*PI)*(2*PAA(3)/AMTAU)
* pn MOMENTUM
      PN(1)=0
      PN(2)=0
      PN(4)=1.D0/(2*AMTAU)*(AMTAU**2+AMNUTA**2-AM3**2)
      PN(3)=-PAA(3)
* Z-AXIS ANTIPARALLEL TO pn MOMENTUM
      EXE=(PAA(4)+PAA(3))/AM3
      CALL BOSTd3(EXE,PIPL,PIPL)
      CALL BOSTd3(EXE,PIM1,PIM1)
      CALL BOSTd3(EXE,PIM2,PIM2)
      CALL BOSTd3(EXE,PR,PR)
      call rotatv(-1,paa,PIPL,PIPL)
      call rotatv(-1,paa,PIM1,PIM1)
      call rotatv(-1,paa,PIM2,PIM2)
      call rotatv(-1,paa,PN,PN)
      call rotatv(-1,paa,pr,pr)
      call rotatv(-1,paa,PAA,PAA)
        EPS=(AM2/AMtau)**2
        EPS=(AMnuta/pn(4))**2
        EPS=(AMnuta/AMtau)**2*max((am3/amtau)**2,1d-4)
        EPS=(am3/amtau)**2
        XL1=LOG((2+EPS)/EPS)
        XL0=LOG(EPS)
      if(ift.eq.0) then
       THET =ACOS(-1.D0+2*RRR(5))
       CTHET=COS(THET)
       PHI = 2*PI*RRR(6)
      else
       IF    (RRR(9).lt.1.d0/3d0) then 
        THET =ACOS(-1.D0+2*RRR(5))
        CTHET=COS(THET)
        PHI = 2*PI*RRR(6)
       elseIF(RRR(9).lt.(1d0+2*ple)/3d0) then
         ETA  =EXP(XL1*RRR(5)+XL0)
         CTHET=-(1+EPS-ETA)
         THET =ACOS(CTHET)
         PHI = 2*PI*RRR(6)
       else
         ETA  =EXP(XL1*RRR(5)+XL0)
         CTHET=(1+EPS-ETA)
         THET =ACOS(CTHET)
         PHI = 2*PI*RRR(6)
       endif
        XL1=LOG((2+EPS)/EPS)
        XL0=LOG(EPS)
        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
        pleft=ple
        prob1=.2
        prob2=Pleft*.4d0
        prob3=(1D0-PLEFT)*.4D0
        prob4=Pleft*.4d0
        prob5=(1D0-PLEFT)*.4D0
      IF    (RRR(9).lt.PROB1) then 
       THET =ACOS(-1.D0+2*RRR(5))
       CTHET=COS(THET)
       PHI = 2*PI*RRR(6)
      elseIF(RRR(9).lt.(PROB1+PROB2)) then
        ETA  =EXP(XL1*RRR(5)+XL0)
        CTHET=-(1+EPS-ETA)
         xx=eps
         beta=sqrt(1d0-eps)
         xlog=-log((1+beta)**2/xx)
         xlog1=-log(16D0/xx)
          u=(log((1D0+beta)/4D0))**2 +xlog*xlog1*rrr(5)
         cthet=-1D0/beta*(4D0*EXP(-SQRT(u))-1)
         CTHET=-cthet
        THET =ACOS(CTHET)
        PHI = 2*PI*RRR(6)
      elseIF    (RRR(9).lt.(PROB1+PROB2+PROB3)) then
        ETA  =EXP(XL1*RRR(5)+XL0)
        CTHET=(1+EPS-ETA)
         xx=eps
         beta=sqrt(1d0-eps)
         xlog=-log((1+beta)**2/xx)
         xlog1=-log(16D0/xx)
          u=(log((1D0+beta)/4D0))**2 +xlog*xlog1*rrr(5)
          cthet=-1D0/beta*(4D0*EXP(-SQRT(u))-1)

        THET =ACOS(CTHET)
        PHI = 2*PI*RRR(6)
      elseIF    (RRR(9).lt.(PROB1+PROB2+PROB3+PROB4)) then
        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
          n=1
          if(n.eq.1) then
         AM2SQX=AMS1/(1D0-RRr(5)*(1-(ams1/ams2)**n))
          elseif(n.eq.2) then
         AM2SQX=AMS1/sqrt(1D0-RRr(5)*(1-(ams1/ams2)**n))
          else
         AM2SQX=AMS1*(1D0-RRr(5)*(1-(ams1/ams2)**n))**(-1d0/n)
          endif
        CTHET=AM2SQX-2D0+sqrt(1d0-eps)
        THET =ACOS(CTHET)
        PHI = 2*PI*RRR(6)
      else
        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
          n=1
          if(n.eq.1) then
         AM2SQX=AMS1/(1D0-RRr(5)*(1-(ams1/ams2)**n))
          elseif(n.eq.2) then
         AM2SQX=AMS1/sqrt(1D0-RRr(5)*(1-(ams1/ams2)**n))
          else
         AM2SQX=AMS1*(1D0-RRr(5)*(1-(ams1/ams2)**n))**(-1d0/n)
          endif
        CTHET=-AM2SQX+2D0-sqrt(1d0-eps)
        THET =ACOS(CTHET)
        PHI = 2*PI*RRR(6)
      endif
      if (cthet**2.gt.1d0) then
       write(*,*) 'station cthet rrr(9); arbit action'
       write(*,*) cthet,rrr(5),rrr(9)
       write(*,*) ams1,ams2,am2sq
       cthet=cthet/cthet**2
        THET =ACOS(CTHET)       
      endif

      endif
      eta1=1+eps+cthet
      eta2=1+eps-cthet
      if (ift.eq.0) then
      ff=1d0
      else
      FF=1d0/3d0/1d0+2*ple/3d0/(XL1/2*ETA1)+2*(1d0-ple)/3d0/(XL1/2*ETA2)
      xx=eps
      beta=sqrt(1d0-eps)
      xx=eps
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
      ct=-cthet

      xccos1=beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))
      ct=cthet
      xccos2=beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))

        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
        n=1
        AM2SQX= CTHET+2D0-sqrt(1d0-eps)
        xj4=am2SQX**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)/2
        n=1
        AM2SQX=-CTHET+2D0-sqrt(1d0-eps)
        xj5=am2SQX**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)/2
       
      FF=PROB1/1d0+PROB2*xccos1+PROB3*xccos2+PROB4/XJ4+PROB5/XJ5

      endif
      PHSPAC=PHSPAC/FF
!      write(*,*) 'prod ff=',ff,xl1,eta1,eta2,eps,cthet
!      write(*,*) 'prod thet=',thet,cthet
      CALL ROTPOd(THET,PHI,PIPL)
      CALL ROTPOd(THET,PHI,PIM1)
      CALL ROTPOd(THET,PHI,PIM2)
      CALL ROTPOd(THET,PHI,PR)
      CALL ROTPOd(THET,PHI,PN)
      CALL ROTPOd(THET,PHI,PAA)
      DGAMT=PHSPAC
      if (ifprint.eq.1) write(*,*) 'kinbre dgamt=',dgamt
      END

      SUBROUTINE RKINT(idoub,amro,gamrox,ple,
     $ SVAR,DGAMT,PN,amnuta,PIM2,AMP1,PIM1,AMP2,PIPL,AMP3)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION  PT(4),PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4)
      DIMENSION  PR(4),PBST(4)
      common /articut/ arbitr,arbitr1,themin,arbitr2
      common /nevik/ nevtru,ifprint
      save /nevik/,/articut/
      SAVE
      DATA PI /3.141592653589793238462643D0/
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.0D0*Y*Z))

C
C FOUR BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)
      PHSPAC=1.D0/2**17/PI**8
      amtau=sqrt(svar)
      ift=0
      if(amnuta.eq.amp2) ift=1
      if(   amnuta.eq.amp1 .and.amp2.eq.amp3
     $   .and.amp1.lt.0.001.and.amp3.ge.amp1) ift=1
      gamro=abs(gamrox)
        if (ifprint.eq.1) write(*,*) 'rkint now ..'
C
      DO K=1,4
        PAA(K)=PIM1(K)+PIM2(K)+PIPL(K)
        PR(K) =PIM1(K)+PIPL(K)
        pt(k) =PIM1(K)-PIM2(K)+PIPL(K)
      ENDDO
C
C.. MASS OF two
      if (gamrox.lt.0d0) then 
        prob1=.2
        prob2=.2!.3
        PROB3=.2
        PROB4=.2!.3
        prob5=.20d0 !this branch was tested negatively must be checked before use
      else
        prob1=.4
        prob2=.0
        PROB3=.4
        PROB4=.0
        prob5=.20d0
      endif

        AMS1=(AMP2+AMP3)**2
        AMS2=(AMTAU-AMNUTA-amp1)**2
        AM2SQ=dmas2(PR)
        AM2 =SQRT(AM2SQ)
        XJ1=(AMS2-AMS1)
C PHASE SPACE WITH SAMPLING FOR RHO RESONANCE
         B=LOG(AMS1)
         A=LOG(AMS2)
        xj2=AM2SQ*(A-B)

      ALP1=ATAN((AMS1-AMRO**2)/AMRO/GAMRO)
      ALP2=ATAN((AMS2-AMRO**2)/AMRO/GAMRO)
      xj3=((AM2SQ-AMRO**2)**2+(AMRO*GAMRO)**2)/(AMRO*GAMRO)
      xj3=xj3*(ALP2-ALP1)
        n=1
        xj4=am2SQ**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)
        arbix=max(1d0,arbitr)
        am2sqx=am2sq+arbix        
        ams1x=ams1+arbix
        ams2x=ams2+arbix
!        n=1
!        xj5=am2SQx**(n+1)*n*(1D0/ams1x**n-1D0/ams2x**n)
         B=LOG(AMS1x)
         A=LOG(AMS2x)
        xj5=AM2SQx*(A-B)
        amrou=sqrt(max(200d0,arbitr))
        gamrou=amrou/4
        amrou=1.5*amrou
 !       amrou=amro
!        gamrou=gamro
        ALP1u=ATAN((AMS1-AMROu**2)/AMROu/GAMROu)
        ALP2u=ATAN((AMS2-AMROu**2)/AMROu/GAMROu)
        xj5=((AM2SQ-AMROu**2)**2+(AMROu*GAMROu)**2)/(AMROu*GAMROu)
        xj5=xj5*(ALP2u-ALP1u)


       PHSPAC=PHSPAC/(PROB1/XJ1+PROB2/XJ2+PROB3/XJ3+PROB4/XJ4+PROB5/XJ5)


C.. mass of three
        AMS1=(AMP1+am2)**2
        AMS2=(AMTAU-AMNUTA)**2
        AM3SQ =dmas2(PAA)
        AM3 =SQRT(AM3SQ)


      if (idoub.ne.1) then
         PROB1=.8d0
         PROB2=0.0d0
         prob3=0.2d0
      elseif (gamrox.lt.0d0) then 
         PROB1=0.4d0
         PROB2=0.4d0
         prob3=0.2d0
      else
         PROB1=.8d0
         PROB2=0d0
         prob3=0.2D0
      endif

         B=LOG(AMS1)
         A=LOG(AMS2)
        XJ1=AM3SQ*(A-B)
        n=1
        xj2=am3SQ**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)
        xj3=ams2-ams1
        PHSPAC=PHSPAC/(PROB1/XJ1+PROB2/XJ2+PROB3/xj3)
!        else
!         B=LOG(AMS1)
!         A=LOG(AMS2)
!         PHSPAC=PHSPAC*AM3SQ*(A-B)
!        endif

        ENQ1=(AM2SQ-AMP2**2+AMP3**2)/(2*AM2)
        PPPI=SQRT(ABS(ENQ1**2-AMP3**2))
        PHSPAC=PHSPAC*(4*PI)*(2*PPPI/AM2)
C
        PR4=1.D0/(2*AM3)*(AM3**2+AM2**2-AMP1**2)
        PR3= SQRT(ABS(PR4**2-AM2**2))
        PHSPAC=PHSPAC*(2*PR3/AM3)
        if (ifprint.eq.1) write(*,*) 'am2-3',am2,am3
        if (ift.eq.1) then
!...     versor of one in two restframe
         PNPAA=dot(pim2,pr)
         prx3=PPPI
         prx4=(AM2SQ-AMP2**2+AMP3**2)/(2*AM2)
         pim14=(AM2SQ+AMP2**2-AMP3**2)/(2*AM2)
         DO K=1,4
          pt(k) =( (PIM1(K)-PIPL(K))
     $            -Pr(K)*(prx4-pim14)/am2   )/2/PRx3
          PBST(K)=Pim2(K)-Pr(K)*PNPAA/am2**2
         ENDDO
         cost=-dot(pt,pbst)/sqrt(-dmas2(pbst))
cc ms 03.07.97         if(sqrt(-dmas2(pt)).gt.1d0) cost=cost/sqrt(-dmas2(pt))
         if(sqrt(dabs(-dmas2(pt))).gt.1d0) 
     $      cost=cost/sqrt(dabs(-dmas2(pt)))
! m.s. 8/24/98 beg
         IF(cost.LT.-1d0) cost=-1d0
         IF(cost.GT. 1d0) cost= 1d0
! m.s. 8/24/98 end
         thet=acos(cost)
         costhe=cost
         cthet=cost
         EPS=(amp3*2*AM2/(AM2**2))**2
         XL1=LOG((2+EPS)/EPS)
         XL0=LOG(EPS)
         eta1=1+eps+cthet
         eta2=1+eps-cthet
         prev=0.3
!         write(*,*) thet, 1/(PREV/(XL1/2*ETA)+(1d0-prev)/1D0)
        PHSPAC=PHSPAC/(PREV/( XL1/2*ETA1)+PREV/(XL1/2*ETA2)
     $                       +(1d0-2*prev)/1D0)
       if (ifprint.eq.1 ) then
       write(*,*) 'R ff2=',1/(XL1/2*ETA1),1/(XL1/2*ETA2),prev
       write(*,*) 'R ',cthet,eps,xl1
       endif
      endif
      

      PHSPAC=PHSPAC*(4*PI)
!        write(*,*) 'thet inv=',thet,eps,xl1,eta,cost


      cthx=pn(3)/sqrt(pn(1)**2+ pn(2)**2+ pn(3)**2)
      thet=acos(cthx)

        EPS=(AMnuta/AMtau)**2*max((am3/amtau)**2,1d-4)
        XL1=LOG((2+EPS)/EPS)
        XL0=LOG(EPS)
      eta1=1+eps+cthx
      eta2=1+eps-cthx
      pleft=ple
!      if(gamrox.lt.0.) pleft=0.5
      if(gamro/amro.gt.0.5) pleft=0.5
      if (gamrox.lt.0d0) then 
        prob1=.2
        prob2=Pleft*.4d0
        prob3=(1D0-PLEFT)*.4D0
        prob4=Pleft*.4d0
        prob5=(1D0-PLEFT)*.4D0
      else
        prob1=.2
        prob2=Pleft*.8d0
        prob3=(1D0-PLEFT)*.8D0
        prob4=0
        prob5=0
      endif
      FF=0d0/3d0/1d0+pleft/(XL1/2*ETA1)+(1d0-pleft)/(XL1/2*ETA2)
      beta=sqrt(1d0-eps)
      xx=eps
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
      ct=-cthx

      xccos1=beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))
      ct=cthx
      xccos2=beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))

        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
        n=1
        AM2SQX= CTHX+2D0-sqrt(1d0-eps)
        xj4=am2SQX**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)/2
        n=1
        AM2SQX=-CTHX+2D0-sqrt(1d0-eps)
        xj5=am2SQX**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)/2

 !     FF=0d0/3d0/1d0+pleft*xccos1+(1d0-pleft)*xccos2
      FF=PROB1/1d0+PROB2*xccos1+PROB3*xccos2+PROB4/XJ4+PROB5/XJ5
       if (ifprint.eq.1 ) then
      write(*,*) 'inv  ff4=',prob1,prob2,prob3,prob4,prob5
      write(*,*) '         ',xccos1,xccos2,xj4,xj5
      write(*,*) '         ',cthx,ams1,ams2
        endif
      PHSPAC=PHSPAC/FF

        PAA4=1.D0/(2*AMTAU)*(AMTAU**2-AMNUTA**2+AM3**2)
        PAA3= SQRT(ABS(PAA(4)**2-AM3**2))
        PHSPAC=PHSPAC*(4*PI)*(2*PAA3/AMTAU)
        pim24=1.D0/(2*AM3)*(AM3**2-AM2**2+AMP1**2)
!... versor of two in three restframe
      PNPAA=dot(pn,paa)
      DO K=1,4
         pt(k) =( (PIM1(K)-PIM2(K)+PIPL(K))
     $           -PAA(K)*(pr4-pim24)/am3   )/2/PR3
         PBST(K)=PN(K)-PAA(K)*PNPAA/am3**2
      ENDDO
        cost=dot(pt,pbst)/sqrt(-dmas2(pbst))
cc ms 03.07.97        if(sqrt(-dmas2(pt)).gt.1d0) cost=cost/sqrt(-dmas2(pt))
        if(sqrt(dabs(-dmas2(pt))).gt.1d0) 
     $     cost=cost/sqrt(dabs(-dmas2(pt)))
        if(abs(cost).gt.1d0) then
        write(*,*) '>>>>>>>>>>>>>>>>>>>>>>>'
        write(*,*) 'cos=',cost
cc ms 03.07.97  write(*,*) dot(pt,pbst),sqrt(-dmas2(pbst)),sqrt(-dmas2(pt))
        write(*,*) dot(pt,pbst),sqrt(dabs(-dmas2(pbst)))
     $      ,sqrt(dabs(-dmas2(pt)))
        write(*,*) '<<<<<<<<<<<<<<<<<<<<<<<'
        cost=cost/cost**2
        endif
        if(idoub.eq.5) cost=-cost
        thet=acos(cost)
        costhe=cost
C-- angles for three:
      t=(amtau**2-am3**2)*(1d0-abs(cthx))

!        EPS=(amp1*2*AM3/(AM3**2-AM2**2+AMP1**2))**2
         EPS=((t+amp1)*2*AM3/(AM3**2-AM2**2+AMP1**2))**2
!         EPS=(amp1*2*AM3/(AM3**2+AMP1**2))**2
         if (eps.ge.1d0) eps=.999d0
        XL1=LOG((2+EPS)/EPS)
        XL0=LOG(EPS)
        eta1=1+eps-cost
        eta2=1+eps-cost
         if(idoub.eq.1) eta2=1+eps+cost
        prev=0.4d0
        if(gamro/amro.gt.0.5) prev=0.6d0
        if(gamrox.lt.0.) prev=0.6d0
        if (amp1.lt.0.3d-3) prev=0
!        if (amp1.lt.0.6d-3) prev=0
        uu=1d0/(PREV/2/(XL1/2*ETA1)+PREV/2/(XL1/2*ETA2)+(1d0-PREV)/1D0)
        PHSPAC=PHSPAC*uu
       if (ifprint.eq.1 ) then
       write(*,*) 'r  ff3  '
       write(*,*) 'r  ff3=',1/(XL1/2*ETA1),prev,cost,idoub
       endif

!      xx=eps
!      beta=sqrt(1d0-eps)
!      xlog=-log((1+beta)**2/xx)
!      xlog1=-log(16D0/xx)
!      ct=cthet
!      if(idoub.eq.1) ct=-cthet

!      xccos=beta/(xlog*xlog1
!     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
!     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))
!         PHSPAC=PHSPAC/(PREV*xccos+(1d0-PREV)/1D0)
!        PHSPAC=PHSPAC/(PREV/(XL1/2*ETA)+(1d0-prev)/1D0)
       if (ifprint.eq.1 ) then
!       write(*,*) 'R ff3=',1/(XL1/2*ETA),prev,cost,idoub
       endif

      DGAMT=PHSPAC
      if (phspac.lt.0d0) then
       write(*,*) 'phspac=',phspac,'ff=',ff
      write(*,*) 'inv  ff=',ff,xl1,eta1,eta2,eps,cthx
      write(*,*) '     thet=',thet,cthx
      write(*,*) 'first angular jacobian=',uu,cost
      write(*,*) 'am2 am3',am2,am3
      endif
      END

      SUBROUTINE KINT(idoub,amro,gamrox,ple,
     $ SVAR,DGAMT,PN,amnuta,PIM2,AMP1,PIM1,AMP2,PIPL,AMP3)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION  PN(4),PAA(4),PIM1(4),PIM2(4),PIPL(4)
      DIMENSION  PR(4)
c ms 12/12/98      REAL*4 RRR(14)
      DOUBLE PRECISION RRR(14)
      common /articut/ arbitr,arbitr1,themin,arbitr2
      common /nevik/ nevtru,ifprint
      save /nevik/,/articut/
      real*4 rrx
      common /erery/ rrx(14)
      save /erery/
      SAVE
      DATA PI /3.141592653589793238462643D0/
      XLAM(X,Y,Z)=SQRT(ABS((X-Y-Z)**2-4.0D0*Y*Z))
      if (ifprint.eq.1) write(*,*) 'kint now ...'
C
C FOUR BODY PHASE SPACE NORMALISED AS IN BJORKEN-DRELL
C D**3 P /2E/(2PI)**3 (2PI)**4 DELTA4(SUM P)
      PHSPAC=1.D0/2**17/PI**8
      amtau=sqrt(svar)
      ift=0
      if(amnuta.eq.amp2) ift=1
      if(   amnuta.eq.amp1 .and.amp2.eq.amp3
     $   .and.amp1.lt.0.001.and.amp3.ge.amp1) ift=1
      gamro=abs(gamrox)
C
!      sum=0
!      sum2=0
!      nn=1000000
!      do k=1,nn
c ms 12/12/98      CALL RANMAR(RRR,14)
      CALL varran(RRR,14)
        if (ift.gt.100) then
        RRR( 1 )= 0.9478749037   
        RRR( 2 )= 0.6414651871E-03
        RRR( 3 )= 0.2186722755
        RRR( 4 )= 0.6770371199
        RRR( 5 )= 0.9959959984
        RRR( 6 )= 0.8483608961
        RRR( 7 )= 0.2983032465
        RRR( 8 )= 0.4594436884
        RRR( 9 )= 0.3546591997
        RRR( 10 )= 0.7370334864
        RRR( 11 )= 0.5411399007
        RRR( 12 )= 0.5843002200
        RRR( 13 )= 0.5636240840
        RRR( 14 )= 0.1165237427

        RRR( 1 )= 0.9120953083 +.07 
        RRR( 2 )= 0.6044536829E-01  -.00314
        RRR( 3 )= 0.6973654628
        RRR( 4 )= 0.6249293685
        RRR( 5 )= 0.2043003440
        RRR( 6 )= 0.5548739433
        RRR( 7 )= 0.9244202971
        RRR( 8 )= 0.1170529723
        RRR( 9 )= 0.4773632288
        RRR( 10 )= 0.3393656611
        RRR( 11 )= 0.2608218789
        RRR( 12 )= 0.7097090483
        RRR( 13 )= 0.1676200032
        RRR( 14 )= 0.4811552763


        RRR( 1 )= 0.6470682025
        RRR( 2 )= 0.2057774663
        RRR( 3 )= 0.5201567411
        RRR( 4 )= 0.2157996893
        RRR( 5 )= 0.2812561989
        RRR( 6 )= 0.5039139986
        RRR( 7 )= 0.8193333149
        RRR( 8 )= 0.5267652273
        RRR( 9 )= 0.3422190547
        RRR( 10 )= 0.2896287441
        RRR( 11 )= 0.3918313980E-01
        RRR( 12 )= 0.6622866988
        RRR( 13 )= 0.8352186084
        RRR( 14 )= 0.7802855968E-01

        RRR( 1 )= 0.4865604639E-01
        RRR( 2 )= 0.7470643520E-01
        RRR( 3 )= 0.5707561970E-02
        RRR( 4 )= 0.1091821194
        RRR( 5 )= 0.5938494205
        RRR( 6 )= 0.3438313007
        RRR( 7 )= 0.1558269262
        RRR( 8 )= 0.5537773371
        RRR( 9 )= 0.8776282072
        RRR( 10 )= 0.4459511638 /2
        RRR( 11 )= 0.1935519576
        RRR( 12 )= 0.5576208830
        RRR( 13 )= 0.9880648255
        RRR( 14 )= 0.9081103206

!        RRR( 1 )= 0.2664464116
!        RRR( 2 )= 0.4497187138
!        RRR( 3 )= 0.8510571718E-01
!        RRR( 4 )= 0.7872920632
!        RRR( 5 )= 0.5956429243
!        RRR( 6 )= 0.5650028586
!        RRR( 7 )= 0.7602188587
!        RRR( 8 )= 0.9214590788
!        RRR( 9 )= 0.8335509300
!        RRR( 10 )= 0.9940457344
!        RRR( 11 )= 0.2682380676
!        RRR( 12 )= 0.8510574102
!        RRR( 13 )= 0.1608982682
!        RRR( 14 )= 0.4655991197

        iflak=3
        write(*,*) 'walek',ift
        endif
      do k=1,14
      rrx(k)=rrr(k)
      enddo

      if (ifprint.eq.1) then
         write(*,*) '=====>'
         do k=1,14
            write(*,*) '       RRR(',k,')=',RRR(k)
         enddo
      endif
c --- mass of two
      if (ifprint.eq.100) then
        rrr(2)= .9
c --- mass of three
        rrr(1)=.1
C
C-- angles for three:
        rrr(3)=.2
C-- angles for four:
        rrr(5)=.975 
C-- angles for two:
        rrr(7)=.8 
      endif
C.. MASS OF two
      if (gamrox.lt.0d0) then 
        prob1=.2
        prob2=.2!.3
        PROB3=.2
        PROB4=.2!.3
        prob5=0.2d0
      else
        prob1=.4
        prob2=.0
        PROB3=.4
        PROB4=.0
        prob5=0.2d0
      endif
        if (iflak.eq.5) write(*,*) prob1,prob2,prob3,prob4,prob5

        RR2=RRR(2)
        AMS1=(AMP2+AMP3)**2      
        AMS2=(AMTAU-AMNUTA-amp1)**2
        ALP1=ATAN((AMS1-AMRO**2)/AMRO/GAMRO)
        ALP2=ATAN((AMS2-AMRO**2)/AMRO/GAMRO)
        amrou=sqrt(max(200d0,arbitr))
        gamrou=amrou/4
        amrou=1.5*amrou
!        amrou=amro
!        gamrou=gamro
        ALP1u=ATAN((AMS1-AMROu**2)/AMROu/GAMROu)
        ALP2u=ATAN((AMS2-AMROu**2)/AMROu/GAMROu)
        IF (RRR(10).LT.PROB1) THEN
         AM2SQ=AMS1+   RR2*(AMS2-AMS1)
         AM2 =SQRT(AM2SQ)
        elseIF (RRR(10).LT.(PROB1+PROB2)) THEN  
         B=LOG(AMS1)
         A=LOG(AMS2)
         AM2SQ=AMS2*EXP((B-A)*RR2)
         AM2 =SQRT(AM2SQ)     
        ELSEIF (RRR(10).LT.(PROB1+PROB2+PROB3)) THEN

         ALP=ALP1+RR2*(ALP2-ALP1)
         AM2SQ=AMRO**2+AMRO*GAMRO*TAN(ALP)
         AM2 =SQRT(AM2SQ)
        ELSEIF (RRR(10).LT.(PROB1+PROB2+PROB3+prob4)) THEN
         n=1
          if(n.eq.1) then
         AM2SQ=AMS1/(1D0-RR2*(1-(ams1/ams2)**n))
          elseif(n.eq.2) then
         AM2SQ=AMS1/sqrt(1D0-RR2*(1-(ams1/ams2)**n))
          else
         AM2SQ=AMS1*(1D0-RR2*(1-(ams1/ams2)**n))**(-1d0/n)
          endif
         AM2 =SQRT(AM2SQ)
         if (am2sq.gt.ams2) WRITE(*,*) 'am2sqx,err',am2sq,ams1,ams2,rr2
         if (am2sq.gt.ams2) stop
         if (am2sq.lt.ams1) WRITE(*,*) 'am2sqx,err',am2sq,ams1,ams2,rr2
         if (am2sq.lt.ams1) stop

        ELSE
         arbix=max(1d0,arbitr)
         ams1x=ams1+arbix
         ams2x=ams2+arbix
         n=1
          if(n.eq.1) then
         AM2SQx=AMS1x/(1D0-RR2*(1-(ams1x/ams2x)**n))
          elseif(n.eq.2) then
         AM2SQx=AMS1x/sqrt(1D0-RR2*(1-(ams1x/ams2x)**n))
          else
         AM2SQx=AMS1x*(1D0-RR2*(1-(ams1x/ams2x)**n))**(-1d0/n)
          endif
         B=LOG(AMS1x)
         A=LOG(AMS2x)
         AM2SQx=AMS2x*EXP((B-A)*RR2)
         am2sq=am2sqx-arbix
         AM2 =SQRT(AM2SQ)
         if (am2sq.gt.ams2) WRITE(*,*) 'am2sqx,err',am2sq,ams1,ams2,rr2
         if (am2sq.gt.ams2) stop
         if (am2sq.lt.ams1) WRITE(*,*) 'am2sqx,err',am2sq,ams1,ams2,rr2
         if (am2sq.lt.ams1) stop
         ALPu=ALP1u+RR2*(ALP2u-ALP1u)
         AM2SQ=AMROu**2+AMROu*GAMROu*TAN(ALPu)
         AM2 =SQRT(AM2SQ)

        ENDIF
        XJ1=(AMS2-AMS1)
         B=LOG(AMS1)
         A=LOG(AMS2)
        xj2=AM2SQ*(A-B)
        xj3=((AM2SQ-AMRO**2)**2+(AMRO*GAMRO)**2)/(AMRO*GAMRO)
        xj3=xj3*(ALP2-ALP1)
        n=1
        xj4=am2SQ**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)
        arbix=max(1d0,arbitr)
        am2sqx=am2sq+arbix        
        ams1x=ams1+arbix
        ams2x=ams2+arbix
!        n=1
!        xj5=am2SQx**(n+1)*n*(1D0/ams1x**n-1D0/ams2x**n)
         B=LOG(AMS1x)
         A=LOG(AMS2x)
        xj5=AM2SQx*(A-B)

        xj5=((AM2SQ-AMROu**2)**2+(AMROu*GAMROu)**2)/(AMROu*GAMROu)
        xj5=xj5*(ALP2u-ALP1u)

      if (ifprint.eq.1) write(*,*) 'a=',ams1,ams2,am2sq
      if (ifprint.eq.1) write(*,*) 'b=',ams1x,ams2x,am2sqx   
      if (ifprint.eq.1) write(*,*) 'amro-gamro',amro,gamro
!        sum=Sum+1d0/(PROB1/XJ1+PROB2/XJ2+PROB3/XJ3+PROB4/XJ4)
!        sum2=Sum2+1d0/(PROB1/XJ1+PROB2/XJ2+PROB3/XJ3+PROB4/XJ4)**2
!        enddo
!        sum=sum/nn
!        sum2=sum2/nn
!        err=sqrt((sum2-sum**2)/nn)
!        write(*,*) sum,'+-',err
!        write(*,*) '37980.9549446302772 +- 59.65'
!        stop
       PHSPAC=PHSPAC/(PROB1/XJ1+PROB2/XJ2+PROB3/XJ3+PROB4/XJ4+PROB5/XJ5)
        if (ifprint.eq.1) write(*,*) 'am2 ',am2,rr2
        if (ifprint.eq.1) write(*,*) 'xj-ts',xj1,xj2,xj3,xj4,xj5
        if (ifprint.eq.1) 
     $     write(*,*) 'probs',prob1,prob2,prob3,prob4,prob5

C.. mass of three
        RR1=RRR(1)
        AMS1=(AMP1+am2)**2
        AMS2=(AMTAU-AMNUTA)**2
         B=LOG(AMS1)
         A=LOG(AMS2)
      if (idoub.ne.1) then
         PROB1=.8d0
         PROB2=0.0d0
         prob3=0.2d0
      elseif (gamrox.lt.0d0) then 
         PROB1=0.4d0
         PROB2=0.4d0
         prob3=0.2d0
      else
         PROB1=.8d0
         PROB2=0d0
         prob3=0.2D0
      endif

         IF (RRR(12).lt.prob1) then
         AM3SQ=AMS2*EXP((B-A)*RR1)
         elseIF (RRR(12).lt.prob1+prob2) then
         n=1
          if(n.eq.1) then
         AM3SQ=AMS1/(1D0-RR1*(1-(ams1/ams2)**n))
          elseif(n.eq.2) then
         AM3SQ=AMS1/sqrt(1D0-RR1*(1-(ams1/ams2)**n))
          else
         AM3SQ=AMS1*(1D0-RR1*(1-(ams1/ams2)**n))**(-1d0/n)
          endif

         else
            AM3SQ=AMS1+(ams2-ams1)*rr1
         endif
         AM3 =SQRT(AM3SQ)
!         PHSPAC=PHSPAC*AM3SQ*(A-B)
        XJ1=AM3SQ*(A-B)
        n=1
        xj2=am3SQ**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)
        xj3=ams2-ams1
        PHSPAC=PHSPAC/(PROB1/XJ1+PROB2/XJ2+PROB3/xj3)

      if (ifprint.eq.1) write(*,*) 'am3 ',am3,AM3SQ*(A-B),rr1
!        else
!         B=LOG(AMS1)
!         A=LOG(AMS2)
!         AM3SQ=AMS2*EXP((B-A)*RR1)
!         AM3 =SQRT(AM3SQ)
!         PHSPAC=PHSPAC*AM3SQ*(A-B)
!        endif

C-- angles for four:
!      sum=0
!      sum2=0
!      nn=1000000
!      do k=1,nn
!      CALL RANMAR(RRR,14)

        EPS=(AMnuta/AMtau)**2*max((am3/amtau)**2,1d-4)
        XL1=LOG((2+EPS)/EPS)
        XL0=LOG(EPS)
        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
        pleft=ple
!        if(gamrox.lt.0.) pleft=0.5
      if(gamro/amro.gt.0.5) pleft=0.5
      if (gamrox.lt.0d0) then 
        prob1=.2
        prob2=Pleft*.4d0
        prob3=(1D0-PLEFT)*.4D0
        prob4=Pleft*.4d0
        prob5=(1D0-PLEFT)*.4D0
      else
        prob1=.2
        prob2=Pleft*.8d0
        prob3=(1D0-PLEFT)*.8D0
        prob4=0
        prob5=0
      endif
      IF    (RRR(9).lt.PROB1) then 
       THET =ACOS(-1.D0+2*RRR(5))
       CTHET=COS(THET)
       PHI = 2*PI*RRR(6)
      elseIF(RRR(9).lt.(PROB1+PROB2)) then
        ETA  =EXP(XL1*RRR(5)+XL0)
        CTHET=-(1+EPS-ETA)
         xx=eps
         beta=sqrt(1d0-eps)
         xlog=-log((1+beta)**2/xx)
         xlog1=-log(16D0/xx)
          u=(log((1D0+beta)/4D0))**2 +xlog*xlog1*rrr(5)
         cthet=-1D0/beta*(4D0*EXP(-SQRT(u))-1)
         CTHET=-cthet
        THET =ACOS(CTHET)
        PHI = 2*PI*RRR(6)
      elseIF    (RRR(9).lt.(PROB1+PROB2+PROB3)) then
        ETA  =EXP(XL1*RRR(5)+XL0)
        CTHET=(1+EPS-ETA)
         xx=eps
         beta=sqrt(1d0-eps)
         xlog=-log((1+beta)**2/xx)
         xlog1=-log(16D0/xx)
          u=(log((1D0+beta)/4D0))**2 +xlog*xlog1*rrr(5)
          cthet=-1D0/beta*(4D0*EXP(-SQRT(u))-1)

*!WP (to avoid floating exceptions)
          IF (ABS(cthet).LT.1d0) THEN
            THET =ACOS(CTHET)
          ELSEIF (cthet.LE.-1d0) THEN
             THET =PI
          ELSE
             THET =0d0
          ENDIF
          PHI = 2*PI*RRR(6)
      elseIF    (RRR(9).lt.(PROB1+PROB2+PROB3+PROB4)) then
        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
          n=1
          if(n.eq.1) then
         AM2SQX=AMS1/(1D0-RRr(5)*(1-(ams1/ams2)**n))
          elseif(n.eq.2) then
         AM2SQX=AMS1/sqrt(1D0-RRr(5)*(1-(ams1/ams2)**n))
          else
         AM2SQX=AMS1*(1D0-RRr(5)*(1-(ams1/ams2)**n))**(-1d0/n)
          endif
        CTHET=AM2SQX-2D0+sqrt(1d0-eps)
        THET =ACOS(CTHET)
        PHI = 2*PI*RRR(6)
      else
        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
          n=1
          if(n.eq.1) then
         AM2SQX=AMS1/(1D0-RRr(5)*(1-(ams1/ams2)**n))
          elseif(n.eq.2) then
         AM2SQX=AMS1/sqrt(1D0-RRr(5)*(1-(ams1/ams2)**n))
          else
         AM2SQX=AMS1*(1D0-RRr(5)*(1-(ams1/ams2)**n))**(-1d0/n)
          endif
        CTHET=-AM2SQX+2D0-sqrt(1d0-eps)
        THET =ACOS(CTHET)
        PHI = 2*PI*RRR(6)
      endif
      if (cthet**2.gt.1d0) then
       cthet=cthet/cthet**2
       write(*,*) 'cthet error -- arbi action'
       write(*,*) cthet,rrr(5),rrr(9)
       write(*,*) ams1,ams2,am2sq
        THET =ACOS(CTHET)
      endif
      eta1=1+eps+cthet
      eta2=1+eps-cthet
      xx=eps
      beta=sqrt(1d0-eps)
      xx=eps
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
      ct=-cthet

      xccos1=beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))
      ct=cthet
      xccos2=beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))

        ams1=1-sqrt(1d0-eps)
        ams2=3-sqrt(1d0-eps)
        n=1
        AM2SQX= CTHET+2D0-sqrt(1d0-eps)
        xj4=am2SQX**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)/2
        n=1
        AM2SQX=-CTHET+2D0-sqrt(1d0-eps)
        xj5=am2SQX**(n+1)*n*(1D0/ams1**n-1D0/ams2**n)/2
       
      FF=PROB1/1d0+PROB2*xccos1+PROB3*xccos2+PROB4/XJ4+PROB5/XJ5
!        sum=Sum+1d0/FF
!        sum2=Sum2+1d0/FF**2
!        enddo
!        sum=sum/nn
!        sum2=sum2/nn
!        err=sqrt((sum2-sum**2)/nn)
!        write(*,*) sum,'+-',err
!        write(*,*) '1.00095748299256204 +- 0.14E-01'
!        stop
       if (ifprint.eq.1 ) then

      write(*,*) '     ff4=',prob1,prob2,prob3,prob4,prob5
      write(*,*) '         ',xccos1,xccos2,xj4,xj5
      write(*,*) '         ',cthet,ams1,ams2
       endif

      PHSPAC=PHSPAC/FF 

C-- angles for three:
      t=(amtau**2-am3**2)*(1d0-abs(cthet))
      rr3=rrr(3)
      rr4=rrr(4)
        prev=0.4d0
        if(gamro/amro.gt.0.5) prev=0.6d0
        if(gamrox.lt.0.) prev=0.6d0
        if (amp1.lt.0.3d-3) prev=0
!        if (amp1.lt.0.6d-3) prev=0
        IF(RRR(11).lt.PREV) then
!         EPS=(amp1*2*AM3/(AM3**2-AM2**2+AMP1**2))**2
         EPS=((t+amp1)*2*AM3/(AM3**2-AM2**2+AMP1**2))**2
!         EPS=(amp1*2*AM3/(AM3**2+AMP1**2))**2
         if (eps.ge.1d0) eps=.999d0
         XL1=LOG((2+EPS)/EPS)
         XL0=LOG(EPS)
         ETA  =EXP(XL1*RR3+XL0)
         CTHET=(1+EPS-ETA)
         xx=eps
         beta=sqrt(1d0-eps)
         xlog=-log((1+beta)**2/xx)
         xlog1=-log(16D0/xx)
!          u=(log((1D0+beta)/4D0))**2 +xlog*xlog1*rr3
!          cthet=-1D0/beta*(4D0*EXP(-SQRT(u))-1)

         if(idoub.eq.1.and.RRR(11).lt.PREV/2) CTHET=-cthet
         THET3 =ACOS(CTHET)
       if (ifprint.eq.1 ) then
         write(*,*) 'ff3 aaa',eps,eta,cthet
         write(*,*) '      t',t,amp1,am3,am2,amtau
       endif
        else
         CTHET=-1+2*rr3
         THET3 =ACOS(CTHET)
!         EPS=(amp1*2*AM3/(AM3**2-AM2**2+AMP1**2))**2
         EPS=((t+amp1)*2*AM3/(AM3**2-AM2**2+AMP1**2))**2
!         EPS=(amp1*2*AM3/(AM3**2+AMP1**2))**2
         if (eps.ge.1d0) eps=.999d0
         XL1=LOG((2+EPS)/EPS)
         XL0=LOG(EPS)
         eta=1+eps-cthet
         if(idoub.eq.1) eta=1+eps+cthet
        endif

      xx=eps
      beta=sqrt(1d0-eps)
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
      ct=cthet
      if(idoub.eq.1) ct=-cthet
!      xccos=beta/(xlog*xlog1
!     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-ct)))
!     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-ct))))!!! +1d0/(1+beta*costhe))      
         XL1=LOG((2+EPS)/EPS)
         XL0=LOG(EPS)
         eta1=1+eps-cthet
         eta2=1+eps-cthet
         if(idoub.eq.1) eta2=1+eps+cthet
        xccos1=1D0/(XL1/2*ETA1)
        xccos2=1D0/(XL1/2*ETA2)
        PHSPAC=PHSPAC/(PREV/2*xccos1+PREV/2*xccos2+(1d0-prev)/1D0)
 
!        PHSPAC=PHSPAC/(PREV/(XL1/2*ETA)+(1d0-prev)/1D0)
        PHI3 = 2*PI*RR4
       if (ifprint.eq.1 ) then
       write(*,*) '  ff3,rr3',rr3
       write(*,*) '  ff3=',1/(XL1/2*ETA1),prev,cthet,idoub
       write(*,*) '  3ff=',eps,xccos1,xccos2
       endif

C-- angles for two: 
       if (ift.eq.1) then
        rr3=rrr(7)
        prev=0.3
        IF(RRR(13).lt.PREV) then
         EPS=(amp3*2*AM2/(AM2**2))**2
         XL1=LOG((2+EPS)/EPS)
         XL0=LOG(EPS)
         ETA  =EXP(XL1*RR3+XL0)
         CTHET=-(1+EPS-ETA)
         THET2 =ACOS(CTHET)
        elseIF(RRR(13).lt.2*PREV) then
         EPS=(amp3*2*AM2/(AM2**2))**2
         XL1=LOG((2+EPS)/EPS)
         XL0=LOG(EPS)
         ETA  =EXP(XL1*RR3+XL0)
         CTHET=(1+EPS-ETA)
         THET2 =ACOS(CTHET)
        else
         CTHET=-1+2*rr3
         THET2 =ACOS(CTHET)
        endif
         EPS=(amp3*2*AM2/(AM2**2))**2
         XL1=LOG((2+EPS)/EPS)
         XL0=LOG(EPS)
         eta1=1+eps+cthet
         eta2=1+eps-cthet
!       write(*,*) thet2, 1/(PREV/(XL1/2*ETA)+(1d0-prev)/1D0)
        PHSPAC=PHSPAC/( PREV/(XL1/2*ETA1)+PREV/(XL1/2*ETA2)
     $                +(1d0-2*prev)/1D0)
        PHI2 = 2*PI*RRR(8)
      if (ifprint.eq.1 ) then
      write(*,*) 'ff2=',1/(XL1/2*ETA1),1/(XL1/2*ETA2),prev
      write(*,*) '  ',cos(thet2),rr3,'<== rrr(7)'
      endif
        else
         THET2 =ACOS(-1.D0+2*RRR(7))
         PHI2 = 2*PI*RRR(8)
        PHSPAC=PHSPAC
        endif
C-- construction:
* two RESTFRAME, DEFINE PIPL AND PIM1
        ENQ1=(AM2SQ-AMP2**2+AMP3**2)/(2*AM2)
        ENQ2=(AM2SQ+AMP2**2-AMP3**2)/(2*AM2)
        PPI=         ENQ1**2-AMP3**2
        PPPI=SQRT(ABS(ENQ1**2-AMP3**2))
        PHSPAC=PHSPAC*(4*PI)*(2*PPPI/AM2)
* PIPL  MOMENTUM IN TWO REST FRAME,RRR(7),RRR(8)
!ms 03.sept.97
c        PIPL(1)=D0
c        PIPL(2)=D0
        PIPL(1)=0D0
        PIPL(2)=0D0
!ms 03.sept.97
        PIPL(3)=PPPI
        PIPL(4)=ENQ1
        CALL ROTPOD(THET2,PHI2,PIPL)
* PIM1 MOMENTUM IN TWO REST FRAME
        DO 30 I=1,3
 30     PIM1(I)=-PIPL(I)
        PIM1(4)=ENQ2
* three REST FRAME, DEFINE momentum of two
*       two  MOMENTUM
        PR(1)=0
        PR(2)=0
        PR(4)=1.D0/(2*AM3)*(AM3**2+AM2**2-AMP1**2)
        PR(3)= SQRT(ABS(PR(4)**2-AM2**2))
        PPI  =          PR(4)**2-AM2**2
*       PIM2 MOMENTUM
        PIM2(1)=0
        PIM2(2)=0
        PIM2(4)=1.D0/(2*AM3)*(AM3**2-AM2**2+AMP1**2)
        PIM2(3)=-PR(3)
        PHSPAC=PHSPAC*(2*PR(3)/AM3)
* PIPL PIM1 BOOSTED FROM two REST FRAME TO three REST FRAME
      EXE=(PR(4)+PR(3))/AM2
      CALL BOSTd3(EXE,PIPL,PIPL)
      CALL BOSTd3(EXE,PIM1,PIM1)

      PHSPAC=PHSPAC*(4*PI)
      CALL ROTPOd(THET3,PHI3,PIPL)
      CALL ROTPOd(THET3,PHI3,PIM1)
      CALL ROTPOd(THET3,PHI3,PIM2)
      CALL ROTPOd(THET3,PHI3,PR)
!      write(*,*) 'thet gen=',thet,eps,xl1,eta,cthet
C
* NOW TO THE  REST FRAME, DEFINE three AND PN MOMENTA
* three  MOMENTUM
      PAA(1)=0
      PAA(2)=0
      PAA(4)=1.D0/(2*AMTAU)*(AMTAU**2-AMNUTA**2+AM3**2)
      PAA(3)= SQRT(ABS(PAA(4)**2-AM3**2))
      PPI   =          PAA(4)**2-AM3**2
      PHSPAC=PHSPAC*(4*PI)*(2*PAA(3)/AMTAU)
* pn MOMENTUM
      PN(1)=0
      PN(2)=0
      PN(4)=1.D0/(2*AMTAU)*(AMTAU**2+AMNUTA**2-AM3**2)
      PN(3)=-PAA(3)
* Z-AXIS ANTIPARALLEL TO pn MOMENTUM
      EXE=(PAA(4)+PAA(3))/AM3
      CALL BOSTd3(EXE,PIPL,PIPL)
      CALL BOSTd3(EXE,PIM1,PIM1)
      CALL BOSTd3(EXE,PIM2,PIM2)
      CALL BOSTd3(EXE,PR,PR)
      call rotatv(-1,pn,PIPL,PIPL)
      call rotatv(-1,pn,PIM1,PIM1)
      call rotatv(-1,pn,PIM2,PIM2)
      call rotatv(-1,pn,PAA,PAA)
      call rotatv(-1,pn,pr,pr)
      call rotatv(-1,pn,PN,PN)
!      write(*,*) 'prod ff=',ff,xl1,eta1,eta2,eps,cthet
!      write(*,*) 'prod thet=',thet,cthet
      CALL ROTPOd(THET,PHI,PIPL)
      CALL ROTPOd(THET,PHI,PIM1)
      CALL ROTPOd(THET,PHI,PIM2)
      CALL ROTPOd(THET,PHI,PR)
      CALL ROTPOd(THET,PHI,PN)
      CALL ROTPOd(THET,PHI,PAA)
      DGAMT=PHSPAC
      END

      SUBROUTINE ROTPOd(THET,PHI,PP)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
C ----------------------------------------------------------------------
C
C ----------------------------------------------------------------------
      DIMENSION PP(4)
C
      CALL ROTOD2(THET,PP,PP)
      CALL ROTOD3( PHI,PP,PP)
      RETURN
      END

      SUBROUTINE cosdecc(mode,iflag,svar,cdec,phi,wt)
*     ***************************************
! Crude generation of decay costhe according to a simplified distribution.
!   mode: 0-generation
!         1-xccos of given cdec
!   cdec:  value of generated cosine
!   xccos: value of distribution function
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
      common / wekin2 / amaw,gammw,gmu,alphaw   
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      DOUBLE PRECISION drvec(100)
      save

      if (iflag.eq.0) then
       KeySpn = 0
      else
       KeySpn = MOD(KeyPhy,10000)/1000
      endif

      IF(keyspn.eq.1) THEN                        !1002=.78
        IF(svar.gt.500**2) THEN                    !502=.4
          delta=0.4d0+ (svar/500**2 -1)/8d0
        ELSEIF(svar.gt.4*amaw**2) THEN             !162=.4
          delta=.4d0
        ELSEIF(svar.gt.4*(amaw-5*gammw)**2) THEN   !142=.78
          delta=.4d0+ (1-svar/(4*amaw**2))*2d0
        ELSEIF(svar.gt.4*(amaw-10*gammw)**2) THEN  !122=40
          delta=.844d0+ (1-svar/(4*(amaw-5*gammw)**2))*100d0
        ELSE
          delta=40d0
        ENDIF

        IF(mode.eq.0)THEN
 11       call varran(drvec,3)
          cdec=2*drvec(1)-1
          xccos=(1+delta+cdec)/(1+delta)
          IF((2+delta)/(1+delta)*drvec(2).gt.xccos) goto 11
          phi =2*pi*drvec(3)
        ELSE
          xccos=(1+delta+cdec)/(1+delta)
        ENDIF
      ELSEIF(keyspn.eq.0) THEN
        IF(mode.eq.0)THEN
          call varran(drvec,3)
          cdec=2*drvec(1)-1
          phi =2*pi*drvec(3)
        ENDIF
        xccos=1D0
      ENDIF
      wt= 4*pi/xccos
      end

      SUBROUTINE cosdec_t(mode,ibeam,svar,sprim,s1,s2,ct,fi,ambeam,
     @                    amfi1,amfi2,   cosu,phi,wt)
*     ***************************************
! Crude generation of decay costhe according to a simplified distribution.
!   mode: 0-generation
!         1-xccos of given cdec
!   cdec:  value of generated cosine
!   xccos: value of distribution function
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
      common / wekin2 / amaw,gammw,gmu,alphaw   
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp
      common /nevik/ nevtru,ifprint
      save /nevik/

      DOUBLE PRECISION drvec(100)
      save
C simplified version of this routine.
      if (mode.ne.0) costhe=cosu*ibeam
      xx=s2/svar
      beta=sqrt(1d0-s2/svar)
      xlog=log((1+beta)**2/xx)
      xlog=-log((1+beta)**2/xx)
      xlog1=-log(16D0/xx)
      IF(mode.eq.0) then
 5      continue
        call varran(drvec,3)
       IF( drvec(2).lt.1d0/4d0 ) then
        costhe=-1d0/beta*(xx/(1+beta)*exp(xlog*drvec(1))-1d0)
        u=(log((1D0+beta)/4D0))**2 +xlog*xlog1*drvec(1)
        costhe=-1D0/beta*(4D0*EXP(-SQRT(u))-1)
       elseIF( drvec(2).lt.1d0/2d0 ) then
        costhe=-1d0/beta*(xx/(1+beta)*exp(-xlog*drvec(1))-1d0)
!       elseIF( drvec(2).lt.3d0/4d0 ) then
!        costhe= 1d0/beta*(xx/(1+beta)*exp(-xlog*drvec(1))-1d0)
       else
        costhe=2*drvec(1)-1 !
       endif
       phi=2*pi*drvec(3)
      IF (COSTHE.eq.1d0.or.COSTHE.eq.-1D0) goto 5
      cosu=costhe*ibeam
      endif

      cost=min(1d0,costhe)
      if (ifprint.eq.1) then
       write(*,*) 'cosdec-t',xx,svar,costhe,cosu
      endif
      xccos=1d0/4d0+1d0/4d0/beta/(-xlog)*
     $      (1d0/(xx/(1d0+beta)+beta*(1D0-cost)))
!      xccos=xccos+1d0/3d0/beta/(-xlog)*
!     $      (1d0/(xx/(1d0+beta)+beta*(1D0+cost)))

      xccos=xccos+1d0/8d0*beta/(xlog*xlog1
     $     /log(4d0/(xx/(1d0+beta)+beta*(1D0-cost)))
     $     /(4d0/(xx/(1d0+beta)+beta*(1D0-cost))))!!! +1d0/(1+beta*costhe))
! ms 17.06.96 here was wrong sign.
      wt= 4*pi/xccos/2
!
      end




      subroutine zz_dumper(nout,nevtru
     $                     ,wtovr,wtu,wtmax,wtmod,wtmod4f,iflav)    
*     **************************************************************     
c overweighted events monitoring, ZBW version
c 
c Written by: ZBW,MS        date: 
c Last update:             by:  
c

      implicit DOUBLE PRECISION (a-h,o-z)
      common / momset / QEFF1(4),QEFF2(4),sphum(4),sphot(100,4),nphot
      COMMON / cms_eff_momdec /
     $      effbeam1(4),effbeam2(4),p1(4),p2(4),p3(4),p4(4)
      save   / momset /

      INCLUDE 'zz_phsp.inc'

      real*4 rrx
      common /erery/ rrx(14)
      save /erery/

      dimension q12(4),q13(4),q14(4),q23(4),q24(4),q34(4),qtot(4)
      dimension qq1(4),qq2(4),qq3(4),qq4(4),XQ1(4),XQ2(4)
      dimension iflav(4)

      do k=1,4
        q12(k)=p1(k)+p2(k)
        q13(k)=p1(k)+p3(k)
        q14(k)=p1(k)+p4(k)
        q23(k)=p2(k)+p3(k)
        q24(k)=p2(k)+p4(k)
        q34(k)=p3(k)+p4(k)
        qtot(k)=p1(k)+p2(k)+p3(k)+p4(k)
      enddo
      xm12=sqrt(dmas2(q12))
      xm13=sqrt(dmas2(q13))
      xm14=sqrt(dmas2(q14))
      xm23=sqrt(dmas2(q23))
      xm24=sqrt(dmas2(q24))
      xm34=sqrt(dmas2(q34))


      do k=1,4
          qq1(k)=      p2(k)+p3(k)+p4(k)
          qq2(k)=p1(k)      +p3(k)+p4(k)
          qq3(k)=p1(k)+p2(k)      +p4(k)
          qq4(k)=p1(k)+p2(k)+p3(k)
      enddo
      xm1=sqrt(dmas2(qq1))
      xm2=sqrt(dmas2(qq2))
      xm3=sqrt(dmas2(qq3))
      xm4=sqrt(dmas2(qq4))
      xmtot=sqrt(dmas2(qtot))

      DO I=1,4
        XQ1(I)=QEFF1(I)
        XQ2(I)=QEFF2(I)
      ENDDO

!        do k=1,3
!         xq1(k)=xq1(k)*(xq1(4)-0.5*amel**2/xq1(4))/xq1(4)
!         xq2(k)=xq2(k)*(xq2(4)-0.5*amel**2/xq2(4))/xq2(4)
!        enddo
      do k=1,4
          qq1(k)=xq1(k)      -p1(k)
          qq2(k)=xq2(k)      -p2(k)
          qq3(k)=xq1(k)      -p3(k) 
          qq4(k)=xq2(k)      -p4(k)
      enddo
      Ym1=sqrt(-dmas2(qq1))
      Ym2=sqrt(-dmas2(qq2))
      Ym3=sqrt(-dmas2(qq3))
      Ym4=sqrt(-dmas2(qq4))
      i6=6
CC      write(i6,*) '====OVERVEIGHTED EVT. NR: NEVTRU=',NEVTRU,'====='
CC      write(i6,*) '===generated with channel: ikan=',ikan,'========'
      write(i6,*) 'Note: this event should be outside your detector'
      write(i6,*) 'if not, increase wtmax (?) and/or check manual ?'
      write(i6,*) 'final state: ',IFLAV
CC      write(i6,*) 'wtu= wtmod*wtmod4f/wtmax=',wtovr/wtmax,
CC     $                ' wtmod4f=',wtmod4f
CC      write(i6,*) 'wtu_max=                 ',wtu,'    ... so far'
CC      write(i6,*) '-----------'
      write(i6,*) 'e-prim=',xmtot,'  wtmod4f=',wtmod4f
      write(i6,*) 'active entries from',mrchan,'to',nrchan,';fak=',fak
      do k=1,nrchan/5+1
           write(i6,*)  'jacobians(',5*k-4,'--',5*k,')=',
     $      real(faki(5*k-4)),real(faki(5*k-3)),real(faki(5*k-2))
     $     ,real(faki(5*k-1)),real(faki(5*k)) 
      enddo
      write(i6,*) '-----------'
      write(i6,*) ' m12=',xm12,' m13=',xm13,' m14=',xm14
      write(i6,*) ' m23=',xm23,' m24=',xm24,' m34=',xm34
      write(i6,*) '-----------'
      write(i6,*) ' m-1=',xm1,' m-2=',xm2,' m-3=',xm3,' m-4=',xm4
      write(i6,*) '-----------'
      write(i6,*) ' T-1=',Ym1,' T-2=',Ym2,' T-3=',Ym3,' T-4=',Ym4
      write(i6,*) '-----------'
      write(i6,*) 'qtot' ,qtot
      write(i6,*) 'qcms',xq1(1)+xq2(1),xq1(2)+xq2(2),
     $           xq1(3)+xq2(3),xq1(4)+xq2(4)
      write(i6,*) '-----------------------'
      write(i6,*) '  QEFF1 ' ,QEFF1
      write(i6,*) 'M-QEFF1 ' ,dmas2(qeff1),' ',sqrt(dmas2(xq1))
      write(i6,*) '-----------------------'
      write(i6,*) '  QEFF2 ',QEFF2
      write(i6,*) 'M-QEFF2 ' ,dmas2(qeff2),' ',sqrt(dmas2(xq2))
      write(i6,*) '-----------------------'
      write(i6,*) '-----------------------'
      write(i6,*) 'p1= ',p1
      write(i6,*) '-----------'
      write(i6,*) 'p2= ',p2
      write(i6,*) '-----------'
      write(i6,*) 'p3= ', p3
      write(i6,*) '-----------'
      write(i6,*) 'p4= ',p4
      write(i6,*) '-----------'
      call dumpw(i6)
      do k=1,14
        write(i6,*) '       RRR(',k,')=',RRX(k)
      enddo

!      write(*,*) '=====amp4f here is problem==============='
!      write(*,*) '=====one of inv is too small============='
!      do i=1,6
!      do j=1,6
!        write(*,*) 'i=',i,' j=',j,' pp=',pp(i,j)
!      enddo
!      enddo

!--------------------------------------------------------------------

      END

      subroutine zz_dumper_short(nout)    
*     **************************************************************     
c overweighted events monitoring, ZBW version
c 
c Written by: ZBW,MS        date: 
c Last update:             by:  
c
      implicit DOUBLE PRECISION (a-h,o-z)

      INCLUDE 'zz_phsp.inc'

      i6=nout
      write(i6,*) 'active entries from',mrchan,'to',nrchan,';'
      do k=1,nrchan/5+1
        write(i6,'(A,I3,A,I3,A,5g16.7)')  
     $  'jac.(',5*k-4,'--',5*k,')=',
     $   real(faki(5*k-4)),real(faki(5*k-3)),real(faki(5*k-2))
     $  ,real(faki(5*k-1)),real(faki(5*k)) 
      enddo
      write(i6,*) '----- END DUMP ------'

      END

      SUBROUTINE z_counter(mode,inumber)
!========================================
! counts the callings, for the moment sends result also to the /nevik/
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      COMMON /nevik/ nevtru,ifprint
      SAVE /nevik/

      SAVE icount

      IF( mode.EQ.-1 ) THEN
! reset
        icount=0d0
      ELSEIF( mode .EQ. 0 ) THEN
! add
        icount=icount+1
      ELSEIF( mode .EQ. 1 ) THEN
! print only
        inumber=icount
      ELSE
        WRITE(6,*)'z_counter=> wrong mode: ',mode
        STOP
      ENDIF

      inumber=icount
      nevtru=icount

      END
