C#########################################################################    
C
C ALPHA 26/5/1995 by F. Caravaglios and M. Moretti
C
C ALPHA 31/7/1996 modified 
C
C ALPHA modified jan-may 1998 
C
C#########################################################################    
C
C  IMPORTANT WARNINGS.
C
C 1)    Routine for color assignement might not work for majorana fermions
C 2)    To optimize computation a few auxiliary arrays are computed once
C       for all in INITCOMPCONF. Dealing with multiple process this is
C       going to fail if the number of particles is not constant
C 3)    Diagonal gluons are assumed not to propagate, should this become
C       important proper modifications have to be made (non identical gluons
C                                                      have to be considered)
C
C#########################################################################    
C***********************************************************************
          subroutine itera(elmat)
C***********************************************************************
C
C This subroutine perform the perturbative iteration.
C 
          implicit none
C
          integer countsour   !to count the number of already initilaized
C                               sources
          integer flagread
          integer jloop,j1,j2,j3,j4,j5 !loop variables
          integer labvma (100,5), labyuk (10,4), labselfgau (50,3),
     >            labgauhg (50,3), labslfh(10,1)   
                                       !labelling non zero interactions
          integer nvma ,nyuk, nselfgau, ngauhg,nslfh
          integer next       !number of external particles
          integer nfermion   !number of external fermions
          integer nmax       !maximum number of external particles
          parameter (nmax=10) !maximal number of external particle. Change here
          integer dimnum       !dimension of the array NUMBfield
          integer dimmom       !dimension of the array MOMfield
          integer dimcons      !dimension of the array CONSTfield
          integer dimop        !dimension of the array OPERATOR
C
          parameter (dimmom=(2**nmax-2)/2 +100) !number of allowed momentum 
C                     configuration 
          parameter (dimnum=nmax/2)      !storing the number of configuration
C                                    with 1,...,4 momenta contributing
          parameter (dimop=2)
          parameter (dimcons=5)      !number of needed particle caracteristics.
          integer iteration   !storing the present iteration stage
          integer niteration !number of iteration steps
          integer operator(dimop)  !a flag to identify the type of operator, 
          integer write       !flag to control output
C
          double precision coupmin
C
          complex*16 a,b,c,count1,count2
          complex*16 elmat      !to store matrix element
C
          common/iter/niteration
          common/external/next,nfermion !containing the number of external
C                                        particles from the subroutine ??????
          common/vecass/a,b,count1,count2
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C  declaring the variables for higgs, gauge-bosons, fermions and fermionbars 
C  fields
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C setting the variables defininig the higgs field
C
          integer nhigbos
          parameter (nhigbos=3) !higgs field 1) higgs 2) Y_h 3) X_h
          double precision consthiggs(dimcons,nhigbos)
C         containing the characteristic of the higgs fields:
C                              1) mass
C                              2) Lorentz nature 
C                              3) coulor
C                              4) particle "name" see ??? for the conventions
C                              5) particle width
          integer dimhiggs     !dimension of the array field for higgs field
          parameter (dimhiggs=dimmom)  !dimension of a scalar, coulor singlet
C                                       higgs field.
          complex*16 higgs(dimhiggs,nhigbos) !array containing the higgs 
C                                             field configurations 
          integer momhiggs(3,dimmom,nhigbos)  !array containing a label for 
C      each indipendent momentum configuration. This is for the higgs field

          integer numbhiggs(dimnum,nhigbos) !NUMBHIGGS(j,k) contains the 
C                                            number of higgs configurations 
C                                            with j momenta
          integer starthiggs(nhigbos) !to help disentangling the structure of
C                                      the variable
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C setting the variables defininig the gauge-bosons field
C
          integer ngaubos
          parameter (ngaubos=14)  ! Gauge bosons fields 
C                                   1)  Z 
C                                   2)  W^+ 
C                                   3)  W^- 
C                                   4)  X_{+-} 
C                                   5)  Y_{+-} 
C                                   6)  X_{3+} 
C                                   7)  Y_{3+} 
C                                   8)  X_{++} 
C                                   9)  Y_{++} 
C                                   10) A
C                                   11) gluons
C                                   12) G_mn 
C                                   13) "Abelian gluon" 
C                                   14) "Dark Matter U" 
C            (for the definition of X, Y G and G_mn see notes)
C
          double precision constgaubosons(dimcons,ngaubos)
          integer dimgaubosons               
          parameter (dimgaubosons=36*dimmom) ! 36  =  
C            4 (lorentz degrees of freedom) * 4 (Z,W,W,A) 
C          + 6 (number of X,Y fields) 
C          + 4 (Lor dof) gluon
C          + 6 (Lor dof) G_mn  
C          + 4 (Lor dof) "Abelian gluon")
          integer momgaubosons(3,dimmom,ngaubos)   
          integer numbgaubosons(dimnum,ngaubos) 
          complex*16 gaubosons(dimgaubosons)     
          integer startgaubosons(ngaubos)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C setting the variables defininig the fermions field
C
          integer nfer1
          integer nfer2
          parameter (nfer1=3) ! 3 fermion families
          parameter (nfer2=4) ! 4 fermions per family          
          double precision constfermion(dimcons,nfer2,nfer1)  
          integer dimfermion    
          parameter (dimfermion=dimmom*16)  
c         dimension of a fermion family
C               4 (2 quarks  and 2 leptons) * 4 (lorentz dof)
          complex*16 fermion(dimfermion,nfer1)      
          integer momfermion(3,dimmom,nfer2,nfer1) 
          integer numbfermion(dimnum,nfer2,nfer1)  
          integer stfermion(nfer2)                 
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C setting the variables defininig the fermionsbar field
C
          double precision constfermionbar(dimcons,nfer2,nfer1) 
          complex*16 fermionbar(dimfermion,nfer1) 
          integer momfermionbar(3,dimmom,nfer2,nfer1) 
          integer numbfermionbar(dimnum,nfer2,nfer1)  
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Containing the common for coupling constants. Shared by the subroutines   C
C ITERA and COUPLINGS.                                                      C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          complex*16 selfhiggs(3), yukawa(3,4,3,4), 
     >               vectorial(ngaubos,3,4,3,4),assial(ngaubos,3,4,3,4),
     >               selfgauge(ngaubos,ngaubos,ngaubos),
     >               higgsgauge(3,ngaubos,ngaubos)  
C
          common/coupconst/selfhiggs,yukawa,vectorial,selfgauge,
     >                    higgsgauge,assial   !add here new couplings
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Containing the common for particles masses. Shared by the subroutines     C
C ITERA and COUPLINGS                                                       C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
c
          double precision masshiggs(3), massgaubosons(ngaubos), 
     >           massfermion(3,4),
     >           widthhiggs(3), widthgaubosons(ngaubos),
     >           widthfermion(3,4)  
C
          common/masses/masshiggs,massgaubosons,massfermion ,
     >                  widthhiggs,widthgaubosons,widthfermion 
                        !add here new particles
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Containing the common for number,coulor and spin of external particles.   C
C Shared by the                                                             C
C subroutines ITERA, PROCESSO and SPINVARIABLE; ITERA and COLORCONF;        C
C ITERA, PROCESSO, FILLCONFSPIN AND SPINCOULOR respectively                 C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
         integer nsourfermion    !to store the number of external fermions
         integer nsourfermionbar !to store the number of external fermions
         integer nsourhiggs      !the number of external higgs particles
         integer nsourgaubosons  !the number of external "topbar" particles
         double precision spinsour   !array containing the spin of the source
         double precision spinsouraux!array containing the spin of the source
         common/source/nsourhiggs(3),nsourgaubosons(ngaubos),
     >                 nsourfermion(3,4),nsourfermionbar(3,4)
         common/spin/spinsour(nmax),spinsouraux(nmax)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
         double precision cf3(nfer2)  !auxiliary variable to initialize the 
C                           array CONSTFERMION: 2=triplet 1=singlet the full 
C                           set of conventions is in subroutine COULOR 

         double precision ch2(3) !same as above for CONSTHIGGS:
C                                2=scalar, 1=auxiliary see subroutine LORENTZ
         double precision cg2(ngaubos) !same as above for CONSTGAUBOSONS:
C     3=massive g.b., 5. auxiliary g.b., 6=massles g.b. 7 aux gluons , 
C           (see subroutine LORENTZ)
         double precision cg3(ngaubos) !same as above for CONSTGAUBOSONS
C                 1=singlet, 3=adjoint see subroutines COLOREPROD e COLORELAG
         integer nnaux
         parameter (nnaux=ngaubos**3)
         integer osg1(ngaubos,ngaubos,ngaubos) !same as above for g.b. self
C         interaction see subroutines  LOROPER
         integer optmulti     !optmulti=1 options to deal with multi
C                              processes simultaneously. no input files
C                              optmulti=0 one process only; LABELINTERACTION 
C                              and PROCESSO.DAT input files 
         common/options/optmulti
C
C
C DATA statment
C
          data flagread/0/
          data write/0/
          data coupmin/1.d-10/
          data starthiggs/0,1,2/             
          data cf3/2.,2.,1.,1./
          data ch2/2.,1.,1./
          data cg2/3*5.,6*1.,2*6.,7.,6.,5./
          data cg3/10*1.,2*3.,1.,1./
          data startgaubosons /0,4,8,12,13,14,15,16,17,18,22,26,32,35/
          data osg1/nnaux*0/
          data stfermion/0,4,8,12/    
C
C Saving variables
C
         save flagread,labvma,labyuk,labselfgau,labgauhg,nvma,nyuk
         save ngauhg,starthiggs,startgaubosons,nselfgau,nslfh,labslfh
         save stfermion
         save constfermionbar, constgaubosons,constfermion, consthiggs
         save cf3,ch2,cg2,osg1,cg3
C
C
           if (optmulti.eq.1) then
c            write(6,*)'optmulti',optmulti   
            call readarrtwobytwo_h(labvma,100,5,nvma)
            call readarrtwobytwo_h(labyuk,10,4,nyuk)
            call readarrtwobytwo_h(labselfgau,50,3,nselfgau)
            call readarrtwobytwo_h(labgauhg,50,3,ngauhg)  
            call readarrtwobytwo_h(labslfh,10,1,nslfh)  
           elseif (optmulti.eq.0) then
            if (flagread.eq.0) then
             write(6,*)'optmulti',optmulti   
             open (1,file='labelinteraction',status='old')
              call readarrtwobytwo(labvma,100,5,nvma,1)
              call readarrtwobytwo(labyuk,10,4,nyuk,1)
              call readarrtwobytwo(labselfgau,50,3,nselfgau,1)
              call readarrtwobytwo(labgauhg,50,3,ngauhg,1)
              call readarrtwobytwo_h(labslfh,10,1,nslfh)  
             close (1)
            endif
           endif
C
C  initializing some auxiliary array
C
C luca
c           if(flagread.eq.0) then
c            flagread=1
c            osg1(1,2,3)=5
c            osg1(10,2,3)=5
c            osg1(4,2,3)=6
c            osg1(5,1,1)=6
c            osg1(5,1,10)=6
c            osg1(5,10,10)=6
c            osg1(5,2,3)=6
c            osg1(6,1,2)=6
c            osg1(6,10,2)=6
c            osg1(7,1,3)=6
c            osg1(7,10,3)=6
c            osg1(8,2,2)=6
c            osg1(9,3,3)=6
c            osg1(11,11,11)=5
c            osg1(12,11,11)=7
c           endif
C
          jloop=nfer1*nfer2*dimnum
          call zeroarrayint(numbfermion,jloop)
          call zeroarrayint(numbfermionbar,jloop)
          jloop=nhigbos*dimnum
          call zeroarrayint(numbhiggs,jloop)
          jloop=ngaubos*dimnum
          call zeroarrayint(numbgaubosons,jloop)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C Initializing the SM particles variables
C 
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
         countsour=1
C
CCCCCC   Initializing fermion fields  CCCCCCC
C
           do j2=1,nfer1                 !loop over family index
            do j1=1,nfer2                 !loop over family members
             constfermion(1,j1,j2)=massfermion(j2,j1)                 
             constfermion(5,j1,j2)=widthfermion(j2,j1)
             constfermion(2,j1,j2)=3. 
             constfermion(3,j1,j2)=cf3(j1)  
             constfermion(4,j1,j2)=100.+dfloat(4*(j2-1)+j1)   !particle label,
C                                                              fermions
C                                                will be labelled 101,...112
             if (nsourfermion(j2,j1).ne.0) then   
              do j3=countsour,countsour+nsourfermion(j2,j1)-1
               call initial(fermion(stfermion(j1)*dimmom+1,j2),
     >              momfermion(1,1,j1,j2),constfermion(1,j1,j2),
     >              numbfermion(1,j1,j2),j3,
     >            countsour,spinsour(j3))
              enddo
              countsour=countsour+nsourfermion(j2,j1)
C
             endif
C
            enddo
           enddo
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
CCCCCC   Initializing fermionbar fields  CCCCCCC
C
           do j1=1,nfer1                  !loop over family index
            do j2=1,nfer2                 !loop over family memebers
             constfermionbar(1,j2,j1)=massfermion(j1,j2)                 
             constfermionbar(5,j2,j1)=widthfermion(j1,j2)
             constfermionbar(2,j2,j1)=4. 
             constfermionbar(3,j2,j1)=cf3(j2) 
             constfermionbar(4,j2,j1)=200.+dfloat(4*(j1-1)+j2)   !particle 
C                           label,  fermionbars will be labelled 201,...212
             if (nsourfermionbar(j1,j2).ne.0) then   
              do j3=countsour,countsour+nsourfermionbar(j1,j2)-1
               call initial(fermionbar(stfermion(j2)*dimmom+1,j1),
     >           momfermionbar(1,1,j2,j1),constfermionbar(1,j2,j1),
     >           numbfermionbar(1,j2,j1),j3,
     >           countsour,spinsour(j3))
              enddo
              countsour=countsour+nsourfermionbar(j1,j2)
C
             endif
C
            enddo
           enddo 
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
CCCCCC   Initializing boson fields  CCCCCCC
C
C Initializing Higgs field. 
C
           do j1=1,nhigbos            !higgs and its auxiliary field
            consthiggs(1,j1)=masshiggs(j1)   !higgs mass 
            consthiggs(5,j1)=widthhiggs(j1)
            consthiggs(2,j1)=ch2(j1)
            consthiggs(3,j1)=1.       !coulor singlet (see subroutine 
C                                      COULOR for the full set of conventions
            consthiggs(4,j1)=300+j1   !particle label:
C                             301,302)    =higgs,auxiliary-higgs
C                             101,...,112)=fermions
C                             201,...,212)=fermionsbar
C                             401,...,403)=Z,W^+,W^-,...,A
            if (nsourhiggs(j1).ne.0) then  !NSOURHIGG contain the number
C                                           of external higgs field
             do j2=countsour,countsour+nsourhiggs(j1)-1
              call initial(higgs(1,j1),momhiggs(1,1,j1),consthiggs(1,j1)
     >         ,numbhiggs(1,j1),j2,countsour,spinsour(j2))      
             enddo
C            momenta configuration in the array MOM1. The array
C            NUMB1(j) contain the number of higgs field configurations
C            with J momenta contributing .The Arrays SPINSOUR and COLOURSOUR 
C            contain the spin and the coulor of the external source 
C            and are setted in the subroutine ???????
C
             countsour=countsour+nsourhiggs(j1)
C 
            endif
C
           enddo
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
CCCCCC   Initializing gauge-boson fields  CCCCCCC
C
           do j1=1,ngaubos
            constgaubosons(1,j1)=massgaubosons(j1)
            constgaubosons(5,j1)=widthgaubosons(j1)
            constgaubosons(2,j1)=cg2(j1)     
            constgaubosons(3,j1)=cg3(j1)
            constgaubosons(4,j1)=400. + dfloat(j1)
            if (nsourgaubosons(j1).ne.0) then   
             do j2=countsour,countsour+nsourgaubosons(j1)-1
              call initial(gaubosons(startgaubosons(j1)*dimmom+1),
     >               momgaubosons(1,1,j1) ,constgaubosons(1,j1),  
     >               numbgaubosons(1,j1),j2,countsour,
     >               spinsour(j2))
             enddo
             countsour=countsour+nsourgaubosons(j1)
C
            endif
C
           enddo
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Obtaining the fields configuration iterating the equation of motion       C
C and the matrix element from G                                             C
C
           elmat=0.   !It will contain the matrix element
C
           niteration=next/2           !number of iteration steps needed
C
           do iteration=2,niteration+1   !loop to generate the iterations.
C                     the last run is needed to compute the matrix element
C
C Using the interaction terms to iterate the equation of motion
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Higgs self-interaction:                                                   C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
            operator(1)=1  !lorentz cubic scalar interaction without
c                    derivatives for the full set of convention see the
c                    subroutine LOROPER
            operator(2)=0  !coulor singlet
            do j1=1,nslfh
C
             if (abs(selfhiggs(j1)).gt.coupmin) then
C
             j2=labslfh(j1,1)
             call compute(higgs(1,j2), !The first 3 entries are the 3 fields
     >         higgs(1,1),             !entering in the interaction. The 
     >         higgs(1,1),             !ordering is fixed in subroutine 
c                                        LOROPER and COLOPER
     >         momhiggs(1,1,j2),!The 4,5,6 entryes contains the corresponding
     >         momhiggs(1,1,1), !configurations label. The orde MUST be
     >         momhiggs(1,1,1), !the same chose for 1,2,3
     >         numbhiggs(1,j2), !The 7,8,9 entries are arrays containing
     >         numbhiggs(1,1),  !additional information about the field
     >         numbhiggs(1,1),  !configuration. The order again as 1,2,3
     >         consthiggs(1,j2),!Containing the mass the lorentz and coulor
     >         consthiggs(1,1), !nature of the field. Again the order as
     >         consthiggs(1,1), !1,2,3
     >         operator,        !To identify the type of operator
     >         iteration,       !Iteration step
     >         selfhiggs(j2),   !Interaction coupling constant
     >         elmat,           !matrix element
     >         niteration)      !number of perturbative iteration steps

             endif
C
            enddo
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Yukawa interactions                                                       C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
            operator(1)=2
            operator(2)=0
C
           if (nyuk.gt.0) then
            do jloop=1,nyuk
             j1=labyuk(jloop,1)
             j2=labyuk(jloop,2)
             j3=labyuk(jloop,3)
             j4=labyuk(jloop,4)
C
                 call compute(higgs(1,1),
     >            fermionbar(stfermion(j2)*dimmom+1,j1),
     >            fermion(stfermion(j4)*dimmom+1,j3),momhiggs(1,1,1),
     >            momfermionbar(1,1,j2,j1),momfermion(1,1,j4,j3),
     >            numbhiggs(1,1),numbfermionbar(1,j2,j1),
     >            numbfermion(1,j4,j3),consthiggs(1,1),
     >            constfermionbar(1,j2,j1),constfermion(1,j4,j3),
     >            operator,iteration,yukawa(j1,j2,j3,j4),elmat,
     >                                                    niteration)  
C
            enddo
           endif
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C V-A interactions with electroweak gauge bosons                            C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

           operator(1)=3     
           if(nvma.gt.0) then      
            do jloop=1,nvma
c              if(j5.eq.11) then     !gluons
c               operator(2)=1        !adjoints q qbar
c              else
c               operator(2)=0        !singlets
c              endif
              j5=labvma(jloop,1)
              j1=labvma(jloop,2)
              j2=labvma(jloop,3)
              j3=labvma(jloop,4)
              j4=labvma(jloop,5)
              if(j5.ne.11) then
               a=vectorial(j5,j1,j2,j3,j4)
               b=assial(j5,j1,j2,j3,j4)
               c=(1.,0.)
              else
               a=1.d0
               b=0.d0
               c=vectorial(j5,j1,j2,j3,j4)
              endif
               count1=j5
               count2=j2
              call compute(gaubosons(startgaubosons(j5)*dimmom+1),
     >                     fermionbar(stfermion(j2)*dimmom+1,j1),
     >                     fermion(stfermion(j4)*dimmom+1,j3),
     >                     momgaubosons(1,1,j5),
     >                     momfermionbar(1,1,j2,j1),
     >                     momfermion(1,1,j4,j3),
     >                     numbgaubosons(1,j5),numbfermionbar(1,j2,j1),
     >                     numbfermion(1,j4,j3),constgaubosons(1,j5),
     >                     constfermionbar(1,j2,j1),
     >                     constfermion(1,j4,j3),
     >                     operator,iteration,c,elmat,niteration)
C
            enddo
           endif
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C gauge bosons self-interactions with electroweak gauge bosons              C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

           if (nselfgau.gt.0) then
            do jloop=1,nselfgau              !loop over gauge bosons

             j5=labselfgau(jloop,1)
             j1=labselfgau(jloop,2)
             j3=labselfgau(jloop,3)
             if(j5.eq.11) then
              operator(2)=3                  !3 gluons
             elseif(j5.eq.12) then
              operator(2)=4                  !auxiliary + 2 gluons
             else
              operator(2)=0
             endif
C
             operator(1)=osg1(j5,j1,j3)
              call compute(gaubosons(startgaubosons(j5)*dimmom+1),
     >             gaubosons(startgaubosons(j1)*dimmom+1), 
     >             gaubosons(startgaubosons(j3)*dimmom+1),
     >             momgaubosons(1,1,j5),
     >             momgaubosons(1,1,j1),momgaubosons(1,1,j3),
     >             numbgaubosons(1,j5),numbgaubosons(1,j1),
     >             numbgaubosons(1,j3),constgaubosons(1,j5),
     >             constgaubosons(1,j1),constgaubosons(1,j3),
     >             operator,iteration,selfgauge(j5,j1,j3),
     >             elmat,niteration)
C
            enddo
C
           endif
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C gauge bosons -higgs  interactions with electroweak gauge bosons           C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

           if (ngauhg.gt.0) then
            do jloop=1,ngauhg              !loop over higgs bosons
             j5=labgauhg(jloop,1)
             j1=labgauhg(jloop,2)
             j3=labgauhg(jloop,3)

             operator(1)=6
             operator(2)=0
             call compute(higgs(1,j5),
     >             gaubosons(startgaubosons(j1)*dimmom+1), 
     >             gaubosons(startgaubosons(j3)*dimmom+1),
     >             momhiggs(1,1,j5),     
     >             momgaubosons(1,1,j1),momgaubosons(1,1,j3),
     >             numbhiggs(1,j5),numbgaubosons(1,j1),
     >             numbgaubosons(1,j3),consthiggs(1,j5),
     >             constgaubosons(1,j1),constgaubosons(1,j3),
     >             operator,iteration,higgsgauge(j5,j1,j3),
     >             elmat,niteration)

            enddo

           endif

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
          if(iteration.le.niteration) then
C
CCCCCC Exchanging fermion and fermionbar fields  CCCCCCC
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
           do j1=1,nfer1                  !loop over family index
            do j2=1,nfer2                 !loop over family memebers
C
             call exchange(fermionbar(stfermion(j2)*dimmom+1,j1),
     >         fermion(stfermion(j2)*dimmom+1,j1),
     >         momfermionbar(1,1,j2,j1),
     >         momfermion(1,1,j2,j1),numbfermionbar(1,j2,j1),
     >         numbfermion(1,j2,j1),constfermion(1,j2,j1),iteration)
C
            enddo
           enddo 

C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
CCCCCC Exchanging W^+ and W^-, X and Y  fields  CCCCCCC

            do j1=1,4

             call exchange(gaubosons(startgaubosons(2*j1)*dimmom+1),
     >         gaubosons(startgaubosons(2*j1+1)*dimmom+1),
     >         momgaubosons(1,1,2*j1),
     >         momgaubosons(1,1,2*j1+1),numbgaubosons(1,2*j1),
     >         numbgaubosons(1,2*j1+1),constgaubosons(1,2*j1),iteration)

            enddo

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
CCCCCC Exchanging X_h and Y_h fields  CCCCCCC

            do j1=1,1

             call exchange(higgs(1,2*j1),higgs(1,2*j1+1),
     >        momhiggs(1,1,2*j1),momhiggs(1,1,2*j1+1),numbhiggs(1,2*j1),
     >        numbhiggs(1,2*j1+1),consthiggs(1,2*j1),iteration)

            enddo
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
            endif
           enddo   !loop to generate the iterations.
C
           if (write.eq.0) then
            open (16,file='cnfg.dat',status='unknown')
            write(16,*)'numbhiggs,'
            write(16,1)numbhiggs
            write(16,*)'numbgaubosons,'
            write(16,1)numbgaubosons
            write(16,*)'numbfermion,'
            write(16,1)numbfermion
            write(16,*)'numbfermionbar,'
            write(16,1)numbfermionbar
            close (16)
 1          format(5(2x,i3))
            write=1
           endif
C
           return
           end
C***********************************************************************
          subroutine initial(field1,mom1,const1,numb1,npart,
     >                       countsour,spinsour)
C***********************************************************************
C
C This subroutine perform the perturbative iteration.
C 
          implicit none
C
          integer dimfiel    !dimension of the array FIELD1
          integer dimmom     !dimension of the array MOM1
          integer dimnum     !dimension of the array NUMB1
          integer dimcons    !dimension of the array CONST1
          integer nlormax
          integer nmax       !maximum number of external particles
          parameter (nlormax=6) !maximum number of lorentz degrees of freedom
C                                of the field of the theory. change here
          parameter (nmax=10)   !N=8=maximum number of external particles. 
          parameter (dimmom=(2**nmax-2)/2 +100)     
          parameter (dimfiel=nlormax*dimmom)  
          parameter (dimcons=5)      
          parameter (dimnum=nmax/2)      
          integer countsour  !to count the number of already initilaized
C                             sources
          integer color(2*nmax) !particles coulors
          integer j1,j2,j3      !loop index
          integer label         !label for setting the array index
          integer menu     !to find out the lorentz character of the particle
          integer mom1(3,dimmom) !array containing a label for each 
C                                 indipendent momentum configuration.
          integer nlor1       !to store the "lorentz" index  of the particle
          integer npart       !the number of initialized external particles
C                              of the current type
          integer numb1(dimnum)  !NUMB1(j) contains the number of particle
C
          double precision const1(dimcons) !containing the characteristic 
C                                           of the higgs field:
C                              1) mass
C                              2) Lorentz nature 
C                              3) coulor
C                              4) particle "name" see ??? for the conventions

          double precision  internal !array of flags to decide wether the 
C                                     particle is internal or external
          double precision mass      !containing the mass of the initializing
C                                     external particle 
          double precision mom(4)    !array containing the momenta of the 
C                                     initializing external particle 
          double precision momenta   !array containing the momenta of the 
C                                     external particles 
          double precision spinsour  !containing the spin of the source
C                                     configurations with j momenta
C
          complex*16 field1(dimfiel) !array containing the field 
C                                     configurations
          complex*16 fieldaux(4)     !array containing the individual field 
C                                     configuration
          complex*16 vtp(4),utpb(4)
          character*1 idecay
          common/tdecflag/idecay
C
          integer ngaubos
          parameter (ngaubos=14)
          double precision masshiggs(3)             !higgs mass
          double precision massgaubosons(ngaubos)   !zboson mass
          double precision massfermion(3,4)         !fermion masses
          double precision widthhiggs(3)            !higgs width
          double precision widthgaubosons(ngaubos)  !zboson width
          double precision widthfermion(3,4)        !fermion widthes
C
          common/momenta/momenta(nmax,4),internal(nmax)
          common/colore/color
          common/masses/masshiggs,massgaubosons,massfermion ,
     >                  widthhiggs,widthgaubosons,widthfermion !in this
C                  common all the  masses of the particles are transferred.
C                       Wishing to add a new particle change here
C initializing mom1
C
c          double precision mb,mw
C
C          double precision idec(4,3,nmax)
c          common/decay/idec     ! idec(1:4,i,j) four momentum of the I-th 
C              decay product of the J-th particle where J is given according
C              to alpha ordering; the four momentum is (E,p_x,p_y,p_z)
C              for the top(topbr) I=1 is b (bbar) I=2 is fermion 
C              I=3 is fermionbar (W-> f fb) 
           double precision pf(4),pfb(4),pb(4)
c           common/pdec/pf,pfb,pb
          common/tspinors/vtp,utpb
          complex*16 eps(4,nmax)
          common/vdec/eps
C
          integer jwz
C
C
c           call initcolore(color)      !returning the particles coulor
c
           if(npart.eq.1) then
             jwz=0
c             do j1=1,4
c               do j2=1,3
c                 do j3=1,nmax
c                   idec(j1,j2,j3)=0.d0
c                 enddo
c               enddo
c             enddo
           endif
c
           mom1(1,npart-countsour+1)=npart
           mom1(2,npart-countsour+1)=color(2*npart-1)
           mom1(3,npart-countsour+1)=color(2*npart)
C
C initializing numb1
C
           numb1(1)=numb1(1)+1
C
C initializing field1
C
           menu=nint(const1(2))
           do j1=1,4
            mom(j1)=momenta(npart,j1)*internal(npart)
           enddo
           goto(1,2,3,4,5,6),menu
C
           write(6,*)'I have failed to find the proper label for the'
           write(6,*)'source labelled with lorentz label',nlor1
           stop
C
 1         write(6,*) 'I have been asked to initialize an auxiliary'
           write(6,*) 'field. I cannot do it since no source exists.'  
C           this should not happen
           stop
C
 2         fieldaux(1)=-1.
           goto 999
C
 3         mass=const1(1)
           if(nint(const1(4)).eq.109.and.idecay.eq.'y') then
c             mb=massfermion(3,2)
c             mw=massgaubosons(2)
c             call vtop(mom,mass,mb,mw,fieldaux)
c             do j1=1,4
c               idec(j1,1,npart)=pb(j1)
c               idec(j1,2,npart)=pf(j1)
c               idec(j1,3,npart)=pfb(j1)
c             enddo
             do j1=1,4
                fieldaux(j1)= vtp(j1)
             enddo
           else
             call  fermionsources(spinsour,fieldaux,mom,mass)
           endif
           goto 999
C
 4         mass=const1(1)
           if(nint(const1(4)).eq.209.and.idecay.eq.'y') then
