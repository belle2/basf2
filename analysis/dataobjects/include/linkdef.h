#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Particle+; // checksum=0x5a78ef64, version=13
#pragma link C++ class Belle2::EventExtraInfo+; // checksum=0x965ad50b, version=2
#pragma link C++ class Belle2::ParticleList+; // checksum=0x98887650, version=3
#pragma link C++ class Belle2::RestOfEvent+; // checksum=0xa87084c6, version=5
#pragma link C++ class Belle2::TagVertex+; // checksum=0xbc37ca67, version=5
#pragma link C++ class Belle2::ContinuumSuppression+; // checksum=0xccdb3c88, version=1
#pragma link C++ class Belle2::FlavorTaggerInfo+; // checksum=0x2e59c562, version=3
#pragma link C++ class Belle2::FlavorTaggerInfoMap+; // checksum=0xcdef997a, version=3
#pragma link C++ class Belle2::StringWrapper+; // checksum=0xf15ef1c4, version=1
#pragma link C++ class Belle2::EventShapeContainer+; // checksum=0x57e119da, version=1
#pragma link C++ class Belle2::EventKinematics+; // checksum=0x2ede9ce0, version=1
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

#pragma read                                    \
  sourceClass="Belle2::Particle"                \
  source="int m_particleType"                   \
  version="[11]"                                \
  targetClass="Belle2::FileMetaData"            \
  target="m_particleSource"                     \
  code="{m_particleSource = m_particleType;}"   \

#pragma read                                    \
  sourceClass="Belle2::Particle"                \
  source=""                                     \
  version="[-12]"                               \
  targetClass="Belle2::Particle"                \
  target="m_momentumScale"                      \
  code="{m_momentumScale = 1.0;}"               \

#pragma read                                    \
  sourceClass="Belle2::TagVertex"               \
  source="float m_MCdeltaT"                     \
  version="[-3]"                                \
  targetClass="Belle2::TagVertex"               \
  target="m_mcDeltaT"                           \
  code="{m_mcDeltaT = onfile.m_MCdeltaT;}"      \

#pragma read                                    \
  sourceClass="Belle2::TagVertex"               \
  source="TVector3 m_MCtagV"                    \
  version="[-3]"                                \
  targetClass="Belle2::TagVertex"               \
  target="m_mcTagV"                             \
  code="{m_mcTagV = onfile.m_MCtagV;}"          \

#pragma read                                                   \
  sourceClass="Belle2::TagVertex"                              \
  source="TMatrixFSym m_tagVertexErrMatrix"                    \
  version="[-3]"                                               \
  targetClass="Belle2::TagVertex"                              \
  target="m_tagVertexErrMatrix"                                \
  code="{m_tagVertexErrMatrix = onfile.m_tagVertexErrMatrix;}" \

#endif
