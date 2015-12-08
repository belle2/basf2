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

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

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

    /** list to store TObjects */
    TList* m_histoList;

    /** method to create TEfficiencies */
    TEfficiency* createEfficiency(const char* name, const char* title,
                                  Int_t nbins, Double_t min, Double_t max, TList* histoList = NULL);

    /** method to fill TEfficiencies */
    void fillEfficiencyHistos(const TrackFitResult* fitResult, const PIDLikelihood* pid, int pdg);

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    int m_nbins; /**< number of bins in momentum range */
    double m_p_lower; /**< lower limit of momentum range */
    double m_p_upper; /**< upper limit of momentum range */

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

    // determine which detectors to use for each particle
    Const::DetectorSet chargedSet;  /**< pions, kaons, protons */
    Const::DetectorSet electronSet; /**< electrons */
    Const::DetectorSet muonSet;     /**< muons */

  };
} // end of namespace

#endif
