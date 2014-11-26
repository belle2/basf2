      subroutine regulate_gg(ng,p1,p2,k1,k2,qph,reg)
      implicit double precision (a-h,k,o-z)
      integer ki
      real*4 csi(1)
      common/ggreg/ich
      dimension p1(0:3),p2(0:3),k1(0:3),k2(0:3),k3(0:3),qph(40,0:3)
      dimension tmp(0:3),qt(40,0:3),k4(0:3)
      reg     = 1.d0
      reg2    = 1.d0
      nphtrue = ng + 2
!       call ranlux(csi,1)
      csi(1) = babayaganlo_rndm(0)

      ich = 1 + csi(1)*nphtrue
      if (ich.gt.nphtrue) ich = nphtrue
*
      do i = 0,3
         qt(1,i) = k1(i)
         qt(2,i) = k2(i)
         do ki = 1,ng
            qt(2+ki,i) = qph(ki,i)
         enddo
      enddo
*
      den = 0.d0
      do ki = 1,nphtrue
         prod = 1.d0
         do i = 1,nphtrue
            if (i.ne.ki) prod = prod * dp1p2qtm1(p1,p2,qt,i)
         enddo
         if (ki.eq.2) an = prod
         den = den + prod
      enddo
      reg = nphtrue*an/den * reg2
      if (ich.eq.2) return
      if (ich.eq.1) then
         call exchange_mom(k1,k2)
      else if (ich.ge.3) then
         do i = 0,3
            tmp(i) = qph(ich-2,i)
         enddo
         call exchange_mom(k2,tmp)
         do i = 0,3
            qph(ich-2,i) = tmp(i)
         enddo
      endif
      return
      end
***************************************************************************
*
*
***************************************************************************
      function dp1p2km1(p1,p2,q)
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),q(0:3)
      dp1p2km1 = 1.d0/dot(p1,q)/dot(p2,q)
      return
      end
***************************************************************************
*
*
***************************************************************************
      function dp1p2qtm1(p1,p2,q,i)
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),q(40,0:3),tmp(0:3)
      do k = 0,3
         tmp(k) = q(i,k)
      enddo
      dp1p2qtm1 = dp1p2km1(p1,p2,tmp)
      return
      end
***************************************************************************
*
*
***************************************************************************
      subroutine  regulate_ggokfor1phot(nphot,p1,p2,k1,k2,qph,reg)
      implicit double precision (a-h,k,o-z)
      real*4 csi(1)
      dimension p1(0:3),p2(0:3),k1(0:3),k2(0:3),k3(0:3),qph(40,0:3)
      dimension tmp(0:3)
c      print*,'ONLY FOR 1 EXTRA PHOTON!! -- sampling.f line 67'
      reg = 1.d0
      npairs = (nphot+2)*(nphot+1)/2
!       call ranlux(csi,1)
      csi(1) = babayaganlo_rndm(0)
      ipair = 1 + csi(1)*npairs
      if (ipair.gt.npairs) ipair = npairs
      do i = 0,3
         k3(i) = qph(1,i)
      enddo
      den =
     .     (
     .     dp1p2km1(p1,p2,k1)*dp1p2km1(p1,p2,k3)+
     .     dp1p2km1(p1,p2,k2)*dp1p2km1(p1,p2,k3)+
     .     dp1p2km1(p1,p2,k1)*dp1p2km1(p1,p2,k2)
     .     )
      an = dp1p2km1(p1,p2,k1)*dp1p2km1(p1,p2,k3)
      reg = npairs*an/den
c      if (ipair.eq.1) doing nothing!
      if (ipair.eq.2) then
         call exchange_mom(k1,k2)
         return
      endif
      if (ipair.eq.3) then
         call exchange_mom(k2,k3)
         do i = 0,3
            qph(1,i) = k3(i)
         enddo
         return
      endif
      return
      end
***************************************************************************
*
*
***************************************************************************
      subroutine photonenergygg(ep,a,b,omin,om,w)
