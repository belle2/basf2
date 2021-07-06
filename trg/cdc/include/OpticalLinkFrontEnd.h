/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a serial link from the CDC Front-end
//-----------------------------------------------------------------------------

#ifndef TRGOpticalLinkFrontEnd_FLAG_
#define TRGOpticalLinkFrontEnd_FLAG_

#include <string>
#include <vector>
#include "trg/trg/Link.h"

#define TRGOpticalLinkVersionFrontEnd 0.00

namespace Belle2 {

  class TRGSignal;
  class TRGSignalVector;
  class TRGClock;

/// A class to represent a serial link between trigger modules
  class TRGOpticalLinkFrontEnd : public TRGOpticalLink {

  public:

    /// Constructor.
    TRGOpticalLinkFrontEnd(const std::string& name, const TRGClock&);

    /// Destructor
    virtual ~TRGOpticalLinkFrontEnd();

  public:// Selectors.

  public:// Modifiers.

  private:

    /// VHDL : begin

    /// test signal
    const TRGSignal* _hit;

    /// Hit pattern of 48 wires
    std::vector<const TRGSignal*> _hitPattern48;

    /// Timing of the priority cells (2ns LSB)
    /// VHDL : type PriTiming is array (2 downto 0) of std_logic;
    std::vector<const TRGSignalVector*> _priTiming16;

    /// Timing of the fastest hit in PTS (2ns LSB)
    /// VHDL : type FasTiming is array (2 downto 0) of std_logic;
    std::vector<const TRGSignalVector*> _fasTiming16;

    /// Clock counter
    /// VHDL : type ClkCounter is array (31 downto 0) of std_logic;
    TRGSignalVector _clkCounter;

    /// VHDL : end
  };

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGOpticalLinkFrontEnd_FLAG_ */
