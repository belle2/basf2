************************************************************************************
      subroutine phasespace(p1,p2,p3,p4,qph,nphot,m1,m2,esoft,
     .     cth,w,phsp,ie)
! written by CMCC, last modified 19/10/2006
      implicit double precision (a-h,m,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3)
      dimension ap3(0:3),ap4(0:3),pcollorig(0:3)
      dimension p4guess(0:3),p3guess(0:3)
      dimension dir3(0:3),dir4(0:3),ptmp(0:3),vers(0:3)
      double precision kone(0:3),pcoll(0:3),ptld(0:3)
      double precision lambda
      logical lcond
      character*2 fs
      common/finalstate/fs
      integer emissionpattern(nphot)
      parameter (pi = 3.1415926535897932384626433832795029d0)
      parameter (imaxph = 40)
      double precision qph(imaxph,0:3),oph(imaxph),wten(nphot)
      real*4 csi(1),csi2(2),rnddummy
      common/for_debug/pcoll,denphsp,dir3,dir4,br,bq,onm
      common/various/beta,betafs
      common/channelref/iref
      common/iwide/iwider,ntot
      common/regulators/ich,idummy,regulator1,regulator2,wcoll3,wcoll4
      common/radpattern/nph(4)
      common/idebugging/idebug
      common/idarkon/idarkon
      double precision babayaganlo_rndm
      external babayaganlo_rndm
      
      w      = 1.d0
      phsp   = 1.d0
      npart = nphot + 2
      nvar  = 3*nphot + 2

      ie     = 0

      duepigrechi = (2.d0*pi)**(-nvar)

      iwider = 0
      ntot   = nphot

      do j = 0,3
         do k = 1,imaxph
            qph(k,j) = 0.d0
         enddo
         nph(j+1) = 0
      enddo

      if (p1(0)+p2(0).lt.(m1+m2+nphot*esoft)) then 
         phsp = 0.d0
         w    = 0.d0
         ie = 1
         return
      endif
*****************
!! I put this outside in order to raise efficiency
!      call get_cos_fer(rnd(1),c,wcos)
!      cth = c
!      w   = w * wcos
      c = cth
*
      if (nphot.eq.0) then
         mass = p1(0)+p2(0)
         sqla = dsqrt(lambda(mass**2,m1**2,m2**2))
         phsp = phsp * sqla/8.d0/mass**2
         s = dsqrt(1.d0 - c**2)
         csi(1) = babayaganlo_rndm(0)
         call getphi(csi(1),phi,wph)
         w = w*wph
         sphi = sin(phi)
         cphi = cos(phi)
         p1mod = sqla/2.d0/mass
         p3(0) = dsqrt(p1mod**2 + m1**2)
         p3(1) = p1mod * sphi * s
         p3(2) = p1mod * cphi * s
         p3(3) = p1mod * c
         p4(0) = mass - p3(0)
         p4(1) = - p3(1)
         p4(2) = - p3(2)
         p4(3) = - p3(3)
         phsp = phsp * duepigrechi
         return
      endif

*******************
******************
      lcond = fs.eq.'ee'.or.fs.eq.'mm'
      
      angle = 30.d0 *pi/180.d0

      angle = 0.d0 *pi/180.d0

      cpmax = cos(angle)
      cpmin = cos(pi - angle)
* the following for ng >= 1
      ncharged = 4
      if (fs.eq.'gg') ncharged = 2
      ir = 1

! cos(th) of electron already generated...
      sth  = dsqrt(1.d0 - cth**2)
      csi(1) = babayaganlo_rndm(0)
      call getphi(csi(1),phi,wph)
      w    = w*wph
      sphi = dsin(phi)
      cphi = dcos(phi)
      ir   = ir + 2
      call get_pattern(ncharged,nphot,emissionpattern)

      do k = 1,nphot
         nph(emissionpattern(k)) = nph(emissionpattern(k)) + 1
      enddo
      
      n1 = nph(1)
      n2 = nph(2)
      n3 = nph(3)
      n4 = nph(4)
      nis = n1 + n2
      nfs = n3 + n4

      ei = p1(0)+p2(0)

      dir3(0) = ei/2d0
      dir3(1) = ei/2d0*betafs*cphi*sth
      dir3(2) = ei/2d0*betafs*sphi*sth
      dir3(3) = ei/2d0*betafs*cth         
      dir4(0) = dir3(0)
      pcoll(0) = p1(0)+p2(0)
      ptld(0) = p1(0)+p2(0)
      vers(0)  = 0.d0
      do k = 1,3
         pcoll(k) = p1(k)+p2(k)
         ptld(k)  = p1(k)+p2(k)
         dir4(k)  = -dir3(k)
         vers(k)  = 0.d0
      enddo               

      csi(1) = babayaganlo_rndm(0)
      iref = 3
      if (csi(1).gt.0.5) iref = 4
      wref = 1.d0
      if (nfs.gt.0) then
         if (n4.eq.0) iref = 3
         if (n3.eq.0) iref = 4
      endif
      w = w*wref
      if (fs.eq.'gg') iref = 3
****
** PHOTONS GENERATION...
      pflatc  = 0.d0
      pflatc3 = 0.d0
      pflatc4 = 0.d0
      pflate  = 0.d0
      expgg = 0.92d0
      ibich = 1
      do k = 1,nphot
         wen = 1.d0
         en  = esoft
         if (fs.eq.'gg') then
            call photon_energy(ibich,pflate,expgg,p1(0),
     .           en,wen)
         endif
         wten(k) = wen
         if (fs.eq.'ee'.or.fs.eq.'mm') w = w * wen
         oph(k) = en
         ir = ir + 1
      enddo
** initial state photons
c  si vede su quale gamba ci sono dei fotoni e se ne calcola la
c  direzione con "collinear", poi se ne calcola l'energia, secondo l'AP.
c  con "photon_energy", "photon_energy_dm" calcola l'energia anche con la BW
c  Poi si toglie l'energia del fotone da quella dell'elettrone.

      emaxgamma = (p1(0)+p2(0))/2.d0
      if (nis.gt.0) then
         if (n1.gt.0) then
            do k = 1,n1
               iaccendi = 1
               csi2(1) = babayaganlo_rndm(0)
               csi2(2) = babayaganlo_rndm(0)
               call collinear(0.d0,csi2(1),csi2(2),p1,vers,wverg)
               iaccendi = 0
               ir = ir + 2
               w = w * wverg
****
               if (lcond) then
                  if (idarkon.eq.1) then
                      call photon_energy_dm(emaxgamma,en,wen)
                  else
                      cp = vers(3)/vers(0)
                      if (cp.gt.cpmax.or.cp.lt.cpmin) then
                         call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                      else
                        call photon_energy(ibich,pflate,expgg,
     .                                     emaxgamma,en,wen)
                      endif
                  endif
                  w = w * wen
                  oph(k) = en
               endif
***
               qph(k,0) = oph(k)
               qph(k,1) = oph(k)*vers(1)
               qph(k,2) = oph(k)*vers(2)
               qph(k,3) = oph(k)*vers(3)

               pcoll(0) = pcoll(0) - qph(k,0)
               pcoll(1) = pcoll(1) - qph(k,1)
               pcoll(2) = pcoll(2) - qph(k,2)
               pcoll(3) = pcoll(3) - qph(k,3)
            enddo
         endif
         if (n2.gt.0) then
            do k = 1,n2
               iaccendi = 2
               csi2(1) = babayaganlo_rndm(0)
               csi2(2) = babayaganlo_rndm(0)
               call collinear(0.d0,csi2(1),csi2(2),p2,vers,wverg)
               iaccendi  = 0
               ir = ir + 2
               w = w * wverg
****
               if (lcond) then
                  if (idarkon.eq.1) then
                      call photon_energy_dm(emaxgamma,en,wen)
                  else
                      cp = vers(3)/vers(0)
                      if (cp.gt.cpmax.or.cp.lt.cpmin) then
                         call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                      else
                        call photon_energy(ibich,pflate,expgg,
     .                                     emaxgamma,en,wen)
                      endif
                  endif
                  w = w * wen
                  oph(k+n1) = en
               endif
***
               qph(k+n1,0) = oph(k+n1)
               qph(k+n1,1) = oph(k+n1)*vers(1)
               qph(k+n1,2) = oph(k+n1)*vers(2)
               qph(k+n1,3) = oph(k+n1)*vers(3)

               pcoll(0) = pcoll(0) - qph(k+n1,0)
               pcoll(1) = pcoll(1) - qph(k+n1,1)
               pcoll(2) = pcoll(2) - qph(k+n1,2)
               pcoll(3) = pcoll(3) - qph(k+n1,3)
            enddo
         endif
      endif

      if (lcond.and.
     .   (dot(pcoll,pcoll).lt.(m1+m2)**2.or.pcoll(0).lt.0.d0)) then
         ie = 1
         phsp = 0.d0
         w  = 0.d0
         return
      endif

