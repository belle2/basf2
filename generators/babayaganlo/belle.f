      subroutine belle(MODE,XPAR,NPAR)

      implicit double precision (a-h,o-z)
      integer*8 npoints,k,naccepted,iwriteout
      integer*8 nmintrials,nmaxtrials
      integer*8 nsearchloop
      parameter (mph=40)
      real*4 csi(1)
      dimension sump(0:mph-1),sum2p(0:mph-1)
      dimension fractions(0:mph-1)
      dimension xsecp(0:mph-1),varp(0:mph-1)
      dimension pin(0:3),p1(0:3),p2(0:3),qph(mph,0:3),q(0:3),qcut(0:3)
      dimension qphcut(mph,0:3)
      dimension pin1(0:3),pin2(0:3),ptmp(0:3),pbeam1(0:3),pbeam2(0:3)
      dimension pbeam1p(0:3),pbeam2p(0:3)
      dimension p1b(0:3),p2b(0:3)
      dimension dir(3)
      dimension p1cut(0:3),p2cut(0:3),pcm(0:3)
      integer isvec(25)
      character*6   darkmod
      character*6   arun
      character*6   ord
      character*10  model
      character*100 outfile,storefile
      character*3   eventlimiter,store
      common/beforesort/p1o(0:3),p2o(0:3),qpho(40,0:3)
      common/testsdebug/elmat2cmn,e2approx,s12,s34,t24,t13,u14,u23
      common/debug2/p1mod,odv
      common/eg_branchsampling/ibranch
*** filled in the subroutine userinterface
      character*2 fs
      common/finalstate/fs
      integer*8 ifs
      common/ecms/ecms
      common/nphot_mode/nphotmode
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/zparameters/zm,gz,ve,ae,rv,ra,wm,s2th,gfermi,sqrt2,um
      common/epssoft/eps
      common/energiabeam/ebeam
      common/parameters/ame,ammu,convfac,alpha,pi
      common/intinput/iwriteout,iseed,nsearch,iverbose
      common/qedORDER/ord
      integer*8 iord
      common/charinput/model,eventlimiter,store,storefile,outfile
       integer*8 imodel
      common/realinput/anpoints,sdifmax
      common/nmaxphalpha/nphmaxalpha
      common/ialpharunning/iarun
      common/idebugging/idebug
      common/regolatori/regulator1,regulator2
*********************************************
      common/ggreg/ipair
      common/coseni/cmax,cmin
      common/channelref/iref
      common/momentainitial/pin1,pin2
      common/radpattern/nph(4)
      common/forborncrosssection/phsp2b,flux2b,bornme,bornmeq2,bornmez
      common/reducedtoborn/p1b,p2b
      common/samplelimits/spmin,omaxext
      common/various/beta,betafs
      common/for_debug/ppp(0:3),denphsp,dir3(0:3),dir4(0:3),br,bq,onm
      common/funvalues/funa,funb,func,funm
      common/enableifirst/ifrstenbld

      common/idarkon/idarkon
      common/darkmatter/amassU,gammaU,gvectU,gaxU
      
      integer iteubn
      common/teubner/iteubn

      character*62 welcome(23)
      data welcome/
     +'12345678901234567890123456789012345678901234567890123456789012',!1
     +' *************************************************************',!2
     +' **********                                         **********',!3
     +' ********                                             ********',!4
     +' ******             Welcome to BabaYaga                 ******',!5
     +' ****             ^^^^^^^^^^^^^^^^^^^^^^^                 ****',!6
     +' **       It is an event generator for QED processes        **',!7
     +' ****      at low energies, matching a QED PS with        ****',!8
     +' ******           exact order alpha corrections         ******',!9
     +' ********                                             ********',!10
     +' **********                                         **********',!11
     +' *************************************************************',!12
     +'  [[ it simulates: e+e- -->>  g  -->> e+e- or mu+mu- or gg ]] ',!13
     +'  [[             : e+e- -->> g,U -->> e+e-g or mu+mu-g     ]] ',!14
     +' *****     *      *****     *           *****   ******    *   ',!15
     +' *        * *     *        * *         *    *   *        * *  ',!16
     +' *        * *     *        * *         *    *   *        * *  ',!17
     +' ****    *   *    ****    *   *         *****   *       *   * ',!18
     +' *   *   *****    *   *   *****          *  *   *       ***** ',!19
     +' *   *  *     *   *   *  *     *        *   *   *      *     *',!20
     +' ****   *     *   ****   *     *        *   *   *      *     *',!21
     +' ',
     +' '/
     
!       =========================================
!       =========================================
!       input parameters
      DIMENSION XPAR(0:99), NPAR(0:99)
