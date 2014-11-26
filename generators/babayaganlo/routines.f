      subroutine printstatus(icode,kl,p1,p2,qph,xs,var,varb,sd,sdm,fm)
      implicit double precision (a-h,o-z)
      integer*8 kl
      character*12 col(20)
      common/colors/col
      dimension p1(0:3),p2(0:3),qph(40,0:3),q(0:3)
      dimension pin1(0:3),pin2(0:3),ptmp(0:3)
      common/ifirstprst/ifirst,icount
      data ifirst /1/
      data icount /0/
      character*2 fs
      common/finalstate/fs
      common/ecms/ecms
      common/nphot_mode/nphotmode
      common/expcuts/thmin,thmax,emin,zmax,egmin,thgmin,thgmax
      common/zparameters/zm,gz,ve,ae,rv,ra,wm,s2th,gfermi,sqrt2,um
      common/energiabeam/ebeam
      common/parameters/ame,ammu,convfac,alpha,pi
      common/intinput/iwriteout,iseed,nsearch,iverbose
      common/ialpharunning/iarun
      common/idebugging/idebug
      common/momentainitial/pin1,pin2
      common/radpattern/nph(4)
      icount = icount + 1
      nfot = nph(1)+nph(2)+nph(3)+nph(4)
      amfs = 0.d0
      if (fs.eq.'ee') amfs = ame
      if (fs.eq.'mm') amfs = ammu
      print*,'>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>'
      print*,'printing point & event n.: ',col(2),icode,col(1),' ',kl
      print*,'final state: ',fs
      write(*,'(1x,A,f16.8,A,f16.8)')'cross section: ',xs,' +-',var
      write(*,'(1x,A,f16.8,A,f16.8)')'var now and before: ',var,',',varb
      write(*,'(1x,A,f25.8)')         'diff. xsect: ',sd
      write(*,'(1x,A,f25.8)')         'max xsect:   ',sdm
      write(*,'(1x,A,f16.8)')         'fmax (should be < max. xs): ',fm
      print*,'n. phot. ',nfot,' and rad. pattern ',nph
      write(*,'(1x,A,f16.8,f16.8,f16.8,f16.8)')'  p1 ',pin1
      write(*,'(1x,A,f16.8,f16.8,f16.8,f16.8)')'  p2 ',pin2
      write(*,'(1x,A,f16.8,f16.8,f16.8,f16.8)')'  p3 ',p1
      write(*,'(1x,A,f16.8,f16.8,f16.8,f16.8)')'  p4 ',p2
      if (nfot.gt.0) then
         do k = 1,nfot
            q(0) = qph(k,0)
            q(1) = qph(k,1)
            q(2) = qph(k,2)
            q(3) = qph(k,3)
            write(*,'(1x,i2,A,f16.8,f16.8,f16.8,f16.8)')k,' q ',q
         enddo
      endif
      write(*,'(1x,A,f16.8)')'th. angle p3-p1',angledeg(p1,pin1)
      write(*,'(1x,A,f16.8)')'th. angle p4-p1',angledeg(p2,pin1)
      write(*,'(1x,A,f16.8)')'th. angle p3-p4',angledeg(p1,p2)
      write(*,'(1x,A,f16.8)')'acoll. p3-p4   ',acollinearity(p1,p2)
      if (nfot.gt.0) then
         do k = 1,nfot
            q(0) = qph(k,0)
            q(1) = qph(k,1)
            q(2) = qph(k,2)
            q(3) = qph(k,3)
            write(*,'(1x,i2,A,f16.8)')k,'  angle q-p1 ',angledeg(q,pin1)
            write(*,'(1x,i2,A,f16.8)')k,'  angle q-p2 ',angledeg(q,pin2)
            write(*,'(1x,i2,A,f16.8)')k,'  angle q-p3 ',angledeg(q,p1)
            write(*,'(1x,i2,A,f16.8)')k,'  angle q-p4 ',angledeg(q,p2)
         enddo
      endif
      print*,'invariants:'
      do k = 0,3
         ptmp(k) = pin1(k)+pin2(k)
      enddo
      s12 = dot(ptmp,ptmp)
      do k = 0,3
         ptmp(k) = p1(k)+p2(k)
      enddo
      s34 = dot(ptmp,ptmp)
      do k = 0,3
         ptmp(k) = pin1(k)-p1(k)
      enddo
      t13 = dot(ptmp,ptmp)
      do k = 0,3
         ptmp(k) = pin2(k)-p2(k)
      enddo
      t24 = dot(ptmp,ptmp)
      do k = 0,3
         ptmp(k) = pin1(k)-p2(k)
      enddo
      u14 = dot(ptmp,ptmp)
      do k = 0,3
         ptmp(k) = pin2(k)-p1(k)
      enddo
      u23 = dot(ptmp,ptmp)
      write(*,'(1x,A,f16.8,f16.8,f16.8)')'s12 s34 t13',s12,s34,t13
      write(*,'(1x,A,f16.8,f16.8,f16.8)')'t24 u14 u23',t24,u14,u23
      if (nfot.gt.0) then
         do k = 1,nfot
            q(0) = qph(k,0)
            q(1) = qph(k,1)
            q(2) = qph(k,2)
            q(3) = qph(k,3)
            write(*,'(1x,i2,A,f16.8)')k,' p1q',dot(pin1,q)
            write(*,'(1x,i2,A,f16.8)')k,' p2q',dot(pin2,q)
            write(*,'(1x,i2,A,f16.8)')k,' p3q',dot(p1,q)
            write(*,'(1x,i2,A,f16.8)')k,' p4q',dot(p2,q)
         enddo
      endif
      print*,'<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<'
      return
      end

      subroutine printvector(ip,p1,p2,qph)
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3),qph(40,0:3),q(0:3)
      common/channelref/iref
      common/ifirstprv/ifirst,icount
      data ifirst /1/
      data icount /0/
      icount = icount + 1
      if (ifirst.eq.1) then
         open(33,file='momenta',status='unknown')
         ifirst = 0
      else
         open(33,file='momenta',status='unknown',access='append')
      endif
c$$$      print*,'set arrow 1 from 0,0,0 to ',p1(1),',',p1(2),',',p1(3)
c$$$      print*,'set arrow 2 from 0,0,0 to ',p2(1),',',p2(2),',',p2(3)
c$$$      print*,'set arrow 3 from 0,0,0 to ',q(1) ,',', q(2),',',q(3)
c$$$      print*,'set'//
c$$$     .' xrange [',min(p1(1),p2(1),q(1)),':',max(p1(1),p2(1),q(1)),'];'//
c$$$     .' set '//
c$$$     .'yrange [',min(p1(2),p2(2),q(2)),':\\',max(p1(2),p2(2),q(2)),'];'
c$$$     .//
c$$$     .' set'//
c$$$     .' zrange [',min(p1(3),p2(3),q(3)),':',max(p1(3),p2(3),q(3)),']'
      write(33,*)'point, count, iref ',ip,icount,iref
