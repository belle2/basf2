c ======================================================================== c
c                                                                          c
c Monte Carlo event generator for simulation of hadron or muon production  c
c with or without radiated photons in e+e- collisions at high luminosity   c
c meson factories                                                          c
c                                                                          c 
c                         PHOKHARA version 10.0                             c
c                                                                          c
c                           (October 2020)                                c
c                                                                          c
c simulates the production of two, three or four pions, two muons,         c
c two nucleons, two kaons, two lambdas (with their subsequent decays into  c
c pion and proton), eta-pi-pi, pi0-gamma, eta-gamma or etaP-gamma     c
c  together  with zero, one or two hard       c
c photons emitted from the initial state (ISR); it includes virtual and    c
c soft photon corrections to single photon emission at NLO and virtual and c
c soft photon corrections to zero photon emission at NNLO. It includes FSR c
c emission, and ISR-FSR interference of  one  photon in  the two pion,     c
c two charged kaon, proton-anti-proton and two muon modes. Emission of two c
c photons, one from the initial state and the other from the final state,  c
c together with the appropriate virtual corrections, is also included in   c
c the two pion, two charged kaon, proton-anti-proton and  two muon modes.  c
c                                                                          c
c ------------------------------------------------------------------------ c
c   The program is based on the following publications:                    c
c                                                                          c
c Started as EVA:                                                          c
c                                                                          c
c     S.Binner, J.H.Kuehn and K.Melnikov, PLB459(1999)279 [hep-ph/9902399] c
c                                                                          c
c Includes NLO corrections to ISR for large angle photons from             c
c                                                                          c 
c     G.Rodrigo, A.Gehrmann, M.Guilleaume and J.H.Kuehn,                   c 
c         Eur.Phys.J.C22(2001)81 [hep-ph/0106132]                          c
c     G.Rodrigo, APPB32(2001)3833 [hep-ph/0111151]                         c
c     G.Rodrigo, H.Czyz, J.H.Kuehn and M.Szopa,                            c
c         Eur.Phys.J.C24(2002)71 [hep-ph/0112184]                          c
c                                                                          c
c and NLO corrections to ISR for small angle photons                       c
c                                                                          c
c     J.H.Kuehn and G.Rodrigo, Eur.Phys.J.C25(2002)215 [hep-ph/0204283]    c
c                                                                          c
c The implementation of four pion final states follows                     c
c                                                                          c
c     H.Czyz and J.H.Kuehn, Eur.Phys.J.C18(2001)497 [hep-ph/0008262]       c
c                                                                          c
c with improvements described in                                           c
c                                                                          c
c     H.Czyz, A.Grzelinska, J.H.Kuehn and G.Rodrigo,                       c
c         Eur.Phys.J.C27(2003)563 [hep-ph/0212225]                         c
c                                                                          c
c where the actual implementation of FSR and the complete  description  of c
c upgrades from PHOKHARA 1.0 to PHOKHARA 2.0 is presented. This paper pre- c
c sents also a lot of physical analysis concerning  the  radiative  return c
c method.                                                                  c
c                                                                          c
c The implementation of IFSNLO -- a simultaneous emission  of  one  photon c
c from initial state and one photon from  final  state  (plus  appropriate c
c radiative corrections) -- is presented in                                c
c                                                                          c
c     H.Czyz, A.Grzelinska, J.H.Kuehn and G.Rodrigo,                       c
c         Eur.Phys.J.C33 (2003) 333 [hep-ph/0308312].                      c
c     for pions                                                            c
c  and in                                                                  c
c     H.Czyz, A.Grzelinska, J.H.Kuehn and G.Rodrigo,                       c
c         Eur. Phys. J. C39 (2005) 411 [hep-ph/0404078].                   c
c     for muons                                                            c
c                                                                          c
c where the physical significance of these corrections  is  discussed  and c 
c the upgrade to PHOKHARA 3.0 and PHOKHARA 4.0 is described.               c
c                                                                          c
c Implementation of two-nucleon final states can be found in               c
c                                                                          c
c  H. Czyz, J.H. Kuhn, E. Nowak and G. Rodrigo,                            c
c         Eur. Phys. J. C35 (2004) 527 [hep-ph/0403062].                   c
c                                                                          c
c where also a method of separation of nucleon form-factors is proposed.   c   
c                                                                          c
c The upgrades to PHOKHARA 5.0 contain the implementation of radiative     c
c phi decay contribution to e+e- -> pi+ pi- gamma (gamma) reaction         c
c                                                                          c
c  H.Czyz, A.Grzelinska, J.H.Kuehn                                         c
c         Phys. Lett. B611 (2005) 116 [hep-ph/0412239].                    c
c                                                                          c
c and also pi+ pi- pi0,                                                    c
c   in PHOKHARA 5.1 the model for  pi+ pi- pi0 current and one of the      c
c   models describing the radiative phi decay contribution                 c
c   to e+e- -> pi+ pi- gamma (gamma) reaction was changed                  c
c     H.Czyz, A.Grzelinska, J.H.Kuehn and G.Rodrigo,                       c
c           Eur. Phys. J. C47 (2006) 617 [hep-ph/0512180]                  c
c   and             K+K-, K \barK                                          c
c                                                                          c
c  The upgrade to PHOKHARA 6.0 contain the modelling of                    c
c  the  e+e- -> Lambda (-> pi- proton) ani-Lambda (-> pi+ anti-proton)     c
c  implemented at LO.                                                      c
c                                                                          c
c    H.Czyz, A.Grzelinska, J.H.Kuehn                                       c
c       Phys.Rev.D75:074026,2007 [hep-ph/0702122]                          c
c                                                                          c
c  The upgrade to  PHOKHARA 7.0 contains:                                  c
c  1)  new modeling of the 4-pion current                                  c
c       H.Czyz, J.H.Kuehn, A.Wapienik                                      c
c       Phys.Rev.D77:114005,2008 [arXiv:0804.0359]                         c
c  2) new models for 2-pion and 2-kaon hadronic currents                   c
c  J/psi and psi(2S) contributions to 2-pion, 2-kaon, 2-muon final states  c
c    H.Czyz, J.H.Kuehn,                                                    c
c    Phys.Rev.D80:034035,2009 [arXiv:0904.0515 ]                           c
c    H.Czyz, A.Grzelinska, J.H.Kuehn                                       c
c    Phys.Rev.D81:094014,2010 [arXiv:1002.0279]                            c
c  3) 2-versions of vacuum polarisation available now:                     c
c     [1]  http://www-com.physik.hu-berlin.de/fjeger/alphaQEDn.uu          c
c     [2]  VP_HLMNT_v1_3nonr                                               c
c    (version 1.3, no narrow resonances, 10 Mar 2010)                      c
c         Daisuke Nomura and Thomas Teubner, 10 March 2010                 c
c         References:                                                      c
c         K Hagiwara et al., Phys. Rev. D 69 (2004) 093003                 c
c         K Hagiwara et al., Phys. Lett. B 649 (2007) 173-179              c
c         S Actis et al., Eur.\ Phys.\ J.\  C {\bf 66} (2010) 585          c
c         T Teubner, AIP Conf. Proc. 1078 (2009) 102-107                   c
c                                                                          c
c  The upgrade to  PHOKHARA 7.1 contains:                                  c
c                                                                          c
c  eta-pi+-pi- mode added. Model as described in note :                    c
c                                                                          c
c H. Czyz - eta-pi+-pi- in Phokhara 7.1                                    c
c                                                                          c
c distributed with this version and also in  JHEP 1308 (2013) 110          c
c                                                                          c
c  The upgrade to  PHOKHARA 8.0                                            c
c  H. Czyz, M. Gunia, J.H.Kuehn, JHEP 1308 (2013) 110                      c
c                                                                          c
c  allows to generate the events for all implemented modes also without    c
c  photon emission. ISR radiative corrections up to NNLO are implemented   c
c  as well as the FSR for proton - anti-proton final state.                c
c                                                                          c
c  The upgrade to  PHOKHARA 9.0                                            c
c  F. Campanario, H. Czyz, J. Gluza, M. Gunia,                             c
c  T. Riemann, G. Rodrigo, V. Yundin, JHEP 1402 (2014) 114                 c
c                                                                          c
c           contains complete NLO radiative corrections                    c 
c        to e+e- -> mu+ mu- gamma reaction                                 c
c                                                                          c
c  The upgrade to  PHOKHARA 9.1                                            c
c    H. Czyz,  J.H.Kuehn,  S. Tracz, Phys.Rev. D90 (2014) no.11, 114021    c
c                                                                          c
c  The upgrade to PHOKHARA 9.2                                             c 
c    H. Czyz,  J.H.Kuehn,  S. Tracz, Phys.Rev. D94 (2016) no.3, 034033     c                                  
c                                                                          c  
c   New channel was added - production of chi_c1 and chic2                 c
c                                                                          c
c  The upgrade to PHOKHARA 9.3                                             c 
c    H. Czyz,  P.Kisza,  S. Tracz,     Phys.Rev.D 97 (2018) 1, 016006 [arXiv:1711.00820]                      c                                   
c                                                                          c  
c   New channel was added - pi0 (eta,etaP) gamma                           c
c                                                                          c
c   The upgrade to PHOKHARA 10.0                                           c
c    F. Campanario, H. Czyz, J. Gluza, T. Jelinski, G. Rodrigo,            c
c    S. Tracz and D. Zhuridov  Phys.Rev.D 100 (2019) 7, 076004                           c
c                                                                          c
c     Complete NLO radiative corrections to e+ e- -> pi+ pi- gamma reaction    c
c      were added                                                          c
c
c
c
c Further reading concerning the radiative return method and the PHOKHARA  c
c event generator:                                                         c    
c     A Chapter in a review:                                               c
c      Quest for precision in hadronic cross sections at low energy:       c
c      Monte Carlo tools vs. experimental data.                            c
c      By Working Group on Radiative Corrections                           c
c      and Monte Carlo Generators for Low Energies (S. Actis et al.).      c
c      Eur.Phys.J.C66:585-686,2010.                                        c
c                                                                          c
c     J.H.Kuehn,                                                           c
c        Nucl.Phys.Proc.Suppl.98(2001)289 [hep-ph/0101100];                c
c        Eur. Phys. J. 33 (2004) S659                                      c
c        Nucl.Phys.Proc.Suppl. 144 (2005) 214.                             c
c     H.Czyz, J.H.Kuehn and G.Rodrigo,                                     c
c        hep-ph/0205097;                                                   c
c        Nucl.Phys.Proc.Suppl.123(2003)167 [hep-ph/0210287];               c
c        Nucl.Phys.Proc.Suppl.116(2003)249 [hep-ph/0211186];               c
c     H.Czyz, A.Grzelinska,                                                c
c        Acta Phys.Polon.B34 (2003) 5219 [hep-ph/0310341];                 c
c        hep-ph/0402030;                                                   c
c        Acta Phys.Polon.B38:2989-2998,2007 [arXiv:0707.1275]              c
c     H.Czyz, A.Grzelinska, A. Wapienik                                    c
c        Acta Phys.Polon.B38:3491,2007 [arXiv:0710.4227]                   c
c        Nucl.Phys.Proc.Suppl.189:216-221,2009                             c
c     G.Rodrigo,                                                           c
c        hep-ph/0311158;                                                   c
c     H.Czyz, A.Grzelinska, J.H.Kuehn and G.Rodrigo,                       c
c        Nucl.Phys.Proc.Suppl. 131 (2004) 39  [hep-ph/0312217].            c
c     H.Czyz and E. Nowak-Kubat,                                           c
c        Acta Phys.Polon.B34 (2003) 5231 [hep-ph/0310335];                 c
c        Acta Phys.Polon.B36 (2005) 3425 [hep-ph/0510287];                 c 
c        Phys. Lett. B634 (2006) 493 [hep-ph/0601169];                     c
c     H.Czyz, A.Grzelinska and E. Nowak-Kubat,                             c
c        Acta Phys.Polon.B36 (2005) 3403 [hep-ph/0510208];                 c
c     H.Czyz,                                                              c
c        Nucl.Phys.Proc.Suppl. 162 (2006) 76 [hep-ph/0606227];             c
c        Nucl.Phys.Proc.Suppl.181-182:264-268,2008                         c
c     S. Ivashyn, H. Czyz, A. Korchin                                      c
c        Acta Phys.Polon.B40:3185-3192,2009.                               c
c     H.Czyz, A.Grzelinska                                                 c
c        Nucl.Phys.Proc.Suppl. 218 (2011) 201-206                          c
c     S. Tracz, H.Czyz, Acta Phys.Polon. B44 (2013) 11, 2281-2287          c
c     S. Tracz, H.Czyz, Acta Phys.Polon. B46 (2015) no.11, 2273            c
c ======================================================================== c
c  version 10.0: (c) October 2020,  http://ific.uv.es/~rodrigo/phokhara/  c
c ======================================================================== c

      subroutine phokhara(MODE,XPAR,NPAR) bind(c) !TF added subroutine and parameters for extern C call
      use, intrinsic :: iso_c_binding
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'

      integer(c_int) MODE !TF, steering of weight calculation, generation and finalization
      real(c_double) XPAR(0:99) !TF, real parameters
      integer(c_int) NPAR(0:99) !TF, integer parameters

      integer BNPHOT,BNHAD !TF
      double precision BP1(0:3),BQ1(0:3),BP2(0:9,0:5),BPHOT(0:1,0:3) !TF
      COMMON / MOMSET / BP1,BQ1,BP2,BPHOT,BNPHOT,BNHAD !TF

      real*8 qqmin,qqmax,
     &  cos1min,cos1max,cos2min,cos2max,cos3min,cos3max,
     &  dsigm1,dsigm2,sigma1,sigma2,sigma,dsigm,Ar(14),Ar_r(0:13),
     &   sigma0,dsigm0

      real*8 cqqmin,cqqmax,
     &  ccos1min,ccos1max,ccos2min,ccos2max,ccos3min,ccos3max
      common / kin / cqqmin,cqqmax,
     &  ccos1min,ccos1max,ccos2min,ccos2max,ccos3min,ccos3max !TF

      real*8 int2ph,int2ph_err,inte,inte_s,int1ph,
     & int1ph_err,intezero,int0ph,int0ph_err
      real*8 sum_MC,sum_MC_err,energi_m
      integer nm,i,s_seed(0:104)
      integer*8 nges,k,j,n_ph
      character outfile*20

      integer nmaxtrials !TF
      common / maxtrials / nmaxtrials !TF
      integer nmaxsearch !TF
      common / maxsearch / nmaxsearch !TF
      common / eventcount / nges !TF

      common / intsigma / int2ph,int2ph_err,int1ph,
     & int1ph_err,int0ph,int0ph_err !T

      integer acc !TF
      
      real*8 ecm !TF
      common / beam / ecm !TF
c
      common/muonfsr/inte
      common/virtsoft/inte_s
      common/zeroph/intezero
      
      
c
c      open(16,file='cross1.dat',ACCESS='append')
c      open(17,file='cross2.dat',ACCESS='append')
c --- reads the seed ------
c      open(9,file='seed.dat',status='old')
c      open(10,file='phokhara.out')!,status='new')
c      read(9,*)s_seed
c      call rlxdresetf(s_seed)
c      call rlxdinit(1,32767)    
c --- input parameters ----------------------------
      

      if(MODE.eq.-1)then
      
        nmaxtrials = NPAR(1)  ! maximum number of trials per event
        nmaxsearch = NPAR(2)  ! events used to search cross section maximum
        pion       = NPAR(20) ! final state (very intuitive name...)
        ph0        = NPAR(30) ! Born: 1ph(0)radiative return; Born: 0ph(1) scan
        nlo        = NPAR(31) ! radiative return mode: Born(0), NLO(1)
        fsr        = NPAR(32) ! ISR only(0), ISR+FSR(1), ISR+INT+FSR(2)
        fsrnlo     = NPAR(33) ! yes(1), no(0)
        ivac       = NPAR(34) ! no(0),yes; by Fred Jegerlehner(1),yes;by Thomas Teubner(2)
        FF_Pion    = NPAR(35) ! KS Pionformfactor(0), GS Pionformfactor(1) old, GS Pionformfactor new(2)
        FF_kaon    = NPAR(36) ! KaonFormFactor constrained (0),KaonFormFactor unconstrained (1),KaonFormFactor old (2)
        f0_model   = NPAR(37) ! f0+f0(600): KK model(0), no structure(1), no f0+f0(600)(2), f0 KLOE(3)
        narr_res   = NPAR(38) ! no narrow resonances (0), J/Psi (1), Psi(2S) (2) (narro resonances only for pion = 0, 1, 6, 7
        FF_pp      = NPAR(39) ! ProtonFormFactor old(0), ProtonFormFactor new(1)        
        nlo2       = NPAR(40) ! full NLO : No(0), Yes(1)
        chi_sw     = NPAR(50) ! Radiative return(0), Chi production(1), Radiative return + Chi production (2).
        be_r       = NPAR(51) ! Chi production without beam resolution(0), with beam resolution(1)
        
        ecm        = XPAR(0)  ! CMS energy
        w          = XPAR(11) ! soft photon cutoff
        q2min      = XPAR(15) ! minimal  hadrons(muons)-gamma-inv mass squared 
        q2_min_c   = XPAR(16) ! minimal inv. mass squared of the hadrons(muons)
        q2_max_c   = XPAR(17) ! maximal inv. mass squared of the hadrons(muons)
        gmin       = XPAR(18) ! minimal photon energy/missing energy
        phot1cut   = XPAR(20) ! minimal photon angle/missing momentum angle
        phot2cut   = XPAR(21) ! maximal photon angle/missing momentum angle
        pi1cut     = XPAR(22) ! minimal hadrons(muons) angle
        pi2cut     = XPAR(23) ! maximal hadrons(muons) angle
        pi2cut     = XPAR(23) ! maximal hadrons(muons) angle
        beamres    = XPAR(30) ! beam resolution for pion==11 and pion==12 only
        
        call input(nges,nm,outfile)

        nges=0
      endif
c      energi_m=sqrt(Sp)

c --- open output file for generated momenta ------
!      if(iprint.ne.0) open (10,file=outfile,status='new')

      if(MODE.eq.-1) then
c --- print run data ------------------------------
      write(*,*) '----------------------------------------------------'

      if(ph0.eq.0)then
      if (pion.eq.0) then 
         write(*,*) '     PHOKHARA 10.0 : e^+ e^- -> mu^+ mu^- gamma'
      elseif (pion.eq.1) then
         write(*,*) '     PHOKHARA 10.0: e^+ e^- -> pi^+ pi^- gamma' 
      elseif (pion.eq.2) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> pi^+ pi^- 2pi^0 gamma' 
      elseif (pion.eq.3) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> 2pi^+ 2pi^- gamma'
      elseif (pion.eq.4) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> p pbar gamma' 
      elseif (pion.eq.5) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> n nbar gamma'  
      elseif (pion.eq.6) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> K^+ K^- gamma' 
      elseif (pion.eq.7) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> K_0 K_0bar gamma'    
      elseif (pion.eq.8) then
         write(*,*)
     1    '   PHOKHARA 10.0: e^+ e^- -> pi^+ pi^- pi^0 gamma'  
      elseif (pion.eq.9) then
         write(*,*) 'PHOKHARA 10.0 : e^+ e^- ->'
      write(*,*)'  Lambda (-> pi^- p) Lambda bar (-> pi^+ pbar) gamma'
      elseif (pion.eq.10) then
         write(*,*)
     1    '   PHOKHARA 10.0: e^+ e^- -> pi^+ pi^- eta gamma'
         elseif(pion.eq.11) then
      write(*,*) 'PHOKHARA 10.0 : Chi_c1 production'
         elseif(pion.eq.12) then   
      write(*,*) 'PHOKHARA 10.0 : Chi_c2 production'
         elseif(pion.eq.13) then   
      write(*,*) 'PHOKHARA 10.0 : e^+ e^- -> pi^0  gamma gamma' 
         elseif(pion.eq.14) then   
      write(*,*) 'PHOKHARA 10.0 : e^+ e^- -> eta  gamma gamma'
         elseif(pion.eq.15) then   
      write(*,*) 'PHOKHARA 10.0 : e^+ e^- -> etaP  gamma gamma'
      else 
         write(*,*) '     PHOKHARA 10.0: not yet implemented'
         stop        
      endif
      endif
      if((ph0.eq.1).or.(ph0.eq.-1))then
      if (pion.eq.0) then 
         write(*,*) '     PHOKHARA 10.0 : e^+ e^- -> mu^+ mu^-'
      elseif (pion.eq.1) then
         write(*,*) '     PHOKHARA 10.0: e^+ e^- -> pi^+ pi^-'
      elseif (pion.eq.2) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> pi^+ pi^- 2pi^0' 
      elseif (pion.eq.3) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> 2pi^+ 2pi^-' 
      elseif (pion.eq.4) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> p pbar' 
      elseif (pion.eq.5) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> n nbar'  
      elseif (pion.eq.6) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> K^+ K^-' 
      elseif (pion.eq.7) then
         write(*,*) 
     1    '   PHOKHARA 10.0: e^+ e^- -> K_0 K_0bar' 
      elseif (pion.eq.8) then
         write(*,*)
     1    '   PHOKHARA 10.0: e^+ e^- -> pi^+ pi^- pi^0'  
      elseif (pion.eq.9) then
         write(*,*) 'PHOKHARA 10.0 : e^+ e^- ->'
      write(*,*)'  Lambda (-> pi^- p) Lambda bar (-> pi^+ pbar)'
      elseif (pion.eq.10) then
         write(*,*)
     1    '   PHOKHARA 10.0: e^+ e^- -> pi^+ pi^- eta'
      elseif(pion.eq.13) then   
      write(*,*) 'PHOKHARA 10.0 : e^+ e^- -> pi^0 gamma'
      elseif(pion.eq.14) then   
      write(*,*) 'PHOKHARA 10.0 : e^+ e^- -> eta gamma'
      elseif(pion.eq.15) then   
      write(*,*) 'PHOKHARA 10.0 : e^+ e^- -> etaP gamma' 
      else 
         write(*,*) '     PHOKHARA 10.0: not yet implemented'
         stop        
      endif
      endif
c --------------------------------
      write(*,*) '----------------------------------------------------'
      write(*,100)   'cms total energy                       = ',
     &                 dSqrt(Sp),' GeV  '
        if(ph0.eq.0)then
        if(((gmin/2.d0/ebeam).lt.0.0098d0))then
             write(*,*)' minimal missing energy set to small'
             stop
        endif
        write(*,100) 'minimal tagged photon energy           = ',
     &                 gmin,' GeV  '
        write(*,110) 'angular cuts on tagged photon          = ',
     &                 phot1cut,',',phot2cut
        endif
c --------------------------------
      if((pion.eq.0).or.(pion.eq.11).or.(pion.eq.12)) then 
         write(*,110)'angular cuts on muons                  = ',
     &                 pi1cut,',',pi2cut
      elseif (pion.eq.4) then 
         write(*,110)'angular cuts on protons                = ',
     &                 pi1cut,',',pi2cut
      elseif (pion.eq.5) then 
         write(*,110)'angular cuts on neutrons               = ',
     &                 pi1cut,',',pi2cut
      elseif ((pion.eq.6).or.(pion.eq.7)) then 
         write(*,110)'angular cuts on kaons                  = ',
     &                 pi1cut,',',pi2cut
      elseif (pion.eq.9) then 
         write(*,110)'angular cuts on pions and protons      = ',
     &                 pi1cut,',',pi2cut
      elseif (pion.eq.10) then 
         write(*,110)'angular cuts on pions and eta          = ',
     &                 pi1cut,',',pi2cut
      elseif(pion.eq.13)then
         write(*,110)'angular cuts on pion                  = ',
     &                 pi1cut,',',pi2cut
      elseif(pion.eq.14)then
         write(*,110)'angular cuts on eta                  = ',
     &                 pi1cut,',',pi2cut
      elseif(pion.eq.15)then
         write(*,110)'angular cuts on etaP                  = ',
     &                 pi1cut,',',pi2cut
      else
         write(*,110)'angular cuts on pions                  = ',
     &                 pi1cut,',',pi2cut
      endif
      if(ph0.eq.0)then
         if ((pion.eq.0).or.(pion.eq.11).or.(pion.eq.12)) then  
         write(*,*)'min. muons-tagged photon inv.mass^2    = ',
     &                 q2min,' GeV^2' 
         elseif (pion.eq.4) then  
         write(*,*)'min. protons-tagged photon inv.mass^2  = ',
     &                 q2min,' GeV^2' 
         elseif (pion.eq.5) then  
         write(*,*)'min. neutrons-tagged photon inv.mass^2 = ',
     &                 q2min,' GeV^2' 
         elseif ((pion.eq.6).or.(pion.eq.7)) then  
         write(*,*)'min. kaons-tagged photon inv.mass^2    = ',
     &                 q2min,' GeV^2' 
         elseif (pion.eq.9) then  
         write(*,*)' min. lambdas-tagged photon inv.mass^2 = ',
     &                 q2min,' GeV^2' 
         elseif (pion.eq.10) then  
         write(*,*)' min. pi-pi-eta-tagged photon inv.mass^2 = ',
     &                 q2min,' GeV^2' 
         elseif (pion.eq.13) then  
         write(*,*)' min. pion-tagged photon inv.mass^2 = ',
     &                 q2min,' GeV^2'
         elseif (pion.eq.14) then  
         write(*,*)' min. eta-tagged photon inv.mass^2 = ',
     &                 q2min,' GeV^2'
         elseif (pion.eq.15) then  
         write(*,*)' min. etaP-tagged photon inv.mass^2 = ',
     &                 q2min,' GeV^2' 
         else
         write(*,*)'min. pions-tagged photon inv.mass^2    = ',	 
     &                 q2min,' GeV^2' 
         endif
      endif
c
c --- book histograms -----------------------------
c      call inithisto
c      call inithistoMC
c --- set cuts ------------------------------------
      cos1min = dCos(phot2cut*pi/180.d0)     ! photon1 angle cuts in the 
      cos1max = dCos(phot1cut*pi/180.d0)     ! LAB rest frame            
      cos2min = -1.d0                        ! photon2 angle limits      
      cos2max =  1.d0                        !                           
      cos3min = -1.d0                        ! hadrons/muons angle limits    
      cos3max =  1.d0                        ! in their rest frame
      if ((pion.eq.0).or.(pion.eq.11).or.(pion.eq.12)) then                    ! virtual photon energy cut 
         qqmin = 4.d0*mmu*mmu
      elseif (pion.eq.1) then
         qqmin = 4.d0*mpi*mpi
      elseif (pion.eq.2) then 
         qqmin = 4.d0*(mpi+mpi0)**2
      elseif (pion.eq.3) then 
         qqmin = 16.d0*mpi*mpi
      elseif (pion.eq.4) then 
         qqmin = 4.d0*mp*mp
      elseif (pion.eq.5) then 
         qqmin = 4.d0*mnt*mnt
      elseif (pion.eq.6) then 
         qqmin = 4.d0*mKp*mKp
      elseif (pion.eq.7) then 
         qqmin = 4.d0*mKn*mKn
      elseif (pion.eq.8) then
         qqmin = (2.d0*mpi+mpi0)**2
      elseif (pion.eq.9) then
         qqmin = 4.d0*mlamb*mlamb
      elseif (pion.eq.10) then
         qqmin = (2.d0*mpi+meta)**2
      elseif (pion.eq.13) then
         qqmin = (mpi0f)**2
      elseif (pion.eq.14) then
         qqmin = (metaf)**2
      elseif (pion.eq.15) then
         qqmin = (metaP)**2
       else
         continue
      endif
      qqmax = Sp-2.d0*dSqrt(Sp)*gmin         ! if only one photon 
      if ((q2_max_c.lt.qqmax).and.(ph0.eq.0)) qqmax=q2_max_c  ! external cuts      
c -------------------
      if ( (q2_min_c.gt.qqmin).and.
     & (q2_min_c.lt. (Sp*(1.d0-2.d0*(gmin/dSqrt(Sp)+w))) )
     &   .and.(ph0.eq.0) )then 
          qqmin=q2_min_c
      else
         write(*,*)'------------------------------'
         write(*,*)' Q^2_min TOO SMALL'
         write(*,*)' Q^2_min CHANGED BY PHOKHARA = ',qqmin,' GeV^2'
         write(*,*)'------------------------------'
      endif
      
        if((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then
         write(*,*)'-- in modes pion=13,14,15,  --'
         write(*,*)'--  Q^2_min = (m_P)^2 is used in generation  -- ' 
         write(*,*)'-- Please put your cuts when selecting events  --'
         write(*,*)' Q^2_min CHANGED BY PHOKHARA = ',qqmin,' GeV^2'
         write(*,*)'------------------------------'
        endif
            
c -------------------
      if(qqmax.le.qqmin)then
         write(*,*)' Q^2_max to small '
         write(*,*)' Q^2_max = ',qqmax
         write(*,*)' Q^2_min = ',qqmin
         stop
      endif
c -------------------
      if(ph0.eq.0)then
      if((pion.eq.0).or.(pion.eq.11).or.(pion.eq.12)) then
         write(*,100) 'minimal muon-pair invariant mass^2     = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal muon-pair invariant mass^2     = ',
     &                 qqmax,' GeV^2'
      elseif (pion.eq.1) then
         write(*,100) 'minimal pion-pair invariant mass^2     = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal pion-pair invariant mass^2     = ',
     &                 qqmax,' GeV^2'
      elseif (pion.eq.4) then
         write(*,100) 'minimal proton-pair invariant mass^2   = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal proton-pair invariant mass^2   = ',
     &                 qqmax,' GeV^2'
      elseif (pion.eq.5) then
         write(*,100) 'minimal neutron-pair invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal neutron-pair invariant mass^2  = ',
     &                 qqmax,' GeV^2'
      elseif ((pion.eq.6).or.(pion.eq.7)) then
         write(*,100) 'minimal kaon-pair invariant mass^2     = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal kaon-pair invariant mass^2     = ',
     &                 qqmax,' GeV^2'
      elseif(pion.eq.8)then
         write(*,100) 'minimal three-pion invariant mass^2    = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal three-pion invariant mass^2    = ',
     &                 qqmax,' GeV^2'
      elseif(pion.eq.9)then
         write(*,100) 'minimal lambda-pair invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal lambda-pair invariant mass^2  = ',
     &                 qqmax,' GeV^2' 
      elseif(pion.eq.10)then
         write(*,100) 'minimal pi-pi-eta invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal pi-pi-eta invariant mass^2  = ',
     &                 qqmax,' GeV^2' 
      elseif(pion.eq.13)then
         write(*,100) 'minimal pion - photon invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal pion - photon invariant mass^2  = ',
     &                 qqmax,' GeV^2'
      elseif(pion.eq.14)then
         write(*,100) 'minimal eta - photon invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal eta - photon invariant mass^2  = ',
     &                 qqmax,' GeV^2'
      elseif(pion.eq.15)then
         write(*,100) 'minimal etaP - photon invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal etaP - photon invariant mass^2  = ',
     &                 qqmax,' GeV^2'
      else
         write(*,100) 'minimal four-pion invariant mass^2     = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal four-pion invariant mass^2     = ',
     &                 qqmax,' GeV^2'
      endif
      elseif(ph0.eq.1)then
      if (pion.eq.0) then
         write(*,100) 'minimal muon-pair invariant mass^2     = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal muon-pair invariant mass^2     = ',
     &                 Sp,' GeV^2'
      elseif (pion.eq.1) then
         write(*,100) 'minimal pion-pair invariant mass^2     = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal pion-pair invariant mass^2     = ',
     &                 Sp,' GeV^2'
      elseif (pion.eq.4) then
         write(*,100) 'minimal proton-pair invariant mass^2   = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal proton-pair invariant mass^2   = ',
     &                 Sp,' GeV^2'
      elseif (pion.eq.5) then
         write(*,100) 'minimal neutron-pair invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal neutron-pair invariant mass^2  = ',
     &                 Sp,' GeV^2'
      elseif ((pion.eq.6).or.(pion.eq.7)) then
         write(*,100) 'minimal kaon-pair invariant mass^2     = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal kaon-pair invariant mass^2     = ',
     &                 Sp,' GeV^2'
      elseif(pion.eq.8)then
         write(*,100) 'minimal three-pion invariant mass^2    = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal three-pion invariant mass^2    = ',
     &                 Sp,' GeV^2'  
      elseif(pion.eq.9)then
         write(*,100) 'minimal lambda-pair invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal lambda-pair invariant mass^2  = ',
     &                 Sp,' GeV^2' 
      elseif(pion.eq.10)then
         write(*,100) 'minimal pi-pi-eta invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal pi-pi-eta invariant mass^2  = ',
     &                 Sp,' GeV^2'
       elseif(pion.eq.13)then
         write(*,100) 'minimal pion - photon invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal pion - photon invariant mass^2  = ',
     &                 Sp,' GeV^2'
       elseif(pion.eq.14)then
         write(*,100) 'minimal eta - photon invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal eta - photon invariant mass^2  = ',
     &                 Sp,' GeV^2'
       elseif(pion.eq.15)then
         write(*,100) 'minimal etaP - photon invariant mass^2  = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal etaP - photon invariant mass^2  = ',
     &                 Sp,' GeV^2'  
      else
         write(*,100) 'minimal four-pion invariant mass^2     = ',
     &                 qqmin,' GeV^2'
         write(*,100) 'maximal four-pion invariant mass^2     = ',
     &                 Sp,' GeV^2'
      endif
      endif
c -------------------
      if(be_r.eq.0)then
      continue
      elseif(be_r.eq.1)then
          if((pion.eq.11).or.(pion.eq.12))then
            write(*,*) 'Beam resolution is included'
            continue
          else
      write(*,*) 'Beam resolution allowed only for pion=11 or pion=12'
      stop 
      endif
      else
      write(*,*) 'Wrong be_r switch'
      stop
      endif
 
      if(ph0.eq.-1)then
         write(*,*) 'Born'
      elseif(ph0.eq.0)then
       if (nlo.eq.0) then 
         write(*,*) 'Born'
         if(fsrnlo.ne.0)then
           write(*,*)
     1  'WRONG FSRNLO flag - only fsrnlo=0 allowed for Born'
           stop
         endif
       endif
      else !ph0.eq.1
        if (nlo.eq.0) then 
         write(*,*) 'ISR NLO'
        elseif(nlo.eq.1)then
         write(*,*) 'ISR NNLO'
        else
         write(*,*) 'wrong nlo switch'
         stop
        endif
      endif
c -------------------
      if((pion.eq.9).and.(nlo.ne.0)) then
        write(*,*)'WRONG NLO flag'
       if(ph0.eq.0) write(*,*)' - only Born allowed for Lambdas'
       if(ph0.eq.1) write(*,*)' - only NLO allowed for Lambdas'
        write(*,*)'If you feel that you need better precision'
        write(*,*)'please contact the authors'
        stop
      endif

        if(((pion.eq.11).or.(pion.eq.12)).and.(nlo.ne.0))then
        write(*,*) 'WRONG NLO flag'
        write(*,*)'only nlo=0 allowed for chi production'
        write(*,*)'If you feel that you need better precision'
        write(*,*)'please contact the authors'
       stop
       endif
     
        if(((pion.eq.11).or.(pion.eq.12)).and.(ph0.ne.0))then
         write(*,*) 'WRONG NLO flag'
        write(*,*)'only ph0=0 allowed for chi production'
        write(*,*)'please contact the authors'
       stop
       endif

       

c------------------
       if((ph0.eq.1).and.(fsr.ne.0))then
        write(*,*) 'Coulomb factor included'
       endif       
c -------------------
c -------------------
      if(ph0.eq.1)then
      if (nlo.eq.0) write(*,*) 
     &                'NLO:    soft photon cutoff w           = ',w
      if (nlo.eq.1) write(*,*) 
     &                'NNLO:    soft photon cutoff w           = ',w
      endif
c-----------------------
      if((ph0.eq.0).and.(pion.eq.0))then
       if(nlo.eq.0)then
        if(fsrnlo.ne.0)then
         write(*,*)'WRONG combination of FSR, FSRNLO flags'
         stop
        else
          if (fsr.eq.0) then
          continue
          elseif (fsr.eq.1) then
           write(*,*) 'ISR+FSR'
          elseif (fsr.eq.2) then
           write(*,*) 'ISR+INT+FSR'
          else
           write(*,*)
     1   'WRONG FSR flag: interference is included only for nlo=0'
           stop
          endif
         endif
        elseif(nlo.eq.1)then
         if(fsrnlo.eq.1)then
           if(fsr.eq.2)then
            write(*,*)'ISR+FSR+INT'
           elseif(fsr.eq.0)then
            write(*,*)'Wrong switch'
            write(*,*)'Combinations for mu+mu- NLO  mode:'
            write(*,*)'nlo = 1 then'
            write(*,*)'fsr = 0, ifsnlo = 0'
            write(*,*)'fsr = 2, ifsnlo = 1'
            stop
           elseif(fsr.eq.1)then
            write(*,*)'Wrong switch'
            write(*,*)'Combinations for mu+mu- NLO  mode:'
            write(*,*)'nlo = 1 then'
            write(*,*)'fsr = 0, ifsnlo = 0'
            write(*,*)'fsr = 2, ifsnlo = 1'
            stop
           endif
         endif
        endif
        endif

c--------------
c pi+pi-
      if(pion.eq.1)then
        if(nlo2.eq.1)then
          if((fsr.eq.2).and.(fsrnlo.eq.1).and.
     1 (nlo.eq.1).and.(ph0.eq.0))then
             continue
          else
        write(*,*)'only ph0=0, nlo=1,frs=2 and fsrnlo=1
     1  is allowed for nlo2=1'
            stop
          endif
        endif
      endif

      if((ph0.eq.0).and.(pion.eq.1))then
      if(((fsr.eq.1).or.(fsr.eq.2)).and.(fsrnlo.eq.0))then
c          if(fsr.eq.1) write(10,*) 'ISR+FSR'
c          if(fsr.eq.2) write(10,*) 'ISR+INT+FSR'
        continue
       elseif((fsr.eq.1).and.(fsrnlo.eq.1))then
c          if(fsrnlo.eq.1) write(10,*) 'IFSNLO included'
         continue
       elseif((fsr.eq.0).and.(fsrnlo.eq.0))then
        continue
       else
        if(nlo2.eq.1)then
        write(*,*)'full NLO'
           continue
        else
        write(*,*)'WRONG combination of FSR, FSRNLO flags'
        stop
        endif
       endif

          if (fsr.eq.0) then
         write(*,*) 'ISR only'
       elseif (fsr.eq.1) then
         write(*,*) 'ISR+FSR'
       elseif (fsr.eq.2) then
         if (nlo.eq.0) then
            write(*,*) 'ISR+INT+FSR'
         else
             if(nlo2.eq.1)then
                 continue
             else
         write(*,*)
     1   'WRONG FSR flag: interference is included only for nlo=0'
         stop
             endif
         endif
       else
          write(*,*)'WRONG FSR flag', fsr
         stop
       endif
         if(fsrnlo.eq.1) then
            write(*,*)'IFSNLO included'
         endif
        
      endif
c
c ------------------

c---------------------------------------------------------------------
c
      if(ph0.eq.0)then
      if (nlo.eq.1) write(*,*) 
     &                'NLO:    soft photon cutoff w           = ',w
      if ((pion.eq.4).or.(pion.eq.6)) then
c
       if(((fsr.eq.1).or.(fsr.eq.2)).and.(fsrnlo.eq.0))then
        continue
       elseif((fsr.eq.1).and.(fsrnlo.eq.1))then
         continue
       elseif((fsr.eq.0).and.(fsrnlo.eq.0))then
        continue
       else
        write(*,*)'WRONG combination of FSR, FSRNLO flags'
        stop
       endif


       if (fsr.eq.0) then
         write(*,*) 'ISR only'
       elseif (fsr.eq.1) then
         write(*,*) 'ISR+FSR'
       elseif (fsr.eq.2) then
         if (nlo.eq.0) then
            write(*,*) 'ISR+INT+FSR'
         else
         write(*,*)
     1   'WRONG FSR flag: interference is included only for nlo=0'
         stop
         endif
       else
          write(*,*)'WRONG FSR flag', fsr
         stop
       endif
         if(fsrnlo.eq.1) then
            write(*,*)'IFSNLO included'
         endif
       else
        if((fsr.eq.0).and.(fsrnlo.eq.0))then
         write(*,*)'ISR only'
        else
          if((pion.eq.0).or.(pion.eq.1) )then 
          continue
          else
        write(*,*)'FSR is implemented only for pi+pi-, mu+mu-,K+K- and
     1 ppbar modes'
       stop
           endif
       endif
      endif
      endif
c-------------------------------------
c      if((pion.eq.0).and.(ph0.eq.0).and.(fsrnlo.eq.1)
c     $     .and.(fsr.eq.2))then
c      write(*,*)'NLO: ISR + FSR + INT'
c      endif
c ------------------
      if(ivac.eq.0)then
        write(*,*)'Vacuum polarization is NOT included'
      elseif(ivac.eq.1)then
        write(*,*)'Vacuum polarization by Fred Jegerlehner' 
        write(*,*)
     1 'http://www-com.physik.hu-berlin.de/fjeger/alphaQEDn.uu'
      elseif(ivac.eq.2)then
        write(*,*)'Vacuum polarization (VP_HLMNT_v1_3nonr) by' 
        write(*,*)'Daisuke Nomura and Thomas Teubner' 
      else
        write(*,*)'WRONG vacuum polarization switch'
        stop      
      endif
       
c -----------------
      if(pion.eq.1)then
        if(FF_pion.eq.0)then
          write(*,*)'Kuhn-Santamaria PionFormFactor'
        elseif(FF_pion.eq.1)then
          write(*,*)'Gounaris-Sakurai PionFormFactor old'
        elseif(FF_pion.eq.2)then
          write(*,*)'Gounaris-Sakurai PionFormFactor new'
        else
        write(*,*)'WRONG PionFormFactor switch'
        stop      
        endif
c ------
        if(fsr.ne.0)then
         if(f0_model.eq.0)then
           write(*,*)'f0+f0(600): K+K- model'
         elseif(f0_model.eq.1)then
           write(*,*)'f0+f0(600): "no structure" model'
         elseif(f0_model.eq.2)then
           write(*,*)'NO f0+f0(600)'
         elseif(f0_model.eq.3)then
           write(*,*)'only f0, KLOE: Cesare Bini-private communication'
         else
         write(*,*)'WRONG f0+f0(600) switch'
         stop      
         endif
        endif
      endif
c --------------------
      if((pion.eq.6).or.(pion.eq.7))then
        if(FF_kaon.eq.0)then
          write(*,*)'constrained KaonFormFactor'
        elseif(FF_kaon.eq.1)then
          write(*,*)'unconstrained KaonFormFactor'
        elseif(FF_kaon.eq.2)then
          write(*,*)'Kuhn-Khodjamirian-Bruch KaonFormFactor'
        else
          write(*,*)'WRONG KaonFormFactor switch'
          stop      
        endif
      endif
c --------------------
      if((pion.eq.0).or.(pion.eq.1).or.(pion.eq.6).or.(pion.eq.7))then
        if(narr_res.eq.0) then
          continue
        elseif(narr_res.eq.1) then
          write(*,*)'THE NARROW RESONANCE J/PSI INLUDED'
        elseif(narr_res.eq.2) then
          write(*,*)'THE NARROW RESONANCE PSI(2S) INLUDED'
        else
          write(*,*)'wrong flag narr_res: only 0, 1 or 2 allowed'
          stop
        endif
      endif

c --- TF copy some of them to the common block ------
        cqqmin   = qqmin
        cqqmax   = qqmax
        ccos1min = cos1min
        ccos1max = cos1max
        ccos2min = cos2min
        ccos2max = cos2max
        ccos3min = cos3min
        ccos3max = cos3max

      endif !endif MODE.e1.-1

c
c =================================================
c --- finding the maximum -------------------------
      if(MODE.eq.-1)then

      do i = 1,3
         Mmax(i-1) = 1.d0
         gross(i-1) = 0.d0
         klein(i-1) = 0.d0      
      enddo 
      if (ph0.eq.0) Mmax(0)=0.d0   ! 0 photon events are not generated
      if (nlo.eq.0) Mmax(2)=0.d0   ! 2 photon events are not generated
      if  (ph0.eq.-1)then   ! only 0 photon events (LO) are generated
         Mmax(1)=0.d0  
         Mmax(2)=0.d0 
      endif      
      
      endif!TF endif MODE.eq.-1


c =================================================
      if(MODE.eq.-1)then
      i=1
      
      tr(0) = 0.d0
      tr(1) = 0.d0      
      tr(2) = 0.d0
      
      count(0) = 0.d0
      count(1) = 0.d0
      count(2) = 0.d0

c                                                                                                                                                            
      int2ph = 0.d0
      int2ph_err = 0.d0
c                                                                                                                                                            
      int1ph=0.d0
      int1ph_err=0.d0
c
      int0ph = 0d0
      int0ph_err = 0d0
c
c =================================================
c --- beginning the MC loop event generation ------
      
      do j = 1,nmaxsearch
          
         call ranlxdf(Ar_r,1)
         Ar(1) = Ar_r(0)
 
         if (Ar(1).le.(Mmax(0)/(Mmax(1)+Mmax(2)+Mmax(0)))) then 
            count(0)=count(0)+1
            call gen_0ph(i,qqmin,Sp,cos3min,cos3max,acc)
           int0ph = int0ph + intezero
           int0ph_err = int0ph_err + intezero*intezero
         elseif(Ar(1).le.(Mmax(0)+Mmax(1))
     &            /(Mmax(1)+Mmax(2)+Mmax(0)))then
            
            count(1) = count(1)+1.d0
            call gen_1ph(i,qqmin,qqmax,cos1min,cos1max,
     &          cos3min,cos3max,acc)
            
   
         int1ph=int1ph + inte_s
         int1ph_err=int1ph_err + inte_s*inte_s
         

         else
            count(2) = count(2)+1.d0
            call gen_2ph(i,qqmin,cos1min,cos1max,
     &           cos2min,cos2max,cos3min,cos3max,acc)
           int2ph=int2ph + inte
           int2ph_err=int2ph_err + inte*inte
         endif


       
      enddo    
c --- end of the MC loop --------------------------

c =================================================
c --- for the second run ---
      if (i.eq.1) then
         Mmax(0) = gross(0)+.1d0*Sqrt(gross(0)*gross(0))
         Mmax(1) = gross(1)+.05d0*Sqrt(gross(1)*gross(1))
         Mmax(2) = gross(2)+(.03d0+.02d0*Sp)*Sqrt(gross(2)*gross(2)) 
c 
       
         if((pion.eq.1).and.(fsrnlo.eq.1)) Mmax(2)=Mmax(2)*1.5d0
         if((pion.eq.0).and.(fsrnlo.eq.1)) Mmax(2)=Mmax(2)*1.5d0

         if((pion.eq.0).and.(fsr.eq.1).and.(fsrnlo.eq.0)) 
     1                                     Mmax(2)=Mmax(2)*1.2d0
         if((pion.eq.2).or.(pion.eq.3))then
          Mmax(1)=Mmax(1)*1.1d0
          Mmax(2)=Mmax(2)*1.1d0
         endif

         if(pion.eq.8)then
          Mmax(1)=Mmax(1)*1.08d0
          Mmax(2)=Mmax(2)*1.1d0
         endif
         if((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then
            Mmax(1)=Mmax(1)*1.5d0
         endif
      endif

      tr(0) = 0.d0
      tr(1) = 0.d0
      tr(2) = 0.d0

      count(0) = 0.d0
      count(1) = 0.d0
      count(2) = 0.d0
c
      int2ph = 0.d0
      int2ph_err = 0.d0
c
      int1ph=0.d0
      int1ph_err=0.d0
c
      int0ph = 0d0
      int0ph_err = 0d0

      nges=0 !TF number of generated events

      endif !TF MODE.eq.-1

c =================================================
      if(MODE.eq.1)then
      i=2

      do j = 1,nmaxtrials

         call ranlxdf(Ar_r,1)
         Ar(1) = Ar_r(0)

         acc=0

         if (Ar(1).le.(Mmax(0)/(Mmax(1)+Mmax(2)+Mmax(0)))) then
            count(0)=count(0)+1
            call gen_0ph(i,cqqmin,Sp,ccos3min,ccos3max,acc)
           int0ph = int0ph + intezero
           int0ph_err = int0ph_err + intezero*intezero
         elseif(Ar(1).le.(Mmax(0)+Mmax(1))
     &            /(Mmax(1)+Mmax(2)+Mmax(0)))then
            count(1) = count(1)+1.d0
            call gen_1ph(i,cqqmin,cqqmax,ccos1min,ccos1max,
     &          ccos3min,ccos3max,acc)
c
         int1ph=int1ph + inte_s
         int1ph_err=int1ph_err + inte_s*inte_s
c
         else
            count(2) = count(2)+1.d0
            call gen_2ph(i,cqqmin,ccos1min,ccos1max,
     &           ccos2min,ccos2max,ccos3min,ccos3max,acc)
           int2ph=int2ph + inte
           int2ph_err=int2ph_err + inte*inte
         endif

         nges=nges+1

         if(acc.eq.1) then
           call belle2event
           goto 600
         endif

      enddo

      endif !TF MODE.eq.1


      if(MODE.eq.2)then

      if(pion.eq.9)then
         Mmax(0) = Mmax(0) * (1.d0 + alpha_lamb)**2 * ratio_lamb**2
         Mmax(1) = Mmax(1) * (1.d0 + alpha_lamb)**2 * ratio_lamb**2
      endif 
c --- save histograms -----------------------------
c      call endhisto()
c      if(pion.ne.9) call endhistoMC()
c --- value of the cross section ------------------
      sigma0 = 0.d0
      sigma1 = 0.d0
      sigma2 = 0.d0
      dsigm0 = 0.d0
      dsigm1 = 0.d0
      dsigm2 = 0.d0
       
      if ((ph0.eq.0).and.(nlo.eq.0)) then 
            sigma1 = Mmax(1)/count(1)*tr(1)
            dsigm1 = 
     1      Mmax(1)*dSqrt((tr(1)/count(1)-(tr(1)/count(1))**2)/count(1))
      elseif((ph0.eq.0).and.(nlo.eq.1))then
            sigma1 = Mmax(1)/count(1)*tr(1)
            dsigm1 = 
     1      Mmax(1)*dSqrt((tr(1)/count(1)-(tr(1)/count(1))**2)/count(1))
            sigma2 = Mmax(2)/count(2)*tr(2)
            dsigm2 = 
     1      Mmax(2)*dSqrt((tr(2)/count(2)-(tr(2)/count(2))**2)/count(2))
      elseif((ph0.eq.1).and.(nlo.eq.0))then
            sigma0 = Mmax(0)/count(0)*tr(0)
            dsigm0 = 
     1      Mmax(0)*dSqrt((tr(0)/count(0)-(tr(0)/count(0))**2)/count(0))
            sigma1 = Mmax(1)/count(1)*tr(1)
            dsigm1 = 
     1      Mmax(1)*dSqrt((tr(1)/count(1)-(tr(1)/count(1))**2)/count(1))
      elseif(ph0.eq.-1)then
             sigma0 = Mmax(0)/count(0)*tr(0)
             dsigm0 = 
     1      Mmax(0)*dSqrt((tr(0)/count(0)-(tr(0)/count(0))**2)/count(0))
      else
            sigma0 = Mmax(0)/count(0)*tr(0)
            dsigm0 = 
     1      Mmax(0)*dSqrt((tr(0)/count(0)-(tr(0)/count(0))**2)/count(0))
            sigma1 = Mmax(1)/count(1)*tr(1)
            dsigm1 = 
     1      Mmax(1)*dSqrt((tr(1)/count(1)-(tr(1)/count(1))**2)/count(1))
            sigma2 = Mmax(2)/count(2)*tr(2)
            dsigm2 = 
     1      Mmax(2)*dSqrt((tr(2)/count(2)-(tr(2)/count(2))**2)/count(2))
      endif
         sigma = sigma0+sigma1+sigma2
         dsigm = dSqrt(dsigm0**2+dsigm1**2+dsigm2**2) 

        

c-----------------------------------------------------------------------                                                                                     
c                                                                                                                                       
      int2ph=int2ph/count(2)
      int2ph_err=dsqrt(dabs(int2ph_err/count(2)/count(2)
     &    - int2ph*int2ph/count(2)))
c                                                                                                                                         
      int1ph=int1ph/count(1)
      int1ph_err=dsqrt(dabs(int1ph_err/count(1)
     & /count(1) - int1ph*int1ph/count(1)))
c     
      int0ph = int0ph/count(0)
      int0ph_err = dsqrt(dabs(int0ph_err/count(0)/count(0)
     &  - int0ph*int0ph/count(0)))
c
      if (ph0.eq.0)then
       int0ph =  0d0
       int0ph_err = 0d0
       if(nlo.eq.0) then
        int2ph =0d0
        int2ph_err = 0d0
       endif
      elseif(ph0.eq.-1)then
        int1ph =0d0
        int1ph_err = 0d0
c
        int2ph =0d0
        int2ph_err = 0d0
       elseif(ph0.eq.1)then
        if(nlo.eq.0) then
        int2ph =0d0
        int2ph_err = 0d0
        endif
       endif
c
      sum_MC = int0ph + int1ph + int2ph
      sum_MC_err = dsqrt(int2ph_err**2
     & + int1ph_err**2 + int0ph_err**2)
c --- output --------------------------------------
 

      write(*,*) '----------------------------------------------------'      
      write(*,*) int(tr(0)+tr(1)+tr(2)),' total events accepted of '
      write(*,*) int(nges),       ' total events generated'
      write(*,*) int(tr(0)),      ' zero photon events accepted of '
      write(*,*) int(count(0)),   ' events generated'
      write(*,*) int(tr(1)),      ' one photon events accepted of '
      write(*,*) int(count(1)),   ' events generated'
      write(*,*) int(tr(2)),      ' two photon events accepted of '
      write(*,*) int(count(2)),   ' events generated'
      write(*,*)

      write(*,*) 'sigma0(nbarn) = ',sigma0,' +- ',dsigm0
      write(*,*) 'sigma1(nbarn) = ',sigma1,' +- ',dsigm1
      write(*,*) 'sigma2(nbarn) = ',sigma2,' +- ',dsigm2
      write(*,*) 'sigma (nbarn) = ',sigma, ' +- ',dsigm
      write(*,*)
      write(*,*) 'maximum0 = ',gross(0),'  minimum1 = ',klein(0)
      write(*,*) 'Mmax0    = ',Mmax(0) 
      write(*,*) 'maximum1 = ',gross(1),'  minimum1 = ',klein(1)
      write(*,*) 'Mmax1    = ',Mmax(1)            
      write(*,*) 'maximum2 = ',gross(2),'  minimum2 = ',klein(2)
      write(*,*) 'Mmax2    = ',Mmax(2)            
      write(*,*) '----------------------------------------------------'

  200 FORMAT('',A16,G20.10E3,A15,G20.10E3)
  201 FORMAT('',A16,G20.10E3)
      
c      write(16,*) energi_m,sigma,dsigm,sigma0,dsigm0,sigma1,dsigm1

c      
      if(pion.ne.9)then
      write(*,*)'Monte Carlo integrand'
      write(*,*)'sigma0_MC(nbarn) = ',int0ph,'+-',int0ph_err 
      write(*,*)'sigma1_MC(nbarn) = ',int1ph,'+-',int1ph_err
      write(*,*)'sigma2_MC(nbarn) = ',int2ph,'+-',int2ph_err
      write(*,*)'sigma_MC (nbarn) = ',sum_MC,'+-',sum_MC_err
      endif

      endif !TF end MODE.eq.2
      
 600  continue     
       
c      write(17,*) energi_m,sum_MC,sum_MC_err,int0ph,int0ph_err,
c     1 int1ph,int1ph_err
      
  
 100  format (a42,f10.6,a6)
 110  format (a42,f6.1,a1,f6.1)
c --- saves the new seed --------------------------

c      close (9,STATUS='delete')
c      open(9,file='seed.dat',status='new')

c      call rlxdgetf(s_seed)
c      write(9,*)s_seed
c      enddo
      end
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      

c!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
c this part contains vacuum polarization procedures taken from
c  http://www-com.physik.hu-berlin.de/fjeger/alphaQEDn.uu
c look also comments in the included file:
c
      include 'vac_pol_hc1.inc'
c
c to change to different procedure a user should supply complex*16 function
c dggvap(..,..) with '..' to be real*8, plus possibly also some initialization
c in the procedure 'input'   
c
c second option for the vacpol
c======================================================================
c VP_HLMNT_v1_3nonr (version 1.3, no narrow resonances, 10 Mar 2010)
c======================================================================
c
      include 'vp_hlmnt_v1_3nonr_hc.inc'
c
c =================================================
c --- print the momenta of the generated event ----
c =================================================
c
c Mode mu^+ mu^- (pion=0)           |  Mode pi^+ pi^-  (pion=1)
c                                   |
c Mode    2pi^0 pi^+ pi^- (pion=2)  |  Mode    2pi^+ 2pi^-     (pion=3)
c q1,q2 - four momenta of pi^0      |  q1,q4 - four momenta of pi^+ 
c q3    - four momentum of pi^-     |  q2,q3 - four momenta of pi^-
c q4    - four momentum of pi^+     |
c                                   |   Mode pi^+ pi^- pi^0 (pion=8)
c Mode    p + pbar (pion=4)         |   q1 - four momentum of pi^+
c Mode    n + nbar (pion=5)         |   q2 - four momentum of pi^-
c Mode    K^+ + K^-(pion=6)         |   q3 - four momentum of pi^0
c Mode    K^0 + K^0bar (pion=7)
c                        note that for 7: KL KS is written out
c
c Mode  lambda (-> pi^- p) lambda bar (-> pi^+ pbar) (pion=9)
c Mode eta pi+  pi-  (pion=10)
c
      subroutine writeevent()
      include 'phokhara_10.0.inc'

      write(10,*)'-------------------------------------------'
      if(pion.lt.13)then
      write(10,*)'Photon1:',momenta(3,0),momenta(3,1),
     &   momenta(3,2),momenta(3,3)
      write(10,*)'Photon2:',momenta(4,0),momenta(4,1),
     &   momenta(4,2),momenta(4,3)
      endif
      if ((pion.eq.0).or.(pion.eq.11).or.(pion.eq.12)) then 
         write(10,*)'Mu+:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'Mu-:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
      elseif(pion.eq.1)then
         write(10,*)'Pi+:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'Pi-:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
      elseif(pion.eq.2)then
         write(10,*)'Pi0:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'Pi0:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
         write(10,*)'Pi-:    ',momenta(8,0),momenta(8,1),
     &      momenta(8,2),momenta(8,3)
         write(10,*)'Pi+:    ',momenta(9,0),momenta(9,1),
     &      momenta(9,2),momenta(9,3)
      elseif(pion.eq.3)then
         write(10,*)'Pi+:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'Pi-:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
         write(10,*)'Pi-:    ',momenta(8,0),momenta(8,1),
     &      momenta(8,2),momenta(8,3)
         write(10,*)'Pi+:    ',momenta(9,0),momenta(9,1),
     &      momenta(9,2),momenta(9,3)
      elseif(pion.eq.4)then
         write(10,*)'Pbar:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'P:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
      elseif(pion.eq.5)then
         write(10,*)'Nbar:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'N:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
      elseif(pion.eq.6)then
         write(10,*)'K+:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'K-:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
      elseif(pion.eq.7)then
         write(10,*)'KL:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'KS:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
      elseif(pion.eq.8)then
         write(10,*)'Pi+:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'Pi-:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
         write(10,*)'Pi0:    ',momenta(8,0),momenta(8,1),
     &      momenta(8,2),momenta(8,3)
      elseif(pion.eq.9)then
         write(10,*)'Lambda bar:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'Lambda:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
         write(10,*)'Pi+:    ',momenta(8,0),momenta(8,1),
     &      momenta(8,2),momenta(8,3)
         write(10,*)'Pbar:   ',momenta(9,0),momenta(9,1),
     &      momenta(9,2),momenta(9,3)
         write(10,*)'Pi-:    ',momenta(10,0),momenta(10,1),
     &      momenta(10,2),momenta(10,3)
         write(10,*)'P:    ',momenta(11,0),momenta(11,1),
     &      momenta(11,2),momenta(11,3)
      elseif(pion.eq.10)then
         write(10,*)'eta:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
         write(10,*)'Pi+:    ',momenta(7,0),momenta(7,1),
     &      momenta(7,2),momenta(7,3)
         write(10,*)'Pi-:    ',momenta(8,0),momenta(8,1),
     &      momenta(8,2),momenta(8,3)
      elseif(pion.eq.13)then
        write(10,*)'Photon1:',momenta(7,0),momenta(7,1),
     &   momenta(7,2),momenta(7,3)
      write(10,*)'Photon2:',momenta(3,0),momenta(3,1),
     &   momenta(3,2),momenta(3,3)
      write(10,*)'Pi0:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
      elseif(pion.eq.14)then
        write(10,*)'Photon1:',momenta(7,0),momenta(7,1),
     &   momenta(7,2),momenta(7,3)
      write(10,*)'Photon2:',momenta(3,0),momenta(3,1),
     &   momenta(3,2),momenta(3,3)
      write(10,*)'eta:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
      elseif(pion.eq.15)then
       write(10,*)'Photon1:',momenta(7,0),momenta(7,1),
     &   momenta(7,2),momenta(7,3)
      write(10,*)'Photon2:',momenta(3,0),momenta(3,1),
     &   momenta(3,2),momenta(3,3)
      write(10,*)'etaP:    ',momenta(6,0),momenta(6,1),
     &      momenta(6,2),momenta(6,3)
      else
       continue
      endif 
      return
      end
c ======================================================================
c --- generates 0 photon ---------------------------------------------
c ======================================================================
      subroutine gen_0ph(i,qqmin,qqmax,cos3min,cos3max,acc)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'
      real*8 Ar(14),Ar_r(0:13),z,inte,amplit,pom
      real*8 qqmin,qqmax,qq,jac4,helicityampLO,Matrix 
      real*8 q0p,q2p,q0b,q2b,jac0,cos3min,cos3max,cos1,phi1
     & ,jac1,cos3,phi3,qqii
      real*8 costhm_ll,phim_ll,costhp_ll,phip_ll,intezero
      complex*16 Hadronic(0:3,0:3),pionggFF_dprime
      integer i,ru,ll
      logical accepted    
      integer acc
      common/zeroph/intezero
      call ranlxdf(Ar_r,7)
      do ru=0,6
       Ar(ru+1)=Ar_r(ru)
      enddo  

      acc = 0

c --- get the variables ---------------------------
c
       call qquadrat_0(qqmin,qqmax,cos3min,cos3max,Ar
     1,Sp,q0p,q2p,q0b,q2b,cos3,phi3,jac4)

c
      z = Mmax(0)*Ar(6)
c --- real photon1 ---
      momenta(3,0) = 0.d0
      momenta(3,1) = 0.d0
      momenta(3,2) = 0.d0
      momenta(3,3) = 0.d0
c --- real photon2 ---
      momenta(4,0) = 0.d0
      momenta(4,1) = 0.d0
      momenta(4,2) = 0.d0
      momenta(4,3) = 0.d0
c virtual part

c --- 4-momenta in the CMS frame ---
      if((pion.le.1).or.(pion.eq.4).or.(pion.eq.5).or.(pion.eq.6)
     1              .or.(pion.eq.7).or.(pion.eq.9).or.(pion.eq.13).or.
     2 (pion.eq.14).or.(pion.eq.15))then
       call hadronmomenta_0ph(Sp,cos3,phi3)
       call HadronicTensorISR(Sp,q0p,q2p,q0b,q2b,Hadronic)
      elseif((pion.eq.2).or.(pion.eq.3))then
       call HadronicTensorISR(Sp,q0p,q2p,q0b,q2b,Hadronic)
      elseif((pion.eq.8).or.(pion.eq.10))then
       call HadronicTensorISR(Sp,q0p,q2p,q0b,q2b,Hadronic)
      endif  
      if(pion.eq.9)then
       call pionanglesLamb(cos3min,cos3max,costhm_ll,phim_ll,costhp_ll,
     1                     phip_ll)
       call hadronmomenta_Lamb0(Sp,cos3,phi3,costhm_ll,phim_ll,costhp_ll
     1                        , phip_ll)
      endif
      
c
      call testcuts(0,accepted)
c
      if (accepted) then
c  
      amplit = Matrix(Leptonic_epl_emin,Hadronic)/Sp**2

c    
      inte = gev2nbarn * amplit *(1.d0 + Ophvirtsoft)* jac4/(8.d0*Sp) 
      intezero = inte
         if (inte.gt.gross(0)) gross(0) = inte
         if (inte.lt.klein(0)) klein(0) = inte

c --- in the second rund ---
         if (i.eq.2) then
      call addiereMC(intezero,Sp,0)
          if (Mmax(0).lt.inte) write(*,*) 'Warning! Max(0) too small!'
          if (inte.lt.0.d0)   write(*,*) '0ph:Warning! negative weight'
          if (inte.lt.0.d0)   write(*,*) 'inte',inte
  203 FORMAT('',A10,G20.12E3)
c --- event accepted? ---
          if (z.le.inte) then
               if((pion.eq.9).and.(alpha_lamb.ne.0.d0))then
                  call test_spin0(Sp,Hadronic,Sp**2*amplit,accepted)   !czynnik dodany 11.II.2013
                  if (accepted) then 
                     tr(0) = tr(0) + 1.d0
                     call addiere(1.d0,qq,0)
                     if (iprint.ne.0) call writeevent()
                     acc = 1
                   endif
               else
               tr(0) = tr(0) + 1.d0 
c --- add to the histogrammes ---
               call addiere(1.d0,Sp,0) ! hadrons invariant mass
c polar angle distribution
       pom = momenta(6,3)/
     &         dSqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
               call addiere(1.d0,pom,3) ! cos(th+)
       pom = momenta(7,3)/
     &         dSqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
              call addiere(1.d0,pom,4) ! cos(th-)
              if (iprint.ne.0) call writeevent()
              acc = 1
              endif
         endif
         endif    
      else
         inte = 0.d0
         intezero =0.d0
      endif
      return
      end

c ======================================================================
c --- generates one photon ---------------------------------------------
c ======================================================================
      subroutine gen_1ph(i,qqmin,qqmax,cos1min,cos1max,
     &                   cos3min,cos3max,acc)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'
      complex*16 Leptonic(0:3,0:3),Hadronic(0:3,0:3),
     1 Leptonic2(0:3,0:3),Hadronic2(0:3,0:3),
     2 LeptonicP(0:3,0:3),HadronicP(0:3,0:3),ptemp(0:3)
      real*8 qqmin,qqmax,qq,q0p,q2p,q0b,q2b,jac0,pom,
     &  cos1min,cos1max,cos1,phi1,jac1,cos3min,cos3max,cos3,phi3,jac4,
     &  Ar(14),Ar_r(0:13),z,inte,helicityampLO,Matrix,amplit,rk1(4),
     & AmpChi,pom1,chi_prod2,NLOpions
      real*8 costhm_ll,phim_ll,costhp_ll,phip_ll,inte_s,dps,chi_prod1
      real*8 virt_full,virt_muons,tracepi0,test_trace,pi0gammaAmp,test1,
     1 test2,check
      real*8 softint_p1p2,softint_pjpj,softint_q1q2,softint_qjqj,
     1 softint_sum,Int_f1,Int_f2,Int_f,ampNLO,amppenta
      real*16 q1(0:3),q2(0:3),p2(0:3),p1(0:3),p4(0:3),qq_quad,virt_pions
      complex*16 fsr_no_em
      integer i,ru,mu,nu
      logical accepted      
      integer acc
      common/Lept_ee/Leptonic
      common/pedf/rk1,dps 
      common/virtsoft/inte_s
      common/test_no_em/fsr_no_em
c
      if(pion.lt.13)then
         call ranlxdf(Ar_r,7)
      do ru=0,6
       Ar(ru+1)=Ar_r(ru)
      enddo
      else
      call ranlxdf(Ar_r,9)
      do ru=0,8
       Ar(ru+1)=Ar_r(ru)
      enddo 
      endif
          
        if(((pion.eq.11).or.(pion.eq.12)).and.(be_r.eq.1))then
             call beam_res
       endif 

       call addiere(1.d0,momenta(1,0),6)
       call addiere(1.d0,momenta(2,0),7)
       call addiere(1.d0,momenta(2,0)+momenta(1,0),8)     

      acc = 0

c --- get the variables ---------------------------

      if((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then  
       z = Mmax(1)*Ar(6)
       call qquadrat_Pg1(qq,qqmin,qqmax,cos1min,cos1max,
     &                   cos3min,cos3max,Ar,jac0)
      else
      if((pion.eq.1).and.(nlo2.eq.1))then
         call momenta_quad(qq_quad,Ar,p1,q1,p2,q2,p4,jac0,jac1,cos1)
c         print*,'q=',jac0,jac1
         qq=dble(qq_quad)
         z = Mmax(1)*Ar(6)
      else
      call qquadrat(qqmin,qqmax,cos3min,cos3max,Ar
     1,qq,q0p,q2p,q0b,q2b,cos3,phi3,jac0)
c       print*,'d=',jac0,jac1
      

      call photonangles1(cos1min,cos1max,Ar,cos1,phi1,jac1)
      z = Mmax(1)*Ar(6)  
c --- 4-momenta in the CMS frame ---
      call leptonmomenta1(qq,cos1,phi1)
      endif
      call LeptonicTensor1ISR(qq,cos1,Leptonic)
      if((pion.le.1).or.(pion.eq.4).or.(pion.eq.5).or.(pion.eq.6)
     1              .or.(pion.eq.7).or.(pion.eq.9).or.(pion.eq.11)
     2 .or.(pion.eq.12))then
       if((pion.eq.1).and.(nlo2.eq.1))then
        continue
       else
       call hadronmomenta(qq,cos3,phi3)
       endif

       if((pion.ne.11).or.(pion.ne.12))then
         call HadronicTensorISR(qq,q0p,q2p,q0b,q2b,Hadronic)
       endif
      elseif((pion.eq.2).or.(pion.eq.3))then
       call hadronmomenta_1(4)
       call HadronicTensorISR(qq,q0p,q2p,q0b,q2b,Hadronic)
      elseif((pion.eq.8).or.(pion.eq.10))then
       call hadronmomenta_1(3)
       call HadronicTensorISR(qq,q0p,q2p,q0b,q2b,Hadronic)
      endif      
      if(pion.eq.9)then
       call pionanglesLamb(cos3min,cos3max,costhm_ll,phim_ll,costhp_ll,
     1                     phip_ll)
       call hadronmomenta_Lamb(qq,cos3,phi3,costhm_ll,phim_ll,costhp_ll,
     1                         phip_ll)
      endif
      
      endif
c --- tests cuts ---
      call testcuts(1,accepted)
c --- value of the integrand ---
      if (accepted) then
c ---
        if(pion.eq.11)then
         amplit= chi_prod1(qq)
         dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
         dps = dps*dSqrt(1.d0-4.d0*mmu*mmu/qq)/(32.d0*pi*pi) 
         elseif(pion.eq.12)then
          amplit= chi_prod2(qq)
         dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
         dps = dps*dSqrt(1.d0-4.d0*mmu*mmu/qq)/(32.d0*pi*pi)
         elseif(pion.eq.13)then
         dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
         dps = dps*(1.d0-mpi0f**2/qq)/(32.d0*pi*pi)        
         amplit=pi0gammaAmp(qq,Ar)*dps
         elseif(pion.eq.14)then
         dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
         dps = dps*(1.d0-metaf**2/qq)/(32.d0*pi*pi)
         amplit=pi0gammaAmp(qq,Ar) *dps
         elseif(pion.eq.15)then
         dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
         dps = dps*(1.d0-metaP**2/qq)/(32.d0*pi*pi)
         amplit=pi0gammaAmp(qq,Ar)*dps              
         else 
           amplit = Matrix(Leptonic,Hadronic)
c           print*, 'ISR old=',amplit
         endif



c
        if((pion.eq.0).and.(ph0.eq.0).and.(fsr.ne.0))then
         virt_full = virt_muons(w*dsqrt(Sp),qq)
         if(nlo.eq.0)then
         inte = gev2nbarn * amplit * jac0*jac1/(4.d0*pi*Sp)
         if(fsr.ne.0) inte = inte+gev2nbarn*helicityampLO(qq)*
     1               jac0*jac1/(4.d0*pi*Sp)

         inte_s=inte
         else
         inte = gev2nbarn * virt_full * jac0*jac1/(4.d0*pi*Sp)
         inte_s=inte
         endif
        elseif((pion.eq.4).and.(ph0.eq.1))then
         inte = gev2nbarn * amplit * jac0*jac1/(4.d0*pi*Sp)
         inte_s=inte
         elseif(pion.eq.11)then
        inte=(4.d0*pi*alpha)**3*amplit * jac0*
     1 jac1/(4.d0*pi*Sp)*dps/4.d0*gev2nbarn
         inte_s=inte
         elseif(pion.eq.12)then
         inte=(4.d0*pi*alpha)**3*amplit * jac0*
     1 jac1/(4.d0*pi*Sp)*dps/4.d0*gev2nbarn
         inte_s=inte
         elseif((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then
         inte=(4.d0*pi*alpha)**4*amplit*jac0
     1 /(4.d0*pi*Sp)/4.d0*gev2nbarn/2.d0 ! 1.d0/2.d0 statistical factor 2 photons in final state  
         inte_s=inte
        else 
         inte = gev2nbarn * amplit * jac0*jac1/(4.d0*pi*Sp)
c
         if(fsr.ne.0) inte = inte+gev2nbarn*helicityampLO(qq)*
     1               jac0*jac1/(4.d0*pi*Sp)
c         print*,'1=', inte
c
c full pi0-------------------------------------------------------------
         if((pion.eq.1).and.(nlo2.eq.1))then !ST - full soft
           amppenta= dble(virt_pions(qq_quad,p1,q1,p2,q2,p4,check))
           inte=inte+gev2nbarn*NLOpions(qq)*jac0*jac1/(4.d0*pi*Sp)
     1     + gev2nbarn*amppenta*jac0*jac1/(4.d0*pi*Sp)
         endif
c----------------------------------------------------------------------
        inte_s=inte
        endif
c ---
         if (inte.gt.gross(1)) gross(1) = inte
         if (inte.lt.klein(1)) klein(1) = inte
c --- in the second rund ---
         if (i.eq.2) then
         call addiereMC(inte_s,qq,1)
         pom = momenta(6,3)/
     &         dSqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
              call addiereMC(inte_s,pom,3)
        pom = momenta(7,3)/
     &         dSqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
              call addiereMC(inte_s,pom,4)
            if (Mmax(1).lt.inte) write(*,*)'Warning! Max(1) too small!'
c          if (inte.lt.0.d0)    write(*,*)'1ph:Warning! negative weight'
c          if (inte.lt.0.d0)    write(*,*)'inte',inte
  203 FORMAT('',A10,G20.12E3)
c --- event accepted? ---
            if (z.le.inte) then

               if((pion.eq.9).and.(alpha_lamb.ne.0.d0))then
                  call test_spin(qq,Hadronic,amplit,accepted)
                  if (accepted) then 
                     tr(1) = tr(1) + 1.d0
                     call addiere(1.d0,qq,1)
                    if (iprint.ne.0) call writeevent()
                    acc = 1
                 endif
               else
                  tr(1) = tr(1) + 1.d0
c --- add to the histogrammes ---
               call addiere(1.d0,qq,1)

c polar angle distributions
c       if(momenta(3,0).gt.momenta(7,0))then
c       pom = momenta(3,3)/
c     &         dSqrt(momenta(3,1)**2+momenta(3,2)**2+momenta(3,3)**2)
c               call addiere(1.d0,pom,4) ! cos(th+)
c        else
c        pom = momenta(7,3)/
c     &         dSqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
c              call addiere(1.d0,pom,4)
c        endif
        pom = momenta(6,3)/
     &         dSqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
              call addiere(1.d0,pom,3) ! cos(th+)
        pom = momenta(7,3)/
     &         dSqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
              call addiere(1.d0,pom,4) ! cos(th-)

                if (iprint.ne.0) call writeevent()
                acc = 1
               endif
            endif 

c
         endif
      else
         inte = 0.d0
         inte_s=inte
      endif
      return
      end
c ======================================================================
c --- generates two photons --------------------------------------------
c ======================================================================
      subroutine gen_2ph(i,qqmin,cos1min,cos1max,
     &   cos2min,cos2max,cos3min,cos3max,acc)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'
      real*8 qqmin,qqmax,qq,jac0,q0p,q2p,q0b,q2b,
     &  cos1min,cos1max,cos1,phi1,jac1,sin1,cos12,
     &  cos2min,cos2max,cos2,phi2,jac2,sin2,
     &  w1,w2,w1min,jac3,cos3min,cos3max,cos3,phi3,jac4,
     &  Ar(14),Ar_r(0:14),z,inte,helicityamp,pom
      integer i,ru
      logical accepted    
      integer acc
c  
      common/muonfsr/inte
c
      call ranlxdf(Ar_r,13)
      do ru=0,12
         Ar(ru+1) = Ar_r(ru)
      enddo
      
      acc = 0

c --- get the variables -----------------------------------------------
c --- one of the photons is generated inside the angular cuts and -----
c --- the other is generated everywhere -------------------------------
      if (Ar(12).lt.0.5d0) then
         call photonangles1(cos1min,cos1max,Ar,cos1,phi1,jac1)
         call photonangles2(cos2min,cos2max,Ar,cos2,phi2,jac2)  
         if (cos2.lt.cos1min.or.cos2.gt.cos1max) jac1=2.d0*jac1
       else
         call photonangles1(cos2min,cos2max,Ar,cos1,phi1,jac1)
         call photonangles2(cos1min,cos1max,Ar,cos2,phi2,jac2)  
         if (cos1.lt.cos1min.or.cos1.gt.cos1max) jac2=2.d0*jac2
      endif  
         sin1 = dSqrt(1.d0-cos1*cos1)
         sin2 = dSqrt(1.d0-cos2*cos2)
         cos12 = sin1*sin2*dCos(phi1-phi2)+cos1*cos2
         w1min = gmin/dSqrt(Sp)
         qqmax = Sp*(1.d0-2.d0*(w1min+w)+2.d0*w1min*w*(1.d0-cos12))
         if(q2_max_c.lt.qqmax)qqmax= q2_max_c  ! external cuts         
      call qquadrat2(qqmin,qqmax,cos3min,cos3max,Ar,
     &               qq,q0p,q2p,q0b,q2b,cos3,phi3,jac0)

           call photonenergy2(qq,cos1min,cos1max,cos1,cos2,cos12
     &                       ,Ar,w1,w2,jac3)

      if((pion.eq.2).or.(pion.eq.3))then
       call pionangles_1(qq,q0p,q2p,q0b,q2b,jac4)
       jac0 = jac0 * jac4
      elseif(pion.eq.8)then
       call pionangles_2(qq,q0p,q2p,jac4)
       jac0 = jac0 * jac4
      elseif(pion.eq.10)then
       call pionangles_eta(qq,q0p,q2p,jac4)
       jac0 = jac0 * jac4
      endif      
c
      z = Mmax(2)*Ar(6)         
c       
c --- 4-momenta in the CMS frame ------
      call leptonmomenta2(qq,w1,w2,cos1,phi1,cos2,phi2)
      if((pion.le.1).or.(pion.eq.4).or.(pion.eq.5).or.(pion.eq.6)
     1              .or.(pion.eq.7))then
       call hadronmomenta(qq,cos3,phi3)
      elseif((pion.eq.2).or.(pion.eq.3))then
       call hadronmomenta_1(4)
      elseif((pion.eq.8).or.(pion.eq.10))then
       call hadronmomenta_1(3)
      endif      
c --- tests cuts ---
      call testcuts(2,accepted)
c --- value of the integrand ---
      if (accepted) then
c --- helicity amplitudes ---
         inte = gev2nbarn*helicityamp(qq,q0p,q2p,q0b,q2b)*
     &       jac0*jac1*jac2*jac3/(4.d0*pi*Sp)
 


c ---     
         if (inte.gt.gross(2)) gross(2) = inte
         if (inte.lt.klein(2)) klein(2) = inte
c --- in the second rund ---
         if (i.eq.2) then
         call addiereMC(inte,qq,2)
         pom = momenta(6,3)/
     &         dSqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
               call addiereMC(inte,pom,5)
       pom = momenta(7,3)/
     &         dSqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
              call addiereMC(inte,pom,6)
            if (Mmax(2).lt.inte) write(*,*)'Warning! Max(2) too small!'
          if (inte.lt.0.d0)    write(*,*)'2ph:Warning! negative weight'
          if (inte.lt.0.d0)    write(*,*)'inte',inte
  203 FORMAT('',A10,G20.12E3)
c --- event accepted? ---
            if (z.le.inte) then
                  tr(2) = tr(2) + 1.d0
               if (iprint.ne.0) call writeevent()
               acc = 1
c --- add to the histogrammes ---
               call addiere(1.d0,qq,2)
c polar angle distribution
       pom = momenta(6,3)/
     &         dSqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
               call addiere(1.d0,pom,5) ! cos(th+)
       pom = momenta(7,3)/
     &         dSqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
              call addiere(1.d0,pom,6) ! cos(th-)
 
            endif
         endif
      else
         inte = 0.d0
      endif
      return
      end
c ======================================================================
c --- input parameters -------------------------------------------------
c ======================================================================
      subroutine input(nges,nm,outfile)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'
      integer nm,i,jj,kk,ii
      integer*8 nges
      character outfile*20
      real*8 E,lll,qq1,soft
      real*8 betarho_pp, betaomega_pp,betaphi_pp,alphaphi_pp,mrho_pp,
     &       momega_pp,mphi_pp,gammarho_pp,theta_pp,gam_pp
      real*8 mm_ph,gg_ph,bp2smm,mm_f0,gg_f0,c_phi_KK_f0_pi,c_phi_gam
     2      ,mm_f0_600,gg_f0_600,c_phi_KK_f0_600_pi,phas_rho_f0
     3      ,phas_f0_f0600,aa_phi,Br_phi_KK
      real*8 mm_f0_exp,phas_rho_f0_exp,c_f0_KK_exp,c_f0_pipi_exp,
     1       ff_phi_exp,c_phi_KK,rho_phi_exp,lamb_phi_exp
      real*8 mm_phi_lam,mm_om_lam,mu_lamb,phase_f2_lamb
      complex*16 dggvap,phas1,phas2,BW_om,PionFormFactor,KaonFormFactor
      complex*16 PionFormFactor_ex
      real*8 Qjp,Qp2s,g_KK_res
      real*8 g_K_jp,g_e_jp,QgK2,g_K_p2s,g_e_p2s,
     1       QgK2_p2s
      complex*16 g_eK_jp,g_eK_p2s
      complex*16 jp_resonance,p2s_resonance
      real*8 g_K1_jp,g_K1_p2s
      real*8 p1(4),p2(4),dme,el_m2             ! inteface to helicity amp
      real*8 gfun_4pi,del_chi
      real*8 modcjp_Kp,phacjp_Kp,modcjp_K0,phacjp_K0,
     1      modcp2s_Kp,phacp2s_Kp,modcp2s_K0,phacp2s_K0
      real*8 m_rho0_P,g_rho0_P,beta_pion,beta_pion_p,ilocz_pion,
     1       p_rho,vv_rho,GAMMA,HH_P
      real*8 vprehadsp,vprehadtm,vpimhad,vprelepsp,vpreleptm,vpimlep,
     & vpretopsp,vpretoptm,g_met(4,4),sm(4)
      real*8 a_chi,b_chi
      complex*16 vacpol_and_nr,result_thomas,iii
      integer mu,nu,j
      complex*16 Jee11(4),Jee12(4),Jee21(4),Jee22(4),
     1           Jeemin(2,2,2,2),cpp(2,2,2,2),adg_chi
      real*8 con1,con2,con3,con4,lambda
      real*8 softint_pjpj,softint_p1p2
c
      common/com1/lambda
c
      common/lepcur/Jee11,Jee12,Jee21,Jee22,Jeemin,cpp
c
      common/schan/con4
c
      common /cp1p2/p1,p2,dme,el_m2
c Proton and Neutron FormFactor common
      common /protparam/ betarho_pp, betaomega_pp,betaphi_pp, 
     &alphaphi_pp,mrho_pp,momega_pp,mphi_pp,gammarho_pp,theta_pp,gam_pp
c Pi+ Pi- commons
      common/param_PFF/aa_phi,mm_ph,gg_ph
      common/param_f0/mm_f0,gg_f0,c_phi_KK_f0_pi,c_phi_gam,mm_f0_600,
     1                gg_f0_600,c_phi_KK_f0_600_pi
      common/phases_f0/phas1,phas2
      common/param_f0_exp/mm_f0_exp,phas_rho_f0_exp,c_f0_KK_exp,
     1      c_f0_pipi_exp,ff_phi_exp,c_phi_KK,rho_phi_exp,lamb_phi_exp
c Lambda (Lambda bar) 
      common/lambparam/mm_phi_lam,mm_om_lam,mu_lamb,phase_f2_lamb
c J/Psi resonance parameters common
      common /jpparam/ Qjp,g_eK_jp 
c Psi(2S) resonance parameters common
      common /p2sparam/ Qp2s,g_eK_p2s 
c J/Psi and Psi(2S) generation common
      common /genres/QgK2,QgK2_p2s
      common/ssmm/sm
      
        
       
      real*8 ecm !TF
      common / beam / ecm !TF
c
c --- input file ---------------------------------
!      open(7,file=belle2_phokhara_parameters%input_file,status='old')
      open(8,file=belle2_phokhara_parameters%parameter_file,
     &  status='old')
c --- input generation parameters ----------------
!      read(7,*)           !                                   
!      read(7,*) nges      ! number of generated events        
!      read(7,*) nm        ! events to determine the maximum   
!      read(7,*) outfile   ! output file                       
!      read(7,*) iprint    ! printing(1), nonprinting(0) generated events
!      read(7,*) ph0       ! Born: 1ph(0)radiative return; Born: 0ph(1) scan
!      read(7,*) nlo       ! radiative return mode: Born(0), NLO(1)
!      read(7,*) nlo2      ! full NLO: No(0), Yes(1)
!      read(7,*) w         ! soft photon cutoff                
!      read(7,*) pion      ! mu+mu-(0),pi+pi-(1),2pi0pi+pi-(2),2pi+2pi-(3),ppbar(4),nnbar(5),K+K-(6),K0K0bar(7),pi+pi-pi0(8), Lamb Lambbar->pi-pi+ppbar(9) 
!      read(7,*) fsr       ! ISR only(0), ISR+FSR(1), ISR+INT+FSR(2)
!      read(7,*) fsrnlo    ! yes(1), no(0)
!      read(7,*) ivac      ! no(0),yes; by Fred Jegerlehner(1),yes;by Thomas Teubner(2)
!      read(7,*) FF_Pion   ! KS Pionformfactor(0), GS Pionformfactor(1) old, GS Pionformfactor new(2)
!      read(7,*) f0_model  ! f0+f0(600): KK model(0), no structure(1), no f0+f0(600)(2), f0 KLOE(3)
!      read(7,*) FF_kaon   ! KaonFormFactor constrained (0),KaonFormFactor unconstrained (1),KaonFormFactor old (2)
!      read(7,*) narr_res  ! no narrow resonances (0), J/Psi (1), Psi(2S) (2) (narro resonances only for pion = 0, 1, 6, 7
!      read(7,*) FF_pp     !ProtonFormFactor old(0), ProtonFormFactor new(1)
!      read(7,*) FF_Pgg
!      read(7,*) chi_sw         !Radiative return(0), Chi production(1), Radiative return + Chi production (2)
!      read(7,*) be_r            !be_r: without beam resolution(0), with beam resolution(1)
c --- input collider parameters -------------------
!      read(7,*)           !                                   
!      read(7,*) E         ! CMS-energy 
!      read(7,*) beamres ! beam resolution                         
c --- input experimental cuts ---------------------
!      read(7,*)           !                                   
!      read(7,*) q2min     ! minimal  hadrons(muons)-gamma-inv mass squared 
!      read(7,*) q2_min_c  ! minimal inv. mass squared of the hadrons(muons)
!      read(7,*) q2_max_c  ! maximal inv. mass squared of the hadrons(muons)
!      read(7,*) gmin      ! minimal photon energy/missing energy             
!      read(7,*) phot1cut  ! minimal photon angle/missing momentum angle
!      read(7,*) phot2cut  ! maximal photon angle/missing momentum angle
!      read(7,*) pi1cut    ! minimal hadrons(muons) angle
!      read(7,*) pi2cut    ! maximal hadrons(muons) angle
c --- read histogram paremeters -------------------
!      read(7,*)           ! 
!      do i = 0,20         ! read title, limits and bins       
!         read(7,*) title(i)
!         read(7,*) xlow(i),xup(i),bins(i)
!      enddo
!      close(7)
c
c --- input couplings, masses and meson widths ---
      read(8,*)           !                                   
      read(8,*) alpha     ! 1/alpha (QED) 
      read(8,*) GFermi    ! Fermi constant
      read(8,*) sinthW2   ! sin^2(theta_W)                  
      read(8,*) me        ! Electron mass   
      read(8,*) mp        ! Proton mass 
      read(8,*) mnt       ! Neutron mass   
      read(8,*) mmu       ! Muon mass                         
      read(8,*) mpi       ! Charged pion mass                         
      read(8,*) mpi0      ! Neutral pion mass                         
      read(8,*) mKp       ! Charged kaon mass                         
      read(8,*) mKn       ! Neutral kaon mass                         
      read(8,*) mlamb     ! Lambda mass
      read(8,*) meta      ! Eta mass

c --- fix constants -------------------------------
      mpi_q=mpi
      alpha = 1.d0/alpha
      alpha_q= alpha
      Sp = ecm*ecm    
      Sp_q= ecm*ecm                           ! CMS-energy squared
      ebeam = dSqrt(Sp)/2.d0                  ! beam energy
      ebeam_q = Sqrt(Sp_q)/2.q0
      w_q=w
      pi = 4.d0*dAtan(1.d0) 
      piq= 4.q0*atan(1.q0)  
      gev2nbarn = .389379292d6               ! from GeV^2 to nbarn
      gev2pbarn = .389379292d9

      dme   = me   
      me_q=me                          ! inteface to helicity amp
      el_m2 = me**2
      el_m2q = me**2
c
c p1 - positron, p2 - electron four momenta
c
      p1(1) = ebeam
      p1(2) = 0.d0
      p1(3) = 0.d0
      p1(4) = sqrt(ebeam**2-el_m2)
      p2(1) = ebeam
      p2(2) = 0.d0
      p2(3) = 0.d0
      p2(4) = -p1(4)
      do i=1,4
         momenta(1,i-1) = p1(i)
         momenta(2,i-1) = p2(i)
      enddo

      do i=1,4
        sm(i)=p1(i)+p2(i)
      enddo
      
      if((pion.ne.1).and.(nlo2.eq.1))then
         write(*,*)'nlo2=1 allowed only for 2pi mode (pion=1)'
         write(10,*)'nlo2=1 allowed only for 2pi mode (pion=1)'
         stop
      endif

      call init_ha
c 
c PDG 2004
c**************************
      read(8,*)
      read(8,*)  mm_ph 
      read(8,*)  gg_ph  
      read(8,*)  Br_phi_ee 
      read(8,*)  Br_phi_KK 
c**************************
c J/Psi resonance parameters and EM and strong couplings 

      if((narr_res.eq.0).or.(narr_res.eq.1).or.(narr_res.eq.2)) then
         continue
      else
         write(*,*)'if you do not use flag narr_res put: 0'
         stop
      endif

      if((pion.eq.11).or.(pion.eq.12))then
        if(narr_res.eq.1)then
       continue
       else
       write(*,*)
     1 'for chi_c1 and chi_c2 production you have to use narr_res=1 !!!' 
       stop
       endif
       endif

      if(narr_res.eq.1)then

        read(8,*)
        read(8,*)  mjp
        mjp_q=mjp
        read(8,*)  gamjp
        gamjp_q=gamjp
        read(8,*)  gamjpee
        read(8,*)  modcjp_Kp
        read(8,*)  phacjp_Kp
        read(8,*)  modcjp_K0
        read(8,*)  phacjp_K0
        Qjp = 3.D0 * gamjpee/gamjp/alpha
        Qjp_q=3.q0 * gamjpee/gamjp/alpha
        phacjp_Kp = phacjp_Kp/180.d0*pi
        phacjp_K0 = phacjp_K0/180.d0*pi
        cjp_Kp = modcjp_Kp*exp(dcmplx(0.d0,phacjp_Kp))
        cjp_K0 = modcjp_K0*exp(dcmplx(0.d0,phacjp_K0))
       

      endif

c Psi(2S) resonance parameters and EM and strong couplings 

      if((narr_res.eq.2))then
        do i=1,9
           read(8,*) 
        enddo

        read(8,*)  mp2s
        read(8,*)  gamp2s
        read(8,*)  gamp2see
        read(8,*)  modcp2s_Kp
        read(8,*)  phacp2s_Kp
        read(8,*)  modcp2s_K0
        read(8,*)  phacp2s_K0
        Qp2s = 3.D0 * gamp2see/gamp2s/alpha
        Qp2s_q = 3.q0 * gamp2see/gamp2s/alpha
        phacp2s_Kp = phacp2s_Kp/180.d0*pi
        phacp2s_K0 = phacp2s_K0/180.d0*pi
        cp2s_Kp = modcp2s_Kp*exp(dcmplx(0.d0,phacp2s_Kp))
        cp2s_K0 = modcp2s_K0*exp(dcmplx(0.d0,phacp2s_K0))

      endif

      if((pion.eq.11).or.(pion.eq.12))then
         read(8,*)
         read(8,*)  mp2s
         read(8,*)  gamp2s
         read(8,*)  gamp2see
      endif


      if(narr_res.eq.0) kk = 16
      if((narr_res.eq.1)) kk = 8
      if(((pion.eq.11).or.(pion.eq.12))) kk=4
      if(narr_res.eq.2) kk = 0
        do i=1,kk
           read(8,*) 
        enddo

        call constants() 
      cvac_s = vacpol_and_nr(Sp)

c
      if(ph0.eq.-1)then
c
       if(fsr.ne.0)then
        write(6,*)'fsr.ne.0; only ISR is implemented in 0-ph mode'
        stop
       endif
       if(fsrnlo.ne.0)then
        write(6,*)'fsrnlo.ne.0; only ISR is implemented in 0-ph mode'
        stop
       endif
       if(narr_res.ne.0)then
        write(6,*)'narr_res.ne.0;'
        write(6,*)' narrow resonances are not implemented in 0-ph mode'
        stop
       endif
       if((phot1cut.ne.0.d0).or.(phot2cut.ne.180.d0))then
        write(6,*)'in 0-ph mode the photons have to be generated with'
        write(6,*)'no angular cuts. Please put your experimental event'
        write(6,*)'selection directly in the subroutine  testcuts '
        stop
       endif
     
                  gmin = w*dSqrt(Sp)
       call Oph_emission_virtualsoft(gmin)
c metric tensor
       do i=1,4
        do j=1,4
          g_met(i,j) =0.d0
        enddo
       enddo   
c
       g_met(1,1)=1.d0
       do i=1,3
        g_met(i+1,i+1)=-1.d0
       enddo

c leptonic tensor for e+e- initial state
       do mu = 1,4
        do nu = 1,4
        Leptonic_epl_emin(mu-1,nu-1) = 4.d0*(
     &   p2(nu)*p1(mu)
     & - 2.d0*ebeam**2*g_met(mu,nu)
     & + p2(mu)*p1(nu))*4.d0*pi*alpha*cdabs(cvac_s)**2
        enddo
       enddo
       

c************************************************************
         elseif(ph0.eq.1)then
c
       if(pion.ne.4)then
       if(fsr.ne.0)then
        write(6,*)'fsr.ne.0; only ISR is implemented in 0-ph mode'
        stop
       endif
       if(fsrnlo.ne.0)then
        write(6,*)'fsrnlo.ne.0; only ISR is implemented in 0-ph mode'
        stop
       endif
       if(narr_res.ne.0)then
        write(6,*)'narr_res.ne.0;'
        write(6,*)' narrow resonances are not implemented in 0-ph mode'
        stop
       endif
       if((phot1cut.ne.0.d0).or.(phot2cut.ne.180.d0))then
        write(6,*)'in 0-ph mode the photons have to be generated with'
        write(6,*)'no angular cuts. Please put your experimental event'
        write(6,*)'selection directly in the subroutine  testcuts '
        stop
         endif
         else
c 

       if((phot1cut.ne.0.d0).or.(phot2cut.ne.180.d0))then
        write(6,*)'in 0-ph mode the photons have to be generated with'
        write(6,*)'no angular cuts. Please put your experimental event'
        write(6,*)'selection directly in the subroutine  testcuts '
        stop
       endif
c         
        endif
       if((pion.eq.4).and.(ph0.eq.1))then
          if((fsr.eq.0).or.(fsr.eq.1))then
            continue
          elseif(fsr.eq.2)then
           write(6,*)'only Coulomb factor is implemented in 0-ph mode'
           stop
          else
            write(6,*)'wrong fsr switch'
            stop
          endif
          if(fsrnlo.eq.0)then
           continue
          else
           write(6,*)'only Coulomb factor is implemented in 0-ph mode'
           write(6,*)'fsrnlo should be set to 0'
           stop
          endif
       endif
c
       gmin = w*dSqrt(Sp)
       call Oph_emission_virtualsoft(gmin)
c metric tensor
       do i=1,4
        do j=1,4
          g_met(i,j) =0.d0
        enddo
       enddo   
c
       g_met(1,1)=1.d0
       do i=1,3
        g_met(i+1,i+1)=-1.d0
       enddo

c leptonic tensor for e+e- initial state
       do mu = 1,4
        do nu = 1,4
        Leptonic_epl_emin(mu-1,nu-1) = 4.d0*(
     &   p2(nu)*p1(mu)
     & - 2.d0*ebeam**2*g_met(mu,nu)
     & + p2(mu)*p1(nu))*4.d0*pi*alpha*cdabs(cvac_s)**2
        enddo
       enddo
      elseif(ph0.eq.0)then
        continue
      else
       write(6,*)'wrong ph0 switch; please check the input file'
       stop
      endif
c
      if(pion.eq.1)then
        cvac_s = 1.d0
        cvac_s_pi  = vacpol_and_nr(Sp)
        if(ivac.eq.0)then
          cvac_s_pi1 = dcmplx(1.d0,0.d0)
        elseif(ivac.eq.1)then
          cvac_s_pi1 = 1.d0/(1.d0-dggvap(Sp,0.d0))
        else
         call vphlmntv1nonr(sqrt(Sp), 
     & vprehadsp,vprehadtm,vpimhad,vprelepsp,vpreleptm,vpimlep,
     & vpretopsp,vpretoptm)
         result_thomas = dcmplx(vprehadsp+vprelepsp+vpretopsp
     1               ,vpimhad+vpimlep)
         cvac_s_pi1 = 1.d0/(1.d0-result_thomas)
        endif
      endif

      ver_s = 0.d0
      if (fsrnlo.eq.1)then
        lll = log(Sp/el_m2)
      if(pion.eq.0)then
      ver_s = 4.d0*(
     1        0.75d0*lll -1.d0 +pi**2/6.d0)
     2   +2.d0*(-lll + pi**2/3.d0 + lll**2/2.d0)!*2.d0  !minus soft       
      else
        ver_s = 2.d0*alpha/pi*( 
     1     (lll-1.d0)*log(2.d0*w) + 0.75d0*lll -1.d0 +pi**2/6.d0)
      endif
      endif

      if(nlo2.eq.1)then !ST soft subtraction
        lll = log(Sp/el_m2)
        ver_s = 2.d0*alpha/pi*( 
     1     (lll-1.d0)*log(2.d0*w) + 0.75d0*lll -1.d0 +pi**2/6.d0)
        soft=alpha/pi*(
     1 (-log(4.d0*w*w))*(1.d0+log(me**2/Sp))
     1 -log(me**2/Sp)**2/2.d0-log(me**2/Sp)-pi**2/3.d0)
        ver_s=ver_s-soft
c       call softintegral_p1p2(softint_p1p2)
c       call softintegral_pjpj(softint_pjpj)
c       print*,'ver_s(soft)=',2.d0*alpha/pi*( 
c     1     (lll-1.d0)*log(2.d0*w)),2.d0*(-softint_p1p2+softint_pjpj)
       endif
c
c 2 pi mode -----------
c
      if(pion.eq.1)then

c -----------------------------------------------------------------
      if(f0_model.eq.0)then
c phi_f0-pipi (K+K- model) Phys.Rev.D56(1997)4084, Phys.Lett.B331(1994)418, 
c                          hep-ph/0001064, Phys.Lett.B611(2005)116
        read(8,*)
        read(8,*) c_phi_gam
        read(8,*) mm_f0
        read(8,*) gg_f0
        read(8,*) c_phi_KK_f0_pi
c phi_f0(600)-pipi
        read(8,*) mm_f0_600
        read(8,*) gg_f0_600
        read(8,*) c_phi_KK_f0_600_pi
c phases : 1. rho and (f0 + f0(600)), 2. f0 and f0(600) 
        read(8,*) phas_f0_f0600
        phas2 = exp(dcmplx(0.d0,phas_f0_f0600*pi/180.d0))
c -----------------------------------------------------------------    
      elseif(f0_model.eq.1)then
c phi_f0-pipi ("no structure" model) hep-ph/0001064
        do i=1,10
           read(8,*) 
        enddo
        read(8,*) c_phi_gam 
        read(8,*) mm_f0
        read(8,*) gg_f0
        read(8,*) c_phi_KK_f0_pi
c phi_f0(600)-pipi
        read(8,*) mm_f0_600 
        read(8,*) gg_f0_600 
        read(8,*) c_phi_KK_f0_600_pi 
c phases : 1. rho and (f0 + f0(600)), 2. f0 and f0(600) 
        read(8,*) phas_rho_f0
        read(8,*) phas_f0_f0600
        phas1 = exp(dcmplx(0.d0,phas_rho_f0*pi/180.d0))
        phas2 = exp(dcmplx(0.d0,phas_f0_f0600*pi/180.d0))
c -----------------------------------------------------------------
      elseif(f0_model.eq.3)then
c KLOE model: mail; hep-ph/9703367; hep-ph/9706363; NP B315,465;
        do i=1,20
           read(8,*) 
        enddo
        read(8,*) mm_f0_exp
        read(8,*) phas_rho_f0_exp
        read(8,*) c_f0_KK_exp
        read(8,*) c_f0_pipi_exp
        ff_phi_exp = sqrt(Br_phi_ee*gg_ph * 3.d0/4.d0/pi/alpha**2
     1                   / mm_ph)  !  \ f_phi

        rho_phi_exp = sqrt(1.d0 - 4.d0*(mKp/mm_ph)**2)
        lamb_phi_exp = log((1.d0+rho_phi_exp)**2 *(mm_ph/mKp)**2/4.d0) 

        c_phi_KK = sqrt( 48.d0 * pi / mm_ph / rho_phi_exp**3
     1           * Br_phi_KK*gg_ph)
      endif

        if(FF_pion.eq.0)then  !------------- KS PionFormFactor ---------
              if(f0_model.eq.0) kk = 16
              if(f0_model.eq.1) kk = 6
              if(f0_model.eq.2) kk = 25
              if(f0_model.eq.3) kk = 1
               do i=1,kk
                  read(8,*)
               enddo

           read(8,*) c_0_pion
           read(8,*) c_1_pion
           read(8,*) c_2_pion
           read(8,*) c_3_pion
           read(8,*) c_n_pion
           read(8,*) c_om_pion
           read(8,*) m_rho0_pion
           read(8,*) g_rho0_pion
           read(8,*) m_rho1_pion
           read(8,*) g_rho1_pion
           read(8,*) m_rho2_pion
           read(8,*) g_rho2_pion
           m_rho3_pion = 0.7755d0*sqrt(7.d0)
           g_rho3_pion = 0.2d0*m_rho3_pion
           read(8,*) m_om0_pion
           read(8,*) g_om0_pion

           PionFormFactor_Sp = PionFormFactor(Sp)

        elseif(FF_pion.eq.1)then  !------------- GS old PionFormFactor ----
               if(f0_model.eq.0) kk = 31
               if(f0_model.eq.1) kk = 21
               if(f0_model.eq.2) kk = 40
               if(f0_model.eq.3) kk = 16
               do i=1,kk
                  read(8,*)
               enddo
           read(8,*) c_0_pion
           read(8,*) c_1_pion
           read(8,*) c_2_pion
           read(8,*) c_3_pion
           read(8,*) c_n_pion
           read(8,*) c_om_pion
           read(8,*) m_rho0_pion
           read(8,*) g_rho0_pion
           read(8,*) m_rho1_pion
           read(8,*) g_rho1_pion
           read(8,*) m_rho2_pion
           read(8,*) g_rho2_pion
           m_rho3_pion = 0.7755d0*sqrt(7.d0) 
           g_rho3_pion = 0.2d0*m_rho3_pion   
           read(8,*) m_om0_pion
           read(8,*) g_om0_pion     

           PionFormFactor_Sp = PionFormFactor(Sp)

        elseif(FF_pion.eq.2)then  !------------- GS new PionFormFactor -----
               if(f0_model.eq.0) kk = 46
               if(f0_model.eq.1) kk = 36
               if(f0_model.eq.2) kk = 55
               if(f0_model.eq.3) kk = 31
               do i=1,kk
                  read(8,*)
               enddo
c
           read(8,*)m_rho0_pion
           read(8,*)g_rho0_pion
           read(8,*)par_pionGS(1)
           read(8,*)par_pionGS(2)
           read(8,*)par_pionGS(3)
           read(8,*)par_pionGS(4)
           read(8,*)par_pionGS(5)
           read(8,*)par_pionGS(6)
           read(8,*)par_pionGS(7)
           read(8,*)par_pionGS(8)
           read(8,*)par_pionGS(9)
           read(8,*)par_pionGS(10)
           read(8,*)par_pionGS(11)
           read(8,*)par_pionGS(12)
           read(8,*)par_pionGS(13)
           read(8,*)par_pionGS(14)
           read(8,*)par_pionGS(15)
           read(8,*)par_pionGS(16)
           read(8,*)par_pionGS(17)
           read(8,*)par_pionGS(18)
           read(8,*)par_pionGS(19)
           read(8,*)par_pionGS(20)
           read(8,*)par_pionGS(21)
           read(8,*)par_pionGS(22)
           read(8,*)par_pionGS(23)
           read(8,*)par_pionGS(24)
           read(8,*)par_pionGS(25)
           read(8,*)beta_pion
c
           beta_pion_p = beta_pion - 1.d0
           ilocz_pion = 1.d0
           c_n_pionGS(0) = par_pionGS(1)
           m_n_pionGS(0) = par_pionGS(2)
           m_n_pionGS(1) = par_pionGS(8)
           m_n_pionGS(2) = par_pionGS(11)
           m_n_pionGS(3) = par_pionGS(13)
           m_n_pionGS(4) = par_pionGS(22)
           m_n_pionGS(5) = par_pionGS(23)
           gam_n_pionGS(0) = par_pionGS(3)
           gam_n_pionGS(1) = par_pionGS(9)
           gam_n_pionGS(2) = par_pionGS(10)
           gam_n_pionGS(3) = par_pionGS(12)
           gam_n_pionGS(4) = par_pionGS(18)
           gam_n_pionGS(5) = par_pionGS(19)
c
           eephi = dcmplx(cos(par_pionGS(7) ),sin(par_pionGS(7)) )
           ee12  = dcmplx(cos(par_pionGS(14)),sin(par_pionGS(14)))
           ee23  = dcmplx(cos(par_pionGS(15)),sin(par_pionGS(15)))
           ee34  = dcmplx(cos(par_pionGS(20)),sin(par_pionGS(20)))
           ee45  = dcmplx(cos(par_pionGS(24)),sin(par_pionGS(24)))
c
           p_rho = sqrt(m_n_pionGS(0)*m_n_pionGS(0)-4.d0*mpi*mpi)/2.d0
           dd_ffpi(0) = 3.d0/2.d0/pi * mpi**2 / p_rho**2 
     1                * log( (m_n_pionGS(0)+2.d0*p_rho)**2/4.d0/mpi**2 )
     2                + m_n_pionGS(0)/2.d0/pi/p_rho 
     3                - mpi**2 * m_n_pionGS(0) /pi/p_rho**3
           dd_ffpi(0) = (m_n_pionGS(0)*m_n_pionGS(0)
     1                + dd_ffpi(0)*gam_n_pionGS(0)*m_n_pionGS(0))
           vv_rho = sqrt( 1.d0 - 4.d0*mpi**2/m_n_pionGS(0)**2 ) 
           poch_ffpi(0) = m_n_pionGS(0)**2 * gam_n_pionGS(0) / pi 
     1                  / sqrt(m_n_pionGS(0)**2-4.d0*mpi**2)**3
     2                  * ( vv_rho**2 + vv_rho/2.d0*(3.d0-vv_rho**2) 
     3          * log( (1.d0+vv_rho)**2/4.d0/mpi**2* m_n_pionGS(0)**2) )

      HH_p_ffpi(0)= HH_p(m_n_pionGS(0),gam_n_pionGS(0),m_n_pionGS(0)**2)
c
        do ii=1,2000
c
           ilocz_pion = ilocz_pion * (1.d0 - beta_pion_p/ii)
           c_n_pionGS(ii) = (-1.d0)**ii * GAMMA(beta_pion-0.5d0)
     1                    * 2.d0/sqrt(pi) / (1.d0+2.d0*ii) 
     3                    * ilocz_pion /pi *GAMMA(2.d0-beta_pion)
     2                    * sin(pi*(beta_pion-1.d0-ii))
        if(ii.ge.6)then
           m_n_pionGS(ii) = sqrt(m_rho0_pion**2*(1.d0+2.d0*ii))
        endif
        if(ii.ge.6)then
           gam_n_pionGS(ii) = g_rho0_pion/m_rho0_pion * m_n_pionGS(ii)
        endif

           p_rho = sqrt(m_n_pionGS(ii)*m_n_pionGS(ii)-4.d0*mpi*mpi)/2.d0
           dd_ffpi(ii) = 3.d0/2.d0/pi * mpi**2 / p_rho**2 
     1                 * log( ( m_n_pionGS(ii) + 2.d0*p_rho )**2 
     2                 / 4.d0/mpi**2 ) + m_n_pionGS(ii)/2.d0/pi/p_rho 
     3                 - mpi**2 * m_n_pionGS(ii) /pi/p_rho**3
           dd_ffpi(ii) = ( m_n_pionGS(ii)*m_n_pionGS(ii)
     1                 + dd_ffpi(ii)*gam_n_pionGS(ii)*m_n_pionGS(ii) )
           vv_rho = sqrt( 1.d0 - 4.d0*mpi**2/m_n_pionGS(ii)**2 ) 

           poch_ffpi(ii) = m_n_pionGS(ii)**2 * gam_n_pionGS(ii) / pi 
     1                   / sqrt(m_n_pionGS(ii)**2-4.d0*mpi**2)**3
     2                   * ( vv_rho**2 + vv_rho/2.d0*(3.d0-vv_rho**2) 
     3        * log( (1.d0+vv_rho)**2/4.d0/mpi**2 * m_n_pionGS(ii)**2) )
           HH_p_ffpi(ii)= HH_p(m_n_pionGS(ii),gam_n_pionGS(ii)
     1                        ,m_n_pionGS(ii)**2)
        enddo

c
           PionFormFactor_Sp = PionFormFactor_ex(Sp)
c
c  making grid for pion FF calculation in the run
c
          do ii=1,1000001
           qq1 = (2.d0*mpi)**2+((11.d0)**2-(2.d0*mpi)**2)*dfloat(ii-1)
     &             /dfloat(1000000)
           grid_qq(ii)   = qq1
           grid_ffpi(ii) = PionFormFactor_ex(qq1)
          enddo
        endif
      endif
c
c 4 pi mode ------------
c
      if((pion.eq.2).or.(pion.eq.3))then

        if(pion.eq.2)then
          rmass(1)= mpi0
          rmass(2)= mpi0
          rmass(3)= mpi
          rmass(4)= mpi
        elseif(pion.eq.3)then
        do i=1,4
          rmass(i)= mpi
        enddo
        else
         continue
        endif
c
c filling commons for 4pi modes
c
c hadronic current parameters: the pion mass (pim_4) which is fixed below
c                  is used in the parametrisation of the hadronic current 
c                  and NOT in the phasespace generation. As we assume isospin 
c                  invariance pi+ and pi0  masses are equal.  
            do i=1,84
               read(8,*)
            enddo
c
      read(8,*) a1m_4pi
      read(8,*) a1g_4pi
      read(8,*) rhom_4pi
      read(8,*) rhog_4pi
      read(8,*) rho1m_4pi
      read(8,*) rho1g_4pi
      read(8,*) rho2m_4pi
      read(8,*) rho2g_4pi
      read(8,*) omm_4pi
      read(8,*) omg_4pi
      read(8,*) f0m_4pi
      read(8,*) f0g_4pi
      read(8,*) pim_4pi
c
       omm2_4pi   = omm_4pi**2
       ommg_4pi   = omm_4pi*omg_4pi
       f0m2_4pi   = f0m_4pi*f0m_4pi
       f0mg_4pi   = f0m_4pi*f0g_4pi
c
        a1m2_4pi = a1m_4pi**2
        con_4pi  = a1g_4pi*a1m_4pi/gfun_4pi(a1m2_4pi)
c
      read(8,*) coupl1_4pi
      read(8,*) coupl2_4pi
      read(8,*) coupl3_4pi
      read(8,*) coupl_om_4pi
      read(8,*) rho1m_f_4pi
      read(8,*) rho1g_f_4pi
      read(8,*) rho2m_f_4pi
      read(8,*) rho2g_f_4pi
      read(8,*) rho3m_f_4pi
      read(8,*) rho3g_f_4pi
      read(8,*) b1_a1_4pi
      read(8,*) b2_a1_4pi
      read(8,*) b3_a1_4pi
      read(8,*) b1_f0_4pi
      read(8,*) b2_f0_4pi
      read(8,*) b3_f0_4pi
      read(8,*) b1_om_4pi
      read(8,*) b2_om_4pi
      read(8,*) b3_om_4pi
c
      endif
c
c Proton and Neutron formfactors parameters  ---------
c
      if((pion.eq.4).or.(pion.eq.5))then
	if(FF_pp.eq.0)then !old ProtonFormFactor
            do i=1,117
               read(8,*)
            enddo
        read(8,*) betarho_pp
        read(8,*) betaomega_pp
        read(8,*) betaphi_pp
        read(8,*) alphaphi_pp
        read(8,*) mrho_pp
        read(8,*) momega_pp
        read(8,*) mphi_pp
        read(8,*) gammarho_pp 
        theta_pp = pi/4.D0
        read(8,*) gam_pp
	elseif(FF_pp.eq.1)then !new ProtonFormFactor
		do i=1,127
	read(8,*)
		enddo
	read(8,*) beta1_pp
	read(8,*) beta2_pp
	read(8,*) beta3_pp
	read(8,*) beta4_pp
	read(8,*) beta5_pp
	read(8,*) beta6_pp
	read(8,*) beta7_pp
	read(8,*) beta8_pp
	read(8,*) beta9_pp
	read(8,*) beta10_pp
	read(8,*) beta11_pp
	read(8,*) beta12_pp
        read(8,*) beta13_pp
	read(8,*) beta14_pp
	read(8,*) beta15_pp
	read(8,*) beta16_pp
	read(8,*) beta17_pp
	read(8,*) beta18_pp
	read(8,*) beta19_pp
	read(8,*) beta20_pp
	read(8,*) mrho_pp0
	read(8,*) gammarho_pp0
	read(8,*) momega_pp0
	read(8,*) gammaomega_pp0
	read(8,*) mrho_pp1
	read(8,*) gammarho_pp1
	read(8,*) momega_pp1
	read(8,*) gammaomega_pp1
	read(8,*) mrho_pp2
	read(8,*) gammarho_pp2
	read(8,*) momega_pp2
	read(8,*) gammaomega_pp2
	read(8,*) mrho_pp3
	read(8,*) gammarho_pp3
	read(8,*) momega_pp3
	read(8,*) gammaomega_pp3

         mrho_pp4=dsqrt(mrho_pp0**2*(1.d0+2.d0*4.d0))
       gammarho_pp4=gammarho_pp0/mrho_pp0*mrho_pp4
       momega_pp4=dsqrt(momega_pp0**2*(1.d0+2.d0*4.d0))
       gammaomega_pp4=0.5d0*momega_pp4


	 iii = dcmplx(0.d0,1.d0)
              par1 = beta1_pp+iii*beta2_pp
      par2 = beta3_pp+iii*beta4_pp
      par3=beta13_pp+iii*beta14_pp

      
      

      par13 =
     &    ( - momega_pp0**2 - momega_pp1**2*par1 - 
     & momega_pp2**2*par2-momega_pp3**2*par3 )/momega_pp4**2
       
        
       
     
     

      par4 = beta5_pp+iii*beta6_pp
      par5=beta7_pp+iii*beta8_pp
      par6=beta15_pp+iii*beta16_pp
      par14 = - (mrho_pp0**2+par4*mrho_pp1**2
     1   + par5*mrho_pp2**2+mrho_pp3**2*par6)/mrho_pp4**2
        
       
       
     
c 
        par7 = beta9_pp+iii*beta10_pp
        par8=beta17_pp+iii*beta18_pp
        
        

        

     
c----------------------------------------------------------------------------------------------------------      
       par9 =(momega_pp0**2*(momega_pp0**2-momega_pp4**2
     & +iii*(momega_pp4*gammaomega_pp4-
     & momega_pp0*gammaomega_pp0))+momega_pp1**2*par7
     & *(momega_pp1**2-momega_pp4**2+iii*(momega_pp4
     &  *gammaomega_pp4-momega_pp1*gammaomega_pp1))
     & +momega_pp2**2*par8*(momega_pp2**2-momega_pp4**2
     & +iii*(momega_pp4*gammaomega_pp4-
     & momega_pp2*gammaomega_pp2)))
     & /(momega_pp3**2*(momega_pp4**2-momega_pp3**2
     & +iii*(momega_pp3*gammaomega_pp3-momega_pp4*
     &   gammaomega_pp4)))
     
        
       par15 =
     &   ( - momega_pp0**2 - momega_pp1**2*par7 - 
     & momega_pp2**2*par8-momega_pp3**2*par9)/momega_pp4**2
      
      


      
c-----------------------------------------------------------------------------------------------------
      par10=beta11_pp+iii*beta12_pp
        par11=beta19_pp+iii*beta20_pp
      par12 = 1.d0/mrho_pp3**2
     1 *(mrho_pp0**2*(mrho_pp0**2 - mrho_pp4**2 
     2   +iii*(mrho_pp4*gammarho_pp4-mrho_pp0*gammarho_pp0))
     3  +mrho_pp1**2*par10*(mrho_pp1**2 - mrho_pp4**2 
     4 +iii*(mrho_pp4*gammarho_pp4-mrho_pp1*gammarho_pp1))
     5  +mrho_pp2**2*par11*(mrho_pp2**2 - mrho_pp4**2 
     2   +iii*(mrho_pp4*gammarho_pp4-mrho_pp2*gammarho_pp2)))
     5 /(mrho_pp4**2 - mrho_pp3**2 
     6   +iii*(mrho_pp3*gammarho_pp3-mrho_pp4*gammarho_pp4))

       par16 = - (mrho_pp0**2+par10*mrho_pp1**2
     1   + par11*mrho_pp2**2+mrho_pp3**2*par12)/mrho_pp4**2
	endif
      endif
c
c Kaon K+K- mode  and Kaon K0K0bar mode, parameters from hep-ph/0409080
c
      if((pion.eq.6).or.(pion.eq.7))then

        if(FF_kaon.eq.0)then  ! --------- KaonFormFactor constrained -------

            do i=1,164
               read(8,*)
            enddo
           read(8,*) c_phi0_Kp
           read(8,*) c_phi1_Kp
           read(8,*) c_phi2_Kp   !!!!!
           read(8,*) c_phin_Kp
           c_phi3_Kp = 1.d0- c_phi0_Kp- c_phi1_Kp- c_phi2_Kp- c_phin_Kp
           read(8,*) c_rho0_Kp
           read(8,*) c_rho1_Kp
           read(8,*) c_rho2_Kp
           read(8,*) c_rho3_Kp   !!!!!
           read(8,*) c_rhon_Kp
           c_rho4_Kp = 1.d0- c_rho0_Kp- c_rho1_Kp- c_rho2_Kp- c_rho3_Kp
     1               - c_rhon_Kp
           c_om0_Kp = c_rho0_Kp
           c_om1_Kp = c_rho1_Kp
           c_om2_Kp = c_rho2_Kp
           c_om3_Kp = c_rho3_Kp
           c_omn_Kp = c_rhon_Kp
           c_om4_Kp = c_rho4_Kp
           read(8,*) gam_phi_Kp
           read(8,*) gam_rho_Kp
           read(8,*) gam_om_Kp
           read(8,*) m_phi0_Kp
           read(8,*) g_phi0_Kp
           read(8,*) m_phi1_Kp
           read(8,*) g_phi1_Kp
           m_phi2_Kp = m_phi0_Kp * sqrt(5.d0)
           g_phi2_Kp = m_phi2_Kp * gam_phi_Kp
           m_phi3_Kp = m_phi0_Kp * sqrt(7.d0)
           g_phi3_Kp = m_phi3_Kp * gam_phi_Kp
           read(8,*) m_om0_Kp
           read(8,*) g_om0_Kp
           read(8,*) m_om1_Kp
           read(8,*) g_om1_Kp
           read(8,*) m_om2_Kp
           read(8,*) g_om2_Kp
           m_om3_Kp = m_om0_Kp * sqrt(7.d0)
           g_om3_Kp = m_om3_Kp * gam_om_Kp
           m_om4_Kp = m_om0_Kp * sqrt(9.d0)
           g_om4_Kp = m_om4_Kp * gam_om_Kp
           read(8,*) m_rho0_Kp
           read(8,*) g_rho0_Kp
           read(8,*) m_rho1_Kp
           read(8,*) g_rho1_Kp
           read(8,*) m_rho2_Kp
           read(8,*) g_rho2_Kp
           m_rho3_Kp = m_rho0_Kp * sqrt(7.d0)
           g_rho3_Kp = m_rho3_Kp * gam_rho_Kp
           m_rho4_Kp = m_rho0_Kp * sqrt(9.d0)
           g_rho4_Kp = m_rho4_Kp * gam_rho_Kp
           read(8,*) beta_phi_Kp
           read(8,*) beta_rho_Kp
           beta_om_Kp = beta_rho_Kp

           read(8,*) eta_phi_Kp ! only for K0 K0bar

           call sum_FF_Kp()

           KaonFormFactor_Sp = KaonFormFactor(Sp)

        elseif(FF_kaon.eq.1)then  ! ------ KaonFormFactor unconstrained ----

            do i=1,196
               read(8,*)
            enddo

           read(8,*) c_phi0_Kp
           read(8,*) c_phi1_Kp
           read(8,*) c_phin_Kp
           c_phi2_Kp = 1.d0 - c_phi0_Kp - c_phi1_Kp - c_phin_Kp
           read(8,*) c_rho0_Kp
           read(8,*) c_rho1_Kp
           read(8,*) c_rho2_Kp
           read(8,*) c_rhon_Kp
           c_rho3_Kp = 1.d0- c_rho0_Kp- c_rho1_Kp- c_rho2_Kp- c_rhon_Kp
           read(8,*)c_om0_Kp
           read(8,*)c_om1_Kp
           read(8,*)c_om2_Kp
           read(8,*)c_omn_Kp
           c_om3_Kp = 1.d0- c_om0_Kp- c_om1_Kp- c_om2_Kp- c_omn_Kp
           read(8,*) gam_phi_Kp
           read(8,*) gam_rho_Kp
           read(8,*) gam_om_Kp
           read(8,*) m_phi0_Kp
           read(8,*) g_phi0_Kp
           read(8,*) m_phi1_Kp
           read(8,*) g_phi1_Kp
           m_phi2_Kp = m_phi0_Kp * sqrt(5.d0)
           g_phi2_Kp = m_phi2_Kp * gam_phi_Kp
           read(8,*) m_om0_Kp
           read(8,*) g_om0_Kp
           read(8,*) m_om1_Kp
           read(8,*) g_om1_Kp
           read(8,*) m_om2_Kp
           read(8,*) g_om2_Kp
           m_om3_Kp = m_om0_Kp * sqrt(7.d0)
           g_om3_Kp = m_om3_Kp * gam_om_Kp
           read(8,*) m_rho0_Kp
           read(8,*) g_rho0_Kp
           read(8,*) m_rho1_Kp
           read(8,*) g_rho1_Kp
           read(8,*) m_rho2_Kp
           read(8,*) g_rho2_Kp
           m_rho3_Kp = m_rho0_Kp * sqrt(7.d0)
           g_rho3_Kp = m_rho3_Kp * gam_rho_Kp
           read(8,*) beta_phi_Kp
           read(8,*) beta_rho_Kp
           read(8,*) beta_om_Kp 

           read(8,*) eta_phi_Kp ! only for K0 K0bar

           call sum_FF_Kp()

           KaonFormFactor_Sp = KaonFormFactor(Sp)

        elseif(FF_kaon.eq.2)then  ! ------ KaonFormFactor old -------

            do i=1,231
               read(8,*)
            enddo
           read(8,*) c_phi0_Kp
           read(8,*) c_phi1_Kp
           read(8,*) c_rho0_Kp
           read(8,*) c_rho1_Kp
           read(8,*) c_rho2_Kp
           read(8,*) c_om0_Kp
           read(8,*) c_om1_Kp
           read(8,*) c_om2_Kp
           read(8,*) m_phi0_Kp
           read(8,*) g_phi0_Kp
           read(8,*) m_phi1_Kp
           read(8,*) g_phi1_Kp
           read(8,*) m_om0_Kp
           read(8,*) g_om0_Kp
           read(8,*) m_om1_Kp
           read(8,*) g_om1_Kp
           read(8,*) m_om2_Kp
           read(8,*) g_om2_Kp
           read(8,*) m_rho0_Kp
           read(8,*) g_rho0_Kp
           read(8,*) m_rho1_Kp
           read(8,*) g_rho1_Kp
           read(8,*) m_rho2_Kp
           read(8,*) g_rho2_Kp

           read(8,*) eta_phi_Kp ! only for K0 K0bar

           KaonFormFactor_Sp = KaonFormFactor(Sp)

        endif

      endif
c
c 3 pi mode ---------
c
      if(pion.eq.8)then
        rmass(1) = mpi
        rmass(2) = mpi
        rmass(3) = mpi0

c ----------------------------------------------
c rho_0 +\rho_1*phi + om_2*rho_2 +  fixed rho*omega(full form of Czyz+Kuhn 
c Eur.Phys.J.C18(2001)497);
c    constrained fit
c    chi^2= 320.870727
c
            do i=1,257
               read(8,*)
            enddo
        read(8,*) omm0_3pi
        read(8,*) omg0_3pi
        read(8,*) omm1_3pi
        read(8,*) omg1_3pi         
        read(8,*) omm2_3pi
        read(8,*) omg2_3pi
        read(8,*) phim0_3pi
        read(8,*) phig0_3pi

        read(8,*) rhom0m_3pi  ! - 
        read(8,*) rhog0m_3pi  ! -
        read(8,*) rhom0p_3pi  ! + 
        read(8,*) rhog0p_3pi  ! +
        read(8,*) rhom0z_3pi  ! 0
        read(8,*) rhog0z_3pi  ! 0
        read(8,*) rhom1m_3pi  ! -
        read(8,*) rhog1m_3pi  ! -
        read(8,*) rhom1p_3pi  ! + 
        read(8,*) rhog1p_3pi  ! + 
        read(8,*) rhom1z_3pi  ! 0
        read(8,*) rhog1z_3pi  ! 0
        read(8,*) rhom2m_3pi  ! -
        read(8,*) rhog2m_3pi  ! -
        read(8,*) rhom2p_3pi  ! + 
        read(8,*) rhog2p_3pi  ! + 
        read(8,*) rhom2z_3pi  ! 0
        read(8,*) rhog2z_3pi  ! 0

        read(8,*) aa_3pi
        read(8,*) bb_3pi
        read(8,*) dd_3pi
        read(8,*) ee_3pi
c
        gg_3pi  = 1.d0/sqrt(2.d0) *1.55d0 *12.924d0 *0.266d0
     1        * 0.1846845502d0*0.77609d0**2
c
        read(8,*) jj_3pi
        read(8,*) kk_3pi
      endif
c
c Lambda (-> pi^- p) Lambda bar (-> pi^+ pbar)  parameters  ---------
c
      if(pion.eq.9)then
            do i=1,290
               read(8,*)
            enddo 
        read(8,*) mm_phi_lam
        read(8,*) mm_om_lam
        read(8,*) mu_lamb
        read(8,*) alpha_lamb
        read(8,*) phase_f2_lamb
        read(8,*) ratio_lamb
c
      endif
c
      if(pion.eq.10)then
        rmass(1) = meta
        rmass(2) = mpi
        rmass(3) = mpi
            do i=1,297
               read(8,*)
            enddo 
        read(8,*) fpi
        par_eta(1)=0.d0
        do i=2,9
          read(8,*) par_eta(i)
        enddo 
c
      endif
       

       if(pion.eq.11)then
        do i=1,307
         read(8,*)
        enddo
        read(8,*) mchic1
        read(8,*) gamchic1
        m2c=1.6945326920515436d0
        ac= 7.8610011134006008d-2
        aj= 0.14744703348488764d0 
        apsi=-6.5841506711080910d-2
c  
c        GFermi=1.1663787d-5
c        sinthW2=0.23126d0
          gg1=adg_chi()
          gamchiee=gg1*dconjg(gg1)/12.d0/pi*mchic1+ac*GFermi/3.d0/pi
     1 /dsqrt(2.d0)/dsqrt(m2c)/(2.d0/3.d0)**2*mchic1*dble(gg1)+
     2 ac**2*GFermi**2/3.d0/pi/m2c/(2.d0/3.d0)**4*mchic1*
     3 (1.d0-4.d0*sinthW2+8.d0*sinthW2**2)


       endif   
       
        if(pion.eq.12) then
          do i=1,307
            read(8,*)
          enddo
         read(8,*) mchic1
         read(8,*) gamchic1      
         read(8,*)
          read(8,*) mchic2
         read(8,*) gamchic2 
        m2c=1.6945326920515436d0
        ac= 7.8610011134006008d-2
        aj= 0.14744703348488764d0 
        apsi=-6.5841506711080910d-2
c
          gg2=adg_chi()  
          gamchiee2=dconjg(gg2)*gg2/40.d0/pi*mchic2
         endif  
      

       if((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then
         if(FF_Pgg.eq.0)then
         do i=1,313
           read(8,*)
         enddo
         elseif(FF_Pgg.eq.1)then
         do i=1,355
           read(8,*)
         enddo
         endif
         read(8,*) hv1 
         print*, hv1
         read(8,*) A1pi0
         read(8,*) A2pi0
         read(8,*) Fsmv2Hv2
         read(8,*) A1eta
         read(8,*) A1etaP
         read(8,*) fsmv1
         read(8,*) H_om
         read(8,*) F_om
         read(8,*) F_phi
         read(8,*) Fchir
         read(8,*) A_pi_phiom
         read(8,*) A_eta_phiom
         read(8,*) Cq_c
         read(8,*) Cs_c
         read(8,*) Cq_P
         read(8,*) Cs_P
         read(8,*) GammaTotalrho_c
         read(8,*) GammaTotalomega_c
         read(8,*) GammaTotalphi_c
         read(8,*) GammaTotalrho_c2
         read(8,*) GammaTotalomega_c2
         read(8,*) GammaTotalphi_c2
         read(8,*) GammaTotalrho_c3
         read(8,*) GammaTotalomega_c3
         read(8,*) GammaTotalphi_c3
         read(8,*) Mrho_c
         read(8,*) Momega_c
         read(8,*) Mphi_c
         read(8,*) Mrho_pr
         read(8,*) Momega_pr
         read(8,*) Mphi_pr
         read(8,*) Mrho_dpr
         read(8,*) Momega_dpr
         read(8,*) Mphi_dpr
         read(8,*) mpi0f
         read(8,*) metaf
         read(8,*) metaP
         read(8,*) A3pi0
         read(8,*) A3eta
         read(8,*) A3etaP
          
         endif

      if(pion.eq.0)then
c
      lambda=1.d0*sqrt(Sp)
c
      con1=2.d0*dme
      con2=(ebeam + p1(4) +el_m2/(p1(4)+ebeam))
      con3=2.d0*ebeam
      con4=con3*con3
c covariant current e+e-
      Jee11(1)=(0.d0,0.d0)
      Jee11(2)=(0.d0,0.d0)
      Jee11(3)=(0.d0,0.d0)
      Jee11(4)=con1*(-1.d0,0.d0)
c
      Jee12(1)=(0.d0,0.d0)
      Jee12(2)=con2*(-1.d0,0.d0)
      Jee12(3)=con2*(0.d0,-1.d0)
      Jee12(4)=(0.d0,0.d0)
c
      Jee21(1)=(0.d0,0.d0)
      Jee21(2)=con2*(-1.d0,0.d0)
      Jee21(3)=con2*(0.d0,1.d0)
      Jee21(4)=(0.d0,0.d0)
c
      Jee22(1)=(0.d0,0.d0)
      Jee22(2)=(0.d0,0.d0)
      Jee22(3)=(0.d0,0.d0)
      Jee22(4)=con1*(1.d0,0.d0)
c Jee-;  Jee-=-Jee+
c
      Jeemin(1,1,1,1)=con1*(1.d0,0.d0)
      Jeemin(1,1,1,2)=(0.d0,0.d0)
      Jeemin(1,1,2,1)=(0.d0,0.d0)
      Jeemin(1,1,2,2)=con1*(-1.d0,0.d0)
c
      Jeemin(1,2,1,1)=(0.d0,0.d0)
      Jeemin(1,2,1,2)=2.d0*con2*(1.d0,0.d0)
      Jeemin(1,2,2,1)=(0.d0,0.d0)
      Jeemin(1,2,2,2)=(0.d0,0.d0)
c
      Jeemin(2,1,1,1)=(0.d0,0.d0)
      Jeemin(2,1,1,2)=(0.d0,0.d0)
      Jeemin(2,1,2,1)=2.d0*con2*(1.d0,0.d0)
      Jeemin(2,1,2,2)=(0.d0,0.d0)
c
      Jeemin(2,2,1,1)=con1*(-1.d0,0.d0)
      Jeemin(2,2,1,2)=(0.d0,0.d0)
      Jeemin(2,2,2,1)=(0.d0,0.d0)
      Jeemin(2,2,2,2)=con1*(1.d0,0.d0)
c
c Jee-*(p1+ + p2+)
c Jee-*(p1+ + p2+)=(p1+ + p2+)*Jee-
c Jee-*(p1+ + p2+)=-Jee+*(p1- + p2-
c)
      cpp(1,1,1,1)=con3*Jeemin(1,1,1,1)
      cpp(1,1,1,2)=(0.d0,0.d0)
      cpp(1,1,2,1)=(0.d0,0.d0)
      cpp(1,1,2,2)=con3*Jeemin(1,1,2,2)
c
      cpp(1,2,1,1)=(0.d0,0.d0)
      cpp(1,2,1,2)=con3*Jeemin(1,2,1,2)
      cpp(1,2,2,1)=(0.d0,0.d0)
      cpp(1,2,2,2)=(0.d0,0.d0)
c
      cpp(2,1,1,1)=(0.d0,0.d0)
      cpp(2,1,1,2)=(0.d0,0.d0)
      cpp(2,1,2,1)=con3*Jeemin(2,1,2,1)
      cpp(2,1,2,2)=(0.d0,0.d0)
c
      cpp(2,2,1,1)=con3*Jeemin(2,2,1,1)
      cpp(2,2,1,2)=(0.d0,0.d0)
      cpp(2,2,2,1)=(0.d0,0.d0)
      cpp(2,2,2,2)=con3*Jeemin(2,2,2,2)
      endif
c
      ! ========================================================================
      !  added by Kirill Chilikin, 2019
      ! ========================================================================
      close(8)
      ! ========================================================================      return
      end
c***********************************************************************
      subroutine init_ha
      implicit none
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1           k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2), 
     3        gampl_ma(2,2),gammi_ma(2,2),mk1(4,2,2),mk2(4,2,2),
     4        sigplsmi(2,2),splsigmi(2,2),sigmispl(2,2),smisigpl(2,2),
     5        smi(2,2),spl(2,2)
      integer i1,i2,i3,i4
      real*8 sm(4)
c
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/newmag/mk1,mk2 
      common/ssmm/sm
c
      I(1,1)=dcmplx(1.d0,0.d0)
      I(1,2)=dcmplx(0.d0,0.d0)
      I(2,1)=dcmplx(0.d0,0.d0)
      I(2,2)=dcmplx(1.d0,0.d0)
c
      do i1 = 1,2
       do i2 = 1,2
        sigpl(1,i1,i2) = I(i1,i2)
        sigmi(1,i1,i2) = I(i1,i2)
       enddo
      enddo
c
      do i1 = 1,2
       sigpl(2,i1,i1)=dcmplx(0.d0,0.d0)
       sigpl(3,i1,i1)=dcmplx(0.d0,0.d0)
       sigpl(4,i1,i1)=dcmplx((-1.d0)**(i1+1),0.d0)
      enddo
c
       sigpl(2,1,2)=dcmplx(1.d0,0.d0)
       sigpl(2,2,1)=dcmplx(1.d0,0.d0)
       sigpl(3,1,2)=dcmplx(0.d0,-1.d0)
       sigpl(3,2,1)=dcmplx(0.d0,1.d0)
       sigpl(4,1,2)=dcmplx(0.d0,0.d0)
       sigpl(4,2,1)=dcmplx(0.d0,0.d0)
c
      do i3 = 2,4
       do i1 = 1,2
        do i2 = 1,2
         sigmi(i3,i1,i2) = -sigpl(i3,i1,i2)
        enddo
       enddo
      enddo
             call plus(sm,spl)
       call minus(sm,smi)
      do i1=1,4
       call matr1(i1,sigpl,smi,sigplsmi)
       call matr2(i1,spl,sigmi,splsigmi)
       call matr1(i1,sigmi,spl,sigmispl)
       call matr2(i1,smi,sigpl,smisigpl)
       call minmat_new(i1,sigplsmi,splsigmi,mk1)
      do i2=1,2
        do i3=1,2
         mk2(i1,i2,i3)=-mk1(i1,i2,i3)
        enddo
       enddo
c       call minmat_new(i1,sigmispl,smisigpl,mk2)
      enddo

      return
      end 
c ========================================================================
      subroutine qquadrat_0(qqmin,qqmax,cosmin,cosmax,Ar
     1,qq,q0p,q2p,q0b,q2b,costheta,phi,jacobian)
      include 'phokhara_10.0.inc'
      real*8 qqmin,qqmax,Ar(14),qq,jacobian,fak1c,
     &  x,a,b,c,amin,amax,bmin,bmax,fak1,fak2,fak3,p,y,ppp,jac4,vv,eqt,
     &  cosmin,cosmax,costheta,phi,delcos,vol1,vol2,vol3,vol4,cmin,cmax,
     &  delyy,yy,ymin,ea,q0p,q2p,q0b,q2b,fak4,d,dmin,dmax,fak6,fak7,
     &  e_1,e_1min,e_1max
      real*8 pionFF,dps
      real*8  Qjp,QgK2,Qp2s,QgK2_p2s 
      complex*16 g_eK_jp,g_eK_p2s
      common /jpparam/ Qjp,g_eK_jp
      common /p2sparam/ Qp2s,g_eK_p2s  
      common /genres/QgK2,QgK2_p2s
      
      x = Ar(1)      
c
c --- muons -----------------------------------------------------
c --- the Q2 distribution is peaked at threshold and Q2->Sp (soft photon). 
      if (pion.eq.0) then
c
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jac4      
c
c --- 2 pions -----------------------------------------------------
c --- the Q2 distribution is peaked at Q2=rho and w mass^2       
c --- resonances (pion form factor) and at Q2->Sp (soft photon). 
c
      elseif(pion.eq.1)then
c  isr only
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jac4      
c
c --- 4 pions -----------------------------------------------------
      elseif((pion.eq.2).or.(pion.eq.3))then
c
       call pionangles_1(qq,q0p,q2p,q0b,q2b,jac4)
c
      jacobian = jac4      
c
c----PPbar and NNbar----------------------------------------------
      elseif((pion.eq.4).or.(pion.eq.5)) then 
c
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jac4 
c 
c ---- K^+K^- and K0K0bar---------------------------------------------
      elseif((pion.eq.6).or.(pion.eq.7))then
c
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jac4      
c       
c --- 3 pions -----------------------------------------------------
c
      elseif((pion.eq.8))then
 
       call pionangles_2(qq,q0p,q2p,jacobian)
c
      elseif((pion.eq.10))then
 
       call pionangles_eta(qq,q0p,q2p,jacobian)
c
c--- Lambda (Lambda bar) -------------------------
      elseif(pion.eq.9) then 
 
c
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)

      pionFF = 4.d0*pi*alpha
      dps = dSqrt(1.d0-4.d0*mlamb*mlamb/qq)/(32.d0*pi*pi)!Phase space factors

      jacobian =  jac4 * pionFF * dps
     
      elseif((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then
   
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jac4 
      else
       continue
      endif
      return
      end
c-----------------------------------------------------------------------
c  virual and soft parts for O photon mode
c
      subroutine Oph_emission_virtualsoft(delta)
      include 'phokhara_10.0.inc'
      real*8 L_log,L_log2,L_log3,L_log4
      real*8 eps_cut,coeff1,coeff1_2
      real*8 dzeta2,dzeta3,delta,betae
      real*8 ReF_1,ImF_1,modF_1_2,ReF_2
      real*8 delta_s1_1,delta_v1_1
      real*8 delta_s2_2,delta_v2_2,delta_s1v1_2 
c
c names from Berends
      L_log  = dlog(Sp/me**2)
      L_log2 = L_log*L_log
      L_log3 = L_log2*L_log
      L_log4 = L_log3*L_log
      eps_cut = 2d0*delta/dsqrt(Sp)
      coeff1 = alpha/pi
      coeff1_2 = coeff1*coeff1
      dzeta2 = pi*pi/6.d0
      dzeta3 = 1.202056903159594d0
c      dzeta3 = 1.2020569032d0
      betae = sqrt(1.d0-4.d0*me**2/Sp)
c functions 

      ReF_1 = coeff1*(-L_log2/4.d0 +3.d0*L_log/4.d0 -1.d0 + 2.d0*dzeta2)
      ImF_1 = coeff1*pi*(L_log/2.d0 -3.d0/4.d0)
      modF_1_2= ReF_1**2 + ImF_1**2
c
      ReF_2 = coeff1_2*(L_log4/32.d0 - 3.d0*L_log3/16.d0
     &      +  ( 17.d0/32.d0 - 5.d0*dzeta2/4.d0)*L_log2
     &      +  (-21.d0/32.d0 + 3.d0*dzeta2 +3.d0/2.d0*dzeta3)*L_log
     &      +   2.d0*dzeta2**2/5.d0 - 9.d0/4.d0*dzeta3 
     &      -   3.d0*dzeta2*log(2.d0) - dzeta2/2.d0 + 405d0/216.d0)

c_____________________ one photon: soft and virtual ____________________
c
c one soft photon emission - without part that canceled with virtual part
c mass corrections added
c      delta_s1_1 = 0.5d0*L_log2 
c     1  + 2.d0*dlog(eps_cut)*((1+betae**2)/2.d0/betae
c     2     *dlog((1.d0+betae)**2/4.d0*Sp/me**2) -1.d0)
c     &   - 2.d0*dzeta2
      delta_s1_1 = 0.5d0*L_log2 + 2.d0*dlog(eps_cut)*L_log
     &  - 2.d0*dlog(eps_cut) - 2.d0*dzeta2
c
      delta_s1_1 = coeff1*delta_s1_1
c
c one photon virtual part
c
c
      delta_v1_1 = 2.d0*ReF_1
c
c_____________________ two photons: soft and virtual ___________________
c
c missing part with 2 soft photons
c
      delta_s2_2 = delta_s1_1*delta_s1_1/2.d0

c two virtual particles
c
      delta_v2_2 = modF_1_2 + 2.d0* ReF_2
c
c one virtual and one soft
c
      delta_s1v1_2 = delta_s1_1*delta_v1_1
c
c     full correction
      if(nlo.eq.0)then
      Ophvirtsoft =  delta_s1_1 + delta_v1_1                 ! one photon
      else
      Ophvirtsoft =  delta_s1_1 + delta_v1_1                 ! one photon
     &   + delta_s2_2 + delta_v2_2 + delta_s1v1_2            ! two photons
      endif
      if(ph0.eq.(-1))Ophvirtsoft = 0.d0
c
      return
      end
c ------------------------------------------------------------------------
c --- Generates: photon virtuality Q2 using a two-component substitution -
c used in one photon emission
c ------------------------------------------------------------------------
      subroutine qquadrat(qqmin,qqmax,cosmin,cosmax,Ar
     1,qq,q0p,q2p,q0b,q2b,costheta,phi,jacobian)
      include 'phokhara_10.0.inc'
      real*8 qqmin,qqmax,Ar(14),qq,jacobian,fak1c,
     &  x,a,b,c,amin,amax,bmin,bmax,fak1,fak2,fak3,p,y,ppp,jac4,vv,eqt,
     &  cosmin,cosmax,costheta,phi,delcos,vol1,vol2,vol3,vol4,cmin,cmax,
     &  delyy,yy,ymin,ea,q0p,q2p,q0b,q2b,fak4,d,dmin,dmax,fak6,fak7,
     &  e_1,e_1min,e_1max,ehmin,ehmax,eh,fakeh
      real*8 pionFF,dps
      real*8  Qjp,QgK2,Qp2s,QgK2_p2s 
      complex*16 g_eK_jp,g_eK_p2s
      common /jpparam/ Qjp,g_eK_jp
      common /p2sparam/ Qp2s,g_eK_p2s  
      common /genres/QgK2,QgK2_p2s
      
      x = Ar(1)      
c
c --- muons -----------------------------------------------------
c --- the Q2 distribution is peaked at threshold and Q2->Sp (soft photon). 
      if ((pion.eq.0).or.(pion.eq.11).or.(pion.eq.12)) then
c
       if(pion.eq.0)then
c  isr only
      if(fsr.eq.0)then
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      bmin = dLog(qqmin/Sp)/Sp
      b    = dLog(qqmax/qqmin)/Sp

c --- which substitution? ---
      p = Ar(7)      

! J/psi
      if(narr_res.eq.1)then  

        fak4 = 1.D0/3700.D0
        cmin = Qjp**2*mjp*gamjp*fak4*dAtan((qqmin-mjp**2)/(mjp*gamjp))
        cmax = Qjp**2*mjp*gamjp*fak4*dAtan((qqmax-mjp**2)/(mjp*gamjp))
        c = cmax-cmin

        ppp  = a+b+c
        if(p.lt.a/ppp)then
           y  = amin+a*x
           qq = Sp-dExp(y/fak1)                                       
        elseif(p.lt.((a+b)/ppp))then
           y  = bmin+b*x
           qq = Sp*exp(Sp*y)
        else
           y = cmin+c*x 
           qq = mjp*gamjp*dtan(y/fak4/mjp/gamjp/Qjp**2) + mjp**2
        endif
        jacobian = ppp/(1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq  
     1  + Qjp**2*mjp**2*gamjp**2*fak4/((mjp**2-qq)**2+mjp**2*gamjp**2) ) 
! Psi(2S)
      elseif(narr_res.eq.2)then 
        fak6 = 1.D0/710.d0
      dmin=Qp2s**2*mp2s*gamp2s*fak6*dAtan((qqmin-mp2s**2)/(mp2s*gamp2s))
      dmax=Qp2s**2*mp2s*gamp2s*fak6*dAtan((qqmax-mp2s**2)/(mp2s*gamp2s))
        d = dmax-dmin

        ppp  = a+b+d
        if(p.lt.a/ppp)then
           y  = amin+a*x
           qq = Sp-dExp(y/fak1)                                       
        elseif(p.lt.((a+b)/ppp))then
           y  = bmin+b*x
           qq = Sp*exp(Sp*y)
        else
           y = dmin+d*x 
           qq = mp2s*gamp2s*dtan(y/fak6/mp2s/gamp2s/Qp2s**2) + mp2s**2
        endif
        jacobian = ppp/(1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq  
     2  + Qp2s**2*mp2s**2*gamp2s**2*fak6
     3                            /((mp2s**2-qq)**2+mp2s**2*gamp2s**2) ) 
! no narrow resonances
      else

         ppp  = a/(a+b)
         if(p.lt.ppp)then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1)                                       
         else
            y  = bmin+b*x
            qq = Sp*exp(Sp*y)
         endif
         jacobian = (a+b)/(1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq)  

      endif

      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jacobian*jac4      
c
c isr+fsr+int
      else
c
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      a = fak1 *dLog((Sp-qqmax)/(Sp-qqmin))
      delcos = (cosmax-cosmin)
      vol1 = a*delcos
c
      bmin = dLog(qqmin/Sp)/Sp
      b    = dLog(qqmax/qqmin)/Sp
      vol2 = b*delcos
c
      vv = sqrt(1.d0-4.d0*mpi**2/Sp)
      delyy = log((1.d0+vv*cosmax)*(1.d0-vv*cosmin)
     1          /(1.d0-vv*cosmax)/(1.d0+vv*cosmin)) /vv
      vol3 = a*delyy 
c
      phi = 2.d0*pi*Ar(5)
c
c --- four channels ---
c 1. soft photon + angles flat
c 2.  1/q2       + angles flat
c 3. soft photon + angles fsr collinear
c 4. J/Psi 
c
      p = Ar(7)      

! J/psi
      if(narr_res.eq.1)then
         fak3 = 1700.D0
       dmin = Qjp**2*mjp*gamjp/fak3*dAtan((qqmin-mjp**2)/(mjp*gamjp))
       dmax = Qjp**2*mjp*gamjp/fak3*dAtan((qqmax-mjp**2)/(mjp*gamjp))
         d = dmax-dmin
         vol4 = d*delcos

         ppp  = vol1+vol2+vol3+vol4
         if(p.lt.vol1/ppp)then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1)                                       
            costheta = cosmin+delcos*Ar(4)
         elseif(p.lt.((vol1+vol2)/ppp))then
            y  = bmin+b*x
            qq = Sp*exp(Sp*y)
            costheta = cosmin+delcos*Ar(4)
         elseif(p.lt.((vol1+vol2+vol3)/ppp))then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1) 
            ymin = log((1.d0+vv*cosmin)/(1.d0-vv*cosmin)) /vv
            yy = ymin + delyy *Ar(4)
            ea = exp(vv*yy)
            costheta = (ea-1.d0)/(1.d0+ea)/vv
         else
            y = dmin+d*x 
            qq = mjp*gamjp*dtan(y*fak3/mjp/gamjp/Qjp**2) + mjp**2
            costheta = cosmin+delcos*Ar(4)
         endif
c
         jacobian = 2.d0*pi*ppp/( 1.d0/(Sp*(Sp-qq))
     1     +  1.d0/Sp/qq + 1.d0/(Sp*(Sp-qq))
     2     * (1.d0/(1.d0-vv*costheta)+1.d0/(1.d0+vv*costheta))
     3     + Qjp**2*mjp**2*gamjp**2/fak3
     4       /((mjp**2-qq)**2+mjp**2*gamjp**2) )
c Psi(2S)
       elseif(narr_res.eq.2)then
        fak6 = 1.D0/1140.d0
      dmin=Qp2s**2*mp2s*gamp2s*fak6*dAtan((qqmin-mp2s**2)/(mp2s*gamp2s))
      dmax=Qp2s**2*mp2s*gamp2s*fak6*dAtan((qqmax-mp2s**2)/(mp2s*gamp2s))
        d = dmax-dmin
         vol4 = d*delcos

         ppp  = vol1+vol2+vol3+vol4
         if(p.lt.vol1/ppp)then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1)                                       
            costheta = cosmin+delcos*Ar(4)
         elseif(p.lt.((vol1+vol2)/ppp))then
            y  = bmin+b*x
            qq = Sp*exp(Sp*y)
            costheta = cosmin+delcos*Ar(4)
         elseif(p.lt.((vol1+vol2+vol3)/ppp))then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1) 
            ymin = log((1.d0+vv*cosmin)/(1.d0-vv*cosmin)) /vv
            yy = ymin + delyy *Ar(4)
            ea = exp(vv*yy)
            costheta = (ea-1.d0)/(1.d0+ea)/vv
         else
            y = dmin+d*x 
            qq = mp2s*gamp2s*dtan(y/fak6/mp2s/gamp2s/Qp2s**2) + mp2s**2
            costheta = cosmin+delcos*Ar(4)
         endif
c
         jacobian = 2.d0*pi*ppp/( 1.d0/(Sp*(Sp-qq))
     1     +  1.d0/Sp/qq + 1.d0/(Sp*(Sp-qq))
     2     * (1.d0/(1.d0-vv*costheta)+1.d0/(1.d0+vv*costheta))
     3     +  Qp2s**2*mp2s**2*gamp2s**2*fak6
     4                            /((mp2s**2-qq)**2+mp2s**2*gamp2s**2) )
c
! no narrow resonaces
       else
         ppp  = vol1+vol2+vol3
         if(p.lt.vol1/ppp)then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1)                                       
            costheta = cosmin+delcos*Ar(4)
         elseif(p.lt.((vol1+vol2)/ppp))then
            y  = bmin+b*x
            qq = Sp*exp(Sp*y)
            costheta = cosmin+delcos*Ar(4)
         else
            y  = amin+a*x
            qq = Sp-dExp(y/fak1) 
            ymin = log((1.d0+vv*cosmin)/(1.d0-vv*cosmin)) /vv
            yy = ymin + delyy *Ar(4)
            ea = exp(vv*yy)
            costheta = (ea-1.d0)/(1.d0+ea)/vv
         endif
c
         jacobian = 2.d0*pi*ppp/( 1.d0/(Sp*(Sp-qq))
     1     +  1.d0/Sp/qq + 1.d0/(Sp*(Sp-qq))
     2     * (1.d0/(1.d0-vv*costheta)+1.d0/(1.d0+vv*costheta)) )

       endif

       endif
       
       elseif(pion.eq.11)then 
c     chi_c production chi_c -> J/psi (-> mu+mu-)+gamma
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      bmin = dLog(qqmin/Sp)/Sp
      b    = dLog(qqmax/qqmin)/Sp

c --- which substitution? ---
      p = Ar(7)      

        fak4 = 1.D0/70.D0
        cmin = Qjp**2*mjp*gamjp*fak4*dAtan((qqmin-mjp**2)/(mjp*gamjp))
        cmax = Qjp**2*mjp*gamjp*fak4*dAtan((qqmax-mjp**2)/(mjp*gamjp))
        c = cmax-cmin

        ppp  = a+b+c
        if(p.lt.a/ppp)then
           y  = amin+a*x
           qq = Sp-dExp(y/fak1)                                       
        elseif(p.lt.((a+b)/ppp))then
           y  = bmin+b*x
           qq = Sp*exp(Sp*y)
        else
           y = cmin+c*x 
           qq = mjp*gamjp*dtan(y/fak4/mjp/gamjp/Qjp**2) + mjp**2
        endif
        jacobian = ppp/(1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq  
     1  + Qjp**2*mjp**2*gamjp**2*fak4/((mjp**2-qq)**2+mjp**2*gamjp**2) ) 


      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jacobian*jac4  
      elseif(pion.eq.12)then 
c     chi_c production chi_c -> J/psi (-> mu+mu-)+gamma
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      bmin = dLog(qqmin/Sp)/Sp
      b    = dLog(qqmax/qqmin)/Sp

c --- which substitution? ---
      p = Ar(7)      

        fak4 = 1.D0/70.D0
        cmin = Qjp**2*mjp*gamjp*fak4*dAtan((qqmin-mjp**2)/(mjp*gamjp))
        cmax = Qjp**2*mjp*gamjp*fak4*dAtan((qqmax-mjp**2)/(mjp*gamjp))
        c = cmax-cmin

        ppp  = a+b+c
        if(p.lt.a/ppp)then
           y  = amin+a*x
           qq = Sp-dExp(y/fak1)                                       
        elseif(p.lt.((a+b)/ppp))then
           y  = bmin+b*x
           qq = Sp*exp(Sp*y)
        else
           y = cmin+c*x 
           qq = mjp*gamjp*dtan(y/fak4/mjp/gamjp/Qjp**2) + mjp**2
        endif
        jacobian = ppp/(1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq  
     1  + Qjp**2*mjp**2*gamjp**2*fak4/((mjp**2-qq)**2+mjp**2*gamjp**2) ) 


      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jacobian*jac4      
        endif
c
c --- 2 pions -----------------------------------------------------
c --- the Q2 distribution is peaked at Q2=rho and w mass^2       
c --- resonances (pion form factor) and at Q2->Sp (soft photon). 
c
      elseif(pion.eq.1)then
c
        if((FF_pion.eq.0).or.(FF_pion.eq.1)) then 
           fak4 = 1.d0/320.d0
           fak6 = 1.d0/320.d0
        endif

        if(FF_pion.eq.2) then
           fak4 = 1.d0/308.d0   
           fak6 = 1.d0/308.d0   
        endif

c  isr only
      if(fsr.eq.0)then

        call qquadrat_1(qqmin,qqmax,x,Ar(7),
     1                  m_rho0_pion,g_rho0_pion,fak4,fak6,qq,jacobian)
c
        call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jacobian*jac4      
c
c isr+fsr+int
      else
c
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      a = fak1 *dLog((Sp-qqmax)/(Sp-qqmin))
      delcos = (cosmax-cosmin)
      vol1 = a*delcos
c
      fak2 = 1.d0/g_rho0_pion/m_rho0_pion
      bmin = fak2*dAtan((qqmin-m_rho0_pion**2)*fak2)
      bmax = fak2*dAtan((qqmax-m_rho0_pion**2)*fak2)
      b = bmax-bmin
      vol2 = b*delcos
c
      vv = sqrt(1.d0-4.d0*mpi**2/Sp)
      delyy = log((1.d0+vv*cosmax)*(1.d0-vv*cosmin)
     1          /(1.d0-vv*cosmax)/(1.d0+vv*cosmin)) /vv
      vol3 = a*delyy 
c
      phi = 2.d0*pi*Ar(5)
c
c --- three channels ---
c 1. soft photon + angles flat
c 2. BW q2       + angles flat
c 3. soft photon + angles fsr collinear
c
      p = Ar(7)      

        ppp  = vol1+vol2+vol3
        if(p.lt.vol1/ppp)then
          y  = amin+a*x
          qq = Sp-dExp(y/fak1)                                       
          costheta = cosmin+delcos*Ar(4)
        elseif(p.lt.((vol1+vol2)/ppp))then
          y  = bmin+b*x
          qq = m_rho0_pion*(m_rho0_pion+g_rho0_pion*dTan(y/fak2))
          costheta = cosmin+delcos*Ar(4)
        else
          y  = amin+a*x
          qq = Sp-dExp(y/fak1) 
          ymin = log((1.d0+vv*cosmin)/(1.d0-vv*cosmin)) /vv
          yy = ymin + delyy *Ar(4)
          ea = exp(vv*yy)
          costheta = (ea-1.d0)/(1.d0+ea)/vv
        endif
c
        jacobian = 2.d0*pi*ppp/( (1.d0/(Sp*(Sp-qq)) +
     &     1.d0/(((qq-m_rho0_pion**2)**2+(g_rho0_pion*m_rho0_pion)**2))) 
     &    +1.d0/(Sp*(Sp-qq))
     &    *(1.d0/(1.d0-vv*costheta)+1.d0/(1.d0+vv*costheta)) )
c
      endif
c
c --- 4 pions -----------------------------------------------------
      elseif((pion.eq.2).or.(pion.eq.3))then
        call qquadrat_1(qqmin,qqmax,x,Ar(7),1.5d0,0.5d0,
     1                  0.d0,0.d0,qq,jacobian)
c
       call pionangles_1(qq,q0p,q2p,q0b,q2b,jac4)
      jacobian = jacobian*jac4      
c
c----PPbar and NNbar----------------------------------------------
      elseif((pion.eq.4).or.(pion.eq.5)) then 
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      bmin = dLog(qqmin/Sp)/Sp
      b    = dLog(qqmax/qqmin)/Sp
c --- which substitution? ---
      p = Ar(7)      
      ppp  = a/(a+b)
      if(p.lt.ppp)then
         y  = amin+a*x
         qq = Sp-dExp(y/fak1)                                       
      else
         y  = bmin+b*x
         qq = Sp*exp(Sp*y)
      endif
      jacobian = (a+b)/(1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq)  
c
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jacobian*jac4 
c 
c ---- K^+K^- and K0K0bar---------------------------------------------
      elseif((pion.eq.6).or.(pion.eq.7))then

      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      fak2 = 1.d0/g_phi0_Kp/m_phi0_Kp
      bmin = fak2*dAtan((qqmin-m_phi0_Kp**2)*fak2)
      bmax = fak2*dAtan((qqmax-m_phi0_Kp**2)*fak2)
      b = bmax-bmin
      cmin = dLog(qqmin/Sp)*Sp
      c = dLog(qqmax/qqmin)*Sp

c --- 5 channels ---
c  1.  1/Sp/(Sp-qq)
c  2.  BW
c  3.  Sp/qq
c  4. J/Psi BW
c  5. Psi(2S) BW
      p = Ar(7)      
!J/psi
      if(narr_res.eq.1)then
         fak6 = 2.5D0
         dmin = QgK2*fak6*dAtan( (qqmin-mjp**2)/(mjp*gamjp) )/mjp/gamjp
         dmax = QgK2*fak6*dAtan( (qqmax-mjp**2)/(mjp*gamjp) )/mjp/gamjp
         d = dmax-dmin

         ppp  = a+b+c+d
c
         if(p.lt.a/ppp)then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1)                                       
         elseif(p.lt.((a+b)/ppp))then
            y  = bmin+b*x
            qq = m_phi0_Kp*(m_phi0_Kp+g_phi0_Kp*dTan(y/fak2))
         elseif(p.lt.((a+b+c)/ppp))then
            y  = cmin+c*x
            qq = Sp*exp(y/Sp)
         else
            y  = dmin+d*x
            qq = mjp*gamjp*dtan(y*mjp*gamjp/QgK2/fak6) + mjp**2
         endif

         jacobian = ppp/( (1.d0/(Sp*(Sp-qq)) +
     1     1.d0/(((qq-m_phi0_Kp**2)**2+(g_phi0_Kp*m_phi0_Kp)**2))) +
     2     1.d0/(qq/Sp) + QgK2*fak6/((mjp**2-qq)**2+mjp**2*gamjp**2) )
!Psi(2S)
      elseif(narr_res.eq.2)then

         fak7 = 75.D0
         e_1min = QgK2_p2s*fak7*dAtan( (qqmin-mp2s**2)/(mp2s*gamp2s) )
     &       /mp2s/gamp2s
         e_1max = QgK2_p2s*fak7*dAtan( (qqmax-mp2s**2)/(mp2s*gamp2s) )
     &       /mp2s/gamp2s
         e_1 = e_1max-e_1min      

         ppp  = a+b+c+e_1
c
         if(p.lt.a/ppp)then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1)                                       
         elseif(p.lt.((a+b)/ppp))then
            y  = bmin+b*x
            qq = m_phi0_Kp*(m_phi0_Kp+g_phi0_Kp*dTan(y/fak2))
         elseif(p.lt.((a+b+c)/ppp))then
            y  = cmin+c*x
            qq = Sp*exp(y/Sp)
         else
            y  = e_1min+e_1*x
            qq = mp2s*gamp2s*dtan(y*mp2s*gamp2s/QgK2_p2s/fak7) + mp2s**2
         endif
   
         jacobian = ppp/( (1.d0/(Sp*(Sp-qq)) +
     1     1.d0/(((qq-m_phi0_Kp**2)**2+(g_phi0_Kp*m_phi0_Kp)**2))) +
     2     1.d0/(qq/Sp) 
     3     + QgK2_p2s*fak7/((mp2s**2-qq)**2+mp2s**2*gamp2s**2) )
!no resonances
      else

         ppp  = a+b+c
c
         if(p.lt.a/ppp)then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1)                                       
         elseif(p.lt.((a+b)/ppp))then
            y  = bmin+b*x
            qq = m_phi0_Kp*(m_phi0_Kp+g_phi0_Kp*dTan(y/fak2))
         else
            y  = cmin+c*x
            qq = Sp*exp(y/Sp)
         endif

         jacobian = ppp/( (1.d0/(Sp*(Sp-qq)) +
     1     1.d0/(((qq-m_phi0_Kp**2)**2+(g_phi0_Kp*m_phi0_Kp)**2))) +
     2     1.d0/(qq/Sp) )

      endif
c
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)
c
      jacobian = jacobian*jac4      
c       
c --- 3 pions -----------------------------------------------------
c
      elseif((pion.eq.8))then
       call qquadrat_2(qq,qqmax,qqmin,x,Ar(7),jac4)  
       call pionangles_2(qq,q0p,q2p,jacobian)
c
       jacobian = jacobian*jac4
      elseif((pion.eq.10))then
       call qquadrat_eta(qq,qqmax,qqmin,x,Ar(7),jac4)  
       call pionangles_eta(qq,q0p,q2p,jacobian)
c
       jacobian = jacobian*jac4
c
c--- Lambda (Lambda bar) -------------------------
      elseif(pion.eq.9) then 
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      bmin = dLog(qqmin/Sp)/Sp
      b    = dLog(qqmax/qqmin)/Sp
c --- which substitution? ---
      p = Ar(7)      
      ppp  = a/(a+b)
      if(p.lt.ppp)then
         y  = amin+a*x
         qq = Sp-dExp(y/fak1)                                       
      else
         y  = bmin+b*x
         qq = Sp*exp(Sp*y)
      endif
      jacobian = (a+b)/(1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq)  
c
      call pionangles(cosmin,cosmax,Ar,costheta,phi,jac4)

      pionFF = 4.d0*pi*alpha
      dps = dSqrt(1.d0-4.d0*mlamb*mlamb/qq)/(32.d0*pi*pi)!Phase space factors

      jacobian = jacobian * jac4 * pionFF * dps
      
      else
       continue
      endif
      return
      end
c-----------------------------------------------------------------------
c---------------------------------------------------------------------
c-----------------------------------------------------------------------      
      subroutine qquadrat_Pg1(qq,qqmin,qqmax,cos1min,cos1max,
     &                   cos3min,cos3max,Ar,jacobian)
      include 'phokhara_10.0.inc'
      real*8 qqmin,qqmax,Ar(14),qq,jacobian,fak1c,
     &  x,a,b,c,amin,amax,bmin,bmax,fak1,fak2,fak3,p,y,ppp,jac4,vv,eqt,
     &  cosmin,cosmax,costheta,phi,delcos,vol1,vol2,vol3,vol4,cmin,cmax,
     &  delyy,yy,ymin,ea,q0p,q2p,q0b,q2b,fak4,d,dmin,dmax,fak6,fak7,
     &  e_1,e_1min,e_1max,ehmin,ehmax,eh,fakeh
      real*8 cos1min,cos1max,cos3min,cos3max
      real*8 mincos1,mincos2,fakch,
     1 volch,pch,minqch1,minqch2,maxqch1,maxqch2,qch1,qch2
      real*8 phi1,phi3,x1,x3,rat,cosmax2,cosmin2,bb,a1,a2,cos3,cos1,
     1 b1,b2,sum_de,sin1,sin3,E,jac2,q2,z1,z2,qq1,qq2,Mpsc,
     2 pompom
      integer i,l1,l2
      common/qqvec12/qq1,qq2
        

      if(pion.eq.13)then !pi0
        Mpsc=mpi0f
         
           fak1 = -1.d0/Sp
           fak4 = 27.d0
           fak6 = 1.d0/3.d0
           fakeh=1.d0/44.d0         
 
      elseif(pion.eq.14)then !eta
       Mpsc=metaf
         
           fak1 = -1.d0/Sp
           fak4 = 1.d0
           fak6 = 42.d0
           fakeh=1.d0
           
      elseif(pion.eq.15)then !eta'
       Mpsc=metaP

           fak1 = -1.d0/Sp
           fak4 = 1.d0
           fak6 = 1.d0
           fakeh=1.d0

      endif
      
      pch=Ar(8)  ! for choosing channel
      x = Ar(1)  ! for generation of q^2
      p = Ar(7)  ! q^2 channels
   
      x1   = Ar(2) !k1
      x3   = Ar(4) !k2
c     
c    azimuthal angles
c
      phi1 = 2.d0*pi*Ar(3) !k1
      phi3 = 2.d0*pi*Ar(5) !k2
      rat = 4.d0*me*me/Sp

c         polar angles      
      
      bb = dSqrt(1.d0-rat)
      mincos1 = dLog((rat)/(1.d0+bb)**2)
     1      /(2.d0*bb)
      a1 = dLog( (1+bb)**2/rat)
     2          / (bb)


c---------------------------------------------------------------------      
c for Q^2 
      qqmax=Sp*(1.d0-2.d0*w)
      qqmin=Mpsc**2
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      bmin = dLog(qqmin/Sp)/Sp
      b    = dLog(qqmax/qqmin)/Sp

c --- which substitution? ---
            
      
      cmin = Momega_c*GammaTotalomega_c*fak4*dAtan((qqmin-Momega_c**2)/
     1 (Momega_c*GammaTotalomega_c))
      cmax = Momega_c*GammaTotalomega_c*fak4*dAtan((qqmax-Momega_c**2)/
     2 (Momega_c*GammaTotalomega_c))
      c = cmax-cmin

        
        dmin = Mphi_c*GammaTotalphi_c*fak6*dAtan((qqmin-Mphi_c**2)/
     1 (Mphi_c*GammaTotalphi_c))
      dmax = Mphi_c*GammaTotalphi_c*fak6*dAtan((qqmax-Mphi_c**2)/
     2 (Mphi_c*GammaTotalphi_c))
        d = dmax-dmin
         
         ehmin=fakeh*qqmin
         ehmax=fakeh*qqmax
         eh=ehmax-ehmin
c----------------------------------------------------------------------
        ppp  = a+b+c+d+eh

 
        if(p.lt.a/ppp)then
           y  = amin+a*x
           qq1 = Sp-dExp(y/fak1)                                       
        elseif(p.lt.((a+b)/ppp))then
           y  = bmin+b*x
           qq1 = Sp*exp(Sp*y)
        elseif(p.lt.((a+b+c)/ppp))then
           y = cmin+c*x 
           qq1 = Momega_c*GammaTotalomega_c*
     1 dtan(y/fak4/Momega_c/GammaTotalomega_c) + Momega_c**2
        elseif(p.lt.((a+b+c+d)/ppp))then
           y = dmin+d*x 
           qq1 = Mphi_c*GammaTotalphi_c*
     1 dtan(y/fak6/Mphi_c/GammaTotalphi_c) + Mphi_c**2
        else
          y=ehmin+x*eh
          qq1=y/fakeh
         
        endif

        
       
       qq=qq1
       
       y = mincos1+x1*a1
       onemb2c2 = 1.d0/(dcosh(bb*y))**2
       cos1 = dTanh(bb*y)/bb
       cos3 = cos3min+(cos3max-cos3min)*x3
       z1=1.d0/(1.d0-bb**2*cos1**2)
       
      
      jac2 = 4.d0*pi*2.d0*pi*a1


c------------------------- Momenta-------------------------------------
c----------------------------------------------------------------------
      q2 = qq/Sp
      sin1 = dSqrt(1.d0-cos1*cos1)
      E = (1.d0-q2)*dSqrt(Sp)/2.d0
      momenta(3,0) = E
      momenta(3,1) = E*sin1*dCos(phi1)
      momenta(3,2) = E*sin1*dSin(phi1)
      momenta(3,3) = E*cos1

c --- virtual photon ---
      momenta(5,0) = (1.d0+q2)*dSqrt(Sp)/2.d0
      do i = 1,3
        momenta(5,i) = -momenta(3,i)
      enddo

      
           sin3 = dSqrt(1.d0-cos3*cos3)
           momenta(6,0)=(qq+Mpsc**2)/(2.d0*dsqrt(qq)) 
           momenta(7,0)=(qq-Mpsc**2)/(2.d0*dsqrt(qq))
           p=momenta(7,0) ! p=momenta(6,0)**2-Mpsc**2
           momenta(6,1) = p*sin3*dCos(phi3)
           momenta(6,2) = p*sin3*dSin(phi3)
           momenta(6,3) = p*cos3
         
         do i = 1,3
             momenta(7,i) = -momenta(6,i)
         enddo
         call hadronmomenta_1(2)

       qq2=(momenta(6,0)+momenta(3,0))**2-(momenta(6,1)+momenta(3,1))**2      
     1 -(momenta(6,2)+momenta(3,2))**2-(momenta(6,3)+momenta(3,3))**2

       cos3=momenta(7,3)/momenta(7,0)

       z2=1.d0/(1.d0-bb**2*cos3**2)


      if(pch.le.0.5d0)then       
      pompom = qq1
      qq1 = qq2
      qq2 = pompom
      pompom = z1
      z1=z2
      z2 = pompom
      do i = 0,3
      pompom = momenta(3,i) 
      momenta(3,i) = momenta(7,i)
      momenta(7,i) = pompom
      enddo
      qq = qq2
      endif
         

        jacobian=2.d0*ppp*jac2/(
     1   z1*(1.d0/(Sp*(Sp-qq1))+ 1.d0/Sp/qq1+fakeh
     1 + Momega_c**2*GammaTotalomega_c**2*fak4/((Momega_c**2-qq1)**2+
     2 Momega_c**2*GammaTotalomega_c**2)
     3 + Mphi_c**2*GammaTotalphi_c**2*fak6/((Mphi_c**2-qq1)**2+
     2 Mphi_c**2*GammaTotalphi_c**2))
     2 +z2*(1.d0/(Sp*(Sp-qq2))+ 1.d0/Sp/qq2+fakeh
     1 + Momega_c**2*GammaTotalomega_c**2*fak4/((Momega_c**2-qq2)**2+
     2 Momega_c**2*GammaTotalomega_c**2)
     3 + Mphi_c**2*GammaTotalphi_c**2*fak6/((Mphi_c**2-qq2)**2+
     2 Mphi_c**2*GammaTotalphi_c**2))
     3)

  



c----------------------------------------------------------------------
c
      jacobian = jacobian


      end
c---------------------------------------------------------------------
c---------------------------------------------------------------------
c ========================================================================
c --- Generates: photon virtuality Q2 using a two-component substitution -
c used in 2 photon part
c ------------------------------------------------------------------------
      subroutine qquadrat2(qqmin,qqmax,cos3min,cos3max,Ar,
     1                         qq,q0p,q2p,q0b,q2b,cos3,phi3,jacobian)
      include 'phokhara_10.0.inc'
      real*8 qqmin,qqmax,Ar(14),qq,jacobian,x,a,b,amin,amax
     1      ,bmin,bmax,fak1,fak2,p,y,ppp,c,cmin,cmax,d,dmin,dmax,fak3
     2      ,mm_1,gg_1,const_1,const_2,fak4,e_1,e_1min,e_1max,fak6,f,
     3       fmin,fmax,fak7
      real*8 cos3min,cos3max,q0p,q2p,q0b,q2b,cos3,phi3,jac4,
     &                 ymin,yy,ea,delcos,vv,delyy,vol1,vol2,vol3,vol4

      real*8  Qjp,QgK2,Qp2s,QgK2_p2s
      complex*16 g_eK_jp,g_eK_p2s
      common /jpparam/ Qjp,g_eK_jp
      common /p2sparam/ Qp2s,g_eK_p2s 
      common /genres/QgK2,QgK2_p2s

      x = Ar(1)      
c --- flat ---
c      a = qqmax-qqmin
c      qq = qqmin+a*x
c      jacobian = a

c --- muons -----------------------------------------------------
c --- the Q2 distribution is peaked at threshold and Q2->Sp (soft photon). 
      if (pion.eq.0) then
         if(fsrnlo.eq.0) then !one photon
            fak1 = -1.d0/Sp
            amin = fak1*dLog(Sp-qqmin)
            amax = fak1*dLog(Sp-qqmax)
            a = amax-amin
            bmin = dLog(qqmin/Sp)/Sp
            b    = dLog(qqmax/qqmin)/Sp
c --- which substitution? ---
c --- 4 channels
c 1 1/Sp/(Sp-qq)
c 2 Sp/qq
c 3 J/Psi BW
c 4 Psi(2S) BW
            p = Ar(7)      
!J/Psi
            if(narr_res.eq.1)then
               fak4 = 1.D0/3500.D0
            cmin=Qjp**2*mjp*gamjp*fak4*dAtan((qqmin-mjp**2)/(mjp*gamjp))
            cmax=Qjp**2*mjp*gamjp*fak4*dAtan((qqmax-mjp**2)/(mjp*gamjp))
               c = cmax-cmin      

               ppp  = a+b+c
               if(p.lt.a/ppp)then
                  y  = amin+a*x
                  qq = Sp-dExp(y/fak1)                                       
               elseif(p.lt.((a+b)/ppp))then
                  y  = bmin+b*x
                  qq = Sp*exp(Sp*y)
               else
                  y = cmin+c*x
                  qq = mjp*gamjp*dtan(y/fak4/mjp/gamjp/Qjp**2) + mjp**2
               endif

               jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq + Qjp**2
     1         * mjp**2*gamjp**2*fak4/((mjp**2-qq)**2+mjp**2*gamjp**2) )
!Psi(2S)
            elseif(narr_res.eq.2)then
               fak6 = 1.D0/700.D0
      dmin=Qp2s**2*mp2s*gamp2s*fak6*dAtan((qqmin-mp2s**2)/(mp2s*gamp2s))
      dmax=Qp2s**2*mp2s*gamp2s*fak6*dAtan((qqmax-mp2s**2)/(mp2s*gamp2s))
               d = dmax-dmin      

               ppp  = a+b+d
               if(p.lt.a/ppp)then
                  y  = amin+a*x
                  qq = Sp-dExp(y/fak1)                                       
               elseif(p.lt.((a+b)/ppp))then
                  y  = bmin+b*x
                  qq = Sp*exp(Sp*y)
               else
                  y = dmin+d*x
                 qq=mp2s*gamp2s*dtan(y/fak6/mp2s/gamp2s/Qp2s**2)+mp2s**2
               endif

               jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq
     2         + Qp2s**2*mp2s**2*gamp2s**2*fak6
     3                   /((mp2s**2-qq)**2+mp2s**2*gamp2s**2) )
!no narrow resonaces
            else

               ppp  = a+b

               if(p.lt.a/ppp)then
                  y  = amin+a*x
                  qq = Sp-dExp(y/fak1)                                       
               else
                  y  = bmin+b*x
                  qq = Sp*exp(Sp*y)
               endif

               jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq )

            endif

            call pionangles(cos3min,cos3max,Ar,cos3,phi3,jac4)

            jacobian = jacobian * jac4
 
         elseif(fsrnlo.eq.1)then 

            fak1 = -1.d0/Sp
            amin = fak1*dLog(Sp-qqmin)
            amax = fak1*dLog(Sp-qqmax)
            a = amax-amin
            bmin = dLog(qqmin/Sp)/Sp
            b    = dLog(qqmax/qqmin)/Sp

c --- which substitution? ---
c --- 4 channels
c 1 1/Sp/(Sp-qq)
c 2 Sp/qq
c 3 J/Psi BW
c 4 Psi(2S) BW
            p = Ar(7)   
!J/psi
            if(narr_res.eq.1)then

               fak4 = 1.D0/3500.D0
            cmin=Qjp**2*mjp*gamjp*fak4*dAtan((qqmin-mjp**2)/(mjp*gamjp))
            cmax=Qjp**2*mjp*gamjp*fak4*dAtan((qqmax-mjp**2)/(mjp*gamjp))
               c = cmax-cmin      
   
               ppp  = a+b+c
               if(p.lt.a/ppp)then
                  y  = amin+a*x
                  qq = Sp-dExp(y/fak1)                                       
               elseif(p.lt.((a+b)/ppp))then
                  y  = bmin+b*x
                  qq = Sp*exp(Sp*y)
               else
                  y = cmin+c*x
                  qq = mjp*gamjp*dtan(y/fak4/mjp/gamjp/Qjp**2) + mjp**2
              endif

               jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq
     1   +Qjp**2*mjp**2*gamjp**2*fak4/((mjp**2-qq)**2+mjp**2*gamjp**2) )
!Psi(2S)
            elseif(narr_res.eq.2)then

               fak6 = 1.D0/700.D0
      dmin=Qp2s**2*mp2s*gamp2s*fak6*dAtan((qqmin-mp2s**2)/(mp2s*gamp2s))
      dmax=Qp2s**2*mp2s*gamp2s*fak6*dAtan((qqmax-mp2s**2)/(mp2s*gamp2s))
               d = dmax-dmin      
   
               ppp  = a+b+d
               if(p.lt.a/ppp)then
                  y  = amin+a*x
                  qq = Sp-dExp(y/fak1)                                       
               elseif(p.lt.((a+b)/ppp))then
                  y  = bmin+b*x
                  qq = Sp*exp(Sp*y)
               else
                  y = dmin+d*x
                 qq=mp2s*gamp2s*dtan(y/fak6/mp2s/gamp2s/Qp2s**2)+mp2s**2
               endif

               jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq
     2         + Qp2s**2*mp2s**2*gamp2s**2*fak6
     3                   /((mp2s**2-qq)**2+mp2s**2*gamp2s**2) )
!no narrow resonaces
            else
               ppp  = a+b
               if(p.lt.a/ppp)then
                  y  = amin+a*x
                  qq = Sp-dExp(y/fak1)                                       
               else
                  y  = bmin+b*x
                  qq = Sp*exp(Sp*y)
               endif

               jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq )

            endif

             call pionangles(cos3min,cos3max,Ar,cos3,phi3,jac4)

             jacobian = jacobian * jac4

         endif
c --- 2 pions -----------------------------------------------------
c --- the Q2 distribution is peaked at Q2=rho and w mass^2       
c --- resonances (pion form factor) and at Q2->Sp (soft photon). 
      elseif(pion.eq.1)then

      fak1 = 1.D0/100000.d0  !/1200.d0      !310.D0
      fak2 = 1.d0/100000.d0
        call qquadrat_1(qqmin,qqmax,x,Ar(7),
     1                 m_rho0_pion,g_rho0_pion,fak1,fak2,qq,jacobian)
       call pionangles(cos3min,cos3max,Ar,cos3,phi3,jac4)
       jacobian = jacobian * jac4
c
c --- 4 pions -----------------------------------------------------
      elseif((pion.eq.2).or.(pion.eq.3))then
        call qquadrat_1(qqmin,qqmax,x,Ar(7),1.5d0,0.5d0,
     1                  0.d0,0.d0,qq,jacobian)
c
c-------- PPbar and NNbar ------------------------------------------
      elseif ((pion.eq.4).or.(pion.eq.5)) then
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      bmin = dLog(qqmin/Sp)/Sp
      b    = dLog(qqmax/qqmin)/Sp
c --- which substitution? ---
      p = Ar(7)      
      ppp  = a/(a+b)
      if(p.lt.ppp)then
         y  = amin+a*x
         qq = Sp-dExp(y/fak1)                                       
      else
         y  = bmin+b*x
         qq = Sp*exp(Sp*y)
      endif
      jacobian = (a+b)/(1.d0/(Sp*(Sp-qq)) + 1.d0/Sp/qq)  
       call pionangles(cos3min,cos3max,Ar,cos3,phi3,jac4)
       jacobian = jacobian * jac4
c 
c ---- K^+K^- and K0K0bar ---------------------------------------------
      elseif((pion.eq.6).or.(pion.eq.7))then

         fak1 = -1.d0/Sp
         amin = fak1*dLog(Sp-qqmin)
         amax = fak1*dLog(Sp-qqmax)
         a = amax-amin
c -----------
         const_2 = 2.d0
         fak2 = 1.d0/g_phi0_Kp/m_phi0_Kp
         bmin = const_2*fak2*dAtan((qqmin-m_phi0_Kp**2)*fak2)
         bmax = const_2*fak2*dAtan((qqmax-m_phi0_Kp**2)*fak2)
         b = bmax-bmin
c -----------
         cmin = dLog(qqmin/Sp)*Sp
         c = dLog(qqmax/qqmin)*Sp
c ------------
         const_1 = 10.d0
         mm_1 = 1.05d0
         gg_1 = 0.05d0
         fak3 = 1.d0/gg_1/mm_1
         dmin = const_1*fak3*dAtan((qqmin-mm_1**2)*fak3)
         dmax = const_1*fak3*dAtan((qqmax-mm_1**2)*fak3)
         d = dmax-dmin
c --- 6 channels ---
c  1.  1/Sp/(Sp-qq)
c  2.  BW m_phi0_Kp
c  3.  Sp/qq
c  4.  BW mm_1
c  5. J/Psi BW
c  6. Psi(2S) BW 
         p = Ar(7)      
!J/Psi
         if(narr_res.eq.1)then

            if(pion.eq.6) fak6 = 2.9D0  !K+K-
            if(pion.eq.7) fak6 = 2.4D0  !K0K0
        e_1min = QgK2*fak6*dAtan( (qqmin-mjp**2)/(mjp*gamjp) )/mjp/gamjp
        e_1max = QgK2*fak6*dAtan( (qqmax-mjp**2)/(mjp*gamjp) )/mjp/gamjp
            e_1 = e_1max-e_1min

            ppp  = a+b+c+d+e_1
c
            if(p.lt.a/ppp)then
               y  = amin+a*x
               qq = Sp-dExp(y/fak1)                                       
            elseif(p.lt.((a+b)/ppp))then
               y  = bmin+b*x
               qq = m_phi0_Kp*(m_phi0_Kp+g_phi0_Kp*dTan(y/fak2/const_2))
            elseif(p.lt.((a+b+c)/ppp))then
               y  = cmin+c*x
               qq = Sp*exp(y/Sp)
            elseif(p.lt.((a+b+c+d)/ppp))then
               y  = dmin+d*x
               qq = mm_1*(mm_1+gg_1*dTan(y/fak3/const_1))
            else
               y  = e_1min+e_1*x
               qq = mjp*gamjp*dtan(y*mjp*gamjp/QgK2/fak6) + mjp**2
            endif

            jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) +
     &      const_2/(((qq-m_phi0_Kp**2)**2+(g_phi0_Kp*m_phi0_Kp)**2)) +
     2      1.d0/(qq/Sp) +
     3      const_1/(((qq-mm_1**2)**2+(gg_1*mm_1)**2)) 
     4      + QgK2*fak6/((mjp**2-qq)**2+mjp**2*gamjp**2)  )
!Psi(2S)
          elseif(narr_res.eq.2)then

            fak7 = 75.D0
            fmin = QgK2_p2s*fak7*dAtan( (qqmin-mp2s**2)/(mp2s*gamp2s) )
     &       /mp2s/gamp2s
            fmax = QgK2_p2s*fak7*dAtan( (qqmax-mp2s**2)/(mp2s*gamp2s) )
     &       /mp2s/gamp2s
            f = fmax-fmin

            ppp  = a+b+c+d+f
c
            if(p.lt.a/ppp)then
               y  = amin+a*x
               qq = Sp-dExp(y/fak1)                                       
            elseif(p.lt.((a+b)/ppp))then
               y  = bmin+b*x
               qq = m_phi0_Kp*(m_phi0_Kp+g_phi0_Kp*dTan(y/fak2/const_2))
            elseif(p.lt.((a+b+c)/ppp))then
               y  = cmin+c*x
               qq = Sp*exp(y/Sp)
            elseif(p.lt.((a+b+c+d)/ppp))then
               y  = dmin+d*x
               qq = mm_1*(mm_1+gg_1*dTan(y/fak3/const_1))
            else
               y  = fmin+f*x
               qq =mp2s*gamp2s*dtan(y*mp2s*gamp2s/QgK2_p2s/fak7)+mp2s**2
            endif

            jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) +
     &      const_2/(((qq-m_phi0_Kp**2)**2+(g_phi0_Kp*m_phi0_Kp)**2)) +
     2      1.d0/(qq/Sp) +
     3      const_1/(((qq-mm_1**2)**2+(gg_1*mm_1)**2)) 
     5     + QgK2_p2s*fak7/((mp2s**2-qq)**2+mp2s**2*gamp2s**2) )
!no narrow resonaces
          else

            ppp  = a+b+c+d
c
            if(p.lt.a/ppp)then
               y  = amin+a*x
               qq = Sp-dExp(y/fak1)                                       
            elseif(p.lt.((a+b)/ppp))then
               y  = bmin+b*x
               qq = m_phi0_Kp*(m_phi0_Kp+g_phi0_Kp*dTan(y/fak2/const_2))
            elseif(p.lt.((a+b+c)/ppp))then
               y  = cmin+c*x
               qq = Sp*exp(y/Sp)
            else
               y  = dmin+d*x
               qq = mm_1*(mm_1+gg_1*dTan(y/fak3/const_1))
            endif

            jacobian = ppp/( 1.d0/(Sp*(Sp-qq)) +
     &     const_2/(((qq-m_phi0_Kp**2)**2+(g_phi0_Kp*m_phi0_Kp)**2)) +
     2     1.d0/(qq/Sp) +
     3     const_1/(((qq-mm_1**2)**2+(gg_1*mm_1)**2)) )

          endif    
       call pionangles(cos3min,cos3max,Ar,cos3,phi3,jac4)
       jacobian = jacobian * jac4
c
c --- 3 pions -----------------------------------------------------
c
      elseif(pion.eq.8)then
       call qquadrat_2a(qq,qqmax,qqmin,x,Ar(7),jacobian)  
c
c --- eta-pi-pi -----------------------------------------------------
c
      elseif(pion.eq.10)then
       call qquadrat_eta(qq,qqmax,qqmin,x,Ar(7),jacobian)  
c
      else
       continue
      endif
c
      return
      end
c *******************************************************************
c
      subroutine qquadrat_1(qqmin,qqmax,x,p,mmm,ggg,fak5,fak6,
     1                                                   qq,jacobian)
      include 'phokhara_10.0.inc'
      real*8 qqmin,qqmax,Ar(14),qq,jacobian,mmm,ggg,
     &  x,a,b,amin,amax,bmin,bmax,fak1,fak2,p,y,ppp,fak5,c,cmin,cmax,
     2  fak6,d,dmin,dmax
c
      real*8 Qjp,Qp2s
      complex*16 g_eK_jp,g_eK_p2s
      common/p2sparam/Qp2s,g_eK_p2s 
      common/jpparam/Qjp,g_eK_jp
c
      fak1 = -1.d0/Sp
      amin = fak1*dLog(Sp-qqmin)
      amax = fak1*dLog(Sp-qqmax)
      a = amax-amin
      fak2 = 1.d0/ggg/mmm
      bmin = fak2*dAtan((qqmin-mmm**2)*fak2)
      bmax = fak2*dAtan((qqmax-mmm**2)*fak2)
      b = bmax-bmin

      if(pion.eq.1)then    ! two pions

c --- 3 channels ---
c 1 1/Sp/(Sp-qq)
c 2 BW m_rho0_pion
c 3 J/Psi BW
c 4 Psi(2S) BW

!J/Psi
         if(narr_res.eq.1)then

            cmin=Qjp**2*mjp*gamjp*fak5*dAtan((qqmin-mjp**2)/(mjp*gamjp))
            cmax=Qjp**2*mjp*gamjp*fak5*dAtan((qqmax-mjp**2)/(mjp*gamjp))
            c = cmax-cmin 

            ppp  = a+b+c
c
            if(p.lt.a/ppp)then
               y  = amin+a*x
               qq = Sp-dExp(y/fak1)                                       
            elseif(p.lt.((a+b)/ppp))then
               y  = bmin+b*x
               qq = mmm*(mmm+ggg*dTan(y/fak2)) 
            else
               y = cmin+c*x
               qq = mjp*gamjp*dtan(y/fak5/mjp/gamjp/Qjp**2) + mjp**2        
            endif
            jacobian = ppp/( (1.d0/(Sp*(Sp-qq)) 
     1      + 1.d0/(((qq-mmm**2)**2+(ggg*mmm)**2))) + Qjp**2*mjp**2
     2        *gamjp**2*fak5/((mjp**2-qq)**2+mjp**2*gamjp**2) )

         else
            ppp  = a+b
c
            if(p.lt.a/ppp)then
               y  = amin+a*x
               qq = Sp-dExp(y/fak1)                                       
            else
               y  = bmin+b*x
               qq = mmm*(mmm+ggg*dTan(y/fak2)) 
            endif
            jacobian = ppp/( (1.d0/(Sp*(Sp-qq)) 
     1               + 1.d0/(((qq-mmm**2)**2+(ggg*mmm)**2))) )

         endif

      else         ! four pions

c --- two channels ---
         ppp  = a/(a+b)
c
         if(p.lt.ppp)then
            y  = amin+a*x
            qq = Sp-dExp(y/fak1)                                       
         else
            y  = bmin+b*x
            qq = mmm*(mmm+ggg*dTan(y/fak2))                    
         endif
         jacobian = (a+b)/( (1.d0/(Sp*(Sp-qq)) +
     &        1.d0/(((qq-mmm**2)**2+(ggg*mmm)**2))) )
      endif
c
      return
      end
c ---------------------------------------------------------------------
c   3 pi 1ph
c ---------------------------------------------------------------------

      subroutine qquadrat_2(qq,qqmax,qqmin,x,p,jac)
      include 'phokhara_10.0.inc'

      real*8 jac,xx_om0_max,xx_om0_min,xx_om1_max,xx_om1_min,
     1   xx_phi0_max,xx_phi0_min,xx_ss_max,xx_ss_min,d_om0,d_om1,d_phi0,
     2   d_ss,vv_3pi,yy_3pi,const_1,const_2,const_3,const_ss
      real*8 qq,qqmax,qqmin,x,p,aa,mm_1_p,mg_1_p,const_1_p,xx_1_p_min,
     1   xx_1_p_max,d_1_p,mm_2_p,mg_2_p,const_2_p,xx_2_p_min,xx_2_p_max,
     2   d_2_p,const_om2,xx_om2_max,xx_om2_min,d_om2

c \om_0 -----------------------------
      if(Sp.lt.25.d0)then
       const_1 = 1000.d0*(omg0_3pi/omm0_3pi)**2
      else
       const_1 = 410000.d0*(omg0_3pi/omm0_3pi)**2
      endif

      xx_om0_max = const_1*omm0_3pi**3/omg0_3pi * 
     1             dAtan((qqmax/omm0_3pi**2-1.d0)*omm0_3pi/omg0_3pi)
      xx_om0_min = const_1*omm0_3pi**3/omg0_3pi * 
     1             dAtan((qqmin/omm0_3pi**2-1.d0)*omm0_3pi/omg0_3pi)
      d_om0 = xx_om0_max - xx_om0_min 

c \phi_0 -----------------------------
      if(Sp.lt.25.d0)then
       const_2 =  20000.d0*(phig0_3pi/phim0_3pi)**2
      else
       const_2 =  240000.d0*(phig0_3pi/phim0_3pi)**2 
      endif

      xx_phi0_max = const_2*phim0_3pi**3/phig0_3pi * 
     1            dAtan((qqmax/phim0_3pi**2-1.d0)*phim0_3pi/phig0_3pi)
      xx_phi0_min = const_2*phim0_3pi**3/phig0_3pi * 
     1            dAtan((qqmin/phim0_3pi**2-1.d0)*phim0_3pi/phig0_3pi)
      d_phi0 = xx_phi0_max - xx_phi0_min

c \om_1 ----- only for sqrt(s) = 10.52  \Gamma_om_1 = omg1 * 2
      const_3 = 6500.d0*(2.d0*omg1_3pi/omm1_3pi)**2

      xx_om1_max = const_3*omm1_3pi**3/(2.d0*omg1_3pi) * 
     1   dAtan((qqmax/omm1_3pi**2-1.d0)*omm1_3pi/(2.d0*omg1_3pi))
      xx_om1_min = (const_3*omm1_3pi**3)/2.d0/omg1_3pi * 
     1   dAtan((qqmin/omm1_3pi**2-1.d0)*omm1_3pi/2.d0/omg1_3pi)
      d_om1 = xx_om1_max - xx_om1_min

c Sp/(Sp-qq) ----------------------------
       xx_ss_max = -Sp * dLog((Sp-qqmax)/Sp) 
       xx_ss_min = -Sp * dLog((Sp-qqmin)/Sp)
       d_ss = xx_ss_max - xx_ss_min

c--------------------------------------
      if(Sp.lt.25.d0)then

       const_ss = 1.d0
      else      
       const_ss = 22.5d0
      endif

      aa = const_ss*(qqmax - qqmin)

c      qq = qqmin + x * aa
c      jac = aa
c --------------------------------------

      if(Sp.lt.25.d0)then

       vv_3pi = aa + d_ss + d_om0 + d_phi0

      if(p.lt.(d_ss/vv_3pi))then
          yy_3pi = xx_ss_min + x * d_ss
          qq = Sp * ( 1.d0 - dExp(-yy_3pi/Sp))
      elseif(p.lt.((d_om0+d_ss)/vv_3pi))then
         yy_3pi = xx_om0_min + x * d_om0
         qq = omg0_3pi*omm0_3pi 
     2      * dTan(yy_3pi*omg0_3pi/omm0_3pi**3/const_1)
     1      + omm0_3pi**2
      elseif(p.lt.((d_om0+d_phi0+d_ss)/vv_3pi))then
         yy_3pi = xx_phi0_min + x * d_phi0
         qq = phig0_3pi*phim0_3pi 
     2      * dTan(yy_3pi*phig0_3pi/phim0_3pi**3/const_2)
     1      + phim0_3pi**2
      else
          qq = qqmin + x * aa/const_ss
      endif

      jac = vv_3pi/(const_ss + Sp/(Sp-qq) + 
     3  const_1/((qq/omm0_3pi**2-1.d0)**2+(omg0_3pi/omm0_3pi)**2) +
     2  const_2/((qq/phim0_3pi**2-1.d0)**2+(phig0_3pi/phim0_3pi)**2))

      else
 
       vv_3pi = d_ss + d_om0 + d_phi0 + aa + d_om1  

      if(p.lt.(d_ss/vv_3pi))then
          yy_3pi = xx_ss_min + x * d_ss
          qq = Sp * ( 1.d0 - dExp(-yy_3pi/Sp))
      elseif(p.lt.((d_om0+d_ss)/vv_3pi))then
         yy_3pi = xx_om0_min + x * d_om0
         qq = omg0_3pi*omm0_3pi 
     2      * dTan(yy_3pi*omg0_3pi/omm0_3pi**3/const_1)
     1      + omm0_3pi**2
      elseif(p.lt.((d_om0+d_phi0+d_ss)/vv_3pi))then
         yy_3pi = xx_phi0_min + x * d_phi0
         qq = phig0_3pi*phim0_3pi 
     2      * dTan(yy_3pi*phig0_3pi/phim0_3pi**3/const_2)
     1      + phim0_3pi**2
      elseif(p.lt.((d_om0+d_phi0+d_ss+d_om1)/vv_3pi))then
         yy_3pi = xx_om1_min + x * d_om1  
         qq = 2.d0*omg1_3pi*omm1_3pi*dTan(yy_3pi*2.d0*omg1_3pi/
     1            omm1_3pi**3/const_3) + omm1_3pi**2
      else
          qq = qqmin + x * aa/const_ss
      endif

      jac = vv_3pi/( const_ss + Sp/(Sp-qq) + 
     3  const_1/((qq/omm0_3pi**2-1.d0)**2+(omg0_3pi/omm0_3pi)**2) +
     2  const_2/((qq/phim0_3pi**2-1.d0)**2+(phig0_3pi/phim0_3pi)**2)+ 
     1  const_3/((qq/omm1_3pi**2-1.d0)**2
     4                            +(2.d0*omg1_3pi/omm1_3pi)**2) )

      endif

      return
      end
c ---------------------------------------------------------------------
c   eta pi pi  1ph and 2 ph
c ---------------------------------------------------------------------

      subroutine qquadrat_eta(qq,qqmax,qqmin,x,p,jac)
      include 'phokhara_10.0.inc'

      real*8 jac,xx_om0_max,xx_om0_min,xx_om1_max,xx_om1_min,
     1   xx_phi0_max,xx_phi0_min,xx_ss_max,xx_ss_min,d_om0,d_om1,d_phi0,
     2   d_ss,vv_3pi,yy_3pi,const_1,const_ss
      real*8 qq,qqmax,qqmin,x,p,aa,rho1m_eta,rho1g_eta
c  -----------------------------
      rho1m_eta = 1.4888d0
      rho1g_eta = 0.1891d0
       const_1 = 1.d0

      xx_om0_max = const_1*rho1m_eta**3/rho1g_eta * 
     1             dAtan((qqmax/rho1m_eta**2-1.d0)*rho1m_eta/rho1g_eta)
      xx_om0_min = const_1*rho1m_eta**3/rho1g_eta * 
     1             dAtan((qqmin/rho1m_eta**2-1.d0)*rho1m_eta/rho1g_eta)
      d_om0 = xx_om0_max - xx_om0_min 

c Sp/(Sp-qq) ----------------------------
       xx_ss_max = -Sp * dLog((Sp-qqmax)/Sp) 
       xx_ss_min = -Sp * dLog((Sp-qqmin)/Sp)
       d_ss = xx_ss_max - xx_ss_min

c--------------------------------------
       const_ss = 1.d0

      aa = const_ss*(qqmax - qqmin)

c --------------------------------------


       vv_3pi = aa + d_ss + d_om0 

      if(p.lt.(d_ss/vv_3pi))then
          yy_3pi = xx_ss_min + x * d_ss
          qq = Sp * ( 1.d0 - dExp(-yy_3pi/Sp))
      elseif(p.lt.((d_om0+d_ss)/vv_3pi))then
         yy_3pi = xx_om0_min + x * d_om0
         qq = rho1g_eta*rho1m_eta 
     2      * dTan(yy_3pi*rho1g_eta/rho1m_eta**3/const_1)
     1      + rho1m_eta**2
      else
          qq = qqmin + x * aa/const_ss
      endif

      jac = vv_3pi/(const_ss + Sp/(Sp-qq) + 
     3  const_1/((qq/rho1m_eta**2-1.d0)**2+(rho1g_eta/rho1m_eta)**2))

      return
      end
c ---------------------------------------------------------------------
c   3 pi 2ph
c ---------------------------------------------------------------------

      subroutine qquadrat_2a(qq,qqmax,qqmin,x,p,jac)
      include 'phokhara_10.0.inc'

      real*8 jac,xx_om0_max,xx_om0_min,xx_om1_max,xx_om1_min,
     1   xx_phi0_max,xx_phi0_min,xx_ss_max,xx_ss_min,d_om0,d_om1,d_phi0,
     2   d_ss,vv_3pi,yy_3pi,const_1,const_2,const_3,const_ss
      real*8 qq,qqmax,qqmin,x,p,aa,mm_1_p,mg_1_p,const_1_p,xx_1_p_min,
     1   xx_1_p_max,d_1_p,mm_2_p,mg_2_p,const_2_p,xx_2_p_min,xx_2_p_max,
     2   d_2_p,const_om2,xx_om2_max,xx_om2_min,d_om2

c \om_0 -----------------------------
      if(Sp.lt.25.d0)then
       const_1 = 1100.d0*(omg0_3pi/omm0_3pi)**2
      else  
       const_1 = 1100000.d0*(omg0_3pi/omm0_3pi)**2
      endif

      xx_om0_max = const_1*omm0_3pi**3/omg0_3pi * 
     1             dAtan((qqmax/omm0_3pi**2-1.d0)*omm0_3pi/omg0_3pi)
      xx_om0_min = const_1*omm0_3pi**3/omg0_3pi * 
     1             dAtan((qqmin/omm0_3pi**2-1.d0)*omm0_3pi/omg0_3pi)
      d_om0 = xx_om0_max - xx_om0_min 

c \phi_0 -----------------------------
      if(Sp.lt.25.d0)then
       const_2 =  10000.d0*(phig0_3pi/phim0_3pi)**2
      else
       const_2 =  300000.d0*(phig0_3pi/phim0_3pi)**2 
      endif

      xx_phi0_max = const_2*phim0_3pi**3/phig0_3pi * 
     1            dAtan((qqmax/phim0_3pi**2-1.d0)*phim0_3pi/phig0_3pi)
      xx_phi0_min = const_2*phim0_3pi**3/phig0_3pi * 
     1            dAtan((qqmin/phim0_3pi**2-1.d0)*phim0_3pi/phig0_3pi)
      d_phi0 = xx_phi0_max - xx_phi0_min

c \om_1 ----- only for sqrt(s)= 10.52  \Gamma_om_1 = omg1 * 2
      const_3 = 6000.d0*(2.d0*omg1_3pi/omm1_3pi)**2

      xx_om1_max = const_3*omm1_3pi**3/(2.d0*omg1_3pi) * 
     1   dAtan((qqmax/omm1_3pi**2-1.d0)*omm1_3pi/(2.d0*omg1_3pi))
      xx_om1_min = (const_3*omm1_3pi**3)/2.d0/omg1_3pi * 
     1   dAtan((qqmin/omm1_3pi**2-1.d0)*omm1_3pi/2.d0/omg1_3pi)
      d_om1 = xx_om1_max - xx_om1_min

c Sp/(Sp-qq) ----------------------------
       xx_ss_max = -Sp * dLog((Sp-qqmax)/Sp) 
       xx_ss_min = -Sp * dLog((Sp-qqmin)/Sp)
       d_ss = xx_ss_max - xx_ss_min

c--------------------------------------
      if(Sp.lt.25.d0)then

       const_ss = 1.d0
      else      
       const_ss = 44.d0
      endif

      aa = const_ss*(qqmax - qqmin)

c      qq = qqmin + x * aa
c      jac = aa
c --------------------------------------

      if(Sp.lt.25.d0)then

       vv_3pi = aa + d_ss + d_om0 + d_phi0

      if(p.lt.(d_ss/vv_3pi))then
          yy_3pi = xx_ss_min + x * d_ss
          qq = Sp * ( 1.d0 - dExp(-yy_3pi/Sp))
      elseif(p.lt.((d_om0+d_ss)/vv_3pi))then
         yy_3pi = xx_om0_min + x * d_om0
         qq = omg0_3pi*omm0_3pi 
     2      * dTan(yy_3pi*omg0_3pi/omm0_3pi**3/const_1)
     1      + omm0_3pi**2
      elseif(p.lt.((d_om0+d_phi0+d_ss)/vv_3pi))then
         yy_3pi = xx_phi0_min + x * d_phi0
         qq = phig0_3pi*phim0_3pi 
     2      * dTan(yy_3pi*phig0_3pi/phim0_3pi**3/const_2)
     1      + phim0_3pi**2
      else
          qq = qqmin + x * aa/const_ss
      endif

      jac = vv_3pi/(const_ss + Sp/(Sp-qq) + 
     3  const_1/((qq/omm0_3pi**2-1.d0)**2+(omg0_3pi/omm0_3pi)**2) +
     2  const_2/((qq/phim0_3pi**2-1.d0)**2+(phig0_3pi/phim0_3pi)**2))

      else
 
       vv_3pi = d_ss + d_om0 + d_phi0 + aa + d_om1  

      if(p.lt.(d_ss/vv_3pi))then
          yy_3pi = xx_ss_min + x * d_ss
          qq = Sp * ( 1.d0 - dExp(-yy_3pi/Sp))
      elseif(p.lt.((d_om0+d_ss)/vv_3pi))then
         yy_3pi = xx_om0_min + x * d_om0
         qq = omg0_3pi*omm0_3pi 
     2      * dTan(yy_3pi*omg0_3pi/omm0_3pi**3/const_1)
     1      + omm0_3pi**2
      elseif(p.lt.((d_om0+d_phi0+d_ss)/vv_3pi))then
         yy_3pi = xx_phi0_min + x * d_phi0
         qq = phig0_3pi*phim0_3pi 
     2      * dTan(yy_3pi*phig0_3pi/phim0_3pi**3/const_2)
     1      + phim0_3pi**2
      elseif(p.lt.((d_om0+d_phi0+d_ss+d_om1)/vv_3pi))then
         yy_3pi = xx_om1_min + x * d_om1  
         qq = 2.d0*omg1_3pi*omm1_3pi*dTan(yy_3pi*2.d0*omg1_3pi/
     1            omm1_3pi**3/const_3) + omm1_3pi**2
      else
          qq = qqmin + x * aa/const_ss
      endif

      jac = vv_3pi/( const_ss + Sp/(Sp-qq) + 
     3  const_1/((qq/omm0_3pi**2-1.d0)**2+(omg0_3pi/omm0_3pi)**2)+
     2  const_2/((qq/phim0_3pi**2-1.d0)**2+(phig0_3pi/phim0_3pi)**2)+ 
     1  const_3/((qq/omm1_3pi**2-1.d0)**2
     4                            +(2.d0*omg1_3pi/omm1_3pi)**2) )

      endif
 
      return
      end
c ========================================================================
c --- Generates: real photon costheta and phi angles in the e^+e^- CMS ---
c ------------------------------------------------------------------------
      subroutine photonangles1(cosmin,cosmax,Ar,costheta,phi,jacobian)
      include 'phokhara_10.0.inc'
      real*8 cosmin,cosmax,Ar(14),costheta,phi,jacobian,x,b,cmin,cmax,y,
     1       cosmin2,cosmax2,cmaxmcmin,rat

      x   = Ar(2)
      phi = 2.d0*pi*Ar(3)

c      fak3 = 2.d0
      rat = 4.d0*me*me/Sp
      cosmin2 = cosmin**2
      cosmax2 = cosmax**2
c --- flat ---
c      costheta = cosmin+(cosmax-cosmin)*x
c      jacobian = 2.d0*pi*(cosmax-cosmin)
c --- peaked at costheta = +-1 ---

      b = dSqrt(1.d0-rat)
      cmin = dLog((1.d0-cosmin2+rat*cosmin2)/(1.d0-b*cosmin)**2)
     1      /(2.d0*b)
      cmaxmcmin = dLog((1.d0+b*cosmax)**2*(1.d0-b*cosmin)**2
     1          /(1.d0-cosmax2+rat*cosmax2)/(1.d0-cosmin2+rat*cosmin2))
     2          / (2.d0*b)

         y = cmin+x*cmaxmcmin
         costheta = dTanh(b*y)/b

c onemb2c2 is used in Bornvirtualsoft
      onemb2c2 = 1.d0/(dcosh(b*y))**2
c
      jacobian = 2.d0*pi*onemb2c2*cmaxmcmin
c
      return
      end
c ========================================================================
c --- Generates: real photon costheta and phi angles in the e^+e^- CMS ---
c ------------------------------------------------------------------------
      subroutine photonangles2(cosmin,cosmax,Ar,costheta,phi,jacobian)
      include 'phokhara_10.0.inc'
      real*8 cosmin,cosmax,Ar(14),costheta,phi,jacobian,x,b,cmin,cmax,y
     1      ,cmaxmcmin,rat,cosmin2,cosmax2

      x   = Ar(10)
      phi = 2.d0*pi*Ar(11)
      rat = 4.d0*me*me/Sp
      cosmin2 = cosmin**2
      cosmax2 = cosmax**2

c --- flat ---
c      costheta = cosmin+(cosmax-cosmin)*x
c      jacobian = 2.d0*pi*(cosmax-cosmin)
c --- peaked at costheta = +-1 ---
      b = dSqrt(1.d0-rat)
      cmin = dLog(((1.d0-cosmin2)+rat*cosmin2)/(1.d0-b*cosmin)**2)
     1      /(2.d0*b)
      cmaxmcmin = dLog((1.d0+b*cosmax)**2/((1.d0-cosmax2)+rat*cosmax2)
     1        /((1.d0-cosmin2)+rat*cosmin2)*(1.d0-b*cosmin)**2)             
     2      /(2.d0*b)
      y = cmin+x*cmaxmcmin
      costheta = dTanh(b*y)/b
      onemb2c2 = 1.d0/(dcosh(b*y))**2
c
      jacobian = 2.d0*pi/(dcosh(b*y))**2*cmaxmcmin
      return
      end      
c ========================================================================
c --- Generates: real photon energy in the e^+e^- CMS normalized to the --
c --- CMS energy ---------------------------------------------------------
c ------------------------------------------------------------------------
      subroutine photonenergy2(qq,c1min,c1max,cos1,cos2,cos12,Ar
     &                        ,w1,w2,jacobian)
      include 'phokhara_10.0.inc'
      real*8 qq,c1min,c1max,cos1,cos2,cos12,Ar(14),
     &  w1,w2,jacobian,x,wmin,q2,qqb,w1max,w2max,u,umin,umax

      x = Ar(9)
c --- flat ---
c      w2 = w+(w2max-w)*x
c      jacobian = (w2max-w)
c --- peaked at w1, w2 = w ---
      wmin = gmin/dSqrt(Sp)
      q2 = qq/Sp
      qqb = Sp*(1.d0-4.d0*wmin+2.d0*wmin*wmin*(1.d0-cos12))
c --- photon 2 inside the angular cuts, photon 1 outside ---
c --- then w2 > wmin and w1 > w ----------------------------
      if(((cos1.gt.c1max).or.(cos1.lt.c1min)).and.
     &   ((cos2.le.c1max).and.(cos2.ge.c1min))) then

         w2max = (1.d0-q2-2.d0*w)/(2.d0*(1.d0-w*(1.d0-cos12)))         
         umin = dLog(wmin/(1.d0-q2-2.d0*wmin))
         umax = dLog(w2max/(1.d0-q2-2.d0*w2max))
         u  = umin+x*(umax-umin)
         w2 = (1.d0-q2)/(2.d0+dExp(-u))
         w1 = (1.d0-q2-2.d0*w2)/(2.d0*(1.d0-w2*(1.d0-cos12)))
	 jacobian = 1.d0/2.d0 * w2*w2*w1*w1 * (umax-umin)/(1.d0-q2)
c         write(6,*)'photonenergy2; 1'
c --- photon 1 inside the angular cuts, photon 2 outside ---
c --- then w1 > wmin and w2 > w ----------------------------
      elseif(((cos2.gt.c1max).or.(cos2.lt.c1min)).and.
     &   ((cos1.le.c1max).and.(cos1.ge.c1min))) then

         w1max = (1.d0-q2-2.d0*w)/(2.d0*(1.d0-w*(1.d0-cos12)))         
         umin = dLog(wmin/(1.d0-q2-2.d0*wmin))
         umax = dLog(w1max/(1.d0-q2-2.d0*w1max))
         u  = umin+x*(umax-umin)
         w1 = (1.d0-q2)/(2.d0+dExp(-u))
         w2 = (1.d0-q2-2.d0*w1)/(2.d0*(1.d0-w1*(1.d0-cos12)))            
         jacobian = 1.d0/2.d0 * w2*w2*w1*w1 * (umax-umin)/(1.d0-q2)
c         write(6,*)'photonenergy2; 2'

c --- both photons pass the angular cuts ---
      elseif(((cos2.le.c1max).and.(cos2.ge.c1min)).and.
     &   ((cos1.le.c1max).and.(cos1.ge.c1min))) then

         if(qq.le.qqb)then
            w1max = (1.d0-q2-2.d0*w)/(2.d0*(1.d0-w*(1.d0-cos12)))         
            umin = dLog(w/(1.d0-q2-2.d0*w))
            umax = dLog(w1max/(1.d0-q2-2.d0*w1max))
            u  = umin+x*(umax-umin)
            w1 = (1.d0-q2)/(2.d0+dExp(-u))
            w2 = (1.d0-q2-2.d0*w1)/(2.d0*(1.d0-w1*(1.d0-cos12)))            
            jacobian = 1.d0/2.d0 * w2*w2*w1*w1 * (umax-umin)/(1.d0-q2)
c        write(6,*)'photonenergy2; 3'

         else
c          write(6,*)'photonenergy2; 4'
           if(Ar(8).lt.0.5d0)then

            w2max = (1.d0-q2-2.d0*w)/(2.d0*(1.d0-w*(1.d0-cos12)))         
            umin = dLog(wmin/(1.d0-q2-2.d0*wmin))
            umax = dLog(w2max/(1.d0-q2-2.d0*w2max))
            u  = umin+x*(umax-umin)
            w2 = (1.d0-q2)/(2.d0+dExp(-u))
            w1 = (1.d0-q2-2.d0*w2)/(2.d0*(1.d0-w2*(1.d0-cos12)))            
            jacobian = w2*w2*w1*w1* (umax-umin)/(1.d0-q2)
	      
            else

            w1max = (1.d0-q2-2.d0*w)/(2.d0*(1.d0-w*(1.d0-cos12)))         
            umin = dLog(wmin/(1.d0-q2-2.d0*wmin))
            umax = dLog(w1max/(1.d0-q2-2.d0*w1max))
            u  = umin+x*(umax-umin)
            w1 = (1.d0-q2)/(2.d0+dExp(-u))
            w2 = (1.d0-q2-2.d0*w1)/(2.d0*(1.d0-w1*(1.d0-cos12)))            
            jacobian = w2*w2*w1*w1* (umax-umin)/(1.d0-q2)

            endif
         endif
      endif

      return
      end
c ------------------------------------------------------------------
c    lepton four-momenta: one real photon                           
c ------------------------------------------------------------------
      subroutine leptonmomenta1(qq,costheta,phi)
      include 'phokhara_10.0.inc'
      real*8 qq,q2,E,costheta,sintheta,phi,ransym
      integer i
      common/sym/ransym
      q2 = qq/Sp
      sintheta = dSqrt(1.d0-costheta*costheta)
c --- real photon1 ---
      E = (1.d0-q2)*dSqrt(Sp)/2.d0
      momenta(3,0) = E
      momenta(3,1) = E*sintheta*dCos(phi)
      momenta(3,2) = E*sintheta*dSin(phi)
      momenta(3,3) = E*costheta
c --- real photon2 ---
      momenta(4,0) = 0.d0
      momenta(4,1) = 0.d0
      momenta(4,2) = 0.d0
      momenta(4,3) = 0.d0
c --- virtual photon ---
      momenta(5,0) = (1.d0+q2)*dSqrt(Sp)/2.d0
      do i = 1,3
        momenta(5,i) = -momenta(3,i)
      enddo
      return
      end
c ------------------------------------------------------------------
c    lepton four-momenta: two real photons                          
c ------------------------------------------------------------------
      subroutine leptonmomenta2(qq,w1,w2,cos1,phi1,cos2,phi2)
      include 'phokhara_10.0.inc'
      real*8 qq,w1,w2,cos1,sin1,phi1,cos2,sin2,phi2,E1,E2
      integer i

      sin1 = dSqrt(1.d0-cos1*cos1)
      sin2 = dSqrt(1.d0-cos2*cos2)
c --- real photon1 ---
      E1 = w1*dSqrt(Sp)
      momenta(3,0) = E1
      momenta(3,1) = E1*sin1*dCos(phi1)
      momenta(3,2) = E1*sin1*dSin(phi1)
      momenta(3,3) = E1*cos1
c --- real photon2 ---
      E2 = w2*dSqrt(Sp)      
      momenta(4,0) = E2
      momenta(4,1) = E2*sin2*dCos(phi2)
      momenta(4,2) = E2*sin2*dSin(phi2)
      momenta(4,3) = E2*cos2
c --- virtual photon ---
      momenta(5,0) = dSqrt(Sp)-E1-E2
      do i = 1,3
        momenta(5,i) = -momenta(3,i)-momenta(4,i)
      enddo
      return
      end
c ========================================================================
c --- Generates: pion costheta and phi angles in the Q2 CMS system, ------
c afterwards boosted into the e^+ e^- CMS --------------------------------
c used in 2 photon emission
c ------------------------------------------------------------------------
      subroutine pionangles(cosmin,cosmax,Ar,costheta,phi,jacobian)
      include 'phokhara_10.0.inc'
      real*8 cosmin,cosmax,Ar(14),costheta,phi,jacobian,x 
      real*8 rat,cosmin2,cosmax2,b,cmin,cmaxmcmin,y
      

      x   = Ar(4)
      phi = 2.d0*pi*Ar(5)

c --- flat ---
      costheta = cosmin+(cosmax-cosmin)*x
      jacobian = 2.d0*pi*(cosmax-cosmin)
     
      
      return
      end
c ========================================================================
c --- Generates: pion costheta and phi angles in Lambda rest-frame
c ------------------------------------------------------------------------
      subroutine pionanglesLamb(cosmin,cosmax,costhm,phim,
     1                          costhp,phip)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'
      real*8 jacobian,x1,x2,jac1,jac2,cosmin,cosmax,x
      real*8 costhm,phim,costhp,phip,rr1_ll(0:3)

      call ranlxdf(rr1_ll,4)
      x1   = rr1_ll(0)
      x2   = rr1_ll(1)
      phim = 2.d0*pi*rr1_ll(2)
      phip = 2.d0*pi*rr1_ll(3)
      
c --- flat ---
      costhm = cosmin+(cosmax-cosmin)*x1
      jac1 = 2.d0*pi*(cosmax-cosmin)
c --- flat ---
      costhp = cosmin+(cosmax-cosmin)*x2
      jac2 = 2.d0*pi*(cosmax-cosmin)
c 
      jacobian = jac1 * jac2

      return
      end
c *******************************************************************
c this subroutine generates  4 massive particle phase space 
c in q=q1+q2+q3+q4 rest frame (bosted afterwards into e^+e^- CMS)
c
c Mode    2pi^0 pi^+ pi^- (pion=2) |   Mode    2pi^+ 2pi^-     (pion=3)
c q1,q2 - four momenta of pi^0     |   q1,q4 - four momenta of pi^+ 
c q3    - four momentum of pi^-    |   q2,q3 - four momenta of pi^-
c q4    - four momentum of pi^+    |
c ------------------------------------------------------------------
      subroutine pionangles_1(qq2,q0p,q2p,q0b,q2b,fac)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'
      integer ip0,i,ru
      real*8 Arp(8),Arp_r(0:7)
      real*8 q1(4),q2(4),q3(4),q4(4),pom_v(4) 
      real*8 q0,q2p,q2p_min,q2p_max,dx1,q0p,x2min,x2max,dx2,
     1       q2b,q0b,qq2,phi1,costh1,sinth1,cosphi1,sinphi1,phi2,costh2,
     2       sinth2,cosphi2,sinphi2,phi3,costh3,sinth3,cosphi3,sinphi3,
     3       rm12,e1,vq1,rm22,e2,vq2,e3,vq3,q2pp,appr1,fac
c
      call ranlxdf(Arp_r,8)
      do ru=0,7
         Arp(ru+1) = Arp_r(ru)
      enddo
c
      q0    = sqrt(qq2)
      q2p_min = (rmass(2)+rmass(3)+rmass(4))**2
      q2p_max = (q0-rmass(1))**2
      if(pion.eq.2)then
       call chann1(Arp(1),q2p_min,q2p_max,q2p,dx1,ip0)
      else
       dx1    = q2p_max - q2p_min
       q2p    = q2p_min + dx1*Arp(1)
      endif
c
      q0p   = sqrt(q2p)
c
      x2min = (rmass(3)+rmass(4))**2
      x2max = (q0p-rmass(2))**2
      dx2   = x2max - x2min
c
      q2b   = x2min + dx2*Arp(2)
      q0b   = sqrt(q2b)
c
      phi1    = 2.d0*pi*Arp(3)
      costh1  = -1.d0 + 2.d0*Arp(4)
      sinth1  = 2.d0*sqrt(Arp(4)*(1.d0-Arp(4)))
      cosphi1 = cos(phi1)
      sinphi1 = sin(phi1)
c
      phi2    = 2.d0*pi*Arp(5)
      costh2  = -1.d0 + 2.d0*Arp(6)
      sinth2  = 2.d0*sqrt(Arp(6)*(1.d0-Arp(6)))
      cosphi2 = cos(phi2)
      sinphi2 = sin(phi2)
c
      phi3    = 2.d0*pi*Arp(7)
      costh3  = -1.d0 + 2.d0*Arp(8)
      sinth3  = 2.d0*sqrt(Arp(8)*(1.d0-Arp(8)))
      cosphi3 = cos(phi3)
      sinphi3 = sin(phi3)
c
c calculating q1 in Q rest frame
c
      rm12 = rmass(1)**2
      e1  = (qq2-q2p+rm12)/2.d0/q0
      vq1 = sqrt(e1**2-rm12)
      q1(1) =  e1
      q1(2) =  vq1 * sinth1 * cosphi1      
      q1(3) =  vq1 * sinth1 * sinphi1      
      q1(4) =  vq1 * costh1
c
c calculating q2 in Q' rest frame
c
      rm22 = rmass(2)**2
      e2  = (q2p-q2b+rm22)/2.d0/q0p
      vq2 = sqrt(e2**2-rm22)
      q2(1) =  e2
      q2(2) =  vq2 * sinth2 * cosphi2      
      q2(3) =  vq2 * sinth2 * sinphi2      
      q2(4) =  vq2 * costh2
c
c calculating q3 and q4 in Q" rest frame
c    
      e3  = (q2b+rmass(3)**2-rmass(4)**2)/2.d0/q0b
      vq3 = sqrt(e3**2-rmass(3)**2)
      q3(1) =  e3
      q3(2) =  vq3 * sinth3 * cosphi3      
      q3(3) =  vq3 * sinth3 * sinphi3      
      q3(4) =  vq3 * costh3
      q4(1) =  q0b - e3
      q4(2) = -q3(2)
      q4(3) = -q3(3)
      q4(4) = -q3(4)
c
c boosting q3 and q4 into Q' rest frame
c
      call boost1(q0p,q0b,e2,vq2,costh2,sinth2,cosphi2,sinphi2,q3)
      call boost1(q0p,q0b,e2,vq2,costh2,sinth2,cosphi2,sinphi2,q4)
c
c boosting q2,q3 and q4 into Q rest frame
c
      call boost1(q0,q0p,e1,vq1,costh1,sinth1,cosphi1,sinphi1,q2)
      call boost1(q0,q0p,e1,vq1,costh1,sinth1,cosphi1,sinphi1,q3)
      call boost1(q0,q0p,e1,vq1,costh1,sinth1,cosphi1,sinphi1,q4)
c
      fac = dx1 * dx2 * (4.d0*pi)**3
      if(pion.eq.2)then
        do i=1,4
         pom_v(i) = q1(i)+q3(i)+q4(i)
        enddo
        q2pp = pom_v(1)**2 - pom_v(2)**2- pom_v(3)**2- pom_v(4)**2
        fac = fac/appr1(q2p,q2pp)
        if(ip0.eq.2)then
        do i=1,4
          pom_v(i) = q1(i)
          q1(i)    = q2(i)
          q2(i)    = pom_v(i)
        enddo
        endif    
      endif
c
c filling the four momenta
c
      do i =0,3
       momenta(6,i) = q1(i+1)
       momenta(7,i) = q2(i+1)
       momenta(8,i) = q3(i+1)
       momenta(9,i) = q4(i+1)
      enddo
c
      return
      end
c *******************************************************************
c this subroutine generates  3 massive particle phase space 
c in q=q1+q2+q3 rest frame (bosted afterwards into e^+e^- CMS)
c
c Mode   pi^+ pi^- pi^0 (pion=4)
c q1    - four momentum of pi^+
c q2    - four momentum of pi^-
c q3    - four momentum of pi^0
c
      subroutine pionangles_2(qq2,q0p,q2p,fac)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'

      real*8 rr,rr_r(0:5),qq2,q1,q2,q3,fac,rlam,a,b,c,x1min,q0,x1max,
     1   dx1,q2p,q0p,pp,xx2,phi1,costh1,sinth1,cosphi1,sinphi1,phi2,
     2   costh2,sinth2,cosphi2,sinphi2,e1,e2,vq1,vq2,x2min,x2max,const2,
     3   dx2,vv_3pi,const_1_p,mm_1_p,mg_1_p,d_1_p,xx_1_p_max,xx_1_p_min
      real*8 gg_1,mm_1,const_s,pom,q12p,q13p,q23p,
     1       qq12_3pi,qq13_3pi,qq23_3pi
      integer i,jj,ru

c
      dimension rr(6),q1(4),q2(4),q3(4)
      common/qqij_3pi/qq12_3pi,qq13_3pi,qq23_3pi
c
      call ranlxdf(rr_r,6)
      do ru=0,5
         rr(ru+1) = rr_r(ru)
      enddo

      pp = rr(6)

      x1min = (rmass(3)+rmass(2))**2  
      q0    = sqrt(qq2)
      x1max = (q0-rmass(1))**2      
      dx1   = x1max - x1min         
c
      if(Sp.lt.4.d0)then
        q2p    = x1min + dx1*rr(1)  
      else
c ---------------------------------------------
      const2 = 1000.d0

      x2min = Sp*log(x1min/Sp)
      dx2 = Sp*log(x1max/x1min)

      mm_1_p = 1.5d0
      mg_1_p = 0.25d0
      const_1_p = 100.d0 *(mg_1_p/mm_1_p)**2

      xx_1_p_max =  const_1_p*mm_1_p**3/mg_1_p * 
     1   dAtan((x1max/mm_1_p**2-1.d0)*mm_1_p/mg_1_p)
      xx_1_p_min = const_1_p*mm_1_p**3/mg_1_p * 
     1   dAtan((x1min/mm_1_p**2-1.d0)*mm_1_p/mg_1_p)
      d_1_p = xx_1_p_max - xx_1_p_min

      vv_3pi = dx1 + const2*dx2 + d_1_p

       if(pp.lt.(dx1/vv_3pi)) then
          q2p = x1min + dx1*rr(1)       
       elseif(pp.lt.((dx1+d_1_p)/vv_3pi)) then
          xx2 = xx_1_p_min + d_1_p * rr(1)
          q2p = mg_1_p*mm_1_p*dTan(xx2*mg_1_p/
     1            mm_1_p**3/const_1_p) + mm_1_p**2
       else
          xx2 = x2min + dx2*rr(1)
          q2p = Sp*exp(xx2/Sp)
       endif

      endif

c ---------------------------------------------
      q0p   = sqrt(q2p)
c
      phi1    = 2.d0*pi*rr(2)
      costh1  = -1.d0 + 2.d0*rr(3)
      sinth1  = 2.d0*sqrt(abs(rr(3)*(1.d0-rr(3))))
      cosphi1 = cos(phi1)
      sinphi1 = sin(phi1)
c
      phi2    = 2.d0*pi*rr(4)
      costh2  = -1.d0 + 2.d0*rr(5)
      sinth2  = 2.d0*sqrt(abs(rr(5)*(1.d0-rr(5))))
      cosphi2 = cos(phi2)
      sinphi2 = sin(phi2)
c calculating q1 in Q rest frame
c
      e1  = (qq2-q2p+rmass(1)**2)/2.d0/q0
      vq1 = sqrt(abs(e1**2-rmass(1)**2))
      q1(1) =  e1
      q1(2) =  vq1 * sinth1 * cosphi1      
      q1(3) =  vq1 * sinth1 * sinphi1      
      q1(4) =  vq1 * costh1
c
c calculating q2 and q3 in Q' rest frame
c
      e2  = (q2p+rmass(2)**2-rmass(3)**2)/2.d0/q0p

      vq2 = sqrt(abs(e2**2-rmass(2)**2))
      q2(1) =  e2
      q2(2) =  vq2 * sinth2 * cosphi2      
      q2(3) =  vq2 * sinth2 * sinphi2      
      q2(4) =  vq2 * costh2

      q3(1) =  q0p-q2(1)
      q3(2) = -q2(2)
      q3(3) = -q2(3)
      q3(4) = -q2(4)
c
c boosting q2 and q3 into Q rest frame
c
      call boost1(q0,q0p,e1,vq1,costh1,sinth1,cosphi1,sinphi1,q2)
      call boost1(q0,q0p,e1,vq1,costh1,sinth1,cosphi1,sinphi1,q3)

c ----------------------
      qq12_3pi =  (q1(1) + q2(1))**2
      do jj=2,4
       qq12_3pi = qq12_3pi - (q1(jj) + q2(jj))**2
      enddo
      qq13_3pi =  (q1(1) + q3(1))**2
      do jj=2,4
       qq13_3pi = qq13_3pi - (q1(jj) + q3(jj))**2
      enddo
      qq23_3pi = (q2(1) + q3(1))**2
      do jj=2,4
       qq23_3pi = qq23_3pi - (q2(jj) + q3(jj))**2
      enddo
c
c calculating the weight
c
      if(Sp.lt.4.d0)then
       fac =  (4.d0*pi)**2*dx1 
      else
       fac =  (4.d0*pi)**2*vv_3pi/( 1.d0 
     1    + const2*Sp/q2p
     2    + const_1_p/((q2p/mm_1_p**2-1.d0)**2 +(mg_1_p/mm_1_p)**2) ) 
      endif
c
c     filling the four momenta
c
      do i=0,3
         momenta(6,i) = q1(i+1)
         momenta(7,i) = q2(i+1)
         momenta(8,i) = q3(i+1)
      enddo

      return
      end
c *******************************************************************
c this subroutine generates  3 massive particle phase space 
c in q=q1+q2+q3 rest frame (bosted afterwards into e^+e^- CMS)
c
c Mode    eta pi^+ pi^-  (pion=10)
c q1    - four momentum of eta
c q2    - four momentum of pi^+
c q3    - four momentum of pi^-
c
      subroutine pionangles_eta(qq2,q0p,q2p,fac)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'

      real*8 rr,rr_r(0:5),qq2,q1,q2,q3,fac,rlam,a,b,c,x1min,q0,x1max,
     1   dx1,q2p,q0p,pp,xx2,phi1,costh1,sinth1,cosphi1,sinphi1,phi2,
     2   costh2,sinth2,cosphi2,sinphi2,e1,e2,vq1,vq2,x2min,x2max,const2,
     3   dx2,vv_3pi,const_1_p,mm_1_p,mg_1_p,d_1_p,xx_1_p_max,xx_1_p_min
      real*8 gg_1,mm_1,const_s,pom,q12p,q13p,q23p,
     1       qq12_3pi,qq13_3pi,qq23_3pi
      integer i,jj,ru
c
      dimension rr(6),q1(4),q2(4),q3(4)
      common/qqij_3pi/qq12_3pi,qq13_3pi,qq23_3pi
c
      call ranlxdf(rr_r,6)
      do ru=0,5
         rr(ru+1) = rr_r(ru)
      enddo

      pp = rr(6)

      x1min = (rmass(3)+rmass(2))**2  
      q0    = sqrt(qq2)
      x1max = (q0-rmass(1))**2      
      dx1   = x1max - x1min         
c
c ---------------------------------------------
      const2 = 1.d0

      x2min = Sp*log(x1min/Sp)
      dx2 = Sp*log(x1max/x1min)

      mm_1_p = 0.77549d0
      mg_1_p = 0.1494d0
      const_1_p = 1.d0 

      xx_1_p_max =  const_1_p*mm_1_p**3/mg_1_p * 
     1   dAtan((x1max/mm_1_p**2-1.d0)*mm_1_p/mg_1_p)
      xx_1_p_min = const_1_p*mm_1_p**3/mg_1_p * 
     1   dAtan((x1min/mm_1_p**2-1.d0)*mm_1_p/mg_1_p)
      d_1_p = xx_1_p_max - xx_1_p_min

      vv_3pi = dx1 + const2*dx2 + d_1_p

       if(pp.lt.(dx1/vv_3pi)) then
          q2p = x1min + dx1*rr(1)       
       elseif(pp.lt.((dx1+d_1_p)/vv_3pi)) then
          xx2 = xx_1_p_min + d_1_p * rr(1)
          q2p = mg_1_p*mm_1_p*dTan(xx2*mg_1_p/
     1            mm_1_p**3/const_1_p) + mm_1_p**2
       else
          xx2 = x2min + dx2*rr(1)
          q2p = Sp*exp(xx2/Sp)
       endif

c ---------------------------------------------
      q0p   = sqrt(q2p)
c
      phi1    = 2.d0*pi*rr(2)
      costh1  = -1.d0 + 2.d0*rr(3)
      sinth1  = 2.d0*sqrt(abs(rr(3)*(1.d0-rr(3))))
      cosphi1 = cos(phi1)
      sinphi1 = sin(phi1)
c
      phi2    = 2.d0*pi*rr(4)
      costh2  = -1.d0 + 2.d0*rr(5)
      sinth2  = 2.d0*sqrt(abs(rr(5)*(1.d0-rr(5))))
      cosphi2 = cos(phi2)
      sinphi2 = sin(phi2)
c calculating q1 in Q rest frame
c
      e1  = (qq2-q2p+rmass(1)**2)/2.d0/q0
      vq1 = sqrt(abs(e1**2-rmass(1)**2))
      q1(1) =  e1
      q1(2) =  vq1 * sinth1 * cosphi1      
      q1(3) =  vq1 * sinth1 * sinphi1      
      q1(4) =  vq1 * costh1
c
c calculating q2 and q3 in Q' rest frame
c
      e2  = (q2p+rmass(2)**2-rmass(3)**2)/2.d0/q0p

      vq2 = sqrt(abs(e2**2-rmass(2)**2))
      q2(1) =  e2
      q2(2) =  vq2 * sinth2 * cosphi2      
      q2(3) =  vq2 * sinth2 * sinphi2      
      q2(4) =  vq2 * costh2

      q3(1) =  q0p-q2(1)
      q3(2) = -q2(2)
      q3(3) = -q2(3)
      q3(4) = -q2(4)
c
c boosting q2 and q3 into Q rest frame
c
      call boost1(q0,q0p,e1,vq1,costh1,sinth1,cosphi1,sinphi1,q2)
      call boost1(q0,q0p,e1,vq1,costh1,sinth1,cosphi1,sinphi1,q3)

c ----------------------
      qq12_3pi =  (q1(1) + q2(1))**2
      do jj=2,4
       qq12_3pi = qq12_3pi - (q1(jj) + q2(jj))**2
      enddo
      qq13_3pi =  (q1(1) + q3(1))**2
      do jj=2,4
       qq13_3pi = qq13_3pi - (q1(jj) + q3(jj))**2
      enddo
      qq23_3pi = (q2(1) + q3(1))**2
      do jj=2,4
       qq23_3pi = qq23_3pi - (q2(jj) + q3(jj))**2
      enddo
c
c calculating the weight
c
       fac =  (4.d0*pi)**2*vv_3pi/( 1.d0 
     1    + const2*Sp/q2p
     2    + const_1_p/((q2p/mm_1_p**2-1.d0)**2 +(mg_1_p/mm_1_p)**2) ) 
c
c     filling the four momenta
c
      do i=0,3
         momenta(6,i) = q1(i+1)
         momenta(7,i) = q2(i+1)
         momenta(8,i) = q3(i+1)
      enddo

      return
      end
c *******************************************************************
c generation of the 3pi invariant mass: flat + omega Breit-Wigner
c *******************************************************************
      subroutine chann1(rrr,q2p_min,q2p_max,q2p,dx1,ip0)
      use belle2_phokhara_interface
      implicit none
      real*8 rrr,q2p_min,q2p_max,q2p,dx1,dm,dg,dm2,dmg,rr1(1),
     1       rr1_r(0:2),vol1,vol2,vol3,vol4,vol,r1_test,r2_test,r3_test,
     2       xx,x_min,x_max
      integer ip0 
c
      dm = 0.782d0
      dg = 0.0085d0
c
      dm2 = dm**2
      dmg = dm*dg
c
      x_min = dm2 *atan((q2p_min-dm2)/dmg)
      x_max = dm2 *atan((q2p_max-dm2)/dmg)
      vol2 = x_max-x_min
      vol1 = q2p_max - q2p_min
      vol3 = vol1
      vol4 = vol2
      vol = vol1 + vol2 + vol3 + vol4
c
      r1_test = vol1/vol
      r2_test = (vol1+vol2)/vol
      r3_test = (vol1+vol2+vol3)/vol
c
      call ranlxdf(rr1_r,1)
      rr1(1) = rr1_r(0)
c
      if(rr1(1).lt.r1_test)then
       q2p = q2p_min + vol1*rrr
       ip0 = 1
      elseif(rr1(1).lt.r2_test)then
       xx = x_min + vol2*rrr
       q2p = dm2 + dmg*tan(xx/dm2) 
       ip0 = 1
      elseif(rr1(1).lt.r3_test)then
       q2p = q2p_min + vol1*rrr
       ip0 = 2
      else
       xx = x_min + vol2*rrr
       q2p = dm2 + dmg*tan(xx/dm2) 
       ip0 = 2
      endif
      dx1 = vol
c
      return
      end
c *******************************************************************
c
      real*8 function appr1(qq12,qq22)
      implicit none
      real*8 dm,dg,dm2,dm3g,dmg2,qq12,qq22
c
      dm = 0.782d0
      dg = 0.0085d0
      dm2 = dm**2
      dm3g = dm2*dm*dg
      dmg2 = dm2*dg**2
      appr1 = 2.d0+dm3g/((qq12-dm2)**2+dmg2)+dm3g/((qq22-dm2)**2+dmg2)
c
      return
      end
c ------------------------------------------------------------------
c     hadron four momenta                                            
c ------------------------------------------------------------------
      subroutine hadronmomenta(qq,costheta,phi)
      include 'phokhara_10.0.inc'
      real*8 qq,p,costheta,sintheta,phi,m2,
     &        cmsvector(0:3),boostvector(0:3),labvector(0:3),
     & ransym
      integer i
      common/sym/ransym
      
      sintheta = dSqrt(1.d0-costheta*costheta)
      if ((pion.eq.0).or.(pion.eq.11).or.(pion.eq.12)) then 
         m2 = mmu*mmu
      elseif (pion.eq.4) then 
         m2 = mp*mp
      elseif (pion.eq.5) then 
         m2 = mnt*mnt
      elseif (pion.eq.6) then 
         m2 = mKp*mKp
      elseif (pion.eq.7) then
         m2 = mKn*mKn
      elseif (pion.eq.9) then
         m2 = mlamb*mlamb
      else
         m2 = mpi*mpi
      endif
c --- pions/muons/protons/neutrons/kaons/lambdas in the qq-rest frame ---
c --- pi^+/mu^+/pbar/nbar/K^+/K^0/lambda bar ---
      momenta(6,0) = dSqrt(qq)/2.d0
      p            = momenta(6,0)*dSqrt(1.d0-4.d0*m2/qq)
      momenta(6,1) = p*sintheta*dCos(phi)
      momenta(6,2) = p*sintheta*dSin(phi)
      momenta(6,3) = p*costheta
c --- pi^-/mu^-/p/n/K^-/K^0bar/lambda---
      momenta(7,0) = momenta(6,0) 
      do i = 1,3
        momenta(7,i) = -momenta(6,i)
      enddo

c --- boost the hadron momenta into the e^+ e^- CMS ---
      call hadronmomenta_1(2)
      return
      end
c ------------------------------------------------------------------
c     hadron four momenta                                            
c ------------------------------------------------------------------
      subroutine hadronmomenta_0ph(qq,costheta,phi)
      include 'phokhara_10.0.inc'
      real*8 qq,p,costheta,sintheta,phi,m2,
     &        cmsvector(0:3),boostvector(0:3),labvector(0:3)
      integer i
      
      sintheta = dSqrt(1.d0-costheta*costheta)
      if (pion.eq.0) then 
         m2 = mmu*mmu
      elseif (pion.eq.4) then 
         m2 = mp*mp
      elseif (pion.eq.5) then 
         m2 = mnt*mnt
      elseif (pion.eq.6) then 
         m2 = mKp*mKp
      elseif (pion.eq.7) then
         m2 = mKn*mKn
      elseif (pion.eq.9) then
         m2 = mlamb*mlamb
      else
         m2 = mpi*mpi
      endif
c --- pions/muons/protons/neutrons/kaons/lambdas in the qq-rest frame ---
c --- pi^+/mu^+/pbar/nbar/K^+/K^0/lambda bar ---
      momenta(6,0) = dSqrt(qq)/2.d0
      p            = momenta(6,0)*dSqrt(1.d0-4.d0*m2/qq)
      momenta(6,1) = p*sintheta*dCos(phi)
      momenta(6,2) = p*sintheta*dSin(phi)
      momenta(6,3) = p*costheta
c --- pi^-/mu^-/p/n/K^-/K^0bar/lambda---
      momenta(7,0) = momenta(6,0) 
      do i = 1,3
        momenta(7,i) = -momenta(6,i)
      enddo
c-----------------------------------------
cST----for pi0 gamma channel-------------
      if(pion.eq.13)then
c momenta(6,mu) - pi0 momentum
c momenta(7,mu) - gamm momentum
      momenta(6,0)=(qq+mpi0f**2)/(2.d0*dsqrt(qq)) 
      momenta(7,0)=(qq-mpi0f**2)/(2.d0*dsqrt(qq))
      p=momenta(7,0) ! p=momenta(6,0)**2-mpi0**2
      momenta(6,1) = p*sintheta*dCos(phi)
      momenta(6,2) = p*sintheta*dSin(phi)
      momenta(6,3) = p*costheta
         do i = 1,3
             momenta(7,i) = -momenta(6,i)
         enddo 
      elseif(pion.eq.14)then
c momenta(6,mu) - eta momentum
c momenta(7,mu) - gamm momentum
      momenta(6,0)=(qq+metaf**2)/(2.d0*dsqrt(qq)) 
      momenta(7,0)=(qq-metaf**2)/(2.d0*dsqrt(qq))
      p=momenta(7,0) ! p=momenta(6,0)**2-mpi0**2
      momenta(6,1) = p*sintheta*dCos(phi)
      momenta(6,2) = p*sintheta*dSin(phi)
      momenta(6,3) = p*costheta
         do i = 1,3
             momenta(7,i) = -momenta(6,i)
         enddo 
      elseif(pion.eq.15)then
c momenta(6,mu) - etaP momentum
c momenta(7,mu) - gamm momentum
      momenta(6,0)=(qq+metaP**2)/(2.d0*dsqrt(qq)) 
      momenta(7,0)=(qq-metaP**2)/(2.d0*dsqrt(qq))
      p=momenta(7,0) ! p=momenta(6,0)**2-mpi0**2
      momenta(6,1) = p*sintheta*dCos(phi)
      momenta(6,2) = p*sintheta*dSin(phi)
      momenta(6,3) = p*costheta
         do i = 1,3
             momenta(7,i) = -momenta(6,i)
         enddo   
      endif
c-----------------------------------------
      return
      end
c ------------------------------------------------------------------
c        boost four momenta from Q rest frame to e^+ e^- CMS             
c ------------------------------------------------------------------
      subroutine hadronmomenta_1(Np)
      include 'phokhara_10.0.inc'
      real*8 p,costheta,sintheta,phi,m2,
     &        cmsvector(0:3),boostvector(0:3),labvector(0:3),
     1        q1(4),q2(4),q3(4),q4(4)
      integer i,j,Np
c      
c --- boost the hadron momenta into the e^+ e^- CMS ---
c
      do i =0,3
         boostvector(i) = momenta(5,i)
      enddo   
c
      do j=1,Np
         do i =0,3
            cmsvector(i) = momenta(5+j,i)
         enddo   
         call boost(cmsvector,boostvector,labvector)
         do i =0,3
            momenta(5+j,i) = labvector(i) 
         enddo         
      enddo
      return
      end
c ------------------------------------------------------------------
c --------------------------------------------------------------------
c        hadron four momenta pi^- p (pi^+ pbar) (pion=9)  
c        boost from Lambda (Lambda bar) rest frame to Q rest frame 
c        boost from Q rest frame to e^+ e^- CMS             
c --------------------------------------------------------------------
      subroutine hadronmomenta_Lamb(qq,costheta,phi,costhm,phim,costhp,
     1                                                            phip)
      include 'phokhara_10.0.inc'
      real*8 qq,p,costheta,sintheta,phi,m2,
     2        cmsvector(0:3),boostvector(0:3),labvector(0:3)
      real*8 costhm,phim,sinthm,costhp,phip,sinthp,e1pi,e1p,vpp1,
     1        pom1(1:2,0:3),wyn1,qq1l(0:3),veclamb1(0:3),veclamb2(0:3),
     2        vecpom(4)
      integer i,j
      common /veclamb/ veclamb1,veclamb2
      
      sintheta = dSqrt(1.d0-costheta*costheta)
      m2 = mlamb*mlamb
      p        = dSqrt(qq)/2.d0 * dSqrt(1.d0-4.d0*m2/qq)

c ----------- 
      sinthm = dSqrt(1.d0-costhm*costhm)
      e1pi = (mlamb**2 - mp**2 + mpi**2)/2.d0/mlamb
      e1p  = (mlamb**2 + mp**2 - mpi**2)/2.d0/mlamb
      vpp1 = Sqrt(abs(e1pi**2-mpi**2))

      veclamb2(0) = 0.d0
      veclamb2(1) = sinthm*Cos(phim)
      veclamb2(2) = sinthm*Sin(phim)
      veclamb2(3) = costhm
      do i=1,4
         vecpom(i) = veclamb2(i-1)
      enddo
c               Lambda rest frame to qq-rest frame
         call boost1(Sqrt(qq),mlamb,dSqrt(qq)/2.d0,p,costheta,sintheta,
     1                                      Cos(phi),Sin(phi),vecpom)
      do i=1,4
         veclamb2(i-1) = vecpom(i)  
      enddo

      sinthp = dSqrt(1.d0-costhp*costhp)

      veclamb1(0) = 0.d0
      veclamb1(1) = sinthp*Cos(phip)
      veclamb1(2) = sinthp*Sin(phip)
      veclamb1(3) = costhp
      do i=1,4
         vecpom(i) = veclamb1(i-1)
      enddo
c               Lambda bar rest frame to qq-rest frame
         call boost1(Sqrt(qq),mlamb,dSqrt(qq)/2.d0,-p,costheta,sintheta,
     1                                       Cos(phi),Sin(phi),vecpom)
      do i=1,4
         veclamb1(i-1) = vecpom(i)  
      enddo

c --- Lambda rest frame --
c --- pion^- 
      momenta(10,0) = e1pi
      momenta(10,1) = vpp1 * sinthm * Cos(phim)
      momenta(10,2) = vpp1 * sinthm * Sin(phim)
      momenta(10,3) = vpp1 * costhm

c --- proton
      momenta(11,0) = e1p
      do i = 1,3
        momenta(11,i) = -momenta(10,i)
      enddo

c --- Lambda bar rest frame --
c --- pion^+ 
      momenta(8,0) = e1pi
      momenta(8,1) = vpp1 * sinthp * Cos(phip)
      momenta(8,2) = vpp1 * sinthp * Sin(phip)
      momenta(8,3) = vpp1 * costhp

c --- proton bar
      momenta(9,0) = e1p 
      do i = 1,3
        momenta(9,i) = -momenta(8,i)
      enddo

c --- boost the hadron momenta from Lambda rest frame to qq-rest frame
      do j=10,11         
         do i=0,3
            vecpom(i+1)=momenta(j,i)
         enddo
         call boost1(Sqrt(qq),mlamb,
     1      Sqrt(qq)/2.d0,p,costheta,sintheta,Cos(phi),Sin(phi),vecpom)
         do i=0,3
            momenta(j,i)=vecpom(i+1)
         enddo
      enddo

c --- boost the hadron momenta from Lambda bar rest frame to qq-rest frame
      do j=8,9         
         do i=0,3
            vecpom(i+1)=momenta(j,i)
         enddo
         call boost1(Sqrt(qq),mlamb,
     1     Sqrt(qq)/2.d0,-p,costheta,sintheta,Cos(phi),Sin(phi),vecpom)
         do i=0,3
            momenta(j,i)=vecpom(i+1)
         enddo
      enddo

c --- boost the hadron momenta into the e^+ e^- CMS ---   !!!!!!!!!!!!!
         do i = 0,3
            boostvector(i) = momenta(5,i)
         enddo   

      do i = 0,3
          cmsvector(i) = veclamb1(i)
      enddo   
        call boost(cmsvector,boostvector,labvector)
      do i = 0,3
        veclamb1(i) = labvector(i) 
        cmsvector(i) = veclamb2(i)
      enddo   
      call boost(cmsvector,boostvector,labvector)
      do i = 0,3
         veclamb2(i) = labvector(i) 
      enddo   
 
      do j = 8,11
         do i = 0,3
            cmsvector(i) = momenta(j,i)
         enddo   
         call boost(cmsvector,boostvector,labvector)
         do i = 0,3
            momenta(j,i) = labvector(i) 
         enddo   
      enddo
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      subroutine hadronmomenta_Lamb0(qq,costheta,phi,costhm,phim,costhp,
     1                                                            phip)
      include 'phokhara_10.0.inc'
      real*8 qq,p,costheta,sintheta,phi,m2,
     2        cmsvector(0:3),boostvector(0:3),labvector(0:3)
      real*8 costhm,phim,sinthm,costhp,phip,sinthp,e1pi,e1p,vpp1,
     1        pom1(1:2,0:3),wyn1,qq1l(0:3),veclamb1(0:3),veclamb2(0:3),
     2        vecpom(4)
      integer i,j
      common /veclamb/ veclamb1,veclamb2

      sintheta = dSqrt(1.d0-costheta*costheta)
      m2 = mlamb*mlamb
      p        = dSqrt(qq)/2.d0 * dSqrt(1.d0-4.d0*m2/qq)

c -----------                                                                                                                                                                                                 
      sinthm = dSqrt(1.d0-costhm*costhm)
      e1pi = (mlamb**2 - mp**2 + mpi**2)/2.d0/mlamb
      e1p  = (mlamb**2 + mp**2 - mpi**2)/2.d0/mlamb
      vpp1 = Sqrt(abs(e1pi**2-mpi**2))

      veclamb2(0) = 0.d0
      veclamb2(1) = sinthm*Cos(phim)
      veclamb2(2) = sinthm*Sin(phim)
      veclamb2(3) = costhm
      do i=1,4
         vecpom(i) = veclamb2(i-1)
      enddo
c               Lambda rest frame to qq-rest frame                                                                                                                                                            
         call boost1(Sqrt(qq),mlamb,dSqrt(qq)/2.d0,p,costheta,sintheta,
     1                                      Cos(phi),Sin(phi),vecpom)
      do i=1,4
         veclamb2(i-1) = vecpom(i)
      enddo

      sinthp = dSqrt(1.d0-costhp*costhp)

      veclamb1(0) = 0.d0
      veclamb1(1) = sinthp*Cos(phip)
      veclamb1(2) = sinthp*Sin(phip)
      veclamb1(2) = sinthp*Sin(phip)
      veclamb1(3) = costhp
      do i=1,4
         vecpom(i) = veclamb1(i-1)
      enddo
c               Lambda bar rest frame to qq-rest frame                                                                                                                                                        
         call boost1(Sqrt(qq),mlamb,dSqrt(qq)/2.d0,-p,costheta,sintheta,
     1                                       Cos(phi),Sin(phi),vecpom)
      do i=1,4
         veclamb1(i-1) = vecpom(i)
      enddo

c --- Lambda rest frame --                                                                                                                                                                                    
c --- pion^-                                                                                                                                                                                                  
      momenta(10,0) = e1pi
      momenta(10,1) = vpp1 * sinthm * Cos(phim)
      momenta(10,2) = vpp1 * sinthm * Sin(phim)
      momenta(10,3) = vpp1 * costhm

c --- proton                                                                                                                                                                                                  
      momenta(11,0) = e1p
      do i = 1,3
        momenta(11,i) = -momenta(10,i)
      enddo

c --- Lambda bar rest frame --                                                                                                                                                                                
c --- pion^+                                                                                                                                                                                                  
      momenta(8,0) = e1pi
      momenta(8,1) = vpp1 * sinthp * Cos(phip)
      momenta(8,2) = vpp1 * sinthp * Sin(phip)
      momenta(8,3) = vpp1 * costhp

c --- proton bar 
      momenta(9,0) = e1p
      do i = 1,3
        momenta(9,i) = -momenta(8,i)
      enddo

c --- boost the hadron momenta from Lambda rest frame to qq-rest frame                                                                                                                                        
      do j=10,11
         do i=0,3
            vecpom(i+1)=momenta(j,i)
         enddo
         call boost1(Sqrt(qq),mlamb,
     1      Sqrt(qq)/2.d0,p,costheta,sintheta,Cos(phi),Sin(phi),vecpom)
         do i=0,3
            momenta(j,i)=vecpom(i+1)
         enddo
      enddo

c --- boost the hadron momenta from Lambda bar rest frame to qq-rest frame                                                                                                                                    
      do j=8,9
         do i=0,3
            vecpom(i+1)=momenta(j,i)
         enddo
         call boost1(Sqrt(qq),mlamb,
     1     Sqrt(qq)/2.d0,-p,costheta,sintheta,Cos(phi),Sin(phi),vecpom)
         do i=0,3
            momenta(j,i)=vecpom(i+1)
         enddo
      enddo 


      return
      end
c -----------------------------------------------------------
c     boost cmsvector by boostvector into labvector          
c -----------------------------------------------------------
      subroutine boost(cmsvector,boostvector,labvector)
      implicit none
      real*8 cmsvector(0:3),boostvector(0:3),labvector(0:3),
     1   m(0:3,0:3),E,p,beta,gamma,costheta,sintheta,cosphi,sinphi
      integer i,j
      
      E = boostvector(0)
      p = dSqrt(boostvector(1)**2+boostvector(2)**2+
     &    boostvector(3)**2)
      beta  = p/E
      gamma = 1.d0/dSqrt(1.d0-beta*beta)
      costheta = boostvector(3)/p
      sintheta = sqrt(boostvector(1)**2+boostvector(2)**2)/p
      if(sintheta.ne.0.d0)then
        cosphi   = boostvector(1)/(p*sintheta)
        sinphi   = boostvector(2)/(p*sintheta)
      else
        cosphi = 1.d0
        sinphi = 0.d0
      endif

        m(0,0) = gamma
        m(0,1) = 0.d0
        m(0,2) = 0.d0
        m(0,3) = beta*gamma
        m(1,0) = beta*gamma*sintheta*cosphi
        m(1,1) = costheta*cosphi
        m(1,2) = -sinphi
        m(1,3) = gamma*sintheta*cosphi
        m(2,0) = beta*gamma*sintheta*sinphi
        m(2,1) = costheta*sinphi
        m(2,2) = cosphi
        m(2,3) = gamma*sintheta*sinphi
        m(3,0) = beta*gamma*costheta
        m(3,1) = -sintheta
        m(3,2) = 0.d0
        m(3,3) = gamma*costheta

      do i=0,3
         labvector(i) = 0.d0
         do j=0,3
            labvector(i) = labvector(i)+m(i,j)*cmsvector(j)
         enddo
      enddo

      return
      end
c ----------------------------------------------------------------------
c this subroutine boost fourvector qq from Q' rest frame to Q rest frame
c           Q' = Q - q1
c
      subroutine boost1(q0,q0p,e1,vq1,c1,s1,cf1,sf1,qq)
      implicit none
      real*8 q0,q0p,e1,vq1,c1,s1,cf1,sf1,qq1(4),qq(4)
     1                ,transf(4,4),fac1,fac2
      integer i,j
c
      fac1 = (q0-e1)/q0p
      fac2 = -vq1/q0p
c
      transf(1,1) =  fac1
      transf(2,1) =  fac2 * s1 * cf1      
      transf(3,1) =  fac2 * s1 * sf1     
      transf(4,1) =  fac2 * c1      
      transf(1,2) =  0.d0      
      transf(2,2) =  c1 * cf1     
      transf(3,2) =  c1 * sf1      
      transf(4,2) = -s1      
      transf(1,3) =  0.d0      
      transf(2,3) = -sf1      
      transf(3,3) =  cf1      
      transf(4,3) =  0.d0     
      transf(1,4) =  fac2     
      transf(2,4) =  fac1 * s1 * cf1     
      transf(3,4) =  fac1 * s1 * sf1  
      transf(4,4) =  fac1 * c1   
c
        do i=1,4
          qq1(i) = 0.d0
          do j = 1,4
             qq1(i) = qq1(i) + transf(i,j)*qq(j)
          enddo
        enddo
c
      do i=1,4
        qq(i) = qq1(i)
      enddo
c
      return
      end
c ========================================================================
c --- Spin Lambda test
c ------------------------------------------------------------------------
      subroutine test_spin(qq,Hadronic,amplit,accepted)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'
      real*8 Matrix,qq,amplit
      real*8 rr_ll,ff_min_ll,ff_max_ll,ff_ll,ff_spin_ll
      complex*16 Leptonic(0:3,0:3),Hadronic(0:3,0:3),
     1           HadronicLamb(0:3,0:3)
      logical accepted
      integer n,i,j,ij,mj

      common/Lept_ee/Leptonic

      call ranlxdf(rr_ll,1)  

      accepted = .true.

      call HadroLamb(qq,Hadronic,HadronicLamb)

       ff_min_ll = 0.d0
       ff_max_ll = (1.d0 + alpha_lamb)**2
       ff_ll = ff_min_ll + (ff_max_ll - ff_min_ll) * rr_ll

       ff_spin_ll = Matrix(Leptonic,HadronicLamb) / amplit * 4.d0

       accepted = (accepted.and.(ff_ll.lt.ff_spin_ll))

      return
      end
c========================================================================
c        Spin lambda test for 0 photon part
c 
      subroutine test_spin0(qq,Hadronic,amplit,accepted)
      use belle2_phokhara_interface
      include 'phokhara_10.0.inc'
      real*8 Matrix,qq,amplit
      real*8 rr_ll,ff_min_ll,ff_max_ll,ff_ll,ff_spin_ll
      complex*16 Leptonic(0:3,0:3),Hadronic(0:3,0:3),
     1           HadronicLamb(0:3,0:3)
      logical accepted
      integer n,i,j,ij,mj



      call ranlxdf(rr_ll,1)

      accepted = .true.

      call HadroLamb(qq,Hadronic,HadronicLamb)
     
       ff_min_ll = 0.d0
       ff_max_ll = (1.d0 + alpha_lamb)**2
       ff_ll = ff_min_ll + (ff_max_ll - ff_min_ll) * rr_ll

       ff_spin_ll= Matrix(Leptonic_epl_emin,HadronicLamb)/ amplit * 4.d0

       accepted = (accepted.and.(ff_ll.lt.ff_spin_ll))

      return
      end
c ========================================================================
c --- Test experimental cuts ---------------------------------------------
c ------------------------------------------------------------------------
      subroutine testcuts(n,accepted)
      include 'phokhara_10.0.inc'
      real*8 piplab,pimlab,m2,phot1,invmom(0:3),invm2,q1long,q2long
      integer n,i,j,ij,mj
      logical accepted,accept(3:4)
      logical BABAR,KLOE,BES
      real*8 qu(4),pipicut1,pmodtr(2),Efot_pi
      real*8 asquared,trkmass,Mphi,pipicut1_min,pipicut1_max
      real*8 ptran1,ptran2,angpho
      
      accepted = .true.
c -----------------------------------------
c --- angular cuts on the hadrons/muons ---
      mj = 6
      if ((pion.le.1).or.(pion.eq.4).or.(pion.eq.5).or.
     1    (pion.eq.6).or.(pion.eq.7).or.(pion.eq.11)
     2 .or.(pion.eq.12)) j = 7
      if ((pion.eq.2).or.(pion.eq.3)) j = 9 
      if ((pion.eq.8).or.(pion.eq.10)) j = 8
c      if (pion.eq.9) j = 7 ! Lambda (Lambda bar) 
       if (pion.eq.9) then  ! pi^-p pi^+pbar
             mj = 8
             j = 11 
       endif
      if((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then
         j=6
      endif

      do i = mj,j
      piplab = dacos(momenta(i,3)/
     &         dSqrt(momenta(i,1)**2+momenta(i,2)**2+momenta(i,3)**2))
     &         *180.d0/pi
      accepted = (accepted.and.(piplab.ge.pi1cut.and.piplab.le.pi2cut))
      enddo 


      BABAR=.false.
      KLOE=.false.
      BES=.false.

      if(BABAR) then
c BABAR
      angpho=-(momenta(6,3)+momenta(7,3))/sqrt(
     1 (momenta(6,1)+momenta(7,1))**2+(momenta(6,2)+momenta(7,2))**2
     2 +(momenta(6,3)+momenta(7,3))**2)

      q1long =dsqrt(momenta(7,1)**2 + momenta(7,2)**2 + momenta(7,3)**2)
      q2long =dsqrt(momenta(6,1)**2 + momenta(6,2)**2 + momenta(6,3)**2)
c
      accepted = (accepted.and.(q1long.gt.1.d0).and.(q2long.gt.1.d0)
     1 .and.(angpho.gt.-0.939692621d0).and.(angpho.lt.0.939692621d0))

      elseif(KLOE)then
c KLOE 
      if(pion.eq.0)then
      qu(1) = momenta(6,0) + momenta(7,0)
      qu(2) = momenta(6,1) + momenta(7,1)
      qu(3) = momenta(6,2) + momenta(7,2)
      qu(4) = momenta(6,3) + momenta(7,3)
c
      pipicut1_min = 50.d0*pi/180.d0
      pipicut1_max = 130.d0*pi/180.d0

c
      pipicut1 = acos( -qu(4)/Sqrt(qu(2)**2 +
     &           qu(3)**2 + qu(4)**2) )  !*180.d0/pi
c
      pmodtr(1)=sqrt(momenta(6,1)**2 + momenta(6,2)**2 +momenta(6,3)**2)
      pmodtr(2)=sqrt(momenta(7,1)**2 + momenta(7,2)**2 +momenta(7,3)**2)
c
      Mphi = 1.02d0
c
      Efot_pi = Sqrt(qu(2)**2+qu(3)**2+qu(4)**2)
c
      asquared = (Efot_pi - Mphi)**2 - pmodtr(1)**2
     1           - pmodtr(2)**2
c
      asquared = 0.5d0*asquared
c
      trkmass = ( asquared**2 - pmodtr(1)**2*pmodtr(2)**2 ) /
     1     ( 2.d0*asquared + pmodtr(1)**2 + pmodtr(2)**2 )
c
      if ( trkmass.ge.0.d0) then
        trkmass = Sqrt(trkmass)
       else
        trkmass = -Sqrt(-trkmass)
      endif

      angpho=-(momenta(6,3)+momenta(7,3))/sqrt(
     1 (momenta(6,1)+momenta(7,1))**2+(momenta(6,2)+momenta(7,2))**2
     2 +(momenta(6,3)+momenta(7,3))**2)
c

       accepted = (accepted.and.
     & ((pipicut1.lt.pipicut1_min).or.(pipicut1.gt.pipicut1_max))
     &  .and.
     & ((trkmass.gt.0.13d0).and.(abs(angpho).gt.0.965925826d0))
     & .and.(abs(momenta(6,3)).gt.0.09d0)
     & .and.(abs(momenta(7,3)).gt.0.09d0)!.and.
c     &  (trkmass.lt.0.115d0))
     &    )

      endif

      elseif(BES)then
      ptran1=sqrt(momenta(6,1)**2+momenta(6,2)**2)
      ptran2=sqrt(momenta(7,1)**2+momenta(7,2)**2)

      angpho=-(momenta(6,3)+momenta(7,3))/sqrt(
     1 (momenta(6,1)+momenta(7,1))**2+(momenta(6,2)+momenta(7,2))**2
     2 +(momenta(6,3)+momenta(7,3))**2)

      accepted=(accepted.and.((abs(angpho).lt.0.8d0).or.
     1 ((abs(angpho).gt.0.86d0).and.(abs(angpho).lt.0.92d0))).and.
     2 (ptran1.gt.0.3d0).and.(ptran2.gt.0.3d0))

      endif

c -------------------------------------------------------------
c    tagged photons: one photon is tagged, a second photon is 
c    generated everywhere
c -------------------------------------------------------------
c --- one of the photons has energy > gmin ---
c without 0
      if((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then
       continue
      else
      if((n.eq.1).or.(n.eq.2))then
      accepted = (accepted.and.
     &           (momenta(3,0).ge.gmin.or.momenta(4,0).ge.gmin))
      endif
c
c --- invariant mass of the tagged photon and the pions
c --- (muons,protons,neutrons,kaons) ---
      if (n.eq.2) then 
      do i = 3,4
         accept(i) = .false.
         phot1 = dacos(momenta(i,3)/momenta(i,0))*180.d0/pi
         if (momenta(i,0).ge.gmin.and.
     &      (phot1.ge.phot1cut.and.phot1.le.phot2cut)) then 
            do ij = 0,3
             invmom(ij) = momenta(i,ij)+momenta(5,ij)
            enddo 
            invm2 = invmom(0)**2-invmom(1)**2-invmom(2)**2-invmom(3)**2
            accept(i) = (invm2.ge.q2min)
         endif
      enddo
      accepted = (accepted.and.(accept(3).or.accept(4)))
      endif  
      endif          
      end
c ====================================================================
c --- Matrix element squared: contract Leptonic and Hadronic tensors -
c --------------------------------------------------------------------
      real*8 function Matrix(Leptonic,Hadronic)
      include 'phokhara_10.0.inc'       
      complex*16 Leptonic(0:3,0:3),Hadronic(0:3,0:3)
      real*8 metric1,metric2
      integer mu,nu
      
      Matrix = 0.d0
      do mu = 0,3
         metric1 = 1.d0
         if (mu.eq.0) metric1 = -1.d0
         do nu = 0,3
             metric2 = 1.d0
             if (nu.eq.0) metric2 = -1.d0               
             Matrix = Matrix + metric1*metric2*
     &          Leptonic(mu,nu)*Hadronic(mu,nu) 
         enddo
      enddo
      end
c **********************************************************************
c --- Metric tensor ----------------------------------------------------
      real*8 function metric(mu,nu)
      implicit none 
      integer mu,nu
      if (mu.ne.nu) then 
         metric = 0.d0
      else 
         if (mu.eq.0) then 
             metric = 1.d0
	 else
	     metric = -1.d0
	 endif
      endif
      end 
c =======================================================================
c     Leptonic tensor for the process                                     
c                                                                         
c         e^+(p1) + e^-(p2) ---->  photon^* + photon                      
c                                                                         
c     ISR at the NLO: virtual corrections + soft photons                  
c                                                                         
c     Sp : CMS energy squared
c     qq : virtuality of the photon coupled to the hadronic system        
c     cosphoton: cosinus of the hard photon-positron angle                
c          in the e^+ e^- CMS system                                       
c     w  : soft photon energy normalized to the CMS energy                 
c ------------------------------------------------------------------------
c                                                                         
c         a00 : coefficient of g(mu,nu)                                   
c         a11 :      "      p1(mu)*p1(nu)/s                               
c         a22 :      "      p2(mu)*p2(nu)/s                               
c         a12 :      "      (p1(mu)*p2(nu)+p2(mu)*p1(nu))/s               
c ------------------------------------------------------------------------
c (c) German Rodrigo 2000                                                 
c ------------------------------------------------------------------------
      subroutine LeptonicTensor1ISR(qq,cosphoton,Leptonic)
      include 'phokhara_10.0.inc' 
      complex*16 Leptonic(0:3,0:3),dggvap,BW_om,vacpol_and_nr,
     1 Leptonics(0:3,0:3)
      real*8 qq,cosphoton,aa_phi,mm_ph,gg_ph,
     &  a00,a11,a22,a12,am1,dps,metric,vac_qq
      real*8 a00s,a11s,a22s,a12s,am1s
      real*8 soft
      integer mu,nu
      common/param_PFF/aa_phi,mm_ph,gg_ph
      common/subtract_soft/a00s,a11s,a22s,a12s,am1s

      dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
      vac_qq = cdabs(vacpol_and_nr(qq))**2 

c --- ISR ---
      call Bornvirtualsoft(qq,cosphoton,a00,a11,a22,a12,am1)
      do mu = 0,3
         do nu = 0,3
           Leptonic(mu,nu) = dcmplx((a00*metric(mu,nu)+
     &        a11*momenta(1,mu)*momenta(1,nu)/Sp+
     &        a22*momenta(2,mu)*momenta(2,nu)/Sp+
     &        a12*(momenta(1,mu)*momenta(2,nu)+
     &             momenta(2,mu)*momenta(1,nu))/Sp),
     &         pi*am1/Sp*(momenta(1,mu)*momenta(2,nu)-
     &             momenta(2,mu)*momenta(1,nu)))*dps*vac_qq
         enddo
      enddo
      if(nlo2.eq.1)then !ST soft subtraction
      soft=alpha/pi*(
     1 (-log(4.d0*w*w))*(1.d0+log(me**2/Sp))
     1 -log(me**2/Sp)**2/2.d0-log(me**2/Sp)-pi**2/3.d0)
c      print*, 'soft=',soft
      do mu = 0,3
         do nu = 0,3
      Leptonics(mu,nu) = dcmplx((a00s*metric(mu,nu)+
     &        a11s*momenta(1,mu)*momenta(1,nu)/Sp+
     &        a22s*momenta(2,mu)*momenta(2,nu)/Sp+
     &        a12s*(momenta(1,mu)*momenta(2,nu)+
     &             momenta(2,mu)*momenta(1,nu))/Sp),
     &         pi*am1s/Sp*(momenta(1,mu)*momenta(2,nu)-
     &             momenta(2,mu)*momenta(1,nu)))*dps*vac_qq

       Leptonic(mu,nu)=Leptonic(mu,nu)-Leptonics(mu,nu)*soft
         enddo
      enddo
      endif
      return
      end
c **********************************************************************
c 
      subroutine Bornvirtualsoft(qq,cosphoton,a00,a11,a22,a12,am1)
      include 'phokhara_10.0.inc'
      real*8 qq,cosphoton,a00,a11,a22,a12,am1,app,m2,q2,uq2,
     1  b,x,y1,y2,globalfactor,a00NLO,a11NLO,a22NLO,a12NLO,api,
     &  t1,t2,t3,t4,t5,t6,t7,t8,t9,s1,t10,t11,t12,t13,t14,Ny1,Ny2,z,
     &  soft,coll,extramass
      real*8 a00s,a11s,a22s,a12s,am1s
      complex*16 cdilog,dcmplx
      common/subtract_soft/a00s,a11s,a22s,a12s,am1s
       
      m2 = me*me/Sp
      q2 = qq/Sp
      uq2 = 1.d0-q2
      b = dSqrt(1.d0-4.d0*m2)
      x = b*cosphoton

c         y1 = uq2*(1.d0-x)/2.d0
c         y2 = uq2*(1.d0+x)/2.d0

      if(x.lt.0.d0)then
         y1 = uq2*(1.d0-x)/2.d0
         y2 = uq2*onemb2c2/(1.d0-x)/2.d0
      else
         y1 = uq2*onemb2c2/(1.d0+x)/2.d0
         y2 = uq2*(1.d0+x)/2.d0
      endif
      globalfactor = (4.d0*pi*alpha/Sp)**2/(q2*q2)

c --- LO ---
      a00 = ( 2.d0*m2*Q2*uq2*uq2/(y1*y2)-
     &  (2.d0*q2+y1*y1+y2*y2) )/(y1*y2)
      a11 = (8.d0*m2/y2-4.d0*q2/y1)/y2
      a22 = (8.d0*m2/y1-4.d0*q2/y2)/y1
      a12 = -8.d0*m2/(y1*y2)
      am1 = 0.d0

      if(nlo2.eq.1)then !ST part for subtract soft emission
            a00s=a00
            a11s=a11
            a22s=a22
            a12s=a12
            am1s=am1
      endif

c --- NLO ---
      if (nlo.ne.0) then    
         api = alpha/pi      
         t1 = dLog(m2)
         t2 = dLog(m2/q2)      
         t3 = pi*pi/3.d0
         t4 = dLog(y1/q2)
         t5 = dLog(y2/q2)
         t6 = dLog(q2)
         t7 = dreal(cdilog(dcmplx(1.d0-1.d0/q2)))
         t8 = dreal(cdilog(dcmplx(-y1/q2)))-t7+dLog(q2+y1)*dLog(y1/q2)
         t9 = dreal(cdilog(dcmplx(-y2/q2)))-t7+dLog(q2+y2)*dLog(y2/q2) 
         s1 = y1*y1+y2*y2      
         t10 = dLog(y1/m2)
	 t11 = dLog(y2/m2)
         t12 = dreal(cdilog(dcmplx(1.d0-y1/m2)))
	 t13 = dreal(cdilog(dcmplx(1.d0-y2/m2)))
         t14 = dreal(cdilog(dcmplx(1.d0-q2)))
         Ny1 = t6*t10+t14+t12-t3/2.d0
         Ny2 = t6*t11+t14+t13-t3/2.d0	 

      a00NLO = ( -q2*uq2/2.d0-y1*y2+
     &  y1/2.d0*(4.d0-y1-3.d0*(1.d0+q2)/(1.d0-y2))*t4+
     &  y2/2.d0*(4.d0-y2-3.d0*(1.d0+q2)/(1.d0-y1))*t5-
     &  (q2+2.d0*y1*y2/uq2)*t6-
     &  (1.d0+(1.d0-y2)**2+y1*q2/y2)*t8-
     &  (1.d0+(1.d0-y1)**2+y2*q2/y1)*t9 )/(y1*y2)

      a11NLO = ( (1.d0+q2)**2*(1.d0/(1.d0-y1)-1.d0/uq2)-
     &  4.d0*(1.d0-y2)*y1/uq2-q2*(1.d0+2.d0/y2)*t4-
     &  q2*(2.d0-3.d0*y1)*(1.d0-y2)**2/(y1*(1.d0-y1)**2)*t5-
     &  2.d0*q2/uq2*((1.d0-y2)*(1.d0/y2+q2/y1+2.d0*y1/uq2)+
     &  2.d0*q2/uq2)*t6-2.d0*q2*(1.d0+1.d0/(y2*y2))*t8-
     &  2.d0*q2*(3.d0+2.d0*q2/y1+q2*q2/(y1*y1))*t9 )/(y1*y2)

      a22NLO = ( (1.d0+q2)**2*(1.d0/(1.d0-y2)-1.d0/uq2)-
     &  4.d0*(1.d0-y1)*y2/uq2-q2*(1.d0+2.d0/y1)*t5-
     &  q2*(2.d0-3.d0*y2)*(1.d0-y1)**2/(y2*(1.d0-y2)**2)*t4-
     &  2.d0*q2/uq2*((1.d0-y1)*(1.d0/y1+q2/y2+2.d0*y2/uq2)+
     &  2.d0*q2/uq2)*t6-2.d0*q2*(1.d0+1.d0/(y1*y1))*t9-
     &  2.d0*q2*(3.d0+2.d0*q2/y2+q2*q2/(y2*y2))*t8 )/(y1*y2)

      a12NLO = ( q2/(1.d0-y1)+q2/(1.d0-y2)-(4.d0*q2+(y1-y2)**2)/uq2-
     &  2.d0*q2/(1.d0-y2)*(1.d0-y1+q2/y2-q2/(2.d0*(1.d0-y2)))*t4-
     &  2.d0*q2/(1.d0-y1)*(1.d0-y2+q2/y1-q2/(2.d0*(1.d0-y1)))*t5-
     &  2.d0*q2*(q2/(y1*y2)+(1.d0+q2-2.d0*y1*y2)/(uq2*uq2))*t6-
     &  2.d0*q2*(1.d0+q2/y2+q2/(y2*y2))*t8-
     &  2.d0*q2*(1.d0+q2/y1+q2/(y1*y1))*t9 )/(y1*y2) 

      am1 = q2*(2.d0*dLog(1.d0-y1)/y1+(1.d0-q2)/(1.d0-y1)+
     &  q2/(1.d0-y1)**2-2.d0*dLog(1.d0-y2)/y2-(1.d0-q2)/(1.d0-y2)-
     &  q2/(1.d0-y2)**2)/(y1*y2) 

c --- NLO mass corrections       
      z = (1.d0-3.d0*q2)/q2
      app = m2 * ( - q2*(t6*dLog(y1**4/(m2*m2*q2))+
     &  4.d0*t14+t12-t3/2.d0) - uq2*(1.d0-t10+
     &  (m2/y1)*(t12-t3/2.d0)) )/(y1*y1)+ q2*extramass(m2,y1,z)/2.d0
     &    + m2 * ( - q2*(t6*dLog(y2**4/(m2*m2*q2))+
     &  4.d0*t14+t13-t3/2.d0) - uq2*(1.d0-t11+
     &  (m2/y2)*(t13-t3/2.d0)) )/(y2*y2)+ q2*extramass(m2,y2,z)/2.d0

      a00NLO = a00NLO - app 
c      a11NLO = a11NLO - 4.d0*q2*app/uq2**2 
c      a22NLO = a22NLO - 4.d0*q2*app/uq2**2
c      a12NLO = a12NLO - 4.d0*q2*app/uq2**2

      z = (3.d0-8.d0*q2+6.d0*q2*q2)/(1.d0-2.d0*q2*q2)
      a11NLO = a11NLO + q2/uq2 * ( 
     &  4.d0*m2/(y1*y1) * (1.d0-t10+(m2/y1)*(t12-t3/2.d0)) - 
     &  extramass(m2,y1,1.d0)+2.d0*m2*q2/(y1*(m2*uq2-y1))*
     &  (t10/q2+t6/uq2+(1.d0+m2/(m2*uq2-y1))*Ny1)) + 
     &  1.d0/uq2 * ( 4.d0*m2*uq2/(y2*y2)* 
     &  (t6*dLog(y2**4/(m2*m2*q2))+4.d0*t14+2.d0*t13-t3)+ 
     &  4.d0*m2*q2/(y2*y2)*(1.d0-t11+(1.d0+m2/y2)*(t13-t3/2.d0))-
     &  (1.d0-2.d0*q2*q2)/q2*extramass(m2,y2,z)+
     &  2.d0*m2/(y2*(m2*uq2-y2))*
     &  (t11/q2+t6/uq2+(3.d0+m2/(m2*uq2-y2))*Ny2))

      a22NLO = a22NLO + q2/uq2 * ( 
     &  4.d0*m2/(y2*y2) * (1.d0-t11+(m2/y2)*(t13-t3/2.d0)) - 
     &  extramass(m2,y2,1.d0)+2.d0*m2*q2/(y2*(m2*uq2-y2))*
     &  (t11/q2+t6/uq2+(1.d0+m2/(m2*uq2-y2))*Ny2)) + 
     &  1.d0/uq2 * ( 4.d0*m2*uq2/(y1*y1)* 
     &  (t6*dLog(y1**4/(m2*m2*q2))+4.d0*t14+2.d0*t12-t3)+ 
     &  4.d0*m2*q2/(y1*y1)*(1.d0-t10+(1.d0+m2/y1)*(t12-t3/2.d0))-
     &  (1.d0-2.d0*q2*q2)/q2*extramass(m2,y1,z)+
     &  2.d0*m2/(y1*(m2*uq2-y1))*
     &  (t10/q2+t6/uq2+(3.d0+m2/(m2*uq2-y1))*Ny1))

      z = 1.d0/uq2
      a12NLO = a12NLO + q2/uq2 * ( 
     &  4.d0*m2/(y1*y1)*(1.d0-t10+(.5d0+m2/y1)*(t12-t3/2.d0))-
     &  uq2/q2*extramass(m2,y1,z)+2.d0*m2/(y1*(m2*uq2-y1))*
     &  (t10/q2+t6/uq2+(2.d0+m2/(m2*uq2-y1))*Ny1) +
     &  4.d0*m2/(y2*y2)*(1.d0-t11+(.5d0+m2/y2)*(t13-t3/2.d0))-
     &  uq2/q2*extramass(m2,y2,z)+2.d0*m2/(y2*(m2*uq2-y2))*
     &  (t11/q2+t6/uq2+(2.d0+m2/(m2*uq2-y2))*Ny2) )

c --- soft and collinear logs 
      soft = -dLog(4.d0*w*w)*(1.d0+t1)
      coll = (-1.5d0*t2-2.d0+t3)

c --- final result
      a00 = a00 + api*(a00*(soft+coll)+a00NLO)!-a00
      a11 = a11 + api*(a11*(soft+coll)+a11NLO)!-a11
      a22 = a22 + api*(a22*(soft+coll)+a22NLO)!-a22
      a12 = a12 + api*(a12*(soft+coll)+a12NLO)!-a12                   
      am1 = api*am1
      endif 
c soft test

c      a00 = a00+api*a00*soft!a00 + api*(a00*(soft+coll)+a00NLO)
c      a11 = a11+api*a11*soft!a11 + api*(a11*(soft+coll)+a11NLO)
c      a22 = a22+api*a22*soft!a22 + api*(a22*(soft+coll)+a22NLO)
c      a12 = a12+api*a12*soft!a12 + api*(a12*(soft+coll)+a12NLO)                   
c      am1 = 0.d0!api*a11*soft!api*am1



      a00 = globalfactor * a00
      a11 = globalfactor * a11
      a22 = globalfactor * a22
      a12 = globalfactor * a12
      am1 = globalfactor * am1

      if(nlo2.eq.1)then !ST part for subtract soft emission
           a00s = globalfactor * a00s
           a11s = globalfactor * a11s
           a22s = globalfactor * a22s
           a12s = globalfactor * a12s
           am1s = globalfactor * am1s
      endif

      return 
      end

c **********************************************************************
      real*8 function extramass(m2,y,z)
      implicit none
      real*8 m2,y,z
      integer n
      extramass = (1.d0+z*dLog(y/m2))/y
      if ((m2-y)**2.le.m2*m2/4.d0) then 
         do n = 0,4
            extramass = extramass - (1.d0/(dble(n)+2.d0)+
     &         z/(dble(n)+1.d0))*(m2-y)**n/(m2**(n+1))
         enddo
      else
         extramass = extramass + (1.d0+z*dLog(y/m2))/(m2-y) +
     &      m2*dLog(y/m2)/(m2-y)**2
      endif
      end
c ============================================================================
c     Hadronic Tensor for the process                                     
c                                                                         
c     photon^*(Q2) -----> mu^+ mu^-, pi^+ pi^-, 2pi^0 pi^+ pi^-, 2pi^+ 2pi^-, 
c                         p pbar, n nbar, K^+ K^-, K^0 K^0bar, pi^+ pi^- pi^0,
c                         lambda lambdabar
c ----------------------------------------------------------------------------
      subroutine HadronicTensorISR(qq,q0p,q2p,q0b,q2b,Hadronic)
      include 'phokhara_10.0.inc'       
      real*8 qq,pionFF,q0p,q2p,q0b,q2b,metric,dps,q1(4),q2(4),
     1                 q3(4),q4(4),rlam,a,b,c,mmu1,bb,
     2 qqvec(0:3)
      complex*16 Hadronic(0:3,0:3),hadr(4),hadr_3pi(4),PionFormFactor,
     1           KaonFormFactor,pionggFF_dprime,etaggFF_dprime2,
     2 etaPggFF_dprime2
      real*8 protGE2,protGM2,tau_pp,Columb_F
      real*8 lambGE2,lambGM2,tau_ll,t1_ll,aa_ll,qqp_ll(0:3),qqm_ll(0:3)
      real*8 mm_phi_lam,mm_om_lam,mu_lamb,phase_f2_lamb,qqsqpi0
      integer mu,nu,i,j,bnu
      complex*16 protF1,protF2,GM_pp,GE_pp,lambF1,lambF2,GM_ll,GE_ll
      common/lambF1F2/lambF1,lambF2,tau_ll,GM_ll,GE_ll,lambGM2,lambGE2,
     1                t1_ll,aa_ll,qqp_ll,qqm_ll
      common/lambparam/mm_phi_lam,mm_om_lam,mu_lamb,phase_f2_lamb
c
      rlam(a,b,c) = sqrt( (1.d0-(b+c)**2/a) * (1.d0-(b-c)**2/a) )
c
c --- muons ---      
      if (pion.eq.0)  then 
      pionFF = 16.d0*pi*alpha
      dps =  dSqrt(1.d0-4.d0*mmu*mmu/qq)/(32.d0*pi*pi)  ! Phase space factors
      do mu = 0,3
         do nu = 0,3
           Hadronic(mu,nu) = pionFF*(momenta(6,mu)*momenta(7,nu)+
     &        momenta(7,mu)*momenta(6,nu)-qq/2.d0*metric(mu,nu))*dps
         enddo
      enddo
     
c --- pions pi^+ pi^- ---        
      elseif(pion.eq.1)then
      
      pionFF = 4.d0*pi*alpha*cdabs (PionFormFactor(qq))**2
      dps =  dSqrt(1.d0-4.d0*mpi*mpi/qq)/(32.d0*pi*pi)  ! Phase space factors
      do mu = 0,3
         do nu = 0,3
            Hadronic(mu,nu) = pionFF*(momenta(6,mu)-momenta(7,mu))*
     &         (momenta(6,nu)-momenta(7,nu))*dps 
         enddo
      enddo
c
c --- pions 2pi^0 pi^+ pi^- --- pion=2
c --- pions 2pi^+ 2pi^-     --- pion=3
c        
      elseif((pion.eq.2).or.(pion.eq.3))then
c
      do i =0,3
       q1(i+1) = momenta(6,i)  
       q2(i+1) = momenta(7,i) 
       q3(i+1) = momenta(8,i)  
       q4(i+1) = momenta(9,i) 
      enddo
c
      pionFF = 4.d0*pi*alpha
c
c          Phase space factors
c
      dps = 1.d0/(2.d0*pi)**2/(32.d0*pi**2)**3
     1     * rlam(qq,q0p,rmass(1)) 
     2     * rlam(q2p,q0b,rmass(2)) 
     3     * rlam(q2b,rmass(3),rmass(4))
c
c --- pions 2pi^0 pi^+ pi^-
c
       if(pion.eq.2)then
        dps = dps * 0.5d0
c
        call had3(qq,q1,q2,q3,q4,hadr)
       endif
c
c --- pions 2pi^+ 2pi^-  
c
       if(pion.eq.3)then
        dps = dps * 0.25d0
c
        call had2(qq,q1,q2,q3,q4,hadr)
       endif
c
      do mu = 0,3
         do nu = 0,3
           Hadronic(mu,nu) = pionFF*hadr(mu+1)*conjg(hadr(nu+1))*dps 
         enddo
      enddo
c
c----- PPbar (pion=4) and  NNbar (pion=5) ----------------
      elseif ((pion.eq.4).or.(pion.eq.5))  then 
      if(FF_pp.eq.0)then
      call  ProtonFormFactor(qq,protF1,protF2)
      elseif(FF_pp.eq.1)then
       call  ProtonFormFactor_new(qq,protF1,protF2)
      endif
      if (pion.eq.4) mmu1 = mp
      if (pion.eq.5) mmu1 = mnt

      tau_pp = qq/4.D0/mmu1**2
      protGM2=cdabs(protF1+protF2)**2
      protGE2=cdabs(protF1+tau_pp*protF2)**2

      pionFF = 4.d0*pi*alpha
      dps =  dSqrt(1.d0-4.d0*mmu1**2/qq)/(32.d0*pi*pi)  ! Phase space factors
      do mu = 0,3
         do nu = 0,3
          Hadronic(mu,nu) = pionFF*dps*2.D0*(
     &      (momenta(7,mu)-momenta(6,mu))*(momenta(7,nu)-momenta(6,nu))
     &     *( protGE2 - protGM2 )/(tau_pp - 1.D0)
     &     -(momenta(7,mu)-momenta(6,mu))*(momenta(7,nu)-momenta(6,nu))
     &     *protGM2 - metric(mu,nu)*protGM2*qq 
     &     +(momenta(7,mu)+momenta(6,mu))*(momenta(7,nu)+momenta(6,nu))
     &     *protGM2 )
         enddo
      enddo

c including Coulumb Factor
        if((pion.eq.4))then
            bb=pi*alpha/sqrt(abs(1.d0-4.d0*mp**2/qq))
           do mu=0,3
             do nu=0,3
              Hadronic(mu,nu)=Hadronic(mu,nu)*bb/(1.d0-exp(-bb))
          enddo
            enddo
          endif 

c
c ---- kaons K^+ K^- --------------
      elseif(pion.eq.6)then
      
      pionFF = 4.d0*pi*alpha*cdabs(KaonFormFactor(qq))**2
      dps =  dSqrt(1.d0-4.d0*mKp*mKp/qq)/(32.d0*pi*pi)  ! Phase space factors
      do mu = 0,3
         do nu = 0,3
            Hadronic(mu,nu) = pionFF*(momenta(6,mu)-momenta(7,mu))*
     &         (momenta(6,nu)-momenta(7,nu))*dps 
         enddo
      enddo
c
c ---- kaons K^0 K^0bar -----------
      elseif(pion.eq.7)then
      
      pionFF = 4.d0*pi*alpha*cdabs(KaonFormFactor(qq))**2
      dps =  dSqrt(1.d0-4.d0*mKn*mKn/qq)/(32.d0*pi*pi)  ! Phase space factors
      do mu = 0,3
         do nu = 0,3
            Hadronic(mu,nu) = pionFF*(momenta(6,mu)-momenta(7,mu))*
     &         (momenta(6,nu)-momenta(7,nu))*dps 
         enddo
      enddo
c
c ---- 3 pions : pi^+ pi^- pi^0 or pi^+ pi^- eta --------------
      elseif((pion.eq.8).or.(pion.eq.10))then

      do i =0,3
       q1(i+1) = momenta(6,i)  
       q2(i+1) = momenta(7,i) 
       q3(i+1) = momenta(8,i)  
      enddo
c
c        Phase space factors
c
      dps = 1.d0/(2.d0*pi)/(32.d0*pi**2)**2
     1    * rlam(qq,q0p,rmass(1)) * rlam(q2p,rmass(2),rmass(3))

       pionFF = 4.d0*pi*alpha
c       print*,'------------------------------------'
c       print*,'oo',rlam(q2p,rmass(2),rmass(3)),qq,q2p,rmass(2),rmass(3)
       if(pion.eq.8)then
         call had_3pi(qq,q1,q2,q3,hadr_3pi,8)
       else
         call had_3pi(qq,q1,q2,q3,hadr_3pi,10)
       endif
      do mu = 0,3
         do nu = 0,3
           Hadronic(mu,nu) = pionFF * hadr_3pi(mu+1)
     1                     * conjg(hadr_3pi(nu+1)) * dps 
         enddo
      enddo
c
c----- Lambda (Lambda bar) pion=9  ----------------
      elseif (pion.eq.9)  then 

      call  LambdaFormFactor(qq,tau_ll,lambF1,lambF2)

      GM_ll = lambF1+lambF2
      GE_ll = ( lambF1+tau_ll*lambF2 ) 
     1      * exp(dcmplx(0.d0,phase_f2_lamb*pi/180.d0))
      lambGM2=cdabs(GM_ll)**2
      lambGE2=cdabs(GE_ll)**2
      t1_ll = tau_ll - 1.D0
      aa_ll = - 2.d0/t1_ll* (tau_ll * lambGM2 - lambGE2)
      do i=0,3
         qqp_ll(i) = momenta(6,i)+momenta(7,i)
         qqm_ll(i) = (momenta(7,i)-momenta(6,i))/2.d0
      enddo

      do mu = 0,3
         do nu = 0,3
          Hadronic(mu,nu) = 2.d0* lambGM2* (qqp_ll(mu) * qqp_ll(nu) 
     1    - metric(mu,nu)*qq) + 4.d0 * aa_ll * qqm_ll(mu)*qqm_ll(nu) 
         enddo
      enddo
c ----   pi0 gamma pion=13 ---------------------------
      elseif(pion.eq.13)then
c
        
       do nu=0,3
        qqvec(nu)=momenta(6,nu)+momenta(7,nu)
       enddo

        
      pionFF=(4.d0*pi*alpha)**2*cdabs(pionggFF_dprime(qq,0.d0))**2
      dps =  (1.d0-mpi0f**2/qq)/(32.d0*pi*pi)  ! Phase space factors
      do nu = 0,3
         do bnu = 0,3
       Hadronic(nu,bnu)=-pionFF*( 1.D0/4.D0*metric(nu,bnu)*qq**2-
     &1.D0/2.D0*metric(nu,bnu)*mpi0f**2*qq 
     &  + 1.D0/4.D0*metric(nu,bnu)*mpi0f**4 - 1.D0/2.D0*
     &    momenta(7,nu)*qqvec(bnu)*qq + 1.D0/2.D0*momenta(7,nu)*
     &    qqvec(bnu)*mpi0f**2 + momenta(7,nu)*momenta(7,bnu)*qq-1.D0/2.
     &    D0*momenta(7,bnu)*qqvec(nu)*qq + 1.D0/2.D0*momenta(7,bnu)*
     &    qqvec(nu)*mpi0f**2 )*dps

         enddo
      enddo
     
c      print*, 'FF=',qq,cdabs(-qq*pionggFF_dprime(-qq,0.d0))



      elseif(pion.eq.14)then
c
        
       do nu=0,3
        qqvec(nu)=momenta(6,nu)+momenta(7,nu)
       enddo

        
      pionFF=(4.d0*pi*alpha)**2*cdabs(etaggFF_dprime2(qq,0.d0))**2
      dps =  (1.d0-metaf**2/qq)/(32.d0*pi*pi)  ! Phase space factors
      do nu = 0,3
         do bnu = 0,3
       Hadronic(nu,bnu)=-pionFF*( 1.D0/4.D0*metric(nu,bnu)*qq**2-
     &1.D0/2.D0*metric(nu,bnu)*metaf**2*qq 
     &  + 1.D0/4.D0*metric(nu,bnu)*metaf**4 - 1.D0/2.D0*
     &    momenta(7,nu)*qqvec(bnu)*qq + 1.D0/2.D0*momenta(7,nu)*
     &   qqvec(bnu)*metaf**2 + momenta(7,nu)*momenta(7,bnu)*qq - 1.D0/2.
     &    D0*momenta(7,bnu)*qqvec(nu)*qq + 1.D0/2.D0*momenta(7,bnu)*
     &    qqvec(nu)*metaf**2 )*dps

         enddo
      enddo
c      print*, 'FF=',qq,cdabs(-qq*etaggFF_dprime2(-qq,0.d0)) 


      elseif(pion.eq.15)then
c
        
       do nu=0,3
        qqvec(nu)=momenta(6,nu)+momenta(7,nu)
       enddo

        
      pionFF=(4.d0*pi*alpha)**2*cdabs(etaPggFF_dprime2(qq,0.d0))**2
      dps =  (1.d0-metaP**2/qq)/(32.d0*pi*pi)  ! Phase space factors
      do nu = 0,3
         do bnu = 0,3
       Hadronic(nu,bnu)=-pionFF*( 1.D0/4.D0*metric(nu,bnu)*qq**2-
     &1.D0/2.D0*metric(nu,bnu)*metaP**2*qq 
     &  + 1.D0/4.D0*metric(nu,bnu)*metaP**4 - 1.D0/2.D0*
     &    momenta(7,nu)*qqvec(bnu)*qq + 1.D0/2.D0*momenta(7,nu)*
     &   qqvec(bnu)*metaP**2 + momenta(7,nu)*momenta(7,bnu)*qq - 1.D0/2.
     &    D0*momenta(7,bnu)*qqvec(nu)*qq + 1.D0/2.D0*momenta(7,bnu)*
     &    qqvec(nu)*metaP**2 )*dps
         enddo
      enddo
c      print*, 'FF=',qq,cdabs(-qq*etaPggFF_dprime2(-qq,0.d0))

      else
       continue
      endif

       
c
      return
      end
c ----------------------------------------------------------------------------
      subroutine HadroLamb(qq,Hadronic,HadronicLamb)
      include 'phokhara_10.0.inc'       
      real*8 qq,metric
      real*8 lambGE2,lambGM2,tau_ll,t1_ll,veclamb1(0:3),veclamb1_d(0:3),
     1       veclamb2(0:3),veclamb2_d(0:3),qqp_ll(0:3),qqp_d_ll(0:3),
     2       qqm_ll(0:3),qqm_d_ll(0:3),aa_ll,alS1pS2_ll(0:3),
     3       veceps_qp_qm_S_ll(0:3),tenseps_qp_qm_ll(0:3,0:3)
      real*8 qqpS1_ll,qqpS2_ll,S1S2_ll,scal_qS1qS2_ll
      complex*16 HadronicLamb(0:3,0:3),lambF1,lambF2,Hadronic(0:3,0:3),
     1           GM_ll,GE_ll,lambGMGEc,lambTT,lambGMmGE2,iiM_ll
      integer mu,nu,jj
      common/veclamb/veclamb1,veclamb2
      common/skalLamb/qqpS1_ll,qqpS2_ll,S1S2_ll
      common/lambF1F2/lambF1,lambF2,tau_ll,GM_ll,GE_ll,lambGM2,lambGE2,
     1                t1_ll,aa_ll,qqp_ll,qqm_ll

      call skalarLamb()
      
      lambGMGEc = GM_ll*conjg(GE_ll)
      lambGMmGE2= cdabs(GM_ll-GE_ll)**2 / mlamb**2/t1_ll**2
      lambTT = (tau_ll*lambGM2 - lambGMGEc)/t1_ll
      iiM_ll = (0.d0,1.d0) / mlamb / t1_ll 

      scal_qS1qS2_ll = - lambGM2 * 0.5d0 
     1                 * alpha_lamb * ( qqpS1_ll + qqpS2_ll ) 

      veclamb1_d(0) = veclamb1(0)
      veclamb2_d(0) = veclamb2(0)
      qqp_d_ll(0)   = qqp_ll(0)
      qqm_d_ll(0)   = qqm_ll(0)
      alS1pS2_ll(0) = alpha_lamb*( veclamb1_d(0) - veclamb2_d(0) )
      do jj = 1,3
         veclamb1_d(jj) = - veclamb1(jj)
         veclamb2_d(jj) = - veclamb2(jj)
         qqp_d_ll(jj)   = - qqp_ll(jj)
         qqm_d_ll(jj)   = - qqm_ll(jj)
         alS1pS2_ll(jj) = alpha_lamb*( veclamb1_d(jj) - veclamb2_d(jj) )
      enddo 

      call Fcoeff_aabb_ll(qqp_d_ll,qqm_d_ll,alS1pS2_ll,scal_qS1qS2_ll,
     1                    veceps_qp_qm_S_ll,tenseps_qp_qm_ll)

      do mu = 0,3
         do nu = 0,3

          HadronicLamb(mu,nu) =   
     1    Hadronic(mu,nu) * 0.25d0  - iiM_ll 
     2 * ( - lambGMGEc        * veceps_qp_qm_S_ll(mu) * qqm_ll(nu)
     3     + conjg(lambGMGEc) * veceps_qp_qm_S_ll(nu) * qqm_ll(mu) 
     4     + tenseps_qp_qm_ll(mu,nu) ) 
     5 - alpha_lamb**2 
     2 * (  lambGM2 * 0.5d0 *( qqpS2_ll*(qqp_ll(mu)*veclamb1(nu)
     3 + qqp_ll(nu)*veclamb1(mu) ) + qqpS1_ll*( qqp_ll(mu)*veclamb2(nu)
     3 + qqp_ll(nu)*veclamb2(mu) ) - S1S2_ll* qqp_ll(mu)* qqp_ll(nu)
     4 - qq*( veclamb1(mu)*veclamb2(nu)+veclamb1(nu)*veclamb2(mu) ) )
     5 + lambGM2* ( 0.5d0*qq*S1S2_ll - qqpS1_ll*qqpS2_ll )*metric(mu,nu) 
     6 + ( lambGMmGE2 * qqpS1_ll * qqpS2_ll 
     7 - aa_ll*S1S2_ll ) * qqm_ll(mu)*qqm_ll(nu) + conjg(lambTT) 
     8 * ( qqpS2_ll * qqm_ll(mu)*veclamb1(nu) - qqpS1_ll * qqm_ll(mu) 
     9 * veclamb2(nu) ) +  lambTT * ( qqpS2_ll * qqm_ll(nu)
     1 * veclamb1(mu) - qqpS1_ll * qqm_ll(nu) * veclamb2(mu) ) )

         enddo
      enddo

      return
      end
c **********************************************************************
      subroutine Fcoeff_aabb_ll(qqp,qqm,alS1pS2,scal,vec,tens)
      implicit none
      real*8 qqp(0:3),qqm(0:3),alS1pS2(0:3),vec(0:3),tens(0:3,0:3),
     1       scal
      integer jj
      
c --- epsylon uper indeces ---------------------------------------------
c      epsyl(0,1,2,3) = 1.d0          epsyl(0,1,3,2) = -1.d0
c      epsyl(0,2,3,1) = 1.d0          epsyl(0,2,1,3) = -1.d0
c      epsyl(0,3,1,2) = 1.d0          epsyl(0,3,2,1) = -1.d0
c      epsyl(1,0,3,2) = 1.d0          epsyl(1,0,2,3) = -1.d0
c      epsyl(1,2,0,3) = 1.d0          epsyl(1,2,3,0) = -1.d0
c      epsyl(1,3,2,0) = 1.d0          epsyl(1,3,0,2) = -1.d0
c      epsyl(2,1,3,0) = 1.d0          epsyl(2,1,0,3) = -1.d0
c      epsyl(2,0,1,3) = 1.d0          epsyl(2,0,3,1) = -1.d0
c      epsyl(2,3,0,1) = 1.d0          epsyl(2,3,1,0) = -1.d0
c      epsyl(3,1,0,2) = 1.d0          epsyl(3,1,2,0) = -1.d0
c      epsyl(3,0,2,1) = 1.d0          epsyl(3,0,1,2) = -1.d0
c      epsyl(3,2,1,0) = 1.d0          epsyl(3,2,0,1) = -1.d0

      vec(0) = qqp(1)*qqm(3)*alS1pS2(2) + qqp(2)*qqm(1)*alS1pS2(3)
     1       + qqp(3)*qqm(2)*alS1pS2(1) - qqp(1)*qqm(2)*alS1pS2(3)
     2       - qqp(2)*qqm(3)*alS1pS2(1) - qqp(3)*qqm(1)*alS1pS2(2)

      vec(1) = qqp(0)*qqm(2)*alS1pS2(3) + qqp(2)*qqm(3)*alS1pS2(0)
     1       + qqp(3)*qqm(0)*alS1pS2(2) - qqp(0)*qqm(3)*alS1pS2(2)
     2       - qqp(2)*qqm(0)*alS1pS2(3) - qqp(3)*qqm(2)*alS1pS2(0)

      vec(2) = qqp(0)*qqm(3)*alS1pS2(1) + qqp(1)*qqm(0)*alS1pS2(3)
     1       + qqp(3)*qqm(1)*alS1pS2(0) - qqp(0)*qqm(1)*alS1pS2(3)
     2       - qqp(1)*qqm(3)*alS1pS2(0) - qqp(3)*qqm(0)*alS1pS2(1)

      vec(3) = qqp(0)*qqm(1)*alS1pS2(2) + qqp(1)*qqm(2)*alS1pS2(0)
     1       + qqp(2)*qqm(0)*alS1pS2(1) - qqp(0)*qqm(2)*alS1pS2(1)
     2       - qqp(1)*qqm(0)*alS1pS2(2) - qqp(2)*qqm(1)*alS1pS2(0)
       
      do jj = 0,3
         tens(jj,jj) = 0.d0
      enddo

      tens(0,1) = scal * ( qqp(2) * qqm(3) - qqp(2) * qqm(3) )
      tens(1,0) = - tens(0,1)

      tens(0,2) = scal * ( qqp(3) * qqm(1) - qqp(1) * qqm(3) )
      tens(2,0) = - tens(0,2)  

      tens(0,3) = scal * ( qqp(1) * qqm(2) - qqp(2) * qqm(1) )
      tens(3,0) = - tens(0,3)

      tens(1,2) = scal * ( qqp(0) * qqm(3) - qqp(3) * qqm(0) )
      tens(2,1) = - tens(1,2)

      tens(1,3) = scal * ( qqp(2) * qqm(0) - qqp(0) * qqm(2) )
      tens(3,1) = - tens(1,3)

      tens(2,3) = scal * ( qqp(0) * qqm(1) - qqp(1) * qqm(0) )
      tens(3,2) = - tens(2,3)

      return
      end
c **********************************************************************
      subroutine skalarLamb()
      include 'phokhara_10.0.inc'       

      real*8 veclamb1(0:3),veclamb2(0:3),qqp_ll(0:3)
      real*8 qqpS1_ll,qqpS2_ll,S1S2_ll
      integer i
      common /veclamb/ veclamb1,veclamb2
      common /skalLamb/ qqpS1_ll,qqpS2_ll,S1S2_ll

      do i=0,3
         qqp_ll(i) = momenta(6,i)+momenta(7,i)
      enddo
c ------
      qqpS1_ll  = qqp_ll(0) * veclamb1(0)
      qqpS2_ll  = qqp_ll(0) * veclamb2(0)
      S1S2_ll   = veclamb1(0) * veclamb2(0)
      do i=1,3
         qqpS1_ll  = qqpS1_ll  - qqp_ll(i) * veclamb1(i)
         qqpS2_ll  = qqpS2_ll  - qqp_ll(i) * veclamb2(i)
         S1S2_ll   = S1S2_ll   - veclamb1(i) * veclamb2(i)
      enddo
c ------
      return
      end
c **********************************************************************
      subroutine LambdaFormFactor(qq,tau_ll,lambF1,lambF2) 
      include 'phokhara_10.0.inc'       
      real*8 qq,tau_ll,mm_phi_lam2,mm_om_lam2,mm_om1_lam2,mm_om2_lam2,
     1       mm_phi1_lam2,mm_phi2_lam2,const_model_lamb
      real*8 mm_phi_lam,mm_om_lam,mu_lamb,phase_f2_lamb    
      complex*16 lambF1,lambF2,F1om_ll,F1phi_ll,F2om_ll,F2phi_ll
      common/lambparam/mm_phi_lam,mm_om_lam,mu_lamb,phase_f2_lamb

      const_model_lamb = 0.9d0

      mm_om_lam2  = mm_om_lam**2
      mm_om1_lam2 = mm_om_lam2 + 1.d0/const_model_lamb
      mm_om2_lam2 = mm_om_lam2 + 2.d0/const_model_lamb

      mm_phi_lam2  = mm_phi_lam**2
      mm_phi1_lam2 = mm_phi_lam2 + 1.d0/const_model_lamb   
      mm_phi2_lam2 = mm_phi_lam2 + 2.d0/const_model_lamb   

      tau_ll = qq/4.d0/mlamb**2

      F1om_ll  = mm_om_lam2 * mm_om1_lam2 /(mm_om_lam2-qq)
     1         / (mm_om1_lam2-qq)
      F1phi_ll = mm_phi_lam2 * mm_phi1_lam2 /(mm_phi_lam2-qq)
     1         / (mm_phi1_lam2-qq)

      F2om_ll  = mm_om_lam2 * mm_om1_lam2 * mm_om2_lam2
     1         /(mm_om_lam2-qq)/(mm_om1_lam2-qq)/(mm_om2_lam2-qq)

      F2phi_ll = mm_phi_lam2 * mm_phi1_lam2 * mm_phi2_lam2
     1         /(mm_phi_lam2-qq)/(mm_phi1_lam2-qq)/(mm_phi2_lam2-qq)

      lambF1 = ( F1om_ll - F1phi_ll ) / 3.d0
      lambF2 = - ( F2om_ll + mu_lamb*F2phi_ll ) / 3.d0

      return
      end
c **********************************************************************
      complex*16 function PionFormFactor(a)
      include 'phokhara_10.0.inc'       
      integer nnn
      double precision a         
      complex*16 BW,BW_rho,BW_GS,BW_om,c_sum,appr_sum,BW_GS_09,tail_sum
 
      c_sum = appr_sum(a)

      if(FF_Pion.eq.0)then     ! KS
      PionFormFactor = 
     1   c_0_pion*BW_rho(m_rho0_pion,g_rho0_pion,a,1)/
     2     (1.d0+c_om_pion)*
     1     (1.d0+c_om_pion*BW_rho(m_om0_pion,g_om0_pion,a,1)) 
     4 + c_1_pion*BW_rho(m_rho1_pion,g_rho1_pion,a,1) 
     3 + c_2_pion*BW_rho(m_rho2_pion,g_rho2_pion,a,1) 
     4 + c_3_pion*BW_rho(m_rho3_pion,g_rho3_pion,a,1) 
     5 + c_sum

      elseif(FF_Pion.eq.1)then ! old GS
      PionFormFactor = 
     1   c_0_pion*BW_GS(m_rho0_pion,g_rho0_pion,a,1)/
     2     (1.d0+c_om_pion)*
     1     (1.d0+c_om_pion*BW_rho(m_om0_pion,g_om0_pion,a,1)) 
     2 + c_1_pion*BW_GS(m_rho1_pion,g_rho1_pion,a,1) 
     3 + c_2_pion*BW_GS(m_rho2_pion,g_rho2_pion,a,1) 
     4 + c_3_pion*BW_rho(m_rho3_pion,g_rho3_pion,a,1) 
     5 + c_sum
      elseif(FF_Pion.eq.2)then ! new GS
c
       nnn = 1+(a-(2.d0*mpi)**2)
     1  /((11.d0)**2-(2.d0*mpi)**2)*1.d6
       PionFormFactor = (grid_ffpi(nnn+1)-grid_ffpi(nnn))
     1                /(grid_qq(nnn+1)-grid_qq(nnn)) * a
     2  + (grid_ffpi(nnn)*grid_qq(nnn+1)-grid_ffpi(nnn+1)*grid_qq(nnn))
     3    /(grid_qq(nnn+1)-grid_qq(nnn))

      else
        write(6,*)' Wrong pion FF switch'
      endif

c       PionFormFactor=dcmplx(1.d0,0.d0)
       
      return
      end
c **********************************************************************
c only for FF_pion = 2
c **********************************************************************
      complex*16 function PionFormFactor_ex(a)
      include 'phokhara_10.0.inc'       
      double precision a         
      complex*16 BW_om,BW_GS_09,tail_sum
 
      PionFormFactor_ex = 
     1   c_n_pionGS(0)*BW_GS_09(0,a)/
     2     ( 1.d0+par_pionGS(4)*eephi )*
     3     ( 1.d0+
     4   par_pionGS(4)*eephi*BW_om(par_pionGS(5),par_pionGS(6),a,1) ) 
     5  +( c_n_pionGS(1)+c_n_pionGS(2)+c_n_pionGS(3)+c_n_pionGS(4)
     5  +c_n_pionGS(5) )
     6    /( 1.d0+ee12*par_pionGS(16)+ ee23*par_pionGS(17)
     7      + ee34*par_pionGS(21)+ ee45*par_pionGS(25) )*(
     8      BW_GS_09(1,a)
     9   + ee12*par_pionGS(16) * BW_GS_09(2,a)
     1   + ee23*par_pionGS(17) * BW_GS_09(3,a)
     2   + ee34*par_pionGS(21) * BW_GS_09(4,a) 
     3   + ee45*par_pionGS(25) * BW_GS_09(5,a))
     4 + tail_sum(a)
      return
      end
cc **********************************************************************
      complex*16 function PionFormFactor_3pi(qq2)
      include 'phokhara_10.0.inc'       
      real*8 qq2,omm_PDG,omg_PDG,qq12_3pi,qq13_3pi,qq23_3pi
      complex*16 RR_3pi,HH_3pi,RRro_3pi
      common/qqij_3pi/qq12_3pi,qq13_3pi,qq23_3pi

c PDG 2004
       omm_PDG = 0.78259d0
       omg_PDG = 0.00849d0

       PionFormFactor_3pi = 
     1 ( aa_3pi * RR_3pi(omm0_3pi,omg0_3pi,qq2) 
     2  + bb_3pi * RR_3pi(phim0_3pi,phig0_3pi,qq2) ) *
     2    HH_3pi(rhom0m_3pi,rhog0m_3pi,
     3                      rhom0p_3pi,rhog0p_3pi,rhom0z_3pi,rhog0z_3pi)
     4 + dd_3pi * RR_3pi(omm2_3pi,omg2_3pi,qq2) * 
     5    HH_3pi(rhom2m_3pi,rhog2m_3pi,
     8                      rhom2p_3pi,rhog2p_3pi,rhom2z_3pi,rhog2z_3pi)
     5 + ee_3pi * RR_3pi(phim0_3pi,phig0_3pi,qq2) *
     4    HH_3pi(rhom1m_3pi,rhog1m_3pi,
     7                      rhom1p_3pi,rhog1p_3pi,rhom1z_3pi,rhog1z_3pi) 
     8 + gg_3pi 
     1  * (RRro_3pi(qq2,0.77609d0,0.14446d0)/0.77609d0**2
     2      -0.1d0*RRro_3pi(qq2,1.7d0,0.26d0)/1.7d0**2)
     9    *RR_3pi(omm_PDG,omg_PDG,qq12_3pi)/omm_PDG**2  
     6 + jj_3pi * RR_3pi(omm1_3pi,omg1_3pi,qq2) *
     2    HH_3pi(rhom0m_3pi,rhog0m_3pi,
     3                      rhom0p_3pi,rhog0p_3pi,rhom0z_3pi,rhog0z_3pi)
     6 + kk_3pi * RR_3pi(omm2_3pi,omg2_3pi,qq2) *
     2    HH_3pi(rhom0m_3pi,rhog0m_3pi,
     3                      rhom0p_3pi,rhog0p_3pi,rhom0z_3pi,rhog0z_3pi)
      return
      end
cc**********************************************************************
c      
c    pi^0 Form Factor
c-------------------------------------------------------------
      complex*16 function pionggFF_dprime(t1_a,t2_a)
      include 'phokhara_10.0.inc'
c      implicit none
c      include 'common.ekhara.inc.for'
      real*8 t1_a,t2_a,W2_a,par(20),A1,A2,Fsmv3Hv3
      complex*16 Drho, Domega, Dphi   ! function (q2t)
      complex*16 Drho2, Domega2, Dphi2   ! function (q2t); 2nd octet
      complex*16 Drho3, Domega3, Dphi3 
c      real*8 theta0, theta8, Cs_c, Cq_c   ! this is enough for ETA
      real*8 coef!,pi,Fpi
      real*8 SIGvFsmv2,SIGv2Fsmv2,SIGv3Fsmv2
      complex*16 ffdummy,cpom
      save
      
      coef = 1.d0 !4.d0 * pi**2 * feta!!!!! changed by HC

!coef stands for the normalization change:
![etaggFF( 0 , 0 , meta^2 ) == Nc/(12 pi^2 Feta)]
!                                   ==>>> [etaggFF( 0 , 0 , meta^2 ) == 1]
!
!  for fits:
      
      Fsmv3Hv3 = (3.d0/4.d0/pi/pi -4.d0*sqrt(2.d0)*hv1*Fsmv1
     &          *(1.d0+F_om*H_om+A1pi0*F_phi)
     & -4.d0*sqrt(2.d0)*Fsmv2Hv2
     &          *(2.d0+A2pi0)
     & )/4.d0/sqrt(2.d0)/(2.d0+A3pi0)
      SIGvFsmv2=dsqrt(2.d0)*Hv1*Fsmv1
      SIGv2Fsmv2=dsqrt(2.d0)*Fsmv2Hv2
      SIGv3Fsmv2=dsqrt(2.d0)*Fsmv3Hv3



       ffdummy =  coef *(
c     & (-1.d0,0.d0) / 4.d0 /pi**2 / Fchir
     & -3.d0/(12.d0*pi**2*Fchir)
     & + 4.d0*sqrt(2.d0)*Fsmv1*Hv1/3.d0/Fchir *t1_a
     & *(Drho(t1_a) + F_om*H_om*Domega(t1_a) 
     & +A1pi0*F_phi*Dphi(t1_a))
     & + 4.d0*sqrt(2.d0)*Fsmv1*Hv1/3.d0/Fchir *t2_a*(Drho(t2_a)
     & +F_om*H_om*Domega(t2_a)+A1pi0*F_phi*Dphi(t2_a))
     & - 4.d0*SIGvFsmv2/Fchir/3.d0 * t1_a*t2_a
     & *(   Drho(t1_a)* Domega(t2_a) +  Drho(t2_a)* Domega(t1_a)
     & +(A1pi0*F_phi-A_pi_phiom)*Dphi(t1_a)*Dphi(t2_a)
     &  + (F_om*H_om-1.d0-A_pi_phiom)*Domega(t2_a)*Domega(t1_a) 
     &  +A_pi_phiom*
     & (Domega(t1_a)*Dphi(t2_a)+Dphi(t1_a)*Domega(t2_a)))
     &   )
c       print*,'1',ffdummy
c      write(6,*)'ffdummy =',ffdummy
      ffdummy = ffdummy +coef*(
     & + 4.d0*sqrt(2.d0)*Fsmv2Hv2/3.d0/Fchir*t1_a
     & *(Drho2(t1_a) + Domega2(t1_a)+A2pi0*Dphi2(t1_a))
     & + 4.d0*sqrt(2.d0)*Fsmv2Hv2/3.d0/Fchir*t2_a
     & *(Drho2(t2_a) + Domega2(t2_a)+A2pi0*Dphi2(t2_a))
     & - 4.d0*SIGv2Fsmv2/Fchir/3.d0 * t1_a*t2_a
     & *(   Drho2(t1_a)* Domega2(t2_a) +  Drho2(t2_a)* Domega2(t1_a)
     &  +A2pi0*Dphi2(t1_a)*Dphi2(t2_a))
     &  )
c      write(6,*)'ffdummy =',ffdummy
c      print*,'2',ffdummy
      ffdummy = ffdummy +coef*(
     & + 4.d0*sqrt(2.d0)*Fsmv3Hv3/3.d0/Fchir*t1_a
     & *(Drho3(t1_a) + Domega3(t1_a)+A3pi0*Dphi3(t1_a))
     & + 4.d0*sqrt(2.d0)*Fsmv3Hv3/3.d0/Fchir*t2_a
     & *(Drho3(t2_a) + Domega3(t2_a)+A3pi0*Dphi3(t2_a))
     & - 4.d0*SIGv3Fsmv2/Fchir/3.d0 * t1_a*t2_a
     & *(   Drho3(t1_a)* Domega3(t2_a) +  Drho3(t2_a)* Domega3(t1_a)
     &  +A3pi0*Dphi3(t1_a)*Dphi3(t2_a))
     &  )
c      print*,'3',ffdummy
      pionggFF_dprime =ffdummy  !dcmplx(1.d0,0.d0)
      return
      end
cc**********************************************************************
c      
c    eta Form Factor
c-------------------------------------------------------------
      complex*16 function etaggFF_dprime2(t1_a,t2_a)
      include 'phokhara_10.0.inc'
      real*8 t1_a,t2_a,W2_a,par(20),Fsmv3Hv3,SIGv3Fsmv2
      complex*16 Drho, Domega, Dphi   ! function (q2t)
      complex*16 Drho2, Domega2, Dphi2   ! function (q2t) SECOND OCTET
      complex*16 Drho3, Domega3, Dphi3 
      real*8 coef
      real*8 SIGvFsmv2,SIGv2Fsmv2,A2eta,B1
      complex*16 ffdummy

      coef = 1.d0 !  HC 4.d0 * pi**2 * feta
      
!coef stands for the normalization change:
![etaggFF( 0 , 0 , meta^2 ) == Nc/(12 pi^2 Feta)]
!                                   ==>>> [etaggFF( 0 , 0 , meta^2 ) == 1]
!
!  for fits:
      
      

      Fsmv3Hv3 = (3.d0/4.d0/pi/pi -4.d0*sqrt(2.d0)*hv1*Fsmv1
     &          *(1.d0+F_om*H_om+A1pi0*F_phi)
     & -4.d0*sqrt(2.d0)*Fsmv2Hv2
     &          *(2.d0+A2pi0)
     & )/4.d0/sqrt(2.d0)/(2.d0+A3pi0)
      SIGvFsmv2=dsqrt(2.d0)*Hv1*Fsmv1
      SIGv2Fsmv2=dsqrt(2.d0)*Fsmv2Hv2
      SIGv3Fsmv2=dsqrt(2.d0)*Fsmv3Hv3

      A2eta= (3.d0/4.d0/pi/pi*(5.d0/3.d0*Cq_c-dsqrt(2.d0)/3.d0*Cs_c)
     1  -4.d0*dsqrt(2.d0)*Hv1*Fsmv1*((3.d0*Cq_c+Cq_c/3.d0*F_om
     3  -2.d0*dsqrt(2.d0)/3.d0*Cs_c*F_phi)+
     2 (5.d0/3.d0*Cq_c-dsqrt(2.d0)/3.d0*Cs_c)*A1eta*F_phi) !1
     4  -4.d0*dsqrt(2.d0)*Fsmv2Hv2*(10.d0/3.d0*Cq_c
     3  -2.d0*dsqrt(2.d0)/3.d0*Cs_c)
     3 -4.d0*dsqrt(2.d0)*Fsmv3Hv3*(10.d0/3.d0*Cq_c
     3  -2.d0*dsqrt(2.d0)/3.d0*Cs_c+
     4 (5.d0/3.d0*Cq_c-dsqrt(2.d0)/3.d0*Cs_c)*A3eta)) !2
     2 /(4.d0*dsqrt(2.d0)*Fsmv2Hv2*
     1 (5.d0/3.d0*Cq_c-dsqrt(2.d0)/3.d0*Cs_c))



c     &  )

      ffdummy =  coef *(
     & -3.d0/12.d0/pi/pi/Fchir
     & *(Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0)
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv1*Hv1/3.d0/Fchir * t1_a
     & *((3.d0*Cq_c* Drho(t1_a) + Cq_c*F_om*Domega(t1_a)/3.d0 
     &     - 2.d0*sqrt(2.d0)/3.d0 *F_phi* Cs_c* Dphi(t1_a))
     & +(Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0)*A1eta*F_phi
     & *Dphi(t1_a))
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv1*Hv1/3.d0/Fchir * t2_a
     & *((3.d0*Cq_c* Drho(t2_a) + Cq_c*F_om* Domega(t2_a)/3.d0 
     &     - 2.d0*sqrt(2.d0)/3.d0 *F_phi* Cs_c* Dphi(t2_a))
     & +(Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0)*A1eta*F_phi
     & *Dphi(t2_a))
     & -(1.d0,0.d0)
     & * 8.d0*SIGvFsmv2/Fchir * t1_a*t2_a
     & *(( Cq_c* Drho(t1_a)* Drho(t2_a)/2.d0 
     &   + Cq_c*F_om* Domega(t1_a)* Domega(t2_a)/18.d0
     &   - sqrt(2.d0)/9.d0*F_phi * Cs_c* Dphi(t1_a)* Dphi(t2_a))
     &   +(Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0)/6.d0
     &   *A1eta*F_phi*Dphi(t1_a)*Dphi(t2_a)
     &   -A_eta_phiom*Dphi(t1_a)*Dphi(t2_a)
     &   -A_eta_phiom*Domega(t1_a)*Domega(t2_a)
     &   +A_eta_phiom*(Dphi(t1_a)*Domega(t2_a) 
     &              + Dphi(t2_a)*Domega(t1_a)))
     &  )

      ffdummy = ffdummy +coef*(
     & (1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv2Hv2/3.d0/Fchir*t1_a
     & *(3.d0*Cq_c*Drho2(t1_a) 
     &   + Cq_c*Domega2(t1_a)/3.d0 
     &     - 2.d0*sqrt(2.d0)/3.d0 * Cs_c*Dphi2(t1_a)
     & +( Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0 )*A2eta*Dphi2(t1_a))
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv2Hv2/3.d0/Fchir * t2_a
     & *(3.d0*Cq_c* Drho2(t2_a) + Cq_c* Domega2(t2_a)/3.d0 
     &     - 2.d0*sqrt(2.d0)/3.d0 * Cs_c* Dphi2(t2_a)
     & +( Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0 )*A2eta*Dphi2(t2_a))
     & -(1.d0,0.d0)
     & * 8.d0*SIGv2Fsmv2/Fchir * t1_a*t2_a
     & *(  Cq_c* Drho2(t1_a)* Drho2(t2_a)/2.d0 
     &   + Cq_c* Domega2(t1_a)* Domega2(t2_a)/18.d0 
     &   - sqrt(2.d0)/9.d0 * Cs_c* Dphi2(t1_a)* Dphi2(t2_a)
     & +( Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0 )*A2eta/6.d0*
     & Dphi2(t1_a)* Dphi2(t2_a))
     &  )
   
      ffdummy = ffdummy +coef*(
     & (1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv3Hv3/3.d0/Fchir*t1_a
     & *(3.d0*Cq_c*Drho3(t1_a) 
     &   + Cq_c*Domega3(t1_a)/3.d0 
     &     - 2.d0*sqrt(2.d0)/3.d0 * Cs_c*Dphi3(t1_a)
     & +( Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0 )*A3eta*Dphi3(t1_a))
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv3Hv3/3.d0/Fchir * t2_a
     & *(3.d0*Cq_c* Drho3(t2_a) + Cq_c* Domega3(t2_a)/3.d0 
     &     - 2.d0*sqrt(2.d0)/3.d0 * Cs_c* Dphi3(t2_a)
     & +( Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0 )*A3eta*Dphi3(t2_a))
     & -(1.d0,0.d0)
     & * 8.d0*SIGv3Fsmv2/Fchir * t1_a*t2_a
     & *(  Cq_c* Drho3(t1_a)* Drho3(t2_a)/2.d0 
     &   + Cq_c* Domega3(t1_a)* Domega3(t2_a)/18.d0 
     &   - sqrt(2.d0)/9.d0 * Cs_c* Dphi3(t1_a)* Dphi3(t2_a)
     & +( Cq_c*5.d0/3.d0- Cs_c*sqrt(2.d0)/3.d0 )*A3eta/6.d0*
     & Dphi3(t1_a)* Dphi3(t2_a))
     &  )


      etaggFF_dprime2 = ffdummy
      return
      end
cc**********************************************************************
c      
c    etaP Form Factor
c-------------------------------------------------------------
      complex*16 function etaPggFF_dprime2(t1_a,t2_a)
      include 'phokhara_10.0.inc'
      real*8 t1_a,t2_a,W2_a,par(20),Fsmv3Hv3,SIGv3Fsmv2
      complex*16 Drho, Domega, Dphi   ! function (q2t)
      complex*16 Drho2, Domega2, Dphi2   ! function (q2t)
      complex*16 Drho3, Domega3, Dphi3
      real*8 coef!,pi,Fpi
      real*8 SIGvFsmv2,SIGv2Fsmv2,A2etaP
     1 ,B1
      complex*16 ffdummy
 
      coef = 1.d0 !  HC 4.d0 * pi**2 * feta

!coef stands for the normalization change:
![etaPggFF( 0 , 0 , meta'^2 ) == Nc/(12 pi^2 FetaP)]
!                                   ==>>> [etaPggFF( 0 , 0 , meta'^2 ) == 1]
!

      
      

      Fsmv3Hv3 = (3.d0/4.d0/pi/pi -4.d0*sqrt(2.d0)*hv1*Fsmv1
     &          *(1.d0+F_om*H_om+A1pi0*F_phi)
     & -4.d0*sqrt(2.d0)*Fsmv2Hv2
     &          *(2.d0+A2pi0)
     & )/4.d0/sqrt(2.d0)/(2.d0+A3pi0)
      SIGvFsmv2=dsqrt(2.d0)*Hv1*Fsmv1
      SIGv2Fsmv2=dsqrt(2.d0)*Fsmv2Hv2
      SIGv3Fsmv2=dsqrt(2.d0)*Fsmv3Hv3

       A2etaP= (3.d0/4.d0/pi/pi*(5.d0/3.d0*Cq_P+dsqrt(2.d0)/3.d0*Cs_P)
     1  -4.d0*dsqrt(2.d0)*Hv1*Fsmv1*((3.d0*Cq_P+Cq_P/3.d0*F_om
     3  +2.d0*dsqrt(2.d0)/3.d0*Cs_P*F_phi)+
     2 (5.d0/3.d0*Cq_P+dsqrt(2.d0)/3.d0*Cs_P)*A1etaP*F_phi) !1
     4  -4.d0*dsqrt(2.d0)*Fsmv2Hv2*(10.d0/3.d0*Cq_P
     3  +2.d0*dsqrt(2.d0)/3.d0*Cs_P)
     3 -4.d0*dsqrt(2.d0)*Fsmv3Hv3*(10.d0/3.d0*Cq_P
     3  +2.d0*dsqrt(2.d0)/3.d0*Cs_P+
     4 (Cq_P*5.d0/3.d0+Cs_P*sqrt(2.d0)/3.d0 )*A3etaP)) !2
     2 /(4.d0*dsqrt(2.d0)*Fsmv2Hv2*
     1 (5.d0/3.d0*Cq_P+dsqrt(2.d0)/3.d0*Cs_P))



      ffdummy =  coef *(
     & -3.d0/12.d0/pi/pi/Fchir
     & *(Cq_P*5.d0/3.d0+Cs_P*sqrt(2.d0)/3.d0)
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv1*Hv1/3.d0/Fchir * t1_a
     & *((3.d0*Cq_P* Drho(t1_a) + Cq_P*F_om* Domega(t1_a)/3.d0 
     &     + 2.d0*sqrt(2.d0)/3.d0 *F_phi* Cs_P* Dphi(t1_a))
     & +(Cq_P*5.d0/3.d0+ Cs_P*sqrt(2.d0)/3.d0)*A1etaP*F_phi
     & *Dphi(t1_a))
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv1*Hv1/3.d0/Fchir * t2_a
     & *((3.d0*Cq_P* Drho(t2_a) + Cq_P*F_om* Domega(t2_a)/3.d0
     &     + 2.d0*sqrt(2.d0)/3.d0 *F_phi* Cs_P* Dphi(t2_a))
     & +(Cq_P*5.d0/3.d0+ Cs_P*sqrt(2.d0)/3.d0)*A1etaP*F_phi
     & *Dphi(t2_a))
     & -(1.d0,0.d0)
     & * 8.d0*SIGvFsmv2/Fchir * t1_a*t2_a
     & *(( Cq_P* Drho(t1_a)* Drho(t2_a)/2.d0 
     &   + Cq_P*F_om* Domega(t1_a)* Domega(t2_a)/18.d0
     &   + sqrt(2.d0)/9.d0 * Cs_P*F_phi*Dphi(t1_a)* Dphi(t2_a))
     &   +(Cq_P*5.d0/3.d0+ Cs_P*sqrt(2.d0)/3.d0)/6.d0
     &   *F_phi*A1etaP*Dphi(t1_a)*Dphi(t2_a))
     &  )

       ffdummy = ffdummy +coef*(
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv2Hv2/3.d0/Fchir*t1_a 
     & *(3.d0*Cq_P*Drho2(t1_a) 
     &      + Cq_P* Domega2(t1_a)/3.d0 
     &     + 2.d0*sqrt(2.d0)/3.d0 * Cs_P*Dphi2(t1_a)
     & +( Cq_P*5.d0/3.d0+Cs_P*sqrt(2.d0)/3.d0 )*A2etaP*Dphi2(t1_a))
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv2Hv2/3.d0/Fchir * t2_a
     & *(3.d0*Cq_P* Drho2(t2_a) + Cq_P* Domega2(t2_a)/3.d0 
     &     + 2.d0*sqrt(2.d0)/3.d0 * Cs_P* Dphi2(t2_a)
     & +( Cq_P*5.d0/3.d0+Cs_P*sqrt(2.d0)/3.d0 )*A2etaP*Dphi2(t2_a))
     & -(1.d0,0.d0)
     & * 8.d0*SIGv2Fsmv2/Fchir * t1_a*t2_a
     & *(  Cq_P* Drho2(t1_a)* Drho2(t2_a)/2.d0 
     &   + Cq_P* Domega2(t1_a)* Domega2(t2_a)/18.d0 
     &   + sqrt(2.d0)/9.d0 * Cs_P* Dphi2(t1_a)* Dphi2(t2_a)
     & +( Cq_P*5.d0/3.d0+Cs_P*sqrt(2.d0)/3.d0 )*A2etaP/6.d0
     & *Dphi2(t1_a)*Dphi2(t2_a))
     &  )

      ffdummy = ffdummy +coef*(
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv3Hv3/3.d0/Fchir*t1_a 
     & *(3.d0*Cq_P*Drho3(t1_a) 
     &      + Cq_P* Domega3(t1_a)/3.d0 
     &     + 2.d0*sqrt(2.d0)/3.d0 * Cs_P*Dphi3(t1_a)
     & +( Cq_P*5.d0/3.d0+Cs_P*sqrt(2.d0)/3.d0 )*A3etaP*Dphi3(t1_a))
     & +(1.d0,0.d0)
     & * 4.d0*sqrt(2.d0)*Fsmv3Hv3/3.d0/Fchir * t2_a
     & *(3.d0*Cq_P* Drho3(t2_a) + Cq_P* Domega3(t2_a)/3.d0 
     &     + 2.d0*sqrt(2.d0)/3.d0 * Cs_P* Dphi3(t2_a)
     & +( Cq_P*5.d0/3.d0+Cs_P*sqrt(2.d0)/3.d0 )*A3etaP*Dphi3(t2_a))
     & -(1.d0,0.d0)
     & * 8.d0*SIGv3Fsmv2/Fchir * t1_a*t2_a
     & *(  Cq_P* Drho3(t1_a)* Drho3(t2_a)/2.d0 
     &   + Cq_P* Domega3(t1_a)* Domega3(t2_a)/18.d0 
     &   + sqrt(2.d0)/9.d0 * Cs_P* Dphi3(t1_a)* Dphi3(t2_a)
     & +( Cq_P*5.d0/3.d0+Cs_P*sqrt(2.d0)/3.d0 )*A3etaP/6.d0
     & *Dphi3(t1_a)*Dphi3(t2_a))
     & )

      etaPggFF_dprime2 = ffdummy
      return
      end
   
c **********************************************************************
c     Vector meson propagators
c----------------------------------------------------------
c
c     Vector meson propagators
c   1st octet
      complex*16 function Drho(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaRhoTotal
      real*8 q2t
c      call const_input
      if (q2t .gt. 0.d0) then
c        Drho = (q2t - Mrho_c**2 
c     &      + (0.d0, 1.d0) * sqrt(q2t)
c     &       * GammaRhoTotal(q2t))**(-1.d0)
        Drho = 1.d0/(q2t - Mrho_c**2 
     &      + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalrho_c)
      else
        Drho = 1.d0/(q2t - Mrho_c**2)
      endif
      return
      end

      complex*16 function Domega(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaOmegaTotal
      real*8 q2t,par(20)
c      call const_input   
      if (q2t .gt. 0.0) then
        Domega = 1.d0/(q2t - Momega_c**2 
     &      + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalomega_c)
      else
        Domega = 1.d0/(q2t - Momega_c**2)
      endif
      return
      end

      complex*16 function DPhi(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaPhiTotal
      real*8 q2t,par(20)
c      call const_input 
      if (q2t .gt. 0.d0) then
        DPhi = 1.d0/(q2t - MPhi_c **2 
     &       + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalphi_c )
      else
        DPhi =1.d0/(q2t - MPhi_c**2)
      endif
      return
      end
c----------------------------------------------------------------------

c     Vector meson propagators
c 2nd octet
      complex*16 function Drho2(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaRhoTotal
      real*8 q2t
c      call const_input
      if (q2t .gt. 0.d0) then
        Drho2 = 1.d0/(q2t - Mrho_pr**2 
     &      + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalrho_c2)
      else
        Drho2 = 1.d0/(q2t - Mrho_pr**2)
      endif
      return
      end

      complex*16 function Domega2(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaOmegaTotal
      real*8 q2t
c      call const_input   
      if (q2t .gt. 0.0) then
        Domega2 = 1.d0/(q2t - Momega_pr**2 
     &      + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalomega_c2)
      else
        Domega2 = 1.d0/(q2t - Momega_pr**2)
      endif
      return
      end

      complex*16 function DPhi2(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaPhiTotal
      real*8 q2t
c      call const_input 

      if (q2t .gt. 0.d0) then
        DPhi2 = 1.d0/(q2t - MPhi_pr **2 
     &       + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalphi_c2 )
      else
        DPhi2 =1.d0/(q2t - MPhi_pr**2)
      endif
      return
      end
c---------------------------------------------------------------------
c     Vector meson propagators
c 3rd octet
      complex*16 function Drho3(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaRhoTotal
      real*8 q2t
c      call const_input
      if (q2t .gt. 0.d0) then
        Drho3 = 1.d0/(q2t - Mrho_dpr**2 
     &      + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalrho_c3)
      else
        Drho3 = 1.d0/(q2t - Mrho_dpr**2)
      endif
      return
      end

      complex*16 function Domega3(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaOmegaTotal
      real*8 q2t
c      call const_input   
      if (q2t .gt. 0.0) then
        Domega3 = 1.d0/(q2t - Momega_dpr**2 
     &      + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalomega_c3)
      else
        Domega3 = 1.d0/(q2t - Momega_dpr**2)
      endif
      return
      end

      complex*16 function DPhi3(q2t)
      include 'phokhara_10.0.inc'
c$$$      implicit none
c$$$      real*8            Mrho_c, Momega_c, Mphi_c
c$$$      common /masses_c/  Mrho_c, Momega_c, Mphi_c
      complex*16 GammaPhiTotal
      real*8 q2t
c      call const_input 

      if (q2t .gt. 0.d0) then
        DPhi3 = 1.d0/(q2t - MPhi_dpr **2 
     &       + (0.d0, 1.d0) * sqrt(q2t)
     &       * GammaTotalphi_c3 )
      else
        DPhi3 =1.d0/(q2t - MPhi_dpr**2)
      endif
      return
      end

c----------------------------------------------------------
cc **********************************************************************
      complex*16 function FormFactor_etapipi(qq2)
      include 'phokhara_10.0.inc'       
      real*8 qq2,rho0m,rho0g,qq12_3pi,qq13_3pi,qq23_3pi,const,s1
      integer ii
      complex*16 RR_3pi,HH_3pi,RRro_3pi,Factor,bb0,bb1,arho
      common/qqij_3pi/qq12_3pi,qq13_3pi,qq23_3pi

c PDG 
      rho0m = 0.77549d0
      rho0g = 0.1494d0
c
      const = 1.d0/4.d0/sqrt(3.d0)/pi**2/fpi**3
c
      s1 = qq23_3pi
c
      Factor =  arho(s1,rho0m,rho0g,mpi,1)*const


      bb1 = exp(dcmplx(0.d0,par_eta(2)))

      bb0 = exp(dcmplx(0.d0,par_eta(8)))
c
      FormFactor_etapipi =   
     1   Factor*(arho(qq2,par_eta(3),par_eta(4),mpi,2)
     1      +   bb1 * par_eta(9) * arho(qq2,par_eta(5),par_eta(6),mpi,2)
     2      +   bb0 * par_eta(7) * arho(qq2,rho0m,rho0g,mpi,1))
     3        /(1.d0 + bb1 * par_eta(9) + bb0 * par_eta(7))
c
c      do ii=2,9
cc       write(6,'(1x,i3,1pd22.15)')ii,par_eta(ii)
c      enddo
c       write(6,'(1x,6(1pd22.15))')qq2,s1,const,mpi,FormFactor_etapipi
c       write(6,'(1x,6(1pd22.15))')bb1,bb0,rho0m,rho0g
c       write(6,'(1x,2(1pd22.15))')arho(s1,rho0m,rho0g,mpi,1)
c       stop
      return
      end
c*****************************************************************
      complex*16 function arho(s,m,gam,mpi,n)
      implicit none
      real*8 g,m,gam,s,mpi
      complex*16 i
      integer n

      i=(0.d0,1.d0)

      if(n.eq.1)then !!! rho(770)
         g = gam*(m**2/s)*sqrt(((s-4.d0*mpi**2)/(m**2-4.d0*mpi**2))**3)
      elseif(n.eq.2)then !!! rho(1450),rho(1700)
         g = gam*s/m**2
      endif

      arho = m*m/(s-m*m+i*sqrt(s)*g)

      end function arho
c **********************************************************************
      complex*16 function KaonFormFactor(a)
      include 'phokhara_10.0.inc'       
      real*8 a
      complex*16 BW_K,BW_om,BW_rho,sum_rho_Kp,sum_om_Kp,sum_phi_Kp,BW_GS

      if(FF_kaon.eq.0) then  ! to na razie tylko
                                                 ! constrained

      call sum_FF_Kp1(a,sum_rho_Kp,sum_om_Kp,sum_phi_Kp)

c       call c_sum_Kp(a,sum_rho_Kp,sum_om_Kp,sum_phi_Kp) 

        if(pion.eq.6)then

           KaonFormFactor = 
     1     0.5d0 * ( c_rho0_Kp * BW_GS(m_rho0_Kp,g_rho0_Kp,a,1) +
     2               c_rho1_Kp * BW_GS(m_rho1_Kp,g_rho1_Kp,a,1) + 
     3               c_rho2_Kp * BW_GS(m_rho2_Kp,g_rho2_Kp,a,1) +
     4               c_rho3_Kp * BW_GS(m_rho3_Kp,g_rho3_Kp,a,1) +
     5               c_rho4_Kp * BW_GS(m_rho4_Kp,g_rho4_Kp,a,1) +
     5               sum_rho_Kp )
     1   + 1.d0/6.d0 * ( c_om0_Kp * BW_om(m_om0_Kp,g_om0_Kp,a,1) +
     2                   c_om1_Kp * BW_om(m_om1_Kp,g_om1_Kp,a,1) + 
     3                   c_om2_Kp * BW_om(m_om2_Kp,g_om2_Kp,a,1) +
     4                   c_om3_Kp * BW_om(m_om3_Kp,g_om3_Kp,a,1) +
     5                   c_om4_Kp * BW_om(m_om4_Kp,g_om4_Kp,a,1) +
     5                   sum_om_Kp ) 
     1   + 1.d0/3.d0 * ( c_phi0_Kp * BW_K(m_phi0_Kp,g_phi0_Kp,a,1) +
     2                   c_phi1_Kp * BW_K(m_phi1_Kp,g_phi1_Kp,a,1) +
     3                   c_phi2_Kp * BW_K(m_phi2_Kp,g_phi2_Kp,a,1) +
     4                   c_phi3_Kp * BW_K(m_phi3_Kp,g_phi3_Kp,a,1) + 
     4                   sum_phi_Kp )


        elseif(pion.eq.7)then
           KaonFormFactor =
     1   - 0.5d0 * ( c_rho0_Kp * BW_GS(m_rho0_Kp,g_rho0_Kp,a,1) +
     2               c_rho1_Kp * BW_GS(m_rho1_Kp,g_rho1_Kp,a,1) + 
     3               c_rho2_Kp * BW_GS(m_rho2_Kp,g_rho2_Kp,a,1) +
     4               c_rho3_Kp * BW_GS(m_rho3_Kp,g_rho3_Kp,a,1) +
     5               c_rho4_Kp * BW_GS(m_rho4_Kp,g_rho4_Kp,a,1) +
     5               sum_rho_Kp )
     1   + 1.d0/6.d0 * ( c_om0_Kp * BW_om(m_om0_Kp,g_om0_Kp,a,1) +
     2                   c_om1_Kp * BW_om(m_om1_Kp,g_om1_Kp,a,1) + 
     3                   c_om2_Kp * BW_om(m_om2_Kp,g_om2_Kp,a,1) +
     4                   c_om3_Kp * BW_om(m_om3_Kp,g_om3_Kp,a,1) +
     5                   c_om4_Kp * BW_om(m_om4_Kp,g_om4_Kp,a,1) +
     5                   sum_om_Kp ) 
     1   + 1.d0/3.d0 * ( eta_phi_Kp * c_phi0_Kp
     2                              * BW_K(m_phi0_Kp,g_phi0_Kp,a,1) +
     3                   c_phi1_Kp  * BW_K(m_phi1_Kp,g_phi1_Kp,a,1) +
     4                   c_phi2_Kp  * BW_K(m_phi2_Kp,g_phi2_Kp,a,1) +
     5                   c_phi3_Kp  * BW_K(m_phi3_Kp,g_phi3_Kp,a,1) +
     5                   sum_phi_Kp )
        endif
c 
      elseif(FF_kaon.eq.1) then

      call sum_FF_Kp1(a,sum_rho_Kp,sum_om_Kp,sum_phi_Kp)

c       call c_sum_Kp(a,sum_rho_Kp,sum_om_Kp,sum_phi_Kp) 

        if(pion.eq.6)then

           KaonFormFactor = 
     1     0.5d0 * ( c_rho0_Kp * BW_GS(m_rho0_Kp,g_rho0_Kp,a,1) +
     2               c_rho1_Kp * BW_GS(m_rho1_Kp,g_rho1_Kp,a,1) + 
     3               c_rho2_Kp * BW_GS(m_rho2_Kp,g_rho2_Kp,a,1) +
     4               c_rho3_Kp * BW_GS(m_rho3_Kp,g_rho3_Kp,a,1) +
     5               sum_rho_Kp )
     1   + 1.d0/6.d0 * ( c_om0_Kp * BW_om(m_om0_Kp,g_om0_Kp,a,1) +
     2                   c_om1_Kp * BW_om(m_om1_Kp,g_om1_Kp,a,1) + 
     3                   c_om2_Kp * BW_om(m_om2_Kp,g_om2_Kp,a,1) +
     4                   c_om3_Kp * BW_om(m_om3_Kp,g_om3_Kp,a,1) +
     5                   sum_om_Kp ) 
     1   + 1.d0/3.d0 * ( c_phi0_Kp * BW_K(m_phi0_Kp,g_phi0_Kp,a,1) +
     2                   c_phi1_Kp * BW_K(m_phi1_Kp,g_phi1_Kp,a,1) +
     3                   c_phi2_Kp * BW_K(m_phi2_Kp,g_phi2_Kp,a,1) +
     4                   sum_phi_Kp )

        elseif(pion.eq.7)then

           KaonFormFactor = 
     1   - 0.5d0 * ( c_rho0_Kp * BW_GS(m_rho0_Kp,g_rho0_Kp,a,1) +
     2               c_rho1_Kp * BW_GS(m_rho1_Kp,g_rho1_Kp,a,1) + 
     3               c_rho2_Kp * BW_GS(m_rho2_Kp,g_rho2_Kp,a,1) +
     4               c_rho3_Kp * BW_GS(m_rho3_Kp,g_rho3_Kp,a,1) +
     5               sum_rho_Kp )
     1   + 1.d0/6.d0 * ( c_om0_Kp * BW_om(m_om0_Kp,g_om0_Kp,a,1) +
     2                   c_om1_Kp * BW_om(m_om1_Kp,g_om1_Kp,a,1) + 
     3                   c_om2_Kp * BW_om(m_om2_Kp,g_om2_Kp,a,1) +
     4                   c_om3_Kp * BW_om(m_om3_Kp,g_om3_Kp,a,1) +
     5                   sum_om_Kp ) 
     1   + 1.d0/3.d0 * ( eta_phi_Kp * c_phi0_Kp
     2                              * BW_K(m_phi0_Kp,g_phi0_Kp,a,1) +
     3                   c_phi1_Kp  * BW_K(m_phi1_Kp,g_phi1_Kp,a,1) +
     4                   c_phi2_Kp  * BW_K(m_phi2_Kp,g_phi2_Kp,a,1) +
     5                   sum_phi_Kp )
        endif

      elseif(FF_kaon.eq.2) then  ! old

        if(pion.eq.6)then

           KaonFormFactor = 
     1     0.5d0 * ( c_rho0_Kp * BW_rho(m_rho0_Kp,g_rho0_Kp,a,1) +
     5               c_rho1_Kp * BW_rho(m_rho1_Kp,g_rho1_Kp,a,1) + 
     2               c_rho2_Kp * BW_rho(m_rho2_Kp,g_rho2_Kp,a,1) )
     3 + 1.d0/6.d0 * ( c_om0_Kp * BW_om(m_om0_Kp,g_om0_Kp,a,1) +
     1                 c_om1_Kp * BW_om(m_om1_Kp,g_om1_Kp,a,1) + 
     2                 c_om2_Kp * BW_om(m_om2_Kp,g_om2_Kp,a,1) ) 
     4 + 1.d0/3.d0 * ( c_phi0_Kp * BW_K(m_phi0_Kp,g_phi0_Kp,a,1) +
     1                 c_phi1_Kp * BW_K(m_phi1_Kp,g_phi1_Kp,a,1) )
        elseif(pion.eq.7)then
           KaonFormFactor = 
     1   - 0.5d0 * ( c_rho0_Kp * BW_rho(m_rho0_Kp,g_rho0_Kp,a,1) +
     5               c_rho1_Kp * BW_rho(m_rho1_Kp,g_rho1_Kp,a,1) + 
     2               c_rho2_Kp * BW_rho(m_rho2_Kp,g_rho2_Kp,a,1) )
     3 + 1.d0/6.d0 * ( c_om0_Kp * BW_om(m_om0_Kp,g_om0_Kp,a,1) +
     1                 c_om1_Kp * BW_om(m_om1_Kp,g_om1_Kp,a,1) + 
     2                 c_om2_Kp * BW_om(m_om2_Kp,g_om2_Kp,a,1) ) 
     4 + 1.d0/3.d0*(eta_phi_Kp*c_phi0_Kp*BW_K(m_phi0_Kp,g_phi0_Kp,a,1) +
     1                   c_phi1_Kp * BW_K(m_phi1_Kp,g_phi1_Kp,a,1) )
        endif

      endif

      return
      end
c **********************************************************************
      complex*16 function tail_sum(qq)
      include 'phokhara_10.0.inc'       
      real*8 qq
      complex*16 BW_GS_09
      integer ii
c
      tail_sum = 0.d0

      do ii=6,2000
        tail_sum = tail_sum + c_n_pionGS(ii) * BW_GS_09(ii,qq)
      enddo
      return
      end
c **********************************************************************
      subroutine sum_FF_Kp1(a,sum_rho_Kp,sum_om_Kp,sum_phi_Kp)
      include 'phokhara_10.0.inc'       
      real*8 a,deltaqq,qq_min_Kp
      complex*16 sum_rho_Kp,sum_om_Kp,sum_phi_Kp,c_sum_rho_Kp,aaa
      integer ii,jj

      if(pion.eq.6) then
         qq_min_Kp = 4.d0*mKp**2
         ii = 40000
      endif
      if(pion.eq.7) then
         qq_min_Kp = 4.d0*mKn**2
         ii = 40000
      endif

      deltaqq = ( 11.0002d0**2 - qq_min_Kp ) / dfloat(ii)
      jj = int ( ( a - qq_min_Kp ) / deltaqq )

      aaa = ( tab_sum_rho_Kp(jj+2) - tab_sum_rho_Kp(jj+1) )
     1    / ( tab_qq_Kp(jj+2) - tab_qq_Kp(jj+1) )
      sum_rho_Kp =aaa *a +( tab_sum_rho_Kp(jj+1) -aaa *tab_qq_Kp(jj+1) ) 

      aaa = ( tab_sum_om_Kp(jj+2) - tab_sum_om_Kp(jj+1) )
     1    / ( tab_qq_Kp(jj+2) - tab_qq_Kp(jj+1) ) 
      sum_om_Kp =aaa *a +( tab_sum_om_Kp(jj+1) - aaa * tab_qq_Kp(jj+1) ) 

      aaa = ( tab_sum_phi_Kp(jj+2) - tab_sum_phi_Kp(jj+1) )
     1    / ( tab_qq_Kp(jj+2) - tab_qq_Kp(jj+1) )
      sum_phi_Kp =aaa *a +( tab_sum_phi_Kp(jj+1) -aaa *tab_qq_Kp(jj+1) )  

      return
      end
c **********************************************************************
      subroutine sum_FF_Kp()
      include 'phokhara_10.0.inc'       
      integer jj,ii
      real*8 GAMMA,qq_min_Kp,qq_max_Kp
      real*8 ilocz_rho_Kp,ilocz_phi_Kp,ilocz_om_Kp,beta_p_rho_Kp,
     1       beta_p_phi_Kp,beta_p_om_Kp
      complex*16 sum_rho_Kp,sum_om_Kp,sum_phi_Kp

      do jj=1,80000
        tab_sum_rho_Kp(jj) = (0.d0,0.d0)
        tab_sum_om_Kp(jj) = (0.d0,0.d0)
        tab_sum_phi_Kp(jj) = (0.d0,0.d0)
        tab_qq_Kp(jj) = 0.d0
      enddo

        qq_max_Kp = 11.0002d0**2

      if(pion.eq.6) qq_min_Kp = 4.d0 * mKp**2
      if(pion.eq.7) qq_min_Kp = 4.d0 * mKn**2

      if(sqrt(Sp).gt.11.d0) then
        write(6,*)' this function is to be used  below sqrt(s) = 11 GeV'
        stop
      endif

        beta_p_rho_Kp = beta_rho_Kp - 1.d0
        ilocz_rho_Kp = 1.d0

        beta_p_phi_Kp = beta_phi_Kp - 1.d0
        ilocz_phi_Kp = 1.d0

      if(FF_kaon.eq.1) then
         beta_p_om_Kp = beta_om_Kp - 1.d0
         ilocz_om_Kp = 1.d0
      endif

      do jj=1,1000
              
        ilocz_rho_Kp = ilocz_rho_Kp * ( 1.d0 - beta_p_rho_Kp/jj )
        ilocz_phi_Kp = ilocz_phi_Kp * ( 1.d0 - beta_p_phi_Kp/jj )
        if(FF_kaon.eq.1) ilocz_om_Kp = ilocz_om_Kp 
     1                               * ( 1.d0 - beta_p_om_Kp/jj )

        coeff_rho_Kp(jj) = (-1.d0)**jj * GAMMA(beta_rho_Kp-0.5d0) 
     1       * 2.d0 / sqrt(pi) / (1.d0+2.d0*jj) * ilocz_rho_Kp /pi 
     2       * GAMMA(2.d0-beta_rho_Kp) * sin(pi*(beta_rho_Kp-1.d0-jj))
        coeff_phi_Kp(jj) = (-1.d0)**jj * GAMMA(beta_phi_Kp-0.5d0) 
     1       * 2.d0 / sqrt(pi) / (1.d0+2.d0*jj) * ilocz_phi_Kp /pi 
     2       * GAMMA(2.d0-beta_phi_Kp) * sin(pi*(beta_phi_Kp-1.d0-jj))
        if(FF_kaon.eq.0) then 
           coeff_om_Kp(jj) = coeff_rho_Kp(jj)
        elseif(FF_kaon.eq.1) then
           coeff_om_Kp(jj) = (-1.d0)**jj * GAMMA(beta_om_Kp-0.5d0) 
     1          * 2.d0 / sqrt(pi) / (1.d0+2.d0*jj) * ilocz_om_Kp /pi 
     2          * GAMMA(2.d0-beta_om_Kp) * sin(pi*(beta_om_Kp-1.d0-jj))
        endif

          mass_n_rho_Kp(jj)=sqrt( m_rho0_Kp**2 * (1.d0 + 2.d0*jj) )
          gam_n_rho_Kp(jj) = gam_rho_Kp * mass_n_rho_Kp(jj)
          mass_n_phi_Kp(jj)=sqrt( m_phi0_Kp**2 * (1.d0 + 2.d0*jj) )
          gam_n_phi_Kp(jj) = gam_phi_Kp * mass_n_phi_Kp(jj)
          mass_n_om_Kp(jj)=sqrt( m_om0_Kp**2 * (1.d0 + 2.d0*jj) )
          gam_n_om_Kp(jj) = gam_om_Kp * mass_n_om_Kp(jj)
      enddo

c -----------------------------------------------------------------------
 
         if(pion.eq.6) ii = 50000
         if(pion.eq.7) ii = 50000

         do jj=1,ii
            tab_qq_Kp(jj) = qq_min_Kp + (qq_max_Kp - qq_min_Kp)
     1                                * dfloat(jj-1) / dfloat(ii)
            call c_sum_Kp(tab_qq_Kp(jj),sum_rho_Kp,sum_om_Kp,sum_phi_Kp)
            tab_sum_rho_Kp(jj) = sum_rho_Kp
            tab_sum_om_Kp(jj)  = sum_om_Kp
            tab_sum_phi_Kp(jj) = sum_phi_Kp
         enddo

      return
      end
c **********************************************************************
      subroutine c_sum_Kp(a,sum_rho_Kp,sum_om_Kp,sum_phi_Kp)
      include 'phokhara_10.0.inc' 
      real*8 a
      complex*16 BW_GS,BW_om,BW_K,sum_rho_Kp,sum_om_Kp,sum_phi_Kp
      integer ii,jj,kk

       sum_rho_Kp = (0.d0,0.d0)
       sum_om_Kp  = (0.d0,0.d0)
       sum_phi_Kp = (0.d0,0.d0)

      if(FF_kaon.eq.0) then
         jj = 4
         kk = 5
      endif
      if(FF_kaon.eq.1)then
         jj = 3
         kk = 4
      endif

      do ii=jj,1000
          sum_phi_Kp = sum_phi_Kp + coeff_phi_Kp(ii) 
     1               * BW_K(mass_n_phi_Kp(ii),gam_n_phi_Kp(ii),a,1)
          if(ii.ge.kk)then
             sum_rho_Kp = sum_rho_Kp + coeff_rho_Kp(ii) 
     1              * BW_GS(mass_n_rho_Kp(ii),gam_n_rho_Kp(ii),a,1)
             sum_om_Kp = sum_om_Kp + coeff_om_Kp(ii) 
     1              * BW_om(mass_n_om_Kp(ii),gam_n_om_Kp(ii),a,1)    
          endif
      enddo
c
      return
      end
c **********************************************************************
      complex*16 function BW_om(m,breite,x,k)
      implicit none       
      integer k
      real*8 m,breite,x
      complex *16 i
      i=(0.d0,1.d0)

      if(k.eq.1)then
         BW_om=m*m/(m*m-x-i*m*breite)
      else
         BW_om=m*m/(m*m-x+i*m*breite)
      endif
      return
      end
c **********************************************************************
      complex*16 function BW_K(m,breite,x,k)
      include 'phokhara_10.0.inc'       
      integer k
      real*8 m,breite,x,g
      complex *16 i

      if(pion.eq.6) g=breite*m*m/x*(sqrt((abs(x-4.d0*mKp*mKp))/
     &     (m*m-4.d0*mKp*mKp))**3)
      if(pion.eq.7) g=breite*m*m/x*(sqrt((abs(x-4.d0*mKn*mKn))/
     &     (m*m-4.d0*mKn*mKn))**3)

      i=(0.d0,1.d0)
      if(k.eq.1)then
         BW_K=m*m/(m*m-x-i*sqrt(x)*g)
      else
         BW_K=m*m/(m*m-x+i*sqrt(x)*g)
      endif
      return
      end
c **********************************************************************
      complex*16 function BW_rho(m,breite,x,k)
      include 'phokhara_10.0.inc'              
      integer k
      real*8 m,breite,x,g
      complex *16 i

         g=breite*m*m/x*(sqrt((abs(x-4.d0*mpi*mpi))/
     &     (m*m-4.d0*mpi*mpi))**3)

      i=(0.d0,1.d0)
      if(k.eq.1)then
         BW_rho=m*m/(m*m-x-i*sqrt(x)*g)
      else
         BW_rho=m*m/(m*m-x+i*sqrt(x)*g)
      endif
      return
      end
c **********************************************************************
      complex*16 function BW_GS(m,breite,x,k)
      include 'phokhara_10.0.inc'       
      integer k
      real*8 m,breite,x,g,HH_GS,dd,p_rho
      complex *16 i

         g=breite*m*m/x*(sqrt((abs(x-4.d0*mpi*mpi))/
     &     (m*m-4.d0*mpi*mpi))**3)

      i=(0.d0,1.d0)

      p_rho = sqrt(m*m-4.d0*mpi*mpi) / 2.d0

      dd = 3.d0/2.d0/pi * mpi**2 / p_rho**2 
     1   * log( ( m + 2.d0*p_rho )**2 / 4.d0/mpi**2 )
     2   + m/2.d0/pi/p_rho 
     3   - mpi**2 * m /pi/p_rho**3

      if(k.eq.1)then
         BW_GS=(m*m+dd*breite*m)/(m*m-x+HH_GS(m,breite,x)-i*sqrt(x)*g)
      else
         BW_GS=(m*m+dd*breite*m)/(m*m-x+HH_GS(m,breite,x)+i*sqrt(x)*g)
      endif
  
      return
      end
c **********************************************************************
      complex*16 function BW_GS_09(kk,x)
      include 'phokhara_10.0.inc'       
      integer kk
      real*8 x,g,HH_GS_09
      complex *16 i

      g=gam_n_pionGS(kk)*m_n_pionGS(kk)*m_n_pionGS(kk)/x
     1 *sqrt((x-4.d0*mpi*mpi)/(m_n_pionGS(kk)*m_n_pionGS(kk)
     2 -4.d0*mpi*mpi))**3

      i=(0.d0,1.d0)

      BW_GS_09 = dd_ffpi(kk)
     1     /(m_n_pionGS(kk)*m_n_pionGS(kk)-x+HH_GS_09(kk,x)-i*sqrt(x)*g)

  
      return
      end
c **********************************************************************
      real*8 function HH_GS(m,breite,x)
      include 'phokhara_10.0.inc'       
      real*8 breite,x,HH_p,poch,vv_rho,m

      vv_rho = sqrt( 1.d0 - 4.d0*mpi**2/m**2 ) 

      poch = m**2 * breite / pi / sqrt(m**2-4.d0*mpi**2)**3
     1     * ( vv_rho**2 + vv_rho/2.d0*(3.d0-vv_rho**2) 
     2       * log( (1.d0+vv_rho)**2/4.d0/mpi**2 * m**2) )
      
      HH_GS = HH_p(m,breite,x) - HH_p(m,breite,m**2) 
     1   - ( x - m**2 ) * poch

      return
      end
c **********************************************************************
      double precision function HH_GS_09(kk,x)
      include 'phokhara_10.0.inc'       
      integer kk
      real*8 x,HH_p
      
      HH_GS_09 = HH_p(m_n_pionGS(kk),gam_n_pionGS(kk),x)
     1               - HH_p_ffpi(kk) 
     1   - ( x - m_n_pionGS(kk)**2 ) * poch_ffpi(kk)

      return
      end
c **********************************************************************
      real*8 function HH_p(m,breite,qq)
      include 'phokhara_10.0.inc'       
      real*8 breite,qq,vv,m

      vv = sqrt( abs(1.d0 - 4.d0*mpi**2/qq) )

      HH_p = m**2 * breite * qq / pi / sqrt(m**2-4.d0*mpi**2)**3
     1     * vv**3 * log( (1.d0+vv)**2 / 4.d0 / mpi**2 * qq )

      return
      end
c **********************************************************************
      complex*16 function HH_3pi(rhomm,rhogm,rhomp,rhogp,rhomz,rhogz)
      real*8 rhomm,rhogm,rhomp,rhogp,rhomz,rhogz,
     1       qq12_3pi,qq13_3pi,qq23_3pi
      complex*16 RRro_3pi
      integer ii
      common/qqij_3pi/qq12_3pi,qq13_3pi,qq23_3pi

      HH_3pi= RRro_3pi(qq12_3pi,rhomz,rhogz)
     1      + RRro_3pi(qq13_3pi,rhomp,rhogp)
     2      + RRro_3pi(qq23_3pi,rhomm,rhogm)

      return
      end
c **********************************************************************
      complex*16 function RRro_3pi(qq,mm,mmg)
      implicit none
      real*8 qq,mm,mmg,Gam_3pi
      complex*16 i

      i = dcmplx(0.d0,1.d0)
       RRro_3pi=1.d0/(qq/mm**2-1.d0+i*sqrt(qq)*Gam_3pi(qq,mm,mmg)/mm**2)
      return
      end
c ***********************************************************************
      real*8 function Gam_3pi(qq,mm,mmg)
      include 'phokhara_10.0.inc'       
      real*8 qq,mm,mmg,qq12_3pi,qq13_3pi,qq23_3pi

      integer ii

      common/qqij_3pi/qq12_3pi,qq13_3pi,qq23_3pi

      if(qq.eq.qq12_3pi) then

       Gam_3pi = mmg*mm**2 / qq*sqrt( ( ( qq - 4.d0*rmass(1)**2 )/
     1           ( mm**2 - 4.d0*rmass(1)**2 ) ) **3 )

       if(qq.lt.(4.d0*rmass(1)**2))then
         if((abs(qq-4.d0*rmass(1)**2)/qq).lt.1.d-10)then
           Gam_3pi = 0.d0
         else
           write(*,*)'qq = ',qq
           write(*,*)'4*rmass(1)**2 = ',4.d0*rmass(1)**2
         endif
       endif

      else
       Gam_3pi = mmg*mm**2 / qq*sqrt( ( ( qq - (rmass(1)+rmass(3))**2 )/
     1           ( mm**2 - ( rmass(1) + rmass(3) )**2 ) )**3 )

       if(qq.lt.(rmass(1)+rmass(3))**2)then
         if((abs(qq-(rmass(1)+rmass(3))**2)/qq).lt.1.d-10)then
           Gam_3pi = 0.d0
         else
           write(*,'(1x,a,1pd23.16)')'qq = ',qq
           write(*,'(1x,a,1pd23.16)')'(rmass(1)+rmass(3))**2 = ',
     1                                (rmass(1)+rmass(3))**2
         endif
       endif

      endif

      return
      end
c **********************************************************************
      complex*16 function RR_3pi(mm,mmg,qq2)
      implicit none
      real*8 mm,mmg,qq2
      complex*16 i

      i = dcmplx(0.d0,1.d0)
      RR_3pi = 1.d0/(qq2/mm**2 - 1.d0 + i*mmg/mm)
      return
      end
c **********************************************************************

      complex*16 function appr_sum(qq)
      include 'phokhara_10.0.inc'       
      real*8 qq,appr_r,appr_i,q_ss
      real*8 aa,bb,cc,tt,dd,ee,ff,gg,hh,jj,kk,ll,mm,nn,oo,pp,rr,ss,uu     
      q_ss = sqrt(qq)
c ---------------------------------------
c imaginary part
c (imaginary part - to be used for 2*mpi < sqrt(s) < 1.9 GeV
      if(q_ss.lt.1.9d0)then             
       tt = q_ss-2.d0*mpi
       aa = -0.000718099867d0
       bb = 1.49883073d0
       cc = -0.599502161d0
       dd = 3.01291361d0
       ee = 0.000112440192d0
       ff = 16.8164504d0
       gg = -4.81600874d0
       hh = 5.54036557d0
       jj = -3.2948141d0
       kk = 0.916269688d0
c
       appr_i = aa*tt**bb
     1 *(1.d0+cc*tt+dd*tt**2+gg*tt**3+hh*tt**4
     1  +jj*tt**5 +kk*tt**6
     2 +ee*tt**ff)
      elseif(q_ss.lt.2.6d0)then
c (imaginary part - to be used for 1.9 GeV < sqrt(s) < 2.6 GeV
c
       aa = -0.039071345d0
       bb = 0.0129735898d0
       cc = 2.30479409d0
       dd = -0.513327099d0
       ee = -1.45771258d0
       ff = 25.2344274d0
       gg = -0.55843411d0
       hh = -43.3152678d0
       jj = -0.328423868d0
       kk = 113.986304d0
       tt = q_ss-cc
      appr_i = (aa +tt*bb+tt**2*dd+tt**3*gg+tt**4*jj)
     1       /(1.d0+tt*ee+tt**2*ff+tt**3*hh+tt**4*kk)
      elseif(q_ss.lt.11.d0)then
c (imaginary part - to be used for 2.6 GeV < sqrt(s) < 10.6 GeV
c
        aa = -0.433813582d0
        bb = -0.00309846176d0
        cc = 2.42660454d0
        dd = -189.541887d0
        ee = 7.73108923d0
        appr_i = aa*(1.d0/q_ss-bb)**cc*(1.d0 + dd/q_ss**ee)
       else
        write(6,*)' this function is to be used  below sqrt(s) = 11 GeV'
        stop
       endif
c ---------------------------------------
c  real part
c ---------------------------------------
c below a fitted function 
c (real part - to be used for sqrt(s) < 2.35 GeV
c
      if(q_ss.lt.2.35d0)then             
       aa = -0.0235588194d0
       bb = 0.0509353703d0
       cc = -2.53568629d0
       dd =-0.0410886763d0
       ee =2.37249385d0
       ff =0.012325431d0
       gg =-0.800187742d0
       hh =-0.000404693395d0
       jj =-0.05464375d0
       kk =0.000491537627d0
       ll =0.012346788d0
       mm =0.000197618d0
       nn =0.0171119177d0
       oo =2.64910139d-05
       pp =0.0270500914d0
       rr = -0.000478819839d0
c
      appr_r = (aa+ bb*(q_ss-1.2d0)+ dd*(q_ss-1.2d0)**2
     1 + ff*(q_ss-1.2d0)**3+ hh*(q_ss-1.2d0)**4
     1 + kk*(q_ss-1.2d0)**5+ mm*(q_ss-1.2d0)**6
     1 + oo*(q_ss-1.2d0)**7+ rr*(q_ss-1.2d0)**8 )
     1  /(1.d0+ cc*(q_ss-1.2d0)+ ee*(q_ss-1.2d0)**2
     1 + gg*(q_ss-1.2d0)**3+ jj*(q_ss-1.2d0)**4
     1 + ll*(q_ss-1.2d0)**5+ nn*(q_ss-1.2d0)**6 
     1 + pp*(q_ss-1.2d0)**7 )
      elseif(q_ss.lt.3.09d0)then
c
      aa = 0.113336875d0
      bb = -0.24255157d0
      cc = -3.45189001d0
      dd = -24.9323934d0
      ee = -80.9049411d0
      ff = 14.8839941d0
      gg = 111.231181d0
      hh = 402.100665d0
      jj = -95.4007448d0
      kk = 687.517122d0
      ll = 66.258075d0
      mm = 409.908489d0
      nn = 234.397174d0
      oo = -154.63515d0
      pp = 130.735593d0
      rr = -175.517676d0
      ss = -47.3948306d0
      tt = 87.7912501d0
      uu = 0.999466217d0
c
      if ((1.d0/q_ss**2-aa).ge.0.d0)then
      appr_r = (1.d0/q_ss**2-aa)**uu
      else
      appr_r = -(-1.d0/q_ss**2+aa)**uu
      endif
      appr_r = appr_r
     1 *(bb+cc*(q_ss-2.9d0)+dd*(q_ss-2.9d0)**2
     2 +ee*(q_ss-2.9d0)**3+jj*(q_ss-2.9d0)**4+ll*(q_ss-2.9d0)**5
     2 +nn*(q_ss-2.9d0)**6+pp*(q_ss-2.9d0)**7+ss*(q_ss-2.9d0)**8 )
     3 /(1.d0+ff*(q_ss-2.9d0)+gg*(q_ss-2.9d0)**2
     4 +hh*(q_ss-2.9d0)**3+kk*(q_ss-2.9d0)**4+mm*(q_ss-2.9d0)**5
     2 +oo*(q_ss-2.9d0)**6+rr*(q_ss-2.9d0)**7+tt*(q_ss-2.9d0)**8 )
      elseif(q_ss.lt.11.d0)then
c
c below a fitted function 
c (real part - to be used for 3.09 GeV < sqrt(s) < 10.6 GeV
      aa = 0.315023275d0
      bb = 49.4458006d0
      cc = 219.202125d0
      dd = 85.4051109d0
      ee = 3254.82596d0
      ff = -4817.48097d0
      gg = -7126.62555d0
c
      appr_r = 1.d0/q_ss**2*
     1 (aa+bb/q_ss**2+dd/q_ss**4+ff/q_ss**6)
     2 /(1.d0+cc/q_ss**2+ee/q_ss**4+gg/q_ss**6)
      else
       continue
      endif     
c ---------------------------------------
      appr_sum = dcmplx(appr_r,appr_i)
c
      return
      end
c **********************************************************************
c this is a code of hadronic current \gamma^* pi+ pi- pi0 
c calculated in e+e- CMS

      subroutine had_3pi(qq,q1,q2,q3,hadr,nn)
      implicit none

      integer nn
      real*8 q1,q2,q3,qq
      complex*16 hadr(4),PionFormFactor_3pi,Factor,FormFactor_etapipi
      dimension q1(4),q2(4),q3(4)

      if(nn.eq.8)then
        Factor = PionFormFactor_3pi(qq)
      else
        Factor = FormFactor_etapipi(qq)
      endif

      hadr(1) = (  q1(2)* ( q2(3)*q3(4)-q2(4)*q3(3) )
     1           - q2(2)* ( q1(3)*q3(4)-q1(4)*q3(3) )
     2           + q3(2)* ( q1(3)*q2(4)-q1(4)*q2(3) ) )  * Factor
      hadr(2) = - ( - q1(1)* ( q2(3)*q3(4)-q2(4)*q3(3) ) 
     1            + q2(1)* ( q1(3)*q3(4)-q1(4)*q3(3) )
     2            - q3(1)* ( q1(3)*q2(4)-q1(4)*q2(3) ) )  * Factor
      hadr(3) = - (   q1(1)* ( q2(2)*q3(4)-q2(4)*q3(2) )
     1            - q2(1)* ( q1(2)*q3(4)-q1(4)*q3(2) )
     2            + q3(1)* ( q1(2)*q2(4)-q1(4)*q2(2) ) )  * Factor
      hadr(4) = - ( - q1(1)* ( q2(2)*q3(3)-q2(3)*q3(2) )
     1            + q2(1)* ( q1(2)*q3(3)-q1(3)*q3(2) )
     2            - q3(1)* ( q1(2)*q2(3)-q1(3)*q2(2) ) )  * Factor
      return
      end
c **********************************************************************
c this is a code of hadronic current rho(0) -> 2pi+ 2pi- 
c
      subroutine had2(qq2,q1,q2,q3,q4,hadr)
      implicit none
      real*8 q1,q2,q3,q4,qq2
      integer i
      dimension q1(4),q2(4),q3(4),q4(4)
      complex*16 hadr(4),hadr1(4),hadr2(4),hadr3(4),hadr4(4)
c
      call had3(qq2,q1,q2,q3,q4,hadr1)
      call had3(qq2,q4,q2,q3,q1,hadr2)
      call had3(qq2,q1,q3,q2,q4,hadr3)
      call had3(qq2,q4,q3,q2,q1,hadr4)
      do i=1,4
       hadr(i) = hadr1(i)+hadr2(i)+hadr3(i)+hadr4(i)
      enddo
c       
      return
      end
c*************************************************************************
c this is a code of hadronic current rho(0) -> pi+ pi- 2pi0
c
      subroutine had3(qq2,q1,q2,q3,q4,hadr)
      include 'phokhara_10.0.inc'
c
      integer i
      real*8 q1(4),q2(4),q3(4),q4(4),q2m4(4),q3m1(4),q4m1(4),q3m2(4)
      real*8 q123(4),q124(4),qq(4),q3m4(4),q134(4),q234(4)
      real*8 qqm1(4),qqm2(4),vfac1(4),vfac2(4)
      real*8 q2p4(4),q1p3(4),q2p3(4),q1p4(4),q1p2(4),q3p4(4)
      real*8 qqm1_2,qqm2_2,q_3m2,q_2m4,q_4m1,q_3m1
      real*8 q3_2m4,q2_3m1, q2_4m1,q3_4m1,q4_3m1,q4_3m2,qmq3_2,qmq4_2
      real*8 q_q1,q_q2,q_q3,q_q4,q2p4_2,q3p4_2,q1p3_2,q1p2_2,q2p3_2
      real*8 q1p4_2,q1p4_3m2,q1p3_2m4,q2p4_3m1,q2p3_4m1,q1p2_3m4
      real*8 q1_2m4,q1_3m2,fac2,sgo,q24,q34,q234_2,q134_2
      real*8 coupl1,coupl2,a1m,a1g,rhom,rhog,rho1m,rho1g,qq2,fac3
      real*8 rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim
      real*8 q1_134,q3_134,q4_134,q2_234,q3_234,q4_234,q12,q13,q14,q23
      real*8 g1,g9,g10
      complex*16 p1,p5,p10,p11,Hrho,bwg_om
      complex*16 tt(4,4,4),ss(4,4,4,4),hadr_rho(4)
      complex*16 bwga1,bwgrho,bwgrho_t,bwgf0,c0,c0f,c1,c2,c3,c4,c5,c6
      complex*16 c1_t,c2_t,c3_t,c4_t,anom_bwg,bwgrho_o
      complex*16 cfac(5),hadr(4),c0om
      complex*16 c0_r,c13_r,c14_r,c23_r,c24_r,bwgrho_r,c11_r,c22_r
c
c constants used in omega and rho channels
c
      g1  = 42.3d0        !3pi fit(paper)
      g9  = 5.997d0       !3pi fit(paper)
      g10 = 0.1212d0      !3pi fit(paper)
c
c     the dot products:
c
      do i=1,4
        q2m4(i) = q2(i)-q4(i)
        q3m1(i) = q3(i)-q1(i)
        q3m4(i) = q3(i)-q4(i)
        q4m1(i) = q4(i)-q1(i)
        q3m2(i) = q3(i)-q2(i)
        q2p4(i) = q2(i)+q4(i)
        q1p3(i) = q1(i)+q3(i)
        q1p2(i) = q1(i)+q2(i)
        q2p3(i) = q2(i)+q3(i)
        q1p4(i) = q1(i)+q4(i)
        q3p4(i) = q3(i)+q4(i)
        q123(i) = q2p3(i)+q1(i)
        q124(i) = q2p4(i)+q1(i)
        qq(i)   = q123(i) + q4(i)
      enddo
      q_3m2  = qq(1)*q3m2(1)-qq(2)*q3m2(2)-qq(3)*q3m2(3)-qq(4)*q3m2(4)
      q_2m4  = qq(1)*q2m4(1)-qq(2)*q2m4(2)-qq(3)*q2m4(3)-qq(4)*q2m4(4)
      q_4m1  = qq(1)*q4m1(1)-qq(2)*q4m1(2)-qq(3)*q4m1(3)-qq(4)*q4m1(4)
      q_3m1  = qq(1)*q3m1(1)-qq(2)*q3m1(2)-qq(3)*q3m1(3)-qq(4)*q3m1(4)
      q1_2m4 = q1(1)*q2m4(1)-q1(2)*q2m4(2)-q1(3)*q2m4(3)-q1(4)*q2m4(4)
      q1_3m2 = q1(1)*q3m2(1)-q1(2)*q3m2(2)-q1(3)*q3m2(3)-q1(4)*q3m2(4)
      q3_2m4 = q3(1)*q2m4(1)-q3(2)*q2m4(2)-q3(3)*q2m4(3)-q3(4)*q2m4(4)
      q2_3m1 = q2(1)*q3m1(1)-q2(2)*q3m1(2)-q2(3)*q3m1(3)-q2(4)*q3m1(4)
      q2_4m1 = q2(1)*q4m1(1)-q2(2)*q4m1(2)-q2(3)*q4m1(3)-q2(4)*q4m1(4)
      q3_4m1 = q3(1)*q4m1(1)-q3(2)*q4m1(2)-q3(3)*q4m1(3)-q3(4)*q4m1(4)
      q4_3m1 = q4(1)*q3m1(1)-q4(2)*q3m1(2)-q4(3)*q3m1(3)-q4(4)*q3m1(4)
      q4_3m2 = q4(1)*q3m2(1)-q4(2)*q3m2(2)-q4(3)*q3m2(3)-q4(4)*q3m2(4)
      qmq3_2 = q124(1)**2 -q124(2)**2 -q124(3)**2 -q124(4)**2
      qmq4_2 = q123(1)**2 -q123(2)**2 -q123(3)**2 -q123(4)**2
      q_q3   = qq(1)*q3(1)-qq(2)*q3(2)-qq(3)*q3(3)-qq(4)*q3(4)
      q_q4   = qq(1)*q4(1)-qq(2)*q4(2)-qq(3)*q4(3)-qq(4)*q4(4)
      q2p4_2 = q2p4(1)**2 - q2p4(2)**2 - q2p4(3)**2 - q2p4(4)**2
      q3p4_2 = q3p4(1)**2 - q3p4(2)**2 - q3p4(3)**2 - q3p4(4)**2
      q1p3_2 = q1p3(1)**2 - q1p3(2)**2 - q1p3(3)**2 - q1p3(4)**2
      q1p2_2 = q1p2(1)**2 - q1p2(2)**2 - q1p2(3)**2 - q1p2(4)**2
      q2p3_2 = q2p3(1)**2 - q2p3(2)**2 - q2p3(3)**2 - q2p3(4)**2
      q1p4_2 = q1p4(1)**2 - q1p4(2)**2 - q1p4(3)**2 - q1p4(4)**2
      q1p2_3m4 = q1p2(1)*q3m4(1)
     1          -q1p2(2)*q3m4(2)-q1p2(3)*q3m4(3)-q1p2(4)*q3m4(4)
      q1p3_2m4 = q1_2m4 + q3_2m4
      q1p4_3m2 = q1_3m2 + q4_3m2 
      q2p4_3m1 = q2_3m1 + q4_3m1
      q2p3_4m1 = q2_4m1 + q3_4m1 
c
      c0  = bwgrho_o(1,qq2)
      c0f = bwgrho_o(2,qq2)
c
      c1_t = bwgrho_t(q2p4_2)
      c2_t = bwgrho_t(q1p3_2)
      c3_t = bwgrho_t(q2p3_2)
      c4_t = bwgrho_t(q1p4_2)
c
      c5 = bwga1(qmq3_2)
      c6 = bwga1(qmq4_2)
c
      tt(1,2,4) = c5*c1_t*coupl1_4pi*c0
      tt(2,1,4) = c5*c4_t*coupl1_4pi*c0
      tt(2,3,1) = c6*c2_t*coupl1_4pi*c0
      tt(1,2,3) = c6*c3_t*coupl1_4pi*c0
c
      ss(3,4,1,2) = bwgrho(q3p4_2)*bwgf0(q1p2_2)*coupl2_4pi*c0f
c
      cfac(1) =  tt(1,2,3) * (-1.d0 - q1_3m2/qmq4_2 )
     1         + tt(1,2,4) * ( 1.d0 - q1_2m4/qmq3_2 )
     2         + tt(2,1,4) * ( 3.d0 + q2_4m1/qmq3_2 )
     3         + tt(2,3,1) * (-3.d0 - q2_3m1/qmq4_2 )
c
      cfac(2) =  tt(1,2,3) * (-3.d0 - q1_3m2/qmq4_2 )
     1         + tt(1,2,4) * ( 3.d0 - q1_2m4/qmq3_2 )
     2         + tt(2,1,4) * ( 1.d0 + q2_4m1/qmq3_2 )
     3         + tt(2,3,1) * (-1.d0 - q2_3m1/qmq4_2 )
c
      cfac(3) =  tt(1,2,3) * ( 1.d0 - q1_3m2/qmq4_2 )
     1         + tt(1,2,4) * ( 1.d0 + q1_2m4/qmq3_2 )
     2         + tt(2,1,4) * ( 1.d0 - q2_4m1/qmq3_2 )
     3         + tt(2,3,1) * ( 1.d0 - q2_3m1/qmq4_2 )
     4         -3.d0*ss(3,4,1,2)
c
      cfac(4) =  tt(1,2,3) 
     1 *(1.d0 -2.d0/qq2*(q_q4*q1_3m2/qmq4_2 +q_3m2) +q1_3m2/qmq4_2 )
     2         + tt(1,2,4) 
     3 *(-1.d0-2.d0/qq2*(q1_2m4/qmq3_2*q_q3 +q_2m4) +q1_2m4/qmq3_2 )
     4         + tt(2,1,4) 
     5 *(-1.d0+2.d0/qq2*(q_q3*q2_4m1/qmq3_2 +q_4m1) -q2_4m1/qmq3_2 )
     6         + tt(2,3,1)
     7 *(1.d0 -2.d0/qq2*(q2_3m1/qmq4_2*q_q4 +q_3m1) +q2_3m1/qmq4_2 )
     8       +3.d0*ss(3,4,1,2)/qq2*q1p2_3m4
c
c a1 part + f0 part
c
      do i=1,4
c
        hadr(i) =  q1(i)  *cfac(1) + q2(i)*cfac(2) 
     1           + q3m4(i)*cfac(3) + qq(i)*cfac(4)
c
      enddo
c
c  from here 'omega part'
c
      do i=1,4
        qqm1(i) = q2(i)+q3(i)+q4(i)
        qqm2(i) = q1(i)+q3(i)+q4(i)
      enddo
      q12    = q1(1)*q2(1)  - q1(2)*q2(2) - q1(3)*q2(3) - q1(4)*q2(4)
      q13    = q1(1)*q3(1)  - q1(2)*q3(2) - q1(3)*q3(3) - q1(4)*q3(4)
      q14    = q1(1)*q4(1)  - q1(2)*q4(2) - q1(3)*q4(3) - q1(4)*q4(4)
      q23    = q2(1)*q3(1)  - q2(2)*q3(2) - q2(3)*q3(3) - q2(4)*q3(4)
      q24    = q2(1)*q4(1)  - q2(2)*q4(2) - q2(3)*q4(3) - q2(4)*q4(4)
      q34    = q3(1)*q4(1)  - q3(2)*q4(2) - q3(3)*q4(3) - q3(4)*q4(4)
      q_q1   = qq(1)*q1(1)  - qq(2)*q1(2) - qq(3)*q1(3) - qq(4)*q1(4)
      q_q2   = qq(1)*q2(1)  - qq(2)*q2(2) - qq(3)*q2(3) - qq(4)*q2(4)
      qqm1_2 = qqm1(1)**2 - qqm1(2)**2 - qqm1(3)**2 - qqm1(4)**2
      qqm2_2 = qqm2(1)**2 - qqm2(2)**2 - qqm2(3)**2 - qqm2(4)**2
c 
      do i=1,4
          vfac1(i) = 2.d0* (- q2(i)*q13*q_q4 + q2(i)*q14*q_q3
     2       + q3(i)*q12*q_q4 - q3(i)*q14*q_q2 - q4(i)*q12*q_q3
     1       + q4(i)*q13*q_q2)
          vfac2(i) = 2.d0* (- q1(i)*q23*q_q4 + q1(i)*q24*q_q3
     2       + q3(i)*q12*q_q4 - q3(i)*q24*q_q1 - q4(i)*q12*q_q3
     1       + q4(i)*q23*q_q1)
      enddo
c
      p10 =  bwg_om(qqm1_2)  
      p11 =  bwg_om(qqm2_2) 
      p1 = Hrho(q2p3_2,q2p4_2,q3p4_2) 
      p5 = Hrho(q1p3_2,q1p4_2,q3p4_2) 
      c0om =  bwgrho_o(3,qq2)
c
      do i=1,4
c omega part added to a1+f0 parts
          hadr(i) = hadr(i)
     1     +( p10*p1*vfac1(i) + p11*p5*vfac2(i))*g1*g9*coupl_om_4pi*c0om
      enddo
c
c  from here the 'rhos only' part
c
c
      q_q1   = qq(1)*q1(1)-qq(2)*q1(2)-qq(3)*q1(3)-qq(4)*q1(4)
      q_q2   = qq(1)*q2(1)-qq(2)*q2(2)-qq(3)*q2(3)-qq(4)*q2(4)
c
      c0_r = g10*g9**3*bwgrho_r(qq2)*coupl3_4pi
c
      c13_r = bwgrho_r(q1p3_2)
      c14_r = bwgrho_r(q1p4_2)
      c23_r = bwgrho_r(q2p3_2)
      c24_r = bwgrho_r(q2p4_2)
c
      c11_r = c13_r*c24_r
      c22_r = c14_r*c23_r
c
      cfac(1) = c11_r*(  q_q2 - q_q4 + 2*q3_2m4) 
     3        + c22_r*(- q_q2 + q_q3 + 2*q4_3m2) 
     1          - 2.d0*c14_r + 2.d0*c13_r

      cfac(2) = c11_r*(- q_q1 + q_q3 + 2*q4_3m1) 
     3        + c22_r*(  q_q1 - q_q4 - 2*q3_4m1)
     2          - 2.d0*c24_r + 2.d0*c23_r

      cfac(3) = c11_r*(- q_q2 + q_q4 - 2*q1_2m4)
     1        + c22_r*(- q_q1 + q_q4 + 2*q2_4m1)
     3          - 2.d0*c13_r - 2.d0*c23_r

      cfac(4) = c11_r*(q_q1 - q_q3 - 2*q2_3m1) 
     3        + c22_r*(q_q2 - q_q3 - 2*q1_3m2)
     4          + 2.d0*c14_r + 2.d0*c24_r

      cfac(5) = c11_r*(- q_q1*q3_2m4 - q_q2*q4_3m1
     1                 + q_q3*q1_2m4 + q_q4*q2_3m1)
     2        + c22_r*(- q_q1*q4_3m2 + q_q2*q3_4m1
     3                 - q_q3*q2_4m1 + q_q4*q1_3m2)
     4        + c13_r*(q_q3 - q_q1) + c14_r*(q_q1 - q_q4)
     6        + c23_r*(q_q3 - q_q2) + c24_r*(q_q2 - q_q4)
c
c rho part added to a1+f0+omega
c
      do i=1,4
          hadr(i) = hadr(i)
     1     +          c0_r * ( q1(i)*cfac(1) + q2(i)*cfac(2)
     2                          + q3(i)*cfac(3) + q4(i)*cfac(4)
     3                          + 2.d0*qq(i)*cfac(5)/qq2 )
      enddo
c
      return
      end
c*************************************************************************
      real*8 function gfun_4pi(q1_2)
      include 'phokhara_10.0.inc'
c
      real*8 q1_2,c1
c
      if(q1_2.gt.((rhom_4pi+pim_4pi)**2))then
        gfun_4pi = q1_2*1.623d0 + 10.38d0 - 9.32d0/q1_2 + 0.65d0/q1_2**2
      else
        c1   = q1_2 - 9.d0*pim_4pi**2
        gfun_4pi = 4.1d0 *c1**3 *(1.d0 - 3.3d0*c1 + 5.8d0*c1**2)
      endif
c
      return
      end
c*************************************************************************
      complex*16 function bwgrho(q1_2)
      include 'phokhara_10.0.inc'
c
      real*8 q1_2
      complex*16 cbw,cbw1,cbw2,cbwo,BW_rho
c
      cbw = BW_rho(rhom_4pi,rhog_4pi,q1_2,1)
      cbw1 = BW_rho(rho1m_4pi,rho1g_4pi,q1_2,1)
      cbw2 = BW_rho(rho2m_4pi,rho2g_4pi,q1_2,1)
      bwgrho = ( cbw 
     1          + 0.08d0*cbw1-0.0075d0*cbw2)/(1.d0+0.08d0-0.0075d0)
      return
      end
c*************************************************************************
      complex*16 function bwgrho_t(q1_2)
      include 'phokhara_10.0.inc'
c
      real*8 q1_2,beta_4pi
      complex*16 cbw,cbw1,cbw2,cbwo,BW_rho
c
      beta_4pi  = -0.145d0
c
       cbw = BW_rho(rhom_4pi,rhog_4pi,q1_2,1)
       cbw1 = BW_rho(rho1m_4pi,rho1g_4pi,q1_2,1)

      bwgrho_t = (cbw+beta_4pi*cbw1)/(1.d0+beta_4pi) 
      return
      end
c*************************************************************************
c i=1: a1, i=2: f0, i=3: omega
c 
      complex*16 function bwgrho_o(ij,qq2)
      include 'phokhara_10.0.inc'
c
      complex*16 cbw,cbw1,cbw2,cbw3
      real*8 qq2,c1,c2,gamrho,gamrho1,gamrho2,gamrho3,beta1,beta2,beta3
      integer ij
c 
      c2     = 4.d0*pim_4pi**2/qq2
      c1     = rhom_4pi**2/qq2
      gamrho  = rhom_4pi*rhog_4pi*sqrt(abs(c1*((1.d0-c2)/(c1-c2))**3))
      c1      = rho1m_f_4pi**2/qq2
      gamrho1 = rho1m_f_4pi*rho1g_f_4pi
     1         *sqrt(abs(c1*((1.d0-c2)/(c1-c2))**3))
      c1      = rho2m_f_4pi**2/qq2
      gamrho2 = rho2m_f_4pi*rho2g_f_4pi
     1         *sqrt(abs(c1*((1.d0-c2)/(c1-c2))**3))
      c1      = rho3m_f_4pi**2/qq2
      gamrho3 = rho3m_f_4pi*rho3g_f_4pi
     1         *sqrt(abs(c1*((1.d0-c2)/(c1-c2))**3))
      cbw   = dcmplx(rhom_4pi**2,0.d0)/dcmplx(rhom_4pi**2-qq2,-gamrho)
      cbw1  = dcmplx(rho1m_f_4pi**2,0.d0)
     1       /dcmplx(rho1m_f_4pi**2-qq2,-gamrho1)
      cbw2  = dcmplx(rho2m_f_4pi**2,0.d0)
     1       /dcmplx(rho2m_f_4pi**2-qq2,-gamrho2)
      cbw3  = dcmplx(rho3m_f_4pi**2,0.d0)
     1       /dcmplx(rho3m_f_4pi**2-qq2,-gamrho3)
c
      if(ij.eq.1)then
        beta1 = b1_a1_4pi
        beta2 = b2_a1_4pi
        beta3 = b3_a1_4pi
      elseif(ij.eq.2)then
        beta1 = b1_f0_4pi
        beta2 = b2_f0_4pi
        beta3 = b3_f0_4pi
      elseif(ij.eq.3)then
        beta1 = b1_om_4pi
        beta2 = b2_om_4pi
        beta3 = b3_om_4pi
      else
        write(6,*)'wrong function bwgrho_o call'
        stop
      endif
c
      bwgrho_o = (cbw + beta1*cbw1+beta2*cbw2+beta3*cbw3)
     1           /(1.d0+beta1+beta2+beta3)
      end
c ************************************************************************
      complex*16 function bwgrho_r(q1_2)
      include 'phokhara_10.0.inc'
c
      real*8 q1_2
      complex*16 BW_rho
c
      bwgrho_r = 
     1     BW_rho(rhom_4pi,rhog_4pi,q1_2,1)/dcmplx(rhom_4pi**2,0.d0)
     1   - BW_rho(rho1m_4pi,rho1g_4pi,q1_2,1)/dcmplx(rho1m_4pi**2,0.d0)
      end 
c*************************************************************************
c inner propagator in omega part
      complex*16 function Hrho(q1_2,q2_2,q3_2)
      include 'phokhara_10.0.inc'
c
      real*8 q1_2,q2_2,q3_2
      complex*16 cbw,cbw1,cbw2,BW_rho
c
      cbw = BW_rho(rhom_4pi,rhog_4pi,q1_2,1)
      cbw1 = BW_rho(rhom_4pi,rhog_4pi,q2_2,1)
      cbw2 = BW_rho(rhom_4pi,rhog_4pi,q3_2,1)
c
      Hrho = cbw + cbw1 + cbw2
      return
      end
c*************************************************************************
      complex*16 function bwga1(q1_2)
      include 'phokhara_10.0.inc'
c
      real*8 q1_2,ggm,gfun_4pi
c
      ggm = gfun_4pi(q1_2)*con_4pi
      bwga1 = dcmplx(a1m2_4pi,0.d0)/dcmplx(a1m2_4pi-q1_2,-ggm)
      return
      end
c ************************************************************************
      complex*16 function bwgf0(q1_2)
      include 'phokhara_10.0.inc'
c
      real*8 q1_2
      real*8 c1,c2,gamf0
c
      c2     = 4.d0*pim_4pi**2/q1_2
      c1     = f0m2_4pi/q1_2
      gamf0  = f0mg_4pi*sqrt(abs(c1*(1.d0-c2)/(c1-c2)))
c
      bwgf0 = dcmplx(f0m2_4pi,0.d0)/dcmplx(f0m2_4pi-q1_2,-gamf0)
      return
      end
c ************************************************************************
      complex*16 function bwg_om(q1_2)
      include 'phokhara_10.0.inc'
c
      real*8 q1_2
c
      bwg_om = dcmplx(omm2_4pi,0.d0)/dcmplx(omm2_4pi-q1_2,-ommg_4pi)
      end function
c*************************************************************************
c ---- complex dilogarithm -----------------------------------------------
c ------------------------------------------------------------------------
       complex*16 function cdilog(z)
       implicit none
       complex*16 z,zl,coef,dilog1,u,caux
       real*8 pi,sign
       integer n,i
       pi=3.141592653589793238462643d0                                  
       zl=z                                                             
       dilog1=dcmplx(pi**2/6.d0)                                        
       if(dreal(zl).eq.1.and.dimag(zl).eq.0.) then                      
          cdilog=dilog1                                                    
          return                                                           
       else if (cdabs(zl).lt.1.d-2) then   
          n=-40./dlog(cdabs(zl))                                           
          caux=(0.d0,0.d0)                                                 
          do i=1,n                                                         
             caux=caux+zl**i/dble(i**2)
          enddo                                                           
          cdilog=caux                                                      
          return                                                           
       else if(cdabs(zl).lt.1.) then                                    
          sign=1.d0
          coef=dcmplx(dble(0.))                                           
       else                                                            
          coef=-cdlog(-zl)**2/2.d0-dilog1                                 
          sign=-1.d0                                                      
          zl=1.d0/zl                                                      
       endif                                                          
       if(dreal(zl).gt.0.5) then                   
          coef=coef+sign*(dilog1-cdlog(zl)*cdlog(1.d0-zl))                
          sign=-sign                                                      
          zl=1.d0-zl                                                      
       else   
       endif  
       u=-cdlog(1.d0-zl)                                               
       cdilog=u-u**2/4.d0+u**3/36.d0-u**5/3600.d0+u**7/211680.d0       
     &  -u**9/10886400.d0+u**11*5.d0/2634508800.d0                     
       cdilog=cdilog-u**13*691.d0/2730.d0/6227020800.d0                
       cdilog=cdilog+u**15*7.d0/6.d0/1.307674368d12                    
       cdilog=cdilog-u**17*3617.d0/510.d0/3.5568742810d14              
       cdilog=cdilog+u**19*43867.d0/798.d0/1.2164510041d17              
       cdilog=cdilog-u**21*174611.d0/330.d0/5.1090942172d19            
       cdilog=sign*cdilog+coef                                         
       return                                                          
       end                                                             
c ========================================================================
c --- all about the histogrammes -----------------------------------------

      subroutine addiere(wgt,qq,i)
      include 'phokhara_10.0.inc'
      real*8 wgt,qq,qq_inv,q_sum(0:3)
      integer i,j
c
      if (i.eq.0) call addhisto(0,qq,wgt)  ! events with no photons
      if (i.eq.1) call addhisto(1,qq,wgt)  ! one photon events
      if (i.eq.2) call addhisto(2,qq,wgt)  ! two photon events
      if (i.eq.3) call addhisto(3,qq,wgt)  ! two photon events
      if (i.eq.4) call addhisto(4,qq,wgt)  ! two photon events
      if(i.eq.5) call addhisto(5,qq,wgt)
      if(i.eq.6) call addhisto(6,qq,wgt)
c      if(i.eq.6) call addhisto(6,qq,wgt)
c      if(i.eq.7) call addhisto(7,qq,wgt)
c      if(i.eq.8) call addhisto(8,qq,wgt)
c
      return
      end
c ------------------------------------------------------
c     create histograms                                 
c ------------------------------------------------------
      subroutine inithisto
      include 'phokhara_10.0.inc'
      integer i,j
      real*8 histo(0:20,200),error(0:20,200)
      common/histograms/histo,error
c --- book the histograms ----
      do i=0,20
        do j=1,200
         histo(i,j)= 0.d0
         error(i,j)= 0.d0
        enddo
      enddo
      return 
      end
c ------------------------------------------------------
c     add value to histo i at x                         
c ------------------------------------------------------
      subroutine addhisto(i,x,value)   
      include 'phokhara_10.0.inc'
      real*8 x,value,histo(0:20,200),error(0:20,200)
      integer i,j
      common/histograms/histo,error
c --- add --------------------
      j = 1+(x-xlow(i))/(xup(i)-xlow(i))*bins(i)
      if (j.ge.1.and.j.le.bins(i)) then 
         histo(i,j)=histo(i,j)+value
      endif
      end
c ------------------------------------------------------
c     save histograms
c ------------------------------------------------------
      subroutine endhisto()
      include 'phokhara_10.0.inc'
      real*8 h,histo(0:20,200),error(0:20,200),x,count_1
      integer i,j
      real*8 suma,sumaerr
      common/histograms/histo,error
c --- fill histograms --------

c      open(21,file='plik1.dat',status='new')
c      open(22,file='plik2.dat',status='new')
           do i=0,2     !=========================
          write(*,*)i
          do j=1,bins(i)
            x = xlow(i)+(j-.5d0)*(xup(i)-xlow(i))/dble(bins(i))
            if (count(i).ne.0.d0) then 
              error(i,j) = Mmax(i)*dSqrt((histo(i,j)/count(i)-
     &	                   (histo(i,j)/count(i))**2)/count(i))
              histo(i,j) = Mmax(i)/count(i)*histo(i,j)
              
c     dividing by interval width -> the result is delta sigma/delta q
c              histo(i,j)=histo(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c              error(i,j)=error(i,j)*dble(bins(i))/(xup(i)-xlow(i))
            else
              histo(i,j) = 0.d0
              error(i,j) = 0.d0
            endif
              write(*,*) x,histo(i,j),error(i,j)
c              if(i.eq.1) write (21,*) x,histo(i,j),error(i,j)
c              if(i.eq.2) write (22,*) x,histo(i,j),error(i,j)
          enddo
        enddo
c
          write(*,*)'0+1+2'
          do j=1,bins(1)
              x = xlow(1)+(j-.5d0)*(xup(1)-xlow(1))/dble(bins(1))
              write(*,*) x,histo(0,j)+histo(1,j)+histo(2,j)
     1                ,sqrt(error(0,j)**2+error(1,j)**2+error(2,j)**2)
          enddo
  202 FORMAT('',G20.10E3,G20.10E3,G20.10E3)
c----------------------------------------------------------------------
c angular distributions 
c      do i=3,4     !=========================
c          write(*,*)i
c          write(10,*)i
c          do j=1,bins(i)
c            x = xlow(i)+(j-.5d0)*(xup(i)-xlow(i))/dble(bins(i))
c            if (count(1).ne.0.d0) then 
c              histo(i,j) = Mmax(1)/count(1)*histo(i,j)
c              error(i,j) = Mmax(1)*dSqrt((histo(i,j)/count(1)-
c     &	                   (histo(i,j)/count(1))**2)/count(1))              
c     dividing by interval width -> the result is delta sigma/delta q
c              histo(i,j)=histo(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c              error(i,j)=error(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c            else
c              histo(i,j) = 0.d0
c              error(i,j) = 0.d0
c            endif
c              write(*,*) x,histo(i,j),error(i,j)
c              write(10,202) x,histo(i,j),error(i,j)
c              if(i.eq.1) write (21,*) x,histo(i,j),error(i,j)
c              if(i.eq.2) write (22,*) x,histo(i,j),error(i,j)
c          enddo
c        enddo

c        do i=5,6     !=========================
c          write(*,*)i
c          write(10,*)i
c          do j=1,bins(i)
c            x = xlow(i)+(j-.5d0)*(xup(i)-xlow(i))/dble(bins(i))
c            if (count(2).ne.0.d0) then 
c              histo(i,j) = Mmax(2)/count(2)*histo(i,j)
c              error(i,j) = Mmax(2)*dSqrt((histo(i,j)/count(2)-
c     &	                   (histo(i,j)/count(2))**2)/count(2))
c     dividing by interval width -> the result is delta sigma/delta q
c              histo(i,j)=histo(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c              error(i,j)=error(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c            else
c              histo(i,j) = 0.d0
c              error(i,j) = 0.d0
c            endif
c              write(*,*) x,histo(i,j),error(i,j)
c              write(10,202) x,histo(i,j),error(i,j)
c              if(i.eq.1) write (21,*) x,histo(i,j),error(i,j)
c              if(i.eq.2) write (22,*) x,histo(i,j),error(i,j)
c          enddo
c        enddo
c
c          write(*,*)'pi+'
c          write(10,*)'pi+'
c          do j=1,bins(3)
c              x = xlow(3)+(j-.5d0)*(xup(3)-xlow(3))/dble(bins(3))
c              write(*,*) x,histo(3,j)+histo(5,j)
c     1                ,sqrt(error(3,j)**2+error(5,j)**2)
c              write(10,202) x,histo(3,j)+histo(5,j)
c     1                ,sqrt(error(3,j)**2+error(5,j)**2)
c          enddo

c         write(*,*)'pi-'
c          write(10,*)'pi-'
c          do j=1,bins(3)
c              x = xlow(3)+(j-.5d0)*(xup(3)-xlow(3))/dble(bins(3))
c              write(*,*) x,histo(4,j)+histo(6,j)
c     1                ,sqrt(error(4,j)**2+error(6,j)**2)
c              write(10,202) x,histo(4,j)+histo(6,j)
c     1                ,sqrt(error(4,j)**2+error(6,j)**2)
c          enddo
c  202 FORMAT('',G20.12E3,G20.12E3,G20.12E3)

          
c polar angle distributions
c        do i=3,5     !=========================
c          write(*,*)i
c          do j=1,bins(i)
c            x = xlow(i)+(j-.5d0)*(xup(i)-xlow(i))/dble(bins(i))
c            if(count(1).ne.0.d0)then

c              error(i,j) = Mmax(1)*dSqrt((histo(i,j)/count(1)-
c     &	                   (histo(i,j)/count(1))**2)/count(1))
c              histo(i,j) = Mmax(1)/count(1)*histo(i,j)
c
c     dividing by interval width -> the result is delta N/delta q
c            histo(i,j)= histo(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c            error(i,j)= error(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c            else
c              histo(i,j) = 0.d0
c              error(i,j) = 0.d0
c           endif
c              write(*,*) x,histo(i,j),error(i,j)
c              if(i.eq.1) write (21,*) x,histo(i,j),error(i,j)
c              if(i.eq.2) write (22,*) x,histo(i,j),error(i,j)
c          enddo
c        enddo
c
 
c----------- histogramowanie ebeam1, ebeam2, ebeam1+ebeam2---------------------
c          do i=6,8     !=========================
c          write(*,*)i
c          do j=1,bins(i)
c            x = xlow(i)+(j-.5d0)*(xup(i)-xlow(i))/dble(bins(i))
c            if(count(1).ne.0.d0)then

c              error(i,j) = Mmax(1)*dSqrt((histo(i,j)/count(1)-
c     &	                   (histo(i,j)/count(1))**2)/count(1))
c              histo(i,j) = Mmax(1)/count(1)*histo(i,j)
c
c     dividing by interval width -> the result is delta N/delta q
c            histo(i,j)= histo(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c            error(i,j)= error(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c            else
c              histo(i,j) = 0.d0
c              error(i,j) = 0.d0
c            endif
c              write(*,*) x,histo(i,j),error(i,j)
c              if(i.eq.1) write (21,*) x,histo(i,j),error(i,j)
c              if(i.eq.2) write (22,*) x,histo(i,j),error(i,j)
c          enddo
c        enddo      

      return
      end
c-------------------------------------------------------
c ========================================================================                                                                                  
c --- all about the histogrammes MC integrand ----------------------------                                                                                  
      subroutine addiereMC(wgt,qq,i)
      include 'phokhara_10.0.inc'
      real*8 wgt,qq,qq_inv,q_sum(0:3)
      integer i,j
c                                                                                                                                                           
      if (i.eq.0) call addhistoMC(0,qq,wgt)  ! events with no photons                                                                                       
      if (i.eq.1) call addhistoMC(1,qq,wgt)  ! one photon events                                                                                            
      if (i.eq.2) call addhistoMC(2,qq,wgt)  ! two photon events 
      if (i.eq.3) call addhistoMC(3,qq,wgt)  ! events with no photons                                                                                       
      if (i.eq.4) call addhistoMC(4,qq,wgt)  ! one photon events                                                                                            
      if (i.eq.5) call addhistoMC(5,qq,wgt)  ! two photon events     
      if (i.eq.6) call addhistoMC(6,qq,wgt)  ! two photon events                                                                                      c                                                                                                                                                  
      return
      end
c ------------------------------------------------------                                                                                                    
c     create histograms MC integrand            
c----------------------------------------------- 
      subroutine inithistoMC
      include 'phokhara_10.0.inc'
      integer i,j
      real*8 histoMC(0:20,200),errorMC(0:20,200)
      common/histogramsMC/histoMC,errorMC
c --- book the histograms ----                                                                                                                              
      do i=0,20
        do j=1,200
         histoMC(i,j)= 0.d0
         errorMC(i,j)= 0.d0
        enddo
      enddo
      return
      end
c ------------------------------------------------------
c  add value to histo i at x                                                                                                                             
c ------------------------------------------------------                                                                                                    
      subroutine addhistoMC(i,x,value)
      include 'phokhara_10.0.inc'
      real*8 x,value,histoMC(0:20,200),errorMC(0:20,200)
      integer i,j
      common/histogramsMC/histoMC,errorMC
c --- add --------------------                                                                                                                              
      j = 1+(x-xlow(i))/(xup(i)-xlow(i))*bins(i)
      if (j.ge.1.and.j.le.bins(i)) then
         histoMC(i,j)=histoMC(i,j)+value
         errorMC(i,j)=errorMC(i,j)+value**2
      endif
      end
c ------------------------------------------------------                                                                                                    
c     save histograms - MC integrand                                                                                                                
c ------------------------------------------------------                                                                                                    
      subroutine endhistoMC()
      include 'phokhara_10.0.inc'
      real*8 h,histoMC(0:20,200),errorMC(0:20,200),x,count_1
      integer i,j
      common/histogramsMC/histoMC,errorMC
c --- fill histograms --------                                                                                                                              
c
      write(*,*)'Histograms Monte Carlo integrand' 
        do i=0,2     !=========================                                                                                                             
          write(*,*)i
          do j=1,bins(i)
            x = xlow(i)+(j-.5d0)*(xup(i)-xlow(i))/dble(bins(i))
            if (count(i).ne.0.d0) then
              histoMC(i,j) = histoMC(i,j)/count(i)
              errorMC(i,j) = dsqrt(dabs(errorMC(i,j)/count(i)/count(i)
     &               - histoMC(i,j)*histoMC(i,j)/count(i)))
c                                                                                        
            else
              histoMC(i,j) = 0.d0
              errorMC(i,j) = 0.d0
            endif
              write(*,*) x,histoMC(i,j),errorMC(i,j)
                                               
          enddo
        enddo
c                                                                                                                                                           
          write(*,*)'0+1+2'
          do j=1,bins(1)
              x = xlow(1)+(j-.5d0)*(xup(1)-xlow(1))/dble(bins(1))
              write(*,*) x,histoMC(0,j)+histoMC(1,j)+histoMC(2,j)
     1            ,sqrt(errorMC(0,j)**2+errorMC(1,j)**2+errorMC(2,j)**2)
          enddo
  202 FORMAT('',G20.10E3,G20.10E3,G20.10E3)
c----------------------------------------------------------------------
c angular distributions 
c      do i=3,4     !=========================
c          write(*,*)i
c          write(10,*)i
c          do j=1,bins(i)
c            x = xlow(i)+(j-.5d0)*(xup(i)-xlow(i))/dble(bins(i))
c            if (count(1).ne.0.d0) then 
c              histoMC(i,j) = histoMC(i,j)/count(1)
c              errorMC(i,j) = dsqrt(dabs(errorMC(i,j)/count(1)/count(1)
c     &               - histoMC(i,j)*histoMC(i,j)/count(1))) 
c     dividing by interval width -> the result is delta sigma/delta q
c              histo(i,j)=histo(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c              error(i,j)=error(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c            else
c              histoMC(i,j) = 0.d0
c              errorMC(i,j) = 0.d0
c            endif
c              write(*,*) x,histoMC(i,j),errorMC(i,j)
c              write(10,202) x,histoMC(i,j),errorMC(i,j)
c              if(i.eq.1) write (21,*) x,histo(i,j),error(i,j)
c              if(i.eq.2) write (22,*) x,histo(i,j),error(i,j)
c          enddo
c        enddo

c        do i=5,6     !=========================
c          write(*,*)i
c          write(10,*)i
c          do j=1,bins(i)
c            x = xlow(i)+(j-.5d0)*(xup(i)-xlow(i))/dble(bins(i))
c            if (count(2).ne.0.d0) then 
c              histoMC(i,j) = histoMC(i,j)/count(2)
c              errorMC(i,j) = dsqrt(dabs(errorMC(i,j)/count(2)/count(2)
c     &               - histoMC(i,j)*histoMC(i,j)/count(2))) 
c     dividing by interval width -> the result is delta sigma/delta q
c              histo(i,j)=histo(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c              error(i,j)=error(i,j)*dble(bins(i))/(xup(i)-xlow(i))
c            else
c              histoMC(i,j) = 0.d0
c              errorMC(i,j) = 0.d0
c            endif
c              write(*,*) x,histoMC(i,j),errorMC(i,j)
c              write(10,202) x,histoMC(i,j),errorMC(i,j)
c              if(i.eq.1) write (21,*) x,histo(i,j),error(i,j)
c              if(i.eq.2) write (22,*) x,histo(i,j),error(i,j)
c          enddo
c        enddo
c
c          write(*,*)'pi+'
c          write(10,*)'pi+'
c          do j=1,bins(3)
c              x = xlow(3)+(j-.5d0)*(xup(3)-xlow(3))/dble(bins(3))
c              write(*,*) x,histoMC(3,j)+histoMC(5,j)
c     1                ,sqrt(errorMC(3,j)**2+errorMC(5,j)**2)
c              write(10,202) x,histoMC(3,j)+histoMC(5,j)
c     1                ,sqrt(errorMC(3,j)**2+errorMC(5,j)**2)
c          enddo

c         write(*,*)'pi-'
c          write(10,*)'pi-'
c          do j=1,bins(3)
c              x = xlow(3)+(j-.5d0)*(xup(3)-xlow(3))/dble(bins(3))
c              write(*,*) x,histoMC(4,j)+histoMC(6,j)
c     1                ,sqrt(errorMC(4,j)**2+errorMC(6,j)**2)
c              write(10,202) x,histoMC(4,j)+histoMC(6,j)
c     1                ,sqrt(errorMC(4,j)**2+errorMC(6,j)**2)
c          enddo

c                                                                                                                                                           
      return
      end
c ************************************************************************
c from here helicity amplitudes: H.C. 04.07.2001
c muons added : H.C. 04.10.2001
c 
c p1 - positron four momenta, p2 electron four momenta
c ************************************************************************
c one photon
c ************************************************************************
      real*8 function helicityampLO(qq)
      include 'phokhara_10.0.inc'     
      real*8 qq,rk1(4),q(4),dps,amp_h,ampLO,ampLOm,ampLOp
      complex*16 gam(4),gammu(4,2,2),v1(2,2),v2(2,2),up1(2,2),up2(2,2)
     1          ,gammu_ma(4,2,2)
      integer i1,ic1,ic2
      common/pedf/rk1,dps
c
      call gam1(gam,gammu,gammu_ma,v1,v2,up1,up2,qq)
      
c
      do i1=1,4
         rk1(i1) = momenta(3,i1-1)
         q(i1)   = momenta(5,i1-1)
      enddo
c --- muons ---
      if(pion.eq.0)then
       dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
       dps = dps*dSqrt(1.d0-4.d0*mmu*mmu/qq)/(32.d0*pi*pi) 
c      
       amp_h = ((4.d0*pi*alpha)**3)
     1 *ampLOm(qq,rk1,gam,gammu,gammu_ma,v1,v2,up1,up2,q)/4.d0 
c --- kaons K+K-
      elseif(pion.eq.6)then
       dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
       dps = dps*dSqrt(1.d0-4.d0*mKp*mKp/qq)/(32.d0*pi*pi) 
       amp_h = 
     1 ((4.d0*pi*alpha)**3)*ampLO(qq,rk1,gam,q)/4.d0

c ------ protons
       elseif(pion.eq.4)then
       dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
       dps = dps*dSqrt(1.d0-4.d0*mp*mp/qq)/(32.d0*pi*pi) 
c      
       amp_h = ((4.d0*pi*alpha)**3)
     1 *ampLOp(qq,rk1,gam,gammu,gammu_ma,v1,v2,up1,up2,q)/4.d0

      else 
c --- two pions ---
       dps = (1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
       dps = dps*dSqrt(1.d0-4.d0*mpi*mpi/qq)/(32.d0*pi*pi) 
       amp_h = 
     1 ((4.d0*pi*alpha)**3)*ampLO(qq,rk1,gam,q)/4.d0
c 
      endif
       helicityampLO = amp_h*dps
      return
      end
c---------------------------------------------------------------------
c-----------------FSR virtual+soft correction -pions and kaons K^+K^-
c---------------------------------------------------------------------
      real*8 function vertLO(qq)
      include 'phokhara_10.0.inc' 
      Complex*16 qlI1,qlI2,qlI3,qlI4,Ival(-2:0),A02,B012,B045,B025,
     1 amp0,amp2,amp8,amp10,ampc,C02fun,C0count
      complex*16 cdilog,pod1,pod2,wsp1,wsp2,fsr_no_emfull
      real*8 betapi,mpi2,qq,vert,pod3
      real*8 softint_q1q2,softint_qjqj,soft
      real*8 softint_q1q2_f,softint_qjqj_f,soft_f
      real*8 s15,musq,betan,cc,vertLO_epjc,tt,soft_notes,cci,pij,sl,
     1 slqjqj,mom1dl,mom2dl,q1(0:3),q2(0:3),EE,theta1,phi1,qmo,
     2 soft_notes2,soft_notes1,soft_f1,soft_f2,berends,berends2,vertLO2,
     3 temp
      complex*16 C02new,C02fin,C02inf,fsr_no_em2
      

      if(pion.eq.1)then
        mpi2 = mpi**2
      elseif(pion.eq.6)then
        mpi2 = mKp**2
      endif

      betapi = Sqrt(1.d0 - 4.d0*mpi2/qq)
      pod1 = dcmplx(2.d0*betapi/(1.d0+betapi),0.d0)
      pod3 = qq*(1.d0+betapi)**2/4.d0/mpi2
      tt=(1.d0-betapi)/(1.d0+betapi)

     
      vertLO = alpha/pi*( (3.d0*qq-4.d0*mpi2)/qq/betapi
     1       * Log(pod3) - 2.d0 
     2       - Log(qq/mpi2)
     3       - (1.d0+betapi**2)/betapi
     4       * ( Log(pod3)* Log( (1.d0+betapi)/2.d0)
     5        + 2.d0*cdilog(pod1)
     6        - pi**2/2.d0 ) ) 
     7       + 2.d0*alpha/pi*( (1.d0+betapi**2)/2.d0/betapi*Log(pod3)
     8       - 1.d0 )*( Log(2.d0*w) + 1.d0 
     9                  + 1.d0/(1.d0-Sp/qq)*Log(Sp/qq) )

c      print*, 'old V+S=',vertLO
c      temp=vertLO

        if(nlo2.eq.1)then 
           vertLO2=alpha/pi*( (3.d0*qq-4.d0*mpi2)/qq/betapi
     1       * Log(pod3) - 2.d0 
     2       - Log(qq/mpi2)
     3       - (1.d0+betapi**2)/betapi
     4       * ( Log(pod3)* Log( (1.d0+betapi)/2.d0)
     5        + 2.d0*cdilog(pod1)
     6        - pi**2/2.d0 ) ) 
c  soft part from Nucl.Phys B57 (1973) 381-400 (eq.(29)) without terms proportional to Log(2w)
       soft=-(1.d0/betapi*log(tt)+(1.d0+betapi**2)/betapi*(
     -   cdilog(dcmplx(2.d0*betapi/(1.d0+betapi),0.d0))        
     -  +log(tt)**2/4.d0 ))*alpha/pi

          call softintegral_q1q2_sub(softint_q1q2_f)
          call softintegral_qjqj_sub(softint_qjqj_f)
          soft_f=2.d0*(softint_qjqj_f-softint_q1q2_f)
          vertLO=vertLO2-soft
c          print*, 'vertlo',vertLO
c          print*,'NEW V+S=',vertLO+soft_f,
c     - (1.d0-(vertLO+soft_f)/(temp))*100.d0,'%'
       endif
      return
      end
c----------------------------------------------------------
c-----------------FSR virtual+soft correction -muons ------
c----------------------------------------------------------
      real*8 function vertLO_mu(qq)
      include 'phokhara_10.0.inc'     
      complex*16 cdilog,pod1,pod2,wsp1,wsp2
      real*8 betamu,mmu2,qq,vert,pod3

      mmu2 = mmu**2
      betamu = Sqrt(1.d0 - 4.d0*mmu2/qq)
      pod1 = dcmplx(2.d0*betamu/(1.d0+betamu),0.d0)
      pod3 = qq*(1.d0+betamu)**2/4.d0/mmu2

      vertLO_mu = alpha/pi*( (2.5d0*qq-6.d0*mmu2)/qq/betamu
     1       * Log(pod3) - 2.d0 
     2       - Log(qq/mmu2)
     3       - (1.d0+betamu**2)/betamu
     4       * ( Log(pod3)* Log( (1.d0+betamu)/2.d0)
     5        + 2.d0*cdilog(pod1)
     6        - pi**2/2.d0 ) ) 
     7       + 2.d0*alpha/pi*( (1.d0+betamu**2)/2.d0/betamu*Log(pod3)
     8       - 1.d0 )*( Log(2.d0*w) + 1.d0 
     9                  + 1.d0/(1.d0-Sp/qq)*Log(Sp/qq) )
      return
      end
c----------------------------------------------------------
c-----------------FSR virtual correction F2 part -muons ---
c----------------------------------------------------------
      complex*16 function vert_ma(qq)
      include 'phokhara_10.0.inc'     
      real*8 betamu,mmu2,qq,pod3

      mmu2 = mmu**2
      betamu = Sqrt(1.d0 - 4.d0*mmu2/qq)
      pod3 = qq*(1.d0+betamu)**2/4.d0/mmu2
c
      vert_ma = alpha/pi*mmu/2.d0/qq/betamu*dcmplx(-Log(pod3),pi)
c
      return
      end
c*****************************************************************************
c----------------------------------------------------------
      real*8 function vertLO_mu2(qq)
      include 'phokhara_10.0.inc'     
      complex*16 cdilog,pod1,pod2,wsp1,wsp2
      real*8 betamu,mmu2,qq,vert,pod3

      mmu2 = mmu**2
      betamu = dSqrt(1.d0 - 4.d0*mmu2/qq)
      pod1 = dcmplx(2.d0*betamu/(1.d0+betamu),0.d0)
      pod3 = qq*(1.d0+betamu)**2/4.d0/mmu2

      vertLO_mu2 = 2.d0*( (2.5q0*qq-6.d0*mmu2)/qq/betamu
     1       * dLog(pod3) - 2.d0 
     2       - dLog(qq/mmu2)
     3       - (1.d0+betamu**2)/betamu
     4       * ( dLog(pod3)* dLog( (1.d0+betamu)/2.d0)
     5        + 2.d0*cdilog(pod1)
     6        - pi**2/2.d0 ) ) 
     7    +2.d0*(-dlog(pod3)/betamu + (1.q0+betamu**2)/betamu   
     8     *((dlog(pod3))**2/4.d0+cdilog(pod1)))  
     9 - (2.d0 -(1.d0+betamu**2)/betamu*dlog(pod3) )*dlog(Sp/qq)
      return
      end
c----------------------------------------------------------
c-----------------FSR virtual correction F2 part -muons ---
c----------------------------------------------------------
      complex*16 function vert_ma2(qq)
      include 'phokhara_10.0.inc'     
      real*8 betamu,mmu2,qq,pod3

      mmu2 = mmu**2
      betamu = dSqrt(1.d0 - 4.d0*mmu2/qq)
      pod3 = qq*(1.q0+betamu)**2/4.d0/mmu2
c
      vert_ma2 = mmu/qq/betamu*dcmplx(-dlog(pod3),pi)
c
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      real*8 function ampLO(qq,rk1,gam,q)
      include 'phokhara_10.0.inc'     
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam,cvac_qq,dggvap
      complex*16 epsk1(2,4),epsk2(2,4),BW_om,vacpol_and_nr
      complex*16 gam(4),gam_ma(4),eck1(4),eck2(4)
      complex*16 ma(2,2),mb(2,2),ddpl(2,2),ddmi(2,2)
     1          ,ma_ma(2,2),mb_ma(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      integer i
      real*8 qq,dme,el_m2,ebppb,vacuumpolarization,vertLO,ver_f
      real*8 rk1(4),rk2(4),p1(4),p2(4),q(4),aa_phi,mm_ph,gg_ph
      real*8 amp_fullLO
      real*8 softint_p1p2,softint_pjpj,softint_q1q2,softint_qjqj,
     1 softint_sum
      complex*16 apl(2,2),ami(2,2),apl2(2,2),ami2(2,2)
      real*8 testISR,dps,trace
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri1/ma,mb,ma_ma,mb_ma
      common /cp1p2/p1,p2,dme,el_m2
      common/param_PFF/aa_phi,mm_ph,gg_ph
c     
      ampLO = 0.d0
      testISR=0.d0
      call pol_vec(rk1,epsk1)
      call skalar1LO(rk1)
      call skalar1aLO(gam,gam_ma)
      
      ebppb = p1(1)+p1(4)

      if(fsrnlo.eq.1)then
       ver_f = vertLO(qq)
      endif
c
c vacuum polarization
c
      cvac_qq = vacpol_and_nr(qq)
c
c sum over photon polarizations
c
      do i=1,2
                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4)
c
           call skalar2LO(rk1,eck1)
           call blocksLO(qq)
           call ddvec(rk1,eck1,uupp1,uupp2,vv1,vv2,ddpl,ddmi,qq)
c
c ddmi is an A status matrix; ddpl is a B status matrix 
c
c the FSR - corrected if fsrnlo=1 (ver_s calculated in subroutine input)
c           for fsrnlo = 0 ver_s =0
c
      ampLO = ampLO
     1         +((dme*cdabs(ddpl(1,1)-ddmi(1,1)))**2
     1         +(dme*cdabs(ddpl(2,2)-ddmi(2,2)))**2
     2         +(cdabs(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1)))**2
     3         +(cdabs(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2)
     4         *(1.d0 + ver_s)*cdabs(cvac_s)**2
c ISR+correction FSR 
        if(fsrnlo.eq.1)then
        ampLO = ampLO + ( (dme*cdabs(mb(1,1)-ma(1,1)))**2
     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
     4          * ver_f* cdabs(cvac_qq)**2
        endif
c
c the FSR x ISR 
      if(fsr.eq.2)then
      ampLO = ampLO
     4         +2.d0*dreal(
     5    dconjg(dme*(mb(1,1)-ma(1,1))*cvac_qq)
     5              *dme*(ddpl(1,1)-ddmi(1,1))*cvac_s
     6   +dconjg(dme*(mb(2,2)-ma(2,2))*cvac_qq)
     6              *dme*(ddpl(2,2)-ddmi(2,2))*cvac_s
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq)*
     8    (-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq)*
     1    (ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s )
c
      endif
c
c FSR x (ISR+correction) + ISR x (FSR+corrections)
      if((nlo2.eq.1).and.(pion.eq.1))then
! vator 2 of this interference taken into account in ver_f and ver_s        
!ST
         ampLO = ampLO
     4         +dreal(
     5    dconjg(dme*(mb(1,1)-ma(1,1))*cvac_qq)
     5              *dme*(ddpl(1,1)-ddmi(1,1))*cvac_s
     6   +dconjg(dme*(mb(2,2)-ma(2,2))*cvac_qq)
     6              *dme*(ddpl(2,2)-ddmi(2,2))*cvac_s
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq)*
     8    (-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq)*
     1    (ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s
     2    *(ver_f+ver_s) )

         call NLO_ISR_no_mass_terms(qq,apl,ami,apl2,ami2,rk1,eck1)
         
c        testISR=testISR+dreal(
c     5    dconjg(dme*(mb(1,1)-ma(1,1))*cvac_qq)
c     5              *dme*(apl(1,1)-ami(1,1))*cvac_qq
c     6   +dconjg(dme*(mb(2,2)-ma(2,2))*cvac_qq)
c     6              *dme*(apl(2,2)-ami(2,2))*cvac_qq
c     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq)*
c     8    (-ebppb*ami(2,1)+el_m2/ebppb*apl(2,1))*cvac_qq
c     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq)*
c     1    (ebppb*apl(1,2)-el_m2/ebppb*ami(1,2))*cvac_qq
c     2     )


       ampLO=ampLO+dreal(
     5    dconjg(dme*(ddpl(1,1)-ddmi(1,1))*cvac_s)
     5              *dme*(apl(1,1)-ami(1,1))*cvac_qq
     6   +dconjg(dme*(ddpl(2,2)-ddmi(2,2))*cvac_s)
     6              *dme*(apl(2,2)-ami(2,2))*cvac_qq
     7   +dconjg((-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s)*
     8    (-ebppb*ami(2,1)+el_m2/ebppb*apl(2,1))*cvac_qq
     9   +dconjg((ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s)*
     1    (ebppb*apl(1,2)-el_m2/ebppb*ami(1,2))*cvac_qq
     2     )

       
            
      endif


c     
       dps=(1.d0-qq/Sp)/(32.d0*pi*pi)        ! Phase space factors
       dps = dps*dSqrt(1.d0-4.d0*mpi*mpi/qq)/(32.d0*pi*pi)
      enddo
c      print*,'new isr=', (4.d0*pi*alpha)**3*(testISR)/4.d0*dps,qq
c
      return
      end
c********************************************************************
c used for muonic mode
c
      real*8 function ampLOm(qq,rk1,gam,gammu,gammu_ma,v1,v2,up1,up2,q)
c
      include 'phokhara_10.0.inc'     
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam,ver_ma,vert_ma
      complex*16 epsk1(2,4),epsk2(2,4),cvac_qq,dggvap
      complex*16 gam(4),eck1(4),eck2(4),gammu(4,2,2),gammu_ma(4,2,2)
     1          ,v1(2,2),v2(2,2),up1(2,2),up2(2,2),gam_ma(4)
      complex*16 ma(2,2),mb(2,2),ddpl(2,2),ddmi(2,2)
     1          ,ma_ma(2,2),mb_ma(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 BW_om,vacpol_and_nr
      integer i,ic1,ic2,imu
      real*8 qq,dme,el_m2,ebppb,vacuumpolarization,ver_f
      real*8 rk1(4),rk2(4),p1(4),p2(4),q(4)
      real*8 vertLO_mu,aa_phi,mm_ph,gg_ph
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri1/ma,mb,ma_ma,mb_ma
      common /cp1p2/p1,p2,dme,el_m2
      common/param_PFF/aa_phi,mm_ph,gg_ph
c     
      ampLOm = 0.d0
      call pol_vec(rk1,epsk1)
      ebppb = p1(1)+p1(4)
      call skalar1LO(rk1)

      if(fsrnlo.eq.1)then
       ver_f  = vertLO_mu(qq)
       ver_ma = vert_ma(qq)
      endif
c
c vacuum polarization
c
      cvac_qq = vacpol_and_nr(qq)
c
c sum over mu+,mu- polarizations
c
      do ic1=1,2
        do ic2=1,2
c
c the spinors
c
         uupp2(1,1) = up2(1,ic1)
         uupp2(1,2) = up2(2,ic1)
         uupp1(1,1) = up1(1,ic1)
         uupp1(1,2) = up1(2,ic1)
    
         vv1(1,1) = v1(1,ic2)                 
         vv1(2,1) = v1(2,ic2)                 
         vv2(1,1) = v2(1,ic2)                 
         vv2(2,1) = v2(2,ic2)                 
c
c now gam is the muon current (no emission)
c
       do imu=1,4
        gam(imu) = gammu(imu,ic1,ic2)
        gam_ma(imu) = gammu_ma(imu,ic1,ic2)
       enddo
      call skalar1aLO(gam,gam_ma)
c
c sum over photon polarizations
c
          do i=1,2
c                  eck1(1)=rk1(1)
c                  eck1(2)=rk1(2)
c                  eck1(3)=rk1(3)
c                  eck1(4)=rk1(4)
                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4)
c
           call skalar2LO(rk1,eck1)
           call blocksLO(qq)
           call ddvec(rk1,eck1,uupp1,uupp2,vv1,vv2,ddpl,ddmi,qq)
c
c ddmi is an A status matrix; ddpl is a B status matrix 
c
c the FSR - corrected if fsrnlo=1 (ver_s calculated in subroutine input)
c           for fsrnlo = 0 ver_s =0
c
      ampLOm = ampLOm
     1         +( (dme*cdabs(ddpl(1,1)-ddmi(1,1)))**2
     1         +(dme*cdabs(ddpl(2,2)-ddmi(2,2)))**2
     2         +(cdabs(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1)))**2
     3         +(cdabs(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2 )
     4         *(1.d0 + ver_s)*cdabs(cvac_s)**2
c
c ISR+correction FSR 
c in the amplitudes 'magnetic' amplitude added at difference from pion case
c 
        if(fsrnlo.eq.1)then
        ampLOm = ampLOm + ( (dme*cdabs(mb(1,1)-ma(1,1)))**2
     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
     4          * ver_f* cdabs(cvac_qq)**2
      ampLOm = ampLOm
     4         +2.d0*dreal(ver_ma*(
     5    dconjg(dme*(mb(1,1)-ma(1,1)))*dme*(mb_ma(1,1)-ma_ma(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))*dme*(mb_ma(2,2)-ma_ma(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ma_ma(2,1)+el_m2/ebppb*mb_ma(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1    (ebppb*mb_ma(1,2)-el_m2/ebppb*ma_ma(1,2))))
     2   * cdabs(cvac_qq)**2
        endif
c
      if(fsr.eq.2)then
c
c the FSR x ISR 
      ampLOm = ampLOm
     4         +2.d0*dreal(
     5    dconjg(dme*(mb(1,1)-ma(1,1))*cvac_qq)
     5     *dme*(ddpl(1,1)-ddmi(1,1))*cvac_s
     6   +dconjg(dme*(mb(2,2)-ma(2,2))*cvac_qq)
     6     *dme*(ddpl(2,2)-ddmi(2,2))*cvac_s
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq)*
     8    (-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq)*
     1    (ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s)
c
      endif
c
          enddo
        enddo
      enddo
c
      return
      end
c*******************************************************************
      real*8 function born_mu(qq,rk1,gam,gammu,gammu_ma,v1,v2,up1,up2,q)
c                                                                                                                                                           
      include 'phokhara_10.0.inc'
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam,ver_ma,vert_ma
      complex*16 epsk1(2,4),epsk2(2,4),cvac_qq,dggvap
      complex*16 gam(4),eck1(4),eck2(4),gammu(4,2,2),gammu_ma(4,2,2)
     1          ,v1(2,2),v2(2,2),up1(2,2),up2(2,2),gam_ma(4)
      complex*16 ma(2,2),mb(2,2),ddpl(2,2),ddmi(2,2)
     1          ,ma_ma(2,2),mb_ma(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 BW_om,vacpol_and_nr
      integer i,ic1,ic2,imu
      real*8 qq,dme,el_m2,ebppb,vacuumpolarization,ver_f
      real*8 rk1(4),rk2(4),p1(4),p2(4),q(4)
      real*8 vertLO_mu,aa_phi,mm_ph,gg_ph
c                                                                                                                                                           
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri1/ma,mb,ma_ma,mb_ma
      common /cp1p2/p1,p2,dme,el_m2
      common/param_PFF/aa_phi,mm_ph,gg_ph
c
c                                                                                                                                                           
      born_mu = 0.d0
      call pol_vec(rk1,epsk1)
      ebppb = p1(1)+p1(4)
      call skalar1LO(rk1)
c                                                                                                                                                           
c vacuum polarization                                                                                                                                       
      cvac_qq = vacpol_and_nr(qq)
c                                                                                                                                                           
c sum over mu+,mu- polarizations                                                                                                                            
c                                                                                                                                                           
      do ic1=1,2
        do ic2=1,2
c                                                                                                                                                           
c the spinors                                                                                                                                               
c                                                                                                                                                           
         uupp2(1,1) = up2(1,ic1)
         uupp2(1,2) = up2(2,ic1)
         uupp1(1,1) = up1(1,ic1)
         uupp1(1,2) = up1(2,ic1)
c
         vv1(1,1) = v1(1,ic2)
         vv1(2,1) = v1(2,ic2)
         vv2(1,1) = v2(1,ic2)
         vv2(2,1) = v2(2,ic2)
c                                                                                                                                                           
c now gam is the muon current (no emission)                                                                                                                 
c                                                                                                                                                           
       do imu=1,4
        gam(imu) = gammu(imu,ic1,ic2)
        gam_ma(imu) = gammu_ma(imu,ic1,ic2)
       enddo
      call skalar1aLO(gam,gam_ma)
c sum over photon polarizations
           do i=1,2
                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4)
c                                                                                                                                                           
           call skalar2LO(rk1,eck1)
           call blocksLO(qq)
           call ddvec(rk1,eck1,uupp1,uupp2,vv1,vv2,ddpl,ddmi,qq)

      born_mu = born_mu
     1   + ((dme*cdabs(mb(1,1)-ma(1,1)))**2
     2   + (dme*cdabs(mb(2,2)-ma(2,2)))**2
     3   + (cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     4   + (cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2)
     5        *cdabs(cvac_qq)**2




c                                                                                                                                                           
c ddmi is an A type matrix; ddpl is a B type matrix                                                                                                         
c                                                                                                                                                           
c the FSR                                                                                    
                                                                                                             
c 
      if(fsr.ge.1)then                                                                                                                             
      born_mu = born_mu
     1         +( (dme*cdabs(ddpl(1,1)-ddmi(1,1)))**2
     1         +(dme*cdabs(ddpl(2,2)-ddmi(2,2)))**2
     2         +(cdabs(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1)))**2
     3         +(cdabs(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2 )
     4         *cdabs(cvac_s)**2
      endif
c                                                                                                                                                           
c ISR+correction FSR                                                                                                                                        
      if(fsr.eq.2)then
c                                                                                                                                                           
c the FSR x ISR                                                                                                                                             
      born_mu = born_mu
     4         +2.d0*dreal(
     5    dconjg(dme*(mb(1,1)-ma(1,1))*cvac_qq)
     5     *dme*(ddpl(1,1)-ddmi(1,1))*cvac_s
     6   +dconjg(dme*(mb(2,2)-ma(2,2))*cvac_qq)
     6     *dme*(ddpl(2,2)-ddmi(2,2))*cvac_s
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq)*
     8    (-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq)*
     1    (ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s)
c                                                                                                                                                           
      endif
c                                                                                                                                                           
          enddo
        enddo
      enddo
c                                                                                                                                                           
      return
      end
c********************************************************************
c used for muonic mode part2
c
      real*8 function
     &  ampLOm2(qq,rk1,gam,gammu,gammu_ma,v1,v2,up1,up2,q)
c
      include 'phokhara_10.0.inc'     
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam,ver_ma,vert_ma2
      complex*16 epsk1(2,4),epsk2(2,4),cvac_qq,dggvap
      complex*16 gam(4),eck1(4),eck2(4),gammu(4,2,2),gammu_ma(4,2,2)
     1          ,v1(2,2),v2(2,2),up1(2,2),up2(2,2),gam_ma(4)
      complex*16 ma(2,2),mb(2,2),ddpl(2,2),ddmi(2,2)
     1          ,ma_ma(2,2),mb_ma(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 BW_om,vacpol_and_nr
      integer i,ic1,ic2,imu
      real*8 qq,dme,el_m2,ebppb,vacuumpolarization,ver_f
      real*8 rk1(4),rk2(4),p1(4),p2(4),q(4)
      real*8 vertLO_mu2,aa_phi,mm_ph,gg_ph
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri1/ma,mb,ma_ma,mb_ma
      common /cp1p2/p1,p2,dme,el_m2
      common/param_PFF/aa_phi,mm_ph,gg_ph

c     
      ampLOm2 = 0.q0

      call pol_vec(rk1,epsk1)
      ebppb = p1(1)+p1(4)
      call skalar1LO(rk1)

c      if(fsrnlo.eq.1)then
       ver_f  = vertLO_mu2(qq)
       ver_ma = vert_ma2(qq)
c      endif


c
c vacuum polarization
c
      cvac_qq = vacpol_and_nr(qq)
c
c sum over mu+,mu- polarizations
c
      do ic1=1,2
        do ic2=1,2
c
c the spinors
c
         uupp2(1,1) = up2(1,ic1)
         uupp2(1,2) = up2(2,ic1)
         uupp1(1,1) = up1(1,ic1)
         uupp1(1,2) = up1(2,ic1)
    
         vv1(1,1) = v1(1,ic2)                 
         vv1(2,1) = v1(2,ic2)                 
         vv2(1,1) = v2(1,ic2)                 
         vv2(2,1) = v2(2,ic2)                 
c
c now gam is the muon current (no emission)
c
       do imu=1,4
        gam(imu) = gammu(imu,ic1,ic2)
        gam_ma(imu) = gammu_ma(imu,ic1,ic2)
       enddo
      call skalar1aLO(gam,gam_ma)
c
c sum over photon polarizations
c
          do i=1,2
c                  eck1(1)=rk1(1)
c                  eck1(2)=rk1(2)
c                  eck1(3)=rk1(3)
c                  eck1(4)=rk1(4)
                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4)
c
           call skalar2LO(rk1,eck1)
           call blocksLO(qq)
           call ddvec(rk1,eck1,uupp1,uupp2,vv1,vv2,ddpl,ddmi,qq)
c
c ddmi is an A type matrix; ddpl is a B type matrix 
c
c the FSR - corrected if fsrnlo=1 (ver_s calculated in subroutine input)
c           for fsrnlo = 0 ver_s =0
c
      ampLOm2 = ampLOm2
     1         +( (dme*cdabs(ddpl(1,1)-ddmi(1,1)))**2
     1         +(dme*cdabs(ddpl(2,2)-ddmi(2,2)))**2
     2         +(cdabs(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1)))**2
     3         +(cdabs(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2 )
     4         *( ver_s)*cdabs(cvac_s)**2
c
c ISR+correction FSR 
c in the amplitudes 'magnetic' amplitude added at difference from pion case
c 

        ampLOm2 = ampLOm2 + ( (dme*cdabs(mb(1,1)-ma(1,1)))**2
     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
     4          * ver_f* cdabs(cvac_qq)**2
      ampLOm2 = ampLOm2
     4         +2.d0*dreal(ver_ma*(
     5    dconjg(dme*(mb(1,1)-ma(1,1)))*dme*(mb_ma(1,1)-ma_ma(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))*dme*(mb_ma(2,2)-ma_ma(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ma_ma(2,1)+el_m2/ebppb*mb_ma(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1    (ebppb*mb_ma(1,2)-el_m2/ebppb*ma_ma(1,2))))
     2   * cdabs(cvac_qq)**2
c
      ampLOm2 = ampLOm2
     4         +2.d0*dreal(
     5    dconjg(dme*(mb(1,1)-ma(1,1))*cvac_qq)
     5     *dme*(ddpl(1,1)-ddmi(1,1))*cvac_s
     6   +dconjg(dme*(mb(2,2)-ma(2,2))*cvac_qq)
     6     *dme*(ddpl(2,2)-ddmi(2,2))*cvac_s
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq)*
     8    (-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq)*
     1    (ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s)*
     2    (ver_s + ver_f)/2.d0
c
      ampLOm2 = ampLOm2
     4         +2.d0*dreal(ver_ma*(
     5    dconjg(dme*(ddpl(1,1)-ddmi(1,1))*cvac_s)
     6     *dme*(mb_ma(1,1)-ma_ma(1,1))*cvac_qq
     7   +dconjg(dme*(ddpl(2,2)-ddmi(2,2))*cvac_s)
     8     *dme*(mb_ma(2,2)-ma_ma(2,2))*cvac_qq
     9   +dconjg((-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s)
     1     *(-ebppb*ma_ma(2,1)+el_m2/ebppb*mb_ma(2,1))*cvac_qq
     2   +dconjg((ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s)
     3     *(ebppb*mb_ma(1,2)-el_m2/ebppb*ma_ma(1,2))*cvac_qq))
    


          enddo
        enddo
      enddo
                                                                                                                                     


c
      return
      end
c-----------------------------------------------------------------------
c***********************************************************************
c-------------------------------------- FSR virtual correction proton---
c-----------------------------------------------------------------------
      real*8 function vertLO_p(qq)
      include 'phokhara_10.0.inc'     
      complex*16 cdilog,pod1,pod2,wsp1,wsp2
      real*8 betamu,mmu2,qq,vert,pod3
      complex*16 protF1,protF2
      
      mmu2 = mp**2
      betamu = Sqrt(1.d0 - 4.d0*mmu2/qq)
      pod1 = dcmplx(2.d0*betamu/(1.d0+betamu),0.d0)
      pod3 = qq*(1.d0+betamu)**2/4.d0/mmu2

      vertLO_p = 2.d0*alpha/pi*( (1.d0+betamu**2)/2.d0/betamu*Log(pod3)
     8       - 1.d0 )* Log(2.d0*w)
      
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c used for proton code
c
      real*8 function ampLOp(qq,rk1,gam,gammu,gammu_ma,v1,v2,up1,up2,q)
      include 'phokhara_10.0.inc'     
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam,ver_ma,vert_ma
      complex*16 epsk1(2,4),epsk2(2,4),cvac_qq,dggvap
      complex*16 gam(4),eck1(4),eck2(4),gammu(4,2,2),gammu_ma(4,2,2)
     1          ,v1(2,2),v2(2,2),up1(2,2),up2(2,2),gam_ma(4)
      complex*16 ma(2,2),mb(2,2),ddpl(2,2),ddmi(2,2)
     1          ,ma_ma(2,2),mb_ma(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 BW_om,vacpol_and_nr
      integer i,ic1,ic2,imu
      real*8 qq,dme,el_m2,ebppb,vacuumpolarization,ver_f
      real*8 rk1(4),rk2(4),p1(4),p2(4),q(4)
      real*8 vertLO_p,aa_phi,mm_ph,gg_ph,Columb_F
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri1/ma,mb,ma_ma,mb_ma
      common /cp1p2/p1,p2,dme,el_m2
      common/param_PFF/aa_phi,mm_ph,gg_ph
      
c     
      ampLOp = 0.d0
      call pol_vec(rk1,epsk1)
      ebppb = p1(1)+p1(4)
      call skalar1LO(rk1)

      if(fsrnlo.eq.1)then
       ver_f  = vertLO_p(qq)
       ver_ma = 0.d0
      endif
c
c vacuum polarization
c
      cvac_qq = vacpol_and_nr(qq)
c
c sum over p,bar p polarizations
c
      do ic1=1,2
        do ic2=1,2
c
c the spinors
c
         uupp2(1,1) = up2(1,ic1)
         uupp2(1,2) = up2(2,ic1)
         uupp1(1,1) = up1(1,ic1)
         uupp1(1,2) = up1(2,ic1)
    
         vv1(1,1) = v1(1,ic2)                 
         vv1(2,1) = v1(2,ic2)                 
         vv2(1,1) = v2(1,ic2)                 
         vv2(2,1) = v2(2,ic2)                 
c
c now gam is the proton current (no emission)
c
       do imu=1,4
        gam(imu) = gammu(imu,ic1,ic2)
        gam_ma(imu) = gammu_ma(imu,ic1,ic2)
       enddo
      call skalar1aLO(gam,gam_ma)
      
c
c sum over photon polarizations
c
          do i=1,2
c                  eck1(1)=rk1(1)
c                  eck1(2)=rk1(2)
c                  eck1(3)=rk1(3)
c                  eck1(4)=rk1(4)
                 eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4)
c
           call skalar2LO(rk1,eck1)
           call blocksLO(qq)
           call ddvec(rk1,eck1,uupp1,uupp2,vv1,vv2,ddpl,ddmi,qq)
           
          
           
c
c ddmi is an A status matrix; ddpl is a B status matrix 
c
c the FSR - corrected if fsrnlo=1 (ver_s calculated in subroutine input)
c           for fsrnlo = 0 ver_s =0
c
      ampLOp = ampLOp
     1         +( (dme*cdabs(ddpl(1,1)-ddmi(1,1)))**2
     1         +(dme*cdabs(ddpl(2,2)-ddmi(2,2)))**2
     2         +(cdabs(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1)))**2
     3         +(cdabs(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2 )
     4         *(1.d0 + ver_s)*cdabs(cvac_s)**2*Columb_F(qq)
      
c
           
c ISR+correction FSR 
c in the amplitudes 'magnetic' amplitude added at difference from pion case
c 
        if(fsrnlo.eq.1)then
        ampLOp = ampLOp + ( (dme*cdabs(mb(1,1)-ma(1,1)))**2
     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
     4          * ver_f* cdabs(cvac_qq)**2*Columb_F(qq)

        endif
c
      if(fsr.eq.2)then
c
c the FSR x ISR 
      ampLOp = ampLOp
     4         +2.d0*dreal(
     5    dconjg(dme*(mb(1,1)-ma(1,1))*cvac_qq)
     5     *dme*(ddpl(1,1)-ddmi(1,1))*cvac_s
     6   +dconjg(dme*(mb(2,2)-ma(2,2))*cvac_qq)
     6     *dme*(ddpl(2,2)-ddmi(2,2))*cvac_s
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq)*
     8    (-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))*cvac_s
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq)*
     1    (ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))*cvac_s)
c

      endif
c
          enddo
        enddo
      enddo
c
      return
      end

c*****************************************************************************
c     matrices and scalar products; mi=minus, pl=plus,
c     eck1=epsilon*(k1) etc.
c
      subroutine skalar1LO(rk1) 
      include 'phokhara_10.0.inc'     
      complex*16 gam(4),eck1(4),eck2(4),pi1eck1,pi2eck1
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2) 
     3       ,gampl_ma(2,2),gammi_ma(2,2)
      real*8 p1(4),p2(4),rk1(4),rk2(4),q(4)
      real*8 rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2,dme,el_m2
     1      ,rat1,cos1,rk1pi1,rk1pi2
      integer i1
c
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common /cp1p2/p1,p2,dme,el_m2
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
c
      rat1 = el_m2/(p1(1)+p1(4))
c
      cos1 = rk1(4) / rk1(1)
c
      rk1p1 = rk1(1) * ( rat1 + p1(4) * (1.d0 - cos1) )
      rk1p2 = rk1(1) * ( rat1 + p1(4) * (1.d0 + cos1) )
c
      rk1pi1 = rk1(1)*momenta(6,0)      
      rk1pi2 = rk1(1)*momenta(7,0)
      do i1 =1,3
       rk1pi1 = rk1pi1 - rk1(i1+1)*momenta(6,i1)
       rk1pi2 = rk1pi2 - rk1(i1+1)*momenta(7,i1)
      enddo     
c
      call plus(rk1,k1pl)
      call minus(rk1,k1mi)
c
      return
      end

c     matrices and scalar products; mi=minus, pl=plus,
c     eck1=epsilon*(k1) etc.
c
      subroutine skalar1LOpi0(rk1,rk2) 
      include 'phokhara_10.0.inc'     
      complex*16 gam(4),eck1(4),eck2(4),pi1eck1,pi2eck1
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2) 
     3       ,gampl_ma(2,2),gammi_ma(2,2)
      real*8 p1(4),p2(4),rk1(4),rk2(4),q(4)
      real*8 rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2,dme,el_m2
     1      ,rat1,cos1,rk1pi1,rk1pi2,cos2
      integer i1
c
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common /cp1p2/p1,p2,dme,el_m2
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
c
c
      rat1 = el_m2/(p1(1)+p1(4))
c
      cos1 = rk1(4) / rk1(1)
c
      rk1p1 = rk1(1) * ( rat1 + p1(4) * (1.d0 - cos1) )
      rk1p2 = rk1(1) * ( rat1 + p1(4) * (1.d0 + cos1) )
      
      cos2 = rk2(4) / rk2(1)
c
      rk2p1 = rk2(1) * ( rat1 + p1(4) * (1.d0 - cos2) )
      rk2p2 = rk2(1) * ( rat1 + p1(4) * (1.d0 + cos2) )
c
c      rk1p1=rk1(1)*momenta(1,0)
c      rk1p2=rk1(1)*momenta(2,0)
c      rk2p1=rk2(1)*momenta(1,0)
c      rk2p2=rk2(1)*momenta(2,0)

c      do i1=2,4
c        rk1p1=rk1p1-rk1(i1)*momenta(1,i1-1)
c        rk1p2=rk1p2-rk1(i1)*momenta(2,i1-1)
c        rk2p1=rk2p1-rk2(i1)*momenta(1,i1-1)
c        rk2p2=rk2p2-rk2(i1)*momenta(2,i1-1)
c      enddo
      call plus(rk1,k1pl)
      call minus(rk1,k1mi)

      call plus(rk2,k2pl)
      call minus(rk2,k2mi)
c
      return
      end
c*****************************************************************************
c     matrices and scalar products; mi=minus, pl=plus,
c     eck1=epsilon*(k1) etc.
c
      subroutine skalar1aLO(gam,gam_ma)
      include 'phokhara_10.0.inc'     
      complex*16 gam(4),gam_ma(4),gampl_ma(2,2),gammi_ma(2,2)
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2) 
c
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
c
      call cplus(gam,gampl)
      call cminus(gam,gammi)

c
      if((pion.eq.0).and.(fsrnlo.eq.1))then
       call cplus(gam_ma,gampl_ma)
       call cminus(gam_ma,gammi_ma)
      endif
c
      return
      end
c*****************************************************************************
c     matrices and scalar products; mi=minus, pl=plus,
c     eck1=epsilon*(k1) etc.
c
      subroutine skalar1aLOpi0(gamk1,gamk2,eck1,eck2)
      include 'phokhara_10.0.inc'     
      complex*16 gam(4),gam_ma(4),gampl_ma(2,2),gammi_ma(2,2)
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2),
     3 eck1(4),eck2(4),gamk1e1(4),gamk2e2(4),
     4 gamk1e1pl(2,2),gamk2e2pl(2,2),gamk1e1mi(2,2),gamk2e2mi(2,2)
      real*8 gamk1(4,4),gamk2(4,4)
      integer mu,nu
c
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/matripi0/gamk1e1pl,gamk2e2pl,gamk1e1mi,gamk2e2mi
c
      do nu=1,4
           gamk1e1(nu)=gamk1(nu,1)*eck1(1)
           gamk2e2(nu)=gamk2(nu,1)*eck2(1)
      enddo
      
      do nu=1,4
         do mu=2,4
           gamk1e1(nu)=gamk1e1(nu)-gamk1(nu,mu)*eck1(mu)
           gamk2e2(nu)=gamk2e2(nu)-gamk2(nu,mu)*eck2(mu)
         enddo
      enddo
   
       
       
      call cplus(gamk1e1,gamk1e1pl)
      call cminus(gamk1e1,gamk1e1mi)
      call cplus(gamk2e2,gamk2e2pl)
      call cminus(gamk2e2,gamk2e2mi)

      return
      end

c*****************************************************************************
c     matrices and scalar products; mi=minus, pl=plus,
c     eck1=epsilon*(k1) etc.
c
      subroutine skalar2LO(rk1,eck1)
      include 'phokhara_10.0.inc'     
      complex*16 gam(4),eck1(4),eck2(4),p1eck1,p1eck2,p2eck1,p2eck2
     1          ,p1gam,p2gam,dd(4)
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2          eck2pl(2,2),eck2mi(2,2),I(2,2)
     3         ,sigpl(4,2,2),sigmi(4,2,2),ddpl(2,2),ddmi(2,2) 
     3       ,gampl_ma(2,2),gammi_ma(2,2)
      complex*16 pi1eck1,pi2eck1,f1,BW
      real*8 rk1pi1,rk1pi2,dme,el_m2,p1(4),p2(4),rk1(4),rk2(4),q(4)
      integer i1
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common /cp1p2/p1,p2,dme,el_m2
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
c
      call cplus(eck1,eck1pl)
      call cminus(eck1,eck1mi)
c
c     scalar products multiplied by 2, not reflected in their names !
c
      p1eck1=2.d0*(p1(1)*eck1(1)-p1(2)*eck1(2)-p1(3)*eck1(3)-
     1            p1(4)*eck1(4))
      p2eck1=2.d0*(p2(1)*eck1(1)-p2(2)*eck1(2)-p2(3)*eck1(3)-
     1            p2(4)*eck1(4))  
c
      pi1eck1 = momenta(6,0)*eck1(1)      
      pi2eck1 = momenta(7,0)*eck1(1)      
      do i1 =1,3
       pi1eck1 = pi1eck1 - momenta(6,i1)*eck1(i1+1)
       pi2eck1 = pi2eck1 - momenta(7,i1)*eck1(i1+1)
      enddo
c
      return
      end

c*****************************************************************************
c     matrices and scalar products; mi=minus, pl=plus,
c     eck1=epsilon*(k1) etc.
c
      subroutine skalar2LOpi0(rk1,eck1,rk2,eck2)
      include 'phokhara_10.0.inc'     
      complex*16 gam(4),eck1(4),eck2(4),p1eck1,p1eck2,p2eck1,p2eck2
     1          ,p1gam,p2gam,dd(4)
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2          eck2pl(2,2),eck2mi(2,2),I(2,2)
     3         ,sigpl(4,2,2),sigmi(4,2,2),ddpl(2,2),ddmi(2,2) 
     3       ,gampl_ma(2,2),gammi_ma(2,2)
      complex*16 pi1eck1,pi2eck1,f1,BW
      real*8 rk1pi1,rk1pi2,dme,el_m2,p1(4),p2(4),rk1(4),rk2(4),q(4)
      integer i1
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common /cp1p2/p1,p2,dme,el_m2
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
c
      call cplus(eck1,eck1pl)
      call cminus(eck1,eck1mi)

      call cplus(eck2,eck2pl)
      call cminus(eck2,eck2mi)
c
c     scalar products multiplied by 2, not reflected in their names !
c
c      p1eck1=(p1(1)*eck1(1)-p1(2)*eck1(2)-p1(3)*eck1(3)-
c     1            p1(4)*eck1(4))
c      p2eck1=(p2(1)*eck1(1)-p2(2)*eck1(2)-p2(3)*eck1(3)-
c     1            p2(4)*eck1(4))  

c      p1eck2=(p1(1)*eck2(1)-p1(2)*eck2(2)-p1(3)*eck2(3)-
c     1            p1(4)*eck2(4))
c      p2eck2=(p2(1)*eck2(1)-p2(2)*eck2(2)-p2(3)*eck2(3)-
c    1            p2(4)*eck2(4))
      p1eck1=2.d0*(p1(1)*eck1(1)-p1(2)*eck1(2)-p1(3)*eck1(3)-
     1            p1(4)*eck1(4))
      p2eck1=2.d0*(p2(1)*eck1(1)-p2(2)*eck1(2)-p2(3)*eck1(3)-
     1            p2(4)*eck1(4))  

      p1eck2=2.d0*(p1(1)*eck2(1)-p1(2)*eck2(2)-p1(3)*eck2(3)-
     1            p1(4)*eck2(4))
      p2eck2=2.d0*(p2(1)*eck2(1)-p2(2)*eck2(2)-p2(3)*eck2(3)-
     1            p2(4)*eck2(4))
c
c
      return
      end
c*****************************************************************************
      subroutine blocksLO(qq)
      include 'phokhara_10.0.inc'         
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
     1          ,pi1eck1,pi2eck1
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2)
     3       ,gampl_ma(2,2),gammi_ma(2,2),
     4 gamk1e1pl(2,2),gamk2e2pl(2,2),gamk1e1mi(2,2),gamk2e2mi(2,2)
      complex*16 m1(2,2),m2(2,2),m3(2,2),m4(2,2),m5(2,2),m6(2,2),
     1         n1(2,2),n2(2,2),n3(2,2),n4(2,2),n5(2,2),n6(2,2),n7(2,2),
     2         n8(2,2),n9(2,2),n10(2,2),n11(2,2),n12(2,2)
      complex*16 block1(2,2),block2(2,2),block3(2,2),block4(2,2),
     1          block5(2,2),block6(2,2),block7(2,2),block8(2,2),
     2          block9(2,2),block10(2,2),block11(2,2),block12(2,2)
      complex*16 m1amp1(2,2),mamp1a(2,2),m2amp1(2,2),mamp1b(2,2),
     1           m1amp2(2,2),mamp2a(2,2),m2amp2(2,2),mamp2b(2,2),
     2           m1amp3(2,2),mamp3a(2,2),m2amp3(2,2),mamp3b(2,2), 
     3           m1amp4(2,2),mamp4a(2,2),m2amp4(2,2),mamp4b(2,2),
     4           m1amp5(2,2),mamp5a(2,2),m2amp5(2,2),mamp5b(2,2),
     5           m1amp6(2,2),mamp6a(2,2),m2amp6(2,2),mamp6b(2,2),
     6  m1amp3_ma(2,2),m1amp4_ma(2,2),m2amp3_ma(2,2),m2amp4_ma(2,2)
      complex*16 ma(2,2),mb(2,2),ma_ma(2,2),mb_ma(2,2)
      real*8 qq,rk1pi1,rk1pi2
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/matripi0/gamk1e1pl,gamk2e2pl,gamk1e1mi,gamk2e2mi
      common/matri1/ma,mb,ma_ma,mb_ma
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
      common/blocks1/block1,block2,block3,block4
c
      call conmat(p2eck1,I,m1)
      call conmat(p1eck1,I,m3)
c
      call matr(eck1mi,k1pl,n5)
      call matr(eck1pl,k1mi,n6)
      call matr(k1pl,eck1mi,n11)
      call matr(k1mi,eck1pl,n12)
c
      call minmat(n5,m3,block5)
      call minmat(n6,m3,block6)
      call minmat(m1,n11,block9)
      call minmat(m1,n12,block10)
c
      call matr(block5,gammi,m1amp3)
      call matr(block6,gampl,m2amp3)
c
      call matr(gammi,block9,m1amp4)
      call matr(gampl,block10,m2amp4)
c
c     adding matrices of the status A and B
c
      call plumatLO(qq,m1amp3,m1amp4,ma)
      call plumatLO(qq,m2amp3,m2amp4,mb)

c
c additional part for muon FSR
c     
      if(pion.eq.0)then
c
c additional part for muon FSR
c     
       call conmat(2.d0*pi1eck1,I,m5)
       call conmat(2.d0*pi2eck1,I,m6)
       call dodmat(m5,n11,block1)
       call dodmat(m5,n12,block2)
       call dodmat(m6,n5,block3)
       call dodmat(m6,n6,block4)
c
c additional part for muon FSRNLO - 'magnetic' amplitude constructed here
c     
       if(fsrnlo.eq.1)then
        call matr(block5,gammi_ma,m1amp3_ma)
        call matr(block6,gampl_ma,m2amp3_ma)
c
        call matr(gammi_ma,block9,m1amp4_ma)
        call matr(gampl_ma,block10,m2amp4_ma)
c
c     adding matrices of the status A and B
c
        call plumatLO(qq,m1amp3_ma,m1amp4_ma,ma_ma)
        call plumatLO(qq,m2amp3_ma,m2amp4_ma,mb_ma)
       endif
c-----------------------------------------proton------------------------
c additional part for proton FSR
c     
      elseif(pion.eq.4)then
c
       call conmat(2.d0*pi1eck1,I,m5)
       call conmat(2.d0*pi2eck1,I,m6)
       call dodmat(m5,n11,block1)
       call dodmat(m5,n12,block2)
       call dodmat(m6,n5,block3)
       call dodmat(m6,n6,block4)     
      
c additional part for proton FSRNLO - 'magnetic' amplitude constructed here
c     
       if(fsrnlo.eq.1)then
        call matr(block5,gammi_ma,m1amp3_ma)
        call matr(block6,gampl_ma,m2amp3_ma)
c
        call matr(gammi_ma,block9,m1amp4_ma)
        call matr(gampl_ma,block10,m2amp4_ma)
c
c     adding matrices of the status A and B
c
        call plumatLO(qq,m1amp3_ma,m1amp4_ma,ma_ma)
        call plumatLO(qq,m2amp3_ma,m2amp4_ma,mb_ma)
       endif

      elseif((pion.eq.13).or.(pion.eq.14).or.(pion.eq.15))then
c k1
       call conmat(p2eck1,I,m1)
       call conmat(p1eck1,I,m3)

c k2
       call conmat(p2eck2,I,m2)
       call conmat(p1eck2,I,m4)
c
c k1
       call matr(k1pl,eck1mi,n1)
       call matr(k1mi,eck1pl,n2)
       call matr(eck1mi,k1pl,n3)
       call matr(eck1pl,k1mi,n4)

c k2
       call matr(k2pl,eck2mi,n5)
       call matr(k2mi,eck2pl,n6)
       call matr(eck2mi,k2pl,n7)
       call matr(eck2pl,k2mi,n8)

c k1
       call minmat(m1,n1,block1)
       call minmat(m1,n2,block2)

       call minmat(n3,m3,block3)
       call minmat(n4,m3,block4)

c k2

       call minmat(m2,n5,block5)
       call minmat(m2,n6,block6)

       call minmat(n7,m4,block7)
       call minmat(n8,m4,block8)
c A matrices
c k1
       call matr(gamk2e2mi,block1,mamp1a) 
       call matr(block3,gamk2e2mi,mamp2a) 
c k2
       call matr(gamk1e1mi,block5,mamp3a)
       call matr(block7,gamk1e1mi,mamp4a)
c B matrices
c k1
       call matr(gamk2e2pl,block2,mamp1b)
       call matr(block4,gamk2e2pl,mamp2b)
c k2
       call matr(gamk1e1pl,block6,mamp3b)
       call matr(block8,gamk1e1pl,mamp4b)
c---------------------------------------------------------------------
       call plumatLOpi01(mamp1a,mamp2a,ma)
       call plumatLOpi01(mamp1b,mamp2b,mb)

       call plumatLOpi02(mamp3a,mamp4a,ma_ma)
       call plumatLOpi02(mamp3b,mamp4b,mb_ma)

c-----------------------------------------------------------------------
      endif
c
      return
      end
c**************************************************************************
      subroutine ddvec(rk1,eck1,uupp1,uupp2,vv1,vv2,ddpl,ddmi,qq)
      include 'phokhara_10.0.inc'     
      complex*16 gam(4),eck1(4),eck2(4),dd(4),dee(4),dmm(4),
     1               protF1,protF2
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2          eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2)
     3         ,ddpl(2,2),ddmi(2,2),gampl_ma(2,2),gammi_ma(2,2)
      complex*16 block1(2,2),block2(2,2),block3(2,2),block4(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 pi1eck1,pi2eck1,f1,wyn1,wyn2,BW_om,phas2
      complex*16 mm1a(2,2),mm1b(2,2),mma(2,2),mm2a(2,2),mm2b(2,2)
     1          ,mmb(2,2),F_phi_KK,Fun_phi,phas1,F_phi_no,F_phi_exp,
     2           mk1(4,2,2),mk2(4,2,2),mm1ap(2,2),mm2ap(2,2),mm3ap(2,2),
     3           mm4ap(2,2),mmap1(2,2),mmap2(2,2),wynap1,wynap2
      real*8 rk1pi1,rk1pi2,qq,aa_phi,mm_ph,gg_ph,phas_rho_f0_exp,
     1       mm_f0_exp,c_f0_KK_exp,c_f0_pipi_exp,ff_phi_exp,c_phi_KK,
     2       rho_phi_exp,lamb_phi_exp,phas_rho_f0,sm(4),sm2

      real*8 rk1(4),rk2(4),q(4),mass_2pi,Columb_F
      integer i1,jj
c
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
      common/blocks1/block1,block2,block3,block4
      common/phases_f0/phas1,phas2
      common/param_PFF/aa_phi,mm_ph,gg_ph
      common/param_f0_exp/mm_f0_exp,phas_rho_f0_exp,c_f0_KK_exp,
     1       c_f0_pipi_exp,ff_phi_exp,c_phi_KK,rho_phi_exp,lamb_phi_exp
       common/newmag/mk1,mk2
       common/ssmm/sm
c
c D vector constructed : FSR
c
c pi+ pi-
      if(pion.eq.1)then
        f1 = PionFormFactor_Sp/Sp
       
      if(f0_model.eq.0)then
        Fun_phi = F_phi_KK(qq,Sp)
        phas_rho_f0 = 75.d0*dSqrt(qq-4.d0*mpi**2)
        phas1 = exp(dcmplx(0.d0,phas_rho_f0*pi/180.d0))
      elseif(f0_model.eq.1)then
         Fun_phi = F_phi_no(qq,Sp)
      elseif(f0_model.eq.2)then
         Fun_phi = (0.d0,0.d0)
      elseif(f0_model.eq.3)then
         Fun_phi = - F_phi_exp(qq,Sp)    
         phas1 = exp(dcmplx(0.d0,
     1           phas_rho_f0_exp*dSqrt(1.d0-4.d0*mpi**2/qq)*pi/180.d0))

      endif

      do i1=1,4
       dd(i1) = 
     1( ((momenta(6,i1-1)+rk1(i1)-momenta(7,i1-1))*pi1eck1/rk1pi1
     2  +(momenta(7,i1-1)+rk1(i1)-momenta(6,i1-1))*pi2eck1/rk1pi2
     3  -2.d0*eck1(i1))*f1 )*cvac_s_pi
     4  + phas1 * Fun_phi/Sp* 
     6   ( (rk1pi1+rk1pi2)*eck1(i1) - (pi1eck1+pi2eck1)*rk1(i1) )
     7                      * cvac_s_pi1
      enddo
c
c kaons K+K-
      elseif(pion.eq.6)then
       f1 = KaonFormFactor_Sp/Sp
      do i1=1,4
       dd(i1) = 
     1    ((momenta(6,i1-1)+rk1(i1)-momenta(7,i1-1))*pi1eck1/rk1pi1
     2    +(momenta(7,i1-1)+rk1(i1)-momenta(6,i1-1))*pi2eck1/rk1pi2
     3         -2.d0*eck1(i1))*f1
      enddo
c
c muons
      elseif(pion.eq.0)then
      do i1=1,4
       call matr2(i1,block3,sigmi,mm1a)
       call matr2(i1,block4,sigpl,mm1b)
       call matr1(i1,sigmi,block1,mm2a)
       call matr1(i1,sigpl,block2,mm2b)
       call plumat1_LO(Sp,mm1a,mm2a,mma)
       call plumat1_LO(Sp,mm1b,mm2b,mmb)
       call stal(uupp1,mma,vv1,wyn1)
       call stal(uupp2,mmb,vv2,wyn2)
       dd(i1) = wyn1+wyn2
      enddo
c protons
      elseif(pion.eq.4)then
         sm2=sm(1)**2
        do jj=2,4
          sm2=sm2-sm(jj)**2
        enddo
        if(FF_pp.eq.0)then
           call ProtonFormFactor(sm2,protF1,protF2)
	elseif(FF_pp.eq.1)then
	   call ProtonFormFactor_new(sm2,protF1,protF2)
	endif

 
      do i1=1,4
c for magnetic part
       call matr1(i1,mk1,block1,mm1ap)
       call matr1(i1,mk2,block2,mm2ap)
       call matr2(i1,block4,mk1,mm3ap)
       call matr2(i1,block3,mk2,mm4ap)
       call plumat1_LO(Sp,mm3ap,mm1ap,mmap1)
       call plumat1_LO(Sp,mm4ap,mm2ap,mmap2)
       call stal(uupp2,mmap1,vv1,wynap1)
       call stal(uupp1,mmap2,vv2,wynap2)
       dmm(i1)=-wynap1-wynap2
           
c for magnetic part
       call matr2(i1,block3,sigmi,mm1a)
       call matr2(i1,block4,sigpl,mm1b)
       call matr1(i1,sigmi,block1,mm2a)
       call matr1(i1,sigpl,block2,mm2b)
       call plumat1_LO(Sp,mm1a,mm2a,mma)
       call plumat1_LO(Sp,mm1b,mm2b,mmb)
       call stal(uupp1,mma,vv1,wyn1)
       call stal(uupp2,mmb,vv2,wyn2)
       dee(i1) = wyn1+wyn2
      enddo
c-----------------------------------------------------------------------------------
      do i1=1,4
      dd(i1)=dmm(i1)*protF2/4.d0/mp+dee(i1)*protF1
      dd(i1)=dmm(i1)*protF2/4.d0/mp+dee(i1)*protF1
 
      enddo 

      else 
       stop
      endif      
c
      call cplus(dd,ddpl)
      call cminus(dd,ddmi)
c
      return
      end
c *************************************************************************
      complex*16 function F_phi_KK(qq,xx)
      include 'phokhara_10.0.inc'     
      real*8 mm_ph,gg_ph,mm_f0,gg_f0,c_phi_KK_f0_pi,qq,aa_phi,
     1       c_phi_gam,mm_f0_600,gg_f0_600,c_phi_KK_f0_600_pi,xx
      complex*16 ff0_II,phas1,phas2,i,ff
      common/param_PFF/aa_phi,mm_ph,gg_ph
      common/param_f0/mm_f0,gg_f0,c_phi_KK_f0_pi,c_phi_gam,
     1                mm_f0_600,gg_f0_600,c_phi_KK_f0_600_pi
      common/phases_f0/phas1,phas2

      i = (0.d0,1.d0)
      ff = ( c_phi_KK_f0_pi / (mm_f0**2-qq-i*mm_f0*gg_f0)
     1   + phas2 * c_phi_KK_f0_600_pi
     3           / (mm_f0_600**2-qq-i*mm_f0_600*gg_f0_600) )
     2   / 2.d0 / pi**2 / mKp**2 * ff0_II(qq) 

      F_phi_KK =   c_phi_gam/(xx-mm_ph**2+i*mm_ph*gg_ph)*ff

      return
      end
c *************************************************************************
      complex*16 function ff0_II(qq)
      include 'phokhara_10.0.inc'         
      real*8 qq,aa,bb,amb,mm_ph,gg_ph,aa_phi
      complex*16 ff_f0,gg_f0
      common/param_PFF/aa_phi,mm_ph,gg_ph
c------
      aa = (mm_ph/mKp)**2
      bb = qq/mKp**2
      amb = aa-bb
      ff0_II = 0.5d0/amb-2.d0/amb**2*(ff_f0(1.d0/bb)-ff_f0(1.d0/aa))
     1    +aa/amb**2*(gg_f0(1.d0/bb)-gg_f0(1.d0/aa))
      return
      end
c ************************************************************************

      complex*16 function ff_f0(xx)
      include 'phokhara_10.0.inc'         
      real*8 xx
c------
      if(xx.ge.0.25d0)then
       ff_f0 = -asin(0.5d0/sqrt(xx))**2
      else
       ff_f0 = 0.25d0*( log((1.d0+sqrt(1.d0-4.d0*xx))**2/4.d0/xx)
     1            -dcmplx(0.d0,-pi))**2
      endif
      return
      end
c ************************************************************************
      complex*16 function gg_f0(xx)
      include 'phokhara_10.0.inc'
      real*8 xx
c------
      if(xx.ge.0.25d0)then
       gg_f0 = sqrt(4.d0*xx-1.d0)*asin(0.5d0/sqrt(xx))
      else
       gg_f0 = 0.5d0*sqrt(1.d0-4.d0*xx)
     1     *( log((1.d0+sqrt(1.d0-4.d0*xx))**2/4.d0/xx)
     1            -dcmplx(0.d0,-pi))
      endif
      return
      end
c *************************************************************************
      complex*16 function F_phi_no(qq,xx)
      include 'phokhara_10.0.inc'     
      real*8 mm_ph,gg_ph,mm_f0,gg_f0,c_phi_KK_f0_pi,qq,aa_phi,
     1       c_phi_gam,mm_f0_600,gg_f0_600,c_phi_KK_f0_600_pi,xx
      complex*16 phas1,phas2,i,ff
      common/param_PFF/aa_phi,mm_ph,gg_ph
      common/param_f0/mm_f0,gg_f0,c_phi_KK_f0_pi,c_phi_gam,
     1                mm_f0_600,gg_f0_600,c_phi_KK_f0_600_pi
      common/phases_f0/phas1,phas2

      i = (0.d0,1.d0)
      ff = ( c_phi_KK_f0_pi / (mm_f0**2-qq-i*mm_f0*gg_f0)
     1   + phas2 * c_phi_KK_f0_600_pi
     3           / (mm_f0_600**2-qq-i*mm_f0_600*gg_f0_600) ) 

      F_phi_no =   c_phi_gam/(xx-mm_ph**2+i*mm_ph*gg_ph)*ff

      return
      end
c *************************************************************************
      complex*16 function F_phi_exp(qq,xx)
      include 'phokhara_10.0.inc' 
      real*8 aa_phi,mm_ph,gg_ph,qq
      real*8 mm_f0_exp,phas_rho_f0_exp,c_f0_KK_exp,c_f0_pipi_exp,
     1       ff_phi_exp,c_phi_KK,rho_phi_exp,lamb_phi_exp,xx
      complex*16 i,D_f0_exp,gg_mm_exp,BW_om,D_phi_exp
      common/param_PFF/aa_phi,mm_ph,gg_ph
      common/param_f0_exp/mm_f0_exp,phas_rho_f0_exp,c_f0_KK_exp,
     1      c_f0_pipi_exp,ff_phi_exp,c_phi_KK,rho_phi_exp,lamb_phi_exp

      i = (0.d0,1.d0)

      D_phi_exp = BW_om(mm_ph,gg_ph,xx,1)

      F_phi_exp = ff_phi_exp * 2.d0 / (xx-qq) * D_phi_exp 
     1          * c_f0_pipi_exp 
c g(m)
     3          * c_phi_KK * c_f0_KK_exp / 8.d0 / pi**2
     4          * gg_mm_exp(qq,rho_phi_exp,lamb_phi_exp) 
c 1/D_f0
     2          /D_f0_exp(qq)
      return
      end
c *************************************************************************
      complex*16 function gg_mm_exp(qq,rho_phi_exp,lamb_phi_exp)
      include 'phokhara_10.0.inc'

      real*8 qq,rho_phi_exp,aa_phi,mm_ph,gg_ph,rho_m_exp,
     1       lamb_m_exp,lamb_phi_exp
      complex*16 i
      common/param_PFF/aa_phi,mm_ph,gg_ph

      i = (0.d0,1.d0)

      rho_m_exp = sqrt(1.d0 - 4.d0*mKp**2/qq)

      if(sqrt(qq).lt.(2.d0*mKp))then

      rho_m_exp = sqrt(-1.d0 + 4.d0*mKp**2/qq)

         gg_mm_exp = 1.d0 + mm_ph**2/(mm_ph**2-qq)
     1           * ( 2.d0 * rho_m_exp * dAtan(1.d0/rho_m_exp)
     2             - rho_phi_exp*lamb_phi_exp + i*pi* rho_phi_exp
     3             - 4.d0*(mKp/mm_ph)**2
     4             * ( (pi+i*lamb_phi_exp)**2/4.d0 
     5                - (dAtan(1.d0/rho_m_exp))**2 ) )   

      else

      rho_m_exp = sqrt(1.d0 - 4.d0*mKp**2/qq)
      lamb_m_exp = log((1.d0+rho_m_exp)**2 *qq/4.d0/mKp**2) 

      gg_mm_exp = 1.d0 +  mm_ph**2/(mm_ph**2-qq)
     1        * ( rho_m_exp*(lamb_m_exp-i*pi)
     2          - rho_phi_exp*(lamb_phi_exp-i*pi)
     3          - (mKp/mm_ph)**2
     4          * ( (pi+i*lamb_phi_exp)**2 - (pi+i*lamb_m_exp)**2 ) )

      endif

      return
      end
c *************************************************************************
      complex*16 function D_f0_exp(qq)
      include 'phokhara_10.0.inc'

      real*8 qq,m_KKp_exp,m_pip_exp,mm_f0_exp,phas_rho_f0_exp,
     2       c_f0_KK_exp,c_f0_pipi_exp,ff_phi_exp,c_phi_KK,rho_phi_exp,
     1       lamb_phi_exp
      complex*16 PP_ab
      common/param_f0_exp/mm_f0_exp,phas_rho_f0_exp,c_f0_KK_exp,
     1      c_f0_pipi_exp,ff_phi_exp,c_phi_KK,rho_phi_exp,lamb_phi_exp

c m_\pi^+ = m_\pi^-
c m_K^+ = m_K^-
      m_KKp_exp = 4.d0*mKp**2
      m_pip_exp   = 4.d0*mpi**2

      D_f0_exp = mm_f0_exp**2 - qq + c_f0_KK_exp**2/16.d0/pi *
     1( dreal(PP_ab(mm_f0_exp**2,m_KKp_exp)) - PP_ab(qq,m_KKp_exp) )
     2                             + c_f0_pipi_exp**2/16.d0/pi *
     1( dreal(PP_ab(mm_f0_exp**2,m_pip_exp)) - PP_ab(qq,m_pip_exp) )

      return
      end
c *************************************************************************
      complex*16 function PP_ab(mm,mm_p)
      include 'phokhara_10.0.inc'

      real*8 mm,mm_p,rho_ab
      complex*16 i

      i = (0.d0,1.d0)

      if(mm.gt.mm_p)then
        rho_ab = sqrt(1.d0-mm_p/mm)

         PP_ab = 1.d0/pi * sqrt(1.d0 - mm_p/mm)
     1         * log(mm_p/(sqrt(mm)+sqrt(mm-mm_p))**2)
     2         + i * rho_ab
      else

        rho_ab = sqrt(-1.d0+mm_p/mm)
        PP_ab = rho_ab * (-1.d0 + 2.d0/pi * dAtan(sqrt(mm_p/mm-1.d0)) )
      endif

      return
      end
c**************************************************************************
c     addind 2 matrices: with proper denominators
c     for one photon initial state emission
c
      subroutine plumatLO(qq,mat1,mat2,mat3)  
      implicit none
      real*8 qq,rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      integer i,j
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2)
c
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
c
      do i=1,2
         do j=1,2
            mat3(i,j)=  0.5d0*mat1(i,j)/rk1p1/qq
     1                 +0.5d0*mat2(i,j)/rk1p2/qq
         enddo
      enddo
      end

c**************************************************************************
c     addind 4 matrices: with proper denominators
c     for one photon initial state emission
c
      subroutine plumatLOpi01(mat1,mat2,mat5) 
      include 'phokhara_10.0.inc' 
      real*8 qq,rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2,qq1,qq2
      integer i,j
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2),mat4(2,2),mat5(2,2)
      complex*16 pionggFF_dprime,etaggFF_dprime2,etaPggFF_dprime2,FFps
c
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common/qqvec12/qq1,qq2
c  
      if(pion.eq.13)then
         FFps=  pionggFF_dprime(qq1,0.d0)
      elseif(pion.eq.14)then
         FFps=  etaggFF_dprime2(qq1,0.d0)
      else
         FFps=  etaPggFF_dprime2(qq1,0.d0)
      endif

      do i=1,2
         do j=1,2
         mat5(i,j)=0.5d0*mat1(i,j)/rk1p2/qq1*FFps
     1            +0.5d0*mat2(i,j)/rk1p1/qq1*FFps
         enddo
      enddo
      end
c**************************************************************************
c**************************************************************************
c     addind 4 matrices: with proper denominators
c     for one photon initial state emission
c
      subroutine plumatLOpi02(mat3,mat4,mat5) 
      include 'phokhara_10.0.inc' 
      real*8 qq,rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2,qq1,qq2
      integer i,j
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2),mat4(2,2),mat5(2,2)
      complex*16 pionggFF_dprime,etaggFF_dprime2,etaPggFF_dprime2,FFps
c
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common/qqvec12/qq1,qq2
c  
      if(pion.eq.13)then
         FFps=  pionggFF_dprime(qq2,0.d0)
      elseif(pion.eq.14)then
         FFps=  etaggFF_dprime2(qq2,0.d0)
      else
         FFps=  etaPggFF_dprime2(qq2,0.d0)
      endif

      do i=1,2
         do j=1,2
         mat5(i,j)=
     2            +0.5d0*mat3(i,j)/rk2p2/qq2*FFps
     3            +0.5d0*mat4(i,j)/rk2p1/qq2*FFps
         enddo
      enddo
      end
c**************************************************************************
c     addind 2 matrices: with proper denominators
c     for one photon final state emission (muons)
c
      subroutine plumat1_LO(dev,mat1,mat2,mat3)      
      implicit none
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2)
      complex*16 pi1eck1,pi2eck1
      real*8 rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2,rk1pi1,rk1pi2,
     1       dev
      integer i,j
c
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
c
      do i=1,2
         do j=1,2
            mat3(i,j)=  0.5d0*mat1(i,j)/rk1pi2/dev
     1                 -0.5d0*mat2(i,j)/rk1pi1/dev
         enddo
      enddo
      end
c**************************************************************************
c     multiplication of the 1x2 2x2 2x1 matrics
c 
      subroutine stal(mat1,mat2,mat3,wyn)
      implicit none
      complex*16  mat1(1,2),mat2(2,2),mat3(2,1),wyn,matp(1,2)
      integer i,k
c
      wyn = (0.d0,0.d0)
      do i=1,2
         matp(1,i) = (0.d0,0.d0)
      enddo
c
      do i=1,2
         do k=1,2
            matp(1,i) = matp(1,i)+mat1(1,k)*mat2(k,i)
         enddo
      enddo
c
      do i=1,2
         wyn = wyn + matp(1,i)*mat3(i,1)
      enddo
      return
      end
c ------------------------------------------------------------------------
c ************************************************************************
c two photons
c ************************************************************************
c ------------------------------------------------------------------------

      real*8 function helicityamp(qq,q0p,q2p,q0b,q2b)
      include 'phokhara_10.0.inc'     
      real*8 qq,rk1(4),rk2(4),q(4),dps,amp_h,amp,rlam
     1                ,pionFF,q0p,q2p,q0b,q2b,a,b,c,mmu_1,
     2             Columb_F,ampsqr_mu,bb
      complex*16 gam(4),gammu(4,2,2),v1(2,2),v2(2,2),up1(2,2),up2(2,2)
     1          ,gammu_ma(4,2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      integer i1,ic1,ic2
c
      rlam(a,b,c) = sqrt( (1.d0-(b+c)**2/a) * (1.d0-(b-c)**2/a) )

      call gam1(gam,gammu,gammu_ma,v1,v2,up1,up2,qq)
      do i1=1,4
         rk1(i1) = momenta(3,i1-1)
         rk2(i1) = momenta(4,i1-1)
         q(i1)   = momenta(5,i1-1)
      enddo
c
      dps = Sp/(4.d0*(2.d0*pi)**5)  ! Phase space factors
c
c --- muons ---
      if(pion.eq.0)then
      mmu_1=mmu
      amp_h=ampsqr_mu(q,qq,gammu,rk1,rk2)

c
c --- 2 pions ---
      elseif(pion.eq.1)then
      dps = dps*dSqrt(1.d0-4.d0*mpi*mpi/qq)/(32.d0*pi*pi) 
      amp_h = ((4.d0*pi*alpha)**4/qq**2)*
     1                 amp(rk1,rk2,gam,q,qq,uupp1,uupp2,vv1,vv2)/4.d0
c
c --- 4 pions ---
      elseif((pion.eq.2).or.(pion.eq.3))then
c
c          Phase space factors
c
      dps = dps/(2.d0*pi)**2/(32.d0*pi**2)**3
     1     * rlam(qq,q0p,rmass(1)) 
     2     * rlam(q2p,q0b,rmass(2)) 
     3     * rlam(q2b,rmass(3),rmass(4))
c
c --- pions 2pi^0 pi^+ pi^-
       if(pion.eq.2)then
        dps = dps * 0.5d0
       endif
c
c --- pions 2pi^+ 2pi^-  
       if(pion.eq.3)then
        dps = dps * 0.25d0
       endif
      amp_h = ((4.d0*pi*alpha)**4/qq**2)*
     1                  amp(rk1,rk2,gam,q,qq,uupp1,uupp2,vv1,vv2)/4.d0
c
c----- PPbar ---------------
      elseif(pion.eq.4)then
      mmu_1=mp
      call fermionamp
     &      (mmu_1,gammu,v1,v2,up1,up2,q,qq,rk1,rk2,dps,amp_h)
       bb=pi*alpha/sqrt(abs(1.d0-4.d0*mp**2/qq))
c      if(fsr.ne.0) 
        amp_h=amp_h*bb/(1.d0-exp(-bb))
       !bez if *bb/(1.d0-exp(-bb)) bb=bb=pi*alpha/sqrt(abs(1.d0-4.d0*mp**2/qq))
      
    
c
c----- NNbar ---------------
      elseif(pion.eq.5)then
      mmu_1=mnt
      call fermionamp
     &      (mmu_1,gammu,v1,v2,up1,up2,q,qq,rk1,rk2,dps,amp_h)
c 
c --- kaons K^+ K^- ---
      elseif(pion.eq.6)then
      dps = dps*dSqrt(1.d0-4.d0*mKp*mKp/qq)/(32.d0*pi*pi) 
      amp_h = ((4.d0*pi*alpha)**4/qq**2)*
     1                 amp(rk1,rk2,gam,q,qq,uupp1,uupp2,vv1,vv2)/4.d0
c
c --- kaons K^0 K^0bar ---
      elseif(pion.eq.7)then
      dps = dps*dSqrt(1.d0-4.d0*mKn*mKn/qq)/(32.d0*pi*pi) 
      amp_h = ((4.d0*pi*alpha)**4/qq**2)*
     1                 amp(rk1,rk2,gam,q,qq,uupp1,uupp2,vv1,vv2)/4.d0
c
c --- pions pi^0 pi^+ pi^-
      elseif((pion.eq.8).or.(pion.eq.10))then
c
c        Phase space factors
c
      dps = dps/(2.d0*pi)/(32.d0*pi**2)**2
     1    * rlam(qq,q0p,rmass(1))
     2    * rlam(q2p,rmass(2),rmass(3))

      amp_h = ((4.d0*pi*alpha)**4/qq**2)*
     1                  amp(rk1,rk2,gam,q,qq,uupp1,uupp2,vv1,vv2)/4.d0
      else
       continue
      endif
c
      helicityamp = amp_h*dps
      return
      end
c*****************************************************************************
      subroutine fermionamp
     &      (mmu_1,gammu,v1,v2,up1,up2,q,qq,rk1,rk2,dps,amp_h)
      include 'phokhara_10.0.inc'     
      real*8 qq,rk1(4),rk2(4),q(4),dps,amp_h,amp,mmu_1
      complex*16 gam(4),gammu(4,2,2),v1(2,2),v2(2,2),up1(2,2),up2(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      integer i1,ic1,ic2

      dps = dps*dSqrt(1.d0-4.d0*mmu_1*mmu_1/qq)/(32.d0*pi*pi)
      amp_h = 0.d0

      do ic1=1,2
      do ic2=1,2
c
c the spinors
c
         if(fsrnlo.eq.1)then
          uupp2(1,1) = up2(1,ic1)
          uupp2(1,2) = up2(2,ic1)
          uupp1(1,1) = up1(1,ic1)
          uupp1(1,2) = up1(2,ic1)
    
          vv1(1,1) = v1(1,ic2)                 
          vv1(2,1) = v1(2,ic2)                 
          vv2(1,1) = v2(1,ic2)                 
          vv2(2,1) = v2(2,ic2)
         endif                 
c
c now gam is the muon current (no emission)
c
        do i1=1,4
         gam(i1)=gammu(i1,ic1,ic2)
        enddo

        amp_h = amp_h + amp(rk1,rk2,gam,q,qq,uupp1,uupp2,vv1,vv2)

      enddo
      enddo
      amp_h = ((4.d0*pi*alpha)**4/qq**2)*amp_h/4.d0

      return
      end
c*****************************************************************************
c the 2pi(pion=1) and mu+, mu-(pion=0) currents + 4pi currents
c
      subroutine gam1(gam,gammu,gammu_ma,v1,v2,up1,up2,qq)
      include 'phokhara_10.0.inc'       
      integer mu,ic1,ic2,ic3,i
      real*8 qq,th1,th2,sphi1,cphi1,sphi2,cphi2,cth1d2,sth1d2,
     &       cth2d2,sth2d2,sq1,sq2,em1,em2,pm1,pm2,sth1,sth2,
     &       q1(4),q2(4),q3(4),q4(4)
      complex*16 PionFormFactor,KaonFormFactor
     1      ,gam(4),f1,gammu(4,2,2),gammu_ma(4,2,2)
     &      ,v1(2,2),v2(2,2),up1(2,2),up2(2,2),ex1,ex2,sss

      real*8 mmu1
      complex*16 gammu1(4,2,2),gammu2(2,2),protF1,protF2 
      integer iic
c
c --- muons ---   
c  four different combinations of \mu^+, \mu^- helicities
c ++,+-,-+,-- ; v1(i,1)==v1(i,+),v1(i,2)==v1(i,-) etc.
c see notes p.4-6   
c
      if ((pion.eq.0).or.(pion.eq.11).or.(pion.eq.12))then 
      mmu1=mmu
      call spinors(mmu1,up1,up2,v1,v2) 
c
c sign change compared to the previous version (see notes p.4)
c does inluence only interference
c
       do ic1=1,2
       do ic2=1,2
c
c LO muon current
c
        gammu(1,ic1,ic2) = -(up1(1,ic1)*v1(1,ic2)+up1(2,ic1)*v1(2,ic2)
     &                    +up2(1,ic1)*v2(1,ic2)+up2(2,ic1)*v2(2,ic2))
        gammu(2,ic1,ic2) = -(-up1(1,ic1)*v1(2,ic2)-up1(2,ic1)*v1(1,ic2)
     &                     +up2(1,ic1)*v2(2,ic2)+up2(2,ic1)*v2(1,ic2))
        gammu(3,ic1,ic2) = -dcmplx(0.d0,1.d0)* 
     &                    (up1(1,ic1)*v1(2,ic2)-up1(2,ic1)*v1(1,ic2)
     &                    -up2(1,ic1)*v2(2,ic2)+up2(2,ic1)*v2(1,ic2))
        gammu(4,ic1,ic2) = -(-up1(1,ic1)*v1(1,ic2)+up1(2,ic1)*v1(2,ic2)
     &                     +up2(1,ic1)*v2(1,ic2)-up2(2,ic1)*v2(2,ic2))
c
c 'magnetic' current for muons FSR virtual corrections to ISR amplitude 
c
        if(fsrnlo.eq.1)then
          sss = -(up1(1,ic1)*v2(1,ic2)+up1(2,ic1)*v2(2,ic2)
     &           +up2(1,ic1)*v1(1,ic2)+up2(2,ic1)*v1(2,ic2))
          do ic3 =0,3
         gammu_ma(ic3+1,ic1,ic2) = (momenta(6,ic3)-momenta(7,ic3))*sss
          enddo
        endif
       enddo
       enddo
c
      elseif((pion.eq.1).or.(pion.eq.6).or.(pion.eq.7))then
c 
c --- 2 pions, kaons K^+K^-, K0K0bar ---

        if(pion.eq.1)f1 = PionFormFactor(qq)
        if((pion.eq.6).or.(pion.eq.7)) f1 = KaonFormFactor(qq)
      do mu = 0,3
         gam(mu+1) = (momenta(6,mu)-momenta(7,mu))*f1
      enddo             
c
c --- pions 2pi^0 pi^+ pi^- --- pion=2
c --- pions 2pi^+ 2pi^-     --- pion=3
c        
      elseif((pion.eq.2).or.(pion.eq.3))then
c
      do i =0,3
       q1(i+1) = momenta(6,i)  
       q2(i+1) = momenta(7,i) 
       q3(i+1) = momenta(8,i)  
       q4(i+1) = momenta(9,i) 
      enddo
c
c --- pions 2pi^0 pi^+ pi^-
c
       if(pion.eq.2)  call had3(qq,q1,q2,q3,q4,gam)
c
c --- pions 2pi^+ 2pi^-  
c
       if(pion.eq.3)  call had2(qq,q1,q2,q3,q4,gam)
c
c ---- PPbar (pion=4) and NNbar (pion=5)  ----
c
      elseif ((pion.eq.4).or.(pion.eq.5)) then

      if(pion.eq.4)  mmu1 = mp
      if(pion.eq.5)  mmu1 = mnt     

      call spinors(mmu1,up1,up2,v1,v2)
	if(FF_pp.eq.0)then
      call ProtonFormFactor(qq,protF1,protF2)
	elseif(FF_pp.eq.1)then
	call ProtonFormFactor_new(qq,protF1,protF2)
	endif
c
c --- no minus sign due to treating proton as a particle 
c --- and antiproton as a antiparticle
c
       do ic1=1,2
       do ic2=1,2
       gammu1(1,ic1,ic2) = (up1(1,ic1)*v1(1,ic2)+up1(2,ic1)*v1(2,ic2)
     &                    +up2(1,ic1)*v2(1,ic2)+up2(2,ic1)*v2(2,ic2))
       gammu1(2,ic1,ic2) = (-up1(1,ic1)*v1(2,ic2)-up1(2,ic1)*v1(1,ic2)
     &                     +up2(1,ic1)*v2(2,ic2)+up2(2,ic1)*v2(1,ic2))
       gammu1(3,ic1,ic2) = dcmplx(0.d0,1.d0)* 
     &                    (up1(1,ic1)*v1(2,ic2)-up1(2,ic1)*v1(1,ic2)
     &                    -up2(1,ic1)*v2(2,ic2)+up2(2,ic1)*v2(1,ic2))
       gammu1(4,ic1,ic2) = (-up1(1,ic1)*v1(1,ic2)+up1(2,ic1)*v1(2,ic2)
     &                     +up2(1,ic1)*v2(1,ic2)-up2(2,ic1)*v2(2,ic2))
    
       gammu2(ic1,ic2) = (up1(1,ic1)*v2(1,ic2)+up1(2,ic1)*v2(2,ic2)
     &                    +up2(1,ic1)*v1(1,ic2)+up2(2,ic1)*v1(2,ic2))
         do iic=1,4
       gammu(iic,ic1,ic2) =  gammu1(iic,ic1,ic2) * (protF1+protF2)
     &                   + protF2 * (momenta(6,iic-1)-momenta(7,iic-1))
     &                   * gammu2(ic1,ic2)/2.D0/mmu1
         enddo
       enddo
       enddo
c
c c --- pions pi^0 pi^+ pi^- or eta pi^+ pi^-
c
      elseif((pion.eq.8).or.(pion.eq.10))then

      do i =0,3
       q1(i+1) = momenta(6,i)  
       q2(i+1) = momenta(7,i) 
       q3(i+1) = momenta(8,i)  
      enddo
       if(pion.eq.8)then
        call had_3pi(qq,q1,q2,q3,gam,8)
       else
        call had_3pi(qq,q1,q2,q3,gam,10)
       endif
      else
       write(6,*)'wrong "pions" switch'
       stop
      endif
      return
      end
c ***************************************************************************
      subroutine  spinors(mmu1,up1,up2,v1,v2)
      include 'phokhara_10.0.inc'      
       real*8 th1,th2,sphi1,cphi1,sphi2,cphi2,cth1d2,sth1d2,
     &       cth2d2,sth2d2,sq1,sq2,em1,em2,pm1,pm2,sth1,sth2,mmu1
      complex*16 v1(2,2),v2(2,2),up1(2,2),up2(2,2),ex1,ex2
   
        em1 = momenta(7,0)
        em2 = momenta(6,0)
        pm1 = sqrt(em1**2-mmu1**2)
        pm2 = sqrt(em2**2-mmu1**2)
        sq1 = sqrt(em1+pm1)
        sq2 = sqrt(em2+pm2)
        th1 = acos(momenta(7,3)/pm1)
        th2 = acos(momenta(6,3)/pm2)
        cth1d2 = cos(th1/2.d0)
        cth2d2 = cos(th2/2.d0)
c
         v1(2,1)= -sq2*cth2d2
         v1(1,2)= mmu1/sq2*cth2d2
         v2(2,1)= v1(1,2)
         v2(1,2)= v1(2,1)
c
         up1(1,1) = mmu1/sq1*cth1d2
         up1(2,2) = sq1*cth1d2
         up2(1,1) = up1(2,2)
         up2(2,2) = up1(1,1)
c
        if((th2.eq.0.d0).or.(th2.eq.pi))then
         v1(1,1)= 0.d0
         v1(2,2)= 0.d0
         v2(1,1)= 0.d0
         v2(2,2)= 0.d0
        else
c
         sth2  = sin(th2)
         sth2d2= sin(th2/2.d0)
         cphi2 = momenta(6,1)/pm2/sth2
         sphi2 = momenta(6,2)/pm2/sth2
         ex2 = cphi2+dcmplx(0.d0,1.d0)*sphi2
c
         v1(1,1)= sq2*sth2d2/ex2
         v1(2,2)= mmu1/sq2*ex2*sth2d2
         v2(1,1)= -mmu1/sq2/ex2*sth2d2
         v2(2,2)= -sq2*ex2*sth2d2
        endif
c
        if((th1.eq.0.d0).or.(th1.eq.pi))then
         up1(1,2)=0.d0
         up1(2,1)=0.d0
         up2(1,2)=0.d0
         up2(2,1)=0.d0
        else
         sth1  = sin(th1)
         sth1d2= sin(th1/2.d0)
         cphi1 = momenta(7,1)/pm1/sth1
         sphi1 = momenta(7,2)/pm1/sth1
         ex1 = cphi1+dcmplx(0.d0,1.d0)*sphi1
c
         up1(1,2)= -sq1*ex1*sth1d2
         up1(2,1)= mmu1/sq1/ex1*sth1d2
         up2(1,2)= -mmu1/sq1*ex1*sth1d2
         up2(2,1)= sq1/ex1*sth1d2
        endif

         return
         end
c---------------------------------------------------------------------
c subroutine ProtonFormFactor includes proton (pion=4) 
c and neutron (pion=5) formractors as well
c---------------------------------------------------------------------
c FF_pp=0
      subroutine ProtonFormFactor(qq,protF1,protF2)
      include 'phokhara_10.0.inc'
      complex*16 protF1,protF2,F1ss,F1vv,F2ss,F2vv,alphafun_pp,
     &           meson_pp,gfun_pp,ex_pp
      real*8 qq
      real*8 betarho_pp, betaomega_pp,betaphi_pp, alphaphi_pp,mrho_pp,
     & momega_pp,mphi_pp,gammarho_pp,theta_pp,gam_pp

      common /protparam/ betarho_pp, betaomega_pp,betaphi_pp, 
     &alphaphi_pp,mrho_pp,momega_pp,mphi_pp,gammarho_pp,theta_pp,gam_pp


      ex_pp = cos(theta_pp)+dcmplx(0.d0,1.d0)*sin(theta_pp)

      alphafun_pp = sqrt((qq-4.D0*mpi**2)/qq)* ( -dcmplx(0.d0,1.d0) 
     &     + 2.D0/pi*log( (sqrt(qq-4.D0*mpi**2)+sqrt(qq))/(2.D0*mpi) ))
      gfun_pp = 1.D0/(1.D0-gam_pp*qq*ex_pp)**2
       
      meson_pp = (mrho_pp**2+8.D0*gammarho_pp*mpi/pi)
     &   /(qq-mrho_pp**2+(qq-4.D0*mpi**2)*gammarho_pp*alphafun_pp/mpi)
  
      F1ss = (1.D0-betaomega_pp-betaphi_pp)
     &    - betaomega_pp*momega_pp**2
     &    /(qq-momega_pp**2) - betaphi_pp*mphi_pp**2/(qq-mphi_pp**2)
 
      F1vv =  (1.D0-betarho_pp) - betarho_pp*meson_pp

      F2ss = (0.12D0+alphaphi_pp) * momega_pp**2/(qq-momega_pp**2)
     &     - alphaphi_pp*mphi_pp**2/(qq-mphi_pp**2) 
    
      F2vv = - 3.706D0*meson_pp

      F1ss = 0.5D0 * gfun_pp * F1ss
      F1vv = 0.5D0 * gfun_pp * F1vv
      F2ss = 0.5D0 * gfun_pp * F2ss
      F2vv = 0.5D0 * gfun_pp * F2vv     

      if(pion.eq.4) then
      protF1 = F1ss+F1vv
      protF2 = F2ss+F2vv
      endif
 
      if(pion.eq.5) then
      protF1 = F1ss-F1vv
      protF2 = F2ss-F2vv
      endif

      return
      end

c FF_pp=1
      subroutine ProtonFormFactor_new(qq,protF1,protF2)
      include 'phokhara_10.0.inc'
      complex*16 protF1,protF2,F1ss,F1vv,F2ss,F2vv,BW_om
      real*8 qq,aaaa,bbbb,mamom_p,mamom_n
      
      mamom_p = 2.792847356D0
      mamom_n = -1.9130427D0
        aaaa =  mamom_p - mamom_n -1.d0
        bbbb = -mamom_p - mamom_n +1.d0 
   
c      save

c      if(qq.gt.0.d0)then
c        ii = dcmplx(0.d0,1.d0)
c      else
c         ii = dcmplx(0.d0,0.d0)
c      endif


      F1ss =  (BW_om(momega_pp0,gammaomega_pp0,qq,1)
     &    +par1*BW_om(momega_pp1,gammaomega_pp1,qq,1)
     &    +par2*BW_om(momega_pp2,gammaomega_pp2,qq,1)
     &    +par3*BW_om(momega_pp3,gammaomega_pp3,qq,1)
     &     +par13*BW_om(momega_pp4,gammaomega_pp4,qq,1))
     &    /(1.d0+par1+par2+par3+par13)
 
     

      F1vv =  (BW_om(mrho_pp0,gammarho_pp0,qq,1)
     &    +par4*BW_om(mrho_pp1,gammarho_pp1,qq,1)
     &    +par5*BW_om(mrho_pp2,gammarho_pp2,qq,1)
     &    +par6*BW_om(mrho_pp3,gammarho_pp3,qq,1)
     &     +par14*BW_om(mrho_pp4,gammarho_pp4,qq,1))
     &   /(1.d0+par4+par5+par6+par14)
      


      F2ss =  -bbbb*(BW_om(momega_pp0,gammaomega_pp0,qq,1)
     &    +par7*BW_om(momega_pp1,gammaomega_pp1,qq,1)
     &    +par8*BW_om(momega_pp2,gammaomega_pp2,qq,1)
     &    +par9*BW_om(momega_pp3,gammaomega_pp3,qq,1)
     &    +par15*BW_om(momega_pp4,gammaomega_pp4,qq,1) )
     &    /(1.d0+par7+par8+par9+par15)
    


      F2vv =  aaaa*(BW_om(mrho_pp0,gammarho_pp0,qq,1)
     &    +par10*BW_om(mrho_pp1,gammarho_pp1,qq,1)
     &    +par11*BW_om(mrho_pp2,gammarho_pp2,qq,1)
     &    +par12*BW_om(mrho_pp3,gammarho_pp3,qq,1)
     &    +par16*BW_om(mrho_pp4,gammarho_pp4,qq,1))
     &   /(1.d0+par10+par11+par12+par16)


      F1ss = 0.5D0 * F1ss
      F1vv = 0.5D0 * F1vv
      F2ss = 0.5D0 * F2ss
      F2vv = 0.5D0 * F2vv     
c

      if(pion.eq.4) then
      protF1 = F1ss+F1vv
      protF2 = F2ss+F2vv
      endif
 
      if(pion.eq.5) then
      protF1 = F1ss-F1vv
      protF2 = F2ss-F2vv
      endif
c
      return
      end
c---------------------------------------------------------------------
c this function includes vacuum polarisation and J/psi or psi(2S) 
c as set by the input parameters     
c---------------------------------------------------------------------
      complex*16 function vacpol_and_nr(qq)
      include 'phokhara_10.0.inc'
      real*8 qq,aa_phi,mm_ph,gg_ph
      real*8 vprehadsp,vprehadtm,vpimhad,vprelepsp,vpreleptm,vpimlep,
     & vpretopsp,vpretoptm
      complex*16 dggvap,BW_om,result_thomas
      common/param_PFF/aa_phi,mm_ph,gg_ph
c
      if(ivac.eq.0)then
        vacpol_and_nr = dcmplx(1.d0,0.d0) 
      elseif(ivac.eq.1)then
        vacpol_and_nr = 1.d0/(1.d0-dggvap(qq,0.d0))
c
        if(pion.le.1)then
          vacpol_and_nr = vacpol_and_nr 
     1   - 3.d0*Br_phi_ee*gg_ph/alpha/mm_ph*BW_om(mm_ph,gg_ph,qq,1)
        endif
c
      elseif(ivac.eq.2)then
      call vphlmntv1nonr(sqrt(qq), 
     & vprehadsp,vprehadtm,vpimhad,vprelepsp,vpreleptm,vpimlep,
     & vpretopsp,vpretoptm)
      result_thomas = dcmplx(vprehadsp+vprelepsp+vpretopsp
     1               ,vpimhad+vpimlep)
        vacpol_and_nr = 1.d0/(1.d0-result_thomas)
c
c   the phi is included into Thomas vacpol (HC)
c        if(pion.le.1)then
c          vacpol_and_nr = vacpol_and_nr 
c     1   - 3.d0*Br_phi_ee*gg_ph/alpha/mm_ph*BW_om(mm_ph,gg_ph,qq,1)
c        endif
c
      else
       write(6,*)' wrong ivac parameter '
       stop
      endif
c
      if(narr_res.eq.1)then
        if((pion.le.1).or.(pion.eq.11).or.(pion.eq.12))then
          vacpol_and_nr = vacpol_and_nr         
     1   - 3.d0*sqrt(qq)*gamjpee/alpha/mjp**2*BW_om(mjp,gamjp,qq,1)
        elseif(pion.eq.6)then
          vacpol_and_nr = vacpol_and_nr*(1.d0+cjp_Kp)         
     1   - 3.d0*sqrt(qq)*gamjpee/alpha/mjp**2*BW_om(mjp,gamjp,qq,1)
        elseif(pion.eq.7)then
          vacpol_and_nr = vacpol_and_nr*(1.d0+cjp_K0)         
     1   - 3.d0*sqrt(qq)*gamjpee/alpha/mjp**2*BW_om(mjp,gamjp,qq,1)
        else
         continue
        endif
      elseif(narr_res.eq.2)then
        if(pion.le.1)then
          vacpol_and_nr = vacpol_and_nr         
     1   - 3.d0*sqrt(qq)*gamp2see/alpha/mp2s**2*BW_om(mp2s,gamp2s,qq,1)
        elseif(pion.eq.6)then
          vacpol_and_nr = vacpol_and_nr*(1.d0+cp2s_Kp)         
     1   - 3.d0*sqrt(qq)*gamp2see/alpha/mp2s**2*BW_om(mp2s,gamp2s,qq,1)
        elseif(pion.eq.7)then
          vacpol_and_nr = vacpol_and_nr*(1.d0+cp2s_K0)         
     1   - 3.d0*sqrt(qq)*gamp2see/alpha/mp2s**2*BW_om(mp2s,gamp2s,qq,1)
        else
         continue
        endif
      else
        continue
      endif
c
      return
      end
c ***************************************************************************
c     matrices and scalar products; mi=minus, pl=plus,
c     eck1=epsilon*(k1) etc.
c
      subroutine skalar1(rk1,rk2,gam,q)
      include 'phokhara_10.0.inc'
      real*8 dme,el_m2,p1,p2,q,rk1,rk2,rat1,cos1,cos2,
     1       rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2,
     2       rk2pi1,rk2pi2,rk1pi1,rk1pi2,qqa1,qqa2
      integer i1
      complex*16 gam,eck1,eck2,p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1        k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2) 
     3        ,pi1eck2,pi2eck2,pi1eck1,pi2eck1  
     3       ,gampl_ma(2,2),gammi_ma(2,2)
      real*8 Dk1k2q1,Dk1k2q2
c
      dimension p1(4),p2(4),rk1(4),rk2(4),gam(4),q(4),eck1(4),eck2(4)
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common /cp1p2/p1,p2,dme,el_m2
c
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
      common/iloczs4/pi1eck2,pi2eck2,rk2pi1,rk2pi2
      common/cqqa12/qqa1,qqa2
      common/newdenpions/Dk1k2q1,Dk1k2q2
c
      rat1 = el_m2/(p1(1)+p1(4))
c
      cos1 = rk1(4) / rk1(1)
      cos2 = rk2(4) / rk2(1)
c
      rk1p1 = rk1(1) * ( rat1 + p1(4) * (1.d0 - cos1) )
      rk1p2 = rk1(1) * ( rat1 + p1(4) * (1.d0 + cos1) )
c
      rk2p1 = rk2(1) * ( rat1 + p1(4) * (1.d0 - cos2) ) 
      rk2p2 = rk2(1) * ( rat1 + p1(4) * (1.d0 + cos2) )
c
      rk1rk2 = rk1(1)*rk2(1) - rk1(2)*rk2(2) - rk1(3)*rk2(3) -
     1          rk1(4)*rk2(4)
c
      anaw1 = rk1rk2 - rk1p1 - rk2p1
      anaw2 = rk1rk2 - rk1p2 - rk2p2
c
      call plus(q,qpl)
      call minus(q,qmi)
c
      call cplus(gam,gampl)
      call cminus(gam,gammi)
c
      call plus(rk1,k1pl)
      call minus(rk1,k1mi)
c
      call plus(rk2,k2pl)
      call minus(rk2,k2mi)
c     
c     scalar products multiplied by 2, not reflected in their names !
c
      p1gam=2.d0*(p1(1)*gam(1)-p1(2)*gam(2)-p1(3)*gam(3)-
     1            p1(4)*gam(4))
      p2gam=2.d0*(p2(1)*gam(1)-p2(2)*gam(2)-p2(3)*gam(3)-
     1            p2(4)*gam(4))      
c
      rk1pi1 = rk1(1)*momenta(6,0)      
      rk1pi2 = rk1(1)*momenta(7,0)
      do i1 =1,3
       rk1pi1 = rk1pi1 - rk1(i1+1)*momenta(6,i1)
       rk1pi2 = rk1pi2 - rk1(i1+1)*momenta(7,i1)
      enddo     

      rk2pi1 = rk2(1)*momenta(6,0)      
      rk2pi2 = rk2(1)*momenta(7,0)
      do i1 =1,3
       rk2pi1 = rk2pi1 - rk2(i1+1)*momenta(6,i1)
       rk2pi2 = rk2pi2 - rk2(i1+1)*momenta(7,i1)
      enddo     
c
      if(fsrnlo.eq.1) call qqa1qqa2(rk1,rk2)

c pions : the two photons 2FSR
c
c******** Denominators: 1 / ((k1+k1+qi)^2 - m^2)
c
      if((nlo2.eq.1).and.(pion.eq.1)) then
c
	Dk1k2q1 = Sp - 2.d0*dsqrt(Sp)*momenta(7,0)
	Dk1k2q2 = Sp - 2.d0*dsqrt(Sp)*momenta(6,0)
c
      endif
c********

      return
      end
c *************************************************************************
c     matrices and scalar products; mi=minus, pl=plus,
c     eck1=epsilon*(k1) etc.
c
      subroutine skalar2(rk1,rk2,gam,q,eck1,eck2,qq)
      include 'phokhara_10.0.inc' 
      real*8 qq
      real*8 dme,el_m2,p1,p2,q,rk1,rk2
      real*8 rk2pi1,rk2pi2,rk1pi1,rk1pi2,qqa1,qqa2
      integer i1
      complex*16 gam,eck1,eck2,p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2) 
     3       ,gampl_ma(2,2),gammi_ma(2,2)
      complex*16 pi1eck2,pi2eck2,pi1eck1,pi2eck1,gam1pimi(2,2),
     1           gam1pipl(2,2),gam2pimi(2,2),gam2pipl(2,2)
      complex*16 rk1eck2,rk2eck1,eck1eck2

      dimension p1(4),p2(4),rk1(4),rk2(4),gam(4),q(4),eck1(4),eck2(4)
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common /cp1p2/p1,p2,dme,el_m2
c
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
      common/iloczs4/pi1eck2,pi2eck2,rk2pi1,rk2pi2
      common/cqqa12/qqa1,qqa2
      common/newpions/rk1eck2,rk2eck1,eck1eck2
c
      call cplus(eck1,eck1pl)
      call cminus(eck1,eck1mi)
c
      call cplus(eck2,eck2pl)
      call cminus(eck2,eck2mi)
c     
c     scalar products multiplied by 2, not reflected in their names !
c
      p1eck1=2.d0*(p1(1)*eck1(1)-p1(2)*eck1(2)-p1(3)*eck1(3)-
     1            p1(4)*eck1(4))
      p1eck2=2.d0*(p1(1)*eck2(1)-p1(2)*eck2(2)-p1(3)*eck2(3)-
     1            p1(4)*eck2(4))
      p2eck1=2.d0*(p2(1)*eck1(1)-p2(2)*eck1(2)-p2(3)*eck1(3)-
     1            p2(4)*eck1(4))
      p2eck2=2.d0*(p2(1)*eck2(1)-p2(2)*eck2(2)-p2(3)*eck2(3)-
     1            p2(4)*eck2(4))
c
      pi1eck1 = momenta(6,0)*eck1(1)      
      pi2eck1 = momenta(7,0)*eck1(1)      
      do i1 =1,3
       pi1eck1 = pi1eck1 - momenta(6,i1)*eck1(i1+1)
       pi2eck1 = pi2eck1 - momenta(7,i1)*eck1(i1+1)
      enddo

      pi1eck2 = momenta(6,0)*eck2(1)      
      pi2eck2 = momenta(7,0)*eck2(1)      
      do i1 =1,3
       pi1eck2 = pi1eck2 - momenta(6,i1)*eck2(i1+1)
       pi2eck2 = pi2eck2 - momenta(7,i1)*eck2(i1+1)
      enddo

c pions : the two photons 2FSR
c
      if((nlo2.eq.1).and.(pion.eq.1)) then
c
      eck1eck2 = eck1(1)*eck2(1)
      rk2eck1 = rk2(1)*eck1(1)
      rk1eck2 = rk1(1)*eck2(1) 
      do i1 =1,3
       eck1eck2 = eck1eck2 - eck1(i1+1)*eck2(i1+1)
       rk2eck1 = rk2eck1 - rk2(i1+1)*eck1(i1+1)
       rk1eck2 = rk1eck2 - rk1(i1+1)*eck2(i1+1)
      enddo
c
      endif
c*****

      if((fsrnlo.eq.1).and.((pion.eq.1).or.(pion.eq.6))) 
     1                            call picurr(rk1,rk2,eck1,eck2,qq)
      
      return
      end
c ************************************************************************
      subroutine qqa1qqa2(rk1,rk2)

      include 'phokhara_10.0.inc'       
      real*8 rk1(4),rk2(4)
      real*8 qqa1,qqa2,prad1(4),prad2(4)
      integer i1,i2
      common/cqqa12/qqa1,qqa2
      common/przekaz/prad1,prad2
      do i1=1,4
       prad1(i1) = momenta(6,i1-1)+momenta(7,i1-1)+rk1(i1)
       prad2(i1) = momenta(6,i1-1)+momenta(7,i1-1)+rk2(i1)
      enddo
     
      qqa1 = prad1(1)**2
      qqa2 = prad2(1)**2
      do i1=1,3
       qqa1 = qqa1 - prad1(i1+1)**2
       qqa2 = qqa2 - prad2(i1+1)**2
      enddo

      return
      end
c ************************************************************************
      subroutine picurr(rk1,rk2,eck1,eck2,qq)
      include 'phokhara_10.0.inc'       
      real*8 rk1(4),rk2(4),qq
      complex*16 eck1(4),eck2(4),pi1eck2,pi2eck2,pi1eck1,pi2eck1
      complex*16 gam1pi(4),gam2pi(4)
      complex*16 gam1pimi(2,2),gam1pipl(2,2),gam2pimi(2,2),gam2pipl(2,2)
      complex*16 f1,f2,PionFormFactor,KaonFormFactor,ii,
     1           Fun_phi1,Fun_phi2,F_phi_KK,phas1,phas2,F_phi_no,
     2           BW_om,cvac_qqa1,cvac_qqa2,dggvap,F_phi_exp,
     3           cvac_qqa1a,cvac_qqa2a,vacpol_and_nr,result_thomas
      real*8 rk2pi1,rk2pi2,rk1pi1,rk1pi2,qqa1,qqa2,prad1(4),prad2(4),
     1       aa_phi,mm_ph,gg_ph,phas_rho_f0_exp,mm_f0_exp,c_f0_KK_exp,
     1       c_f0_pipi_exp,ff_phi_exp,c_phi_KK,
     2       rho_phi_exp,lamb_phi_exp,phas_rho_f0
      real*8 vprehadsp,vprehadtm,vpimhad,vprelepsp,vpreleptm,vpimlep,
     & vpretopsp,vpretoptm
      integer i1,i2
c
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
      common/iloczs4/pi1eck2,pi2eck2,rk2pi1,rk2pi2
      common/matri3/gam1pimi,gam1pipl,gam2pimi,gam2pipl
      common/phases_f0/phas1,phas2
      common/cqqa12/qqa1,qqa2
      common/param_PFF/aa_phi,mm_ph,gg_ph
      common/param_f0_exp/mm_f0_exp,phas_rho_f0_exp,c_f0_KK_exp,
     1      c_f0_pipi_exp,ff_phi_exp,c_phi_KK,rho_phi_exp,lamb_phi_exp

c --- pi+ pi- ---
      if(pion.eq.1)then
        cvac_qqa1 = vacpol_and_nr(qqa1)
        cvac_qqa2 = vacpol_and_nr(qqa2)
      if(ivac.eq.0)then
        cvac_qqa1a = dcmplx(1.d0,0.d0) 
        cvac_qqa2a = dcmplx(1.d0,0.d0)
      elseif(ivac.eq.1)then
        cvac_qqa1a = 1.d0/(1.d0-dggvap(qqa1,0.d0))
        cvac_qqa2a = 1.d0/(1.d0-dggvap(qqa2,0.d0))
      else
      call vphlmntv1nonr(sqrt(qqa1), 
     & vprehadsp,vprehadtm,vpimhad,vprelepsp,vpreleptm,vpimlep,
     & vpretopsp,vpretoptm)
      result_thomas = dcmplx(vprehadsp+vprelepsp+vpretopsp
     1               ,vpimhad+vpimlep)
        cvac_qqa1a = 1.d0/(1.d0-result_thomas)
      call vphlmntv1nonr(sqrt(qqa2), 
     & vprehadsp,vprehadtm,vpimhad,vprelepsp,vpreleptm,vpimlep,
     & vpretopsp,vpretoptm)
      result_thomas = dcmplx(vprehadsp+vprelepsp+vpretopsp
     1               ,vpimhad+vpimlep)
        cvac_qqa2a = 1.d0/(1.d0-result_thomas)
      endif

      f1 = PionFormFactor(qqa1)
c ---- f0 + f0(600)
      if(rk1(1).ge.gmin) then
        if(f0_model.eq.0)then
           Fun_phi1 = F_phi_KK(qq,qqa1)
           phas_rho_f0 = 75.d0*dSqrt(qq-4.d0*mpi**2)
           phas1 = exp(dcmplx(0.d0,phas_rho_f0*pi/180.d0))
        elseif(f0_model.eq.1)then
           Fun_phi1 = F_phi_no(qq,qqa1)
        elseif(f0_model.eq.2)then
           Fun_phi1 = (0.d0,0.d0)
        elseif(f0_model.eq.3)then
           Fun_phi1 = - F_phi_exp(qq,qqa1)    
           phas1 = exp(dcmplx(0.d0,
     1           phas_rho_f0_exp*dSqrt(1.d0-4.d0*mpi**2/qq)*pi/180.d0))
        endif
      else
         Fun_phi1 = (0.d0,0.d0)
      endif

         do i1=1,4
         gam1pi(i1) = 
     1   ( (momenta(6,i1-1)+rk1(i1)-momenta(7,i1-1))*pi1eck1/rk1pi1
     2   + (momenta(7,i1-1)+rk1(i1)-momenta(6,i1-1))*pi2eck1/rk1pi2
     3    - 2.d0*eck1(i1) )*f1*cvac_qqa1 
     4   + phas1 * Fun_phi1 * 
     6      ( (rk1pi1+rk1pi2)*eck1(i1) - (pi1eck1+pi2eck1)*rk1(i1) )
     7   * cvac_qqa1a
         enddo

         f2 = PionFormFactor(qqa2)
c ---- f0 + f0(600)
      if(rk2(1).ge.gmin) then
        if(f0_model.eq.0)then
           Fun_phi2 = F_phi_KK(qq,qqa2)
           phas_rho_f0 = 75.d0*dSqrt(qq-4.d0*mpi**2)
           phas1 = exp(dcmplx(0.d0,phas_rho_f0*pi/180.d0))
        elseif(f0_model.eq.1)then
           Fun_phi2 = F_phi_no(qq,qqa2)
        elseif(f0_model.eq.2)then
           Fun_phi2 = (0.d0,0.d0)
        elseif(f0_model.eq.3)then
           Fun_phi2 = - F_phi_exp(qq,qqa2)    
           phas1 = exp(dcmplx(0.d0,
     1           phas_rho_f0_exp*dSqrt(1.d0-4.d0*mpi**2/qq)*pi/180.d0))
        endif
      else
        Fun_phi2 = (0.d0,0.d0)
      endif

         do i1=1,4
         gam2pi(i1) = 
     1   ( (momenta(6,i1-1)+rk2(i1)-momenta(7,i1-1))*pi1eck2/rk2pi1
     2   + (momenta(7,i1-1)+rk2(i1)-momenta(6,i1-1))*pi2eck2/rk2pi2
     3             - 2.d0*eck2(i1) )*f2*cvac_qqa2
     4   + phas1 * Fun_phi2 * 
     6      ( (rk2pi1+rk2pi2)*eck2(i1) - (pi1eck2+pi2eck2)*rk2(i1) )
     7   * cvac_qqa2a
         enddo

c --- K+ K- ---
      elseif(pion.eq.6) then
      f1 = KaonFormFactor(qqa1)
         do i1=1,4
         gam1pi(i1) = 
     1   ( (momenta(6,i1-1)+rk1(i1)-momenta(7,i1-1))*pi1eck1/rk1pi1
     2   + (momenta(7,i1-1)+rk1(i1)-momenta(6,i1-1))*pi2eck1/rk1pi2
     3             - 2.d0*eck1(i1) )*f1
         enddo
      f2 = KaonFormFactor(qqa2)
         do i1=1,4
         gam2pi(i1) = 
     1   ( (momenta(6,i1-1)+rk2(i1)-momenta(7,i1-1))*pi1eck2/rk2pi1
     2   + (momenta(7,i1-1)+rk2(i1)-momenta(6,i1-1))*pi2eck2/rk2pi2
     3             - 2.d0*eck2(i1) )*f2
         enddo
       endif

       call cplus(gam1pi,gam1pipl)
       call cminus(gam1pi,gam1pimi)

       call cplus(gam2pi,gam2pipl)
       call cminus(gam2pi,gam2pimi)
      return
      end
c****************************************************************************
c     multiplication of the 2x2 matrices
c
      subroutine matr(mat1,mat2,mat3)
      implicit none
      integer i,j,k
      complex*16  mat1(2,2),mat2(2,2),mat3(2,2)
c
      do i=1,2
         do j=1,2
            mat3(i,j)=(0.d0,0.d0)
         enddo
      enddo
c   
      do i=1,2
         do j=1,2
            do k=1,2
               mat3(i,j)=mat3(i,j)+mat1(i,k)*mat2(k,j)
            enddo
         enddo
      enddo
c
      end
c****************************************************************************
c     multiplication of the 4x2x2 and 2x2 matrices with the (4)index fixed
c     mu - is this index; the result is 2x2 matrix
c
      subroutine matr1(mu,mat1,mat2,mat3)
      implicit none
      complex*16  mat1(4,2,2),mat2(2,2),mat3(2,2)
      integer i,j,k,mu
c
      do i=1,2
         do j=1,2
            mat3(i,j)=(0.d0,0.d0)
         enddo
      enddo
c   
      do i=1,2
         do j=1,2
            do k=1,2
               mat3(i,j)=mat3(i,j)+mat1(mu,i,k)*mat2(k,j)
            enddo
         enddo
      enddo
c
      end
c****************************************************************************
c     multiplication of the 2x2 and 4x2x2 matrices with the (4)index fixed
c     mu - is this index; the result is 2x2 matrix
c
      subroutine matr2(mu,mat1,mat2,mat3)
      implicit none
      complex*16  mat1(2,2),mat2(4,2,2),mat3(2,2)
      integer i,j,k,mu
c
      do i=1,2
         do j=1,2
            mat3(i,j)=(0.d0,0.d0)
         enddo
      enddo
c   
      do i=1,2
         do j=1,2
            do k=1,2
               mat3(i,j)=mat3(i,j)+mat1(i,k)*mat2(mu,k,j)
            enddo
         enddo
      enddo
c
      end
c****************************************************************************
c     multiplication of a 2x2 matrix by a constant
c
      subroutine conmat(alfa,mat,amat)
      implicit none
      integer i,j
      complex*16 alfa
      complex*16 mat(2,2),amat(2,2)
c
      do i=1,2
         do j=1,2
            amat(i,j)=alfa*mat(i,j)
         enddo
      enddo
c
      end
c****************************************************************************
c     2x2 matrix subtraction
c
      subroutine minmat(mat1,mat2,mat3)
      implicit none
      integer i,j
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2)
c
      do i=1,2
         do j=1,2
            mat3(i,j)=mat1(i,j)-mat2(i,j)
         enddo
      enddo
c
      end
c*****************************************************************************
c     2x2 matrix subtraction
c
      subroutine minmat_new(mu,mat1,mat2,mat3)
      implicit none
      integer i,j,mu
      complex*16 mat1(2,2),mat2(2,2),mat3(4,2,2)
c
      do i=1,2
         do j=1,2
            mat3(mu,i,j)=mat1(i,j)-mat2(i,j)
         enddo
      enddo
c
      end
c****************************************************************************
c     2x2 matrix adding two matrices
c
      subroutine dodmat(mat1,mat2,mat3)
      implicit none
      integer i,j
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2)
c
      do i=1,2
         do j=1,2
            mat3(i,j)=mat1(i,j)+mat2(i,j)
         enddo
      enddo
c
      end
c****************************************************************************
c     addind 6 matrices: with proper denominators
c
      subroutine plumat(mat1,mat2,mat3,mat4,mat5,mat6,mat7)      
      implicit none
      real*8 rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      integer i,j
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2),mat4(2,2),mat5(2,2),
     1          mat6(2,2),mat7(2,2)
c
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
c
      do i=1,2
         do j=1,2
            mat7(i,j)= -0.25d0*mat1(i,j)/rk2p1/anaw1
     1                 -0.25d0*mat2(i,j)/rk1p1/anaw1
     2                 +0.25d0*mat3(i,j)/rk1p1/rk2p2
     3                 +0.25d0*mat4(i,j)/rk2p1/rk1p2
     4                 -0.25d0*mat5(i,j)/rk2p2/anaw2
     5                 -0.25d0*mat6(i,j)/rk1p2/anaw2
         enddo
      enddo
c
      end
c****************************************************************************

      subroutine blocks(rk1,rk2)
c
      include 'phokhara_10.0.inc'       
      real*8 rk1(4),rk2(4)
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2         eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2)
     3       ,gampl_ma(2,2),gammi_ma(2,2)
      complex*16 m1(2,2),m2(2,2),m3(2,2),m4(2,2),m5(2,2),m6(2,2),
     1          n1(2,2),n2(2,2),n3(2,2),n4(2,2),n5(2,2),n6(2,2),n7(2,2),
     2          n8(2,2),n9(2,2),n10(2,2),n11(2,2),n12(2,2)
      complex*16 block1(2,2),block2(2,2),block3(2,2),block4(2,2),
     1          block5(2,2),block6(2,2),block7(2,2),block8(2,2),
     2          block9(2,2),block10(2,2),block11(2,2),block12(2,2)
      complex*16 m1amp1(2,2),mamp1a(2,2),m2amp1(2,2),mamp1b(2,2),
     1           m1amp2(2,2),mamp2a(2,2),m2amp2(2,2),mamp2b(2,2),
     2           m1amp3(2,2),mamp3a(2,2),m2amp3(2,2),mamp3b(2,2), 
     3           m1amp4(2,2),mamp4a(2,2),m2amp4(2,2),mamp4b(2,2),
     4           m1amp5(2,2),mamp5a(2,2),m2amp5(2,2),mamp5b(2,2),
     5           m1amp6(2,2),mamp6a(2,2),m2amp6(2,2),mamp6b(2,2)
      complex*16 m1amp7pi1(2,2),m2amp7pi1(2,2),m1amp8pi1(2,2),
     1           m2amp8pi1(2,2),m1amp7pi2(2,2),m2amp7pi2(2,2),
     2           m1amp8pi2(2,2),m2amp8pi2(2,2)
      complex*16 gam1pimi(2,2),gam1pipl(2,2),gam2pimi(2,2),gam2pipl(2,2)
      complex*16 mapi1(2,2),mbpi1(2,2),mapi2(2,2),mbpi2(2,2)
      complex*16 ma(2,2),mb(2,2),ma_ma(2,2),mb_ma(2,2)
      complex*16 Jvect(1:4),Jplus(1:2,1:2),Jmin(1:2,1:2)
      integer i1,i2
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1             eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/matri1/ma,mb,ma_ma,mb_ma
      common/matri2/mapi1,mbpi1,mapi2,mbpi2
      common/matri3/gam1pimi,gam1pipl,gam2pimi,gam2pipl
      common/Jvectcom/Jplus,Jmin
c
c two photons ISR
c
      call conmat(p2eck1,I,m1)
      call conmat(p2eck2,I,m2)
      call conmat(p1eck1,I,m3)
      call conmat(p1eck2,I,m4)
      call conmat(p2gam,I,m5)
      call conmat(p1gam,I,m6)
c
      call matr(qpl,gammi,n1)
      call matr(qmi,gampl,n2)
      call matr(gammi,qpl,n3)
      call matr(gampl,qmi,n4)
      call matr(eck1mi,k1pl,n5)
      call matr(eck1pl,k1mi,n6)
      call matr(eck2mi,k2pl,n7)
      call matr(eck2pl,k2mi,n8)
      call matr(k2pl,eck2mi,n9)
      call matr(k2mi,eck2pl,n10)
      call matr(k1pl,eck1mi,n11)
      call matr(k1mi,eck1pl,n12)
c
      call minmat(n7,m4,block1)
      call minmat(n8,m4,block2)
      call minmat(m5,n1,block3)
      call minmat(m5,n2,block4)
      call minmat(n5,m3,block5)
      call minmat(n6,m3,block6)
      call minmat(m2,n9,block7)
      call minmat(m2,n10,block8)
      call minmat(m1,n11,block9)
      call minmat(m1,n12,block10)
      call minmat(n3,m6,block11)
      call minmat(n4,m6,block12)
c
c     m1amp1=macierz powstala w wyniku mnozenia pierwszych dwoch 
c     macierzy, z pierwszego czlony w ampl1
c     mamp1a=macierz powstala w wyniku wymnozenia wszystkich macierzy
c     w pierwszym czlonie amp1 itd.
c
c mnozenie macierzy dla amp1
      call matr(block1,eck1mi,m1amp1)
      call matr(m1amp1,block3,mamp1a)
      call matr(block2,eck1pl,m2amp1)
      call matr(m2amp1,block4,mamp1b)
c
c mnozenie macierzy dla amp2
      call matr(block5,eck2mi,m1amp2)
      call matr(m1amp2,block3,mamp2a)
      call matr(block6,eck2pl,m2amp2)
      call matr(m2amp2,block4,mamp2b)
c
c mnozenie macierzy dla amp3
      call matr(block5,gammi,m1amp3)
      call matr(m1amp3,block7,mamp3a)
      call matr(block6,gampl,m2amp3)
      call matr(m2amp3,block8,mamp3b)
c
c mnozenie macierzy dla amp4
      call matr(block1,gammi,m1amp4)
      call matr(m1amp4,block9,mamp4a)
      call matr(block2,gampl,m2amp4)
      call matr(m2amp4,block10,mamp4b)
c
c mnozenie macierzy dla amp5
      call matr(block11,eck1mi,m1amp5)
      call matr(m1amp5,block7,mamp5a)
      call matr(block12,eck1pl,m2amp5)
      call matr(m2amp5,block8,mamp5b)
c
c mnozenie macierzy dla amp6
      call matr(block11,eck2mi,m1amp6)
      call matr(m1amp6,block9,mamp6a)
      call matr(block12,eck2pl,m2amp6)
      call matr(m2amp6,block10,mamp6b)
c
c dodawanie macierzy typu a i typu b
      call plumat(mamp1a,mamp2a,mamp3a,mamp4a,mamp5a,mamp6a,ma)
      call plumat(mamp1b,mamp2b,mamp3b,mamp4b,mamp5b,mamp6b,mb)
c
c pions : the two photons 1ISR and 1FSR
c
      if((fsrnlo.eq.1).and.((pion.eq.1).or.(pion.eq.6))) then

c k1 emitted from final state
      call matr(block1,gam1pimi,m1amp7pi1)
      call matr(block2,gam1pipl,m2amp7pi1)

      call matr(gam1pimi,block7,m1amp8pi1)
      call matr(gam1pipl,block8,m2amp8pi1)
c
      call plumat1(m1amp7pi1,m1amp8pi1,mapi1)
      call plumat1(m2amp7pi1,m2amp8pi1,mbpi1)

c k2 emitted from final state
      call matr(block5,gam2pimi,m1amp7pi2)
      call matr(block6,gam2pipl,m2amp7pi2)
c
      call matr(gam2pimi,block9,m1amp8pi2)
      call matr(gam2pipl,block10,m2amp8pi2)
c
      call plumat2(m1amp7pi2,m1amp8pi2,mapi2)
      call plumat2(m2amp7pi2,m2amp8pi2,mbpi2)

      endif

c pions : the two photons 2FSR
c
      if((nlo2.eq.1).and.(pion.eq.1)) then
c
      call Jvector(Jvect,rk1,rk2)
      call cplus(Jvect,Jplus)
      call cminus(Jvect,Jmin)
c
      endif
c
      return
      end
c*************************************************************************

      subroutine blocks_mu(uupp1,uupp2,vv1,vv2) 
      include 'phokhara_10.0.inc'     
      real*8 rk2pi1,rk2pi2,rk1pi1,rk1pi2,qqa1,qqa2
      complex*16 mapi1(2,2),mbpi1(2,2),mapi2(2,2),mbpi2(2,2)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 wyn1_k1,wyn1_k2,wyn2_k1,wyn2_k2,dd_k1(4),dd_k2(4),
     1           ddpl_k1(2,2),ddpl_k2(2,2),ddmi_k1(2,2),ddmi_k2(2,2)
      complex*16 m1(2,2),m2(2,2),m3(2,2),m4(2,2),m5(2,2),m6(2,2),
     1           m7(2,2),m8(2,2),n1(2,2),n2(2,2),n3(2,2),n4(2,2),
     2           n5(2,2),n6(2,2),n7(2,2),n8(2,2)
      complex*16 block1(2,2),block2(2,2),block3(2,2),block4(2,2),
     1          block5(2,2),block6(2,2),block7(2,2),block8(2,2),
     2          block9(2,2),block10(2,2),block11(2,2),block12(2,2),
     3          block13(2,2),block14(2,2),block15(2,2),block16(2,2)
      complex*16 m1amp3(2,2),m2amp3(2,2),m1amp4(2,2),m2amp4(2,2),
     4           m1amp5(2,2),m2amp5(2,2),m1amp6(2,2),m2amp6(2,2)
      complex*16 mm1a(2,2),mm1b(2,2),mm2a(2,2),mm2b(2,2),
     1           mm3a(2,2),mm3b(2,2),mm4a(2,2),mm4b(2,2),
     2           mmb_k1(2,2),mmb_k2(2,2),mma_k1(2,2),mma_k2(2,2)
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2),k1pl(2,2),
     1          k1mi(2,2),k2pl(2,2),k2mi(2,2),eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),sigpl(4,2,2),sigmi(4,2,2),
     3        gampl_ma(2,2),gammi_ma(2,2)
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      complex*16 pi1eck2,pi2eck2,pi1eck1,pi2eck1
      real*8 prad1(4),prad2(4)
      complex*16 q2pl(2,2),q2mi(2,2),q1pl(2,2),q1mi(2,2),
     1 sigplq1mi(2,2),q1plsigmi(2,2),sigmiq1pl(2,2),q1misigpl(2,2),
     2 sigplq2mi(2,2),q2plsigmi(2,2),sigmiq2pl(2,2),q2misigpl(2,2),
     3 mk3(4,2,2),mk4(4,2,2),mk5(4,2,2),mk6(4,2,2),protF1,protF2,
     4 ww1(2,2),ww2(2,2),ww3(2,2),ww4(2,2),wa1_k2(2,2),wa2_k2(2,2),
     5 wym1_k2,wym2_k2,ww5(2,2),ww6(2,2),ww7(2,2),ww8(2,2),
     6 wa1_k1(2,2),wa2_k1(2,2),wym1_k1,wym2_k1,protF1_2,protF2_2
      integer i1
c FSR muons
      complex*16 newblock1(2,2),newblock2(2,2),newblock3(2,2),
     1           newblock4(2,2),newblock5(2,2),newblock6(2,2),
     2           newblock7(2,2),newblock8(2,2)
c
      common/matri2/mapi1,mbpi1,mapi2,mbpi2
      common/cqqa12/qqa1,qqa2
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1            eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
      common/iloczs4/pi1eck2,pi2eck2,rk2pi1,rk2pi2
      common/przekaz/prad1,prad2
      common/newbloc_mu/newblock1,newblock2,newblock3,newblock4,
     1                  newblock5,newblock6,newblock7,newblock8,
     2                  block1,block2,block3,block4,block9,
     3                  block10,block11,block12


c

       call plus(prad1,q2pl)
       call minus(prad1,q2mi)
       call plus(prad2,q1pl)
       call minus(prad2,q1mi)
      do i1=1,4
       call matr1(i1,sigpl,q1mi,sigplq1mi)
       call matr2(i1,q1pl,sigmi,q1plsigmi)
       call matr1(i1,sigmi,q1pl,sigmiq1pl)
       call matr2(i1,q1mi,sigpl,q1misigpl)
       call matr1(i1,sigpl,q2mi,sigplq2mi)
       call matr2(i1,q2pl,sigmi,q2plsigmi)
       call matr1(i1,sigmi,q2pl,sigmiq2pl)
       call matr2(i1,q2mi,sigpl,q2misigpl)
       call minmat_new(i1,sigplq1mi,q1plsigmi,mk3)
       call minmat_new(i1,sigmiq1pl,q1misigpl,mk4)
       call minmat_new(i1,sigplq2mi,q2plsigmi,mk5)
       call minmat_new(i1,sigmiq2pl,q2misigpl,mk6)
          enddo
c
c the two photons: one ISR, one FSR (muons)
c
c k1 FSR
c
c muons + k1
       call conmat(2.d0*pi1eck1,I,m1)
       call conmat(2.d0*pi2eck1,I,m2)
       call matr(eck1mi,k1pl,n1)
       call matr(eck1pl,k1mi,n2)
       call matr(k1pl,eck1mi,n3)
       call matr(k1mi,eck1pl,n4)
       call dodmat(m1,n3,block1)
       call dodmat(m1,n4,block2)
       call dodmat(m2,n1,block3)
       call dodmat(m2,n2,block4)

      do i1=1,4
       call matr2(i1,block3,sigmi,mm1a)
       call matr2(i1,block4,sigpl,mm1b)
       call matr1(i1,sigmi,block1,mm2a)
       call matr1(i1,sigpl,block2,mm2b)
c 
       call plumat1_LO(qqa1,mm1a,mm2a,mma_k1)
       call plumat1_LO(qqa1,mm1b,mm2b,mmb_k1)
       call stal(uupp1,mma_k1,vv1,wyn1_k1)
       call stal(uupp2,mmb_k1,vv2,wyn2_k1)
       dd_k1(i1) = wyn1_k1+wyn2_k1
      enddo
c        magnetic part proton k1
       if(pion.eq.4)then
       if(FF_pp.eq.0)then
      call  ProtonFormFactor(qqa1,protF1,protF2)
      elseif(FF_pp.eq.1)then
       call  ProtonFormFactor_new(qqa1,protF1,protF2)
      endif

      do i1=1,4
       call matr1(i1,mk5,block1,ww5)
       call matr2(i1,block4,mk5,ww6)
       call matr1(i1,mk6,block2,ww7)
       call matr2(i1,block3,mk6,ww8)
       call plumat1_LO(qqa1,ww6,ww5,wa1_k1)
       call plumat1_LO(qqa1,ww8,ww7,wa2_k1)
       call stal(uupp2,wa1_k1,vv1,wym1_k1)
       call stal(uupp1,wa2_k1,vv2,wym2_k1)
       dd_k1(i1)=protF1*dd_k1(i1)-(wym1_k1+wym2_k1)*protF2/4.d0/mp

      enddo
      endif
c*************************************************************************
c
      call cplus(dd_k1,ddpl_k1)
      call cminus(dd_k1,ddmi_k1)

c e+ e- + k2
      call conmat(p2eck2,I,m4)
      call conmat(p1eck2,I,m3)
c
      call matr(eck2mi,k2pl,n5)
      call matr(eck2pl,k2mi,n6)
      call matr(k2pl,eck2mi,n7)
      call matr(k2mi,eck2pl,n8)
c
      call minmat(n5,m3,block5)
      call minmat(n6,m3,block6)
      call minmat(m4,n7,block7)
      call minmat(m4,n8,block8)
c
      call matr(block5,ddmi_k1,m1amp3)
      call matr(block6,ddpl_k1,m2amp3)
c
      call matr(ddmi_k1,block7,m1amp4)
      call matr(ddpl_k1,block8,m2amp4)
c
c     adding matrices of the status A and B
c
      call plumatLO_2(m1amp3,m1amp4,mapi1)
      call plumatLO_2(m2amp3,m2amp4,mbpi1)
 
c k2 FSR
c
c muons or protons + k2
       call conmat(2.d0*pi1eck2,I,m5)
       call conmat(2.d0*pi2eck2,I,m6)
       call dodmat(m5,n7,block9)
       call dodmat(m5,n8,block10)
       call dodmat(m6,n5,block11)
       call dodmat(m6,n6,block12)

      do i1=1,4
       call matr2(i1,block11,sigmi,mm3a)
       call matr2(i1,block12,sigpl,mm3b)
       call matr1(i1,sigmi,block9,mm4a)
       call matr1(i1,sigpl,block10,mm4b)
       call plumat1_LO_2(mm3a,mm4a,mma_k2)
       call plumat1_LO_2(mm3b,mm4b,mmb_k2)
       call stal(uupp1,mma_k2,vv1,wyn1_k2)
       call stal(uupp2,mmb_k2,vv2,wyn2_k2)
       dd_k2(i1) = wyn1_k2+wyn2_k2
      enddo
      
c magnetic part proton k2
       if(pion.eq.4)then
       if(FF_pp.eq.0)then
      call  ProtonFormFactor(qqa2,protF1_2,protF2_2)
      elseif(FF_pp.eq.1)then
       call  ProtonFormFactor_new(qqa2,protF1_2,protF2_2)
      endif
      do i1=1,4
       call matr1(i1,mk3,block9,ww1)
       call matr2(i1,block12,mk3,ww2)
       call matr1(i1,mk4,block10,ww3)
       call matr2(i1,block11,mk4,ww4)
       call plumat1_LO_2(ww2,ww1,wa1_k2)
       call plumat1_LO_2(ww4,ww3,wa2_k2)
       call stal(uupp2,wa1_k2,vv1,wym1_k2)
       call stal(uupp1,wa2_k2,vv2,wym2_k2)
       dd_k2(i1)=protF1_2*dd_k2(i1)-(wym1_k2+wym2_k2)*protF2_2/4.d0/mp
      enddo
      
c*************************************************************************
      
       endif
        
c
      call cplus(dd_k2,ddpl_k2)
      call cminus(dd_k2,ddmi_k2)

c e+ e- + k1
      call conmat(p2eck1,I,m7)
      call conmat(p1eck1,I,m8)

      call minmat(n1,m8,block13)
      call minmat(n2,m8,block14)
      call minmat(m7,n3,block15)
      call minmat(m7,n4,block16)
c
      call matr(block13,ddmi_k2,m1amp5)
      call matr(block14,ddpl_k2,m2amp5)
c
      call matr(ddmi_k2,block15,m1amp6)
      call matr(ddpl_k2,block16,m2amp6)
c
c     adding matrices of the status A and B
c
      call plumatLO(qqa2,m1amp5,m1amp6,mapi2)
      call plumatLO(qqa2,m2amp5,m2amp6,mbpi2)
c  FSR 2 muons 2 photons, adding new objects
c
      call matr(eck2mi,block1,newblock1)
      call matr(eck1mi,block9,newblock2)
      call matr(block11,eck1mi,newblock3)
      call matr(block3,eck2mi,newblock4)
      call matr(eck2pl,block2,newblock5) 
      call matr(eck1pl,block10,newblock6)
      call matr(block12,eck1pl,newblock7)
      call matr(block4,eck2pl,newblock8) 
c
      return
      end
c*****************************************************************************
c*****************************************************************************
c polarization vector definitions
c
      subroutine pol_vec(rk,epsk)
      implicit none
      real*8 rk(4),cth1,sth1,cphi1,sphi1
      complex*16 epsk(2,4)
      integer i
c
      cth1 = rk(4)/rk(1)
      sth1 = sqrt(1.d0-cth1**2)
c
      if(sth1.ne.0.d0)then
        cphi1 = rk(2)/sth1/rk(1)
        sphi1 = rk(3)/sth1/rk(1)
      else
        if(cth1.gt.0.d0)then
           cphi1 = 1.d0
           sphi1 = 0.d0
        else
           cphi1 = -1.d0
           sphi1 = 0.d0
        endif
      endif
c
c      do i=1,4
c        epsk(1,i)=rk(i)
c        epsk(2,i)=rk(i)
c      enddo
c
c helicity basis (the complex conjugated polarization vectors)
c
      epsk(1,1)=dcmplx(0.d0,0.d0)
      epsk(1,2)=dcmplx(cth1*cphi1,-sphi1)/dsqrt(2.D0)
      epsk(1,3)=dcmplx(cth1*sphi1,cphi1)/dsqrt(2.D0)
      epsk(1,4)=dcmplx(-sth1,0.d0)/dsqrt(2.D0)
      epsk(2,1)=dcmplx(0.d0,0.d0)
      epsk(2,2)=dcmplx(-cth1*cphi1,-sphi1)/dsqrt(2.D0)
      epsk(2,3)=dcmplx(-cth1*sphi1,cphi1)/dsqrt(2.D0)
      epsk(2,4)=dcmplx(sth1,0.d0)/dsqrt(2.D0)
c
c
c cartesian basis
c
c      epsk(1,1)=dcmplx(0.d0,0.d0)
c      epsk(1,2)=dcmplx(cth1*cphi1,0.d0)
c      epsk(1,3)=dcmplx(cth1*sphi1,0.d0)
c      epsk(1,4)=dcmplx(-sth1,0.d0)
c      epsk(2,1)=dcmplx(0.d0,0.d0)
c      epsk(2,2)=dcmplx(-sphi1,0.d0)
c      epsk(2,3)=dcmplx(cphi1,0.d0)
c      epsk(2,4)=dcmplx(0.d0,0.d0)
c
      return
      end
c**************************************************************************
c
      real*8 function amp(rk1,rk2,gam,q,qq,uupp1,uupp2,vv1,vv2)
      include 'phokhara_10.0.inc'     
      real*8 qq
      real*8 rk1,rk2,p1,p2,dme,el_m2,q,ebppb,vac_qq,vac_qqa1,vac_qqa2
      real*8 qqa1,qqa2,aa_phi,mm_ph,gg_ph
      integer i,j
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      complex*16 epsk1(2,4),epsk2(2,4),BW_om,KaonFormFactor
      complex*16 jp_resonance_s,jp_resonance
      complex*16 vacpol_and_nr
      complex*16 vac_qqpi,vac_qqa1pi,vac_qqa2pi
      complex*16 gam(4),eck1(4),eck2(4),dggvap
      complex*16 ma(2,2),mb(2,2),ma_ma(2,2),mb_ma(2,2)
      complex*16 mapi1(2,2),mbpi1(2,2),mapi2(2,2),mbpi2(2,2)
      complex*16 Jplus(1:2,1:2),Jmin(1:2,1:2)
      complex*16 Fplus(2,2),Fmin(2,2)
c
      dimension rk1(4),rk2(4),p1(4),p2(4),q(4)
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/matri1/ma,mb,ma_ma,mb_ma
      common/matri2/mapi1,mbpi1,mapi2,mbpi2
      common /cp1p2/p1,p2,dme,el_m2
      common/cqqa12/qqa1,qqa2
      common/param_PFF/aa_phi,mm_ph,gg_ph
      common/Jvectcom/Jplus,Jmin
      common/polvec_pi/eck1,eck2
c     
      amp = 0.d0
      call pol_vec(rk1,epsk1)
      call pol_vec(rk2,epsk2)
c
      call skalar1(rk1,rk2,gam,q)
      ebppb = p1(1)+p1(4)
c
c vacuum polarization
      vac_qq = cdabs(vacpol_and_nr(qq))**2
       if(pion.eq.1)then
c in picurr()
           vac_qqa1 = 1.d0
           vac_qqa2 = 1.d0
       elseif((pion.eq.0).or.(pion.eq.6).or.(pion.eq.7)
     4 .or.(pion.eq.4))then
      vac_qqa1 = cdabs(vacpol_and_nr(qqa1))**2
      vac_qqa2 = cdabs(vacpol_and_nr(qqa2))**2
       else
        continue
       endif

      if(pion.eq.1)then
         vac_qqpi=vacpol_and_nr(qq)
         vac_qqa1pi=vacpol_and_nr(qqa1)
         vac_qqa2pi=vacpol_and_nr(qqa2)
      endif
c
  
      do i=1,2
         do j=1,2
                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4)
c                  
                  eck2(1)=epsk2(j,1)
                  eck2(2)=epsk2(j,2)
                  eck2(3)=epsk2(j,3)
                  eck2(4)=epsk2(j,4)
c
                  call skalar2(rk1,rk2,gam,q,eck1,eck2,qq)
c pions : the two photons ISR and the two photons 1ISR and 1FSR
c muons : the two photons ISR
                  call blocks(rk1,rk2)
        if((pion.eq.1).and.(nlo2.eq.1))then
c         Jplus(1,1)=dcmplx(0.d0,0.d0)
c         Jplus(2,1)=dcmplx(0.d0,0.d0)
c         Jplus(1,2)=dcmplx(0.d0,0.d0)
c         Jplus(2,2)=dcmplx(0.d0,0.d0)
c         Jmin(1,1)=dcmplx(0.d0,0.d0)
c         Jmin(2,1)=dcmplx(0.d0,0.d0)
c         Jmin(1,2)=dcmplx(0.d0,0.d0)
c         Jmin(2,2)=dcmplx(0.d0,0.d0)
c         mapi1(1,1)=dcmplx(0.d0,0.d0)
c         mapi1(2,1)=dcmplx(0.d0,0.d0)
c         mapi1(1,2)=dcmplx(0.d0,0.d0)
c         mapi1(2,2)=dcmplx(0.d0,0.d0)
c         mbpi1(1,1)=dcmplx(0.d0,0.d0)
c         mbpi1(2,1)=dcmplx(0.d0,0.d0)
c         mbpi1(1,2)=dcmplx(0.d0,0.d0)
c         mbpi1(2,2)=dcmplx(0.d0,0.d0)
c         mapi2(1,1)=dcmplx(0.d0,0.d0)
c         mapi2(2,1)=dcmplx(0.d0,0.d0)
c         mapi2(1,2)=dcmplx(0.d0,0.d0)
c         mapi2(2,2)=dcmplx(0.d0,0.d0)
c         mbpi2(1,1)=dcmplx(0.d0,0.d0)
c         mbpi2(2,1)=dcmplx(0.d0,0.d0)
c         mbpi2(1,2)=dcmplx(0.d0,0.d0)
c         mbpi2(2,2)=dcmplx(0.d0,0.d0)
        
        Fplus(1,1)=Jplus(1,1)*cvac_s/Sp*qq+ma(1,1)*vac_qqpi+
     1 mapi1(1,1)*vac_qqa1pi*qq+mapi2(1,1)*vac_qqa2pi*qq
        Fplus(2,2)=Jplus(2,2)*cvac_s/Sp*qq+ma(2,2)*vac_qqpi+
     2 mapi1(2,2)*vac_qqa1pi*qq+mapi2(2,2)*vac_qqa2pi*qq
        Fplus(2,1)=Jplus(2,1)*cvac_s/Sp*qq+ma(2,1)*vac_qqpi+
     3 mapi1(2,1)*vac_qqa1pi*qq+mapi2(2,1)*vac_qqa2pi*qq
        Fplus(1,2)=Jplus(1,2)*cvac_s/Sp*qq+ma(1,2)*vac_qqpi+
     4 mapi1(1,2)*vac_qqa1pi*qq+mapi2(1,2)*vac_qqa2pi*qq

        Fmin(1,1)=Jmin(1,1)*cvac_s/Sp*qq+mb(1,1)*vac_qqpi+
     1 mbpi1(1,1)*vac_qqa1pi*qq+mbpi2(1,1)*vac_qqa2pi*qq
        Fmin(2,2)=Jmin(2,2)*cvac_s/Sp*qq+mb(2,2)*vac_qqpi+
     2 mbpi1(2,2)*vac_qqa1pi*qq+mbpi2(2,2)*vac_qqa2pi*qq
        Fmin(2,1)=Jmin(2,1)*cvac_s/Sp*qq+mb(2,1)*vac_qqpi+
     3 mbpi1(2,1)*vac_qqa1pi*qq+mbpi2(2,1)*vac_qqa2pi*qq
        Fmin(1,2)=Jmin(1,2)*cvac_s/Sp*qq+mb(1,2)*vac_qqpi+
     4 mbpi1(1,2)*vac_qqa1pi*qq+mbpi2(1,2)*vac_qqa2pi*qq

        amp = amp+
     1          ( (dme*cdabs(Fmin(1,1)-Fplus(1,1)))**2 
     1            +(dme*cdabs(Fmin(2,2)-Fplus(2,2)))**2
     2            +(cdabs(-ebppb*Fplus(2,1) + el_m2/ebppb*Fmin(2,1)))**2
     3            +(cdabs(ebppb*Fmin(1,2)-el_m2/ebppb*Fplus(1,2)))**2
     4          )
c     amp = amp+
c     1          ( (dme*cdabs(Jmin(1,1)-Jplus(1,1)))**2 
c     1            +(dme*cdabs(Jmin(2,2)-Jplus(2,2)))**2
c     2            +(cdabs(-ebppb*Jplus(2,1) + el_m2/ebppb*Jmin(2,1)))**2
c     3            +(cdabs(ebppb*Jmin(1,2)-el_m2/ebppb*Jplus(1,2)))**2
c     4          )*cdabs(cvac_s/Sp*qq)**2
c     5          -( (dme*cdabs(Jmin(1,1)-Jplus(1,1)))**2
c     1            +(dme*cdabs(Jmin(2,2)-Jplus(2,2)))**2
c     2           +(cdabs(-ebppb*Jplus(2,1) + el_m2/ebppb*Jmin(2,1)))**2
c     3            +(cdabs(ebppb*Jmin(1,2)-el_m2/ebppb*Jplus(1,2)))**2
c     4          )*cdabs(cvac_s/Sp*qq)**2
c        amp = amp+((dme*cdabs(mb(1,1)-ma(1,1)))**2
c     1         +(dme*cdabs(mb(2,2)-ma(2,2)))**2
c     2         +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
c     3         +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2)
c     4           * vac_qq
c      amp = amp+( ((dme*cdabs(mbpi1(1,1)-mapi1(1,1)))**2
c     1         +(dme*cdabs(mbpi1(2,2)-mapi1(2,2)))**2
c     2         +(cdabs(-ebppb*mapi1(2,1)+el_m2/ebppb*mbpi1(2,1)))**2
c     3         +(cdabs(ebppb*mbpi1(1,2)-el_m2/ebppb*mapi1(1,2)))**2)
c     4          *vac_qqa1
c     4         +((dme*cdabs(mbpi2(1,1)-mapi2(1,1)))**2
c     1         +(dme*cdabs(mbpi2(2,2)-mapi2(2,2)))**2
c     2         +(cdabs(-ebppb*mapi2(2,1)+el_m2/ebppb*mbpi2(2,1)))**2
c     3         +(cdabs(ebppb*mbpi2(1,2)-el_m2/ebppb*mapi2(1,2)))**2)
c     4          *vac_qqa2 )* qq**2
c        amp = amp+((dme*cdabs(mb(1,1)-ma(1,1)))**2
c     1         +(dme*cdabs(mb(2,2)-ma(2,2)))**2
c     2         +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
c     3         +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2)
c     4           * vac_qq
c        print*, 'FSR2=',amp,qq
c + interferences      
       else

      amp = amp+((dme*cdabs(mb(1,1)-ma(1,1)))**2
     1         +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2         +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3         +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2)
     4           * vac_qq


c
c muons : the two photons 1ISR and 1FSR
      if(((pion.eq.0).or.(pion.eq.4)).and.(fsrnlo.eq.1))then
        call blocks_mu(uupp1,uupp2,vv1,vv2) 
      endif
c the two photons 1ISR and 1FSR
      if(fsrnlo.eq.1) then
      amp = amp+( ((dme*cdabs(mbpi1(1,1)-mapi1(1,1)))**2
     1         +(dme*cdabs(mbpi1(2,2)-mapi1(2,2)))**2
     2         +(cdabs(-ebppb*mapi1(2,1)+el_m2/ebppb*mbpi1(2,1)))**2
     3         +(cdabs(ebppb*mbpi1(1,2)-el_m2/ebppb*mapi1(1,2)))**2)
     4          *vac_qqa1
     4         +((dme*cdabs(mbpi2(1,1)-mapi2(1,1)))**2
     1         +(dme*cdabs(mbpi2(2,2)-mapi2(2,2)))**2
     2         +(cdabs(-ebppb*mapi2(2,1)+el_m2/ebppb*mbpi2(2,1)))**2
     3         +(cdabs(ebppb*mbpi2(1,2)-el_m2/ebppb*mapi2(1,2)))**2)
     4          *vac_qqa2 )* qq**2
      
      endif
      endif



         enddo
      enddo
c
      return
      end
c*************************************************************************
c*************************************************************************
c the two photons 2FSR
c
      subroutine Jvector(Jvect,rk1,rk2)
      include 'phokhara_10.0.inc'
      real*8 Dk1k2q1,Dk1k2q2,rk1(1:4),rk2(1:4)
      complex*16 eck1(1:4),eck2(1:4),rk1eck2,rk2eck1,eck1eck2,
     & ff01,ff02,ff03,ff04,
     & Afactor,Bfactor,Cfactor,Efactor,Ffactor,
     & Jvect(1:4)
      complex*16 pi1eck1,pi2eck1,pi1eck2,pi2eck2,PionFormFactor
      real*8 rk1pi1,rk1pi2,rk2pi1,rk2pi2
      integer i1
      common/newpions/rk1eck2,rk2eck1,eck1eck2
      common/newdenpions/Dk1k2q1,Dk1k2q2
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
      common/iloczs4/pi1eck2,pi2eck2,rk2pi1,rk2pi2
      common/polvec_pi/eck1,eck2
c
c******** k1.q2=rk1pi2,..., e1.q2=pi1eck1,..., e1.e2=eck1eck2
c
        ff01= (2.d0*eck1eck2) / Dk1k2q2 
     1       - (2.d0*pi2eck1*(rk1eck2 + pi2eck2)) / (rk1pi2*Dk1k2q2) 
     2       - (2.d0*pi2eck2*(rk2eck1 + pi2eck1)) / (rk2pi2*Dk1k2q2)
c
        ff02= (pi1eck1*pi2eck2) / (rk1pi1*rk2pi2)
c
        ff03= (2.d0*eck1eck2) / Dk1k2q1  
     1       - (2.d0*pi1eck1*(rk1eck2 + pi1eck2))/(rk1pi1*Dk1k2q1) 
     2       - (2.d0*pi1eck2*(rk2eck1 + pi1eck1)) / (rk2pi1*Dk1k2q1)
c
        ff04= (pi2eck1*pi1eck2) / (rk1pi2*rk2pi1)

   
c
c********
c
        Afactor =  2.d0*(ff02 + ff03)
c
        Bfactor =  2.d0*(ff03 + ff04)
c
        Cfactor =  2.d0*(ff01 + ff02 + ff03 + ff04)
c
        Efactor = (2.d0*pi1eck2)/(rk2pi1) - (2.d0*pi2eck2)/(rk2pi2)
c
        Ffactor = (2.d0*pi1eck1)/(rk1pi1) - (2.d0*pi2eck1)/(rk1pi2)

        
c
c********* J^μ(i,j) / e^3 / i
c
      do i1 =1,4
      Jvect(i1) = (Afactor*rk1(i1) + Bfactor*rk2(i1) + Cfactor
     1    *momenta(6,i1-1) + Efactor*eck1(i1) + Ffactor*eck2(i1))
     2 *PionFormFactor(Sp)
      enddo
      
      return
      end
c***********************************************************************
      subroutine plus(vect,mat)
      implicit none
      real*8 vect(4)
      complex*16 mat(2,2)
         mat(1,1) = vect(1)-vect(4)
         mat(1,2) = dcmplx(-vect(2),vect(3))
         mat(2,1) = dcmplx(-vect(2),-vect(3))
         mat(2,2) = vect(1)+vect(4)
      return
      end
c**************************************************************************
      subroutine minus(vect,mat)
      implicit none
      real*8 vect(4)
      complex*16 mat(2,2)
         mat(1,1) = vect(1)+vect(4)
         mat(1,2) = dcmplx(vect(2),-vect(3))
         mat(2,1) = dcmplx(vect(2),vect(3))
         mat(2,2) = vect(1)-vect(4)
      return
      end   
c**************************************************************************
      subroutine cplus(vect,mat)
      implicit none
      complex*16 vect(4),mat(2,2)
         mat(1,1) = vect(1)-vect(4)
         mat(1,2) = -vect(2)+dcmplx(0.d0,1.d0)*vect(3)
         mat(2,1) = -vect(2)-dcmplx(0.d0,1.d0)*vect(3)
         mat(2,2) = vect(1)+vect(4)
      return
      end
c**************************************************************************
      subroutine cminus(vect,mat)
      implicit none
      complex*16 vect(4),mat(2,2)
         mat(1,1) = vect(1)+vect(4)
         mat(1,2) = vect(2)-dcmplx(0.d0,1.d0)*vect(3)
         mat(2,1) = vect(2)+dcmplx(0.d0,1.d0)*vect(3)
         mat(2,2) = vect(1)-vect(4)
      return
      end   
c**************************************************************************
      subroutine plumat1(mat1,mat2,mat3)
      implicit none
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2)
      complex*16 gam1pimi(2,2),gam1pipl(2,2),gam2pimi(2,2)
     1 ,gam2pipl(2,2)
      real*8 qqa1,qqa2,rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      integer i,j
c
      common/matri3/gam1pimi,gam1pipl,gam2pimi,gam2pipl
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common/cqqa12/qqa1,qqa2

      do i=1,2
         do j=1,2
            mat3(i,j)=  0.5d0*mat1(i,j)/rk2p1/qqa1
     1                 +0.5d0*mat2(i,j)/rk2p2/qqa1
         enddo
      enddo
c
      end
c**************************************************************************
      subroutine plumat2(mat1,mat2,mat3)
      implicit none
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2)
      complex*16 gam1pimi(2,2),gam1pipl(2,2),gam2pimi(2,2)
     1           ,gam2pipl(2,2)
      real*8 qqa1,qqa2,rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      integer i,j
c
      common/matri3/gam1pimi,gam1pipl,gam2pimi,gam2pipl
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common/cqqa12/qqa1,qqa2
 
      do i=1,2
         do j=1,2
            mat3(i,j)=  0.5d0*mat1(i,j)/rk1p1/qqa2
     1                 +0.5d0*mat2(i,j)/rk1p2/qqa2
         enddo
      enddo
c
      end      
c**************************************************************************
c     addind 2 matrices: for k2 initial state emission
c
      subroutine plumatLO_2(mat1,mat2,mat3)      
      implicit none
      real*8 rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      integer i,j
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2)
c
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
c
      do i=1,2
         do j=1,2
            mat3(i,j)=  0.5d0*mat1(i,j)/rk2p1
     1                 +0.5d0*mat2(i,j)/rk2p2
         enddo
      enddo
c
      end
c**************************************************************************
c     addind 2 matrices: with proper denominators
c     for k2 photon final state emission (muons)
c
      subroutine plumat1_LO_2(mat1,mat2,mat3)
      implicit none
      complex*16 mat1(2,2),mat2(2,2),mat3(2,2),pi1eck2,pi2eck2  
      real*8 rk2pi1,rk2pi2
      integer i,j
c
      common/iloczs4/pi1eck2,pi2eck2,rk2pi1,rk2pi2
c
      do i=1,2
         do j=1,2
            mat3(i,j)=  0.5d0*mat1(i,j)/rk2pi2
     1                 -0.5d0*mat2(i,j)/rk2pi1
         enddo
      enddo
c
      end


c*********************************************************************************
c Columb Factor
      real*8 function Columb_F(qq)
      include 'phokhara_10.0.inc'
      real*8 bb,ff,qq,f0,b0
      integer nn,cf
      if(ph0.eq.0)then
       if(nlo.eq.0)then
      Columb_F=1.d0
       elseif(nlo.eq.1)then
      bb=pi*alpha/sqrt(abs(1.d0-4.d0*mp**2/qq))
      b0=pi*alpha
      ff=bb/(1.d0-exp(-bb))
      f0=b0/(1.d0-exp(-b0))
      Columb_F=ff-f0+1.d0
       endif
       elseif(ph0.eq.1)then
         if((nlo.eq.0).or.(nlo.eq.1))then
        bb=pi*alpha/sqrt(abs(1.d0-4.d0*mp**2/qq))
       Columb_F=bb/(1.d0-exp(-bb))
         endif
        elseif(ph0.eq.-1)then
         Columb_F=1.d0
       endif

      return
      end
c***********************************************************************
c***********************************************************************
c **********************************************************************
      complex*16 function isr_mu_amp
     &  (rk1,rk2,polepl,polemin,eck1,eck2,gam,q,qq,uupp1,uupp2,vv1,vv2)
      include 'phokhara_10.0.inc'
c
      real*8 rk1,rk2,p1,p2,q,ebppb,dme,el_m2,qq
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 eck1(4),eck2(4)
      complex*16 ma(2,2),mb(2,2),ma_ma(2,2),mb_ma(2,2),gam(4)
      complex*16  cvac_qq,vacpol_and_nr
      integer polepl,polemin
c
      dimension rk1(4),rk2(4),p1(4),p2(4),q(4)
c
      common/matri1/ma,mb,ma_ma,mb_ma
      common /cp1p2/p1,p2,dme,el_m2
c
      call skalar1(rk1,rk2,gam,q)
      cvac_qq = vacpol_and_nr(qq)
      ebppb = p1(1)+p1(4)
      call skalar2(rk1,rk2,gam,q,eck1,eck2,qq)
c muons : the two photons ISR
      call blocks(rk1,rk2)

      if((polepl.eq.1).and.(polemin.eq.1))then
      isr_mu_amp=dme*(ma(2,2)-mb(2,2))*cvac_qq
c 
      elseif((polepl.eq.1).and.(polemin.eq.2))then
      isr_mu_amp=(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))*cvac_qq
c
      elseif((polepl.eq.2).and.(polemin.eq.1))then
      isr_mu_amp= (ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))*cvac_qq
c
      elseif((polepl.eq.2).and.(polemin.eq.2))then
      isr_mu_amp=dme*(ma(1,1)-mb(1,1))*cvac_qq
c
      endif
c
      return
      end
c-----------------------------------------------------------------------
c **********************************************************************
      complex*16 function isr_fsr_mu_amp
     &  (rk1,rk2,polepl,polemin,eck1,eck2,gam,q,qq,uupp1,uupp2,vv1,vv2)
      include 'phokhara_10.0.inc'
c
      real*8 rk1,rk2,p1,p2,q,ebppb,dme,el_m2,qq,qqa1,qqa2
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 eck1(4),eck2(4)
      complex*16 mapi1(2,2),mbpi1(2,2),mapi2(2,2),mbpi2(2,2),gam(4)
      complex*16 cvac_qqa1,cvac_qqa2,vacpol_and_nr
      integer polepl,polemin
c
      dimension rk1(4),rk2(4),p1(4),p2(4),q(4)
c
      common/matri2/mapi1,mbpi1,mapi2,mbpi2
      common /cp1p2/p1,p2,dme,el_m2
      common/cqqa12/qqa1,qqa2
c
      call skalar1(rk1,rk2,gam,q)
c
      ebppb = p1(1)+p1(4)
c
      call skalar2(rk1,rk2,gam,q,eck1,eck2,qq)
c
      call blocks_mu(uupp1,uupp2,vv1,vv2)
      cvac_qqa1 = vacpol_and_nr(qqa1)
      cvac_qqa2 = vacpol_and_nr(qqa2)

c
      if((polepl.eq.1).and.(polemin.eq.1))then
c
      isr_fsr_mu_amp=dme*(
     &                    (mapi1(2,2)-mbpi1(2,2))*cvac_qqa1
     &                   +
     &                    (mapi2(2,2)-mbpi2(2,2))*cvac_qqa2
     &                                            )
      elseif((polepl.eq.1).and.(polemin.eq.2))then
   
      isr_fsr_mu_amp=
     &              (-ebppb*mapi1(2,1)+el_m2/ebppb*mbpi1(2,1))*cvac_qqa1
     &             + 
     &              (-ebppb*mapi2(2,1)+el_m2/ebppb*mbpi2(2,1))*cvac_qqa2
c
      elseif((polepl.eq.2).and.(polemin.eq.1))then
c
      isr_fsr_mu_amp=
     &             (ebppb*mbpi1(1,2)-el_m2/ebppb*mapi1(1,2))*cvac_qqa1
     &            +
     &             (ebppb*mbpi2(1,2)-el_m2/ebppb*mapi2(1,2))*cvac_qqa2
c
      elseif((polepl.eq.2).and.(polemin.eq.2))then
c
      isr_fsr_mu_amp=dme*(
     &                    (mapi1(1,1)-mbpi1(1,1))*cvac_qqa1
     &                   +
     &                    (mapi2(1,1)-mbpi2(1,1))*cvac_qqa2
     &                                            )
c
      endif
c
      return
      end

c **********************************************************************
c-----------------------------------------------------------------------
      complex*16 function fsr_mu_amp
     &  (q,qq,gam,rk1,rk2,polepl,polemin,eck1,eck2,uupp1,uupp2,vv1,vv2)
      include 'phokhara_10.0.inc' 
      complex*16 newblock1(2,2),newblock2(2,2),newblock3(2,2),
     1           newblock4(2,2),newblock5(2,2),newblock6(2,2),
     2           newblock7(2,2),newblock8(2,2),block1(2,2),block2(2,2),
     3           block3(2,2),block4(2,2),block9(2,2),block10(2,2),
     4           block11(2,2),block12(2,2)
      complex*16 diss1(2,2),diss2(2,2)
c
      complex*16 Jee11(4),Jee12(4),Jee21(4),Jee22(4),
     1           Jeemin(2,2,2,2),cpp(2,2,2,2)
c
      integer polepl,polemin
c
      complex*16 M11(2,2),M12(2,2),M13(2,2),M14(2,2),M15(2,2),M16(2,2),
     1           M21(2,2),M22(2,2),M23(2,2),M24(2,2),M25(2,2),M26(2,2),
     2           aa1(2,2),aa2(2,2),aa3(2,2),aa4(2,2),aa3a(2,2),aa4a(2,2)
      complex*16 wkl11,wkl12,wkl13,wkl14,wkl15,wkl16,wkl21,wkl22,wkl23,
     1           wkl24,wkl25,wkl26
c
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
c
      complex*16 eck1(4),eck2(4),vv(2,2)
c
      real*8 dme,el_m2,p1(4),p2(4),rk1(4),rk2(4),qq,q(4)
c
      complex*16 p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      real*8 rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      complex*16 k1pl(2,2),k1mi(2,2),k2pl(2,2),k2mi(2,2)
     1       ,eck1pl(2,2),eck1mi(2,2),
     2        eck2pl(2,2),eck2mi(2,2),I(2,2),gam(4)
c    
      complex*16 pi1eck2,pi2eck2,pi1eck1,pi2eck1
      complex*16 qpl(2,2),qmi(2,2),gampl(2,2),gammi(2,2)
      complex*16 sigpl(4,2,2),sigmi(4,2,2), 
     3        gampl_ma(2,2),gammi_ma(2,2)
      real*8 rk2pi1,rk2pi2,rk1pi1,rk1pi2
c
      real*8 den1,den2,den3,den4,den5,den6
c
      real*8 g1(4),g2(4)
c
      common/newbloc_mu/newblock1,newblock2,newblock3,newblock4,
     1                  newblock5,newblock6,newblock7,newblock8,
     2                  block1,block2,block3,block4,block9,
     3                  block10,block11,block12
c
      common/lepcur/Jee11,Jee12,Jee21,Jee22,Jeemin,cpp
c
      common/iloczs1/p1eck1,p1eck2,p2eck1,p2eck2,p1gam,p2gam
      common/iloczs2/rk1p1,rk1p2,rk2p1,rk2p2,rk1rk2,anaw1,anaw2
      common/iloczs3/pi1eck1,pi2eck1,rk1pi1,rk1pi2
      common/iloczs4/pi1eck2,pi2eck2,rk2pi1,rk2pi2
      common/matri/qpl,qmi,gampl,gammi,k1pl,k1mi,k2pl,k2mi,eck1pl,
     1            eck1mi,eck2pl,eck2mi,I,sigpl,sigmi,gampl_ma,gammi_ma
      common /cp1p2/p1,p2,dme,el_m2

 
      
c
      call skalar1(rk1,rk2,gam,q)
      call skalar2(rk1,rk2,gam,q,eck1,eck2,qq)
      call skalar1LO(rk1)
      call skalar2LO(rk1,eck1)

c
      call blocks_mu(uupp1,uupp2,vv1,vv2)
c
      g1(1)=momenta(6,0)
      g1(2)=momenta(6,1)
      g1(3)=momenta(6,2)
      g1(4)=momenta(6,3)
c
      g2(1)= momenta(7,0)
      g2(2)= momenta(7,1)
      g2(3)= momenta(7,2)
      g2(4)= momenta(7,3)
c
      call currprod(polepl,polemin,g2,diss1,diss2) !q2
      call matr(diss1,newblock1,M11)
      call matr(diss1,newblock2,M12)
      call matr(diss2,newblock5,M21)
      call matr(diss2,newblock6,M22)
c
c
      if((polepl.eq.1).and.(polemin.eq.1))then
      call matr3(1,1,Jeemin,block1,aa1)
      call matr(block11,aa1,M13)
      call matr3(1,1,Jeemin,block9,aa2)
      call matr(block3,aa2,M14)
      call matr3(1,1,Jeemin,block2,aa3)
      call conmat_re(-1.d0,aa3,aa3a)
      call matr(block12,aa3a,M23)
      call matr3(1,1,Jeemin,block10,aa4)
      call conmat_re(-1.d0,aa4,aa4a)
      call matr(block4,aa4a,M24)
      endif
c
      if((polepl.eq.1).and.(polemin.eq.2))then
      call matr3(1,2,Jeemin,block1,aa1)
      call matr(block11,aa1,M13)
      call matr3(1,2,Jeemin,block9,aa2)
      call matr(block3,aa2,M14)
      call matr3(1,2,Jeemin,block2,aa3)
      call conmat_re(-1.d0,aa3,aa3a)
      call matr(block12,aa3a,M23)
      call matr3(1,2,Jeemin,block10,aa4)
      call conmat_re(-1.d0,aa4,aa4a)
      call matr(block4,aa4a,M24)
      endif
c
      if((polepl.eq.2).and.(polemin.eq.1))then
      call matr3(2,1,Jeemin,block1,aa1)
      call matr(block11,aa1,M13)
      call matr3(2,1,Jeemin,block9,aa2)
      call matr(block3,aa2,M14)
      call matr3(2,1,Jeemin,block2,aa3)
      call conmat_re(-1.d0,aa3,aa3a)
      call matr(block12,aa3a,M23)
      call matr3(2,1,Jeemin,block10,aa4)
      call conmat_re(-1.d0,aa4,aa4a)
      call matr(block4,aa4a,M24)
      endif
c
      if((polepl.eq.2).and.(polemin.eq.2))then
      call matr3(2,2,Jeemin,block1,aa1)
      call matr(block11,aa1,M13)
      call matr3(2,2,Jeemin,block9,aa2)
      call matr(block3,aa2,M14)
      call matr3(2,2,Jeemin,block2,aa3)
      call conmat_re(-1.d0,aa3,aa3a)
      call matr(block12,aa3a,M23)
      call matr3(2,2,Jeemin,block10,aa4)
      call conmat_re(-1.d0,aa4,aa4a)
      call matr(block4,aa4a,M24)
      endif 
c
      call currprod(polepl,polemin,g1,diss1,diss2) !q1
      call matr(newblock3,diss1,M15)
      call matr(newblock4,diss1,M16)
      call matr(newblock7,diss2,M25)
      call matr(newblock8,diss2,M26)
c
c
      call stal(uupp1,M11,vv1,wkl11)
      call stal(uupp1,M12,vv1,wkl12)
      call stal(uupp1,M13,vv1,wkl13)
      call stal(uupp1,M14,vv1,wkl14)
      call stal(uupp1,M15,vv1,wkl15)
      call stal(uupp1,M16,vv1,wkl16)
c
      call stal(uupp2,M21,vv2,wkl21)
      call stal(uupp2,M22,vv2,wkl22)
      call stal(uupp2,M23,vv2,wkl23)
      call stal(uupp2,M24,vv2,wkl24)
      call stal(uupp2,M25,vv2,wkl25)
      call stal(uupp2,M26,vv2,wkl26)    
c denominators
      den1=rk1pi1
      den2=rk1rk2 +rk1pi1 + rk2pi1
      den3=rk2pi1
      den4=rk2pi2
      den5=rk1pi2
      den6=rk1rk2 +rk1pi2 + rk2pi2
c
      wkl11=wkl11/den1/den2/4.d0
      wkl12=wkl12/den2/den3/4.d0
      wkl13=-wkl13/den1/den4/4.d0
      wkl14=-wkl14/den3/den5/4.d0
      wkl15=wkl15/den4/den6/4.d0
      wkl16=wkl16/den5/den6/4.d0

      wkl21=wkl21/den1/den2/4.d0
      wkl22=wkl22/den2/den3/4.d0
      wkl23=-wkl23/den1/den4/4.d0
      wkl24=-wkl24/den3/den5/4.d0
      wkl25=wkl25/den4/den6/4.d0
      wkl26=wkl26/den5/den6/4.d0
      
      fsr_mu_amp=(wkl11+wkl12+wkl13+wkl14+wkl15+wkl16
     1            +wkl21+wkl22+wkl23+wkl24+wkl25+wkl26)*cvac_s

      return
      end
c***********************************************************************
c-----------------------------------------------------------------------
c muon mode, hard part with 2 real hard photon
c sum over all polarisations for e+,e-,mu+,mu-,gamma,gamma 
c-----------------------------------------------------------------------
      real*8 function ampsqr_mu(q,qq,gammu,rk1,rk2)
      include 'phokhara_10.0.inc'
      real*8 rk1(4),rk2(4),ampsgr,qq,q(4),con4,dps,B6,B5,error,B5ISR
      complex*16 fsr_mu_amp,isr_mu_amp,isr_fsr_mu_amp,isr_fsr_mu_amp2
      complex*16 eck1(4),eck2(4)
      complex*16 uupp1(1,2),uupp2(1,2),vv1(2,1),vv2(2,1)
      complex*16 epsk1(2,4),epsk2(2,4)
      complex*16 v1(2,2),v2(2,2),up1(2,2),up2(2,2),gammu(4,2,2),gam(4)
      integer ic1,ic2,i,j,ii1,ii2,i1,switch
      complex*16  vacpol_and_nr,vac_qq
c
      common/schan/con4
c
      switch=0
      dps = dSqrt(1.d0-4.d0*mmu*mmu/qq)/(32.d0*pi*pi)
c
      call pol_vec(rk1,epsk1)
      call pol_vec(rk2,epsk2)
      call spinors(mmu,up1,up2,v1,v2)
c 
      ampsgr=0.d0
c
                                                                                                                                                                                        
     
c
ccccccccccccccccccccccccccc
c gauge test for hard part
c      do i=1,4
c        epsk2(1,i)=rk2(i)
c        epsk2(2,i)=rk2(i)
c        epsk1(1,i)=rk1(i)
c        epsk1(2,i)=rk1(i)
c      enddo
ccccccccccccccccccccccccccc
c
      do ic1=1,2
      do ic2=1,2
c
        uupp2(1,1) = up2(1,ic1)
        uupp2(1,2) = up2(2,ic1)
        uupp1(1,1) = up1(1,ic1)
        uupp1(1,2) = up1(2,ic1)
c    
        vv1(1,1) = v1(1,ic2)                 
        vv1(2,1) = v1(2,ic2)                 
        vv2(1,1) = v2(1,ic2)                 
        vv2(2,1) = v2(2,ic2)
c
c now gam is the muon current (no emission)
        do i1=1,4
         gam(i1)=gammu(i1,ic1,ic2)
        enddo
c
      do i=1,2
      do j=1,2
c
        eck1(1)=epsk1(i,1)
        eck1(2)=epsk1(i,2)
        eck1(3)=epsk1(i,3)
        eck1(4)=epsk1(i,4)
c
        eck2(1)=epsk2(j,1)
        eck2(2)=epsk2(j,2)
        eck2(3)=epsk2(j,3)
        eck2(4)=epsk2(j,4)
c
      do ii1=1,2
      do ii2=1,2
c
      if(fsrnlo.eq.0)then
c
      if(fsr.eq.0)then
      ampsgr=ampsgr  
     & + ((4.q0*pi*alpha)**4/qq**2)*cdabs(isr_mu_amp
     &  (rk1,rk2,ii1,ii2,eck1,eck2,
     &   gam,q,qq,uupp1,uupp2,vv1,vv2))**2/4.d0
      else
      write(*,*)'Wrong switch'
      write(*,*)'Combinations for mu+mu- NLO  mode:'
      write(*,*)'nlo = 1 then'
      write(*,*)'fsr = 0, ifsnlo = 0'
      write(*,*)'fsr = 2, ifsnlo = 1'
      stop
      endif
c
      elseif(fsrnlo.eq.1)then
c
      if(fsr.eq.0)then
c      write(*,*)'Wrong switch'
c      write(*,*)'Combinations for mu+mu- NLO  mode:'
c      write(*,*)'nlo = 1 then'
c      write(*,*)'fsr = 0, ifsnlo = 0'
c      write(*,*)'fsr = 2, ifsnlo = 1'
c      stop
      elseif(fsr.eq.1)then
c      write(*,*)'Wrong switch'
c      write(*,*)'Combinations for mu+mu- NLO  mode:'
c      write(*,*)'nlo = 1 then'
c      write(*,*)'fsr = 0, ifsnlo = 0'
c      write(*,*)'fsr = 2, ifsnlo = 1'
c      stop

      elseif(fsr.eq.2)then
      ampsgr=ampsgr
     & + (4.q0*pi*alpha)**4/4.d0*cdabs(isr_mu_amp
     &  (rk1,rk2,ii1,ii2,eck1,eck2,
     &   gam,q,qq,uupp1,uupp2,vv1,vv2)/qq
     & + isr_fsr_mu_amp
     &  (rk1,rk2,ii1,ii2,eck1,eck2,
     &   gam,q,qq,uupp1,uupp2,vv1,vv2)
     & - fsr_mu_amp(q,qq,gam,rk1,rk2,ii1,ii2,eck1,eck2
     &    ,uupp1,uupp2,vv1,vv2)/con4)**2
      endif
      endif
      enddo
      enddo   
c
      enddo
      enddo
c  
      enddo
      enddo
      ampsqr_mu=ampsgr*dps
c      write(*,*)'2ph',ampsgr
c      stop
      return
      end
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c **********************************************************************
      subroutine currprod(polepl,polemin,xx,diss1,diss2)
      implicit none
      complex*16 diss1(2,2),diss2(2,2)
      complex*16 Jee11(4),Jee12(4),Jee21(4),Jee22(4),
     1           Jeemin(2,2,2,2),cpp(2,2,2,2)
      real*8 xx(4)
      integer polepl,polemin
c
      common/lepcur/Jee11,Jee12,Jee21,Jee22,Jeemin,cpp
      
c
c Jee-(p1+ + p2+) - 2* xJee
c -Jee-(p1+ + p2+) - 2* xJee = Jee+(p1- + p2-) - 2* xJee
      if((polepl.eq.1).and.(polemin.eq.1))then
      diss1(1,1)=cpp(1,1,1,1)-2.d0*xx(4)*Jee11(4)
      diss1(1,2)=(0.d0,0.d0)
      diss1(2,1)=(0.d0,0.d0)
      diss1(2,2)=cpp(1,1,2,2)-2.d0*xx(4)*Jee11(4)
c
      diss2(1,1)=-cpp(1,1,1,1)-2.d0*xx(4)*Jee11(4)
      diss2(1,2)=(0.d0,0.d0)
      diss2(2,1)=(0.d0,0.d0)
      diss2(2,2)=-cpp(1,1,2,2)-2.d0*xx(4)*Jee11(4)
c
      elseif((polepl.eq.1).and.(polemin.eq.2))then
      diss1(1,1)=-2.d0*(xx(2)*Jee12(2)+xx(3)*Jee12(3))
      diss1(1,2)=cpp(1,2,1,2)
      diss1(2,1)=(0.d0,0.d0)
      diss1(2,2)=-2.d0*(xx(2)*Jee12(2)+xx(3)*Jee12(3))
c
      diss2(1,1)=-2.d0*(xx(2)*Jee12(2)+xx(3)*Jee12(3))
      diss2(1,2)=-cpp(1,2,1,2)
      diss2(2,1)=(0.d0,0.d0)
      diss2(2,2)=-2.d0*(xx(2)*Jee12(2)+xx(3)*Jee12(3))
c
      elseif((polepl.eq.2).and.(polemin.eq.1))then
      diss1(1,1)=-2.d0*(xx(2)*Jee21(2)+xx(3)*Jee21(3))
      diss1(1,2)=(0.d0,0.d0)
      diss1(2,1)=cpp(2,1,2,1)
      diss1(2,2)=-2.d0*(xx(2)*Jee21(2)+xx(3)*Jee21(3))
c
      diss2(1,1)=-2.d0*(xx(2)*Jee21(2)+xx(3)*Jee21(3))
      diss2(1,2)=(0.d0,0.d0)
      diss2(2,1)=-cpp(2,1,2,1)
      diss2(2,2)=-2.d0*(xx(2)*Jee21(2)+xx(3)*Jee21(3))
c
      elseif((polepl.eq.2).and.(polemin.eq.2))then
      diss1(1,1)=cpp(2,2,1,1)-2.d0*xx(4)*Jee22(4)
      diss1(1,2)=(0.d0,0.d0)
      diss1(2,1)=(0.d0,0.d0)
      diss1(2,2)=cpp(2,2,2,2)-2.d0*xx(4)*Jee22(4)
c
      diss2(1,1)=-cpp(2,2,1,1)-2.d0*xx(4)*Jee22(4)
      diss2(1,2)=(0.d0,0.d0)
      diss2(2,1)=(0.d0,0.d0)
      diss2(2,2)=-cpp(2,2,2,2)-2.d0*xx(4)*Jee22(4)
c
      endif
c
      return
      end
c****************************************************************************
c     multiplication of the 2x2x2x2 and 2x2 matrices with the (2,2)index fixed
c     mu1,mu2 - are this index; the result is 2x2 matrix
c
      subroutine matr3(mu1,mu2,mat1,mat2,mat3)
      implicit none
      complex*16  mat1(2,2,2,2),mat2(2,2),mat3(2,2)
      integer i,j,k,mu1,mu2
c
      do i=1,2
         do j=1,2
            mat3(i,j)=(0.d0,0.d0)
         enddo
      enddo
c   
      do i=1,2
         do j=1,2
            do k=1,2
               mat3(i,j)=mat3(i,j)+mat1(mu1,mu2,i,k)*mat2(k,j)
            enddo
         enddo
      enddo
c
      end
c****************************************************************************
c****************************************************************************
c     multiplication of a 2x2 matrix by a constant
c
      subroutine conmat_re(alfa,mat,amat)
      implicit none
      integer i,j
      real*8 alfa
      complex*16 mat(2,2),amat(2,2)
c
      do i=1,2
         do j=1,2
            amat(i,j)=alfa*mat(i,j)
         enddo
      enddo
c
      end
c***********************************************************************

cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                         SOFT PART
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c***********************************************************************
c soft part for 1 ISR and 1 FSR photon - one hard,one soft
c*********************************************************************** 
      real*8 function Int_f(delta_2)
      include 'phokhara_10.0.inc'
      real*8 delta_2
      real*8 t_a,t_b,t_3pl,t_4min
      real*8 p1(4),p2(4),q1(4),q2(4)
      real*8 Int1,Int2,Int3,Int4,integr_I2
      real*8 int_end1,int_end2,int_end3,int_end4,integr_Int3
      real*8 const2,coeff_a
c
c_______________________________________________________________________
c  p1 - e+
      p1(1)=momenta(1,0)
      p1(2)=momenta(1,1)
      p1(3)=momenta(1,2)
      p1(4)=momenta(1,3)
c
c  p2 - e-
      p2(1)=momenta(2,0)
      p2(2)=momenta(2,1)
      p2(3)=momenta(2,2)
      p2(4)=momenta(2,3)
c q1 - mu+
      q1(1)=momenta(6,0)
      q1(2)=momenta(6,1)
      q1(3)=momenta(6,2)
      q1(4)=momenta(6,3)
c
c  q2 - mu-
      q2(1)=momenta(7,0)
      q2(2)=momenta(7,1)
      q2(3)=momenta(7,2)
      q2(4)=momenta(7,3)
c_______________________________________________________________________

      call element_t(p1,q2,t_3pl,t_4min)
      call limits_t(p1,q2,t_a,t_b)
      int_end1=integr_Int3(p1,q2,t_3pl,t_4min,t_a,t_b)

      call element_t(p1,q1,t_3pl,t_4min)
      call limits_t(p1,q1,t_a,t_b)
      int_end2=integr_Int3(p1,q1,t_3pl,t_4min,t_a,t_b)

      call element_t(p2,q2,t_3pl,t_4min)
      call limits_t(p2,q2,t_a,t_b)
      int_end3=integr_Int3(p2,q2,t_3pl,t_4min,t_a,t_b)

      call element_t(p2,q1,t_3pl,t_4min)
      call limits_t(p2,q1,t_a,t_b)
      int_end4=integr_Int3(p2,q1,t_3pl,t_4min,t_a,t_b)

      
      Int1=integr_I2(p1,q2,delta_2) + int_end1
      Int2=integr_I2(p1,q1,delta_2) + int_end2
      Int3=integr_I2(p2,q2,delta_2) + int_end3
      Int4=integr_I2(p2,q1,delta_2) + int_end4

c
      Int_f=-alpha/4.d0/pi/pi*(Int1-Int2-Int3+Int4)


c
      return
      end
c***********************************************************************
c***********************************************************************
c soft part for 2 FSR photon - one hard,one soft
c***********************************************************************

      real*8 function Int_f2(delta_2,qq)
      include 'phokhara_10.0.inc' 
      real*8 delta_2,qq
      real*8 q1(4),q2(4),p1(4),p2(4)
      real*8 integr_I2_FSR2,int_end
      real*8 t_3pl,t_4min,t_a,t_b
      real*8 integr_Int12,integr_Int3
c________________________________________________________________________
c q1 - mu+
      q1(1)=momenta(6,0)
      q1(2)=momenta(6,1)
      q1(3)=momenta(6,2)
      q1(4)=momenta(6,3)
c
c  q2 - mu-
      q2(1)=momenta(7,0)
      q2(2)=momenta(7,1)
      q2(3)=momenta(7,2)
      q2(4)=momenta(7,3)
c_______________________________________________________________________
c
      call element_t(q1,q2,t_3pl,t_4min)
      call limits_t(q1,q2,t_a,t_b)
c
      int_end=integr_Int12(q1)+integr_Int12(q2)
     &  -2.d0*integr_Int3(q1,q2,t_3pl,t_4min,t_a,t_b)
c
      Int_f2=-alpha/4.d0/pi/pi*(integr_I2_FSR2(q1,q2,delta_2,qq)
     &                                                 +int_end)
c
      return
      end
c***********************************************************************
c***********************************************************************
c soft part for 2 ISR photons - one hard,one soft
c*********************************************************************** 
      real*8 function Int_f1(delta_2)
      include 'phokhara_10.0.inc' 
      real*8 delta_2
      real*8 p1(4),p2(4),q1(4),q2(4)
      real*8 integr_I2_ISR2,int_end
      real*8 integr_Int12,integr_Int4
c________________________________________________________________________
c  p1 - e+
      p1(1)=momenta(1,0)
      p1(2)=momenta(1,1)
      p1(3)=momenta(1,2)
      p1(4)=momenta(1,3)
c
c  p2 - e-
      p2(1)=momenta(2,0)
      p2(2)=momenta(2,1)
      p2(3)=momenta(2,2)
      p2(4)=momenta(2,3)

c________________________________________________________________________
c
      int_end=integr_Int12(p1)+integr_Int12(p2)
     &  -2.d0*integr_Int4(p1,p2)
      
c
      Int_f1=-alpha/4.d0/pi/pi*(integr_I2_ISR2(p1,p2,delta_2)
     &                                              +   int_end)

      return
      end

c***********************************************************************

c***********************************************************************
      real*8 function integr_Int12(mom)
      include 'phokhara_10.0.inc'
      real*8 mom(4),vec_leng
c
      call leng_v(mom,vec_leng)
c
      integr_Int12=2.d0*pi*mom(1)*dlog((mom(1)-vec_leng)
     1             /(mom(1)+vec_leng))/vec_leng
c
      return
      end
c***********************************************************************
      subroutine element_t(mom1,mom2,t_3pl,t_4min)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 delta2,coeff_b,coeff_a
      real*8 t_3pl,t_4min
      real*8 x1plx2
c
      call coeff1_soft(mom1,mom2,coeff_a)
      call coeff2_soft(mom1,mom2,coeff_b)
      call x_sum(mom1,mom2,x1plx2)

c
      delta2=(4.d0*mom1(1)+2.d0*(mom2(1)-mom1(1))*x1plx2)**2
     2      +4.d0*((mom2(1)-mom1(1))**2-coeff_a)*coeff_b*coeff_b
c
      t_3pl=-(-4.d0*mom1(1)-2.d0*(mom2(1)-mom1(1))
     1     *x1plx2+dsqrt(delta2))/2.d0/coeff_b
     2     /((mom2(1)-mom1(1))+dsqrt(coeff_a))
c
      t_4min=-(-4.d0*mom1(1)-2.d0*(mom2(1)-mom1(1))
     1     *x1plx2-dsqrt(delta2))/2.d0/coeff_b
     2     /((mom2(1)-mom1(1))+dsqrt(coeff_a))
c      
      return
      end
c***********************************************************************
c***********************************************************************
      subroutine limits_t(mom1,mom2,t_a,t_b)
      implicit none
c
      real*8 mom1(4), mom2(4),coeff_b,t_a,t_b
      real*8 x1plx2,x1tmx2
c
      call coeff2_soft(mom1,mom2,coeff_b)
      call x_sum(mom1,mom2,x1plx2)
      call x_mult(mom1,mom2,x1tmx2)
c
      t_a=(-x1plx2-2.d0*dsqrt(x1tmx2))/coeff_b
      t_b=(2.d0-x1plx2-2.d0*dsqrt(1.d0-x1plx2+x1tmx2))/coeff_b
c      
      return
      end
c***********************************************************************
c*********************************************************************** 
      real*8 function integr_Int3(mom1,mom2,tt3,tt4,tx,ty)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 tt3,tt4,tx,ty
      real*8 I11,I12,I13,I14
      real*8 coefA_1,coefB_1,coefC_1,coefD_1,const1
      real*8 integr_I1_1,integr_I1_2,integr_I1_3,integr_I1_4
c
      call coeff3_soft(mom1,mom2,const1)
      call coeff5_soft(mom1,mom2,coefA_1,coefB_1,coefC_1,coefD_1)
c
      I11=integr_I1_1(mom1,mom2,coefA_1,tt3,tt4,tx,ty)
      I12=integr_I1_2(mom1,mom2,coefB_1,tt3,tt4,tx,ty)
      I13=integr_I1_3(mom1,mom2,coefC_1,tt3,tt4,tx,ty)
      I14=integr_I1_4(mom1,mom2,-coefD_1,tt3,tt4,tx,ty)
c
      integr_Int3=const1*(I11+I12+I13+I14)
c
      return
      end
c***********************************************************************
c*********************************************************************** 
c    logharytmic part for 1 fsr 1 isr
      real*8 function integr_I2(mom1_el,mom2_mu,delta)
      include 'phokhara_10.0.inc'
c
      real*8 mom1_el(4),mom2_mu(4)
      real*8 lambda,delta
      real*8 lg,quad,beta,rat1,quad2,delta_x
      real*8 vec_4prod,wsp,mel,m_mu
      common/com1/lambda
c
      pi=3.141592653589793238462643d0 
      mel=0.51099906d-3
      m_mu=0.1056583568d0
c      m_mu=0.13957018d0 !for test !ST
c
      call prod_4(mom1_el,mom2_mu,vec_4prod)
c
      lg=dlog((2.d0*delta/dsqrt(Sp))**2)
      quad=vec_4prod
      quad2=vec_4prod*vec_4prod
      beta=sqrt(1.d0-mel**2*m_mu**2/quad2)
      rat1=(1.d0+beta)**2/mel**2/m_mu**2*quad2
      delta_x=4.d0*(quad2-mel**2*m_mu**2)
c
       integr_I2=2.d0*pi*lg*dlog(rat1)*quad/dsqrt(delta_x)

      return
      end
 
c***********************************************************************
c***********************************************************************
c four vector x=(x_1,x_2,x_3,x_4)
c
      subroutine prod_3(x,y,vec_3prod)
      implicit none
c
      real*8 x(4),y(4),vec_3prod
c
      vec_3prod=x(2)*y(2)+x(3)*y(3)+x(4)*y(4)
c
      return
      end
c***********************************************************************
      subroutine prod_4(x,y,vec_4prod)
      implicit none
c
      real*8 x(4),y(4),vec_3prod,vec_4prod
c
      call prod_3(x,y,vec_3prod)
c
      vec_4prod=x(1)*y(1)-vec_3prod
c
      return
      end
c***********************************************************************
c***********************************************************************
c
      subroutine leng_v(x,vec_leng)
      implicit none 
c
      real*8 vec_leng,x(4)
c
      vec_leng=dsqrt(x(2)*x(2)+x(3)*x(3)+x(4)*x(4))
c
      return
      end
c***********************************************************************
c coeff_a
c 
      subroutine coeff1_soft(mom1,mom2,coeff_a)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 dl1,dl2,vec_leng,vec_3prod
      real*8 coeff_a
c
      call leng_v(mom1,vec_leng)
      dl1=vec_leng
c
      call leng_v(mom2,vec_leng)
      dl2=vec_leng
c        
      call prod_3(mom1,mom2,vec_3prod)
c
      coeff_a=dl1**2+dl2**2-2.d0*vec_3prod
c      
      return
      end
c***********************************************************************  
c***********************************************************************
c coeff_b
      subroutine coeff2_soft(mom1,mom2,coeff_b)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 coeff_b,vec_3prod,vec_leng,dl1,dl2
      real*8 coeff_a,Delta
c
      call coeff1_soft(mom1,mom2,coeff_a)
c
      call prod_3(mom1,mom2,vec_3prod)
      call leng_v(mom1,vec_leng)
      dl1=vec_leng
      call leng_v(mom2,vec_leng)
      dl2=vec_leng
c
      Delta=4.d0*(vec_3prod**2-dl1**2*dl2**2)
c
      coeff_b=-dsqrt(dabs(Delta))/coeff_a
c      
      return
      end
c***********************************************************************
c***********************************************************************
      subroutine x_sum(mom1,mom2,x1plx2)
      implicit none
      real*8 mom1(4), mom2(4)
      real*8 vec_leng,vec_3prod
      real*8 x1plx2,coeff_a
c
      call coeff1_soft(mom1,mom2,coeff_a)
      call prod_3(mom1,mom2,vec_3prod)
      call leng_v(mom1,vec_leng)

c
      x1plx2=2.d0*(vec_leng**2-vec_3prod)/coeff_a
c      
      return
      end

c***********************************************************************
      subroutine x_mult(mom1,mom2,x1tmx2)
      implicit none
      real*8 mom1(4), mom2(4)
      real*8 x1tmx2,x1plx2,coeff_b
c
      call x_sum(mom1,mom2,x1plx2)
      call coeff2_soft(mom1,mom2,coeff_b)
c
      x1tmx2=((x1plx2)**2+coeff_b**2)/4.d0
c
      return
      end

c***********************************************************************
c*********************************************************************** 
      real*8 function integr_I1_1(mom1,mom2,constant_a,tt3,tt4,tx,ty)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 constant_a
      real*8 integr_I1_1_a1,integr_I1_1_a2,integr_I1_1_a3
      real*8 integr_I1_1_a4,integr_I1_1_a5
      real*8 const2,tt3,tt4,tx,ty
      complex*16 cdilog
c
      call coeff4_soft(mom1,mom2,const2)
c
      if(const2.lt.0.d0)then

      integr_I1_1_a1=dlog(-const2)*(dlog(ty-1.d0/tt3)
     1              -dlog(tx-1.d0/tt3))
c
      integr_I1_1_a2=(dlog(ty-1.d0/tt3)*dlog(ty-1.d0/tt3)
     1               -dlog(tx-1.d0/tt3)*dlog(tx-1.d0/tt3))/2.d0

c
       integr_I1_1_a3=(dlog(1.d0/tt4-1.d0/tt3)*dlog(ty-1.d0/tt3)
     1  -dreal(cdilog(dcmplx((ty-1.d0/tt3)/(1.d0/tt4-1.d0/tt3),0.d0))))
     2  -(dlog(1.d0/tt4-1.d0/tt3)*dlog(tx-1.d0/tt3)
     3  -dreal(cdilog(dcmplx((tx-1.d0/tt3)/(1.d0/tt4-1.d0/tt3),0.d0))))

c
      integr_I1_1_a4=-((dlog(-tt3-1.d0/tt3)*dlog(ty-1.d0/tt3)
     1  -dreal(cdilog(dcmplx((ty-1.d0/tt3)/(-tt3-1.d0/tt3),0.d0))))
     2  -(dlog(-tt3-1.d0/tt3)*dlog(tx-1.d0/tt3)
     3  -dreal(cdilog(dcmplx((tx-1.d0/tt3)/(-tt3-1.d0/tt3),0.d0)))))
c
      integr_I1_1_a5=-((dlog(-tt4-1.d0/tt3)*dlog(ty-1.d0/tt3)
     1  -dreal(cdilog(dcmplx((ty-1.d0/tt3)/(-tt4-1.d0/tt3),0.d0))))
     2  -(dlog(-tt4-1.d0/tt3)*dlog(tx-1.d0/tt3)
     3  -dreal(cdilog(dcmplx((tx-1.d0/tt3)/(-tt4-1.d0/tt3),0.d0)))))

      elseif(const2.ge.0.d0)then
 
      integr_I1_1_a1=dlog(const2)*(dlog(-ty+1.d0/tt3)
     1              -dlog(-tx+1.d0/tt3))
      
c 
      integr_I1_1_a2=(dlog(-ty+1.d0/tt3)*dlog(-ty+1.d0/tt3)
     1               -dlog(-tx+1.d0/tt3)*dlog(-tx+1.d0/tt3))/2.d0
c
       integr_I1_1_a3=(dlog(-1.d0/tt4+1.d0/tt3)*dlog(-ty+1.d0/tt3)
     1 -dreal(cdilog(dcmplx((-ty+1.d0/tt3)/(-1.d0/tt4+1.d0/tt3),0.d0))))
     2  -(dlog(-1.d0/tt4+1.d0/tt3)*dlog(-tx+1.d0/tt3)
     3 -dreal(cdilog(dcmplx((-tx+1.d0/tt3)/(-1.d0/tt4+1.d0/tt3),0.d0))))
c 
      integr_I1_1_a4=-((dlog(tt3+1.d0/tt3)*dlog(-ty+1.d0/tt3)
     1  -dreal(cdilog(dcmplx((-ty+1.d0/tt3)/(tt3+1.d0/tt3),0.d0))))
     2  -(dlog(tt3+1.d0/tt3)*dlog(-tx+1.d0/tt3)
     3  -dreal(cdilog(dcmplx((-tx+1.d0/tt3)/(tt3+1.d0/tt3),0.d0)))))

      integr_I1_1_a5=(-dlog(1.d0/tt3-ty)*dlog(-ty-tt4) 
     1  +dlog(1.d0/tt3+tt4)*dlog(-ty-tt4)
     2  -dreal(cdilog(dcmplx((-ty-tt4)/(-1.d0/tt3-tt4),0.d0))))
     3  -(-dlog(1.d0/tt3-tx)*dlog(-tx-tt4) 
     1  +dlog(1.d0/tt3+tt4)*dlog(-tx-tt4)
     2  -dreal(cdilog(dcmplx((-tx-tt4)/(-1.d0/tt3-tt4),0.d0))))

      endif
c
      integr_I1_1=constant_a*(integr_I1_1_a1+integr_I1_1_a2
     1           +integr_I1_1_a3+integr_I1_1_a4+integr_I1_1_a5)
c

      return
      end
c***********************************************************************
c*********************************************************************** 
c*********************************************************************** 
      real*8 function integr_I1_2(mom1,mom2,constant_b,tt3,tt4,tx,ty)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 constant_b
      real*8 integr_I1_1_b1,integr_I1_1_b2,integr_I1_1_b3
      real*8 integr_I1_1_b4,integr_I1_1_b5
      real*8 const2,tt3,tt4,tx,ty
      complex*16 cdilog
c
      call coeff4_soft(mom1,mom2,const2)
c
      if(const2.lt.0.d0)then
      integr_I1_1_b1=dlog(-const2)*(dlog(ty-1.d0/tt4)
     1              -dlog(tx-1.d0/tt4))
c
      integr_I1_1_b2=(dlog(1.d0/tt4-1.d0/tt3)*dlog(ty-1.d0/tt4)
     1 -dreal(cdilog(dcmplx(-(ty-1.d0/tt4)/(1.d0/tt4-1.d0/tt3),0.d0))))
     2 -(dlog(1.d0/tt4-1.d0/tt3)*dlog(tx-1.d0/tt4)
     3 -dreal(cdilog(dcmplx(-(tx-1.d0/tt4)/(1.d0/tt4-1.d0/tt3),0.d0))))
c
      elseif(const2.ge.0.d0)then
c
      integr_I1_1_b1=dlog(const2)*(dlog(ty-1.d0/tt4)
     1              -dlog(tx-1.d0/tt4))
c
      integr_I1_1_b2=(dlog(-1.d0/tt4+1.d0/tt3)*dlog(ty-1.d0/tt4)
     1 -dreal(cdilog(dcmplx((ty-1.d0/tt4)/(-1.d0/tt4+1.d0/tt3),0.d0))))
     2 -(dlog(-1.d0/tt4+1.d0/tt3)*dlog(tx-1.d0/tt4)
     3 -dreal(cdilog(dcmplx((tx-1.d0/tt4)/(-1.d0/tt4+1.d0/tt3),0.d0))))
      endif


c
      integr_I1_1_b3=(dlog(ty-1.d0/tt4)*dlog(ty-1.d0/tt4)
     1               -dlog(tx-1.d0/tt4)*dlog(tx-1.d0/tt4))/2.d0
c
      integr_I1_1_b4=-((dlog(ty-1.d0/tt4)*dlog(ty+tt3)
     1  -dlog(-tt3-1.d0/tt4)*dlog(ty+tt3)
     2  +dreal(cdilog(dcmplx(-(ty+tt3)/(-tt3-1.d0/tt4),0.d0))))
     3  -(dlog(tx-1.d0/tt4)*dlog(tx+tt3)
     4  -dlog(-tt3-1.d0/tt4)*dlog(tx+tt3)
     5  +dreal(cdilog(dcmplx(-(tx+tt3)/(-tt3-1.d0/tt4),0.d0)))))
c
      integr_I1_1_b5=-((dlog(-tt4-1.d0/tt4)*dlog(ty-1.d0/tt4)
     1  -dreal(cdilog(dcmplx((ty-1.d0/tt4)/(-tt4-1.d0/tt4),0.d0))))
     2  -(dlog(-tt4-1.d0/tt4)*dlog(tx-1.d0/tt4)
     3  -dreal(cdilog(dcmplx((tx-1.d0/tt4)/(-tt4-1.d0/tt4),0.d0)))))


      integr_I1_2=constant_b*(integr_I1_1_b1+integr_I1_1_b2
     1           +integr_I1_1_b3+integr_I1_1_b4+integr_I1_1_b5)

      return
      end

c***********************************************************************
c***********************************************************************
c***********************************************************************
      real*8 function integr_I1_3(mom1,mom2,constant_c,tt3,tt4,tx,ty)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 constant_c
      real*8 integr_I1_1_c1,integr_I1_1_c2,integr_I1_1_c3
      real*8 integr_I1_1_c4,integr_I1_1_c5
      real*8 const2,tt3,tt4,tx,ty
      complex*16 cdilog
c
      call coeff4_soft(mom1,mom2,const2)
c
      if(const2.lt.0.d0)then
      integr_I1_1_c1=dlog(-const2)*(dlog(ty+tt3)
     1              -dlog(tx+tt3))
c
      integr_I1_1_c2=(dlog(-1.d0/tt3-tt3)*dlog(ty+tt3)
     1  -dreal(cdilog(dcmplx(-(ty+tt3)/(-1.d0/tt3-tt3),0.d0))))
     2  -(dlog(-1.d0/tt3-tt3)*dlog(tx+tt3)
     3  -dreal(cdilog(dcmplx(-(tx+tt3)/(-1.d0/tt3-tt3),0.d0))))
c
      elseif(const2.ge.0.d0)then
c
      integr_I1_1_c1=dlog(const2)*(dlog(ty+tt3)
     1              -dlog(tx+tt3))

c
      integr_I1_1_c2=(dlog(1.d0/tt3+tt3)*dlog(ty+tt3)
     1  -dreal(cdilog(dcmplx((ty+tt3)/(1.d0/tt3+tt3),0.d0))))
     2  -(dlog(1.d0/tt3+tt3)*dlog(tx+tt3)
     3  -dreal(cdilog(dcmplx((tx+tt3)/(1.d0/tt3+tt3),0.d0))))
      endif
c
      integr_I1_1_c3=(dlog(-1.d0/tt4-tt3)*dlog(ty+tt3)
     1  -dreal(cdilog(dcmplx(-(ty+tt3)/(-1.d0/tt4-tt3),0.d0))))
     2  -(dlog(-1.d0/tt4-tt3)*dlog(tx+tt3)
     3  -dreal(cdilog(dcmplx(-(tx+tt3)/(-1.d0/tt4-tt3),0.d0))))
c
      integr_I1_1_c4=-(dlog(ty+tt3)*dlog(ty+tt3)
     1               -dlog(tx+tt3)*dlog(tx+tt3))/2.d0
c
      integr_I1_1_c5=-((dlog(-tt4+tt3)*dlog(ty+tt3)
     1  -dreal(cdilog(dcmplx((ty+tt3)/(-tt4+tt3),0.d0))))
     2  -(dlog(-tt4+tt3)*dlog(tx+tt3)
     3  -dreal(cdilog(dcmplx((tx+tt3)/(-tt4+tt3),0.d0)))))


      integr_I1_3=constant_c*(integr_I1_1_c1+integr_I1_1_c2
     1           +integr_I1_1_c3+integr_I1_1_c4+integr_I1_1_c5)

c
  
      return
      end
c*********************************************************************** 
c*********************************************************************** 
c*********************************************************************** 
      real*8 function integr_I1_4(mom1,mom2,constant_d,tt3,tt4,tx,ty)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 constant_d
      real*8 integr_I1_1_d1,integr_I1_1_d2,integr_I1_1_d3
      real*8 integr_I1_1_d4,integr_I1_1_d5
      real*8 const2,tt3,tt4,tx,ty
      complex*16 cdilog
c
      call coeff4_soft(mom1,mom2,const2)
c
      if(const2.lt.0.d0)then
      integr_I1_1_d1=-dlog(-const2)*(dlog(-ty-tt4)
     1              -dlog(-tx-tt4))
c
      integr_I1_1_d2=-((dlog(-1.d0/tt3-tt4)*dlog(-ty-tt4)
     1  -dreal(cdilog(dcmplx(-(-ty-tt4)/(1.d0/tt3+tt4),0.d0))))
     2  -(dlog(-1.d0/tt3-tt4)*dlog(-tx-tt4)
     3  -dreal(cdilog( dcmplx(-(-tx-tt4)/(1.d0/tt3+tt4),0.d0)))))
c
      elseif(const2.ge.0.d0)then
c
      integr_I1_1_d1=-dlog(const2)*(dlog(-ty-tt4)
     1              -dlog(-tx-tt4))
c
      integr_I1_1_d2=-((dlog(1.d0/tt3+tt4)*dlog(-ty-tt4)
     1  -dreal(cdilog(dcmplx(-(-ty-tt4)/(1.d0/tt3+tt4),0.d0))))
     2  -(dlog(1.d0/tt3+tt4)*dlog(-tx-tt4)
     3  -dreal(cdilog( dcmplx(-(-tx-tt4)/(1.d0/tt3+tt4),0.d0)))))
      endif

c
      integr_I1_1_d3=-((dlog(-1.d0/tt4-tt4)*dlog(-ty-tt4)
     1  -dreal(cdilog(dcmplx(-(-ty-tt4)/(1.d0/tt4+tt4),0.d0))))
     2  -(dlog(-1.d0/tt4-tt4)*dlog(-tx-tt4)
     3  -dreal(cdilog(dcmplx(-(-tx-tt4)/(1.d0/tt4+tt4),0.d0)))))
c
      integr_I1_1_d4=(dlog(-tt4+tt3)*dlog(-ty-tt4)
     1  -dreal(cdilog(dcmplx(-(-ty-tt4)/(tt4-tt3),0.d0))))
     2  -(dlog(-tt4+tt3)*dlog(-tx-tt4)
     3  -dreal(cdilog(dcmplx(-(-tx-tt4)/(tt4-tt3),0.d0))))
c
      integr_I1_1_d5=(dlog(-ty-tt4)*dlog(-ty-tt4)
     1               -dlog(-tx-tt4)*dlog(-tx-tt4))/2.d0
c
      integr_I1_4=constant_d*(integr_I1_1_d1+integr_I1_1_d2
     1           +integr_I1_1_d3+integr_I1_1_d4+integr_I1_1_d5)


c
      return
      end

c***********************************************************************
c***********************************************************************
c***********************************************************************
c***********************************************************************
c const1 - name on paper calculations : const1 (page 12)
      subroutine coeff3_soft(mom1,mom2,const1)
      implicit none
c
      real*8 mom1(4),mom2(4),vec_4prod
      real*8 coeff_a,coeff_b,roz,pi,const1

c
      call prod_4(mom1,mom2,vec_4prod)
      call coeff1_soft(mom1,mom2,coeff_a)
      call coeff2_soft(mom1,mom2,coeff_b)
c
      pi=3.141592653589793238462643d0 
      roz=(mom2(1)-mom1(1))**2-coeff_a
c
      const1=8.d0*pi*vec_4prod/dsqrt(coeff_a)/coeff_b/coeff_b/roz
c      
      return
      end
c***********************************************************************
c*********************************************************************** 
c const2 - name on paper calculations : const2 (page 12)
      subroutine coeff4_soft(mom1,mom2,const2)
      implicit none
c
      real*8 mom1(4), mom2(4)
      real*8 coeff_a,const2
c
      call coeff1_soft(mom1,mom2,coeff_a)
c
      const2=((mom2(1)-mom1(1))-dsqrt(coeff_a))
     1      /((mom2(1)-mom1(1))+dsqrt(coeff_a))
c
      return
      end
c***********************************************************************
c*********************************************************************** 
      subroutine coeff5_soft(mom1,mom2,coefA_1,coefB_1,coefC_1,coefD_1)
      implicit none
c
      real*8 mom1(4),mom2(4)
      real*8 coefA_1,coefB_1,coefC_1,coefD_1
      real*8 coeff_b,t_3pl,t_4min,coeff_a
      real*8 roznica,suma,suma1,suma2,suma3
      real*8 suma4,suma5,suma6,vec_leng,vec_3prod,x1plx2
c      
      call coeff1_soft(mom1,mom2,coeff_a)
      call coeff2_soft(mom1,mom2,coeff_b)
      call element_t(mom1,mom2,t_3pl,t_4min)
      call x_sum(mom1,mom2,x1plx2)
c
c
      roznica=mom2(1)-mom1(1)
      suma1=t_3pl+1.d0/t_3pl
      suma2=t_4min+1.d0/t_3pl
      suma3=1.d0/t_3pl-1.d0/t_4min
      suma4=t_3pl+1.d0/t_4min
      suma5=t_4min+1.d0/t_4min
      suma6=t_3pl-t_4min
c
      coefA_1=(roznica*(-coeff_b+2.d0*x1plx2/t_3pl+coeff_b/t_3pl/t_3pl)
     1       +4.d0*mom1(1)/t_3pl)/suma1/suma2/suma3

      coefB_1=-(roznica*(-coeff_b+2.d0*x1plx2/t_4min
     1       +coeff_b/t_4min/t_4min)+4.d0*mom1(1)/t_4min)
     2       /suma3/suma4/suma5

      coefC_1=(roznica*(coeff_b+2.d0*x1plx2*t_3pl-coeff_b*t_3pl*t_3pl)
     1       +4.d0*mom1(1)*t_3pl)/suma6/suma1/suma4
c
      coefD_1=(roznica*(-coeff_b-2.d0*x1plx2*t_4min
     1       +coeff_b*t_4min*t_4min)-4.d0*mom1(1)*t_4min)
     2       /suma6/suma2/suma5
c      
      return
      end
c***********************************************************************
c    logarithmic part for 2 fsr
      real*8 function integr_I2_FSR2(mom1_mu,mom2_mu,delta,qq)
      include 'phokhara_10.0.inc' 
c
      real*8 mom1_mu(4),mom2_mu(4)
      real*8 lambda,delta,qq
      real*8 px1_pl,px2_min,numer1,numer2
      real*8 vec_4prod,wsp,m_mu
      real*8 lg,quad1,quad2,quad3
      real*8 coeff1_FSR,coeff2_FSR,coeff3_FSR
      real*8 delta_x,dif,rat1,lg2,beta
c
      m_mu = mmu 

c
      call prod_4(mom1_mu,mom1_mu,vec_4prod)
      quad1=vec_4prod
      call prod_4(mom2_mu,mom2_mu,vec_4prod)
      quad2=vec_4prod
      call prod_4(mom1_mu,mom2_mu,vec_4prod)
      quad3=vec_4prod
c
      lg=dlog((2.d0*delta/dsqrt(Sp))**2)
      lg = lg !+  2.d0*( 1.d0/(1.d0-Sp/qq)*Log(Sp/qq))
c
      delta_x=4.d0*(quad3*quad3-m_mu**4)
      dif=quad3-m_mu**2
c
      beta = sqrt(1.d0-4.d0*m_mu**2/qq)
      delta_x = (qq*beta)**2
c
      rat1 = ((1.d0+beta)**2/(4.d0*m_mu**2/qq))**2
      lg2=dlog(rat1)
c
      coeff1_FSR=2.d0*pi
      coeff2_FSR=2.d0*pi

      coeff3_FSR=2.d0*pi*quad3*lg2/dsqrt(delta_x)
c

      integr_I2_FSR2=lg*(coeff1_FSR+coeff2_FSR-2.d0*coeff3_FSR)
c
      return
      end
c*********************************************************************** 
c******************************************************************************
c    logarithmic part for 2 isr
      real*8 function integr_I2_ISR2(mom1_el,mom2_el,delta)
      include 'phokhara_10.0.inc' 
c
      real*8 mom1_el(4),mom2_el(4)
      real*8 lambda,delta,qq
      real*8 px1_pl,px2_min,numer1,numer2
      real*8 vec_4prod,wsp,m_mu
      real*8 lg,quad1,quad2,quad3
      real*8 coeff1_ISR,coeff2_ISR,coeff3_ISR
      real*8 delta_x,dif,rat1,lg2,beta

c

      call prod_4(mom1_el,mom2_el,vec_4prod)
      quad3=vec_4prod
c
      lg=dlog((2.d0*delta/dsqrt(Sp))**2)
c
      beta = sqrt(1.d0+me**2/mom1_el(4)**2)
      delta_x =(mom1_el(4)**2*beta)**2
c
      rat1 = ((1.d0+beta)**2/me**2*mom1_el(4)**2)**2
      lg2=dlog(rat1)
c


      coeff1_ISR=2.d0*pi
      coeff2_ISR=2.d0*pi

      coeff3_ISR=pi*quad3*lg2/dsqrt(delta_x)/2.d0
c

      integr_I2_ISR2=lg*(coeff1_ISR+coeff2_ISR-2.d0*coeff3_ISR)
c
      return
      end

c****************************************************************************** 
c******************************************************************************
      real*8 function integr_Int4(mom1,mom2)
       include 'phokhara_10.0.inc'
c
      real*8 mom1(4),mom2(4)
      real*8 vec_4prod,const
      real*8 constA,constB
      real*8 inte1,inte2,inte3
      complex*16 cdilog
c
      
      call prod_4(mom1,mom2,vec_4prod)
c
      const=pi*vec_4prod/2.d0/mom1(1)/abs(mom1(4))
c
      constA=mom1(1)/2.d0/mom1(4) +0.5d0
      constB=constA-1.d0
c
      inte1=2.d0*(cdilog(dcmplx(0.5d0/(0.5d0-constA),0.d0))
     &          - cdilog(dcmplx(0.5d0/(0.5d0+constB),0.d0))
     &          - cdilog(dcmplx(0.5d0/(constA-0.5d0),0.d0))
     &          + cdilog(dcmplx(-0.5d0/(0.5d0+constB),0.d0)))
c
      inte2=-0.5d0*(dlog(constB)**2-dlog(constA)**2)
     &      +(dlog(constA+constB)*(dlog(constB)-dlog(constA))
     &      -cdilog(dcmplx(constB/(constA+constB),0.d0))
     &      +cdilog(dcmplx(constA/(constA+constB),0.d0)))
c
      inte3=-0.5d0*(dlog(constA)**2-dlog(constB)**2)
     &     +(dlog(constA+constB)*(dlog(constA)-dlog(constB))
     &      -cdilog(dcmplx(constA/(constA+constB),0.d0))
     &      +cdilog(dcmplx(constB/(constA+constB),0.d0)))
c
      integr_Int4=const*(inte1+inte2-inte3)

      return
      end
c***********************************************************************

c***********************************************************************
c*********************************************************************** 
      real*8 function virt_muons(delta_2,qq)
      include 'phokhara_10.0.inc'
      real*8     ISR_Born,FSR_Born,ISRFSR_Born
      real*8     virt_isr_fsr_box,virt_isr_fsr_penta,virt_triangle
      real*8     Int_f1,Int_f,Int_f2,soft,ampLOm2
      real*8     p1(0:3),p2(0:3),p3(0:3),p4(0:3),p5(0:3),q(4),rk1(4)
      real*8     mle2,mlm2,s12,mu2,dps1,dps2,delta,vamp,delta_2,qq
      complex*16 tree,loop,loop1,loop2,loop3,loop4,gammu(4,2,2)
      complex*16 v1(2,2),v2(2,2),up1(2,2),up2(2,2),gam(4)
      complex*16 gammu_ma(4,2,2),tree1
      complex*16 cvac_qq,vacpol_and_nr
      real*8 test, born_mu
      integer    ip,eps
      integer ic1,ic2,i1


c--------------
      q(1)   = momenta(5,0)
      q(2)   = momenta(5,1)
      q(3)   = momenta(5,2)
      q(4)   = momenta(5,3)
c

      rk1(1)=momenta(3,0)
      rk1(2)=momenta(3,1)
      rk1(3)=momenta(3,2)
      rk1(4)=momenta(3,3)
c--------------
      mu2=Sp
      mle2=me**2
      mlm2=mmu**2
      eps=0
c
      call spinors(mmu,up1,up2,v1,v2)
      call gam1(gam,gammu,gammu_ma,v1,v2,up1,up2,qq)
      cvac_qq = vacpol_and_nr(qq)
c
      do ic1=1,2
      do ic2=1,2

        do i1=1,4
         gam(i1)=gammu(i1,ic1,ic2)
        enddo
c
      enddo
      enddo

c
      dps1=(1.d0-qq/Sp)/(32.d0*pi*pi)
      dps2=dSqrt(1.d0-4.d0*mmu*mmu/qq)/(32.d0*pi*pi)

      do ip=0,3
      p1(ip)=momenta(1,ip)
      p2(ip)=momenta(2,ip)
      p3(ip)=momenta(6,ip)
      p4(ip)=momenta(7,ip)
      p5(ip)=momenta(3,ip)
      enddo





      if(fsr.eq.0)then

      call eemmgloopisr(loop,tree,p1,p2,p3,p4,p5,mle2,mlm2,mu2,eps)

      ISR_Born = dreal(tree)
      virt_isr_fsr_box = dreal(loop)

      virt_muons = 0.25D0*dps1*dps2*(4*pi*alpha)**3*
     &    (
     &       (alpha/(2*pi))*virt_isr_fsr_box                            ! BOX ISR part
     &    + Int_f1(delta_2)*ISR_Born                                    ! soft part 1hard 1 soft from electron line
     &    + ISR_Born)                                                   ! ISR BORN

      elseif(fsr.eq.1)then

      call eemmgloopisr(loop,tree,p1,p2,p3,p4,p5,mle2,mlm2,mu2,eps)

      ISR_Born = dreal(tree)
      virt_isr_fsr_box = dreal(loop)

      call eemmgloopisr(loop,tree,-p3,-p4,-p1,-p2,p5,mlm2,mle2,mu2,eps)

      FSR_Born = dreal(tree)
      virt_isr_fsr_box = virt_isr_fsr_box + dreal(loop)


      virt_muons = 0.25D0*dps1*dps2*(4*pi*alpha)**3*
     &    (
     &       (alpha/(2*pi))*virt_isr_fsr_box                            ! BOX ISR and FSR part
     &    + Int_f1(delta_2)*ISR_Born                                    ! soft part 1hard 1 soft from electron line
     &    + ISR_Born                                                    ! ISR BORN
     &    + Int_f2(delta_2,qq)*FSR_Born                                 ! soft part 1hard 1 soft from muon line
     &    + FSR_Born)                                                   ! FSR BORN
      elseif(fsr.eq.2)then
      soft = (Int_f1(delta_2)+2.d0*Int_f(delta_2)+Int_f2(delta_2,qq))
c
      call eemmgloopisr(loop1,tree1,p1,p2,p3,p4,p5,mle2,mlm2,mu2,eps)
      call eemmgloopisr(loop2,tree,-p3,-p4,-p1,-p2,p5,mlm2,mle2,mu2,eps)
      call eemmgloopisrfsr(loop3,tree,p1,p2,p3,p4,p5,mle2,mlm2,mu2,eps)
      call eemmgloopisrfsr(loop4,tree,-p3,-p4,-p1,-p2,p5
     & ,mlm2,mle2,mu2,eps)
c
c       virt_isr_fsr_box = dreal(loop1+loop2+loop3+loop4) ! version without vacpol - independent test of Born
       virt_isr_fsr_box =
     &   dreal(loop1)*cdabs(cvac_qq)**2       ! isr only
     & + dreal(loop2)*cdabs(cvac_s)**2        ! fsr only
     & + dreal(loop3)*dreal(cvac_qq*dconjg(cvac_s))                         ! fsr born * virt isr
     & + dreal(loop4)*dreal(cvac_s*dconjg(cvac_qq))                         ! isr born * virt fsr
c
      virt_triangle = ampLOm2(qq,rk1,gam,gammu,gammu_ma,v1,v2,up1,up2,q)
c
      call eemmgloop5(loop,tree,p1,p2,p3,p4,p5,mle2,mlm2,mu2,eps)
      virt_isr_fsr_penta = dreal(loop)*dreal(cvac_qq*dconjg(cvac_s))
c
      ISRFSR_Born = dreal(tree)
      ISRFSR_Born = born_mu(qq,rk1,gam,gammu,gammu_ma,v1,v2,up1,up2,q)
c
      virt_muons = 0.25D0*dps1*dps2*(4*pi*alpha)**3*
     &    (
     &       (alpha/(2.d0*pi))*virt_isr_fsr_box
     &     + (alpha/(2.d0*pi))*virt_isr_fsr_penta
     &     +  (alpha/(2.d0*pi))*virt_triangle
     &     + soft*ISRFSR_Born
     &     +                ISRFSR_Born)


      endif
c
      return
      end

c-----------------------------------------------------------------------      
c********************chi production*************************************
c-----------------------------------------------------------------------     
      real*8 function chi_prod1(qq)
      include 'phokhara_10.0.inc' 
      real*8 qq, rk1(4),p1(4),p2(4),dme,el_m2,ebppb
     1 ,facchi
      complex*16 ma(2,2),mb(2,2),ma_ma(2,2),mb_ma(2,2)
      complex*16 up1(2,2),up2(2,2),v1(2,2),v2(2,2),uupp1(1,2),uupp2(1,2)
     1 ,vv1(2,1),vv2(2,1),eck1(4),epsk1(2,4),ten_chi(4,4),
     1 gammu(4,2,2),gammu_ma(4,2,2),gam(4),gam_ma(4),ddpl(2,2),
     1 ddmi(2,2),ddpl_ph(2,2),ddmi_ph(2,2),ddpl_Z(2,2),ddmi_Z(2,2),
     2 ddmi_Zph(2,2),ddpl_Zph(2,2),mat_Z_vec,mat_Z_avec,
     3 ve1mi(2,2),ve2mi(2,2),ve1pl(2,2),ve2pl(2,2),ax1mi(2,2),
     4 ax2mi(2,2),ax1pl(2,2),ax2pl(2,2),vec_part,avec_part,
     5 mbZ(2,2),maZ(2,2),mbZph(2,2),maZph(2,2),
     6 ddpl_p2s(2,2),ddmi_p2s(2,2)
       complex*16 cvac_qq,vacpol_and_nr
      integer ic1,ic2,i1,i,imu
        common/matri1/ma,mb,ma_ma,mb_ma
        common /cp1p2/p1,p2,dme,el_m2
        
        
      

       facchi=(mchic1/2.d0+m2c)*mchic1*2.d0*dsqrt(2.d0)*dsqrt(m2c)*
     1 (2.d0/3.d0)**2

       vec_part=ac*facchi**(-1)*GFermi*( 2.D0*mchic1**2-8.D0*mchic1**2*
     &    sinthW2 + 4.D0*m2c*mchic1 - 16.D0*m2c*mchic1*sinthW2 )

       avec_part=  ac*facchi**(-1)*GFermi * ( 2.D0*mchic1**2 + 4.D
     &    0*m2c*mchic1 )

              
       
             chi_prod1=0.d0
             
            do i1=1,4
             rk1(i1) = momenta(3,i1-1)
             enddo
             ebppb=p1(1)+p1(4)
              call pol_vec(rk1,epsk1)    
              call skalar1LO(rk1)   
              
            call gam1(gam,gammu,gammu_ma,v1,v2,up1,up2,qq)            
           
           cvac_qq = vacpol_and_nr(qq)

            do ic1=1,2
             do ic2=1,2
c
c the spinors
c
         uupp2(1,1) = up2(1,ic1)
         uupp2(1,2) = up2(2,ic1)
         uupp1(1,1) = up1(1,ic1)
         uupp1(1,2) = up1(2,ic1)
    
         vv1(1,1) = v1(1,ic2)                 
         vv1(2,1) = v1(2,ic2)                 
         vv2(1,1) = v2(1,ic2)                 
         vv2(2,1) = v2(2,ic2)    


          do imu=1,4
           gam(imu) = gammu(imu,ic1,ic2)           
           gam_ma(imu) = gammu_ma(imu,ic1,ic2)
         enddo
         call skalar1aLO(gam,gam_ma)

c           sum over photon polarizations
c
          do i=1,2
c                  eck1(1)=rk1(1)
c                  eck1(2)=rk1(2)
c                  eck1(3)=rk1(3)
c                  eck1(4)=rk1(4)
                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4) 

         call skalar2LO(rk1,eck1) 
         call blocksLO(qq)
         call tensorchi(rk1,eck1,ten_chi)

         call block_chi(qq,ten_chi,ddpl,ddmi,ddpl_ph,ddmi_ph,rk1,gam,
     1 ddmi_Z,ddpl_Z,ddmi_Zph,ddpl_Zph,ddpl_p2s,ddmi_p2s)

         call conmat(vec_part,ddmi_Z,ve1mi)
         call conmat(avec_part,ddmi_Z,ax1mi)
         call conmat(vec_part,ddpl_Z,ve1pl)
         call conmat(avec_part,ddpl_Z,ax1pl)

         call minmat(ve1pl,ax1pl,maZ)
         call dodmat(ve1mi,ax1mi,mbZ)
  
         call conmat(vec_part,ddmi_Zph,ve2mi)
         call conmat(avec_part,ddmi_Zph,ax2mi)
         call conmat(vec_part,ddpl_Zph,ve2pl)
         call conmat(avec_part,ddpl_Zph,ax2pl)

         call minmat(ve2pl,ax2pl,maZph)
         call dodmat(ve2mi,ax2mi,mbZph)

     

            ! ma,mb --- radiative return
            ! ddpl,ddmi --- J/psi gamma
            ! ddpl_ph,ddmi_ph --- gamma gamma
            ! ddpl_Z,ddmi_Z --- gamma gamma
           

! radiative return
          if((chi_sw.eq.0).or.(chi_sw.eq.2))then
          chi_prod1=chi_prod1+ ( (dme*cdabs(mb(1,1)-ma(1,1)))**2
     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
     4          *cdabs(cvac_qq)**2
          endif
       
! chi production with decay to J/psi gamma and 2 gamma
       if((chi_sw.eq.1).or.(chi_sw.eq.2))then
        chi_prod1=chi_prod1+ ( (dme*cdabs(-ddpl(1,1)-ddmi(1,1)))**2
     1         +(dme*cdabs(-ddpl(2,2)-ddmi(2,2)))**2
     2         +(cdabs(-ebppb*ddmi(2,1)-el_m2/ebppb*ddpl(2,1)))**2
     3         +(cdabs(-ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2 )
     4    /(4.d0*pi*alpha)**3

       chi_prod1=chi_prod1+((dme*cdabs(-ddpl_ph(1,1)-ddmi_ph(1,1)))**2
     1       +(dme*cdabs(-ddpl_ph(2,2)-ddmi_ph(2,2)))**2
     2       +(cdabs(-ebppb*ddmi_ph(2,1)-el_m2/ebppb*ddpl_ph(2,1)))**2
     3       +(cdabs(-ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2)))**2)
     4    /(4.d0*pi*alpha)**3

        chi_prod1=chi_prod1+2.d0*dreal(
     1    dconjg(dme*(-ddpl(1,1)-ddmi(1,1)))
     2    *(dme*(-ddpl_ph(1,1)-ddmi_ph(1,1)))
     3    +dconjg(dme*(-ddpl(2,2)-ddmi(2,2)))
     4    *(dme*(-ddpl_ph(2,2)-ddmi_ph(2,2)))
     5    +dconjg(-ebppb*ddmi(2,1)-el_m2/ebppb*ddpl(2,1))
     6    *(-ebppb*ddmi_ph(2,1)-el_m2/ebppb*ddpl_ph(2,1))
     7    +(dconjg(-ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))
     8    *(-ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2))
     9    )/(4.d0*pi*alpha)**3
c psi' contribution
      chi_prod1=chi_prod1+((dme*cdabs(-ddpl_p2s(1,1)-ddmi_p2s(1,1)))**2
     1      +(dme*cdabs(-ddpl_p2s(2,2)-ddmi_p2s(2,2)))**2
     2      +(cdabs(-ebppb*ddmi_p2s(2,1)-el_m2/ebppb*ddpl_p2s(2,1)))**2
     3      +(cdabs(-ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2)))**2)
     4    /(4.d0*pi*alpha)**3

      chi_prod1=chi_prod1+2.d0*dreal(
     1    dconjg(dme*(-ddpl(1,1)-ddmi(1,1)))
     2    *(dme*(-ddpl_p2s(1,1)-ddmi_p2s(1,1)))
     3    +dconjg(dme*(-ddpl(2,2)-ddmi(2,2)))
     4    *(dme*(-ddpl_p2s(2,2)-ddmi_p2s(2,2)))
     5    +dconjg(-ebppb*ddmi(2,1)-el_m2/ebppb*ddpl(2,1))
     6    *(-ebppb*ddmi_p2s(2,1)-el_m2/ebppb*ddpl_p2s(2,1))
     7    +(dconjg(-ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))
     8    *(-ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2))
     9    )/(4.d0*pi*alpha)**3


      chi_prod1=chi_prod1+2.d0*dreal(
     1    dconjg(dme*(-ddpl_p2s(1,1)-ddmi_p2s(1,1)))
     2    *(dme*(-ddpl_ph(1,1)-ddmi_ph(1,1)))
     3    +dconjg(dme*(-ddpl_p2s(2,2)-ddmi_p2s(2,2)))
     4    *(dme*(-ddpl_ph(2,2)-ddmi_ph(2,2)))
     5    +dconjg(-ebppb*ddmi_p2s(2,1)-el_m2/ebppb*ddpl_p2s(2,1))
     6    *(-ebppb*ddmi_ph(2,1)-el_m2/ebppb*ddpl_ph(2,1))
     7    +(dconjg(-ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2)))
     8    *(-ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2))
     9    )/(4.d0*pi*alpha)**3
c
! neutral current parts 

       chi_prod1=chi_prod1+ ( (dme*cdabs(mbZ(1,1)-maZ(1,1)))**2
     1          +(dme*cdabs(mbZ(2,2)-maZ(2,2)))**2
     2          +(cdabs(-ebppb*maZ(2,1)+el_m2/ebppb*mbZ(2,1)))**2
     3          +(cdabs(ebppb*mbZ(1,2)-el_m2/ebppb*maZ(1,2)))**2 )
     4    /(4.d0*pi*alpha)**3

       chi_prod1=chi_prod1+ ( (dme*cdabs(mbZph(1,1)-maZph(1,1)))**2
     1          +(dme*cdabs(mbZph(2,2)-maZph(2,2)))**2
     2          +(cdabs(-ebppb*maZph(2,1)+el_m2/ebppb*mbZph(2,1)))**2
     3          +(cdabs(ebppb*mbZph(1,2)-el_m2/ebppb*maZph(1,2)))**2 )
     4 /(4.d0*pi*alpha)**3

      chi_prod1=chi_prod1+2.d0*dreal(
     1    dconjg(dme*(mbZ(1,1)-maZ(1,1)))
     2    *(dme*(mbZph(1,1)-maZph(1,1)))
     3    +dconjg(dme*(mbZ(2,2)-maZ(2,2)))
     4    *(dme*(mbZph(2,2)-maZph(2,2)))
     5    +dconjg(-ebppb*maZ(2,1)+el_m2/ebppb*mbZ(2,1))
     6    *(-ebppb*maZph(2,1)+el_m2/ebppb*mbZph(2,1))
     7    +(dconjg(ebppb*mbZ(1,2)-el_m2/ebppb*maZ(1,2)))
     8    *(ebppb*mbZph(1,2)-el_m2/ebppb*maZph(1,2))
     9    )/(4.d0*pi*alpha)**3
! interferences of Z with QED amplitudes
      
      chi_prod1=chi_prod1+2.d0*dreal((
     5    dconjg(dme*(mbZ(1,1)-maZ(1,1)))
     5     *dme*(-ddpl(1,1)-ddmi(1,1))
     6   +dconjg(dme*(mbZ(2,2)-maZ(2,2)))
     6     *dme*(-ddpl(2,2)-ddmi(2,2))
     7   +dconjg((-ebppb*maZ(2,1)+el_m2/ebppb*mbZ(2,1)))*
     8    (-ebppb*ddmi(2,1)-el_m2/ebppb*ddpl(2,1))
     9   +dconjg((ebppb*mbZ(1,2)-el_m2/ebppb*maZ(1,2)))*
     1    (-ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))
     2     )/(4.d0*pi*alpha)**3

       chi_prod1=chi_prod1+2.d0*dreal((
     5    dconjg(dme*(mbZph(1,1)-maZph(1,1)))
     5     *dme*(-ddpl_ph(1,1)-ddmi_ph(1,1))
     6   +dconjg(dme*(mbZph(2,2)-maZph(2,2)))
     6     *dme*(-ddpl_ph(2,2)-ddmi_ph(2,2))
     7   +dconjg((-ebppb*maZph(2,1)+el_m2/ebppb*mbZph(2,1)))*
     8    (-ebppb*ddmi_ph(2,1)-el_m2/ebppb*ddpl_ph(2,1))
     9   +dconjg((ebppb*mbZph(1,2)-el_m2/ebppb*maZph(1,2)))*
     1 (-ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2)))
     2  )/(4.d0*pi*alpha)**3

       chi_prod1=chi_prod1+2.d0*dreal((
     5    dconjg(dme*(mbZ(1,1)-maZ(1,1)))
     5     *dme*(-ddpl_ph(1,1)-ddmi_ph(1,1))
     6   +dconjg(dme*(mbZ(2,2)-maZ(2,2)))
     6     *dme*(-ddpl_ph(2,2)-ddmi_ph(2,2))
     7   +dconjg((-ebppb*maZ(2,1)+el_m2/ebppb*mbZ(2,1)))*
     8    (-ebppb*ddmi_ph(2,1)-el_m2/ebppb*ddpl_ph(2,1))
     9   +dconjg((ebppb*mbZ(1,2)-el_m2/ebppb*maZ(1,2)))*
     1    (-ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2)))
     2     )/(4.d0*pi*alpha)**3

       chi_prod1=chi_prod1+2.d0*dreal((
     5    dconjg(dme*(mbZph(1,1)-maZph(1,1)))
     5     *dme*(-ddpl(1,1)-ddmi(1,1))
     6   +dconjg(dme*(mbZph(2,2)-maZph(2,2)))
     6     *dme*(-ddpl(2,2)-ddmi(2,2))
     7   +dconjg((-ebppb*maZph(2,1)+el_m2/ebppb*mbZph(2,1)))*
     8    (-ebppb*ddmi(2,1)-el_m2/ebppb*ddpl(2,1))
     9   +dconjg((ebppb*mbZph(1,2)-el_m2/ebppb*maZph(1,2)))*
     1 (-ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))
     2  )/(4.d0*pi*alpha)**3

c psi' interferences with Z
       chi_prod1=chi_prod1+2.d0*dreal((
     5    dconjg(dme*(mbZ(1,1)-maZ(1,1)))
     5     *dme*(-ddpl_p2s(1,1)-ddmi_p2s(1,1))
     6   +dconjg(dme*(mbZ(2,2)-maZ(2,2)))
     6     *dme*(-ddpl_p2s(2,2)-ddmi_p2s(2,2))
     7   +dconjg((-ebppb*maZ(2,1)+el_m2/ebppb*mbZ(2,1)))*
     8    (-ebppb*ddmi_p2s(2,1)-el_m2/ebppb*ddpl_p2s(2,1))
     9   +dconjg((ebppb*mbZ(1,2)-el_m2/ebppb*maZ(1,2)))*
     1    (-ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2)))
     2     )/(4.d0*pi*alpha)**3


      chi_prod1=chi_prod1+2.d0*dreal((
     5    dconjg(dme*(mbZph(1,1)-maZph(1,1)))
     5     *dme*(-ddpl_p2s(1,1)-ddmi_p2s(1,1))
     6   +dconjg(dme*(mbZph(2,2)-maZph(2,2)))
     6     *dme*(-ddpl_p2s(2,2)-ddmi_p2s(2,2))
     7   +dconjg((-ebppb*maZph(2,1)+el_m2/ebppb*mbZph(2,1)))*
     8    (-ebppb*ddmi_p2s(2,1)-el_m2/ebppb*ddpl_p2s(2,1))
     9   +dconjg((ebppb*mbZph(1,2)-el_m2/ebppb*maZph(1,2)))*
     1 (-ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2)))
     2  )/(4.d0*pi*alpha)**3
  
       endif
! interferences with radiative return

      if(chi_sw.eq.2)then
         chi_prod1=chi_prod1+2.d0*dreal((
     5    dconjg(dme*(mb(1,1)-ma(1,1)))
     5     *dme*(-ddpl(1,1)-ddmi(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))
     6     *dme*(-ddpl(2,2)-ddmi(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ddmi(2,1)-el_m2/ebppb*ddpl(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1    (-ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))*dconjg(cvac_qq)
     2     )/(sqrt(4.d0*pi*alpha))**3

       chi_prod1=chi_prod1+2.d0*dreal(
     5    dconjg(dme*(mb(1,1)-ma(1,1)))
     5     *dme*(-ddpl_ph(1,1)-ddmi_ph(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))
     6     *dme*(-ddpl_ph(2,2)-ddmi_ph(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ddmi_ph(2,1)-el_m2/ebppb*ddpl_ph(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1 (-ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2))*dconjg(cvac_qq)
     2  )/(sqrt(4.d0*pi*alpha))**3

c psi'
      chi_prod1=chi_prod1+2.d0*dreal(
     5    dconjg(dme*(mb(1,1)-ma(1,1)))
     5     *dme*(-ddpl_p2s(1,1)-ddmi_p2s(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))
     6     *dme*(-ddpl_p2s(2,2)-ddmi_p2s(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ddmi_p2s(2,1)-el_m2/ebppb*ddpl_p2s(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1 (-ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2))*dconjg(cvac_qq)
     2  )/(sqrt(4.d0*pi*alpha))**3
c

! interferences of Z with radiative return

      chi_prod1=chi_prod1+2.d0*dreal((
     1    dconjg(dme*(mbZ(1,1)-maZ(1,1)))
     2    *(dme*(mb(1,1)-ma(1,1)))
     3    +dconjg(dme*(mbZ(2,2)-maZ(2,2)))
     4    *(dme*(mb(2,2)-ma(2,2)))
     5    +dconjg(-ebppb*maZ(2,1)+el_m2/ebppb*mbZ(2,1))
     6    *(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))
     7    +(dconjg(ebppb*mbZ(1,2)-el_m2/ebppb*maZ(1,2)))
     8    *(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2))
     9    )*dconjg(cvac_qq))/(sqrt(4.d0*pi*alpha))**3

      chi_prod1=chi_prod1+2.d0*dreal((
     1    dconjg(dme*(mb(1,1)-ma(1,1)))
     2    *(dme*(mbZph(1,1)-maZph(1,1)))
     3    +dconjg(dme*(mb(2,2)-ma(2,2)))
     4    *(dme*(mbZph(2,2)-maZph(2,2)))
     5    +dconjg(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1))
     6    *(-ebppb*maZph(2,1)+el_m2/ebppb*mbZph(2,1))
     7    +(dconjg(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))
     8    *(ebppb*mbZph(1,2)-el_m2/ebppb*maZph(1,2))
     9    )*dconjg(cvac_qq))/(sqrt(4.d0*pi*alpha))**3
      
        endif    
         

         enddo
        enddo
         enddo
         
         

      return
      end
c-----------------------------------------------------------------------    
c----------------------tensor chi production----------------------------
       subroutine tensorchi(vec1,vec2,ten_chi)
       complex*16 vec2(4),ten_chi(4,4)
       real*8 vec1(4)
       integer ii,nn
         do ii=1,4
          ten_chi(ii,ii)=0.d0
         enddo
     
        do ii=1,4
          do nn=ii+1,4
           ten_chi(ii,nn)=vec2(ii)*vec1(nn)-vec2(nn)*vec1(ii)
           ten_chi(nn,ii)=-ten_chi(ii,nn)
          enddo
        enddo
        return
        end
c-------------------------two parts for chi production------------------
       subroutine block_chi(qq,ten1,ddpl,ddmi,ddpl_ph,ddmi_ph,rk1,gam,
     1 ddmi_Z,ddpl_Z,ddmi_Zph,ddpl_Zph,ddpl_p2s,ddmi_p2s)
       include 'phokhara_10.0.inc'
       complex*16 ten1(4,4),gam(4),jj_chi(4),jj_chi_pho(4),
     1 ddpl(2,2),ddmi(2,2),c_jp,pi_chi,const_chi,const_chi_pho,
     2 s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,ddpl_ph(2,2),ddmi_ph(2,2),
     3 ddmi_Z(2,2),ddpl_Z(2,2),jj_chiZ(4),const_chi_Z
     4 ,ddmi_Zph(2,2),ddpl_Zph(2,2),jj_chiZph(4),const_chi_Zph,
     5 ddpl_p2s(2,2),ddmi_p2s(2,2),c_p2s,c_p2s_prop,jj_chi_p2s(4),
     6 const_chi_p2s
       real*8 qq,rk1(4),jppho2,c_pho,d1
       integer rr,rr2,jj,beta,kl
       
       
        jppho2=(momenta(6,0)+momenta(7,0)-rk1(1))**2
        do kl=2,4
        jppho2=jppho2-(momenta(6,kl-1)+momenta(7,kl-1)-rk1(kl))**2
        enddo




        c_chi=16.d0*alpha*pi/sqrt(m2c)*ac    
     1 /(0.5d0**2*jppho2-m2c**2)**2

       c_chipsi=4.d0*sqrt(4.d0*pi*alpha)*aj/sqrt(m2c)/
     1 (0.5d0**2*jppho2-m2c**2)**2

       c_p2s=4.d0*sqrt(4.d0*pi*alpha)*apsi/sqrt(m2c)/
     1 (0.5d0**2*jppho2-m2c**2)**2
       


       c_jp=sqrt(3.d0/sqrt(qq)/alpha*gamjpee)/(qq-mjp**2+
     1 dcmplx(0.d0,1.d0)*mjp*gamjp)

       c_pho=1.d0/qq

      c_p2s_prop=sqrt(3.d0/sqrt(qq)/alpha*gamp2see)/(qq-mp2s**2+
     1 dcmplx(0.d0,1.d0)*mp2s*gamp2s)
        
         
 
         pi_chi=1.d0/(Sp-mchic1**2+
     1 dcmplx(0.d0,1.d0)*mchic1*gamchic1)
 
       const_chi=sqrt(4.d0*pi*alpha)*pi_chi*c_jp*c_chipsi
     1 *dcmplx(0.d0,-1.d0)/2.d0*gg1

       const_chi_p2s=sqrt(4.d0*pi*alpha)*pi_chi*c_p2s*c_p2s_prop
     1 *dcmplx(0.d0,-1.d0)/2.d0*gg1
       
       const_chi_pho=sqrt(4.d0*pi*alpha)*pi_chi*c_pho*c_chi
     1 *dcmplx(0.d0,-1.d0)/2.d0*gg1

       const_chi_Z=sqrt(4.d0*pi*alpha)*pi_chi*c_jp*c_chipsi
     1 *dcmplx(0.d0,-1.d0)/2.d0

       const_chi_Zph=sqrt(4.d0*pi*alpha)*pi_chi*c_pho*c_chi
     1 *dcmplx(0.d0,-1.d0)/2.d0
                         
       s1=-ten1(2,3)+ten1(3,2)
       s2=ten1(2,4)-ten1(4,2)
       s3=ten1(4,3)-ten1(3,4)
       s4=ten1(1,3)-ten1(3,1)
       s5=ten1(4,1)-ten1(1,4)
       s6=ten1(3,4)-ten1(4,3) 
       s7=ten1(2,1)-ten1(1,2)
       s8=-ten1(2,4)+ten1(4,2)
       s9=ten1(1,4)-ten1(4,1)
       s10=ten1(1,2)-ten1(2,1)
       s11=ten1(3,1)-ten1(1,3)
       s12=ten1(2,3)-ten1(3,2)
c******************** contribution from gamma J/psi
        jj_chi(1)=(s1*gam(4)+s2*gam(3)+s3*gam(2))*qq*const_chi
        jj_chi(2)=-(s4*gam(4)+s5*gam(3)+s6*gam(1))*qq*const_chi
        jj_chi(3)=-(s7*gam(4)+s8*gam(1)+s9*gam(2))*qq*const_chi
        jj_chi(4)=-(s10*gam(3)+s11*gam(2)+s12*gam(1))*qq*const_chi  
c************contribution from gamma gamma  
        jj_chi_pho(1)=(s1*gam(4)+s2*gam(3)+s3*gam(2))*qq*const_chi_pho
        jj_chi_pho(2)=-(s4*gam(4)+s5*gam(3)+s6*gam(1))*qq*const_chi_pho
        jj_chi_pho(3)=-(s7*gam(4)+s8*gam(1)+s9*gam(2))*qq*const_chi_pho
       jj_chi_pho(4)=-(s10*gam(3)+s11*gam(2)+s12*gam(1))*qq*
     & const_chi_pho 
 
        jj_chiZ(1)=(s1*gam(4)+s2*gam(3)+s3*gam(2))*const_chi_Z*qq
        jj_chiZ(2)=-(s4*gam(4)+s5*gam(3)+s6*gam(1))*const_chi_Z*qq
        jj_chiZ(3)=-(s7*gam(4)+s8*gam(1)+s9*gam(2))*const_chi_Z*qq
        jj_chiZ(4)=-(s10*gam(3)+s11*gam(2)+s12*gam(1))*const_chi_Z*qq

        jj_chiZph(1)=(s1*gam(4)+s2*gam(3)+s3*gam(2))*const_chi_Zph*qq
       jj_chiZph(2)=-(s4*gam(4)+s5*gam(3)+s6*gam(1))*const_chi_Zph*qq
       jj_chiZph(3)=-(s7*gam(4)+s8*gam(1)+s9*gam(2))*const_chi_Zph*qq
       jj_chiZph(4)=-(s10*gam(3)+s11*gam(2)+s12*gam(1))*const_chi_Zph*qq

       jj_chi_p2s(1)=(s1*gam(4)+s2*gam(3)+s3*gam(2))*qq*const_chi_p2s
        jj_chi_p2s(2)=-(s4*gam(4)+s5*gam(3)+s6*gam(1))*qq*const_chi_p2s
        jj_chi_p2s(3)=-(s7*gam(4)+s8*gam(1)+s9*gam(2))*qq*const_chi_p2s
      jj_chi_p2s(4)=-(s10*gam(3)+s11*gam(2)+s12*gam(1))*qq*const_chi_p2s  

      
       call cplus(jj_chi,ddpl)
       call cminus(jj_chi,ddmi) 

       call cplus(jj_chi_pho,ddpl_ph)
       call cminus(jj_chi_pho,ddmi_ph)

       call cplus(jj_chiZ,ddpl_Z)
       call cminus(jj_chiZ,ddmi_Z)

       call cplus(jj_chiZph,ddpl_Zph)
       call cminus(jj_chiZph,ddmi_Zph)

       call cplus(jj_chi_p2s,ddpl_p2s)
       call cminus(jj_chi_p2s,ddmi_p2s)
         
       
     
      return 
      end

c----------beam resolution--------------------------------------------------------------------------------
       subroutine beam_res
      use belle2_phokhara_interface
       include 'phokhara_10.0.inc'
       real*8 r1(0:1),r2,rr,phi,ebeam1,ebeam2,p1(4),p2(4),sm(4)
       real*8 dme,el_m2
       integer i
       common /cp1p2/p1,p2,dme,el_m2
       common/ssmm/sm
       
       call ranlxdf(r1,2)
       
           
           rr=dsqrt(-2.d0*beamres**2*dlog(r1(0)))
           phi=2.d0*pi*r1(1)
           ebeam1=rr*dcos(phi)+ebeam
           ebeam2=rr*dsin(phi)+ebeam    
           p1(1)=ebeam1
           p1(2)=0.d0
           p1(3)=0.d0
           p1(4)=dsqrt(ebeam1**2-el_m2)
           p2(1)=ebeam2
           p2(2)=0.d0
           p2(3)=0.d0
           p2(4)=-dsqrt(ebeam2**2-el_m2)
           Sp=(ebeam1+ebeam2)**2
          do i=1,4
             momenta(1,i-1) = p1(i)
             momenta(2,i-1) = p2(i)
          enddo
           do i=1,4
                sm(i)=p1(i)+p2(i)
          enddo
       end
c---------------chi_c2 production-----------------------------------------------------
      real*8 function chi_prod2(qq)
      include 'phokhara_10.0.inc' 
      real*8 qq, rk1(4),p1(4),p2(4),dme,el_m2,ebppb
      real*8 chi_spin2(4,4,4,4)
      complex*16 ma(2,2),mb(2,2),ma_ma(2,2),mb_ma(2,2),ddpl_ph(2,2),
     1 ddmi_ph(2,2),ddpl_p2s(2,2),ddmi_p2s(2,2)
      complex*16 up1(2,2),up2(2,2),v1(2,2),v2(2,2),uupp1(1,2),uupp2(1,2)
     1 ,vv1(2,1),vv2(2,1),eck1(4),epsk1(2,4),ten_chi(4,4),
     1 gammu(4,2,2),gammu_ma(4,2,2),gam(4),gam_ma(4),ddpl(2,2),
     1 ddmi(2,2)
       complex*16 cvac_qq,vacpol_and_nr
      integer ic1,ic2,i1,i,imu
        common/matri1/ma,mb,ma_ma,mb_ma
        common /cp1p2/p1,p2,dme,el_m2
        
        
         
              

              call prop_chic2(chi_spin2)

             chi_prod2=0.d0
             
            do i1=1,4
             rk1(i1) = momenta(3,i1-1)
             enddo
             ebppb=p1(1)+p1(4)
              call pol_vec(rk1,epsk1)    
              call skalar1LO(rk1)   
              
            call gam1(gam,gammu,gammu_ma,v1,v2,up1,up2,qq)            
           
           cvac_qq = vacpol_and_nr(qq)

            do ic1=1,2
             do ic2=1,2
c
c the spinors
c
         uupp2(1,1) = up2(1,ic1)
         uupp2(1,2) = up2(2,ic1)
         uupp1(1,1) = up1(1,ic1)
         uupp1(1,2) = up1(2,ic1)
    
         vv1(1,1) = v1(1,ic2)                 
         vv1(2,1) = v1(2,ic2)                 
         vv2(1,1) = v2(1,ic2)                 
         vv2(2,1) = v2(2,ic2)    


          do imu=1,4
           gam(imu) = gammu(imu,ic1,ic2)           
           gam_ma(imu) = gammu_ma(imu,ic1,ic2)
         enddo
         call skalar1aLO(gam,gam_ma)

c           sum over photon polarizations
c
          do i=1,2
c                  eck1(1)=rk1(1)
c                  eck1(2)=rk1(2)
c                  eck1(3)=rk1(3)
c                  eck1(4)=rk1(4)
                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4) 

         call skalar2LO(rk1,eck1) 
         call blocksLO(qq)
         call tensorchi(rk1,eck1,ten_chi)
         call block_chi2(qq,ten_chi,chi_spin2,ddpl,ddmi,ddpl_ph,
     1 ddmi_ph,rk1,gam,ddpl_p2s,ddmi_p2s)
            
          if((chi_sw.eq.0).or.(chi_sw.eq.2))then
          chi_prod2=chi_prod2+ ( (dme*cdabs(mb(1,1)-ma(1,1)))**2
     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
     4          *cdabs(cvac_qq)**2
          endif


       if((chi_sw.eq.1).or.(chi_sw.eq.2))then
        chi_prod2=chi_prod2+ ( (dme*cdabs(ddpl(1,1)-ddmi(1,1)))**2
     1         +(dme*cdabs(ddpl(2,2)-ddmi(2,2)))**2
     2         +(cdabs(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1)))**2
     3         +(cdabs(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))**2 )
     4    /(4.d0*pi*alpha)**3
      
      chi_prod2=chi_prod2+ ( (dme*cdabs(ddpl_ph(1,1)-ddmi_ph(1,1)))**2
     1      +(dme*cdabs(ddpl_ph(2,2)-ddmi_ph(2,2)))**2
     2      +(cdabs(-ebppb*ddmi_ph(2,1)+el_m2/ebppb*ddpl_ph(2,1)))**2
     3      +(cdabs(ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2)))**2 )
     4    /(4.d0*pi*alpha)**3


       chi_prod2=chi_prod2+2.d0*dreal(
     1    dconjg(dme*(ddpl(1,1)-ddmi(1,1)))
     2    *(dme*(ddpl_ph(1,1)-ddmi_ph(1,1)))
     3    +dconjg(dme*(ddpl(2,2)-ddmi(2,2)))
     4    *(dme*(ddpl_ph(2,2)-ddmi_ph(2,2)))
     5    +dconjg(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))
     6    *(-ebppb*ddmi_ph(2,1)+el_m2/ebppb*ddpl_ph(2,1))
     7    +(dconjg(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))
     8    *(ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2))
     9    )/(4.d0*pi*alpha)**3
c psi' contributions

       chi_prod2=chi_prod2+((dme*cdabs(ddpl_p2s(1,1)-ddmi_p2s(1,1)))**2
     1     +(dme*cdabs(ddpl_p2s(2,2)-ddmi_p2s(2,2)))**2
     2     +(cdabs(-ebppb*ddmi_p2s(2,1)+el_m2/ebppb*ddpl_p2s(2,1)))**2
     3     +(cdabs(ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2)))**2 )
     4    /(4.d0*pi*alpha)**3

       chi_prod2=chi_prod2+2.d0*dreal(
     1    dconjg(dme*(ddpl_p2s(1,1)-ddmi_p2s(1,1)))
     2    *(dme*(ddpl_ph(1,1)-ddmi_ph(1,1)))
     3    +dconjg(dme*(ddpl_p2s(2,2)-ddmi_p2s(2,2)))
     4    *(dme*(ddpl_ph(2,2)-ddmi_ph(2,2)))
     5    +dconjg(-ebppb*ddmi_p2s(2,1)+el_m2/ebppb*ddpl_p2s(2,1))
     6    *(-ebppb*ddmi_ph(2,1)+el_m2/ebppb*ddpl_ph(2,1))
     7    +(dconjg(ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2)))
     8    *(ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2))
     9    )/(4.d0*pi*alpha)**3


       chi_prod2=chi_prod2+2.d0*dreal(
     1    dconjg(dme*(ddpl_p2s(1,1)-ddmi_p2s(1,1)))
     2    *(dme*(ddpl(1,1)-ddmi(1,1)))
     3    +dconjg(dme*(ddpl_p2s(2,2)-ddmi_p2s(2,2)))
     4    *(dme*(ddpl(2,2)-ddmi(2,2)))
     5    +dconjg(-ebppb*ddmi_p2s(2,1)+el_m2/ebppb*ddpl_p2s(2,1))
     6    *(-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))
     7    +(dconjg(ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2)))
     8    *(ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2))
     9    )/(4.d0*pi*alpha)**3
c

       endif

      if(chi_sw.eq.2)then
         chi_prod2=chi_prod2+2.d0*dreal((
     5    dconjg(dme*(mb(1,1)-ma(1,1)))
     5     *dme*(ddpl(1,1)-ddmi(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))
     6     *dme*(ddpl(2,2)-ddmi(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ddmi(2,1)+el_m2/ebppb*ddpl(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1    (ebppb*ddpl(1,2)-el_m2/ebppb*ddmi(1,2)))*dconjg(cvac_qq)
     2   )/(sqrt(4.d0*pi*alpha))**3

       chi_prod2=chi_prod2+2.d0*dreal((
     5    dconjg(dme*(mb(1,1)-ma(1,1)))
     5     *dme*(ddpl_ph(1,1)-ddmi_ph(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))
     6     *dme*(ddpl_ph(2,2)-ddmi_ph(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ddmi_ph(2,1)+el_m2/ebppb*ddpl_ph(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1 (ebppb*ddpl_ph(1,2)-el_m2/ebppb*ddmi_ph(1,2)))*dconjg(cvac_qq)
     2  )/(sqrt(4.d0*pi*alpha))**3

c psi'
       chi_prod2=chi_prod2+2.d0*dreal((
     5    dconjg(dme*(mb(1,1)-ma(1,1)))
     5     *dme*(ddpl_p2s(1,1)-ddmi_p2s(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))
     6     *dme*(ddpl_p2s(2,2)-ddmi_p2s(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ddmi_p2s(2,1)+el_m2/ebppb*ddpl_p2s(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1 (ebppb*ddpl_p2s(1,2)-el_m2/ebppb*ddmi_p2s(1,2)))*dconjg(cvac_qq)
     2  )/(sqrt(4.d0*pi*alpha))**3
        endif    
         

         enddo
        enddo
         enddo

         
         

      return
      end
c*********************************************************************
       subroutine block_chi2(qq,ten1,chi_spin2,ddpl,ddmi,ddpl_ph,
     1  ddmi_ph,rk1,gam,ddpl_p2s,ddmi_p2s)
       include 'phokhara_10.0.inc'
       complex*16 ten1(4,4),gam(4),jj_chi(4),
     1 ddpl(2,2),ddmi(2,2),c_jp,pi_chi,const_chi,bl2(4,4,4),
     2 bb_chi(4,4),ddpl_ph(2,2),ddmi_ph(2,2),const_chi_pho,jj_chi_pho(4)
     3 ,bl2_ph(4,4,4),bb_chi_ph(4,4),ddpl_p2s(2,2),ddmi_p2s(2,2),
     4 c_p2s_prop,const_chi_p2s,jj_chi_p2s(4),bl2_p2s(4,4,4),
     5 bb_chi_p2s(4,4)
       real*8 qq,rk1(4),g_m(4,4),chi_spin2(4,4,4,4),ten_jp(4,4,4),
     1 p_jp(4),sm(4),bl1(4,4,4),jppho2,c_pho,ten_pho(4,4,4),d1,c_p2s,
     2 ten_p2s(4,4,4)
       integer rr,rr2,jj,beta,i,j,k,mu,nu,rho,sigma,gami,kl
       common/ssmm/sm

        do i=1,4
        do j=1,4
          g_m(i,j) =0.d0
        enddo
       enddo   
c
       g_m(1,1)=1.d0
       do i=1,3
        g_m(i+1,i+1)=-1.d0
       enddo
          
          do i=1,4
             p_jp(i)=momenta(6,i-1)+momenta(7,i-1)
            enddo
       
        do i=1,4
           do j=1,4
             do k=1,4
           ten_jp(i,j,k)=(-g_m(i,j)+p_jp(i)*p_jp(j)/mjp**2)*p_jp(k)-
     1   (-g_m(k,j)+p_jp(k)*p_jp(j)/mjp**2)*p_jp(i)

          ten_pho(i,j,k)=-g_m(i,j)*p_jp(k)+g_m(k,j)*p_jp(i)

           ten_p2s(i,j,k)=(-g_m(i,j)+p_jp(i)*p_jp(j)/mp2s**2)*p_jp(k)-
     1   (-g_m(k,j)+p_jp(k)*p_jp(j)/mp2s**2)*p_jp(i)
        enddo
          enddo
            enddo


          do mu=1,4
             do rho=1,4
                do sigma=1,4        
           bl1(mu,rho,sigma)=(momenta(1,0)-
     & momenta(2,0))*chi_spin2(mu,1,rho,sigma)-(momenta(1,1)-
     & momenta(2,1))*chi_spin2(mu,2,rho,sigma)-(momenta(1,2)-
     & momenta(2,2))*chi_spin2(mu,3,rho,sigma)-(momenta(1,3)-
     & momenta(2,3))*chi_spin2(mu,4,rho,sigma)
             enddo
                enddo
                    enddo         


          do rho=1,4
         do sigma=1,4
         do gami=1,4
           bl2(rho,sigma,gami)=ten1(1,rho)*ten_jp(sigma,gami,1)
     & -ten1(2,rho)*ten_jp(sigma,gami,2)
     & -ten1(3,rho)*ten_jp(sigma,gami,3)
     & -ten1(4,rho)*ten_jp(sigma,gami,4)
         enddo
         enddo
         enddo

      do rho=1,4
         do sigma=1,4
         do gami=1,4
           bl2_ph(rho,sigma,gami)=ten1(1,rho)*ten_pho(sigma,gami,1)
     & -ten1(2,rho)*ten_pho(sigma,gami,2)
     & -ten1(3,rho)*ten_pho(sigma,gami,3)
     & -ten1(4,rho)*ten_pho(sigma,gami,4)
         enddo
         enddo
         enddo


      do rho=1,4
         do sigma=1,4
         do gami=1,4
           bl2_p2s(rho,sigma,gami)=ten1(1,rho)*ten_p2s(sigma,gami,1)
     & -ten1(2,rho)*ten_p2s(sigma,gami,2)
     & -ten1(3,rho)*ten_p2s(sigma,gami,3)
     & -ten1(4,rho)*ten_p2s(sigma,gami,4)
         enddo
         enddo
         enddo
        

           
          

          do mu=1,4
            do gami=1,4
               
          bb_chi(mu,gami)=bl1(mu,1,1)*bl2(1,1,gami)-
     1 bl1(mu,1,2)*bl2(1,2,gami)
     2 -bl1(mu,1,3)*bl2(1,3,gami)-bl1(mu,1,4)*bl2(1,4,gami)
     3-bl1(mu,2,1)*bl2(2,1,gami)-bl1(mu,3,1)*bl2(3,1,gami)
     4 -bl1(mu,4,1)*bl2(4,1,gami)+bl1(mu,2,2)*bl2(2,2,gami)
     5+bl1(mu,2,3)*bl2(2,3,gami)+bl1(mu,2,4)*bl2(2,4,gami)
     6+bl1(mu,3,2)*bl2(3,2,gami)+bl1(mu,3,3)*bl2(3,3,gami)
     7+bl1(mu,3,4)*bl2(3,4,gami)+bl1(mu,4,2)*bl2(4,2,gami)
     8 +bl1(mu,4,3)*bl2(4,3,gami)+bl1(mu,4,4)*bl2(4,4,gami)
         enddo
           enddo

       do mu=1,4
            do gami=1,4
               
          bb_chi_ph(mu,gami)=bl1(mu,1,1)*bl2_ph(1,1,gami)-
     1 bl1(mu,1,2)*bl2_ph(1,2,gami)
     2 -bl1(mu,1,3)*bl2_ph(1,3,gami)-bl1(mu,1,4)*bl2_ph(1,4,gami)
     3-bl1(mu,2,1)*bl2_ph(2,1,gami)-bl1(mu,3,1)*bl2_ph(3,1,gami)
     4 -bl1(mu,4,1)*bl2_ph(4,1,gami)+bl1(mu,2,2)*bl2_ph(2,2,gami)
     5+bl1(mu,2,3)*bl2_ph(2,3,gami)+bl1(mu,2,4)*bl2_ph(2,4,gami)
     6+bl1(mu,3,2)*bl2_ph(3,2,gami)+bl1(mu,3,3)*bl2_ph(3,3,gami)
     7+bl1(mu,3,4)*bl2_ph(3,4,gami)+bl1(mu,4,2)*bl2_ph(4,2,gami)
     8 +bl1(mu,4,3)*bl2_ph(4,3,gami)+bl1(mu,4,4)*bl2_ph(4,4,gami)
         enddo
           enddo


      do mu=1,4
            do gami=1,4
               
          bb_chi_p2s(mu,gami)=bl1(mu,1,1)*bl2_p2s(1,1,gami)-
     1 bl1(mu,1,2)*bl2_p2s(1,2,gami)
     2 -bl1(mu,1,3)*bl2_p2s(1,3,gami)-bl1(mu,1,4)*bl2_p2s(1,4,gami)
     3-bl1(mu,2,1)*bl2_p2s(2,1,gami)-bl1(mu,3,1)*bl2_p2s(3,1,gami)
     4 -bl1(mu,4,1)*bl2_p2s(4,1,gami)+bl1(mu,2,2)*bl2_p2s(2,2,gami)
     5+bl1(mu,2,3)*bl2_p2s(2,3,gami)+bl1(mu,2,4)*bl2_p2s(2,4,gami)
     6+bl1(mu,3,2)*bl2_p2s(3,2,gami)+bl1(mu,3,3)*bl2_p2s(3,3,gami)
     7+bl1(mu,3,4)*bl2_p2s(3,4,gami)+bl1(mu,4,2)*bl2_p2s(4,2,gami)
     8 +bl1(mu,4,3)*bl2_p2s(4,3,gami)+bl1(mu,4,4)*bl2_p2s(4,4,gami)
         enddo
           enddo


        jppho2=(momenta(6,0)+momenta(7,0)-rk1(1))**2
        do kl=2,4
        jppho2=jppho2-(momenta(6,kl-1)+momenta(7,kl-1)-rk1(kl))**2
        enddo
           



          c_chi2=16.d0*alpha*pi/sqrt(m2c)*ac 
     1 /(0.5d0**2*jppho2-m2c**2)**2


       c_chi2psi=4.d0*sqrt(4.d0*pi*alpha)*aj/sqrt(m2c)/
     1 (0.5d0**2*jppho2-m2c**2)**2

       c_p2s=4.d0*sqrt(4.d0*pi*alpha)*apsi/sqrt(m2c)/
     1 (0.5d0**2*jppho2-m2c**2)**2

                
        
       c_jp=sqrt(3.d0/sqrt(qq)/alpha*gamjpee)/(qq-mjp**2+
     1 dcmplx(0.d0,1.d0)*mjp*gamjp)

       c_p2s_prop=sqrt(3.d0/sqrt(qq)/alpha*gamp2see)/(qq-mp2s**2+
     1 dcmplx(0.d0,1.d0)*mp2s*gamp2s)

       c_pho=1.d0/qq
       

 
        
          
         pi_chi=1.d0/(Sp-mchic2**2+
     1 dcmplx(0.d0,1.d0)*mchic2*gamchic2)
 
       const_chi=-sqrt(4.d0*pi*alpha)*pi_chi*c_jp*c_chi2psi
     1 *gg2*sqrt(2.d0)

       const_chi_pho=-sqrt(4.d0*pi*alpha)*pi_chi*c_pho*c_chi2
     1 *gg2*sqrt(2.d0)

      const_chi_p2s=sqrt(4.d0*pi*alpha)*pi_chi*c_p2s*c_p2s_prop
     1 *dcmplx(0.d0,-1.d0)/2.d0*gg2

        do mu=1,4
          jj_chi(mu)=0.d0
          jj_chi_pho(mu)=0.d0
          jj_chi_p2s(mu)=0.d0
          enddo
                         
        do mu=1,4
          jj_chi(mu)=bb_chi(mu,1)*gam(1)*const_chi
     1 -bb_chi(mu,2)*gam(2)*const_chi-bb_chi(mu,3)*gam(3)*const_chi
     2 -bb_chi(mu,4)*gam(4)*const_chi
            enddo
      do mu=1,4
          jj_chi_pho(mu)=bb_chi_ph(mu,1)*gam(1)*const_chi_pho
     1 -bb_chi_ph(mu,2)*gam(2)*const_chi_pho-
     3 bb_chi_ph(mu,3)*gam(3)*const_chi_pho
     2 -bb_chi_ph(mu,4)*gam(4)*const_chi_pho
            enddo

       do mu=1,4
          jj_chi_p2s(mu)=bb_chi_p2s(mu,1)*gam(1)*const_chi_p2s
     1 -bb_chi_p2s(mu,2)*gam(2)*const_chi_p2s-
     3 bb_chi_p2s(mu,3)*gam(3)*const_chi_p2s
     2 -bb_chi_p2s(mu,4)*gam(4)*const_chi_p2s
            enddo

      
       call cplus(jj_chi,ddpl)
       call cminus(jj_chi,ddmi) 
         
       call cplus(jj_chi_pho,ddpl_ph)
       call cminus(jj_chi_pho,ddmi_ph)

       call cplus(jj_chi_p2s,ddpl_p2s)
       call cminus(jj_chi_p2s,ddmi_p2s)
     
      return 
      end
c-------------------------------------------------------------------------------------------------------
       subroutine prop_chic2(chi_spin2)
       include 'phokhara_10.0.inc'
       real*8 vec1(4),chi_spin2(4,4,4,4),sm(4),Pc(4,4),g_m(4,4)
       integer mu,nu,rho,sigma,i,j
       common/ssmm/sm
       
        do i=1,4
        do j=1,4
          g_m(i,j) =0.d0
        enddo
       enddo   
c
       g_m(1,1)=1.d0
       do i=1,3
        g_m(i+1,i+1)=-1.d0
       enddo
       
        do i=1,4
          do j=1,4
           Pc(i,j)=-g_m(i,j)+sm(i)*sm(j)/mchic2**2
          enddo
            enddo
          
          do mu=1,4
            do nu=1,4
              do rho=1,4
                do sigma=1,4
           chi_spin2(mu,nu,rho,sigma)=0.5d0*(Pc(mu,rho)*Pc(nu,sigma)+
     &     Pc(mu,sigma)*Pc(nu,rho))-Pc(mu,nu)*Pc(rho,sigma)/3.d0


            enddo
              enddo
                enddo
                  enddo        
        return
        end
    
c---------------------------------------------------------------------
c            Function from loop integrals for chi1 and chi2
c---------------------------------------------------------------------
      complex*16 function adg_chi()
      include 'phokhara_10.0.inc'
      real*8 xx,yy
      complex*16 gg(2),cdilog,ff0,ff1,ff2,ff3,ff4,ff5,ii
      real*8 rr,aat,rr1,d1,d2,mpsi_p,gamjpee_p

c      gampsiee = 5.55d-6
c       mpsi_p=3.686109d0
c       gamjpee_p = 2.35d-6
       d1=dsqrt(gamjpee*3.d0/4.d0/pi/alpha**2/mjp)
       d2=dsqrt(gamp2see*3.d0/4.d0/pi/alpha**2/mp2s)
       ii=dcmplx(0.d0,1.d0)
       
      if(pion.eq.11)then

      xx = 4.d0*m2c**2/mchic1**2
      yy = 4.d0*mjp**2/mchic1**2
      rr = sqrt(xx -(1.d0-yy+xx)**2/4.d0)
      rr1 = sqrt(yy -(1.d0+yy-xx)**2/4.d0)
      
      
c
c g-g part
c
   
      gg(1) = alpha**2/mchic1**2*16.d0*ac/dsqrt(m2c)
     1  *(log(xx/(1.d0+xx))*(1.d0-xx)
     1 -(log(xx/(1.d0-xx))+dcmplx(0.d0,1.d0)*pi)
     2        *(1.d0+xx))

c
c g-J/psi part
c
      aat = atan((1.d0-yy+xx)/2.d0/rr)-atan((-1.d0-yy+xx)/2.d0/rr)
      ff0 = (1.d0+yy-xx)/4.d0*log(xx/yy)-rr*aat
      ff1 = log(xx/yy)+(1.d0+yy-xx)/rr*aat
      ff2 = 2.d0*log(2.d0)-xx*log(xx)+yy/2.d0*log(yy/2.d0) ! znak przy yy/2.d0*log(yy/2.d0)
     1 -(1.d0-xx)*(log(1.d0-xx)-dcmplx(0.d0,pi))
     2 +(2.d0-yy/2.d0)*(log(2.d0-yy/2.d0)-dcmplx(0.d0,pi))
     3 +(-1.d0-xx+yy)/2.d0*log(xx)+(-1.d0+xx-yy)/2.d0*log(yy)
     4 -2.d0*rr*aat
      ff3 = -3.d0/2.d0*log(xx)
     1 +log(1.d0-xx)-dcmplx(0.d0,1.d0)*pi+0.5d0*log(yy)
     2 -(1.d0-xx+yy)/2.d0/rr*aat
      ff4 =(log(1.d0-2.d0/yy)*log(yy/2.d0) 
     4 - cdilog(dcmplx(2.d0/yy,0.d0)) 
     5 +cdilog(dcmplx((1.d0-yy/2.d0)/(1.d0+xx-yy/2.d0),0.d0))
     6 -cdilog(dcmplx((-yy/2.d0)/(1.d0+xx-yy/2.d0),0.d0))
     7 -cdilog(dcmplx(1.d0-yy/2.d0,0.d0)/
     8         dcmplx((1.d0-xx)/2.d0,rr1))
     9 -cdilog(dcmplx(1.d0-yy/2.d0,0.d0)/
     1         dcmplx((1.d0-xx)/2.d0,-rr1))
     2 +cdilog(dcmplx(-yy/2.d0,0.d0)/
     3         dcmplx((1.d0-xx)/2.d0,rr1))
     4 +cdilog(dcmplx(-yy/2.d0,0.d0)/
     5         dcmplx((1.d0-xx)/2.d0,-rr1)))
      ff5 = (-1.d0/(1.d0+xx-yy/2.d0)*log((1.d0+xx)/xx)
     7 +dcmplx(-rr1,(1.d0+yy-xx)/2.d0)/rr1/dcmplx(1.d0-xx,2.d0*rr1)
     8*log(dcmplx((1.d0-xx+yy)/2.d0,rr1)/dcmplx((-1.d0-xx+yy)/2.d0,rr1))
     9 -dcmplx(rr1,(1.d0+yy-xx)/2.d0)/rr1/dcmplx(1.d0-xx,-2.d0*rr1)
     1*log(dcmplx((1.d0-xx+yy)/2.d0,-rr1)
     2/dcmplx((-1.d0-xx+yy)/2.d0,-rr1)) )

      gg(1)=gg(1)
     1 + 0.5d0*alpha**2/mchic1**2*16.d0/sqrt(m2c)
     2 *((log(xx/(1.d0-xx))+dcmplx(0.d0,1.d0)*pi)*(1.d0+xx-yy/2.d0)
     3 + ff0
     4 -(3.d0+xx+yy)/4.d0*ff1
     5 -yy*(4.d0+yy)/2.d0/(2.d0+2.d0*xx-yy)**2*ff2
     6 +yy*(1.d0+yy-xx)/2.d0/(2.d0+2.d0*xx-yy)*ff3
     7 -yy/2.d0*ff4
     8 +yy/2.d0*(3.d0-xx)*ff5)/2.d0/sqrt(pi*alpha)*d1*aj


c----------------------------------------------------------------------
c psi` contribution
c--------------------------------------------------------------------
      xx = 4.d0*m2c**2/mchic1**2
      yy = 4.d0*mp2s**2/mchic1**2
      
      rr = sqrt((1.d0-yy+xx)**2/4.d0-xx)
      rr1 = sqrt( (1.d0+yy-xx)**2/4.d0-xx)

          
      aat = 0.5d0/rr*(
     1 log( ((1.d0-yy+xx)/2.d0-rr)/((1.d0-yy+xx)/2.d0+rr) )
     2 -log( ((-1.d0-yy+xx)/2.d0-rr)/((-1.d0-yy+xx)/2.d0+rr) ) )
      ff0 = (1.d0+yy-xx)/4.d0*log(xx/yy)-rr*aat
      ff1 = log(xx/yy)+(1.d0+yy-xx)/rr*aat
      ff2 = 2.d0*log(2.d0)-xx*log(xx)+yy/2.d0*log(yy/2.d0) ! znak przy yy/2.d0*log(yy/2.d0)
     1 -(1.d0-xx)*(log(1.d0-xx)-dcmplx(0.d0,pi))
     2 +(2.d0-yy/2.d0)*(log(yy/2.d0-2.d0))
     3 +(-1.d0-xx+yy)/2.d0*log(xx)+(-1.d0+xx-yy)/2.d0*log(yy)
     4 -2.d0*rr*aat
      ff3 = -3.d0/2.d0*log(xx)
     1 +log(1.d0-xx)-dcmplx(0.d0,1.d0)*pi+0.5d0*log(yy)
     2 -(1.d0-xx+yy)/2.d0/rr*aat
      ff4 =(log(1.d0-2.d0/yy)*log(yy/2.d0) 
     4 - cdilog(dcmplx(2.d0/yy,0.d0)) 
     5 +cdilog(dcmplx((1.d0-yy/2.d0)/(1.d0+xx-yy/2.d0),0.d0))
     6 -cdilog(dcmplx((-yy/2.d0)/(1.d0+xx-yy/2.d0),0.d0))
     7 -cdilog(dcmplx(1.d0-yy/2.d0,0.d0)/
     8         dcmplx((1.d0-xx)/2.d0-rr1,0.d0))
     9 -cdilog(dcmplx(1.d0-yy/2.d0,0.d0)/
     1         dcmplx((1.d0-xx)/2.d0+rr1,0.d0))
     2 +cdilog(dcmplx(-yy/2.d0,0.d0)/
     3         dcmplx((1.d0-xx)/2.d0-rr1,0.d0))
     4 +cdilog(dcmplx(-yy/2.d0,0.d0)/
     5         dcmplx((1.d0-xx)/2.d0+rr1,0.d0)))
      ff5 = (-1.d0/(1.d0+xx-yy/2.d0)*log((1.d0+xx)/xx)
     7 +dcmplx(0.d0,-rr1+(1.d0+yy-xx)/2.d0)/ii
     8 /rr1/dcmplx(1.d0-xx-2.d0*rr1,0.d0)
     8*log(dcmplx((1.d0-xx+yy)/2.d0-rr1,0.d0)/
     9 dcmplx((-1.d0-xx+yy)/2.d0-rr1,0.d0))
     9 -dcmplx(0.d0,rr1+(1.d0+yy-xx)/2.d0)/ii
     1 /rr1/dcmplx(1.d0-xx+2.d0*rr1,0.d0)
     1*log(dcmplx((1.d0-xx+yy)/2.d0+rr1,0.d0)
     2/dcmplx((-1.d0-xx+yy)/2.d0+rr1,0.d0)) )

      
     

      gg(1)=gg(1)+0.5d0*alpha**2/mchic1**2*16.d0/sqrt(m2c)
     3 *((log(xx/(1.d0-xx))+dcmplx(0.d0,1.d0)*pi)*(1.d0+xx-yy/2.d0)
     4 + ff0
     5 -(3.d0+xx+yy)/4.d0*ff1
     6 -yy*(4.d0+yy)/2.d0/(2.d0+2.d0*xx-yy)**2*ff2
     9 +yy*(1.d0+yy-xx)/2.d0/(2.d0+2.d0*xx-yy)*ff3
     3 -yy/2.d0*ff4
     6 +yy/2.d0*(3.d0-xx)*ff5)/2.d0/sqrt(pi*alpha)*d2*apsi
     
       adg_chi=gg(1)

      

      elseif(pion.eq.12)then
c
      xx = 4.d0*m2c**2/mchic2**2
      yy = 4.d0*mjp**2/mchic2**2
      rr = sqrt(yy-(1.d0-yy+xx)**2/4.d0)
c
      rr = sqrt(xx -(1.d0-yy+xx)**2/4.d0)
      rr1 = sqrt(yy -(1.d0+yy-xx)**2/4.d0)  
c***********************************************************************
      aat = atan((1.d0-yy+xx)/2.d0/rr)-atan((-1.d0-yy+xx)/2.d0/rr)
      ff0 = (1.d0+yy-xx)/4.d0*log(xx/yy)-rr*aat
      ff1 = log(xx/yy)+(1.d0+yy-xx)/rr*aat
      ff2 = 2.d0*log(2.d0)-xx*log(xx)+yy/2.d0*log(yy/2.d0) ! znak przy yy/2.d0*log(yy/2.d0)
     1 -(1.d0-xx)*(log(1.d0-xx)-dcmplx(0.d0,pi))
     2 +(2.d0-yy/2.d0)*(log(2.d0-yy/2.d0)-dcmplx(0.d0,pi))
     3 +(-1.d0-xx+yy)/2.d0*log(xx)+(-1.d0+xx-yy)/2.d0*log(yy)
     4 -2.d0*rr*aat
      ff3 = -3.d0/2.d0*log(xx)
     1 +log(1.d0-xx)-dcmplx(0.d0,1.d0)*pi+0.5d0*log(yy)
     2 -(1.d0-xx+yy)/2.d0/rr*aat
      ff4 =(log(1.d0-2.d0/yy)*log(yy/2.d0) 
     4 - cdilog(dcmplx(2.d0/yy,0.d0)) 
     5 +cdilog(dcmplx((1.d0-yy/2.d0)/(1.d0+xx-yy/2.d0),0.d0))
     6 -cdilog(dcmplx((-yy/2.d0)/(1.d0+xx-yy/2.d0),0.d0))
     7 -cdilog(dcmplx(1.d0-yy/2.d0,0.d0)/
     8         dcmplx((1.d0-xx)/2.d0,rr1))
     9 -cdilog(dcmplx(1.d0-yy/2.d0,0.d0)/
     1         dcmplx((1.d0-xx)/2.d0,-rr1))
     2 +cdilog(dcmplx(-yy/2.d0,0.d0)/
     3         dcmplx((1.d0-xx)/2.d0,rr1))
     4 +cdilog(dcmplx(-yy/2.d0,0.d0)/
     5         dcmplx((1.d0-xx)/2.d0,-rr1)))
      ff5 = (-1.d0/(1.d0+xx-yy/2.d0)*log((1.d0+xx)/xx)
     7 +dcmplx(-rr1,(1.d0+yy-xx)/2.d0)/rr1/dcmplx(1.d0-xx,2.d0*rr1)
     8*log(dcmplx((1.d0-xx+yy)/2.d0,rr1)/dcmplx((-1.d0-xx+yy)/2.d0,rr1))
     9 -dcmplx(rr1,(1.d0+yy-xx)/2.d0)/rr1/dcmplx(1.d0-xx,-2.d0*rr1)
     1*log(dcmplx((1.d0-xx+yy)/2.d0,-rr1)
     2/dcmplx((-1.d0-xx+yy)/2.d0,-rr1)) )  



c*********************************************************************  

      

      gg(2) = alpha**2/mchic2**2*32.d0*ac/sqrt(m2c)*sqrt(2.d0)/3.d0
     1  *(log(1.d0-xx)*((1.d0+xx)/2.d0+8.d0/(1.d0+xx)**2)
     2   +log(1.d0+xx)*3.d0/2.d0*(1.d0+xx)
     3   -2.d0*log(xx)*(1.d0+xx+2.d0/(1.d0+xx)**2)
     4   -8.d0/(1.d0+xx)**2*log(2.d0)-1.d0
     5   -dcmplx(0.d0,1.d0)*pi/2.d0*(1.d0+xx+8.d0/(1.d0+xx)**2))
     
      
      gg(2)=gg(2)
     1 +0.5d0*alpha**2/mchic2**2*32.d0/sqrt(m2c)*sqrt(2.d0)/3.d0
     2    *(2.d0+log(2.d0)*(-3.d0+16d0/(1.d0+xx)**2) 
     3   + log(xx)*(1.d0-yy+2.d0*xx+8.d0/(1.d0+xx)**2)
     4 +log(1.d0-xx)*(0.5d0+yy-2.d0*xx-16d0/(1.d0+xx)**2)
     5 -3.d0*yy/8.d0*log(yy/4.d0)+log(1.d0-yy/4.d0)
     6 *(-1.5d0+3.d0*yy/8.d0)+dcmplx(0.d0,1.d0)*pi*
     7 (1.d0-11.d0*yy/8.d0+2.d0*xx+8.d0/(1.d0+xx)**2)
     8 -ff0-ff1*(0.5d0+yy-xx/4.d0)
     9 +(-55.d0-123.d0*xx*yy+126.d0*xx+93.d0*xx**2-94.d0*yy+38.d0*yy**2)
     1   /(2.d0+2.d0*xx-yy)**2/16.d0*ff2
     2 +(87.d0-5.d0*xx*yy-2.d0*yy+2.d0*yy**2+2.d0*xx+3.d0*xx**2)
     3   /(2.d0+2.d0*xx-yy)/2.d0*ff3-3.d0*yy/4.d0*ff4
     4  -3.d0*yy/4.d0*(1.d0+xx)*ff5)/2.d0/sqrt(pi*alpha)*d1*aj

c-----------------------------------------------------------------------
c  formulae for psi` contributions
c-----------------------------------------------------------------------     
      xx = 4.d0*m2c**2/mchic2**2
      yy = 4.d0*mp2s**2/mchic2**2

      rr = sqrt((1.d0-yy+xx)**2/4.d0-xx)
      rr1 = sqrt( (1.d0+yy-xx)**2/4.d0-xx)
     
      
      aat = 0.5d0/rr*(
     1 log( ((1.d0-yy+xx)/2.d0-rr)/((1.d0-yy+xx)/2.d0+rr) )
     2 -log( ((-1.d0-yy+xx)/2.d0-rr)/((-1.d0-yy+xx)/2.d0+rr) ) )
     

       aat = 0.5d0/rr*(
     1 log( ((1.d0-yy+xx)/2.d0-rr)/((1.d0-yy+xx)/2.d0+rr) )
     2 -log( ((-1.d0-yy+xx)/2.d0-rr)/((-1.d0-yy+xx)/2.d0+rr) ) )
      ff0 = (1.d0+yy-xx)/4.d0*log(xx/yy)-rr*aat
      ff1 = log(xx/yy)+(1.d0+yy-xx)/rr*aat
      ff2 = 2.d0*log(2.d0)-xx*log(xx)+yy/2.d0*log(yy/2.d0) ! znak przy yy/2.d0*log(yy/2.d0)
     1 -(1.d0-xx)*(log(1.d0-xx)-dcmplx(0.d0,pi))
     2 +(2.d0-yy/2.d0)*(log(yy/2.d0-2.d0))
     3 +(-1.d0-xx+yy)/2.d0*log(xx)+(-1.d0+xx-yy)/2.d0*log(yy)
     4 -2.d0*rr*aat
      ff3 = -3.d0/2.d0*log(xx)
     1 +log(1.d0-xx)-dcmplx(0.d0,1.d0)*pi+0.5d0*log(yy)
     2 -(1.d0-xx+yy)/2.d0/rr*aat
      ff4 =(log(1.d0-2.d0/yy)*log(yy/2.d0) 
     4 - cdilog(dcmplx(2.d0/yy,0.d0)) 
     5 +cdilog(dcmplx((1.d0-yy/2.d0)/(1.d0+xx-yy/2.d0),0.d0))
     6 -cdilog(dcmplx((-yy/2.d0)/(1.d0+xx-yy/2.d0),0.d0))
     7 -cdilog(dcmplx(1.d0-yy/2.d0,0.d0)/
     8         dcmplx((1.d0-xx)/2.d0-rr1,0.d0))
     9 -cdilog(dcmplx(1.d0-yy/2.d0,0.d0)/
     1         dcmplx((1.d0-xx)/2.d0+rr1,0.d0))
     2 +cdilog(dcmplx(-yy/2.d0,0.d0)/
     3         dcmplx((1.d0-xx)/2.d0-rr1,0.d0))
     4 +cdilog(dcmplx(-yy/2.d0,0.d0)/
     5         dcmplx((1.d0-xx)/2.d0+rr1,0.d0)))
      ff5 = (-1.d0/(1.d0+xx-yy/2.d0)*log((1.d0+xx)/xx)
     7 +dcmplx(0.d0,-rr1+(1.d0+yy-xx)/2.d0)/ii
     8 /rr1/dcmplx(1.d0-xx-2.d0*rr1,0.d0)
     8*log(dcmplx((1.d0-xx+yy)/2.d0-rr1,0.d0)/
     9 dcmplx((-1.d0-xx+yy)/2.d0-rr1,0.d0))
     9 -dcmplx(0.d0,rr1+(1.d0+yy-xx)/2.d0)/ii
     1 /rr1/dcmplx(1.d0-xx+2.d0*rr1,0.d0)
     1*log(dcmplx((1.d0-xx+yy)/2.d0+rr1,0.d0)
     2/dcmplx((-1.d0-xx+yy)/2.d0+rr1,0.d0)) )


      gg(2)=gg(2)+0.5d0*alpha**2/mchic2**2*32.d0/sqrt(m2c)
     & *sqrt(2.d0)/3.d0
     1    *(
     2     2.d0+log(2.d0)*(-3.d0+16d0/(1.d0+xx)**2) 
     3   + log(xx)*(1.d0-yy+2.d0*xx+8.d0/(1.d0+xx)**2)
     4 +log(1.d0-xx)*(0.5d0+yy-2.d0*xx-16d0/(1.d0+xx)**2)
     5 -3.d0*yy/8.d0*log(yy/4.d0)+(log(yy/4.d0-1.d0)+ii*pi)
     6 *(-1.5d0+3.d0*yy/8.d0)+dcmplx(0.d0,1.d0)*pi*
     7 (1.d0-11.d0*yy/8.d0+2.d0*xx+8.d0/(1.d0+xx)**2)
     8 -ff0-ff1*(0.5d0+yy-xx/4.d0)
     9 +(-55.d0-123.d0*xx*yy+126.d0*xx+93.d0*xx**2-94.d0*yy+38.d0*yy**2)
     1   /(2.d0+2.d0*xx-yy)**2/16.d0*ff2
     2 +(87.d0-5.d0*xx*yy-2.d0*yy+2.d0*yy**2+2.d0*xx+3.d0*xx**2)
     3   /(2.d0+2.d0*xx-yy)/2.d0*ff3-3.d0*yy/4.d0*ff4
     4  -3.d0*yy/4.d0*(1.d0+xx)*ff5)/2.d0/sqrt(pi*alpha)*d2*apsi

       adg_chi=gg(2)

      
       endif

      end

      
c******************************************************************
      real*8 function pi0gammaAmp(qq,Ar)
      include 'phokhara_10.0.inc' 
      real*8 qq,rk1(4),p1(4),p2(4),dme,el_m2,ebppb,rk2(4)
      real*8 qq2,qq1,cosphoton,q0p,q2p,q0b,q2b,amplit,matrix
      complex*16 ma(2,2),mb(2,2),ma_ma(2,2),mb_ma(2,2)
      complex*16 up1(2,2),up2(2,2),v1(2,2),v2(2,2),uupp1(1,2),uupp2(1,2)
     1 ,vv1(2,1),vv2(2,1),eck1(4),epsk1(2,4),ten_chi(4,4),
     1 gammu(4,2,2),gammu_ma(4,2,2),gam(4),gam_ma(4),ddpl(2,2),
     1 ddmi(2,2),ddpl_ph(2,2),ddmi_ph(2,2),ddpl_Z(2,2),ddmi_Z(2,2),
     2 ddmi_Zph(2,2),ddpl_Zph(2,2),mat_Z_vec,mat_Z_avec,
     3 ve1mi(2,2),ve2mi(2,2),ve1pl(2,2),ve2pl(2,2),ax1mi(2,2),
     4 ax2mi(2,2),ax1pl(2,2),ax2pl(2,2),vec_part,avec_part,
     5 mbZ(2,2),maZ(2,2),mbZph(2,2),maZph(2,2),
     6 ddpl_p2s(2,2),ddmi_p2s(2,2),eck2(4),epsk2(2,4),
     7 Leptonic(0:3,0:3),Hadronic(0:3,0:3)
      real*8 gamk1(4,4),gamk2(4,4)
      real*8 Ar(14)
       complex*16 cvac_qq,vacpol_and_nr
      integer ic1,ic2,i1,i,imu,i2
        common/matri1/ma,mb,ma_ma,mb_ma
        common /cp1p2/p1,p2,dme,el_m2
       common/qqvec12/qq1,qq2
       
        
                   
            
             pi0gammaAmp=0.d0
             
            do i1=1,4
             rk1(i1) = momenta(3,i1-1)
             rk2(i1) = momenta(7,i1-1)
             enddo
             ebppb=p1(1)+p1(4)
              call pol_vec(rk1,epsk1)  
              call pol_vec(rk2,epsk2)  
              call skalar1LOpi0(rk1,rk2)   
             
              
            call gampi0(gamk1,gamk2,rk1,rk2)            
           
           cvac_qq = vacpol_and_nr(qq)

c           sum over photon polarizations
c
          do i=1,2

                  eck1(1)=epsk1(i,1)
                  eck1(2)=epsk1(i,2)
                  eck1(3)=epsk1(i,3)
                  eck1(4)=epsk1(i,4)
c                  eck1(1)=dcmplx(rk1(1),0.d0)
c                  eck1(2)=dcmplx(rk1(2),0.d0)
c                  eck1(3)=dcmplx(rk1(3),0.d0)
c                  eck1(4)=dcmplx(rk1(4),0.d0)

           do i2=1,2
                  eck2(1)=epsk2(i2,1)
                  eck2(2)=epsk2(i2,2)
                  eck2(3)=epsk2(i2,3)
                  eck2(4)=epsk2(i2,4) 
c                  eck2(1)=dcmplx(rk2(1),0.d0)
c                  eck2(2)=dcmplx(rk2(2),0.d0)
c                  eck2(3)=dcmplx(rk2(3),0.d0)
c                  eck2(4)=dcmplx(rk2(4),0.d0)


         call skalar1aLOpi0(gamk1,gamk2,eck1,eck2)
         call skalar2LOpi0(rk1,eck1,rk2,eck2) 
         call blocksLO(qq)

     

           
     
! 
          pi0gammaAmp=pi0gammaAmp+ ((dme*cdabs(mb(1,1)-ma(1,1)))**2
     1          +(dme*cdabs(mb(2,2)-ma(2,2)))**2
     2          +(cdabs(-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))**2
     3          +(cdabs(ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))**2 )
     4          *cdabs(cvac_qq)**2 

        pi0gammaAmp=pi0gammaAmp+((dme*cdabs(mb_ma(1,1)-ma_ma(1,1)))**2
     1          +(dme*cdabs(mb_ma(2,2)-ma_ma(2,2)))**2
     2          +(cdabs(-ebppb*ma_ma(2,1)+el_m2/ebppb*mb_ma(2,1)))**2
     3          +(cdabs(ebppb*mb_ma(1,2)-el_m2/ebppb*ma_ma(1,2)))**2 )
     4          *cdabs(cvac_qq)**2


       pi0gammaAmp=pi0gammaAmp+2.d0*dreal((
     5    dconjg(dme*(mb(1,1)-ma(1,1)))*dme*(mb_ma(1,1)-ma_ma(1,1))
     6   +dconjg(dme*(mb(2,2)-ma(2,2)))*dme*(mb_ma(2,2)-ma_ma(2,2))
     7   +dconjg((-ebppb*ma(2,1)+el_m2/ebppb*mb(2,1)))*
     8    (-ebppb*ma_ma(2,1)+el_m2/ebppb*mb_ma(2,1))
     9   +dconjg((ebppb*mb(1,2)-el_m2/ebppb*ma(1,2)))*
     1    (ebppb*mb_ma(1,2)-el_m2/ebppb*ma_ma(1,2))))
     2   * cdabs(cvac_qq)**2
         

         enddo
         enddo
  
      return
      end
      real*8 function metric2(mu,nu)
      implicit none
      integer mu,nu
      if (mu.ne.nu) then
         metric2 = 0.d0
      else
         if (mu.eq.1) then
             metric2 = 1.d0
         else
             metric2 = -1.d0
         endif
      endif
      end

      subroutine gampi0(gamk1,gamk2,rk1,rk2)
      include 'phokhara_10.0.inc'
      real*8 gamk1(4,4),gamk2(4,4)
      real*8 k1(4),k2(4),qqvec1(4),qqvec2(4),metric2,qq1,qq2,rk1(4),
     1 rk2(4),qq
      integer be,nu,bnu,ii
      common/qqvec12/qq1,qq2

      do ii=1,4
      k1(ii)=rk1(ii)
      k2(ii)=rk2(ii)
      qqvec1(ii)=momenta(6,ii-1)+k2(ii)
      qqvec2(ii)=momenta(6,ii-1)+k1(ii)
      enddo

      qq1=qqvec1(1)**2
      qq2=qqvec2(1)**2
       do ii=2,4
         qq1=qq1-qqvec1(ii)**2
         qq2=qq2-qqvec2(ii)**2
       enddo

        do nu=1,4
         do be=1,4
       gamk1(nu,be) =
     & metric2(1,nu)*metric2(2,be)*k1(3)*qqvec2(4) - metric2(1,nu)*
     & metric2(2,be)*k1(4)*qqvec2(3) - metric2(1,nu)*metric2(3,be)*
     & k1(2)*qqvec2(4) + metric2(1,nu)*metric2(3,be)*k1(4)*qqvec2(2) + 
     & metric2(1,nu)*metric2(4,be)*k1(2)*qqvec2(3) - metric2(1,nu)*
     & metric2(4,be)*k1(3)*qqvec2(2) - metric2(1,be)*metric2(2,nu)*
     & k1(3)*qqvec2(4) + metric2(1,be)*metric2(2,nu)*k1(4)*qqvec2(3) + 
     & metric2(1,be)*metric2(3,nu)*k1(2)*qqvec2(4) - metric2(1,be)*
     & metric2(3,nu)*k1(4)*qqvec2(2) - metric2(1,be)*metric2(4,nu)*
     & k1(2)*qqvec2(3) + metric2(1,be)*metric2(4,nu)*k1(3)*qqvec2(2) + 
     & metric2(2,nu)*metric2(3,be)*k1(1)*qqvec2(4) - metric2(2,nu)*
     & metric2(3,be)*k1(4)*qqvec2(1) - metric2(2,nu)*metric2(4,be)*
     & k1(1)*qqvec2(3) + metric2(2,nu)*metric2(4,be)*k1(3)*qqvec2(1) - 
     & metric2(2,be)*metric2(3,nu)*k1(1)*qqvec2(4) + metric2(2,be)*
     & metric2(3,nu)*k1(4)*qqvec2(1) + metric2(2,be)*metric2(4,nu)*
     & k1(1)*qqvec2(3)
      gamk1(nu,be) = gamk1(nu,be)
     &  - metric2(2,be)*metric2(4,nu)*k1(3)*qqvec2(1) + 
     & metric2(3,nu)*metric2(4,be)*k1(1)*qqvec2(2) - metric2(3,nu)*
     & metric2(4,be)*k1(2)*qqvec2(1) - metric2(3,be)*metric2(4,nu)*
     & k1(1)*qqvec2(2) + metric2(3,be)*metric2(4,nu)*k1(2)*qqvec2(1)

      gamk2(nu,be) =
     & metric2(1,nu)*metric2(2,be)*k2(3)*qqvec1(4) - metric2(1,nu)*
     & metric2(2,be)*k2(4)*qqvec1(3) - metric2(1,nu)*metric2(3,be)*
     & k2(2)*qqvec1(4) + metric2(1,nu)*metric2(3,be)*k2(4)*qqvec1(2) + 
     & metric2(1,nu)*metric2(4,be)*k2(2)*qqvec1(3) - metric2(1,nu)*
     & metric2(4,be)*k2(3)*qqvec1(2) - metric2(1,be)*metric2(2,nu)*
     & k2(3)*qqvec1(4) + metric2(1,be)*metric2(2,nu)*k2(4)*qqvec1(3) + 
     & metric2(1,be)*metric2(3,nu)*k2(2)*qqvec1(4) - metric2(1,be)*
     & metric2(3,nu)*k2(4)*qqvec1(2) - metric2(1,be)*metric2(4,nu)*
     & k2(2)*qqvec1(3) + metric2(1,be)*metric2(4,nu)*k2(3)*qqvec1(2) + 
     & metric2(2,nu)*metric2(3,be)*k2(1)*qqvec1(4) - metric2(2,nu)*
     & metric2(3,be)*k2(4)*qqvec1(1) - metric2(2,nu)*metric2(4,be)*
     & k2(1)*qqvec1(3) + metric2(2,nu)*metric2(4,be)*k2(3)*qqvec1(1) - 
     & metric2(2,be)*metric2(3,nu)*k2(1)*qqvec1(4) + metric2(2,be)*
     & metric2(3,nu)*k2(4)*qqvec1(1) + metric2(2,be)*metric2(4,nu)*
     & k2(1)*qqvec1(3)
      gamk2(nu,be) = gamk2(nu,be)
     &  - metric2(2,be)*metric2(4,nu)*k2(3)*qqvec1(1) + 
     & metric2(3,nu)*metric2(4,be)*k2(1)*qqvec1(2) - metric2(3,nu)*
     & metric2(4,be)*k2(2)*qqvec1(1) - metric2(3,be)*metric2(4,nu)*
     & k2(1)*qqvec1(2) + metric2(3,be)*metric2(4,nu)*k2(2)*qqvec1(1)

         enddo
      enddo

      end
c ========================================================================
c === PHOKHARA 10.0, (c) October 2020       ================================
c ========================================================================

















