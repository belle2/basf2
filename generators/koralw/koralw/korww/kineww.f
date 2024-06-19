

! this is the original kineww.f, parts common for both kinematics
! other auxilliary common routines moved to  kinelib.f

!============================================
! used by M for original WW presampler only
! used by Z also 
!============================================
! invkin is used by Born !!! Not any more,11/20/97 ms. 
!! YES again, 3/4/98 ms.

      subroutine invkin(ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $                  amwmn,amwpn,  bp1,bp2,bp3,bp4)
*     **********************************************************
c This routine calculates inverse kinematics for W-W+ pair production
c and decay 
c OUTPUT:
c         ctn,fin - W-  production angles 
c         ct1n,fi1n - W- decay products angles
c         ct2n,fi2n - W+ decay products angles
c         amwm, amwp - masses of W- and W+ resonances
c INPUT:
c         bp1(4), bp2(4) - four-momenta of W- decay products
c         bp3(4), bp4(4) - four-momenta of W+ decay products
c
c Written by: Wieslaw Placzek            date: 22.07.1994
c Rewritten by: M. Skrzypek              date: 3/15/95
c Last update: 9/5/96                    by: Z.W.
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
      save   / matpar /
      save
      DOUBLE PRECISION bq1(4),bq2(4),aa(4)
      dimension bp1(4),bp2(4),bp3(4),bp4(4)

      do i=1,4
           aa(i)= bp1(i)+bp2(i)
          bq1(i)= bp1(i)+bp2(i)
      enddo
        amwmn=sqrt(dmas2(aa))
      do i=1,4
           aa(i)= bp3(i)+bp4(i)
          bq2(i)= bp3(i)+bp4(i)
      enddo
        amwpn=sqrt(dmas2(aa))
cc ms      qq=dsqrt( (bq1(4)-amwmn)*(bq1(4)+amwmn) )      
      qq=dsqrt(bq1(1)**2+bq1(2)**2+bq1(3)**2)
      qqt=dsqrt(bq1(1)**2+bq1(2)**2)
cc ms
      ctn=bq1(3)/qq
      stn=sqrt(1d0-ctn**2)
cc ms      cfi=bq1(1)/stn/qq
cc ms      sfi=bq1(2)/stn/qq
      IF (QQT.GT.0D0) THEN                                             !cav
        cfi=bq1(1)/qqt
        sfi=bq1(2)/qqt
      ELSE                                                             !cav
        CFI=1D0                                                        !cav
        SFI=0D0                                                        !cav
!        WRITE (6,*)                                                    !cav
!     &    '+++ INVKIN +++ Warning: ThetaW=0. Phi undefined, set=0.'    !cav
      ENDIF                                                            !cav
cc ms
      fin=acos(cfi)
      if(sfi.le.0d0) fin=2*pi-fin
!
      call boostv(1,bq1,bp1,aa)
      qq=sqrt(aa(1)**2+aa(2)**2+aa(3)**2)
      ct1n=aa(3)/qq
cc ms      stn=sqrt(1d0-ct1n**2)
cc ms      cfi=aa(1)/stn/qq
cc ms      sfi=aa(2)/stn/qq
      qqt=sqrt(aa(1)**2+aa(2)**2)
      IF (QQT.GT.0D0) THEN                                             !cav
        cfi=aa(1)/qqt
        sfi=aa(2)/qqt
      ELSE                                                             !cav
        CFI=1D0                                                        !cav
        SFI=0D0                                                        !cav
!        WRITE (6,*)                                                    !cav
!     &    '+++ INVKIN +++ Warning: ThetaW1=0. Phi undefined, set=0.'   !cav
      ENDIF                                                            !cav
cc ms
      fi1n=acos(cfi)
      if(sfi.le.0d0) fi1n=2*pi-fi1n
!
      call boostv(1,bq2,bp3,aa)
      qq=sqrt(aa(1)**2+aa(2)**2+aa(3)**2)
      ct2n=aa(3)/qq
cc ms      stn=sqrt(1d0-ct2n**2)
cc ms      cfi=aa(1)/stn/qq
cc ms      sfi=aa(2)/stn/qq
      qqt=sqrt(aa(1)**2+aa(2)**2)
      IF (QQT.GT.0D0) THEN                                             !cav
        cfi=aa(1)/qqt
        sfi=aa(2)/qqt
      ELSE                                                             !cav
        CFI=1D0                                                        !cav
        SFI=0D0                                                        !cav
!        WRITE (6,*)                                                    !cav
!     &    '+++ INVKIN +++ Warning: ThetaW2=0. Phi undefined, set=0.'   !cav
      ENDIF                                                            !cav

cc ms
      fi2n=acos(cfi)
      if(sfi.le.0d0) fi2n=2*pi-fi2n
! 
      end

      subroutine kineww(sprim,ct,fi,ct1,fi1,ct2,fi2,
     $            amwm,amwp,amdec,  q1,q2,p1,p2,p3,p4)
