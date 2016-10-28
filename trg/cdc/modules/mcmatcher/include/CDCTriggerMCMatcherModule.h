#ifndef CDCTRIGGERMCMATCHERMODULE_H
#define CDCTRIGGERMCMATCHERMODULE_H

#include <framework/core/Module.h>

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
  };
}
#endif