c
      write(33,*)'set arrow 1 from 0,0,0 to',p1(3),',',p1(1),',',p1(2),
     .' lt 1'
      write(33,*)'set arrow 2 from 0,0,0 to',p2(3),',',p2(1),',',p2(2),
     .' lt 2'
      n = 0
      do k = 1,40
         if (qph(k,0).gt.0.d0) n = n+1
      enddo
      if (n.gt.0) then
         do k = 3,n+2
            q(0) = qph(k-2,0)
            q(1) = qph(k-2,1)
            q(2) = qph(k-2,2)
            q(3) = qph(k-2,3)
      write(33,*)'set arrow',k
     .,' from 0,0,0 to',q(3),',',q(1),',',q(2),''//
     .' lt 3'
        enddo
      endif

      close(33)

      return
      end

      subroutine randperm(n,irand)
      implicit double precision (a-h,o-z)
      dimension irand(n),ind(n)
      real*4 csi(1)
      do k = 1,n
         ind(k) = k
      enddo
      do j = 0,n-1
         k = n-j
!          call ranlux(csi,1)
         csi(1) = babayaganlo_rndm(0)

         ii = k * csi(1) + 1
         if (ii.gt.k) ii = k
         irand(j+1) = ind(ii)
         itmp = ind(k)
         ind(k) = ind(ii)
         ind(ii) = itmp
      enddo
      return
      end