c             mb=massfermion(3,2)
c             mw=massgaubosons(2)
c             call utopb(mom,mass,mb,mw,fieldaux)
c             do j1=1,4
c               idec(j1,1,npart)=pb(j1)
c               idec(j1,2,npart)=pf(j1)
c               idec(j1,3,npart)=pfb(j1)
c             enddo
             do j1=1,4
                fieldaux(j1)= utpb(j1)
             enddo
           else
             call  fermionbarsources(spinsour,fieldaux,mom,mass)
           endif
           goto 999
C
 5         mass=const1(1)
           if(idecay.eq.'y') then
             jwz=jwz+1
             do j1=1,4
               fieldaux(j1)=eps(j1,jwz)
             enddo              
           else
             call  sourcemassboson(spinsour,fieldaux,mom,mass)
           endif
           goto 999
C
 6         mass=const1(1)
           call sourcemassboson(spinsour,fieldaux,mom,mass)
ccc fulvio
c             do j1=1,4
c               fieldaux(j1)=mom(j1)
c             enddo              
ccc fulvio
c           call michelangelo(spinsour,fieldaux,momenta,color)
           goto 999
C
C### add  new particles here
C
 999       call lorentz(const1(2),nlor1)
           label=nlor1*(numb1(1)-1)
           do j1=1,nlor1
            field1(j1+label)=fieldaux(j1)
           enddo
C
           return
C
           end
C***********************************************************************
          subroutine interaction(field1,field2,field3,mom1,mom2,mom3,
     >     numb1,numb2,numb3,                   
     >     const1,const2,const3,operator,iteration,couplingconst)
C***********************************************************************
C
C This subroutine given three field configuration FIELD1, FIELD2, FIELD3,
C whose corresponding parents momenta are recorded in MOM1, MOM2, MOM3,
C return the new field configurations produced at the relevant ITERATION
C step.
C
          implicit none
C
          integer dimfiel      !dimension of the array field1,2,3
          integer dimnum       !dimension of the array numb1,2,3
          integer dimmom       !dimension of the array mom1,2,3
          integer dimcons      !dimension of the array cons1,2,3
          integer dimop        !dimension of the array operator
          integer nlormax    !maximum number of lorentz degrees of freedom, 
          integer ncolmax    !maximum number of coulor degrees of freedom,
          integer nmax
C                             of the fields of the theory. 
          parameter (ncolmax=8)
          parameter (nlormax=6)
          parameter (nmax=10)   !maximum number of external particles. 
          parameter (dimcons=5)      !number of needed particle caracteristics.
          parameter (dimmom=(2**nmax-2)/2 +100) !number of allowed momentum 
          parameter (dimfiel=ncolmax*nlormax*dimmom)  !this will set the dimensions for the 
C                     array for the different fields. 
          parameter (dimnum=nmax/2)      !storing the number of configuration
          parameter (dimop=2)
C
          integer e12, e13, e23, e123           !flags to check fields equality
          integer extr1         !loop extremum
          integer iteration   !storing the present iteration stage
          integer mom1(3,dimmom), mom2(3,dimmom), mom3(3,dimmom)  !arrays containing 
C                    a label for each indipendent momentum configuration. This 
C                    number will allow to determine the complete configuration
          integer multiplicity !flag to obtain the right combinatorial factors
          integer niteration !number of iteration steps
          integer nlor1      !storing the number of lorentz degrees of freedom
          integer numb1(dimnum), numb2(dimnum), numb3(dimnum) !storing 
C                        the number of configuration according to the number 
C                       of momenta contributing to the configuration under exam
          integer operator(dimop)    !a flag to identify the type of operator
          integer orderfield(3) !containing the ordering of FIELD1, FIELD2 
C           and FIELD3 to pick up the correct colour and lorentz matrix element
C
          double precision const1(dimcons), const2(dimcons), 
     >                     const3(dimcons) !containing 
C                                              the characteristic of the field:
          double precision weight      !to help in computing the square of equal fields
C
          complex*16 couplingconst
          complex*16 field1(dimfiel), field2(dimfiel), field3(dimfiel)
C arrays containing the field configuration of of the three field to multiply
C
          common/combinr/weight
          common/combini/multiplicity
          common/iter/niteration
C
           e12=1
           e13=1
           e23=1
           e123=1
           if (nint(const1(4)).ne.nint(const2(4))) e12=0
           if (nint(const1(4)).ne.nint(const3(4))) e13=0
           if (nint(const2(4)).ne.nint(const3(4))) e23=0
           e123=e12*e13            
C
C Computing the contribution to field1 equation of motion
C
           multiplicity=1+e23
           weight=1.
           if(e13.eq.1.or.e12.eq.1)weight=2.
           if(e123.eq.1)weight=3.
C
           orderfield(1)=1     !setting the coulor order of the fields
           orderfield(2)=2
           orderfield(3)=3
C
           call prodinteraction(field1,field2,field3, mom1,mom2,mom3,
     >     numb1,numb2,numb3,const1,const2,const3,operator, 
     >     iteration,couplingconst,orderfield)
C
C Computing the contribution to field2 equation of motion
C
           if (e12.eq.0) then        !only in this case we need to compute
C                                     the iteration
            multiplicity=1+e13
            weight=1.
            if(e23.eq.1)weight=2.
C
            orderfield(1)=2     !setting the coulor order of the fields
            orderfield(2)=1
            orderfield(3)=3
C
            call prodinteraction(field2,field1,field3,mom2,mom1,
     >      mom3,numb2,numb1,numb3,const2,const1,const3,operator, 
     >      iteration,couplingconst,orderfield)
           else
            if (numb1(iteration).ne.0) then
             call lorentz(const1(2),nlor1)    ! the number of lorentz 
C                                               degrees of freedom NLOR1
             call sumintegarr(numb1,iteration-1,extr1)
C
             call equalintarrays(numb1,numb2,dimnum)
             call equalintarrays(mom1(1,extr1+1),mom2(1,extr1+1),
     >                           3*numb1(iteration))
             extr1=extr1*nlor1
             call equalcomparrays(field1(extr1+1),field2(extr1+1),
     >                           numb1(iteration)*nlor1)
            endif
           endif
C
C Computing the contribution to field3 equation of motion
C
           if (e13.eq.0) then        !only in this case we need to compute
C                                    the iteration
            if (e23.eq.0) then        !only in this case we need to compute
C                                    the iteration
             multiplicity=1+e12
             weight=1.
C
             orderfield(1)=3     !setting the coulor order of the fields
             orderfield(2)=1
             orderfield(3)=2
C
             call prodinteraction(field3,field1,field2,mom3,mom1,
     >       mom2,numb3,numb1,numb2,const3,const1,const2,operator,      
     >       iteration,couplingconst,orderfield)
            else
             call lorentz(const2(2),nlor1)    
             call sumintegarr(numb2,iteration-1,extr1)
C
             call equalintarrays(numb2,numb3,dimnum)
             call equalintarrays(mom2(1,extr1+1),mom3(1,extr1+1),
     >                           3*numb2(iteration))
             extr1=extr1*nlor1
             call equalcomparrays(field2(extr1+1),field3(extr1+1),
     >                           numb2(iteration)*nlor1)
C
            endif
C
           else
C
            call lorentz(const1(2),nlor1)    
            call sumintegarr(numb1,iteration-1,extr1)
C
            call equalintarrays(numb1,numb2,dimnum)
            call equalintarrays(mom1(1,extr1+1),mom2(1,extr1+1),
     >                           3*numb1(iteration))
            extr1=extr1*nlor1
            call equalcomparrays(field1(extr1+1),field2(extr1+1),
     >                           numb1(iteration)*nlor1)
           endif
C
           return
           end
C***********************************************************************
          subroutine prodinteraction(field1,field2,field3,mom1,mom2,
     >     mom3,numb1,numb2,numb3,const1,const2,const3,operator,       
     >     iteration,couplingconst,orderfield)
C***********************************************************************
C
C This subroutine given three field configurations FIELD1, FIELD2, FIELD3,
C whose corresponding parents momenta are recorded in MOM1, MOM2, MOM3,
C return the new field configurations produced at the relevant ITERATION
C step.
C
          implicit none
C
          integer nlormax    !maximum number of lorentz degrees of freedom
C                             of the fields of the theory. 
          integer nmax
          integer dimamp       !dimension of the array newamplitude
          integer dimfiel      !dimension of the array field1,2,3
          integer dimcons      !dimension of the array cons1,2,3
          integer dimmom       !dimension of the array mom1,2,3
          integer dimnum       !dimension of the array numb1,2,3
          integer dimop        !dimension of the array operator
          parameter (nmax=10) !maximum number of external particles. 
          parameter (nlormax=6)
          parameter (dimamp=nlormax) 
          parameter (dimcons=5)     !number of needed particle caracteristics.
          parameter (dimmom=(2**nmax-2)/2 +100)     
          parameter (dimfiel=nlormax*dimmom) 
          parameter (dimnum=nmax/2)      
          parameter (dimop=2)     
          integer addition(2)!flag to check wether a new configuration or a 
C                            new contribution to an old one has been computed
          integer extrem1,extrem2!loop lower and upper extrema
          integer ex1,ex2,ex3!to store the number of excitation of field1,2,3
          integer iteration      !storing the present iteration stage
          integer j1, j2, j3, j4 !loop index
          integer labelstart2    !to set label index properly
          integer mom1(3,dimmom),mom2(3,dimmom),mom3(3,dimmom) !array 
C                          containing a number for each indipendent
C                          momentum configuration. This numbers will allow
C                          to determine the complete configuration   
          integer momaux(6)  !containing the label of the momenta currently
C                             used
          integer momcol(4)  !containig the color indexes of the two field 
C                             to be multiplied
          integer momnew(4)  !containig the color indexes of the resulting 
C                             field
          double precision coeff(2) !it contain the coefficients of the 
C                                    different coulor combination 
C                                    generated (<=2)
          integer multiplicity!flag to obtain the right combinatorial factors
          integer nadd        !array label
          integer newc        !to store a new label number
          integer nlor1,nlor2,nlor3  !storing the number of lorentz degrees 
C                                     of freedom of field1
          integer niteration !number of iteration steps
          integer nstart2,nstart3!storing the label of the relevant amplitude
C                                 for the array field1
          integer numb1(dimnum),numb2(dimnum),numb3(dimnum) 
                    !storing the number of configuration according
C                    to the number of momenta contributing
          integer operator(dimop) !a flag to identify the type of operator, 
C                          up to now only one different operators are know:
C                          1)  A h h   (h=higgs, A auxiliar higgs)
          integer orderfield(3) !containing the ordering of FIELD1,
C                                FIELD2 and FIELD3 to pick up the correct
C                                colour and lorentz matrix element
          integer pos           !flag to determine the loop extrema
          integer positio       !flag to determine the loop extrema
          integer st(3)
          integer dmax
          parameter (dmax=(2**nmax-2)/2)
          integer prodconfprod(7,dmax,dmax) !array computed in INITCOMPCONF
          integer n1,n2,start(nmax/2) !to compute the labels for PRODCONFPROD
          common/prodprod/prodconfprod
          common/strt/start
C
          double precision const1(dimcons), const2(dimcons), 
     >                     const3(dimcons) 
          double precision perm        !sign of the fermion permutation
          double precision sign        !correct sign for the momenta
C
          double precision weig        !containing the combinatorial factor
          double precision weight      !to help in computing the square of 
C                                       equal fields
          double precision ww          !to store the result of intermediate 
C                                       products
C
          complex*16 alfaprime(nlormax**3)  !storing the lorentz interaction 
C                                            matrix for subroutine LOROPER
          complex*16 couplingconst  !coupling constant
          complex*16 field1(dimfiel),field2(dimfiel), field3(dimfiel) !arrays
C                                 containing the field configuration of one  
C                                 of the three field to multiply
          complex*16 newamplitude(dimamp)  !storing the result of the 
C                                           multiplication
          complex*16 newamplitudeprime(dimamp)  !storing the result of the 
C                                                multiplication
          complex*16 propaux(nlormax**2)     !propagator returned by 
C                                             the subroutine PROPAGATOR
C
          common/combinr/weight
          common/combini/multiplicity
          common/iter/niteration
          common/sign/sign(3)            !correct sign for the four momenta
C
          integer flgdual                !dual (0) or su3 (1) amplitudes
          common/dual/flgdual
C
         integer ndirac                  !for fermi/dirac statistic
         common/dirac/ndirac
C
C data statments
C
          data st/0,2,4/
C
C Static variables
C
C Computing the number of configuration of the field 2,3 contributing
C at the present perturbative stage.
C
            call sumintegarr(numb1,iteration-1,ex1)
            call sumintegarr(numb2,iteration-1,ex2)
            call sumintegarr(numb3,iteration-1,ex3)
C
C  We now iterate field1
C
           if (ex2.ne.0.and.ex3.ne.0) then !do it only if neither field2 nor 
C                                          field3 are empty
C           
C
C Pickig up the particle characteristics. Outside the loop is less expensive
C
            call lorentz(const1(2),nlor1)    !returning the number of lorentz
C                                             degrees of freedom of FIELD1
            call lorentz(const2(2),nlor2)
            call lorentz(const3(2),nlor3)
C
                sign(orderfield(1))=-1.      !the sign of the momenta of this
C                                            configuration must be reversed
                sign(orderfield(2))=1.
                sign(orderfield(3))=1.
C
            pos=1                           
            positio=numb2(pos)
            do j1=1,ex2                !loop over the field2 configuration
C
C Calculating the configurations of field3 contributing to the sum
C
C The pourpose of the following lines is to store in POS the number
C of momenta contributing to the field2 configuration. This in turn
C allow to select only the configuration in field3 with a number of
C momenta ITERATION-POS. The variable positio stores the total
C number of configuration with less than POS momenta allowing for the check
C
             do while (j1.gt.positio) 
              pos=pos+1
              positio=positio+numb2(pos)
             enddo
C
             extrem1=0
             if(iteration.gt.pos-1)
     >          call sumintegarr(numb3,iteration-pos-1,extrem1)
             extrem1=1+extrem1
             extrem2=numb3(iteration-pos)-1 +extrem1
C
             weig=1
             if (multiplicity.ne.1) then    !two equal fields
              weig=2.
              extrem1=max(extrem1,j1+1)     !avoiding doublecounting
             endif
C
             if (extrem2.ge.extrem1) then
C
              do j2=extrem1,extrem2
C
              n1=start(pos)+mom2(1,j1)
              n2=start(iteration-pos)+mom3(1,j2)
              newc=prodconfprod(1,n1,n2)
              perm=prodconfprod(ndirac,n1,n2) !to compose the two momenta 
C                             configuration. If NEWC is 0
C                             it means that the no configuration is possible
C                             as a product of the two. Otherwise NEWC carries
C                             the label of the new configuration. PERM will
C                             return the sign of the fermion permutation
C                             PRODCONFPROD is computed in INITCOMPCONF
C
              if (newc.ne.0) then
               momcol(1)=mom2(2,j1)
               momcol(2)=mom2(3,j1)
               momcol(3)=mom3(2,j2)
               momcol(4)=mom3(3,j2)
               if (flgdual.eq.0) then
                call coloreprod(momcol,momnew,newc,perm,
     >                         nint(const1(3)),orderfield(1),coeff)
               elseif (flgdual.eq.1) then
                call coloreprodsu3(momcol,momnew,newc,perm,
     >                         nint(const1(3)),orderfield(1),coeff)
               else
                write(6,*)'wrong assignment to FLGDUAL'
                stop
               endif
              endif
C
               if (newc.ne.0) then  !an acceptable configuration has been  
C                                     created
C
C Extracting the amplitude of the present configuration
C
                momaux(st(orderfield(1))+1)=iteration
                momaux(st(orderfield(1))+2)=newc
                momaux(st(orderfield(2))+1)=pos
                momaux(st(orderfield(2))+2)=mom2(1,j1)
                momaux(st(orderfield(3))+1)=iteration-pos
                momaux(st(orderfield(3))+2)=mom3(1,j2)
                call loroper(operator(1),alfaprime,momaux)  !lorentz 
C                                                          interaction matrix
                call reorder(orderfield(1),alfaprime,nlor1,nlor2,nlor3)
C
                    nstart2=(j1-1)*nlor2
                    nstart3=nlor3*(j2-1)
                call matprod_prodint(newamplitude,field2(nstart2+1),
     >                 field3(nstart3+1),nlor1,nlor2,nlor3,alfaprime)
C
C  Multiplying for the inverse propagator                            
C
                call propagator (const1(2),const1(1),const1(5),
     >                propaux,newc,iteration)!PROP will return the propagator
C
                ww=weight*weig*perm   !outside the loop is cheaper
 1              format(5(2x,e20.13))
 2              format(2x,e20.13)
                call propprod(newamplitudeprime,newamplitude
     >                       ,propaux,nlor1)
c
                do j3=1,2
                 if (abs(coeff(j3)).lt.1.d-10) then
                   addition(j3)=-1
                 else
                  addition(j3)=0         !checking if a new configuration has
C                           been obtained or a new contribution to an old one
                 endif
                enddo
                if(numb1(iteration).ne.0) then
                 do j3=ex1+1,numb1(iteration)+ex1
                  if (newc.eq.mom1(1,j3))then
                   do j4=1,2
                    if(addition(j4).ge.0) then
                      if(momnew(1+2*(j4-1)).eq.mom1(2,j3).and.
     >                momnew(2+2*(j4-1)).eq.mom1(3,j3))addition(j4)= j3
                    endif
                   enddo 
                  endif
                 enddo
                endif
                do j4=1,2
                 if (addition(j4).eq.0) then      ! a new configuration
                  numb1(iteration)=numb1(iteration)+1
                  labelstart2=(ex1+numb1(iteration)-1)*nlor1
                  do j3=1,nlor1
                   field1(labelstart2+j3)=newamplitudeprime(j3)
     >                                  *couplingconst*ww*coeff(j4)
                  enddo
                  mom1(1,ex1+numb1(iteration))=newc
                  mom1(2,ex1+numb1(iteration))=momnew(1+2*(j4-1))
                  mom1(3,ex1+numb1(iteration))=momnew(2+2*(j4-1))
                 elseif(addition(j4).gt.0) then   ! a new contribution to an
C                                                   old configuration
                  nadd=(addition(j4)-1)*nlor1
                  do j3=1,nlor1
                   field1(nadd+j3)=newamplitudeprime(j3)*ww
     >                      *couplingconst*coeff(j4) +field1(nadd+j3)
                  enddo
                 endif
                enddo
C
               endif
C
              enddo
C
             endif
C
            enddo          
C
           endif
C
           return
           end
C*****************************************************************
            subroutine propprod(newpr,new,propagator,nlor1)
C*****************************************************************
C
C
C
            implicit none
C
            integer j4,j5,label,nlormax,nlor1
            parameter (nlormax=6)       !maximum number of lorentz d.o.f 
            complex*16 new(nlormax),newpr(nlormax),
     >                 propagator(nlormax**2)
C
                label=0
                 do j4=1,nlor1
                  newpr(j4)=0.
                  do j5=1,nlor1
                   label=label+1
                   newpr(j4)=propagator(label)*new(j5)+newpr(j4)
                  enddo
                 enddo
C
                return
                end
C*****************************************************************
              subroutine matprod_prodint(new,field2,field3,
     >                            nlor1,nlor2,nlor3,alfaprime)
C*****************************************************************
C
C 
C
              implicit none 
C
              integer j4,j6,j8,label,nlor1,nlor2,nlor3,nlormax
              parameter (nlormax=6)   !maximum number of lorentz d.o.f. 
              complex*16 field2(nlormax),field3(nlormax),
     >                   new(nlormax),aux
     >                   ,zero,alfaprime(nlormax**3)
              data zero /(0.,0.)/ 
C
                  label=0
                    do j4=1,nlor1  
                        !looping over the lorentz index of particle 1
                     new(j4)=0.                     
                     do j6=1,nlor2
                      if(field2(j6).ne.zero) then
                       aux=zero
                       do j8=1,nlor3
C
C Performing the product
C
                        label=label+1
                        aux=alfaprime(label)*
     >                               field3(j8)+aux
                       enddo  
                       new(j4)=new(j4) + aux*field2(j6)
                      else
                       label=label+nlor3
                      endif
                     enddo
                    enddo
C
                end
C***********************************************************************
          subroutine lagint(field1,field2,field3,mom1,mom2,mom3,
     >     numb1,numb2,numb3,const1,const2,const3,operator,
     >     couplingconst,elmataux)
C***********************************************************************
C
C This subroutine given three field configuration FIELD1, FIELD2, FIELD3,
C whose corresponding parents momenta are recorded in MOM1, MOM2, MOM3,
C return the new contribution to the amplitude in ELAMATAUX
C
          implicit none
C
          integer dimfiel      !dimension of the array field1,2,3
          integer dimcons      !dimension of the array cons1,2,3
          integer dimmom       !dimension of the array mom1,2,3
          integer dimnum       !dimension of the array numb1,2,3
          integer dimop        !dimension of the array operator
          integer nlormax    !maximum number of lorentz degrees of freedom
C                             of the fields of the theory. 
          integer nmax
          parameter (nlormax=6)
          parameter (nmax=10)       !maximum number of external particles, 
          parameter (dimcons=5)      
          parameter (dimmom=(2**nmax-2)/2 +100)     
          parameter (dimfiel=nlormax*dimmom)  
          parameter (dimnum=nmax/2)      
          parameter (dimop=2)     
C
          integer e12         !flag to chek field1,2 equality
          integer e23         !flag to chek field2,3 equality
          integer e123        !flag to chek field1,2,3 equality
          integer extrem1     !loop lower extremum
          integer extrem2     !loop upper extremum
          integer ex1         !to store the number of excitation of field1
          integer ex2         
          integer ex3
          integer find1       !flag to find the field3 configuration
          integer j1,j2,j3          !loop index
          integer label3     !to store the label of field3
          integer mom1(3,dimmom),mom2(3,dimmom),mom3(3,dimmom)       
                             !array containing a number for each indipendent
C                             momentum configuration. This numbers will allow
C                             to determine the complete configuration   
          integer momaux(6)!storing the labels of the three momenta currently
C                           under use
          integer momcol(6)  !to compute correct color factor
          integer newc       !to store a new label number
          integer next       !number of external particle
          integer nfermion
          integer nlor1,nlor2,nlor3      !storing the number of lorentz 
C                                         degrees of freedom of field1
          integer nstart1,nstart2,nstart3    !storing the label of the 
C                                     relevant amplitude for the array field1
          integer numb1(dimnum),numb2(dimnum),numb3(dimnum)      
                               !storing the number of configuration according
          integer operator(dimop)    !a flag to identify the type of operator, up to
          integer pos1,pos2,pos3        !flag to determine the loop extrema
          integer positio1,positio2        !flag to determine the loop extrema
          integer dmax
          parameter (dmax=(2**nmax-2)/2)
          integer prodconflag(7,dmax,dmax)
          integer n1,n2,start(nmax/2)         !to compute PRODCONFLAG labels
C
          double precision const1(dimcons),const2(dimcons),
     >                     const3(dimcons)
          double precision perm         !the sign of the fermion permutation
C                              now only one different operators are know:
          double precision sign         !to be poassed to the subroutine LOROPER
C                              1)  A h h   (h=higgs, A auxiliar higgs)
          double precision weight      !to help in computing the square of equal fields
C
          complex*16 alfaprime (nlormax**3)   !storing the lorentz interaction matrix for
C                              subroutine LOROPER
          complex*16 aux3 !auxiliar variables. 
C
          complex*16 couplingconst !coupling constant
          complex*16  elmataux     !containing the contribution to the amplitude
          complex*16 field1(dimfiel),field2(dimfiel),field3(dimfiel)   !array containing the field configuration of one  
C                              of the three fields to multiply
          common/prodlag/prodconflag
          common/strt/start
          common/external/next,nfermion  !containing the number of external
C                                       particles from the subroutine ?????????
          common/sign/sign(3)            !correct sign for the four momenta
C
          integer ncmomfx(8),j4,find2
          double precision perm0
C
          integer flgdual                !dual (0) or su3 (1) amplitudes
          common/dual/flgdual
C
          integer ndirac                      !for fermi/dirac statistic
          common/dirac/ndirac
C
          integer n3lp
C
           elmataux=0
C
C Setting flags to manage products of equal fields
C
           e12=1
           if (const1(4).ne.const2(4)) e12=0
           e23=1
           if (const3(4).ne.const2(4)) e23=0
           e123=e12*e23
           weight=1
           if(e12.eq.1.or.e23.eq.1) weight = 2.
           if(e123.eq.1) weight = 6.
C
C Computing the number of configuration of the field 2,3 contributing
C
           call sumintegarr(numb1,dimnum,ex1)
           call sumintegarr(numb2,dimnum,ex2)
           call sumintegarr(numb3,dimnum,ex3)
C
C  We now iterate field1
C
           if (ex1.ne.0.and.ex2.ne.0.and.ex3.ne.0) then !do it only if neither field2 nor 
C                                          field3 are empty           
C
C Pickig up the particle characteristics. Outside the loop is less expensive
C
            call lorentz(const1(2),nlor1)    !returning the number of lorentz
C                                         degrees of freedom of FIELD1
            call lorentz(const2(2),nlor2)
            call lorentz(const3(2),nlor3)
C
            pos1=1                           
            positio1=numb1(pos1)
c
            do j1=1,ex1                !loop over the field1 configuration
C
C Calculating the configurations of field3 contributing to the sum
C
C The pourpose of the following lines is to store in POS1 the number
C of momenta contributing to the field2 configuration. 
C
             do while (j1.gt.positio1) 
              pos1=pos1+1
              positio1=positio1+numb1(pos1)
             enddo
C
             pos2=1                           
             positio2=numb2(pos2)
             extrem1=1
             if (e12.eq.1) then    !field1 and 2 equal
              extrem1=j1+1         !avoiding to repeat the same computation
              if(extrem1.le.ex2) then
               do while(extrem1.gt.positio2)
                pos2=pos2+1
                positio2=numb2(pos2)+positio2
               enddo
              endif
             endif
             do while (next-pos1-pos2.gt.dimnum) 
              pos2=pos2+1
              positio2=positio2+numb2(pos2)
             enddo
             j2=max(extrem1,positio2-numb2(pos2)+1)
             do while(pos1+pos2.lt.next.and.j2.le.ex2) !loop over field1
              do while (j2-positio2.gt.0) 
               pos2=pos2+1
               positio2=positio2+numb2(pos2)
              enddo
              pos3=next-pos1-pos2  !number of momenta of the third fields
              if(pos3.gt.0) then
               if(numb3(pos3).gt.0) then !only in this case a non zero value is
C                                       possible
                n1=start(pos1)+mom1(1,j1)
                n2=start(pos2)+mom2(1,j2)
                newc=prodconflag(1,n1,n2)
                perm=prodconflag(ndirac,n1,n2)    !to  compose the two momenta configuration. If NEWC is 0
C                               it means that the no configuration is possible
C                               as a product of the two. Otherwise NEWC carries
C                               the label of the new configuration. PERM is
C                               the sign of the fermion permutation. 
C
                if (e23.eq.1) then
                 if (pos3.lt.pos2) newc=0
!                 if (pos3.eq.pos2.and.newc.lt.mom2(1,j2)) newc=0
                endif
                if (newc.ne.0) then  !an acceptable configuration has been  
C                                     created
C
C Now NEWC contain the label of the configuration conf1
C
                 extrem2=0
                 if (pos3.gt.1) 
     >               call sumintegarr(numb3,pos3-1,extrem2)
                 extrem2=0+extrem2
                 n3lp=extrem2+numb3(pos3)
C
                 if (e23.eq.1.and.pos3.eq.pos2) extrem2=j2
                 j3=extrem2                 !finding the amplitude of the
ccccccccccc
                 find1=0                      !third configuration
c                 do while(find1.eq.0.and.j3.lt.extrem2+numb3(pos3)) 
c                  j3=j3+1
c                  if(mom3(1,j3).eq.newc) find1=1
c                 enddo
c                 do while(find1.eq.0.and.j3.lt.extrem2+numb3(pos3)) 
                 do j3=extrem2+1,n3lp
c                  j3=j3+1
                  if(mom3(1,j3).eq.newc) then
                   find1=1+find1
                   ncmomfx(find1)=j3
                  endif
                 enddo
                 if (find1.ne.0) then
                  do j4=1,find1
                   perm0=perm
                   j3=ncmomfx(j4)  
                   momcol(1)=mom1(2,j1)
                   momcol(2)=mom1(3,j1)
                   momcol(3)=mom2(2,j2)
                   momcol(4)=mom2(3,j2)
                   momcol(5)=mom3(2,j3)
                   momcol(6)=mom3(3,j3)
                   if (flgdual.eq.0) then
                    call colorelag(momcol,newc,perm0,operator(2))
                    find2=0
                    if (newc.ne.0) find2=1
                   elseif(flgdual.eq.1) then
                    call colorelagsu3(momcol,find2,perm0)
                   else
                    write(6,*)'wrong FLGDUAL label'
                    stop
                   endif
                   if (find2.eq.1) then
                    label3=ncmomfx(j4)
C
C Extracting the amplitude of the present configuration
C
                    momaux(1)=pos1     !containing the label of the present
                    momaux(2)=mom1(1,j1) !momenta configuration for the subroutine
                    momaux(3)=pos2     !LOROPER
                    momaux(4)=mom2(1,j2)
                    momaux(5)=pos3
                    momaux(6)=newc
C
                    sign(1)=1.      !all the momenta configuration we pass to 
                    sign(2)=1.      !the subroutine LOROPER shoul keep the
                    sign(3)=1.      !same sign for the momenta
                    call loroper(operator(1),alfaprime,momaux)!lorentz 
C                                                           interaction matrix
C
                    nstart1=(j1-1)*nlor1
                    nstart2=nlor2*(j2-1)
                    nstart3=(label3-1)*nlor3
C 
                    call matprod_lagint(field1(nstart1+1),
     >                      field2(nstart2+1),field3(nstart3+1)
     >                         ,nlor1,nlor2,nlor3,alfaprime,aux3)
                    elmataux=elmataux+aux3*weight*perm0
