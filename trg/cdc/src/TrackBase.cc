//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackBase.cc
// Section  : TrackBaseing
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track object in TRGCDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "trg/cdc/TrackBase.h"
#include "trg/cdc/TrackSegment.h"

using namespace std;

namespace Belle2 {

TRGCDCTrackBase::TRGCDCTrackBase(const string & name, float charge)
    : _name(name),
      _status(0),
      _charge(charge) {
}

TRGCDCTrackBase::~TRGCDCTrackBase() {
}

void
TRGCDCTrackBase::dump(const string & cmd, const string & pre) const {
    cout << pre;
    if (cmd.find("detail") != string::npos)
        cout << name() << ":" << status() << ":" << _p << ":" << _v << endl;
    cout << pre << "ts:";
    for (unsigned i = 0; i < TRGCDCTrackBase_Max_Layers; i++) {
        cout << i << " " << _ts[i].size();
        for (unsigned j = 0; j < _ts[i].size(); j++) {
            if (j == 0) cout << "(";
            else cout << ",";
            cout << _ts[i][j]->id();
        }
        if (_ts[i].size())
            cout << ")";
        cout << ":";
    }
    cout << endl;
}

void
TRGCDCTrackBase::append(const TRGCDCTrackSegment * a) {
    _ts[a->superLayerId()].push_back(a);
}

const std::vector<const TRGCDCTrackSegment *> &
TRGCDCTrackBase::trackSegments(unsigned layerId) const {
    return _ts[layerId];
}

} // namespace Belle2
