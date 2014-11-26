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
!          call ranlux(csi,1)
         csi(1)  = babayaganlo_rndm(1)
!          print *,'csi(1)  = babayaganlo_rndm(1)', csi(1)
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
!       call ranlux(csi,1)
!       print *,csi(1)

      csi(1)  = babayaganlo_rndm(1)
!          print *,'csi(1)  = babayaganlo_rndm(1)', csi(1)

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

!       call ranlux(csi,1)
      csi(1)  = babayaganlo_rndm(1)
!          print *,'csi(1)  = babayaganlo_rndm(1)', csi(1)

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
!                call ranlux(csi2,2)
               csi2(1)  = babayaganlo_rndm(1)
               csi2(2)  = babayaganlo_rndm(1)
!          print *,'csi2(1)  = babayaganlo_rndm(1)', csi2(1)
!          print *,'csi2(2)  = babayaganlo_rndm(1)', csi2(2)

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
!                call ranlux(csi2,2)
               csi2(1)  = babayaganlo_rndm(1)
               csi2(2)  = babayaganlo_rndm(1)
!          print *,'csi2(1)  = babayaganlo_rndm(1)', csi2(1)
!          print *,'csi2(2)  = babayaganlo_rndm(1)', csi2(2)

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
!                   call ranlux(csi2,2)
                  csi2(1)  = babayaganlo_rndm(1)
                  csi2(2)  = babayaganlo_rndm(1)
!          print *,'csi2(1)  = babayaganlo_rndm(1)', csi2(1)
!          print *,'csi2(2)  = babayaganlo_rndm(1)', csi2(2)
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
!                   call ranlux(csi2,2)
                  csi2(1)  = babayaganlo_rndm(1)
                  csi2(2)  = babayaganlo_rndm(1)
!          print *,'csi2(1)  = babayaganlo_rndm(1)', csi2(1)
!          print *,'csi2(2)  = babayaganlo_rndm(1)', csi2(2)

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
!                   call ranlux(csi2,2)
                  csi2(1)  = babayaganlo_rndm(1)
                  csi2(2)  = babayaganlo_rndm(1)
!          print *,'csi2(1)  = babayaganlo_rndm(1)', csi2(1)
!          print *,'csi2(2)  = babayaganlo_rndm(1)', csi2(2)

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
!                   call ranlux(csi2,2)
                  csi2(1)  = babayaganlo_rndm(1)
                  csi2(2)  = babayaganlo_rndm(1)
!          print *,'csi2(1)  = babayaganlo_rndm(1)', csi2(1)
!          print *,'csi2(2)  = babayaganlo_rndm(1)', csi2(2)

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
!          call ranlux(csi,1)
         csi(1)  = babayaganlo_rndm(1)
!          print *,'csi(1)  = babayaganlo_rndm(1)', csi(1)

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
*
      subroutine get_pattern(ncharged,n,ep)
!       last edited by torben.ferber (2014-Nov-14)
!       included c++ random generator, avoid implicit declaration
      double precision babayaganlo_rndm
      external babayaganlo_rndm
      integer n,ep(n),ncharged
      real*4 csi(1)
      do k = 1,n
         csi(1)  = babayaganlo_rndm(1)

         ep(k) = 1.d0*ncharged*csi(1) + 1
         if (ep(k).gt.ncharged) ep(k) = ncharged ! to avoid round-offs...
      enddo
      return
      end