*     **********************************************************
c This routine calculates kinematics for W-W+ pair production
c and decay in e+e- collision in the CMS with z-axis pointing 
c in the e- direction.
c fixes also the 'effective beams', qeff1,qeff2
c INPUT:  s    - beams energy squared (in GeV**2) !THIS IS DUMMY
                                                  !!!!!!!!!!! ms
c         sprim - actual center mass energy squared (in GeV**2)
c         cthe,fi - W-  production angles 
c         cdec1,fi1 - W- decay products angles
c         cdec2,fi2 - W+ decay products angles
c         amwm, amwp - masses of W- and W+ resonances
c         amdec(4) - decay products masses
c OUTPUT:
c         qeff1(4)      -effective (massless) e- beam in /MOMSET/
c         qeff2(4)      -effective (massless) e+ beam in /MOMSET/
c         q1(4)        - four-momentum of W-  
c         q2(4)        - four-momentum of W+
c         p1(4), p2(4) - four-momenta of W- decay products
c         p3(4), p4(4) - four-momenta of W+ decay products
c
c Written by: Wieslaw Placzek            date: 22.07.1994
c Rewritten by: M. Skrzypek              date: 3/15/95
c Last update: 4/1/95                by: M.S.
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
! sphum,sphot and nphot are almost-dummy (used for printout only)!!!! ms
      common / momset / qeff1(4),qeff2(4),sphum(4),sphot(100,4),nphot  
      save   / matpar /,/ momset /
      save
      DOUBLE PRECISION ef1(4),ef2(4)
      dimension amdec(4),  q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
!
c to activate dumps KINDMP=1
      KINDMP=0

      do i=1,4
        q1(i)=0d0      
        q2(i)=0d0      
        p1(i)=0d0      
        p2(i)=0d0      
        p3(i)=0d0      
        p4(i)=0d0      
        ef1(i)=0d0      
        ef2(i)=0d0      
      enddo
      ecm=sqrt(sprim)
      amwm2=amwm**2
      amwp2=amwp**2
      s1=amwm2
      s2=amwp2
      amp1s=amdec(1)**2
      amp2s=amdec(2)**2
      amp3s=amdec(3)**2
      amp4s=amdec(4)**2
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*) AMDEC(1),AMDEC(2),AMDEC(3),AMDEC(4)
        WRITE(6,*) AMP1S,AMP2S,AMP3S,AMP4S
        WRITE(6,*)'S,SPRIM,S1,S2',S,SPRIM,S1,S2
        WRITE(6,*)'DECAY COS1,2',CT1,CT2
      ENDIF
      st =sqrt(max(0D0,((1d0-ct )*(1d0+ct ))))
c..
      st1=sqrt(max(0D0,((1d0-ct1)*(1d0+ct1))))
      st2=sqrt(max(0D0,((1d0-ct2)*(1d0+ct2))))
c..
!... Momentum q1 of the first resonance
      q1(4)=(sprim+amwm2-amwp2)/(2d0*ecm)
!      qq=dsqrt( (q1(4)-amwm)*(q1(4)+amwm) )
      qq=dsqrt( (sprim-amwm2-amwp2)**2 -4*amwm2*amwp2 )/(2*ecm)
      q1(1)=qq*st*cos(fi)
      q1(2)=qq*st*sin(fi)
      q1(3)=qq*ct
      q1(4)=dsqrt(amwm2+q1(1)**2+q1(2)**2+q1(3)**2)
!... Momentum p1 in the rest frame of the first resonance
      ppene=(s1+amp1s-amp2s)/(2d0*amwm)
!      ppe=dsqrt( (ppene-amdec(1))*(ppene+amdec(1)) )
      ppe=dsqrt( (s1-amp1s-amp2s)**2 -4*amp1s*amp2s )/(2d0*amwm)
      p1(1)=ppe*st1*cos(fi1)
      p1(2)=ppe*st1*sin(fi1)
      p1(3)=ppe*ct1
c      p1(4)=ppene
      p1(4)=dsqrt(amdec(1)**2+p1(1)**2+p1(2)**2+p1(3)**2)
c...
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'Q1,P1 '
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
!... Boost to CMS-WW frame
      call boostv(-1,q1,p1,p1)
!... Momentum p2 of the second product of first resonance decay
      do 10 k=1,4
 10   p2(k)=q1(k)-p1(k)
c.. fine tuning on masses
      p1(4)=dsqrt(amdec(1)**2+p1(1)**2+p1(2)**2+p1(3)**2)
      p2(4)=dsqrt(amdec(2)**2+p2(1)**2+p2(2)**2+p2(3)**2)
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'Q1,P1,P2 '
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
!... Momentum q2 of the second resonance
!ms      q2(4)=ecm-q1(4)
      do 20 k=1,3
 20   q2(k)=-q1(k)
      q2(4)=dsqrt(amwp2+q2(1)**2+q2(2)**2+q2(3)**2)
!... Momentum p3 in the rest frame of the second resonance
      ppene=(s2+amp3s-amp4s)/(2d0*amwp)