C
                   endif
                  enddo
                 endif
cccccccccccc
C
                endif
C
               endif
C
              endif
C
              j2=j2+1
             enddo
C
            enddo          
C
           endif
C
           elmataux=elmataux*couplingconst
C
           return
           end

C*************************************************************************
              subroutine matprod_lagint(field1,field2,field3,nlor1,
     >                                nlor2,nlor3,alfaprime,aux3)
C*************************************************************************
C
C
C
              implicit none
C
              integer label,indaux,j4,j6,j8,nlor1,nlor2,nlor3,nlormax
              parameter (nlormax=6)   !maximum number of lorentz d.o.f. 
              complex*16  field2(nlormax),field1(nlormax),aux3
     >                    ,field3(nlormax),aux2,aux1,zero
     >                    ,alfaprime(nlormax**3)
              data zero/(0.,0.)/ 
C
                      aux3=0.
                      label=0
                      indaux=nlor2*nlor3
c
                      do j4=1,nlor1      !looping over the lorentz index of particle 1
                       if (field1(j4).ne.zero) then
                        aux2=zero
                        do j6=1,nlor2
                         if (field2(j6).ne.zero) then
                          aux1=zero
                          do j8=1,nlor3
                           label=label+1
                           aux1=aux1+alfaprime(label)*field3(j8)
                          enddo  
                          aux2=aux2+aux1*field2(j6)
                         else
                          label=label+nlor3
                         endif
                        enddo
                        aux3=aux3+aux2*field1(j4)
                       else
                        label=label+indaux
                       endif
                      enddo
C
                  return
                  end
C***********************************************************************
          subroutine propagator (c1,mm1,gam1,prop,newc,iteration)   
C***********************************************************************
C
C Returns the inverse propagator of lorentz index LOR1 LOR2 in the
C variable PROP for the particle C1 with momentum defined by NEWC
C
          implicit none
C
          integer iteration     !iteration number
          integer j1          !loop index
          integer newc       !to store a new label number
          integer nlormax    !maximum number of lorentz degrees of freedom
C                             of the fields of the theory. 
          parameter (nlormax=6)
C
          double precision c1       !containing the characteristic of the field:
C                              2) Lorentz nature 
          double precision mm1           !particle mass
          double precision gam1         !particle width
          double precision pq         !four momentum squared
C
          complex*16 re
          parameter (re=(1.,0.))
          complex*16 im
          parameter (im=(0,1))
          complex*16 m1           !particle  complex mass  (M + i \Gamma)
          complex*16  prop(nlormax**2)        !propagator
          complex*16  zz
C
          character*2 wmode
          character resonance
          double precision winsize
          common/gauinv/winsize,resonance,wmode
C
           m1=mm1
           goto(1,2,3,4,5,6,7),nint(c1)
           write(6,*)'I have been asked to compute the propagator of'
           write(6,*)'the particle with Lorentz Index',c1,'which is not'
           write(6,*)'in my table of propagators'  !this should not happen
           stop
C
 1         prop(1)=1.*re     !auxiliary field  propagator
           return
C
 2         if(resonance.eq.'y') then
             prop(1)=0.d0             
             return
           endif
           call momsq(newc,iteration,pq) !scalar field propagator
           if(wmode.eq.'yy') then
             zz=pq-mm1*mm1+mm1*gam1*im
             resonance='n'
           elseif(wmode.eq.'yn') then
             if(abs(mm1*mm1-pq).lt.winsize*mm1*gam1) then
              resonance='y'
            else
              resonance='n'
            endif
            if(abs(mm1*mm1-pq).eq.0.d0) then
              prop(1)=1.d0
              return
            endif
             zz=pq-mm1*mm1
           elseif(wmode.eq.'nn') then
             zz=pq-mm1*mm1
             resonance='n'
           endif
C
           prop(1)=1./zz
           return        
C
 3         call pslashplusmtr(newc,iteration,prop,mm1,gam1)
           return        
C
 4         call pslashplusm(newc,iteration,prop,mm1,gam1)
           return        
C
 5         call gmnminkmkn(newc,iteration,prop,mm1,gam1)
           return        
C
 6         call momsq(newc,iteration,pq)  !massless gauge boson propagator
c           call gmnkmkn(newc,iteration,prop)
           pq=1/pq
           do j1=1,16
            prop(j1)=0.
           enddo
           prop(1)=-pq
           prop(6)=pq
           prop(11)=pq
           prop(16)=pq
           return        
C
  7        do j1=1,36
            prop(j1)=0.
           enddo       
           prop(1)=1.
           prop(8)=1.
           prop(15)=1.
           prop(22)=-1.
           prop(29)=-1.
           prop(36)=-1.
           return
C
           end
C***********************************************************************
          subroutine momsq(newc,nmom,pq)
C***********************************************************************
C
C returning the four momenta squared in PQ for a configuration with
C NMOM momenta and label NEWC
C
          implicit none
C
          integer nmax       !maximal number of external particles
          parameter (nmax=10)  !maximal number of external particles, 
          integer newc       !configuration label
          integer next       !number of external particles
          integer nfermion
          integer nmom       !number of momenta
C
          double precision  internal    !array of flags to decide wether the particle is
C                              internal or external
          double precision momenta     !array containing the external momenta
          double precision p(4)           !storing the j-th component of the momentum
          double precision pq          !squared momentum
C
          common/momenta/momenta(nmax,4),internal(nmax)
          common/external/next,nfermion  !containing the number of external
C                                       particles from the subroutine ?????????
C
          call findmom(nmom,newc,p)
C
          pq=p(1)*p(1)-p(2)*p(2)-p(3)*p(3)-p(4)*p(4)
C
          return
          end
C***********************************************************************
          subroutine loroper(oper,alfaprime,momaux)
C***********************************************************************
C
C This subroutine returns in ALFA the lorentz interaction matrix. The flag
C OPER allows to chose the correct lorentz operator. 
C
          implicit none
C
          integer nmax     !maximum number of external particles
          parameter (nmax=10)  !maximum number of external particles, 
          integer nlormax    !maximum number of lorentz degrees of freedom
C                             of the fields of the theory. 
          parameter (nlormax=6)
          integer j1       !loop index
          integer momaux(6)   !containing the labels of the momenta currently used
          double precision  momenta  !storing the external momenta
          double precision  mom1(4),mom2(4),mom3(4)     !momentum of configuration 1
          integer next     !number of external particles
          integer nfermion !number of external fermions
          integer oper     !operator flag to choose the right subroutine
C
          double precision internal !labels to discriminate among incoming and outcoming
C                           particles
          double precision sign      !correct sign for the momenta
C
          complex*16 alfaprime(nlormax**3)        !storing the result of the "lorentz product"
C
          common/external/next,nfermion  !containing the number of external
C                                       particles from the subroutine ?????????
          common/momenta/momenta(nmax,4),internal(nmax)
          common/sign/sign(3)            !correct sign for the four momenta
C
          if(oper.eq.5) then
           call findmom(momaux(1),momaux(2),mom1)
           call findmom(momaux(3),momaux(4),mom2)
           call findmom(momaux(5),momaux(6),mom3)
           if (sign(1).lt.0) then
            do j1=1,4                                
             mom1(j1)=-mom1(j1)                              
            enddo
           elseif (sign(2).lt.0) then
            do j1=1,4                                
             mom2(j1)=-mom2(j1)                              
            enddo
           elseif (sign(3).lt.0) then
            do j1=1,4                                
             mom3(j1)=-mom3(j1)                              
            enddo
           endif
C
          endif
C
           goto(1,2,3,4,5,6,7), oper
C
           write(6,*)'I have been given the label',oper,'to search for'
           write(6,*)'in subroutine LOROPER. It is not in my table'
C
 1         call scalscalscal(alfaprime)            !cubic scalar interaction
           return
C
 2         call psibarpsiphi(alfaprime)            !yukawa interaction
           return
C
 3         call gammamat(alfaprime)                !gammamu(a+b*gamma5)
           return
C
 4         write(6,*) 'nonexixsting label', oper,'in LOROPER'  !for historical reasons
           return
C
 5         call wpwmz(mom1,mom2,mom3,alfaprime)    !three bosons vertex
           return
C
 6         call gmunumat(alfaprime)      !auxiliary-gaubosons(W,Z)
           return
C
 7         call auxgluglu(alfaprime)
           return
C
           end
C***********************************************************************
          subroutine auxgluglu(alfaprime)            
C***********************************************************************
C
C This subroutine return the lorentz interaction matrix for the glu - glu
C auxiliar field interaction
C
          implicit none
C
          integer j1
          complex*16 alfaprime(96)        !storing the result of the "lorentz product"
C
          do j1=1,96
           alfaprime(j1)=0.
          enddo
C
          alfaprime(2)=-1.
          alfaprime(5)=1.
          alfaprime(19)=-1.
          alfaprime(25)=1.
          alfaprime(36)=-1.
          alfaprime(45)=1.
          alfaprime(55)=1.
          alfaprime(58)=-1.
          alfaprime(72)=1.
          alfaprime(78)=-1.
          alfaprime(92)=1.
          alfaprime(95)=-1.
C
          return
          end
C***********************************************************************
          subroutine scalscalscal(alfaprime)            
C***********************************************************************
C
C This subroutine return the lorentz interaction matrix for a cubic
C scalar interaction
C
          implicit none
C
          complex*16 alfaprime        !storing the result of the "lorentz product"
C
          dimension alfaprime(1)
C
          alfaprime(1) = (1.,0.)
C 
          return
          end
C***********************************************************************
          subroutine psibarpsiphi(alfaprime)     
C***********************************************************************
C
C This subroutine return the lorentz interaction matrix for a yukawa interaction
C scalar interaction
C
          implicit none
C
          integer j1          !loop index
          complex*16 alfaprime        !storing the result of the "lorentz product"
C
          dimension alfaprime(16)
C
          do j1=1,16
           alfaprime(j1)=0.
          enddo
          alfaprime(1)=(1.,0.)
          alfaprime(6)=(1.,0.)
          alfaprime(11)=(1.,0.)
          alfaprime(16)=(1.,0.)
C 
C add here
C
          return
          end
C***********************************************************************
          subroutine lorentz(const1,nlor1)
C***********************************************************************
C
C This subroutine returns the number of lorentz degrees of freedom of
C the field corresponding to const1
C
          implicit none
C
          integer nlor1       !storing the numer of coulor degrees of freedom of
C                              the particles in field1
          double precision const1       !containing the characteristic of the field:
C                              2) Lorentz nature 
C
          if (nint(const1).eq.1) then
           nlor1=1  !auxiliar scalar field 
          elseif (nint(const1).eq.2) then 
           nlor1=1  !scalar field
          elseif (nint(const1).eq.3) then
           nlor1=4    !fermion
          elseif (nint(const1).eq.4) then
           nlor1=4    !fermionbar
          elseif (nint(const1).eq.5) then
           nlor1=4    !massive gauge boson
          elseif (nint(const1).eq.6) then
           nlor1=4    !massless gauge boson
          elseif (nint(const1).eq.7) then
           nlor1=6    !auxiliar tensor field G_mn  (antisymmetric) for gluons
          else
           write(6,*)'wrong coulor index',const1,'in COULOR'
           stop
          endif
C
C add here
C
          return
          end
C***********************************************************************
          subroutine compconf(nmom1,nmom2,num1,num2,newc,perm) 
C***********************************************************************
C
C This subroutine given the label of two field configuration NC1 and NC2
C return the new field configuration NEWC produced as the product of the
C former one. If no allowed product arise NEWC is set to 0.
C
          implicit none
C
          integer nmax     !dimension of the arrays CONF, CONF1 and CONF2
          parameter (nmax=10)  !maximum number of external particle, 
          integer acceptable  !flag to check wether the building up 
C                              configuration is allowed
          integer conf(nmax)        !to store the new field configuration
          integer conf1(nmax),conf2(nmax)       !to store the old field configuration
          integer confperm(nmax)    !array to compute the sign of the permutation
          integer j1,j2          !loop label
          integer label       !array label
          integer nmom1,nmom2         !to store the number of momenta 
          integer next        !number of external particles
          integer nfermion
          integer num1,num2        !to store the label of the configuration
          integer newc       !to store a new label number
          integer nfer
C
          double precision perm(6)
C
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
C
          call seleconf(nmom1,num1,conf1)  !reconstructing the field 
C                                           configuration
          call seleconf(nmom2,num2,conf2)
C
          j1=1
          acceptable=1
          do while (acceptable.eq.1.and.j1.le.next)
           conf(j1)=conf1(j1)+conf2(j1)            !making the "product of the
C                                                     two configurations
           if(conf(j1).ne.0.and.conf(j1).ne.j1) acceptable=0 !each external 
C                        momenta should not contribute to a new configuration
C                        more than one
           j1=j1+1
          enddo
C
          if (acceptable.eq.1) then
           call findlabel(conf,newc,nmom1+nmom2)  !searching for the labels 
C
           do j2=1,next/2+1
            label=0                                
            nfer=2*(j2-1)
            if (nfer.ne.0) then                 
             do j1=1,nfer                       
              if (conf1(j1).ne.0) then              
               label=label+1                        
               confperm(label)=conf1(j1)            
              endif                               
             enddo                                
             do j1=1,nfer                    
              if (conf2(j1).ne.0) then
               label=label+1
               confperm(label)=conf2(j1)
              endif
             enddo
            endif
C
            perm(j2)=1.
            if (label.ge.2) call permutation(confperm,perm(j2),label) !it will  
C                                    return the sign of the permutation in PERM
C
           enddo
          else
           newc=0                       !this configuration is not allowed
          endif
C
          return
          end
C***********************************************************************
          subroutine compconflag(nmom1,nmom2,num1,num2,newc,perm) 
C***********************************************************************
C
C This subroutine given the label of two field configuration NC1 and NC2
C return the new field configuration NEWC produced as the product of the
C former one. If no allowed product arise NEWC is set to 0.
C
          implicit none
C
          integer nmax  !dimension of the arrays CONF, CONF1, CONF2 and CONFPERM
          parameter (nmax=10)  !maximum number of external particle, 
          integer acceptable  !flag to check wether the building up 
C                              configuration is allowed
          integer conf(nmax)        !to store the new field configuration
          integer conf1(nmax),conf2(nmax)       !to store the old field configuration
          integer confperm(nmax)    !array to be used to compute the sign of the
C                              fermion permutation       
          integer j1,j2          !loop label
          integer label       !array label
          integer next        !number of external particles
          integer nfermion
          integer nmom1,nmom2         !to store the number of momenta 
          integer num1,num2        !to sotre the label of the configuration
          integer newc       !to store a new label number
          integer nfer
C
          double precision perm(6)        !sign of the fermion permutation
C
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
C
          call seleconf(nmom1,num1,conf1)  !reconstructing the field 
C                                           configuration
          call seleconf(nmom2,num2,conf2)
C
          j1=1
          acceptable=1
          do while (acceptable.eq.1.and.j1.le.next)
           conf(j1)=conf1(j1)+conf2(j1)            !making the "product of the
C                                                     two configurations
           if(conf(j1).ne.0.and.conf(j1).ne.j1) acceptable=0 
           j1=j1+1
          enddo
C
          if (acceptable.eq.1) then
           do j1=1,next
            if (conf(j1).eq.0) then     
             conf(j1)=j1
            else
             conf(j1)=0
            endif
           enddo
           call findlabel(conf,newc,next-nmom1-nmom2)  
C
           do j2=1,next/2+1
            nfer=2*(j2-1)
            label=0                                
            if (nfer.ne.0) then                 
             do j1=1,nfer                       
              if (conf1(j1).ne.0) then              
              label=label+1                        
               confperm(label)=conf1(j1)           
              endif                              
             enddo                               
             do j1=1,nfer                    
              if (conf2(j1).ne.0) then
               label=label+1
               confperm(label)=conf2(j1)
              endif
             enddo
             do j1=1,nfer                    
              if (conf(j1).ne.0) then
               label=label+1
               confperm(label)=conf(j1)
              endif
             enddo
            endif
            perm(j2)=1.
            if (label.ge.2) call permutation(confperm,perm(j2),label) 
           enddo
          else
           newc=0                       !this configuration is not allowed
          endif
C
          return
          end
C***********************************************************************
          subroutine seleconf(nmom1,num1,conf1)
C***********************************************************************
C
C This subroutine given the label of a field configuration NUM1 and
C the number of the momenta NMOM1 which contribute to it 
C return the field configuration CONF1 which is an array of N=8=maximal
C number of external particle element. The n-th element of the array is
C n if the n-th external momentum contribute to this configuration,
C 0 otherwise
C
          implicit none
C
          integer dim1,dim2,dim3,dim4,dim5,dim6  !dimension of the array CONFIGURATION1,2,3,4
          integer nmax     !dimension of the array CONF
          parameter (dim1=11,dim2=55,dim3=165,dim4=330,dim5=462
     >                ,dim6=462) !these numbers are calculated as:
c          parameter (dim1=12,dim2=66,dim3=220,dim4=495,dim5=792
c     >                ,dim6=924) !these numbers are calculated as:
C                    DIMj=N!/j!/(N-j)!    where again N=NMAX number of external
C                     particle. Wishing to change NMAX these numbers have to be   changed
          parameter (nmax=10)  !maximum number of external particle, 
          integer conf1(nmax)   !to store the new field configuration
          integer configuration1,configuration2,configuration3,
     >             configuration4,configuration5,configuration6  !it stores all the configuration with 1,2,3,4
C                                   momenta contributing
          integer j1          !loop indices
          integer nmom1       !label of the configuration
          integer num1      !number of mometa contributing to the configuration
          integer next        !containing the number of external particles
          integer nfermion
C
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          common/momconfig/configuration1(dim1,nmax),
     >             configuration2(dim2,nmax),configuration3(dim3,nmax),
     >             configuration4(dim4,nmax),configuration5(dim5,nmax),
     >             configuration6(dim6,nmax)   !All this configuration are
C                            obtained in the subroutine CONFIGURATION. 
C
          goto (1,2,3,4,5,6),nmom1 !the pourpose of the labels is only
C                                           to create a menu
C
          write(6,*)'In subroutine SELECONF I have been given the'
          write(6,*)'label',nmom1,' to search for. This label is not'
          write(6,*)'in my labels table.'
C
          stop
C
  1       do j1=1,nmax 
           conf1(j1)=configuration1(num1,j1)
          enddo
          return
C
  2       do j1=1,nmax
           conf1(j1)=configuration2(num1,j1)
          enddo
          return
C
  3       do j1=1,nmax
           conf1(j1)=configuration3(num1,j1)
          enddo
          return
C
  4       do j1=1,nmax
           conf1(j1)=configuration4(num1,j1)
          enddo
          return
C
  5       do j1=1,nmax
           conf1(j1)=configuration5(num1,j1)
          enddo
          return
C
  6       do j1=1,nmax
           conf1(j1)=configuration6(num1,j1)
          enddo
          return
C
          end
C***********************************************************************
          subroutine findlabel(conf1,newc,nmom)
C***********************************************************************
C
C This subroutine given  a field configuration CONF made up of NMOM momenta
C return its corresponding label NEWC. 
C
          implicit none
C
          integer dim1,dim2,dim3,dim4,dim5,dim6  !dimension of the array CONFIGURATION1,2,3,4
          integer nmax     !dimension of the array CONF
          parameter (dim1=11,dim2=55,dim3=165,dim4=330,dim5=462
     >                ,dim6=462) !these numbers are calculated as:
c          parameter (dim1=12,dim2=66,dim3=220,dim4=495,dim5=792
c     >                ,dim6=924) !these numbers are calculated as:
C                    DIMj=N!/j!/(N-j)!    where again N=NMAX number of external
C                     particle. Wishing to change NMAX these numbers have to be   changed
          parameter (nmax=10)  !maximum number of external particle, change  here
          integer conf1(nmax)   !to store the new field configuration
          integer configuration1,configuration2,configuration3,
     >             configuration4,configuration5,configuration6  !it stores all the configuration with 1,2,3,4
C                                   momenta contributing
          integer ddim1,ddim2,ddim3,ddim4,ddim5,ddim6        !number of elemEnts in CONFIGURATION1 among
C                               which the configuration has to be looked for
          integer find1,find2       !flag to check wether the correct configuration
C                              has been found
          integer j1,j2          !loop index
          integer next        !number of external particles
          integer newc        !to store the label of the new configuration
          integer nmom        !containing the number ofo contributing momenta
C                              on which searching for the configuration
          integer nfermion
C
          common/dimens/ddim1,ddim2,ddim3,ddim4,ddim5,ddim6 !number of excitations for
C                 CONFIGURATION1,2,3,4,5,6 computed in subroutine CONFIGURATION
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          common/momconfig/configuration1(dim1,nmax),
     >           configuration2(dim2,nmax),configuration3(dim3,nmax),
     >           configuration4(dim4,nmax),configuration5(dim5,nmax),
     >           configuration6(dim6,nmax)         !All this configuration are
C                             obtained in the subroutine CONFIGURATION.
C
C we now have to implement the fact that if NEXT is odd only half
C of the dim-NEXT/2 elements contribute
C
          newc=0
          goto (1,2,3,4,5,6), nmom      !the pourpose of the labels is only
C                                           to create a menu
          write (6,*)'In subroutine FINDLABEL I have been given the '
          write (6,*)'label',nmom,' to search for. This label is not'
          write (6,*)'in my labels table.'
C
          stop
C
C All the following instruction separated be a string of CCCCCC are exactly equal. 
C It changes only the array which is used to search for the configuration.
C
 1        j1=1
          find1=0
          do while (find1.eq.0.and.j1.le.ddim1)    !loop to search for the right
C                             configuration. It has to stop when this is found
           j2=1
           find2=1
           do while (find2.eq.1.and.j2.le.next)
            if (configuration1(j1,j2).ne.conf1(j2)) find2=0 !the two 
C                   configurations are different, stop the j2 loop.           
            j2=j2+1
           enddo
C
           if (find2.eq.1) then     !we have found the right configuration
            find1=1                 !stop the j1 loop
            newc=j1                 !store the label of the new configuration
           endif
C
           j1=j1+1
C
          enddo
          if (find1.ne.1.and.dim1.eq.ddim1) 
     >       write(6,*)'mistake in FINDLABEL'
          return
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
 2        j1=1
          find1=0
          do while (find1.eq.0.and.j1.le.ddim2)    !loop to search for the right
C                             configuration. It has to stop when this is found
           j2=1
           find2=1
           do while (find2.eq.1.and.j2.le.next)
            if (configuration2(j1,j2).ne.conf1(j2)) find2=0 !the two 
C                   configurations are different, stop the j2 loop.           
            j2=j2+1
           enddo
C
           if (find2.eq.1) then     !we have found the right configuration
            find1=1                 !stop the j1 loop
            newc=j1                 !store the label of the new configuration
           endif
C
           j1=j1+1
C
          enddo
          if (find1.ne.1.and.dim2.eq.ddim2) 
     >       write(6,*)'mistake in FINDLABEL'
          return
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
 3        j1=1
          find1=0
          do while (find1.eq.0.and.j1.le.ddim3)    !loop to search for the right
C                             configuration. It has to stop when this is found
           j2=1
           find2=1
           do while (find2.eq.1.and.j2.le.next)
            if (configuration3(j1,j2).ne.conf1(j2)) find2=0 !the two 
C                   configurations are different, stop the j2 loop.           
            j2=j2+1
           enddo
C
           if (find2.eq.1) then     !we have found the right configuration
            find1=1                 !stop the j1 loop
            newc=j1                 !store the label of the new configuration
           endif
C
           j1=j1+1
C
          enddo
          if (find1.ne.1.and.dim3.eq.ddim3) 
     >       write(6,*)'mistake in FINDLABEL'
          return
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
 4        j1=1
          find1=0
          do while (find1.eq.0.and.j1.le.ddim4)    !loop to search for the right
C                             configuration. It has to stop when this is found
           j2=1
           find2=1
           do while (find2.eq.1.and.j2.le.next)
            if (configuration4(j1,j2).ne.conf1(j2)) find2=0 !the two 
C                   configurations are different, stop the j2 loop.           
            j2=j2+1
           enddo
C
           if (find2.eq.1) then     !we have found the right configuration
            find1=1                 !stop the j1 loop
            newc=j1                 !store the label of the new configuration
           endif
C
           j1=j1+1
C
          enddo
          if (find1.ne.1.and.dim4.eq.ddim4) 
     >       write(6,*)'mistake in FINDLABEL'
          return
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
 5        j1=1
          find1=0
          do while (find1.eq.0.and.j1.le.ddim5)    !loop to search for the right
C                             configuration. It has to stop when this is found
           j2=1
           find2=1
           do while (find2.eq.1.and.j2.le.next)
            if (configuration5(j1,j2).ne.conf1(j2)) find2=0 !the two 
C                   configurations are different, stop the j2 loop.           
            j2=j2+1
           enddo
C
           if (find2.eq.1) then     !we have found the right configuration
            find1=1                 !stop the j1 loop
            newc=j1                 !store the label of the new configuration
           endif
C
           j1=j1+1
C
          enddo
          if (find1.ne.1.and.dim5.eq.ddim5) 
     >       write(6,*)'mistake in FINDLABEL'
          return
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
 6        j1=1
          find1=0
          do while (find1.eq.0.and.j1.le.ddim6)    !loop to search for the right
C                             configuration. It has to stop when this is found
           j2=1
           find2=1
           do while (find2.eq.1.and.j2.le.next)
            if (configuration6(j1,j2).ne.conf1(j2)) find2=0 !the two 
C                   configurations are different, stop the j2 loop.           
            j2=j2+1
           enddo
C
           if (find2.eq.1) then     !we have found the right configuration
            find1=1                 !stop the j1 loop
            newc=j1                 !store the label of the new configuration
           endif
C
           j1=j1+1
C
          enddo
          if (find1.ne.1.and.dim6.eq.ddim6) 
     >       write(6,*)'mistake in FINDLABEL'
          return
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          end
C***********************************************************************
          subroutine configuration
C***********************************************************************
C
C This subroutine stores the momenta configurations with 1,...,4
C momenta in the array CONFIGURATION1,2,3,4
C
          implicit none
C
          integer dim1,dim2,dim3,dim4,dim5,dim6  !dimension of the array CONFIGURATION1,2,3,4
          integer nmax     !dimension of the array CONF
          parameter (dim1=11,dim2=55,dim3=165,dim4=330,dim5=462
     >                ,dim6=462) !these numbers are calculated as:
c          parameter (dim1=12,dim2=66,dim3=220,dim4=495,dim5=792
c     >                ,dim6=924) !these numbers are calculated as:
C                    DIMj=N!/j!/(N-j)!    where again N=NMAX number of external
C                     particle. Wishing to change NMAX these numbers have to be   changed
          parameter (nmax=10)   !maximal number of external particles 
C                               of esternal particles 
          integer configuration1,configuration2,configuration3,
     >             configuration4,configuration5,configuration6  !it stores all the configuration with 1,2,3,4
C                                   momenta contributing
          integer ddim1,ddim2,ddim3,ddim4,ddim5,ddim6        !number of elemEnts in CONFIGURATION1 among
C                               which the configuration has to be looked for
          integer j1,j2,j3,j4,j5,j6,j7          !loop index
          integer next        !number of external particles
          integer nfermion
          common/dimens/ddim1,ddim2,ddim3,ddim4,ddim5,ddim6 !number of excitations for
C                 CONFIGURATION1,2,3,4,5,6 computed in subroutine CONFIGURATION
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          common/momconfig/configuration1(dim1,nmax),
     >           configuration2(dim2,nmax),configuration3(dim3,nmax),
     >           configuration4(dim4,nmax),configuration5(dim5,nmax),
     >           configuration6(dim6,nmax)         !All this configuration are
C                             obtained in the subroutine CONFIGURATION.
C
           call zeroarrayint(configuration1,nmax*dim1)
           call zeroarrayint(configuration2,nmax*dim2)
           call zeroarrayint(configuration3,nmax*dim3)
           call zeroarrayint(configuration4,nmax*dim4)
           call zeroarrayint(configuration5,nmax*dim5)
           call zeroarrayint(configuration6,nmax*dim6)
C
C  filling CONFIGURATION1
C
           j1=1
           do j2=1,next
            configuration1(j1,j2)=j2
            j1=j1+1
           enddo
           ddim1=j1-1
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C  filling CONFIGURATION2
C
           j1=1
           do j2=1,next
            if (j2+1.le.next) then
             do j3=j2+1,next
              configuration2(j1,j2)=j2
              configuration2(j1,j3)=j3
              j1=j1+1
             enddo
            endif
           enddo
           ddim2=j1-1
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C  filling CONFIGURATION3
C
           j1=1
           do j2=1,next
            if (j2+1.le.next) then
             do j3=j2+1,next
              if (j3+1.le.next) then
               do j4=j3+1,next
                configuration3(j1,j2)=j2
                configuration3(j1,j3)=j3
                configuration3(j1,j4)=j4
                j1=j1+1
               enddo
              endif
             enddo
            endif
           enddo
           ddim3=j1-1
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C  filling CONFIGURATION4
C
           j1=1
           do j2=1,next
            if (j2+1.le.next) then
             do j3=j2+1,next
              if (j3+1.le.next) then
               do j4=j3+1,next
                if (j4+1.le.next) then
                 do j5=j4+1,next
                  configuration4(j1,j2)=j2
                  configuration4(j1,j3)=j3
                  configuration4(j1,j4)=j4
                  configuration4(j1,j5)=j5
                  j1=j1+1
                 enddo
                endif
               enddo
              endif
             enddo
            endif
           enddo
           ddim4=j1-1
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C  filling CONFIGURATION5
C
           j1=1
           do j2=1,next
            if (j2+1.le.next) then
             do j3=j2+1,next
              if (j3+1.le.next) then
               do j4=j3+1,next
                if (j4+1.le.next) then
                 do j5=j4+1,next
                  if (j5+1.le.next) then
                   do j6=j5+1,next
                    configuration5(j1,j2)=j2
                    configuration5(j1,j3)=j3
                    configuration5(j1,j4)=j4
                    configuration5(j1,j5)=j5
                    configuration5(j1,j6)=j6
                    j1=j1+1
                   enddo
                  endif
                 enddo
                endif
               enddo
              endif
             enddo
            endif
           enddo
           ddim5=j1-1
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

