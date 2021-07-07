*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                                                                                 //
*//                         Pseudo-CLASS  Taoula                                    //
*//                                                                                 //
*//       Purpose:                                                                  //
*//       (a) Simulation of tau decays                                              //
*//       (b) Provides polarimeter vector                                           //
*//       (c) Transforms decay products to CMS frame                                //
*//       (d) Interfaces Photos to Tauola (partly???)                               //
*//                                                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////

      DOUBLE PRECISION      m_pi
      PARAMETER( m_pi = 3.1415926535897932384626433832d+00)

*
      DOUBLE PRECISION       m_HvecTau1,         m_HvecTau2
      DOUBLE PRECISION       m_HvClone1,         m_HvClone2
      DOUBLE PRECISION       m_beta1,     m_alfa1,     m_gamma1
      DOUBLE PRECISION       m_beta2,     m_alfa2,     m_gamma2
      DOUBLE PRECISION       m_phi1,      m_thet1
      DOUBLE PRECISION       m_phi2,      m_thet2
      INTEGER     m_out,       m_IsInitialized,  m_idyfs, m_KeyClone, m_IFPHOT
*
      COMMON /c_Taupair/
     $    m_HvecTau1(4),      ! Spin Polarimeter vector first  Tau
     $    m_HvecTau2(4),      ! Spin Polarimeter vector second Tau
     $    m_HvClone1(4),      ! Clone Spin Polarimeter vector first  Tau
     $    m_HvClone2(4),      ! Clone Spin Polarimeter vector second Tau
     $    m_beta1,            ! Random Euler angle for cloning 1-st tau
     $    m_alfa1,            ! Random Euler angle for cloning 1-st tau
     $    m_gamma1,           ! Random Euler angle for cloning 1-st tau
     $    m_beta2,            ! Random Euler angle for cloning 2-nd tau
     $    m_alfa2,            ! Random Euler angle for cloning 2-nd tau
     $    m_gamma2,           ! Random Euler angle for cloning 2-nd tau
     $    m_phi1,             ! phi   of HvecTau1
     $    m_thet1,            ! theta of HvecTau1
     $    m_phi2,             ! phi   of HvecTau2
     $    m_thet2,            ! theta of HvecTau2
     $    m_IsInitialized,    ! super key, for inhibiting all tauola activity
     $    m_IFPHOT,           ! key for PHOTOS
     $    m_KeyClone,         ! switch for cloning procedure =1,2
     $    m_idyfs,            ! pointer for internal histograming
     $    m_out               ! output unit number
      SAVE /c_Taupair/

* common block of TAUOLA !!!!&&&
      INTEGER         m_NP1, m_NP2 
      COMMON /TAUPOS/ m_NP1, m_NP2                
*-------------------------------------------------------------------------------------

*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                      End of CLASS  Tauola                                       //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
