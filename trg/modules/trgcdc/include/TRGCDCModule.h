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
#include "trg/cdc/SteppingAction.h"

namespace Belle2 {

/// A module to process CDC trigger data
class TRGCDCModule : public Module {

  public:

    /// Constructor
    TRGCDCModule();
//  TRGCDCModule(const std::string & type);

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

  private: // Parameters

    /// Debug level.
    int _debugLevel;

    /// Config. file name.
    std::string _configFilename;

    /// Curl back stop parameter. 0:do nothing, 1:stop curling. Default is 0.
    int _curlBackStop;

    /// Switch for the perfect finder.
    bool _hFinderPerfect;

    /// \# of X mesh for the Hough finder.
    unsigned _hFinderMeshX;

    /// \# of Y mesh for the Hough finder.
    unsigned _hFinderMeshY;

  private:

    /// A pointer to a TRGCDC;
    TRGCDC * _cdc;

    /// A pointer to a TRGCDCSteppingAction which stops curl backs.
    TRGCDCSteppingAction * _sa;
};

} // namespace Belle2

#endif // TRGCDCModule_H