! written by CMCC, last modified June 2007
! generates according to 1/(a-om*b)/om with omin < om < ep
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1),csib(1)
      character*2 fs
      logical boh
      integer*8 icount,ipippo
      common/finalstate/fs
      common/intphengg/ipippo,icount
      common/idebugging/idebug
      data icount /0/
      data ipippo /0/

      icount = icount + 1

      r = a/b

! not needed, the control is done outside...
c$$$      icount = icount + 1
c$$$      ctrl = abs(r-ep)*1d20
c$$$      if (ctrl.lt.1.d0) then
c$$$         ipippo = ipippo + 1
c$$$         if (ipippo.le.100) then
c$$$            print*,'WARNING !! (printed up to 100 times) (',ipippo,')'
c$$$            print*,'HIT CTRL in photonenergygg !! ',ctrl,icount
c$$$         endif
c$$$         om = omin
c$$$         w  = 0.d0        
c$$$         return
c$$$      endif
      ! THIS RESOLVES THE g77 CRASH, EVEN I DID NOT UNDERSTAND COMPLETELY
      boh = (r-ep.eq.0.d0).or.((a/b-ep).le.0.d0)
      if (boh) then
         om = omin
         w = 0.d0
         return
      endif





      an = 1.d0/a*log((r-omin)/(r-ep) * ep/omin )
!       call ranlux(csi,1)
      csi(1) = babayaganlo_rndm(0)

      ak = exp(an*a*csi(1))
      om = omin * r * ak /(r-omin+omin*ak)
      w  = (a-om*b)*om*an
      return
      end
***************************************************************************
*
*
***************************************************************************
      subroutine photonenergyee(phard,e,omax,omin,a,b,om,w)
      implicit double precision (a-h,o-z)
      real*4 csi(2)
      xmin = omin/e
      xmax = omax/e
      alpha = 2.d0
      w = 1.d0
      psoft = 1.d0 - phard

      anhard = 1.d0/b*log((a-b*omin)/(a-b*omax))
cccc      anhard = 1.d0/b/alpha*(1.d0/(a-b*omax)**alpha-
cccc     .                       1.d0/(a-b*omin)**alpha)

      ansoft = 2.d0*log(xmax/xmin)+0.5d0*(xmax**2-xmin**2)
     .        -2.d0*(xmax-xmin)

*      print*,omax,e,a,b
*      print*,ansoft,anhard,' sampling line 158'

c      ansoft = log(omax/omin)
!       call ranlux(csi,2)
      csi(1) = babayaganlo_rndm(0)
      csi(2) = babayaganlo_rndm(0)

      if (csi(1).le.psoft) then
         call ap_vertexcut(xmin,xmax,x)
         y  = 1.d0-x
         om = e*x
         w  = w*e
c     wi = 1.d0/anorm*(1+y**2)/(1.d0-y)
c     w = omax*1.d0/(psing*wi + pflat/(omax-omin))
c     om = omin*exp(csi(2)*ansoft)
      else
         om = (a-(a-b*omin)*exp(-b*csi(2)*anhard))/b
cccc         cnba  = csi(2)*anhard*b*alpha
cccc         ambo = (a-b*omin)**alpha/(1.d0+cnba*(a-b*omin)**alpha)
cccc         ambo = ambo**(1.d0/alpha)
cccc         om   = (a-ambo)/b
         x = om/e
         y = 1.d0 - x
      endif
      w = w/(psoft/ansoft*(1.d0+y**2)/(1.d0-y)+phard/anhard/(a-b*om))
cccc      w = w/(psoft/ansoft*(1.d0+y**2)/(1.d0-y)
cccc     .      +phard/anhard/(a-b*om)**(1.d0+alpha))
      return
      end
***************************************************************************
*
*
***************************************************************************
      subroutine photon_energy(ibich,pflat,expgg,omax,om,w)
! written by CMCC, last modified (I don't remember)
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1),csib(1)
      character*2 fs
      common/finalstate/fs
      common/epssoft/eps

      double precision omin,anir,anorm,eps

      integer ifirst
      data ifirst /0/

      save ifirst,omin,anir,anorm

      if (ibich.ne.0.and.ibich.ne.1) then
         print*,'WRONG IBICH in photon_energy!'
         stop
      endif

      psing = 1.d0 - pflat

      if (ifirst.eq.0) then
        ifirst = 1
        omin = eps*omax
        anir = dlog(omax/omin)
        anorm  = -2.d0*dlog(eps)-1.5d0+2.d0*eps-0.5d0*eps**2
      endif

      if (pflat.gt.0.d0) then
