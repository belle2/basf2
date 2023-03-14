C***********************************************************************
          subroutine couplings           
C***********************************************************************
C
C This subroutine returns the couplings of the model
C 
          implicit none
C
          integer ngaubos
          parameter (ngaubos=14)
          complex*16 re,im
          parameter(re=(1.,0.))
          parameter(im=(0.,1.))
C
          double precision conver   !to convert to the chosen mass units
          parameter (conver=1.d0)   !25 GeV units
          double precision minfer   !conventionally no mass fermion is less
C                                    than minfer
          parameter (minfer=0.2*conver) !200 MeV limit
C
          double precision higgsvev     !higgs vacuum expectation value
          double precision gstrong      !SU(3) gauge coupling
          double precision gsu2l        !SU(2)_L gauge coupling
          double precision gu1y         !U(1)_Y gauge coupling
          double precision gbar
          double precision ctetaw
          double precision stetaw       !Weinberg angle
          double precision stetaw2
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C COUPLINGS.COMM                                                            C
C                                                                           C
C Containing the common for coupling constant. Shared by the subroutines    C
C ITERA and COUPLINGS                                                       C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          complex*16 selfhiggs(3)    !higgs and auxiliary-higgs selfcoupling 
C                                     constant
          complex*16 yukawa(3,4,3,4) !yukawa couplings
          complex*16 vectorial(ngaubos,3,4,3,4)   !V fermions-gauge-bosons 
C                                                  interactions
          complex*16 assial(ngaubos,3,4,3,4)      !V fermions-gauge-bosons 
C                                                  interactions
          complex*16 selfgauge(ngaubos,ngaubos,ngaubos) !gauge-bosons 
C                                                        self-interactions
          complex*16 higgsgauge(3,ngaubos,ngaubos)  !higgs-gauge bosons 
C                                                    couplings
C
          common/coupconst/selfhiggs,yukawa,vectorial,selfgauge,
     >                    higgsgauge,assial   !add here new couplings
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C MASSES.COMM                                                               C
C                                                                           C
C Containing the common for particles masses. Shared by the subroutines     C
C ITERA and COUPLINGS                                                       C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          double precision masshiggs(3)    !higgs mass
          double precision massgaubosons(ngaubos)   !zboson mass
          double precision massfermion(3,4)  !fermion masses
          double precision widthhiggs(3)    !higgs width
          double precision widthgaubosons(ngaubos)   !zboson width
          double precision widthfermion(3,4)  !fermion widthes
C
          common/masses/masshiggs,massgaubosons,massfermion ,
     >                  widthhiggs,widthgaubosons,widthfermion 
!         in this common all the  masses of the particles are transferred.
C         Wishing to add a new particle change here


         integer nnaux
         parameter (nnaux=ngaubos**3)
         integer ndathig
         parameter (ndathig=3)         
         integer ndatyuk
         parameter (ndatyuk=3**2*4**2)         
         integer ndatvma
         parameter (ndatvma=3**2*4**2*ngaubos)         
         integer ndathga
         parameter (ndathga=3*ngaubos**2)
C
          data masshiggs/3*0./
          data massgaubosons/ngaubos*0./
          data massfermion/12*0./
          data widthhiggs/3*0./
          data widthgaubosons/ngaubos*0./
          data widthfermion/12*0./
C
          data selfhiggs/ndathig*0./
          data yukawa/ndatyuk*0./
          data vectorial/ndatvma*0./
          data assial/ndatvma*0./
          data selfgauge/nnaux*0./
          data higgsgauge/ndathga*0./
C
          double precision apar
          common/alphapar/apar(100)
c
          integer init
          data init/0/
          save init
C
          if(init.eq.1) return
C
          init=1

          stetaw2 = 0.2311914d0                 !from PDG
          stetaw  = sqrt(stetaw2)
          ctetaw  = sqrt(1.d0-stetaw2)
          gsu2l   = apar(20)/stetaw             !apar(20) = electron charge
          gbar    = gsu2l/ctetaw
    