!       output parameters
      COMMON / BRESULTS / RESCROSS, RESCROSSERR, RESCROSSPHOT(0:39), RESCROSSPHOTERR(0:39), RESCROSSPHOTFRAC(0:39), RESNPHMAX
      COMMON / BHITNMISS / HNMCROSS, HNMCROSSERR, HNMEFF, HNMSDIFMAX, HNMFMAX, HNMMAXTRIALLIMIT, HNMMAXTRIAL
      COMMON / BBIAS / BIASBIASHIT, BIASBIASHITPMISS, BIASBIASNEGHIT, BIASBIASNEGHITMISS, BIASSEZOVER, BIASERRSEZOVER, BIASSEZNEG, BIASERRSEZNEG, BIASNOVER, BIASNNEG
      integer BNPHOT
      COMMON / MOMSET / BP1(0:3),BQ1(0:3),BP2(0:3),BQ2(0:3),BPHOT(0:99,0:3),BNPHOT
      double precision MONSDIF
      COMMON / BMONITORING / MONSDIF
      
!       =========================================
!       =========================================
!       =========================================
!       =========================================
!       =========================================
!       =========================================
!       =========================================
!       =========================================
!       =========================================
!       INIT
      IF(MODE.EQ.-1) THEN
      
!       do k = 2,14
!          print*,welcome(k)
!       enddo      

      ecms    = XPAR(0)      
      pi      = XPAR(1)
      degtorad = pi/180.D0

      convfac = XPAR(2)
      alpha   = XPAR(3)
      ame     = XPAR(4)
      ammu    = XPAR(5)      
      wm      = XPAR(6)
      zm      = XPAR(7)
      gz      = XPAR(8)
      emin    = XPAR(9)
      zmax    = XPAR(10)*degtorad
      eps     = XPAR(11)
!       eps     = 5d-4
      
      s2th    = 1.d0 - wm**2/zm**2
      sqrt2   = sqrt(2.d0)
      gfermi  = pi*alpha/2.d0/s2th/wm**2*sqrt2
      ve      = -1.d0 + 4.d0*s2th
      ae      = -1.d0
      rv      = ve**2/(ae**2+ve**2)
      ra      = ae**2/(ae**2+ve**2)
      um      = sqrt(pi*alpha/sqrt2/gfermi)
      

      fmax    = XPAR(30) 
      
!        WRAP STRING PARAMETER, ONLY CALLED ONCE, SO DONT MIND ANY PERFOMANCE ISSUES (TF)
      fs = 'ee'
      ifs = NPAR(20)
      IF (ifs.EQ.1) THEN 
	fs = 'ee'
      ELSEIF (ifs.EQ.2) THEN 
	fs = 'gg'
      ELSEIF (ifs.EQ.3) THEN 
	fs = 'mm'
      END IF      

      model = 'matched'
      imodel = NPAR(21)
      IF (imodel.EQ.1) THEN 
	model = 'matched'
      ELSEIF (imodel.EQ.2) THEN 
	model = 'ps'
      END IF      

      ord = 'exp'
      iord = NPAR(22)
      IF(iord.EQ.1) THEN 
	ord = 'exp'
      END IF
      
      nphotmode = NPAR(0)

      thmin = XPAR(20)
      thmax = XPAR(21)
      thmin = thmin*degtorad
      thmax = thmax*degtorad
      thgmin = thmin 
      thgmax = thmax
      ebeam = ecms/2.d0
      
!       iseed    = 700253512
      
      nsearch  = NPAR(1)
      anpoints = nsearch
      egmin    = 0.02d0
      sdifmax  = 1.d-18
!       darkmod      = 'off'
      amassU       = 0.4d0
      gammaU       = -1. 
      gvectU       = 1.d-3
      gaxU         = 0.d0
      amassainvmin = 0.d0
      amassainvmax = ecms
      idarkon = 0
      iarun = 0

!     RUNNING ALPHA
      arun='hadr5'
      iarun = 0
      if (arun.eq.'on'.or.arun.eq.'hadr5') then
        iarun = 1
        iteubn = 0
      elseif (arun.eq.'hmnt') then
        iarun = 1
        iteubn = 1
      else
        iarun = iarun
      endif
      
!       if (iarun.eq.1) then
! 	if (iteubn.lt.1) then
! 	  print*,'                  '
! 	  print*, 'Using HADR5N09 routine for Delta alpha.'
! 	else
! 	  print*,'                  '
! 	  print*, 'Using HMNT routine for Delta alpha.'
! 	endif
!       endif
      
!       call rluxgo(lux,iseed,k1,k2)

! CALLED FROM INTERFACE.f (TF)
      nphmaxalpha = 6

      idebug = 0
      cmax   = dcos(thmin)
      cmin   = dcos(thmax)
      npoints = anpoints           

