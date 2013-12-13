//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TLayer.cc
// Section  : CDC Tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire layer.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iostream>
#include "tracking/modules/trasan/TLayer.h"
#include "tracking/modules/trasan/TCell.h"

using namespace std;

namespace Belle {

const TLayer * TUndefinedLayer = new TLayer(9999,
					    9999,
					    9999,
					    9999,
					    9999,
					    0,
					    0,
					    9999,
					    9999,
					    0,
					    0);

TLayer::TLayer(unsigned id,
	       unsigned superLayerId,
	       unsigned localLayerId,
	       unsigned axialStereoLayerId,
	       unsigned axialStereoSuperLayerId,
	       float offset,
	       int nShifts,
	       float cellSize,
	       unsigned nCells,
	       float innerRadius,
	       float outerRadius)
    :  _name("unknown"),
       _id(id),
       _superLayerId(superLayerId),
       _localLayerId(localLayerId),
       _axialStereoLayerId(axialStereoLayerId),
       _axialStereoSuperLayerId(axialStereoSuperLayerId),
       _offset(offset),
       _nShifts(nShifts),
       _cellSize(cellSize),
       _nCells(nCells),
       _innerRadius(innerRadius),
       _outerRadius(outerRadius) {
}

TLayer::TLayer(unsigned id,
	       const TCell & w)
    :  _id(id),
       _superLayerId(w.superLayerId()),
       _localLayerId(0),
       _axialStereoLayerId(0),
       _axialStereoSuperLayerId(0),
       _offset(w.layer().offset()),
       _nShifts(w.layer().nShifts()),
       _cellSize(w.cellSize()),
       _nCells(w.layer().nCells()) {
}

TLayer::~TLayer() {
}

void
TLayer::dump(const string &, const string & pre) const {
    cout << pre;
    cout << "layer " << _id;
    cout << ", " << stereoType();
    cout << ", super layer " << _superLayerId;
    cout << ", local layer " << _localLayerId;
    if (axial()) cout << ", axial ";
    else         cout << ", stereo ";
    cout << _axialStereoLayerId;
    if (axial()) cout << ", axial super ";
    else         cout << ", stereo super ";
    cout << _axialStereoSuperLayerId;
    cout << ", " << _nCells << " wires";
    cout << endl;
    //    for (int i=0;i<_nCells;++i) wire(i)->dump(pre);
}

const TCell &
TLayer::cell(int id) const {
    if (_nCells == 0) {
	cout << "TLayer !!! This has no cell member : " << name() << endl;
	return * (TCell *)(TUndefinedLayer);
    }

    if (id < 0)
        while (id < 0)
            id += _nCells;

    if (id >= (int) _nCells)
        id %= (int) _nCells;

    return * (* this)[id];
}

} // namespace Belle
