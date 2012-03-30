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

TRGCDCTrackBase::TRGCDCTrackBase(const TRGCDCTrackBase & t)
    : _name("CopyOf" + t._name),
      _status(t._status),
      _charge(t._charge),
      _ts(0),
      _tsAll(t._tsAll),
      _nTs(t._nTs),
      _fitter(t._fitter),
      _fitted(t._fitted) {
    _ts = new vector<TCLink *>[_nTs];
    for (unsigned i = 0; i < _nTs; i++)
	_ts[i].assign(t._ts[i].begin(), t._ts[i].end());
}

TRGCDCTrackBase::TRGCDCTrackBase(const string & name, double charge)
    : _name(name),
      _status(0),
      _charge(charge),
      _ts(0),
      _nTs(TRGCDC::getTRGCDC()->nSuperLayers()),
      _fitter(0),
      _fitted(false) {
    _ts = new vector<TCLink *>[_nTs];
}

TRGCDCTrackBase::~TRGCDCTrackBase() {
    delete[] _ts;
    _tsAll.clear();
}

void
TRGCDCTrackBase::dump(const string & , const string & pre) const {
//  bool detail = (cmd.find("detail") != string::npos);

    string tab = TRGDebug::tab() + pre;
    cout << tab << "Dump of " << name() << endl;
    tab += "    ";

    cout << tab << "status=" << status() << ":p=" << p() << ":x=" << x()
	 << endl;
    cout << tab;
    for (unsigned i = 0; i < _nTs; i++) {
	cout << i << ":" << _ts[i].size();
        for (unsigned j = 0; j < _ts[i].size(); j++) {
            if (j == 0)
		cout << "(";
            else
		cout << ",";
	    const TCLink & l = * _ts[i][j];
	    cout << l.wire()->name();
        }
        if (_ts[i].size())
            cout << "),";
    }
//     if (detail) {

//     }
    cout << endl;
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

void
TRGCDCTrackBase::append(TCLink * a) {
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

const std::vector<TCLink *> &
TRGCDCTrackBase::links(void) const {
    return _tsAll;
}

const std::vector<TCLink *> &
TRGCDCTrackBase::links(unsigned layerId) const {
    return _ts[layerId];
}

int
TRGCDCTrackBase::approach2D(TCLink &) const {
    cout << "TRGCDCTrackBase::approach2D !!! not implemented" << endl;
    return -1;
}


} // namespace Belle2