!       print*,'SETTING UP'
!       print *, "events for maximum search: " ,nsearch
!       print *, "fmax:                      " ,fmax
!       print*,'fs:    ',fs
!       print*,'ord:   ',ord
!       print*,'eps:   ',eps
!       print*,'  soft cutoff:   ',eps * ecms/2.d0
!       print*,'model: ',model
!       print*,'E_CMS: ',ecms
!       print*,'THMIN: ',thmin
!       print*,'THMAX: ',thmax
!       print*,'ACOL: ',zmax
!       print*,'EMIN: ',emin
!       print*,'Mass (Z): ',zm
!       print*,'Mass (W): ',wm
!       print*,'Mass (e): ',ame
!       print*,'Mass (mu): ',ammu
!       print*,'Width (Z): ',gz
!       print*,'s2th (on shell):   ',s2th
!       print*,'GFermi (on shell): ',gfermi
!       print*,'hbarc --> nb:      ',convfac
!       print*,'alpha(0):          ',alpha  
!       print*,'nphotmode:         ',nphotmode  
!       print*,'running alpha:           ',iarun  
!       print*,'running alpha (Teubner): ',iteubn  

      am1 = ame
      am2 = ame
      if (fs.eq.'gg') then
         am1 = 0.d0
         am2 = 0.d0
      endif
      if (fs.eq.'mm') then
         am1 = ammu
         am2 = ammu
      endif
      in_conf_spin = 4

      sum = 0.d0
      sum2 = 0.d0
      do k = 0,mph-1
         sump(k) = 0.d0
         sum2p(k) = 0.d0
      enddo
      
      nmintrials   = 0
      nmaxtrials   = 0
      nphmax       = 0
      xsec         = 0.d0
      var          = 0.1d0
      naccepted    = 0
      sumb1        = 0.d0
      sumb2        = 0.d0
      bornmax      = 0.d0
      phspmax      = 0.d0
      nwhenmax     = 0
      wmax         = 0.d0
      prodmax      = 0.d0
      elmmax       = 0.d0
      nover        = 0
      hitpmiss     = 0.d0
      hit          = 0.d0
      istopsearch  = 0
      nneg         = 0
      sumover      = 0.d0
      sum2over     = 0.d0
      sumneg       = 0.d0
      sum2neg      = 0.d0
      
      k = 0
      ntot = 0d0
      nhnm = 0d0
      
      wnpoints = nsearch
      if(fmax.gt.0d0) wnpoints = 0
      
      do j = 0,3
         do ki = 1,mph
            qph(ki,j) = 0.d0
         enddo
      enddo
      ng = 0
      
!       FIND MAXIMUM
        do while(k.lt.wnpoints)
**** THE FOLLOWING LINES BETWEEN "*[[ start" AND "* end ]]" MUST
**** BE RECALCULATED FOR EACH EVENT IF ECMS CHANGES EVENT BY EVENT.
**** FURTHERMORE, IF ECMS CHANGES EVENT BY EVENT ifrstenbld MUST 
**** BE SET TO 0
*[[ start
        ifrstenbld = 0
        esoft = eps * ecms/2.d0
        pin(0) = ecms
        pin(2) = 0.d0
        pin(3) = 0.d0
        pin(1) = 0.d0
        betafs = sqrt(1.d0 - 4.d0*am1**2/pin(0)**2)
        beta   = sqrt(1.d0 - 4.d0*ame**2/pin(0)**2)
        pin1(0) = pin(0)/2.d0
        pin1(1) = 0.d0
        pin1(2) = 0.d0
        pin1(3) = beta * pin1(0)
        pin2(0) = pin(0)/2.d0
        pin2(1) = 0.d0
        pin2(2) = 0.d0
        pin2(3) = -beta * pin2(0)
        do ki = 0,3
           pbeam1(ki) = pin1(ki)
           pbeam2(ki) = pin2(ki)
           ptmp(ki)   = pin1(ki)+pin2(ki)
        enddo
        s = dot(ptmp,ptmp)
* end ]]

** [[end initialization]]

         k = k + 1
         ntot = ntot + 1
         
         flux = 8.d0 * (ecms/2.d0)**2
         if (fs.eq.'ee'.or.fs.eq.'mm') call get_cos_fer(cth,wcos)

         call multiplicity(eps,ecms,cth,ng,wnphot)

         if (fs.eq.'gg') then
            if (ng.eq.0) then 
               call get_cos_2g_born(cth,wcos)
            else
               call get_cos_2g(cth,wcos)
            endif
         endif
         sdif = wnphot*wcos

***************************************************
** initial state radiation with structure functions
*
      if (ord.eq.'struct') then
          call emission(pbeam1,pbeam2,wisr,'e')

          betafs = sqrt(1.d0 - 4d0*am1**2/(pbeam1(0)+pbeam2(0))**2)
          beta   = sqrt(1.d0 - 4d0*ame**2/(pbeam1(0)+pbeam2(0))**2)
