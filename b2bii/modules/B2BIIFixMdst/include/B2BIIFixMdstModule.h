#ifndef B2BII_FIX_MDST_H
#define B2BII_FIX_MDST_H

#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module fixes Belle MDST objects (Panther records).
   */

  class B2BIIFixMdstModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    B2BIIFixMdstModule();
    virtual ~B2BIIFixMdstModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

  };

} // end namespace Belle2

#endif
