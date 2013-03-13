*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  GPS                                    //
*//                                                                          //
*//   Purpose:  Calculation of spin amplitudes using spinor methods          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
*//////////////////////////////////////////////////////////////////////////////
      DOUBLE PRECISION      m_pi
      PARAMETER( m_pi=3.1415926535897932d0)
      INTEGER    m_lenwt                               ! max. no. of aux weights
      PARAMETER (m_lenwt=200)
      INTEGER     m_phmax             ! maximum photon multiplicity ISR+FSR
      PARAMETER ( m_phmax = 100)
*//////////////////////////////////////////////////////////////////////////////
      DOUBLE COMPLEX  m_Pauli,    m_Pauli4
      DOUBLE COMPLEX  m_AmpBorn
      DOUBLE COMPLEX  m_AmpBoxy
      DOUBLE COMPLEX  m_AmpBorn1
      DOUBLE COMPLEX  m_AmpBorn2
      DOUBLE COMPLEX  m_AmpExpo0, m_AmpExpo1, m_AmpExpo2, m_AmpExpo2p
      DOUBLE COMPLEX  m_Rmat
      DOUBLE COMPLEX  m_IntReson, m_BoxGGtu,  m_BoxGZtu,  m_BoxGGut,  m_BoxGZut !
      DOUBLE COMPLEX  m_F1ini1,   m_F1fin1,   m_F1ini2,   m_F1fin2,   m_F1iniPair2, m_F1finPair2 !
      DOUBLE PRECISION      m_WtSet,    m_WtBest,   m_Debg,     m_Vcut
      DOUBLE PRECISION      m_e_QED,    m_Alfpi ,   m_Alfinv
      DOUBLE PRECISION      m_Loren1,   m_Lorin1
      DOUBLE PRECISION      m_Loren2,   m_Lorin2
      DOUBLE PRECISION      m_xi,       m_eta,    m_b,m_b1,  m_b2,    m_b3
      DOUBLE PRECISION      m_Sw2,      m_MZ,     m_GammZ,   m_Gmu
      INTEGER               m_KeyArb,   m_isr,     m_out
      INTEGER               m_Phel
      INTEGER               m_KeyISR,   m_KeyFSR, m_KeyINT,  m_KeyElw,  m_KeyZet,  m_KeyGPS
      INTEGER               m_HasFSR
      DOUBLE PRECISION      m_PolBeam1,         m_PolBeam2
      DOUBLE PRECISION      m_HvecFer1,         m_HvecFer2
      DOUBLE PRECISION      m_RhoCrud,      m_ExpoNorm
      DOUBLE PRECISION      m_RhoExp0,      m_RhoExp1,       m_RhoExp2,       m_RhoExp2p !
      DOUBLE COMPLEX  m_SDMat1,       m_SDMat2,       m_SDMat3,       m_SDMat4
