#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::CalcMeanCov<2, float>+; //implicit
#pragma link C++ class Belle2::CalcMeanCov<2, double>+; //implicit
#pragma link C++ class Belle2::ModuleStatistics+;
#pragma link C++ class vector<Belle2::ModuleStatistics>+;
#pragma link C++ class Belle2::ProcessStatistics+;
#pragma link C++ class Belle2::Environment+;
#pragma link C++ class Belle2::RandomGenerator+;
#pragma link C++ class Belle2::MetadataService-;

#endif
