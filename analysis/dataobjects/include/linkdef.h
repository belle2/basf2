#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Particle;
#pragma link C++ class vector<Belle2::Particle*>;
#pragma link C++ class Belle2::ParticleList;
#pragma link C++ class Belle2::ParticleInfo;

#pragma link C++ class Belle2::ParticleExtraInfoMap+;
#pragma link C++ class map<string, unsigned int>+;
#pragma link C++ class vector<map<string, unsigned int> >+;

#endif
