      subroutine closest(ng,qph,p1,p2,p3,p4,q1,q2,q3,q4,iclose)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      parameter (maxph = 40)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3)
      dimension q1(0:3),q2(0:3),q3(0:3),q4(0:3)
      character*2 fs
      common/finalstate/fs
      dimension qph(maxph,0:3)
      dimension iclose(maxph)
      integer emission(maxph) 
      common/radpattern/nph(4)

      do k = 1,maxph
         iclose(k) = 0
      enddo
      
      do k = 0,3
         q1(k) = 0.d0
         q2(k) = 0.d0
         q3(k) = 0.d0
         q4(k) = 0.d0
      enddo

c      p1modmu = 1.d0/sqrt(tridot(p1,p1))
c      p2modmu = 1.d0/sqrt(tridot(p2,p2))
c      p3modmu = 1.d0/sqrt(tridot(p3,p3))
c      p4modmu = 1.d0/sqrt(tridot(p4,p4))
      
      do k = 1,ng
c---
c         if (k.le.nph(1)) emission(k) = 1
c         if (k.gt.nph(1).and.k.le.(nph(1)+nph(2))) emission(k) = 2
c         if (k.gt.(nph(1)+nph(2)).and.k.le.(nph(1)+nph(2)+nph(3)))
c     .        emission(k) = 3
c         if (k.gt.(nph(1)+nph(2)+nph(3))) emission(k) = 4
c---
         do i = 0,3
            q(i) = qph(k,i)
         enddo     
c         c1 = tridot(p1,q)*p1modmu/q(0)
c         c2 = tridot(p2,q)*p2modmu/q(0)
c         c3 = tridot(p3,q)*p3modmu/q(0)
c         c4 = tridot(p4,q)*p4modmu/q(0)        

         c1 = dot(p1,q)
         c2 = dot(p2,q)
         c3 = dot(p3,q)
         c4 = dot(p4,q)
         
         if (fs.eq.'ee'.or.fs.eq.'mm') then
            if (c1.lt.c2.and.c1.lt.c3.and.c1.lt.c4) iclose(k) = 1
            if (c2.lt.c1.and.c2.lt.c3.and.c2.lt.c4) iclose(k) = 2
            if (c3.lt.c2.and.c3.lt.c1.and.c3.lt.c4) iclose(k) = 3
            if (c4.lt.c1.and.c4.lt.c2.and.c4.lt.c3) iclose(k) = 4
         elseif (fs.eq.'gg') then
            if (c1.lt.c2) iclose(k) = 1
            if (c2.lt.c1) iclose(k) = 2
         endif
!!!
ccc         iclose(k) = emission(k)
!!!         
         ic = iclose(k)
         if (ic.eq.1) then
            do j = 0,3
               q1(j) = q1(j) + q(j)
            enddo
         endif            
         if (ic.eq.2) then
            do j = 0,3
               q2(j) = q2(j) + q(j)
            enddo
         endif
         if (ic.eq.3) then
            do j = 0,3
               q3(j) = q3(j) + q(j)
            enddo
         endif
         if (ic.eq.4) then
            do j = 0,3
               q4(j) = q4(j) + q(j)
            enddo
         endif
      enddo
      return
      end
*

      subroutine closest_BCK(ng,qph,p1,p2,p3,p4,q1,q2,q3,q4,iclose)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      parameter (maxph = 40)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3)
      dimension q1(0:3),q2(0:3),q3(0:3),q4(0:3)
      character*2 fs
      common/finalstate/fs
      dimension qph(maxph,0:3)
      dimension iclose(maxph)
      integer emission(maxph) 
      common/radpattern/nph(4)

      do k = 1,maxph
         iclose(k) = 0
      enddo
      
      do k = 0,3
         q1(k) = 0.d0
         q2(k) = 0.d0
         q3(k) = 0.d0
         q4(k) = 0.d0
      enddo

      p1modmu = 1.d0/sqrt(tridot(p1,p1))
      p2modmu = 1.d0/sqrt(tridot(p2,p2))
      p3modmu = 1.d0/sqrt(tridot(p3,p3))
      p4modmu = 1.d0/sqrt(tridot(p4,p4))
      
      do k = 1,ng
c---
         if (k.le.nph(1)) emission(k) = 1
         if (k.gt.nph(1).and.k.le.(nph(1)+nph(2))) emission(k) = 2
         if (k.gt.(nph(1)+nph(2)).and.k.le.(nph(1)+nph(2)+nph(3)))
     .        emission(k) = 3
         if (k.gt.(nph(1)+nph(2)+nph(3))) emission(k) = 4
