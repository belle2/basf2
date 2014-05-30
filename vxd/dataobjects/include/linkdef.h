#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

//CINT doesn't like the bitsets when using '+' :/
#pragma link C++ class Belle2::VxdID;
#pragma link C++ class Belle2::VXDElectronDeposit;
#pragma link C++ class std::vector<Belle2::VxdID>+;
#pragma link C++ class Belle2::VXDSimHit+;
#pragma link C++ class Belle2::VXDTrueHit+;
#endif
