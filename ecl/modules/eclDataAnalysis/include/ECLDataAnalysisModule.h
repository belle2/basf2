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

#ifndef ECLDATAANALYSISMODULE_H_
#define ECLDATAANALYSISMODULE_H_

#include <framework/core/Module.h>
#include <string>
#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  /** The ECL Data Analysis Module
   *
   * this module dump an ntuple containing ECL-related infos starting from mdst
   *
   */

  class ECLDataAnalysisModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ECLDataAnalysisModule();

    /**
     * Destructor of the module.
     */
    virtual ~ECLDataAnalysisModule();

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

    /** members of ECLReconstructor Module
     */

    TFile* m_rootFilePtr; /**< pointer at root file used for storing info */
    std::string m_rootFileName; /**< name of the root file */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */
    bool m_doTracking; /**< if true, info on tracking will be stored, job will fail if doTracking==1 and
        the tracking modules are not enabled at phyton level */

    /** Root tree and file for saving the output */
    TTree* m_tree; /** Output tree */
    //TFile* m_rootFile;

    // variables
    int m_iExperiment; /** Experiment number */
    int m_iRun; /** Run number */
    int m_iEvent; /** Event number */

    /*int m_eclTriggerMultip;
    std::vector<int>* m_eclTriggerIdx;
    std::vector<int>* m_eclTriggerCellId;
    std::vector<double>* m_eclTriggerTime;*/

    int m_eclDigitMultip; /** Number of ECLDigits per event */
    std::vector<int>* m_eclDigitIdx; /** ECLDigit index */
    std::vector<int>* m_eclDigitToMc; /** Index of MCParticle related to that ECLDigit */
    std::vector<int>* m_eclDigitCellId; /** Number of ECLDigit CellId */
    std::vector<int>* m_eclDigitAmp;  /** ECLDigit amplitude */
    std::vector<int>* m_eclDigitTimeFit;  /** ECLDigit timing */
    std::vector<int>* m_eclDigitFitQuality;  /** ECLDigit fit quality */

    int m_eclSimHitMultip;  /** Number of ECLSimHits per event */
    std::vector<int>* m_eclSimHitIdx;  /** Index of ECLSimHit*/
    std::vector<int>* m_eclSimHitToMc; /** Index of MCParticle related to that ECLSimHit */
    std::vector<int>* m_eclSimHitCellId; /** ECLSimHit CellId */
    std::vector<int>* m_eclSimHitPdg; /** PDG code of MCParticle associted to that ECLDigit */
    std::vector<double>* m_eclSimHitEnergyDep; /** Energy deposition of ECLSimHit */
    std::vector<double>* m_eclSimHitFlightTime; /** ??? */
    std::vector<double>* m_eclSimHitX; /** ECLDigit X position */
    std::vector<double>* m_eclSimHitY; /** ECLDigit Y position */
    std::vector<double>* m_eclSimHitZ; /** ECLDigit Z position */
    std::vector<double>* m_eclSimHitPx; /** ECLDigit PX */
    std::vector<double>* m_eclSimHitPy; /** ECLDigit PY */
    std::vector<double>* m_eclSimHitPz; /** ECLDigit PZ */

    int m_eclHitMultip; /** Number of ECLHits per event */
    std::vector<int>* m_eclHitIdx; /** Index of ECLHits */
    std::vector<int>* m_eclHitToMc; /** Index of MCParticle related to ECLHit */
    std::vector<int>* m_eclHitCellId; /** ECLHit CellID */
    std::vector<double>* m_eclHitEnergyDep; /** ECLHit energy */
    std::vector<double>* m_eclHitTimeAve; /** ECLHit time */

    //int m_eclShowerMultip; /** Number of ECLShowers per event */
    //std::vector<int>* m_eclShowerIdx; /** ECLShower index */
    //std::vector<int>* m_eclShowerToMc; /** Index of MCParticle related to ECLShower */
    //std::vector<int>* m_eclShowerToGamma; /** Index of ECLGamma related to ECLShower */
    //std::vector<double>* m_eclShowerEnergy; /** ECLShower energy */
    //std::vector<double>* m_eclShowerTheta; /** Polar direction of ECLShower */
    //std::vector<double>* m_eclShowerPhi; /** Azimuthal direction of ECLShower */
    //std::vector<double>* m_eclShowerR; /** Distance from IP of ECLShower */
    //std::vector<int>* m_eclShowerNHits; /** Number of ECLHits related to ECLShower */
    //std::vector<double>* m_eclShowerE9oE25; /** Ratio of 3x3 over 5x5 crystal matrices energies for ECLShower*/
    //std::vector<double>* m_eclShowerUncEnergy; /** Uncorrected ECLShower energy */

    int m_eclClusterMultip;  /** Number of ECLClusterss per event */
    std::vector<int>* m_eclClusterIdx;  /** ECLCluster index */
    std::vector<int>* m_eclClusterToMc1; /** Index of first MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight1; /** Energy contribution of first MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMc2; /** Index of second MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight2; /** Energy contribution of second MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMc3; /** Index of third MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight3; /** Energy contribution of third MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMc4; /** Index of fourth MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight4; /** Energy contribution of fourth MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMc5; /** Index of fifth MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMcWeight5; /** Energy contribution of fifth MCParticle related to ECLCluster */
    std::vector<double>*
    m_eclClusterToBkgWeight; /** Remaining energy contribution not associated to first five MCParticles related to ECLCluster */
    std::vector<double>* m_eclClusterSimHitSum; /** Energy contribution of first MCParticle related to ECLCluster */
    //std::vector<int>* m_eclClusterToShower; /** Index of ECLShower related to ECLCluster */
    //std::vector<int>* m_eclClusterToTrack;
    std::vector<double>* m_eclClusterEnergy; /** ECLCluster energy */
    std::vector<double>* m_eclClusterEnergyError; /** ECLCluster energy error*/
    std::vector<double>* m_eclClusterTheta;  /** ECLCluster polar direction */
    std::vector<double>* m_eclClusterThetaError;  /** ECLCluster error on polar direction */
    std::vector<double>* m_eclClusterPhi;  /** ECLCluster azimuthal direction */
    std::vector<double>* m_eclClusterPhiError;  /** ECLCluster error on azimuthal direction */
    std::vector<double>* m_eclClusterR;  /** ECLCluster distance from IP */
    std::vector<double>* m_eclClusterEnergyDepSum;  /** ECLCluster simulated energy */
    std::vector<double>* m_eclClusterTiming;  /** ECLCluster time */
    std::vector<double>* m_eclClusterTimingError;  /** ECLCluster time error */
    std::vector<double>* m_eclClusterE9oE25;  /** Ratio of 3x3 over 5x5 crystal matrices energies for ECLCluster*/
    std::vector<double>* m_eclClusterHighestE; /** Highest energy deposit (per crystal) in ECLCluster */
    std::vector<double>* m_eclClusterLat; /** ECLCluster shape parameter */
    std::vector<int>* m_eclClusterNofCrystals;  /** Number of crystals in ECLCluster */
    std::vector<int>* m_eclClusterCrystalHealth;  /** Crystal healt flag */
    std::vector<double>* m_eclClusterMergedPi0;  /** Flag for merged pi0 */
    std::vector<double>* m_eclClusterPx;  /** Reconstructed momentum along X */
    std::vector<double>* m_eclClusterPy;  /** Reconstructed momentum along Y */
    std::vector<double>* m_eclClusterPz;  /** Reconstructed momentum along Z */
    std::vector<bool>* m_eclClusterIsTrack; /** Flag for charged clusters */
    std::vector<double>* m_eclClusterPi0Likel; /** Flag for pi0 */
    std::vector<double>* m_eclClusterEtaLikel; /** Flag for eta */
    std::vector<double>* m_eclClusterDeltaL; /** ??? */
    std::vector<double>* m_eclClusterBeta; /** ??? */
    /*
    int m_eclGammaMultip;
    std::vector<int>* m_eclGammaIdx;
    std::vector<double>* m_eclGammaEnergy;
    std::vector<double>* m_eclGammaTheta;
    std::vector<double>* m_eclGammaPhi;
    std::vector<double>* m_eclGammaPx;
    std::vector<double>* m_eclGammaPy;
    std::vector<double>* m_eclGammaPz;
    std::vector<double>* m_eclGammaR;
    std::vector<int>* m_eclGammaToPi0;

    int m_eclPi0Multip;
    std::vector<int>* m_eclPi0Idx;
    //    std::vector<int>* m_eclPi0ToGamma;
    std::vector<int>* m_eclPi0ShowerId1;
    std::vector<int>* m_eclPi0ShowerId2;
    std::vector<double>* m_eclPi0Energy;
    std::vector<double>* m_eclPi0Px;
    std::vector<double>* m_eclPi0Py;
    std::vector<double>* m_eclPi0Pz;
    std::vector<double>* m_eclPi0Mass;
    std::vector<double>* m_eclPi0MassFit;
    std::vector<double>* m_eclPi0Chi2;
    std::vector<double>* m_eclPi0PValue;
    */
    int m_mcMultip; /** Multiplicity of MCParticles */
    std::vector<int>* m_mcIdx; /** MCParticle index */
    std::vector<int>* m_mcPdg; /** MCParticle PDG code */
    std::vector<int>* m_mcMothPdg; /** MCParticle mother particle PDG code */
    std::vector<int>* m_mcGMothPdg; /** MCParticle grandmother particle PDG code */
    std::vector<int>* m_mcGGMothPdg; /** MCParticle greand-grandmother particle PDG code */
    std::vector<double>* m_mcEnergy; /** MCParticle energyx */
    std::vector<double>* m_mcPx; /** MCParticle momentum X direction */
    std::vector<double>* m_mcPy; /** MCParticle momentum Y direction */
    std::vector<double>* m_mcPz; /** MCParticle momentum Z direction */
    std::vector<double>* m_mcDecayVtxX; /** MCParticle decay vertex X */
    std::vector<double>* m_mcDecayVtxY; /** MCParticle decay vertex Y */
    std::vector<double>* m_mcDecayVtxZ; /** MCParticle decay vertex Z */
    std::vector<double>* m_mcProdVtxX; /** MCParticle production vertex X */
    std::vector<double>* m_mcProdVtxY; /** MCParticle production vertex Y */
    std::vector<double>* m_mcProdVtxZ; /** MCParticle production vertex Z */
    std::vector<int>* m_mcSecondaryPhysProc; /** Flag for secondary physics process */

    int m_trkMultip; /** Track Multiplicity */
    std::vector<int>* m_trkPdg; /** Track PDG code */
    std::vector<int>* m_trkIdx; /** Track index */
    std::vector<int>* m_trkCharge; /** Track charge */
    std::vector<double>* m_trkPx; /** Track momentum along X direction */
    std::vector<double>* m_trkPy; /** Track momentum along Y direction */
    std::vector<double>* m_trkPz; /** Track momentum along Z direction */
    std::vector<double>* m_trkP; /** Track momentum */
    std::vector<double>* m_trkTheta; /** Track polar direction */
    std::vector<double>* m_trkPhi; /** Track azimuthal direction */
    std::vector<double>* m_trkX; /** Track DOCA X (?) */
    std::vector<double>* m_trkY; /** Track DOCA Y (?) */
    std::vector<double>* m_trkZ; /** Track DOCA Z (?) */

    std::vector<int>* m_eclpidtrkIdx; /** PID track index */
    std::vector<double>* m_eclpidEnergy; /** PID track energy */
    std::vector<double>* m_eclpidEop; /** PID track E/p */
    std::vector<double>* m_eclpidE9E25; /** PID track ration of 3x3 over 5x5 crystal matrices energies */
    std::vector<int>* m_eclpidNCrystals; /** PID track number of crystals */
    std::vector<int>* m_eclpidNClusters; /** PID track number of clusters */
    std::vector<double>* m_eclLogLikeEl; /** PID track electron likelyhood */
    std::vector<double>* m_eclLogLikeMu; /** PID track muon likelyhood */
    std::vector<double>* m_eclLogLikePi; /** PID track pion likelyhood */
  };
}

#endif