** final state photons
c      en3guess = dir3(0)
c      en4guess = dir4(0)
      wcoll3 = 1.d0
      wcoll4 = 1.d0
      if (nfs.gt.0) then
         if (iref.eq.3) then
            if (n3.gt.0) then
               do k = 1,n3
                  iaccendi = 3
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear(pflatc3,csi2(1),csi2(2),dir3,vers,
     .                 wverg)
                  iaccendi = 0
                  ir = ir + 2
                  w = w * wverg
****
                  if (lcond) then
                    if (idarkon.eq.1) then
                       call photon_energy_dm(emaxgamma,en,wen)
                    else
                       cp = vers(3)/vers(0)
                       if (cp.gt.cpmax.or.cp.lt.cpmin) then
                            call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                       else
                            call photon_energy(ibich,pflate,expgg,
     .                                         emaxgamma,en,wen)
                       endif
                     endif
                     w = w * wen
                     oph(k+n1+n2) = en
                  endif
***
                  wcoll3 = wcoll3 * wverg
                  qph(k+n1+n2,0) = oph(k+n1+n2)
                  qph(k+n1+n2,1) = oph(k+n1+n2)*vers(1)
                  qph(k+n1+n2,2) = oph(k+n1+n2)*vers(2)
                  qph(k+n1+n2,3) = oph(k+n1+n2)*vers(3)

                  pcoll(0) = pcoll(0) - qph(k+n1+n2,0)
                  pcoll(1) = pcoll(1) - qph(k+n1+n2,1)
                  pcoll(2) = pcoll(2) - qph(k+n1+n2,2)
                  pcoll(3) = pcoll(3) - qph(k+n1+n2,3)
c                  en3guess = en3guess - oph(k+n1+n2)
               enddo
            endif
            if (dot(pcoll,pcoll).lt.(m1+m2)**2.or.pcoll(0).lt.0.d0) then
               ie = 1
               phsp = 0.d0
               w  = 0.d0
               return
            endif

            do k = 0,3
               pcoll(k) = pcoll(k) - dir3(k)
               pcollorig(k) = pcoll(k)
            enddo

            pcmodmu = 1.d0/sqrt(tridot(pcoll,pcoll))            
            pcoll(0) = dir4(0)
            bcoll    = sqrt(1.d0 - m1**2/pcoll(0)**2)
            pcoll(1) = pcoll(1)*pcmodmu*bcoll*pcoll(0)
            pcoll(2) = pcoll(2)*pcmodmu*bcoll*pcoll(0)
            pcoll(3) = pcoll(3)*pcmodmu*bcoll*pcoll(0)

            if (n4.gt.0) then
               do k = 1,n4
                  iaccendi = 4
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear(pflatc4,csi2(1),csi2(2),pcoll,vers,
     .                 wverg)
                  iaccendi = 0
                  ir = ir + 2
                  w = w * wverg
****
                  if (lcond) then
                    if (idarkon.eq.1) then
                       call photon_energy_dm(emaxgamma,en,wen)
                    else
                       cp = vers(3)/vers(0)
                       if (cp.gt.cpmax.or.cp.lt.cpmin) then
                            call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                       else
                            call photon_energy(ibich,pflate,expgg,
     .                                         emaxgamma,en,wen)
                       endif
                     endif
                     w = w * wen
                     oph(k+n1+n2+n3) = en
                  endif
***
                  wcoll4 = wcoll4 * wverg
                  qph(k+n1+n2+n3,0) = oph(k+n1+n2+n3)
                  qph(k+n1+n2+n3,1) = oph(k+n1+n2+n3)*vers(1)
                  qph(k+n1+n2+n3,2) = oph(k+n1+n2+n3)*vers(2)
                  qph(k+n1+n2+n3,3) = oph(k+n1+n2+n3)*vers(3)
               enddo
            endif
         else                   ! if iref = 4
            if (n4.gt.0) then
               do k = 1,n4
                  iaccendi = 4
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear(pflatc4,csi2(1),csi2(2),dir4,vers,
     .                 wverg)
                  iaccendi = 0
                  ir = ir + 2
                  w = w * wverg
****
                  if (lcond) then
                    if (idarkon.eq.1) then
                       call photon_energy_dm(emaxgamma,en,wen)
                    else
                       cp = vers(3)/vers(0)
                       if (cp.gt.cpmax.or.cp.lt.cpmin) then
                            call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                       else
                            call photon_energy(ibich,pflate,expgg,
     .                                         emaxgamma,en,wen)
                       endif
                     endif
                     w = w * wen
                     oph(k+n1+n2+n3) = en
                  endif
***   
                  wcoll4 = wcoll4 * wverg
                  qph(k+n1+n2+n3,0) = oph(k+n1+n2+n3)
                  qph(k+n1+n2+n3,1) = oph(k+n1+n2+n3)*vers(1)
                  qph(k+n1+n2+n3,2) = oph(k+n1+n2+n3)*vers(2)
                  qph(k+n1+n2+n3,3) = oph(k+n1+n2+n3)*vers(3)

                  pcoll(0) = pcoll(0) - qph(k+n1+n2+n3,0)
                  pcoll(1) = pcoll(1) - qph(k+n1+n2+n3,1)
                  pcoll(2) = pcoll(2) - qph(k+n1+n2+n3,2)
                  pcoll(3) = pcoll(3) - qph(k+n1+n2+n3,3)
c                  en4guess = en4guess - oph(k+n1+n2+n3)
               enddo
            endif
            if (dot(pcoll,pcoll).lt.(m1+m2)**2.or.pcoll(0).lt.0.d0) then
               ie = 1
               phsp = 0.d0
               w  = 0.d0
               return
            endif

            do k = 0,3
               pcoll(k) = pcoll(k) - dir4(k)
               pcollorig(k) = pcoll(k)
            enddo

            pcmodmu = 1.d0/sqrt(tridot(pcoll,pcoll))            
            pcoll(0) = dir3(0)
            bcoll    = sqrt(1.d0 - m2**2/pcoll(0)**2)
            pcoll(1) = pcoll(1)*pcmodmu*bcoll*pcoll(0)
            pcoll(2) = pcoll(2)*pcmodmu*bcoll*pcoll(0)
            pcoll(3) = pcoll(3)*pcmodmu*bcoll*pcoll(0)

            if (n3.gt.0) then
               do k = 1,n3
                  iaccendi = 3
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear(pflatc3,csi2(1),csi2(2),pcoll,vers,
     .                 wverg)
                  iaccendi = 0
                  ir = ir + 2
                  w = w * wverg
****
                  if (lcond) then
                    if (idarkon.eq.1) then
                       call photon_energy_dm(emaxgamma,en,wen)
                    else
                       cp = vers(3)/vers(0)
                       if (cp.gt.cpmax.or.cp.lt.cpmin) then
                            call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                       else
                            call photon_energy(ibich,pflate,expgg,
     .                                         emaxgamma,en,wen)
                       endif
                     endif
                     w = w * wen
                     oph(k+n1+n2) = en
                  endif
*** 
                  wcoll3 = wcoll3 * wverg
                  qph(k+n1+n2,0) = oph(k+n1+n2)
                  qph(k+n1+n2,1) = oph(k+n1+n2)*vers(1)
                  qph(k+n1+n2,2) = oph(k+n1+n2)*vers(2)
                  qph(k+n1+n2,3) = oph(k+n1+n2)*vers(3)
               enddo               
            endif            
         endif
      endif
***********************************
      do j = 0,3
         kone(j) = 0.d0
         do k = 1,nphot
            kone(j) = kone(j) + qph(k,j)
         enddo
         ptmp(j) = p1(j) + p2(j) - kone(j)
      enddo
***** TEST
      if (fs.eq.'gg') then
         wwen = 1.d0
         if (nphot.gt.1) then
            do k = 1,nphot-1
               wwen = wwen * wten(k)
            enddo
         endif
         w = w * wwen
         do j = 0,3
            kone(j) = kone(j)-qph(nphot,j)
         enddo

         pk2 = dot(kone,kone)
         br  = kone(0) - pk2/2.d0/ei
         bq  = kone(0) - tridot(kone,vers)
         onm = ei/2.d0 - br
         onm = onm/(1.d0 - bq/ei)
         a  = ei/2.d0 - kone(0) + pk2/2.d0/ei
         b  = 1.d0  - dot(kone,vers)/ei
         c  = ei - kone(0) + tridot(kone,dir3)/beta/dir3(0)
         d  = 1.d0 - tridot(vers,dir3)/beta/dir3(0)
         ep = (ei*a - esoft*c)/(ei*b-esoft*d)
         if (ep.lt.esoft.or.ep.ge.a/b.or.ep.eq.p1(0)) then
            w = 0.d0
            phsp = 0.d0
            ie = 1
            return
         endif
         call photonenergygg(ep,a,b,esoft,en,wen)
         w = w * wen
         qph(nphot,0) = en
         qph(nphot,1) = en*vers(1)
         qph(nphot,2) = en*vers(2)
         qph(nphot,3) = en*vers(3)
         do j = 0,3
            kone(j) = kone(j)+qph(nphot,j)
            ptmp(j) = p1(j) + p2(j) - kone(j)
         enddo
      endif
