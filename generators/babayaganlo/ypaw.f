*** storage routines, storing to a simple ASCII file
      subroutine initstorage(storfile)
! written by CMCC, last modified 12/1/2006 (from HORACE)
      implicit double precision (a-h,o-z)
      character*(*) storfile
      common/storageunit/istorunit
*  PAW stuff....
c      parameter (nwpawc = 10000000)
c      common/pawc/paw(nwpawc)
c      common/quest/iquest(100)
c      parameter (nsize_event = 16)
c      parameter (nsize_event = 12)
c      real*4 event(nsize_event)
c      character*4 chtags(nsize_event)
c      call init_chtags(chtags,nsize_event)
c      call hlimit(nwpawc)
c      iquest(10)=256000
c      call hropen(1,'ntuple',storfile,'nqe',1024,istat)
c      call hbookn(10,'ntuple',nsize_event,'ntuple',50000,chtags)
      istorunit = 61
      open(istorunit,file=storfile,status='unknown')
      return
      end
*********************************************************
      subroutine eventstorage(p3,p4,qph)
      implicit double precision (a-h,o-z)
      dimension p3(0:3),p4(0:3),qph(40,0:3),qph1(0:3),qph2(0:3)
      common/storageunit/istorunit
      common/storedevents/nstored
      data nstored /0/

      nstored = nstored + 1

ccc     counting photons
      ng = 0            ! number of photons in the record
      do k = 1,40
         if (qph(k,0).gt.0.d0) ng = ng +1
      enddo

c uncomment the 'CX' comments if you want to store only the two most energetic photons
CX      if (ng.gt.0) then
CX         call twomostenergetic(ng,qph,qph1,qph2)
CX      else
CX         qph1 = 0.d0
CX         qph2 = 0.d0
CX      endif

ccc   
***   writing event. Format:
**  # EVENT N. [event number]      
**  [number of final state particles 2 + ng]
**  [E fermion -, Px, Py ,Pz]
**  [E fermion +, Px, Py ,Pz]
**  [E photon 1,  Px, Py ,Pz]
**  ...
**  ...      
**  ...
**  [E photon ng, Px, Py ,Pz]
      write(istorunit,*)'# EVENT N.',nstored
      write(istorunit,*)2+ng ! number of final state particles
      write(istorunit,*)p3
      write(istorunit,*)p4
CX      write(istorunit,*)qph1
CX      write(istorunit,*)qph2
c comment 'if (ng.gt.0) ... endif' if you store only the two most energetic photons
      if (ng.gt.0) then
         do k = 1, ng
            write(istorunit,*)qph(k,:)
         enddo
      endif
*
      return
      end
*******************************************
      subroutine finalizestorage
      common/storageunit/istorunit
c      call hrout(0,icycle,' ')
c      call hrend('ntuple')
      close(istorunit)
      end
*** end of storage routines

*********************************************************
      subroutine twomostenergetic(ng,q,q1,q2)
      implicit double precision (a-h,o-z)
      integer iloc(1)
      dimension q(1:ng,0:3),q1(0:3),q2(0:3),ql(1:ng,0:3)

      q1 = 0.d0
      q2 = 0.d0

      ql = q
      
      iloc = maxloc(ql(:,0))
      q1 = ql(iloc(1),:)

      ql(iloc(1),:) = 0.d0
      
      iloc = maxloc(ql(:,0))
      q2 = ql(iloc(1),:)

      return
      end
      
      subroutine sortmomenta(ng,qm)
      implicit double precision (a-h,o-z)
      dimension pa(0:3),pb(0:3),qm(40,0:3),q1(0:3),q2(0:3),qmord(40,0:3)
      if (ng.eq.0) return
      ngl = ng
      call orderinverseqm(ngl,qm,qmord)
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
***
c
      subroutine init_chtags(chtags,n)
! written by CMCC, last modified 12/1/2006 (from HORACE)
      integer n
      character*4 chtags(n)
      chtags(1)  = 'e1'
      chtags(2)  = 'p1x'
      chtags(3)  = 'p1y'
      chtags(4)  = 'p1z'
      chtags(5)  = 'e2'
      chtags(6)  = 'p2x'
      chtags(7)  = 'p2y'
      chtags(8)  = 'p2z'
      chtags(9)  = 'q10'
      chtags(10) = 'q1x'
      chtags(11) = 'q1y'
      chtags(12) = 'q1z'
c      chtags(13) = 'q20'
c      chtags(14) = 'q2x'
c      chtags(15) = 'q2y'
c      chtags(16) = 'q2z'
      return
      end
*
****************************************************
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

      subroutine twomostenergetic_OLD(q,q1,q2,i1,i2)
      implicit real*8 (a-h,o-z)
      dimension q(40,0:3),q1(0:3),q2(0:3)
      do i = 0,3
         q1(i) = 0.d0
         q2(i) = 0.d0
      enddo
      ENPHOT = Q(1,0)
      ENHARD = ENPHOT
      J=1
* 
      DO I = 1,40
         ENPHOT = Q(I,0)
         IF (ENHARD.GE.ENPHOT) THEN
            ENHARD = ENHARD
         ELSE
            J = I
            ENHARD = ENPHOT
         ENDIF
      ENDDO 
*     
      ENHARDL = ENHARD
      JL = J
*     
      if (j.gt.0) then
         DO I = 0,3
            Q1(I) = Q(J,I)
         ENDDO
      endif
      i1 = j
      j1 = j
*     NEXT-TO-LEADING ENERGETIC PHOTON IS EXTRACTED
      ENHARD = 0.D0
      J = 0
      DO I = 1,40
         IF (I.NE.JL) THEN
            ENPHOT = Q(I,0)
            IF (ENHARD.GE.ENPHOT) THEN
               ENHARD = ENHARD
            ELSE
               J = I
               ENHARD = ENPHOT
            ENDIF
         ENDIF
      ENDDO 
      ENHARD = ENHARD
      if (j.gt.0) then
         DO I = 0,3
            Q2(I) = Q(J,I)
         ENDDO
      endif
      i2 = j
      j2 = j
      return
      end
*
      
