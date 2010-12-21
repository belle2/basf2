//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackSegment.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDCTrackSegment_INLINE_DEFINE_HERE
#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/TrackSegment.h"

using namespace std;

#define P3D HepGeom::Point3D<double>

namespace Belle2 {

TRGCDCTrackSegment::TRGCDCTrackSegment(unsigned id,
				       const TCWire & w,
				       const TRGCDCLayer * layer,
			     const std::vector<const TRGCDCWire *> & cells)

    : TCWire::TCWire(w),
      _state(0),
      _id(id),
      _localId(w.localId()),
      _layer(layer),
      _wires(cells),
      _signal(std::string("TS_") + TRGUtil::itostring(id)) {
}

TRGCDCTrackSegment::~TRGCDCTrackSegment() {
}
 
void
TRGCDCTrackSegment::dump(const string & msg,
			     const string & pre) const {
    cout << pre << name() << endl;
    if ((msg.find("geometry") != string::npos) ||
	(msg.find("detail") != string::npos)) {
	cout << pre << "w " << _id;
	cout << ",local " << _localId;
	cout << ",layer " << layerId();
	cout << ",super layer " << superLayerId();
	cout << ",local layer " << localLayerId();
	cout << endl;
    }
//     if (msg.find("neighbor") != string::npos ||
// 	msg.find("detail") != string::npos) {
// 	for (unsigned i = 0; i < 7; i++)
// 	    if (neighbor(i))
// 		neighbor(i)->dump("", pre + TRGCDC::itostring(i) + "   ");
//     }
    if ((msg.find("trigger") != string::npos) ||
	(msg.find("detail") != string::npos) ){
	if (_signal.active())
	    _signal.dump(msg, pre);
	else
	    cout << pre << "no trigger signal" << endl;
    }
}
  
void
TRGCDCTrackSegment::clear(void) {
    _state = 0;
    _signal.clear();
}

string
TRGCDCTrackSegment::name(void) const {
    string t;
    if (axial())
	t = "-";
    else
	t = "=";
    string n0 = string("TS") + TRGUtil::itostring(layerId());
    string n1 = TRGUtil::itostring(_localId);
    return n0 + t + n1;
}

void
TCTSegment::simulate(void) {

#ifdef TRGCDC_DEBUG
//    cout << name() << endl;
#endif

    //...Get wire informtion...
    const unsigned n = _wires.size();
    unsigned nHits = 0;
    vector<TRGSignal> signals;
    for (unsigned i = 0; i < n; i++) {
	const TRGSignal & s = _wires[i]->triggerOutput();
	signals.push_back(s);

	if (s.active()) {
	    ++nHits;
#ifdef TRGCDC_DEBUG
//	    s->dump("", "    ");
#endif
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
#ifdef TRGCDC_DEBUG
//	signals[i].dump("", "    ");
#endif
    }

    //...Simple simulation assuming 3:2:1:2:3 shape...
    TRGSignal l0 = signals[0] | signals[1] | signals[2];
    TRGSignal l1 = signals[3] | signals[4];
    TRGSignal l2 = signals[5];
    TRGSignal l3 = signals[6] | signals[7];
    TRGSignal l4 = signals[8] | signals[9] | signals[10];

    //...Coincidence of all layers...
    TRGSignal all = l0 & l1 & l2 & l3 & l4;

    if (all.nEdges())
	_signal = all;

#ifdef TRGCDC_DEBUG
//     l0.dump("", "       -> ");
//     l1.dump("", "       -> ");
//     l2.dump("", "       -> ");
//     l3.dump("", "       -> ");
//     l4.dump("", "       -> ");
//     if (all.nEdges())
// 	cout << "===========" << endl;
//     all.dump("", "    ----> ");
#endif
    
}

} // namespace Belle2