!          call ranlux(csi,1)
	 csi(1) = babayaganlo_rndm(0)

      else
         csi(1) = 0.5
      endif

      if ((fs.eq.'ee'.or.fs.eq.'mm').or.ibich.eq.0) then

         if (csi(1).lt.psing) then
            call ap_vertex(y,x)
            om = omax*x
            wi = 1.d0/anorm*(1+y**2)/(1.d0-y)
            w = omax*1.d0/(psing*wi + pflat/(omax-omin))
         else
!             call ranlux(csi,1)
            csi(1) = babayaganlo_rndm(0)
            om = (omax - omin)*csi(1) + omin
            x = om/omax
            y = 1.d0 - x
            wsing = 1.d0/anorm*(1+y**2)/(1.d0-y)
            w = 1.d0/(psing*wsing + pflat/(omax-omin))
         endif

      elseif (fs.eq.'gg'.and.ibich.eq.1) then
         a = expgg
         b = -a
         upb = 1.d0 + b
         an = 1.d0/upb*(omax - omin)**upb
!          call ranlux(csi,1)
         csi(1) = babayaganlo_rndm(0)
!          call ranlux(csib,1)
         csib(1) = babayaganlo_rndm(0)
         if (csi(1).lt.0.5d0) then
            om = omax - (omax-omin)*(1.d0-csib(1))**(1.d0/upb)
            wnf = anir*an*om*(omax-om)**a/(anir*om+an*(omax-om)**a)
            w = wnf  * 2.d0
         else
            om = omin*dexp(anir*csib(1))
            wnf = anir*an*om*(omax-om)**a/(anir*om+an*(omax-om)**a)
            w = wnf * 2.d0
         endif

      endif

      return
      end
***************************************************
      subroutine photon_energy_new(ibich,pflat,expgg,omax,omin,
     .     ptld,v,om,w)
! written by CMCC, last modified 20/9/2007
      implicit double precision (a-h,o-z)
      dimension ptld(0:3),v(0:3)
      real*4 rnd,csi(1),csib(1)
      common/idebugging/idebug
      character*2 fs
      common/finalstate/fs
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/parameters/ame,ammu,convfac,alpha,pi
      common/newomegasampling/smin,ifirst
      data ifirst /0/
      if (ibich.ne.0.and.ibich.ne.1) then
         print*,'WRONG IBICH in photon_energy!'
         stop
      endif
      psing = 1.d0 - pflat
      anir = log(omax/omin)
      if (pflat.gt.0.d0) then
!          call ranlux(csi,1)
         csi(1) = babayaganlo_rndm(0)
      else
         csi(1) = 0.5
      endif
      eps   = omin/omax
      if ((fs.eq.'ee'.or.fs.eq.'mm').or.ibich.eq.0) then
         if (ifirst.eq.0) then
            am = ame
            if (fs.eq.'mm') am = ammu
            e    = max(am,emin)
            be2  = 1.d0-am*am/e/e
            czmax = 1.d0 !cos(zmax)
            smin = 2.d0*am*am + 2.d0*e*e*(1.d0-be2*czmax)
            ifirst = 1
         endif
         a = dot(ptld,ptld)
         b = 2.d0*(ptld(0)-tridot(ptld,v))
         omi = omin
         oMA = (a-smin)/b

         r   = b/a
         q   = (1.d0-r*omi)/omi
         an  = 1.d0/a*log(oMA/(1.d0-r*oMA)*q)
!          call ranlux(csib,1)
         csi(1) = babayaganlo_rndm(0)

         z   = exp(an*a*csib(1))/q
         om  = z/(1.d0+r*z)
         w   = an * a*om*(1.d0-r*om)

c         al = 1.01d0
c         uma = 1.d0 - al
c         an = 1.d0/b/uma*((a-b*omi)**uma - (a-b*oMA)**uma)
c         call ranlux(csib,1)
c         q  = (a-b*omi)**uma - an*csib(1)*b*uma
c         om = (a-q**(1.d0/uma))/b
c         w  = an * (a-b*om)**al