c---
         do i = 0,3
            q(i) = qph(k,i)
         enddo     
         c1 = tridot(p1,q)*p1modmu/q(0)
         c2 = tridot(p2,q)*p2modmu/q(0)
         c3 = tridot(p3,q)*p3modmu/q(0)
         c4 = tridot(p4,q)*p4modmu/q(0)        

         if (fs.eq.'ee'.or.fs.eq.'mm') then
            if (c1.gt.c2.and.c1.gt.c3.and.c1.gt.c4) iclose(k) = 1
            if (c2.gt.c1.and.c2.gt.c3.and.c2.gt.c4) iclose(k) = 2
            if (c3.gt.c2.and.c3.gt.c1.and.c3.gt.c4) iclose(k) = 3
            if (c4.gt.c1.and.c4.gt.c2.and.c4.gt.c3) iclose(k) = 4
         elseif (fs.eq.'gg') then
            if (c1.gt.c2) iclose(k) = 1
            if (c2.gt.c1) iclose(k) = 2
         endif
!!!
ccc         iclose(k) = emission(k)
!!!         
         ic = iclose(k)
         if (ic.eq.1) then
            do j = 0,3
               q1(j) = q1(j) + q(j)
            enddo
         endif            
         if (ic.eq.2) then
            do j = 0,3
               q2(j) = q2(j) + q(j)
            enddo
         endif
         if (ic.eq.3) then
            do j = 0,3
               q3(j) = q3(j) + q(j)
            enddo
         endif
         if (ic.eq.4) then
            do j = 0,3
               q4(j) = q4(j) + q(j)
            enddo
         endif
      enddo
      return
      end
*
      subroutine searchclosest(q,p1,p2,p3,p4,ic)
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3),q(0:3)
      p1modmu = 1.d0/sqrt(tridot(p1,p1))
      p2modmu = 1.d0/sqrt(tridot(p2,p2))
      p3modmu = 1.d0/sqrt(tridot(p3,p3))
      p4modmu = 1.d0/sqrt(tridot(p4,p4))
      c1 = tridot(p1,q)*p1modmu/q(0)
      c2 = tridot(p2,q)*p2modmu/q(0)
      c3 = tridot(p3,q)*p3modmu/q(0)
      c4 = tridot(p4,q)*p4modmu/q(0)        
      if (c1.gt.c2.and.c1.gt.c3.and.c1.gt.c4) ic = 1
      if (c2.gt.c1.and.c2.gt.c3.and.c2.gt.c4) ic = 2
      if (c3.gt.c2.and.c3.gt.c1.and.c3.gt.c4) ic = 3
      if (c4.gt.c1.and.c4.gt.c2.and.c4.gt.c3) ic = 4
      return
      end
**************************
      subroutine maptozero2(p1,p2,p3,p4,q1,q2,q3,q4,
     .     p1r,p2r,p3r,p4r,ierror)
! written by CMCC, last modified 18/9/2007
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3)
      dimension q1(0:3),q2(0:3),q3(0:3),q4(0:3)
      dimension p1r(0:3),p2r(0:3),p3r(0:3),p4r(0:3)
      dimension q(0:3),p(0:3),ptmp(0:3)
      double precision masses(4)
      character*2 fs
      common/momentainitial/pin1(0:3),pin2(0:3)
      common/mapto0_2first/ifirst
      common/iclosest/iclose(40)
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      data ifirst /0/

      ierror = 0

      masses(1) = ame
      masses(2) = ame
      masses(3) = ame
      masses(4) = ame
      if (fs.eq.'gg') then
         masses(3) = 0.d0
         masses(4) = 0.d0
      endif
      if (fs.eq.'mm') then
         masses(3) = ammu
         masses(4) = ammu
      endif

      do k=0,3
         q(k) = p1(k) + p2(k)
         p(k) = p3(k) + p4(k)
      enddo

      ic = iclose(1)
      shat = dot(q,q)
      sshat = sqrt(shat)    ! invariant

c      if (ic.eq.2.or.ic.eq.4) then
      if (ic.eq.1.or.ic.eq.3) then
