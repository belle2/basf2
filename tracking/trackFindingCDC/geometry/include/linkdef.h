
#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;


#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::Vector2D+;
#pragma link C++ class Belle2::TrackFindingCDC::Vector3D+;

#pragma link C++ class Belle2::TrackFindingCDC::Line2D+;
#pragma link C++ class Belle2::TrackFindingCDC::Circle2D+;
#pragma link C++ class Belle2::TrackFindingCDC::GeneralizedCircle+;

#pragma link C++ class Belle2::TrackFindingCDC::ParameterLine2D+;
#pragma link C++ defined_in "LineParameters.h";
#pragma link C++ class Belle2::TrackFindingCDC::UncertainParameterLine2D+;

#pragma link C++ class Belle2::TrackFindingCDC::PerigeeCircle+;
#pragma link C++ defined_in "PerigeeParameters.h";
#pragma link C++ class Belle2::TrackFindingCDC::UncertainPerigeeCircle+;

#pragma link C++ class Belle2::TrackFindingCDC::SZLine+;
#pragma link C++ defined_in "SZParameters.h";
#pragma link C++ class Belle2::TrackFindingCDC::UncertainSZLine+;

#pragma link C++ class Belle2::TrackFindingCDC::Helix+;
#pragma link C++ defined_in "HelixParameters.h";
#pragma link C++ class Belle2::TrackFindingCDC::UncertainHelix+;

#endif
