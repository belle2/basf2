C***********************************************************************
       subroutine matrix(flvmlm,posi,impul,hel,rst,labcol,
     >                   wgcol,colfl)
C***********************************************************************
C
C Input parameters: FLVMLM(NMAX) particles according
C                   to michelangelo convention, POSI(NMAX) positions of
C                   incoming particles IMPUL(4,8) particles four momenta,
C                   HEL, array of particle helicities 
C                   LABCOL to choose whether su(3) mode only (LABCOL=0) or
C                   color flow is required (LABCOL=1) or dual mode only
C                   (LABCOL=2), WGCOL random number for color flow unweighting
C Output parameters:   RESULT squared matrix element (modulus), COLFLOW 
C                       string representing the selected coulor flow 
C
C
C At present:
C
C     the color flow is returned in the following way: each particle is
C     represented by a pair of integers (n1,n2); two particle (n1,n2), (n3,n4)
C     are colour connected if either n2=n3 or n4=n1 (in this case the coulor
C     flows from 2 to 1). COLFLOW will contain a string of pairs of integers 
C     ordered as follows: d (or dbar) nu_ebar bbar ubar (or u) e^- b glu glu
C     (with gluons ordered according to the ordering of momenta).
C     (COLFLOW=(n1,....,nn,....) only the first nn=2*particles_number
C       elements to be used)   
C
C
C   IMPUL(J,NPART) ===   J=1 Energy of the NPART-th particle
C   IMPUL(J,NPART) ===   J=2,3,4 x,y,z components  of the NPART-th particle 
C                                                         three momentum
C    the order of the momenta is: 
C        dbar nubar bbar u e- b ( glu glu glu)  (processes 1,2,3,4)
C        dbar nubar cbar bbar u e- c b (glu)  (processes > 5 )
C    where all particles are assumed outcoming and incoming gluons
C    must be before outcoming ones
C
         implicit none
C
         integer nmax        !maximum number of external particles, 
         parameter (nmax=10)
         integer nlb
         parameter (nlb=4)    
         double precision impul(4,nmax),wgcol    !the contribution to the integral
         double precision rst
c         complex*16 rst
         integer labcol,colfl(2*nmax),flvmlm(nmax)
         integer posi(nmax)
         integer hel(nmax)             !particles helicities
C
         integer flgdual          !dual (0) or su3 (1) amplitudes
         common/dual/flgdual
         integer color(2*nmax)    !coulor string
         integer colst(2*nmax)
         common/colore/color
         integer ncls,nprc,ant4(4,2),ant6a(6,6),ant2(2,1)
         integer nant,nantl,j3
         parameter (ncls=2)       !different class of processes
         integer nx,proc(nmax),antns(6),ndual,j1,j2
         parameter (ndual=41000)
         double precision dualamp(ndual),colfac,damp,dampref,avgspin
         integer colaux(ndual,2*nmax),ndl,ndla(0:10,1:3),class
         integer rep(nmax),nqrk,nprt,nglu,nlep,ngb,nphot,nh
         common/process/rep,nqrk,nprt,nglu,nlep,ngb,nphot,nh
         integer flginit/0/
         integer fq(nmax),pq(nmax)
         data ndla/0,0,1 ,5 ,23 ,119,719,5039,40319,0,0,
     >             0,1,5 ,23,119,719,5039,0,0,0,0,
     >             0,2,11,59,359,6*0/

         save ndla,ant4,ant2,ant6a,flginit
         complex*16 result
         integer inpint(1000)
         common/initinter/inpint  
C
         data ant2  /1,2/
         data ant4  /1,4,2,3,
     >               1,3,2,4/
         data ant6a  /1,5,2,6,3,4,
     >               1,6,2,4,3,5,
     >               1,6,2,5,3,4,
     >               1,4,2,6,3,5,
     >               1,4,2,5,3,6,
     >               1,5,2,4,3,6/         