c  gauge masses and widths
          massgaubosons(14)    = apar(1)*re     !U mass
          widthgaubosons(14)   = apar(2)*re     !U Width

          massgaubosons(1)     = 91.1876d0*re   !Z mass from PDG
          widthgaubosons(1)    = 2.4952d0*re    !Z width

c lepton masses
          massfermion(1,4)     = apar(11)       !e  mass
          massfermion(2,4)     = apar(12)       !mu mass

c Couplings
C   U boson
         vectorial(14,1,4,1,4) = apar(21)*re    !U \bar e e    V
         vectorial(14,2,4,2,4) = apar(21)*re    !U \bar mu mu  V 
         assial(14,1,4,1,4)    = apar(22)*re    !U \bar e e    A 
         assial(14,2,4,2,4)    = apar(22)*re    !U \bar mu mu  A

C   Photon
         vectorial(10,1,4,1,4) = apar(20)*re    !A \bar e  e
         vectorial(10,2,4,2,4) = apar(20)*re    !A \bar mu mu

C   Z boson
         vectorial(1,1,4,1,4)  = -gbar*(-1.d0+stetaw2*4.d0)/4.d0*re !Z \bar  e e    V
         vectorial(1,2,4,2,4)  = -gbar*(-1.d0+stetaw2*4.d0)/4.d0*re !Z \bar \mu \mu V 
         assial(1,1,4,1,4)     = -gbar/4.d0*re  !Z \bar e e     A
         assial(1,2,4,2,4)     = -gbar/4.d0*re  !Z \bar \mu \mu A

         return
         end

















C***********************************************************************
          subroutine allcouplings           
C***********************************************************************
C
C This subroutine returns the couplings of the model
C 
          implicit none
C
          integer ngaubos
          parameter (ngaubos=14)
          complex*16 re,im
          parameter(re=(1.,0.))
          parameter(im=(0.,1.))
C
          double precision conver       !to convert to the chosen mass units
          parameter (conver=1.d0)       !25 GeV units
          double precision minfer       !conventionally no mass fermion is less
C                                        than minfer
          parameter (minfer=0.2*conver) !200 MeV limit
C
          double precision higgsvev     !higgs vacuum expectation value
          double precision gstrong      !SU(3) gauge coupling
          double precision gsu2l        !SU(2)_L gauge coupling
          double precision gu1y         !U(1)_Y gauge coupling
          double precision gbar
          double precision ctetaw
          double precision stetaw       !Weinberg angle
          complex*16 vud       !V_{ud}  Cabibbo-Kobayashi-Maskawa entry
          complex*16 vus
          complex*16 vub
          complex*16 vcd
          complex*16 vcs
          complex*16 vcb
          complex*16 vtd
          complex*16 vts
          complex*16 vtb
          complex*16 vdu
          complex*16 vdc
          complex*16 vdt
          complex*16 vsu
          complex*16 vsc
          complex*16 vst
          complex*16 vbu
          complex*16 vbc
          complex*16 vbt
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C COUPLINGS.COMM                                                            C
C                                                                           C
C Containing the common for coupling constan. Shared by the subroutines     C
C ITERA and COUPLINGS                                                       C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          complex*16 selfhiggs(3)     !higgs and auxiliary-higgs selfcoupling
C                                      constant
          complex*16 yukawa(3,4,3,4)  !yukawa couplings
          complex*16 vectorial(ngaubos,3,4,3,4)   !V fermions-gauge-bosons 
C                                                  interactions
          complex*16 assial(ngaubos,3,4,3,4)     !V fermions-gauge-bosons 
C                                                  interactions
          complex*16 selfgauge(ngaubos,ngaubos,ngaubos)   !gauge-bosons 
C                                                          self-interactions
          complex*16 higgsgauge(3,ngaubos,ngaubos)  !higgs-gauge bosons 
C                                                    couplings
C
          common/coupconst/selfhiggs,yukawa,vectorial,selfgauge,
     >                    higgsgauge,assial   !add here new couplings
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C MASSES.COMM                                                               C
C                                                                           C
C Containing the common for particles masses. Shared by the subroutines     C
C ITERA and COUPLINGS                                                       C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          double precision masshiggs(3)             !higgs mass
          double precision massgaubosons(ngaubos)   !zboson mass
          double precision massfermion(3,4)         !fermion masses
          double precision widthhiggs(3)            !higgs width
          double precision widthgaubosons(ngaubos)  !zboson width
          double precision widthfermion(3,4)        !fermion widthes
