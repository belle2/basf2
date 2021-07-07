      subroutine regulate_gg(ng,p1,p2,k1,k2,qph,reg)
      implicit double precision (a-h,k,o-z)
      integer ki
      real*4 csi(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm
      common/ggreg/ich
      dimension p1(0:3),p2(0:3),k1(0:3),k2(0:3),k3(0:3),qph(40,0:3)
      dimension tmp(0:3),qt(40,0:3),k4(0:3)
      
      reg     = 1.d0
      reg2    = 1.d0
      nphtrue = ng + 2
      csi = babayaganlo_rndm(1)
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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      dimension p1(0:3),p2(0:3),k1(0:3),k2(0:3),k3(0:3),qph(40,0:3)
      dimension tmp(0:3)
c      print*,'ONLY FOR 1 EXTRA PHOTON!! -- sampling.f line 67'
      reg = 1.d0
      npairs = (nphot+2)*(nphot+1)/2
      csi = babayaganlo_rndm(1)
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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

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
      csi = babayaganlo_rndm(1)
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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

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
	 csi(1) = babayaganlo_rndm(0)
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
      subroutine photon_energy_IS(omin,omax,om,w)
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1),csib(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      eps = omin/omax
      anorm  = -2.d0*log(eps)-1.5d0+2.d0*eps-0.5d0*eps**2
      call ap_vertexNEW(eps,y,x)
      om = omax*x
      w  = omax*anorm/(1+y**2)*(1.d0-y)
      return
***      

      anir = log(omax/omin)
      csi(1) = babayaganlo_rndm(0)
      om = omin*exp(anir*csi(1))
      w = om*anir
      return
      end
***************************************************
***************************************************
      subroutine photon_energy_FSno(omin,omax,om,w)
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      alpha = 0.1d0

      umalpha = 1.d0 - alpha
      A       = omin**alpha/omin
      
      an1 = A * omin**umalpha/umalpha

      an2 = log(omax/omin)
      an = an1+an2
      
      csi(1) = babayaganlo_rndm(0)
      csiN = an * csi(1)

      if (csiN.le.an1) then
         om = (umalpha/A*csiN)**(1.d0/umalpha)
         w  = om**alpha * an / A
      else
         om = omin * exp(csiN-an1)
         w  = om * an
      endif
      
      return
      end
***************************************************
***************************************************
      subroutine photon_energy_FS(omin,omax,om,w)
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

!! sampling as om / omin**2 in [0,omin] and as 1/om in [omin,omax]
      
      an1 = 0.5d0
      an2 = log(omax/omin)      
      an  = an1+an2
      
      csi(1) = babayaganlo_rndm(0)
      csiN = an * csi(1)

      if (csiN.le.an1) then
         om = omin*sqrt(2.d0*csiN)
         w  = omin*omin * an/om
      else
         om = omin * exp(csiN-an1)
         w  = om * an
      endif
      
      return
      end
***************************************************

***************************************************
      subroutine photon_energy_FSok(omin,omax,om,w)
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      an1 = 1.d0
      an2 = log(omax/omin)      
      an = an1+an2
      
      csi(1) = babayaganlo_rndm(0)
      csiN = an * csi(1)

      if (csiN.le.an1) then
         om = omin*csiN
         w  = omin * an
      else
         om = omin * exp(csiN-an1)
         w  = om * an
      endif
      
      return
      end
***************************************************
      function fakeBW(s)
      implicit double precision (a-h,o-z)
      am = 0.9d0
      g  = 0.0005d0      
      fakeBW = 1.d0/((s-am*am)**2 + g*g*am*am)

      fakeBW = fakeBW/1d4 ! for printing
      
      return
      end
*****
      subroutine get_pattern(ncharged,n,ep)
      integer n,ep(n),ncharged
      real*4 csi(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm
      
      do k = 1,n
         csi(1) = babayaganlo_rndm(0)
         ep(k) = 1.d0*ncharged*csi(1) + 1
         if (ep(k).gt.ncharged) ep(k) = ncharged ! to avoid round-offs...
      enddo
      return
      end 
*****
      subroutine get_patternNEW(ncharged,n,ep,w)
      implicit double precision (a-h,o-z)
      integer n,ep(n),ncharged
      real*4 csi(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      double precision pc(0:4)
      common/emispattern/pis,pfs,pc,ifirst
      data ifirst /0/

      if (ifirst.eq.0) then
         pis = 0.5d0 ! 0.96d0
         pfs = 1.d0 - pis
c         do k = 1,5
c            print*,pis*100,' % photons from IS!!'
c         enddo
         pc(0) = 0.d0
         pc(1) = pc(0) + pis*0.5d0
         pc(2) = pc(1) + pis*0.5d0
         pc(3) = pc(2) + pfs*0.5d0
         pc(4) = pc(3) + pfs*0.5d0
         ifirst = 1
      endif
      
      w    = 1.d0
      do k = 1,n
         csi(1) = babayaganlo_rndm(0)
         i = 1
         do while(csi(1).gt.pc(i))
            i = i + 1
         enddo
         ep(k) = i
         w = w * 1.d0/(pc(i)-pc(i-1))
      enddo

      return
ccc old      
c      do k = 1,n
c         call ranlux(csi,1)
c         ep(k) = 1.d0*ncharged*csi(1) + 1
c         if (ep(k).gt.ncharged) ep(k) = ncharged ! to avoid round-offs...
c      enddo
c      w = ncharged**n
c      return
      end 
*-----------------------------------------------------
      subroutine scanvpols
      implicit double precision (a-h,o-z)
      parameter (n=100000)
      double complex vpolc,result
      external vpolc
      common/resonances/ires
      common/jpsiparameters/amjpsi,gtjpsi,gejpsi,effcjpsi
      common/jpsiparameters2/am2s,gt2s,ge2s,coup2s ! from vpolc
      
      Emin = 0.1d0
      Emax = 5.5d0

c      Emin = (amjpsi - 10.d0*gtjpsi)*.5d0
c      Emax = (amjpsi + 100.d0*gtjpsi)*.5d0
c      Emin = 0.d0
c      Emax = 4.43*.5d0
c      Emin = -Emax

      Emin = (amjpsi - 50.d0*gtjpsi)*.5d0
      Emax = (amjpsi + 50.d0*gtjpsi)*.5d0

      emin = 1.5d0
      emax = 1.55d0
      
      d = Emax - Emin
      d = d / n
      E = Emin
c      if (ires.eq.1) open(33,file='vpolnsk',status='unknown')
c      if (ires.eq.0) open(33,file='vpolres0',status='unknown')
      open(33,file='arunres2',status='unknown')
      do k = 0,n
         s = 4.d0*E*E
         if (E.lt.0.d0) s = -s
         result = vpolc(s)
         write(33,*)2.d0*E,Real(result),Imag(result)
         E = E + d
      enddo
      close(33)
      print*,'STOP IN SCANVPOLS'
      stop
      
      return
      end
***************************************************
      subroutine get_shat(q2,smin,smax,s,w)
      implicit double precision (a-h,o-z)
      real*4 csi(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

c      parameter (npb = 1500,npJpsi=5000,npa=1500)
c      parameter (np = npb + npJpsi + npa)
c      dimension cik(0:np),tgk(0:np),sk(0:np),fk(0:np),cikcumul(0:np)
c      dimension ds(0:np)
c      common/linearcurve/ds,totint,sq2max,cik,cikcumul,tgk,sk,fk,ifirst
      data ifirst /0/

      common/ialpharunning/iarun      

      common/jpsiparameters/am,gt,ge,coup ! from vpolc
      common/jpsiparameters2/am2s,gt2s,ge2s,coup2s ! from vpolc
      common/jpsiparameters3/am3s,gt3s,ge3s,coup3s ! from vpolc
      common/jpsiparameters4/am4s,gt4s,ge4s,coup4s ! from vpolc
      common/jpsiparameters5/am5s,gt5s,ge5s,coup5s ! from vpolc

      common/samplintervals/eminJpsi,emaxJpsi,sminJpsi,smaxJpsi      
      common/forbwsampling/ag,am2,ag2,amag,amsag,am2ag2            
      common/forbwsampling2s/ag2s,am22s,ag22s,amag2s,amsag2s,am2ag22s            
      common/forbwsampling3s/ag3s,am23s,ag23s,amag3s,amsag3s,am2ag23s            
      common/forbwsampling4s/ag4s,am24s,ag24s,amag4s,amsag4s,am2ag24s            
      common/forbwsampling5s/ag5s,am25s,ag25s,amag5s,amsag5s,am2ag25s            

      double complex vpolc,cdummy
      external vpolc
      
**** from babayaga3.5, Chinese version (11/2014)      
      if (ifirst.eq.0) then

         cdummy = vpolc(smax)   ! just to fill the common jpsiparameters!

         eminJpsi = am - 50d0*gt
         emaxJpsi = am + 30d0*gt
         sminJpsi = eminJpsi**2
         smaxJpsi = emaxJpsi**2

         ag     = gt
         am2    = am*am
         ag2    = ag*ag
         amag   = am*ag
         amsag  = am/ag
         am2ag2 = am2*ag2
         
         ag2s     = gt2s
         am22s    = am2s*am2s
         ag22s    = ag2s*ag2s
         amag2s   = am2s*ag2s
         amsag2s  = am2s/ag2s
         am2ag22s = am22s*ag22s

         ag3s     = gt3s
         am23s    = am3s*am3s
         ag23s    = ag3s*ag3s
         amag3s   = am3s*ag3s
         amsag3s  = am3s/ag3s
         am2ag23s = am23s*ag23s

         ag4s     = gt4s
         am24s    = am4s*am4s
         ag24s    = ag4s*ag4s
         amag4s   = am4s*ag4s
         amsag4s  = am4s/ag4s
         am2ag24s = am24s*ag24s

         ag5s     = gt5s
         am25s    = am5s*am5s
         ag25s    = ag5s*ag5s
         amag5s   = am5s*ag5s
         amsag5s  = am5s/ag5s
         am2ag25s = am25s*ag25s

c         uuu = spezzasdif(smax)
         
c         call scanvpols
c         print*,'STOPPING IN get_shat!'
c         stop
         
         ifirst = 1
      endif
***************** BEGIN TEST
c$$$c      goto 666
c$$$*     Trying IR + BW 
c$$$      an1  = log((q2-smin)/(q2-smax))
c$$$      an2 = log(smax/smin)
c$$$      anir= an1 + an2
c$$$
c$$$      anbw   = bwnorm(smax,smin)
c$$$      anbw2s = bwnorm2s(smax,smin)
c$$$      
c$$$      p1s = 0.9d0
c$$$      p2s = 1.d0 - p1s
c$$$      
c$$$      pir = 0.5d0
c$$$c      print*,'uncomment the following line again!'
c$$$      if (iarun.eq.0) pir = 1.d0
c$$$      pbw = 1.d0 - pir
c$$$
c$$$      call ranlux(csi,1)
c$$$      if (csi(1).lt.pir) then
c$$$         call ranlux(csi,1)
c$$$         if (csi(1).lt.an1/anir) then
c$$$            call ranlux(csi,1)
c$$$            s = q2-(q2-smin)*exp(-an1*csi(1))
c$$$         else
c$$$            call ranlux(csi,1)
c$$$            s = smin*exp(an2*csi(1))
c$$$         endif
c$$$      else
c$$$         call ranlux(csi,1)
c$$$         if (csi(1).lt.p1s) then
c$$$            call ranlux(csi,1)
c$$$            tmp   = atan(smin/amag-amsag)
c$$$            s     = amag*( tan(amag*anbw*csi(1)+tmp)+amsag )
c$$$         else
c$$$            call ranlux(csi,1)
c$$$            tmp   = atan(smin/amag2s-amsag2s)
c$$$            s     = amag2s*( tan(amag2s*anbw2s*csi(1)+tmp)+amsag2s )
c$$$         endif
c$$$      endif
c$$$
c$$$      w = pir/anir/(q2-s)/s*q2 +
c$$$     .     pbw*(  p1s/anbw/((s-am2)**2+am2ag2)
c$$$     .        +   p2s/anbw2s/((s-am22s)**2+am2ag22s) )
c$$$      w = 1.d0/w      
c$$$
c$$$*************************************************
c$$$      return
***************** END TEST
      
 666  continue
*     Trying IR + BW 
      an1  = log((q2-smin)/(q2-smax))
      anir = an1
      anbw   = bwnorm(smax,smin)
      anbw2s = bwnorm2s(smax,smin)
      anbw3s = bwnorm3s(smax,smin)
      anbw4s = bwnorm4s(smax,smin)
      anbw5s = bwnorm5s(smax,smin)
*******************      
      p1s = 0.35d0
      p2s = 0.1d0
      p3s = 0.35d0
      p4s = 0.1d0
      p5s = 1.d0 - p1s - p2s - p3s - p4s
*****************
      pir = 0.5d0
c      print*,'uncomment the following line again!'
      if (iarun.eq.0) pir = 1.d0
      pbw = 1.d0 - pir

      csi(1) = babayaganlo_rndm(0)
      if (csi(1).le.pir) then
         csi(1) = babayaganlo_rndm(0)
         s = q2-(q2-smin)*exp(-an1*csi(1))
      else
         csi(1) = babayaganlo_rndm(0)
         if (csi(1).le.p1s) then
            csi(1) = babayaganlo_rndm(0)
            tmp   = atan(smin/amag-amsag)
            s     = amag*( tan(amag*anbw*csi(1)+tmp)+amsag )
         elseif (csi(1).le.p1s+p2s) then
            csi(1) = babayaganlo_rndm(0)
            tmp   = atan(smin/amag2s-amsag2s)
            s     = amag2s*( tan(amag2s*anbw2s*csi(1)+tmp)+amsag2s )
         elseif (csi(1).le.p1s+p2s+p3s) then
            csi(1) = babayaganlo_rndm(0)
            tmp   = atan(smin/amag3s-amsag3s)
            s     = amag3s*( tan(amag3s*anbw3s*csi(1)+tmp)+amsag3s )
         elseif (csi(1).le.p1s+p2s+p3s+p4s) then
            csi(1) = babayaganlo_rndm(0)
            tmp   = atan(smin/amag4s-amsag4s)
            s     = amag4s*( tan(amag4s*anbw4s*csi(1)+tmp)+amsag4s )
         else!if (csi(1).le.p1s+p2s+p3s+p4s+p5s) then
            csi(1) = babayaganlo_rndm(0)
            tmp   = atan(smin/amag5s-amsag5s)
            s     = amag5s*( tan(amag5s*anbw5s*csi(1)+tmp)+amsag5s )
         endif
      endif

      w = pir/anir/(q2-s) +
     .     pbw*(  p1s/anbw  /((s-am2)**2   + am2ag2)
     .     +      p2s/anbw2s/((s-am22s)**2 + am2ag22s)
     .     +      p3s/anbw3s/((s-am23s)**2 + am2ag23s)
     .     +      p4s/anbw4s/((s-am24s)**2 + am2ag24s)
     .     +      p5s/anbw5s/((s-am25s)**2 + am2ag25s)
     .     )
      w = 1.d0/w

      return
      end
**************************************************
      function bwnorm(q2max,q2min)
      implicit double precision (a-h,o-z)
      common/bwnormcommon/uozmzw,zmszw,ifirst
      common/jpsiparameters/am,gt,ge,coup ! from vpolc
      common/jpsiparameters2/am2s,gt2s,ge2s,coup2s ! from vpolc
      common/jpsiparameters3/am3s,gt3s,ge3s,coup3s ! from vpolc
      common/jpsiparameters4/am4s,gt4s,ge4s,coup4s ! from vpolc
      common/jpsiparameters5/am5s,gt5s,ge5s,coup5s ! from vpolc

      data ifirst/0/
      if (ifirst.eq.0) then
         uozmzw = 1.d0/am/gt
         zmszw  = am/gt
         ifirst = 1
      endif
      bwnorm=uozmzw*(atan(q2max*uozmzw-zmszw)
     >        - atan( q2min*uozmzw -zmszw) )
      return
      end      
*********************************************
**************************************************
      function bwnorm2s(q2max,q2min)
      implicit double precision (a-h,o-z)
      common/bwnormcommon2s/uozmzw,zmszw,ifirst
      common/jpsiparameters/am,gt,ge,coup ! from vpolc
      common/jpsiparameters2/am2s,gt2s,ge2s,coup2s ! from vpolc
      common/jpsiparameters3/am3s,gt3s,ge3s,coup3s ! from vpolc
      common/jpsiparameters4/am4s,gt4s,ge4s,coup4s ! from vpolc
      common/jpsiparameters5/am5s,gt5s,ge5s,coup5s ! from vpolc

      data ifirst/0/
      if (ifirst.eq.0) then
         uozmzw = 1.d0/am2s/gt2s
         zmszw  = am2s/gt2s
         ifirst = 1
      endif
      bwnorm2s=uozmzw*(atan(q2max*uozmzw-zmszw)
     >        - atan( q2min*uozmzw -zmszw) )
      return
      end      
*********************************************
**************************************************
      function bwnorm3s(q2max,q2min)
      implicit double precision (a-h,o-z)
      common/bwnormcommon3s/uozmzw,zmszw,ifirst
      common/jpsiparameters/am,gt,ge,coup ! from vpolc
      common/jpsiparameters2/am2s,gt2s,ge2s,coup2s ! from vpolc
      common/jpsiparameters3/am3s,gt3s,ge3s,coup3s ! from vpolc
      common/jpsiparameters4/am4s,gt4s,ge4s,coup4s ! from vpolc
      common/jpsiparameters5/am5s,gt5s,ge5s,coup5s ! from vpolc

      data ifirst/0/
      if (ifirst.eq.0) then
         uozmzw = 1.d0/am3s/gt3s
         zmszw  = am3s/gt3s
         ifirst = 1
      endif
      bwnorm3s=uozmzw*(atan(q2max*uozmzw-zmszw)
     >        - atan( q2min*uozmzw -zmszw) )
      return
      end      
*********************************************
**************************************************
      function bwnorm4s(q2max,q2min)
      implicit double precision (a-h,o-z)
      common/bwnormcommon4s/uozmzw,zmszw,ifirst
      common/jpsiparameters/am,gt,ge,coup ! from vpolc
      common/jpsiparameters2/am2s,gt2s,ge2s,coup2s ! from vpolc
      common/jpsiparameters3/am3s,gt3s,ge3s,coup3s ! from vpolc
      common/jpsiparameters4/am4s,gt4s,ge4s,coup4s ! from vpolc
      common/jpsiparameters5/am5s,gt5s,ge5s,coup5s ! from vpolc

      data ifirst/0/
      if (ifirst.eq.0) then
         uozmzw = 1.d0/am4s/gt4s
         zmszw  = am4s/gt4s
         ifirst = 1
      endif
      bwnorm4s=uozmzw*(atan(q2max*uozmzw-zmszw)
     >        - atan( q2min*uozmzw -zmszw) )
      return
      end      
*********************************************
**************************************************
      function bwnorm5s(q2max,q2min)
      implicit double precision (a-h,o-z)
      common/bwnormcommon5s/uozmzw,zmszw,ifirst
      common/jpsiparameters/am,gt,ge,coup ! from vpolc
      common/jpsiparameters2/am2s,gt2s,ge2s,coup2s ! from vpolc
      common/jpsiparameters3/am3s,gt3s,ge3s,coup3s ! from vpolc
      common/jpsiparameters4/am4s,gt4s,ge4s,coup4s ! from vpolc
      common/jpsiparameters5/am5s,gt5s,ge5s,coup5s ! from vpolc

      data ifirst/0/
      if (ifirst.eq.0) then
         uozmzw = 1.d0/am5s/gt5s
         zmszw  = am5s/gt5s
         ifirst = 1
      endif
      bwnorm5s=uozmzw*(atan(q2max*uozmzw-zmszw)
     >        - atan( q2min*uozmzw -zmszw) )
      return
      end      
*********************************************
*********************************************
      function spezzasdif(s)
      IMPLICIT REAL*8 (A-H,O-Z)
      parameter (n=10000000)
c      parameter (n=1000)
      dimension ssd(0:n)!,sx(0:n)
      common/forshatsampling/den,omin,omax,ei

      common/cthcommon/cth
      character*2 fs
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi

      common/spezzasd/d,smax,smin,cx,ssd,ifirst
      data ifirst /0/

      if (ifirst.eq.0) then
c         open(321,file='spezzatasdif.txt',status='unknown')
         smax = ei*ei
         smin = ame*ame*4.d0
         if (fs.eq.'mm') smin = 4.d0*ammu*ammu
         smin = smin*(1.d0+1d-4) ! to avoid NaNs in the eval of sdif

         d = (smax - smin)/n

         print*,'evaluating the function...'
         print*,'npoints =',n
         print*,'smin,smax',smin,smax,' Gev^2'
         print*,'sqrt(smin,smax)',sqrt(smin),sqrt(smax),'GeV'
         print*,'delta = ',d,' GeV^2'

         sx = smin
         cx = 0.d0
         do k = 0,n
            sx = smin + k*d
            ssd(k) = sdif(sx,cx)
c            write(321,*)sx,ssd(k)
         enddo
         
         ifirst = 1
         print*,'...done!'
c         close(321)
      endif

      i = (s - smin)/d
      s0 = smin + i*d
      spezzasdif = ssd(i)+ (ssd(i+1)-ssd(i))/d*(s-s0)
      
      return
      end
*-----------------------------------------------------
*   DIFFERENTIAL CROSS SECTION
*
       FUNCTION SDIF(S,C)
       IMPLICIT REAL*8 (A-H,O-Z)
       REAL*8 AM(14),GT(14),GE(14)
       common/parameters/ame,ammu,convfac,alpha,pi     

       COMMON/CHANNEL/ICH
       COMMON/PIINVMASS/PIQ2
       common/resonances/ires
       double complex vpolc,vpols,vpolt
       external vpolc
*     
* AM=HADRON MASSES, GT=TOTAL WIDTH, GE=PARTIAL E+E- WIDTH.
*
* 1=RHO, 2=OMEGA, 3=PHI, 4=J/PSI, 5=PSI(2S), 6=PSI(3770), 7=PSI(4040),
* 8=PSI(4160), 9=PSI(4415), 10=Y, 11=Y(2S), 12=Y(4S), 13=Y(10860),
* 14=Y(11020).
*
       DATA AM/0.770D0,0.782D0,1.019413D0,4.09688D0,3.686D0,3.7699D0,
     >         4.040D0,4.159D0,4.415D0,9.46037D0,10.02330D0,10.5800D0,
     >         10.865D0,11.019D0/
       DATA GT/0.1507D0,8.41D-3,4.43D-3,87.D-6,277.D-6,23.6D-3,
     >         52.D-3,78.D-3,43.D-3,52.5D-6,44.D-6,10.D-3,
     >         110.D-3,79.D-3/
       DATA GE/6.77D-6,0.60D-6,1.32457D-6,5.26D-6,2.14D-6,0.26D-6,
     >         0.75D-6,0.77D-6,0.47D-6,1.32D-6,0.520D-6,0.248D-6,
     >         0.31D-6,0.130D-6/

       common/ialpharunning/iarun

       print*,'TO BE UPDATED WITH THE NEW ONE WHICH USED VPOLC!!'
       
* QED DIFFERENTIAL CROSS SECTIONS (WITH OR WITHOUT RUNNING ALPHA):
*
       ICH = 1
       IRES = 0


      IF (ICH.EQ.3) then 
         SQED = 0.5D0*2.D0*PI*ALPHA**2*
     >        (1.D0+C**2)/(1.D0-C**2)/S
         SDIF = SQED
         return
      endif
*
      VPOLS = VPOLC(S)
*     
* DIFFERENTIAL CROSS SECTIONS SUM 
*
      COS=PI*ALPHA*ALPHA/(2.D0*S)*abs(VPOLS)**2
      SDS=COS*(1.D0+C**2)

c      print*,s,Real(vpols),Imag(vpols)
      
      IF (ICH.EQ.1) THEN
         T=-(1.D0-C)*S/2.D0
         VPOLT = VPOLC(T)
         COT=PI*(ALPHA*VPOLT)**2/S
         SDT=COT*((1.D0+C)**2+4.D0)/((1.D0-C)**2) 
         COST=PI*ALPHA**2*VPOLS*VPOLT/S
         SDST=-COST*(1.D0+C)**2/(1.D0-C)
         SQED = SDS+SDT+SDST
      ENDIF
      IF (ICH.EQ.2) THEN
         BET=SQRT(1.D0-4.D0*AMMU**2/S)
         SQED=COS*BET*(1.D0+C**2+
     >        4.D0*AMMU**2/S*(1.D0-C**2))

         if (SQED.lt.0.d0) then
            print*,'IN SDIF',SQED,COS,BET
         endif
      ENDIF
*      
      SDIF = SQED
*
      RETURN
      END
********************************************************
      subroutine photon_energy_new(ibich,pflat,expgg,omax,omin,
     .     ptld,v,om,w)
! written by CMCC, last modified 20/9/2007
      implicit double precision (a-h,o-z)
      dimension ptld(0:3),v(0:3)
      real*4 rnd,csi(1),csib(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

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
         csi(1) = babayaganlo_rndm(0)
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
      subroutine ap_vertexNEW(eps,x,omx)
      implicit double precision (a-h,o-z)
! written by CMCC, essentially from old BABAYAGA
*  x generation according to ap splitting function
*  (1+x^2)/(1-x), 0 <= x <= 1-eps
* added 15/10/2005, modified from babayaga, older releases by CMCC
      double precision eps
      real*4 r(2)
      alne = log(eps)
      irigenera = 1
      do while(irigenera.eq.1)
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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      character*2 fs
      common/finalstate/fs
      common/coseni/cmax,cmin
      common/iwide/iwider,ntot  ! ntot is the number of photons
      common/enableifirst/ifrstenbld

      double precision anorm, fmax
      integer ifirst
      data ifirst /0/

      save anorm,fmax,ifirst

c$$$      if (ifirst.eq.0) then
c$$$         ifirst = 1
c$$$         print*,'FLAT COS in get_cos_fer!!!'
c$$$      endif
c$$$      call ranlux(csi,1)      
c$$$      c = (cmax - cmin) * csi(1) + cmin
c$$$      w = cmax - cmin
c$$$      return

      if (fs.eq.'mm') then
         if (ifirst.eq.0.or.ifrstenbld.eq.0) then
            fmax = 1.d0+max(cmax**2,cmin**2)
            anorm = (cmax-cmin) + 1.d0/3.d0*(cmax**3-cmin**3)
         endif
         iagain = 1

         do while(iagain.eq.1)
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
     .        + cmax**2 + cmax**3/3.d0 + 16.d0*log(1.d0 - cmax)

         anorm = anorm +31.d0/3.d0 + 16.d0/(cmin - 1.d0) - 9.d0*cmin
     .        - cmin**2 - cmin**3/3.d0 - 16.d0*log(1.d0 - cmin)
         anorm = anorm/2.d0
         fmax = fcos(cmax)
         ifirst = 1
      endif

      istop = 0

      do while(istop.eq.0)

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
      subroutine get_cos_ferNEW(c,w)
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1),r(2)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      character*2 fs
      common/finalstate/fs
      common/coseni/cmax,cmin
      common/iwide/iwider,ntot  ! ntot is the number of photons
      common/enableifirst/ifrstenbld

      double precision anorm, fmax
      integer ifirst
      data ifirst /0/

      parameter (np = 1000)
      dimension cik(0:np),tgk(0:np),sk(0:np),fk(0:np),cikcumul(0:np)
      common/linearcurvecos/ds,totint,sq2max,cik,
     .     cikcumul,tgk,sk,fk,ifirst

      if (fs.eq.'mm') then

         print*,'get_cos_ferNEW - not ready for mm!! stopping...'
         stop

         if (ifirst.eq.0.or.ifrstenbld.eq.0) then
            fmax = 1.d0+max(cmax**2,cmin**2)
            anorm = (cmax-cmin) + 1.d0/3.d0*(cmax**3-cmin**3)
         endif
         iagain = 1

         do while(iagain.eq.1)
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
      
      if (ifirst.eq.0) then
         smin = -1.d0
         smax =  1.d0
         ds = (smax-smin)/np
         sk(0) = smin
         do k = 1,np
            sk(k) = sk(k-1) + ds
         enddo
!     that's ok        
c         ramp  = 0.001d0
c         upr   = 1.d0 + ramp
c         cmins = cmin + (-1.d0-cmin)*0.d0
c         cmaxs = cmax + ( 1.d0-cmax)*0.d0
c         do k = 0,np
c            if (sk(k).le.cmins) then
c               fk(k) = fcos(cmins)*(1.d0+ramp*sk(k)/cmins)
c            elseif (sk(k).ge.cmaxs) then
c               fk(k) = fcos(cmaxs)*upr*(1.d0+ramp*sk(k)/cmaxs)
c            else
c               fk(k) = fcos(sk(k))*upr
c            endif
c         enddo
! trying better one
         floor = 0.d0 !fcos(cmin)*0.5d0
         do k = 0,np
            if (sk(k).le.cmin) then
               fk(k) = (sk(k)+1.d0) * fcos(cmin)/(cmin+1.d0)+floor
            elseif (sk(k).ge.cmax) then
               fk(k) = fcos(cmax)*(1.d0-sk(k))/(1.d0-cmax)+floor
            else
               fk(k) = fcos(sk(k))+floor
            endif
         enddo
        
         do k = 0,np-1
            tgk(k) = (fk(k+1)-fk(k))/ds
         enddo
         tgk(np) = 0.d0
         cik(0) = 0.d0
         do k = 1,np
            cik(k)=ds*(fk(k-1)+0.5d0*tgk(k-1)*ds)
         enddo
         
         cikcumul(0) = 0.d0
         do k = 1,np
            cikcumul(k) = cikcumul(k-1) + cik(k)
         enddo
         totint = cikcumul(np)
                  
         ifirst = 1
      endif

      csi(1) = babayaganlo_rndm(0)
      
      csiN = csi(1)*totint
      k = 0
      do while(csiN.gt.cikcumul(k))
         k = k + 1
      enddo
      k = k -1
      cumul = cikcumul(k)

      a = tgk(k)
      b = fk(k)
      c = 2.d0*(cumul-csiN)
      if (a.ne.0.d0) then
         xt = (-b + sqrt(b*b-a*c))/a ! only this solution is physical
      else
         xt = -0.5d0*c / b
      endif
      
      c = xt + sk(k)
      
      fs1 = fk(k) + tgk(k)*(c-sk(k))      
      w = totint/fs1

      return
      end
**************************************************************************

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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

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
         csi(1) = babayaganlo_rndm(0)
         rnd = csi(1)
      else
         rnd = 0.5
      endif
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
c*******************************************************************
      subroutine get_cos_2g_born(c,w)
! written by CMCC, last modified 18/10/2005
      implicit double precision (a-h,o-z)
      real*4 rnd,csi(1)
      double precision babayaganlo_rndm
      external babayaganlo_rndm

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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      character*2 fs
      character*1 s
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      data ifirst /0/
      save ifirst,b1e,ane,b1m,anm

      pflatl = pflat !! error??

      if (ifirst.eq.0) then
         ifirst = 1
         b1e   = dsqrt((1.d0 - ame/p(0))*(1.d0 + ame/p(0)))
         ane   = 1.d0/b1e*dlog((1.d0+b1e)/(1.d0-b1e))
         b1m   = dsqrt((1.d0 - ammu/p(0))*(1.d0 + ammu/p(0)))
         anm   = 1.d0/b1m*dlog((1.d0+b1m)/(1.d0-b1m))
      endif

      psing = 1.d0 - pflatl
      if (pflatl.gt.0.d0) then
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

*****
      subroutine collinear_simple(pflat,rnd1,rnd2,p,vers,w)
! written by CMCC, last modified 16/10/2005
      implicit double precision (a-h,o-z)
      dimension p(0:3),vers(0:3)
      real*4 csi(1),r(2),rnd1,rnd2
      double precision babayaganlo_rndm
      external babayaganlo_rndm

      character*2 fs
      character*1 s
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi

      pflatl = pflat !! error??

      b1e   = sqrt((1.d0 - ame/p(0))*(1.d0 + ame/p(0)))
      ane   = 1.d0/b1e*log((1.d0+b1e)/(1.d0-b1e))
      b1m   = sqrt((1.d0 - ammu/p(0))*(1.d0 + ammu/p(0)))
      anm   = 1.d0/b1m*log((1.d0+b1m)/(1.d0-b1m))

      psing = 1.d0 - pflatl
      if (pflatl.gt.0.d0) then
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
      double precision babayaganlo_rndm
      external babayaganlo_rndm

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
      
      csi(1) = babayaganlo_rndm(0)

      if (csi(1).lt.(1.d0-pbw)) then
         csi(1) = babayaganlo_rndm(0)
         om = (omax - omin)*csi(1)*1.d0 + omin
         x  = om/omax
         y  = 4.d0*omax*(omax - om)
      else
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
