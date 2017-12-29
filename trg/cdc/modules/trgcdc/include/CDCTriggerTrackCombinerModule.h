#ifndef CDCTRIGGERTRACKCOMBINERModule_H
#define CDCTRIGGERTRACKCOMBINERModule_H

#include "framework/core/Module.h"
#include <string>

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>

namespace Belle2 {

  /** Module to combine the information from the various track trigger stages. */
  class CDCTriggerTrackCombinerModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    CDCTriggerTrackCombinerModule();

    /** Destructor */
    virtual ~CDCTriggerTrackCombinerModule() {}

    /** Initialize the module and register DataStore arrays. */
    virtual void initialize();

    /** Combine tracks. */
    virtual void event();

  protected:
    /** name of the 2D finder track list */
    std::string m_2DfinderCollectionName;
    /** name of the 2D fitter track list */
    std::string m_2DfitterCollectionName;
    /** name of the 3D fitter track list */
    std::string m_3DfitterCollectionName;
    /** name of the neuro track list */
    std::string m_neuroCollectionName;
    /** name of the output track list for the combined tracks */
    std::string m_outputCollectionName;
    /** name of track segment hit list (for relations) */
    std::string m_hitCollectionName;
    /** defines how to calculate cot(theta) of the combined track */
    std::string m_thetaDefinition;
    /** defines how to calculate z0 of the combined track */
    std::string m_zDefinition;

  private:
    /** list of 2D finder tracks (all others are obtained via relations) */
    StoreArray<CDCTriggerTrack> m_tracks2Dfinder;
    /** list of combined output tracks */
    StoreArray<CDCTriggerTrack> m_tracksCombined;
    /** list of track segment hits */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
  };

} // namespace Belle2

#endif // CDCTriggerTrackCombinerModule_H
