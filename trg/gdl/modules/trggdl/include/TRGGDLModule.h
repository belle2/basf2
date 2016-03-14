//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGDLModule.h
// Section  : TRG GDL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for GDL
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#ifndef TRGGDLModule_H
#define TRGGDLModule_H

#include <string>
#include "framework/core/Module.h"
#include "trg/gdl/TRGGDL.h"

namespace Belle2 {

/// A module to process GDL trigger data
class TRGGDLModule : public Module {

  public:

    /// Constructor
    TRGGDLModule();
//  TRGGDLModule(const std::string & type);

    /// Destructor
    virtual ~TRGGDLModule();

    /// Initilizes TRGGDLModule.
    virtual void initialize();

    /// Called when new run started.
    virtual void beginRun();

    /// Called event by event.
    virtual void event();

    /// Called when run ended.
    virtual void endRun();

    /// Called when processing ended.
    virtual void terminate();

  public:

    /// returns version of TRGGDLModule.
    std::string version(void) const;

  private: // Parameters

    /// Debug level.
    int _debugLevel;

    /// Config. file name.
    std::string _configFilename;

    /// Mode for TRGGDL simulation. 0th bit : fast simulation switch,
    /// 1st bit : firmware simulation switch.
    int _simulationMode;

    /// Switch for the fast simulation. 0:do everything, 1:stop after
    /// the track segment simulation. Default is 0.
    int _fastSimulationMode;

    /// Switch for the firmware simulation. 0:do nothing, 1:do everything
    int _firmwareSimulationMode;

  private:

    /// A pointer to a TRGGDL;
    TRGGDL * _gdl;
};

} // namespace Belle2

#endif // TRGGDLModule_H
