
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
// #pragma link C++ all namespaces;

#pragma link C++ class Belle2::ExtHit+; // checksum=0x6b296224, version=8
#pragma link C++ class Belle2::TrackClusterSeparation+; // checksum=0x8d25ce5e, version=3
#pragma link C++ class Belle2::V0ValidationVertex+; // checksum=0xceabe91f, version=1
#pragma link C++ class Belle2::FullSecID+; // checksum=0xa4543536, version=-1

#pragma link C++ class pair <bool, double>+; // checksum=0x90c0afc6, version=-1
#pragma link C++ class pair <bool, int>+; // checksum=0xe86a3d30, version=-1
#pragma link C++ class pair <unsigned int, pair < double, double > >+; // checksum=0xea1f577c, version=-1
#pragma link C++ class vector < pair <unsigned int, pair < double, double > > >+; // checksum=0x3073834d, version=6
#pragma link C++ class pair < unsigned int, vector < pair <unsigned int, pair < double, double > > > >+; // checksum=0x7a732554, version=-1
#pragma link C++ class vector < pair < unsigned int, vector < pair <unsigned int, pair < double, double > > > > >+; // checksum=0x80707160, version=6
#pragma link C++ class pair < unsigned int, vector < pair < unsigned int, vector < pair <unsigned int, pair < double, double > > > > > >+; // checksum=0x53c18924, version=-1
#pragma link C++ class vector < pair < unsigned int, vector < pair < unsigned int, vector < pair <unsigned int, pair < double, double > > > > > > >+; // checksum=0x47fe6901, version=6
#pragma link C++ class pair <unsigned int, double >+; // checksum=0x71a4cb1e, version=-1
#pragma link C++ class vector < pair <unsigned int, double > >+; // checksum=0x6b073150, version=6

#pragma link C++ class Belle2::ROIid+; // checksum=0x67e7bffd, version=1
#pragma link C++ class Belle2::ROIpayload+; // checksum=0x24692265, version=2
#pragma link C++ class Belle2::VXDIntercept+; // checksum=0xbb836403, version=1
#pragma link C++ class Belle2::PXDIntercept+; // checksum=0x756ab835, version=2
#pragma link C++ class Belle2::SVDIntercept+; // checksum=0x14d0197f, version=4
#pragma link C++ class Belle2::RecoTrack+; // checksum=0xf6a2843, version=9
#pragma link C++ class Belle2::RecoHitInformation+; // checksum=0xcf616f27, version=6
#pragma link C++ class Belle2::BremHit+; // checksum=0x56052b3a, version=1
#pragma link C++ class Belle2::SectorMapConfig+; // checksum=0x4ef40736, version=2
#pragma link C++ class Belle2::hitXP+; // checksum=0xdd9d6ff4, version=1
#pragma link C++ class Belle2::hitXPDerivate+; // checksum=0xfa2e4b3c, version=1
#pragma link C++ class Belle2::DATCONSVDDigit+; // checksum=0x45b64e93, version=1
#pragma link C++ class Belle2::DATCONSVDSpacePoint+; // checksum=0xb78110ca, version=1
#pragma link C++ class Belle2::DATCONMostProbableHit+; // checksum=0x75b048b2, version=2
#pragma link C++ class Belle2::DATCONTrack+; // checksum=0xa157a1e1, version=1
#pragma link C++ class std::vector<Belle2::hitXP>+; // checksum=0x26380ecc, version=6
#pragma link C++ class std::vector<Belle2::hitXPDerivate>+; // checksum=0x82b91048, version=6

#pragma link C++ class Belle2::SpacePointInfo+; // checksum=0xa7b6f75a, version=1
#pragma link C++ class Belle2::FilterInfo+; // checksum=0xc33c4990, version=1
#pragma link C++ class std::vector<Belle2::FilterInfo>+; // checksum=0x4718205b, version=6
#pragma link C++ class std::vector<Belle2::SpacePointInfo>+; // checksum=0x8273c275, version=6
#pragma link C++ class Belle2::ObserverInfo+; // checksum=0xb1e62507, version=1

// ----------------------------------------------------------------------------

// Muid evolution
// In version 5- (-4),
//   o   m_Momentum is not stored (stored in a TVector)
//   o   m_MuonPDFValue is stored in a float (double)
//   o   (ditto for m_PionPDFValue, m_KaonPDFValue, m_ProtonPDFValue,
//        m_DeuteronPDFValue, m_ElectronPDFValue, m_JunkPDFValue,
//        m_LogL_mu, m_LogL_pi, m_LogL_K, m_LogL_p, m_LogL_d,
//        m_LogL_e, m_ChiSquared)
#pragma read \
  sourceClass="Belle2::Muid" source="double m_MuonPDFValue" version="[-4]" \
  targetClass="Belle2::Muid" target="m_MuonPDFValue" \
  code = "{ m_MuonPDFValue = onfile.m_MuonPDFValue; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_PionPDFValue" version="[-4]" \
  targetClass="Belle2::Muid" target="m_PionPDFValue" \
  code = "{ m_PionPDFValue = onfile.m_PionPDFValue; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_KaonPDFValue" version="[-4]" \
  targetClass="Belle2::Muid" target="m_KaonPDFValue" \
  code = "{ m_KaonPDFValue = onfile.m_KaonPDFValue; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_ProtonPDFValue" version="[-4]" \
  targetClass="Belle2::Muid" target="m_ProtonPDFValue" \
  code = "{ m_ProtonPDFValue = onfile.m_ProtonPDFValue; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_DeuteronPDFValue" version="[-4]" \
  targetClass="Belle2::Muid" target="m_DeuteronPDFValue" \
  code = "{ m_DeuteronPDFValue = onfile.m_DeuteronPDFValue; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_ElectronPDFValue" version="[-4]" \
  targetClass="Belle2::Muid" target="m_ElectronPDFValue" \
  code = "{ m_ElectronPDFValue = onfile.m_ElectronPDFValue; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_JunkPDFValue" version="[-4]" \
  targetClass="Belle2::Muid" target="m_JunkPDFValue" \
  code = "{ m_JunkPDFValue = onfile.m_JunkPDFValue; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_LogL_mu" version="[-4]" \
  targetClass="Belle2::Muid" target="m_LogL_mu" \
  code = "{ m_LogL_mu = onfile.m_LogL_mu; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_LogL_pi" version="[-4]" \
  targetClass="Belle2::Muid" target="m_LogL_pi" \
  code = "{ m_LogL_pi = onfile.m_LogL_pi; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_LogL_K" version="[-4]" \
  targetClass="Belle2::Muid" target="m_LogL_K" \
  code = "{ m_LogL_K = onfile.m_LogL_K; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_LogL_p" version="[-4]" \
  targetClass="Belle2::Muid" target="m_LogL_p" \
  code = "{ m_LogL_p = onfile.m_LogL_p; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_LogL_d" version="[-4]" \
  targetClass="Belle2::Muid" target="m_LogL_d" \
  code = "{ m_LogL_d = onfile.m_LogL_d; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_LogL_e" version="[-4]" \
  targetClass="Belle2::Muid" target="m_LogL_e" \
  code = "{ m_LogL_e = onfile.m_LogL_e; }"
