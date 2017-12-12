/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Elisa Manoni, Benjamin Oberhof                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <string>

// ECL
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>
#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <ecl/dataobjects/ECLEventInformation.h>
#include <mdst/dataobjects/MCParticle.h>

// FRAMEWORK
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// GEOMETRY
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/geometry/ECLGeometryPar.h>

class TFile;
class TTree;

namespace Belle2 {
  class Track;
  class TrackFitResult;
  class ECLPidLikelihood;





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

    /** members of ECLReconstructor Module */
    TFile* m_rootFilePtr; /**< root file used for storing info */
    std::string m_rootFileName; /**< name of the root file */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */
    bool m_doTracking; /**< if true, info on tracking will be stored, job will fail if doTracking==1 and the tracking modules are not enabled at phyton level */
    bool m_doPureCsIStudy; /**< if true, info on pureCsI upgrade is stored*/
    bool m_doSimulation; /**< if true, info on Hits and SimHits is stored*/

    //dataStore objects
    StoreArray<Track> m_tracks;  /**< Tracks storeArray */
    StoreArray<TrackFitResult> m_trackFitResults;  /**< TrackFitResult storeArray */
    StoreArray<ECLPidLikelihood> m_eclPidLikelihoods; /**< ECLPidLikelihood storeArray */

    /** Store array: ECLSimHit. */
    StoreArray<ECLSimHit> m_eclSimHits;
    /** Store array: ECLHit. */
    StoreArray<ECLHit> m_eclHits;
    /** Store array: ECLDigit. */
    StoreArray<ECLDigit> m_eclDigits;
    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;
    /** Store array: ECLConnectedRegion. */
    StoreArray<ECLConnectedRegion> m_eclConnectedRegions;
    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;
    /** Store array: ECLCluster. */
    StoreArray<ECLCluster> m_eclClusters;
    /** Store array: ECLLocalMaximum. */
    StoreArray<ECLLocalMaximum> m_eclLocalMaximums;
    /** Store object pointer: ECLEventInformation. */
    StoreObjPtr<ECLEventInformation> m_eclEventInformation;

