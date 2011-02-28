//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Track.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a reconstructed charged track in TRGCDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/cdc/Track.h"

using namespace std;

namespace Belle2 {

vector<const TRGCDCTrack *>
TRGCDCTrack::_list = vector<const TRGCDCTrack *>();

TRGCDCTrack::TRGCDCTrack()
    : TCTBase("unknown", 0) {
}

TRGCDCTrack::~TRGCDCTrack() {
    if (_list.size()) {
        for (unsigned i = 0; i < _list.size(); i++)
            if (_list[i] == this)
                _list.erase(_list.begin(), _list.begin() + i);
    }
}

vector<const TRGCDCTrack *>
TRGCDCTrack::list(void) {
    vector<const TRGCDCTrack *> t;
    t.assign(_list.begin(), _list.end());
    return t;
}

} // namespace Belle2