c      if (ic.lt.10) then
         do i = 0,3
            ptmp(i) = p1(i) - p3(i) - q3(i)
         enddo
         that = dot(ptmp,ptmp)
         cstar = 1.d0+2.d0*that/shat

         if (cstar.le.-1.d0) then 
            cstar = -1.d0 + 0.1d-8
         endif
         sstar = sqrt(1.d0-cstar**2)

         do i = 0,3
            ptmp(i) = p3(i) + q3(i)
         enddo
         ph = getphiangle(ptmp)

         sph = sin(ph)
         cph = cos(ph)
         
         p3r(0) = 1.d0
         p3r(1) = sph*sstar
         p3r(2) = cph*sstar
         p3r(3) = cstar

         p4r(0) = 1.d0
         p4r(1) = -p3r(1)
         p4r(2) = -p3r(2)
         p4r(3) = -p3r(3)

      else

         do i = 0,3
            ptmp(i) = p2(i) - p4(i) - q4(i)
         enddo
         that = dot(ptmp,ptmp)
         cstar = 1.d0+2.d0*that/shat
         if (cstar.le.-1.d0) cstar = -1.d0 + 0.1d-8

         cstar = -cstar     !!!!!!!!!

         sstar = sqrt(1.d0-cstar**2)

         do i = 0,3
            ptmp(i) = p4(i) + q4(i)
         enddo
         ph = getphiangle(ptmp)
         sph = sin(ph)
         cph = cos(ph)
         
         p4r(0) = 1.d0
         p4r(1) = sph*sstar
         p4r(2) = cph*sstar
         p4r(3) = cstar

         p3r(0) = 1.d0
         p3r(1) = -p4r(1)
         p3r(2) = -p4r(2)
         p3r(3) = -p4r(3)

      endif
c      sshat = sqrt(dot(q,q)) ! invariant

      e = sshat/2.d0
      ei   = 2.d0*e

      ea = (ei**2 + masses(1)**2 -masses(2)**2)/2.d0/ei
      eb = ei - ea

      if (ea.lt.masses(1).or.eb.lt.masses(2)) then
         print*,' WARNING IN MAPTOZERO'
         print*,' UNPHYSICAL CONFIGURATION, KILLING THIS EVENT!'
         print*,'ea,eb,masses',ea,eb,masses
         if (ea.lt.masses(1)) ea = masses(1)
         if (eb.lt.masses(2)) eb = masses(2)
         ierror = 1
      endif

      beta1 = sqrt(1.d0 - masses(1)**2/ea**2)
      beta2 = sqrt(1.d0 - masses(2)**2/eb**2)

      p1r(0) = ea
      p1r(1) = 0.d0
      p1r(2) = 0.d0
      p1r(3) = ea*beta1

      p2r(0) =  eb
      p2r(1) =  0.d0
      p2r(2) =  0.d0
      p2r(3) = -eb*beta2

      abig = ei**2 + masses(3)**2 - masses(4)**2
      bbig = -2.d0 * ei 
      cbig = 0.d0
      arg  = abig**2*bbig**2-masses(3)**2*bbig**2*(bbig**2-cbig**2)

      p1mod = abig*cbig + sqrt(arg)
      p1mod = p1mod / (bbig**2 - cbig**2)

      e1    = sqrt(masses(3)**2 + p1mod**2)
      beta3 = p1mod/e1  

      p3mod = sqrt(tridot(p3r,p3r))
      p4mod = sqrt(tridot(p4r,p4r))

      p3r(0) = e1
      p3r(1) = e1*beta3 * p3r(1)/p3mod
      p3r(2) = e1*beta3 * p3r(2)/p3mod
      p3r(3) = e1*beta3 * p3r(3)/p3mod

      p4r(0) = ei - p3r(0)
      p4r(1) = -p3r(1)
      p4r(2) = -p3r(2)
      p4r(3) = -p3r(3)
      return
      end
**************************
      subroutine maptozero(p1,p2,p3,p4,p1r,p2r,p3r,p4r,ierror)
