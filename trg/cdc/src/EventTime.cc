//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : EventTime.cc
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to get Event Time information
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Debug.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/EventTime.h"
#include <cstdlib>
#include <iostream>
#include <vector>

#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/trg/Utilities.h"

using namespace std;

namespace Belle2{
  TRGCDCEventTime::TRGCDCEventTime(const TRGCDC & TRGCDC)
    : _cdc(TRGCDC){
    }
  TRGCDCEventTime::~TRGCDCEventTime(){}

  void TRGCDCEventTime::initialize(void){
  }

  double TRGCDCEventTime::getT0 (void)const{
    TRGDebug::enterStage("Event Time");
    bool foundT0 = 0;
    double tmin=65535;
    for (unsigned i=0;i<_cdc.nSegmentLayers();i++){
      const Belle2::TRGCDCLayer *l=_cdc.segmentLayer(i);
      const unsigned nWires = l->nCells();
      for(unsigned j=0;j<nWires;j++){
        const TCSegment &s = (TCSegment &)*(*l)[j];
        const vector<const TCWire *> &wires = s.wires();
        const TRGSignal & timing=s.signal();
        if(timing.active()){
          for(unsigned k=0;k<wires.size();k++){
            if(wires[k]->hit()){
              //		  double dl=wires[k]->hit()->drift()*10*1000/40;
              double dt= wires[k]->signal()[0]->time();
              //		  cout << dt << endl;
              if(tmin>dt) {
                tmin=dt;
                foundT0 = 1;
              }
            }
          }
        }
      }
    }
    if(foundT0 == 0) tmin = 0;
    TRGDebug::leaveStage("Event Time");
    return tmin;
  }

  void TRGCDCEventTime::terminate(void){
  }
}
