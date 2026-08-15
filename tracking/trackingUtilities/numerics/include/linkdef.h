#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackingUtilities::SinEqLine-;

// added template specialization for the matrix class which is used
// in the UncertaintyHelix which needs to be streamable
#pragma link C++ class Belle2::TrackingUtilities::PlainMatrix<double,5,5>+; // checksum=0xfe5b20c2, version=-1
#pragma link C++ class Belle2::TrackingUtilities::PlainMatrix<double,3,3>+; // checksum=0x7fdfeeb6, version=-1
