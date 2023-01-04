#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ class Belle2::Particle+; // checksum=0x694ee2d8, version=16
#pragma link C++ class Belle2::ParticleList+; // checksum=0x98887650, version=3
#pragma link C++ class Belle2::RestOfEvent+; // checksum=0xe86f1658, version=8
#pragma link C++ class Belle2::TagVertex+; // checksum=0xf7b6169c, version=6
#pragma link C++ class Belle2::ContinuumSuppression+; // checksum=0x8f4ac744, version=2
#pragma link C++ class Belle2::FlavorTaggerInfo+; // checksum=0xa85ce063, version=4
#pragma link C++ class Belle2::FlavorTaggerInfoMap+; // checksum=0xcdef997a, version=3
#pragma link C++ class Belle2::StringWrapper+; // checksum=0xf15ef1c4, version=1
#pragma link C++ class Belle2::EventShapeContainer+; // checksum=0xe048bccd, version=2
#pragma link C++ class Belle2::EventKinematics+; // checksum=0xabfc6580, version=3
#pragma link C++ class Belle2::TauPairDecay+; // checksum=0x15ea58a, version=1
#pragma link C++ class Belle2::ECLEnergyCloseToTrack+; // checksum=0xeadb37d4, version=1
#pragma link C++ class Belle2::ECLTRGInformation+; // checksum=0x744abff9, version=3
#pragma link C++ class Belle2::ECLTriggerCell+; // checksum=0xfcbe0110, version=2
//these two are needed when using these types (returned by Particle) in PyROOT
#pragma link C++ class vector<Belle2::Particle*>-;
#pragma link C++ class vector<const Belle2::Particle*>-;

#pragma link C++ class Belle2::ParticleExtraInfoMap+; // checksum=0x82838333, version=1
#pragma link C++ class Belle2::ParticleExtraInfoMap::IndexMap+; // checksum=0xf9eb593, version=6
#pragma link C++ class map<string, unsigned int>+; // checksum=0xf9eb593, version=6
#pragma link C++ class vector<map<string, unsigned int> >+; // checksum=0x267ce51a, version=6
#pragma link C++ class Belle2::Btube+; // checksum=0x772238ab, version=1
#pragma link C++ class Eigen::Matrix<double,3,1,0,3,1>+; // checksum=0x43805c43, version=-1
#pragma link C++ class Eigen::PlainObjectBase<Eigen::Matrix<double,3,1,0,3,1> >+; // checksum=0x6464cd47, version=-1
#pragma link C++ class Eigen::EigenBase<Eigen::Matrix<double,3,1,0,3,1> >+; // checksum=0x76f10151, version=-1
#pragma link C++ class Eigen::MatrixBase<Eigen::Matrix<double,3,1,0,3,1> >+; // checksum=0x3988eec2, version=-1
#pragma link C++ class Eigen::DenseStorage<double,3,3,1,0>+; // checksum=0xe13ab357, version=-1
#pragma link C++ class Eigen::DenseBase<Eigen::Matrix<double,3,1,0,3,1> >+; // checksum=0xddba3f8c, version=-1
#pragma link C++ class Eigen::DenseCoeffsBase<Eigen::Matrix<double,3,1,0,3,1>,0>+; // checksum=0x3b876532, version=-1
#pragma link C++ class Eigen::DenseCoeffsBase<Eigen::Matrix<double,3,1,0,3,1>,1>+; // checksum=0x2bf33e25, version=-1
#pragma link C++ class Eigen::DenseCoeffsBase<Eigen::Matrix<double,3,1,0,3,1>,3>+; // checksum=0xd3a06597, version=-1
#pragma link C++ class Eigen::internal::plain_array<double,3,0,0>+; // checksum=0xa73f796a, version=-1

// ----------------------------------------------------------------------------
// Particle evolution
// In version 14 the member m_momentumScalingFactor has been added
#pragma read                                                                          \
  sourceClass="Belle2::Particle"                                                      \
  source="float m_momentumScale"                                                      \
  version="[13-14]"                                                                   \
  targetClass="Belle2::Particle"                                                      \
  target="m_momentumScalingFactor"                                                    \
  code="{m_momentumScalingFactor = onfile.m_momentumScale;}"                          \

// ----------------------------------------------------------------------------
// Particle evolution
// In version 12 the member m_particleType has been renamed to m_particleSource.
#pragma read                                                                          \
  sourceClass="Belle2::Particle"                                                      \
  source="Belle2::Particle::EParticleSourceObject m_particleType"                     \
  version="[-11]"                                                                     \
  targetClass="Belle2::Particle"                                                      \
  target="m_particleSource"                                                           \
  code="{if (onfile.m_particleType == 4)                                              \
           m_particleSource = Belle2::Particle::EParticleSourceObject::c_MCParticle;  \
         else if (onfile.m_particleType == 5)                                         \
           m_particleSource = Belle2::Particle::EParticleSourceObject::c_Composite;   \
         else m_particleSource = onfile.m_particleType;                               \
       }"                                                                             \

