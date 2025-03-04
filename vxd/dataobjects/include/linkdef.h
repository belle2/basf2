#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

//CINT doesn't like the bitsets when using '+' :/
#pragma link C++ class Belle2::VxdID-; // checksum=0x53ef1299, version=-1
#pragma link C++ class Belle2::VXDElectronDeposit+; // checksum=0xb2ed2d88, version=-1
#pragma link C++ class std::vector<Belle2::VxdID>+; // checksum=0x2638001a, version=6
#pragma link C++ class Belle2::VXDSimHit+; // checksum=0x36b527c4, version=2
#pragma link C++ class Belle2::VXDTrueHit+; // checksum=0x122ecf2d, version=7
#endif
