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
   * This module dumps a tree and a set of histograms of ECL PID-related info used for validation,
   * starting from an input file w/ particle-gun-generated charged stable particles (and antiparticles).
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
     * This includes particles and antiparticles.
     */
    static constexpr unsigned int c_chargedStableHypos = 2 * Const::ChargedStable::c_SetSize;

    /**
     * The pdgId list of the charged stable particles of interest.
     * This is a configurable parameter.
     */
    std::vector<int> m_inputPdgIdList;

    /**
     * The pdgId set of the charged stable particles of interest.
     * Using `std::set` ensures its elements are unique.
     */
    std::set<int> m_inputPdgIdSet;

    /**
     * Output `ROOT::TFile` that contains the info to plot.
     *
     * Book one `TFile` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<TFile*> m_outputFile = std::vector<TFile*>(c_chargedStableHypos);

    /**
     * Base name of the output `ROOT::TFile`.
     * This is a configurable parameter.
     */
    std::string m_outputFileName;

    /**
     * Save the `TTree` in the output file alongside the histograms.
     * This is a configurable parameter.
     */
    bool m_saveValidationTree;

    /**
     * A `ROOT::TTree` filled with the info to make control plots.
     *
     * Book one `TTree` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<TTree*> m_tree = std::vector<TTree*>(c_chargedStableHypos);

    /**
     * Track momentum in [GeV/c].
     *
     * Use the MC-matched reconstructed track w/ highest momentum.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_p = std::vector<float>(c_chargedStableHypos);

    /**
     * Track transverse momentum in [GeV/c].
     *
     * Use the MC-matched reconstructed track w/ highest momentum.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_pt = std::vector<float>(c_chargedStableHypos);

    /**
     * Track polar angle in [rad].
     *
     * Use the MC-matched reconstructed track w/ highest momentum.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_trkTheta = std::vector<float>(c_chargedStableHypos);

    /**
     * Track azimuthal angle in [rad].
     *
     * Use the MC-matched reconstructed track w/ highest momentum.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_trkPhi = std::vector<float>(c_chargedStableHypos);

    /**
     * Cluster polar angle in [rad].
     *
     * Use the most energetic ECL cluster associated to the MC-matched reconstructed track w/ highest momentum.
     * A NaN value is stored if no matching is found.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_clusterTheta = std::vector<float>(c_chargedStableHypos);

    /**
     * Cluster azimuthal angle in [rad].
     *
     * Use the most energetic ECL cluster associated to the MC-matched reconstructed track w/ highest momentum.
     * A NaN value is stored if no matching is found.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_clusterPhi = std::vector<float>(c_chargedStableHypos);

    /**
     * Flag for track-cluster matching condition.
     *
     * Book one `char` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<char> m_trackClusterMatch = std::vector<char>(c_chargedStableHypos);

    /**
     * Log-likelihood for the "signal" particle hypothesis.
     *
     * Here, "signal" refers to the charged stable particle under exam.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_logl_sig = std::vector<float>(c_chargedStableHypos);

    /**
     * Log-likelihood for the "background" particle hypothesis.
     *
     * The "background" hypothesis is defined according to the charged stable particle under exam:
     *
     *     chargedStableBkg = (chargedStable != Const::pion) ? Const::pion : Const::kaon.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_logl_bkg = std::vector<float>(c_chargedStableHypos);

    /**
     * Delta Log-likelihood "signal" vs. "background".
     *
     * \f[
     *   \Delta log(\mathcal{L}) = log(\mathcal{L}_{bkg}) - log(\mathcal{L}_{sig}).
     * \f]
     *
     * Here, "signal" refers to the charged stable particle under exam.
     * The "background" hypothesis is defined according to the charged stable particle under exam:
     *
     *     chargedStableBkg = (chargedStable != Const::pion) ? Const::pion : Const::kaon.
     *
     * Book one `float` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<float> m_deltalogl_sig_bkg = std::vector<float>(c_chargedStableHypos);

    /**
     * List of global PIDs, defined by the likelihod ratio:
     *
     * \f[
     *   PID_{i} = \frac{\matchal{L}_{i}}{\sum_{j}\matchal{L}_{j}},
     * \f]
     *
     * where \f$i\fS represents each charged stable particle hypothesis as defined in `Const::chargedStableSet`.
     *
     * Book one `std::vector<float>` for each charged stable particle (and antiparticle) candidate.
     */
    std::vector<std::vector<float>> m_pids_glob = std::vector<std::vector<float>>(c_chargedStableHypos,
                                                  std::vector<float>(Const::ChargedStable::c_SetSize));

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
     * Dump PID vars.
     */
    void dumpPIDVars(TTree* sampleTree, const Const::ChargedStable& sigHypo, const int sigCharge, const Const::ChargedStable& bkgHypo);

    /**
     * Dump PID efficiency / fake rate vs clusterTheta, clusterPhi, p... for a fixed cut on PID as previously initialised.
     *
     * \param[in] sampleTree the `TTree` of the charged particle under consideration.
     * \param[in] sampleHypo the `Const::ChargedStable` hypothesis corresponding to the charged particle under consideration.
     * \param[in] sampleCharge the charge (+/- 1) of the charged particle under consideration.
     * \param[in] sigHypo the `Const::ChargedStable` "signal" hypothesis to test.
     *
     * If sampleHypo == sigHypo, will be measuring an efficiency, otherwise a fake rate.
     */
    void dumpPIDEfficiencyFakeRate(TTree* sampleTree, const Const::ChargedStable& sampleHypo, const int sampleCharge,
                                   const Const::ChargedStable& sigHypo);

    /**
     * Dump track-to-ECL-cluster matching efficiency vs clusterTheta, clusterPhi, pt....
     *
     * \param[in] sampleTree the `TTree` of the charged particle under consideration.
     * \param[in] sampleHypo the `Const::ChargedStable` hypothesis corresponding to the charged particle under consideration.
     * \param[in] sampleCharge the charge of the charged particle under consideration.
     *
     */
    void dumpTrkClusMatchingEfficiency(TTree* sampleTree, const Const::ChargedStable& sampleHypo, const int sampleCharge);

    /**
     * Check if the input pdgId is that of a valid charged stable particle.
     */
    inline bool isValidChargedPdg(const int pdg) const
    {
      return (Const::chargedStableSet.find(pdg) != Const::invalidParticle);
    }

    /**
     * Draw u/oflow content on top of first/last visible bin.
     */
    void paintUnderOverflow(TH1F* h);

  };
}
