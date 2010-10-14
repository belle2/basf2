//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerTrackSegment.cc
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define CDCTriggerTrackSegment_INLINE_DEFINE_HERE

#include "trigger/cdc/CDCTrigger.h"
#include "trigger/cdc/CDCTriggerTrackSegment.h"

#define P3D HepGeom::Point3D<double>

namespace Belle2 {

CDCTriggerTrackSegment::CDCTriggerTrackSegment(const CDCTriggerWire & w,
					       unsigned n,
					       const int * shape,
					       const CDCTriggerLayer * layer)
    : CDCTriggerWire::CDCTriggerWire(w),
      _state(0),
      _id(),
      _localId(),
      _layer(layer) {
}

CDCTriggerTrackSegment::~CDCTriggerTrackSegment() {
}

void
CDCTriggerTrackSegment::dump(const std::string & msg, const std::string & pre) const {
    std::cout << pre;
    std::cout << "w " << _id;
    std::cout << ",local " << _localId;
    std::cout << ",layer " << layerId();
    std::cout << ",super layer " << superLayerId();
    std::cout << ",local layer " << localLayerId();
    std::cout << std::endl;
    if (msg.find("neighbor") != std::string::npos ||
	msg.find("detail") != std::string::npos) {
	for (unsigned i = 0; i < 7; i++)
	    if (neighbor(i))
		neighbor(i)->dump("", pre + CDCTrigger::itostring(i) + "   ");
    }    
}
  
void
CDCTriggerTrackSegment::clear(void) {
    _state = 0;
}

std::string
CDCTriggerTrackSegment::name(void) const {
    if (axial())
	return CDCTrigger::itostring(layerId()) + std::string("-") + CDCTrigger::itostring(_localId);
    return CDCTrigger::itostring(layerId()) + std::string("=") + CDCTrigger::itostring(_localId);
}

} // namespace Belle2

