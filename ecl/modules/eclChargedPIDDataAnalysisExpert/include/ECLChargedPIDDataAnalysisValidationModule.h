/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi                                             *
 * Contact: marco.milesi@unimelb.edu.au                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/

#pragma once

// ROOT
#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>
#include <TEfficiency.h>

// ECL
#include <ecl/dataobjects/ECLPidLikelihood.h>

// MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /**
   * This module dumps a tree and a set of histograms of ECL PID-related info used for validation.
   */
  class ECLChargedPIDDataAnalysisValidationModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ECLChargedPIDDataAnalysisValidationModule();

    /**
     * Destructor of the module.
     */
    virtual ~ECLChargedPIDDataAnalysisValidationModule();

    /**
     * Initializes the module.
     */
    virtual void initialize() override;

    /**
     * Called once before a new run begins.
     */
    virtual void beginRun() override;

    /**
     * Called once for each event.
     */
    virtual void event() override;

    /**
     * Called once when a run ends.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     */
    virtual void terminate() override;

  private:

    /**
     * Defintion of the PID cut threshold to compute the efficiency.
     * The chosen value is arbitrary.
     */
    static constexpr float c_PID = 0.5;

    /**
     * The maximal number of charged stable particle hypotheses.
     * this takes particles & antiparticles into account.
     */
    static constexpr unsigned int c_chargedStableHypos = 2 * Const::ChargedStable::c_SetSize;

    /**
     * The pdgId list of the charged stable particles of interest.
     */
    std::vector<int> m_inputPdgIdList;

    /**
     * The pdgIds of the charged stable particles of interest.
     * Make it a std::set to ensure its elements are unique.
     */
    std::set<int> m_inputPdgIdSet;

    /**
     * Output ROOT file used for storing info.
     */
    std::vector<TFile*> m_outputFile = std::vector<TFile*>(c_chargedStableHypos);

    /**
     * Base name of the output ROOT file.
     */
    std::string m_outputFileName;

    /**
     * TTree.
     */
    std::vector<TTree*> m_tree = std::vector<TTree*>(c_chargedStableHypos);

    /**
     * Track momentum in [GeV/c].
     */
    std::vector<float> m_p = std::vector<float>(c_chargedStableHypos);

    /**
     * Track polar angle in [rad].
     */
    std::vector<float> m_trkTheta = std::vector<float>(c_chargedStableHypos);

    /**
     * Track azimuthal angle in [rad].
     */
    std::vector<float> m_trkPhi = std::vector<float>(c_chargedStableHypos);

    /**
     * Cluster polar angle in [rad].
     */
    std::vector<float> m_clusterTheta = std::vector<float>(c_chargedStableHypos);

    /**
     * Cluster azimuthal angle in [rad].
     */
    std::vector<float> m_clusterPhi = std::vector<float>(c_chargedStableHypos);

    /**
     * Track has matching ECL cluster.
     */
    std::vector<char> m_trackClusterMatch = std::vector<char>(c_chargedStableHypos);

    /**
     * Global PID as likelihod ratio:
     *
     * pid = L_sig / (L_sig + L_bkg0 + L_bkg1 + ...)
     *
     * for the m_inputPdgId particle hypothesis.
     */
    std::vector<float> m_pid = std::vector<float>(c_chargedStableHypos);

    /**
     * Binning w/ variable bin size for track momentum (in [GeV/c]).
     * It should match the binning used for parametrisation of the PID likelihood.
     */
    std::vector<float> m_p_binedges = {0.0, 0.5, 0.75, 1.0, 3.0, 5.0};

    /**
     * Binning w/ variable bin size for cluster polar angle (in [rad]).
     * It should match the binning used for parametrisation of the PID likelihood.
     */
    std::vector<float> m_th_binedges = {0.0, 0.2164208, 0.5480334, 0.561996, 2.2462387, 2.2811453, 2.7070057, 3.1415926};

    /**
     * Compute PID efficiency vs clusterTheta, clusterPhi, p... for a fixed cut on PID as previously initialised.
     */
    void computePIDEfficiency(TTree* tree, const std::string& pdgIdStr);

    /**
     * Compute track-to-ECL-cluster matching efficiency vs clusterTheta, clusterPhi, p....
     */
    void computeMatchingEfficiency(TTree* tree, const std::string& pdgIdStr);

    /**
     * Check if the input pdgId is that of a valid charged particle.
     */
    bool isValidChargedPdg(const int pdg) const
    {
      return (Const::chargedStableSet.find(pdg) != Const::invalidParticle);
    }

  };
}