*
          sreduced =   (pbeam1(0)+pbeam2(0))**2 
     +               - (pbeam1(3)+pbeam2(3))**2

          pbeam1p(0) = sqrt(sreduced)/2.d0
          pbeam1p(1) = 0d0
          pbeam1p(2) = 0d0
          pbeam1p(3) = beta*pbeam1p(0)
*
          pbeam2p(0) = pbeam1p(0)
          pbeam2p(1) = 0d0
          pbeam2p(2) = 0d0
          pbeam2p(3) = -pbeam1p(3)
*
          sdif = sdif * wisr
      else
          do kk=0,3
              pbeam1p(kk) = pbeam1(kk)
              pbeam2p(kk) = pbeam2(kk)
          enddo
      endif
!       print *,sdif

*
** end of initial state radiation
*******************************************

         call phasespace(pbeam1p,pbeam2p,p1,p2,qph,ng,am1,am2,
     .        esoft,cth,w,phsp,ie)
         sdif = sdif * phsp * w

*
** final state radiation with structure function
*
          if (ord.eq.'struct'.and.ie.eq.0) then
              do kk = 0,3
                  p1cut(kk) = p1(kk)
                  p2cut(kk) = p2(kk)
                  pcm(kk)   = pbeam1(kk) + pbeam2(kk)
              enddo
              do ii = 1,ng
                  do kk = 0,3
                      qcut(kk)  = qph(ii,kk)
                  enddo
                  call new_boost(pcm,qcut,qcut,-1)
                  do kk = 0,3
                      qphcut(ii,kk)  = qcut(kk)
                  enddo
               enddo
                  
              call new_boost(pcm,p1cut,p1cut,-1)
              call new_boost(pcm,p2cut,p2cut,-1)
     
              if (fs.eq.'mm') call emission(p1cut,p2cut,wfsr,'m')
              if (fs.eq.'ee') call emission(p1cut,p2cut,wfsr,'e')
     
          else
              do kk = 0,3
                  p1cut(kk) = p1(kk)
                  p2cut(kk) = p2(kk)
              enddo
              do ii = 1,ng
                  do kk = 0,3
                      qphcut(ii,kk)  = qph(ii,kk)
                  enddo
              enddo
          endif
*
** end of final state radiation
*****************************************

         if (ie.ge.1) ie = 1
         if (fs.eq.'gg'.and.ie.eq.0) then
            call mixandsortmomenta(ng,p1,p2,qph)
         endif

         if (ie.lt.1) then 
             if (fs.eq.'ee'.or.fs.eq.'mm') then
                 call cuts(p1cut,p2cut,qphcut,icut)
             elseif (fs.eq.'gg') then
                 call cutsgg(ng,p1cut,p2cut,qphcut,icut)
             endif
         else
            icut = 1
         endif
         ie = ie + icut

         if (ng.gt.nphmax.and.ie.eq.0) nphmax = ng
         if (icut.eq.0) naccepted = naccepted + 1

!!         ie = 1   ! uncomment  for phase space integral

         call squared_matrix(model,ng,ecms,p1,p2,pbeam1,pbeam2,qph,
     >         ie,icalc,emtx,prod)

         bck  = emtx
         emtx = emtx/in_conf_spin ! divided by initial spin conf. 
         emtx = emtx*convfac/flux ! divided by the flux

         if (fs.eq.'gg') emtx = emtx/(ng+2)/(ng+1)

         if (ie.eq.0) then
            call svfactor(model,ng,ecms,p1,p2,eps,sv,deltasv)
            sdif = sdif * sv
         else
            sdif = 0.d0
         endif

         sdif = sdif * emtx

         if (sdif.lt.0.d0) then
             sdif = 0.d0
             w    = 0.d0
             ie   = 1
         endif

         if (ord.eq.'struct'.and.ie.eq.0) then
            do kk = 0,3
                p1(kk) = p1cut(kk)
                p2(kk) = p2cut(kk)
                qph(1,kk) = qcut(kk)
            enddo
            sdif = sdif * wfsr
         endif

! rescaling for Z exchange (bornmez = 0 for gg...)
         sdif = sdif/bornme*(bornme + bornmez)
!!         emtx = 1.d0 ! uncomment  for phase space integral
         iii = 0
         if (sdif.gt.sdifmax) then 
            sdifmax = sdif
            nwhenmax = ng
            iii = 1
         endif
         if (istopsearch.eq.0) then            
            if (iverbose.gt.0.and.iii.eq.1) then
               call printstatus(1,k,p1,p2,qph,xsec,var,varbefore,sdif,
     .              sdifmax,fmax)
            endif
         endif
  
         sum  = sum  + sdif
         sum2 = sum2 + sdif**2

         if (ibranch.eq.1) sumb1 = sumb1 + sdif
         if (ibranch.eq.2) sumb2 = sumb2 + sdif

         sump(ng)  = sump(ng)  + sdif
         sum2p(ng) = sum2p(ng) + sdif**2

         varbefore = var
         xsec = sum/k
         var  = sqrt(abs((sum2/k-xsec**2)/k))
         tollerate = 2.d0
         if (var.gt.tollerate*varbefore
     .        .and.varbefore.gt.0.d0.and.ie.eq.0) then
            if (iverbose.gt.0) then
               call printstatus(3,k,p1,p2,qph,xsec,var,varbefore,sdif,
     .              sdifmax,fmax)
            endif