C
C  filling CONFIGURATION6
C
           j1=1
           do j2=1,next
            if (j2+1.le.next) then
             do j3=j2+1,next
              if (j3+1.le.next) then
               do j4=j3+1,next
                if (j4+1.le.next) then
                 do j5=j4+1,next
                  if (j5+1.le.next) then
                   do j6=j5+1,next
                    if (j6+1.le.next) then
                     do j7=j6+1,next
                      configuration6(j1,j2)=j2
                      configuration6(j1,j3)=j3
                      configuration6(j1,j4)=j4
                      configuration6(j1,j5)=j5
                      configuration6(j1,j6)=j6
                      configuration6(j1,j7)=j7
                      j1=j1+1
                     enddo
                    endif
                   enddo
                  endif
                 enddo
                endif
               enddo
              endif
             enddo
            endif
           enddo
           ddim6=j1-1
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C Again everything is tuned to N=8=maximal number of external particles 
C everything has to be changed accordingly if one wishes to change N
C
          if(next.eq.4)ddim2=ddim2/2      !to keep only the relevant configurations
          if(next.eq.6)ddim3=ddim3/2
          if(next.eq.8)ddim4=ddim4/2
          if(next.eq.10)ddim5=ddim5/2
          if(next.eq.12)ddim6=ddim6/2
C
          return
          end
C***********************************************************************
          subroutine fermionsources(spinsoura,fieldaux ,mom,mass)
C***********************************************************************
C
C This subroutine return the initialized fermion field.
C Elicity heigenstates
C 
          implicit none
C
          double precision  mass         !containing the mass of the external particle
          double precision  mom(4)          !array containing the momenta of the external particle
          double precision spinsoura    !array containing the spin of the source
          double precision p,p3p,p3m,mp,coeffp,coeffm
C                               
          complex*16 fieldaux(4)  !array returning the fermion field configuration
          complex*16 im  !immaginary unity in complex representation
          complex*16 p1p
          data im/(0.,1.)/
          integer spinsour
C
C Static variables
C
          save im
C
C  
C
          p=sqrt(mom(2)**2+mom(3)**2+mom(4)**2)
          p3p=p+mom(4)
          p3m=p-mom(4)
          mp=mass+mom(1)
          p1p=mom(2)+mom(3)*im
C
          if(abs(spinsoura).lt.0.5) spinsoura=-spinsoura
          spinsour=nint(100*spinsoura)
          if(abs(p3m).lt.1.d-10.or.abs(p3p).lt.1.d-10)then
           call fs(spinsour,fieldaux ,mom,mass)
           return
          endif
C
          coeffp=1./Sqrt(2.*p*mp*p3p)
          coeffm=1./Sqrt(2.*p*mp*p3m)
C
C "spin up" ingoing fermion
C
           if (spinsour.eq.49) then
            fieldaux(1)=coeffp*p3p*mp
            fieldaux(2)=coeffp*p1p*mp
            fieldaux(3)=coeffp*p3p*p
            fieldaux(4)=coeffp*p1p*p
           endif
C
C "spin down" ingoing fermion
C
           if (spinsour.eq.-49) then
            fieldaux(1)=coeffm*p3m*mp
            fieldaux(2)=-coeffm*p1p*mp
            fieldaux(3)=-coeffm*p3m*p
            fieldaux(4)=coeffm*p1p*p
           endif
C
C "spin up" outgoing antifermion
C
           if (spinsour.eq.51) then
            fieldaux(1)=-coeffm*p3m*p
            fieldaux(2)=coeffm*p1p*p
            fieldaux(3)=coeffm*p3m*mp
            fieldaux(4)=-coeffm*p1p*mp
           endif
C
C "spin down" outgoing antifermion
C
           if (spinsour.eq.-51) then
            fieldaux(1)=coeffp*p3p*p 
            fieldaux(2)=coeffp*p1p*p
            fieldaux(3)=coeffp*p3p*mp
            fieldaux(4)=coeffp*p1p*mp
           endif
C
           return
           end

C***********************************************************************
          subroutine fermionbarsources(spinsoura,fieldaux ,mom,mass)
C***********************************************************************
C
C This subroutine return the initialized fermionbar field.
C Elicity heigenstates
C 
          implicit none
C
          double precision  mass         !containing the mass of the external particle
          double precision  mom(4)          !array containing the momenta of the external particle
          double precision spinsoura    !array containing the spin of the source
          double precision p,p3p,p3m,mp,coeffp,coeffm
C                               
          complex*16 fieldaux(4)     !array returning the fermion field configuration
          complex*16 im  !immaginary unity in complex representation
          complex*16 p1p
          data im/(0.,1.)/
          integer spinsour
C
C Static variables
C
          save im
C
C
          p=sqrt(mom(2)**2+mom(3)**2+mom(4)**2)
          p3p=p+mom(4)
          p3m=p-mom(4)
          mp=mass+mom(1)
          p1p=mom(2)-mom(3)*im
C
          if(abs(spinsoura).lt.0.5) spinsoura=-spinsoura
          spinsour=nint(100*spinsoura)
          if(abs(p3m).lt.1.d-10.or.abs(p3p).lt.1.d-10)then
           call fbs(spinsour,fieldaux ,mom,mass)
           return
          endif
C
          coeffp=1./Sqrt(2.*p*mp*p3p)
          coeffm=1./Sqrt(2.*p*mp*p3m)
C
C "spin up" ingoing fermion
C
           if (spinsour.eq.49) then
            fieldaux(1)=coeffp*p3p*mp
            fieldaux(2)=coeffp*p1p*mp
            fieldaux(3)=-coeffp*p3p*p
            fieldaux(4)=-coeffp*p1p*p
           endif
C
C "spin down" ingoing fermion
C
           if (spinsour.eq.-49) then
            fieldaux(1)=coeffm*p3m*mp
            fieldaux(2)=-coeffm*p1p*mp
            fieldaux(3)=coeffm*p3m*p
            fieldaux(4)=-coeffm*p1p*p
           endif
C
C "spin up" outgoing antifermion
C
           if (spinsour.eq.51) then
            fieldaux(1)=-coeffm*p3m*p
            fieldaux(2)=coeffm*p1p*p
            fieldaux(3)=-coeffm*p3m*mp
            fieldaux(4)=coeffm*p1p*mp
           endif
C
C "spin down" outgoing antifermion
C
           if (spinsour.eq.-51) then
            fieldaux(1)=coeffp*p3p*p 
            fieldaux(2)=coeffp*p1p*p
            fieldaux(3)=-coeffp*p3p*mp
            fieldaux(4)=-coeffp*p1p*mp
           endif
C
           return
           end

C***********************************************************************
          subroutine fs(spinsour,fieldaux ,mom,mass)
C***********************************************************************
C
C This subroutine return the initialized fermion field.
C 
          implicit none
C
          double precision  mass         !containing the mass of the external particle
          double precision  mom(4)       !array containing the momenta of the external particle
          integer spinsour    !array containing the spin of the source
C                               
          complex*16 fieldaux(4)     !array returning the fermion field configuration
          complex*16 im  !immaginary unity in complex representation
          data im/(0.,1.)/
C
C Static variables
C
          save im
C
C
C "spin up" ingoing fermion
C
           if(mom(4).lt.0) spinsour=-spinsour
           if (spinsour.eq.49) then
            fieldaux(1)=sqrt((mom(1)+mass))
            fieldaux(2)=0. 
            fieldaux(3)=mom(4)/sqrt((mass+mom(1)))         
            fieldaux(4)=mom(2)/sqrt((mass+mom(1)))
     >                         + mom(3)/sqrt((mass+mom(1)))*im          
           endif
C
C "spin down" ingoing fermion
C
           if (spinsour.eq.-49) then
            fieldaux(1)=0.
            fieldaux(2)=sqrt((mom(1)+mass)) 
            fieldaux(3)= mom(2)/sqrt((mass+mom(1)))
     >                         - mom(3)/sqrt((mass+mom(1)))*im         
            fieldaux(4)=-mom(4)/sqrt((mass+mom(1)))
                    endif
C
C "spin up" outgoing antifermion
C
           if (spinsour.eq.51) then
            fieldaux(1)=mom(4)/sqrt((mass+mom(1)))         
            fieldaux(2)=mom(2)/sqrt((mass+mom(1)))
     >                         + mom(3)/sqrt((mass+mom(1)))*im          
            fieldaux(3)=sqrt((mom(1)+mass))
            fieldaux(4)=0. 
           endif
C
C "spin down" outgoing antifermion
C
           if (spinsour.eq.-51) then
            fieldaux(1)= mom(2)/sqrt((mass+mom(1)))
     >                         - mom(3)/sqrt((mass+mom(1)))*im         
            fieldaux(2)=-mom(4)/sqrt((mass+mom(1)))
            fieldaux(3)=0.
            fieldaux(4)=sqrt((mom(1)+mass))          
           endif
C
           return
           end
C***********************************************************************
          subroutine fbs(spinsour,fieldaux,mom,mass)
C***********************************************************************
C
C This subroutine return the initialized fermion field.
C 
          implicit none
C
          double precision  mass        !containing the mass of the external particle
          double precision  mom(4)      !array containing the momenta of the external particle
          integer spinsour    !array containing the spin of the source
C                               
          complex*16 fieldaux(4)   !array returning the fermionbar field configuration
          complex*16 im           !immaginary unity in complex representation
          data im/(0.,1.)/
C
C Static variables
C
          save im
C
C
C "spin up" outgoing fermion
C
           if(mom(4).gt.0) spinsour=-spinsour
           if (spinsour.eq.49) then
            fieldaux(1)=sqrt((mom(1)+mass))
            fieldaux(2)=0. 
            fieldaux(3)=-mom(4)/sqrt((mass+mom(1)))         
            fieldaux(4)=-mom(2)/sqrt((mass+mom(1)))
     >                         +mom(3)/sqrt((mass+mom(1)))*im          
           endif
C
C "spin down" outgoing fermion
C
           if (spinsour.eq.-49) then
            fieldaux(1)=0.
            fieldaux(2)=sqrt((mom(1)+mass)) 
            fieldaux(3)= -mom(2)/sqrt((mass+mom(1)))
     >                         - mom(3)/sqrt((mass+mom(1)))*im         
            fieldaux(4)=mom(4)/sqrt((mass+mom(1)))
                    endif
C
C "spin up" ingoing antifermion
C
           if (spinsour.eq.51) then
            fieldaux(1)=-mom(4)/sqrt((mass+mom(1)))         
            fieldaux(2)=-mom(2)/sqrt((mass+mom(1)))
     >                          +mom(3)/sqrt((mass+mom(1)))*im          
            fieldaux(3)=sqrt((mom(1)+mass))
            fieldaux(4)=0. 
           endif
C
C "spin down" ingoing antifermion
C
           if (spinsour.eq.-51) then
            fieldaux(1)= -mom(2)/sqrt((mass+mom(1)))
     >                         - mom(3)/sqrt((mass+mom(1)))*im         
            fieldaux(2)=mom(4)/sqrt((mass+mom(1)))
            fieldaux(3)=0.
            fieldaux(4)=sqrt((mom(1)+mass))          
           endif
C
           return
           end
C***********************************************************************
          subroutine pslashplusmtr(newc,iteration,pslplm,m1,w1)   
C***********************************************************************
C
C Returns the inverse propagator in the array PSLPLM for a fermionbar particle 
C with momentum defined by NEWC and ITERATION
C
          implicit none
C
          integer nmax       !maximal number of external particles, 
          parameter (nmax=10)  
          integer iteration     !iteration number
          integer j1         !loop index
C
          double precision  internal    !array of flags to decide wether the particle is
C                              internal or external
          integer newc       !to store a new label number
          integer next       !number of external particles
          integer nfermion
C
          double precision m1          !particle mass
          double precision mom(4)      !momentum fo teh present configuration
          double precision momenta     !array containing the external momenta
          double precision  w1          !width
C
          complex*16 den
          complex*16 im  !immaginary unit
          complex*16 pslplm(16)   !storing the relevant component of pslash+m
C
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          common/momenta/momenta(nmax,4),internal(nmax)
          data im/(0.,1.)/ 
C
C Static variables
C
          save im
C
C
          call findmom(iteration,newc,mom)
          do j1=1,4
           mom(j1)=-mom(j1)
          enddo
C
          pslplm(1)= mom(1)+m1
          pslplm(2)= 0.
          pslplm(3)= mom(4)
          pslplm(4)= mom(2)+mom(3)*im
          pslplm(5)= 0.
          pslplm(6)=  mom(1)+m1
          pslplm(7)= mom(2)-mom(3)*im
          pslplm(8)= -mom(4)
          pslplm(9)= -mom(4)
          pslplm(10)= -mom(2)-mom(3)*im
          pslplm(11)= -mom(1)+m1
          pslplm(12)= 0.
          pslplm(13)= -mom(2)+mom(3)*im
          pslplm(14)= mom(4)
          pslplm(15)= 0.
          pslplm(16)= -mom(1)+m1
C
          den=mom(1)*mom(1)-mom(2)*mom(2)-mom(3)*mom(3)-mom(4)*mom(4)
          den=den-m1*m1
          den=1./den
          do j1=1,16
           pslplm(j1)=pslplm(j1)*den
          enddo
C
          return
          end
C***********************************************************************
          subroutine pslashplusm(newc,iteration,pslplm,m1,w1)   
C***********************************************************************
C
C Returns the inverse propagator in the array PSLPLM for a fermion particle with
C momentum defined by NEWC and ITERATION
C
          implicit none
C
          integer nmax       !maximal number of external particles, 
          parameter (nmax=10)  
          integer iteration   !iteration number
          integer j1       !loop index
          integer newc       !to store a new label number
          integer next       !number of external particles
          integer nfermion
C
          double precision  internal    !array of flags to decide wether the particle is
          double precision m1           !particle mass
          double precision mom(4)       !momentum fo teh present configuration
          double precision momenta     !array containing the external momenta
C                              internal or external
          double precision  w1          !width
C
          complex*16 im     !immaginary unit
          complex*16 pslplm(16)     !storing the relevant component of pslash+m
          complex*16 den
C
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          common/momenta/momenta(nmax,4),internal(nmax)
          data im/(0.,1.)/
C
C Static variables
C
          save im
C
C
          call findmom(iteration,newc,mom)
C
          pslplm(1)= mom(1)+m1
          pslplm(2)= 0.
          pslplm(3)= -mom(4)
          pslplm(4)= -mom(2)+mom(3)*im
          pslplm(5)= 0.
          pslplm(6)=  mom(1)+m1
          pslplm(7)= -mom(2)-mom(3)*im
          pslplm(8)= mom(4)
          pslplm(9)= mom(4)
          pslplm(10)= mom(2)-mom(3)*im
          pslplm(11)= -mom(1)+m1
          pslplm(12)= 0.
          pslplm(13)= mom(2)+mom(3)*im
          pslplm(14)= -mom(4)
          pslplm(15)= 0.
          pslplm(16)= -mom(1)+m1
C
          den=mom(1)*mom(1)-mom(2)*mom(2)-mom(3)*mom(3)-mom(4)*mom(4)
          den=den-m1*m1
          den=1./den
          do j1=1,16
           pslplm(j1)=pslplm(j1)*den
          enddo
C
          return
          end
C***********************************************************************
          subroutine permutation(confperm,perm,nfermion) 
C***********************************************************************
C
C This Subroutine returns the sign of the fermion permutation in the variable
C PERM
C
          implicit none
C
          integer nmax        !maximal number of external particles, 
          parameter (nmax=10)  
          integer confperm(nmax) !array to compute the sign of the permutation
          integer nfermion    !number of external fermions
          integer j1,j2          !loop label
          integer parking1    !to store momentarily elemnts of 
C
          double precision perm
C
          perm=1.
          if (nfermion.gt.1) then       
           do j1=nfermion-1,1,-1
            do j2=1,j1
             if(confperm(j2).gt.confperm(j2+1)) then
              parking1=confperm(j2)
              confperm(j2)=confperm(j2+1)
              confperm(j2+1)=parking1
              perm=-perm
             endif
            enddo
           enddo
          endif
C
          return
          end
C***********************************************************************
          subroutine order (label,ordering)
C***********************************************************************
C
C It will return in LABEL the correct label order for the relevant operator
C according ot the ordering setted in order
C
          implicit none
C
          integer j1       !loop index
          integer label(3)    !it will contain the correct indices for the relevant
C                           operator
          integer ordering(3)    !it contains the order of the indices for the 
C                           relevant operator
          integer parking(3)  !for temporary storing
C
          do j1=1,3
           parking(j1)=label(j1)
          enddo
          do j1=1,3
           label(ordering(j1))=parking(j1)
          enddo
C 
          return
          end
C***********************************************************************
          subroutine exchange(field1,field2,mom1,mom2,numb1,numb2,
     >     const1,iteration)
C***********************************************************************
C
C This subroutine given two  field configuration FIELD1 and  FIELD2
C whose corresponding parents momenta are recorded in MOM1, and MOM2
C exchange all the information (FIELD, MOM and NUMB) obtained at the ITERATION
C step. 
C
          implicit none
C
          integer dimfiel      !dimension of the array field1,2,3
          integer dimnum       !dimension of the array numb1,2,3
          integer dimmom       !dimension of the array mom1,2,3
          integer dimcons      !dimension of the array cons1,2,3
          integer nmax         !maximal numer of external particles, change here
          integer nlormax !maximal number of lorentz and color 
C                                   degrees of freedom 
          parameter (nmax=10)
          parameter (nlormax=6)
          parameter (dimcons=5)      
          parameter (dimmom=(2**nmax-2)/2 +100)     
          parameter (dimfiel=nlormax*dimmom)  
          parameter (dimnum=nmax/2)      !storing the number of configuration
C                                    with 1,...,7 momenta contributing
          integer iteration   !storing the present iteration stage
          integer interval1,interval2   !setting the interval for the loops for particle 1
          integer j1,j2          !loop variable
          integer mom1(3,dimmom),mom2(3,dimmom)        !array containing a label for each indipendent
C                              momentum configuration. 
          integer mompark(3,dimmom)    !for temporay storing of MOM1
          integer nlor1      !storing the number of lorentz degrees of freedom
          integer numb1(dimnum),numb2(dimnum)      !to the number of momenta contributing
C                             to the configuration under exam
          integer numbpark(dimnum)   !for temporay storing of NUM1
          integer start1,start2 !setting the starting point for the loops for particle 1
C
          double precision const1(dimcons)
C
          complex*16 field1(dimfiel),field2(dimfiel)      !array containing the 
C                        field configuration of the two field to exchange
          complex*16 fieldpark(dimfiel)   !for temporay storing of FIELD1
C
           start1=0                        !all this configurations are ok
           start2=0                        !all this configurations are ok
           do j1=1,iteration-1
            start1=start1+numb1(j1)
            start2=start2+numb2(j1)
           enddo
C
           call lorentz(const1(2),nlor1)    !returning the number of lorentz
C                                         degrees of freedom of FIELD1,2
C
           interval1=numb1(iteration)     !all this configurations should be 
C                                          exchanged   
           interval2=numb2(iteration)     !all this configurations should be 
C                                          exchanged   
C
C  exchanging NUMB1(ITERATION) abd NUMB2(ITERATION)
C
          numbpark(iteration)=numb1(iteration)
          numb1(iteration)=numb2(iteration)
          numb2(iteration)=numbpark(iteration)
C
C  exchanging MOM1 abd MOM2 generated at the iteration step
C
          if (interval1.ne.0) then
           do j1=1,interval1
            do j2=1,3
             mompark(j2,j1)=mom1(j2,j1+start1)
            enddo
           enddo
          endif
          if (interval2.ne.0) then
           do j1=1,interval2
            do j2=1,3
             mom1(j2,j1+start1)=mom2(j2,j1+start2)
            enddo
           enddo
          endif
          if (interval1.ne.0) then
           do j1=1,interval1
            do j2=1,3
             mom2(j2,j1+start2)=mompark(j2,j1)
            enddo
           enddo
          endif
C
C  exchanging FIELD1 abd FIELD2 generated at the iteration step
C
          start1=start1*nlor1
          start2=start2*nlor1
          interval1=interval1*nlor1
          interval2=interval2*nlor1
          if (interval1.ne.0) then
           do j1=1,interval1
            fieldpark(j1)=field1(j1+start1)
           enddo
          endif
          if (interval2.ne.0) then
           do j1=1,interval2
            field1(j1+start1)=field2(j1+start2)
           enddo
          endif
          if (interval1.ne.0) then
           do j1=1,interval1
            field2(j1+start2)=fieldpark(j1)
           enddo
          endif
C
           return
           end
C***************************************************************************
          subroutine sourcemassboson(spinsour,fieldaux,mom,mass)
C***************************************************************************
C
C This subroutine given the four momentum of a massive boson MOM the required
C source polarization SPINSOUR and the boson mass MASS returns in
C FIELDAUX the source term
C
          implicit none
C
          integer n                    !loop index
C
          double precision knorm2      !squared modulus of the three momentum
          double precision mass        !massive boson mass
          double precision massa2      !squared mass
          double precision mom(4)      !four momentum
          double precision nz(4),nx(4)
          double precision sour1(4),sour2(4),sour3(4)   !longitudinal (1)
C                                and transverse (2,3) polarizations
          double precision spinsour   !required polarization
          double precision scalar3
          double precision xnorm      !to normalize the sources
          complex*16 fieldaux(4)      !to return the relevan polarization    
C
          data nz/0.,0.,0.,1./
          data nx/0.,1.,0.,0./
          data sour1/4*0./
          data sour2/4*0./
          data sour3/4*0./
C
          save nx,nz
C
C 
C
          knorm2=mom(2)**2+mom(3)**2+mom(4)**2  
C
          if (knorm2.eq.0) then
           sour1(2)=1.
           sour2(3)=1.
           sour3(4)=1.
           return
          endif
          massa2=mass**2
C
C    pol. longitudinal
C
          do n=2,4
           sour1(n)=mom(n)
          enddo
          sour1(1)=knorm2/mom(1)
          xnorm=sqrt(scalar3(sour1,sour1))
          if(xnorm.ne.0.) then
           do n=1,4
            sour1(n)=sour1(n)/xnorm
           enddo
          endif
C
C    pol. transverse +
C
          if (mom(4)**2.gt.mom(2)**2) then 
           call vector3prod(mom,nx,sour2)
          else 
           call vector3prod(mom,nz,sour2)  
          endif
          xnorm=Sqrt(scalar3(sour2,sour2))
          do n=1,4
           sour2(n)=sour2(n)/xnorm
          enddo
C
C    pol. transverse -
C
          call vector3prod(mom,sour2,sour3)
          xnorm=sqrt(scalar3(sour3,sour3))
          do n=1,4
           sour3(n)=sour3(n)/xnorm
          enddo
C
          do n=1,4
           fieldaux(n)=0.
           if(nint(spinsour).eq.0)fieldaux(n)=sour1(n)
           if(nint(spinsour).eq.1)fieldaux(n)=sour2(n)
           if(nint(spinsour).eq.-1)fieldaux(n)=sour3(n)
          enddo
C
          return
          end
C*********************************************************************
          subroutine vector3prod(v1,v2,vfin)
C*********************************************************************
C
          implicit none
C
           double precision v1(4)
           double precision v2(4)
           double precision vfin(4)
C
           vfin(2)=v1(3)*v2(4)-v1(4)*v2(3)   
           vfin(3)=v1(4)*v2(2)-v1(2)*v2(4)   
           vfin(4)=v1(2)*v2(3)-v1(3)*v2(2)   
           vfin(1)=0 
C                                               
           return 
           end 
C*********************************************************************** 
           function scalar3(v1,v2)
C***********************************************************************
C
           implicit none
C
           double precision scalar3
           double precision v1(4)
           double precision v2(4)
C
           scalar3=abs(v1(1)*v2(1)-v1(2)*v2(2)-v1(3)*v2(3)-v1(4)*v2(4)) 
C
            return
            end                                                  
C***********************************************************************
        subroutine gammamat(gamtemp)
C***********************************************************************
C
C Returns a*V + b*A interactions, needs to be modified for multiple processes
C
        implicit none
C
        integer n1,n2,n3,m1,sign,label,flag(100)
C
        complex*16 a,b,c1,c2
        complex*16  gamtemp(64),gamtemp1(4,4,4),gam(4,4,5),
     >              iden(4,4),gammastore(64,100)
C
        common/vecass/a,b,c1,c2
C
        data flag/100*0/
        data gam/(1.,0.),(0.,0),(0.,0.),(0.,0.),(0.,0.)
     >     ,(1.,0.),(0.,0.),(0.,0.),(0.,0.),(0.,0.),
     >  (-1.,0.),(0.,0.),(0.,0.),(0.,0.), (0.,0.),(-1.,0.),          !gamma0
     >  (0.,0.),(0.,0.),(0.,0.),(1.,0.),(0.,0.),
     >  (0.,0.),(1.,0.),(0.,0.),(0.,0.),(-1.,0.),
     >  (0.,0.),(0.,0.),(-1.,0.),(0.,0.),(0.,0.),(0.,0.),             !gamma1
     >  (0.,0.),(0.,0.),(0.,0.),(0.,-1.),(0.,0.), 
     >  (0.,0.),(0.,1.),(0.,0.),(0.,0.),(0.,1.), 
     >  (0.,0.),(0.,0.),(0.,-1.),(0.,0.),(0.,0.), (0.,0.),           !gamma2
     >  (0.,0.),(0.,0.),(1.,0.),(0.,0.),(0.,0.),
     >  (0.,0.),(0.,0.),(-1.,0.),(-1.,0.),(0.,0.),
     >  (0.,0.),(0.,0.),(0.,0.),(1.,0.),(0.,0.),(0.,0.),              !gamma3
     >  (0.,0.),(0.,0.),(1.,0.),(0.,0.),(0.,0.), 
     >  (0.,0.),(0.,0.),(1.,0.),(1.,0.),(0.,0.),
     >   (0.,0.),(0.,0.),(0.,0.),(1.,0.),(0.,0.),(0.,0.)/           !gamma5
        data iden/(1.,0.), (0.,0.), (0.,0.), (0.,0.),
     *   (0.,0.), (1.,0.),(0.,0.), (0.,0.), (0.,0.), (0.,0.),
     *   (1.,0.), (0.,0.),(0.,0.), (0.,0.), (0.,0.), (1.,0.)/
C
C
C Static variables
C
          save gam,iden,gammastore
C
C
        label=nint(abs(4*(c1-1)+c2))
        if (label.gt.100) then
         write(6,*)'label overflow in GAMMAMAT'
         stop
        endif
        if (flag(label).eq.0) then
         flag(label)=1
         do n1=1,4
       	  do n2=1,4
           do n3=1,4
            gamtemp1(n1,n2,n3)=0
            do m1=1,4
             gamtemp1(n1,n2,n3)=gam(m1,n1,n3)*
     *         (a*iden(m1,n2)+b*gam(n2,m1,5))+gamtemp1(n1,n2,n3)
            enddo 
           enddo
          enddo
         enddo         
C
c < *** gcc3.1 fix 
         n1= 0
         n2= 0
         n3= 0
c *** gcc3.1 fix >
         do n1=0,3
          do n2=0,3
           do n3=0,3
            if (16*n3+4*n2+n1+1.gt.16) then
             sign=-1
            else
             sign=1
            endif
            gamtemp(16*n3+4*n2+n1+1)=gamtemp1(n2+1,n1+1,n3+1)*sign
           enddo
          enddo
         enddo
C
         do n1=1,64
          gammastore(n1,label)=gamtemp(n1)
         enddo
         else
         call equalcomparrays(gammastore(1,label),gamtemp,64)
        endif
C
        return
      end
C***********************************************************************
          subroutine gmnkmkn(newc,iteration,pslplm)
C***********************************************************************
C
C Returns the inverse propagator in the array PSLPLM for a massive boson
C  particle with momentum defined by NEWC and ITERATION
C
          implicit none
C
          integer nmax          !maximal number of external particles, 
          parameter (nmax=10)  
          integer iteration        !iteration number
          integer j1               !loop index
          integer newc          !to store a new label number
          integer next          !number of external particles
          integer nfermion
C
          double precision internal
          double precision m1,w1                !particle mass and width
          double precision mom(4)            !momentum for the present configuration
          double precision momenta        !array containing the external momenta
          double precision theta
C
          complex*16 pqm,pq            !momentum squared - particle mass squared
          complex*16 pslplm(16)  !storing the relevant component of pslash+m
C
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          common/momenta/momenta(nmax,4),internal(nmax)
C
          call findmom(iteration,newc,mom)
C
          pq=mom(1)**2-mom(2)**2-mom(3)**2 -mom(4)**2
          pqm=pq
          pqm=pqm/17.3                                 !17.3 gauge parameter
C
          pslplm(1)= pqm-mom(1)*mom(1)
          pslplm(2)= -mom(1)*mom(2)
          pslplm(3)= -mom(1)*mom(3)
          pslplm(4)= -mom(1)*mom(4)
          pslplm(5)= pslplm(2)
          pslplm(6)=-pqm-mom(2)*mom(2)
          pslplm(7)= -mom(2)*mom(3)
          pslplm(8)= -mom(2)*mom(4)
          pslplm(9)= pslplm(3)
          pslplm(10)= pslplm(7)
          pslplm(11)= -pqm-mom(3)*mom(3)
          pslplm(12)= -mom(3)*mom(4)
          pslplm(13)= pslplm(4)
          pslplm(14)= pslplm(8)
          pslplm(15)= pslplm(12)
          pslplm(16)= -pqm-mom(4)*mom(4)
C
          pqm=-1./pqm/pq
C
          do j1=1,16
           pslplm(j1)=pslplm(j1)*pqm
          enddo
C
          return
          end
C***********************************************************************
          subroutine gmnminkmkn(newc,iteration,pslplm,m1,w1)
