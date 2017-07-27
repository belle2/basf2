#ifndef TRGGDLCOSMICRUNMODULE_H
#define TRGGDLCOSMICRUNMODULE_H

#include "framework/core/Module.h"

namespace Belle2 {

  /** Module that returns true if the back-to-back condition
   *  of track segments in SL 2 is fulfilled (for 2017 cosmic test). */
  class TRGGDLCosmicRunModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    TRGGDLCosmicRunModule();

    /** Destructor */
    virtual ~TRGGDLCosmicRunModule() {}

    /** Initialize the module. */
    virtual void initialize();

    /** Check the back-to-back condition and set return value. */
    virtual void event();

  protected:
    /** name of track segment hit list */
    std::string m_hitCollectionName;

  };

} // namespace Belle2

#endif // TRGGDLCOSMICRUNMODULE_H
