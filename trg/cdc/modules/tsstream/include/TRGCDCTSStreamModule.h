/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------

#ifndef TRGCDCTSStreamModule_H
#define TRGCDCTSStreamModule_H

#include <string>
#include "framework/core/Module.h"
#include "trg/cdc/TRGCDC.h"

namespace Belle2 {

/// A module to process CDC trigger data
  class TRGCDCTSStreamModule : public Module {

  public:

    /// Constructor
    TRGCDCTSStreamModule();

    /// Destructor
    virtual ~TRGCDCTSStreamModule();

    /// Initilizes TRGCDCTSStreamModule.
    virtual void initialize() override;

    /// Called when new run started.
    virtual void beginRun() override;

    /// Called event by event.
    virtual void event() override;

    /// Called when run ended.
    virtual void endRun() override;

    /// Called when processing ended.
    virtual void terminate() override;

  public:

    /// returns version of TRGCDCTSStreamModule.
    std::string version(void) const;

  private: // Parameters

    /// Debug level.
    int _debugLevel;

    /// Mode for streaming data.
    int _mode;

    /// The filename of bit stream.
    std::string _streamFilename;

  private:

    /// A pointer to a TRGCDC;
    TRGCDC* _cdc;

    /// Storage for TS central wires.
    std::vector<const TRGCDCWire*> _wires[9];

    /// A pointer to an output file.
    std::ofstream* _out;
  };

} // namespace Belle2

#endif // TRGCDCTSStreamModule_H