! written by CMCC, last modified 18/9/2007
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3)
      dimension p1r(0:3),p2r(0:3),p3r(0:3),p4r(0:3)
      dimension q(0:3),p(0:3)
      double precision masses(4)
      character*2 fs
      common/mapto0first/ifirst
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      data ifirst /0/
      ierror = 0
      masses(1) = ame
      masses(2) = ame
      masses(3) = ame
      masses(4) = ame
      if (fs.eq.'gg') then
         masses(3) = 0.d0
         masses(4) = 0.d0
      endif
      if (fs.eq.'mm') then
         masses(3) = ammu
         masses(4) = ammu
      endif

      do k=0,3
         q(k) = p1(k) + p2(k)
         p(k) = p3(k) + p4(k)
      enddo

      call new_boost(p,p3,p3r,1)
      call new_boost(p,p4,p4r,1)
      
      sshat = sqrt(dot(q,q)) ! invariant

      e = sshat/2.d0
      ei   = 2.d0*e

      ea = (ei**2 + masses(1)**2 -masses(2)**2)/2.d0/ei
      eb = ei - ea

      if (ea.lt.masses(1).or.eb.lt.masses(2)) then
         print*,' WARNING IN MAPTOZERO'
         print*,' UNPHYSICAL CONFIGURATION, KILLING THIS EVENT!'
         print*,'ea,eb,masses',ea,eb,masses
         if (ea.lt.masses(1)) ea = masses(1)
         if (eb.lt.masses(2)) eb = masses(2)
         ierror = 1
      endif

      beta1 = sqrt(1.d0 - masses(1)**2/ea**2)
      beta2 = sqrt(1.d0 - masses(2)**2/eb**2)

      p1r(0) = ea
      p1r(1) = 0.d0
      p1r(2) = 0.d0
      p1r(3) = ea*beta1

      p2r(0) =  eb
      p2r(1) =  0.d0
      p2r(2) =  0.d0
      p2r(3) = -eb*beta2

      abig = ei**2 + masses(3)**2 - masses(4)**2
      bbig = -2.d0 * ei 
      cbig = 0.d0
      arg  = abig**2*bbig**2-masses(3)**2*bbig**2*(bbig**2-cbig**2)

      p1mod = abig*cbig + sqrt(arg)
      p1mod = p1mod / (bbig**2 - cbig**2)

      e1    = sqrt(masses(3)**2 + p1mod**2)
      beta3 = p1mod/e1  

      p3mod = sqrt(tridot(p3r,p3r))
      p4mod = sqrt(tridot(p4r,p4r))

      p3r(0) = e1
      p3r(1) = e1*beta3 * p3r(1)/p3mod
      p3r(2) = e1*beta3 * p3r(2)/p3mod
      p3r(3) = e1*beta3 * p3r(3)/p3mod

      p4r(0) = ei - p3r(0)
      p4r(1) = -p3r(1)
      p4r(2) = -p3r(2)
      p4r(3) = -p3r(3)
      return
      end
*******************
      subroutine maptozero_TEST(p1,p2,p3,p4,
     .     p1r,p2r,p3r,p4r,ierror)
! written by CMCC, last modified 18/9/2007
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3)
      dimension q1(0:3),q2(0:3),q3(0:3),q4(0:3)
      dimension p1r(0:3),p2r(0:3),p3r(0:3),p4r(0:3)
      dimension q(0:3),p(0:3),ptmp(0:3)
      double precision masses(4)
      character*2 fs
      common/momentainitial/pin1(0:3),pin2(0:3)
      common/mapto0newfirst/ifirst
      common/iclosest/iclose(40)
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      data ifirst /0/

      ierror = 0

      masses(1) = ame
      masses(2) = ame
      masses(3) = ame
      masses(4) = ame
      if (fs.eq.'gg') then
         masses(3) = 0.d0
         masses(4) = 0.d0
      endif
      if (fs.eq.'mm') then
         masses(3) = ammu
         masses(4) = ammu
      endif

      do k=0,3
         q(k) = p1(k) + p2(k)
         p(k) = p3(k) + p4(k)
      enddo

      call new_boost(p,p3,p3r,1)
      call new_boost(p,p4,p4r,1)

