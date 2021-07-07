      !       program main  !TF, 2015
      subroutine main_belle2(MODE,ECM,XPAR,NPAR) !TF, 2015

! written by CMCC, last modified 5/3/2006
      implicit double precision (a-h,o-z)
      integer*8 npoints,k,naccepted,iwriteout
      parameter (mph=40)
      real*4 csi(1)
      dimension sump(0:mph-1),sum2p(0:mph-1)
      dimension fractions(0:mph-1)
      dimension xsecp(0:mph-1),varp(0:mph-1)
      dimension pin(0:3),p1(0:3),p2(0:3),qph(mph,0:3),q(0:3),qcut(0:3)
      dimension qphcut(mph,0:3)
      dimension pin1(0:3),pin2(0:3),ptmp(0:3),pbeam1(0:3),pbeam2(0:3)
      dimension pbeam1p(0:3),pbeam2p(0:3),p1lab(0:3),p2lab(0:3)
      dimension p1b(0:3),p2b(0:3),p12lab(0:3)
      dimension dir(3)
      dimension p1cut(0:3),p2cut(0:3),pcm(0:3)
      integer isvec(25)
      common/rlxstatus/isvec
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
      common/ecms/ecms,ecmsnom
      common/nphot_mode/nphotmode
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/zparameters/zm,gz,ve,ae,rv,ra,wm,s2th,gfermi,sqrt2,um
      common/epssoft/eps
      common/energiabeam/ebeam
      common/parameters/ame,ammu,convfac,alpha,pi
      common/intinput/iwriteout,iseed,nsearch,iverbose
      common/qedORDER/ord
      common/charinput/model,eventlimiter,store,storefile,outfile
      common/realinput/anpoints,sdifmax
      common/nmaxphalpha/nphmaxalpha

      character*100 vpuncout
      common/vpuncfile/vpuncout

      common/ialpharunning/iarun
      common/teubner/iteubn
      common/nskvp/inskvp

      common/idebugging/idebug
      common/regolatori/regulator1,regulator2
*********************************************
      common/ggreg/ipair
      common/coseni/cmax,cmin ! these now are in the cm frame, beware of beam energy spread!
      common/channelref/iref
      common/momentainitial/pin1,pin2
      common/radpattern/nph(4)
      common/forborncrosssection/phsp2b,flux2b,bornme,bornmeq2,bornmez,
     > bornmenovp
      common/reducedtoborn/p1b,p2b
      common/samplelimits/spmin,omaxext
      common/various/beta,betafs
      common/for_debug/ppp(0:3),denphsp,dir3(0:3),dir4(0:3),br,bq,onm
      common/enableifirst/ifrstenbld
      common/tmintmax/tmin,tmax,tcur
      common/weightwcos/wcos ! to be depurated in phasespaceNEW
      
      common/idarkon/idarkon
      common/darkmatter/amassU,gammaU,gvectU,gaxU

      common/rescaledhaderr/scal_err_dhad
      common/beamspreadsigmas/esig1,esig2
      dimension immloc(1)

      dimension iswvperr(3),iswvperrt(6)
      common/iswitchvperror/iswvperr,iswvperrt

      parameter (nvps = 26)
      dimension sumv(nvps),sum2v(nvps)
      dimension xsecv(nvps),varv(nvps),diffv(nvps)
      dimension remtx(nvps)
      common/rationsvpunc/remtx,invp

      parameter (nvpst = 728)
      dimension sumvt(nvpst),sum2vt(nvpst)
      dimension xsecvt(nvpst),varvt(nvpst),diffvt(nvpst)
      dimension remtxt(nvpst)
      integer ist(nvpst,6)
      common/rationsvpunct/remtxt,invpt,ist
      
      
      
      !  parameters for Belle II, TF 2015
      !  ----------------------------------------------------------
      DIMENSION XPAR(0:99), NPAR(0:99)
      COMMON / BRESULTS / RESCROSS, RESCROSSERR, RESCROSSPHOT(0:39), RESCROSSPHOTERR(0:39), RESCROSSPHOTFRAC(0:39), RESNPHMAX
      COMMON / BHITNMISS / HNMCROSS, HNMCROSSERR, HNMEFF, HNMSDIFMAX, HNMFMAX, HNMMAXTRIALLIMIT, HNMMAXTRIAL
      COMMON / BBIAS / BIASBIASHIT, BIASBIASHITPMISS, BIASBIASNEGHIT, BIASBIASNEGHITMISS, BIASSEZOVER, BIASERRSEZOVER, BIASSEZNEG, BIASERRSEZNEG, BIASNOVER, BIASNNEG
      integer BNPHOT
      double precision BWEIGHT
      COMMON / MOMSET / BWEIGHT, BP1(0:3), BQ1(0:3), BP2(0:3), BQ2(0:3), BPHOT(0:99,0:3), BNPHOT
      double precision MONSDIF
      COMMON / BMONITORING / MONSDIF      
      double precision RANDOMCMSENERGY
      COMMON / BEAMPARAMETERS / RANDOMCMSENERGY      
      double precision babayaganlo_rndm
      external babayaganlo_rndm      
      character*10  weightmode
      double precision ECM
      
      
      double precision BEEMIN, BTEMIN, BEGMIN, BTGMIN, BEEVETO, BTEVETO, BEGVETO, BTGVETO, BPRESCALE
      integer BUSERCUTS, IBUSERMODE
      character*10  BUSERMODE
      COMMON / USERCUTS / BEEMIN, BTEMIN, BEGMIN, BTGMIN, BEEVETO, BTEVETO, BEGVETO, BTGVETO
      COMMON / PRESCALE / BPRESCALE
      COMMON / USERCUTSINT / BUSERCUTS
      COMMON / USERCUTSMODE / BUSERMODE

      !  ----------------------------------------------------------
      character*6   arun,darkmod
      character*10  menu2,menud
      character*80  path
      integer*8 nloop
      integer*4 unweightedhit
      integer*4 weightedhit
      double precision bweightsum, bnevents, lumeq, lumeqerr
      COMMON / USERTEST / bweightsum,bnevents
      
      irepeat = 0  
      
      IF (MODE.EQ.-1) THEN
