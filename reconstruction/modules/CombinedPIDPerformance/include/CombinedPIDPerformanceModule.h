/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef COMBINEDPIDPERFORMANCEMODULE_H_
#define COMBINEDPIDPERFORMANCEMODULE_H_

#include <framework/core/Module.h>
#include <TTree.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TEfficiency.h>
#include <TH3F.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {
  class MCParticle;
  class Track;
  class TrackFitResult;
  class PIDLikelihood;

  template< class T >
  class StoreArray;

  /** This module takes the MCParticles, the Tracks, and the PIDLikelihoods as input and
   *  produces a root file containing various histograms showing the PID performance
   */
  class CombinedPIDPerformanceModule : public Module {

  public:

    /** Default constructor */
    CombinedPIDPerformanceModule();

    /** Destructor */
    virtual ~CombinedPIDPerformanceModule();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each event */
    virtual void event();

    /** End of the event processing */
    virtual void terminate();

  private:

    // required input
    StoreArray<Track> m_tracks; /**< Required array of input Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required array of input TrackFitResults */
    StoreArray<PIDLikelihood> m_pidLikelihoods; /**< Required array of input PIDLikelihoods */
    StoreArray<MCParticle> m_mcParticles; /**< Required array of input MCParticles */

    /** list to store TObjects */
    TList* m_histoList;

    /** method to create TEfficiencies */
    TEfficiency* createEfficiency(const char* name, const char* title,
                                  Int_t nbins, Double_t min, Double_t max, TList* histoList = NULL);

    /** method to fill TEfficiencies */
    void fillEfficiencyHistos(const TrackFitResult* fitResult, const PIDLikelihood* pid, int pdg);

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_mdstType;       /**< flag for Belle/BelleII mdst files */
    int m_nbins;                  /**< number of momentum bins */
    double m_pLow;                /**< lower bound of momentum range */
    double m_pHigh;               /**< upper bound of momentum range */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    /* list of histograms filled per MCParticle found in the event */
    // TEfficiencies for each particle type
    std::vector< TEfficiency* > m_piK_Efficiencies; /**< pion efficiencies */
    std::vector< TEfficiency* > m_Kpi_Efficiencies; /**< kaon efficiencies */
    std::vector< TEfficiency* > m_ppi_Efficiencies; /**< proton efficiencies */
    std::vector< TEfficiency* > m_pK_Efficiencies;  /**< proton efficiencies */
    std::vector< TEfficiency* > m_dpi_Efficiencies; /**< deuteron efficiencies */
    std::vector< TEfficiency* > m_epi_Efficiencies; /**< electron efficiencies */
    std::vector< TEfficiency* > m_mpi_Efficiencies; /**< muon efficiencies */

    // TEfficiencies for fake rates
    std::vector< TEfficiency* > m_piK_FakeRates;    /**< pion fake rates */
    std::vector< TEfficiency* > m_Kpi_FakeRates;    /**< kaon fake rates */
    std::vector< TEfficiency* > m_ppi_FakeRates;    /**< proton fake rates */
    std::vector< TEfficiency* > m_pK_FakeRates;     /**< proton fake rates */
    std::vector< TEfficiency* > m_dpi_FakeRates;    /**< deuteron fake rates */
    std::vector< TEfficiency* > m_epi_FakeRates;    /**< electron fake rates */
    std::vector< TEfficiency* > m_mpi_FakeRates;    /**< muon fake rates */

    TH3F* h_ROC[5][10]; /**< ROC histograms */

    /** returns the likelihood ratio for given log likelihoods */
    double pidvalue(float pida, float pidb);

    /** determine the availability of the given detector(s) */
    bool pidavail(const PIDLikelihood* pid, Const::DetectorSet dets);

    // determine which detectors to use for each particle
    Const::DetectorSet chargedSet;  /**< pions, kaons, protons */
    Const::DetectorSet electronSet; /**< electrons */
    Const::DetectorSet muonSet;     /**< muons */

    /** enumeration for detector numbering */
    enum dettype {svd, cdc, top, arich, klm, ecl, dedx, dedxtop, dedxecl, all};
    std::vector<int> detset;  /**< set of detectors used for hadrons */
    std::vector<int> edetset; /**< set of detectors used electrons */

  };
} // end of namespace

#endif
