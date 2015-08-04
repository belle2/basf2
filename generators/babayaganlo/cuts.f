!       subroutine cuts(p1,p2,qph,icut)
      subroutine cuts(p1,p2,qph,ng,icut)
! written by CMCC, last modified 7/4/2006
! p1(0...3): final state electron four-momentum
! p2(0...3): final state positron four-momentum
! qph(0...40,0...3): four-momenta of 40 emitted photons
! icut: icut = 1 event rejected, icut = 0 event accepted
! ng: number of gammas TF
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),qph(40,0:3),q(0:3),ptmp(0:3)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/angularranges/thmine,thmaxe,thminp,thmaxp
      common/momentainitial/pin1(0:3),pin2(0:3)
      common/massainv/amassainvmin,amassainvmax
      common/ifirstcuts/ifirst
      data ifirst /0/
      common/idarkon/idarkon
      dimension ptvector(0:3)
      
      double precision BEEMIN, BTEMIN, BEGMIN, BTGMIN, BEEVETO, BTEVETO, BEGVETO, BTGVETO, BPRESCALE
      integer BUSERCUTS, IBUSERMODE
      character*10  BUSERMODE
      COMMON / USERCUTS / BEEMIN, BTEMIN, BEGMIN, BTGMIN, BEEVETO, BTEVETO, BEGVETO, BTGVETO
      COMMON / PRESCALE / BPRESCALE
      COMMON / USERCUTSINT / BUSERCUTS
      COMMON / USERCUTSMODE / BUSERMODE
      double precision maxbhabhaborninv, minbhabhaborn
      
      if (ifirst.eq.0) then

c         print*,'invariant mass cut hardwired!!'
c         print*,'invariant mass cut hardwired!!'
c         print*,'invariant mass cut hardwired!!'
c         print*,'invariant mass cut hardwired!!'
c         print*,'invariant mass cut hardwired!!'
c         print*,'invariant mass cut hardwired!!'
c         print*,'invariant mass cut hardwired!!'
c         print*,'invariant mass cut hardwired!!'
         
         
         ifirst = 1
      endif
      
      icut = 1 ! event rejected

      if(p1(0).lt.emin.or.p2(0).lt.emin) return

      z = acollinearityrad(p1,p2)
      if (z.gt.zmax) return

      c1 = p1(3)/sqrt(tridot(p1,p1))
      c2 = p2(3)/sqrt(tridot(p2,p2))
      th1 = acos(c1)
      th2 = acos(c2)

      thmine = thmin
      thmaxe = thmax
      thminp = thmin
      thmaxp = thmax
***********************************
*      additional cuts
c      drad = 3.d0*pi/180.d0
c      thmaxe = thmine+drad
c      thminp = thmaxp-drad
***********************************
      if (th1.lt.thmine.or.th1.gt.thmaxe) return
      if (th2.lt.thminp.or.th2.gt.thmaxp) return


!       USER CUTS
      if (BUSERCUTS.gt.0) then
      
