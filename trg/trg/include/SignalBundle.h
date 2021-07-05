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
// Filename : TRGSignalBundle.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a bundle of SignalVectors.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGSignalBundle_FLAG_
#define TRGSignalBundle_FLAG_

#include <vector>
#include "trg/trg/Clock.h"

namespace Belle2 {

  class TRGSignal;
  class TRGSignalVector;
  class TRGState;

/// A class to represent a bundle of SignalVectors.
  class TRGSignalBundle : public std::vector<TRGSignalVector*> {

  public:

    /// Default constructor.
//  TRGSignalBundle(const TRGClock & = Belle2_GDL::GDLSystemClock);
    explicit TRGSignalBundle(const TRGClock&);

    /// Constructor with name.
    TRGSignalBundle(const std::string& name,
//        const TRGClock & = Belle2_GDL::GDLSystemClock);
                    const TRGClock&);

    /// Constructor with a packer.
    TRGSignalBundle(const std::string& name,
                    const TRGClock& clock,
                    const TRGSignalBundle& input,
                    const unsigned outputBitSize,
                    TRGState(* packer)(const TRGState&));

    /// Constructor with a packer which can handle multiple clock states.
    TRGSignalBundle(const std::string& name,
                    const TRGClock& clock,
                    const TRGSignalBundle& input,
                    const unsigned outputBitSize,
                    const unsigned registerBitSize,
                    TRGState(* packer)(const TRGState& in,
                                       TRGState& registers,
                                       bool& logicStillActive));

    /// Destructor
    virtual ~TRGSignalBundle();

  public:// Selectors

    /// returns name.
    const std::string& name(void) const;

    /// sets and returns name.
    const std::string& name(const std::string& newName);

    /// returns clock.
    const TRGClock& clock(void) const;

    /// returns true if there is a signal.
    bool active(void) const;

    /// returns a list of clock position of state change.
    std::vector<int> stateChanges(void) const;

    /// returns state at given clock position.
    TRGState state(int clockPosition) const;

    /// returns signal of all ORed.
    TRGSignal ored(void) const;

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string& message = "",
              const std::string& pre = "") const;

    /// makes coe output.
    void dumpCOE(const std::string& fileName = "",
                 int start = 0,
                 int stop = 0) const;

  public:// Operators

    /// changes clock.
    const TRGClock& clock(const TRGClock&);

    /// reads data from a file.
    void readCOE(const std::string& fileName = "");

  private:

    /// Name
    std::string _name;

    /// Clock
    const TRGClock* _clock;
  };

//-----------------------------------------------------------------------------

  inline
  const std::string&
  TRGSignalBundle::name(void) const
  {
    return _name;
  }

  inline
  const std::string&
  TRGSignalBundle::name(const std::string& newName)
  {
    return _name = newName;
  }

  inline
  const TRGClock&
  TRGSignalBundle::clock(void) const
  {
    return * _clock;
  }

} // namespace Belle2

#endif /* TRGSignalBundle_FLAG_ */
