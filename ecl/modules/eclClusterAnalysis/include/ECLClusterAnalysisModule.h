/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Elisa Manoni, Benjamin Oberhof                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <string>

class TFile;
class TTree;

namespace Belle2 {
  class ECLCluster;
  class MCParticle;
  class Track;
  class TrackFitResult;

  /** The ECL Data Analysis Module
   *
   * this module dump an ntuple containing ECL-related infos starting from mdst
   *
   */




  class ECLClusterAnalysisModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ECLClusterAnalysisModule();

    /**
     * Destructor of the module.
     */
    virtual ~ECLClusterAnalysisModule();

    /**
     *Initializes the Module.
     */
    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  private:

    /** members of ECLReconstructor Module */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing info */
    std::string m_rootFileName; /**< name of the root file */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */
    bool m_doTracking; /**< if true, info on tracking will be stored, job will fail if doTracking==1 and the tracking modules are not enabled at phyton level */

    TTree* m_tree; /**< Root tree and file for saving the output */

    // variables
    int m_iExperiment; /**< Experiment number */
    int m_iRun; /**< Run number */
    int m_iEvent; /**< Event number */

    int m_eclClusterMultip;  /**< Number of ECLClusterss per event */
    std::vector<int>* m_eclClusterIdx;  /**< ECLCluster index */
    std::vector<int>* m_eclClusterToMc1; /**< Index of first MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight1; /**< Energy contribution of first MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMc2; /**< Index of second MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight2; /**< Energy contribution of second MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMc3; /**< Index of third MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight3; /**< Energy contribution of third MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMc4; /**< Index of fourth MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight4; /**< Energy contribution of fourth MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMc5; /**< Index of fifth MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight5; /**< Energy contribution of fifth MCParticle related to ECLCluster */
    std::vector<double>*
    m_eclClusterToBkgWeight; /**< Remaining energy contribution not associated to first five MCParticles related to ECLCluster */
    std::vector<double>* m_eclClusterSimHitSum; /**< Energy contribution of first MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterEnergy; /**< ECLCluster energy */
    std::vector<double>* m_eclClusterEnergyError; /**< ECLCluster energy error*/
    std::vector<double>* m_eclClusterTheta;  /**< ECLCluster polar direction */
    std::vector<double>* m_eclClusterThetaError;  /**< ECLCluster error on polar direction */
    std::vector<double>* m_eclClusterPhi;  /**< ECLCluster azimuthal direction */
    std::vector<double>* m_eclClusterPhiError;  /**< ECLCluster error on azimuthal direction */
    std::vector<double>* m_eclClusterR;  /**< ECLCluster distance from IP */
    std::vector<double>* m_eclClusterEnergyDepSum;  /**< ECLCluster simulated energy */
    std::vector<double>* m_eclClusterTiming;  /**< ECLCluster time */
    std::vector<double>* m_eclClusterTimingError;  /**< ECLCluster time error */
    std::vector<double>* m_eclClusterE9oE25;  /**< Ratio of 3x3 over 5x5 crystal matrices energies for ECLCluster*/
    std::vector<double>* m_eclClusterHighestE; /**< Highest energy deposit (per crystal) in ECLCluster */
    std::vector<double>* m_eclClusterLat; /**< ECLCluster shape parameter */
    std::vector<int>* m_eclClusterNofCrystals;  /**< Number of crystals in ECLCluster */
    std::vector<int>* m_eclClusterCrystalHealth;  /**< Crystal healt flag */
    std::vector<double>* m_eclClusterPx;  /**< Reconstructed momentum along X */
    std::vector<double>* m_eclClusterPy;  /**< Reconstructed momentum along Y */
    std::vector<double>* m_eclClusterPz;  /**< Reconstructed momentum along Z */
    std::vector<bool>* m_eclClusterIsTrack; /**< Flag for charged clusters */
    std::vector<double>* m_eclClusterDeltaL; /**< ??? */

    int m_mcMultip; /**< Multiplicity of MCParticles */
    std::vector<int>* m_mcIdx; /**< MCParticle index */
    std::vector<int>* m_mcPdg; /**< MCParticle PDG code */
    std::vector<int>* m_mcMothPdg; /**< MCParticle mother particle PDG code */
    std::vector<int>* m_mcGMothPdg; /**< MCParticle grandmother particle PDG code */
    std::vector<int>* m_mcGGMothPdg; /**< MCParticle greand-grandmother particle PDG code */
    std::vector<double>* m_mcEnergy; /**< MCParticle energyx */
    std::vector<double>* m_mcPx; /**< MCParticle momentum X direction */
    std::vector<double>* m_mcPy; /**< MCParticle momentum Y direction */
    std::vector<double>* m_mcPz; /**< MCParticle momentum Z direction */
    std::vector<double>* m_mcDecayVtxX; /**< MCParticle decay vertex X */
    std::vector<double>* m_mcDecayVtxY; /**< MCParticle decay vertex Y */
    std::vector<double>* m_mcDecayVtxZ; /**< MCParticle decay vertex Z */
    std::vector<double>* m_mcProdVtxX; /**< MCParticle production vertex X */
    std::vector<double>* m_mcProdVtxY; /**< MCParticle production vertex Y */
    std::vector<double>* m_mcProdVtxZ; /**< MCParticle production vertex Z */
    std::vector<int>* m_mcSecondaryPhysProc; /**< Flag for secondary physics process */

    int m_trkMultip; /**< Track Multiplicity */
    std::vector<int>* m_trkPdg; /**< Track PDG code */
    std::vector<int>* m_trkIdx; /**< Track index */
    std::vector<int>* m_trkCharge; /**< Track charge */
    std::vector<double>* m_trkPx; /**< Track momentum along X direction */
    std::vector<double>* m_trkPy; /**< Track momentum along Y direction */
    std::vector<double>* m_trkPz; /**< Track momentum along Z direction */
    std::vector<double>* m_trkP; /**< Track momentum */
    std::vector<double>* m_trkTheta; /**< Track polar direction */
    std::vector<double>* m_trkPhi; /**< Track azimuthal direction */
    std::vector<double>* m_trkX; /**< Track DOCA X (?) */
    std::vector<double>* m_trkY; /**< Track DOCA Y (?) */
    std::vector<double>* m_trkZ; /**< Track DOCA Z (?) */

    std::vector<int>* m_eclpidtrkIdx; /**< PID track index */
    std::vector<double>* m_eclpidEnergy; /**< PID track energy */
    std::vector<double>* m_eclpidEop; /**< PID track E/p */
    std::vector<double>* m_eclpidE9E25; /**< PID track ration of 3x3 over 5x5 crystal matrices energies */
    std::vector<int>* m_eclpidNCrystals; /**< PID track number of crystals */
    std::vector<int>* m_eclpidNClusters; /**< PID track number of clusters */
    std::vector<double>* m_eclLogLikeEl; /**< PID track electron likelyhood */
    std::vector<double>* m_eclLogLikeMu; /**< PID track muon likelyhood */
    std::vector<double>* m_eclLogLikePi; /**< PID track pion likelyhood */

    //dataStore objects
    StoreArray<ECLCluster> m_eclClusters;  /**< ECLCluster storeArray */
    StoreArray<MCParticle> m_mcParticles;  /**< MCParticle storeArray */
    StoreArray<Track> m_tracks;  /**< Tracks storeArray */
    StoreArray<TrackFitResult> m_trackFitResults;  /**< TrackFitResult storeArray */
  };
}
