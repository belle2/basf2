*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  BornV                                  //
*//                                                                          //
*//  Purpose:                                                                //
*//  Provide Born angular distribution and integrated x-section              //
*//  as a function of s.                                                     //
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
*  Class members:
*
*//////////////////////////////////////////////////////////////////////////////
      DOUBLE PRECISION   m_pi
      PARAMETER         (m_pi =3.1415926535897932d0)
      DOUBLE PRECISION   m_fleps
****  PARAMETER (m_fleps = 1d-35)  ! original
****  PARAMETER (m_fleps = 1d-45)  ! enough???
      PARAMETER (m_fleps = 1d-100)  ! enough!!!
****      PARAMETER (m_fleps = 1d-200)  ! enough!!!
*//////////////////////////////////////////////////////////////////////////////
*//       Energy limits in the EW grid, w=sqrt(s) in GeV units.              //
*//////////////////////////////////////////////////////////////////////////////
*     340-point grid, only 80pt for NLC to be improved/tested in future
      INTEGER           m_poin1, m_poin2, m_poin3, m_poin4, m_poinG , m_poinQ !
      INTEGER           m_poTh1, m_poTh2, m_poTh3, m_poTh4
      PARAMETER(        m_poinG =   7 )                      ! No of EW formfactors
      PARAMETER(        m_poinQ =   4 )                      ! No of QCD corrections
*----------- Low energies and LEP1
      PARAMETER(        m_poin1 = 100 )                              ! LEP1 LOG(SQRT(s)) spacing
      PARAMETER(        m_poTh1 =   0 )                              ! Cost(heta) spacing
      DOUBLE PRECISION  m_WminLEP1,          m_WmaxLEP1              ! LEP1 basic range (m_WminLEP1,m_WmaxLEP1)
      PARAMETER(        m_WminLEP1=0.010d0,  m_WmaxLEP1= 95.000d0 )  ! LEP1 basic range (m_WminLEP1,m_WmaxLEP1)
***   PARAMETER(        m_WminLEP1=0.010d0,  m_WmaxLEP1=100.001d0 )  ! LEP1 basic range (m_WminLEP1,m_WmaxLEP1)
*----------- Z resonance
      PARAMETER(        m_poin2 =  20 )                              ! Z range sqrt(s)    spacing
***   PARAMETER(        m_poTh2 =   0 )                              ! Cost(heta) spacing
      PARAMETER(        m_poTh2 =  14 )                              ! =14 is overkill?
      DOUBLE PRECISION  m_WminZ, m_WmaxZ, m_WdelZ                    ! Z range (amz + m_WdelZ)
      PARAMETER(        m_WdelZ = 5.000d0)                           ! Z range (amz + m_WdelZ)
***   PARAMETER(        m_WdelZ = 2d0 *2.495342704946d0)             ! Old range 2*Gamma
*----------- LEP2
      PARAMETER(        m_poTh3 =  30 )                              ! Overkill, bit lets kkep it
***   PARAMETER(        m_poTh3 =  14 )                              ! Cost(heta) spacing
      PARAMETER(        m_poin3 = 145 )                              ! LEP2 interval sqrt(s)    spacing
***   PARAMETER(        m_poin3 = 140 )                              ! as in KORALZ, also enough
      DOUBLE PRECISION  m_WmaxLEP2                                   ! LEP2 interval (m_WmaxLEP1,m_WmaxLEP2)
      PARAMETER(        m_WmaxLEP2  =240.001d0 )                     ! LEP2 interval (m_WmaxLEP1,m_WmaxLEP2)
*----------- Linear Colliders
      PARAMETER(        m_poin4 =  80 )                              ! NLC range sqrt(s)    spacing
      PARAMETER(        m_poTh4 =  14 )                              ! Cost(heta) spacing
      DOUBLE PRECISION  m_WmaxNLC                                    ! NLC range (m_WmaxLEP2,m_WmaxNLC)
      PARAMETER(        m_WmaxNLC  =1040.001d0 )                     ! NLC range (m_WmaxLEP2,m_WmaxNLC)
*//////////////////////////////////////////////////////////////////////////////
* EW formfactors, all flavours!!
      DOUBLE COMPLEX     m_cyy,     m_czz,     m_ctt,    m_clc   ! Electroweak FFactors
      DOUBLE PRECISION   m_syy,     m_szz,     m_stt,    m_slc   ! QCD corr.
      DOUBLE COMPLEX     m_GSW
      DOUBLE PRECISION   m_QCDcor   ! obsolete
      DOUBLE PRECISION   m_QCDcorR
*//////////////////////////////////////////////////////////////////////////////
* EW parameters
      DOUBLE PRECISION   m_Gmu
      DOUBLE PRECISION   m_MZ,      m_amh,     m_amtop
      DOUBLE PRECISION   m_swsq,    m_gammz,   m_MW,    m_GammW
