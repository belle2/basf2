#ifndef TOP_BACK_SPLASH_TIMING_MODULE_H
#define TOP_BACK_SPLASH_TIMING_MODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBackSplashFitResult.h>
#include <top/dataobjects/TOPDigit.h>

#include <RooAbsPdf.h>
#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooFitResult.h>
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
    TOPBackSplashFitResult* fitTimingDigits(int, std::vector<int>, float, int);
    //void makePlot();
    //void getMCNbar();
    StoreArray<ECLCluster> m_eclClusters;
    StoreArray<TOPDigit> m_digits;
    StoreArray<MCParticle> m_MCParticles; // for debugging nbar
    StoreArray<TOPBackSplashFitResult> m_fitresult;
    bool m_debug = false; // Debug mode: save fits to TOP timing and print generated nbar info
    void makePlot(float, int, int, RooAbsPdf*, RooAbsPdf*, RooAbsPdf*, RooRealVar*, RooDataSet, double, RooFitResult*);
  };

} // namespace Belle2

#endif
