//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerModule.h
// Section  : Trigger CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/08 : First version
//-----------------------------------------------------------------------------

#ifndef CDCTriggerModule_H
#define CDCTriggerModule_H

#include <string>
#include "framework/core/Module.h"
#include "trigger/cdc/CDCTrigger.h"

namespace Belle2 {

/// A module to process CDC trigger data
class CDCTriggerModule : public Module {

  public:

//     /// returns a pointe to CDCTriggerModule.
//     virtual ModulePtr newModule();

    /// Constructor
    CDCTriggerModule();
//    CDCTriggerModule(const std::string & type);

    /// Destructor
    virtual ~CDCTriggerModule();

    /// Initilizes CDCTriggerModule.
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

    /// returns version of CDCTriggerModule.
    std::string version(void) const;

  private:
    int _debugLevel;
//  const CDCTrigger * _cdc;
//  CDCTrigger * _cdc;
    int _testParamInt;
};

} // namespace Belle2

#endif // CDCTriggerModule_H