!         write(*,*) "MODE.EQ.-1, INITIALIZE"

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
	
	fs = 'ee'
	ifs = NPAR(20)
	IF (ifs.EQ.1) THEN 
	  fs = 'ee'
	ELSEIF (ifs.EQ.2) THEN 
	  fs = 'gg'
	ELSEIF (ifs.EQ.3) THEN 
	  fs = 'mm'
	END IF
	
	ord = 'exp'
	iord = NPAR(22)
	IF(iord.EQ.1) THEN 
	  ord = 'born'
	ELSEIF (iord.EQ.2) THEN 
	  ord = 'alpha'
	ELSEIF (iord.EQ.3) THEN
	  ord = 'exp'
	END IF

	model = 'matched'
	imodel = NPAR(21)
	IF (imodel.EQ.1) THEN 
	  model = 'matched'
	ELSEIF (imodel.EQ.2) THEN
	  model = 'ps'
	END IF

	weightmode = 'unweighted'
	iweightmode = NPAR(24)
	IF (iweightmode.EQ.1) THEN 
	  weightmode = 'unweighted'
	ELSEIF (iweightmode.EQ.2) THEN
	  weightmode = 'weighted'
	END IF	
	
	BUSERMODE = 'NONE'
	IBUSERMODE = NPAR(25)
	IF (IBUSERMODE.EQ.1) THEN 
	  BUSERMODE = 'NONE'
	ELSEIF (IBUSERMODE.EQ.2) THEN
	  BUSERMODE = 'GAMMA'
	ELSEIF (IBUSERMODE.EQ.3) THEN
	  BUSERMODE = 'EGAMMA'
	ELSEIF (IBUSERMODE.EQ.4) THEN
	  BUSERMODE = 'ETRON'
	ELSEIF (IBUSERMODE.EQ.5) THEN
	  BUSERMODE = 'PRESCALE'
	END IF

	nphotmode = NPAR(0)
	thmin = XPAR(20)
	thmax = XPAR(21)
	thmin = thmin*degtorad
	thmax = thmax*degtorad
	thgmin = thmin 
	thgmax = thmax
        emin   = XPAR(9)
        zmax   = XPAR(10)*degtorad
        eps    = XPAR(11)
 	egmin  = 0.02d0
 	
 	fmax     = XPAR(30)
 	
        nsearch = NPAR(1)
!     RUNNING ALPHA
	arun = 'hlmnt'
	ivacpol = NPAR(23)
	IF (ivacpol.EQ.1) THEN
	  arun = 'off'
	ELSEIF (ivacpol.EQ.2) THEN
	  arun = 'hadr5'
	ELSEIF (ivacpol.EQ.3) THEN
	  arun = 'hlmnt'
	END IF
	
! 	convfac = 0.389379660D6
! 	pi      = 3.1415926535897932384626433832795029D0
! 
	! CONSTANTS
! 	ame  =   0.51099906d-3
! 	ammu = 105.65836900d-3 
! 	alpha = 1.D0/137.0359895D0
! 	zm  = 91.1867D0
! 	wm  = 80.35d0 
! 	gz  = 2.4952D0
	s2th = 1.d0 - wm**2/zm**2
	gfermi = 1.16639d-5
	sqrt2  = sqrt(2.d0)
	gfermi = pi*alpha/2.d0/s2th/wm**2*sqrt2
	ve = -1.d0 + 4.d0*s2th
	ae = -1.d0
	rv = ve**2/(ae**2+ve**2)
	ra = ae**2/(ae**2+ve**2)
	um = sqrt(pi*alpha/sqrt2/gfermi)

	! USER PARAMETER
! 	fs   = 'ee' ! 'ee' or 'gg' or 'mm'
	path = 'test-run/' !TF 2015
! 	ecms = 1.02d0
! 	ord  = 'exp'
! 	nphotmode = -1
! 	model  = 'matched'
! 	thmin  =  15.d0
! 	thmax  = 165.d0
! 	zmax   =  10.d0
! 	emin   =   0.1d0
	anpoints = 10000.d0
	  
  ! OLD DEFAULT      