***** TEST
      if (dot(ptmp,ptmp).lt.(m1+m2)**2.or.ptmp(0).lt.0.d0) then
         ie = 1
         phsp = 0.d0
         w  = 0.d0
         return
      endif
      amkone2 = dot(kone,kone)
***
      if (iref.eq.3) then
         vx  = sth*cphi
         vy  = sth*sphi
         vz  = cth
      else
         vx  = -sth*cphi
         vy  = -sth*sphi
         vz  = -cth
      endif
      o   = kone(0)
      ox  = kone(1)
      oy  = kone(2)
      oz  = kone(3)
      odv = ox*vx + oy*vy + oz*vz
      abig = ei**2 - 2.d0 * ei * o + amkone2 + m1**2 - m2**2
      bbig = -2.d0 * (ei - o)
      cbig = -2.d0 * odv
      arg  = abig**2*bbig**2-m1**2*bbig**2*(bbig**2-cbig**2)
      if (arg.lt.0.d0) then
         phsp = 0.d0
         w    = 0.d0
         ie = 2
         return
      endif
      p1mod_1 = abig*cbig + dsqrt(arg)
      p1mod_1 = p1mod_1 / (bbig**2 - cbig**2)
      p1mod_2 = abig*cbig - dsqrt(arg)
      p1mod_2 = p1mod_2 / (bbig**2 - cbig**2)
      p1jac = 1.d0
      if (p1mod_1.gt.0.d0.and.p1mod_2.gt.0.d0) then
         csi(1) = babayaganlo_rndm(0)
         if (csi(1).lt.0.5) p1mod = p1mod_1
         if (csi(1).ge.0.5) p1mod = p1mod_2
         p1jac = 2.d0
      endif
      if (p1mod_1.gt.0.d0.and.p1mod_2.le.0.d0) then
         p1mod = p1mod_1
      endif
      if (p1mod_1.le.0.d0.and.p1mod_2.gt.0.d0) then
         p1mod = p1mod_2
      endif
      if (p1mod_1.le.0.d0.and.p1mod_2.le.0.d0) then ! bestiale: con gg ci vuole .le.!!
         phsp = 0.d0
         w    = 0.d0
         ie   = 3
         return
      endif
      w = w * p1jac
      p3(0) = dsqrt(p1mod**2 + m1**2)
      p3(1) = p1mod * vx
      p3(2) = p1mod * vy
      p3(3) = p1mod * vz
      p4(0) =  ei - p3(0) - o
      p4(1) = -ox - p3(1)
      p4(2) = -oy - p3(2)
      p4(3) = -oz - p3(3)
***
      if (fs.eq.'gg'.and.(p3(0).lt.esoft.or.p4(0).lt.esoft)) then
         phsp = 0.d0
         w    = 0.d0
         ie = 1
         return
      endif
***
!      phasespace!
      prodomega = 1.d0
      do k = 1,nphot
         prodomega = prodomega * qph(k,0)
      enddo
      den = dabs( p4(0)*p1mod + p3(0)*(p1mod + odv) )
      denphsp = den
      phsp = prodomega * p1mod**2 / denphsp
      phsp = phsp*duepigrechi/2.d0**(nphot+2)
      if (iref.eq.4) call exchange_mom(p3,p4)

***********************      
! "regulator"... I try only on FINAL STATE (active only for ee and mm...)
      flatconst = 0.d0 !!!
      den       = 0.d0
      if (n3.gt.0.and.n4.gt.0) then
         prodp3 = 1.d0
         prodp4 = 1.d0         
         do k = nis+1,nphot
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)                        
            if (k.le.(n1+n2+n3)) then
               prodp3=prodp3*
     .              (1.d0/p3q*p3(0)*ptmp(0))
            endif
            if (k.gt.(n1+n2+n3)) then
               prodp4=prodp4*
     .              (1.d0/p4q*p4(0)*ptmp(0))
            endif 
         enddo
         if (iref.eq.3) anum = prodp3
         if (iref.eq.4) anum = prodp4
         regulator1 = anum/(prodp3+prodp4) * 2.d0
         w = w * regulator1
      endif
*******************
* second "regulator"
      den  = 1.d0
      anum = 1.d0
      iswitch = 1
      if (fs.eq.'gg') iswitch = 0
      if (n1.gt.0) then
         do k = 1,n1
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p1q = dot(p1,ptmp)
            p2q = dot(p2,ptmp)
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)
            den = den * (
     .           1.d0/p1q*p1(0)*ptmp(0)+
     .           1.d0/p2q*p2(0)*ptmp(0)+
     .           iswitch/p3q*p3(0)*ptmp(0)+
     .           iswitch/p4q*p4(0)*ptmp(0))
            anum = anum*(1.d0/p1q*p1(0)*ptmp(0))
         enddo
      endif
      if (n2.gt.0) then
         do k = n1+1,n1+n2
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p1q = dot(p1,ptmp)
            p2q = dot(p2,ptmp)
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)
            den = den * (
     .           1.d0/p1q*p1(0)*ptmp(0)+
     .           1.d0/p2q*p2(0)*ptmp(0)+
     .           iswitch/p3q*p3(0)*ptmp(0)+
     .           iswitch/p4q*p4(0)*ptmp(0))
            anum = anum*(1.d0/p2q*p2(0)*ptmp(0))
         enddo
      endif
      if (n3.gt.0) then
         do k = n1+n2+1,n1+n2+n3
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p1q = dot(p1,ptmp)
            p2q = dot(p2,ptmp)
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)
            den = den * (
     .           1.d0/p1q*p1(0)*ptmp(0)+
     .           1.d0/p2q*p2(0)*ptmp(0)+
     .           iswitch/p3q*p3(0)*ptmp(0)+
     .           iswitch/p4q*p4(0)*ptmp(0))
            anum = anum*(1.d0/p3q*p3(0)*ptmp(0))
         enddo
      endif
      if (n4.gt.0) then
         do k = n1+n2+n3+1,nphot
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p1q = dot(p1,ptmp)
            p2q = dot(p2,ptmp)
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)
            den = den * (
     .           1.d0/p1q*p1(0)*ptmp(0)+
     .           1.d0/p2q*p2(0)*ptmp(0)+
     .           iswitch/p3q*p3(0)*ptmp(0)+
     .           iswitch/p4q*p4(0)*ptmp(0))
            anum = anum*(1.d0/p4q*p4(0)*ptmp(0))
         enddo
      endif
      regulator2 = 1.d0* ncharged**nphot * anum/den
      w = w * regulator2
!-- extra regulator for gg ------------------------
      if (fs.eq.'gg') then
! I try a more symmetryc regulator ! test
         call regulate_gg(nphot,p1,p2,p3,p4,qph,reg)
         w=w/regulator2*reg
      endif
!-------------------------------------------------
      return
      end


************************************************************************************
      subroutine phasespaceTMP(p1,p2,p3,p4,qph,nphot,m1,m2,esoft,
     .     cth,w,phsp,ie)
! written by CMCC, last modified 19/10/2006
      implicit double precision (a-h,m,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3)
      dimension ap3(0:3),ap4(0:3),pcollorig(0:3)
      dimension p4guess(0:3),p3guess(0:3),qone(0:3)
      dimension dir3(0:3),dir4(0:3),ptmp(0:3),vers(0:3)
      double precision kone(0:3),pcoll(0:3),ptld(0:3)
      double precision lambda
      logical lcond
      character*2 fs
      common/finalstate/fs
      integer emissionpattern(nphot)
      parameter (pi = 3.1415926535897932384626433832795029d0)
      parameter (imaxph = 40)
      double precision qph(imaxph,0:3),oph(imaxph),wten(nphot)
      real*4 csi(1),csi2(2),rnddummy
      common/for_debug/pcoll,denphsp,dir3,dir4,br,bq,onm
      common/various/beta,betafs
      common/channelref/iref
      common/iwide/iwider,ntot
      common/regulators/ich,idummy,regulator1,regulator2,wcoll3,wcoll4
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/coseni/cmax,cmin

      common/forshatsampling/den,omin,omax,ei
      common/cthcommon/cthc
      
      common/radpattern/nph(4)
      common/idebugging/idebug
      common/idarkon/idarkon

      common/weightwcos/wcos    ! to be depurated in phasespace if needed
      
      double precision babayaganlo_rndm
      external babayaganlo_rndm
      
      w      = 1.d0
      phsp   = 1.d0
      npart = nphot + 2
      nvar  = 3*nphot + 2

      ie     = 0

      duepigrechi = (2.d0*pi)**(-nvar)

      iwider = 0
      ntot   = nphot

      do j = 0,3
         do k = 1,imaxph
            qph(k,j) = 0.d0
         enddo
         nph(j+1) = 0
      enddo

      if (p1(0)+p2(0).lt.(m1+m2+nphot*esoft)) then 
         phsp = 0.d0
         w    = 0.d0
         ie = 1
         return
      endif
*****************
!! I put this outside in order to raise efficiency
!      call get_cos_fer(rnd(1),c,wcos)
!      cth = c
!      w   = w * wcos
      c    = cth
! cthc in common for the sampling
      cthc = cth
