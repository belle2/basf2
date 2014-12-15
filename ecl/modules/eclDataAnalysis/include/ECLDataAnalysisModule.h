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

    TFile* m_rootFilePtr; /**< pointer at root file used for storing info */
    std::string m_rootFileName; /**< name of the root file */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */
    bool m_doTracking; /**< if true, info on tracking will be stored, job will fail if doTracking==1 and
        the tracking modules are not enabled at phyton level */

    /** Root tree and file for saving the output */
    TTree* m_tree;
    TFile* m_rootFile;

    // variables
    int m_iExperiment;
    int m_iRun;
    int m_iEvent;

    /*int m_eclTriggerMultip;
    std::vector<int>* m_eclTriggerIdx;
    std::vector<int>* m_eclTriggerCellId;
    std::vector<double>* m_eclTriggerTime;*/

    int m_eclDigitMultip;
    std::vector<int>* m_eclDigitIdx;
    std::vector<int>* m_eclDigitToMc;
    std::vector<int>* m_eclDigitCellId;
    std::vector<int>* m_eclDigitAmp;
    std::vector<int>* m_eclDigitTimeFit;
    std::vector<int>* m_eclDigitFitQuality;

    int m_eclSimHitMultip;
    std::vector<int>* m_eclSimHitIdx;
    std::vector<int>* m_eclSimHitToMc;
    std::vector<int>* m_eclSimHitCellId;
    std::vector<int>* m_eclSimHitPdg;
    std::vector<double>* m_eclSimHitEnergyDep;
    std::vector<double>* m_eclSimHitFlightTime;
    std::vector<double>* m_eclSimHitX;
    std::vector<double>* m_eclSimHitY;
    std::vector<double>* m_eclSimHitZ;
    std::vector<double>* m_eclSimHitPx;
    std::vector<double>* m_eclSimHitPy;
    std::vector<double>* m_eclSimHitPz;

    int m_eclHitMultip;
    std::vector<int>* m_eclHitIdx;
    std::vector<int>* m_eclHitToMc;
    std::vector<int>* m_eclHitCellId;
    std::vector<double>* m_eclHitEnergyDep;
    std::vector<double>* m_eclHitTimeAve;

    int m_eclShowerMultip;
    std::vector<int>* m_eclShowerIdx;
    std::vector<int>* m_eclShowerToMc;
    std::vector<int>* m_eclShowerToGamma;
    std::vector<double>* m_eclShowerEnergy;
    std::vector<double>* m_eclShowerTheta;
    std::vector<double>* m_eclShowerPhi;
    std::vector<double>* m_eclShowerR;
    std::vector<int>* m_eclShowerNHits;
    std::vector<double>* m_eclShowerE9oE25;
    std::vector<double>* m_eclShowerUncEnergy;

    int m_eclClusterMultip;
    std::vector<int>* m_eclClusterIdx;
    std::vector<int>* m_eclClusterToMc;
    std::vector<int>* m_eclClusterToShower;
    std::vector<int>* m_eclClusterToTrack;
    std::vector<double>* m_eclClusterEnergy;
    std::vector<double>* m_eclClusterEnergyError;
    std::vector<double>* m_eclClusterTheta;
    std::vector<double>* m_eclClusterThetaError;
    std::vector<double>* m_eclClusterPhi;
    std::vector<double>* m_eclClusterPhiError;
    std::vector<double>* m_eclClusterR;
    std::vector<double>* m_eclClusterEnergyDepSum;
    std::vector<double>* m_eclClusterTiming;
    std::vector<double>* m_eclClusterTimingError;
    std::vector<double>* m_eclClusterE9oE25;
    std::vector<double>* m_eclClusterHighestE;
    std::vector<double>* m_eclClusterLat;
    std::vector<int>* m_eclClusterNofCrystals;
    std::vector<int>* m_eclClusterCrystalHealth;
    std::vector<double>* m_eclClusterMergedPi0;
    std::vector<double>* m_eclClusterPx;
    std::vector<double>* m_eclClusterPy;
    std::vector<double>* m_eclClusterPz;
    std::vector<bool>* m_eclClusterIsTrack;
    std::vector<double>* m_eclClusterPi0Likel;
    std::vector<double>* m_eclClusterEtaLikel;
    std::vector<double>* m_eclClusterDeltaL;
    std::vector<double>* m_eclClusterBeta;

    int m_eclGammaMultip;
    std::vector<int>* m_eclGammaIdx;
    std::vector<double>* m_eclGammaEnergy;
    std::vector<double>* m_eclGammaPx;
    std::vector<double>* m_eclGammaPy;
    std::vector<double>* m_eclGammaPz;
    std::vector<double>* m_eclGammaTheta;
    std::vector<double>* m_eclGammaPhi;
    std::vector<double>* m_eclGammaR;

    int m_eclPi0Multip;
    std::vector<int>* m_eclPi0Idx;
    std::vector<int>* m_eclPi0ToGamma;
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

    int m_mcMultip;
    std::vector<int>* m_mcIdx;
    std::vector<int>* m_mcPdg;
    std::vector<int>* m_mcMothPdg;
    std::vector<int>* m_mcGMothPdg;
    std::vector<int>* m_mcGGMothPdg;
    std::vector<double>* m_mcEnergy;
    std::vector<double>* m_mcPx;
    std::vector<double>* m_mcPy;
    std::vector<double>* m_mcPz;
    std::vector<double>* m_mcDecayVtxX;
    std::vector<double>* m_mcDecayVtxY;
    std::vector<double>* m_mcDecayVtxZ;
    std::vector<double>* m_mcProdVtxX;
    std::vector<double>* m_mcProdVtxY;
    std::vector<double>* m_mcProdVtxZ;
    std::vector<int>* m_mcSecondaryPhysProc;

    int m_trkMultip;
    std::vector<int>* m_trkPdg;
    std::vector<int>* m_trkIdx;
    std::vector<double>* m_trkPx;
    std::vector<double>* m_trkPy;
    std::vector<double>* m_trkPz;
    std::vector<double>* m_trkX;
    std::vector<double>* m_trkY;
    std::vector<double>* m_trkZ;

  };

}

#endif