c$$$*** storage routines 
c$$$      subroutine initstorage(storfile)
c$$$! written by CMCC, last modified 12/1/2006 (from HORACE)
c$$$      implicit double precision (a-h,o-z)
c$$$      character*(*) storfile
c$$$* PAW stuff....
c$$$      parameter (nwpawc = 10000000)
c$$$      common/pawc/paw(nwpawc)
c$$$      common/quest/iquest(100)
c$$$c      parameter (nsize_event = 16)
c$$$      parameter (nsize_event = 12)
c$$$      real*4 event(nsize_event)
c$$$      character*4 chtags(nsize_event)
c$$$cc      call init_chtags(chtags,nsize_event)
c$$$cc      call hlimit(nwpawc)
c$$$      iquest(10)=256000
c$$$cc      call hropen(1,'ntuple',storfile,'nqe',1024,istat)
c$$$cc      call hbookn(10,'ntuple',nsize_event,'ntuple',50000,chtags)
c$$$      return
c$$$      end
c$$$c
c$$$      subroutine init_chtags(chtags,n)
c$$$! written by CMCC, last modified 12/1/2006 (from HORACE)
c$$$      integer n
c$$$      character*4 chtags(n)
c$$$      chtags(1)  = 'e1'
c$$$      chtags(2)  = 'p1x'
c$$$      chtags(3)  = 'p1y'
c$$$      chtags(4)  = 'p1z'
c$$$      chtags(5)  = 'e2'
c$$$      chtags(6)  = 'p2x'
c$$$      chtags(7)  = 'p2y'
c$$$      chtags(8)  = 'p2z'
c$$$      chtags(9)  = 'q10'
c$$$      chtags(10) = 'q1x'
c$$$      chtags(11) = 'q1y'
c$$$      chtags(12) = 'q1z'
c$$$c      chtags(13) = 'q20'
c$$$c      chtags(14) = 'q2x'
c$$$c      chtags(15) = 'q2y'
c$$$c      chtags(16) = 'q2z'
c$$$      return
c$$$      end
c$$$*
c$$$      subroutine sortmomenta(ng,qm)
c$$$      implicit double precision (a-h,o-z)
c$$$      dimension pa(0:3),pb(0:3),qm(40,0:3),q1(0:3),q2(0:3),qmord(40,0:3)
c$$$      if (ng.eq.0) return
c$$$      ngl = ng
c$$$      call orderinverseqm(ngl,qm,qmord)
c$$$! in qmord photons are from the less energetic (k=1) to the most energetic 
c$$$! (k=ng). In qm I want to sort from the most to the less energetic...
c$$$      do k = 1,ng
c$$$         qm(ng+1-k,0) = qmord(k,0)
c$$$         qm(ng+1-k,1) = qmord(k,1)
c$$$         qm(ng+1-k,2) = qmord(k,2)
c$$$         qm(ng+1-k,3) = qmord(k,3)
c$$$      enddo
c$$$      return
c$$$      end
c$$$***
c$$$      subroutine mixandsortmomenta(ng,pa,pb,qm)
c$$$      implicit double precision (a-h,o-z)
c$$$      dimension pa(0:3),pb(0:3),qm(40,0:3),q1(0:3),q2(0:3),qmord(40,0:3)
c$$$      common/beforesort/p1o(0:3),p2o(0:3),qpho(40,0:3)
c$$$***** TEST
c$$$***** TEST
c$$$***** TEST
c$$$***** TEST
c$$$c$$$      do k = 0,3
c$$$c$$$         p1o(k) = pa(k)
c$$$c$$$         p2o(k) = pb(k)
c$$$c$$$         do i = 1,ng
c$$$c$$$            qpho(i,k) = qm(i,k)
c$$$c$$$         enddo
c$$$c$$$      enddo
c$$$c$$$      return
c$$$***** TEST
c$$$***** TEST
c$$$***** TEST
c$$$***** TEST
c$$$      if (ng.eq.0) return
c$$$      do k = 0,3
c$$$         p1o(k) = pa(k)
c$$$         p2o(k) = pb(k)
c$$$         do i = 1,ng
c$$$            qpho(i,k) = qm(i,k)
c$$$         enddo
c$$$      enddo
c$$$      ngl = ng + 2
c$$$      do k = 0,3
c$$$         qm(ng+1,k) = pa(k)
c$$$         qm(ng+2,k) = pb(k)
c$$$      enddo
c$$$      call orderinverseqm(ngl,qm,qmord)
c$$$      do k = 0,3
c$$$         pa(k)= qmord(ng+2,k)
c$$$         pb(k)= qmord(ng+1,k)
c$$$         qm(ng+1,k) = 0.d0
c$$$         qm(ng+2,k) = 0.d0
c$$$      enddo
c$$$! in qmord photons are from the less energetic (k=1) to the most energetic 
c$$$! (k=ng). In qm I want to sort from the most to the less energetic...
c$$$      do k = 1,ng
c$$$         qm(ng+1-k,0) = qmord(k,0)
c$$$         qm(ng+1-k,1) = qmord(k,1)
c$$$         qm(ng+1-k,2) = qmord(k,2)
c$$$         qm(ng+1-k,3) = qmord(k,3)
c$$$      enddo
c$$$      return
c$$$      end
c$$$c
c$$$      subroutine orderinverseqm(nph,qm,qmord)
c$$$      implicit double precision (a-h,o-z)
c$$$      parameter (nmax = 40)
c$$$      dimension qm(nmax,0:3),qmord(nmax,0:3),en(nph),sen(nph)
c$$$      dimension indexes(nph)
c$$$      do k = 1,nph
c$$$         en(k) = qm(k,0)
c$$$      enddo
c$$$      call sort_vector_inverse(en,sen,indexes,nph)
c$$$      do k = 1,nph
c$$$         qmord(k,0) = qm(indexes(k),0)
c$$$         qmord(k,1) = qm(indexes(k),1)
c$$$         qmord(k,2) = qm(indexes(k),2)
c$$$         qmord(k,3) = qm(indexes(k),3)
c$$$      enddo
c$$$      do k = nph+1,nmax
c$$$         qmord(k,0) = 0.d0
c$$$         qmord(k,1) = 0.d0
c$$$         qmord(k,2) = 0.d0
c$$$         qmord(k,3) = 0.d0
c$$$      enddo
c$$$      return
c$$$      end
c$$$
c$$$      subroutine sort_vector(v,sv,iold,n)
c$$$! pensare quick sort!!
c$$$      implicit double precision (a-h,o-z)
c$$$      dimension iold(n)
c$$$      dimension v(n),sv(n)
c$$$      do k = 1,n
c$$$         iold(k) = k
c$$$         sv(k) = v(k)
c$$$      enddo
c$$$      do i = 1,n
c$$$         vmax = sv(i)
c$$$         do j=i+1,n
c$$$            if (v(j).gt.vmax) then
c$$$               vmax = sv(j)
c$$$               vtmp = sv(i)
c$$$               sv(i) = sv(j)
c$$$               sv(j) = vtmp
c$$$
c$$$               ktmp = iold(i)
c$$$               iold(i) = iold(j)
c$$$               iold(j) = ktmp
c$$$            endif
c$$$         enddo
c$$$      enddo
c$$$      return
c$$$      end
c$$$
c$$$      subroutine sort_vector_inverse(v,sv,iold,n)
c$$$! pensare quick sort!!
c$$$      implicit double precision (a-h,o-z)
c$$$      dimension iold(n)
c$$$      dimension v(n),sv(n)
c$$$C this uses the quick sort algorithm reported at the end of the file
c$$$c      call SORTRX(n,v,iold)
c$$$c      do k = 1,n
c$$$c         sv(k) = v(iold(k))
c$$$c      enddo
c$$$c      return
c$$$      do k = 1,n
c$$$         iold(k) = k
c$$$         sv(k) = v(k)
c$$$      enddo
c$$$      do i = 1,n
c$$$         vmin = sv(i)
c$$$         do j=i+1,n
c$$$            if (v(j).lt.vmin) then
c$$$               vmin = sv(j)
c$$$               vtmp = sv(i)
c$$$               sv(i) = sv(j)
c$$$               sv(j) = vtmp
c$$$               ktmp = iold(i)
c$$$               iold(i) = iold(j)
c$$$               iold(j) = ktmp
c$$$            endif
c$$$         enddo
c$$$      enddo
c$$$      return
c$$$      end
c$$$
c$$$      subroutine twomostenergetic(q,q1,q2,i1,i2)
c$$$      implicit double precision (a-h,o-z)
c$$$      dimension q(40,0:3),q1(0:3),q2(0:3)
c$$$      do i = 0,3
c$$$         q1(i) = 0.d0
c$$$         q2(i) = 0.d0
c$$$      enddo
c$$$      ENPHOT = Q(1,0)
c$$$      ENHARD = ENPHOT
c$$$      J=1
c$$$* 
c$$$      DO I = 1,40
c$$$         ENPHOT = Q(I,0)
c$$$         IF (ENHARD.GE.ENPHOT) THEN
c$$$            ENHARD = ENHARD
c$$$         ELSE
c$$$            J = I
c$$$            ENHARD = ENPHOT
c$$$         ENDIF
c$$$      ENDDO 
c$$$*     
c$$$      ENHARDL = ENHARD
c$$$      JL = J
c$$$*     
c$$$      if (j.gt.0) then
c$$$         DO I = 0,3
c$$$            Q1(I) = Q(J,I)
c$$$         ENDDO
c$$$      endif
c$$$      i1 = j
c$$$      j1 = j
c$$$*     NEXT-TO-LEADING ENERGETIC PHOTON IS EXTRACTED
c$$$      ENHARD = 0.D0
c$$$      J = 0
c$$$      DO I = 1,40
c$$$         IF (I.NE.JL) THEN
c$$$            ENPHOT = Q(I,0)
c$$$            IF (ENHARD.GE.ENPHOT) THEN
c$$$               ENHARD = ENHARD
c$$$            ELSE
c$$$               J = I
c$$$               ENHARD = ENPHOT
c$$$            ENDIF
c$$$         ENDIF
c$$$      ENDDO 
c$$$      ENHARD = ENHARD
c$$$      if (j.gt.0) then
c$$$         DO I = 0,3
c$$$            Q2(I) = Q(J,I)
c$$$         ENDDO
c$$$      endif
c$$$      i2 = j
c$$$      j2 = j
c$$$      return
c$$$      end
c$$$*
c$$$      subroutine eventstorage(p3,p4,qph)
c$$$! written by CMCC, last modified 12/1/2006 (from HORACE)
c$$$      implicit double precision (a-h,o-z)
c$$$      dimension p3(0:3),p4(0:3),qph(40,0:3),qph1(0:3),qph2(0:3)
c$$$c      parameter (nsize_event = 16)
c$$$      parameter (nsize_event = 12)
c$$$      real*4 event(nsize_event)
c$$$      call mostenergeticphoton(qph,qph1)
c$$$      do i=1,4
c$$$         event(i)    = p3(i-1)
c$$$         event(i+4)  = p4(i-1)
c$$$         event(i+8)  = qph1(i-1)
c$$$c         event(i+12) = qph2(i-1)
c$$$      enddo
c$$$cc       call hfn(10,event)
c$$$      return
c$$$      end
c$$$*******************************************
c$$$      subroutine finalizestorage
c$$$! written by CMCC, last modified 12/1/2006 (from HORACE)
c$$$c      call hrout(0,icycle,' ')
c$$$c      call hrend('ntuple')
c$$$      end
c$$$*** end of storage routines

      function acollinearity(p1,p2)
! written by CMCC, last modified 5/3/2006
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      a1 = acos(p1(3)/sqrt(tridot(p1,p1)))*180.d0/pi
      a2 = acos(p2(3)/sqrt(tridot(p2,p2)))*180.d0/pi
      acollinearity = abs(180.d0 - a1 - a2)
      return
      end

      function acollinearityrad(p1,p2)
! written by CMCC, last modified 5/3/2006
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      a1 = acos(p1(3)/sqrt(tridot(p1,p1)))
      a2 = acos(p2(3)/sqrt(tridot(p2,p2)))
      acollinearityrad = abs(pi - a1 - a2)
      return
      end

      function cosine(p1,p2)
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3)
      p1modm1 = 1.d0/sqrt(tridot(p1,p1))
      p2modm1 = 1.d0/sqrt(tridot(p2,p2))
      cosine = tridot(p1,p2)*p1modm1*p2modm1
      return
      end

      function angledeg(p1,p2)
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      c = cosine(p1,p2)
      if (c.ge. 1.d0) c =  1.d0
      if (c.le.-1.d0) c = -1.d0
      angledeg = acos(c)*180.d0/pi
      return
      end

      function anglerad(p1,p2)
      implicit double precision (a-h,o-z)
      dimension p1(0:3),p2(0:3)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      c = cosine(p1,p2)
      if (c.ge. 1.d0) c =  1.d0
      if (c.le.-1.d0) c = -1.d0
      anglerad = acos(c)
      return
      end

      subroutine rot(idir,vect,pin,pout)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)       
      double precision pin(0:3),pout(0:3),pp(0:3),r(3,3),
     >     vers(3),vect(0:3)