c         print*,omi,om,oMA

c         print*,w,om
c$$$         anorm = -2.d0*log(eps)-1.5d0+2.d0*eps-0.5d0*eps**2
c$$$         if (csi(1).lt.psing) then
c$$$            call ap_vertex(omin,omax,y,x)
c$$$            om = omax*x
c$$$            wi = 1.d0/anorm*(1+y**2)/(1.d0-y)
c$$$            w = omax*1.d0/(psing*wi + pflat/(omax-omin))
c$$$         else
c$$$            call ranlux(csi,1)
c$$$            om = (omax - omin)*csi(1) + omin
c$$$            x = om/omax
c$$$            y = 1.d0 - x
c$$$            wsing = 1.d0/anorm*(1+y**2)/(1.d0-y)
c$$$            w = 1.d0/(psing*wsing + pflat/(omax-omin))
c$$$         endif
      elseif (fs.eq.'gg'.and.ibich.eq.1) then
         a = expgg
         b = -a
         upb = 1.d0 + b
         an = 1.d0/upb*(omax - omin)**upb
!          call ranlux(csi,1)
	    csi(1) = babayaganlo_rndm(0)

!          call ranlux(csib,1)
	    csib(1) = babayaganlo_rndm(0)

         if (csi(1).lt.0.5d0) then
            om = omax - (omax-omin)*(1.d0-csib(1))**(1.d0/upb)
            wnf = anir*an*om*(omax-om)**a/(anir*om+an*(omax-om)**a)
            w = wnf  * 2.d0
         else
            om = omin*exp(anir*csib(1))
            wnf = anir*an*om*(omax-om)**a/(anir*om+an*(omax-om)**a)
            w = wnf * 2.d0
         endif
      endif
      return
      end
***************************************************
      subroutine ap_vertex(x,omx)
      implicit double precision (a-h,o-z)
! written by CMCC, essentially from old BABAYAGA
*  x generation according to ap splitting function
*  (1+x^2)/(1-x), 0 <= x <= 1-eps
* added 15/10/2005, modified from babayaga, older releases by CMCC
      double precision eps
      common/epssoft/eps
      real*4 r(2)

      integer ifirst
      data ifirst /0/
      save ifirst

      double precision alne
      save alne

      if (ifirst.eq.0) then
        ifirst = 1
        alne = dlog(eps)
      endif

      irigenera = 1
      do while(irigenera.eq.1)
!          call ranlux(r,2)
         r(1) = babayaganlo_rndm(0)
         r(2) = babayaganlo_rndm(0)

         cx  = r(1)
         omx = dexp(cx*alne)
         x   = 1.d0 - omx
         rx  = r(2)*2.d0/omx
         px  = (1.d0 + x**2)/omx
         if (rx.lt.px) irigenera = 0
      enddo
      return
      end
***
      subroutine ap_vertexcut(xmin,xmax,x)
! om = x*e
      implicit double precision (a-h,o-z)
      real*4 r(2)
      alne = log(xmax/xmin)
      irigenera = 1
      do while(irigenera.eq.1)
!          call ranlux(r,2)
	 r(1) = babayaganlo_rndm(0)
         r(2) = babayaganlo_rndm(0)

         x = xmin*exp(r(1)*alne)
         rx = r(2)*2.d0/x
         px = (1.d0 + (1.d0-x)**2)/x
         if (rx.lt.px) irigenera = 0
      enddo
      return
      end
***
      subroutine photonenergyggfirst(ep,omax,omin,om,w)
! written by CMCC, last modified (I don't remember)
! generates according to 1/(omax-om)/om with omin < om < ep
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1),csib(1)
      character*2 fs
      common/finalstate/fs
      an = 1.d0/omax*log((omax-omin)/(omax-ep) * ep/omin )
!       call ranlux(csi,1)
      csi(1) = babayaganlo_rndm(0)

      ak = exp(an*omax*csi(1))
      om = omin* omax * ak /(omax-omin+omin*ak)
      w  = (omax-om)*om*an
      return
      end
