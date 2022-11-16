
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
// #pragma link C++ all namespaces;

#pragma link C++ class Belle2::ExtHit+; // checksum=0x6b296224, version=8
#pragma link C++ class Belle2::TrackClusterSeparation+; // checksum=0x8d25ce5e, version=3
#pragma link C++ class Belle2::V0ValidationVertex+; // checksum=0xce9006fc, version=2
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
#pragma link C++ class pair <bool, bool>+; // checksum=0xd82e1600, version=-1

#pragma link C++ class Belle2::ROIid+; // checksum=0x67e7bffd, version=1
#pragma link C++ class Belle2::ROIpayload+; // checksum=0x24692265, version=2
#pragma link C++ class Belle2::VXDIntercept+; // checksum=0xbb836403, version=1
#pragma link C++ class Belle2::PXDIntercept+; // checksum=0x756ab835, version=2
#pragma link C++ class Belle2::SVDIntercept+; // checksum=0x14d0197f, version=4
#pragma link C++ class Belle2::RecoTrack+; // checksum=0x980d7fa7, version=13
#pragma link C++ class Belle2::RecoHitInformation+; // checksum=0xcf616f27, version=6
#pragma link C++ class Belle2::BremHit+; // checksum=0xc51e119f, version=2
#pragma link C++ class Belle2::SectorMapConfig+; // checksum=0x4ef40736, version=2
#pragma link C++ class Belle2::hitXP+; // checksum=0x30c07c0a, version=2
#pragma link C++ class Belle2::hitXPDerivate+; // checksum=0x4d515752, version=2
#pragma link C++ class std::vector<Belle2::hitXP>+; // checksum=0x26380ecc, version=6
#pragma link C++ class std::vector<Belle2::hitXPDerivate>+; // checksum=0x82b91048, version=6

#pragma link C++ class Belle2::SpacePointInfo+; // checksum=0xb7462bee, version=2
#pragma link C++ class Belle2::FilterInfo+; // checksum=0xc33c4990, version=1
#pragma link C++ class std::vector<Belle2::FilterInfo>+; // checksum=0x4718205b, version=6
#pragma link C++ class std::vector<Belle2::SpacePointInfo>+; // checksum=0x8273c275, version=6
#pragma link C++ class Belle2::ObserverInfo+; // checksum=0xb1e62507, version=1

// ----------------------------------------------------------------------------

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

// V0ValidationVertex evolution
// In version 2- (1)
//   o   m_fittedVertexPosition is of type ROOT::Math::XYZVector (TVector3)
#pragma read \
  sourceClass="Belle2::V0ValidationVertex" source="TVector3 m_fittedVertexPosition" version="[-1]" \
  targetClass="Belle2::V0ValidationVertex" target="m_fittedVertexPosition" \
  code = "{ \
            m_fittedVertexPosition.SetXYZ(onfile.m_fittedVertexPosition.X(), \
                                          onfile.m_fittedVertexPosition.Y(), \
                                          onfile.m_fittedVertexPosition.Z()); \
          }"

// BremHit evolution
// In version 2- (1)
//   o   m_position is of type ROOT::Math::XYZVector (TVector3)
#pragma read \
  sourceClass="Belle2::BremHit" source="TVector3 m_position" version="[-1]" \
  targetClass="Belle2::BremHit" target="m_position" \
  code = "{ \
            m_position.SetXYZ(onfile.m_position.X(), \
                              onfile.m_position.Y(), \
                              onfile.m_position.Z()); \
          }"

// SpacePointInfo evolution
// In version 2- (1)
//   o   m_position is of type ROOT::Math::XYZVector (TVector3)
//   o   m_positionError is of type ROOT::Math::XYZVector (TVector3)
#pragma read \
  sourceClass="Belle2::SpacePointInfo" source="TVector3 m_position" version="[-1]" \
  targetClass="Belle2::SpacePointInfo" target="m_position" \
  code = "{ \
            m_position.SetXYZ(onfile.m_position.X(), \
                              onfile.m_position.Y(), \
                              onfile.m_position.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::SpacePointInfo" source="TVector3 m_positionError" version="[-1]" \
  targetClass="Belle2::SpacePointInfo" target="m_positionError" \
  code = "{ \
            m_positionError.SetXYZ(onfile.m_positionError.X(), \
                                   onfile.m_positionError.Y(), \
                                   onfile.m_positionError.Z()); \
          }"

