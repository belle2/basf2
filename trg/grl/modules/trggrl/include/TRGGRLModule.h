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
// Filename : TRGGRLModule.h
// Section  : TRG GRL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for GRL
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#ifndef TRGGRLModule_H
#define TRGGRLModule_H

#include <string>
#include "framework/core/Module.h"
#include "trg/grl/TRGGRL.h"

namespace Belle2 {

/// A module to simulate the Global Decision Logic. This module
/// requires sub-trigger simulation data (CDC, ECL, TOP, and KLM). The
/// output is GRL response.

  class TRGGRLModule : public Module {

  public:

    /// Constructor
    TRGGRLModule();
//  TRGGRLModule(const std::string & type);

    /// Destructor
    virtual ~TRGGRLModule();

    /// Initilizes TRGGRLModule.
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

    /// returns version of TRGGRLModule.
    std::string version(void) const;

  private: // Parameters

    /// Debug level.
    int _debugLevel;

    /// Config. file name.
    std::string _configFilename;

    /// Mode for TRGGRL simulation. 0th bit : fast simulation switch,
    /// 1st bit : firmware simulation switch.
    int _simulationMode;

    /// Switch for the fast simulation. 0:do everything, 1:stop after
    /// the track segment simulation. Default is 0.
    int _fastSimulationMode;

    /// Switch for the firmware simulation. 0:do nothing, 1:do everything
    int _firmwareSimulationMode;

  private:

    /// A pointer to a TRGGRL;
    static TRGGRL* _grl;
  };

} // namespace Belle2

#endif // TRGGRLModule_H