c
c$$$         data inpint/ 16,                                                ! N V-A 
c     >        1,1,1,1,1,   1,1,2,1,2,   2,1,1,1,2,   3,1,2,1,1, ! zuu, zdd, w+ud, w-ud,
c     >       10,1,1,1,1,  10,1,2,1,2,   1,2,1,2,1,   1,2,2,2,2, ! Auu, Add, zcc, zss
c     >        2,2,1,2,2,   3,2,2,2,1,  10,2,1,2,1,  10,2,2,2,2, ! W+cs, W-sc, Acc, Ass
c     >       11,1,1,1,1,  11,1,2,1,2,  11,2,1,2,1,  11,2,2,2,2, ! guu, gdd, gcc, gss
c     >        1,1,1,1,1,   1,1,2,1,2,   2,1,1,1,2,   3,1,2,1,1, ! zuu, zdd, w+ud, w-ud,
c$$$     >       10,1,1,1,1,  10,1,2,1,2,   1,1,3,1,3,   1,1,4,1,4, ! Auu, Add, znn, zee
c$$$     >       1,2,4,2,4,   10,2,4,2,4,   1,2,3,2,3,               ! Carlo           ! Zmumu Z numunumu Amumu
c$$$     >        2,1,3,1,4,   3,1,4,1,3,  10,1,4,1,4,              ! W+ne, W-en, Aee
c$$$     >       10,3,1,3,1,  10,3,2,3,2,   1,3,1,3,1,   1,3,2,3,2, ! Att, Abb, ztt,zbb
c$$$     >        2,3,1,3,2,   3,3,2,3,1,                           ! W+tb,W-bt
c$$$     >       11,1,1,1,1,  11,1,2,1,2,  11,2,1,2,1,  11,2,2,2,2, ! guu, gdd, gcc, gss
c$$$     >                0,                                          ! N of yukawa
c$$$     >               15,                                          ! N self-gauge
c$$$     >          1, 2, 3,  10, 2, 3,   4, 2, 3,   5, 1, 1,         ! ZWW, AWW, auxiliary
c$$$     >          5, 1,10,   5,10,10,   5, 2, 3,   6, 1, 2,         ! auxiliary
c$$$     >          6,10, 2,   7, 1, 3,   7,10, 3,   8, 2, 2,         ! auxiliary
c$$$     >          9, 3, 3,  11,11,11,  12,11,11,                    ! auxiliary, ggg, Xgg
c$$$     >                4,                                          ! N H-GAUGE
c$$$     >                1, 1, 1,   1, 2, 3,   3,1,1,   3,2,3,        ! HZZ, HWW, HHZZ, HHWW
c$$$     >                3,                                          ! N of self higgs
c$$$     >                1, 2, 3,                                     ! HHH, HHHH e aux per higgs quartici
c$$$     >                855*-100/                                   ! EOF signal
c$$$
         data inpint/ 6,
     >        1,1,4,1,4, ! Zee
     >        1,2,4,2,4, ! Zmumu
     >       10,1,4,1,4, ! Aee
     >       10,2,4,2,4, ! Amumu
     >       14,1,4,1,4, ! Uee
     >       14,2,4,2,4, ! Umumu
     >       969*-100   /
         

c         data inpint/ 6,
c     >               11,1,1,1,1,  11,1,2,1,2,  11,3,2,3,2,   2,1,1,1,2,  ! guu, gdd, gbb, w+ud
c     >                3,1,4,1,3,  11,2,1,2,1,                            ! w-en, gcc
c     >                0,                                                 ! N of yukawa
c     >                2,                                                 ! N self-gauge
c     >                11,11,11,  12,11,11,                               ! ggg Auxgg
c     >                961*-100/
c
         if (labcol.eq.0) then
C
          flgdual=1
          call matrix0(flvmlm,posi,impul,hel,result)
c
         elseif (labcol.eq.1) then
c     
          j1=0
          do j3=1,nmax
           if (abs(flvmlm(j3)).gt.0.and.abs(flvmlm(j3)).le.6) then
            j1=j1+1
            fq(j1)=flvmlm(j3)
            pq(j1)=j3
           endif
          enddo
          if(j1.ne.nqrk) then
           write(*,*)'something wrong in matrix, NQRK not ok',nqrk,j3
          endif
c
          flgdual=0
          do j1=1,2*nmax
           colst(j1)=color(j1)
          enddo 
c
          do j1=1,nmax
           proc(j1)=abs(rep(j1))
          enddo
c
          ndl=0
          if(nqrk.eq.2) then
           nantl=1
          elseif(nqrk.eq.4) then
           nantl=2
          elseif(nqrk.eq.6) then
           nantl=6
          else
           write(*,*)'Cannot deal with',nqrk,'quarks'
           stop
          endif
          do j3=1,nantl
           do j1=1,nqrk
            if(nqrk.eq.2) then
             antns(j1)=pq(ant2(j1,j3))
            elseif(nqrk.eq.4) then
             antns(j1)=pq(ant4(j1,j3))
            elseif(nqrk.eq.6) then
             antns(j1)=pq(ant6a(j1,j3))
            endif
           enddo
           j2=nqrk/2
           do j1=0,ndla(nglu,j2)
            nx=j1
            if(nqrk.eq.2) then
             call gendual2q(proc,antns,nglu,colst,nx,colfac) 
            elseif(nqrk.eq.4) then
             call gendual4q(proc,antns,nglu,colst,nx,colfac) 
            elseif(nqrk.eq.6) then
             call gendual6q(proc,antns,nglu,colst,nx,colfac) 
            endif 
            if(abs(colfac).gt.1.d-20) then
             ndl=ndl+1
             call matrix0(flvmlm,posi,impul,hel,result)
             dualamp(ndl)=abs(result*colfac)**2
             do j2=1,2*nmax
              colaux(ndl,j2)=color(j2)
             enddo
            endif
           enddo
          enddo
c
          damp=0
          do j1=1,ndl
           damp=damp+dualamp(j1)
          enddo
          dampref=damp*wgcol
          j1=0
          damp=0.
          do while(damp.lt.dampref.and.j1.lt.ndl)
           j1=j1+1
           damp=damp+dualamp(j1)
          enddo
          ndl=j1
          if(ndl.eq.0) then
             do j1=1,2*nmax
                colfl(j1)=colst(j1)
             enddo
          else 
             do j1=1,2*nmax
                colfl(j1)=colaux(ndl,j1)
             enddo
          endif
C
         else
          write(6,*)'wrong LABCOL in matrix',labcol
          stop
         endif
C
         rst=abs(result)**2
c         rst=result
C
         return
         end







