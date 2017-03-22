//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : EventTimeFinder.h
// Section  : TRG CDC
// Owner    : Yoshihito IWASAKI
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find an event timing
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCEventTimeFinder_FLAG_
#define TRGCDCEventTimeFinder_FLAG_

#include "trg/trg/Board.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCETFinder TRGCDCEventTimeFinder
#define ETFinder TRGCDCEventTimeFinder
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCSegment;
  class TRGCDCSegmentHit;
  class TRGCDCTrackSegmentFinder;

  class TRGCDCEventTimeFinder
    : public TRGBoard,
      public std::vector <const TRGCDCTrackSegmentFinder*> {

  public:

    // Constructor.
    TRGCDCEventTimeFinder(const std::string& name,
                          const TRGClock& systemClock,
                          const TRGClock& dataClock,
                          const TRGClock& userClockInput,
                          const TRGClock& userClockOutput);

    // Destructor.
    ~TRGCDCEventTimeFinder();

  public:

    /// return version
    static std::string version(void);

    void push_back(const TRGCDCTrackSegmentFinder*);

    /// Firmware simulation. yi
    void simulate(void);

    /// Output packer for tracker
    TRGSignalVector* packerForTracker(vector<TRGSignalVector*>&,
                                      vector<int>&,
                                      const unsigned);

  private:

    /// Input signal bundle.
    //TRGSignalBundle * _tisb;

    /// Output signal bundle.
  };

} // namespace Belle2

#endif
