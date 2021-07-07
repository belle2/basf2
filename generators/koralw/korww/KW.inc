*////////////////////////////////////////////////////////////////////////////////
*//                                                                            //
*//   ======================================================================   //
*//   ======================================================================   //
*//   ===========================KORALW=====================================   //
*//   ======================WW pair production==============================   //
*//   =================initial state exponentiation=========================   //
*//   ======================================================================   //
*//                                                                            //
*////////////////////////////////////////////////////////////////////////////////
*     Author list:
*          S. Jadach      (Stanislaw.Jadach@cern.ch)
*          W. Placzek     (Wieslaw.Placzek@cern.ch)
*          M. Skrzypek    (Maciej.Skrzypek@cern.ch)
*          B.F.L. Ward    (bflw@slac.stanford.edu)
*          Z. Was         (Zbigniew.Was@cern.ch)
*////////////////////////////////////////////////////////////////////////////////
*
      INTEGER     m_Npar
      REAL*8      m_Xpar
      INTEGER     m_IdGen,     idyfs
      REAL*8      m_Xcrude,  m_svar,      m_alpha_s,   Preco,    Wtu
      REAL*8      m_XSecMC,  m_XErrMC
      REAL*8      m_XSecNR,  m_XErrNR
      INTEGER     m_KeyWgt,  m_KeySmp,    m_KeyIsr,  m_Key4f,  m_KeyAcc
      INTEGER     m_NevTru,  IevAcc,      NevTot
      INTEGER     m_i_4f,    m_i_beta
*
      COMMON   /c_KW/
     $  m_Npar( 1000),           ! old Npar local Temporary/Obsolete
     $  m_Xpar(10000),           ! Xpar local
     $  m_XSecMC,                ! Final Main Best Monte Carlo Cross Section [pb]
     $  m_XErrMC,                ! and its error [pb]
     $  m_XSecNR,                ! Normalization Carlo Cross Section [pb]
     $  m_XErrNR,                ! and its error [pb]
     $  m_svar,                  ! CMSene**2
     $  m_alpha_s,               ! Alpha strong
     $  Preco,                   ! Should go to lower level??
     $  m_Xcrude,                ! Crude x-section
     $  Wtu,                     ! Should go to lower level??
     $  m_IdGen,                 ! THIS generator ident
     $  idyfs,                   ! Used/Defined in lower levels?
     $  m_KeyWgt,                ! Weighted events
     $  m_KeySmp,                ! Type of sampler
     $  m_KeyIsr,                ! ISR on/off
     $  m_Key4f,                 ! Matrix element
     $  m_KeyAcc,                ! Anomalous couplings
     $  IevAcc,                  ! Counter of accepted events
     $  m_NevTru,                ! Counter of MC events broadcost to user
     $  NevTot,                  ! Probably the same as m_NevTru, to be eliminated???
     $  m_i_4f,                  ! Monitoring
     $  m_i_beta                 ! Monitoring
*
      SAVE    /c_KW/
*////////////////////////////////////////////////////////////////////////////////
*//                                                                            //
*//                                                                            //
*//                                                                            //
*//                                                                            //
*////////////////////////////////////////////////////////////////////////////////
