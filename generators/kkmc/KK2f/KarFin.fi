*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  KarFin                                 //
*//                                                                          //
*//   Purpose:                                                               //
*//   Top level  Monte-Carlo event generator for FSR radiadion.              //
*//////////////////////////////////////////////////////////////////////////////
*
*  Class members:
      INTEGER     m_npmx      ! maximum photon multiplicity, see also m_nmax
      PARAMETER ( m_npmx = 100)
      DOUBLE PRECISION     m_alfinv, m_emin,    m_delta,   m_MasPhot
      DOUBLE PRECISION     m_phot,   m_yfin,    m_zfin,    m_phsu
      DOUBLE PRECISION     m_Xenph
      DOUBLE PRECISION     m_q1,     m_q2,      m_r1,      m_r2
      DOUBLE PRECISION     m_VoluMC, m_YFS_IR,  m_YFSkon,  m_WtMass
      INTEGER   m_KeyPia, m_MltFSR,  m_nmax
      INTEGER   m_out,    m_idyfs,   m_Nevgen
      INTEGER   m_KeyFSR, m_KeyQSR,  m_IsFSR
      INTEGER   m_nphot,  m_MarTot
*
      COMMON /c_KarFin/
     $ m_out,                       ! Output file
     $ m_NevGen,                    ! Event serial number
     $ m_alfinv,                    ! alpha QED at Thomson limit
     $ m_emin,                      ! minimum energy at CMS for remooval 
     $ m_delta,                     ! infrared cut, dimensionless
     $ m_MasPhot,                   ! dummy photon mass, IR regulator
     $ m_Xenph,                     ! crude photon multiplicity enhancement factor
*-----------------EVENT---------------------
     $ m_VoluMC,                    ! MC crude volume of PhhSpace*Sfactors
     $ m_YFS_IR,                    ! YFS formfactor IR part only
     $ m_YFSkon,                    ! YFS formfactor non-IR finite part
     $ m_WtMass,                    ! mass weight, product
     $ m_q1(4),                     ! fermion momentum
     $ m_q2(4),                     ! fermion momentum
     $ m_r1(4),                     ! shadow of q1 fermion momentum
     $ m_r2(4),                     ! shadow of q2 fermion momentum
     $ m_phsu(4),                   ! sum of photon momenta
     $ m_phot(m_npmx,4),            ! photon momenta
     $ m_yfin(m_npmx),              ! Sudakov y-variables
     $ m_zfin(m_npmx),              ! Sudakov z-variables
     $ m_nphot,                     ! Photon multiplicity
*--------------------------------------------
     $ m_idyfs,                     ! Pointer for Histograms
     $ m_KeyPia,                    ! soft photon remooval switch
     $ m_MltFSR,                    ! special test switch
     $ m_nmax,                      ! max photon multiplicity
     $ m_KeyFSR,                    ! FSR switch general
     $ m_KeyQSR,                    ! FSR switch for quarks
     $ m_IsFSR,                     ! FSR flag for actual event
     $ m_MarTot                     ! marked events counter
*
      SAVE /c_KarFin/
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  KarLud                                //
*//////////////////////////////////////////////////////////////////////////////
