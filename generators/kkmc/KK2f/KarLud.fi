*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  KarLud                                 //
*//                                                                          //
*//   Purpose:                                                               //
*//   Top level  Monte-Carlo event generator for ISR radiadion.              //
*//   Administrates directly generation of v-variable                        //
*//   and indirectly of all other variables.                                 //
*//////////////////////////////////////////////////////////////////////////////
*  Class members:
      INTEGER     m_npmx      ! maximum photon multiplicity, see also m_nmax
      PARAMETER ( m_npmx = 100)
      DOUBLE PRECISION     m_CMSene,  m_XXXene,  m_DelEne,   m_exe,    m_amel
      DOUBLE PRECISION     m_vv,      m_x1,      m_x2   
      DOUBLE PRECISION     m_vvmin,   m_vvmax,   m_HadMin,   m_alfinv
      DOUBLE PRECISION     m_XCrude,  m_xcgaus,  m_ErGaus,   m_Xenph
      INTEGER    m_KeyZet, m_KeyISR,  m_MltISR,  m_KeyFix,   m_KeyWtm
      INTEGER    m_nmax,   m_out,     m_NevGen,  m_idyfs
      DOUBLE PRECISION     m_sphot,   m_sphum,   m_PX,       m_yini,   m_zini
      DOUBLE PRECISION     m_p1,      m_p2,      m_q1,       m_q2
      DOUBLE PRECISION     m_WtBasic, m_Wt_KF
      DOUBLE PRECISION     m_WtIni,   m_WtMass,  m_WtDil,    m_WtCut
      INTEGER              m_nphot,   m_KFfin
*
      COMMON /c_KarLud/
     $  m_CMSene,            ! Input CMS energy [GeV]
     $  m_XXXene,            ! CMS energy after beam-spread/beam-strahlung [GeV]
     $  m_DelEne,            ! Beam energy spread [GeV]
     $  m_exe,               ! z-boost due to beam spread
     $  m_amel,              ! electron beam mass
* ff-pair spectrum
     $  m_vv,                ! v = 1-s'/s
     $  m_x1,                ! x=1-z for 1-st beam
     $  m_x2,                ! x=1-z for 2-nd beam
     $  m_vvmin,             ! minimum v, infrared cut
     $  m_vvmax,             ! maximum v
     $  m_HadMin,            ! minimum hadronization mass [GeV]
* Basic QED
     $  m_alfinv,            ! Alpha_QED at Thomson limit
* Normalization, MC technicalities
     $  m_XCrude,            ! Crude xsect. from Vesk1
     $  m_xcgaus,            ! from gauss integration
     $  m_ErGaus,            ! gauss precision
     $  m_Xenph,             ! crude photon multiplicity enhancement factor
* ----------------- EVENT -------------------------
     $  m_p1(4),             ! beam momentum
     $  m_p2(4),             ! beam momentum
     $  m_q1(4),             ! final momentum UNUSED for FSR switched ON
     $  m_q2(4),             ! final momentum UNUSED for FSR switched ON
     $  m_sphot(m_npmx,4),   ! photon momenta
     $  m_sphum(4),          ! sum of photon momenta
     $  m_PX(4),             ! total 4-momentum minus ISR photons
     $  m_yini(m_npmx),      ! Sudakov y-variables
     $  m_zini(m_npmx),      ! Sudakov z-variables
     $  m_WtBasic,           ! Weight associated with V generation
     $  m_Wt_KF,             ! Weight associated with KF generation
     $  m_WtIni,             ! Weight associated with photon momenta generation
     $  m_WtMass,            ! WtIni component
     $  m_WtDil,             ! WtIni component  m_WtIni=m_WtMass*m_WtDil*m_WtCut
     $  m_WtCut,             ! WtIni component
     $  m_nphot,             ! Photon multiplicity
     $  m_KFfin,             ! final fermion code
* -------------------------------------------------
* Test Switches
     $  m_KeyZet,            ! Z-boson on/off
     $  m_KeyISR,            ! ISR on/off
     $  m_MltISR,            ! fixed photon mult. VERY SPECIAL tests
     $  m_KeyFix,            ! fixed v=vvmav.     VERY SPECIAL tests
     $  m_KeyWtm,            ! omitting mass terms in S-factors
* Miscelaneous
     $  m_nmax,              ! maximum photon multiplicity
     $  m_out,               ! output unit number
     $  m_NevGen,            ! event counter
     $  m_idyfs              ! pointer for histograms
*
      SAVE /c_KarLud/
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  KarLud                                //
*//////////////////////////////////////////////////////////////////////////////