C***********************************************************************
C
C Returns the inverse propagator in the array PSLPLM for a massive boson
C  particle with momentum defined by NEWC and ITERATION
C
          implicit none
C
          integer nmax          !maximal number of external particles, 
          parameter (nmax=10)  
          integer iteration        !iteration number
          integer j1               !loop index
          integer newc          !to store a new label number
          integer next          !number of external particles
          integer nfermion
C
          double precision internal
          double precision m1,w1                !particle mass and width
          double precision mom(4)            !momentum for the present configuration
          double precision momenta        !array containing the external momenta
          double precision theta,pq
C
          complex*16 pqm            !momentum squared - particle mass squared
          complex*16 pslplm(16)  !storing the relevant component of pslash+m
C
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          common/momenta/momenta(nmax,4),internal(nmax)
C
          character*2 wmode
          character resonance
          double precision winsize
          common/gauinv/winsize,resonance,wmode
C
          if (resonance.eq.'y') then
            do j1=1,16
              pslplm(j1)=0.d0
            enddo
            return
          endif
C
          call findmom(iteration,newc,mom)
C
          pqm=m1**2
C
          pslplm(1)= pqm-mom(1)*mom(1)
          pslplm(2)= -mom(1)*mom(2)
          pslplm(3)= -mom(1)*mom(3)
          pslplm(4)= -mom(1)*mom(4)
          pslplm(5)= pslplm(2)
          pslplm(6)=-pqm-mom(2)*mom(2)
          pslplm(7)= -mom(2)*mom(3)
          pslplm(8)= -mom(2)*mom(4)
          pslplm(9)= pslplm(3)
          pslplm(10)= pslplm(7)
          pslplm(11)= -pqm-mom(3)*mom(3)
          pslplm(12)= -mom(3)*mom(4)
          pslplm(13)= pslplm(4)
          pslplm(14)= pslplm(8)
          pslplm(15)= pslplm(12)
          pslplm(16)= -pqm-mom(4)*mom(4)
C
          pq=mom(1)**2-mom(2)**2-mom(3)**2 -mom(4)**2

c          theta=1.
c          if (pq.gt.0)theta=1
C
          wmode = 'yy'
c          pqm=-1./(pq-pqm+theta*pq*w1*(0.,1.)/m1)/pqm
          if(wmode.eq.'yy') then
            pqm=-1./((pq-pqm)+m1*w1*(0.,1.))/pqm
            resonance='n'
          elseif(wmode.eq.'yn') then
            if(abs(pqm-pq).lt.winsize*m1*w1) then
              resonance='y'
            else
              resonance='n'
            endif
            if(abs(pqm-pq).eq.0.d0) then
              pqm=1.d0
              return
            endif
            pqm=-1./(pq-pqm)/pqm            
          elseif(wmode.eq.'nn') then
            pqm=-1./(pq-pqm)/pqm 
            resonance='n'
          endif
C
          do j1=1,16
           pslplm(j1)=pslplm(j1)*pqm
          enddo
C
          return
          end
C***********************************************************************
        subroutine wpwmz(k1,k2,k3,alpha)
C***********************************************************************
C
C Returns the trilinear boson self interaction (su(2) group W^+ W^- Z)
C Optimize here!
        implicit none
C
        double precision k1(4),k2(4),k3(4)
C
        complex*16 alpha(64)
C
        k3(1)=-k3(1)
        k2(1)=-k2(1)
        k1(1)=-k1(1)
C
        alpha(1)=0.
        alpha(2)=k2(2)-k1(2)
        alpha(3)=k2(3)-k1(3)
        alpha(4)=k2(4)-k1(4)
        alpha(5)=k1(2)-k3(2)
        alpha(6)=k2(1)-k3(1)
        alpha(7)=0.
        alpha(8)=0.
        alpha(9)=k1(3)-k3(3)
        alpha(10)=0.
        alpha(11)=k2(1)-k3(1)
        alpha(12)=0.
        alpha(13)=k1(4)-k3(4)
        alpha(14)=0.
        alpha(15)=0.
        alpha(16)=k2(1)-k3(1)
        alpha(17)=k3(2)-k2(2)
        alpha(18)=k3(1)-k1(1)
        alpha(19)=0.
        alpha(20)=0.
        alpha(21)=k1(1)-k2(1)
        alpha(22)=0.
        alpha(23)=k1(3)-k2(3)
        alpha(24)=k1(4)-k2(4)
        alpha(25)=0.
        alpha(26)=k3(3)-k1(3)
        alpha(27)=k2(2)-k3(2)
        alpha(28)=0.
        alpha(29)=0.
        alpha(30)=k3(4)-k1(4)
        alpha(31)=0.
        alpha(32)=k2(2)-k3(2)
        alpha(33)=k3(3)-k2(3)
        alpha(34)=0.
        alpha(35)=k3(1)-k1(1)
        alpha(36)=0.
        alpha(37)=0.
        alpha(38)=k2(3)-k3(3)
        alpha(39)=k3(2)-k1(2)
        alpha(40)=0.
        alpha(41)=k1(1)-k2(1)
        alpha(42)=k1(2)-k2(2)
        alpha(43)=0.
        alpha(44)=k1(4)-k2(4)
        alpha(45)=0.
        alpha(46)=0.
        alpha(47)=k3(4)-k1(4)
        alpha(48)=k2(3)-k3(3)
        alpha(49)=k3(4)-k2(4)
        alpha(50)=0.
        alpha(51)=0.
        alpha(52)=k3(1)-k1(1)
        alpha(53)=0.
        alpha(54)=k2(4)-k3(4)
        alpha(55)=0.
        alpha(56)=k3(2)-k1(2)
        alpha(57)=0.
        alpha(58)=0.
        alpha(59)=k2(4)-k3(4)
        alpha(60)=k3(3)-k1(3)
        alpha(61)=k1(1)-k2(1)
        alpha(62)=k1(2)-k2(2)
        alpha(63)=k1(3)-k2(3)
        alpha(64)=0.
C
        return
        end
C***********************************************************************
         subroutine compute(field1,field2,field3,mom1,mom2,mom3,
     >                   numb1,numb2,numb3,const1,const2,const3,
     >             operator,iteration,couplingconst,elmat,niteration)
C***********************************************************************
C
C It calls the SUBROUTINE INTERACTION to compute the perturbative solution
C
          implicit none
C
          complex*16 couplingconst
          complex*16 elmat      !matrix element
          complex*16 elmataux   !for intermediate storage of ELMAT
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C  setting the variables for the dummy fields FIELD1,FIELD2,FIELD3,.....
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C  FIELD.COMMM            shared by ITERA, SELECTFIELD and REPLACEFIELD
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          integer dimfiel      !dimension of the array field1,2,3
          integer dimnum       !dimension of the array numb1,2,3
          integer dimmom       !dimension of the array mom1,2,3
          integer dimcons      !dimension of the array cons1,2,3
          integer dimop        !dimension of the array operator
          integer nmax         !maximum number of external particles, 
          integer nlormax      !maximum number of lorentz d.o.f., 
          parameter (nmax=10)
          parameter (nlormax=6)
          parameter (dimmom=(2**nmax-2)/2 +100)     
          parameter (dimnum=nmax/2)      
          parameter (dimop=2)
          parameter (dimcons=5)      
          parameter (dimfiel=nlormax*dimmom)  
          integer iteration   !storing the present iteration stage
          integer mom1(3,dimmom),mom2(3,dimmom),mom3(3,dimmom)  !array containing a label for each indipendent
C                              momentum configuration. This number will allow
C                              to determine the complete configuration   
          integer niteration !number of iteration steps
          integer numb1(dimnum),numb2(dimnum),numb3(dimnum) 
          integer operator(dimop)  !a flag to identify the type of operator, 
C
          double precision const1(dimcons) , const2(dimcons), 
     >                     const3(dimcons) !containing the characteristic of the field:
C
          complex*16 field1(dimfiel),field2(dimfiel),field3(dimfiel)  !array containing the field configuration
C                                        of one of the three field to multiply
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C          save elmat
C
           if (iteration.le.niteration) then
            call interaction(field1,field2,field3,mom1,mom2,mom3,
     >                        numb1,numb2,numb3,const1,const2,const3
     >                            ,operator,iteration,couplingconst)
           else
            call lagint(field1,field2,field3,mom1,mom2,mom3,
     >                  numb1,numb2,numb3,const1,const2,const3
     >                  ,operator,couplingconst,elmataux)
            elmat=elmat+elmataux
C
           endif
C
           return
           end
C***********************************************************************
        subroutine gmunumat(metric)
C***********************************************************************
      implicit none
C
        complex*16 metric(16)
        integer j1
C
        do j1=1,16
         metric(j1)=0.
        enddo
C
        metric(1)=(1.,0.)
        metric(6)=(-1.,0.)
        metric(11)=(-1.,0.)
        metric(16)=(-1.,0.)
C
        return
        end    
C***********************************************************************
          subroutine processo_h(flvmlm,posi,nparticle)
C***********************************************************************
          implicit none
C
          integer ngaubos
          parameter (ngaubos=14)
          integer nmax     !maximum number of external particles, change here
          parameter (nmax=10)          
          integer posi(2)  !position of incoming particles
          integer flvmlm(nmax) !type of incoming particles (mlm convention)
          integer nparticle(2) !number of total (1) and incoming (2) particle
          integer flag
          integer indexoutgoing(nmax) !reporting the order of storage of  
C                                      outgoing momenta
          integer indexingoing(nmax) !reporting the order of storage of  
C                                      ingoing momenta
          integer j1,j3,j4    !loop variable
          integer next        !number of external particles
          integer nfermion    !counting the number of anticommuting particles
          integer noutgoing   !number of outgoing particles
          integer nsourfermion, nsourfermionbar, nsourhiggs,
     >            nsourgaubosons
C
          double precision  internal(nmax)    !array of flags to decide 
C                                 wether the particle is internal or external
          double precision momenta      !particles momenta
          double precision  massoutgoing(nmax) !containing the masses of
C                                               outcoming particles
          double precision masshiggs(3)              !higgs mass
          double precision massgaubosons(ngaubos)    !zboson mass
          double precision massfermion(3,4)          !fermion masses
          double precision widthhiggs(3)             !higgs width
          double precision widthgaubosons(ngaubos)   !zboson width
          double precision widthfermion(3,4)         !fermion widthes
          double precision  spinsour, spinsouraux  
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Containing the common for number and spin of external particles. Shared byC
C the subroutines ITERA, PROCESSO and SPINVARIABLE;                         C
C ITERA, PROCESSO, FILLCONFSPIN AND SPINCOULOR respectively                 C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
         common/source/nsourhiggs(3),nsourgaubosons(ngaubos),
     >                 nsourfermion(3,4),nsourfermionbar(3,4)
         common/spin/spinsour(nmax),spinsouraux(nmax)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C MASSES.COMM                                                               C
C                                                                           C
C Containing the common for particles masses. Shared by the subroutines     C
C ITERA and COUPLINGS                                                       C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          common/masses/masshiggs,massgaubosons,massfermion ,
     >                  widthhiggs,widthgaubosons,widthfermion !in this
C              common all the  masses of the particles are transferred.
C                  Wishing to add a new particle change here
CCCCCCCCC
          common/external/next,nfermion  !containing the number of external
C                                      particles from the subroutine ??????
          common/momenta/momenta(nmax,4),internal
          common/integra/massoutgoing,
     >                       noutgoing,indexoutgoing,indexingoing
C
         integer inputinteraction(1000),count3   !to initialize relevant 
C                                                 interaction terms
         common/interactions/inputinteraction,count3 !shared with subroutine
         integer next_old                !old number of esternal particles
C
         integer tabmlm(3,0:100),i2,i3,i1    !flavour table according to 
C                                             mlm convention
C
         integer inpint(1000)
         common/initinter/inpint 
C
         integer ndirac                      !for fermi/dirac statistic
         common/dirac/ndirac
C
         data tabmlm/11, 0, 3,         !gluon
     >                1, 2, 2,         !d quark
     >                1, 1, 2,         !u quark
     >                2, 2, 2,         !s quark
     >                2, 1, 2,         !c quark
     >                3, 2, 2,         !b quark
     >                3, 1, 2,         !t quark
     >                12*0,
     >                1, 4, 2,         !electron
     >                1, 3, 2,         !neutrino
     >                2, 4, 2,         !muon
     >                2, 3, 2,         !neutrino mu
     >                 21*0,
     >               10, 0, 3,         !photon
     >                1, 0, 3,         !Z
     >               14, 0, 3,         !DM U
     >                2, 0, 3,         !W+
     >                1, 0, 1,         !higgs
     >                222*0/     
C
         data flag/0/
         data indexoutgoing/nmax*0/
         data indexingoing/nmax*0/
         data internal/nmax*1./
         data next_old/0/
C                                                       READARRAYTWOBYTWO_H
C
         call couplings
c         if (flag.eq.0) call couplings
C
         do j3=1,3
          do j4=1,4
           nsourfermion(j3,j4)=0
           nsourfermionbar(j3,j4)=0
          enddo
         enddo
C
         do j3=1,3
          nsourhiggs(j3)=0
         enddo
C
         do j3=1,ngaubos
          nsourgaubosons(j3)=0
         enddo
C
         do j3=1,nmax
          spinsouraux(j3)=0
          massoutgoing(j3)=0.
         enddo
C
         do j3=1,nmax
          indexingoing(j3)=0
          indexoutgoing(j3)=j3
          internal(j3)=-1.
         enddo        
C
        nfermion=0
        do j1=1,nparticle(2)
         flvmlm(posi(j1))=-flvmlm(posi(j1))        
        enddo
        do j1=1,nparticle(1)
         i1=tabmlm(1,abs(flvmlm(j1)))
         i2=tabmlm(2,abs(flvmlm(j1)))
         i3=tabmlm(3,abs(flvmlm(j1)))
         if (i3.eq.3.and.i1.eq.2.and.flvmlm(j1).lt.0) i1=3
         if(i3.eq.2) nfermion=nfermion+1
         if (i3.eq.1) then
          nsourhiggs(i1)=nsourhiggs(i1)+1
          spinsouraux(j1)=0.
         elseif (i3.eq.2) then
          if(flvmlm(j1).lt.0) then
           nsourfermion(i1,i2)=nsourfermion(i1,i2)+1
          else
           nsourfermionbar(i1,i2)=nsourfermionbar(i1,i2)+1
          endif
          if (flvmlm(j1).gt.0) then
           spinsouraux(j1)=0.49
          else
           spinsouraux(j1)=0.51
          endif
         elseif (i3.eq.3) then
          nsourgaubosons(i1)=nsourgaubosons(i1)+1
          if(i1.le.3) then
           spinsouraux(j1)=1.
          elseif (i1.eq.10.or.i1.eq.11) then
           spinsouraux(j1)=1.1
          endif
         endif
        enddo
C
        if(posi(2).lt.posi(1)) then               !????????????????????
         j1=posi(1)
         posi(1)=posi(2)
         posi(2)=j1
        endif    
C
        do j1=1,nparticle(2)
         i1=10*spinsouraux(posi(j1))
         if(i1.eq.4) then
          spinsouraux(posi(j1))=0.51
         elseif (i1.eq.5) then
          spinsouraux(posi(j1))=0.49
         endif        
        enddo
C
        do j1=1,posi(1)-1
         indexoutgoing(j1)=j1
        enddo
        do j1=posi(1),posi(2)-2
         indexoutgoing(j1)=j1+1
        enddo
        do j1=posi(2)-1,nparticle(1)-2
         indexoutgoing(j1)=j1+2
        enddo
C
        do j1=1,nparticle(2)
         internal(posi(j1))=1.
        enddo
C
        next=nparticle(1)
        noutgoing=nparticle(1)-nparticle(2)
C
C
C  Initializing relevant interaction terms
C
         count3=0
c         inputinteraction(1)=7    !      Number of V-A interaction
c         inputinteraction(2)=11
c         inputinteraction(3)=1
c         inputinteraction(4)=1
c         inputinteraction(5)=1
c         inputinteraction(6)=1    !      glu ubar u
c         inputinteraction(7)=11
c         inputinteraction(8)=1
c         inputinteraction(9)=2
c         inputinteraction(10)=1
c         inputinteraction(11)=2   !     glu dbar d
c         inputinteraction(12)=11
c         inputinteraction(13)=3
c         inputinteraction(14)=2
c         inputinteraction(15)=3
c         inputinteraction(16)=2   !     glu bbar b  
c         inputinteraction(17)=2
c         inputinteraction(18)=1
c         inputinteraction(19)=1
c         inputinteraction(20)=1
c         inputinteraction(21)=2   !     w  ubar d
c         inputinteraction(17)=3
c         inputinteraction(18)=1
c         inputinteraction(19)=2
c         inputinteraction(20)=1
c         inputinteraction(21)=1   !     w dbar u
c         inputinteraction(22)=2  
c         inputinteraction(23)=1 
c         inputinteraction(24)=3  
c         inputinteraction(25)=1  
c         inputinteraction(26)=4   !     w nubar e  
c         inputinteraction(22)=3  
c         inputinteraction(23)=1  
c         inputinteraction(24)=4  
c         inputinteraction(25)=1  
c         inputinteraction(26)=3  !      w ebar nu  
c         inputinteraction(27)=11
c         inputinteraction(28)=2
c         inputinteraction(29)=1
c         inputinteraction(30)=2
c         inputinteraction(31)=1   !     glu cbar c  
c         inputinteraction(32)=11
c         inputinteraction(33)=3
c         inputinteraction(34)=1
c         inputinteraction(35)=3
c         inputinteraction(36)=1
c         inputinteraction(37)=0  !      number of Yukawa interaction
c         inputinteraction(38)=2  !      number of selfgauge
c         inputinteraction(39)=12
c         inputinteraction(40)=11
c         inputinteraction(41)=11  !     aux glu glu
c         inputinteraction(42)=11
c         inputinteraction(43)=11
c         inputinteraction(44)=11  !     glu glu glu 
c         inputinteraction(45)=0   !     number of Gauge-higgs couplings
C
C common environment
C
         do j3=1,next
          spinsour(j3)=spinsouraux(j3)
         enddo
C
         if(next.ne.next_old) flag=0
         next_old=next
C
         if(flag.eq.0) then    !initialization step for coupling constants
          flag=1         !and to define product among momentum configurations
          j3=0
           do while(j1.ge.0)
            j3=j3+1
            j1=inpint(j3)
            inputinteraction(j3)=j1
           enddo
          call configuration
          call initcompconf
         endif
C
        ndirac=nfermion/2+2
C
C
        do j1=1,nparticle(2)
         flvmlm(posi(j1))=-flvmlm(posi(j1))        
        enddo
C
        return
        end
C***********************************************************************
          subroutine processo_hx(nproc)
C***********************************************************************
          implicit none
C
          integer ngaubos
          parameter (ngaubos=14)
          integer nmax     !maximum number of external particles, change here
          parameter (nmax=10)          
          integer nlb
          parameter (nlb=4)            
          integer flag
          integer indexoutgoing(nmax) !reporting the order of storage of  
C                                      outgoing momenta
          integer indexingoing(nmax) !reporting the order of storage of  
C                                      ingoing momenta
          integer j3,j4          !loop variable
          integer next        !number of external particles
          integer nfermion    !counting the number of anticommuting particles
          integer nproc(nlb)       !to choose among different processes
          integer noutgoing    !number of outgoing particles
          integer nsourfermion, nsourfermionbar, nsourhiggs,
     >           nsourgaubosons
C
          double precision  internal(nmax)    !array of flags to decide 
C                             wether the particle is internal or external
          double precision momenta      !particles momenta
          double precision  massoutgoing(nmax) !containing the masses of 
C          outcoming particles
          double precision masshiggs(3)             !higgs mass
          double precision massgaubosons(ngaubos)   !zboson mass
          double precision massfermion(3,4)         !fermion masses
          double precision widthhiggs(3)            !higgs width
          double precision widthgaubosons(ngaubos)  !zboson width
          double precision widthfermion(3,4)        !fermion widthes
          double precision spinsour,spinsouraux  
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Containing the common for number and spin of external particles. Shared byC
C the subroutines ITERA, PROCESSO and SPINVARIABLE;                         C
C ITERA, PROCESSO, FILLCONFSPIN AND SPINCOULOR respectively                 C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
         common/source/nsourhiggs(3),nsourgaubosons(ngaubos),
     >                 nsourfermion(3,4),nsourfermionbar(3,4)
         common/spin/spinsour(nmax),spinsouraux(nmax)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C MASSES.COMM                                                               C
C                                                                           C
C Containing the common for particles masses. Shared by the subroutines     C
C ITERA and COUPLINGS                                                       C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          common/masses/masshiggs,massgaubosons,massfermion ,
     >                  widthhiggs,widthgaubosons,widthfermion !in this
C                    common all the  masses of the particles are transferred.
C                                  Wishing to add a new particle change here
CCCCCCCCC
          common/external/next,nfermion  !containing the number of external
C                                        particles from the subroutine ??????
          common/momenta/momenta(nmax,4),internal
          common/integra/massoutgoing,
     >                   noutgoing,indexoutgoing,indexingoing
C
          integer inputinteraction(1000),count3   !to initialize relevant 
C                                                  interaction terms
          common/interactions/inputinteraction,count3 !shared with subroutine
          integer next_old                !old number of esternal particles
C
C
         data flag/0/
c         data indexoutgoing/nmax*0/
c         data indexingoing/nmax*0/
c         data internal/nmax*1./
         data next_old/0/
C                                                       READARRAYTWOBYTWO_H
C
         if (flag.eq.0) call couplings
C
         do j3=1,3
          do j4=1,4
           nsourfermion(j3,j4)=0
           nsourfermionbar(j3,j4)=0
          enddo
         enddo
C
         do j3=1,3
          nsourhiggs(j3)=0
         enddo
C
         do j3=1,ngaubos
          nsourgaubosons(j3)=0
         enddo
C
         do j3=1,nmax
          spinsouraux(j3)=0
          massoutgoing(j3)=0.
         enddo
C
         do j3=1,nmax
          indexingoing(j3)=0
          indexoutgoing(j3)=j3
          internal(j3)=-1.
         enddo        
C
        if(nproc(1).le.4.and.nproc(1).ge.1) then
C
         if(nproc(4).gt.3) nproc(4)=-1
         if(nproc(4).eq.1.and.nproc(1).eq.4) nproc(4)=-1
         if(nproc(4).eq.0.and.nproc(1).gt.1) nproc(4)=-1
         if(nproc(4).lt.0) goto 1
C
         nsourfermion(1,2)=1       ! ingoing D/outgoing DBAR
         nsourfermion(1,3)=1       ! outgoing NUEBAR
         nsourfermion(3,2)=1       ! outgoin BBAR
         nsourfermionbar(1,1)=1    ! ingoing UBAR/outgoing U
         nsourfermionbar(1,4)=1    ! outgoing E^-
         nsourfermionbar(3,2)=1    ! ougoing B
         nsourgaubosons(11)=nproc(4)      ! nproc(4) outgoing/ingoing gluons
         if(nproc(1).eq.1) then
C
C processo  d ubar -->nu_ebar e^- b bbar glu glu 
C ALPHA order 1) D 2) NUEBAR 3) BBAR 4) UBAR 5) E^- 6) B 7) GLU 8) GLU 
C
          massoutgoing(1)=massfermion(1,3)
          massoutgoing(2)=massfermion(3,2)
          massoutgoing(3)=massfermion(1,4)
          massoutgoing(4)=massfermion(3,2)
          massoutgoing(5)=massgaubosons(11)
          massoutgoing(6)=massgaubosons(11)
          massoutgoing(7)=massgaubosons(11)
C
          indexoutgoing(1)=2
          indexoutgoing(2)=3
          indexoutgoing(3)=5
          indexoutgoing(4)=6
          indexoutgoing(5)=7
          indexoutgoing(6)=8
          indexoutgoing(7)=9
          indexingoing(1)=1
          indexingoing(2)=4
          spinsouraux(1)=0.49
          spinsouraux(2)=0.51
          spinsouraux(3)=0.51
          spinsouraux(4)=0.51
          spinsouraux(5)=0.49
          spinsouraux(6)=0.49
          spinsouraux(7)=1.1
          spinsouraux(8)=1.1
          spinsouraux(9)=1.1
          internal(1)=1.
          internal(4)=1.
         elseif(nproc(1).eq.2) then
C
C processo d glu -->nu_ebar e^- b bbar u glu 
C ALPHA order 1) D 2) NUEBAR 3) BBAR 4) U 5) E^- 6) B 7) GLU-in 8) GLU-out 
C
          massoutgoing(1)=massfermion(1,3)
          massoutgoing(2)=massfermion(3,2)
          massoutgoing(3)=massfermion(1,1)
          massoutgoing(4)=massfermion(1,4)
          massoutgoing(5)=massfermion(3,2)
          massoutgoing(6)=massgaubosons(11)
          massoutgoing(7)=massgaubosons(11)
C
          indexoutgoing(1)=2
          indexoutgoing(2)=3
          indexoutgoing(3)=4
          indexoutgoing(4)=5
          indexoutgoing(5)=6
          indexoutgoing(6)=8
          indexoutgoing(7)=9
          indexingoing(1)=1
          indexingoing(2)=7
          spinsouraux(1)=0.49
          spinsouraux(2)=0.51
          spinsouraux(3)=0.51
          spinsouraux(4)=0.49
          spinsouraux(5)=0.49
          spinsouraux(6)=0.49
          spinsouraux(7)=1.1
          spinsouraux(8)=1.1
          spinsouraux(9)=1.1
          internal(1)=1.
          internal(7)=1.
         elseif(nproc(1).eq.3) then
C
C processo ubar glu -->nu_ebar e^- b bbar dbar glu 
C ALPHA order 1) DBAR 2) NUEBAR 3) BBAR 4) UBAR 5) E^- 6) B 7)GLU-in 8)GLU-out 
C
          massoutgoing(1)=massfermion(1,2)
          massoutgoing(2)=massfermion(1,3)
          massoutgoing(3)=massfermion(3,2)
          massoutgoing(4)=massfermion(1,4)
          massoutgoing(5)=massfermion(3,2)
          massoutgoing(6)=massgaubosons(11)
          massoutgoing(7)=massgaubosons(11)
C
          indexoutgoing(1)=1
          indexoutgoing(2)=2
          indexoutgoing(3)=3
          indexoutgoing(4)=5
          indexoutgoing(5)=6
          indexoutgoing(6)=8
          indexoutgoing(7)=9
          indexingoing(1)=4
          indexingoing(2)=7
          spinsouraux(1)=0.51
          spinsouraux(2)=0.51
          spinsouraux(3)=0.51
          spinsouraux(4)=0.51
          spinsouraux(5)=0.49
          spinsouraux(6)=0.49
          spinsouraux(7)=1.1
          spinsouraux(8)=1.1
          spinsouraux(9)=1.1
          internal(4)=1.
          internal(7)=1.
         elseif(nproc(1).eq.4) then
C
C processo glu glu -->nu_ebar e^- b bbar u dbar glu 
C ALPHA order 1)DBAR 2)NUEBAR 3) BBAR 4) U 5) E^- 6) B 7)GLU-in 8)GLU-in 
C
          massoutgoing(1)=massfermion(1,2)
          massoutgoing(2)=massfermion(1,3)
          massoutgoing(3)=massfermion(3,2)
          massoutgoing(4)=massfermion(1,1)
          massoutgoing(5)=massfermion(1,4)
          massoutgoing(6)=massfermion(3,2)
          massoutgoing(7)=massgaubosons(11)
C
          indexoutgoing(1)=1
          indexoutgoing(2)=2
          indexoutgoing(3)=3
          indexoutgoing(4)=4
          indexoutgoing(5)=5
          indexoutgoing(6)=6
          indexoutgoing(7)=9
          indexingoing(1)=7
          indexingoing(2)=8
          spinsouraux(1)=0.51
          spinsouraux(2)=0.51
          spinsouraux(3)=0.51
          spinsouraux(4)=0.49
          spinsouraux(5)=0.49
          spinsouraux(6)=0.49
          spinsouraux(7)=1.1
          spinsouraux(8)=1.1
          spinsouraux(9)=1.1
          internal(7)=1.
          internal(8)=1.
         endif
C
         nfermion=6
         next=6+nproc(4)
         noutgoing=4+nproc(4)
C
C
C  Initializing relevant interaction terms
C
         count3=0
         inputinteraction(1)=7    !                      Number of V-A interaction
         inputinteraction(2)=11
         inputinteraction(3)=1
         inputinteraction(4)=1
         inputinteraction(5)=1
         inputinteraction(6)=1    !                     glu ubar u
         inputinteraction(7)=11
         inputinteraction(8)=1
         inputinteraction(9)=2
         inputinteraction(10)=1
         inputinteraction(11)=2   !                     glu dbar d
         inputinteraction(12)=11
         inputinteraction(13)=3
         inputinteraction(14)=2
         inputinteraction(15)=3
         inputinteraction(16)=2   !                     glu bbar b  
         inputinteraction(17)=2
         inputinteraction(18)=1
         inputinteraction(19)=1
         inputinteraction(20)=1
         inputinteraction(21)=2   !                     w  ubar d
         inputinteraction(22)=3
         inputinteraction(23)=1
         inputinteraction(24)=2
         inputinteraction(25)=1
         inputinteraction(26)=1   !                     w dbar u
         inputinteraction(27)=2  
         inputinteraction(28)=1 
         inputinteraction(29)=3  
         inputinteraction(30)=1  
         inputinteraction(31)=4   !                     w nubar e  
         inputinteraction(32)=3  
         inputinteraction(33)=1  
         inputinteraction(34)=4  
         inputinteraction(35)=1  
         inputinteraction(36)=3  !                      w ebar nu  
         inputinteraction(37)=0  !                      number of Yukawa interaction
         inputinteraction(38)=2 !                       number of selfgauge
         inputinteraction(39)=12
         inputinteraction(40)=11
         inputinteraction(41)=11  !                     aux glu glu
         inputinteraction(42)=11
         inputinteraction(43)=11
         inputinteraction(44)=11  !                     glu glu glu 
         inputinteraction(45)=0 !                        number of Gauge-higgs couplings
