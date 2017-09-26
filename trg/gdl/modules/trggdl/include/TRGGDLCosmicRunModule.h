#ifndef TRGGDLCOSMICRUNMODULE_H
#define TRGGDLCOSMICRUNMODULE_H

#include "framework/core/Module.h"

namespace Belle2 {

  /** Module that returns true if the trigger condition
   *  for the 2017 cosmic runs is fulfilled. */
  class TRGGDLCosmicRunModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    TRGGDLCosmicRunModule();

    /** Destructor */
    virtual ~TRGGDLCosmicRunModule() {}

    /** Initialize the module. */
    virtual void initialize();

    /** Check the trigger condition and set return value. */
    virtual void event();

  protected:
    /** name of track segment hit list */
    std::string m_tsHitCollectionName;
    /** switch for back-to-back condition */
    bool m_backToBack;
    /** switch for turning off the ECL part */
    bool m_skipECL;

  };

} // namespace Belle2

#endif // TRGGDLCOSMICRUNMODULE_H