* This subroutine rotates the 4-vector pin in the frame where the z-axis is
* directed along the 4-vector vect(0,1,2,3). The rotated vector is stored
* in pout
* idir =  1 ---> direct rotation matrix
* idir = -1 ---> inverse rotation matrix
      pp(0) = pin(0)
      pp(1) = pin(1)
      pp(2) = pin(2)
      pp(3) = pin(3)

      vmo = 1.d0/sqrt(vect(1)**2+vect(2)**2+vect(3)**2)
      vers(1) = vect(1)*vmo
      vers(2) = vect(2)*vmo
      vers(3) = vect(3)*vmo
      vt = sqrt(vers(1)**2+vers(2)**2)

!   BUG - pointed out by CLEO people
!      v1ovt = vers(1)/vt
!      if (vt.eq.0.d0) v1ovt = 0.d0
!      v2ovt = vers(2)/vt
!      if (vt.eq.0.d0) v2ovt = 1.d0
 
      v1ovt = 0.d0
      v2ovt = 1.d0
      if (vt.gt.0.d0) then
         v1ovt = vers(1)/vt
         v2ovt = vers(2)/vt
      endif
      
      if (idir.eq.(-1)) then    !! INVERSE rotation matrix
         r(1,1) =  vers(3)*v1ovt
         r(1,2) = -v2ovt
         r(1,3) =  vers(1)      
         r(2,1) =  vers(3)*v2ovt
         r(2,2) =  v1ovt
         r(2,3) =  vers(2)
         r(3,1) = -vt
         r(3,2) =  0.d0
         r(3,3) =  vers(3)
      else  ! if (idir.eq.1) !! DIRECT rotation matrix
         r(1,1) =  vers(3)*v1ovt
         r(2,1) = -v2ovt
         r(3,1) =  vers(1)
         r(1,2) =  vers(3)*v2ovt
         r(2,2) =  v1ovt
         r(3,2) =  vers(2)
         r(1,3) = -vt
         r(2,3) =  0.d0
         r(3,3) =  vers(3)
      endif
      pout(0) = pp(0)
      pout(1) = r(1,1)*pp(1) + r(1,2)*pp(2) + r(1,3)*pp(3)
      pout(2) = r(2,1)*pp(1) + r(2,2)*pp(2) + r(2,3)*pp(3)
      pout(3) = r(3,1)*pp(1) + r(3,2)*pp(2) + r(3,3)*pp(3)
      return
      end
*
      subroutine getphi(rnd,ph,w)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      real*4 rnd
      ph = 2.d0*pi*rnd
      w  = 2.d0*pi
      return
      end
*
      function getphiangle(ppp)
      implicit double precision (a-h,o-z)
      dimension ppp(0:3)
      parameter (pi = 3.1415926535897932384626433832795029D0)
      pm = sqrt(ppp(1)**2+ppp(2)**2+ppp(3)**2)
      c  = ppp(3)/pm
      s  = sqrt(1.d0-c**2)
      if (s.eq.0.d0) then
         getphiangle = 0.d0
         return
      else   
         arg = ppp(1)/pm/s
*  avoiding numerical problems......
         if (abs(arg).ge.1.d0) then
            iarg = arg
            arg  = iarg
         endif   
         if (ppp(2).ge.0.d0) getphiangle = acos(arg)
         if (ppp(2).lt.0.d0) getphiangle = 2.d0*pi-acos(arg)
      endif             
      return
      end
*     
      function factorial(n)
!     written by CMCC, last modified 21/1/2006
      integer n,nl      
      double precision factorial
      nl = n
      factorial = 1.d0
      do while(nl.gt.0)
         factorial = 1.d0*nl * factorial
         nl = nl - 1
      enddo
      return
      end
*
      function eikonalintegral(p1,p2,p3,p4,ch,masses)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,l,m,o-z)
      parameter (npart = 4)
      dimension eta(npart),p(0:3),q(0:3),pmat(npart,0:3)
      dimension masses(npart),ch(npart)
      dimension p1(0:3),p2(0:3),p3(0:3),p4(0:3)
! ch are the charges of the field (not anti-field!!)
! Use this convention: the integer factor in front of the charge
! must be
! -1 --> for incoming particle
! -1 --> for outgoing anti-particle
! +1 --> for outgoing particle
! +1 --> for incoming anti-particle
      eta(1) = -1.d0 * ch(1)
      eta(2) =  1.d0 * ch(2)
      eta(3) =  1.d0 * ch(3)
      eta(4) = -1.d0 * ch(4)
*
      do k = 0,3
         pmat(1,k) = p1(k)
         pmat(2,k) = p2(k)
         pmat(3,k) = p3(k)
         pmat(4,k) = p4(k)
      enddo

! off-diagonal contributions
      softint = 0.d0
      do i = 1,npart-1
         do j = i+1,npart
            etaij = eta(i) * eta(j)
            if (abs(etaij).gt.1.d-3) then
               call rescale_momenta(npart,i,j,pmat,masses,p,q)
               q2 = dot(q,q)
               vl = dot(p,p) - q2
               vl = vl/2.d0
               v  = vl/(p(0) - q(0)) 
               arglog = 1.d0+2.d0*vl/q2
               terminfra = log(arglog)         

               tot = terminfra ! + termfinite(p,q,v)
               tot = - tot *2.d0*dot(p,q)/vl* etaij
               tot = tot * 2.d0 ! this is the double product when
                                ! squaring the eikonal
               softint  = softint + tot
            endif
         enddo
      enddo      
      soft_integral = softint
      softint = -4.d0 * (eta(1)**2+eta(2)**2+eta(3)**2+eta(4)**2)
      soft_integral = soft_integral + softint
      soft_integral = soft_integral/16.d0
      eikonalintegral = soft_integral
      return
      end
**
      subroutine rescale_momenta(npart,i,j,pmat,masses,p,q)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,m,o-z)
      dimension p(0:3),q(0:3),pmat(npart,0:3),masses(npart)
      dimension p1(0:3),p2(0:3)
      do k = 0,3
         p1(k) = pmat(i,k)
         p2(k) = pmat(j,k)
      enddo
      m12 = masses(i)**2
      m22 = masses(j)**2
      p1p2 = dot(p1,p2)
      rho1 = p1p2 + sqrt(p1p2**2 - m12*m22)
      rho1 = rho1 / m12
      rho2 = p1p2 - sqrt(p1p2**2 - m12*m22)
      rho2 = rho2 / m12
      if ( (rho1*p1(0)-p2(0)) .gt. 0.d0) rho = rho1
      if ( (rho2*p1(0)-p2(0)) .gt. 0.d0) rho = rho2
      do k = 0,3
         p(k) = rho * p1(k)
         q(k) = p2(k)
      enddo
      return
      end