c         inputinteraction(38)=1 !                       number of selfgauge
c         inputinteraction(39)=11
c         inputinteraction(40)=11
c         inputinteraction(41)=11  !                     aux glu glu
c         inputinteraction(42)=0 !                        number of Gauge-higgs couplings
C
        elseif(nproc(1).le.2000.and.nproc(1).ge.1001) then
C

C
         nsourfermion(1,2)=1       ! ingoing D/outgoing DBAR
         nsourfermion(1,3)=1       ! outgoing NUEBAR
         nsourfermion(2,1)=1       ! outgoin CBAR
         nsourfermion(3,2)=1       ! outgoin BBAR
         nsourfermionbar(1,1)=1    ! ingoing UBAR/outgoing U
         nsourfermionbar(1,4)=1    ! outgoing E^-
         nsourfermionbar(2,1)=1    ! ougoing C
         nsourfermionbar(3,2)=1    ! ougoing B
         nsourgaubosons(11)=nproc(4)      ! nproc(4) outgoing/ingoing gluons
C
         if(nproc(1).eq.1001) then
C
C processo  d ubar -->nu_ebar e^- b bbar c cbar (n glu) 
C ALPHA order 1) D 2) NUEBAR 3) CBAR 4) BBAR 5) UBAR 6) E^- 7) C 8) B ... GLU
C
          massoutgoing(1)=massfermion(1,3)
          massoutgoing(2)=massfermion(2,1)
          massoutgoing(3)=massfermion(3,2)
          massoutgoing(4)=massfermion(1,4)
          massoutgoing(5)=massfermion(2,1)
          massoutgoing(6)=massfermion(3,2)
          massoutgoing(7)=massgaubosons(11)
C
          indexoutgoing(1)=2
          indexoutgoing(2)=3
          indexoutgoing(3)=4
          indexoutgoing(4)=6
          indexoutgoing(5)=7
          indexoutgoing(6)=8
          indexoutgoing(7)=9
          indexingoing(1)=1
          indexingoing(2)=5
          spinsouraux(1)=0.49
          spinsouraux(2)=0.51
          spinsouraux(3)=0.51
          spinsouraux(4)=0.51
          spinsouraux(5)=0.51
          spinsouraux(6)=0.49
          spinsouraux(7)=0.49
          spinsouraux(8)=0.49
          spinsouraux(9)=1.1
          internal(1)=1.
          internal(5)=1.
C
         elseif(nproc(1).eq.1002) then
C
C processo  d cbar -->nu_ebar e^- b bbar u cbar (n glu) 
C ALPHA order 1) D 2) NUEBAR 3) CBAR-out 4) BBAR 5) U 6) E^- 7) CBAR-in 
C                                                            8) B ... GLU
C
          massoutgoing(1)=massfermion(1,3)
          massoutgoing(2)=massfermion(2,1)
          massoutgoing(3)=massfermion(3,2)
          massoutgoing(4)=massfermion(1,1)
          massoutgoing(5)=massfermion(1,4)
          massoutgoing(6)=massfermion(3,2)
          massoutgoing(7)=massgaubosons(11)
C
          indexoutgoing(1)=2
          indexoutgoing(2)=3
          indexoutgoing(3)=4
          indexoutgoing(4)=5
          indexoutgoing(5)=6
          indexoutgoing(6)=8
          indexoutgoing(7)=9
          indexingoing(1)=1
          indexingoing(2)=7
          spinsouraux(1)=0.49
          spinsouraux(2)=0.51
          spinsouraux(3)=0.51
          spinsouraux(4)=0.51
          spinsouraux(5)=0.49
          spinsouraux(6)=0.49
          spinsouraux(7)=0.51
          spinsouraux(8)=0.49
          spinsouraux(9)=1.1
          internal(1)=1.
          internal(7)=1.
C
         elseif(nproc(1).eq.1003) then
C
C processo  c cbar -->nu_ebar e^- b bbar u dbar (n glu) 
C ALPHA order 1) DBAR 2) NUEBAR 3) C 4) BBAR 5) U 6) E^- 7) CBAR 
C                                                            8) B ... GLU
C
          massoutgoing(1)=massfermion(1,2)
          massoutgoing(2)=massfermion(1,3)
          massoutgoing(3)=massfermion(3,2)
          massoutgoing(4)=massfermion(1,1)
          massoutgoing(5)=massfermion(1,4)
          massoutgoing(6)=massfermion(3,2)
          massoutgoing(7)=massgaubosons(11)
C
          indexoutgoing(1)=1
          indexoutgoing(2)=2
          indexoutgoing(3)=4
          indexoutgoing(4)=5
          indexoutgoing(5)=6
          indexoutgoing(6)=8
          indexoutgoing(7)=9
          indexingoing(1)=3
          indexingoing(2)=7
          spinsouraux(1)=0.51
          spinsouraux(2)=0.51
          spinsouraux(3)=0.49
          spinsouraux(4)=0.51
          spinsouraux(5)=0.49
          spinsouraux(6)=0.49
          spinsouraux(7)=0.51
          spinsouraux(8)=0.49
          spinsouraux(9)=1.1
          internal(3)=1.
          internal(7)=1.
C
         elseif(nproc(1).eq.1101) then
C
C processo  d glu -->nu_ebar e^- b bbar u c cbar (n glu) 
C ALPHA order 1) D 2) NUEBAR 3) CBAR 4) BBAR 5) U 6) E^- 7) C 
C                                                            8) B ... GLU
C
          massoutgoing(1)=massfermion(1,3)
          massoutgoing(2)=massfermion(2,1)
          massoutgoing(3)=massfermion(3,2)
          massoutgoing(4)=massfermion(1,1)
          massoutgoing(5)=massfermion(1,4)
          massoutgoing(6)=massfermion(2,1)
          massoutgoing(7)=massfermion(3,2)
C
          indexoutgoing(1)=2
          indexoutgoing(2)=3
          indexoutgoing(3)=4
          indexoutgoing(4)=5
          indexoutgoing(5)=6
          indexoutgoing(6)=7
          indexoutgoing(7)=8
          indexingoing(1)=1
          indexingoing(2)=9
          spinsouraux(1)=0.49
          spinsouraux(2)=0.51
          spinsouraux(3)=0.51
          spinsouraux(4)=0.51
          spinsouraux(5)=0.49
          spinsouraux(6)=0.49
          spinsouraux(7)=0.49
          spinsouraux(8)=0.49
          spinsouraux(9)=1.1
          internal(1)=1.
          internal(9)=1.
C
         endif
C
         nfermion=8
         next=8+nproc(4)
         noutgoing=6+nproc(4)
C
C
C  Initializing relevant interaction terms
C
         count3=0
         inputinteraction(1)=8    !                      Number of V-A interaction
         inputinteraction(2)=11
         inputinteraction(3)=1
         inputinteraction(4)=1
         inputinteraction(5)=1
         inputinteraction(6)=1    !                     glu ubar u
         inputinteraction(7)=11
         inputinteraction(8)=1
         inputinteraction(9)=2
         inputinteraction(10)=1
         inputinteraction(11)=2   !                     glu dbar d
         inputinteraction(12)=11
         inputinteraction(13)=3
         inputinteraction(14)=2
         inputinteraction(15)=3
         inputinteraction(16)=2   !                     glu bbar b  
         inputinteraction(17)=2
         inputinteraction(18)=1
         inputinteraction(19)=1
         inputinteraction(20)=1
         inputinteraction(21)=2   !                     w  ubar d
         inputinteraction(22)=3
         inputinteraction(23)=1
         inputinteraction(24)=2
         inputinteraction(25)=1
         inputinteraction(26)=1   !                     w dbar u
         inputinteraction(27)=2  
         inputinteraction(28)=1 
         inputinteraction(29)=3  
         inputinteraction(30)=1  
         inputinteraction(31)=4   !                     w nubar e  
         inputinteraction(32)=3  
         inputinteraction(33)=1  
         inputinteraction(34)=4  
         inputinteraction(35)=1  
         inputinteraction(36)=3  !                      w ebar nu  
         inputinteraction(37)=11
         inputinteraction(38)=2
         inputinteraction(39)=1
         inputinteraction(40)=2
         inputinteraction(41)=1   !                     glu cbar c  
         inputinteraction(42)=0  !                      number of Yukawa interaction
         inputinteraction(43)=2 !                       number of selfgauge
         inputinteraction(44)=12
         inputinteraction(45)=11
         inputinteraction(46)=11  !                     aux glu glu
         inputinteraction(47)=11
         inputinteraction(48)=11
         inputinteraction(49)=11  !                     glu glu glu 
         inputinteraction(50)=0 !                        number of Gauge-higgs couplings
c         inputinteraction(38)=1 !                       number of selfgauge
c         inputinteraction(39)=11
c         inputinteraction(40)=11
c         inputinteraction(41)=11  !                     aux glu glu
c         inputinteraction(42)=0 !                        number of Gauge-higgs couplings
C
        endif
C
        if(nproc(1).eq.-11) then
C
         nsourfermion(1,1)=1       ! ingoing U/outgoing UBAR
         nsourfermionbar(1,1)=1    ! ingoing UBAR/outgoing U
         nsourgaubosons(11)=5      ! 5 ingoing/outgoing gluons
C
C processo u ubar  -->glu glu glu glu glu 
C ALPHA order 1) U 2) UBAR 3) GLU 4) GLU 5) GLU 6) GLU 7) GLU
C
          massoutgoing(1)=massgaubosons(11)
          massoutgoing(2)=massgaubosons(11)
          massoutgoing(3)=massgaubosons(11)
          massoutgoing(4)=massgaubosons(11)
          massoutgoing(5)=massgaubosons(11)
C
          indexoutgoing(1)=3
          indexoutgoing(2)=4
          indexoutgoing(3)=5
          indexoutgoing(4)=6
          indexoutgoing(5)=7
          indexingoing(1)=1
          indexingoing(2)=2
          spinsouraux(1)=0.49
          spinsouraux(2)=0.51
          spinsouraux(3)=1.1
          spinsouraux(4)=1.1
          spinsouraux(5)=1.1
          spinsouraux(6)=1.1
          spinsouraux(7)=1.1
          internal(1)=1.
          internal(2)=1.
C
         nfermion=2
         next=7
         noutgoing=5
C
C
C  Initializing relevant interaction terms
C
         count3=0
         inputinteraction(1)=1   !                      Number of V-A interaction
         inputinteraction(2)=11
         inputinteraction(3)=1
         inputinteraction(4)=1
         inputinteraction(5)=1
         inputinteraction(6)=1    !                     glu ubar u
         inputinteraction(7)=0  !                      number of Yukawa interaction
         inputinteraction(8)=2 !                       number of selfgauge
         inputinteraction(9)=12
         inputinteraction(10)=11
         inputinteraction(11)=11  !                     aux glu glu
         inputinteraction(12)=11
         inputinteraction(13)=11
         inputinteraction(14)=11  !                     glu glu glu 
         inputinteraction(15)=0 !                        number of Gauge-higgs couplings
        endif
C
        if(nproc(1).eq.-21) then
C
         nsourgaubosons(11)=10      ! 10 ingoing/outgoing gluons
C
C processo glu glu  -->glu glu glu glu glu glu glu glu 
C
          massoutgoing(1)=massgaubosons(11)
          massoutgoing(2)=massgaubosons(11)
          massoutgoing(3)=massgaubosons(11)
          massoutgoing(4)=massgaubosons(11)
          massoutgoing(5)=massgaubosons(11)
          massoutgoing(6)=massgaubosons(11)
          massoutgoing(7)=massgaubosons(11)
          massoutgoing(8)=massgaubosons(11)
C
          indexoutgoing(1)=3
          indexoutgoing(2)=4
          indexoutgoing(3)=5
          indexoutgoing(4)=6
          indexoutgoing(5)=7
          indexoutgoing(6)=8
          indexoutgoing(7)=9
          indexoutgoing(8)=10
          indexingoing(1)=1
          indexingoing(2)=2
          spinsouraux(1)=1.1
          spinsouraux(2)=1.1
          spinsouraux(3)=1.1
          spinsouraux(4)=1.1
          spinsouraux(5)=1.1
          spinsouraux(6)=1.1
          spinsouraux(7)=1.1
          spinsouraux(8)=1.1
          spinsouraux(9)=1.1
          spinsouraux(10)=1.1
          internal(1)=1.
          internal(2)=1.
C
         nfermion=0
         next=10
         noutgoing=8
C
C
C  Initializing relevant interaction terms
C
         count3=0
         inputinteraction(1)=0   !                      Number of V-A interaction
         inputinteraction(2)=0  !                      number of Yukawa interaction
         inputinteraction(3)=2 !                       number of selfgauge
         inputinteraction(4)=12
         inputinteraction(5)=11
         inputinteraction(6)=11  !                     aux glu glu
         inputinteraction(7)=11
         inputinteraction(8)=11
         inputinteraction(9)=11  !                     glu glu glu 
         inputinteraction(10)=0 !                        number of Gauge-higgs couplings
        endif
C
        if(nproc(1).eq.-22) then
C
         nsourgaubosons(11)=4      ! 4 ingoing/outgoing gluons
C
C processo glu glu  -->glu glu glu glu glu glu glu glu 
C
          massoutgoing(1)=massgaubosons(11)
          massoutgoing(2)=massgaubosons(11)
          massoutgoing(3)=massgaubosons(11)
          massoutgoing(4)=massgaubosons(11)
          massoutgoing(5)=massgaubosons(11)
          massoutgoing(6)=massgaubosons(11)
          massoutgoing(7)=massgaubosons(11)
          massoutgoing(8)=massgaubosons(11)
C
          indexoutgoing(1)=3
          indexoutgoing(2)=4
          spinsouraux(1)=1.1
          spinsouraux(2)=1.1
          spinsouraux(3)=1.1
          spinsouraux(4)=1.1
          indexingoing(1)=1
          indexingoing(2)=2
          internal(1)=1.
          internal(2)=1.
C
         nfermion=0
         next=4
         noutgoing=2
C
C
C  Initializing relevant interaction terms
C
         count3=0
         inputinteraction(1)=0   !                      Number of V-A interaction
         inputinteraction(2)=0  !                      number of Yukawa interaction
         inputinteraction(3)=2 !                       number of selfgauge
         inputinteraction(4)=12
         inputinteraction(5)=11
         inputinteraction(6)=11  !                     aux glu glu
         inputinteraction(7)=11
         inputinteraction(8)=11
         inputinteraction(9)=11  !                     glu glu glu 
         inputinteraction(10)=0 !                        number of Gauge-higgs couplings
        endif
C
C common environment
C
         do j3=1,next
          spinsour(j3)=spinsouraux(j3)
         enddo
C
         if(next.ne.next_old) flag=0
         next_old=next
C
         if(flag.eq.0) then    !initialization step for coupling constants
          flag=1               !and to define product among momentum configurations
          call configuration
          call initcompconf
         endif
C
         return
C
 1       write(6,*)'wrong specification of NPROC for the process'
         write(6,*) nproc
         stop 
C
         return
         end







C***********************************************************************
          subroutine processo
C***********************************************************************
          implicit none
C
          integer ngaubos
          parameter (ngaubos=14)
          integer nmax        !maximum number of external particles, 
          parameter (nmax=10)          
          integer flag
          integer indexoutgoing(nmax) !reporting the order of storage of  
C                                      outgoing momenta
          integer indexingoing(nmax)  !reporting the order of storage of  
C                                      outgoing momenta
          integer j1,j2,j3,j4         !loop variable
          integer naux        !auxiliar counter
          integer next        !number of external particles
          integer nfermion    !counting the number of anticommuting particles
          integer noutgoing   !number of outgoing particles
          integer nsourfermion, nsourfermionbar, nsourhiggs,
     >            nsourgaubosons
C
          double precision  internal(nmax)    !array of flags to decide 
C                                              wether the particle is
C                                              internal or external
          double precision momenta            !particles momenta
          double precision massoutgoing(nmax) !containing the masses of 
C                                              outcoming particles
          double precision masshiggs(3)       !higgs mass
          double precision massgaubosons(ngaubos)   !zboson mass
          double precision massfermion(3,4)         !fermion masses
          double precision widthhiggs(3)            !higgs width
          double precision widthgaubosons(ngaubos)  !zboson width
          double precision widthfermion(3,4)        !fermion widthes
          double precision spinsour, spinsouraux  
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C Containing the common for number and spin of external particles.          C
C Shared by the                                                             C
C subroutines ITERA, PROCESSO and SPINVARIABLE;                             C
C ITERA, PROCESSO, FILLCONFSPIN AND SPINCOULOR respectively                 C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
         common/source/nsourhiggs(3),nsourgaubosons(ngaubos),
     >                 nsourfermion(3,4),nsourfermionbar(3,4)
         common/spin/spinsour(nmax),spinsouraux(nmax)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C MASSES.COMM                                                               C
C                                                                           C
C Containing the common for particles masses. Shared by the subroutines     C
C ITERA and COUPLINGS                                                       C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          common/masses/masshiggs,massgaubosons,massfermion ,
     >                  widthhiggs,widthgaubosons,widthfermion !in this
C             common all the  masses of the particles are transferred.
C                     Wishing to add a new particle add here
CCCCCCCCC
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          common/momenta/momenta(nmax,4),internal
          common/integra/massoutgoing,
     >                       noutgoing,indexoutgoing,indexingoing
          data flag/0/
c          data indexingoing/nmax*0/
c          data indexoutgoing/nmax*0/
c          data internal/nmax*1/
C
         save flag
         if (flag.eq.0) then
          call couplings
         else
          write(6,*)'more than one call to PROCESSO?'
          stop
         endif
         next=0
C
         open(10,file='processo.dat',status='old')
         do j3=1,3
          do j4=1,4
C
           read(10,*)nsourfermion(j3,j4)
C
           if(nsourfermion(j3,j4).ne.0) then
            do j1=next+1,next+nsourfermion(j3,j4)
c             do j2=1,4
c              read(10,*) momenta(j1,j2)             
c             enddo
             spinsour(j1)=0.49
            enddo
            next=nsourfermion(j3,j4)+next
           endif
C
           read(10,*)naux
           nsourfermion(j3,j4)=naux+nsourfermion(j3,j4)
C
           if(naux.ne.0) then
            do j1=next+1,next+naux
             internal(j1)=-1.
             spinsour(j1)=0.51
             indexoutgoing(j1)=j1
             massoutgoing(j1)=massfermion(j3,j4)
            enddo
            next=naux+next
           endif
          enddo
         enddo
C
         do j3=1,3
          do j4=1,4
C
           read(10,*)nsourfermionbar(j3,j4)
C
           if(nsourfermionbar(j3,j4).ne.0) then
            do j1=next+1,next+nsourfermionbar(j3,j4)
             internal(j1)=-1.
             spinsour(j1)=0.49
             indexoutgoing(j1)=j1
             massoutgoing(j1)=massfermion(j3,j4)
            enddo
           next=nsourfermionbar(j3,j4)+next
           endif
C
           read(10,*)naux
           nsourfermionbar(j3,j4)=naux+nsourfermionbar(j3,j4)
C
           if(naux.ne.0) then
            do j1=next+1,next+naux
c             do j2=1,4
c              read(10,*)momenta(j1,j2)
c             enddo
             spinsour(j1)=0.51
            enddo
            next=naux+next
           endif
C
          enddo
         enddo
C
           read(10,*)nsourhiggs(1)
           nsourhiggs(2)=0
           nsourhiggs(3)=0
C
           if(nsourhiggs(1).ne.0) then
            do j1=next+1,next+nsourhiggs(1)
c             do j2=1,4
c              read(10,*)momenta(j1,j2)
c             enddo
             spinsour(j1)=0.
            enddo
            next=nsourhiggs(1)+next
           endif
C
           read(10,*)naux
C
           if(naux.ne.0) then
            do j1=next+1,next+naux
             internal(j1)=-1.
             spinsour(j1)=0.
             indexoutgoing(j1)=j1
             massoutgoing(j1)=masshiggs(1)
            enddo
            next=naux+next
           endif
           nsourhiggs(1)=nsourhiggs(1)+naux
C
         do j3=1,ngaubos
          if(j3.gt.3.and.j3.lt.10.or.j3.gt.11) then
           nsourgaubosons(j3)=0
          else
            read(10,*)nsourgaubosons(j3)
C
            if(nsourgaubosons(j3).ne.0) then
             do j1=next+1,next+nsourgaubosons(j3)
c              do j2=1,4
c               read(10,*)momenta(j1,j2)
c              enddo
              spinsour(j1)=1.+dfloat(j3)*1.d-2
             enddo
             next=nsourgaubosons(j3)+next
            endif
C
            read(10,*)naux
C
            if(naux.ne.0) then
             do j1=next+1,next+naux
             internal(j1)=-1.
              spinsour(j1)=1.+dfloat(j3)*1.d-2
              indexoutgoing(j1)=j1
              massoutgoing(j1)=massgaubosons(j3)
             enddo
             next=naux+next
            endif
             nsourgaubosons(j3)=nsourgaubosons(j3)+naux
           endif
          enddo
C
         close(10)
C
           nfermion=0
           do j1=1,3
            do j2=1,4
             nfermion=nsourfermion(j1,j2)+
     >                nsourfermionbar(j1,j2)+nfermion
            enddo
           enddo
C
c           do j1=1,4
c            initialmomentum(j1)=0.
c           enddo
C
           if(flag.eq.0) then    !initialization step for coupling constants
            flag=1               !and to define product among 
C                                 momentum configurations
            call configuration
            call initcompconf
           endif
C
           noutgoing=next
           do j1=1,next
            if (indexoutgoing(j1).eq.0) then
c             do j2=1,4
c              initialmomentum(j2)=initialmomentum(j2)+momenta(j1,j2)
c             enddo 
             noutgoing=noutgoing-1
c             write(6,*) noutgoing,next,'n'
            endif           
           enddo
C
          do j3=1,next-noutgoing
           j2=1
           do j1=1,next
            if (indexoutgoing(j1).eq.0) then
             indexingoing(j2)=j1
             j2=j2+1
            endif
           enddo
           do j1=1,next-1
            if (indexoutgoing(j1).eq.0) then
             do j2=j1,next-1
              indexoutgoing(j2)=indexoutgoing(j2+1)
              massoutgoing(j2)=massoutgoing(j2+1)
             enddo
            endif
           enddo
          enddo
C
           do j1=1,next
            spinsouraux(j1)=spinsour(j1)
           enddo
C
c          call initcolore(color)
C
           return
           end
C****************************************************************************
         subroutine initcolore(color)
C****************************************************************************
C
C
C
         integer nmax
         parameter (nmax=10)      !maximum number of external particles. 
         integer color(2*nmax),color1(2*nmax),j1,flag
         data flag/0/
C
         if (flag.eq.0) then
          open (1,file='colore.dat',status='old')
           do j1=1,nmax*2
            read(1,*)color1(j1)
           enddo
          close(1)
          flag=1
         endif
         do j1=1,2*nmax
          color(j1)=color1(j1)
         enddo
C
         return
         end                                                                 
C**************************************************************************** 
         subroutine initcolore4q(ant1,ant2,nfer)
C****************************************************************************
C
C  NFER number of fermions in the process (including coulorless object)
C       used for internal consistency checks. this feature might not work
C       once majorana fermion are included.    
C
C        Color assignment for qqqq ngluons
C  ANT1(1) particle number (as from FOR010.DAT) of the quark of the first antenna
C  ANT1(2) particle number (as from FOR010.DAT) of the antiquark of the first antenna
C  ANT1(3) number of gluons of the first antenna
C  ANT1(3+j) particle number (as from FOR010.DAT) of the j-th gluon of the first antenna 
C  the structure of ANT2 is entirely analogous
C
         integer nmax
         parameter (nmax=10)      !maximum number of external particles. 
         integer color(2*nmax),ant1(nmax+2),ant2(nmax+2),nfer,j1
         common/colore/color
C
         if(ant1(1).le.nfer/2.or.ant1(2).gt.nfer/2.or
     >              .ant2(1).le.nfer/2.or.ant2(2).gt.nfer/2 ) then
          write(6,*)'input error in INITCOLORE'
          write(6,*)' wrong antenna assignement'
          stop
         endif
C
         do j1=1,2*nmax
          color(j1)=0
         enddo
         color(2*ant1(1)-1)=0
         color(2*ant1(1))=ant1(3)+1
         color(2*ant1(2)-1)=1
         color(2*ant1(2))=0
         do j1=1,ant1(3)
          color(2*ant1(j1+3)-1)= ant1(3)-j1+2
          color(2*ant1(j1+3))= ant1(3)-j1+1
         enddo
         color(2*ant2(1)-1)=0
         color(2*ant2(1))=ant1(3)+2+ant2(3)
         color(2*ant2(2)-1)=ant1(3)+2
         color(2*ant2(2))=0
         do j1=1,ant2(3)
          color(2*ant2(j1+3)-1)= ant2(3)+ant1(3)-j1+3
          color(2*ant2(j1+3))= ant2(3)+ant1(3)-j1+2
         enddo
C
         return
         end
C**************************************************************************** 
         subroutine initcolore2q(ant1,nfer)
C****************************************************************************
C
C  NFER number of fermions in the process (including coulorless object)
C       used for internal consistency checks. this feature might not work
C       once majorana fermion are included.    
C
C        Color assignment for qq ngluons
C  ANT1(1) particle number (as from FOR010.DAT) of the quark of the first antenna
C  ANT1(2) particle number (as from FOR010.DAT) of the antiquark of the first antenna
C  ANT1(3) number of gluons of the first antenna
C  ANT1(3+j) particle number (as from FOR010.DAT) of the j-th gluon of the first antenna 
C
         integer nmax
         parameter (nmax=10)      !maximum number of external particles. 
         integer color(2*nmax),ant1(nmax+1),nfer,j1
         common/colore/color
C
         if(ant1(1).le.nfer/2.or.ant1(2).gt.nfer/2 ) then
          write(6,*)'input error in INITCOLORE'
          write(6,*)' wrong antenna assignement'
          write(6,*) ant1,'    ',nfer
          stop
         endif
C
         do j1=1,2*nmax
          color(j1)=0
         enddo
         color(2*ant1(1)-1)=0
         color(2*ant1(1))=ant1(3)+1
         color(2*ant1(2)-1)=1
         color(2*ant1(2))=0
         do j1=1,ant1(3)
          color(2*ant1(j1+3)-1)= ant1(3)-j1+2
          color(2*ant1(j1+3))= ant1(3)-j1+1
         enddo
C
         return
         end                                                                 
C****************************************************************************
         subroutine spinvariable(spinconf,nconfspin)
C****************************************************************************
C
C After the execution of this subroutine the array SPINSOUR will contain
C the spin of the particles chosen randomly according to the values
C of 0 <= RANDNUMB(J) <= 1 .
C
         implicit none
C
         integer ngaubos
         integer nmax        !maximum number of external particles, 
         integer nspinmax    !maximum number of spinconfiguration, 
         parameter (ngaubos=14)
         parameter (nmax=10)          
         parameter (nspinmax=2**nmax)          
         integer l1,l2,l3
         integer nconfspin
         integer nsourfermion, nsourfermionbar, nsourhiggs,
     >           nsourgaubosons 
C
         double precision dummy,max,dummyspin(nmax)
         double precision inte
         double precision lattice(nspinmax)
         double precision partinteg(nspinmax)
         double precision spinconf(nmax,nspinmax)
         double precision wei(nspinmax)
C
         common/rescal/wei,lattice
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                             C
C Containing the common for number external particles. Shared by the       C
C subroutines ITERA, PROCESSO and SPINVARIABLE; 
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
         common/source/nsourhiggs(3),nsourgaubosons(ngaubos),
     >                 nsourfermion(3,4),nsourfermionbar(3,4)
C
         open(41,file='spinconf.dat',status='old')
         read (41,*)nconfspin
         do l1=1,nconfspin
          read(41,*)(spinconf(l2,l1),l2=1,nmax)
         enddo
         close(41)
C
         open(41,file='spinconf_old.dat',status='unknown')
         write (41,*)nconfspin
         do l1=1,nconfspin
          write(41,*)(spinconf(l2,l1),l2=1,nmax)
         enddo
         close(41)
C
         inte=0.
         open(31,file='spinbin.dat',status='old')
         do l1=1,nconfspin
          read(31,*)partinteg(l1)
          inte=inte+partinteg(l1)
         enddo
         close(31)
C
         open(31,file='spinbin_old.dat',status='unknown')
         do l1=1,nconfspin
          write(31,*)partinteg(l1)
          inte=inte+partinteg(l1)
         enddo
         close(31)
C
         do l1=1,nconfspin-1
          max=partinteg(l1)
          do l2=l1+1,nconfspin
           if (partinteg(l2).gt.max)then
            max=partinteg(l2)
            dummy=partinteg(l1)
            partinteg(l1)=partinteg(l2)
            partinteg(l2)=dummy
            do l3=1,nmax
             dummyspin(l3)=spinconf(l3,l1)
             spinconf(l3,l1)=spinconf(l3,l2)
             spinconf(l3,l2)=dummyspin(l3)
            enddo
           endif
          enddo
         enddo
C
         open(41,file='spinconf.dat',status='old')
         write(41,*)nconfspin
         do l1=1,nconfspin
          write(41,*)(spinconf(l2,l1),l2=1,nmax)
         enddo
         close(41)
C
         return
         end
C****************************************************************************
         subroutine fillconfspin(label,spinconf)
C****************************************************************************
C
C After the execution of this subroutine the array SPINSOUR will contain
C the spin of the particles chosen randomly according to the values
C of 0 <= RANDNUMB(J) <= 1 .
C
         implicit none
C
         integer nmax        !maximum number of external particles,
         integer nspinmax     !maximum number of spin configuration, 
         parameter (nmax=10)          
         parameter (nspinmax=2**nmax)          
         integer j1,l1,l2,l3,l4,l5,l6,l7,l8,l9,l10       !     ,l11,l12
         integer j(nmax)
         integer label
         integer loop(nmax)
         data loop/nmax*1/
         integer next,nfermion
         common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
