#ifndef TRGBACKTOBACKMODULE_H
#define TRGBACKTOBACKMODULE_H

#include "framework/core/Module.h"

namespace Belle2 {

  /** Module that returns true if the back-to-back condition
   *  of track segments in SL 2 is fulfilled (for 2017 cosmic test). */
  class TRGBackToBackModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    TRGBackToBackModule();

    /** Destructor */
    virtual ~TRGBackToBackModule() {}

    /** Initialize the module. */
    virtual void initialize();

    /** Check the back-to-back condition and set return value. */
    virtual void event();

  protected:
    /** name of track segment hit list */
    std::string m_hitCollectionName;

  };

} // namespace Belle2

#endif // TRGBACKTOBACKMODULE_H