******************************************************************
*
*
******************************************************************
      subroutine multiplicity(eps,ecms,cth,n,w)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,l,o-z)
      real*4 csi(1)
      dimension vect(0:8)
      character*6 ord
      character*2 fs
      common/finalstate/fs
      common/qedORDER/ord
      common/nphot_mode/nphotmode
      common/parameters/ame,ammu,convfac,alpha,pi
      common/coseni/cmax,cmin

      if (nphotmode.ge.0) then
         w = 1.d0
         n = nphotmode
         return
      endif

      if (ord.eq.'born') then
         w = 1.d0
         n = 0
         if (idarkon.eq.1) n = 1
         return
      endif

      if (ord.eq.'struct') then
         w = 1.d0
         n = 0
         if (idarkon.eq.1) n = 1
         return
      endif

      if (ord.eq.'alpha2') then
!          call ranlux(csi,1)
         csi(1) = babayaganlo_rndm(0)
         unte = 1.d0/3.d0
         dute = 2.d0/3.d0
         if (csi(1).lt.unte) n = 0
         if (csi(1).ge.unte.and.csi(1).lt.dute) n = 1
         if (csi(1).ge.dute) n = 2
         w = 1.d0/unte

         unte = 57.d0/100.d0
         dute = 64.d0/100.d0
         if (csi(1).lt.unte) then 
            n = 0
            w = 1.d0/unte
         endif
         if (csi(1).ge.unte.and.csi(1).lt.dute) then
            n = 1
            w = 1.d0/(dute-unte)
         endif
         if (csi(1).ge.dute) then 
            n = 2
            w = 1.d0/(1.d0 - dute)
         endif
         return
      endif

      if (fs.eq.'ee') then
         scale = ecms**2 * (1.d0 - cth)/(1.d0 + cth)
         aieps = -2.d0*log(eps)-1.5d0+2.d0*eps-0.5d0*eps**2
         lcoll = log(scale/ame**2) - 1.d0
         arg = 2.d0*alpha/pi * lcoll * aieps
      elseif (fs.eq.'mm') then
         scale = ecms**2 * (1.d0 - cth)/(1.d0 + cth)
         aieps = -2.d0*log(eps)-1.5d0+2.d0*eps-0.5d0*eps**2
         lcoll = 0.5d0*log(scale/ame**2)+0.5d0*log(scale/ammu**2)-1.d0
         arg = 2.d0*alpha/pi * lcoll * aieps
      elseif (fs.eq.'gg') then
         scale = ecms**2
         aieps = -2.d0*log(eps)-1.5d0+2.d0*eps-0.5d0*eps**2
         lcoll = log(scale/ame**2) - 1
         arg = alpha/pi * lcoll * aieps
      endif

      if (ord.eq.'alpha') then
         sffa = 1.d0 - arg
         if (sffa.lt.0.d0) then
            sffa = 0.1d0
         endif
         p0 = sffa
         p1 = 1.d0 - sffa      
         ptot = p0 + p1
!          call ranlux(csi,1)
	 csi(1) = babayaganlo_rndm(0)

         if (csi(1).lt.p0) then 
            n = 0
            w = ptot / p0
         else
            n = 1
            w = ptot / p1
         endif
      endif

      if (ord.eq.'exp') then
         ffs = dexp(-arg)
!          call ranlux(csi,1)
         csi(1) = babayaganlo_rndm(0)

         cs = 1.d0 * csi(1)
         ptot = ffs
         pn   = ptot
         n = 0
         w = 1.d0/pn
         do while(cs.gt.ptot)
            n = n + 1
            pn = pn*arg/n
            ptot = ptot + pn
         enddo
         w = 1.d0/pn
         return
      endif     
      return
      end
*******************************************************************
*
*
*******************************************************************
      subroutine get_cos_fer(c,w)