*     
      if (nphot.eq.0) then
         mass = p1(0)+p2(0)
         sqla = dsqrt(lambda(mass**2,m1**2,m2**2))
         phsp = phsp * sqla/8.d0/mass**2
         s = dsqrt(1.d0 - c**2)
         csi(1) = babayaganlo_rndm(0)
         call getphi(csi(1),phi,wph)
         w = w*wph
         sphi = sin(phi)
         cphi = cos(phi)
         p1mod = sqla/2.d0/mass
         p3(0) = dsqrt(p1mod**2 + m1**2)
         p3(1) = p1mod * sphi * s
         p3(2) = p1mod * cphi * s
         p3(3) = p1mod * c
         p4(0) = mass - p3(0)
         p4(1) = - p3(1)
         p4(2) = - p3(2)
         p4(3) = - p3(3)
         phsp = phsp * duepigrechi
         return
      endif
*******************
******************
      lcond = fs.eq.'ee'.or.fs.eq.'mm'
      
      angle = 30.d0 *pi/180.d0

      angle = 0.d0 *pi/180.d0

      cpmax = cos(angle)
      cpmin = cos(pi - angle)
* the following for ng >= 1
      ncharged = 4
      if (fs.eq.'gg') ncharged = 2
      ir = 1

      csi(1) = babayaganlo_rndm(0)
      call getphi(csi(1),phi,wph)
      w    = w*wph
      sphi = dsin(phi)
      cphi = dcos(phi)
      ir   = ir + 2

      call get_patternNEW(ncharged,nphot,emissionpattern,wpat)
      do k = 1,nphot
         nph(emissionpattern(k)) = nph(emissionpattern(k)) + 1
      enddo
      w = w * wpat
      n1 = nph(1)
      n2 = nph(2)
      n3 = nph(3)
      n4 = nph(4)
      nis = n1 + n2
      nfs = n3 + n4
      
      ei = p1(0)+p2(0)

      ptld(0) = p1(0)+p2(0)
      vers(0)  = 0.d0
      do k = 1,3
         ptld(k)  = p1(k)+p2(k)
         vers(k)  = 0.d0
      enddo               

      csi(1) = babayaganlo_rndm(0)
      iref = 3
      if (csi(1).gt.0.5) iref = 4
      wref = 1.d0
      if (nfs.gt.0) then
         if (n4.eq.0) iref = 3
         if (n3.eq.0) iref = 4
      endif
      w = w*wref
      if (fs.eq.'gg') iref = 3
****
** PHOTONS GENERATION...
      pflatc  = 0.d0
      pflatc3 = 0.d0
      pflatc4 = 0.d0
      pflate  = 0.d0
      expgg = 0.92d0
      ibich = 1
      do k = 1,nphot
         wen = 1.d0
         en  = esoft
         if (fs.eq.'gg') then
            call photon_energy(ibich,pflate,expgg,p1(0),
     .           en,wen)
         endif
         wten(k) = wen
         if (fs.eq.'ee'.or.fs.eq.'mm') w = w * wen
         oph(k) = en
         ir = ir + 1
      enddo
** initial state photons
      
      emaxgamma = (p1(0)+p2(0))/2.d0
      omax = emaxgamma
      omin = esoft
      kone = 0
      prodomega = 1.d0      
      if (nis.gt.0) then

         csi(1) = babayaganlo_rndm(0)
         nlast = csi(1)*nis + 1
         
         if (n1.gt.0) then
            do k = 1,n1
            IF (k.ne.nlast) THEN
               if (omax.lt.omin) then
                  phsp = 0.d0
                  w    = 0.d0
                  ie   = 1
                  return
               endif

               iaccendi = 1
               csi2(1) = babayaganlo_rndm(0)
               csi2(2) = babayaganlo_rndm(0)
               call collinear(0.d0,csi2(1),csi2(2),p1,vers,wverg)
               iaccendi = 0
               ir = ir + 2
               w = w * wverg
****
               if (lcond) then
                  if (idarkon.eq.1) then
                      call photon_energy_dm(emaxgamma,en,wen)
                  else
                     call photon_energy_IS(omin,omax,en,wen)
                  endif
                  w = w * wen
                  oph(k) = en
               endif
***
               qph(k,0) = oph(k)
               qph(k,1) = oph(k)*vers(1)
               qph(k,2) = oph(k)*vers(2)
               qph(k,3) = oph(k)*vers(3)

               kone = kone + qph(k,:)
               omax = omax - en
               prodomega = prodomega * en

            ENDIF
            enddo
         endif
         if (n2.gt.0) then
            do k = n1+1,n1+n2
            IF (k.ne.nlast) THEN
               if (omax.lt.omin) then
                  phsp = 0.d0
                  w    = 0.d0
                  ie   = 1
                  return
               endif

               iaccendi = 2
               csi2(1) = babayaganlo_rndm(0)
               csi2(2) = babayaganlo_rndm(0)
               call collinear(0.d0,csi2(1),csi2(2),p2,vers,wverg)
               iaccendi  = 0
               ir = ir + 2
               w = w * wverg
****
               if (lcond) then
                  if (idarkon.eq.1) then
                      call photon_energy_dm(emaxgamma,en,wen)
                  else
                     call photon_energy_IS(omin,omax,en,wen)
                  endif
                  w = w * wen
                  oph(k) = en
               endif
***
               qph(k,0) = oph(k)
               qph(k,1) = oph(k)*vers(1)
               qph(k,2) = oph(k)*vers(2)
               qph(k,3) = oph(k)*vers(3)

               kone = kone + qph(k,:)
               omax = omax - en

               prodomega = prodomega * en
               
            ENDIF
            enddo
         endif

*** last IS photon
         if (omax.lt.omin) then
            phsp = 0.d0
            w    = 0.d0
            ie   = 1
            return
         endif

         csi2(1) = babayaganlo_rndm(0)
         csi2(2) = babayaganlo_rndm(0)
         if (nlast.le.n1) then
            iaccendi = 1
            call collinear(0.d0,csi2(1),csi2(2),p1,vers,wverg)
            iaccendi = 0
         else
            iaccendi = 2
            call collinear(0.d0,csi2(1),csi2(2),p2,vers,wverg)
            iaccendi = 0
         endif
         w = w * wverg
         
         ptld   = p1 + p2 - kone
         ptld2  = dot(ptld,ptld)
         E1pE2  = p1(0)+p2(0)
         v1ltld = dot(vers,kone)
         den    = (E1pE2 - v1ltld)
         
         if (den.lt.0.d0) then
            phsp = 0.d0
            w    = 0.d0
            ie   = 1
            return
         endif
         smin = ptld2 - 2.d0*omax*den
! default
         smin = max(4.d0*m1*m1,smin)

! this takes into account the acollinearity cut         
c         bmin = sqrt(1.d0 - m1*m1/emin/emin)
c         cccc = cos(pi-zmax)
c         smin = 2.d0*m1*m1 + 2.d0*emin*emin*(1.d0-bmin*bmin*cccc)
!!
         smax = ptld2 - 2.d0*omin*den
         
         if (smin.gt.smax) then
            phsp = 0.d0
            w    = 0.d0
            ie   = 1
            return
         endif
         
         call get_shat(ptld2,smin,smax,shat,wshat)
         w = w * wshat

         en   = 0.5d0*(ptld2 - shat)/den

         prodomega = prodomega*en*en/(ptld2-shat)
         
         k = nlast
         oph(k)   = en
         qph(k,0) = oph(k) 
         qph(k,1) = oph(k)*vers(1)
         qph(k,2) = oph(k)*vers(2)
         qph(k,3) = oph(k)*vers(3)
 111     continue
         omax = omax - en
         kone = kone + qph(k,:)
      endif
      
      qone  = p1 + p2 - kone
      pcoll = qone
      qone2 = dot(qone,qone)

      if (lcond.and.
     .   (qone2.lt.(m1+m2+nfs*esoft)**2.or.pcoll(0).lt.0.d0)) then
         ie = 1
         phsp = 0.d0
         w  = 0.d0
         return
      endif

      
******
ccc      w = w * fakeBW(qone2)
******
      
*** phase space for inital state photons
      phspis = prodomega
***   

***   if I'm really close to J/Psi, better to generate cth flat...
c      sqone2 = sqrt(qone2)
c      if (sqone2.gt.3.097d0.and.sqone2.lt.3.098d0) then
c         w = w/wcos
c         call ranlux(csi,1)
c         cth = (cmax - cmin) * csi(1) + cmin
c         w   = w * (cmax - cmin)
c      endif
      
! cos(th) and phi of electron already generated...
      sth  = dsqrt(1.d0 - cth**2)
      dir3(0) = ei/2d0
      dir3(1) = ei/2d0*betafs*cphi*sth
      dir3(2) = ei/2d0*betafs*sphi*sth
      dir3(3) = ei/2d0*betafs*cth         
      dir4(0) = dir3(0)
      dir4(1)  = -dir3(1)
      dir4(2)  = -dir3(2)
      dir4(3)  = -dir3(3)

*******************************************      
      