! 	eps    = 5.d-4
  ! NEW DEFAULT (it has to be much smaller than \Gamma/M for the M resonance...)
! 	eps    = 5.d-7

! 	thgmin = thmin 
! 	thgmax = thmax

	iseed        = 700253512
	iwriteout    = 1000000
! 	nsearch      = 5000
	iverbose     = 0
	eventlimiter = 'w'  ! 'unw' or 'w'
	store        = 'no'
	sdifmax      = 1.d-18
! 	arun         = 'hlmnt'
	darkmod      = 'off'
	amassU       = 0.4d0
	gammaU       = -1. 
	gvectU       = 1.d-3
	gaxU         = 0.d0
	menu2        = 'off'
	menud        = 'off'
	amassainvmin = 0.d0
	amassainvmax = ecms
	
	bweightsum   = 0.d0
	bnevents     = 0.d0
	
	! VP
	iarun = 0
	iteubn = 0
	inskvp = 0
	if (arun.eq.'hadr5') then
	    iarun = 1
	    iteubn = 0
	elseif (arun.eq.'hlmnt') then
	    iarun = 1
	    iteubn = 1
	elseif (arun.eq.'on'.or.arun.eq.'nsk') then
	    iarun  = 1
	    inskvp = 1
	else
	    iarun = iarun
	endif

	! DARK MODEL
	idarkon = 0
	if (darkmod.eq.'on') then
	    idarkon = 1
	    if(model.eq.'matched') model = 'ps'
	endif
	
	! VP UNCERTAINTY
	if (NPAR(5).GT.0) then
	  scal_err_dhad = 1.d0
	else 
	  scal_err_dhad = 0.d0
	endif
	
	!BEAM ENERGY SPREAD
	esig1  = XPAR(40)
	esig2  = XPAR(40)
	
	! USER CUTS
	BEEMIN  = XPAR(50)
	BTEMIN  = XPAR(51)
	BTEMIN  = BTEMIN*degtorad
	BEGMIN  = XPAR(52)
	BTGMIN  = XPAR(53)
	BTGMIN  = BTGMIN*degtorad
	BEEVETO = XPAR(54)
	BTEVETO = XPAR(55)
	BTEVETO = BTEVETO*degtorad
	BEGVETO = XPAR(56)
	BTGVETO = XPAR(57)
	BTGVETO = BTGVETO*degtorad
	MAXPRESCALE = XPAR(58)
	
	! CHECK USER CUTS
	isok = 1
	BUSERCUTS = 0
	IF (BUSERMODE.eq.'GAMMA') THEN
	  IF ((BEGMIN.lt.0.).or.(BTGMIN.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  IF ((BEEVETO.lt.0.).or.(BTEVETO.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  IF ((BEGVETO.lt.0.).or.(BTGVETO.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  IF ((BTEVETO.le.thmin).or.((pi-BTEVETO).ge.thmax)) THEN
	    isok = 0
	  ENDIF
	  
	ELSEIF (BUSERMODE.eq.'EGAMMA') THEN
	  IF ((BEEMIN.lt.0.).or.(BTEMIN.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  IF ((BEGMIN.lt.0.).or.(BTGMIN.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  IF ((BEEVETO.lt.0.).or.(BTEVETO.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  IF ((BEGVETO.lt.0.).or.(BTGVETO.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  
	ELSEIF (BUSERMODE.eq.'ETRON') THEN
	  IF ((BEEMIN.lt.0.).or.(BTEMIN.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  IF ((BEEVETO.lt.0.).or.(BTEVETO.lt.0.)) THEN
	    isok = 0
	  ENDIF
	  IF ((BEGVETO.lt.0.).or.(BTGVETO.lt.0.)) THEN
	    isok = 0
	  ENDIF
	
	ELSEIF (BUSERMODE.eq.'PRESCALE') THEN
	  IF ((MAXPRESCALE.lt.1.d-20)) THEN
	    isok = 0
	  ENDIF
	ENDIF
	
	IF ((BUSERMODE.ne.'NONE').and.(fs.ne.'ee')) THEN
	  CALL babayaganlo_fatal_usercutsfs()
	ENDIF
	
	IF ((isok.eq.0)) THEN
	  IF (BUSERMODE.ne.'PRESCALE') THEN
	    print*,'      EEMIN:  ', BEEMIN
	    print*,'      TEMIN:  ', BTEMIN
	    print*,'      EGMIN:  ', BEGMIN
	    print*,'      TGMIN:  ', BTGMIN
	    print*,'      EEVETO: ', BEEVETO
	    print*,'      TEVETO: ', BTEVETO
	    print*,'      EGVETO: ', BEGVETO
	    print*,'      TGVETO: ', BTGVETO
	    CALL babayaganlo_fatal_usercuts()
	  ELSE
	    print*,' MAXPRESCALE: ', MAXPRESCALE
	    CALL babayaganlo_fatal_usercutsprescale()
	  ENDIF
	ELSE
	  BUSERCUTS = 1
	ENDIF	
	
! 	BUSERCUTS = 0
	IF((BEEMIN.GT.0d0).OR.(BTEMIN.GT.0d0)) THEN
	  BUSERCUTS = 1
	ENDIF
	IF((BEGMIN.GT.0d0).OR.(BTGMIN.GT.0d0)) THEN
	  BUSERCUTS = 1
	ENDIF
	IF((BEEVETO.GT.0d0).OR.(BTEVETO.GT.0d0)) THEN
	  BUSERCUTS = 1
	ENDIF
	IF((BEGVETO.GT.0d0).OR.(BTGVETO.GT.0d0)) THEN
	  BUSERCUTS = 1
	ENDIF	
	
! 	call system('mkdir -p '//path)
! 	outfile  = path(:lpath)//'statistics.txt'
! 	vpuncout = path(:lpath)//'vpuncertainties.txt'

	ebeam = ecms/2.d0
! 	thmin = thmin * pi/180.d0
! 	thmax = thmax * pi/180.d0
! 	thgmin = thgmin * pi/180.d0
! 	thgmax = thgmax * pi/180.d0
! 	zmax  = zmax * pi/180.d0

	if ((weightmode.EQ.'weighted').and.(BUSERMODE.eq.'PRESCALE')) then
	  call babayaganlo_fatal_weightedprescale()
	endif
	
	if (fs.eq.'gg'.or.fs.eq.'ee'.or.fs.eq.'mm') then
	    continue
	else
	    print*,'  '
	    print*,'Wrong final-state selected (',fs,')'
	    print*,'Select one of ''ee'',''gg'' or ''mm'''
	    print*,'  '
	    stop
	endif
	
	nphmaxalpha = 6
	
	print*,'BABAYAGA.NLO configuration:'
	print*,'-------------------------------------------'
	print*,'   events for maximum search: ' ,nsearch
	print*,'   fmax:                      ' ,fmax
	print*,'   final state:  ',fs
	print*,'   ord:   ',ord
	print*,'   eps:   ',eps
	print*,'   soft cutoff (eps * ebeam):   ',eps * ecms/2.d0
	print*,'   model: ',model
	print*,'   cms energy (nominal): ', ecms
	print*,'   beam energy (nominal): ', ecms/2.d0
	print*,'   THMIN: ',thmin, thmin/degtorad
	print*,'   THMAX: ',thmax, thmax/degtorad
	print*,'   ACOL: ',zmax, zmax/degtorad
	print*,'   EMIN: ',emin
	print*,'   Mass (Z): ',zm
	print*,'   Mass (W): ',wm
	print*,'   Mass (e): ',ame
	print*,'   Mass (mu): ',ammu
	print*,'   Width (Z): ',gz
	print*,'   s2th (on shell):   ',s2th
	print*,'   GFermi (on shell): ',gfermi
	print*,'   hbarc to nb conversion: ',convfac
	print*,'   alpha(0):          ',alpha , 1./alpha
	print*,'   nphotmode:         ',nphotmode  
	print*,'   running alpha:           ',iarun  
	print*,'   running alpha (Teubner): ',iteubn  
	print*,'   weightmode: ',weightmode  
	print*,'   usermode: ',BUSERMODE

        IF ((BUSERMODE.ne.'NONE').and.(BUSERMODE.ne.'PRESCALE')) THEN
	  if((BUSERMODE.eq.'ETRON').or.(BUSERMODE.eq.'EGAMMA'))print*,'      EEMIN:  ', BEEMIN
	  if((BUSERMODE.eq.'ETRON').or.(BUSERMODE.eq.'EGAMMA'))print*,'      TEMIN:  ', BTEMIN
	  if((BUSERMODE.eq.'GAMMA').or.(BUSERMODE.eq.'EGAMMA')) print*,'      EGMIN:  ', BEGMIN
	  if((BUSERMODE.eq.'GAMMA').or.(BUSERMODE.eq.'EGAMMA')) print*,'      TGMIN:  ', BTGMIN
	  print*,'      EEVETO: ', BEEVETO
	  print*,'      TEVETO: ', BTEVETO
	  print*,'      EGVETO: ', BEGVETO
	  print*,'      TGVETO: ', BTGVETO
	ELSEIF (BUSERMODE.eq.'PRESCALE') THEN
	  print*,' MAXPRESCALE: ', MAXPRESCALE
	ENDIF
	print*,'-------------------------------------------'

!       call userinterface !TF, 2015

** [[begin initialization]]
	if (idarkon.eq.1) then
	  if (gammaU.lt.0.) then
	    gammaU=gammafunc(amassU,gvectU)
	  endif
	  if (amassU.eq.0.) then
	    amassU=1.d-13
	  endif
	  gvectU=gvectU*sqrt(4.*pi*alpha)
	  gaxU=gaxU*sqrt(4.*pi*alpha)
	  call init_apar
	endif

	idebug = 0

	ecmsnom = ecms

	call getcmaxmin(cmax,cmin)
! 	      print *,cmax, cmin

! 	E1 = ecmsnom * 0.5d0
! 	b1 = sqrt(1.d0 - ame**2/E1/E1)
! 	bmin = sqrt(1.d0 - ame**2/emin/emin)
! 	tmin = 2.d0*E1**2*(1.d0-b1**2*cmin)
! 	tmin = 2.d0*ame*ame - tmin
! 	tmax = 2.d0*E1*emin*(1.d0-b1*bmin*cmax)
! 	tmax = 2.d0*ame*ame - tmax

	npoints = anpoints

* for ALPHA, not released
*      call init_apar
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
	sumv  = 0.d0
	sum2v = 0.d0
	remtx = 1.d0
	diffv = 0.d0
	invp  = 0
	xsecv = 0.d0
	sumvt  = 0.d0
	sum2vt = 0.d0
	remtxt = 1.d0
	diffvt = 0.d0
	invpt  = 0
	xsecvt = 0.d0

	iswvperr(1) = 0
	iswvperr(2) = 0
	iswvperr(3) = 0
	iswvperrt(1) = 0
	iswvperrt(2) = 0
	iswvperrt(3) = 0
	iswvperrt(4) = 0
	iswvperrt(5) = 0
	iswvperrt(6) = 0
	
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
	ieb          = 0
	ie           = 0

	if (store.eq.'yes') then
	  call initstorage(storefile)
	endif
	k = 0
	wnpoints = npoints
	do j = 0,3
	  do ki = 1,mph
	      qph(ki,j) = 0.d0
	  enddo
	enddo
	ng = 0
	  
c     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
c     END OF INITIALIZATION (MODE.EQ.-1)	  
c     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      ELSEIF (MODE.EQ.1) THEN
c     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
c     END OF RUN (see below) (MODE.EQ.1)	  
c     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~      
      ELSEIF (MODE.EQ.2) THEN
!         write(*,*) "MODE.EQ.2, FINALIZE"
        CALL babayaganlo_result_nominalcmsenergy(ecmsnom)

! weighted cross section
	xsec = sum/k
	var  = sqrt((abs(sum2/k-xsec**2))/k)        
!         print *,'total (weighted):   ',sum, bweightsum
!         print *,'nevents:   ',k
        
        if (weightmode.eq.'weighted') CALL babayaganlo_result_weightsum(sum)
        CALL babayaganlo_result_weightedxsec(xsec,var)
        
	do i = 0,nphmax
	  xsecp(i) = sump(i)/k
	  varp(i)  = sqrt((abs(sum2p(i)/k-xsecp(i)**2))/k)
	  fractions(i) = xsecp(i)/xsec * 100.d0
	  iii = i
	  if (fs.eq.'gg') iii = i + 2
!           print *,iii, xsecp(i),' +-',varp(i),' (',fractions(i),' %)'
	enddo
        
! hit or miss cross section
	hmxsect = 0.d0
	hmerr   = 0.d0
	if (hitpmiss.gt.0.d0)  then 
	  hmeff   = hit/hitpmiss
	  hmxsect = fmax*hmeff
	  hmerr   = fmax * sqrt(hmeff*(1-hmeff)/hitpmiss)
	endif
        
! bias  
	biashit        = 0.d0
	biashitpmiss   = 0.d0
	biasneghit     = 0.d0
	biasneghitmiss = 0.d0
	sezover        = 0.d0
	errsezover     = 0.d0
	if (hit.gt.0.d0) then 
	  biashit      = 1.d0*nover/hit
	  biashitpmiss = 1.d0*nover/hitpmiss
	  biasneghit   = 1.d0*nneg/hit
	  biasneghitmiss = 1.d0*nneg/hitpmiss
	  sezover    = sumover/hitpmiss
	  errsezover = (sum2over/hitpmiss - sezover**2)/hitpmiss
	  errsezover = sqrt(abs(errsezover))
	  sezneg    = sumneg/hitpmiss
	  errsezneg = (sum2neg/hitpmiss - sezneg**2)/hitpmiss
	  errsezneg = sqrt(abs(errsezneg))
	endif        
        
!         print *,'hit or miss efficiency ',hmeff*100,' %'
!         print *,'total (unweighted):   ',hmxsect,' +-',hmerr,' nb'
        CALL babayaganlo_result_hitormisseff(hmeff)
        CALL babayaganlo_result_unweightedxsec(hmxsect, hmerr)
        
! 	print *,'-------------'
!         print *,'bias over fmax:   ',sezover,' +-',errsezover,' nb'
!         print *,'bias negative:    ',-sezneg,' +-',errsezneg,' nb'
        CALL babayaganlo_result_unweightedxsec_overweight(sezover,errsezover)
        CALL babayaganlo_result_unweightedxsec_underweight(sezneg,errsezneg)
!         print *,'total + biases:    ',hmxsect+sezover-sezneg,' +-',hmerr+errsezover+errsezneg,' nb'
        hmxsectbias = hmxsect+sezover-sezneg
        hmxsectbiaserr = hmerr+errsezover+errsezneg
        CALL babayaganlo_result_unweightedxsec_biascorrected(hmxsectbias, hmxsectbiaserr)
        
        CALL babayaganlo_result_nover(nover)
        CALL babayaganlo_result_biashit(biashit)
        CALL babayaganlo_result_biashitpmiss(biashitpmiss)
        CALL babayaganlo_result_nneg(nneg)
        CALL babayaganlo_result_biasneghit(biasneghit)
        CALL babayaganlo_result_biasneghitmiss(biasneghitmiss)        
        
        CALL babayaganlo_result_maxweight(sdifmax, fmax)
        
	if (abs(scal_err_dhad).gt.1.d-5) then
	    IF (iteubn.eq.0) THEN
	    ii = 0
	    do i1 = -1,1
	      do i2 = -1,1
		  do i3 = -1,1
		    if (i1.ne.0.or.i2.ne.0.or.i3.ne.0) then
			ii = ii + 1
			diffv(ii) = xsecv(ii)-xsec
			perc      = (xsecv(ii)-xsec)/xsec*100
			percerr   = varv(ii)/xsecv(ii)*100
! 			write(15,*)ii,'>',i1,i2,i3,xsecv(ii),'+-',
!   :                          varv(ii),' (',perc,'%)'
		    endif
		  enddo
	      enddo
	    enddo
	    immloc = minloc(xsecv,MASK=xsecv.gt.0.d0)
	    immloc = maxloc(xsecv,MASK=xsecv.gt.0.d0)
	    ELSE ! for Teubner et al. parameterization
	      ii = 0
	      do ii = 1,nvpst
		diffvt(ii) = xsecvt(ii)-xsec
		perc      = (xsecvt(ii)-xsec)/xsec*100
		percerr   = varvt(ii)/xsecvt(ii)*100
	      enddo
	      immloc = minloc(xsecvt,MASK=xsecvt.gt.0.d0)
	      vpval = xsecvt(immloc(1))
	      vpvalerr = varvt(immloc(1))
	      vpvalfrac= (xsecvt(immloc(1))-xsec)/xsec*100
	      
	      CALL babayaganlo_result_vpmin(vpval, vpvalerr, vpvalfrac)
	      CALL babayaganlo_result_vpcentral(xsec, var)
	      
	      immloc = maxloc(xsecvt,MASK=xsecvt.gt.0.d0)
	      vpval = xsecvt(immloc(1))
	      vpvalerr = varvt(immloc(1))
	      vpvalfrac= (xsecvt(immloc(1))-xsec)/xsec*100
	      
	      CALL babayaganlo_result_vpmax(vpval, vpvalerr, vpvalfrac)

	    ENDIF
	endif
	
	! luminosity equivalent in fb-1 (1e-15 barn)
	if (weightmode.eq.'weighted') then
	  if (xsec.gt.0.d0) then
	    lumeq = (sum/(xsec*1.d9))*1.d-15
	    lumeqerr = var/xsec*lumeq
	  else 
	    lumeq = -1.
	    lumeqerr = -1.
	  endif
	elseif (weightmode.eq.'unweighted') then
	  if (xsec.gt.0.d0) then
	    lumeq = (hit/(xsec*1.d-9))*1.d-15
	    lumeqerr = var/xsec*lumeq
	  else 
	    lumeq = -1.
	    lumeqerr = -1.
	  endif
	else
	  lumeq = -1.
	  lumeqerr = -1.
	endif
        call babayaganlo_result_intlum(lumeq, lumeqerr)
	
c     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
c     END OF FINALIZE  (MODE.EQ.2)	  
c     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~      
      ENDIF
      
      

      nloop = 0
      n = 0
      IF (MODE.EQ.-1) THEN
	normfirst = 0
        nloop = nsearch !user given number
! 	print *,weightmode,normxsec
        IF ((weightmode.EQ.'weighted')) THEN
	  nloop=0; !no maximum search for weighted event generation
	ENDIF
      ELSEIF (MODE.EQ.1) THEN
        nloop = 1d12 !huge number, we will exit if we have one event
        unweightedhit = 0
        weightedhit = 0
!         IF (normfirst.eq.0) THEN
! 	  IF ((weightmode.EQ.'weighted').and.(normxsec.lt.1.d-20)) THEN
! 	    normxsec = sum/k
! 	    normfirst = 1
! 	  ELSE
! 	    normfirst = 1
! 	  ENDIF
! 	ENDIF
      ENDIF     

!       do while(k.lt.wnpoints)
      do while(n.lt.nloop)

****  THE FOLLOWING LINES BETWEEN "*[[ start" AND "* end ]]" MUST
**** BE RECALCULATED FOR EACH EVENT IF ECMS CHANGES EVENT BY EVENT.
**** FURTHERMORE, IF ECMS CHANGES EVENT BY EVENT ifrstenbld MUST 
**** BE SET TO 0
*[[ start
        ifrstenbld = 0

****** Beam energy spread
!         call spreadbeams(esig1,esig2,ame,am1,p1lab,p2lab,ieb)
        call spreadbeams_belle2(ECM,ame,am1,p1lab,p2lab,ieb,MODE)
        p12lab = p1lab + p2lab
        call new_boost(p12lab,p1lab,pin1,1)
        call new_boost(p12lab,p2lab,pin2,1)
        
!         print *, p1lab(0), p1lab(1), p1lab(2), p1lab(3)
!         print *, p2lab(0), p2lab(1), p2lab(2), p2lab(3)
        
**** to avoid round offs, I recalculate off-shell relations
        et = pin1(0)+pin2(0)
        ebt = 0.5d0*et
        pin1(0) = ebt
        pin1(3) = ebt*sqrt((1.d0+ame/ebt)*(1.d0-ame/ebt))
        pin2(0) =  pin1(0)
        pin2(3) = -pin1(3)
        ecms=sqrt(dot(pin1+pin2,pin1+pin2))
******
        esoft  = eps * ecms/2.d0 ! ecms is in a common, it is changed in spreadbeams
        pin(0) = ecms
        pin(2) = 0.d0
        pin(3) = 0.d0
        pin(1) = 0.d0
        betafs = sqrt(1.d0 - 4.d0*am1*am1/pin(0)**2)
        beta   = sqrt(1.d0 - 4.d0*ame*ame/pin(0)**2)
        do ki = 0,3
           pbeam1(ki) = pin1(ki)
           pbeam2(ki) = pin2(ki)
        enddo
        s = ecms*ecms
* end ]]

** [[end initialization]]

         n = n + 1
         k = k + 1

         flux = 8.d0 * (ecms/2.d0)**2
         if (fs.eq.'ee'.or.fs.eq.'mm') then
c            call get_cos_ferNEW(cth,wcos)
            call get_cos_fer(cth,wcos)
         endif
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
     +           - (pbeam1(3)+pbeam2(3))**2
            
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
            pbeam1p = pbeam1
            pbeam2p = pbeam2
         endif

*
** end of initial state radiation
*******************************************
      
         if (fs.eq.'ee'.or.fs.eq.'mm') then
ccc            call phasespaceNEW(pbeam1p,pbeam2p,p1,p2,qph,ng,am1,am2,
ccc     .           esoft,cth,w,phsp,ie)
            call phasespaceTMP(pbeam1p,pbeam2p,p1,p2,qph,ng,am1,am2,
     .           esoft,cth,w,phsp,ie)
c            call phasespace(pbeam1p,pbeam2p,p1,p2,qph,ng,am1,am2,
c     .        esoft,cth,w,phsp,ie)
         else
            call phasespace(pbeam1p,pbeam2p,p1,p2,qph,ng,am1,am2,
     .        esoft,cth,w,phsp,ie)
         endif

***
         ie = ie + ieb          ! ieb if beam energy spread fails
         if (ieb.gt.0) phsp = 0.d0
***         
         sdif = sdif * phsp * w
         
         if (isnan(phsp).or.isnan(w)) then
!             print*,'NAN found (pos 1)!!!'
!             print*,isvec
!             print*,'end ----!!!'
            CALL babayaganlo_error_isnan1(phsp, w)
         endif
*
** final state radiation with structure function
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
         endif
** end of final state radiation
*****************************************
         if (ie.ge.1) ie = 1
         if (fs.eq.'gg'.and.ie.eq.0) then
            call mixandsortmomenta(ng,p1,p2,qph)
         endif

         if (ord.ne.'struct'.and.ie.eq.0) then
            call new_boost(p12lab,p1,p1cut,-1)
            call new_boost(p12lab,p2,p2cut,-1)

            qphcut = 0.d0
            if (ng.gt.0) then
               do ii = 1,ng
                  ptmp = qph(ii,:)
                  call new_boost(p12lab,ptmp,ptmp,-1)
                  qphcut(ii,:) = ptmp
               enddo
            endif
         endif
         
         if (ie.lt.1) then 
             if (fs.eq.'ee'.or.fs.eq.'mm') then
!                  call cuts(p1cut,p2cut,qphcut,icut)
                 call cuts(p1cut,p2cut,qphcut,ng,icut)
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

         irepeat = irepeat + 1

         iswvperr  = 0
         iswvperrt = 0
         call squared_matrix(model,ng,ecms,p1,p2,pbeam1,pbeam2,qph,
     >         ie,icalc,emtx,prod,deltah)
         irepeat = 0

         bck  = emtx
         emtx = emtx/in_conf_spin ! divided by initial spin conf. 
         emtx = emtx*convfac/flux ! divided by the flux

         if (fs.eq.'gg') emtx = emtx/(ng+2)/(ng+1)

         if (ie.eq.0) then
            call svfactor(model,ng,ecms,p1,p2,eps,sv,deltasv)
            sdif = sdif * sv
ccc additive form            sdif = sdif / prod/(1.d0+deltasv) * (1.d0+deltasv+deltah)
         else
            sdif = 0.d0
         endif

         sdif = sdif * emtx

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
         
!          moved before any cross section calculation (TF)
         if (isnan(sdif)) then
           CALL babayaganlo_error_isnan2(sdif)
           CYCLE
         endif 
         
!!         emtx = 1.d0 ! uncomment  for phase space integral
         iii = 0
         if (sdif.gt.sdifmax) then 
            sdifmax = sdif
            nwhenmax = ng
            iii = 1
         endif
!          if (istopsearch.eq.0) then            
!             if (iverbose.gt.0.and.iii.eq.1) then
!                call printstatus(1,k,p1,p2,qph,xsec,var,varbefore,sdif,
!      .              sdifmax,fmax)
!             endif
!          endif

! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!! UNWEIGHTED EVENT GENERATION
!          if (k.gt.nsearch) then
         if ((MODE.eq.1).and.(weightmode.eq.'unweighted')) then
!             istopsearch = 1
            if (hitpmiss.lt.1.d0) then
               if(fmax.lt.0d0) fmax = 1.05d0*sdifmax !TF
!                print*,'Starting now also unweighted generation!'
               CALL babayaganlo_finishedmaxsearch(fmax);
            endif
            hitpmiss = hitpmiss + 1.d0
!             call ranlux(csi,1)
            csi(1) = babayaganlo_rndm(0)
            
!             print *,fmax, csi(1), sdif !TF
            if (fmax*csi(1).lt.sdif) then
               hit = hit + 1.d0
c               if (store.eq.'yes') call eventstorage(p1,p2,qph)
!                if (store.eq.'yes') call eventstorage(p1cut,p2cut,qphcut)
               unweightedhit = 1 !TF, 2015
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
         endif ! END UNWEIGHTED EVENT GENERATION 
       
!        MOVED (TF)
         if (isnan(sdif)) then
              CALL babayaganlo_error_isnan2(sdif)
            print*,'--> NAN2 - this should never happen again'

!             print*,'NAN found (pos 2)!!!'
!             print*,sdif,phsp,w
!             if (iverbose.gt.0) then
!                call printstatus(5,k,p1,p2,qph,xsec,var,varbefore,
!      .              sdif,sdifmax,fmax)
!             endif
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
c         tollerate = 1.2d0
         tollerate = 1.8d0
         if (var.gt.tollerate*varbefore
     .        .and.varbefore.gt.0.d0.and.ie.eq.0) then
            if (iverbose.gt.0) then
               call printstatus(3,k,p1,p2,qph,xsec,var,varbefore,sdif,
     .              sdifmax,fmax)
            endif
!---------- Nullifying this event ------------!
            ratio = var/varbefore
c            if (k.gt.5000000.and.ratio.gt.3) then
            if (k.gt.5000000.and.ratio.gt.6) then
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' >>>>>>>> REJECTING THE EVENT <<<<<<<<'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                print*,' ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^'
!                if (iverbose.gt.0) then
!                   call printstatus(4,k,p1,p2,qph,xsec,var,varbefore,
!      .                 sdif,sdifmax,fmax)
!                endif
c               print*,'WELL... actually not rejecting it...'
c               goto 12345
	       CALL babayaganlo_error_rejection(ratio)
               sum = sum - sdif
               sum2 = sum2 - (sdif)**2
               sump(ng) = sump(ng) - sdif
               sum2p(ng) = sum2p(ng) - (sdif**2)
               xsec = sum/k
               var  = sqrt((sum2/k-xsec**2)/k)
               sdif = 0.d0
12345          continue
            endif
         endif

*[[[  START VP PARAMETRIC UNCERTAINTY
         if (iteubn.eq.0) then
            if (invp.gt.0) then
               do ii = 1,invp
                  sumv(ii)  = sumv(ii)  +  sdif*remtx(ii)
                  sum2v(ii) = sum2v(ii) + (sdif*remtx(ii))**2
                  xsecv(ii) = sumv(ii)/k
                  varv(ii)  = sqrt(abs((sum2v(ii)/k-xsecv(ii)**2)/k))
               enddo
            endif  
         else
            if (invpt.gt.0) then
               do ii = 1,nvpst
                  sumvt(ii)  = sumvt(ii)  +  sdif*remtxt(ii)
                  sum2vt(ii) = sum2vt(ii) + (sdif*remtxt(ii))**2
                  xsecvt(ii) = sumvt(ii)/k
                  varvt(ii)  = sqrt(abs((sum2vt(ii)/k-xsecvt(ii)**2)/k))
               enddo
            endif  
         endif
*]]]  END VP PARAMETRIC UNCERTAINTY

         if(unweightedhit.eq.1) goto 100
         if((weightmode.eq.'weighted').and.(sdif.gt.1.d-20)) goto 100

         if (eventlimiter.eq.'unw') wnpoints = 1.d13
         if (eventlimiter.eq.'unw'.and.hit.ge.npoints) goto 100
      enddo
      
      return
      
100   BP1(0) = pbeam2(1) !POSITRON
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
      
      BWEIGHT = 1.0
      if(weightmode.eq.'weighted') then
	BWEIGHT = sdif
      endif
      
      if(BUSERMODE.ne.'PRESCALE') then
	BPRESCALE = 1.0
      endif
 
      continue

      end
