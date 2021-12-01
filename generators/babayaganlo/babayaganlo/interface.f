      subroutine alpha_me(spinmode,ng,pin1,pin2,p1,p2,qph,
     &                    ialpha,emtxalpha)
      implicit double precision (a-h,o-z)
      parameter (imaxph = 40)
      dimension p1(0:3),p2(0:3),pin1(0:3),pin2(0:3)
      dimension qph(imaxph,0:3),randvec(3)
      character*1 spinmode
      real*4 csi(1)
      parameter (maxpar = 10)
      integer alphafl(maxpar),instate(2),iflag
      double precision alphap(4,maxpar)
      common/forALPHA/alphafl,instate,alphap
      common/nmaxphalpha/nphmaxalpha
      common/parameters/ame,ammu,convfac,alpha,pi
! spinmode = 'S' : summed over spins
! spinmode = 'R' : random over spins

      nspinconf = 2**(4+ng)
      ialpha    = 0
      emtxalpha = 0.d0

      if (ng.ge.(nphmaxalpha+1)) then 
!     max particles in alpha = 10 --> ng_max = 6...
         print*,'event accepted with',ng,' photons'
         print*,'for ng >= ',nphmaxalpha,' ALPHA does not work.'
         print*,'Event rejected'
         return
      endif

      ialpha = 1
      call fill_alpha_mom(ng,pin1,pin2,p1,p2,qph)

      randvec(1) = (1.-0.5)/nspinconf
      randvec(2) = 0.5d0
      
      if (spinmode.eq.'S') then
         do j=1,nspinconf
            randvec(3)=(float(j)-0.5)/nspinconf
            call new_alpha(ng,randvec,alphap,alphafl,instate,
     >                     emtrx,iflag)                           
            emtxalpha = emtxalpha + emtrx 
*            print*,nspinconf,j2,emtrx
         enddo
      else
         call ranlux(csi,1)
         randvec(3) = csi(1)
         call new_alpha(ng,randvec,alphap,alphafl,instate,
     >                  emtrx,iflag)                           
         emtxalpha = emtrx
         emtxalpha = emtxalpha*nspinconf ! this if spin config are random      
      endif

      emtxalpha = emtxalpha
c      emtxalpha = emtxalpha/factorial(ng)
      return
      end
****************************************
      subroutine new_alpha(ng,rvec,alphap,alphafl,instate,evtme,iflag)
C
C  Input :  ALPHAFL is an array of integers, ALPHAFL(j) is the flavour
C             of the jth particle coded according to data particle 
C             convenctions
C           ALPHAFL(NPAR+1) must be equal to 1001, where NPAR=number of
C             external particle. The order of the particles in the array
C             must be in agreement with alpha convenctions.
C           ALPHAP contains the particles momenta, ALPHAP(1:4,j) is the
C             four momentum of the j-th particle 
C             (order is the same of ALPHAFL)
C               ---  ALPHAP(1:4,j) == (E,p_x,p_y,p_z)  ---
C           INSTATE is an array of integers, it contains the positions in
C             the array  ALPHAFL of the two incoming partons 
C             (for e^+ e^- -> .. INSTATE(1) is always the position of e^- 
C             and INSTATE(2) the position of e^+)
C           RVEC three random numbers to select spin and coulors randomly
C
C  Output : EVTME squared Matrix Element
C           IFLAG = 0 if a non vanishing coulor and spin configuration is
C             selected 1 otherwise
C
      implicit none
c
      integer maxpar
      parameter (maxpar=10)
      integer alphafl(maxpar),instate(2),iflag
      double precision rvec(3),alphap(4,maxpar),evtme
c
      double precision rn1,rn2,rspin,wgcol,spinweight
      integer hel(maxpar),colfl(2*maxpar),ng
      character*2 fs
      common/finalstate/fs
c
      save hel

      rn1   = rvec(1)
      rn2   = rvec(2)
      rspin = rvec(3)
      
      iflag = 0
!      call selcol(alphafl,instate,rn1,rn2,iflag)      
      if(iflag.eq.1)  then
         evtme = 0.d0
         return                 ! trovare l'efficienza di colore
      endif

      call selspin_CARLO(ng,rspin,hel)
c      print*,hel
      call matrix(alphafl,instate,alphap,hel,evtme,0,wgcol,
     $            colfl)
c      call matrix0(alphafl,instate,alphap,hel,evtme)
      return
      end