***
      function termfinite(p,q,v)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      dimension p(0:3),q(0:3)

      u0   = p(0)
      umod = sqrt(tridot(p,p))
      pp = u0 + umod
      pm = u0 - umod

      u0   = q(0)
      umod = sqrt(tridot(q,q))
      qp = u0 + umod
      qm = u0 - umod

! dilog from TOPAZ0
      arg1 = (v-pm)/v
      arg2 = (v-pp)/v
      arg3 = (v-qm)/v
      arg4 = (v-qp)/v

c      ddlog1 = ddilog(arg1)
c      ddlog2 = ddilog(arg2)
c      ddlog3 = ddilog(arg3)
c      ddlog3 = ddilog(arg4)

      call tspence(arg1,0.d0,1.d0-arg1,ddlog1,ddim)
      call tspence(arg2,0.d0,1.d0-arg2,ddlog2,ddim)
      call tspence(arg3,0.d0,1.d0-arg3,ddlog3,ddim)
      call tspence(arg4,0.d0,1.d0-arg4,ddlog4,ddim)

      termfin = log(pp/pm)**2/4.d0 + ddlog1 + ddlog2
     >        - log(qp/qm)**2/4.d0 - ddlog3 - ddlog4

      termfinite = termfin
      return
      end
*
      double precision function lambda(x,y,z)
! written by CMCC, last modified 9/10/2005
      double precision x,y,z
      lambda = x**2+y**2+z**2 - 2.d0*x*y - 2.d0*x*z - 2.d0*y*z
      return
      end
*
      subroutine exchange_mom(p1,p2)
! written by CMCC, last modified 9/10/2005
      implicit none 
      double precision p1(0:3),p2(0:3),p4(0:3)      
      integer i
      do i=0,3
         p4(i)=p1(i)
      enddo      
      do i=0,3
         p1(i)=p2(i)
      enddo      
      do i=0,3
         p2(i)=p4(i)
      enddo      
      return
      end
*
      subroutine new_boost(p,q,qq,idir)
! written by CMCC, last modified 9/10/2005
      implicit none
      integer idir
      double precision p(0:3),q(0:3),qq(0:3),vboost(0:3)
      double precision b,g,p0mu
! idir =  1: q is boosted where p is at rest and put in qq     
! idir = -1: q, in the frame where p is at rest, is boosted where p is not
!            at rest. The boosted q is put in qq.
      p0mu = 1.d0/p(0)
      if (idir.eq.1) then
         vboost(0) = 0.d0
         vboost(1) = p(1)*p0mu!/p(0)
         vboost(2) = p(2)*p0mu!/p(0)
         vboost(3) = p(3)*p0mu!/p(0)
      else
         vboost(0) = 0.d0
         vboost(1) = -p(1)*p0mu!/p(0)
         vboost(2) = -p(2)*p0mu!/p(0)
         vboost(3) = -p(3)*p0mu!/p(0)
      endif
      b=dsqrt(vboost(1)**2+vboost(2)**2+vboost(3)**2)
      g=1.d0/dsqrt(1.d0-b**2)
      call boost(g,vboost,q,qq)
      return
      end
**
      function dot(p,q)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      double precision p(0:3),q(0:3)
      dot=p(0)*q(0)-p(1)*q(1)-p(2)*q(2)-p(3)*q(3)
      return
      end
*
      function tridot(p,q)
! written by CMCC, last modified 9/10/2005
      implicit double precision (a-h,o-z)
      double precision p(0:3),q(0:3)
      tridot=p(1)*q(1)+p(2)*q(2)+p(3)*q(3)
      return
      end
*
      subroutine boost(g,v,p,q)
! written by CMCC, last modified 24/10/2005 (from old BABAYAGA)
      implicit double precision (a-h,o-z)
      double precision p(0:3),q(0:3),pp(0:3),v(0:3)
      if (g.eq.1.d0) then 
         do i=0,3
            q(i)=p(i)
         enddo
      else     
! this is essential if boost is called with the same momenta as arguments!!!!
         do i=0,3
            pp(i)=p(i)
         enddo
         ppdv = tridot(pp,v)
         v2   = v(1)**2+v(2)**2+v(3)**2
*     
         q(0)=g*(pp(0)-ppdv)
         do i=1,3
            q(i)=pp(i)+(g-1.d0)*ppdv/v2*v(i)-g*v(i)*pp(0)
         enddo
      endif
      return
      end
***
*=========== DILOG FROM TOPAZ0 =====================================
*-----SPENCE--------------------------------------------------------
*     COMPUTES  LI_2(X). ACCURACY IS ABOUT 16 DIGITS               
      SUBROUTINE TSPENCE(XR,XI,OMXR,CLI2R,CLI2I)
! from TOPAZ0
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
*
      COMMON/TQPARAM/QPI,QPIS,QEPS,QDELTA
*
      DIMENSION B(0:14),BF(0:14)
      DIMENSION CLNX(2),CLNOMX(2),CLNOY(2),CLNZ(2),CLNOMZ(2)
      DIMENSION ADD1(2),ADD2(2),ADD3(2),PAR(2),RES(2),CT(15),SN(15)
      DIMENSION X(2),OMX(2),Y(2),OY(2),OMY(2),Z(2),OMZ(2),T(2),OMT(2)
      common/storedtspence/b,bf,ifirst
      data ifirst /0/
c CARLO
      if (ifirst.eq.0) then
         QDELTA= 9.025809333D0
         QEPS= 1.D-30
         QPI= 3.141592653589793238462643D0
         QPIS= QPI*QPI
      endif