    /** Default name ECLCalDigits array*/
    virtual const char* eclSimHitArrayName() const
    { return "ECLSimHits" ; }
    /** Default name ECLCalDigits array*/
    virtual const char* eclHitArrayName() const
    { return "ECLHits" ; }
    /** Default name ECLDigits array*/
    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }
    /** Default name ECLCalDigits array*/
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }
    /** Default name ECLShower array*/
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }
    /** Default name ECLClusters array*/
    virtual const char* eclClusterArrayName() const
    { return "ECLClusters" ; }
    /** Default name ECLConnectedRegions array*/
    virtual const char* eclConnectedRegionArrayName() const
    { return "ECLConnectedRegions" ; }
    /** Default name ECLLocalMaxima array*/
    virtual const char* eclLocalMaximumArrayName() const
    { return "ECLLocalMaximums" ; }


    /** Store array: ECLPureDigit. */
    StoreArray<ECLDigit> m_eclPureDigits;
    /** Store array: ECLPureCalDigit. */
    StoreArray<ECLCalDigit> m_eclPureCalDigits;
    /** Store array: ECLPureConnectedRegion. */
    StoreArray<ECLConnectedRegion> m_eclPureConnectedRegions;
    /** Store array: ECLPureShower. */
    StoreArray<ECLShower> m_eclPureShowers;
    /** Store array: ECLPureCluster. */
    StoreArray<ECLCluster> m_eclPureClusters;
    /** Store array: ECLPureLocalMaximum. */
    StoreArray<ECLLocalMaximum> m_eclPureLocalMaximums;

    /** Default name ECLPureDigits array*/
    virtual const char* eclPureDigitArrayName() const
    { return "ECLDigitsPureCsI" ; }
    /** Default name ECLPureCalDigits array*/
    virtual const char* eclPureCalDigitArrayName() const
    { return "ECLCalDigitsPureCsI" ; }
    /** Default name ECLPureShower array*/
    virtual const char* eclPureShowerArrayName() const
    { return "ECLShowersPureCsI" ; }
    /** Default name ECLPureClusters array*/
    virtual const char* eclPureClusterArrayName() const
    { return "ECLClustersPureCsI" ; }
    /** Default name ECLPureConnectedRegions array*/
    virtual const char* eclPureConnectedRegionArrayName() const
    { return "ECLConnectedRegionsPureCsI" ; }
    /** Default name ECLPureLocalMaxima array */
    virtual const char* eclPureLocalMaximumArrayName() const
    { return "ECLLocalMaximumsPureCsI" ; }

    StoreArray<MCParticle> m_mcParticles; /**< MCParticles StoreArray*/

    TTree* m_tree; /**< Root tree and file for saving the output */

    // variables
    int m_iExperiment; /**< Experiment number */
    int m_iRun; /**< Run number */
    int m_iEvent; /**< Event number */

    int m_eclDigitMultip; /**< Number of ECLDigits per event */
    std::vector<int>* m_eclDigitIdx; /**< ECLDigit index */
    std::vector<int>* m_eclDigitToMC; /**< Index of MCParticle related to that ECLDigit */
    std::vector<int>* m_eclDigitCellId; /**< Number of ECLDigit CellId */
    std::vector<int>* m_eclDigitAmp;  /**< ECLDigit amplitude */
    std::vector<int>* m_eclDigitTimeFit;  /**< ECLDigit timing */
    std::vector<int>* m_eclDigitFitQuality;  /**< ECLDigit fit quality */
    std::vector<int>* m_eclDigitToCalDigit; /**< Index of CalDigit related to that ECLDigit */

    int m_eclCalDigitMultip; /**< Number of ECLCalDigits per event */
    std::vector<int>* m_eclCalDigitIdx; /**< ECLCalDigit index */
    std::vector<int>* m_eclCalDigitToMC1; /**< Index of first MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC1PDG; /**< PDG code of first MCParticle related to ECLCalDigit */
    std::vector<double>* m_eclCalDigitToMCWeight1; /**< Energy contribution of first MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC2; /**< Index of second MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC2PDG; /**< PDG code of second MCParticle related to ECLCalDigit */
    std::vector<double>* m_eclCalDigitToMCWeight2; /**< Energy contribution of second MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC3; /**< Index of third MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC3PDG; /**< PDG code of third MCParticle related to ECLCalDigit */
    std::vector<double>* m_eclCalDigitToMCWeight3; /**< Energy contribution of third MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC4; /**< Index of fourth MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC4PDG; /**< PDG code of fourth MCParticle related to ECLCalDigit */
    std::vector<double>* m_eclCalDigitToMCWeight4; /**< Energy contribution of fourth MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC5; /**< Index of fifth MCParticle related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToMC5PDG; /**< PDG code of fifth MCParticle related to ECLCalDigit */
    std::vector<double>* m_eclCalDigitToMCWeight5; /**< Energy contribution of fifth MCParticle related to ECLCalDigit */
    std::vector<double>*
    m_eclCalDigitToBkgWeight; /**< Remaining energy contribution not associated to first five MCParticles related to ECLCalDigit */
    std::vector<double>* m_eclCalDigitSimHitSum; /**< Full energy contribution related to ECLCalDigit */
    std::vector<int>* m_eclCalDigitToShower; /**< Index of ECLShower related to that ECLCalDigit */
    std::vector<int>* m_eclCalDigitCellId; /**< Number of ECLCalDigit CellId */
    std::vector<double>* m_eclCalDigitAmp;  /**< ECLCalDigit amplitude */
    std::vector<double>* m_eclCalDigitTimeFit;  /**< ECLCalDigit timing */
    std::vector<int>* m_eclCalDigitFitQuality;  /**< ECLCalDigit fit quality */
    std::vector<int>* m_eclCalDigitToCR; /**< Index of CR related to that ECLCalDigit */
    std::vector<int>* m_eclCalDigitToLM; /**< Index of LM related to that ECLCalDigit */

    std::vector<int>*    m_eclCRIdx; /**< Connected Region ID */
    std::vector<int>*    m_eclCRIsTrack;  /**< Int for Connected Region - Track Match*/
    std::vector<double>* m_eclCRLikelihoodMIPNGamma;  /**< Connected Region MIP Likelihood */
    std::vector<double>*    m_eclCRLikelihoodChargedHadron;  /**< Connected Region Charged Hadron Likelihood */
    std::vector<double>*    m_eclCRLikelihoodElectronNGamma;  /**< Connected Region Electron Likelihood */
    std::vector<double>*    m_eclCRLikelihoodNGamma;  /**< Connected Region Gamma Likelihood */
    std::vector<double>*    m_eclCRLikelihoodNeutralHadron;  /**< Connected Region Neutral Hadron Likelihood*/
    std::vector<double>*    m_eclCRLikelihoodMergedPi0;  /**< Connected Region Merged Pi0 Likelihood */

    int m_eclLMMultip;  /**< Local Maxima multiplicity */
    std::vector<int>* m_eclLMId; /**< Local Maximum ID */
    std::vector<int>* m_eclLMType; /**< Local Maximum type */
    std::vector<int>* m_eclLMCellId; /**< Local Maximum Cell ID */

    int m_eclSimHitMultip;  /**< Number of ECLSimHits per event */
    std::vector<int>* m_eclSimHitIdx;  /**< Index of ECLSimHit*/
    std::vector<int>* m_eclSimHitToMC; /**< Index of MCParticle related to that ECLSimHit */
    std::vector<int>* m_eclSimHitCellId; /**< ECLSimHit CellId */
    std::vector<int>* m_eclSimHitPdg; /**< PDG code of MCParticle associted to that ECLDigit */
    std::vector<double>* m_eclSimHitEnergyDep; /**< Energy deposition of ECLSimHit */
    std::vector<double>* m_eclSimHitFlightTime; /**< ECLSimhit Flight Time */
    std::vector<double>* m_eclSimHitX; /**< ECLSimHit X position */
    std::vector<double>* m_eclSimHitY; /**< ECLSimHit Y position */
    std::vector<double>* m_eclSimHitZ; /**< ECLSimHit Z position */
    std::vector<double>* m_eclSimHitPx; /**< ECLSimHit PX */
    std::vector<double>* m_eclSimHitPy; /**< ECLSimHit PY */
    std::vector<double>* m_eclSimHitPz; /**< ECLSimHit PZ */

    int m_eclHitMultip; /**< Number of ECLHits per event */
    std::vector<int>* m_eclHitIdx; /**< Index of ECLHits */
    std::vector<int>* m_eclHitToMC; /**< Index of MCParticle related to ECLHit */
    std::vector<int>* m_eclHitToDigit; /**< Index of ECLDigit related to ECLHit */
    std::vector<int>* m_eclHitToDigitAmp; /**< Amplitude of ECLDigit related to ECLHit */
    std::vector<int>* m_eclHitToPureDigit; /**< Index of ECLDigit related to ECLHit, PureCsI option */
    std::vector<int>* m_eclHitToPureDigitAmp; /**< Amplitude of ECLDigit related to ECLHit, PureCsI option */
    std::vector<int>* m_eclHitCellId; /**< ECLHit CellID */
    std::vector<double>* m_eclHitEnergyDep; /**< ECLHit energy */
    std::vector<double>* m_eclHitTimeAve; /**< ECLHit time */

    int m_eclClusterMultip;  /**< Number of ECLClusters per event */
    int m_eclClusterTrueMultip;  /**< Number of ECLClusters per event */
    int m_eclClusterGammaMultip;  /**< Number of ECLClusters per event */
    std::vector<int>* m_eclClusterIdx;  /**< ECLCluster index */
    std::vector<int>* m_eclClusterToMC1; /**< Index of first MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMCWeight1; /**< Energy contribution of first MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC1PDG; /**< PDG code of first MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC2; /**< Index of second MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMCWeight2; /**< Energy contribution of second MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC2PDG; /**< PDG code of second MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC3; /**< Index of third MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMCWeight3; /**< Energy contribution of third MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC3PDG; /**< PDG code of third MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC4; /**< Index of fourth MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMCWeight4; /**< Energy contribution of fourth MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC4PDG; /**< PDG code of fourth MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC5; /**< Index of fifth MCParticle related to ECLCluster */
    std::vector<double>* m_eclClusterToMCWeight5; /**< Energy contribution of 5th MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToMC5PDG; /**< PDG code of fifth MCParticle related to ECLCluster */
    std::vector<double>*
    m_eclClusterToBkgWeight; /**< Remaining energy contribution not associated to first five MCParticles related to ECLCluster */
    std::vector<double>* m_eclClusterSimHitSum; /**< Energy contribution of 1st MCParticle related to ECLCluster */
    std::vector<int>* m_eclClusterToShower; /**< Index of ECLShower related to ECLCluster */
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
    std::vector<double>* m_eclClusterE9oE21;  /**< Ratio of 3x3 over 5x5 crystal matrices energies for ECLCluster*/
    std::vector<double>* m_eclClusterHighestE; /**< Highest energy deposit (per crystal) in ECLCluster */
    std::vector<int>* m_eclClusterNofCrystals;  /**< Number of crystals in ECLCluster */
    std::vector<int>* m_eclClusterCrystalHealth;  /**< Crystal healt flag */
    std::vector<bool>* m_eclClusterIsTrack; /**< Flag for charged clusters */
    std::vector<double>* m_eclClusterClosestTrackDist; /**< Flag for charged clusters */
    std::vector<double>* m_eclClusterDeltaL; /**< Reconstructed Cluster DeltaL */
    std::vector<double>* m_eclClusterAbsZernike40; /**< Reconstructed Abs Zernike40 */
    std::vector<double>* m_eclClusterAbsZernike51; /**< Reconstructed Abs Zernike51 */
    std::vector<double>* m_eclClusterZernikeMVA; /**< Zernike MVA */
    std::vector<double>* m_eclClusterE1oE9; /**< Reconstructed E1 over E9 */
    std::vector<double>* m_eclClusterSecondMoment; /**< Reconstructed Second Moment */
    std::vector<double>* m_eclClusterLAT; /**< Reconstructed LAT */
    std::vector<double>* m_eclClusterDeltaTime99; /**< DeltaTime99 */
    std::vector<int>* m_eclClusterDetectorRegion; /**< Cluster Detector Region */
    std::vector<int>* m_eclClusterHypothesisId; /**< Cluster Detector Region */

    int m_eclPureDigitMultip; /**< Number of ECLDigits per event, PureCsI option */
    std::vector<int>* m_eclPureDigitIdx; /**< ECLDigit index, PureCsI option */
    std::vector<int>* m_eclPureDigitToMC; /**< Index of MCParticle related to that ECLDigit, PureCsI option */
    std::vector<int>* m_eclPureDigitCellId; /**< Number of ECLDigit CellId, PureCsI option */
    std::vector<int>* m_eclPureDigitAmp;  /**< ECLDigit amplitude, PureCsI option */
    std::vector<int>* m_eclPureDigitTimeFit;  /**< ECLDigit timing, PureCsI option */
    std::vector<int>* m_eclPureDigitFitQuality;  /**< ECLDigit fit quality, PureCsI option */
    std::vector<int>* m_eclPureDigitToCluster;  /**< ECLDigit To Cluster, PureCsI option */

    int m_eclPureCalDigitMultip; /**< Number of ECLCalDigits per event, PureCsI option */
    std::vector<int>* m_eclPureCalDigitIdx; /**< ECLCalDigit index, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC1; /**< Index of first MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC1PDG; /**< PDG code of first MCParticle related to CalDigit, PureCsI option */
    std::vector<double>*
    m_eclPureCalDigitToMCWeight1; /**< Energy contribution of first MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC2; /**< Index of second MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC2PDG; /**< PDG code of second MCParticle related to CalDigit, PureCsI option */
    std::vector<double>*
    m_eclPureCalDigitToMCWeight2; /**< Energy contribution of second MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC3; /**< Index of third MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC3PDG; /**< PDG code of third MCParticle related to CalDigit, PureCsI option */
    std::vector<double>*
    m_eclPureCalDigitToMCWeight3; /**< Energy contribution of third MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC4; /**< Index of fourth MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC4PDG; /**< PDG code of fourth MCParticle related to CalDigit, PureCsI option */
    std::vector<double>*
    m_eclPureCalDigitToMCWeight4; /**< Energy contribution of fourth MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC5; /**< Index of fifth MCParticle related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToMC5PDG; /**< PDG code of fifth MCParticle related to CalDigit, PureCsI option */
    std::vector<double>*
    m_eclPureCalDigitToMCWeight5; /**< Energy contribution of fifth MCParticle related to CalDigit, PureCsI option */
    std::vector<double>*
    m_eclPureCalDigitToBkgWeight; /**< Remaining energy contribution not associated to first five MCParticles related to CalDigit, PureCsI option */
    std::vector<double>* m_eclPureCalDigitSimHitSum; /**< Full energy contribution related to CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToShower; /**< Index of ECLShower related to that CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitCellId; /**< Number of CalDigit CellId, PureCsI option */
    std::vector<double>* m_eclPureCalDigitAmp;  /**< CalDigit amplitude, PureCsI option */
    std::vector<double>* m_eclPureCalDigitTimeFit;  /**< CalDigit timing, PureCsI option */
    std::vector<int>* m_eclPureCalDigitFitQuality;  /**< CalDigit fit quality, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToCR; /**< Index of CR related to that CalDigit, PureCsI option */
    std::vector<int>* m_eclPureCalDigitToLM; /**< Index of LM related to that CalDigit, PureCsI option */

    std::vector<int>*    m_eclPureCRIdx; /**< Connected Region ID, PureCsI option */
    std::vector<int>*    m_eclPureCRIsTrack;  /**< Int for Connected Region - Track Match, PureCsI option */
    std::vector<double>* m_eclPureCRLikelihoodMIPNGamma;  /**< Connected Region MIP Likelihood, PureCsI option */
    std::vector<double>*    m_eclPureCRLikelihoodChargedHadron;  /**< Connected Region Charged Hadron Likelihood, PureCsI option */
    std::vector<double>*    m_eclPureCRLikelihoodElectronNGamma;  /**< Connected Region Electron Likelihood, PureCsI option */
    std::vector<double>*    m_eclPureCRLikelihoodNGamma;  /**< Connected Region Gamma Likelihood, PureCsI option */
    std::vector<double>*    m_eclPureCRLikelihoodNeutralHadron;  /**< Connected Region Neutral Hadron Likelihood, PureCsI option */
    std::vector<double>*    m_eclPureCRLikelihoodMergedPi0;  /**< Connected Region Merged Pi0 Likelihood, PureCsI option */

    int m_eclPureLMMultip;  /**< Local Maxima multiplicity, PureCsI option */
    std::vector<int>* m_eclPureLMId; /**< Local Maximum ID, PureCsI option */
    std::vector<int>* m_eclPureLMType; /**< Local Maximum type, PureCsI option */
    std::vector<int>* m_eclPureLMCellId; /**< Local Maximum Cell ID, PureCsI option */

    int m_eclPureClusterMultip;  /**< Number of ECLClusterss per event, PureCsI option */
    std::vector<int>* m_eclPureClusterIdx;  /**< ECLCluster index, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC1; /**< Index of first MCParticle related to ECLCluster, PureCsI option */
    std::vector<double>*
    m_eclPureClusterToMCWeight1; /**< Energy contribution of first MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC1PDG; /**< PDG code of first MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC2; /**< Index of second MCParticle related to ECLCluster, PureCsI option */
    std::vector<double>*
    m_eclPureClusterToMCWeight2; /**< Energy contribution of second MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC2PDG; /**< PDG code of second MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC3; /**< Index of third MCParticle related to ECLCluster, PureCsI option */
    std::vector<double>*
    m_eclPureClusterToMCWeight3; /**< Energy contribution of third MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC3PDG; /**< PDG code of third MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC4; /**< Index of fourth MCParticle related to ECLCluster, PureCsI option */
    std::vector<double>*
    m_eclPureClusterToMCWeight4; /**< Energy contribution of fourth MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC4PDG; /**< PDG code of fourth MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC5; /**< Index of fifth MCParticle related to ECLCluster, PureCsI option */
    std::vector<double>*
    m_eclPureClusterToMCWeight5; /**< Energy contribution of 5th MCParticle related to ECLCluster, PureCsI option */
    std::vector<int>* m_eclPureClusterToMC5PDG; /**< PDG code of fifth MCParticle related to ECLCluster, PureCsI option */
    std::vector<double>*
    m_eclPureClusterToBkgWeight; /**< Remaining energy contribution not associated to first five MCParticles related to ECLCluster, PureCsI option */
    std::vector<double>* m_eclPureClusterEnergy; /**< Cluster energy, PureCsI option */
    std::vector<double>* m_eclPureClusterEnergyError; /**< Cluster energy error, PureCsI option */
    std::vector<double>* m_eclPureClusterTheta;  /**< Cluster polar direction, PureCsI option */
    std::vector<double>* m_eclPureClusterThetaError;  /**< Cluster error on polar direction, PureCsI option */
    std::vector<double>* m_eclPureClusterPhi;  /**< Cluster azimuthal direction, PureCsI option */
    std::vector<double>* m_eclPureClusterPhiError;  /**< Cluster error on azimuthal direction, PureCsI option */
    std::vector<double>* m_eclPureClusterR;  /**< Cluster distance from IP, PureCsI option */
    std::vector<double>* m_eclPureClusterEnergyDepSum;  /**< Cluster simulated energy, PureCsI option */
    std::vector<double>* m_eclPureClusterTiming;  /**< Cluster time, PureCsI option */
    std::vector<double>* m_eclPureClusterTimingError;  /**< Cluster time error, PureCsI option */
    std::vector<double>* m_eclPureClusterE9oE21;  /**< Ratio of 3x3 over 5x5 crystal matrices energies for Cluster, PureCsI option */
    std::vector<double>* m_eclPureClusterHighestE; /**< Highest energy deposit (per crystal) in Cluster, PureCsI option */
    std::vector<double>* m_eclPureClusterLat; /**< Cluster shape parameter LAT, PureCsI option */
    std::vector<int>* m_eclPureClusterNofCrystals;  /**< Number of crystals in Cluster, PureCsI option */
    std::vector<int>* m_eclPureClusterCrystalHealth;  /**< Crystal healt flag, PureCsI option */
    std::vector<bool>* m_eclPureClusterIsTrack; /**< Flag for charged clusters, PureCsI option */
    std::vector<double>* m_eclPureClusterDeltaL; /**< Reconstructed Cluster DeltaL, PureCsI option */
    std::vector<double>* m_eclPureClusterClosestTrackDist;  /**< Reconstructed Distance to Closest Track, PureCsI option */
    std::vector<double>* m_eclPureClusterAbsZernike40;  /**< Reconstructed Zernike40, PureCsI option */
    std::vector<double>* m_eclPureClusterAbsZernike51;  /**< Reconstructed Zernike51, PureCsI option */
    std::vector<double>* m_eclPureClusterZernikeMVA;  /**< Output of MVA classifier based on Zernike Momenta, PureCsI option */
    std::vector<double>* m_eclPureClusterSecondMoment;  /**< Reconstructed Cluster Second Moment, PureCsI option */
    std::vector<double>* m_eclPureClusterE1oE9;  /**< Reconstructed E1oE9, PureCsI option */
    std::vector<double>* m_eclPureClusterDeltaTime99;  /**< Reconstructed DeltaT99, PureCsI option */
    std::vector<int>* m_eclPureClusterDetectorRegion; /**< Clusters detector region, PureCsI option */
    std::vector<int>* m_eclPureClusterHypothesisId; /**< Cluster ID Hyp, PureCsI option */

    int m_eclShowerMultip; /**< Number of ECLShowers per event */
    std::vector<int>* m_eclShowerIdx; /**< Shower Index */
    std::vector<int>* m_eclShowerToMC1; /**< Index of first MCParticle related to ECLShower */
    std::vector<double>* m_eclShowerToMCWeight1; /**< Energy contribution of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC1PDG; /**< PDG code of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC1Moth; /**< Mother index of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC1MothPDG; /**< PDG code of parent of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC1GMoth; /**< GMother index of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC1GMothPDG; /**< PDG code of Gparent of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC2; /**< Index of second MCParticle related to ECLShower */
    std::vector<double>* m_eclShowerToMCWeight2; /**< Energy contribution of second MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC2PDG; /**< PDG code of second MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC2Moth; /**< Mother index of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC2MothPDG; /**< PDG code of parent of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC2GMoth; /**< GMother index of second MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC2GMothPDG; /**< PDG code of Gparent of second MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC3; /**< Index of third MCParticle related to ECLShower */
    std::vector<double>* m_eclShowerToMCWeight3; /**< Energy contribution of third MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC3PDG; /**< PDG code of third MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC3Moth; /**< Mother index of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC3MothPDG; /**< PDG code of parent of first MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC3GMoth; /**< GMother index of third MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC3GMothPDG; /**< PDG code of Gparent of third MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC4; /**< Index of fourth MCParticle related to ECLShower */
    std::vector<double>* m_eclShowerToMCWeight4; /**< Energy contribution of fourth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC4PDG; /**< PDG code of fourth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC4Moth; /**< Mother index of fourth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC4MothPDG; /**< PDG code of parent of fourth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC4GMoth; /**< GMother index of fourth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC4GMothPDG; /**< PDG code of Gparent of fourth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC5; /**< Index of fifth MCParticle related to ECLShower */
    std::vector<double>* m_eclShowerToMCWeight5; /**< Energy contribution of fifth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC5PDG; /**< PDG code of fifth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC5Moth; /**< Mother index of fifth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC5MothPDG; /**< PDG code of parent of fifth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC5GMoth; /**< GMother index of fifth MCParticle related to ECLShower */
    std::vector<int>* m_eclShowerToMC5GMothPDG; /**< PDG code of Gparent of fifth MCParticle related to ECLShower */
    std::vector<double>*
    m_eclShowerToBkgWeight; /**< Remaining energy contribution not associated to first five MCParticles related to ECLShower */
    std::vector<int>* m_eclShowerToLM1; /**< Index of first maximum related to ECLShower */
    std::vector<int>* m_eclShowerToLM2; /**< Index of 2nd maximum related to ECLShower */
    std::vector<int>* m_eclShowerToLM3; /**< Index of 3rd maximum related to ECLShower */
    std::vector<int>* m_eclShowerToLM4; /**< Index of 4th maximum related to ECLShower */
    std::vector<int>* m_eclShowerToLM5; /**< Index of 5th maximum related to ECLShower */
    std::vector<double>* m_eclShowerSimHitSum; /**< Full energy contribution related to ECLShower */
    std::vector<double>* m_eclShowerUncEnergy; /**< Shower bare energy */
    std::vector<double>* m_eclShowerEnergy; /**< Shower Energy */
    std::vector<double>* m_eclShowerTheta; /**< Shower Theta */
    std::vector<double>* m_eclShowerPhi; /**< Shower Phi */
    std::vector<double>* m_eclShowerR; /**< Shower R */
    std::vector<double>* m_eclShowerNHits; /**< Shower NHits */
    std::vector<double>* m_eclShowerE9oE21; /**< Shower E9oE21 */
    std::vector<double>* m_eclShowerTime;  /**< Shower Timing */
    std::vector<double>* m_eclShowerT99;  /**< Shower T99 */
    std::vector<int>* m_eclShowerConnectedRegionId; /**< Matched Connetcted Region Idx */
    std::vector<int>* m_eclShowerHypothesisId; /**< Shower Particle Hypothesis ID */
    std::vector<int>* m_eclShowerCentralCellId; /**< Cell ID for most energetic crystal */
    std::vector<double>* m_eclShowerEnergyError; /**< Shower Energy Error */
    std::vector<double>* m_eclShowerThetaError; /**< Shower Theta Error */
    std::vector<double>* m_eclShowerPhiError; /**< Shower Phi Error */
    std::vector<double>* m_eclShowerTimeResolution; /**< Shower Time Resolution */
    std::vector<double>* m_eclShowerHighestEnergy; /**< Shower Highest Energy Crystal Energy */
    std::vector<double>* m_eclShowerLateralEnergy; /**< Shower Lateral Energy */
    std::vector<double>* m_eclShowerMinTrkDistance; /**< Shower Min Dist to Track */
    std::vector<double>* m_eclShowerTrkDepth; /**< Shower Track Depth */
    std::vector<double>* m_eclShowerShowerDepth; /**< Shower Depth */
    std::vector<double>* m_eclShowerAbsZernike40; /**< Shower Zernike40 Moment */
    std::vector<double>* m_eclShowerAbsZernike51; /**< Shower Zernike51 Moment */
    std::vector<double>* m_eclShowerZernikeMVA; /**< Shower ZernikeMVA */
    std::vector<double>* m_eclShowerSecondMoment; /**< Shower Second Moment */
    std::vector<double>* m_eclShowerE1oE9; /**< Shower E1/E9 */
    std::vector<int>* m_eclShowerIsTrack; /**< Shower Track Match */
    std::vector<bool>* m_eclShowerIsCluster; /**< Shower Cluster Match */
    std::vector<int>*   m_eclShowerMCVtxInEcl; /**< Int, 1 if particle decays (interacts) in ECL, 0 otherwise*/
    std::vector<int>*   m_eclShowerMCFlightMatch; /**< Int, 1 if particle flight direction is "well" reconstructed in ECL, 0 otherwise*/
    std::vector<int>*
    m_eclShowerMCFFlightMatch; /**< Int, 1 if primary particle flight direction is "well" reconstructed in ECL, 0 otherwise, DEBUG PURPOSE*/
    std::vector<double>*   m_eclShowerHighestE1mE2; /**< Energy difference for 2 highest energy deposits in shower*/

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
    std::vector<double>* m_eclpidE9E21; /**< PID track ration of 3x3 over 5x5 crystal matrices energies */
    std::vector<int>* m_eclpidNCrystals; /**< PID track number of crystals */
    std::vector<int>* m_eclpidNClusters; /**< PID track number of clusters */
    std::vector<double>* m_eclLogLikeEl; /**< PID track electron likelyhood */
    std::vector<double>* m_eclLogLikeMu; /**< PID track muon likelyhood */
    std::vector<double>* m_eclLogLikePi; /**< PID track pion likelyhood */
  };

}
