#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackFindingCDC::SinEqLine-;

// added template specialization for the matrix class which is used
// in the UncertaintyHelix which needs to be streamable
#pragma link C++ class Belle2::TrackFindingCDC::PlainMatrix<double,5,5>+; // checksum=0x28d57919, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::PlainMatrix<double,3,3>+; // checksum=0x8e08b6d3, version=-1
