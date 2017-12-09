#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Particle+;
#pragma link C++ class Belle2::EventExtraInfo+;
#pragma link C++ class Belle2::ParticleList+;
#pragma link C++ class Belle2::RestOfEvent+;
#pragma link C++ class Belle2::Vertex+;
#pragma link C++ class Belle2::ContinuumSuppression+;
#pragma link C++ class Belle2::FlavorTaggerInfo+;
#pragma link C++ class Belle2::FlavorTaggerInfoMap+;
#pragma link C++ class Belle2::StringWrapper+;
#pragma link C++ class Belle2::ThrustOfEvent+;
//these two are needed when using these types (returned by Particle) in PyROOT
#pragma link C++ class vector<Belle2::Particle*>-;
#pragma link C++ class vector<const Belle2::Particle*>-;

#pragma link C++ class Belle2::ParticleExtraInfoMap+;
#pragma link C++ class Belle2::ParticleExtraInfoMap::IndexMap+;
#pragma link C++ class map<string, unsigned int>+;
#pragma link C++ class vector<map<string, unsigned int> >+;

#endif
