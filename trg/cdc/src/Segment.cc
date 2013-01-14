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
#include "cdc/geometry/CDCGeometryPar.h"

#include "trg/cdc/EventTime.h"

//... Global varibles...
double wireR[9];
int nWires[9];
bool flagevtTime;

using namespace std;

#define P3D HepGeom::Point3D<double>

namespace Belle2 {

TRGCDCSegment::TRGCDCSegment(unsigned id,
			     const TCLayer & layer,
			     const TCWire & w,
			     const TCLUT * lut,
			     const TRGCDCEventTime * eventTime,
			     const std::vector<const TCWire *> & cells)
    : TCCell(id,
	     layer.size(),
	     layer,
	     w.forwardPosition(),
	     w.backwardPosition()),
      _lut(lut),
      _wires(cells),
      _signal(std::string("TS_") + TRGUtil::itostring(id)),
      _eventTime(eventTime) {
}


TRGCDCSegment::~TRGCDCSegment() {
}
 
void
TRGCDCSegment::initialize(bool fevtTime){
  flagevtTime=fevtTime;
  CDC::CDCGeometryPar& cdcp=CDC::CDCGeometryPar::Instance();
  wireR[0]=cdcp.senseWireR(2)*0.01;
  nWires[0]=cdcp.nWiresInLayer(2)*2;
  for(int i=0;i<4;i++){
    wireR[2*i+1]=cdcp.senseWireR(12*i+10)*0.01;
    wireR[2*(i+1)]=cdcp.senseWireR(12*(i+1)+4)*0.01;
    nWires[2*i+1]=cdcp.nWiresInLayer(12*i+10)*2;
    nWires[2*(i+1)]=cdcp.nWiresInLayer(12*(i+1)+4)*2;
  }
}

void
TRGCDCSegment::dump(const string & msg,
		    const string & pre) const {
    cout << pre << name() << " (ptn=" << hitPattern() << ")" << endl;
    if ((msg.find("geometry") != string::npos) ||
        (msg.find("detail") != string::npos)) {
        cout << pre << "id " << id();
        cout << ",local " << localId();
        cout << ",layer " << layerId();
        cout << ",super layer " << superLayerId();
        cout << ",local layer " << localLayerId();
        cout << endl;
    }
    if ((msg.find("hit") != string::npos) ||
        (msg.find("detail") != string::npos)) {
	cout << pre << "Wires ";
	for (unsigned i = 0; i < _wires.size(); i++) {
	    cout << _wires[i]->name();
	    if (i < _wires.size() - 1)
		cout << ",";
	    else
		cout << endl;
	}
	if (_hits.size() == 0) {
	    cout << pre << "no wire hit" << endl;
	}
	else {
	    cout << pre << "WHit dump : ";
	    for (unsigned i = 0; i < _hits.size(); i++) {
		cout << _hits[i]->cell().name();
		if (i < _hits.size() - 1)
		    cout << ",";
		else
		    cout << endl;
	    }
	    for (unsigned i = 0; i < _hits.size(); i++) {
		_hits[i]->dump(msg, pre + "    ");
	    }
	}
	if (hit()) {
	    cout << pre << "SHit dump" << endl;
	    hit()->dump(msg, pre + "    ");
	}
	else {
	    cout << pre << "no TSHit" << endl;
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
    TCCell::clear();
    _signal.clear();
    _hits.clear();
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

    //...System clocks...
    const TRGClock & systemClock = TRGCDC::getTRGCDC()->systemClock();
    const TRGClock & systemClockFE = TRGCDC::getTRGCDC()->systemClockFE();

    //...Get wire informtion...
    const unsigned n = _wires.size();
    unsigned nHits = 0;
    vector<TRGSignal> signals;
    for (unsigned i = 0; i < n; i++) {

	//...Store wire hit information...
	const TCWHit * h = _wires[i]->hit();
	if (h)
	    _hits.push_back(h);

	//...Copy signal from a wire...
        const TRGSignal & s = _wires[i]->timing();
        signals.push_back(s);

	//...Change clock...
	signals.back().clock(systemClock);

 	//...Widen it...
 	static const unsigned width = systemClock.unit(400);
 	signals.back().widen(width);

        if (s.active())
            ++nHits;
    }

    //...Check number of hit wires...
    if (nHits < 4)
        return;

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

//     if (iwd) {
// 	l0.dump("", "     l0-> ");
// 	l1.dump("", "     l1-> ");
// 	l2.dump("", "     l2-> ");
// 	l3.dump("", "     l3-> ");
// 	l4.dump("", "     l4-> ");
// 	if (all.nEdges())
// 	    cout << "===========" << endl;
// 	all.dump("", "    ----> ");
//     }    
}

unsigned
TRGCDCSegment::hitPattern(void) const {
    unsigned ptn = 0;
    for (unsigned i = 0; i < _wires.size(); i++) {
        const TRGSignal & s = _wires[i]->timing();
        if (s.active())
	    ptn |= (1 << i);
    }
    return ptn;
}

double
TRGCDCSegment::phiPosition(void) const {
  float evtTime=EvtTime()->getT0();
  evtTime=evtTime*40/1000;
	double phi=(double)localId()/nWires[superLayerId()]*4*M_PI;
	int lutcomp=LUT()->getLRLUT(hitPattern(),superLayerId());
	float dphi=hit()->drift()*10;
	if(flagevtTime){
		dphi-=evtTime;
	}
	dphi=atan(dphi/wireR[superLayerId()]/1000);
	if(lutcomp==0){phi-=dphi;}
	else if(lutcomp==1){phi+=dphi;}
	else{phi=phi;}
	
	return phi;
}

bool
TRGCDCSegment::hasMember(const std::string & a) const {
    const unsigned n = _wires.size();
    for (unsigned i = 0; i < n; i++) {
	if (_wires[i]->hasMember(a))
	    return true;
    }
    return false;
}

} // namespace Belle2