! written by CMCC, last modified 18/9/2007
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1),r(2)
      character*2 fs
      common/finalstate/fs
      common/coseni/cmax,cmin
      common/iwide/iwider,ntot  ! ntot is the number of photons
      common/enableifirst/ifrstenbld

      double precision anorm, fmax
      integer ifirst
      data ifirst /0/

      save anorm,fmax,ifirst

      if (fs.eq.'mm') then
         if (ifirst.eq.0.or.ifrstenbld.eq.0) then
            fmax = 1.d0+max(cmax**2,cmin**2)
            anorm = (cmax-cmin) + 1.d0/3.d0*(cmax**3-cmin**3)
         endif
         iagain = 1

         do while(iagain.eq.1)
!             call ranlux(r,2)
            r(1) = babayaganlo_rndm(0)
            r(2) = babayaganlo_rndm(0)

            c  = (cmax-cmin)*r(1) + cmin
            f  = fmax*r(2)
            fc = (1.d0+c*c)
            if (f.lt.fc) iagain = 0            
         enddo
         w = anorm/(1.d0+c*c)
         ifirst = 1
         return
      endif

      cmaxh = cmax 
      cminh = cmin 

      if (ifirst.eq.0.or.ifrstenbld.eq.0) then
         anorm = -31.d0/3.d0 - 16.d0/(cmax - 1.d0) + 9.d0*cmax
     .        + cmax**2 + cmax**3/3.d0 + 16.d0*dlog(1.d0 - cmax)

         anorm = anorm +31.d0/3.d0 + 16.d0/(cmin - 1.d0) - 9.d0*cmin
     .        - cmin**2 - cmin**3/3.d0 - 16.d0*dlog(1.d0 - cmin)
         anorm = anorm/2.d0
         fmax = fcos(cmax)
         ifirst = 1
      endif

      istop = 0

      do while(istop.eq.0)

!          call ranlux(r,2)
         r(1) = babayaganlo_rndm(0)
         r(2) = babayaganlo_rndm(0)

         c = ((cmaxh-cminh)*r(1)+(1.d0-cmaxh)*cminh)/
     >        (1.d0-cmaxh+(cmaxh-cminh)*r(1))   
         fc = 8.d0/(1.d0 - c)**2
         ratio = fcos(c)/fc
         if (r(2).lt.ratio) istop = 1

      enddo

      w = 1.d0/fcos(c)*anorm

      return
      end
**************************************************************************
*
*
**************************************************************************
      function fcos(c)
      implicit none
      double precision fcos,c,c2,upc2
      c2 = c*c
      upc2 = (1.d0+c)*(1.d0+c)
      fcos = (1.d0+c2)/2.d0 + (4.d0+upc2)/(1.d0-c)/(1.d0-c)
     .     - upc2/(1.d0 - c)
      return
      end
**************************************************************************
*
*
**************************************************************************
      subroutine get_cos_2g(c,w)
! written by CMCC, last modified 18/10/2005
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1)
      common/coseni/cmax,cmin
      common/various/beta
      common/iwide/iwider,ntot  ! ntot is the number of photons
      common/ifirst_cos_2g/anorm,al1pb1mb,betah,ifirst
      common/enableifirst/ifrstenbld
      data ifirst /0/
      cmaxh = cmax 
      cminh = cmin
      if (ifirst.eq.0.or.ifrstenbld.eq.0) then
         betah    = 1.d0*beta
         al1pb1mb = log((1.d0+betah)/(1.d0-betah))
         anorm    = 1.d0/betah * al1pb1mb
         ifirst   = 1
      endif
      pflat = 0.d0
      psing = 1.d0 - pflat
      if (pflat.gt.0.00000001d0) then
!          call ranlux(csi,1)
         csi(1) = babayaganlo_rndm(0)

         rnd = csi(1)
      else
         rnd = 0.5
      endif
!       call ranlux(csi,1)
      csi(1) = babayaganlo_rndm(0)

      if (rnd.lt.pflat) then
         c = 2.d0*csi(1) - 1.d0
      else
         abig = 2.d0*betah*anorm*csi(1) - al1pb1mb
         c    = 1.d0/betah * (exp(abig)-1.d0)/(exp(abig)+1.d0)
      endif
      w = 1.d0 /(pflat/2.d0 + psing/anorm/(1.d0-betah**2*c**2))
      return
      end
      subroutine get_cos_2g_born(c,w)