*
      DOUBLE COMPLEX  sAph1(2),sBph1(2),sAph2(2),sBph2(2),sAph1o(2),sBph1o(2),sAph2o(2),sBph2o(2)
      COMMON /c_GPS/
     $    m_Pauli( 0:3, 1:2, 1:2),! Pauli matrices
     $    m_Pauli4(1:4, 1:2, 1:2),! Pauli matrices, other vector index numbering
     $    m_IntReson,             ! Virtual: resonance part of correction
     $    m_BoxGGtu,              ! Virtual: part of box
     $    m_BoxGZtu,              ! Virtual: part of box
     $    m_BoxGGut,              ! Virtual: part of box
     $    m_BoxGZut,              ! Virtual: part of box
     $    m_F1ini1,               ! Virtual: vertex ISR, O(alf1) corr.
     $    m_F1fin1,               ! Virtual: vertex ISR, O(alf2) corr.
     $    m_F1ini2,               ! Virtual: vertex FSR, O(alf1) corr.
     $    m_F1fin2,               ! Virtual: vertex FSR, O(alf2) corr.
     $    m_F1iniPair2,           ! Virtual: IS Pairs in vertex, O(alf2) corr.
     $    m_F1finPair2,           ! Virtual: FS Pairs in vertex, O(alf2) corr.
     $    m_Rmat( 0:3, 0:3),      ! Spin correlation matrix for final particles (test)
     $    m_AmpBorn( 2,2,2,2),    ! Born spin amplitudes, final mass kept exactly
     $    m_AmpBoxy( 2,2,2,2),    ! Box spin amplitudes, gamma-gamma and gamma-Z
     $    m_AmpBorn1(2,2,2,2),    ! Born spin amplitudes, massles
     $    m_AmpBorn2(2,2,2,2),    ! Born spin amplitudes, massles
     $    m_AmpExpo0(2,2,2,2),    ! CEEX spin amplitudes O(alf0)ceex
     $    m_AmpExpo1(2,2,2,2),    ! CEEX spin amplitudes O(alf1)ceex
     $    m_AmpExpo2(2,2,2,2),    ! CEEX spin amplitudes O(alf2)ceex
     $    m_AmpExpo2p(2,2,2,2),   ! CEEX spin amplitudes O(alf2)ceex with PAIRS
     $    m_PolBeam1(4),          ! POLARIZATION vector 1-st beam      PRIVATE no getter!!!
     $    m_PolBeam2(4),          ! POLARIZATION vector 2-nd beam      PRIVATE no getter!!!
     $    m_HvecFer1(4),          ! POLARIMETER  vector 1-st fin.ferm. PRIVATE no getter!!!
     $    m_HvecFer2(4),          ! POLARIMETER  vector 2-nd fin.ferm. PRIVATE no getter!!!
     $    m_SDMat1(2,2),          ! Spin Density Matrix  1-st beam 
     $    m_SDMat2(2,2),          ! Spin Density Matrix  2-nd beam 
     $    m_SDMat3(2,2),          ! Polarimeter Density Matrix  1-st fin.ferm
     $    m_SDMat4(2,2),          ! Polarimeter Density Matrix  2-nd fin.ferm
     $    m_RhoCrud,              ! Diff.distr. Crude MC,    normalized to LIPS
     $    m_RhoExp0,              ! Diff.distr. O(alf0)ceex, normalized to LIPS
     $    m_RhoExp1,              ! Diff.distr. O(alf1)ceex, normalized to LIPS
     $    m_RhoExp2,              ! Diff.distr. O(alf2)ceex, normalized to LIPS
     $    m_RhoExp2p,             ! Diff.distr. O(alf2)ceex+PAIRS, normalized to LIPS
     $    m_ExpoNorm,             ! Normalized factor to LIPS for CEEX amplitudes
     $    m_Debg(0:200),          ! Debug facility
     $    m_WtSet(m_lenwt),       ! List of weights
     $    m_WtBest,               ! The best of the weights
     $    m_Vcut(3),              ! technical cut on E/Ebeam for non-IR real contributions
     $    m_xi(4),                ! GPS axial vector,    lightlike xi**2=0
     $    m_eta(4),               ! GPS auxiliar vector, spacelike eta**2=-1
     $    m_b(4),                 ! Beta vector for photon polarization (Axial gauge)
     $    m_b1(4),                ! Beta vector as above for tests
     $    m_b2(4),                ! Beta vector as above for tests
     $    m_b3(4),                ! Beta vector as above for tests
     $    m_Loren1(4,4),          ! Lorenz tranfrom ONE for TRALOR (down to CMS)
     $    m_Lorin1(4,4),          !        and its inverse (for tests)
     $    m_Loren2(4,4),          ! Lorenz tranfrom TWO for TRALOR (down to CMS)
     $    m_Lorin2(4,4),          !        and its inverse (for tests)
     $    m_Sw2,                  ! Electroweak: mixing angle sin(theta_W)^2
     $    m_MZ,                   ! Electroweak: Z mass
     $    m_GammZ,                ! Electroweak: Z width
     $    m_Gmu,                  ! Electroweak: Fermi constant
     $    m_e_QED,                ! Electroweak: Electric coupling constant
     $    m_Alfpi,                ! Electroweak: alfQED/pi
     $    m_Alfinv,               ! Electroweak: 1/alfQED
     $    m_KeyArb,               ! Key for switching on/off the use of m_b
     $    m_isr( m_phmax),        ! partition       =1,0 for isr/fsr
     $    m_Phel(m_phmax),        ! photon helicity =1,0 for +,-
     $    m_KeyElw,               ! Electroweak switch
     $    m_KeyZet,               ! Z switch
     $    m_KeyISR,               ! ISR static switch
     $    m_KeyFSR,               ! FSR static switch
     $    m_KeyINT,               ! INT static interference switch
     $    m_KeyGPS,               ! CEEX perturb. level switch
     $    m_HasFSR,               ! FSR dynamic switch
     $    m_out                   ! output unit number

      SAVE /c_GPS/
*
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  GPS                                   //
*//////////////////////////////////////////////////////////////////////////////



