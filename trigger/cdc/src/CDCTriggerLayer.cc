//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerLayer.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire layer.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define CDCTriggerLayer_INLINE_DEFINE_HERE

#include <iostream>
#include "trigger/cdc/CDCTrigger.h"
#include "trigger/cdc/CDCTriggerWire.h"
#include "trigger/cdc/CDCTriggerLayer.h"

namespace Belle2 {

CDCTriggerLayer::CDCTriggerLayer(unsigned id,
				 unsigned superLayerId,
				 unsigned localLayerId,
				 unsigned axialStereoLayerId,
				 unsigned axialStereoSuperLayerId,
				 float offset,
				 int nShifts,
				 float cellSize,
				 unsigned nWires)
:  _id(id),
   _superLayerId(superLayerId),
   _localLayerId(localLayerId),
   _axialStereoLayerId(axialStereoLayerId),
   _axialStereoSuperLayerId(axialStereoSuperLayerId),
   _offset(offset),
   _nShifts(nShifts),
   _cellSize(cellSize),
   _nWires(nWires) {
}

CDCTriggerLayer::CDCTriggerLayer(unsigned id,
				 const CDCTriggerWire & w)
:  _id(id),
   _superLayerId(w.superLayerId()),
   _localLayerId(0),
   _axialStereoLayerId(0),
   _axialStereoSuperLayerId(0),
   _offset(w.layer().offset()),
   _nShifts(w.layer().nShifts()),
   _cellSize(w.cellSize()),
   _nWires(w.layer().nWires()) {
}

CDCTriggerLayer::~CDCTriggerLayer() {
}

void
CDCTriggerLayer::dump(const std::string &, const std::string & pre) const {
    std::cout << pre;
    std::cout << "layer " << _id;
    std::cout << ", " << stereoType();
    std::cout << ", super layer " << _superLayerId;
    std::cout << ", local layer " << _localLayerId;
    if (axial()) std::cout << ", axial ";
    else         std::cout << ", stereo ";
    std::cout << _axialStereoLayerId;
    if (axial()) std::cout << ", axial super ";
    else         std::cout << ", stereo super ";
    std::cout << _axialStereoSuperLayerId;
    std::cout << ", " << _nWires << " wires";
    std::cout << std::endl;
    //    for (int i=0;i<_nWires;++i) wire(i)->dump(pre);
}

const CDCTriggerWire * const
CDCTriggerLayer::wire(int id) const {
    if (_nWires == 0) return 0;

    if (id < 0)
	while (id < 0)
	    id += _nWires;

    if (id >= (int) _nWires)
	id %= (int) _nWires;

    return (* this)[id];
}

} // namespace Belle2