C
         double precision vecspin(nmax,3)
         double precision spinconf(nmax,nspinmax)
         double precision spinsour,spinsouraux
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                             C
C Containing the common for spin of external particles. Shared by the       C
C subroutines ITERA, PROCESSO, FILLCONFSPIN AND SPINCOULOR 
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
         common/spin/spinsour(nmax),spinsouraux(nmax)
         do j1=1,next
C                                 if more than one coulor tensor contribute
          if (nint(spinsouraux(j1)).eq.0) then
           loop(j1)=1
           vecspin(j1,1)=0.
          endif
          if (spinsouraux(j1).gt.0.4) then                !fermion
           loop(j1)=2
           vecspin(j1,1)=0.49
           vecspin(j1,2)=-0.49
          endif
          if (spinsouraux(j1).gt.0.5) then
           loop(j1)=2
           vecspin(j1,1)=0.51
           vecspin(j1,2)=-0.51
          endif
          if (spinsouraux(j1).gt.1.) then               !massive vector boson
           loop(j1)=3
           vecspin(j1,1)=1.
           vecspin(j1,2)=0.
           vecspin(j1,3)=-1.
          endif
          if (spinsouraux(j1).gt.1.05) then              !massless vector boson
           loop(j1)=2
           vecspin(j1,1)=1.
           vecspin(j1,2)=-1.
          endif
C
         enddo
C
         label=0
         do l1=1,loop(1)
          do l2=1,loop(2)
           do l3=1,loop(3)
            do l4=1,loop(4)
             do l5=1,loop(5)
              do l6=1,loop(6)
               do l7=1,loop(7)
                do l8=1,loop(8)
                 do l9=1,loop(9)
                  do l10=1,loop(10)
c                   do l11=1,loop(11)
c                    do l12=1,loop(12)
                     label=label+1
                     j(1)=l1
                     j(2)=l2
                     j(3)=l3
                     j(4)=l4
                     j(5)=l5
                     j(6)=l6
                     j(7)=l7
                     j(8)=l8
                     j(9)=l9
                     j(10)=l10
c                     j(11)=l11
c                     j(12)=l12
                     do j1=1,nmax
                      spinconf(j1,label)=vecspin(j1,j(j1))
                     enddo
c                    enddo
c                   enddo
                  enddo
                 enddo
                enddo
               enddo
              enddo
             enddo
            enddo
           enddo
          enddo
         enddo
C
c         open(41,file='spinconf.dat',status='unknown')
c         write (41,*)label
c         do j1=1,label
c          write (41,*)(spinconf(l1,j1),l1=1,nmax)
c         enddo
c         close(41)
C
         return
         end
C****************************************************************************
         subroutine spincoulor (randnumb,nspinconf,label)
C****************************************************************************
C
C After the execution of this subroutine the array SPINSOUR will contain
C the spin of the particles chosen randomly according to the values
C of 0 <= RANDNUMB(J) <= 1 .
C
         implicit none
C
         integer nmax        !maximum number of external particles,
         integer nspinmax    !maximum number of spin configuration, 
         parameter (nmax=10)          
         parameter (nspinmax=2**nmax)          
         integer j1
         integer label
         integer nspinconf
         integer spinflag
         integer spinflag1
C
         double precision  randnumb
         double precision  spinconf(nmax,nspinmax)
         double precision  spinsour, spinsouraux  
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                             C
C Containing the common for spin of external particles. Shared by the       C
C subroutines ITERA, PROCESSO, FILLCONFSPIN AND SPINCOULOR 
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
         common/spin/spinsour(nmax),spinsouraux(nmax)
         data spinflag/0/
C
C Static variables
C
         save spinflag,spinconf  
C
C 
C
         if (spinflag.eq.0)then
          spinflag=1
c          write(6,*)'do you want me to try to define a "clever" spin'
c          write(6,*)'function? (0=no, 1=yes)'
c          read (5,*)spinflag1
           spinflag1=0
          if (spinflag1.eq.0)then
           call fillconfspin(nspinconf,spinconf)
          else
           call spinvariable(spinconf,nspinconf)
          endif
         endif
C
         label=int(randnumb*float(nspinconf))+1
         if (label.gt.nspinconf)label=nspinconf
C
         do j1=1,nmax
          spinsour(j1)=spinconf(j1,label)
         enddo
C
         return
         end
C****************************************************************************
         subroutine spincoulor_h (hel)
C****************************************************************************
C
C After the execution of this subroutine the array SPINSOUR will contain
C the spin of the particles chosen randomly according to the values
C of 0 <= RANDNUMB(J) <= 1 .
C
         implicit none
C
         integer nmax        !maximum number of external particles,
         parameter (nmax=10)          
         integer hel(nmax)
         integer j1,j2
         double precision  spinsour, spinsouraux  
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                             C
C Containing the common for spin of external particles. Shared by the       C
C subroutines ITERA, PROCESSO, FILLCONFSPIN AND SPINCOULOR 
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
         common/spin/spinsour(nmax),spinsouraux(nmax)
C
         do j1=1,nmax
          j2=nint(spinsouraux(j1)*100)
          spinsour(j1)=0
          if (j2.eq.49) then
           if(hel(j1).eq.1) then
            spinsour(j1)=0.49
           elseif(hel(j1).eq.-1) then
            spinsour(j1)=-0.49
           else
            write(6,*)'WRONG HELICITY ASSIGNMENT',j2,hel(j1)
            stop
           endif
          elseif (j2.eq.51) then
           if(hel(j1).eq.1) then
            spinsour(j1)=0.51
           elseif(hel(j1).eq.-1) then
            spinsour(j1)=-0.51
           else
            write(6,*)'WRONG HELICITY ASSIGNMENT',j2,hel(j1)
            stop
           endif
          elseif (j2.eq.110) then
           if(hel(j1).eq.1) then
            spinsour(j1)=1.1
           elseif(hel(j1).eq.-1) then
            spinsour(j1)=-1.1
           else
            write(6,*)'WRONG HELICITY ASSIGNMENT',j2,hel(j1)
            stop
           endif
          elseif (j2.eq.100) then
           if(hel(j1).eq.1) then
            spinsour(j1)=1.
           elseif(hel(j1).eq.0) then
            spinsour(j1)=0
           elseif(hel(j1).eq.-1) then
            spinsour(j1)=-1.
           else
            write(6,*)'WRONG HELICITY ASSIGNMENT',j2,hel(j1)
            stop
           endif
          endif
         enddo
C
         return
         end
C****************************************************************************
         subroutine spincoulor_hx (randnumb,nspinconf,label,labproc)
C****************************************************************************
C
C After the execution of this subroutine the array SPINSOUR will contain
C the spin of the particles chosen randomly according to the values
C of 0 <= RANDNUMB(J) <= 1 .
C
         implicit none
C
         integer nmax        !maximum number of external particles,
         integer nspinmax    !maximum number of spin configuration, 
         integer nprcmax     !maximum number of simultaneous processes,
         parameter (nmax=10)          
         parameter (nspinmax=2**nmax)          
         parameter (nprcmax=50)
         integer j1
         integer label,labproc
         integer nspinconf
         integer spinflag(nprcmax)
         integer spinflag1
C
         double precision  randnumb
         double precision  spinconf(nmax,nspinmax,nprcmax)
         double precision  spinsour, spinsouraux  
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                             C
C Containing the common for spin of external particles. Shared by the       C
C subroutines ITERA, PROCESSO, FILLCONFSPIN AND SPINCOULOR 
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
         common/spin/spinsour(nmax),spinsouraux(nmax)
         data spinflag/nprcmax*0/
C
C Static variables
C
         save spinflag,spinconf  
C
C 
C
         if (spinflag(labproc).eq.0)then
          spinflag(labproc)=1
c          write(6,*)'do you want me to try to define a "clever" spin'
c          write(6,*)'function? (0=no, 1=yes)'
c          read (5,*)spinflag1
          spinflag1=0
          if (spinflag1.eq.0)then
           call fillconfspin(nspinconf,spinconf(1,1,labproc))
          else
           call spinvariable(spinconf(1,1,labproc),nspinconf)
          endif
         endif
C
         label=int(randnumb*float(nspinconf))+1
         if (label.gt.nspinconf)label=nspinconf
C
         do j1=1,nmax
          spinsour(j1)=spinconf(j1,label,labproc)
         enddo
C
         return
         end
C*****************************************************************************
          subroutine fillmom(momen,noutgoingbis)
C*****************************************************************************
C
C Returns MOMENTA filled according to what needed by the subroutine ITERA
C
          implicit none
C
          integer nmax        !maximum number of external particles, 
          parameter (nmax=10)  
          integer indexoutgoing(nmax)  !reporting the order of storage of  
C                                       outgoing momenta
          integer indexingoing(nmax)  !reporting the order of storage of  
C                                       ingoing momenta
          integer j1,j2,j3          !loop variable
          integer noutgoingbis   !number of outgoing particles
          integer noutgoing            !number of outgoing particles
C
          double precision  internal    !array of flags to decide wether the particle is
C                              internal or external
          double precision momenta      !particles momenta
          double precision momen(4,noutgoingbis)  !outgoing particles momenta
          double precision  massoutgoing(nmax)   !containing the masses of outcoming 
C                                       particles
C
          double precision momentaprime(nmax*4),flag
          common/momenta/momenta(nmax,4),internal(nmax)
          common/momprime/momentaprime,flag
          common/integra/massoutgoing,
     >                       noutgoing,indexoutgoing,indexingoing
C
          flag=-1.
          do j1=1,noutgoing
           do j2=1,4
            momenta(indexoutgoing(j1),j2)=-momen(j2,j1)
           enddo
          enddo
C
          j3=0
          do j1=1,nmax
           do j2=1,4
            j3=j3+1
            momentaprime(j3)=momenta(j1,j2)
           enddo
          enddo
C
          return
          end
C*****************************************************************************
          subroutine fillmom_lhc(momen,ningoing)
C*****************************************************************************
C
C Returns MOMENTA filled according to what needed by the subroutine ITERA
C
          implicit none
C
          integer nmax        !maximum number of external particles, 
          parameter (nmax=10)  
          integer indexoutgoing(nmax)  !reporting the order of storage of  
C                                       outgoing momenta
          integer indexingoing(nmax)  !reporting the order of storage of  
C                                       ingoing momenta
          integer j1,j2,j3          !loop variable
          integer noutgoing            !number of outgoing particles
          integer ningoing            !number of outgoing particles
C
          double precision  internal    !array of flags to decide wether the particle is
C                              internal or external
          double precision momenta      !particles momenta
          double precision momen(4,nmax)  !outgoing particles momenta
          double precision  massoutgoing(nmax)   !containing the masses of outcoming 
C                                       particles
C
          double precision momentaprime(nmax*4),flag
          common/momenta/momenta(nmax,4),internal(nmax)
          common/momprime/momentaprime,flag
          common/integra/massoutgoing,
     >                       noutgoing,indexoutgoing,indexingoing
C
          flag=-1.
          do j1=1,noutgoing+ningoing
           do j2=1,4
            momenta(j1,j2)=momen(j2,j1)
           enddo
          enddo
C
          do j1=1,noutgoing
           do j2=1,4
            momenta(indexoutgoing(j1),j2)=-momenta(indexoutgoing(j1),j2)
           enddo
          enddo
C
          j3=0
          do j1=1,nmax
           do j2=1,4
            j3=j3+1
            momentaprime(j3)=momenta(j1,j2)
           enddo
          enddo
C
          return
          end
C*****************************************************************************
          subroutine fillmom_lhc0(momen,momen_in,noutgoingbis,ningoing)
C*****************************************************************************
C
C Returns MOMENTA filled according to what needed by the subroutine ITERA
C
          implicit none
C
          integer nmax        !maximum number of external particles, 
          parameter (nmax=10)  
          integer indexoutgoing(nmax)  !reporting the order of storage of  
C                                       outgoing momenta
          integer indexingoing(nmax)  !reporting the order of storage of  
C                                       ingoing momenta
          integer j1,j2,j3          !loop variable
          integer noutgoing            !number of outgoing particles
          integer noutgoingbis            !number of outgoing particles
          integer ningoing            !number of outgoing particles
C
          double precision  internal    !array of flags to decide wether the particle is
C                              internal or external
          double precision momenta      !particles momenta
          double precision momen(4,noutgoingbis)  !outgoing particles momenta
          double precision momen_in(4,ningoing)  !outgoing particles momenta
          double precision  massoutgoing(nmax)   !containing the masses of outcoming 
C                                       particles
C
          double precision momentaprime(nmax*4),flag
          common/momenta/momenta(nmax,4),internal(nmax)
          common/momprime/momentaprime,flag
          common/integra/massoutgoing,
     >                       noutgoing,indexoutgoing,indexingoing
C
          flag=-1.
          do j1=1,noutgoing
           do j2=1,4
            momenta(indexoutgoing(j1),j2)=-momen(j2,j1)
           enddo
          enddo
C
          do j1=1,ningoing
           do j2=1,4
            momenta(indexingoing(j1),j2)=momen_in(j2,j1)
           enddo
          enddo
C
          j3=0
          do j1=1,nmax
           do j2=1,4
            j3=j3+1
            momentaprime(j3)=momenta(j1,j2)
           enddo
          enddo
C
          return
          end
C**************************************************************************
         subroutine writespin
C**************************************************************************
C
         implicit none
C
         integer nmax           !maximum number of external particles, 
         integer nspinmax       !maximum number of spin configuration, 
         parameter (nmax=10)
         parameter (nspinmax=2**nmax)
         integer j1
         integer nspinconf1,ncolconf1
         integer spinflag
C
         double precision  partinteg(nspinmax),partinteg1(nspinmax)
C
         common/partspin/partinteg,partinteg1
         common/partspin1/nspinconf1,ncolconf1
         data spinflag/0/
C
C Static variables
C
         save spinflag    
C
C 
C
         if (spinflag.eq.0) then
          open(31,file='spinbin.dat',status='unknown')
          open(32,file='colbin.dat',status='unknown')
          spinflag=1
         else
          open(31,file='spinbin.dat',status='old')
          open(32,file='colbin.dat',status='old')
         endif
C
         do j1=1,nspinconf1
          write(31,*)partinteg(j1)
         enddo
         close(31)
C
         do j1=1,ncolconf1
          write(32,*)partinteg1(j1)
         enddo
         close(32)
C
         return
         end
C**************************************************************************
         subroutine regspin(nspinconf,label,ncolconf,label1,
     >                                            result,weight)
C**************************************************************************
C
         implicit none
C
         integer nmax           !maximum number of external particles, 
         integer nspinmax       !maximum number of spin configuration, change here
         parameter (nmax=10)
         parameter (nspinmax=2**nmax)
         integer j1
         integer flag
         integer nspinconf,nspinconf1
         integer ncolconf,ncolconf1
         integer label,label1
C
         double precision  partinteg(nspinmax),partinteg1(nspinmax)
         double precision  weight
         double precision  result
C
         common/partspin/partinteg,partinteg1
         common/partspin1/nspinconf1,ncolconf1
         data flag/0/
C
C Static variables
C
         save flag  
C
C 
C
         if (flag.eq.0)then
          do j1=1,nspinmax
           partinteg(j1)=0
           partinteg1(j1)=0
          enddo
          nspinconf1=nspinconf
          ncolconf1=ncolconf
          flag=1
         endif
C
         partinteg(label)=partinteg(label) +result*weight
         if (label1.gt.nspinmax) then
          write(6,*)'number of coulor configurations too big'
          stop
         endif
         partinteg1(label1)=partinteg1(label1) +result*weight
C
         return
         end
C***********************************************************************
          subroutine readarrtwobytwo_h(arr,ndm1,ndm2,ninteraction)
C***********************************************************************
C
C It reads NINTERACTION entries (for the first index) of an integer
C array ARR, NDM1 * NDM2 dimensioned, from the logical unit NUNIT
C
          implicit none
C
          integer ndm1,ndm2,ninteraction,j1,j2,arr(ndm1,ndm2)
          integer inputinteraction(1000),count3   !to initialize relevant interaction terms
          common/interactions/inputinteraction,count3
C
          count3=count3+1
          ninteraction=inputinteraction(count3)
          if(ninteraction.gt.0) then
           do j1=1,ninteraction
            do j2=1,ndm2
             count3=count3+1
             arr(j1,j2)=inputinteraction(count3)
            enddo
           enddo
          endif
C
          return
          end
C**********************************************************************
          subroutine readarrtwobytwo(arr,ndm1,ndm2,ninteraction,nunit)
C**********************************************************************
C
C It reads NINTERACTION entries (for the first index) of an integer
C array ARR, NDM1 * NDM2 dimensioned, from the logical unit NUNIT
C
          implicit none
C
          integer ndm1,ndm2,ninteraction,nunit,j1,j2,arr(ndm1,ndm2)
C
          read (nunit,*) ninteraction
          if(ninteraction.gt.0) then
           do j1=1,ninteraction
            do j2=1,ndm2
             read(nunit,*) arr(j1,j2)
            enddo
           enddo
          endif
C
          return
          end
C**************************************************************************
          subroutine zeroarrayint(arr,nel)
C**************************************************************************
C
C Setting to zero the NEL elements of an integer array ARR
C
          implicit none
          integer nel,arr(nel),j1
C
          do j1=1,nel
           arr(j1)=0
          enddo
C
          return
          end
C**********************************************************************
            subroutine sumintegarr(intarr,npt,sum)
C**********************************************************************
C
C Perform the sum SUM of the first NPT entries of an integer array INTARR(DIM)
C DIM > 0  it is very important!!!!!!
C
            implicit none
C
            integer npt,intarr(npt),sum,j1            
C
            sum=0
            do j1=1,npt
             sum=sum+intarr(j1)
            enddo
C
            return
            end

C*************************************************************************
           subroutine equalintarrays(intarr1,intarr2,npt)
C*************************************************************************
C
C It sets the integer array INTARR2(NPT) equal to INTARR1(NPT)
C
             implicit none
C
             integer npt,intarr1(npt),intarr2(npt),j1
C
             do j1=1,npt
              intarr2(j1)=intarr1(j1)
             enddo
C
             return
             end
C*************************************************************************
           subroutine equalcomparrays(intarr1,intarr2,npt)
C*************************************************************************
C
C It sets the complex array INTARR2(NPT) equal to INTARR1(NPT)
C
             implicit none
C
             integer npt,j1
             complex*16 intarr1(npt),intarr2(npt)
C
             do j1=1,npt
              intarr2(j1)=intarr1(j1)
             enddo
C
             return
             end
C*********************************************************************
          subroutine findmom(nmom,labmom,mom)
C*********************************************************************
C
C Given the number of momenta NMOM contributing to a given configuration
C with label LABMOM returns in MOM the four momenta
C
          implicit none
C
          integer nmax        !maximum number of external particles, 
          parameter (nmax=10)
          integer nmom,labmom,aux,conf(nmax),start(nmax/2),aux1,j1,j2
          double precision mom(4),momentaprime(nmax*4),flag,
     >                 momstore(5*(2**nmax-2)/2)
          common/momprime/momentaprime,flag
          common/strt/start
C
C Static variables
C
          save momstore
C
C 
C
          if (flag.lt.0) then
           flag=1.
           aux=0
           do j1=1,(2**nmax-2)/2
            momstore(1+aux)=-1.
            aux=aux+5
           enddo
          endif
C
         aux1=start(nmom)+labmom-1
         aux1=aux1*5+1
         if(momstore(aux1).lt.0) then
          call seleconf(nmom,labmom,conf)
          aux=0
          do j1=1,4
           mom(j1)=0.
          enddo
          do j1=1,nmax
           if (conf(j1).ne.0) then
            do j2=1,4
             mom(j2)=mom(j2)+momentaprime(j2+aux)
            enddo
           endif
           aux=aux+4
          enddo
C
          momstore(aux1)=1.
          do j1=1,4
           momstore(aux1+j1)=mom(j1)
          enddo
C
         else
          do j1=1,4
           mom(j1)=momstore(aux1+j1)
          enddo
         endif
C
          return
          end
C**********************************************************************72
          subroutine initcompconf
C**********************************************************************72
C
C  It computes all possible products among momenta configuration:
C  Those used in PRODINTERACTION are storedi in the array PRODCONFPROD,
C  those used in LAGINT in the array PRODCONFLAG
C
          implicit none
C
          integer nmax        !maximum number of external particles, 
          integer dmax
          parameter (nmax=10)
          parameter (dmax=(2**nmax-2)/2)
          integer ddim1,ddim2,ddim3,ddim4,ddim5,ddim6
          integer j1,j2,j3,n1,n2,nn1,nn2,newc
          integer next,nfermion,endloop
          common/external/next,nfermion  !containing the number of external
C                                     particles from the subroutine ??????
          integer prodconfprod(7,dmax,dmax),
     >            prodconflag(7,dmax,dmax)
          integer start(nmax/2)
C
          double precision perm(6)
C
          common/dimens/ddim1,ddim2,ddim3,ddim4,ddim5,ddim6 !number of excitations for
C                 CONFIGURATION1,2,3,4,5,6 computed in subroutine CONFIGURATION
          common/prodprod/prodconfprod
          common/prodlag/prodconflag
          common/strt/start
C
          start(1)=0
          start(2)=ddim1
          start(3)=ddim1+ddim2
          start(4)=ddim1+ddim2+ddim3
          start(5)=ddim1+ddim2+ddim3+ddim4
C change here (NMAX)
C
          if(next/2.lt.nmax/2) then
           endloop=start(next/2+1)
          else
           endloop=start(nmax/2)+ddim5
          endif
C
          do j1=1,endloop
           do j2=1,endloop
            do j3=1,next/2
             if(start(j3).lt.j1) n1=j3
            enddo
            nn1=j1-start(n1)
            do j3=1,next/2
             if(start(j3).lt.j2) n2=j3
            enddo
            nn2=j2-start(n2)
C
            if(n1+n2.ge.next/2) then
             call compconflag(n1,n2,nn1,nn2,newc,perm) 
             prodconflag(1,j1,j2)=newc
             do j3=2,7
              prodconflag(j3,j1,j2)=perm(j3-1)
             enddo
            else
             do j3=1,7
              prodconflag(j3,j1,j2)=0
             enddo
            endif
            if(n1+n2.le.next/2) then
             call compconf(n1,n2,nn1,nn2,newc,perm) 
             prodconfprod(1,j1,j2)=newc
             do j3=2,7
              prodconfprod(j3,j1,j2)=perm(j3-1)
             enddo
            else
             do j3=2,7
              prodconfprod(j3,j1,j2)=0
             enddo
            endif
C
           enddo
          enddo
C
          return
          end
C***************************************************************************
             subroutine reordero(first,tens,nl1,nl2,nl3) 
C***************************************************************************
C
C to use in PRODINTERACTION. given the particle FIRST whose equation is
C being computed it returns the interaction vector TENS in the proper order
C NL1, NL2, NL3 are the number of lorentz dof of the particle
C
             implicit none
C
             integer first,nl1,nl2,nl3,j1,j2,j3,lbp1,lbp2,lb,aux,nlormax
             parameter (nlormax=10)
             complex*16 tens(nlormax**3),tensprime(nlormax**3)
C
             if (first.eq.1) then        !T^{1,2,3} --> T^{1,2,3} do nothing
             elseif (first.eq.2) then    !T^{1,2,3} --> T^{2,1,3} 
              lb=0
              aux=nl3*nl2
              lbp1=0
              do j1=1,nl2
               lbp2=0
               do j2=1,nl1
                do j3=1,nl3
                 lb=lb+1
                 tensprime(lb)=tens(j3+lbp1+lbp2)
                enddo
               lbp2=lbp2+aux
               enddo
               lbp1=lbp1+nl3
              enddo
C
              aux=nl1*nl2*nl3
              do j1=1,aux
               tens(j1)=tensprime(j1)
              enddo
C
             elseif (first.eq.3) then    !T^{1,2,3} --> T^{3,1,2} 
              lb=0
              aux=nl3*nl2
              do j3=1,nl3
               lbp1=0
               do j1=1,nl1
                lbp2=0
                do j2=1,nl2
                 lb=lb+1
                 tensprime(lb)=tens(j3+lbp1+lbp2)
                 lbp2=lbp2+nl3
                enddo
                lbp1=lbp1+aux
               enddo
              enddo
C
              aux=nl1*nl2*nl3
              do j1=1,aux
               tens(j1)=tensprime(j1)
              enddo
C
             else
              write(6,*)'FIRST=',first,'in subroutine REORDER !!!!!'
             endif
C
             return
             end

C***************************************************************************
             subroutine reorder(first,tens,nl1,nl2,nl3) 
C***************************************************************************
C
C to use in PRODINTERACTION. given the particle FIRST whose equation is
C being computed it returns the interaction vector TENS in the proper order
C NL1, NL2, NL3 are the number of lorentz dof of the particle
C
             implicit none
C
             integer first,nl1,nl2,nl3,j1,j2,j3,lbp1,lbp2,lb,aux,nlormax
             parameter (nlormax=6)
             complex*16 tens(nlormax**3),tensprime(nlormax**3)
C
             if (first.eq.1) then        !T^{1,2,3} --> T^{1,2,3} do nothing
             elseif (first.eq.2) then    !T^{1,2,3} --> T^{2,1,3} 
              lb=0
              aux=nl3*nl1
              lbp1=0
              do j1=1,nl1
               lbp2=0
               do j2=1,nl2
                do j3=1,nl3
                 lb=lb+1
                 tensprime(lb)=tens(j3+lbp1+lbp2)
                enddo
               lbp2=lbp2+aux
               enddo
               lbp1=lbp1+nl3
              enddo
C
              aux=nl1*nl2*nl3
              do j1=1,aux
               tens(j1)=tensprime(j1)
              enddo
C
             elseif (first.eq.3) then    !T^{1,2,3} --> T^{3,1,2} 
              lb=0
              aux=nl1*nl3
              do j1=1,nl1
               lbp1=0
               do j2=1,nl2
                lbp2=0
                do j3=1,nl3
                 lb=lb+1
                 tensprime(lb)=tens(j1+lbp1+lbp2)
                 lbp2=lbp2+nl1
                enddo
                lbp1=lbp1+aux
               enddo
              enddo
C
              aux=nl1*nl2*nl3
              do j1=1,aux
               tens(j1)=tensprime(j1)
              enddo
C
             else
              write(6,*)'FIRST=',first,'in subroutine REORDER !!!!!'
             endif
C
             end
C**************************************************************************
              subroutine coloreprodx(momcol,momnew,newc,perm,cnew,flgord
     >                               ,coeff)
C**************************************************************************
C
C given two pairs of Q Qbar coulor indexes in MOMCOL(4) it returns
C in MOMNEW the resulting coulor. NEWC returns 0 if the two field cannot be
C combined and PERM return the correct coulor factor in the limit Ncolor = infinity
C CNEW labels the coulor representation of the new field.
C
              implicit none
C
              integer momcol(4),momnew(4),newc,cnew,flgord
              double precision perm,coeff(2)
C
C Static variables
C

C 
C
              coeff(1)=1.
              coeff(2)=0.
              if(cnew.eq.3) then                         !gluons
               if (momcol(2).eq.momcol(3)) then
                momnew(1)=momcol(1)
                momnew(2)=momcol(4)
                if (momcol(2).eq.0) then
                 write(6,*)'wrong color assignment to q/qbar'
                 stop
                endif
                if(momcol(1).eq.momcol(4).and.momcol(1).eq.momcol(3)) 
     >                                                         newc=0
                if(momcol(1).eq.momcol(4).and.newc.ne.0.
     >              and.momcol(1).ne.0) then
c                if(momcol(1).eq.momcol(4).and.newc.ne.0) then
c                 write(6,*)'inconsistency detected into COLOREPROD'
c                 write(6,*)'algorithm not equipped to deal with this' 
c                 write(6,*)'color configuration (see note in the code)'   ! al momento (2,1) * (1,2)
C                           da solo (2,2) e non (1,1). In caso di necessita'
C                           per ovviare al problema occorre considerare
C                           gluoni distinguibili
C
                endif
               elseif (momcol(1).eq.momcol(4)) then
                momnew(1)=momcol(3)
                momnew(2)=momcol(2)
                if(momnew(1).eq.momnew(2)) newc=0
               else
                newc=0
               endif
C
               if (flgord.eq.2) perm=-perm             !also the equation of
C                                              motion are color ordered
               if(momnew(1).eq.0.or.momnew(2).eq.0) perm=-perm/3.d0 
C               
              elseif(cnew.eq.2) then                    !quarks
               if (momcol(2).eq.momcol(3)) then
                momnew(1)=momcol(1)
                momnew(2)=momcol(4)
               elseif (momcol(1).eq.momcol(4)) then
                momnew(1)=momcol(3)
                momnew(2)=momcol(2)
               else
                newc=0.
               endif               
              elseif(cnew.eq.1) then                    !coulorless object
               if (momcol(1).eq.momcol(4).and.momcol(2).eq.momcol(3)) 
     >                                                          then
                momnew(1)=0
                momnew(2)=0
               else
                newc=0
               endif     
              else
               write(6,*)'disaster in COLOREPROD'
              endif
C
              return
              end
C**************************************************************************
              subroutine coloreprod(momcol,momnew,newc,perm,cnew,flgord
     >                               ,coeff)
C**************************************************************************
C
C given two pairs of Q Qbar coulor indexes in MOMCOL(4) it returns
C in MOMNEW the resulting coulor. NEWC returns 0 if the two field cannot be
C combined and PERM return the correct coulor factor in the limit Ncolor = infinity
C CNEW labels the coulor representation of the new field.
C
              implicit none
C
              integer momcol(4),momnew(4),newc,cnew,flgord
              double precision perm,coeff(2)
C
C Static variables
C

C 
C
              coeff(1)=1.
              coeff(2)=0.
              if(cnew.eq.3) then                         !gluons
               if (momcol(2).eq.momcol(3)) then
                momnew(1)=momcol(1)
                momnew(2)=momcol(4)
                if (momcol(2).eq.0) then
                  newc=0
                  return 
