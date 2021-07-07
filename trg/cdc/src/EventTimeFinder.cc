/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to find an event timing
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Debug.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/EventTimeFinder.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"

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
