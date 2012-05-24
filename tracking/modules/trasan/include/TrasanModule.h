//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrasanModule.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A track finding module
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/15 : First version
//-----------------------------------------------------------------------------

#ifndef TrasanModule_H
#define TrasanModule_H

#include <string>
#include "framework/core/Module.h"
#include "tracking/modules/trasan/Trasan.h"

namespace Belle {
  class Trasan;
}

namespace Belle2 {

/// A track finding module using Belle track finder Trasan.
  class TrasanModule : public Module {

  public:

    /// Constructor
    TrasanModule();

    /// Destructor
    virtual ~TrasanModule();

    /// Initilizes TrasanModule.
    virtual void initialize();

    /// Called when new run started.
    virtual void beginRun();

    /// Called event by event.
    virtual void event();

    /// Called when run ended.
    virtual void endRun();

    /// Called when processing ended.
    virtual void terminate();

  private:

    /// Debug level.
    int _debugLevel;

    /// GFTrackCandidate name.
    std::string _gfTrackCandsName;

    /// Trasan.
    Belle::Trasan _tra;
  };

} // namespace Belle2

#endif // TrasanModule_H
