      subroutine cuts(p1,p2,qph,icut)
! written by CMCC, last modified 7/4/2006
! p1(0...3): final state electron four-momentum
! p2(0...3): final state positron four-momentum
! qph(0...40,0...3): four-momenta of 40 emitted photons
! icut: icut = 1 event rejected, icut = 0 event accepted
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),qph(40,0:3),q(0:3),ptmp(0:3)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/angularranges/thmine,thmaxe,thminp,thmaxp
      common/momentainitial/pin1(0:3),pin2(0:3)
      common/massainv/amassainvmin,amassainvmax
      common/idarkon/idarkon
      dimension ptvector(0:3)

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
*
************************************
*
      if (idarkon.eq.1) then
*
** Mi restringo alla regione della distribuzione intorno al picco della
** BW che voglio osservare nel dettaglio
*
          if (p1(0).lt.emin.or.p2(0).lt.emin) return
          do k=0,3
            ptmp(k) = p1(k) + p2(k)
            q(k) = qph(1,k)
          enddo
          amassainv = sqrt(abs(dot(ptmp,ptmp)))

          if (amassainv.lt.amassainvmin) return
          if (amassainv.gt.amassainvmax) return

          c  = q(3)/sqrt(tridot(q,q))
          th = acos(c)

          if (q(0).lt.egmin.or.th.lt.thgmin.or.th.gt.thgmax) return
      endif

      icut = 0 ! event accepted
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
            call cuts(q1,q2,qph,icuttmp)
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