!      ppe=dsqrt( (ppene-amdec(3))*(ppene+amdec(3)) )
      ppe=dsqrt( (s2-amp3s-amp4s)**2 -4*amp3s*amp4s )/(2d0*amwp)
      p3(1)=ppe*st2*cos(fi2)
      p3(2)=ppe*st2*sin(fi2)
      p3(3)=ppe*ct2
c      p3(4)=ppene
      p3(4)=dsqrt(amdec(3)**2+p3(1)**2+p3(2)**2+p3(3)**2)
c...
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'Q1,P1,P2,Q2,P3 '
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
!... Boost to CMS-WW frame
      call boostv(-1,q2,p3,p3)
!... Momentum p2 of the second product of first resonance decay
      do 30 k=1,4
 30   p4(k)=q2(k)-p3(k)
c.. fine tuning on masses
      p4(4)=dsqrt(amdec(4)**2+p4(1)**2+p4(2)**2+p4(3)**2)
      p3(4)=dsqrt(amdec(3)**2+p3(1)**2+p3(2)**2+p3(3)**2)
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'Q1,P1,P2,Q2,P3,P4, WW frame '
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'LAB NO PHOTS'
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF

      end

      SUBROUTINE res2gn(mode,svar,sprim,rmas,rgam,amdec,s1,s2,wt)
! #############################################
! LIBRARY of angular etc sub-generators start #
! #############################################

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
      DOUBLE PRECISION AMDEC(4)
      DOUBLE PRECISION drvec(100)
      SAVE a,b,winf,wmi,wma,wsqr,wrec,prsqr,prrec
      SAVE uma,umi,uinf,usqr,urec
      SAVE
!
!        write(6,*)'resms2',rmas,rgam
      a=rmas**2
      b=rmas*rgam
! arctg
      winf = 1/b*atan((svar   -a)/b)
      wma  = 1/b*atan((sprim/4d0-a)/b)
      wmi  = 1/b*atan(        -a /b)
! logarithm
      uinf =1/2d0/a*dlog((svar   -a)**2 +b**2)
      uma  =1/2d0/a*dlog((sprim/4d0-a)**2 +b**2)
      umi  =1/2d0/a*dlog(                b**2)
! thetas
      thespr=1d0
      thesvr=1d0
      IF((sprim/4d0).lt.a) thespr=0d0
      IF( svar      .lt.a) thesvr=0d0
      ulo= thespr*uma +(1d0-thespr)*umi
! normalisations
      wsqr=wma-wmi 
      usqr=thespr*(uma-umi)
      prsqr=(wsqr+usqr)**2
      wrec=winf-wma 
      urec=thesvr*(uinf -ulo)
      prrec=(wsqr+usqr)*(wrec+urec)
      prnorm=prsqr+2*prrec
!
!     ====================
      if (mode.ne.1) then 
!     ====================
!
 10   call varran(drvec,5)
      r1=drvec(1)
      r2=drvec(2)
      r3=drvec(3)
      r4=drvec(4)
      r5=drvec(5)

      IF(r3.le.prsqr/prnorm) THEN
!     ..square

!     ....s1
        IF(r4.ge.usqr/(wsqr+usqr)) THEN
!       ..arctg
          w1=r1*(wma-wmi) +wmi
          s1=b*tan(b*w1) +a
        ELSE
!       ..log
          u1=r1*(uma-umi) +umi
          s1=dsqrt(exp(2*a*u1) -b**2)+a
        ENDIF
!     ....s2
        IF(r5.ge.usqr/(wsqr+usqr)) THEN
!       ..arctg
          w2=r2*(wma-wmi) +wmi
          s2=b*tan(b*w2) +a
        ELSE
!       ..log
          u2=r2*(uma-umi) +umi
          s2=dsqrt(exp(2*a*u2) -b**2)+a
        ENDIF

      ELSEIF(r3.le.(prsqr+prrec)/prnorm) THEN
!     ..rectangle 1

!     ....s1
        IF(r4.ge.usqr/(wsqr+usqr)) THEN
!       ..arctg
          w1=r1*(wma-wmi) +wmi
          s1=b*tan(b*w1) +a
        ELSE
!       ..log
          u1=r1*(uma-umi) +umi
          s1=dsqrt(exp(2*a*u1) -b**2)+a
        ENDIF
!     ....s2
        IF(r5.ge.urec/(wrec+urec)) THEN
!       ..arctg
          w2=r2*(winf-wma) +wma     
          s2=b*tan(b*w2) +a
        ELSE
!       ..log
          u2=r2*(uinf-ulo) +ulo
          s2=dsqrt(exp(2*a*u2) -b**2)+a
        ENDIF

      ELSE
!     ..rectangle 2
!         write(6,*)'rect 1'
!     ....s1
        IF(r4.ge.urec/(wrec+urec)) THEN
!       ..arctg
          w1=r1*(winf-wma) +wma     
          s1=b*tan(b*w1) +a
        ELSE
!       ..log
          u1=r1*(uinf-ulo) +ulo
          s1=dsqrt(exp(2*a*u1) -b**2)+a
        ENDIF