!---------- Nullifying this event ------------!
            ratio = var/varbefore
c            if (k.gt.5000000.and.ratio.gt.3) then
            if (k.gt.500000.and.ratio.gt.10) then
		CALL babayaganlo_error_rejection(ratio)
   
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' >>>>>>>> REJECTING THE EVENT <<<<<<<<'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
 
               sum = sum - sdif
               sum2 = sum2 - (sdif)**2
               sump(ng) = sump(ng) - sdif
               sum2p(ng) = sum2p(ng) - (sdif**2)
               xsec = sum/k
               var  = sqrt((sum2/k-xsec**2)/k)
               sdif = 0.d0
            endif
         endif
      enddo

!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       ==============================
!       PER EVENT (need sdif max)
      ELSE IF(MODE.EQ.1) THEN
!        print *,'MODE == 1'       
       
       k = nsearch
       nsearchloop = 50000
       wnpoints = nsearchloop + nsearch
       hit = 0d0
       
!       FIND MAXIMUM
        do while(k.lt.wnpoints)
**** THE FOLLOWING LINES BETWEEN "*[[ start" AND "* end ]]" MUST
**** BE RECALCULATED FOR EACH EVENT IF ECMS CHANGES EVENT BY EVENT.
**** FURTHERMORE, IF ECMS CHANGES EVENT BY EVENT ifrstenbld MUST 
**** BE SET TO 0
*[[ start
        ifrstenbld = 0
        esoft = eps * ecms/2.d0
        pin(0) = ecms
        pin(2) = 0.d0
        pin(3) = 0.d0
        pin(1) = 0.d0
        betafs = sqrt(1.d0 - 4.d0*am1**2/pin(0)**2)
        beta   = sqrt(1.d0 - 4.d0*ame**2/pin(0)**2)
        pin1(0) = pin(0)/2.d0
        pin1(1) = 0.d0
        pin1(2) = 0.d0
        pin1(3) = beta * pin1(0)
        pin2(0) = pin(0)/2.d0
        pin2(1) = 0.d0
        pin2(2) = 0.d0
        pin2(3) = -beta * pin2(0)
        do ki = 0,3
           pbeam1(ki) = pin1(ki)
           pbeam2(ki) = pin2(ki)
           ptmp(ki)   = pin1(ki)+pin2(ki)
        enddo
        s = dot(ptmp,ptmp)
* end ]]



** [[end initialization]]

         k = k + 1 !running variable
         ntot = ntot + 1 ! all events used for weighted xsec
!          nhnm = nhnm + 1 !all events used in hit and miss (unweighted events)
         
         flux = 8.d0 * (ecms/2.d0)**2
         if (fs.eq.'ee'.or.fs.eq.'mm') call get_cos_fer(cth,wcos)

         call multiplicity(eps,ecms,cth,ng,wnphot)

         if (fs.eq.'gg') then
            if (ng.eq.0) then 
               call get_cos_2g_born(cth,wcos)
            else
               call get_cos_2g(cth,wcos)
            endif
         endif
         sdif = wnphot*wcos
!          print *,sdif

***************************************************
** initial state radiation with structure functions
*
      if (ord.eq.'struct') then
          call emission(pbeam1,pbeam2,wisr,'e')

          betafs = sqrt(1.d0 - 4d0*am1**2/(pbeam1(0)+pbeam2(0))**2)
          beta   = sqrt(1.d0 - 4d0*ame**2/(pbeam1(0)+pbeam2(0))**2)
*
          sreduced =   (pbeam1(0)+pbeam2(0))**2 
     +               - (pbeam1(3)+pbeam2(3))**2

          pbeam1p(0) = sqrt(sreduced)/2.d0
          pbeam1p(1) = 0d0
          pbeam1p(2) = 0d0
          pbeam1p(3) = beta*pbeam1p(0)
*
          pbeam2p(0) = pbeam1p(0)
          pbeam2p(1) = 0d0
          pbeam2p(2) = 0d0
          pbeam2p(3) = -pbeam1p(3)
*
          sdif = sdif * wisr
      else
          do kk=0,3
              pbeam1p(kk) = pbeam1(kk)
              pbeam2p(kk) = pbeam2(kk)
          enddo
      endif
!       print *,sdif

*
** end of initial state radiation
*******************************************

         call phasespace(pbeam1p,pbeam2p,p1,p2,qph,ng,am1,am2,
     .        esoft,cth,w,phsp,ie)
         sdif = sdif * phsp * w