** final state photons
c      en3guess = dir3(0)
c      en4guess = dir4(0)
      wcoll3 = 1.d0
      wcoll4 = 1.d0
      if (nfs.gt.0) then
         if (iref.eq.3) then
            if (n3.gt.0) then
               do k = 1,n3
                  iaccendi = 3
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear(pflatc3,csi2(1),csi2(2),dir3,vers,
     .                 wverg)
                  iaccendi = 0
                  ir = ir + 2
                  w = w * wverg
****
                  if (lcond) then
                    if (idarkon.eq.1) then
                       call photon_energy_dm(emaxgamma,en,wen)
                    else
                       cp = vers(3)/vers(0)
                       if (cp.gt.cpmax.or.cp.lt.cpmin) then
                            call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                       else
                            call photon_energy(ibich,pflate,expgg,
     .                                         emaxgamma,en,wen)
                       endif
                     endif
                     w = w * wen
                     oph(k+n1+n2) = en
                  endif
***
                  wcoll3 = wcoll3 * wverg
                  qph(k+n1+n2,0) = oph(k+n1+n2)
                  qph(k+n1+n2,1) = oph(k+n1+n2)*vers(1)
                  qph(k+n1+n2,2) = oph(k+n1+n2)*vers(2)
                  qph(k+n1+n2,3) = oph(k+n1+n2)*vers(3)

                  kone = kone + qph(k+n1+n2,:)
                  
                  pcoll(0) = pcoll(0) - qph(k+n1+n2,0)
                  pcoll(1) = pcoll(1) - qph(k+n1+n2,1)
                  pcoll(2) = pcoll(2) - qph(k+n1+n2,2)
                  pcoll(3) = pcoll(3) - qph(k+n1+n2,3)
c                  en3guess = en3guess - oph(k+n1+n2)
               enddo
            endif
            if (dot(pcoll,pcoll).lt.(m1+m2)**2.or.pcoll(0).lt.0.d0) then
               ie = 1
               phsp = 0.d0
               w  = 0.d0
               return
            endif

            do k = 0,3
               pcoll(k) = pcoll(k) - dir3(k)
               pcollorig(k) = pcoll(k)
            enddo

            pcmodmu = 1.d0/sqrt(tridot(pcoll,pcoll))            
            pcoll(0) = dir4(0)
            bcoll    = sqrt(1.d0 - m1**2/pcoll(0)**2)
            pcoll(1) = pcoll(1)*pcmodmu*bcoll*pcoll(0)
            pcoll(2) = pcoll(2)*pcmodmu*bcoll*pcoll(0)
            pcoll(3) = pcoll(3)*pcmodmu*bcoll*pcoll(0)

            if (n4.gt.0) then
               do k = 1,n4
                  iaccendi = 4
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear(pflatc4,csi2(1),csi2(2),pcoll,vers,
     .                 wverg)
                  iaccendi = 0
                  ir = ir + 2
                  w = w * wverg
****
                  if (lcond) then
                    if (idarkon.eq.1) then
                       call photon_energy_dm(emaxgamma,en,wen)
                    else
                       cp = vers(3)/vers(0)
                       if (cp.gt.cpmax.or.cp.lt.cpmin) then
                            call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                       else
                            call photon_energy(ibich,pflate,expgg,
     .                                         emaxgamma,en,wen)
                       endif
                     endif
                     w = w * wen
                     oph(k+n1+n2+n3) = en
                  endif
***
                  wcoll4 = wcoll4 * wverg
                  qph(k+n1+n2+n3,0) = oph(k+n1+n2+n3)
                  qph(k+n1+n2+n3,1) = oph(k+n1+n2+n3)*vers(1)
                  qph(k+n1+n2+n3,2) = oph(k+n1+n2+n3)*vers(2)
                  qph(k+n1+n2+n3,3) = oph(k+n1+n2+n3)*vers(3)

                  kone = kone + qph(k+n1+n2+n3,:)
                  
               enddo
            endif
         else                   ! if iref = 4
            if (n4.gt.0) then
               do k = 1,n4
                  iaccendi = 4
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear(pflatc4,csi2(1),csi2(2),dir4,vers,
     .                 wverg)
                  iaccendi = 0
                  ir = ir + 2
                  w = w * wverg
****
                  if (lcond) then
                    if (idarkon.eq.1) then
                       call photon_energy_dm(emaxgamma,en,wen)
                    else
                       cp = vers(3)/vers(0)
                       if (cp.gt.cpmax.or.cp.lt.cpmin) then
                            call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                       else
                            call photon_energy(ibich,pflate,expgg,
     .                                         emaxgamma,en,wen)
                       endif
                     endif
                     w = w * wen
                     oph(k+n1+n2+n3) = en
                  endif
***   
                  wcoll4 = wcoll4 * wverg
                  qph(k+n1+n2+n3,0) = oph(k+n1+n2+n3)
                  qph(k+n1+n2+n3,1) = oph(k+n1+n2+n3)*vers(1)
                  qph(k+n1+n2+n3,2) = oph(k+n1+n2+n3)*vers(2)
                  qph(k+n1+n2+n3,3) = oph(k+n1+n2+n3)*vers(3)

                  kone = kone + qph(k+n1+n2+n3,:)
                  
                  pcoll(0) = pcoll(0) - qph(k+n1+n2+n3,0)
                  pcoll(1) = pcoll(1) - qph(k+n1+n2+n3,1)
                  pcoll(2) = pcoll(2) - qph(k+n1+n2+n3,2)
                  pcoll(3) = pcoll(3) - qph(k+n1+n2+n3,3)
c                  en4guess = en4guess - oph(k+n1+n2+n3)
               enddo
            endif
            if (dot(pcoll,pcoll).lt.(m1+m2)**2.or.pcoll(0).lt.0.d0) then
               ie = 1
               phsp = 0.d0
               w  = 0.d0
               return
            endif

            do k = 0,3
               pcoll(k) = pcoll(k) - dir4(k)
               pcollorig(k) = pcoll(k)
            enddo

            pcmodmu = 1.d0/sqrt(tridot(pcoll,pcoll))            
            pcoll(0) = dir3(0)
            bcoll    = sqrt(1.d0 - m2**2/pcoll(0)**2)
            pcoll(1) = pcoll(1)*pcmodmu*bcoll*pcoll(0)
            pcoll(2) = pcoll(2)*pcmodmu*bcoll*pcoll(0)
            pcoll(3) = pcoll(3)*pcmodmu*bcoll*pcoll(0)

            if (n3.gt.0) then
               do k = 1,n3
                  iaccendi = 3
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear(pflatc3,csi2(1),csi2(2),pcoll,vers,
     .                 wverg)
                  iaccendi = 0
                  ir = ir + 2
                  w = w * wverg
****
                  if (lcond) then
                    if (idarkon.eq.1) then
                       call photon_energy_dm(emaxgamma,en,wen)
                    else
                       cp = vers(3)/vers(0)
                       if (cp.gt.cpmax.or.cp.lt.cpmin) then
                            call photon_energy_new(ibich,pflate,expgg,
     .                              emaxgamma,esoft,ptld,vers,en,wen)
                       else
                            call photon_energy(ibich,pflate,expgg,
     .                                         emaxgamma,en,wen)
                       endif
                     endif
                     w = w * wen
                     oph(k+n1+n2) = en
                  endif
*** 
                  wcoll3 = wcoll3 * wverg
                  qph(k+n1+n2,0) = oph(k+n1+n2)
                  qph(k+n1+n2,1) = oph(k+n1+n2)*vers(1)
                  qph(k+n1+n2,2) = oph(k+n1+n2)*vers(2)
                  qph(k+n1+n2,3) = oph(k+n1+n2)*vers(3)

                  kone = kone + qph(k+n1+n2,:)
                  
               enddo               
            endif            
         endif
      endif
***********************************
      do j = 0,3
         ptmp(j) = p1(j) + p2(j) - kone(j)
      enddo
***** TEST
      if (fs.eq.'gg') then
         wwen = 1.d0
         if (nphot.gt.1) then
            do k = 1,nphot-1
               wwen = wwen * wten(k)
            enddo
         endif
         w = w * wwen
         do j = 0,3
            kone(j) = kone(j)-qph(nphot,j)
         enddo

         pk2 = dot(kone,kone)
         br  = kone(0) - pk2/2.d0/ei
         bq  = kone(0) - tridot(kone,vers)
         onm = ei/2.d0 - br
         onm = onm/(1.d0 - bq/ei)
         a  = ei/2.d0 - kone(0) + pk2/2.d0/ei
         b  = 1.d0  - dot(kone,vers)/ei
         c  = ei - kone(0) + tridot(kone,dir3)/beta/dir3(0)
         d  = 1.d0 - tridot(vers,dir3)/beta/dir3(0)
         ep = (ei*a - esoft*c)/(ei*b-esoft*d)
         if (ep.lt.esoft.or.ep.ge.a/b.or.ep.eq.p1(0)) then
            w = 0.d0
            phsp = 0.d0
            ie = 1
            return
         endif
         call photonenergygg(ep,a,b,esoft,en,wen)
         w = w * wen
         qph(nphot,0) = en
         qph(nphot,1) = en*vers(1)
         qph(nphot,2) = en*vers(2)
         qph(nphot,3) = en*vers(3)
         do j = 0,3
            kone(j) = kone(j)+qph(nphot,j)
            ptmp(j) = p1(j) + p2(j) - kone(j)
         enddo
      endif
