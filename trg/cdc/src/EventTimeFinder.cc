//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : EventTimeFinder.cc
// Section  : TRG CDC
// Owner    : Yoshihito IWASAKI
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find an event timing
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <bitset>
#include "trg/trg/Debug.h"
#include "trg/trg/State.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/EventTimeFinder.h"
#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "mdst/dataobjects/MCParticle.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/TRGCDC.h"

using namespace std;

namespace Belle2 {

//...For debug : yi...
// vector<TRGSignalVector * > dbgIn;
// vector<TRGSignalVector * > dbgOut;

  TRGCDCEventTimeFinder::TRGCDCEventTimeFinder(const std::string& name,
                                               const TRGClock& systemClock,
                                               const TRGClock& dataClock,
                                               const TRGClock& userClockInput,
                                               const TRGClock& userClockOutput)
    : TRGBoard(name, systemClock, dataClock, userClockInput, userClockOutput)
  {
    // _tisb(0),
    // _tosbE(0),
    // _tosbT(0) {
  }

  TRGCDCEventTimeFinder::~TRGCDCEventTimeFinder()
  {
  }

  void
  TRGCDCEventTimeFinder::push_back(const TRGCDCTrackSegmentFinder* a)
  {
    std::vector<const TRGCDCTrackSegmentFinder*>::push_back(a);
  }

  void
  TRGCDCEventTimeFinder::simulate(void)
  {

    const string sn = "ETF::simulate : " + name();
    TRGDebug::enterStage(sn);

    //...Delete old objects...
    for (unsigned i = 0; i < nOutput(); i++) {
      if (output(i))
        if (output(i)->signal())
          delete output(i)->signal();
    }
    TRGDebug::leaveStage(sn);
  }

} // namespace Belle2