C
!in this common all the  masses of the particles are transferred.
          common/masses/masshiggs,massgaubosons,massfermion ,
     >                  widthhiggs,widthgaubosons,widthfermion 
C       Wishing to add a new particle change here


         integer nnaux
         parameter (nnaux=ngaubos**3)
         integer ndathig
         parameter (ndathig=3)         
         integer ndatyuk
         parameter (ndatyuk=3**2*4**2)         
         integer ndatvma
         parameter (ndatvma=3**2*4**2*ngaubos)         
         integer ndathga
         parameter (ndathga=3*ngaubos**2)
C
c          data masshiggs/3*0./
c          data massgaubosons/ngaubos*0./
c          data massfermion/12*0./
c          data widthhiggs/3*0./
c          data widthgaubosons/ngaubos*0./
c          data widthfermion/12*0./
C
c          data selfhiggs/ndathig*0./
c          data yukawa/ndatyuk*0./
c          data vectorial/ndatvma*0./
c          data assial/ndatvma*0./
c          data selfgauge/nnaux*0./
c          data higgsgauge/ndathga*0./
C
c          call warning
C
c          goto 1
C
          gstrong=1.
          gsu2l=0.651698d0
          gu1y =0.357213d0
          gbar =sqrt(gsu2l**2+gu1y**2)
          ctetaw=gsu2l/gbar
          stetaw=gu1y/gbar
          higgsvev=246.221d0*conver
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C  MASSES.MATRIX, used in COUPLINGS                                         C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C Masses, units 25 GeV. The constant CONVER is used to allow the possibility
C to choose different units
C
          masshiggs(1)=75.*conver  !at some point we need to fix values 
C                                   and normalization
C
          massfermion(3,1)=175.d0*conver      !top mass
          massfermion(3,2)=4.8d0*conver       !bottom mass
          massfermion(3,3)=minfer             !\nu_\tau mass conventionally
C                                              no mass is less than minfer
          massfermion(3,4)=1.7771d0*conver    !\tau mass      
          massfermion(2,1)=0.d0 !1.3d0*conver !charm mass
          massfermion(2,2)=0.3d0 !minfer      !strange mass
          massfermion(2,3)=0.7d0 !minfer      !\nu_\mu mass
          massfermion(2,4)=0.105658389d0*conver !\mu mass
          massfermion(1,1)=0.d0   !5.d-3*conver !up mass
          massfermion(1,2)=0.d0   !1.d-2*conver !down mass
          massfermion(1,3)=0.d0  !minfe       !\nu_e mass
          massfermion(1,4)=0.d0  !minfer      !e^- mass
C
C          massgaubosons(1)=gbar*higgsvev/2.  !Z mass
          massgaubosons(1)=91.1888d0*conver   !Z mass LEPII agreed
C          massgaubosons(2)=gsu2l*higgsvev/2. !W^+ mass
          massgaubosons(2)=80.23d0*conver     !W^+ mass LEPII agreed
          massgaubosons(3)=massgaubosons(2)   !W^- mass 
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C  WIDTHS.MATRIX, used in COUPLINGS
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C widthes, units 25 GeV. The constant CONVER is used to allow the possibility
C to choose different units
C
          widthhiggs(1)=1.*conver     !at some point we need to fix values
C                                      and normalization
C
c          widthfermion(3,1)=5.*conver         !top width
c          widthfermion(3,2)=0.5*conver        !bottom width
c          widthfermion(3,3)=0.5*conver       !\nu_\tau width  Conventionally
C                                               no width is less than 50 MeV
c          widthfermion(3,4)=0.05*conver       !\tau width      
c          widthfermion(2,1)=0.05*conver       !charm width
c          widthfermion(2,2)=0.05*conver       !strange width
c          widthfermion(2,3)=0.05*conver       !\nu_\mu width
c          widthfermion(2,4)=0.0005*conver     !\mu width
c          widthfermion(1,1)=0.05*conver       !up width
c          widthfermion(1,2)=0.05*conver       !down width
c          widthfermion(1,3)=0.05*conver       !\nu_e width
c          widthfermion(1,4)=0.05*conver       !e^- width
C
          widthgaubosons(1)=2.4974d0*conver   !Z width
          widthgaubosons(2)=2.03367d0*conver  !W^+ width
          widthgaubosons(3)=widthgaubosons(2) !W^- width