cTEST
c$$$      if (p3(0).ge.p4(0)) then
c$$$         do i = 0,3
c$$$            p3r(i) = p3(i)
c$$$         enddo
c$$$         p4r(0) = p3r(0)
c$$$         do i = 1,3
c$$$            p4r(i) =-p3(i)
c$$$         enddo
c$$$      else
c$$$         do i = 0,3
c$$$            p4r(i) = p4(i)
c$$$         enddo
c$$$         p3r(0) = p4r(0)
c$$$         do i = 1,3
c$$$            p3r(i) =-p4(i)
c$$$         enddo
c$$$      endif
c$$$      if (angledeg(p3,p4).le.140d0) then
c$$$         th1 = angledeg(p3,pin1)
c$$$         th2 = angledeg(p4,pin1)
c$$$         ph1 = getphiangle(p3)*180.d0/pi
c$$$         ph2 = getphiangle(p4)*180.d0/pi
c$$$         if (th1.lt.90d0) then
c$$$            if (ph1.lt.90.d0) then
c$$$               i1 = 1
c$$$            elseif (ph1.gt.90.d0.and.ph1.lt.180) then
c$$$               i1 = 2
c$$$            elseif (ph1.gt.180.d0.and.ph1.lt.270) then
c$$$               i1 = 3
c$$$            elseif (ph1.gt.270.d0) then
c$$$               i1 = 4
c$$$            endif
c$$$         else
c$$$            if (ph1.lt.90.d0) then
c$$$               i1 = 5
c$$$            elseif (ph1.gt.90.d0.and.ph1.lt.180) then
c$$$               i1 = 6
c$$$            elseif (ph1.gt.180.d0.and.ph1.lt.270) then
c$$$               i1 = 7
c$$$            elseif (ph1.gt.270.d0) then
c$$$               i1 = 8
c$$$            endif
c$$$         endif
c$$$*
c$$$         if (th2.lt.90d0) then
c$$$            if (ph2.lt.90.d0) then
c$$$               i2 = 1
c$$$            elseif (ph2.gt.90.d0.and.ph2.lt.180) then
c$$$               i2 = 2
c$$$            elseif (ph2.gt.180.d0.and.ph2.lt.270) then
c$$$               i2 = 3
c$$$            elseif (ph2.gt.270.d0) then
c$$$               i2 = 4
c$$$            endif
c$$$         else
c$$$            if (ph2.lt.90.d0) then
c$$$               i2 = 5
c$$$            elseif (ph2.gt.90.d0.and.ph2.lt.180) then
c$$$               i2 = 6
c$$$            elseif (ph2.gt.180.d0.and.ph2.lt.270) then
c$$$               i2 = 7
c$$$            elseif (ph2.gt.270.d0) then
c$$$               i2 = 8
c$$$            endif
c$$$         endif
c$$$
      ic = iclose(1)

      shat = dot(q,q)
      sshat = sqrt(shat)    ! invariant

      if (ic.eq.2.or.ic.eq.4) then
         do i = 0,3
            ptmp(i) = pin1(i) - p3(i)
         enddo
         that = dot(ptmp,ptmp)
         cstar = 1.d0+2.d0*that/shat
c         print*,'NON FUNZIONA PERCHE IL COS puo'' essere fuori...'
c         print*,'1',cstar,that,sshat

         if (cstar.le.-1.d0) cstar = -1.d0 + 0.1d-8

         if (abs(cstar).gt.1.d0) then
            print*,'------ aaa'
            print*,ic
            print*,pin1
            print*,pin2
            print*,p3
            print*,p4
            do i = 0,3
               ptmp(i) = pin2(i) + pin1(i) - p3(i) - p4(i)
            enddo
            print*,ptmp
            print*,'------ bbb'
         endif


         sstar = sqrt(1.d0-cstar**2)
         ph = getphiangle(p3)
         sph = sin(ph)
         cph = cos(ph)
         
         p3r(0) = 1.d0
         p3r(1) = sph*sstar
         p3r(2) = cph*sstar
         p3r(3) = cstar

         p4r(0) = 1.d0
         p4r(1) = -p3r(1)
         p4r(2) = -p3r(2)
         p4r(3) = -p3r(3)

      else

         do i = 0,3
            ptmp(i) = pin2(i) - p4(i)
         enddo
         that = dot(ptmp,ptmp)
         cstar = 1.d0+2.d0*that/shat
c         print*,'2',cstar,that,sshat
 
         if (cstar.le.-1.d0) cstar = -1.d0 + 0.1d-8

         cstar = -cstar !!!!

         sstar = sqrt(1.d0-cstar**2)
         ph = getphiangle(p4)
         sph = sin(ph)
         cph = cos(ph)
         
         p4r(0) = 1.d0
         p4r(1) = sph*sstar
         p4r(2) = cph*sstar
         p4r(3) = cstar

         p3r(0) = 1.d0
         p3r(1) = -p4r(1)
         p3r(2) = -p4r(2)
         p3r(3) = -p4r(3)

      endif
      
c$$$      if (ic.eq.2.or.ic.eq.4) then
c$$$         do i = 0,3
c$$$            p3r(i) = p3(i)
c$$$         enddo
c$$$         p4r(0) = p3r(0)
c$$$         do i = 1,3
c$$$            p4r(i) =-p3(i)
c$$$         enddo
c$$$      else
c$$$         do i = 0,3
c$$$            p4r(i) = p4(i)
c$$$         enddo
c$$$         p3r(0) = p4r(0)
c$$$         do i = 1,3
c$$$            p3r(i) =-p4(i)
c$$$         enddo
c$$$      endif