// ----------------------------------------------------------------------------
// TagVertex evolution
// In version 4 the member m_MCdeltaT has been renamed to m_mcDeltaT.
#pragma read                                    \
  sourceClass="Belle2::TagVertex"               \
  source="float m_MCdeltaT"                     \
  version="[-3]"                                \
  targetClass="Belle2::TagVertex"               \
  target="m_mcDeltaT"                           \
  code="{m_mcDeltaT = onfile.m_MCdeltaT;}"      \

// In version 4 the member m_MCtagV has been renamed to m_mcTagV.
#pragma read                                    \
  sourceClass="Belle2::TagVertex"               \
  source="TVector3 m_MCtagV"                    \
  version="[-3]"                                \
  targetClass="Belle2::TagVertex"               \
  target="m_mcTagV"                             \
  code="{m_mcTagV = onfile.m_MCtagV;}"          \

// In version 4 the data type of the member m_tagVertexErrMatrix has been
// changed from TMatrixFSym to TMatrixDSym
#pragma read                                                   \
  sourceClass="Belle2::TagVertex"                              \
  source="TMatrixFSym m_tagVertexErrMatrix"                    \
  version="[-3]"                                               \
  targetClass="Belle2::TagVertex"                              \
  target="m_tagVertexErrMatrix"                                \
  code="{m_tagVertexErrMatrix = onfile.m_tagVertexErrMatrix;}" \

// In version 6 the data type of the member m_tagVertex has been
// changed from TVector3 to XYZVector
#pragma read                                    \
  sourceClass="Belle2::TagVertex"               \
  source="TVector3 m_tagVertex"                 \
  version="[-5]"                                \
  targetClass="Belle2::TagVertex"               \
  target="m_tagVertex"                          \
  code="{m_tagVertex = ROOT::Math::XYZVector(onfile.m_tagVertex.X(), onfile.m_tagVertex.Y(), onfile.m_tagVertex.Z());}"    \

// In version 6 the data type of the member m_mcTagV has been
// changed from TVector3 to XYZVector
#pragma read                                    \
  sourceClass="Belle2::TagVertex"               \
  source="TVector3 m_mcTagV"                    \
  version="[4-5]"                               \
  targetClass="Belle2::TagVertex"               \
  target="m_mcTagV"                             \
  code="{m_mcTagV = ROOT::Math::XYZVector(onfile.m_mcTagV.X(), onfile.m_mcTagV.Y(), onfile.m_mcTagV.Z());}"          \

// In version 6 the data type of the member m_constraintCenter
// has been changed from TVector3 to XYZVector
#pragma read                                                \
  sourceClass="Belle2::TagVertex"                           \
  source="TVector3 m_constraintCenter"                      \
  version="[-5]"                                            \
  targetClass="Belle2::TagVertex"                           \
  target="m_constraintCenter"                               \
  code="{m_constraintCenter = ROOT::Math::XYZVector(onfile.m_constraintCenter.X(), onfile.m_constraintCenter.Y(), onfile.m_constraintCenter.Z());}"  \

// ----------------------------------------------------------------------------
// EventKinematics evolution
// In version 2 the new member m_builtFromMC is introduced. Since the MC
// functionality was not present before it is set to false for version 1.
#pragma read                                   \
  sourceClass="Belle2::EventKinematics"        \
  source="bool m_builtFromMC"                  \
  version="[1]"                                \
  targetClass="Belle2::EventKinematics"        \
  target="m_builtFromMC"                       \
  code="{m_builtFromMC = false;}"              \

// In version 3 the data type of the member m_missingMomentum has been
// changed from TVector3 to XYZVector
#pragma read                                              \
  sourceClass="Belle2::EventKinematics"                   \
  source="TVector3 m_missingMomentum"                     \
  version="[-2]"                                          \
  targetClass="Belle2::EventKinematics"                   \
  target="m_missingMomentum"                              \
  code="{m_missingMomentum = ROOT::Math::XYZVector(onfile.m_missingMomentum.X(), onfile.m_missingMomentum.Y(), onfile.m_missingMomentum.Z());}"  \

// In version 3 the data type of the member m_missingMomentumCMS has been
// changed from TVector3 to XYZVector
#pragma read                                                   \
  sourceClass="Belle2::EventKinematics"                        \
  source="TVector3 m_missingMomentumCMS"                       \
  version="[-2]"                                               \
  targetClass="Belle2::EventKinematics"                        \
  target="m_missingMomentumCMS"                                \
  code="{m_missingMomentumCMS = ROOT::Math::XYZVector(onfile.m_missingMomentumCMS.X(), onfile.m_missingMomentumCMS.Y(), onfile.m_missingMomentumCMS.Z());}" \

// ----------------------------------------------------------------------------
// RestOfEvent evolution
// In version 6 the new member m_useKLMEnergy is introduced. Since this
// functionality was not present before it is set to false for versions < 6.
#pragma read                                   \
  sourceClass="Belle2::RestOfEvent"            \
  source="bool m_useKLMEnergy"                 \
  version="[-5]"                               \
  targetClass="Belle2::RestOfEvent"            \
  target="m_useKLMEnergy"                      \
  code="{m_useKLMEnergy = false;}"             \