C
C Coupling constants
C
c          selfhiggs(1)=masshiggs(1)**2./higgsvev/2.*re     
                    !higgs trilinear self coupling
c          selfhiggs(2)= re                               !Y_h h h coupling
c          selfhiggs(3)=masshiggs(1)**2./higgsvev**2/8.*re  !X_h h h coupling
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C YUKAWA.MATRIX used in COUPLINGS
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
c          yukawa(3,1,3,1)=175./higgsvev*re*conver           !top yukawa
c          yukawa(3,2,3,2)=4.3/higgsvev*re*conver            !bottom yukawa
c          yukawa(3,3,3,3)=0.5/higgsvev*re*conver            !\nu_\tau yukawa
c          yukawa(3,4,3,4)=1.7771/higgsvev*re*conver         !\tau yukawa
c          yukawa(2,1,2,1)=1.3/higgsvev*re*conver            !charm yukawa
c          yukawa(2,2,2,2)=0.5/higgsvev*re*conver            !strange yukawa
c          yukawa(2,3,2,4)=0.5/higgsvev*re*conver            !\nu_\mu yukawa
c          yukawa(2,4,2,4)=0.5/higgsvev*re*conver            !muon yukawa
c          yukawa(1,1,1,1)=0.5/higgsvev*re*conver            !up yukawa
c          yukawa(1,2,1,2)=0.5/higgsvev*re*conver            !down yukawa
c          yukawa(1,3,1,3)=0.5/higgsvev*re*conver            !\nu_e yukawa
c          yukawa(1,4,1,4)=0.5/higgsvev*re*conver            !electron yukawa

