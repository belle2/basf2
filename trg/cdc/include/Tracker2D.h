/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

  class TRGState;
  class TRGCDCTrackSegmentFinder;

/// A class to represent a CDC 2D tracker board
  class TRGCDCTracker2D
    : public TRGBoard,
      public std::vector<const TRGCDCTrackSegmentFinder*> {

  public:
    /// Constructor.
    TRGCDCTracker2D(const std::string& name,
                    const TRGClock& systemClock,
                    const TRGClock& dataClock,
                    const TRGClock& userClockInput,
                    const TRGClock& userClockOutput);

    /// Destructor
    virtual ~TRGCDCTracker2D();

  public:// Selectors.

    /// returns  version.
    static std::string version(void);

  public:// Modifiers

    /// simulates firmware.
    void simulate(void);

  public:// VHDL utilities

    /// returns \# of TSF.
    static unsigned nTSF(void);

    /// returns \# of TSF in super layer i. (i=0 to 4)
    static unsigned nTSF(unsigned i);

    /// Unpack TSF output.
    static void unpacker(const TRGState& input, TRGState& output);

    /// Packer for 3D tracker.
    static TRGState packer(const TRGState& input,
                           TRGState& registers,
                           bool& logicStillActive);

    /// Do core logic simulation for positive charge.
    static void HoughMappingPlus(void);
    /// Do core logic simulation for negative charge.
    static void HoughMappingMinus(void);

  public:// Configuration

    /// Appends a TSF board.
    void push_back(const TRGCDCTrackSegmentFinder*);

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

  private:

    /// Sets constants.
    void setConstants(void);

    /// Gets TSF hit information for one certin clock from the registers.
    static void hitInformation(const TRGState& registers);

  private:

    /// \# of TSFs.
    static unsigned _nTSF;

    /// \# of TSFs in super layer i.
    static std::vector<unsigned> _n;

    /// Keeps TS hit info.
    static TRGState _ts;
  };

//-----------------------------------------------------------------------------

  inline
  unsigned
  TCTracker2D::nTSF(void)
  {
    return 160 + 192 + 256 + 320 + 384;
  }

  inline
  unsigned
  TCTracker2D::nTSF(unsigned i)
  {
    if (i == 0)
      return 160;
    else if (i == 1)
      return 192;
    else if (i == 2)
      return 256;
    else if (i == 3)
      return 320;
    else if (i == 4)
      return 384;
    else
      return 0;
  }

} // namespace Belle2

