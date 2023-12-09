/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <TMatrixDSym.h>
#include <string>

namespace Belle2 {

  /** This module uses the simulated truth information (MCParticles and their relations) to determine which hits belong to which particles
   * and writes track candidates filled with necessary information into the DataStore.
   *
   *  The Relations MCParticles -> Hits for PXD, SVD and CDC are used.
   *  At the moment CDCHits, PXDTrueHits, SVDTrueHits, PXDCluster hits and SVDCluster hits can be used
   *  By default only track candidates for primary particles (= particles from the generator) are created
   *  but this can be changed with the WhichParticles option.
   *  For every hit the true time information is extracted from the trueHits or simHit hits.
   *  This Information is used to sort the hits in the correct order for the fitting of curling tracks.
   *  The created genfit::TrackCandidates can be fitted with GenFitterModule.
   *
   *  @todo: maybe the asingment of planeIds and the true timing information is not 100 % accurate in every use case
   */
  class TrackFinderMCTruthRecoTracksModule : public Module {

  public:
    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    TrackFinderMCTruthRecoTracksModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize() override;

    /** Called when entering a new run.
     */
    void beginRun() override;

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event() override;
    /** This method is called if the current run ends.
     */
    void endRun() override;

  private:

    /** helper function which returns true if the current hit is within n loops
     * the template give the hit type and the according sim hit type (e.g. CDCHit and CDCSimHit)
     * @param Bz: the z-component of the B field
     * @param aHit: pointer to the hit under investiation
     * @param nLoops: the number of loops the hit should be in
     * @return : returns true if the hit is on the < nLoops th loop of the track*/
    template<class THit, class TSimHit>
    bool isWithinNLoops(double Bz, const THit* aHit, double nLoops);

    /** helper function which returns true if the current hit has a creation time after
     * the MCParticle left the CDC and entered TOP. These tracks that re-enter the tracking
     * volume are more difficult to reconstruct due to the energy loss in the CDC outer all and in TOP.
     * The template gives the hit type and the according sim hit type (e.g. CDCHit and CDCSimHit)
     * @param aHit: pointer to the hit under investiation
     * @return : returns true if the the track left CDC and entered TOP before this hit was registered
     */
    template<class THit, class TSimHit>
    bool didParticleExitCDC(const THit* aHit);

    bool m_usePXDHits;                                          /**< Boolean to select if PXDHits should be used*/
    bool m_useSVDHits;                                          /**< Boolean to select if SVDHits should be used*/
    bool m_useCDCHits;                                          /**< Boolean to select if CDCHits should be used*/
    bool m_useOnlyAxialCDCHits;                                 /**< Boolean to select if only axial CDCHits should be used*/
    bool m_useOnlyBeforeTOP;                                    /**< Boolean to select if hits after TOP detector are discarded. */
    float m_useNLoops;                                          /**< Number of loops to include in the MC tracks. */
    bool m_useReassignedHits;                                   /**< Boolean to select the inclusion of hits form discarded secondary daughters*/
    bool m_useSecondCDCHits;                  /**< Also includes the CDC 2nd hit information in the mc tracks.*/

    bool m_enforceTrueHit;                                      /**< If set true only cluster hits that have a relation to a TrueHit will be included in the track candidate */
    std::vector<std::string>
    m_whichParticles;                  /**< List of keywords to mark what properties particles must have to get a track candidate . */
    int m_particleProperties =
      0;                               /**< Internal encoding of m_whichParticles to avoid string comparisons */
    double m_energyCut;                                         /**< Create track candidates only for MCParticles with energy above this cut*/
    bool m_neutrals;                                            /**< Boolean to mark if track candidates should also be created for neutral particles.*/
    bool m_mergeDecayInFlight;                                  /**< Boolean to merge decay in flight chains that involve a single charged particle */