c CARLO
      X(1)= XR
      X(2)= XI
      OMX(1)= OMXR
      OMX(2)= -XI
      IF(XR.LT.0.D0) THEN
          Y(1)= OMXR
          Y(2)= -XI
          SIGN1= -1.D0
          CALL TCQLNX(X,CLNX)
          CALL TCQLNOMX(X,OMX,CLNOMX)
          ADD1(1)= QPIS/6.D0-CLNX(1)*CLNOMX(1)+CLNX(2)*CLNOMX(2)
          ADD1(2)= -CLNX(1)*CLNOMX(2)-CLNX(2)*CLNOMX(1)
      ELSE
          Y(1)= X(1)
          Y(2)= X(2)
          SIGN1= 1.D0
          ADD1(1)= 0.D0
          ADD1(2)= 0.D0
      ENDIF
      OMY(1)= 1.D0-Y(1)
      OMY(2)= -Y(2)
      YM2= Y(1)*Y(1)+Y(2)*Y(2)
      YM= SQRT(YM2)
      IF(YM.GT.1.D0) THEN
          Z(1)= Y(1)/YM2
          Z(2)= -Y(2)/YM2
          SIGN2= -1.D0
          OY(1)= -Y(1)
          OY(2)= -Y(2)
          CALL TCQLNX(OY,CLNOY)
          ADD2(1)= -QPIS/6.D0-0.5D0*((CLNOY(1))**2-(CLNOY(2))**2)
          ADD2(2)= -CLNOY(1)*CLNOY(2)
      ELSE
          Z(1)= Y(1)
          Z(2)= Y(2)
          SIGN2= 1.D0
          ADD2(1)= 0.D0
          ADD2(2)= 0.D0
      ENDIF
      OMZ(1)= 1.D0-Z(1)
      OMZ(2)= -Z(2)
      ZR= Z(1)
      IF(ZR.GT.0.5D0) THEN
          T(1)= 1.D0-Z(1)
          T(2)= -Z(2)
          OMT(1)= 1.D0-T(1)
          OMT(2)= -T(2)
          SIGN3= -1.D0
          CALL TCQLNX(Z,CLNZ)
          CALL TCQLNOMX(Z,OMZ,CLNOMZ)
          ADD3(1)= QPIS/6.D0-CLNZ(1)*CLNOMZ(1)+CLNZ(2)*CLNOMZ(2)
          ADD3(2)= -CLNZ(1)*CLNOMZ(2)-CLNZ(2)*CLNOMZ(1)
      ELSE
          T(1)= Z(1)
          T(2)= Z(2)
          OMT(1)= 1.D0-T(1)
          OMT(2)= -T(2)
          SIGN3= 1.D0
          ADD3(1)= 0.D0
          ADD3(2)= 0.D0
      ENDIF
      CALL TCQLNOMX(T,OMT,PAR)

      if (ifirst.eq.0) then
         B(0)= 1.D0
         B(1)= -1.D0/2.D0
         B(2)= 1.D0/6.D0
         B(4)= -1.D0/30.D0
         B(6)= 1.D0/42.D0
         B(8)= -1.D0/30.D0
         B(10)= 5.D0/66.D0
         B(12)= -691.D0/2730.D0
         B(14)= 7.D0/6.D0
         FACT= 1.D0
         DO N=0,14
            BF(N)= B(N)/FACT
            FACT= FACT*(N+2.D0)
         ENDDO
      endif
      PARR= PAR(1)
      PARI= PAR(2)
      PARM2= PARR*PARR+PARI*PARI
      PARM= SQRT(PARM2)
      CT(1)= PARR/PARM
      SN(1)= PARI/PARM
      DO N=2,15
          CT(N)= CT(1)*CT(N-1)-SN(1)*SN(N-1)
          SN(N)= SN(1)*CT(N-1)+CT(1)*SN(N-1)
      ENDDO
