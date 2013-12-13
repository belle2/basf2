//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TCell.cc
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a cell (a wire or a track segment) in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "tracking/modules/trasan/TCDC.h"
#include "tracking/modules/trasan/TCell.h"

#define P3D HepGeom::Point3D<double>

using namespace std;

namespace Belle {

TCell::TCell(unsigned id,
	     unsigned localId,
	     const TLayer & l,
	     const P3D & fp,
	     const P3D & bp)
    : _id(id),
      _localId(localId),
      _layer(l),
      _xyPosition(0.5 * (fp + bp)),
      _forwardPosition(fp),
      _backwardPosition(bp),
      _direction((fp - bp).unit()),
      _state(0),
      _hit(0) {
}

TCell::~TCell() {
}

void
TCell::dump(const string & msg, const string & pre) const {
    cout << pre;
    cout << "w " << _id;
    cout << ",local " << _localId;
    cout << ",layer " << layerId();
    cout << ",super layer " << superLayerId();
    cout << ",local layer " << localLayerId();
    cout << endl;
    if (msg.find("neighbor") != string::npos) {
//        for (unsigned i = 0; i < 7; i++)
//             if (neighbor(i))
//                 neighbor(i)->dump("",
//        pre + TDebugUtilities::itostring(i) + "   ");
    }    
}
  
int
TCell::localIdDifference(const TCell & a) const {

    if (superLayerId() != a.superLayerId()) {
        cout << "TCell::localIdDifference !!!";
        cout << "super layer assumption violation" << endl;
    }

    int diff = int(a.localId()) - int(localId());
    unsigned nCells = layer().nCells();
    if (diff > 0) {
        int difR = nCells - diff;
        if (diff < difR) return diff;
        else return - difR;
    }
    else {
        int difR = nCells + diff;
        if (- diff < difR) return diff;
        else return difR;
    }
}

} // namespace Belle
