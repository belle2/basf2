#ifndef TOP_BACK_SPLASH_TIMING_MODULE_H
#define TOP_BACK_SPLASH_TIMING_MODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPDigit.h>

namespace Belle2 {

  class TOPBackSplashTimingModule : public Module {

  public:
    TOPBackSplashTimingModule();
    virtual ~TOPBackSplashTimingModule() = default;

    void initialize() override;
    void event() override;

  private:
    StoreArray<ECLCluster> m_eclClusters;
    StoreArray<TOPDigit> m_digits;
    StoreArray<MCParticle> m_MCParticles; // for debugging nbar
    //std:: ;//fitparams?
  };

} // namespace Belle2

#endif
