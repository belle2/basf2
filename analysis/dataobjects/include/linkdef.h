#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Particle+;
#pragma link C++ class Belle2::EventExtraInfo+;
#pragma link C++ class Belle2::ParticleList+;
#pragma link C++ class Belle2::RestOfEvent+;
#pragma link C++ class Belle2::TagVertex+;
#pragma link C++ class Belle2::ContinuumSuppression+;
#pragma link C++ class Belle2::FlavorTaggerInfo+;
#pragma link C++ class Belle2::FlavorTaggerInfoMap+;
#pragma link C++ class Belle2::StringWrapper+;
#pragma link C++ class Belle2::EventShapeContainer+;
#pragma link C++ class Belle2::EventKinematics+;
#pragma link C++ class Belle2::TauPairDecay+;
#pragma link C++ class Belle2::ECLEnergyCloseToTrack+;
#pragma link C++ class Belle2::ECLTRGInformation+;
#pragma link C++ class Belle2::ECLTriggerCell+;
//these two are needed when using these types (returned by Particle) in PyROOT
#pragma link C++ class vector<Belle2::Particle*>-;
#pragma link C++ class vector<const Belle2::Particle*>-;

#pragma link C++ class Belle2::ParticleExtraInfoMap+;
#pragma link C++ class Belle2::ParticleExtraInfoMap::IndexMap+;
#pragma link C++ class map<string, unsigned int>+;
#pragma link C++ class vector<map<string, unsigned int> >+;
#pragma link C++ class Belle2::Btube+;

#pragma read                                    \
  sourceClass="Belle2::Particle"                \
  source="int m_particleType"                   \
  version="[11]"                                \
  targetClass="Belle2::FileMetaData"            \
  target="m_particleSource"                     \
  code="{m_particleSource = m_particleType;}"   \

#endif
