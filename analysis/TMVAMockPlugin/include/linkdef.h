#ifdef __CINT__

#pragma link off all global;
#pragma link off all class;
#pragma link off all function;
#pragma link off all namespace;
#pragma link C++ nestedclasses;

// Dirty hack
// We cannot define the real MethodMockPlugin in the Belle2 namespace,
// because TMVA explicitly searches for a TMVA::MethodBase class.
// Our scons system explicitly searches for Belle2:: namespace classes
// constructs the corresponding filenames and passes them to rootcint (see root_dict.py).
// Therefore we must define a Belle2::MethodMockPlugin which is found by our scons system and
// adds the correct filename to the files processed by rootcint for the real TMVA::MethodMockPlugin.
#pragma link C++ class Belle2::MethodMockPlugin+;
#pragma link C++ class TMVA::MethodMockPlugin+;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#endif
