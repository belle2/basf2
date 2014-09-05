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
#include "trg/trg/Debug.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/State.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/LUT.h"
#include "cdc/geometry/CDCGeometryPar.h"

#include "trg/cdc/EventTime.h"
#include <bitset>

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
			     const TRGClock & clock,
			     const TRGCDCEventTime * eventTime,
			     const std::string & TSLUTFile,
			     const std::vector<const TCWire *> & cells)
    : TCCell(id,
	     layer.size(),
	     layer,
	     w.forwardPosition(),
	     w.backwardPosition()),
      _wires(cells),
      _signal(std::string("TS_") + TRGUtil::itostring(id), clock),
      _eventTime(eventTime),
      m_TSLUTFileName(TSLUTFile){
      m_TSLUT = new TCLUT();
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
TRGCDCSegment::initialize(){
    if(center().superLayerId()){
      m_TSLUT->setDataFile(m_TSLUTFileName, 12);
    }
    else{
      m_TSLUT->setDataFile(m_TSLUTFileName, 16);
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
TCSegment::simulate(bool clockSimulation, bool logicLUTFlag) {

    //...Get wire informtion for speed-up...
    const unsigned n = _wires.size();
    unsigned nHits = 0;
    vector<TRGSignal> signals;
    for (unsigned i = 0; i < n; i++) {

	//...Copy signal from a wire...
        const TRGSignal & s = _wires[i]->signal();
        if (s.active())
            ++nHits;
    }

    //..No wire hit case...
    if (nHits == 0)
	return;

    if (clockSimulation)
	cout << "this part is replaced with simulateBoard TrackSegmentFinder class" << endl;
//	simulateWithClock(logicLUTFlag);
    else
	simulateWithoutClock(logicLUTFlag);
}

void
TCSegment::simulateWithoutClock(bool logicLUTFlag) {

    TRGDebug::enterStage("TS sim");

    //...System clocks... Freq: 125 MHz
    const TRGClock & systemClock = TRGCDC::getTRGCDC()->systemClock();
    //systemClock.dump();

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
      const TRGSignal & s = _wires[i]->signal();
      signals.push_back(s);

      //...Change clock...
      signals.back().clock(systemClock);

      //...Widen it...
      static const unsigned width = systemClock.unit(400);
      signals.back().widen(width);

      if (s.active())
        ++nHits;
    }


    if( logicLUTFlag == 0) {
      ///// TS Logic Finder
      //...Check number of hit wires...
      //cout<<"TSF: nHits is "<<nHits<<endl;
      if (nHits < 4) {
        TRGDebug::leaveStage("TS sim");
        return;
      }

      //...Signal simulation...
      TRGSignal l0, l1, l2, l3, l4;
      TRGSignal wo1,wo2,wo3,wo4;
      TRGSignal all;
      if (n == 11) {

        //...Simple simulation assuming 3:2:1:2:3 shape...
        l0 = signals[0] | signals[1] | signals[2];
        l1 = signals[3] | signals[4];
        l2 = signals[5];
        l3 = signals[6] | signals[7];
        l4 = signals[8] | signals[9] | signals[10];
        //l0.dump();
        //l1.dump();
        //l2.dump();
        //l3.dump();
        //l4.dump();
        wo1 = l1 & l3 & l4;
        wo2 = l0 & l3 & l4;
        wo3 = l0 & l1 & l4;
        wo4 = l0 & l1 & l3;
        all = l2 & (wo1|wo2|wo3|wo4);

      }
      else if (n == 15) {

        //...Simple simulation assuming 1:2:3:4:5 shape...
        l0 = signals[0];
        l1 = signals[1] | signals[2];
        l2 = signals[3] | signals[4] | signals[5];
        l3 = signals[6] | signals[7] | signals[8] | signals[9];
        l4 = signals[10] |signals[11] | signals[12] | signals[13] |signals[14];
        wo1 = l2 & l3 & l4;
        wo2 = l1 & l3 & l4;
        wo3 = l1 & l2 & l4;
        wo4 = l1 & l2 & l3;
        all = l0 & (wo1|wo2|wo3|wo4);
      }

      //...Coincidence of all layers...
//      TRGSignal all = l0 & l1 & l2 & l3 & l4;

      if (all.nEdges()) {
        //cout<<"TSF is found"<<endl;
        all.name(name());
        _signal = all;
        //cout<<all.name()<<":#signals="<<all.nSignals()<<endl;;
        //all.dump();
      }
      ///// End of TS logic finder
    }

    if (logicLUTFlag == 1) {
      ///// TS LUT finder
      //... Find hit wires ...
      vector<TRGSignal> hitSignals;
      for (unsigned iWire = 0; iWire < signals.size(); iWire++ ) {
        if (signals[iWire].active()) hitSignals.push_back(signals[iWire]);
      }
      //... Coincidence all hit wires ...
      TRGSignal allSignals;
      if (hitSignals.size() != 0) {
        allSignals = hitSignals[0];
        for (unsigned iHitWire = 1; iHitWire < hitSignals.size(); iHitWire++) {
          allSignals = allSignals & hitSignals[iHitWire];
        }
      }


      //bitset<15> strHitPattern(this->hitPattern());
      //cout<<"SuperLayerID: "<<this->superLayerId()<<" hitPattern: "<<this->hitPattern()<<" "<<strHitPattern<<endl;
      //cout<<"LUT result: "<<this->LUT()->getHitLRLUT(this->hitPattern(),this->superLayerId())<<endl;
      //cout<<"Is center hit fired? " << (this->center().hit() != 0)<<endl;
//      int lutHit = atoi(&(this->LUT()->getHitLRLUT(this->hitPattern(),this->superLayerId()).at(4)));
      //
      // Only when center wire is hit
 //     if(lutHit == 1 && (this->center().hit() != 0) ) {

//work here
      //int lutValue = this->nLUT()->getValue(this->hitPattern());
      int lutValue = this->LUT()->getValue(this->lutPattern());
//      if((lutValue != 0) && (this->center().hit() != 0) ) {
      if((lutValue != 0) && (this->priority().hit() != 0) ) {
      //if((lutValue != 0) ){
        allSignals.name(name());
        _signal = allSignals;
      }
      ///// End of TS LUT finder
    }

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

    TRGDebug::leaveStage("TS sim");

}

float
TCSegment::fastestTime(void)const{
  float tmpFastTime = 9999;
  if((this->LUT()->getValue(this->lutPattern()))&&(this->priority().hit())){
    for(unsigned i=0;i<_wires.size();i++){
      if(_wires[i]->hit()){
        float dt= _wires[i]->hit()->drift()*10*1000/40;
        if(dt< tmpFastTime){
          tmpFastTime = dt;
        }
      }
    }
    return tmpFastTime;
  }
  else 
    return -1;
}

float
TCSegment::priorityTime(void){
  if(this->center().hit()){
    return this->center().hit()->drift()*10*1000/40;
  } else if(this->LUT()->getValue(this->lutPattern())){
    const TRGCDCWire* priorityL;
    const TRGCDCWire* priorityR;
    if(_wires.size() == 15){
      priorityL = _wires[2];
      priorityR = _wires[1];
    }else{
      priorityL = _wires[7];
      priorityR = _wires[6];
    }
    return fasterWire(priorityL, priorityR).hit()->drift()*10*1000/40;
  } else return -1;
}

int
TCSegment::priorityPosition(void)const{
  if(this->center().hit()){
    return 3;
  }else if(this->hit()){
    const TRGCDCWire* priorityL;
    const TRGCDCWire* priorityR;
    if(_wires.size() == 15){
      priorityL = _wires[2];
      priorityR = _wires[1];
    }else{
      priorityL = _wires[7];
      priorityR = _wires[6];
    }
    if(priorityL->hit()){
      if(priorityR->hit()){
        if((priorityR->hit()->drift())>(priorityR->hit()->drift())) return 1;
        else return 2;
      }else return 2;
    }else if(priorityR->hit()){
      return 1;
    } else return -1;
  }else return 0;
}

const TRGCDCWire &
TCSegment::priority(void) const{
  if(this->center().hit()){
    if(_wires.size()==15){
      return *_wires[0];
    }else return *_wires[5];
  } else if(m_TSLUT->getValue(this->lutPattern())){
    if(_wires.size()==15){return fasterWire(_wires[1],_wires[2]);
    }else return fasterWire(_wires[6],_wires[7]);
  }else{ 
    if(_wires.size()==15){
      return *_wires[0];
    }else return *_wires[5];
  }
}

const TRGCDCWire &
TCSegment::fasterWire(const TRGCDCWire* w1, const TRGCDCWire* w2)const{
  if(w1->hit()){
    if(w2->hit()){
      if(w1->hit()->drift()>w2->hit()->drift()) return *w2;
      else return *w1;
    }else return *w1;
  } else return *w2;
} 

/*void
TCSegment::simulateWithClock(bool logicLUTFlag) {

    //cout<<"Start TSF with clock"<<endl;

    const string stage = "TS sim w/clock" + name();
    TRGDebug::enterStage(stage);

    //...System clocks...
    const TRGClock & dataClock = TRGCDC::getTRGCDC()->dataClock();

    //...Get wire informtion...
    const unsigned n = _wires.size();
    unsigned nHits = 0;
    TRGSignalVector signals(name(), dataClock);
    for (unsigned i = 0; i < n; i++) {

	//...Copy signal from a wire...
	TRGSignal s = _wires[i]->signal();

// 	if (TRGDebug::level() > 2) {
// 	    s.dump("detail", TRGDebug::tab() + "wire TDC ");
// 	}

	//...Change clock... (from FE clock to data clock)
	s.clock(dataClock);

 	//...Widen it...
 	static const unsigned width = dataClock.unit(400);
 	s.widen(width);

// 	if (TRGDebug::level() > 2) {
// 	    s.dump("detail", TRGDebug::tab() + "data CLK ");
// 	}

        signals += s;

        if (s.active())
            ++nHits;
    }

//  signals.dump("detail", TRGDebug::tab() + "TS wire dump:");

    //...Check state changes...
    vector<int> clocks = signals.stateChanges();
    const unsigned nStates = clocks.size();
    int tsSize = signals.size();
    // TS timing
    int tsRise, tsFall = 0;
    bool tsPrevious = 0;
    TRGSignal tsSignal(dataClock);
    for (unsigned i = 0; i < nStates; i++) {
      const TRGState state = signals.state(clocks[i]);
      //	state.dump("dump of state:", TRGDebug::tab());
      //cout << TRGDebug::tab() << i << ":c=" << clocks[i] << ":" << state
      //<< endl;

      bool tsAll = 0;

      if ( logicLUTFlag == 0 ) {
        ////// TS logic Finder
        bool tsL0,tsL1,tsL2,tsL3,tsL4=0;
        if(tsSize == 11) {
          //cout<<"Outer TS"<<endl;
          //...Simple simulation assuming 3:2:1:2:3 shape...
          tsL0 = state[0] | state[1] | state[2];
          tsL1 = state[3] | state[4];
          tsL2 = state[5];
          tsL3 = state[6] | state[7];
          tsL4 = state[8] | state[9] | state[10];
        } else if (tsSize == 15) {
          //cout<<"Inner TS"<<endl;
          //...Simple simulation assuming 1:2:3:4:5 shape...
          tsL0 = state[0];
          tsL1 = state[1] | state[2];
          tsL2 = state[3] | state[4] | state[5];
          tsL3 = state[6] | state[7] | state[8] | state[9];
          tsL4 = state[10] |state[11] | state[12] | state[13] |state[14];
        }

        tsAll = tsL0 & tsL1 & tsL2 & tsL3 & tsL4;
        //cout<<"L0,L1,L2,L3,L4: "<<tsL0<<tsL1<<tsL2<<tsL3<<tsL4<<endl;
        //cout<<"tsAll: "<<tsAll<<endl;
        //// TS logic Finder
      }

      if (logicLUTFlag == 1) {
        ////// TS LUT Finder
        bool centerState;
        if(tsSize==15) {
          centerState = state[0];
        } else if (tsSize==11) {
          centerState = state[5];
        }
        //cout<<"State: "<<unsigned(state)<<endl;
        //unsigned iHitPattern = unsigned(state);
        //bitset<15> strHitPattern(iHitPattern);
        //cout<<"SuperLayerID: "<<this->superLayerId()<<" hitPattern: "<<unsigned(state)<<" "<<strHitPattern<<endl;
        //cout<<"LUT result: "<<this->LUT()->getHitLRLUT(unsigned(state),this->superLayerId())<<" TS logic result: "<<tsAll<<endl;
        //cout<<"Is center hit fired? " <<centerState<<endl;
        int lutHit = atoi(&(this->LUT()->getHitLRLUT(unsigned(state),this->superLayerId()).at(4)));
        if(lutHit == 1 && (centerState != 0) ) {
          tsAll = 1;
        }
        ////// TS LUT Finder
      }

      // Makes signal for TS
      if(tsPrevious != tsAll) {
        if(tsAll == 1) {
          //cout<<"Start of new signal"<<endl;
          tsRise = clocks[i];
          tsPrevious = tsAll;
        }
        if(tsAll == 0) {
          tsFall = clocks[i];
          tsSignal |= TRGSignal(dataClock, tsRise, tsFall);
          tsPrevious = tsAll;
          //cout<<"End of new signal"<<endl;
          //tsSignal.dump();
        }
      }

    } // Loop over all state changes

    //cout<<"End of all signals for TS"<<endl;
    //tsSignal.dump();


    if(tsSignal.nEdges()) {
      tsSignal.name(name());
      _signal = tsSignal;
      //cout<<tsSignal.name()<<":#signals="<<tsSignal.nSignals()<<endl;
      //tsSignal.dump();
    }


    //cout<<"End TSF with clock"<<endl;

    TRGDebug::leaveStage(stage);
}*/

unsigned
TRGCDCSegment::hitPattern(void) const {
    unsigned ptn = 0;
    for (unsigned i = 0; i < _wires.size(); i++) {
        const TRGSignal & s = _wires[i]->signal();
        if (s.active())
	    ptn |= (1 << i);
    }
    return ptn;
}


unsigned 
TRGCDCSegment::lutPattern(void) const{
  unsigned outValue = (this->hitPattern())*2;
  if (priorityPosition()==2){
   outValue+=1;
  }
  return outValue;
}

double
TRGCDCSegment::phiPosition(void) const {
//  cout << "this function(phiPosition in Segment class) will be removed. for 2D & 3D fitting, calculate this information by your own class" <<endl;
  float evtTime=EvtTime()->getT0();
  evtTime=evtTime*40/1000;
	double phi=(double)localId()/nWires[superLayerId()]*4*M_PI;
	int lutcomp=LUT()->getValue(lutPattern());
	float dphi=hit()->drift()*10;
	if(flagevtTime){
		dphi-=evtTime;
	}
	dphi=atan(dphi/wireR[superLayerId()]/1000);
	if(lutcomp==2){phi-=dphi;}
	else if(lutcomp==1){phi+=dphi;}
	
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
