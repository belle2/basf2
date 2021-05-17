#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Particle+; // checksum=0xd7cf258f, version=14
#pragma link C++ class Belle2::EventExtraInfo+; // checksum=0x965ad50b, version=2
#pragma link C++ class Belle2::ParticleList+; // checksum=0x98887650, version=3
#pragma link C++ class Belle2::RestOfEvent+; // checksum=0xe86f1658, version=7
#pragma link C++ class Belle2::TagVertex+; // checksum=0xbc37ca67, version=5
#pragma link C++ class Belle2::ContinuumSuppression+; // checksum=0xccdb3c88, version=1
#pragma link C++ class Belle2::FlavorTaggerInfo+; // checksum=0xa85ce063, version=4
#pragma link C++ class Belle2::FlavorTaggerInfoMap+; // checksum=0xcdef997a, version=3
#pragma link C++ class Belle2::StringWrapper+; // checksum=0xf15ef1c4, version=1
#pragma link C++ class Belle2::EventShapeContainer+; // checksum=0x57e119da, version=1
#pragma link C++ class Belle2::EventKinematics+; // checksum=0x23d1bfc2, version=2
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

// In version 4 the member m_MCdeltaT has been renamed to m_mcDeltaT.
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



#endif