*
** final state radiation with structure function
*
          if (ord.eq.'struct'.and.ie.eq.0) then
              do kk = 0,3
                  p1cut(kk) = p1(kk)
                  p2cut(kk) = p2(kk)
                  pcm(kk)   = pbeam1(kk) + pbeam2(kk)
              enddo
              do ii = 1,ng
                  do kk = 0,3
                      qcut(kk)  = qph(ii,kk)
                  enddo
                  call new_boost(pcm,qcut,qcut,-1)
                  do kk = 0,3
                      qphcut(ii,kk)  = qcut(kk)
                  enddo
               enddo
                  
              call new_boost(pcm,p1cut,p1cut,-1)
              call new_boost(pcm,p2cut,p2cut,-1)
     
              if (fs.eq.'mm') call emission(p1cut,p2cut,wfsr,'m')
              if (fs.eq.'ee') call emission(p1cut,p2cut,wfsr,'e')
     
          else
              do kk = 0,3
                  p1cut(kk) = p1(kk)
                  p2cut(kk) = p2(kk)
              enddo
              do ii = 1,ng
                  do kk = 0,3
                      qphcut(ii,kk)  = qph(ii,kk)
                  enddo
              enddo
          endif
*
** end of final state radiation
*****************************************
!       print *,sdif

         if (ie.ge.1) ie = 1
         if (fs.eq.'gg'.and.ie.eq.0) then
            call mixandsortmomenta(ng,p1,p2,qph)
         endif

         if (ie.lt.1) then 
             if (fs.eq.'ee'.or.fs.eq.'mm') then
                 call cuts(p1cut,p2cut,qphcut,icut)
             elseif (fs.eq.'gg') then
                 call cutsgg(ng,p1cut,p2cut,qphcut,icut)
             endif
         else
            icut = 1
         endif
         ie = ie + icut

         if (ng.gt.nphmax.and.ie.eq.0) nphmax = ng
         if (icut.eq.0) naccepted = naccepted + 1

!!         ie = 1   ! uncomment  for phase space integral

         call squared_matrix(model,ng,ecms,p1,p2,pbeam1,pbeam2,qph,
     >         ie,icalc,emtx,prod)

         bck  = emtx
         emtx = emtx/in_conf_spin ! divided by initial spin conf. 
         emtx = emtx*convfac/flux ! divided by the flux

         if (fs.eq.'gg') emtx = emtx/(ng+2)/(ng+1)

         if (ie.eq.0) then
            call svfactor(model,ng,ecms,p1,p2,eps,sv,deltasv)
            sdif = sdif * sv
         else
            sdif = 0.d0
         endif

         sdif = sdif * emtx

         if (sdif.lt.0.d0) then
             sdif = 0.d0
             w    = 0.d0
             ie   = 1
         endif

         if (ord.eq.'struct'.and.ie.eq.0) then
            do kk = 0,3
                p1(kk) = p1cut(kk)
                p2(kk) = p2cut(kk)
                qph(1,kk) = qcut(kk)
            enddo
            sdif = sdif * wfsr
         endif
!       print *,sdif

! rescaling for Z exchange (bornmez = 0 for gg...)
         sdif = sdif/bornme*(bornme + bornmez)
!!         emtx = 1.d0 ! uncomment  for phase space integral
         iii = 0
         if (sdif.gt.sdifmax) then 
            sdifmax = sdif
            nwhenmax = ng
            iii = 1
         endif
         if (istopsearch.eq.0) then            
            if (iverbose.gt.0.and.iii.eq.1) then
               call printstatus(1,k,p1,p2,qph,xsec,var,varbefore,sdif,
     .              sdifmax,fmax)
            endif
         endif
  
!! unweightening for unweighted events...
!          if (k.gt.nsearch) then
            istopsearch = 1
            if (hitpmiss.lt.1.d0) then
               if(fmax.lt.0d0) fmax = 1.05d0*sdifmax
            endif
            hitpmiss = hitpmiss + 1.d0
!             call ranlux(csi,1)
            csi = babayaganlo_rndm(1)
            
            if (fmax*csi(1).lt.sdif) then
               hit = hit + 1.d0
	       nhnm = nhnm + 1.d0 !all events used in hit and miss (unweighted events)
            endif
            if (sdif.lt.-1.d-20) then 
	       CALL babayaganlo_error_negative(sdif)
	       
               nneg  = nneg  + 1
               sumneg  = sumneg + abs(sdif)
               sum2neg = sum2neg + sdif**2
            endif
            if (sdif.gt.fmax) then 
	       CALL babayaganlo_warning_overweight(sdif, fmax)

	       nover = nover + 1
               sumover  = sumover  + sdif - fmax
               sum2over = sum2over + (sdif - fmax)**2
            endif