! written by CMCC, last modified 18/10/2005
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1)
      common/coseni/cmax,cmin
      common/various/beta
      common/iwide/iwider,ntot  ! ntot is the number of photons
      common/ifirstcos2gborn/anorm,al1pb1mb,ifirst
      common/enableifirst/ifrstenbld
      data ifirst /0/
      if (ifirst.eq.0.or.ifrstenbld.eq.0) then
         al1pb1mb = log((1.d0-beta*cmin)/(1.d0+beta*cmin))
         pippo    = log((1.d0+beta*cmax)/(1.d0-beta*cmax))
         anorm    = 0.5d0/beta * (al1pb1mb+pippo)
         ifirst   = 1
c         write(*,'(A,f30.18)')'beta =',beta
c         write(*,'(A,f30.18)')'al1. =',al1pb1mb
c         write(*,'(A,f30.18)')'pipp =',pippo
c         write(*,'(A,f30.18)')'anor =',anorm
c         write(*,'(f30.18)')sqrt((1.d0 - beta**2)*
c     $        (1.01964566130134870/2.d0)**2)
      endif
!       call ranlux(csi,1)
      csi(1) = babayaganlo_rndm(0)

      abig = 2.d0*beta*anorm*csi(1) - al1pb1mb
      c    = 1.d0/beta * (exp(abig)-1.d0)/(exp(abig)+1.d0)
      w    = anorm*(1.d0-beta**2*c**2)
      return
      end
************************************************************************
*
*
************************************************************************

      subroutine collinear(pflat,rnd1,rnd2,p,vers,w)
! written by CMCC, last modified 16/10/2005
      implicit double precision (a-h,o-z)
      dimension p(0:3),vers(0:3)
      real*4 csi(1),r(2),rnd1,rnd2
      character*2 fs
      character*1 s
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      data ifirst /0/
      save ifirst,b1e,ane,b1m,anm

      if (ifirst.eq.0) then
         ifirst = 1
         b1e   = dsqrt((1.d0 - ame/p(0))*(1.d0 + ame/p(0)))
         ane   = 1.d0/b1e*dlog((1.d0+b1e)/(1.d0-b1e))
         b1m   = dsqrt((1.d0 - ammu/p(0))*(1.d0 + ammu/p(0)))
         anm   = 1.d0/b1m*dlog((1.d0+b1m)/(1.d0-b1m))
      endif

      psing = 1.d0 - pflatl
      if (pflatl.gt.0.d0) then
!          call ranlux(csi,1)
         csi(1) = babayaganlo_rndm(0)

      else
         csi(1) = 0.5
      endif
      vers(0) = 1.d0
      phph = 2.d0*pi*rnd1

*****
      diff = dabs(dot(p,p) - ame*ame)
      if (diff.lt.1.d-7) then
         s = 'e' 
      else
         s = 'm'
      endif
*****
      if (fs.eq.'ee'.or.fs.eq.'mm') then
         if (s.eq.'e') then
            b1 = b1e
            an = ane
         else
            b1 = b1m
            an = anm
         endif
         if (csi(1).le.psing) then
            cth = (1.d0 - (1.d0+b1)*dexp(-b1*an*rnd2))/b1
            umcth2 = dabs((1.d0 + cth)*(1.d0 - cth)) ! abs to avoid prob.
            sth = dsqrt(umcth2)
            cph = dcos(phph)
            sph = dsin(phph)
            vers(1) = vers(0)*sph*sth
            vers(2) = vers(0)*cph*sth
            vers(3) = vers(0)*cth
            call rot(-1,p,vers,vers)
            wi = 1.d0/(1.d0 - b1*cth)/an
            wi = 1.d0/(psing*wi + pflatl/2.d0)
            w  = wi*2.d0*pi
         else
            cth = 2.d0*rnd2 - 1.d0
            sth = dsqrt(1.d0-cth*cth)
            cph = dcos(phph)
            sph = dsin(phph)
            vers(1) = vers(0)*sph*sth
            vers(2) = vers(0)*cph*sth
            vers(3) = vers(0)*cth
            cthp = tridot(p,vers)/b1/p(0)
            wi = 1.d0/(1.d0 - b1*cthp)/an
            wi = 1.d0/(psing*wi + pflatl/2.d0)
            w  = wi*2.d0*pi
         endif
      elseif (fs.eq.'gg') then
         call get_cos_2g(cth,wcos)
         w = wcos*2.d0*pi
         sth = dsqrt(1.d0 - cth**2)
         cph = dcos(phph)
         sph = dsin(phph)
         vers(1) = vers(0)*sph*sth
         vers(2) = vers(0)*cph*sth
         vers(3) = vers(0)*cth
      endif
      return
      end