c                 write(6,*)'wrong color assignment to q/qbar'
c                 stop
                endif
                if(momcol(1).eq.momcol(4).and.momcol(1).eq.momcol(3)) 
     >                                                         newc=0
                if(momcol(1).eq.momcol(4).and.newc.ne.0.
     >              and.momcol(1).ne.0) then
c                if(momcol(1).eq.momcol(4).and.newc.ne.0) then
c                 write(6,*)'inconsistency detected into COLOREPROD'
c                 write(6,*)'algorithm not equipped to deal with this' 
c                 write(6,*)'color configuration (see note in the code)'   ! al momento (2,1) * (1,2)
C                           da solo (2,2) e non (1,1). In caso di necessita'
C                           per ovviare al problema occorre considerare
C                           gluoni distinguibili
C
                endif
               elseif (momcol(1).eq.momcol(4)) then
                momnew(1)=momcol(3)
                momnew(2)=momcol(2)
                if(momcol(1).ne.0) coeff(1)=-1.
                if(momnew(1).eq.momnew(2)) newc=0
               else
                newc=0
               endif
C
               if (flgord.eq.2) perm=-perm             !also the equation of
C                                              motion are color ordered
               if(momnew(1).eq.0.or.momnew(2).eq.0) perm=-perm/3.d0 
C               
              elseif(cnew.eq.2) then                    !quarks
               if (momcol(2).eq.momcol(3)) then
                momnew(1)=momcol(1)
                momnew(2)=momcol(4)
               elseif (momcol(1).eq.momcol(4)) then
                momnew(1)=momcol(3)
                momnew(2)=momcol(2)
               else
                newc=0.
               endif               
              elseif(cnew.eq.1) then                    !coulorless object
               if (momcol(1).eq.momcol(4).and.momcol(2).eq.momcol(3)) 
     >                                                          then
                momnew(1)=0
                momnew(2)=0
               else
                newc=0
               endif     
              else
               write(6,*)'disaster in COLOREPROD'
              endif
C
              return
              end
C**************************************************************************
              subroutine coloreprodsu3(momcol,momnew,newc,perm,cnew,
     >                              flgord,coeff)
C**************************************************************************
C
C given two pairs of Q Qbar coulor indexes in MOMCOL(4) it returns
C in MOMNEW the resulting coulor. NEWC returns 0 if the two field cannot be
C combined and PERM return the correct coulor factor in the limit Ncolor = infinity
C CNEW labels the coulor representation of the new field.
C
              implicit none
C
              integer momcol(4),momnew(4),newc,cnew,flgord
              double precision perm,coeff(2),tabcoeff(0:3,0:3,0:3,0:3,2)
              integer tabmom(0:3,0:3,0:3,0:3,4),flag
              integer tabnew(0:3,0:3,0:3,0:3),i1,i2,i3,i4,j1
              data tabnew/256*0/,tabmom/1024*0/,flag/0/,tabcoeff/512*0./
C
C Static variables
C
              save flag,tabnew,tabmom,tabcoeff   
C
C 
C
c              coeff(1)=1.
c              coeff(2)=0.
C
              if (flag.eq.0) then
               flag=1
               tabnew(1,1,1,1)=0
c
               tabnew(1,1,1,2)=1
               tabcoeff(1,1,1,2,1)=1./sqrt(2.d0)
               tabmom(1,1,1,2,1)=1
               tabmom(1,1,1,2,2)=2
c
               tabnew(1,1,1,3)=1
               tabcoeff(1,1,1,3,1)=sqrt(2.d0)
               tabmom(1,1,1,3,1)=1
               tabmom(1,1,1,3,2)=3
c
               tabnew(1,1,2,1)=1
               tabcoeff(1,1,2,1,1)=-1./sqrt(2.d0)
               tabmom(1,1,2,1,1)=2
               tabmom(1,1,2,1,2)=1
c
               tabnew(1,1,2,2)=0
c
               tabnew(1,1,2,3)=1
               tabcoeff(1,1,2,3,1)=1./sqrt(2.d0)
               tabmom(1,1,2,3,1)=2
               tabmom(1,1,2,3,2)=3
c
               tabnew(1,1,3,1)=1
               tabcoeff(1,1,3,1,1)=-sqrt(2.d0)
               tabmom(1,1,3,1,1)=3
               tabmom(1,1,3,1,2)=1
c
               tabnew(1,1,3,2)=1
               tabcoeff(1,1,3,2,1)=-1./sqrt(2.d0)
               tabmom(1,1,3,2,1)=3
               tabmom(1,1,3,2,2)=2
c
               tabnew(1,2,1,1)=1
               tabcoeff(1,2,1,1,1)=-1./sqrt(2.d0)
               tabmom(1,2,1,1,1)=1
               tabmom(1,2,1,1,2)=2
c
               tabnew(1,2,1,2)=0
c
               tabnew(1,2,1,3)=0
c
               tabnew(1,2,2,1)=1
               tabcoeff(1,2,2,1,1)=1./sqrt(2.d0)
               tabcoeff(1,2,2,1,2)=-sqrt(3.d0)/sqrt(2.d0)
               tabmom(1,2,2,1,1)=1
               tabmom(1,2,2,1,2)=1
               tabmom(1,2,2,1,3)=2
               tabmom(1,2,2,1,4)=2
c
               tabnew(1,2,2,2)=1
               tabcoeff(1,2,2,2,1)=sqrt(3.d0)/sqrt(2.d0)
               tabmom(1,2,2,2,1)=1
               tabmom(1,2,2,2,2)=2
c
               tabnew(1,2,2,3)=1
               tabcoeff(1,2,2,3,1)=1.
               tabmom(1,2,2,3,1)=1
               tabmom(1,2,2,3,2)=3
c
               tabnew(1,2,3,1)=1
               tabcoeff(1,2,3,1,1)=-1.
               tabmom(1,2,3,1,1)=3
               tabmom(1,2,3,1,2)=2
c
               tabnew(1,2,3,2)=0
c
               tabnew(1,3,1,1)=1
               tabcoeff(1,3,1,1,1)=-sqrt(2.d0)
               tabmom(1,3,1,1,1)=1
               tabmom(1,3,1,1,2)=3
c
               tabnew(1,3,1,2)=0
c
               tabnew(1,3,1,3)=0
c
               tabnew(1,3,2,1)=1
               tabcoeff(1,3,2,1,1)=-1.
               tabmom(1,3,2,1,1)=2
               tabmom(1,3,2,1,2)=3
c
               tabnew(1,3,2,2)=0
c
               tabnew(1,3,2,3)=0
c
               tabnew(1,3,3,1)=1
               tabcoeff(1,3,3,1,1)=sqrt(2.d0)
               tabmom(1,3,3,1,1)=1
               tabmom(1,3,3,1,2)=1
c
               tabnew(1,3,3,2)=1
               tabcoeff(1,3,3,2,1)=1.
               tabmom(1,3,3,2,1)=1
               tabmom(1,3,3,2,2)=2
c
               tabnew(2,1,1,1)=1
               tabcoeff(2,1,1,1,1)=1./sqrt(2.d0)
               tabmom(2,1,1,1,1)=2
               tabmom(2,1,1,1,2)=1
c
               tabnew(2,1,1,2)=1
               tabcoeff(2,1,1,2,1)=-1./sqrt(2.d0)
               tabcoeff(2,1,1,2,2)=sqrt(3.d0)/sqrt(2.d0)
               tabmom(2,1,1,2,1)=1
               tabmom(2,1,1,2,2)=1
               tabmom(2,1,1,2,3)=2
               tabmom(2,1,1,2,4)=2
c
               tabnew(2,1,1,3)=1
               tabcoeff(2,1,1,3,1)=1.
               tabmom(2,1,1,3,1)=2
               tabmom(2,1,1,3,2)=3
c
               tabnew(2,1,2,1)=0
c
               tabnew(2,1,2,2)=1
               tabcoeff(2,1,2,2,1)=-sqrt(3.d0)/sqrt(2.d0)
               tabmom(2,1,2,2,1)=2
               tabmom(2,1,2,2,2)=1
c
               tabnew(2,1,2,3)=0
c
               tabnew(2,1,3,1)=0
c
               tabnew(2,1,3,2)=1
               tabcoeff(2,1,3,2,1)=-1.
               tabmom(2,1,3,2,1)=3
               tabmom(2,1,3,2,2)=1
c
               tabnew(2,2,1,1)=0
c
               tabnew(2,2,1,2)=1
               tabcoeff(2,2,1,2,1)=-sqrt(3.d0)/sqrt(2.d0)
               tabmom(2,2,1,2,1)=1
               tabmom(2,2,1,2,2)=2
c
               tabnew(2,2,1,3)=0
c
               tabnew(2,2,2,1)=1
               tabcoeff(2,2,2,1,1)=sqrt(3.d0)/sqrt(2.d0)
               tabmom(2,2,2,1,1)=2
               tabmom(2,2,2,1,2)=1
c
               tabnew(2,2,2,2)=0
c
               tabnew(2,2,2,3)=1
               tabcoeff(2,2,2,3,1)=sqrt(3.d0)/sqrt(2.d0)
               tabmom(2,2,2,3,1)=2
               tabmom(2,2,2,3,2)=3
c
               tabnew(2,2,3,1)=0
c
               tabnew(2,2,3,2)=1
               tabcoeff(2,2,3,2,1)=-sqrt(3.d0)/sqrt(2.d0)
               tabmom(2,2,3,2,1)=3
               tabmom(2,2,3,2,2)=2
c
               tabnew(2,3,1,1)=1
               tabcoeff(2,3,1,1,1)=-1./sqrt(2.d0)
               tabmom(2,3,1,1,1)=2
               tabmom(2,3,1,1,2)=3
c
               tabnew(2,3,1,2)=1
               tabcoeff(2,3,1,2,1)=-1
               tabmom(2,3,1,2,1)=1
               tabmom(2,3,1,2,2)=3
c
c
               tabnew(2,3,1,3)=0
c
               tabnew(2,3,2,1)=0
c
               tabnew(2,3,2,2)=1
               tabcoeff(2,3,2,2,1)=-sqrt(3.d0)/sqrt(2.d0)
               tabmom(2,3,2,2,1)=2
               tabmom(2,3,2,2,2)=3
c
               tabnew(2,3,2,3)=0
c
               tabnew(2,3,3,1)=1
               tabcoeff(2,3,3,1,1)=1.
               tabmom(2,3,3,1,1)=2
               tabmom(2,3,3,1,2)=1
c
               tabnew(2,3,3,2)=1
               tabcoeff(2,3,3,2,1)=1./sqrt(2.d0)
               tabcoeff(2,3,3,2,2)=sqrt(3.d0)/sqrt(2.d0)
               tabmom(2,3,3,2,1)=1
               tabmom(2,3,3,2,2)=1
               tabmom(2,3,3,2,3)=2
               tabmom(2,3,3,2,4)=2
c
               tabnew(3,1,1,1)=1
               tabcoeff(3,1,1,1,1)=sqrt(2.d0)
               tabmom(3,1,1,1,1)=3
               tabmom(3,1,1,1,2)=1
c
               tabnew(3,1,1,2)=1
               tabcoeff(3,1,1,2,1)=1.
               tabmom(3,1,1,2,1)=3
               tabmom(3,1,1,2,2)=2
c
               tabnew(3,1,1,3)=1
               tabcoeff(3,1,1,3,1)=-sqrt(2.d0)
               tabmom(3,1,1,3,1)=1
               tabmom(3,1,1,3,2)=1
c
               tabnew(3,1,2,1)=0
c
               tabnew(3,1,2,2)=0
c
               tabnew(3,1,2,3)=1
               tabcoeff(3,1,2,3,1)=-1.
               tabmom(3,1,2,3,1)=2
               tabmom(3,1,2,3,2)=1
c
               tabnew(3,1,3,1)=0
c
               tabnew(3,1,3,2)=0
c
               tabnew(3,2,1,1)=1
               tabcoeff(3,2,1,1,1)=1./sqrt(2.d0)
               tabmom(3,2,1,1,1)=3
               tabmom(3,2,1,1,2)=2
c
               tabnew(3,2,1,2)=0
c
               tabnew(3,2,1,3)=1
               tabcoeff(3,2,1,3,1)=-1.
               tabmom(3,2,1,3,1)=1
               tabmom(3,2,1,3,2)=2
c
               tabnew(3,2,2,1)=1
               tabcoeff(3,2,2,1,1)=1.
               tabmom(3,2,2,1,1)=3
               tabmom(3,2,2,1,2)=1
c
               tabnew(3,2,2,2)=1
               tabcoeff(3,2,2,2,1)=sqrt(3.d0/2.d0)
               tabmom(3,2,2,2,1)=3
               tabmom(3,2,2,2,2)=2
c
               tabnew(3,2,2,3)=1
               tabcoeff(3,2,2,3,1)=-1./sqrt(2.d0)
               tabcoeff(3,2,2,3,2)=-sqrt(3.d0)/sqrt(2.d0)
               tabmom(3,2,2,3,1)=1
               tabmom(3,2,2,3,2)=1
               tabmom(3,2,2,3,3)=2
               tabmom(3,2,2,3,4)=2
c
               tabnew(3,2,3,1)=0
c
               tabnew(3,2,3,2)=0
c
               tabnew(0,1,1,0)=1
               tabcoeff(0,1,1,0,1)=1./sqrt(2.d0)
               tabcoeff(0,1,1,0,2)=-1./sqrt(6.d0)
               tabmom(0,1,1,0,1)=1
               tabmom(0,1,1,0,2)=1
               tabmom(0,1,1,0,3)=2
               tabmom(0,1,1,0,4)=2
c
               tabnew(0,1,2,0)=1
               tabcoeff(0,1,2,0,1)=1.
               tabmom(0,1,2,0,1)=2
               tabmom(0,1,2,0,2)=1
c
               tabnew(0,1,3,0)=1
               tabcoeff(0,1,3,0,1)=1.
               tabmom(0,1,3,0,1)=3
               tabmom(0,1,3,0,2)=1
c
               tabnew(0,2,1,0)=1
               tabcoeff(0,2,1,0,1)=1.
               tabmom(0,2,1,0,1)=1
               tabmom(0,2,1,0,2)=2
c
               tabnew(0,2,2,0)=1
               tabcoeff(0,2,2,0,1)=sqrt(2.d0/3.d0)
               tabmom(0,2,2,0,1)=2
               tabmom(0,2,2,0,2)=2
c
               tabnew(0,2,3,0)=1
               tabcoeff(0,2,3,0,1)=1.
               tabmom(0,2,3,0,1)=3
               tabmom(0,2,3,0,2)=2
c
               tabnew(0,3,1,0)=1
               tabcoeff(0,3,1,0,1)=1.
               tabmom(0,3,1,0,1)=1
               tabmom(0,3,1,0,2)=3
c
               tabnew(0,3,2,0)=1
               tabcoeff(0,3,2,0,1)=1.
               tabmom(0,3,2,0,1)=2
               tabmom(0,3,2,0,2)=3
c
               tabnew(0,3,3,0)=1
               tabcoeff(0,3,3,0,1)=-1./sqrt(2.d0)
               tabcoeff(0,3,3,0,2)=-1./sqrt(6.d0)
               tabmom(0,3,3,0,1)=1
               tabmom(0,3,3,0,2)=1
               tabmom(0,3,3,0,3)=2
               tabmom(0,3,3,0,4)=2
c
               tabnew(1,1,0,1)=1
               tabcoeff(1,1,0,1,1)=1./sqrt(2.d0)
               tabmom(1,1,0,1,1)=0
               tabmom(1,1,0,1,2)=1
c
               tabnew(1,1,0,3)=1
               tabcoeff(1,1,0,3,1)=-1./sqrt(2.d0)
               tabmom(1,1,0,3,1)=0
               tabmom(1,1,0,3,2)=3
c
               tabnew(1,2,0,1)=1
               tabcoeff(1,2,0,1,1)=1.
               tabmom(1,2,0,1,1)=0
               tabmom(1,2,0,1,2)=2
c
               tabnew(1,3,0,1)=1
               tabcoeff(1,3,0,1,1)=1.
               tabmom(1,3,0,1,1)=0
               tabmom(1,3,0,1,2)=3
c
               tabnew(2,1,0,2)=1
               tabcoeff(2,1,0,2,1)=1.
               tabmom(2,1,0,2,1)=0
               tabmom(2,1,0,2,2)=1
c
               tabnew(2,2,0,1)=1
               tabcoeff(2,2,0,1,1)=-1./sqrt(6.d0)
               tabmom(2,2,0,1,1)=0
               tabmom(2,2,0,1,2)=1
c
               tabnew(2,2,0,2)=1
               tabcoeff(2,2,0,2,1)=sqrt(2.d0/3.d0)
               tabmom(2,2,0,2,1)=0
               tabmom(2,2,0,2,2)=2
c
               tabnew(2,2,0,3)=1
               tabcoeff(2,2,0,3,1)=-1./sqrt(6.d0)
               tabmom(2,2,0,3,1)=0
               tabmom(2,2,0,3,2)=3
c
               tabnew(2,3,0,2)=1
               tabcoeff(2,3,0,2,1)=1.
               tabmom(2,3,0,2,1)=0
               tabmom(2,3,0,2,2)=3
c
               tabnew(3,1,0,3)=1
               tabcoeff(3,1,0,3,1)=1.
               tabmom(3,1,0,3,1)=0
               tabmom(3,1,0,3,2)=1
c
               tabnew(3,2,0,3)=1
               tabcoeff(3,2,0,3,1)=1.
               tabmom(3,2,0,3,1)=0
               tabmom(3,2,0,3,2)=2
c
               tabnew(1,1,1,0)=1
               tabcoeff(1,1,1,0,1)=1./sqrt(2.d0)
               tabmom(1,1,1,0,1)=1
               tabmom(1,1,1,0,2)=0
c
               tabnew(1,1,3,0)=1
               tabcoeff(1,1,3,0,1)=-1./sqrt(2.d0)
               tabmom(1,1,3,0,1)=3
               tabmom(1,1,3,0,2)=0
c
               tabnew(1,2,2,0)=1
               tabcoeff(1,2,2,0,1)=1.
               tabmom(1,2,2,0,1)=1
               tabmom(1,2,2,0,2)=0
c
               tabnew(1,3,3,0)=1
               tabcoeff(1,3,3,0,1)=1.
               tabmom(1,3,3,0,1)=1
               tabmom(1,3,3,0,2)=0
c
               tabnew(2,1,1,0)=1
               tabcoeff(2,1,1,0,1)=1.
               tabmom(2,1,1,0,1)=2
               tabmom(2,1,1,0,2)=0
c
               tabnew(2,2,1,0)=1
               tabcoeff(2,2,1,0,1)=-1./sqrt(6.d0)
               tabmom(2,2,1,0,1)=1
               tabmom(2,2,1,0,2)=0
c
               tabnew(2,2,2,0)=1
               tabcoeff(2,2,2,0,1)=sqrt(2.d0/3.d0)
               tabmom(2,2,2,0,1)=2
               tabmom(2,2,2,0,2)=0
c
               tabnew(2,2,3,0)=1
               tabcoeff(2,2,3,0,1)=-1./sqrt(6.d0)
               tabmom(2,2,3,0,1)=3
               tabmom(2,2,3,0,2)=0
c
               tabnew(2,3,3,0)=1
               tabcoeff(2,3,3,0,1)=1.
               tabmom(2,3,3,0,1)=2
               tabmom(2,3,3,0,2)=0
c
               tabnew(3,1,1,0)=1
               tabcoeff(3,1,1,0,1)=1.
               tabmom(3,1,1,0,1)=3
               tabmom(3,1,1,0,2)=0
c
               tabnew(3,2,2,0)=1
               tabcoeff(3,2,2,0,1)=1.
               tabmom(3,2,2,0,1)=3
               tabmom(3,2,2,0,2)=0
c
               tabnew(1,0,0,0)=1
               tabcoeff(1,0,0,0,1)=1.
               tabmom(1,0,0,0,1)=1
               tabmom(1,0,0,0,2)=0
c
               tabnew(2,0,0,0)=1
               tabcoeff(2,0,0,0,1)=1.
               tabmom(2,0,0,0,1)=2
               tabmom(2,0,0,0,2)=0
c
               tabnew(3,0,0,0)=1
               tabcoeff(3,0,0,0,1)=1.
               tabmom(3,0,0,0,1)=3
               tabmom(3,0,0,0,2)=0
c
               tabnew(0,0,1,0)=1
               tabcoeff(0,0,1,0,1)=1.
               tabmom(0,0,1,0,1)=1
               tabmom(0,0,1,0,2)=0
c
               tabnew(0,0,2,0)=1
               tabcoeff(0,0,2,0,1)=1.
               tabmom(0,0,2,0,1)=2
               tabmom(0,0,2,0,2)=0
c
               tabnew(0,0,3,0)=1
               tabcoeff(0,0,3,0,1)=1.
               tabmom(0,0,3,0,1)=3
               tabmom(0,0,3,0,2)=0
c
               tabnew(0,1,0,0)=1
               tabcoeff(0,1,0,0,1)=1.
               tabmom(0,1,0,0,1)=0
               tabmom(0,1,0,0,2)=1
c
               tabnew(0,2,0,0)=1
               tabcoeff(0,2,0,0,1)=1.
               tabmom(0,2,0,0,1)=0
               tabmom(0,2,0,0,2)=2
c
               tabnew(0,3,0,0)=1
               tabcoeff(0,3,0,0,1)=1.
               tabmom(0,3,0,0,1)=0
               tabmom(0,3,0,0,2)=3
c
               tabnew(0,0,0,1)=1
               tabcoeff(0,0,0,1,1)=1.
               tabmom(0,0,0,1,1)=0
               tabmom(0,0,0,1,2)=1
c
               tabnew(0,0,0,2)=1
               tabcoeff(0,0,0,2,1)=1.
               tabmom(0,0,0,2,1)=0
               tabmom(0,0,0,2,2)=2
c
               tabnew(0,0,0,3)=1
               tabcoeff(0,0,0,3,1)=1.
               tabmom(0,0,0,3,1)=0
               tabmom(0,0,0,3,2)=3
c
             endif
C
              if(cnew.gt.1.and.cnew.lt.4) then       !gluons and quarks
               if (cnew.eq.3.and.flgord.eq.2) perm=-perm  !also the equation of
C                                              motion are color ordered
               i1=momcol(1)
               i2=momcol(2)
               i3=momcol(3)
               i4=momcol(4)
               if(tabnew(i1,i2,i3,i4).eq.1) then
                coeff(1)= tabcoeff(i1,i2,i3,i4,1)               
                coeff(2)= tabcoeff(i1,i2,i3,i4,2)
                do j1=1,4
                 momnew(j1)=tabmom(i1,i2,i3,i4,j1)
                enddo       
               else
                newc=0
               endif
              elseif(cnew.eq.1) then                    !coulorless object
               if (momcol(1).eq.momcol(4).and.momcol(2).eq.momcol(3)) 
     >                                                          then
                coeff(1)=1.
                coeff(2)=0.
                do j1=1,4
                 momnew(j1)=0
                enddo       
               else
                newc=0
               endif     
              else
               write(6,*)'disaster in COLOREPROD'
              endif
C
              return
              end
C**************************************************************************
              subroutine colorelagsu3(momcol,newc,perm)
C**************************************************************************
C
C given three pairs of Q Qbar coulor indexes in MOMCOL(6) 
C NEWC returns 0 if the two field cannot be
C combined and PERM return the correct coulor factor in the limit Ncolor = infinity
C
              implicit none
C
              integer momcol(6),newc,flag
              double precision perm,tabperm(0:3,0:3,0:3,0:3,0:3,0:3)
              save flag,tabperm
              data tabperm/4096*0./
              dataflag/0/ 
C
              if(flag.eq.0) then
               flag=1
               tabperm(1,1,1,2,2,1)=1./sqrt(2.d0)
               tabperm(1,1,1,3,3,1)=1.*sqrt(2.d0)
               tabperm(1,1,2,1,1,2)=-1./sqrt(2.d0)
               tabperm(1,1,2,3,3,2)=1./sqrt(2.d0)
               tabperm(1,1,3,1,1,3)=-1.*sqrt(2.d0)
               tabperm(1,1,3,2,2,3)=-1./sqrt(2.d0)
               tabperm(1,2,1,1,2,1)=-1./sqrt(2.d0)
               tabperm(1,2,2,1,1,1)=1./sqrt(2.d0)
               tabperm(1,2,2,1,2,2)=-1.*sqrt(1.5d0)
               tabperm(1,2,2,2,2,1)=1.*sqrt(1.5d0)
               tabperm(1,2,2,3,3,1)=1.
               tabperm(1,2,3,1,2,3)=-1.
               tabperm(1,3,1,1,3,1)=-1.*sqrt(2.d0)
               tabperm(1,3,2,1,3,2)=-1.
               tabperm(1,3,3,1,1,1)=1.*sqrt(2.d0)
               tabperm(1,3,3,2,2,1)=1.
               tabperm(2,1,1,1,1,2)=1./sqrt(2.d0)
               tabperm(2,1,1,2,1,1)=-1./sqrt(2.d0)
               tabperm(2,1,1,2,2,2)=1.*sqrt(1.5d0)
               tabperm(2,1,1,3,3,2)=1.
               tabperm(2,1,2,2,1,2)=-1.*sqrt(1.5d0)
               tabperm(2,1,3,2,1,3)=-1.
               tabperm(2,2,1,2,2,1)=-1.*sqrt(1.5d0)
               tabperm(2,2,2,1,1,2)=1.*sqrt(1.5d0)
               tabperm(2,2,2,3,3,2)=1.*sqrt(1.5d0)
               tabperm(2,2,3,2,2,3)=-1.*sqrt(1.5d0)
               tabperm(2,3,1,1,3,2)=-1./sqrt(2.d0)
               tabperm(2,3,1,2,3,1)=-1.
               tabperm(2,3,2,2,3,2)=-1.*sqrt(1.5d0)
               tabperm(2,3,3,1,1,2)=1.
               tabperm(2,3,3,2,1,1)=1./sqrt(2.d0)
               tabperm(2,3,3,2,2,2)=1.*sqrt(1.5d0)
               tabperm(3,1,1,1,1,3)=1.*sqrt(2.d0)
               tabperm(3,1,1,2,2,3)=1.
               tabperm(3,1,1,3,1,1)=-1.*sqrt(2.d0)
               tabperm(3,1,2,3,1,2)=-1.
               tabperm(3,2,1,1,2,3)=1./sqrt(2.d0)
               tabperm(3,2,1,3,2,1)=-1.
               tabperm(3,2,2,1,1,3)=1.
               tabperm(3,2,2,2,2,3)=1.*sqrt(1.5d0)
               tabperm(3,2,2,3,1,1)=-1./sqrt(2.d0)
               tabperm(3,2,2,3,2,2)=-1.*sqrt(1.5d0)
               tabperm(1,1,0,1,1,0)=1./sqrt(2.d0)
               tabperm(1,1,0,3,3,0)=-1./sqrt(2.d0)
               tabperm(1,2,0,1,2,0)=1.
               tabperm(1,3,0,1,3,0)=1.
               tabperm(2,1,0,2,1,0)=1.
               tabperm(2,2,0,1,1,0)=-1./sqrt(6.d0)
               tabperm(2,2,0,2,2,0)=sqrt(2.d0/3.d0)
               tabperm(2,2,0,3,3,0)=-1./sqrt(6.d0)
               tabperm(2,3,0,2,3,0)=1.
               tabperm(3,1,0,3,1,0)=1.
               tabperm(3,2,0,3,2,0)=1.
               tabperm(0,0,0,1,1,0)=1.
               tabperm(0,0,0,2,2,0)=1.
               tabperm(0,0,0,3,3,0)=1.
               tabperm(0,0,0,0,0,0)=1.
              endif
C
              if (abs(tabperm(momcol(1),momcol(2),momcol(3),
     >             momcol(4),momcol(5),momcol(6))).gt.1.d-10) then
               perm=perm*tabperm(momcol(1),momcol(2),momcol(3),
     >             momcol(4),momcol(5),momcol(6))
               newc=1
              else
               newc=0
              endif
C
              return
              end

C**************************************************************************
              subroutine colorelag(momcol,newc,perm,oper)
C**************************************************************************
C
C given three pairs of Q Qbar coulor indexes in MOMCOL(6) 
C NEWC returns 0 if the two field cannot be
C combined and PERM return the correct coulor factor in the limit Ncolor = infinity
C
              implicit none
C
              integer momcol(6),newc,oper
              double precision perm
C
              if(momcol(2).eq.momcol(3)) then
               if(momcol(4).eq.momcol(5)) then
                if(momcol(1).ne.momcol(6)) newc=0 
               else
                 newc=0
               endif
              elseif(momcol(1).eq.momcol(4))then
               if(momcol(1).ne.0.and.momcol(3).ne.0.and.momcol(5).ne.0)
     >             perm=-perm
               if(momcol(2).eq.momcol(5)) then
                if(momcol(3).ne.momcol(6)) newc=0 
                else
                 newc=0
                endif
              else
               newc=0
              endif
C
              if (oper.ge.3.and.momcol(1).eq.0) newc=0   !three singlet gluons 
C
              return
              end
C**************************************************************************
              subroutine colorelagx(momcol,newc,perm)
C**************************************************************************
C
C given three pairs of Q Qbar coulor indexes in MOMCOL(6) 
C NEWC returns 0 if the two field cannot be
C combined and PERM return the correct coulor factor in the limit Ncolor = infinity
C
              implicit none
C
              integer momcol(6),newc
              double precision perm
C
              if(momcol(2).eq.momcol(3)) then
               if(momcol(4).eq.momcol(5)) then
                if(momcol(1).ne.momcol(6)) newc=0 
               else
                 newc=0
               endif
              elseif(momcol(1).eq.momcol(4))then
               if(momcol(1).ne.0.and.momcol(3).ne.0.and.momcol(5).ne.0)
     >             perm=-perm
               if(momcol(2).eq.momcol(5)) then
                if(momcol(3).ne.momcol(6)) newc=0 
                else
                 newc=0
                endif
              else
               newc=0
              endif
C
              return
              end

