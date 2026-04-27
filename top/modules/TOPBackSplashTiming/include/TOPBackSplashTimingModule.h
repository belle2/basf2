/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

# pragma once
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TOPBackSplashFitResult.h>
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
    /**
     * Constructor
     */
    TOPBackSplashTimingModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override;

    /**
     * Event processor.
     */
    void event() override;

  private:
    StoreArray<ECLCluster> m_eclClusters; /**< StoreArray of ECLCluster */
    StoreArray<TOPBackSplashFitResult> m_fitresult; /**< StoreArray of TOPBackSplashFitResult */
    StoreArray<TOPDigit> m_digits; /**< StoreArray of TOPDigit */

    TOPBackSplashFitResult* fitTimingDigits(int, std::vector<const TOPDigit*>&, double,
                                            double); /**< Perform fitting of TOP timing in nearby slot  */
    std::array<std::array<double, 11>, 15> m_fitparams; /**< container of 11 TOP timing fit params per cosTheta */
    std::vector<RooWorkspace> m_wss; /**< container of RooWorkSpaces, containing fit funcs per cosTheta */
    void prepareFitModels(); /**< constructs RooFit objects for fitting */

    bool m_saveFits = false; /**< Debug mode: plot and save fits of TOP timing */
    double m_minClusterE = 0.5; /**< minimum energy of ECL clusters to consider [GeV] */
    int m_minNphotons = 2; /**< minimum no. of Cherenkov photons to perform fit */
    double m_minClusterNHits = 1.0; /**< minimum no. of crystals in clusters (non-integer with overlapping clusters) */

    int convertCosThetaToIndex(double); /**< maps rounded cosTheta to appropriate row index of fit parameters */
    int getModuleFromPhi(double); /**< maps azimuthal angle to corresponding TOP slot no. */
    /** Function to create and save RooPlots of fitted TOP timing */
    void makePlot(double, double, int, RooAbsPdf*, RooRealVar*, RooDataSet, RooFitResult*);
  };

} //end namespace Belle2
