//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Module.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/08 : First version
//-----------------------------------------------------------------------------

#ifndef TRGCDCModule_H
#define TRGCDCModule_H

#include <string>
#include "framework/core/Module.h"
#include "trg/cdc/TRGCDC.h"

namespace Belle2 {

/// A module to process CDC trigger data
class TRGCDCModule : public Module {

  public:

//     /// returns a pointe to TRGCDCModule.
//     virtual ModulePtr newModule();

    /// Constructor
    TRGCDCModule();
//    TRGCDCModule(const std::string & type);

    /// Destructor
    virtual ~TRGCDCModule();

    /// Initilizes TRGCDCModule.
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

    /// returns version of TRGCDCModule.
    std::string version(void) const;

  private:
    int _debugLevel;
//  const TRGCDC * _cdc;
//  TRGCDC * _cdc;
    int _testParamInt;
    std::string _configFilename;
};

} // namespace Belle2

#endif // TRGCDCModule_H