C
          vud=1.d0*re
          vus=0.d0*re
          vub=0.d0*re
          vcd=0.d0*re
          vcs=1.d0*re
          vcb=0.d0*re
          vtd=0.d0*re
          vts=0.d0*re
          vtb=1.d0*re
          vdu=1.d0*re
          vsu=0.d0*re
          vbu=0.d0*re
          vdc=0.d0*re
          vsc=1.d0*re
          vbc=0.d0*re
          vdt=0.d0*re
          vst=0.d0*re
          vbt=1.d0*re
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C VECTORIAL.MATRIX used in COUPLINGS                                        C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          vectorial(1,3,1,3,1)=-gbar*(1.d0-stetaw**2*8.d0/3.d0)/4.d0*re
             !Z \bar t t     V coupling
          vectorial(1,2,1,2,1)=-gbar*(1.d0-stetaw**2*8.d0/3.d0)/4.d0*re
             !Z \bar c c     V coupling

          vectorial(1,3,2,3,2)=-gbar*(-1.d0+stetaw**2*4.d0/3.d0)/4.d0*re
             !Z \bar b b     V coupling
          vectorial(1,2,2,2,2)=-gbar*(-1.d0+stetaw**2*4.d0/3.d0)/4.d0*re
             !Z \bar s s     V coupling
          vectorial(1,1,2,1,2)=-gbar*(-1.d0+stetaw**2*4.d0/3.d0)/4.d0*re
             !Z \bar d d     V coupling
          vectorial(1,3,3,3,3)=-gbar*(1.d0)/4.d0*re
             !Z \bar \nu_\tau \nu_\tau     V coupling
          vectorial(1,2,3,2,3)=-gbar*(1.d0)/4.d0*re
             !Z \bar \nu_\mu \nu_\mu     V coupling
          vectorial(1,1,3,1,3)=-gbar*(1.d0)/4.d0*re
             !Z \bar \nu_e \nu_e     V coupling
          vectorial(1,3,4,3,4)=-gbar*(-1.d0+stetaw**2*4.d0)/4.d0*re
             !Z \bar \tau \tau    V 
          vectorial(1,2,4,2,4)=-gbar*(-1.d0+stetaw**2*4.d0)/4.d0*re
             !Z \bar \mu \mu     V 
          vectorial(1,1,4,1,4)=-gbar*(-1.d0+stetaw**2*4.d0)/4.d0*re
             !Z \bar  e e  V 
          vectorial(10,3,1,3,1)=-gbar*ctetaw*stetaw*2.d0/3.d0*re
             !A \bar t t     V coupling
          vectorial(10,2,1,2,1)=-gbar*ctetaw*stetaw*2.d0/3.d0*re
             !A \bar c c     V coupling
          vectorial(10,1,1,1,1)=-gbar*ctetaw*stetaw*2.d0/3.d0*re
             !A \bar u u     V coupling
          vectorial(10,3,2,3,2)=gbar*ctetaw*stetaw/3.d0*re
             !A \bar b b     V coupling
          vectorial(10,2,2,2,2)=gbar*ctetaw*stetaw/3.d0*re
             !A \bar s s     V coupling
          vectorial(10,1,2,1,2)=gbar*ctetaw*stetaw/3.d0*re
             !A \bar d d     V coupling
          vectorial(10,3,4,3,4)=gbar*ctetaw*stetaw*re
             !A \bar \tau \tau     V coupling
          vectorial(10,2,4,2,4)=gbar*ctetaw*stetaw*re
             !A \bar \mu \mu       V coupling
          vectorial(10,1,4,1,4)=gbar*ctetaw*stetaw*re
             !A \bar e e           V coupling
          vectorial(11,1,1,1,1)=-gstrong*re  !gluon \bar u u   V coupling
          vectorial(11,1,2,1,2)=-gstrong*re  !gluon \bar d d   V coupling
          vectorial(11,2,1,2,1)=-gstrong*re  !gluon \bar c c   V coupling
          vectorial(11,2,2,2,2)=-gstrong*re  !gluon \bar s s   V coupling
          vectorial(11,3,1,3,1)=-gstrong*re  !gluon \bar t t   V coupling
          vectorial(11,3,2,3,2)=-gstrong*re  !gluon \bar b b   V coupling
C
          vectorial(2,3,1,3,2)=-gsu2l/sqrt(2.d0)/2.d0*vtb
             !W^+ \bar t b     V coupling
          vectorial(3,3,2,3,1)=-gsu2l/sqrt(2.d0)/2.d0*vbt
             !W^- \bar b t     V coupling
          vectorial(2,3,1,2,2)=-gsu2l/sqrt(2.d0)/2.d0*vts
             !W^+ \bar t s     V coupling
          vectorial(3,2,2,3,1)=-gsu2l/sqrt(2.d0)/2.d0*vst
             !W^- \bar s t     V coupling
          vectorial(2,3,1,1,2)=-gsu2l/sqrt(2.d0)/2.d0*vtd
             !W^+ \bar t d     V coupling
          vectorial(3,1,2,3,1)=-gsu2l/sqrt(2.d0)/2.d0*vdt
             !W^- \bar d t     V coupling
C
          vectorial(2,2,1,3,2)=-gsu2l/sqrt(2.d0)/2.d0*vcb
             !W^+ \bar c b     V coupling
          vectorial(3,3,2,2,1)=-gsu2l/sqrt(2.d0)/2.d0*vbc
             !W^- \bar b c     V coupling
          vectorial(2,2,1,2,2)=-gsu2l/sqrt(2.d0)/2.d0*vcs
             !W^+ \bar c s     V coupling
          vectorial(3,2,2,2,1)=-gsu2l/sqrt(2.d0)/2.d0*vsc
             !W^- \bar s c     V coupling
          vectorial(2,2,1,1,2)=-gsu2l/sqrt(2.d0)/2.d0*vcd
             !W^+ \bar c d     V coupling
          vectorial(3,1,2,2,1)=-gsu2l/sqrt(2.d0)/2.d0*vdc
             !W^- \bar d c     V coupling
