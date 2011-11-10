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

#define TRGCDC_SHORT_NAMES

#include "trg/trg/Debug.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/TrackBase.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Fitter.h"

using namespace std;

namespace Belle2 {

TRGCDCTrackBase::TRGCDCTrackBase(const string & name, float charge)
    : _name(name),
      _status(0),
      _charge(charge),
      _ts(0),
      _nTs(TRGCDC::getTRGCDC()->nSuperLayers()),
      _fitter(0),
      _fitted(false) {
//  _ts = new vector<const TCLink *>[_nTs + 1];
    _ts = new vector<TCLink *>[_nTs + 1];

//     TCLink * t = 0;
//     for (unsigned i = 0; i < n + 1; i++)
// 	_ts[i].push_back(t);

}

TRGCDCTrackBase::~TRGCDCTrackBase() {
    delete[] _ts;
}

void
TRGCDCTrackBase::dump(const string & cmd, const string & pre) const {
    const string tab = TRGDebug::tab() + pre;

    if (cmd.find("detail") != string::npos)
        cout << tab << name() << ":status=" << status() << ":p="
	     << _p << ":v=" << _v << endl;
    cout << tab;
    for (unsigned i = 0; i < _nTs - 1; i++) {
	cout << i << "=" << _ts[i].size();
        for (unsigned j = 0; j < _ts[i].size(); j++) {
            if (j == 0)
		cout << "(";
            else
		cout << ",";
	    const TCLink & l = * _ts[i][j];
	    cout << l.wire()->name();
        }
        if (_ts[i].size())
            cout << ")";
    }
    cout << endl;
}

void
TRGCDCTrackBase::append(TRGCDCLink * a) {
//    (* _ts)[a->wire()->superLayerId()]->push_back(a);
    _ts[a->wire()->superLayerId()].push_back(a);
    _ts[_nTs - 1].push_back(a);
}

void
TRGCDCTrackBase::append(const vector<TCLink *> links) {
    for (unsigned i = 0; i < links.size(); i++) {
	append(links[i]);
    }
}

const std::vector<TRGCDCLink *> &
TRGCDCTrackBase::links(void) const {
    return _ts[_nTs - 1];
}

const std::vector<TRGCDCLink *> &
TRGCDCTrackBase::links(unsigned layerId) const {
    return _ts[layerId];
}

int
TRGCDCTrackBase::fit(void) {
    if (_fitter) {
	return _fitter->fit(* this);
    }
    else {
	cout << "TRGCDCTrackBase !!! no fitter available" << endl;
	return -1;
    }
}

} // namespace Belle2