!          endif

         sum  = sum  + sdif
         sum2 = sum2 + sdif**2

         if (ibranch.eq.1) sumb1 = sumb1 + sdif
         if (ibranch.eq.2) sumb2 = sumb2 + sdif

         sump(ng)  = sump(ng)  + sdif
         sum2p(ng) = sum2p(ng) + sdif**2

         varbefore = var
         xsec = sum/ntot
         var  = sqrt(abs((sum2/ntot-xsec**2)/ntot))
         tollerate = 2.d0
         if (var.gt.tollerate*varbefore.and.varbefore.gt.0.d0.and.ie.eq.0) then

!---------- Nullifying this event ------------!
            ratio = var/varbefore
c            if (k.gt.5000000.and.ratio.gt.3) then
            if (k.gt.500000.and.ratio.gt.10) then
	       CALL babayaganlo_error_rejection(ratio)
            
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' >>>>>>>> REJECTING THE EVENT <<<<<<<<'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'

               sum = sum - sdif
               sum2 = sum2 - (sdif)**2
               sump(ng) = sump(ng) - sdif
               sum2p(ng) = sum2p(ng) - (sdif**2)
               xsec = sum/k
               var  = sqrt((sum2/k-xsec**2)/k)
               sdif = 0.d0
            endif
         endif
         
! 	if (eventlimiter.eq.'unw'.and.hit.ge.npoints) goto 100
	IF (hit.ge.1) THEN
	  if((k-nsearch).gt.nmaxtrials) nmaxtrials = k-nsearch

	  goto 100
	ENDIF

      enddo
      
 100    BP1(0) = pbeam2(1) !POSITRON
	BP1(1) = pbeam2(2)
	BP1(2) = pbeam2(3)
	BP1(3) = pbeam2(0)
	
	BQ1(0) = pbeam1(1) !ELECTRON
	BQ1(1) = pbeam1(2) 
	BQ1(2) = pbeam1(3)
	BQ1(3) = pbeam1(0)
	
	BP2(0) = p2(1) !OUTGOING ANTIFERMION
	BP2(1) = p2(2)
	BP2(2) = p2(3)
	BP2(3) = p2(0)
	
	BQ2(0) = p1(1) !OUTGOING FERMION
	BQ2(1) = p1(2)
	BQ2(2) = p1(3)
	BQ2(3) = p1(0)
	
	BNPHOT = ng
	
	do i=1,ng
	  BPHOT(i-1,0) = qph(i,1)
	  BPHOT(i-1,1) = qph(i,2)
	  BPHOT(i-1,2) = qph(i,3)
	  BPHOT(i-1,3) = qph(i,0)
        enddo
 
        MONSDIF = sdif
        
	continue 
       
!       TERMINATE (dump cross section?)
      ELSE IF(MODE.EQ.2) THEN
              
       !! weighted cross section
	RESCROSS = xsec
	RESCROSSERR = var
	RESNPHMAX = nphmax
	DO i = 0,nphmax
	  RESCROSSPHOT(i)     = sump(i)/ntot
	  RESCROSSPHOTERR(i)  = sqrt((abs(sum2p(i)/ntot-xsecp(i)**2))/ntot)
	  RESCROSSPHOTFRAC(i) = xsecp(i)/xsec * 100.d0
	ENDDO
	
       !! hit or miss cross section, efficiency and bias
	hmxsect = 0.d0
	hmerr   = 0.d0
	IF (hitpmiss.gt.0.d0)  THEN 
	  hmeff   = nhnm/hitpmiss
	  hmxsect = fmax * hmeff
	  hmerr   = fmax * sqrt(hmeff*(1-hmeff)/hitpmiss)
	ENDIF

	HNMEFF           = hmeff
	HNMCROSS         = hmxsect
	HNMCROSSERR      = hmerr
	HNMFMAX          = fmax
	HNMSDIFMAX       = sdifmax
	HNMMAXTRIAL      = nmaxtrials
	HNMMAXTRIALLIMIT = nsearchloop
	
       !! hit or miss  bias
	biashit        = 0.d0
	biashitpmiss   = 0.d0
	biasneghit     = 0.d0
	biasneghitmiss = 0.d0
	sezover        = 0.d0
	errsezover     = 0.d0
	IF (nhnm.gt.0.d0) THEN 
	  biashit        = 1.d0 * nover/nhnm
	  biashitpmiss   = 1.d0 * nover/hitpmiss
	  biasneghit     = 1.d0 * nneg/nhnm
	  biasneghitmiss = 1.d0 * nneg/hitpmiss
	  sezover        = sumover/hitpmiss
	  errsezover     = (sum2over/hitpmiss - sezover**2)/hitpmiss
	  errsezover     = sqrt(abs(errsezover))
	  sezneg         = sumneg/hitpmiss
	  errsezneg      = (sum2neg/hitpmiss - sezneg**2)/hitpmiss
	  errsezneg      = sqrt(abs(errsezneg))
	ENDIF
	
	BIASBIASHIT        = biashit
	BIASBIASHITPMISS   = biashitpmiss
	BIASBIASNEGHIT     = biasneghit
	BIASBIASNEGHITMISS = biasneghitmiss
	BIASSEZOVER        = sezover
	BIASERRSEZOVER     = errsezover
	BIASSEZNEG         = sezneg
	BIASERRSEZNEG      = errsezneg
        BIASNNEG           = nneg
        BIASNOVER          = nover
      END IF
      
      END
      
      
      
      
      
      
      
      
      
      
      subroutine mixandsortmomenta(ng,pa,pb,qm)
      implicit double precision (a-h,o-z)
      dimension pa(0:3),pb(0:3),qm(40,0:3),q1(0:3),q2(0:3),qmord(40,0:3)
      common/beforesort/p1o(0:3),p2o(0:3),qpho(40,0:3)