!     ....s2
        IF(r5.ge.usqr/(wsqr+usqr)) THEN
!       ..arctg
          w2=r2*(wma-wmi) +wmi
          s2=b*tan(b*w2) +a
        ELSE
!       ..log
          u2=r2*(uma-umi) +umi
          s2=dsqrt(exp(2*a*u2) -b**2)+a
        ENDIF

      ENDIF
!
!     =====
      endif
!     =====

!
! crude distrib. value is 1/W(s_1)*1/W(s_2) see manual for definition
      xcrud=1d0
      IF(s1.gt.a) xcrud=xcrud*a/s1
      IF(s2.gt.a) xcrud=xcrud*a/s2
      xcrud=xcrud
     $  *((s1-rmas**2)**2 +(rmas*rgam)**2)
     $  *((s2-rmas**2)**2 +(rmas*rgam)**2)
!
      wt=prnorm*xcrud
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
      END


      SUBROUTINE cospro(mode,s,s1,s2,costhe,phipro,wt)
*     ***************************************************
! Crude generation of costhe according to a simplified distribution.
! OUTPUT: costhe - cos(theta), theta - polar angle of W- in the CMS 
!         of the incoming beams (+z axis along e- direction)
!         xccos - value of the function
!                      (for mode=1 costhe becames input for xccos
!                                     - no generation)
c
! Written by: M. Skrzypek            date: 3/1/95
! Last update:                         by: 
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
      DOUBLE PRECISION drvec(100)
      save
!
CC==>>

!ms      wlambd=s**2+s1**2+s2**2-2*s*s1-2*s*s2-2*s1*s2
      wlambd=max(0d0,(s-s1-s2)**2 -4*s1*s2)
!      write(6,*)s,s1,s2,wlambd
      aa=(s-s1-s2)/2d0
      bb=-dsqrt(wlambd)/2d0
      ymi=dlog(aa-bb)/bb
      yma=dlog(s1*s2/(aa-bb))/bb

!      z=.4d0/s*(aa-bb)*2/s
      z=0d0  ! auxilliary, supposed to be 0