C
          vectorial(2,1,1,3,2)=-gsu2l/sqrt(2.d0)/2.d0*vub
             !W^+ \bar u b     V coupling
          vectorial(3,3,2,1,1)=-gsu2l/sqrt(2.d0)/2.d0*vbu
             !W^- \bar b u     V coupling
          vectorial(2,1,1,2,2)=-gsu2l/sqrt(2.d0)/2.d0*vus
             !W^+ \bar u b     V coupling
          vectorial(3,2,2,1,1)=-gsu2l/sqrt(2.d0)/2.d0*vsu
             !W^- \bar b u     V coupling
          vectorial(2,1,1,1,2)=-gsu2l/sqrt(2.d0)/2.d0*vud
             !W^+ \bar u b     V coupling
          vectorial(3,1,2,1,1)=-gsu2l/sqrt(2.d0)/2.d0*vdu
             !W^- \bar b u     V coupling
C
          vectorial(2,3,3,3,4)=-gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar \nu_\tau \tau    V coupling
          vectorial(3,3,4,3,3)=-gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar \tau \nu_\tau     V coupling
C
          vectorial(2,2,3,2,4)=-gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar \nu_\mu \mu    V coupling
          vectorial(3,2,4,2,3)=-gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar \mu \nu_\mu     V coupling
C
          vectorial(2,1,3,1,4)=-gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar \nu_e e  V coupling
          vectorial(3,1,4,1,3)=-gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar e \nu_e  V coupling
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C ASSIAL.MATRIX used in COUPLINGS                                           C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
          assial(1,3,1,3,1)=gbar/4.d0*re  !Z \bar t t     A coupling
          assial(1,2,1,2,1)=gbar/4.d0*re  !Z \bar c c     A coupling
          assial(1,1,1,1,1)=gbar/4.d0*re  !Z \bar u u     A coupling
          assial(1,3,2,3,2)=-gbar/4.d0*re !Z \bar b b     A coupling
          assial(1,2,2,2,2)=-gbar/4.d0*re !Z \bar s s     A coupling
          assial(1,1,2,1,2)=-gbar/4.d0*re !Z \bar d d     A coupling
          assial(1,3,3,3,3)=gbar/4.d0*re  !Z \bar \nu_\tau \nu_\tauA coupling
          assial(1,2,3,2,3)=gbar/4.d0*re  !Z \bar \nu_\mu \nu_\mu  A coupling
          assial(1,1,3,1,3)=gbar/4.d0*re  !Z \bar \nu_e \nu_e      A coupling
          assial(1,3,4,3,4)=-gbar/4.d0*re  !Z \bar \tau \tau       A coupling
          assial(1,2,4,2,4)=-gbar/4.d0*re  !Z \bar \mu \mu         A coupling
          assial(1,1,4,1,4)=-gbar/4.d0*re  !Z \bar e e             A coupling
C
          assial(2,3,1,3,2)=gsu2l/sqrt(2.d0)/2.d0*vtb!W^+ \bar t b A coupling
          assial(3,3,2,3,1)=gsu2l/sqrt(2.d0)/2.d0*vbt!W^- \bar b t A coupling
          assial(2,3,1,2,2)=gsu2l/sqrt(2.d0)/2.d0*vts!W^+ \bar t s A coupling
          assial(3,2,2,3,1)=gsu2l/sqrt(2.d0)/2.d0*vst!W^- \bar s t A coupling
          assial(2,3,1,1,2)=gsu2l/sqrt(2.d0)/2.d0*vtd!W^+ \bar t d A coupling
          assial(3,1,2,3,1)=gsu2l/sqrt(2.d0)/2.d0*vdt!W^- \bar d t A coupling
