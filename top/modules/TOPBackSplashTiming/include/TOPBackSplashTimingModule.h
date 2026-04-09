#ifndef TOP_BACK_SPLASH_TIMING_MODULE_H
#define TOP_BACK_SPLASH_TIMING_MODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBackSplashFitResult.h>
#include <top/dataobjects/TOPDigit.h>

#include <RooWorkspace.h>

namespace Belle2 {

  class TOPBackSplashTimingModule : public Module {

  public:
    TOPBackSplashTimingModule();
    virtual ~TOPBackSplashTimingModule() = default;

    void initialize() override;
    void event() override;

  private:
    std::vector<RooWorkspace> prepareFitModels();
    std::array<std::array<double, 11>, 15> m_fitparams;
    std::vector<RooWorkspace> m_wss;
    int convertCosThetaToIndex(float);
    int getModuleFromPhi(double);
    void fitTimingDigits(TOPBackSplashFitResult*, int, std::vector<int>, float, int);
    //void makePlot();
    //void getMCNbar();
    StoreArray<ECLCluster> m_eclClusters;
    StoreArray<TOPDigit> m_digits;
    StoreArray<MCParticle> m_MCParticles; // for debugging nbar
    StoreArray<TOPBackSplashFitResult> m_fitresult;
    //DBObjPtr<TObject> m_fitPdfCosTheta0p0;
    //std::map<float,DBObjPtr<TObject>> m_fitPdfs;
    //std::map<float,DBObjPtr<TObject>> m_fitPdfXs;
  };

} // namespace Belle2

#endif