c$$$      endif
cTEST      
      
      sshat = sqrt(dot(q,q)) ! invariant

      e = sshat/2.d0
      ei   = 2.d0*e

      ea = (ei**2 + masses(1)**2 -masses(2)**2)/2.d0/ei
      eb = ei - ea

      if (ea.lt.masses(1).or.eb.lt.masses(2)) then
         print*,' WARNING IN MAPTOZERO'
         print*,' UNPHYSICAL CONFIGURATION, KILLING THIS EVENT!'
         print*,'ea,eb,masses',ea,eb,masses
         if (ea.lt.masses(1)) ea = masses(1)
         if (eb.lt.masses(2)) eb = masses(2)
         ierror = 1
      endif

      beta1 = sqrt(1.d0 - masses(1)**2/ea**2)
      beta2 = sqrt(1.d0 - masses(2)**2/eb**2)

      p1r(0) = ea
      p1r(1) = 0.d0
      p1r(2) = 0.d0
      p1r(3) = ea*beta1

      p2r(0) =  eb
      p2r(1) =  0.d0
      p2r(2) =  0.d0
      p2r(3) = -eb*beta2

      abig = ei**2 + masses(3)**2 - masses(4)**2
      bbig = -2.d0 * ei 
      cbig = 0.d0
      arg  = abig**2*bbig**2-masses(3)**2*bbig**2*(bbig**2-cbig**2)

      p1mod = abig*cbig + sqrt(arg)
      p1mod = p1mod / (bbig**2 - cbig**2)

      e1    = sqrt(masses(3)**2 + p1mod**2)
      beta3 = p1mod/e1  

      p3mod = sqrt(tridot(p3r,p3r))
      p4mod = sqrt(tridot(p4r,p4r))

      p3r(0) = e1
      p3r(1) = e1*beta3 * p3r(1)/p3mod
      p3r(2) = e1*beta3 * p3r(2)/p3mod
      p3r(3) = e1*beta3 * p3r(3)/p3mod

      p4r(0) = ei - p3r(0)
      p4r(1) = -p3r(1)
      p4r(2) = -p3r(2)
      p4r(3) = -p3r(3)
      return
      end
**********************
      subroutine maptoone(is,bigq,p3,p4,q,p1r,p2r,p3r,p4r,qr)
! written by CMCC, last modified 18/9/2007
      implicit double precision (a-h,o-z)
      dimension bigq(0:3),p3(0:3),p4(0:3)
      dimension p1r(0:3),p2r(0:3),p3r(0:3),p4r(0:3)
      dimension q(0:3),p(0:3),qr(0:3),ptmp(0:3),ptmp2(0:3)
      double precision masses(4)
      character*2 fs
      common/finalstate/fs
      common/parameters/ame,ammu,convfac,alpha,pi
      common/momentainitial/pin1(0:3),pin2(0:3)
      common/counter_mapptoone/icountmto
      data icountmto /0/
      icountmto = icountmto + 1