! 	print *,'usercuts'
	icutuser = 1 ! event rejected
	icut = icut + icutuser

        if (BUSERMODE.eq.'GAMMA') then
          ! both electrons: theta below teveto or energy below eeveto
          if((p1(0).gt.BEEVETO).and.(th1.gt.BTEVETO).and.(th1.lt.(pi-BTEVETO))) then
	    return
	  endif
          if((p2(0).gt.BEEVETO).and.(th2.gt.BTEVETO).and.(th2.lt.(pi-BTEVETO))) then
	    return
	  endif
          ! one gamma above TGMIN with TEMIN and no more above TGVETO and EGVETO
          ngammaaccept = 0
          ngammaveto = 0
	  if (ng.gt.0) then
	  
	    ! accept
	    igammaaccept = -1
	    do k = 1,ng
	      do i = 0,3
                q(i) = qph(k,i)
	      enddo
	      cg1 = q(3)/sqrt(tridot(q,q))
	      thg1 = acos(cg1)
	      if ((q(0).gt.BEGMIN).and.(thg1.gt.BTGMIN).and.(thg1.lt.(pi-BTGMIN))) then
	        ngammaaccept = ngammaaccept + 1
	        igammaaccept = k
	      endif
	    enddo
	    if (ngammaaccept.ne.1) return
	    
	    ! veto
	    do k = 1,ng
	      if (k.ne.igammaaccept) then
		do i = 0,3
		  q(i) = qph(k,i)
		enddo
		cg1 = q(3)/sqrt(tridot(q,q))
		thg1 = acos(cg1)
		if ((q(0).gt.BEGVETO).and.(thg1.gt.BTGVETO).and.(thg1.lt.(pi-BTGVETO))) then
		  ngammaveto = ngammaveto + 1
		endif
	      endif
	    enddo
	    if (ngammaveto.gt.0) return
	  else
	    return
	  endif	
	! end GAMMA
	
	elseif (BUSERMODE.eq.'EGAMMA') then
	  ! one electron accepted, the other vetoed, dont care which one
	  veto1   = 0
	  accept1 = 0
	  veto2   = 0
	  accept2 = 0
          if((p1(0).gt.BEEMIN).and.(th1.gt.BTEMIN).and.(th1.lt.(pi-BTEMIN))) accept1 = 1
          if((p1(0).gt.BEEVETO).and.(th1.gt.BTEVETO).and.(th1.lt.(pi-BTEVETO))) veto1 = 1
          if((p2(0).gt.BEEMIN).and.(th2.gt.BTEMIN).and.(th2.lt.(pi-BTEMIN))) accept2 = 1
          if((p2(0).gt.BEEVETO).and.(th2.gt.BTEVETO).and.(th2.lt.(pi-BTEVETO))) veto2 = 1

	  if((accept1.eq.0).and.(accept2.eq.0)) return !no good electron
	  if((accept1.eq.1).and.(veto2.eq.1)) return
	  if((accept2.eq.1).and.(veto1.eq.1)) return

	  ! one gamma above TGMIN with TEMIN and no more above TGVETO and EGVETO
          ngammaaccept = 0
          ngammaveto = 0
	  if (ng.gt.0) then

	    ! accept
	    igammaaccept = -1
	    do k = 1,ng
	      do i = 0,3
                q(i) = qph(k,i)
	      enddo
	      cg1 = q(3)/sqrt(tridot(q,q))
	      thg1 = acos(cg1)
	      if ((q(0).gt.BEGMIN).and.(thg1.gt.BTGMIN).and.(thg1.lt.(pi-BTGMIN))) then
	        ngammaaccept = ngammaaccept + 1
	        igammaaccept = k
	      endif
	    enddo
	    if (ngammaaccept.ne.1) return

	    ! veto
	    do k = 1,ng
	      if (k.ne.igammaaccept) then
		do i = 0,3
		  q(i) = qph(k,i)
		enddo
		cg1 = q(3)/sqrt(tridot(q,q))
		thg1 = acos(cg1)
		if ((q(0).gt.BEGVETO).and.(thg1.gt.BTGVETO).and.(thg1.lt.(pi-BTGVETO))) then
		  ngammaveto = ngammaveto + 1
		endif
	      endif
	    enddo
	    if (ngammaveto.gt.0) return
	  else
	    return
	  endif	
	! end EGAMMA
	
	elseif (BUSERMODE.eq.'ETRON') then
	! one electron accepted, the other vetoed, dont care which one
	  veto1   = 0
	  accept1 = 0
	  veto2   = 0
	  accept2 = 0
          if((p1(0).gt.BEEMIN).and.(th1.gt.BTEMIN).and.(th1.lt.(pi-BTEMIN))) accept1 = 1
          if((p1(0).gt.BEEVETO).and.(th1.gt.BTEVETO).and.(th1.lt.(pi-BTEVETO))) veto1 = 1
          if((p2(0).gt.BEEMIN).and.(th2.gt.BTEMIN).and.(th2.lt.(pi-BTEMIN))) accept2 = 1
          if((p2(0).gt.BEEVETO).and.(th2.gt.BTEVETO).and.(th2.lt.(pi-BTEVETO))) veto2 = 1

	  if((accept1.eq.0).and.(accept2.eq.0)) return !no good electron
	  if((accept1.eq.1).and.(veto2.eq.1)) return
	  if((accept2.eq.1).and.(veto1.eq.1)) return

	  ! one gamma above TGMIN with TEMIN and no more above TGVETO and EGVETO
          ngammaveto = 0
	  if (ng.gt.0) then
	    ! veto
	    do k = 1,ng
	      do i = 0,3
		q(i) = qph(k,i)
	      enddo
	      cg1 = q(3)/sqrt(tridot(q,q))
	      thg1 = acos(cg1)
	      if ((q(0).gt.BEGVETO).and.(thg1.gt.BTGVETO).and.(thg1.lt.(pi-BTGVETO))) then
		ngammaveto = ngammaveto + 1
	      endif
	    enddo
	    if (ngammaveto.gt.0) return
	  endif	
        ! end ETRON
        	
	elseif (BUSERMODE.eq.'PRESCALE') then