// ----------------------------------------------------------------------------
// ContinuumSuppression evolution
// In version 2 the data type of the members m_thrustO and m_thrustB has been
// changed from TVector3 to XYZVector
#pragma read                                  \
  sourceClass="Belle2::ContinuumSuppression"  \
  source="TVector3 m_thrustO"                 \
  version="[1]"                               \
  targetClass="Belle2::ContinuumSuppression"  \
  target="m_thrustO"                          \
  code="{m_thrustO = ROOT::Math::XYZVector(onfile.m_thrustO.X(), onfile.m_thrustO.Y(), onfile.m_thrustO.Z());}"      \

#pragma read                                  \
  sourceClass="Belle2::ContinuumSuppression"  \
  source="TVector3 m_thrustB"                 \
  version="[1]"                               \
  targetClass="Belle2::ContinuumSuppression"  \
  target="m_thrustB"                          \
  code="{m_thrustB = ROOT::Math::XYZVector(onfile.m_thrustB.X(), onfile.m_thrustB.Y(), onfile.m_thrustB.Z());}"      \

// ----------------------------------------------------------------------------
// EventShapeContainer evolution
// In version 2 the data type of the member m_thrustAxis has been
// changed from TVector3 to XYZVector
#pragma read                                    \
  sourceClass="Belle2::EventShapeContainer"     \
  source="TVector3 m_thrustAxis"                \
  version="[1]"                                 \
  targetClass="Belle2::EventShapeContainer"     \
  target="m_thrustAxis"                         \
  code="{m_thrustAxis = ROOT::Math::XYZVector(onfile.m_thrustAxis.X(), onfile.m_thrustAxis.Y(), onfile.m_thrustAxis.Z());}"  \

// In version 2 the data type of the member m_spherocityAxis has been
// changed from TVector3 to XYZVector
#pragma read                                            \
  sourceClass="Belle2::EventShapeContainer"             \
  source="TVector3 m_spherocityAxis"                    \
  version="[1]"                                         \
  targetClass="Belle2::EventShapeContainer"             \
  target="m_spherocityAxis"                             \
  code="{m_spherocityAxis = ROOT::Math::XYZVector(onfile.m_spherocityAxis.X(), onfile.m_spherocityAxis.Y(), onfile.m_spherocityAxis.Z());}"  \

// In version 2 the data type of the member m_sphericityEigenvector
// has been changed from array of TVector3 to array of XYZVector
#pragma read                                                                                          \
  sourceClass="Belle2::EventShapeContainer"                                                           \
  source="TVector3 m_sphericityEigenvector[3]"                                                        \
  version="[1]"                                                                                       \
  targetClass="Belle2::EventShapeContainer"                                                           \
  target="m_sphericityEigenvector"                                                                    \
  code="{for (int i = 0; i < 3; i++)                                                                  \
           m_sphericityEigenvector[i] = ROOT::Math::XYZVector(onfile.m_sphericityEigenvector[i].X(),  \
                                                              onfile.m_sphericityEigenvector[i].Y(),  \
                                                              onfile.m_sphericityEigenvector[i].Z()); \
       }"                                                                                             \

// In version 2 the data type of the member m_forwardHemisphere4Momentum
// has been changed from TLorentzVector to PxPyPzEVector
#pragma read                                                                                                  \
  sourceClass="Belle2::EventShapeContainer"                                                                   \
  source="TLorentzVector m_forwardHemisphere4Momentum"                                                        \
  version="[1]"                                                                                               \
  targetClass="Belle2::EventShapeContainer"                                                                   \
  target="m_forwardHemisphere4Momentum"                                                                       \
  code="{m_forwardHemisphere4Momentum = ROOT::Math::PxPyPzEVector(onfile.m_forwardHemisphere4Momentum.Px(),   \
                                                                  onfile.m_forwardHemisphere4Momentum.Py(),   \
                                                                  onfile.m_forwardHemisphere4Momentum.Pz(),   \
                                                                  onfile.m_forwardHemisphere4Momentum.E());}" \

// In version 2 the data type of the member m_backwardHemisphere4Momentum
// has been changed from TLorentzVector to PxPyPzEVector
#pragma read                                                                                                    \
  sourceClass="Belle2::EventShapeContainer"                                                                     \
  source="TLorentzVector m_backwardHemisphere4Momentum"                                                         \
  version="[1]"                                                                                                 \
  targetClass="Belle2::EventShapeContainer"                                                                     \
  target="m_backwardHemisphere4Momentum"                                                                        \
  code="{m_backwardHemisphere4Momentum = ROOT::Math::PxPyPzEVector(onfile.m_backwardHemisphere4Momentum.Px(),   \
                                                                   onfile.m_backwardHemisphere4Momentum.Py(),   \
                                                                   onfile.m_backwardHemisphere4Momentum.Pz(),   \
                                                                   onfile.m_backwardHemisphere4Momentum.E());}" \

#endif
