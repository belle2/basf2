//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerTrackSegment.cc
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define CDCTriggerTrackSegment_INLINE_DEFINE_HERE
#define CDCTRIGGER_SHORT_NAMES

#include <iostream>
#include "trigger/cdc/CDCTrigger.h"
#include "trigger/cdc/CDCTriggerTrackSegment.h"

using namespace std;

#define P3D HepGeom::Point3D<double>

namespace Belle2 {

CDCTriggerTrackSegment::CDCTriggerTrackSegment(unsigned id,
					       const CTWire & w,
					       const CDCTriggerLayer * layer,
			     const std::vector<const CDCTriggerWire *> & cells)

    : CTWire::CTWire(w),
      _state(0),
      _id(id),
      _localId(w.localId()),
      _layer(layer),
      _wires(cells),
      _signal(0) {
}

CDCTriggerTrackSegment::~CDCTriggerTrackSegment() {
}
 
void
CDCTriggerTrackSegment::dump(const string & msg,
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
// 		neighbor(i)->dump("", pre + CDCTrigger::itostring(i) + "   ");
//     }
    if ((msg.find("trigger") != string::npos) ||
	(msg.find("detail") != string::npos) ){
	if (_signal)
	    _signal->dump(msg, pre);
	else
	    cout << pre << "no trigger signal" << endl;
    }
}
  
void
CDCTriggerTrackSegment::clear(void) {
    _state = 0;
    if (_signal)
	delete _signal;
    _signal = 0;
}

string
CDCTriggerTrackSegment::name(void) const {
    string t;
    if (axial())
	t = "-";
    else
	t = "=";
    string n0 = string("TS") + CDCTrigger::itostring(layerId());
    string n1 = CDCTrigger::itostring(_localId);
    return n0 + t + n1;
}

void
CTTSegment::simulate(void) {

#ifdef CDCTRIGGER_DEBUG
//    cout << name() << endl;
#endif

    //...Get wire informtion...
    const unsigned n = _wires.size();
    unsigned nHits = 0;
    vector<GDLSignal> signals;
    for (unsigned i = 0; i < n; i++) {
	const GDLSignal * s = _wires[i]->triggerOutput();

	if (s) {
	    signals.push_back(* s);
	    ++nHits;
#ifdef CDCTRIGGER_DEBUG
//	    s->dump("", "    ");
#endif
	}
	else {
	    signals.push_back(GDLSignal());
	}
    }

    //...Check number of hit wires...
    if (nHits < 4)
	return;

    //...Widen signal...400 ns
    static const unsigned width =
	CDCTrigger::getCDCTrigger()->systemClock().unit(400);
    for (unsigned i = 0; i < n; i++) {
	signals[i].widen(width);
#ifdef CDCTRIGGER_DEBUG
//	signals[i].dump("", "    ");
#endif
    }

    //...Simple simulation assuming 3:2:1:2:3 shape...
    GDLSignal l0 = signals[0] | signals[1] | signals[2];
    GDLSignal l1 = signals[3] | signals[4];
    GDLSignal l2 = signals[5];
    GDLSignal l3 = signals[6] | signals[7];
    GDLSignal l4 = signals[8] | signals[9] | signals[10];

    //...Coincidence of all layers...
    GDLSignal all = l0 & l1 & l2 & l3 & l4;

    if (all.nEdges())
	_signal = new GDLSignal(all);

#ifdef CDCTRIGGER_DEBUG
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