*********************************************************************
* calcola l'energia del fotone emesso secondo una distribuzione piatta
* o secondo BW di larghezza gammaU centrata a amassU. pbw è la 
* probabilità che l'energia del fotone sia distribuita secondo BW, 
* (1-pbw) è la probabilità che sia distribuita piatta.
********************************************************************
      subroutine photon_energy_dm(omax,om,w)
!       implicit none
      implicit double precision (a-h,o-z)
      double precision omax,omin,om,w
      double precision q2max,q2min
      double precision mw,gwl,pbw,anbw,A,B,C,D
      double precision A1,B1,msugamma,mpergamma,mw2,BmenoA
      double precision x,y
      real*4 csi(1)
      double precision ame,ammu,convfac,alpha,pi
      common/parameters/ame,ammu,convfac,alpha,pi
      double precision amassU,gammaU,gvectU,gaxU
      common/darkmatter/amassU,gammaU,gvectU,gaxU
      double precision eps
      common/epssoft/eps

      integer ifirst
      data ifirst /0/
      save ifirst

      save q2min,mw,gwl,pbw,D,B,B1
      save msugamma, mpergamma, mw2

      if(ifirst.eq.0) then
          ifirst = 1

          pbw = 0.1d0

          mw     = amassU
          if (mw.eq.0.d0) then
            mw  = 1.d-6
            pbw = 0.d0
          endif

          gwl    = gammaU
          if (gwl.eq.0.d0) then
            gwl = 1.d-2
            pbw = 0.d0
          endif

          msugamma  = mw/gwl
          mpergamma = mw*gwl
          mw2       = mw**2

          q2min  = 4.d0*ame**2
          D  = (q2min/mw-mw)/gwl
          B  = 2.d0*datan(D/(1.d0+dsqrt(1.d0+D**2)))
          B1 = 1.d0/(q2min/mw-mw)
      endif

      omin   = eps*omax

** costanti per Breit Wigner
      q2max  = 4.d0*omax*(omax-omin)

      C  = (q2max/mw-mw)/gwl

      A  = 2.d0*datan(C/(1.d0+dsqrt(1.d0+C**2)))
      A1 = 1.d0/(q2max/mw-mw)

      anbw   = (A - B)/(4.d0*omax*mpergamma)
      BmenoA =  B - A

!       call ranlux(csi,1)
      csi(1) = babayaganlo_rndm(0)

      if (csi(1).lt.(1.d0-pbw)) then
!          call ranlux(csi,1)
	 csi(1) = babayaganlo_rndm(0)

         om = (omax - omin)*csi(1)*1.d0 + omin
         x  = om/omax
         y  = 4.d0*omax*(omax - om)
      else
!          call ranlux(csi,1)
         csi(1) = babayaganlo_rndm(0)

         y = csi(1)*1.d0
         if(y.lt.1.D-5) then
             x = gammaU * ((A1-B1)*y-A1)
             y = mpergamma * dcos(pi*y-x)/(pi*y-x) + mw2
         elseif(y.gt.1.d0-1.D-5) then
             x = gammaU * ((A1-B1)*y-A1)
             y = mpergamma * dcos(pi*y-x)/(pi*(1.d0-y)+x) + mw2
         else
             y = mpergamma * dtan(A*(1.d0-y)+B*y) + mw2
         endif
         om = omax-y/(4.d0*omax)
      endif
      w = 1.d0/
     &        ( pbw/((y-mw2)**2+mpergamma**2)/anbw
     &         + (1.d0-pbw)/(omax-omin))
      return
      end
