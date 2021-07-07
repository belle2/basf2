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

#ifndef TRGCDCEventTimeFinder_FLAG_
#define TRGCDCEventTimeFinder_FLAG_

#include "trg/trg/Board.h"
#include "trg/trg/SignalVector.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCETFinder TRGCDCEventTimeFinder
#define ETFinder TRGCDCEventTimeFinder
#endif

namespace Belle2 {

  class TRGCDCTrackSegmentFinder;

  /// A class of TRGCDC Event Time Finder
  class TRGCDCEventTimeFinder
    : public TRGBoard,
      public std::vector <const TRGCDCTrackSegmentFinder*> {

  public:

    /// Constructor.
    TRGCDCEventTimeFinder(const std::string& name,
                          const TRGClock& systemClock,
                          const TRGClock& dataClock,
                          const TRGClock& userClockInput,
                          const TRGClock& userClockOutput);

    /// Destructor.
    ~TRGCDCEventTimeFinder();

  public:

    /// return version
    static std::string version(void);

    /// push back the TRGCDCTrackSegmentFinder pointer
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
