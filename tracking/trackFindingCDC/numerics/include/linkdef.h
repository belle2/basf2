#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackFindingCDC::SinEqLine-;

// added template specialization for the matrix class which is used
// in the UncertaintyHelix which needs to be streamable
#pragma link C++ class Belle2::TrackFindingCDC::PlainMatrix<double,5,5>+;
#pragma link C++ class Belle2::TrackFindingCDC::PlainMatrix<double,3,3>+;
