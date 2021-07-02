#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

//CINT doesn't like the bitsets when using '+' :/
#pragma link C++ class Belle2::VxdID-; // checksum=0x53ef1299, version=-1
#pragma link C++ class Belle2::VXDElectronDeposit+; // checksum=0xb2ed2d88, version=-1
#pragma link C++ class std::vector<Belle2::VxdID>+; // checksum=0x2638001a, version=6
#pragma link C++ class Belle2::VXDSimHit+; // checksum=0x71cca24a, version=1
#pragma link C++ class Belle2::VXDTrueHit+; // checksum=0x6f9f06b5, version=6
#endif