! 	  print *,'PRESCALE'
	  if (firstprescale.eq.0) then 
	    firstprescale = 1
	    minbhabhaborn = findminborn(10000)
	  endif
	  	  
	  !get random value
	  rr = babayaganlo_rndm(0)
	  if (rr.gt.(minbhabhaborn/bhabhaborn(th1))) then
	    return
          endif
          BPRESCALE = bhabhaborn(th1)/minbhabhaborn
          if ((BPRESCALE-1.d0).lt.1.d-20) then
            call babayaganlo_warning_prescaleweight(BPRESCALE)
          endif
          ! end 
	endif
	
        icutuser = 0 ! event accepted
      else
      	icutuser = 0 ! event accepted
      endif

      icut = 0 ! event accepted
      icut = icut + icutuser
      
      return
      end
**************************************************************************
*
*
**************************************************************************
      subroutine cutsgg(ng,p1,p2,qph,icut)
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),qph(40,0:3),q(0:3),q1(0:3),q2(0:3)
      dimension qphtot(40,0:3)
      dimension icutmat(1:ng+1,2:ng+2)
      dimension icutvec((ng+2)*(ng+1)/2)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      character*2 fs
      common/finalstate/fs
      icut = 1 ! event rejected
      nph    = ng + 2
      npairs = (nph)*(nph-1)/2
      do i=0,3
         qphtot(1,i) = p1(i)
         qphtot(2,i) = p2(i)
      enddo
      if (ng.gt.0) then
         do i = 0,3
            do k = 1,ng
               qphtot(k+2,i) = qph(k,i)
            enddo
         enddo
      endif
      ip = 0
      icuttot = 0
      do ka = 1,nph-1
         do kb = ka+1,nph
            ip = ip + 1
            do i = 0,3
               q1(i) = qphtot(ka,i)
               q2(i) = qphtot(kb,i)
            enddo
            call cuts(q1,q2,qph,ng,icuttmp)
            icuttot = icuttot + icuttmp
c            icutvec(ip) = icuttmp

c            if (icuttmp.eq.0) then
c               icut = 0
c               return
c            endif

        enddo
      enddo
c      if (ng.gt.0.and.icuttot.lt.npairs.and.icutvec(1).ne.0) 
c     .     print*,icutvec
      if (icuttot.lt.npairs) icut = 0
      return
      end
************************************************************************



************************************************************************
* Torben Ferber, 2015
      double precision function bhabhaborn(theta)
      implicit none
      double precision theta, xsecA, tA1, tA2, tA3

      tA1 = (1.d0 + cos(theta/2.d0)**4)/(sin(theta/2.d0)**4)
      tA2 = (1.d0 + cos(theta)**2)/2.d0
      tA3 = -(2.d0*cos(theta/2.d0)**4)/(sin(theta/2.d0)**2)
      xsecA = tA1 + tA2 + tA3
      
      bhabhaborn  = xsecA
      
      end function bhabhaborn
************************************************************************

************************************************************************
* Torben Ferber, 2015
      double precision function findminborn(steps)
      implicit none
      double precision minborn, thisborn, thistheta, rr, thrange, thstep
      
      double precision bhabhaborn
      external bhabhaborn
      
      double precision thmine,thmaxe,thminp,thmaxp
      common/angularranges/thmine,thmaxe,thminp,thmaxp
      
      integer steps, k

      if(steps.lt.100) then
        print *, 'Need more steps to find maximum inverese xsec! (ABORT)'
        call abort
      endif
      
      thrange = thmaxe - thmine
      thstep = abs(thrange/steps)
      minborn = 999.d10;
      
      do k = 0,steps-1
        thistheta = thmine + k*thstep
        thisborn = bhabhaborn(thistheta)
        
        if (thisborn.lt.minborn) then
          minborn = thisborn
        endif
        
      enddo    
      
      findminborn = minborn * 0.90
      
      end function findminborn
************************************************************************