******************************************

      subroutine init_apar
      implicit double precision (a-h,m,o-z)
      common/parameters/ame,ammu,convfac,alpha,pi
      common/alphapar/apar(100)
      common/darkmatter/amassU,gammaU,gvectU,gaxU
      common/idarkon/idarkon
      character*2 fs
      common/finalstate/fs
      data apar/100*0.d0/

      ME  = ame
      MMU = ammu

      MU = amassU

      WU = gammaU

      apar(1) = MU
      apar(2) = WU
      apar(11)= ME      ! EM      ! ELECTRON MASS
      apar(12)= MMU     ! AMU     ! MUON MASS
!
      

      apar(20)= sqrt(4.d0*pi*alpha)
      apar(21)= gvectU
      apar(22)= gaxU

      return
      end


*************************************************************
      subroutine fill_alpha_mom(ng,pin1,pin2,p1,p2,qph)
      implicit double precision (a-h,o-z)
      parameter (imaxph = 40)
      dimension p1(0:3),p2(0:3),pin1(0:3),pin2(0:3),con(1:4)
      dimension qph(imaxph,0:3),q(0:3)
      parameter (maxpar = 10)
      integer alphafl(maxpar),instate(2 ),iflag
      double precision alphap(4,maxpar),con
      common/forALPHA/alphafl,instate,alphap
      character*2 fs
      common/finalstate/fs

      instate(1) = 1
      instate(2) = 3

! mu- is 13, mu+ is -13.
! e-  is 11, e+  is -11.

      alphafl(1) =  11 ! initial e-
      alphafl(3) = -11 ! initial e+
      if (fs.eq.'mm') then
        alphafl(2) = -13 ! final mu+
        alphafl(4) =  13 ! final mu-
      elseif (fs.eq.'ee') then
        alphafl(2) = -11 ! final e+
        alphafl(4) =  11 ! final e-
      endif
      if (ng.gt.0) then
         do i = 1,ng
            alphafl(4+i) = 22
         enddo
      endif
      do i=4+ng+1,maxpar
         alphafl(i) = 1001
      enddo
c      if (ng.lt.6) alphafl(4+ng+1) = 1001
      do k = 0,3
         alphap(k+1,1) = pin1(k)
         alphap(k+1,2) = p2(k)
         alphap(k+1,3) = pin2(k)
         alphap(k+1,4) = p1(k)
      enddo
      if (ng.gt.0) then
         do i = 1,ng
            do k = 0,3
               alphap(k+1,4+i) = qph(i,k)
            enddo
         enddo
      endif
      con(1)=0.
      con(2)=0.
      con(3)=0.
      con(4)=0.
      do i=1,5
         con(1)=con(1)+alphap(1,i)
         con(2)=con(2)+alphap(2,i)
         con(3)=con(3)+alphap(3,i)
         con(4)=con(4)+alphap(4,i)
      enddo
      
      return
      end
*
C**************************************************************************
         subroutine selspin_CARLO(ng,rspin,hel)
C**************************************************************************
C
C Input parameters:   RSPIN random number to select spin configuration
C Output parameters:  HEL helicity configuration 
C
         implicit none
C
         integer nmax
         parameter (nmax=10)
C
         double precision rspin
         integer hel(nmax),ng
C
         integer rep(nmax),nqrk,nprt,nglu,nlep,ngb,nphot,nh
         common/process/rep,nqrk,nprt,nglu,nlep,ngb,nphot,nh
         integer nspmax
         parameter (nspmax=1024)
         integer j1,j2
C
         nh=nprt-nglu-nqrk-ngb-nlep-nphot
C

CARLO
!	      print*,'Aint.f --- CARLO'
         nprt = 4 + ng
         nglu = 0
         nqrk = 0
         ngb  = 0
         nlep = 4
         nphot= ng
CARLO
         j2=rspin*2**(nglu+nqrk+nphot+nlep)*3**ngb
         do j1=1,nqrk+nlep
           hel(j1)=mod(j2,2)
           if(hel(j1).eq.0)hel(j1)=-1
           j2=j2/2       
         enddo
         do j1=nqrk+nlep+1,nqrk+nlep+nh
            hel(j1)=0
         enddo
         do j1=nqrk+nlep+nh+1,nqrk+nlep+nh+ngb
           hel(j1)=mod(j2,3)-1
           j2=j2/3       
         enddo
         do j1=nqrk+nlep+nh+ngb+1,nqrk+nlep+nh+ngb+nglu+nphot
            hel(j1)=mod(j2,2)
            if(hel(j1).eq.0) hel(j1) = -1
            j2=j2/2       
         enddo
C
         return
         end

