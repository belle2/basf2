#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::RelationElement+; // checksum=0x70423e46, version=1
#pragma link C++ class Belle2::RelationContainer+; // checksum=0x66dbc4f5, version=1
#pragma link C++ class Belle2::EventMetaData+; // checksum=0xb2c44743, version=4
#pragma link C++ class Belle2::FileMetaData+; // checksum=0xcccf5ab6, version=11
#pragma link C++ class Belle2::ProfileInfo+; // checksum=0xf2e3190a, version=2
#pragma link C++ class Belle2::MCInitialParticles+; // checksum=0x77c00ada, version=5
#pragma link C++ class Belle2::DigitBase+; // checksum=0xd2fac950, version=2
#pragma link C++ class Belle2::MergedEventConsistency+; // checksum=0x39886d50, version=1
#pragma link C++ class Belle2::EventExtraInfo+; // checksum=0x6e52097a, version=3

#pragma link C++ class Belle2::Helix+; // checksum=0xc11c13b7, version=2
#pragma link C++ class Belle2::UncertainHelix+; // checksum=0xe2ab4508, version=2
#pragma link C++ class Belle2::EventT0+; // checksum=0xa507415a, version=5
#pragma link C++ class Belle2::EventT0::EventT0Component+; // checksum=0x2ca9debe, version=3
#pragma link C++ class Belle2::BinnedEventT0+; // checksum=0x17a28b5d, version=1
#pragma link C++ class Belle2::TestChunkData+; // checksum=0x25c3db4e, version=1

#pragma link C++ class Belle2::BackgroundMetaData+; // checksum=0x60d78a20, version=4
#pragma link C++ class Belle2::BackgroundInfo+; // checksum=0x6eb1ad8, version=4

#pragma read sourceClass="Belle2::FileMetaData" version="[-3]" \
  source="unsigned int m_randomSeed" \
  targetClass="Belle2::FileMetaData" target="m_randomSeed" \
  code="{m_randomSeed = TString::Format("%d", onfile.m_randomSeed);}"
#pragma read sourceClass="Belle2::FileMetaData" version="[-4]" \
  source="int m_events" \
  targetClass="Belle2::FileMetaData" target="m_nEvents" \
  code="{m_nEvents = onfile.m_events;}"

#pragma link C++ class Belle2::DisplayData+; // checksum=0x8b755e12, version=5
#pragma link C++ class std::vector<TVector3>+; // checksum=0x907dc885, version=6
#pragma link C++ class std::map<string, vector<TVector3>>+; // checksum=0xd74970c5, version=6
#pragma link C++ class std::map<string, vector<ROOT::Math::XYZVector>>+; // checksum=0x91d6f4ec, version=6
#pragma link C++ class std::pair<string, TVector3>+; // checksum=0x4d496280, version=-1
#pragma link C++ class std::vector<std::pair<std::string, TVector3>>+; // checksum=0x1998b952, version=6
#pragma link C++ class std::vector<std::pair<std::string, ROOT::Math::XYZVector>>+; // checksum=0xa2b6087d, version=6
#pragma link C++ class std::vector<TH1*>+; // checksum=0x1c8a598, version=6
#pragma link C++ class Belle2::DisplayData::Arrow+; // checksum=0xaf5e21d6, version=-1
#pragma link C++ class std::vector<Belle2::DisplayData::Arrow>+; // checksum=0xf328c034, version=6
#pragma link C++ class std::pair<Belle2::Const::EDetector, std::map<std::string, int>>+; // checksum=0x32be0e15, version=-1
#pragma link C++ class std::pair<Belle2::Const::EDetector, std::map<std::string, double>>+; // checksum=0x7add857, version=-1


#pragma read                                                                                                              \
  sourceClass="Belle2::MCInitialParticles"                                                                                \
  source="TLorentzVector m_her"                                                                                           \
  version="[-3]"                                                                                                          \
  targetClass="Belle2::MCInitialParticles"                                                                                \
  target="m_her"                                                                                                          \
  include="TLorentzVector.h"                                                                                              \
  code="{m_her = ROOT::Math::PxPyPzEVector(onfile.m_her.Px(), onfile.m_her.Py(), onfile.m_her.Pz(), onfile.m_her.E());}"  \

#pragma read                                                                                                              \
  sourceClass="Belle2::MCInitialParticles"                                                                                \
  source="TLorentzVector m_ler"                                                                                           \
  version="[-3]"                                                                                                          \
  targetClass="Belle2::MCInitialParticles"                                                                                \
  target="m_ler"                                                                                                          \
  include="TLorentzVector.h"                                                                                              \
  code="{m_ler = ROOT::Math::PxPyPzEVector(onfile.m_ler.Px(), onfile.m_ler.Py(), onfile.m_ler.Pz(), onfile.m_ler.E());}"  \

#pragma read                                \
  sourceClass="Belle2::MCInitialParticles"  \
  source="TVector3 m_vertex"                \
  version="[-3]"                            \
  targetClass="Belle2::MCInitialParticles"  \
  target="m_vertex"                         \
  code="{m_vertex = ROOT::Math::XYZVector(onfile.m_vertex.X(), onfile.m_vertex.Y(), onfile.m_vertex.Z());}"      \

#pragma read                                                                  \
  sourceClass="Belle2::DisplayData"                                           \
  source="std::map<std::string, std::vector<TVector3>> m_pointSets"           \
  version="[-4]"                                                              \
  targetClass="Belle2::DisplayData"                                           \
  target="m_pointSets"                                                        \
  code="{for (const auto& [key, value] : onfile.m_pointSets) {                \
           std::vector<ROOT::Math::XYZVector> tmp;                            \
           for (const auto& vec : value)                                      \
             tmp.push_back(ROOT::Math::XYZVector(vec.X(), vec.Y(), vec.Z())); \
           m_pointSets[key] = tmp;                                            \
         }                                                                    \
        }"                                                                    \

#pragma read                                                         \
  sourceClass="Belle2::DisplayData"                                  \
  source="std::vector<std::pair<std::string, TVector3>> m_labels"    \
  version="[-4]"                                                     \
  targetClass="Belle2::DisplayData"                                  \
  target="m_labels"                                                  \
  code="{for (const auto& labelPair : onfile.m_labels)               \
           m_labels.emplace_back(labelPair);                         \
        }"                                                           \

#endif
