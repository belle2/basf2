#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Mergeable+; // checksum=0x65043de0, version=0

//#pragma link C++ class Belle2::MapMergeable<map<int, unsigned int> >+;
#pragma link C++ class Belle2::SetMergeable<std::unordered_set<unsigned long, std::hash<unsigned long>, std::equal_to<unsigned long>, std::allocator<unsigned long> >>+; // checksum=0x1e3b1f1f, version=1

#pragma link C++ class Belle2::ProcHandler+; // checksum=0x8d518b56, version=-1

#pragma link C++ class Belle2::RootMergeable<TTree>+; // checksum=0xf6ba9df1, version=2
#pragma link C++ class Belle2::RootMergeable<TNtuple>+; // checksum=0xc7170c49, version=2
#pragma link C++ class Belle2::RootMergeable<TH1F>+; // checksum=0x3453e94d, version=2
#pragma link C++ class Belle2::RootMergeable<TH1D>+; // checksum=0xe79e7241, version=2
#pragma link C++ class Belle2::RootMergeable<TH2F>+; // checksum=0x27641bdf, version=2
#pragma link C++ class Belle2::RootMergeable<TH2D>+; // checksum=0xdaaea4d3, version=2

#endif