! is = 1 q is initial state photon
! is = 0 q is final state photon
! This is identical to WP
      masses(1) = ame
      masses(2) = ame
      masses(3) = ame
      masses(4) = ame
      if (fs.eq.'gg') then
         masses(3) = 0.d0
         masses(4) = 0.d0
      endif
      if (fs.eq.'mm') then
         masses(3) = ammu
         masses(4) = ammu
      endif
      shat = dot(bigq,bigq)
      if (is.eq.1) then
         do k = 0,3
            p(k) = p3(k)+p4(k)
         enddo
         call new_boost(p,p3,p3r,1)
         call new_boost(p,p4,p4r,1)
         e = sqrt(shat)/2.d0

         ei   = 2.d0*e
         abig = ei**2 + masses(3)**2 - masses(4)**2
         bbig = -2.d0 * ei
         cbig = 0.d0
         arg  = abig**2*bbig**2-masses(3)**2*bbig**2*(bbig**2-cbig**2)

         p1mod = abig*cbig + sqrt(arg)
         p1mod = p1mod / (bbig**2 - cbig**2)

         e1    = sqrt(masses(3)**2 + p1mod**2)
         beta3 = p1mod/e1

         p3mod = sqrt(tridot(p3r,p3r))
         p4mod = sqrt(tridot(p4r,p4r))

         p3r(0) = e1
         p3r(1) = e1*beta3 * p3r(1)/p3mod
         p3r(2) = e1*beta3 * p3r(2)/p3mod
         p3r(3) = e1*beta3 * p3r(3)/p3mod

         p4r(0) = ei - p3r(0)
         p4r(1) = -p3r(1)
         p4r(2) = -p3r(2)
         p4r(3) = -p3r(3)
         ! p3r p4r to be intended in the bigq frame

         do k = 0,3
            p(k)    = bigq(k)+q(k)
            ptmp(k) = pin1(k)+pin2(k)-q(k)
         enddo

         scale = sqrt(dot(bigq,bigq)/dot(ptmp,ptmp))

         do k = 0,3
            qr(k) = scale*q(k)
            ptmp(k) = pin1(k)+pin2(k)
         enddo

         e = sqrt(dot(ptmp,ptmp))/2.d0*scale

         ei = 2.d0*e
         ea = (ei**2 + masses(1)**2 -masses(2)**2)/2.d0/ei
         eb = ei - ea
         beta1 = sqrt(1.d0 - masses(1)**2/ea**2)
         beta2 = sqrt(1.d0 - masses(2)**2/eb**2)

         beta1 = sqrt(1.d0 - masses(1)**2/ea**2)
         beta2 = sqrt(1.d0 - masses(2)**2/eb**2)

         p1r(0) = ea
         p1r(1) = 0.d0
         p1r(2) = 0.d0
         p1r(3) = ea*beta1

         p2r(0) = eb
         p2r(1) = 0.d0
         p2r(2) = 0.d0
         p2r(3) = -eb*beta2

         do k = 0,3
            ptmp(k) = p1r(k)+p2r(k)-qr(k)
         enddo
 !        call new_boost(ptmp,qr,qr,1)
         call new_boost(ptmp,p3r,p3r,-1)
         call new_boost(ptmp,p4r,p4r,-1)
      else

         e=sqrt(shat)/2.d0

         ei = 2.d0*e
         ea = (ei**2 + masses(1)**2 -masses(2)**2)/2.d0/ei
         eb = ei - ea
         beta1 = sqrt(1.d0 - masses(1)**2/ea**2)
         beta2 = sqrt(1.d0 - masses(2)**2/eb**2)

         beta1 = sqrt(1.d0 - masses(1)**2/ea**2)
         beta2 = sqrt(1.d0 - masses(2)**2/eb**2)

         p1r(0) = ea
         p1r(1) = 0.d0
         p1r(2) = 0.d0
         p1r(3) = ea*beta1

         p2r(0) =  eb
         p2r(1) =  0.d0
         p2r(2) =  0.d0
         p2r(3) = -eb*beta2

         do k = 0,3
            ptmp(k) = p3(k)+p4(k)
         enddo

         call new_boost(ptmp,p3,p3r,1)
         call new_boost(ptmp,p4,p4r,1)
         call new_boost(ptmp,q,qr,1)

         do k = 0,3
            p(k)    = bigq(k)-q(k)
            ptmp(k) = p3(k)+p4(k)+q(k)
         enddo

         scale = sqrt(dot(bigq,bigq)/dot(ptmp,ptmp))

         do k = 0,3
            qr(k)   = scale*qr(k)
            ptmp(k) = p4(k)+p3(k)
         enddo

         e = sqrt(dot(ptmp,ptmp))/2.d0*scale

         ei   =  2.d0*e
         abig =  ei**2 + masses(3)**2 - masses(4)**2
         bbig = -2.d0 * ei
         cbig =  0.d0
         arg  =  abig**2*bbig**2-masses(3)**2*bbig**2*(bbig**2-cbig**2)

         p1mod = abig*cbig + sqrt(arg)
         p1mod = p1mod / (bbig**2 - cbig**2)

         e1    = sqrt(masses(3)**2 + p1mod**2)
         beta3 = p1mod/e1

         p3mod = sqrt(tridot(p3r,p3r))
         p4mod = sqrt(tridot(p4r,p4r))

         p3r(0) = e1
         p3r(1) = e1*beta3 * p3r(1)/p3mod
         p3r(2) = e1*beta3 * p3r(2)/p3mod
         p3r(3) = e1*beta3 * p3r(3)/p3mod

         p4r(0) = ei - p3r(0) 

         p4r(1) = -p3r(1)
         p4r(2) = -p3r(2)
         p4r(3) = -p3r(3)
         ! p3r p4r to be intended in the current p frame

         do k = 0,3
            ptmp(k) = p3r(k)+p4r(k)+qr(k)
         enddo

         call new_boost(ptmp,p3r,p3r,1)
         call new_boost(ptmp,p4r,p4r,1)
         call new_boost(ptmp,qr,qr,1)
      endif
      return
      end
