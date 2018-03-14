#ifndef CDCTRIGGERMCMATCHERMODULE_H
#define CDCTRIGGERMCMATCHERMODULE_H

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {

  /** A module to match CDCTriggerTracks to MCParticles.
   */
  class CDCTriggerMCMatcherModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerMCMatcherModule();

    /** Destructor. */
    virtual ~CDCTriggerMCMatcherModule() {}

    /** Initialize the module. */
    virtual void initialize();

    /** Called once for each event. */
    virtual void event();

  protected:
    /** Name of the MCParticle StoreArray to be matched */
    std::string m_MCParticleCollectionName;
    /** Name of the CDCTriggerTrack Store Array to be matched */
    std::string m_TrgTrackCollectionName;
    /** Name of a new StoreArray holding MCParticles considered as trackable */
    std::string m_MCTrackableCollectionName;
    /** Name of the StoreArray containing the hits that are used for the matching. */
    std::string m_hitCollectionName;
    /** minimum number of axial hits to consider a MCParticle as trackable */
    int m_minAxial;
    /** minimum number of stereo hits to consider a MCParticle as trackable */
    int m_minStereo;
    /** switch for 2D matching */
    bool m_axialOnly;
    /** minimum purity */
    double m_minPurity;
    /** minimum efficiency */
    double m_minEfficiency;
    /** switch for ignoring secondary particles */
    bool m_onlyPrimaries;
    /** switch for creating relations for clones and merged tracks */
    bool m_relateClonesAndMerged;

    /** list of hits that are used for the matching */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    /** list of CDCTriggerTracks to be matched */
    StoreArray<CDCTriggerTrack> m_prTracks;
    /** list of MCParticles to be matched */
    StoreArray<MCParticle> m_mcParticles;
    /** list of MCParticles considered as trackable */
    StoreArray<MCParticle> m_mcTracks;
  };
}
#endif