#pragma read \
  sourceClass="Belle2::Muid" source="double m_ChiSquared" version="[-4]" \
  targetClass="Belle2::Muid" target="m_ChiSquared" \
  code = "{ m_ChiSquared = onfile.m_ChiSquared; }"
// ExtHit evolution
// In version 4- (3 or earlier),
//   o   m_TOF is stored in a float (double)
//   o   m_Position is stored in a float[3] (TVector)
//   o   m_Momentum is stored in a float[3] (TVector)
//   o   m_Covariance is stored in a linearized float[21] (6x6 TMatrixDSym)
#pragma read \
  sourceClass="Belle2::ExtHit" source="double m_TOF" version="[-3]"\
  targetClass="Belle2::ExtHit" target="m_TOF" \
  code = "{ m_TOF = onfile.m_TOF; }"
#pragma read \
  sourceClass="Belle2::ExtHit" source="TVector3 m_Position" version="[-3]"\
  targetClass="Belle2::ExtHit" target="m_Position" \
  code = "{ \
            m_Position[0] = onfile.m_Position.X(); \
            m_Position[1] = onfile.m_Position.Y(); \
            m_Position[2] = onfile.m_Position.Z(); \
          }"
#pragma read \
  sourceClass="Belle2::ExtHit" source="TVector3 m_Momentum" version="[-3]"\
  targetClass="Belle2::ExtHit" target="m_Momentum" \
  code = "{ \
            m_Momentum[0] = onfile.m_Momentum.X(); \
            m_Momentum[1] = onfile.m_Momentum.Y(); \
            m_Momentum[2] = onfile.m_Momentum.Z(); \
          }"
#pragma read \
  sourceClass="Belle2::ExtHit" source="TMatrixDSym m_Covariance" version="[-3]" \
  targetClass="Belle2::ExtHit" target="m_Covariance" \
  code = "{ \
            if (onfile.m_Covariance.GetNrows() == 6) { \
              int k = 0; \
              for (int i = 0; i < 6; ++i) { \
                for (int j = 0; j <= i; ++j) { \
                  m_Covariance[k++] = onfile.m_Covariance(i,j); \
                } \
              } \
            } \
          }"
// TrackClusterSeparation evolution
// In version 3- (-2),
//   o   m_TrackIndex is not stored (stored in an int)
//   o   m_Direction is not stored (stored in a TVector3)
//   o   m_Distance is stored in a float (double)
//   o   (ditto for m_TrackClusterAngle, m_TrackClusterInitialSeparationAngle,
//        m_TrackClusterSeparationAngle, m_TrackRotationAngle)
#pragma read \
  sourceClass="Belle2::TrackClusterSeparation" source="double m_Distance" version="[-2]" \
  targetClass="Belle2::TrackClusterSeparation" target="m_Distance" \
  code = "{ m_Distance = onfile.m_Distance; }"
#pragma read \
  sourceClass="Belle2::TrackClusterSeparation" source="double m_TrackClusterAngle" version="[-2]" \
  targetClass="Belle2::TrackClusterSeparation" target="m_TrackClusterAngle" \
  code = "{ m_TrackClusterAngle = onfile.m_TrackClusterAngle; }"
#pragma read \
  sourceClass="Belle2::TrackClusterSeparation" source="double m_TrackClusterInitialSeparationAngle" version="[-2]" \
  targetClass="Belle2::TrackClusterSeparation" target="m_TrackClusterInitialSeparationAngle" \
  code = "{ m_TrackClusterInitialSeparationAngle = onfile.m_TrackClusterInitialSeparationAngle; }"
#pragma read \
  sourceClass="Belle2::TrackClusterSeparation" source="double m_TrackClusterSeparationAngle" version="[-2]" \
  targetClass="Belle2::TrackClusterSeparation" target="m_TrackClusterSeparationAngle" \
  code = "{ m_TrackClusterSeparationAngle = onfile.m_TrackClusterSeparationAngle; }"
#pragma read \
  sourceClass="Belle2::TrackClusterSeparation" source="double m_TrackRotationAngle" version="[-2]" \
  targetClass="Belle2::TrackClusterSeparation" target="m_TrackRotationAngle" \
  code = "{ m_TrackRotationAngle = onfile.m_TrackRotationAngle; }"
