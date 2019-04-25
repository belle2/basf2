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

    /**
     * Compute PID efficiency vs clusterTheta, clusterPhi, p... for a fixed cut on PID as speciified below.
     */
    void computePIDEfficiency();

    /**
     * Compute track-to-ECL-cluster matching efficiency vs clusterTheta, clusterPhi, p....
     */
    void computeMatchingEfficiency();

  private:

    /**
     * Defintion of the PID cut threshold to compute the efficiency.
     * The chosen value is arbitrary.
     */
    static constexpr float c_PID = 0.5;

    /**
     * Output ROOT file used for storing info.
     */
    TFile* m_outputFile = nullptr;

    /**
     * Name of the output ROOT file.
     */
    std::string m_outputFileName;

    /**
     *The pdgId of the charged stable particle of interest.
     */
    int m_inputPdgId;

    /**
     *The pdgId (as a string) of the charged stable particle of interest.
     */
    std::string m_inputPdgIdStr;

    /**
     * TTree.
     */
    TTree* m_tree = nullptr;

    /**
     * Track momentum in [GeV/c].
     */
    float m_p;

    /**
     * Cluster polar angle in [rad]
     */
    float m_clusterTheta;

    /**
     * Cluster azimuthal angle in [rad]
     */
    float m_clusterPhi;

    /**
     * Track has matching ECL cluster.
     */
    char m_trackClusterMatch;

    /**
     * Global PID as likelihod ratio:
     *
     * pid = L_sig / (L_sig + L_bkg0 + L_bkg1 + ...)
     *
     * for the m_inputPdgId particle hypothesis.
     */
    float m_pid;

  };
}