C
          assial(2,2,1,3,2)=gsu2l/sqrt(2.d0)/2.d0*vcb!W^+ \bar c b     A coupling
          assial(3,3,2,2,1)=gsu2l/sqrt(2.d0)/2.d0*vbc!W^- \bar b c     A coupling
          assial(2,2,1,2,2)=gsu2l/sqrt(2.d0)/2.d0*vcs!W^+ \bar c s     A coupling
          assial(3,2,2,2,1)=gsu2l/sqrt(2.d0)/2.d0*vsc!W^- \bar s c     A coupling
          assial(2,2,1,1,2)=gsu2l/sqrt(2.d0)/2.d0*vcd!W^+ \bar c d     A coupling
          assial(3,1,2,2,1)=gsu2l/sqrt(2.d0)/2.d0*vdc!W^- \bar d c     A coupling
C
          assial(2,1,1,3,2)=gsu2l/sqrt(2.d0)/2.d0*vub!W^+ \bar u b     A coupling
          assial(3,3,2,1,1)=gsu2l/sqrt(2.d0)/2.d0*vbu!W^- \bar b u     A coupling
          assial(2,1,1,2,2)=gsu2l/sqrt(2.d0)/2.d0*vus!W^+ \bar u b     A coupling
          assial(3,2,2,1,1)=gsu2l/sqrt(2.d0)/2.d0*vsu!W^- \bar b u     A coupling
          assial(2,1,1,1,2)=gsu2l/sqrt(2.d0)/2.d0*vud!W^+ \bar u b     A coupling
          assial(3,1,2,1,1)=gsu2l/sqrt(2.d0)/2.d0*vdu!W^- \bar b u     A coupling
C
          assial(2,3,3,3,4)=gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar \nu_\tau \tau  A coupling
          assial(3,3,4,3,3)=gsu2l/sqrt(2.d0)/2.d0*re
             !W^- \bar \tau \nu_\tau  A coupling
C
          assial(2,2,3,2,4)=gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar \nu_\mu \mu    A coupling
          assial(3,2,4,2,3)=gsu2l/sqrt(2.d0)/2.d0*re
             !W^- \bar \mu \nu_\mu    A coupling
C
          assial(2,1,3,1,4)=gsu2l/sqrt(2.d0)/2.d0*re
             !W^+ \bar \nu_e e        A coupling
          assial(3,1,4,1,3)=gsu2l/sqrt(2.d0)/2.d0*re
             !W^- \bar e \nu_e        A coupling
C


C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C                                                                           C
C SELFGAUGE.MATRIX, used in COUPLINGS                                       C
C                                                                           C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

          selfgauge(1,2,3)   = gsu2l*ctetaw*re
          selfgauge(10,2,3)  = gsu2l*stetaw*re
          selfgauge(4,2,3)   = gsu2l*re          
          selfgauge(5,1,1)   = gsu2l*ctetaw**2*re          
          selfgauge(5,1,10)  = 2.d0*gsu2l*ctetaw*stetaw*re          
          selfgauge(5,10,10) = gsu2l*stetaw**2*re          
          selfgauge(5,2,3)   = gsu2l/2.d0*re  
          selfgauge(6,1,2)   = gsu2l*ctetaw*re          
          selfgauge(6,10,2)  = gsu2l*stetaw*re  
          selfgauge(7,1,3)   = - gsu2l*ctetaw*re          
          selfgauge(7,10,3)  = - gsu2l*stetaw*re          
          selfgauge(8,2,2)   = gsu2l*re          
          selfgauge(9,3,3)   = -gsu2l/2.d0*re          
          selfgauge(11,11,11)   = gstrong*re/6.d0       !glu-glu-glu
          selfgauge(12,11,11)   = gstrong*re/2.d0       !G - glu-glu
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C HIGGSGAUGE.MATRIX used in subroutine COUPLINGS
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
c         higgsgauge(1,1,1)=-gbar*massgaubosons(1)/2.d0*re
c         higgsgauge(1,2,3)=-gsu2l*massgaubosons(2)*re
c         higgsgauge(3,1,1)=-gbar**2/8.d0*re
c         higgsgauge(3,2,3)=-gsu2l**2/4.d0*re
C
 1        continue
          open(75,file='input_pars.dat',status='old')
c           read(75,*)gstrong              !strong couplings constant, REAL
          gstrong=1d0
          read(75,*)massgaubosons(2) !M_W                        REAL
          read(75,*)widthgaubosons(2) !G_W                        REAL
