*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  KK2f                                   //
*//                                                                          //
*//   Purpose: KK2f generator, top level class                               //
*//                                                                          //
*//                                                                          //
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
*  Class members:
*
*////////////////////////////////////////////////////////////////////////////////////////
      DOUBLE PRECISION    m_version
      CHARACTER*14        m_Date
      PARAMETER ( m_Version     =          4.19d0 )
      PARAMETER ( m_Date        =  ' 25 Sept  2002')
*////////////////////////////////////////////////////////////////////////////////////////
      INTEGER     m_phmax             ! maximum photon multiplicity ISR+FSR
      PARAMETER ( m_phmax = 100)
      INTEGER     m_jlim
      PARAMETER(  m_jlim  = 10000)
      INTEGER     m_lenwt             ! maximum number of auxiliary weights in WtSet
      PARAMETER ( m_lenwt = 1000)
*////////////////////////////////////////////////////////////////////////////////////////
      DOUBLE PRECISION       m_xpar,   m_ypar
      INTEGER            m_out,    m_Idyfs,  m_nevgen, m_idgen,  m_KeyWgt
      INTEGER            m_npmax,  m_idbra,  m_nphot
      INTEGER            m_KeyHad, m_KeyISR, m_KeyFSR, m_KeyINT, m_KeyGPS
      INTEGER            m_Phel,   m_isr,    m_KFini,  m_IsBeamPolarized
      DOUBLE PRECISION       m_CMSene, m_DelEne, m_Xcrunb, m_WTmax,  m_HadMin, m_MasPhot,  m_Emin
      DOUBLE PRECISION       m_BornCru,m_WtCrud, m_WtMain, m_sphot
      DOUBLE PRECISION       m_WtSet,  m_WtList, m_alfinv, m_vcut,   m_Xenph
      DOUBLE PRECISION       m_p1,     m_p2,     m_q1,     m_q2
      DOUBLE PRECISION       m_PolBeam1,         m_PolBeam2
      DOUBLE PRECISION       m_xSecPb, m_xErrPb
*
      COMMON /c_KK2f/
     $    m_CMSene,           ! CMS energy average
     $    m_PolBeam1(4),      ! Spin Polarization vector first beam
     $    m_PolBeam2(4),      ! Spin Polarization vector second beam
     $    m_DelEne,           ! Beam energy spread [GeV]
     $    m_HadMin,           ! minimum hadronization energy [GeV]
     $    m_xpar(m_jlim),     ! input parameters, READ ONLY, never touch them!!!!
     $    m_ypar(m_jlim),     ! debug facility
     $    m_WTmax,            ! maximum weight
     $    m_alfinv,           ! inverse alfaQED
     $    m_Emin,             ! minimum real photon energy, IR regulator
     $    m_MasPhot,          ! ficticious photon mass,     IR regulator
     $    m_Xenph,            ! Enhancement factor for Crude photon multiplicity
     $    m_vcut(3),          ! technical cut on E/Ebeam for non-IR real contributions
     $    m_xSecPb,           ! output cross-section available through getter
     $    m_xErrPb,           ! output crossxsection available through getter
*----------- EVENT-----------------
     $    m_p1(4),            ! e- beam
     $    m_p2(4),            ! e+ beam
     $    m_q1(4),            ! final fermion
     $    m_q2(4),            ! final anti-fermion
     $    m_sphot(m_phmax,4), ! photon momenta
     $    m_WtMain,           ! MAIN weight of KK2f
     $    m_WtCrud,           ! crude distr from ISR and FSR
     $    m_WtSet(m_lenwt),   ! complete list of weights
     $    m_WtList(m_lenwt),  ! complete list of weights
     $    m_Xcrunb,           ! crude in nanobarns
     $    m_BornCru,          ! Crude Born
     $    m_isr( m_phmax),    ! =1 for isr, 0 for fsr
     $    m_Phel(m_phmax),    ! photon helicity =1,0 for +,-
     $    m_nphot,            ! Total Photon multiplicity
*----------------------------------
     $    m_IsBeamPolarized,  ! status of beam polarization
     $    m_KFini,            ! KF of beam fermion
     $    m_KeyWgt,           ! weight=1 on/off switch
     $    m_KeyHad,           ! hadronization switch
     $    m_nevgen,           ! serial number of the event
     $    m_npmax,            ! maximum photon multiplicity
     $    m_KeyISR,           ! ISR switch
     $    m_KeyFSR,           ! FSR switch
     $    m_KeyINT,           ! ISR/FSR INTereference switch
     $    m_KeyGPS,           ! New exponentiation switch
     $    m_out,              ! output unit number
     $    m_Idyfs,            ! pointer for histograming
     $    m_idbra,            ! pointer for brancher
     $    m_idgen             ! special histogram for this generator
*
      SAVE /c_KK2f/
*
*
*  Class procedures:
*
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  KK2f                                  //
*//////////////////////////////////////////////////////////////////////////////
