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
    _ts = new vector<TCLink *>[_nTs];

//    cout << ">>> " << _ts[0].size() << "," << _ts[1].size() << "," << _ts[2].size() << endl;
}

TRGCDCTrackBase::~TRGCDCTrackBase() {
    delete[] _ts;
    _tsAll.clear();
}

void
TRGCDCTrackBase::dump(const string & cmd, const string & pre) const {
    string tab = TRGDebug::tab() + pre;
    cout << tab << "Dump of " << name() << endl;
    tab += "    ";

    cout << tab << "status=" << status() << ":p=" << p() << ":x=" << x()
	 << endl;
    cout << tab;
    for (unsigned i = 0; i < _nTs; i++) {
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
    _tsAll.push_back(a);
}

void
TRGCDCTrackBase::append(const vector<TCLink *> links) {
    for (unsigned i = 0; i < links.size(); i++) {
	append(links[i]);
    }
}

const std::vector<TRGCDCLink *> &
TRGCDCTrackBase::links(void) const {
    return _tsAll;
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
