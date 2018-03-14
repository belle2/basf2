#ifndef TRGGDLCOSMICRUNMODULE_H
#define TRGGDLCOSMICRUNMODULE_H

#include "framework/core/Module.h"

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/ecl/dataobjects/TRGECLTrg.h>

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

    /** list of track segment hits */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    /** list of ECL trigger hits */
    StoreArray<TRGECLTrg> m_tchit;
  };

} // namespace Belle2

#endif // TRGGDLCOSMICRUNMODULE_H