// hitXP evolution
// In version 2- (1)
//   o   m_position is of type ROOT::Math::XYZVector (TVector3)
//   o   m_positionEntry is of type ROOT::Math::XYZVector (TVector3)
//   o   m_positionExit is of type ROOT::Math::XYZVector (TVector3)
//   o   m_momentumMid is of type ROOT::Math::XYZVector (TVector3)
//   o   m_momentumEntry is of type ROOT::Math::XYZVector (TVector3)
//   o   m_momentumExit is of type ROOT::Math::XYZVector (TVector3)
//   o   m_positionLocalMid is of type ROOT::Math::XYZVector (TVector3)
//   o   m_positionLocalEntry is of type ROOT::Math::XYZVector (TVector3)
//   o   m_positionLocalExit is of type ROOT::Math::XYZVector (TVector3)
//   o   m_momentumLocalMid is of type ROOT::Math::XYZVector (TVector3)
//   o   m_momentumLocalEntry is of type ROOT::Math::XYZVector (TVector3)
//   o   m_momentumLocalExit is of type ROOT::Math::XYZVector (TVector3)
//   o   m_position0 is of type ROOT::Math::XYZVector (TVector3)
//   o   m_momentum0 is of type ROOT::Math::XYZVector (TVector3)
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_positionMid" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_positionMid" \
  code = "{ \
            m_positionMid.SetXYZ(onfile.m_positionMid.X(), \
                                 onfile.m_positionMid.Y(), \
                                 onfile.m_positionMid.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_positionEntry" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_positionEntry" \
  code = "{ \
            m_positionEntry.SetXYZ(onfile.m_positionEntry.X(), \
                                   onfile.m_positionEntry.Y(), \
                                   onfile.m_positionEntry.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_positionExit" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_positionExit" \
  code = "{ \
            m_positionExit.SetXYZ(onfile.m_positionExit.X(), \
                                  onfile.m_positionExit.Y(), \
                                  onfile.m_positionExit.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_momentumMid" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_momentumMid" \
  code = "{ \
            m_momentumMid.SetXYZ(onfile.m_momentumMid.X(), \
                                 onfile.m_momentumMid.Y(), \
                                 onfile.m_momentumMid.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_momentumEntry" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_momentumEntry" \
  code = "{ \
            m_momentumEntry.SetXYZ(onfile.m_momentumEntry.X(), \
                                   onfile.m_momentumEntry.Y(), \
                                   onfile.m_momentumEntry.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_momentumExit" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_momentumExit" \
  code = "{ \
            m_momentumExit.SetXYZ(onfile.m_momentumExit.X(), \
                                  onfile.m_momentumExit.Y(), \
                                  onfile.m_momentumExit.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_positionLocalMid" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_positionLocalMid" \
  code = "{ \
            m_positionLocalMid.SetXYZ(onfile.m_positionLocalMid.X(), \
                                      onfile.m_positionLocalMid.Y(), \
                                      onfile.m_positionLocalMid.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_positionLocalEntry" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_positionLocalEntry" \
  code = "{ \
            m_positionLocalEntry.SetXYZ(onfile.m_positionLocalEntry.X(), \
                                        onfile.m_positionLocalEntry.Y(), \
                                        onfile.m_positionLocalEntry.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_positionLocalExit" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_positionLocalExit" \
  code = "{ \
            m_positionLocalExit.SetXYZ(onfile.m_positionLocalExit.X(), \
                                       onfile.m_positionLocalExit.Y(), \
                                       onfile.m_positionLocalExit.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_momentumLocalMid" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_momentumLocalMid" \
  code = "{ \
            m_momentumLocalMid.SetXYZ(onfile.m_momentumLocalMid.X(), \
                                      onfile.m_momentumLocalMid.Y(), \
                                      onfile.m_momentumLocalMid.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_momentumLocalEntry" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_momentumLocalEntry" \
  code = "{ \
            m_momentumLocalEntry.SetXYZ(onfile.m_momentumLocalEntry.X(), \
                                        onfile.m_momentumLocalEntry.Y(), \
                                        onfile.m_momentumLocalEntry.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_momentumLocalExit" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_momentumLocalExit" \
  code = "{ \
            m_momentumLocalExit.SetXYZ(onfile.m_momentumLocalExit.X(), \
                                       onfile.m_momentumLocalExit.Y(), \
                                       onfile.m_momentumLocalExit.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_position0" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_position0" \
  code = "{ \
            m_position0.SetXYZ(onfile.m_position0.X(), \
                               onfile.m_position0.Y(), \
                               onfile.m_position0.Z()); \
          }"
#pragma read \
  sourceClass="Belle2::hitXP" source="TVector3 m_momentum0" version="[-1]" \
  targetClass="Belle2::hitXP" target="m_momentum0" \
  code = "{ \
            m_momentum0.SetXYZ(onfile.m_momentum0.X(), \
                               onfile.m_momentum0.Y(), \
                               onfile.m_momentum0.Z()); \
          }"