! this was `dead code' 05.06.96 ZW:      wt=2*pi/xccos
!
      IF(mode.ne.1) then
      call varran(drvec,3)
      y=drvec(1)*(yma-ymi)+ymi
      costhe=(exp(bb*y)-aa)/bb
      IF( drvec(2).gt.(yma-ymi)/(z+yma-ymi) )  costhe=2*drvec(1)-1  !
c++      write(6,*)'tran cosgen',aa+bb*costhe
      phipro=2*pi*drvec(3)
      endif
!
      xccos=(1/((s1*s2/(aa-bb))+(bb*costhe-bb)) +z/2d0)/(yma-ymi+z) 
      wt=2*pi/xccos 
      end

      SUBROUTINE cosdec(mode,svar,cdec,phi,wt)
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

      KeySpn = MOD(KeyPhy,10000)/1000

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

!====================== unused by MS ====================!
!====================== unused by MS ====================!
!====================== unused by MS ====================!

      subroutine invkintt(ctn,fin,ct1n,fi1n,ct2n,fi2n,
     $                  amwmn,amwpn,  bp1,bp2,bp3,bp4)
*     **********************************************************
c This routine calculates inverse kinematics for W-W+ pair production
c and decay 
c OUTPUT:
c         ctn,fin - W-  production angles 
c         ct1n,fi1n - W- decay products angles
c         ct2n,fi2n - W+ decay products angles
c         amwm, amwp - masses of W- and W+ resonances
c INPUT (to be taken from bormom!):
c         bp1(4), bp2(4) - four-momenta of W- decay products
c         bp3(4), bp4(4) - four-momenta of W+ decay products
c
c Written by: Wieslaw Placzek            date: 22.07.1994
c Rewritten by: M. Skrzypek              date: 3/15/95
c Last update: 9/5/96                    by: Z.W.
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
c.. four momenta in CMS' (effective e- beam along z+, exclusively for WWBORN)
      save  / matpar / 
      save
      DOUBLE PRECISION bq1(4),bq2(4),aa(4),bb(4),e1(4)
      dimension bp1(4),bp2(4),bp3(4),bp4(4)
      do i=1,4
           aa(i)= bp1(i)+bp2(i)
          bq1(i)= bp1(i)+bp2(i)
      enddo
        amwmn=sqrt(dmas2(aa))
      do i=1,4
           aa(i)= bp3(i)+bp4(i)
          bq2(i)= bp3(i)+bp4(i)
      enddo
        amwpn=sqrt(dmas2(aa))
      do i=1,4
           bb(i)= bp1(i)+bp2(i)+bp3(i)+bp4(i)
      enddo
        s=(dmas2(bb))
cc ms      qq=dsqrt( (bq1(4)-amwmn)*(bq1(4)+amwmn) )      
      qq=dsqrt(bq1(1)**2+bq1(2)**2+bq1(3)**2)
      qqt=dsqrt(bq1(1)**2+bq1(2)**2)
cc ms
      ctn=bq1(3)/qq
      stn=sqrt(1d0-ctn**2)
cc ms      cfi=bq1(1)/stn/qq
cc ms      sfi=bq1(2)/stn/qq
      IF (QQT.GT.0D0) THEN                                             !cav
        cfi=bq1(1)/qqt
        sfi=bq1(2)/qqt
      ELSE                                                             !cav
        CFI=1D0                                                        !cav
        SFI=0D0                                                        !cav
!        WRITE (6,*)                                                    !cav
!     &    '+++ INVKINTT +++ Warning: ThetaW=0. Phi undefined, set=0.'  !cav
      ENDIF                                                            !cav

cc ms
      fin=acos(cfi)
      if(sfi.le.0d0) fin=2*pi-fin
!
      e1(4)=dsqrt(s/4d0)
      e1(3)=dsqrt(s/4d0)
      e1(2)=0d0
      e1(1)=0d0
      call boostv(1,bq1,bp1,aa)
      call boostv(1,bq1,e1,e1)
      call rotatv(-1,e1,aa,aa)
      qq=sqrt(aa(1)**2+aa(2)**2+aa(3)**2)
      ct1n=aa(3)/qq
cc ms      stn=sqrt(1d0-ct1n**2)
cc ms      cfi=aa(1)/stn/qq
cc ms      sfi=aa(2)/stn/qq
      qqt=sqrt(aa(1)**2+aa(2)**2)
      IF (QQT.GT.0D0) THEN                                             !cav
        cfi=aa(1)/qqt
        sfi=aa(2)/qqt
      ELSE                                                             !cav
        CFI=1D0                                                        !cav
        SFI=0D0                                                        !cav
!        WRITE (6,*)                                                    !cav
!     &    '+++ INVKINTT +++ Warning: ThetaW1=0. Phi undefined, set=0.' !cav
      ENDIF                                                            !cav
cc ms
      fi1n=acos(cfi)
      if(sfi.le.0d0) fi1n=2*pi-fi1n
! dotad ok
!
!
      e1(4)=dsqrt(s/4d0)
      e1(3)=-dsqrt(s/4d0)
      e1(2)=0d0
      e1(1)=0d0
      call boostv(1,bq2,bp4,aa)
      call boostv(1,bq2,e1,e1)
      aa(3)=-aa(3)
      call rotatv(-1,e1,aa,aa)
      qq=sqrt(aa(1)**2+aa(2)**2+aa(3)**2)
      ct2n=aa(3)/qq
cc ms      stn=sqrt(1d0-ct2n**2)
cc ms      cfi=aa(1)/stn/qq
cc ms      sfi=aa(2)/stn/qq
      qqt=sqrt(aa(1)**2+aa(2)**2)
      IF (QQT.GT.0D0) THEN                                             !cav
        cfi=aa(1)/qqt
        sfi=aa(2)/qqt
      ELSE                                                             !cav
        CFI=1D0                                                        !cav
        SFI=0D0                                                        !cav
!        WRITE (6,*)                                                    !cav
!     &    '+++ INVKINTT +++ Warning: ThetaW2=0. Phi undefined, set=0.' !cav
      ENDIF                                                            !cav

cc ms
      fi2n=acos(cfi)
      if(sfi.le.0d0) fi2n=2*pi-fi2n
! 
      end

      SUBROUTINE cosprozz(mode,s,s1,s2,costhe,phipro,wt)
*     ***************************************************
! Crude generation of costhe according to a simplified distribution.
! OUTPUT: costhe - cos(theta), theta - polar angle of W- in the CMS 
!         of the incoming beams (+z axis along e- direction)
!         xccos - value of the function
!                      (for mode=1 costhe becames input for xccos
!                                     - no generation)
c
! Written by: M. Skrzypek            date: 3/1/95
! Last update:                         by: 
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler 
      common /nevik/ nevtru,ifprint
      save /nevik/

      DOUBLE PRECISION drvec(100)
      save
!
CC==>>

!ms      wlambd=s**2+s1**2+s2**2-2*s*s1-2*s*s2-2*s1*s2
      wlambd=max(0d0,(s-s1-s2)**2 -4*s1*s2)
!      write(6,*)s,s1,s2,wlambd
      aa=(s-s1-s2)/2d0
      bb=-dsqrt(wlambd)/2d0
      ymi=dlog(aa-bb)/bb
!      yma=dlog(aa+bb)/bb
      yma=dlog(s1*s2/(aa-bb))/bb
!      z=.4d0/s*(aa-bb)*2/s
      z=0d0  ! auxilliary, supposed to be 0
      IF(mode.eq.0) then

       call varran(drvec,3)

       y=drvec(1)*(yma-ymi)+ymi
       IF( drvec(2).lt.1d0/3d0) then
        costhe=(exp(bb*y)-aa)/bb
       elseIF( drvec(2).lt.2d0/3d0) then
        costhe=-(exp(bb*y)-aa)/bb
       else
        costhe=2*drvec(1)-1  !
       endif
       phipro=2*pi*drvec(3)
      endif
      xccos=1d0/3d0/((s1*s2/(aa-bb))+(bb*costhe-bb))/(yma-ymi)
     $     +1d0/3d0/((s1*s2/(aa-bb))-(bb*costhe+bb))/(yma-ymi)
     $     +1d0/6d0
c++      write(6,*)'tran cosgen',aa+bb*costhe
      wt=2*pi/xccos 
      if (ifprint.eq.1) then
      write(*,*) 'cosprozz',aa,bb,costhe
      write(*,*) ((s1*s2/(aa-bb))+(bb*costhe-bb))*(yma-ymi)
      write(*,*) (s1*s2/(aa-bb)-bb)*(yma-ymi),bb*(yma-ymi)
      write(*,*) ((s1*s2/(aa-bb))-(bb*costhe+bb))*(yma-ymi)
      endif

      end

!========== unused by anybody, kept for future generations =========
!========== unused by anybody, kept for future generations =========
!========== unused by anybody, kept for future generations =========

      subroutine kinett(s,sprim,ct,fi,ct1,fi1,ct2,fi2,
     $                  amwm,amwp,amdec,  q1,q2,p1,p2,p3,p4)
*     **********************************************************
c This routine calculates kinematics for W-W+ pair production
c and decay in e+e- collision in the CMS with z-axis pointing 
c in the e- direction.
c fixes also the 'effective beams', qeff1,qeff2
c INPUT:  s    - beams energy squared (in GeV**2)
c         sprim - actual center mass energy squared (in GeV**2)
c         cthe,fi - W-  production angles 
c         cdec1,fi1 - W- decay products angles
c         cdec2,fi2 - W+ decay products angles
c         amwm, amwp - masses of W- and W+ resonances
c         amdec(4) - decay products masses
c OUTPUT:
c         qeff1(4)      -effective (massless) e- beam in /MOMSET/
c         qeff2(4)      -effective (massless) e+ beam in /MOMSET/
c         q1(4)        - four-momentum of W-  
c         q2(4)        - four-momentum of W+
c         p1(4), p2(4) - four-momenta of W- decay products
c         p3(4), p4(4) - four-momenta of W+ decay products
c
c Written by: Wieslaw Placzek            date: 22.07.1994
c Rewritten by: M. Skrzypek              date: 3/15/95
c Last update: 4/1/95                by: M.S.
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / matpar / pi,ceuler     
      common / momset / qeff1(4),qeff2(4),sphum(4),sphot(100,4),nphot  
      save   / matpar /,/ momset /
      save
      DOUBLE PRECISION ef1(4),ef2(4)
      dimension amdec(4), q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      dimension e1(4),e2(4)   !! t-channel stuff
!
c to activate dumps KINDMP=1
      KINDMP=0
c to activate t-channel KINTCH=1
      KINTCH=1

      do i=1,4
        q1(i)=0d0      
        q2(i)=0d0      
        p1(i)=0d0      
        p2(i)=0d0      
        p3(i)=0d0      
        p4(i)=0d0      
        ef1(i)=0d0      
        ef2(i)=0d0      
      enddo

      ecm=sqrt(sprim)
      amwm2=amwm**2
      amwp2=amwp**2
      s1=amwm2
      s2=amwp2
      amp1s=amdec(1)**2
      amp2s=amdec(2)**2
      amp3s=amdec(3)**2
      amp4s=amdec(4)**2
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*) AMDEC(1),AMDEC(2),AMDEC(3),AMDEC(4)
        WRITE(6,*) AMP1S,AMP2S,AMP3S,AMP4S
        WRITE(6,*)'S,SPRIM,S1,S2',S,SPRIM,S1,S2
        WRITE(6,*)'DECAY COS1,2',CT1,CT2
      ENDIF
      st =sqrt(max(0D0,((1d0-ct )*(1d0+ct ))))
c..
      st1=sqrt(max(0D0,((1d0-ct1)*(1d0+ct1))))
      st2=sqrt(max(0D0,((1d0-ct2)*(1d0+ct2))))
c..
!... Momentum q1 of the first resonance
      q1(4)=(sprim+amwm2-amwp2)/(2d0*ecm)
!      qq=dsqrt( (q1(4)-amwm)*(q1(4)+amwm) )
      qq=dsqrt( (sprim-amwm2-amwp2)**2 -4*amwm2*amwp2 )/(2*ecm)
      q1(1)=qq*st*cos(fi)
      q1(2)=qq*st*sin(fi)
      q1(3)=qq*ct
      q1(4)=dsqrt(amwm2+q1(1)**2+q1(2)**2+q1(3)**2)
!... Momentum p1 in the rest frame of the first resonance
      ppene=(s1+amp1s-amp2s)/(2d0*amwm)
!      ppe=dsqrt( (ppene-amdec(1))*(ppene+amdec(1)) )
      ppe=dsqrt( (s1-amp1s-amp2s)**2 -4*amp1s*amp2s )/(2d0*amwm)
      p1(1)=ppe*st1*cos(fi1)
      p1(2)=ppe*st1*sin(fi1)
      p1(3)=ppe*ct1
c      p1(4)=ppene
      p1(4)=dsqrt(amdec(1)**2+p1(1)**2+p1(2)**2+p1(3)**2)
      IF(KINTCH.EQ.1)THEN
!!!!!!! for cosdec_t !!!!!!!
!beam
      e1(4) =        dsqrt(s/4d0)
      e1(3) =        dsqrt(s/4d0)
      e1(2) =        0d0
      e1(1) =        0d0
!boost beam to W- rest fr.
      call boostv(1,q1,e1,e1)
!rotate p1 from frame parallel to ef1 to CMS-oriented
      call rotatv(1,e1,p1,p1)
!!!!!!! end for cosdec_t !!!!!!!
      ENDIF

c...
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'Q1,P1 '
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
!... Boost to CMS-WW frame
      call boostv(-1,q1,p1,p1)
!... Momentum p2 of the second product of first resonance decay
      do 10 k=1,4
 10   p2(k)=q1(k)-p1(k)
c.. fine tuning on masses
      p1(4)=dsqrt(amdec(1)**2+p1(1)**2+p1(2)**2+p1(3)**2)
      p2(4)=dsqrt(amdec(2)**2+p2(1)**2+p2(2)**2+p2(3)**2)
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'Q1,P1,P2 '
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
!... Momentum q2 of the second resonance
!ms      q2(4)=ecm-q1(4)
      do 20 k=1,3
 20   q2(k)=-q1(k)
      q2(4)=dsqrt(amwp2+q2(1)**2+q2(2)**2+q2(3)**2)
      IF(KINTCH.NE.1)THEN
!... Momentum p3 in the rest frame of the second resonance
      ppene=(s2+amp3s-amp4s)/(2d0*amwp)
!      ppe=dsqrt( (ppene-amdec(3))*(ppene+amdec(3)) )
      ppe=dsqrt( (s2-amp3s-amp4s)**2 -4*amp3s*amp4s )/(2d0*amwp)
      p3(1)=ppe*st2*cos(fi2)
      p3(2)=ppe*st2*sin(fi2)
      p3(3)=ppe*ct2
c      p3(4)=ppene
      p3(4)=dsqrt(amdec(3)**2+p3(1)**2+p3(2)**2+p3(3)**2)
c...
      ELSE
!!!!!!! for cosdec_t !!!!!!!
!... Momentum p4 in the rest frame of the second resonance
      ppene=(s2+amp4s-amp3s)/(2d0*amwp)
!      ppe=dsqrt( (ppene-amdec(4))*(ppene+amdec(4)) )
      ppe=dsqrt( (s2-amp3s-amp4s)**2 -4*amp3s*amp4s )/(2d0*amwp)
      p4(1)=ppe*st2*cos(fi2)
      p4(2)=ppe*st2*sin(fi2)
      p4(3)=ppe*ct2
c      p4(4)=ppene
      p4(4)=dsqrt(amdec(4)**2+p4(1)**2+p4(2)**2+p4(3)**2)
!beam
      e2(4) =        dsqrt(s/4d0)
      e2(3) =       -dsqrt(s/4d0)
      e2(2) =        0d0
      e2(1) =        0d0
!boost beam to W- rest fr.
      call boostv(1,q2,e2,e2)
!rotate p1 from frame parallel to ef1 to CMS-oriented
      call rotatv(1,e2,p4,p4)
      p4(1)=p4(1)
      p4(2)=p4(2)
      p4(3)=-p4(3)
!!!!!!! end for cosdec_t !!!!!!!
      ENDIF

      IF(KINDMP.EQ.1)THEN
        IF(KINTCH.NE.1) WRITE(6,*)'Q1,P1,P2,Q2,P3 '
        IF(KINTCH.EQ.1) WRITE(6,*)'Q1,P1,P2,Q2,P4 '
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
!... Boost to CMS-WW frame
        IF(KINTCH.NE.1) THEN
          call boostv(-1,q2,p3,p3)
        ELSE
          call boostv(-1,q2,p4,p4)
        ENDIF
!... Momentum p3 of the second product of second resonance decay
      do 30 k=1,4
        IF(KINTCH.NE.1) THEN
          p4(k)=q2(k)-p3(k)
        ELSE
          p3(k)=q2(k)-p4(k)
        ENDIF
 30     continue
c.. fine tuning on masses
      p4(4)=dsqrt(amdec(4)**2+p4(1)**2+p4(2)**2+p4(3)**2)
      p3(4)=dsqrt(amdec(3)**2+p3(1)**2+p3(2)**2+p3(3)**2)
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'Q1,P1,P2,Q2,P3,P4, WW frame '
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF
      IF(KINDMP.EQ.1)THEN
        WRITE(6,*)'LAB NO PHOTS'
        CALL DUMPL(6,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)
      ENDIF

      end


      SUBROUTINE cosdec_t_mm(mode,ibeam,svar,sprim,s1,s2,ct,fi,ambeam,
     @                    amfi1,amfi2,   costhe,phi,wt)
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
C simplified version of this routine.
      xx=4*ambeam**2/svar
      beta=sqrt(1d0-4*ambeam**2/svar)
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
      endif
      cost=min(1d0,costhe)
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

      SUBROUTINE cosdec_xt(mode,ibeam,svar,sprim,s1,s2,ct,fi,ambeam,
     @                    amfi1,amfi2,   costhe,phi,wt)
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
      DOUBLE PRECISION drvec(100),q1(4),e1(4),e1p(4)
      save

      ecm=sqrt(sprim)
      amwm=dsqrt(s1)
      amwp=dsqrt(s2)
      amwm2=s1
      amwp2=s2
      st =sqrt((1-ct )*(1+ct ))

!... Momentum q1 of the first resonance
      q1(4)=(sprim+amwm2-amwp2)/(2d0*ecm)
!      qq=dsqrt( (q1(4)-amwm)*(q1(4)+amwm) )
      qq=dsqrt( (sprim-amwm2-amwp2)**2 -4*amwm2*amwp2 )/(2*ecm)
      q1(1)=-ibeam*qq*st*cos(fi)
      q1(2)=-ibeam*qq*st*sin(fi)
      q1(3)=-ibeam*qq*ct
      q1(4)=dsqrt(amwm2+q1(1)**2+q1(2)**2+q1(3)**2)

!beam
      e1(4) =        dsqrt(svar/4d0)
      e1(3) = -ibeam*dsqrt(svar/4d0 -ambeam**2)
      e1(2) =        0d0
      e1(1) =        0d0
!transform beam to W rest fr.
!      call boostv(1,q1,q1,e1p)
!      write(6,*)'cosdec_t=>',e1
!      write(6,*)'        =>',e1p,amwm
!      call rotatv(-1,q1,q1,e1p)
!      write(6,*)'        =>',e1p
      call boostv(1,q1,e1,e1p)
!first fermion
      x0 = 1/2d0/amwm*(amwm2-amfi2**2+amfi1**2)
      xx = dsqrt(x0**2 -amfi1**2)
      ee = dsqrt(e1p(4)**2 -ambeam**2)


      aa = x0*e1p(4)   !-(ambeam**2+amfi1**2)/2d0
      bb = -xx*ee
cc      write(6,*)'aa,bb',aa,bb,aa+bb
cc      write(6,*)'x0,e1p(4),xx,ee',x0,e1p(4),xx,ee

      cosmax =   1-2d-8
      cosmin = -(1-2d-8)

      ymi=dlog(aa+cosmin*bb)/bb
      yma=dlog(aa+cosmax*bb)/bb
cc      tmin=11d0
cc      yma=dlog(tmin)/bb

      z=.4d0/svar*(aa-bb)*2/svar
      z=.1d0*(aa-bb)/svar
      z=.5d0*(aa-bb)**2/svar**2
      z=.02d0
      z=.004d0*svar/(aa-bb)          !! best of all (180gev) !!
!!      write(6,*)'zety ',(aa-bb)/svar,(aa-bb)**2/svar,(aa-bb)**2/svar**2
!      z=0d0  ! auxilliary, supposed to be 0

      IF(mode.eq.0) then

       call varran(drvec,3)

       y=drvec(1)*(yma-ymi)+ymi
       costhe=(exp(bb*y)-aa)/bb
       IF( drvec(2).gt.(yma-ymi)/(z+yma-ymi) )  costhe=2*drvec(1)-1 !
       phi=2*pi*drvec(3)
      endif
      xccos=2*(1/(aa+bb*costhe) +z/(cosmax-cosmin))/(yma-ymi+z) 
      wt= 4*pi/xccos
!
cc      write(6,*)'tran cosgen',aa+bb*costhe,costhe,xccos,xcc/xccos
cc      write(6,*)'tran yma,ymi',yma,ymi,yma-ymi

      end

      SUBROUTINE res3gn_mm(mode,svar,sprim,rmas,rgam,amdec,s1,s2,wt)
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
!      SAVE / matpar /,/ articut /,/ WEKIN3 /     
      SAVE / matpar /,/ WEKIN3 /  
      DOUBLE PRECISION AMDEC(4),amd(4)
      DOUBLE PRECISION drvec(100)
      SAVE
      do k=1,4
       amd(k)=amdec(k)
       if (amd(k).lt.0.0005d0) amd(k)=0.000511d0
      enddo

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
 10   call varran(drvec,5)
      r1=drvec(1)
      r2=drvec(2)
      r3=drvec(3)
      r4=drvec(4)
      r5=drvec(5)

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
      prnorm=ph1*ph2
      wt=prnorm
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
      write(6,*) 'vol=',ph1,ph2
      write(6,*) sqrt(s1),'+',sqrt(s2),'.gt.',sqrt(sprim)
      write(6,*) amdec
      endif
      END