*
      subroutine maptozero_BCK(p1,p2,p3,p4,p1r,p2r,p3r,p4r)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3)
      dimension p1r(0:3),p2r(0:3),p3r(0:3),p4r(0:3)
      dimension q(0:3),p(0:3)

      do k=0,3
         q(k) = p1(k) + p2(k)
         p(k) = p3(k) + p4(k)
      enddo

      call new_boost(p,p3,p3r,1)
      call new_boost(p,p4,p4r,1)
      
      amel = sqrt(dot(p3,p3))
      
      sshat = sqrt(dot(q,q)) ! invariant

      p3mod = sqrt(tridot(p3r,p3r))
      p4mod = sqrt(tridot(p4r,p4r))

      e = sshat/2.d0
      beta = sqrt(1.d0 - amel**2/e**2)

      p1r(0) = e
      p1r(1) = 0.d0
      p1r(2) = 0.d0
      p1r(3) = e*beta

      p2r(0) =  e
      p2r(1) =  0.d0
      p2r(2) =  0.d0
      p2r(3) = -e*beta

      p3r(0) = e
      p3r(1) = e*beta * p3r(1)/p3mod
      p3r(2) = e*beta * p3r(2)/p3mod
      p3r(3) = e*beta * p3r(3)/p3mod

      p4r(0) = e
      p4r(1) = e*beta * p4r(1)/p4mod
      p4r(2) = e*beta * p4r(2)/p4mod
      p4r(3) = e*beta * p4r(3)/p4mod
      return
      end
*******
      subroutine maptoone_BCK(is,bigq,p3,p4,q,p1r,p2r,p3r,p4r,qr)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      dimension bigq(0:3),p3(0:3),p4(0:3)
      dimension p1r(0:3),p2r(0:3),p3r(0:3),p4r(0:3)
      dimension q(0:3),p(0:3),qr(0:3),ptmp(0:3)
! is = 1 q is initial state photon
! is = 0 q is final state photon

      shat = dot(bigq,bigq)
      amel = sqrt(dot(p3,p3))

      if (is.eq.1) then
         do k = 0,3
            p(k) = p3(k)+p4(k)
         enddo
         call new_boost(p,p3,p3r,1)
         call new_boost(p,p4,p4r,1)
         e = sqrt(shat)/2.d0
         beta = sqrt(1.d0 - amel**2/e**2)

         p3mod = sqrt(tridot(p3r,p3r))
         p4mod = sqrt(tridot(p4r,p4r))

         p3r(0) = e
         p3r(1) = e*beta * p3r(1)/p3mod
         p3r(2) = e*beta * p3r(2)/p3mod
         p3r(3) = e*beta * p3r(3)/p3mod

         p4r(0) = e
         p4r(1) = e*beta * p4r(1)/p4mod
         p4r(2) = e*beta * p4r(2)/p4mod
         p4r(3) = e*beta * p4r(3)/p4mod
         ! p3r p4r to be intended in the bigq frame

         call new_boost(bigq,p3r,p3r,-1)
         call new_boost(bigq,p4r,p4r,-1)

         do k = 0,3
            p(k) = bigq(k)+q(k)
         enddo
         
         e = sqrt(dot(p,p))/2.d0
         beta = sqrt(1.d0 - amel**2/e**2)

         p1r(0) = e
         p1r(1) = 0.d0
         p1r(2) = 0.d0
         p1r(3) = e*beta

         p2r(0) = e
         p2r(1) = 0.d0
         p2r(2) = 0.d0
         p2r(3) = -e*beta

         call new_boost(p,q,qr,1)
         call new_boost(p,p3r,p3r,1)
         call new_boost(p,p4r,p4r,1)
      else
         e=sqrt(shat)/2.d0
         beta = sqrt(1.d0 - amel**2/e**2)

         p1r(0) = e
         p1r(1) = 0.d0
         p1r(2) = 0.d0
         p1r(3) = e*beta

         p2r(0) = e
         p2r(1) =  0.d0
         p2r(2) =  0.d0
         p2r(3) = -e*beta

         do k = 0,3
            p(k) = bigq(k)-q(k)
         enddo
         e = sqrt(dot(p,p))/2.d0

         do k = 0,3
            ptmp(k) = p3(k)+p4(k)
         enddo
         call new_boost(ptmp,p3,p3r,1)
         call new_boost(ptmp,p4,p4r,1)

         beta = sqrt(1.d0 - amel**2/e**2)

         p3mod = sqrt(tridot(p3r,p3r))
         p4mod = sqrt(tridot(p4r,p4r))

         p3r(0) = e
         p3r(1) = e*beta * p3r(1)/p3mod
         p3r(2) = e*beta * p3r(2)/p3mod
         p3r(3) = e*beta * p3r(3)/p3mod

         p4r(0) = e
         p4r(1) = e*beta * p4r(1)/p4mod
         p4r(2) = e*beta * p4r(2)/p4mod
         p4r(3) = e*beta * p4r(3)/p4mod
         ! p3r p4r to be intended in the current p frame
         call new_boost(p,p3r,p3r,-1)
         call new_boost(p,p4r,p4r,-1)

         call new_boost(bigq,q,qr,1)
         call new_boost(bigq,p3r,p3r,1)
         call new_boost(bigq,p4r,p4r,1)         
      endif
      return
      end