***** TEST
      if (dot(ptmp,ptmp).lt.(m1+m2)**2.or.ptmp(0).lt.0.d0) then
         ie = 1
         phsp = 0.d0
         w  = 0.d0
         return
      endif
      amkone2 = dot(kone,kone)
***
      if (iref.eq.3) then
         vx  = sth*cphi
         vy  = sth*sphi
         vz  = cth
      else
         vx  = -sth*cphi
         vy  = -sth*sphi
         vz  = -cth
      endif
      o   = kone(0)
      ox  = kone(1)
      oy  = kone(2)
      oz  = kone(3)
      odv = ox*vx + oy*vy + oz*vz
      abig = ei**2 - 2.d0 * ei * o + amkone2 + m1**2 - m2**2
      bbig = -2.d0 * (ei - o)
      cbig = -2.d0 * odv
      arg  = abig**2*bbig**2-m1**2*bbig**2*(bbig**2-cbig**2)
      if (arg.lt.0.d0) then
         phsp = 0.d0
         w    = 0.d0
         ie = 2
         return
      endif
      p1mod_1 = abig*cbig + dsqrt(arg)
      p1mod_1 = p1mod_1 / (bbig**2 - cbig**2)
      p1mod_2 = abig*cbig - dsqrt(arg)
      p1mod_2 = p1mod_2 / (bbig**2 - cbig**2)
      p1jac = 1.d0
      if (p1mod_1.gt.0.d0.and.p1mod_2.gt.0.d0) then
         csi(1) = babayaganlo_rndm(0)
         if (csi(1).lt.0.5) p1mod = p1mod_1
         if (csi(1).ge.0.5) p1mod = p1mod_2
         p1jac = 2.d0
      endif
      if (p1mod_1.gt.0.d0.and.p1mod_2.le.0.d0) then
         p1mod = p1mod_1
      endif
      if (p1mod_1.le.0.d0.and.p1mod_2.gt.0.d0) then
         p1mod = p1mod_2
      endif
      if (p1mod_1.le.0.d0.and.p1mod_2.le.0.d0) then ! bestiale: con gg ci vuole .le.!!
         phsp = 0.d0
         w    = 0.d0
         ie   = 3
         return
      endif
      w = w * p1jac
      p3(0) = dsqrt(p1mod**2 + m1**2)
      p3(1) = p1mod * vx
      p3(2) = p1mod * vy
      p3(3) = p1mod * vz
      p4(0) =  ei - p3(0) - o
      p4(1) = -ox - p3(1)
      p4(2) = -oy - p3(2)
      p4(3) = -oz - p3(3)
***
      if (fs.eq.'gg'.and.(p3(0).lt.esoft.or.p4(0).lt.esoft)) then
         phsp = 0.d0
         w    = 0.d0
         ie = 1
         return
      endif
***
!      phasespace!
      if (nfs.gt.0) then
         do k = nis+1,nis+nfs
            prodomega = prodomega * qph(k,0)
         enddo
      endif
      den = dabs( p4(0)*p1mod + p3(0)*(p1mod + odv) )
      denphsp = den
      phsp = prodomega * p1mod**2 / denphsp
      phsp = phsp*duepigrechi/2.d0**(nphot+2)
      if (iref.eq.4) call exchange_mom(p3,p4)

***********************      
! "regulator"... I try only on FINAL STATE (active only for ee and mm...)
      flatconst = 0.d0 !!!
      den       = 0.d0
      if (n3.gt.0.and.n4.gt.0) then
         prodp3 = 1.d0
         prodp4 = 1.d0         
         do k = nis+1,nphot
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)                        
            if (k.le.(n1+n2+n3)) then
               prodp3=prodp3*
     .              (1.d0/p3q*p3(0)*ptmp(0))
            endif
            if (k.gt.(n1+n2+n3)) then
               prodp4=prodp4*
     .              (1.d0/p4q*p4(0)*ptmp(0))
            endif 
         enddo
         if (iref.eq.3) anum = prodp3
         if (iref.eq.4) anum = prodp4
         regulator1 = anum/(prodp3+prodp4) * 2.d0
         w = w * regulator1
      endif
*******************
* second "regulator"
      den  = 1.d0
      anum = 1.d0

***   regulator(s) ***      
      IF (lcond) THEN
      if (n1.gt.0) then
         do k = 1,n1
            anum = anum /dot(p1,qph(k,:)) !* p1(0)*qph(k,0)
         enddo
      endif
      if (n2.gt.0) then
         do k = n1+1,n1+n2
            anum = anum /dot(p2,qph(k,:)) !* p2(0)*qph(k,0)
         enddo
      endif
      if (n3.gt.0) then
         do k = n1+n2+1,n1+n2+n3
            anum = anum /dot(p3,qph(k,:)) !* p3(0)*qph(k,0)
         enddo
      endif
      if (n4.gt.0) then
         do k = n1+n2+n3+1,n1+n2+n3+n4
            anum = anum /dot(p4,qph(k,:)) !* p4(0)*qph(k,0)
         enddo
      endif
      
cc      anum = anum*fakeBW(qone2)
cc      anum = anum * sdif(qone2,cthc)
cc      anum = anum * spezzasdif(qone2)
      
      call handregulator(nphot,p1,p2,p3,p4,qph(1:nphot,:),den)
      
      else
      
      iswitch = 1
      if (fs.eq.'gg') iswitch = 0
      if (n1.gt.0) then
         do k = 1,n1
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p1q = dot(p1,ptmp)
            p2q = dot(p2,ptmp)
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)
            den = den * (
     .           1.d0/p1q*p1(0)*ptmp(0)+
     .           1.d0/p2q*p2(0)*ptmp(0)+
     .           iswitch/p3q*p3(0)*ptmp(0)+
     .           iswitch/p4q*p4(0)*ptmp(0))
            anum = anum*(1.d0/p1q*p1(0)*ptmp(0))
         enddo
      endif
      if (n2.gt.0) then
         do k = n1+1,n1+n2
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p1q = dot(p1,ptmp)
            p2q = dot(p2,ptmp)
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)
            den = den * (
     .           1.d0/p1q*p1(0)*ptmp(0)+
     .           1.d0/p2q*p2(0)*ptmp(0)+
     .           iswitch/p3q*p3(0)*ptmp(0)+
     .           iswitch/p4q*p4(0)*ptmp(0))
            anum = anum*(1.d0/p2q*p2(0)*ptmp(0))
         enddo
      endif
      if (n3.gt.0) then
         do k = n1+n2+1,n1+n2+n3
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p1q = dot(p1,ptmp)
            p2q = dot(p2,ptmp)
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)
            den = den * (
     .           1.d0/p1q*p1(0)*ptmp(0)+
     .           1.d0/p2q*p2(0)*ptmp(0)+
     .           iswitch/p3q*p3(0)*ptmp(0)+
     .           iswitch/p4q*p4(0)*ptmp(0))
            anum = anum*(1.d0/p3q*p3(0)*ptmp(0))
         enddo
      endif
      if (n4.gt.0) then
         do k = n1+n2+n3+1,nphot
            do i = 0,3
               ptmp(i) = qph(k,i)
            enddo
            p1q = dot(p1,ptmp)
            p2q = dot(p2,ptmp)
            p3q = dot(p3,ptmp)
            p4q = dot(p4,ptmp)
            den = den * (
     .           1.d0/p1q*p1(0)*ptmp(0)+
     .           1.d0/p2q*p2(0)*ptmp(0)+
     .           iswitch/p3q*p3(0)*ptmp(0)+
     .           iswitch/p4q*p4(0)*ptmp(0))
            anum = anum*(1.d0/p4q*p4(0)*ptmp(0))
         enddo
      endif

 321  continue
      ENDIF
      
      regulator2 = 1.d0 * anum/den
      w = w * regulator2
!-- extra regulator for gg ------------------------
      if (fs.eq.'gg') then
! I try a more symmetryc regulator ! test
         call regulate_gg(nphot,p1,p2,p3,p4,qph,reg)
         w=w/regulator2*reg
      endif
!-------------------------------------------------
      return
      end
************************
      subroutine phasespaceNEW(p1,p2,p3,p4,qph,nphot,m1,m2,esoft,
     .     cth,w,phsp,ie)