c          read(75,*)gsu2l       !Su2 coupling constant g2   REAL
          gsu2l=0.6517d0
c           read(75,*)massfermion(1,1)     !u mass                     REAL
c           read(75,*)massfermion(1,2)     !d mass                     REAL
c           read(75,*)massfermion(1,3)     !nu mass                     REAL
c           read(75,*)massfermion(1,4)     !e mass                     REAL
c           read(75,*)massfermion(2,1)     !c mass                     REAL
          massfermion(1,1) =0d0
          massfermion(1,2) =0d0
          massfermion(1,3) =0d0
          massfermion(1,4) =0d0
          massfermion(2,1) =0d0
          read(75,*)massfermion(3,2) !b mass                     REAL
          read(75,*)massfermion(3,1) !b mass                     REAL
          gstrong=gstrong/sqrt(2.d0)
          close(75)
C

          selfgauge(1,2,3)   = gsu2l*ctetaw*re
          selfgauge(10,2,3)  = gsu2l*stetaw*re
          selfgauge(4,2,3)   = gsu2l*re          
          selfgauge(5,1,1)   = gsu2l*ctetaw**2*re          
          selfgauge(5,1,10)  = 2.d0*gsu2l*ctetaw*stetaw*re          
          selfgauge(5,10,10) = gsu2l*stetaw**2*re          
          selfgauge(5,2,3)   = gsu2l/2.d0*re  
          selfgauge(6,1,2)   = gsu2l*ctetaw*re          
          selfgauge(6,10,2)  = gsu2l*stetaw*re  
          selfgauge(7,1,3)   = - gsu2l*ctetaw*re          
          selfgauge(7,10,3)  = - gsu2l*stetaw*re          
          selfgauge(8,2,2)   = gsu2l*re          
          selfgauge(9,3,3)   = -gsu2l/2.d0*re          
          selfgauge(11,11,11)   = gstrong*re/6.d0       !glu-glu-glu
          selfgauge(12,11,11)   = gstrong*re/2.d0       !G - glu-glu
C
          vectorial(11,1,1,1,1)=-gstrong*re  
          vectorial(11,1,2,1,2)=-gstrong*re  
          vectorial(11,2,1,2,1)=-gstrong*re  
          vectorial(11,2,2,2,2)=-gstrong*re  
          vectorial(11,3,1,3,1)=-gstrong*re  
          vectorial(11,3,2,3,2)=-gstrong*re  
          massgaubosons(3)=massgaubosons(2)
          widthgaubosons(3)=widthgaubosons(2)
          vectorial(2,1,1,1,2)=-gsu2l/2.d0/sqrt(2.d0)
          vectorial(2,1,3,1,4)=vectorial(2,1,1,1,2)
          vectorial(3,1,2,1,1)=vectorial(2,1,1,1,2)
          vectorial(3,1,4,1,3)=vectorial(2,1,1,1,2)
          assial(2,1,1,1,2)=-vectorial(2,1,1,1,2)
          assial(2,1,3,1,4)=-vectorial(2,1,1,1,2)
          assial(3,1,2,1,1)=-vectorial(2,1,1,1,2)
          assial(3,1,4,1,3)=-vectorial(2,1,1,1,2)
C
c          write(6,*)'coupling constants'
c          write(6,*)
c          write(6,*)'g-strong', sqrt(2.d0)
c          write(6,*)'M_W',massgaubosons(2)
c          write(6,*)'Gamma_W',widthgaubosons(2)
c          write(6,*)'nu e W vector/axial coupling',
c     >               vectorial(2,1,3,1,4)      
c          write(6,*)'u d W vector/axial coupling',
c     >               vectorial(2,1,1,1,2)      
c          write(6,*)'u mass  ',massfermion(1,1)
c          write(6,*)'d mass  ',massfermion(1,2)
c          write(6,*)'nue mass',massfermion(1,3)
c          write(6,*)'e mass  ',massfermion(1,4)
c          write(6,*)'c mass  ',massfermion(2,1)
c          write(6,*)'b mass  ',massfermion(3,2)
C
C
          return
          end