***** TEST
***** TEST
***** TEST
***** TEST
c$$$      do k = 0,3
c$$$         p1o(k) = pa(k)
c$$$         p2o(k) = pb(k)
c$$$         do i = 1,ng
c$$$            qpho(i,k) = qm(i,k)
c$$$         enddo
c$$$      enddo
c$$$      return
***** TEST
***** TEST
***** TEST
***** TEST
      if (ng.eq.0) return
      do k = 0,3
         p1o(k) = pa(k)
         p2o(k) = pb(k)
         do i = 1,ng
            qpho(i,k) = qm(i,k)
         enddo
      enddo
      ngl = ng + 2
      do k = 0,3
         qm(ng+1,k) = pa(k)
         qm(ng+2,k) = pb(k)
      enddo
      call orderinverseqm(ngl,qm,qmord)
      do k = 0,3
         pa(k)= qmord(ng+2,k)
         pb(k)= qmord(ng+1,k)
         qm(ng+1,k) = 0.d0
         qm(ng+2,k) = 0.d0
      enddo
! in qmord photons are from the less energetic (k=1) to the most energetic 
! (k=ng). In qm I want to sort from the most to the less energetic...
      do k = 1,ng
         qm(ng+1-k,0) = qmord(k,0)
         qm(ng+1-k,1) = qmord(k,1)
         qm(ng+1-k,2) = qmord(k,2)
         qm(ng+1-k,3) = qmord(k,3)
      enddo
      return
      end
c
      subroutine orderinverseqm(nph,qm,qmord)
      implicit double precision (a-h,o-z)
      parameter (nmax = 40)
      dimension qm(nmax,0:3),qmord(nmax,0:3),en(nph),sen(nph)
      dimension indexes(nph)
      do k = 1,nph
         en(k) = qm(k,0)
      enddo
      call sort_vector_inverse(en,sen,indexes,nph)
      do k = 1,nph
         qmord(k,0) = qm(indexes(k),0)
         qmord(k,1) = qm(indexes(k),1)
         qmord(k,2) = qm(indexes(k),2)
         qmord(k,3) = qm(indexes(k),3)
      enddo
      do k = nph+1,nmax
         qmord(k,0) = 0.d0
         qmord(k,1) = 0.d0
         qmord(k,2) = 0.d0
         qmord(k,3) = 0.d0
      enddo
      return
      end

      subroutine sort_vector(v,sv,iold,n)
! pensare quick sort!!
      implicit double precision (a-h,o-z)
      dimension iold(n)
      dimension v(n),sv(n)
      do k = 1,n
         iold(k) = k
         sv(k) = v(k)
      enddo
      do i = 1,n
         vmax = sv(i)
         do j=i+1,n
            if (v(j).gt.vmax) then
               vmax = sv(j)
               vtmp = sv(i)
               sv(i) = sv(j)
               sv(j) = vtmp

               ktmp = iold(i)
               iold(i) = iold(j)
               iold(j) = ktmp
            endif
         enddo
      enddo
      return
      end

      subroutine sort_vector_inverse(v,sv,iold,n)
! pensare quick sort!!
      implicit double precision (a-h,o-z)
      dimension iold(n)
      dimension v(n),sv(n)
C this uses the quick sort algorithm reported at the end of the file
c      call SORTRX(n,v,iold)
c      do k = 1,n
c         sv(k) = v(iold(k))
c      enddo
c      return
      do k = 1,n
         iold(k) = k
         sv(k) = v(k)
      enddo
      do i = 1,n
         vmin = sv(i)
         do j=i+1,n
            if (v(j).lt.vmin) then
               vmin = sv(j)
               vtmp = sv(i)
               sv(i) = sv(j)
               sv(j) = vtmp
               ktmp = iold(i)
               iold(i) = iold(j)
               iold(j) = ktmp
            endif
         enddo
      enddo
      return
      end