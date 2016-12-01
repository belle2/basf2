#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Mergeable+;

//#pragma link C++ class Belle2::MapMergeable<map<int, unsigned int> >+;
#pragma link C++ class Belle2::SetMergeable<std::unordered_set<unsigned long, std::hash<unsigned long>, std::equal_to<unsigned long>, std::allocator<unsigned long> >>+;

#pragma link C++ class Belle2::ProcHandler;

#pragma link C++ class Belle2::RootMergeable<TTree>+;
#pragma link C++ class Belle2::RootMergeable<TNtuple>+;
#pragma link C++ class Belle2::RootMergeable<TH1F>+;
#pragma link C++ class Belle2::RootMergeable<TH1D>+;
#pragma link C++ class Belle2::RootMergeable<TH2F>+;
#pragma link C++ class Belle2::RootMergeable<TH2D>+;

#endif