! written by CMCC, last modified 19/10/2006
      implicit double precision (a-h,m,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3)
      dimension ap3(0:3),ap4(0:3),pcollorig(0:3)
      dimension p4guess(0:3),p3guess(0:3),direm(0:3)
      dimension dir3(0:3),dir4(0:3),ptmp(0:3),vers(0:3)
      double precision kone(0:3),pcoll(0:3),ptld(0:3)
      double precision qone(0:3),konefs(0:3),pboo1(0:3),pboo2(0:3)
      double precision lambda
      logical lcond
      character*2 fs
      common/finalstate/fs
      integer emissionpattern(nphot)
      parameter (pi = 3.1415926535897932384626433832795029d0)
      parameter (imaxph = 40)
      double precision qph(imaxph,0:3),oph(imaxph),wten(nphot)
      double precision qphfs(imaxph,0:3),ophfs(imaxph)
      real*4 csi(1),csi2(2),rnddummy
      common/for_debug/pcoll,denphsp,dir3,dir4,br,bq,onm
      common/various/beta,betafs
      common/channelref/iref
      common/iwide/iwider,ntot
      common/regulators/ich,idummy,regulator1,regulator2,wcoll3,wcoll4
      common/radpattern/nph(4)
      common/idebugging/idebug
      common/tmintmax/tmin,tmax,tcur
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax

      common/epssoft/eps

      common/weightwcos/wcos ! to be depurated in phasespaceNEW
      
      double precision babayaganlo_rndm
      external babayaganlo_rndm
      
      w      = 1.d0
      npart = nphot + 2
      nvar  = 3*nphot + 2

      ie     = 0

      duepigrechi = (2.d0*pi)**(-nvar)
      phsp   = 1.d0

      iwider = 0
      ntot   = nphot

      do j = 0,3
         do k = 1,imaxph
            qph(k,j) = 0.d0
         enddo
         nph(j+1) = 0
      enddo

      if (p1(0)+p2(0).lt.(m1+m2+nphot*esoft)) then 
         phsp = 0.d0
         w    = 0.d0
         ie = 1
         return
      endif
*****************
!! I put this outside in order to raise efficiency
!      call get_cos_fer(rnd(1),c,wcos)
!      cth = c
!      w   = w * wcos
      c = cth
*
      if (nphot.eq.0) then
         mass = p1(0)+p2(0)
         sqla = dsqrt(lambda(mass**2,m1**2,m2**2))
         phsp = phsp * sqla/8.d0/mass**2
         s = dsqrt(1.d0 - c**2)
         csi(1) = babayaganlo_rndm(0)
         call getphi(csi(1),phi,wph)
         w = w*wph
         sphi = sin(phi)
         cphi = cos(phi)
         p1mod = sqla/2.d0/mass
         p3(0) = dsqrt(p1mod**2 + m1**2)
         p3(1) = p1mod * sphi * s
         p3(2) = p1mod * cphi * s
         p3(3) = p1mod * c
         p4(0) = mass - p3(0)
         p4(1) = - p3(1)
         p4(2) = - p3(2)
         p4(3) = - p3(3)
         phsp  = phsp*duepigrechi
         return
      endif
*******************
      lcond = fs.eq.'ee'.or.fs.eq.'mm'
! here only if lcond = .true. !!!
      
* the following for ng >= 1
      ncharged = 4
      ir = 1

! cos(th) of electron already generated...
      sth  = dsqrt(1.d0 - cth**2)
      csi(1) = babayaganlo_rndm(0)
      call getphi(csi(1),phi,wph)
      w    = w*wph
      sphi = dsin(phi)
      cphi = dcos(phi)
      ir   = ir + 2
***
      direm(0) = 1.d0
      direm(1) = sphi*sth
      direm(2) = cphi*sth
      direm(3) = cth
***
      call get_pattern(ncharged,nphot,emissionpattern)
      do k = 1,nphot
         nph(emissionpattern(k)) = nph(emissionpattern(k)) + 1
      enddo
      n1 = nph(1)
      n2 = nph(2)
      n3 = nph(3)
      n4 = nph(4)
      nis = n1 + n2
      nfs = n3 + n4
      ei = p1(0)+p2(0)
****
** PHOTONS GENERATION...
      omin = eps * ei * 0.5d0
      omax = ei  * 0.5d0

** initial state photons
      kone = 0
      prodomega = 1.d0
      if (nis.gt.0) then
***
         csi(1) = babayaganlo_rndm(0)
         nlast = csi(1)*nis + 1
c     
cc         nlast = nis + 1
cc
***
         if (n1.gt.0) then
            do k = 1,n1
               if (k.ne.nlast) then
                  if (omax.lt.omin) then
                     phsp = 0.d0
                     w    = 0.d0
                     ie   = 1
                     return
                  endif
                  
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear_simple(0.d0,csi2(1),csi2(2),p1,
     .                 vers,wverg)
                  w = w * wverg
                  call photon_energy_IS(omin,omax,en,wen)
                  w = w * wen
                  oph(k)  = en
                  qph(k,0)= oph(k) 
                  qph(k,1)= oph(k)*vers(1)
                  qph(k,2)= oph(k)*vers(2)
                  qph(k,3)= oph(k)*vers(3)
               
                  omax = omax - en
                  prodomega = prodomega * 0.5 * en
                  kone = kone + qph(k,:)
               endif
            enddo
         endif
         if (n2.gt.0) then
            do k = n1+1,n1+n2
               if (k.ne.nlast) then
                  if (omax.lt.omin) then
                     phsp = 0.d0
                     w    = 0.d0
                     ie = 1
                     return
                  endif
                  csi2(1) = babayaganlo_rndm(0)
                  csi2(2) = babayaganlo_rndm(0)
                  call collinear_simple(0.d0,csi2(1),csi2(2),p2,
     .                 vers,wverg)
                  w = w * wverg
                  call photon_energy_IS(omin,omax,en,wen)
                  w = w * wen
                  oph(k)  = en
                  qph(k,0)= oph(k) 
                  qph(k,1)= oph(k)*vers(1)
                  qph(k,2)= oph(k)*vers(2)
                  qph(k,3)= oph(k)*vers(3)
                  
                  omax = omax - en
                  prodomega = prodomega * 0.5 * en
                  kone = kone + qph(k,:)
               endif
            enddo
         endif
*** last IS photon
cc         goto 111

         if (omax.lt.omin) then
            phsp = 0.d0
            w    = 0.d0
            ie = 1
            return
         endif
         csi2(1) = babayaganlo_rndm(0)
         csi2(2) = babayaganlo_rndm(0)
         if (nlast.le.n1) then
            call collinear_simple(0.d0,csi2(1),csi2(2),p1,vers,wverg)
         else
            call collinear_simple(0.d0,csi2(1),csi2(2),p2,vers,wverg)
         endif
         w = w * wverg

         ptld   = p1 + p2 - kone
         ptld2  = dot(ptld,ptld)
         E1pE2  = p1(0)+p2(0)
         v1ltld = dot(vers,kone)
         den    = (E1pE2 - v1ltld)
         
         if (den.lt.0.d0) then
            phsp = 0.d0
            w    = 0.d0
            ie   = 1
            return
         endif
         smin = ptld2 - 2.d0*omax*den
         smin = max(4.d0*m1*m1,smin)
c         smin = 4.d0*emin**2

         smax = ptld2 - 2.d0*omin*den

         if (smin.gt.smax) then
            phsp = 0.d0
            w    = 0.d0
            ie   = 1
            return
         endif
         
         call get_shat(ptld2,smin,smax,shat,wshat)
         w = w * wshat
         
         en   = 0.5d0*(ptld2 - shat)/den

         prodomega = 0.5d0*prodomega*en*en/(ptld2-shat)
         
         k = nlast
         oph(k)   = en
         qph(k,0) = oph(k) 
         qph(k,1) = oph(k)*vers(1)
         qph(k,2) = oph(k)*vers(2)
         qph(k,3) = oph(k)*vers(3)
 111     continue
         omax = omax - en
         kone = kone + qph(k,:)
      endif

      qone  = p1 + p2 - kone
      qone2 = dot(qone,qone)

      if (qone(0).lt.0.d0.or.qone2.lt.(m1+m2+nfs*omin)**2) then
         phsp = 0.d0
         w    = 0.d0
         ie   = 1
         return
      endif

*** phase space for inital state photons
      phspis = prodomega
***      
      betaq  = sqrt(1.d0-qone2/qone(0)**2)
      gammaq = 1.d0/sqrt(1.d0-betaq**2)

cc      print*,(1.d0-betaq)*gammaq,sqrt((1.d0-betaq)/(1.d0+betaq))
      
!! I think now it's better to generate fs momenta in the CRF where p3 is along z, then I boost back where qone is at rest
!! and I rotate everything back

      dir3(0) =  sqrt(qone2)*0.5
      dir3(1) =  0.d0
      dir3(2) =  0.d0
      dir3(3) =  sqrt((1.d0 - m1/dir3(0))*(1.d0 + m1/dir3(0)))*dir3(0)
      dir4(0) =  dir3(0)
      dir4(1) =  0.d0
      dir4(2) =  0.d0
      dir4(3) = -dir3(3) ! m1 = m2

      dir3(1) = dir3(3)*direm(1)
      dir3(2) = dir3(3)*direm(2)
      dir3(3) = dir3(3)*direm(3)      
      dir4    = -dir3
      dir4(0) = -dir4(0)

** omin for FS photons, omin it was for IS photons
!  this is really a not trivial task, because ominfs is correct in the frame where q is at rest
!  but not where p3+p4 is at rest and where I generate photons! This affects also Hto4f. Think better.
!  ** Update: I solved this by generating the energy from 0 for FS photons, see the routine
      ominfs = gammaq * omin * ( 1.d0 - betaq)

