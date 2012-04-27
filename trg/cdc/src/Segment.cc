//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Segment.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/LUT.h"

using namespace std;

#define P3D HepGeom::Point3D<double>

namespace Belle2 {

TRGCDCSegment::TRGCDCSegment(unsigned id,
			     const TCLayer & layer,
			     const TCWire & w,
			     const TCLUT * lut,
			     const std::vector<const TCWire *> & cells)
    : TCCell(id,
	     layer.size(),
	     layer,
	     w.forwardPosition(),
	     w.backwardPosition()),
      _lut(lut),
      _wires(cells),
      _signal(std::string("TS_") + TRGUtil::itostring(id)),
      _hit(0) {
}

TRGCDCSegment::~TRGCDCSegment() {
}
 
void
TRGCDCSegment::dump(const string & msg,
		    const string & pre) const {
    cout << pre << name() << " (ptn=" << hitPattern() << ")" << endl;
    if ((msg.find("geometry") != string::npos) ||
        (msg.find("detail") != string::npos)) {
        cout << pre << "w " << id();
        cout << ",local " << localId();
        cout << ",layer " << layerId();
        cout << ",super layer " << superLayerId();
        cout << ",local layer " << localLayerId();
        cout << endl;
    }
    if ((msg.find("hit") != string::npos) ||
        (msg.find("detail") != string::npos)) {
        cout << pre;
	if (hit()) {
	    cout << "WHit dump" << endl;
	    hit()->dump("", pre + "    ");
	}
	else {
	    cout << "no TSHit" << endl;
	}
	if (_hits.size() == 0) {
	    cout << pre << "no wire hit" << endl;
	}
	else {
	    cout << pre;
	    for (unsigned i = 0; i < _hits.size(); i++) {
		cout << _hits[i]->cell().name();
		if (i < _hits.size() - 1)
		    cout << ",";
		else
		    cout << endl;
	    }
	}
    }
//     if (msg.find("neighbor") != string::npos ||
//         msg.find("detail") != string::npos) {
//         for (unsigned i = 0; i < 7; i++)
//             if (neighbor(i))
//                 neighbor(i)->dump("", pre + TRGCDC::itostring(i) + "   ");
//     }
    if ((msg.find("trigger") != string::npos) ||
        (msg.find("detail") != string::npos) ){
        if (_signal.active())
            _signal.dump(msg, pre + "    ");
        else
            cout << pre << "no trigger signal" << endl;
    }
}
  
void
TRGCDCSegment::clear(void) {
    _signal.clear();
    TCCell::clear();
    _hits.clear();
    _hit = 0;
}

string
TRGCDCSegment::name(void) const {
    string t;
    if (axial())
        t = "-";
    else
        t = "=";
    string n0 = string("TS") + TRGUtil::itostring(layerId());
    string n1 = TRGUtil::itostring(localId());
    return n0 + t + n1;
}

void
TCSegment::simulate(void) {

    //...Get wire informtion...
    const unsigned n = _wires.size();
    unsigned nHits = 0;
    vector<TRGSignal> signals;
    for (unsigned i = 0; i < n; i++) {
        const TRGSignal & s = _wires[i]->triggerOutput();
        signals.push_back(s);

        if (s.active()) {
            ++nHits;
        }
    }

    //...Check number of hit wires...
    if (nHits < 4)
        return;

    //...Widen signal...400 ns
    static const unsigned width =
        TRGCDC::getTRGCDC()->systemClock().unit(400);
    for (unsigned i = 0; i < n; i++) {
        signals[i].widen(width);
    }

    //...Signal simulation...
    TRGSignal l0, l1, l2, l3, l4;
    if (n == 11) {

	//...Simple simulation assuming 3:2:1:2:3 shape...
	l0 = signals[0] | signals[1] | signals[2];
	l1 = signals[3] | signals[4];
	l2 = signals[5];
	l3 = signals[6] | signals[7];
	l4 = signals[8] | signals[9] | signals[10];

    }
    else if (n == 15) {

	//...Simple simulation assuming 1:2:3:4:5 shape...
	l0 = signals[0];
	l1 = signals[1] | signals[2];
	l2 = signals[3] | signals[4] | signals[5];
	l3 = signals[6] | signals[7] | signals[8] | signals[9];
	l4 = signals[10] |signals[11] | signals[12] | signals[13] |signals[14];
    }
    
    //...Coincidence of all layers...
    TRGSignal all = l0 & l1 & l2 & l3 & l4;

    if (all.nEdges())
	_signal = all;

//     l0.dump("", "       -> ");
//     l1.dump("", "       -> ");
//     l2.dump("", "       -> ");
//     l3.dump("", "       -> ");
//     l4.dump("", "       -> ");
//     if (all.nEdges())
//         cout << "===========" << endl;
//     all.dump("", "    ----> ");
    
}

unsigned
TRGCDCSegment::hitPattern(void) const {
    unsigned ptn = 0;
    for (unsigned i = 0; i < _wires.size(); i++) {
        const TRGSignal & s = _wires[i]->triggerOutput();
        if (s.active())
	    ptn |= (1 << i);
    }
    return ptn;
}

} // namespace Belle2