    bool m_setTimeSeed;                                         /**< Boolean to forward the production time as seed time*/
    double m_smearing;                                          /**< Smearing of MCMomentum and MCVertex in %. This adds a relative error to the initial values without changing the default large initial covariance matrix using for fitting*/
    std::vector<double>
    m_smearingCov;                          /**< Covariance matrix used to smear the true pos and mom before passed to track candidate. This matrix will also passed to Genfit as the initial covarance matrix. If any diagonal value is negative this feature will not be used. OFF DIAGNOLA ELEMENTS DO NOT HAVE AN EFFECT AT THE MOMENT */
    TMatrixDSym
    m_initialCov;                                   /**< The std::vector m_smearingCov will be translated into this TMatrixD*/
    int m_notEnoughtHitsCounter =
      0;                                /**< will hold number of tracks that do not have enough hits to form a track candidate (total NDF less than 5)*/
    int m_noTrueHitCounter =
      0;                                     /**< will hold number of cluster hits that do not have a corresponding true hit*/
    int m_nRecoTracks = 0;                                          /**< will hold the total number of created track candidates*/
    std::string m_recoTracksStoreArrayName;                          /**< RecoTracks StoreArray name */
    int m_minPXDHits;                                           /**< Minimum number of PXD hits per track to allow track candidate creation*/
    int m_minSVDHits;                                           /**< Minimum number of SVD hits per track to allow track candidate creation*/
    int m_minCDCAxialHits;                                      /**< Minimum number of CDC hits from axial wires per track to allow track candidate creation*/
    int m_minCDCStereoHits;                                     /**< Minimum number of CDC hits from stereo wires per track to allow track candidate creation*/
    bool m_allowFirstCDCSuperLayerOnly;                         /**< Boolean to allow tracks to pass the stereo hit requirement if they touched only the first (axial) CDC layer */
    int m_minimalNdf;                                           /**< Minimum number of total hits per track to allow track candidate creation. 2D hits are counted as 2*/
    std::vector<int>
    m_fromPdgCodes;                            /**< if size() is not 0, only for particles having an ancestor (parent or parent of parent etc) with PDG codes same as in this vector a track candidate be created*/
    std::vector<int>
    m_particlePdgCodes;                        /**< if size() is not 0, only for particles with PDG codes same as in this vector a track candidate will be created*/
    /** To be used together with WhichParticles to select the ancestor and daughters.
     *  If true, only check the direct parent to be contained in the list of possible ancestors.
     *  If false, check all ancestors in in the list of possible ancestors.
     *  This could be used to e.g. only create MCRecoTracks for slow pions from D* decays.
     */
    bool m_onlyCheckDirectParentPdgCode = false;

    bool m_mcParticlesPresent =
      false; /**< This flag is set to false if there are no MC Particles in the data store (probably data run?) and we can not create MC Reco tracks. */
    double m_splitAfterDeltaT; /**< Minimal time delay between two sim hits (in ns) after which MC reco track will be split into seperate tracks. If < 0, don't do splitting.*/

    bool m_discardAuxiliaryHits = false; /**< if true hits marked as auxiliary will not be included in the RecoTrack */

    std::vector<uint> m_param_useCDCSuperLayers;  /**< List of super layers to be used */

    std::vector<uint> m_param_useCDCLayers;       /**< List of layers to be used */

    std::vector<uint> m_param_ignoreCDCLayers;    /**< List of layers to be ignored in tracking */

    std::array<bool, 9> m_useCDCSuperLayers{};   /**< Bits for the used super layers ATTENTION: hardcoded value for number of super layers */

    std::array<bool, 56> m_useCDCLayers{};       /**< Bits for the used layers ATTENTION: hardcoded value for number of layers */

    StoreArray<MCParticle> m_MCParticles;   /**< MCParticles StoreArray */
    StoreArray<RecoTrack> m_RecoTracks;     /**< RecoTracks StoreArray */
    StoreArray<PXDTrueHit> m_PXDTrueHits;   /**< PXDTrueHits StoreArray */
    StoreArray<PXDCluster> m_PXDClusters;   /**< PXDClusters StoreArray */
    StoreArray<SVDTrueHit> m_SVDTrueHits;   /**< SVDTrueHits StoreArray */
    StoreArray<SVDCluster> m_SVDClusters;   /**< SVDTrueHits StoreArray */
    StoreArray<CDCHit> m_CDCHits;           /**< CDCHits StoreArray */
    StoreArray<CDCSimHit> m_CDCSimHits;     /**< CDCSimHits StoreArray */
  };
}