c      ominfs = omin
      
!     nota gamma * (1-b) = (1-b)/sqrt((1-b)*(1+b)) = sqrt((1-b)/(1+b))
      
c      ominfs = ominfs * 0.5d0

c      btest  = ominfs/sqrt(qone2+ominfs**2)
c      bah = (dir3(0)*0.02d0)**2
c      btest = (1.d0-ominfs**2/bah)/(1.d0+ominfs**2/bah)
c      ominfs = ominfs * sqrt((1.d0-btest)/(1.d0+btest))
c      print*,sqrt((1.d0-btest)/(1.d0+btest))
      
      omaxfs = dir3(0)          ! * gammaq * (1.d0+betaq)

** final state photons
      konefs      = 0.d0
      prodomegafs = 1.d0
      if (n3.gt.0) then
         do k = 1,n3
            if (omaxfs.lt.ominfs) then
               phsp = 0.d0
               w    = 0.d0
               ie = 1
               return
            endif

            csi2(1) = babayaganlo_rndm(0)
            csi2(2) = babayaganlo_rndm(0)
            
            call collinear_simple(0.d0,csi2(1),csi2(2),dir3,vers,wverg)
            w = w * wverg
            call photon_energy_FS(ominfs,omaxfs,en,wen)
            w = w * wen
            ophfs(k)   = en
            qphfs(k,0) = ophfs(k) 
            qphfs(k,1) = ophfs(k)*vers(1)
            qphfs(k,2) = ophfs(k)*vers(2)
            qphfs(k,3) = ophfs(k)*vers(3)

            do i = 0,3
               konefs(i) = konefs(i) + qphfs(k,i)
            enddo
            omaxfs      = omaxfs - en
            prodomegafs = prodomegafs *0.5*en
         enddo
      endif
      if (n4.gt.0) then
         do k = n3+1,n3+n4
            if (omaxfs.lt.ominfs) then
               phsp = 0.d0
               w    = 0.d0
               ie = 1
               return
            endif

            csi2(1) = babayaganlo_rndm(0)
            csi2(2) = babayaganlo_rndm(0)
            
            call collinear_simple(0.d0,csi2(1),csi2(2),dir4,vers,wverg)
            w = w * wverg
            call photon_energy_FS(ominfs,omaxfs,en,wen)
            w = w * wen
            ophfs(k)   = en
            qphfs(k,0) = ophfs(k) 
            qphfs(k,1) = ophfs(k)*vers(1)
            qphfs(k,2) = ophfs(k)*vers(2)
            qphfs(k,3) = ophfs(k)*vers(3)

            do i = 0,3
               konefs(i) = konefs(i) + qphfs(k,i)
            enddo

            omaxfs      = omaxfs - en
            prodomegafs = prodomegafs *0.5*en
         enddo
      endif

      E3 = (sqrt(qone2+tridot(konefs,konefs))-konefs(0))*0.5d0
      if (E3.lt.m1) then
         ie = 1
         w  = 0.d0
         phsp = 0.d0
         return
      endif
      
      bfs   = sqrt((1.d0-m1/E3)*(1.d0+m1/E3))
      
      p3(0) = E3
      p3(1) = E3*bfs*direm(1) !0.d0
      p3(2) = E3*bfs*direm(2)  !0.d0
      p3(3) = bfs*E3*direm(3)
      
      p4    = -p3
      p4(0) = -p4(0)

      sp     = 4.d0*E3*E3 ! = dot(p3+p4,p3+p4)
      phspfs = 0.25d0*bfs *0.5d0*sp/qone2 *
     .         prodomegafs*1.d0/(1.d0+konefs(0)/sqrt(sp))

      phsp = duepigrechi*phspis*phspfs
      
**** at this stage p3+p4 is at rest, now I go where p3+p4+konefs (i.e. qone) is at rest
      ptld = p3+p4+konefs

      call new_boost(ptld,p3,p3,1)
      call new_boost(ptld,p4,p4,1)
      if (nfs.gt.0) then
         do k = 1,nfs
            dir3 = qphfs(k,:)
            call new_boost(ptld,dir3,dir3,1)
            qphfs(k,:) = dir3
         enddo
      endif
      
** and now I boost in the lab, where qone has its components
      call new_boost(qone,p3,p3,-1)
      call new_boost(qone,p4,p4,-1)
      if (nfs.gt.0) then
         do k = 1,nfs
            dir3 = qphfs(k,:)
            call new_boost(qone,dir3,dir3,-1)
            qphfs(k,:) = dir3
!! checking in the lab that the FS photons are > omin
            if (qphfs(k,0).lt.omin) then
               ie   = 1
               w    = 0.d0
               phsp = 0.d0
               return
            endif
!!
         enddo
      endif
****
      if (nfs.gt.0) then
         do k = 1,nfs
            qph(nis+k,:) = qphfs(k,:)
            kone = kone + qphfs(k,:)
         enddo
      endif

***   regulator(s) ***      
      anum = 1.d0
      if (n1.gt.0) then
         do k = 1,n1
            anum = anum /dot(p1,qph(k,:)) !* p1(0)*qph(k,0)
         enddo
      endif
      if (n2.gt.0) then
         do k = n1+1,n1+n2
            anum = anum /dot(p2,qph(k,:)) !* p2(0)*qph(k,0)
         enddo
      endif
      if (n3.gt.0) then
         do k = n1+n2+1,n1+n2+n3
            anum = anum /dot(p3,qph(k,:)) !* p3(0)*qph(k,0)
         enddo
      endif
      if (n4.gt.0) then
         do k = n1+n2+n3+1,n1+n2+n3+n4
            anum = anum /dot(p4,qph(k,:)) !* p4(0)*qph(k,0)
         enddo
      endif

      call handregulator(nphot,p1,p2,p3,p4,qph(1:nphot,:),den)
c$$$      den = 1.d0
c$$$      do k = 1,nphot
c$$$         den = den * (
c$$$     .         1.d0/dot(p1,qph(k,:)) !* p1(0)*qph(k,0)
c$$$     .       + 1.d0/dot(p2,qph(k,:)) !* p2(0)*qph(k,0)
c$$$     .       + 1.d0/dot(p3,qph(k,:)) !* p3(0)*qph(k,0)
c$$$     .       + 1.d0/dot(p4,qph(k,:)) !* p4(0)*qph(k,0)
c$$$     .        )
c$$$      enddo

      regulator = anum/den * 4**nphot
      w         = w * regulator

      return
      end
*************************************************************************************
**********************************************************
      subroutine handregulator(ng,p1,p2,p3,p4,qph,den)
      implicit double precision (a-h,m,o-z)

      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3)
      dimension qph(ng,0:3),q1(0:3),q2(0:3),q3(0:3)
      dimension p12(0:3),p34(0:3),pz1(0:3),pz2(0:3)

      dimension p14(0:3),p23(0:3),pz12(0:3),pz34(0:3)
      dimension pz14(0:3),pz23(0:3)

      dimension p12q1(0:3),p12q2(0:3)
      dimension p12q1q2(0:3)
      dimension p34q1(0:3),p34q2(0:3)
      dimension p34q1q2(0:3)
      dimension coll1(ng),coll2(ng),coll3(ng),coll4(ng)
      dimension collt(4,ng)
      dimension pt(0:3,4)
      integer   pow4(0:ng)
      integer   pow2(0:ng)

      common/cthcommon/cthc
      
      pow2(0) = 1
      do k = 1,ng
         pow2(k)    = pow2(k-1)*2
         collt(1,k) = 1.d0/dot(p1,qph(k,:))! * p1(0)*qph(0,k)
         collt(2,k) = 1.d0/dot(p2,qph(k,:))! * p2(0)*qph(0,k)
         collt(3,k) = 1.d0/dot(p3,qph(k,:))! * p3(0)*qph(0,k)
         collt(4,k) = 1.d0/dot(p4,qph(k,:))! * p4(0)*qph(0,k) 
      enddo

* OPTIMIZED
      p12 = p1+p2
c      p34 = p3+p4 ! not needed here
      den = 0.d0
      do j = 0,2**ng-1
         pz12 = p12
c         pz34 = p34 ! not needed here
         prod = 1.d0
         do k = 1,ng
            iz = mod(j,pow2(k)) / pow2(k-1) + 1
            if (iz.eq.1) then
c               pz12  = pz12 + qph(k,:)
               pz12  = pz12 - qph(k,:) ! here I need the -...
               prod  = prod * (collt(1,k)+collt(2,k))
            else
c               pz34  = pz34 + qph(k,:) ! not needed here
               prod = prod * (collt(3,k)+collt(4,k))
            endif
         enddo
c     prod = prod*bw(dot(pz12,pz12))*bw(dot(pz34,pz34)) ! to be modified for bhabha
cc         prod = prod * fakeBW(dot(pz12,pz12))
cc         prod = prod * sdif(dot(pz12,pz12),cthc)
cc         prod = prod * spezzasdif(dot(pz12,pz12))
         den  = den + prod
      enddo

      return

      end
      