*      
      RES(1)= -((((((((BF(14)*CT(15)*PARM2+BF(12)*CT(13))*PARM2+
     #                 BF(10)*CT(11))*PARM2+BF(8)*CT(9))*PARM2+
     #                 BF(6)*CT(7))*PARM2+BF(4)*CT(5))*PARM2+
     #                 BF(2)*CT(3))*(-PARM)+BF(1)*CT(2))*(-PARM)+
     #                 BF(0)*CT(1))*PARM 
      RES(2)= -((((((((BF(14)*SN(15)*PARM2+BF(12)*SN(13))*PARM2+
     #                 BF(10)*SN(11))*PARM2+BF(8)*SN(9))*PARM2+
     #                 BF(6)*SN(7))*PARM2+BF(4)*SN(5))*PARM2+
     #                 BF(2)*SN(3))*(-PARM)+BF(1)*SN(2))*(-PARM)+
     #                 BF(0)*SN(1))*PARM 
      CLI2R= SIGN1*(SIGN2*(SIGN3*RES(1)+ADD3(1))+ADD2(1))+ADD1(1)
      CLI2I= SIGN1*(SIGN2*(SIGN3*RES(2)+ADD3(2))+ADD2(2))+ADD1(2)
*
      ifirst = 1
      RETURN
      END
*
*-----CQLNX---------------------------------------------
*     COMPUTES  LN(Z)                              
*
      SUBROUTINE TCQLNX(ARG,RES)
! from TOPAZ0
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
*
      DIMENSION ARG(2),AARG(2),RES(2)
*
      QPI= 3.141592653589793238462643D0
      DO I= 1,2
          AARG(I)= ABS(ARG(I))
      ENDDO
      ZM2= (ARG(1))**2+(ARG(2))**2
      ZM= SQRT(ZM2)
      RES(1)= LOG(ZM)
      IF(ARG(1).EQ.0.D0) THEN
          IF(ARG(2).GT.0.D0) THEN
              TETA= QPI/2.D0
          ELSE
              TETA= -QPI/2.D0
          ENDIF
          RES(2)= TETA
          RETURN
      ELSE IF(ARG(2).EQ.0.D0) THEN 
               IF(ARG(1).GT.0.D0) THEN
                   TETA= 0.D0
               ELSE
                   TETA= QPI
               ENDIF
          RES(2)= TETA
          RETURN
      ELSE
          TNTETA= AARG(2)/AARG(1)
          TETA= ATAN(TNTETA)
          SR= ARG(1)/AARG(1)
          SI= ARG(2)/AARG(2)
          IF(SR.GT.0.D0) THEN
              RES(2)= SI*TETA
          ELSE
              RES(2)= SI*(QPI-TETA)
          ENDIF
          RETURN
      ENDIF
      END
*
*-----CQLNOMX---------------------------------------
*     COMPUTES LN(1-X)                 
*     USUALLY |X| << 1                 
      SUBROUTINE TCQLNOMX(ARG,OMARG,RES)
! from TOPAZ0
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DIMENSION ARG(2),OMARG(2),RES(2),ARES(2),CT(10),SN(10)
*
      ZR= ARG(1)
      ZI= ARG(2)
      ZM2= ZR*ZR+ZI*ZI
      ZM= SQRT(ZM2)
      IF(ZM.LT.1.D-7) THEN
          CT(1)= ZR/ZM
          SN(1)= ZI/ZM
          DO N=2,10
              CT(N)= CT(1)*CT(N-1)-SN(1)*SN(N-1)
              SN(N)= SN(1)*CT(N-1)+CT(1)*SN(N-1)
          ENDDO
          ARES(1)= CT(10)/10.D0
          ARES(2)= SN(10)/10.D0
          DO K=9,1,-1
              ARES(1)= ARES(1)*ZM+CT(K)/K
              ARES(2)= ARES(2)*ZM+SN(K)/K
          ENDDO
          ARES(1)= -ARES(1)*ZM
          ARES(2)= -ARES(2)*ZM
      ELSE
          CALL TCQLNX(OMARG,ARES)
      ENDIF
      RES(1)= ARES(1)
      RES(2)= ARES(2)
      RETURN
      END


c==================================================================
C this sort subroutine works very well...
c
C From Leonard J. Moss of SLAC:
C Here's a hybrid QuickSort I wrote a number of years ago.  It's
C based on suggestions in Knuth, Volume 3, and performs much better
C than a pure QuickSort on short or partially ordered input arrays.
      SUBROUTINE SORTRX(N,DATA,INDEX)
C===================================================================
C     SORTRX -- SORT, Real*8 input, indeX output
C     Input:  N     INTEGER
C             DATA  REAL
C     Output: INDEX INTEGER (DIMENSION N)
C This routine performs an in-memory sort of the first N elements of
C array DATA, returning into array INDEX the indices of elements of
C DATA arranged in ascending order.  Thus,
C    DATA(INDEX(1)) will be the smallest number in array DATA;
C    DATA(INDEX(N)) will be the largest number in DATA.
C The original data is not physically rearranged.  The original order
C of equal input values is not necessarily preserved.
C===================================================================
C SORTRX uses a hybrid QuickSort algorithm, based on several
C suggestions in Knuth, Volume 3, Section 5.2.2.  In particular, the
C "pivot key" [my term] for dividing each subsequence is chosen to be
C the median of the first, last, and middle values of the subsequence;
C and the QuickSort is cut off when a subsequence has 9 or fewer
C elements, and a straight insertion sort of the entire array is done
C at the end.  The result is comparable to a pure insertion sort for
C very short arrays, and very fast for very large arrays (of order 12
C micro-sec/element on the 3081K for arrays of 10K elements).  It is
C also not subject to the poor performance of the pure QuickSort on
C partially ordered data.
C Created:  15 Jul 1986  Len Moss
C===================================================================
      INTEGER   N,INDEX(N)
      DOUBLE PRECISION      DATA(N)
      INTEGER   LSTK(31),RSTK(31),ISTK
      INTEGER   L,R,I,J,P,INDEXP,INDEXT
      DOUBLE PRECISION      DATAP
C     QuickSort Cutoff
C     Quit QuickSort-ing when a subsequence contains M or fewer
C     elements and finish off at end with straight insertion sort.
C     According to Knuth, V.3, the optimum value of M is around 9.
      INTEGER   M
CARLO
cc      PARAMETER (M=30)
CARLO
      PARAMETER (M=9)
 
C===================================================================
C
C     Make initial guess for INDEX
 
      DO 50 I=1,N
         INDEX(I)=I
   50    CONTINUE
 
C     If array is short, skip QuickSort and go directly to
C     the straight insertion sort.
 
      IF (N.LE.M) GOTO 900
 
C===================================================================
C
C     QuickSort
C
C     The "Qn:"s correspond roughly to steps in Algorithm Q,
C     Knuth, V.3, PP.116-117, modified to select the median
C     of the first, last, and middle elements as the "pivot
C     key" (in Knuth's notation, "K").  Also modified to leave
C     data in place and produce an INDEX array.  To simplify
C     comments, let DATA[I]=DATA(INDEX(I)).
 
C Q1: Initialize
      ISTK=0
      L=1
      R=N
 
  200 CONTINUE
 
C Q2: Sort the subsequence DATA[L]..DATA[R].
C
C     At this point, DATA[l] <= DATA[m] <= DATA[r] for all l < L,
C     r > R, and L <= m <= R.  (First time through, there is no
C     DATA for l < L or r > R.)
 
      I=L
      J=R
 
C Q2.5: Select pivot key
C
C     Let the pivot, P, be the midpoint of this subsequence,
C     P=(L+R)/2; then rearrange INDEX(L), INDEX(P), and INDEX(R)
C     so the corresponding DATA values are in increasing order.
C     The pivot key, DATAP, is then DATA[P].
 
      P=(L+R)/2
      INDEXP=INDEX(P)
      DATAP=DATA(INDEXP)
 
      IF (DATA(INDEX(L)) .GT. DATAP) THEN
         INDEX(P)=INDEX(L)
         INDEX(L)=INDEXP
         INDEXP=INDEX(P)
         DATAP=DATA(INDEXP)
      ENDIF
 
      IF (DATAP .GT. DATA(INDEX(R))) THEN
         IF (DATA(INDEX(L)) .GT. DATA(INDEX(R))) THEN
            INDEX(P)=INDEX(L)
            INDEX(L)=INDEX(R)
         ELSE
            INDEX(P)=INDEX(R)
         ENDIF
         INDEX(R)=INDEXP
         INDEXP=INDEX(P)
         DATAP=DATA(INDEXP)
      ENDIF
 
C     Now we swap values between the right and left sides and/or
C     move DATAP until all smaller values are on the left and all
C     larger values are on the right.  Neither the left or right
C     side will be internally ordered yet; however, DATAP will be
C     in its final position.
 
  300 CONTINUE
 
C Q3: Search for datum on left >= DATAP
C
C     At this point, DATA[L] <= DATAP.  We can therefore start scanning
C     up from L, looking for a value >= DATAP (this scan is guaranteed
C     to terminate since we initially placed DATAP near the middle of
C     the subsequence).
 
         I=I+1
         IF (DATA(INDEX(I)).LT.DATAP) GOTO 300
 
  400 CONTINUE
 
C Q4: Search for datum on right <= DATAP
C
C     At this point, DATA[R] >= DATAP.  We can therefore start scanning
C     down from R, looking for a value <= DATAP (this scan is guaranteed
C     to terminate since we initially placed DATAP near the middle of
C     the subsequence).
 
         J=J-1
         IF (DATA(INDEX(J)).GT.DATAP) GOTO 400
 
C Q5: Have the two scans collided?
 
      IF (I.LT.J) THEN
 
C Q6: No, interchange DATA[I] <--> DATA[J] and continue
 
         INDEXT=INDEX(I)
         INDEX(I)=INDEX(J)
         INDEX(J)=INDEXT
         GOTO 300
      ELSE
 
C Q7: Yes, select next subsequence to sort
C
C     At this point, I >= J and DATA[l] <= DATA[I] == DATAP <= DATA[r],
C     for all L <= l < I and J < r <= R.  If both subsequences are
C     more than M elements long, push the longer one on the stack and
C     go back to QuickSort the shorter; if only one is more than M
C     elements long, go back and QuickSort it; otherwise, pop a
C     subsequence off the stack and QuickSort it.
 
         IF (R-J .GE. I-L .AND. I-L .GT. M) THEN
            ISTK=ISTK+1
            LSTK(ISTK)=J+1
            RSTK(ISTK)=R
            R=I-1
         ELSE IF (I-L .GT. R-J .AND. R-J .GT. M) THEN
            ISTK=ISTK+1
            LSTK(ISTK)=L
            RSTK(ISTK)=I-1
            L=J+1
         ELSE IF (R-J .GT. M) THEN
            L=J+1
         ELSE IF (I-L .GT. M) THEN
            R=I-1
         ELSE
C Q8: Pop the stack, or terminate QuickSort if empty
            IF (ISTK.LT.1) GOTO 900
            L=LSTK(ISTK)
            R=RSTK(ISTK)
            ISTK=ISTK-1
         ENDIF
         GOTO 200
      ENDIF
  900 CONTINUE
C===================================================================
C Q9: Straight Insertion sort
      DO 950 I=2,N
         IF (DATA(INDEX(I-1)) .GT. DATA(INDEX(I))) THEN
            INDEXP=INDEX(I)
            DATAP=DATA(INDEXP)
            P=I-1
  920       CONTINUE
               INDEX(P+1) = INDEX(P)
               P=P-1
               IF (P.GT.0) THEN
                  IF (DATA(INDEX(P)).GT.DATAP) GOTO 920
               ENDIF
            INDEX(P+1) = INDEXP
         ENDIF
  950    CONTINUE
C===================================================================
C     All done 
      END

*********************************************************************
* Function to calculate the width of the U boson knowing its mass and
* its vectorial coupling
*********************************************************************
      double precision function gammafunc(mass,k)
      implicit none
      double precision mass,k

      double precision ame,ammu,convfac,alpha,pi
      common/parameters/ame,ammu,convfac,alpha,pi

      double precision me, mmu, r, dr

      me=ame
      mmu=ammu

      gammafunc=0.

      if(mass.gt.2.*me) then
         gammafunc = gammafunc + 1./3.*alpha*k**2*mass*
     >               sqrt(1-4.*me**2/mass**2)*(1+2.*me**2/mass**2)
      endif
      if(mass.gt.2.*mmu) then
         gammafunc = gammafunc + 1./3.*alpha*k**2*mass*
     >               sqrt(1-4.*mmu**2/mass**2)*(1+2.*mmu**2/mass**2)
         call r_intpl(mass,r,dr)
         gammafunc = gammafunc + 1./3.*alpha*k**2*mass*
     >               sqrt(1-4.*mmu**2/mass**2)*(1+2.*mmu**2/mass**2)*r
      endif

      return

      end

**********************************************************************
*
* Sceglie il fotone da calcolare con l'elemento di matrice a caso fra 
* quelli che rientrano nei tagli angolari
*********************************************************************
      subroutine sceglifot(ifscelto,ng)
!       implicit none
      integer ifscelto,ng,nrilev,nphotscelto
      integer k,i
      real*4 csi(1)
      integer ifnontagliati(40)
      common/fnontagliati/ifnontagliati

      ifscelto = 1
      if (ng.eq.1) return

      nrilev = 0
      do k=1,ng
        nrilev = nrilev + ifnontagliati(k) 
      enddo
      if (nrilev.eq.1) then
          nphotscelto = 1
      else
!           call ranlux(csi,1)
          csi(1) = babayaganlo_rndm(0)

          nphotscelto = csi(1)*nrilev + 1
      endif
      i = 0
      k = 1
      do while (k.le.ng)
        if (ifnontagliati(k).eq.1) i = i + 1
        if (i.eq.nphotscelto) then 
            ifscelto = k
            return 
        endif
        k=k+1
      enddo

      return
      end
*
      double precision function trinorma(p)
      implicit none
      double precision p(0:3), tridot

      trinorma=sqrt(tridot(p,p))

      return
      end


*************************************************************
*     FROM nunugpv
*
*.....FUNCTION DOP: D(X)/P(X)/ANORMD(XMIN,XMAX). D(X) IS THE 
*                   ELECTRON STRUCTURE FUNCTION
*
*************************************************************

      double precision function dop(x,am,p1,p2)
      implicit none
      double precision x, omx, opx, am
      double precision s
      double precision alx, alomx, alox
      double precision beta, betam, sdelta
      double precision dfunh, pm1
      double precision dot
      integer kk

      double precision ame, ammu, convfac, alpha, pi
      common/parameters/ame,ammu,convfac,alpha,pi

      double precision p1(0:3),p2(0:3),ptmp(0:3) 

      save beta, betam, sdelta


      do kk = 0,3
        ptmp(kk)   = p1(kk)+p2(kk)
      enddo
      s = dot(ptmp,ptmp)
*
      beta = 2.d0*alpha/pi*(dlog(s/am**2) - 1.d0)
      betam = beta/2.d0
*
      sdelta = 1+3.d0/8.d0*beta+(9.d0/128.d0-pi**2/48.d0)*beta**2
*
      omx = 1.D0 - x
      opx = 1.D0 + x
*
      alx = dlog(x)
*
      if (omx.LT.1D-28) then
         alomx = 0.D0
         alox = -1.D0
      else
         alomx = dlog(omx)
         alox  = alx/omx
      endif
*
      pm1 = 1.d0/betam*(1.d0 - x)**(1.d0 - betam)

      dfunh = - beta/4.D0*opx
     #        + beta**2/32.D0*(
     #        - 4.d0*opx*alomx + 3.d0*opx*alx - 4.d0*alox
     #        - 5.d0 - x)
      dop = sdelta + dfunh*pm1
*
      return
      end

****************************************************************************
      subroutine emission (p1, p2, w, particle)
!       implicit none
      
      double precision p1(0:3), p2(0:3), ptmp(0:3)
      double precision dir1(1:3), dir2(1:3)
      real*4 csi(2)
      double precision x1,x2
      double precision s
      double precision w
      double precision x1max, x2max, x1min, x2min
      character*1 particle
      integer kk
      double precision dop
      double precision tridot, dot, am
      double precision and1, and2
      double precision ame, ammu, convfac, alpha, pi
      common/parameters/ame,ammu,convfac,alpha,pi
      double precision beta,betam
      double precision amassainv
      character*6   ord
      common/qedORDER/ord


      if (ord.eq.'born') then
        w = 1.d0
        return
      endif

      if (ord.eq.'struct') then

        do kk=0,3
        ptmp(kk)=p1(kk)+p2(kk)
        enddo

        s = dot(ptmp,ptmp)

        if (particle.eq.'e') then
          am = ame
        elseif (particle.eq.'m') then
          am = ammu
        endif

        beta  = 2.d0*alpha/pi * (dlog(s/(am**2))-1.d0)
        betam = beta/2.d0

        x1min = am/p1(0)
        x2min = am/p2(0)
        x1max = 1.d0
        x2max = 1.d0

!         call ranlux (csi,2)
         csi(1) = babayaganlo_rndm(0)
         csi(2) = babayaganlo_rndm(0)

        x1=1.d0-((1.D0 - x1min)**(betam)*(1.D0-csi(1)*1.d0)
     >         +(1.D0 - x1max)**(betam)*csi(1))**(1.d0/betam)
*
        x2=1.d0-((1.D0 - x2min)**(betam)*(1.D0-csi(2)*1.d0)
     >         +(1.D0 - x2max)**(betam)*csi(2))**(1.d0/betam)

        w = dop(x1,am,p1,p2) * dop(x2,am,p1,p2)

        p1(0) = x1*p1(0)
        p2(0) = x2*p2(0)

        do kk=1,3
          dir1(kk)=p1(kk)/dsqrt(tridot(p1,p1))
          dir2(kk)=p2(kk)/dsqrt(tridot(p2,p2))
        enddo

        do kk=1,3
          p1(kk) = dir1(kk)*dsqrt(p1(0)**2-am**2)
          p2(kk) = dir2(kk)*dsqrt(p2(0)**2-am**2)
        enddo

        and1 = (1.D0-x1min)**(betam)-(1.D0-x1max)**(betam)
        and2 = (1.D0-x2min)**(betam)-(1.D0-x2max)**(betam)

        w = w * and1 * and2

        return
      endif

      end
