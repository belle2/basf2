#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Const-;
#pragma link C++ class Belle2::Const::DetectorSet+;
#pragma link C++ class Belle2::Const::ChargedStable+;
#pragma link C++ class Belle2::Const::PIDDetectors+;
#pragma link C++ class Belle2::Const::RestrictedDetectorSet<Belle2::Const::PIDDetectors>+;

//not meant to be streamed
#pragma link C++ class Belle2::Const::ParticleType-;
#pragma link C++ class Belle2::Const::ParticleSet-;

#endif
