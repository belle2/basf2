#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::RelationElement+;
#pragma link C++ class Belle2::RelationContainer+;
#pragma link C++ class Belle2::EventMetaData+;
#pragma link C++ class Belle2::FileMetaData+;
#pragma link C++ class Belle2::ProfileInfo+;
#pragma link C++ class Belle2::MCInitialParticles+;
#pragma link C++ class Belle2::BeamParameters+;
#pragma link C++ class Belle2::DigitBase+;

#pragma link C++ class Belle2::SimpleVec<int>+;
#pragma link C++ class Belle2::SimpleVec<double>+;
#pragma link C++ class Belle2::SimpleVec<float>+;
#pragma link C++ class Belle2::SimpleVec<char>+;

#pragma link C++ class Belle2::Helix+;
#pragma link C++ class Belle2::UncertainHelix+;

#pragma read sourceClass="Belle2::FileMetaData" version="[-3]" \
  source="unsigned int m_randomSeed" \
  targetClass="Belle2::FileMetaData" target="m_randomSeed" \
  code="{m_randomSeed = TString::Format("%d", onfile.m_randomSeed);}"
#pragma read sourceClass="Belle2::FileMetaData" version="[-3]" \
  source="int m_events" \
  targetClass="Belle2::FileMetaData" target="m_nEvents" \
  code="{m_nEvents = onfile.m_events;}"

#endif