*
      DOUBLE PRECISION   m_CMSene,  m_XXXene,  m_HadMin, m_vvmin,  m_vvmax !
      DOUBLE PRECISION   m_AvMult,  m_YFSkon,  m_YFS_IR, m_alfinv, m_alfpi, m_Xenph !
      DOUBLE PRECISION   m_vv,      m_x1,      m_x2
      DOUBLE PRECISION   m_Qf,      m_T3f,     m_helic,  m_amferm, m_auxpar !
      DOUBLE PRECISION   m_gnanob
      INTEGER            m_IsGenerated, m_KFferm,  m_NCf
      INTEGER            m_KFini,       m_KeyINT,  m_KeyQCD,   m_KeyRes
      INTEGER            m_KeyElw,      m_KeyZet,  m_KeyWtm
      INTEGER            m_out

      COMMON /c_BornV/
* Tables of EW formfactors
     $  m_cyy(m_poin1+1,          m_poinG,16), ! formfactor, table
     $  m_czz(m_poin2+1,m_poTh2+1,m_poinG,16), ! formfactor, table
     $  m_ctt(m_poin3+1,m_poTh3+1,m_poinG,16), ! formfactor, table, costheta dependent
     $  m_clc(m_poin4+1,m_poTh4+1,m_poinG,16), ! formfactor, table, costheta dependent
     $  m_syy(m_poin1+1,          m_poinQ,16), ! QCD correction,
     $  m_szz(m_poin2+1,          m_poinQ,16), ! QCD correction,
     $  m_stt(m_poin3+1,          m_poinQ,16), ! QCD correction,
     $  m_slc(m_poin3+1,          m_poinQ,16), ! QCD correction,
     $  m_GSW(    m_poinG),                    ! form-factors,   at the actual energy/angle
     $  m_QCDcorR(m_poinQ),                    ! QCD correction, at the actual energy/angle
     $  m_QCDcor,                              ! obsolete!!!!
*
     $  m_CMSene,                       ! Initial value of CMS energy
     $  m_XXXene,                       ! CMS energy after beamsstrahlung or beam spread
* -------------------- EVENT --------------------------
     $  m_x1,                           ! 1-z1 = x1 for first  beam(strahlung)
     $  m_x2,                           ! 1-z2 = x2 for second beam(strahlung)
     $  m_vv,                           ! v = 1-sprim/s
     $  m_AvMult,                       ! Average photon multiplicity CRude at given v
     $  m_YFSkon,                       ! YFS formfactor finite part
     $  m_YFS_IR,                       ! YFS formfactor IR part
* -----------------------------------------------------
     $  m_vvmin,                        ! minimum v, infrared cut
     $  m_vvmax,                        ! maximum v
     $  m_HadMin,                       ! minimum hadronization mass [GeV]
* Basic QED and QCD
     $  m_alfinv,                       ! 1/alphaQED, Thomson limit (Q^2=0)
     $  m_alfpi,                        ! alphaQED/pi
     $  m_Xenph,                        ! Enhancement factor for Crude photon multiplicity
* EW parameters
     $  m_MZ,                           ! Z mass
     $  m_amh,                          ! Higgs mass
     $  m_amtop,                        ! Top mass
     $  m_swsq,                         ! sin(thetaW)**2
     $  m_gammz,                        ! Z width
     $  m_MW,                           ! W mass
     $  m_GammW,                        ! W width
     $  m_Gmu,                          ! Fermi constant (from muon decay)
* Table of fermion paramerets, quarks (1->6) and leptons (11->16)
     $  m_KFferm(20),                   ! fermion KFcode (1->6) and (11->16)
     $  m_NCf(20),                      ! number of colours
     $  m_Qf(20),                       ! electric charge
     $  m_T3f(20),                      ! isospin, L-hand component
     $  m_helic(20),                    ! helicity or polarization
     $  m_amferm(20),                   ! fermion mass
     $  m_auxpar(20),                   ! auxiliary parameter
     $  m_IsGenerated(20),              ! Generation flag, only for SAN !!! 
* Normalization
     $  m_gnanob,                       ! GeV^(-2) to nanobarns
* Initial/final fermion types
     $  m_KFini,                        ! KF code of beam
* Test switches
     $  m_KeyQCD,                       ! QCD FSR corr. switch
     $  m_KeyINT,                       ! ISR/FSR INTereference switch
     $  m_KeyElw,                       ! Type of Electrowak Library
     $  m_KeyZet,                       ! Z-boson on/off
     $  m_KeyWtm,                       ! Photon emission without mass terms
     $  m_KeyRes,                       ! experim. R for gamma* decays switch
     $  m_out                           ! output unit for printouts
      SAVE /c_BornV/
*
* Formats for writing EW tables onto disk file.
      CHARACTER*80  m_fmt0, m_fmt1, m_fmt2
      PARAMETER (
     $  m_fmt0 ='(4g20.13)',                      ! Mz,Mt,Mh etc.
     $  m_fmt1 ='( a,  i4,  f10.5, i4,  f10.5 )', ! header
     $  m_fmt2 ='(6g13.7)'   )                    ! complex formfactors
*
*  Class procedures:
*
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  BornV                                 //
*//////////////////////////////////////////////////////////////////////////////
