/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
  /**
   * A module to extract and save the timing of TOP signal in front of neutral
   * hadron ECL clusters (i.e. from hadrons showers where charged particles
   * 'backsplash' from ECL towards TOP)
   */
  class TOPBackSplashTimingModule : public Module {

  public:
    TOPBackSplashTimingModule();
    virtual ~TOPBackSplashTimingModule() = default;

    void initialize() override;
    void event() override;

  private:
    StoreArray<ECLCluster> m_eclClusters;
    StoreArray<TOPBackSplashFitResult> m_fitresult;
    StoreArray<TOPDigit> m_digits;

    TOPBackSplashFitResult* fitTimingDigits(int, std::vector<int>, float, int);
    std::array<std::array<double, 11>, 15> m_fitparams;
    std::vector<RooWorkspace> m_wss;
    std::vector<RooWorkspace> prepareFitModels();

    bool m_saveFits = false; // Debug mode: save fits to TOP timing and print generated nbar info
    int convertCosThetaToIndex(float);
    int getModuleFromPhi(double);
    void makePlot(float, int, int, RooAbsPdf*, RooRealVar*, RooDataSet, double, RooFitResult*);
  };

} // namespace Belle2

#endif
