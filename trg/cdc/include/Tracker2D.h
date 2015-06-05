//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Tracker2D.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#pragma once

#include <float.h>
#include <string>
#include "trg/trg/Board.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTracker2D TRGCDCTracker2D
#endif

namespace Belle2 {

class TRGCDCTrackSegmentFinder;

/// A class to represent a CDC 2D tracker board
class TRGCDCTracker2D
    : public TRGBoard,
      public std::vector<const TRGCDCTrackSegmentFinder *> {
    
  public:
    /// Constructor.
    TRGCDCTracker2D(const std::string & name,
                    const TRGClock & systemClock,
                    const TRGClock & dataClock,
                    const TRGClock & userClockInput,
                    const TRGClock & userClockOutput);

    /// Destructor
    virtual ~TRGCDCTracker2D();

  public:// Selectors.

    /// returns  version.
    static std::string version(void);

    /// returns \# of TSF in this 2D tracker board.
    static unsigned nTSFs(void);

  public:// Modifiers

    /// simulates firmware.
    void simulate(void);

  public:// VHDL utilities

    /// Unpack TSF output.
    static void unpacker(const TRGState & input,
                         TRGState & output);

    /// Packer for 3D tracker.
    static TRGState packer(const TRGState & input,
                           TRGState & registers,
                           bool & logicStillActive);

  public:// Configuration

    /// Appends a TSF board.
    void push_back(const TRGCDCTrackSegmentFinder *);

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
              const std::string & pre = "") const;
    
  private: 
};

//-----------------------------------------------------------------------------

} // namespace Belle2

