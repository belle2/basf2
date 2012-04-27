//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : SegmentHit.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track segment hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"

using namespace std;

namespace Belle2 {

vector<TRGCDCSegmentHit *> TRGCDCSegmentHit::_all;

TRGCDCSegmentHit::TRGCDCSegmentHit(const TCSegment & w)
    : TCCHit(w) {
}

TRGCDCSegmentHit::~TRGCDCSegmentHit() {
}

int
TRGCDCSegmentHit::sortById(const TRGCDCSegmentHit ** a,
			   const TRGCDCSegmentHit ** b) {
    if ((* a)->cell().id() > (* b)->cell().id())
        return 1;
    else if ((* a)->cell().id() == (* b)->cell().id())
        return 0;
    else
        return -1;
}

void
TRGCDCSegmentHit::dump(const std::string & message,
		       const std::string & prefix) const {
    TCCHit::dump(message, prefix);
}

const TRGCDCSegment &
TRGCDCSegmentHit::segment(void) const {
    return dynamic_cast<const TRGCDCSegment &>(cell());
}

void
TRGCDCSegmentHit::removeAll(void) {
    while (_all.size())
	delete _all.back();
}

void *
TRGCDCSegmentHit::operator new(size_t size) {
    void * p = malloc(size);
    _all.push_back((TRGCDCSegmentHit *) p);

//     cout << ">---------------------" << endl;
//     for (unsigned i = 0; i < _all.size(); i++)
// 	cout << "> " << i << " " << _all[i] << endl;

    return p;
}

void
TRGCDCSegmentHit::operator delete(void * t) {
    for (vector<TRGCDCSegmentHit *>::iterator it = _all.begin();
	 it != _all.end();
	 it++) {
	if ((* it) == (TRGCDCSegmentHit *) t) {
	    _all.erase(it);
	    break;
	}
    }
    free(t);

//     cout << "<---------------------" << endl;
//     cout << "==> " << t << " erased" << endl;
//     for (unsigned i = 0; i < _all.size(); i++)
// 	cout << "< " << i << " " << _all[i] << endl;
}

} // namespace Belle2
