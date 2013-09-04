#ifdef __CINT__
#include <framework/gearbox/Const.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Track;
#pragma link C++ class Belle2::TrackFitResult;
#pragma link C++ class Belle2::TrackingOutput;
#pragma link C++ class Belle2::ExtHit;
#pragma link C++ class Belle2::Muid;
#pragma link C++ class Belle2::MuidHit;
#pragma link C++ class Belle2::VXDTFInfoBoard;
#pragma link C++ class Belle2::VXDTFSecMap+;
// #pragma link C++ class pair< double, double >+;
// #pragma link C++ class pair< unsigned int, pair< double, double > >+;
// #pragma link C++ class vector< pair< unsigned int, pair< double, double > > >+;
// #pragma link C++ class pair< unsigned int, vector< pair< unsigned int, pair< double, double > > > >+;
// #pragma link C++ class vector< pair< unsigned int, pair< unsigned int, vector< pair< unsigned int, pair< double, double > > > > > >+;
// #pragma link C++ class pair< unsigned int, vector< pair< unsigned int, pair< unsigned int, vector< pair< unsigned int, pair< double, double > > > > > > >+;
// #pragma link C++ class vector< pair< unsigned int, vector< pair< unsigned int, pair< unsigned int, vector< pair< unsigned int, pair< double, double > > > > > > > >+;
// #pragma link C++ class pair < double, double >+;
#pragma link C++ class pair <unsigned int, pair < double, double > >+;
#pragma link C++ class vector < pair <unsigned int, pair < double, double > > >+;
#pragma link C++ class pair < unsigned int, vector < pair <unsigned int, pair < double, double > > > >+;
#pragma link C++ class vector < pair < unsigned int, vector < pair <unsigned int, pair < double, double > > > > >+;
#pragma link C++ class pair < unsigned int, vector < pair < unsigned int, vector < pair <unsigned int, pair < double, double > > > > > >+;
#pragma link C++ class vector < pair < unsigned int, vector < pair < unsigned int, vector < pair <unsigned int, pair < double, double > > > > > > >+;

#pragma link C++ class Belle2::ROIid;
#pragma link C++ class Belle2::PXDIntercept;

#endif
