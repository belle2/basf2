/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCTRACKFIDNERMODULE_H_
#define MCTRACKFIDNERMODULE_H_

#include <framework/core/Module.h>
#include <string>
#include <TMatrixDSym.h>

namespace Belle2 {
  /**
   *  \addtogroup modules
   *  @{
   *  \addtogroup tracking_modules
   *  \ingroup modules
   *  @{ MCTrackFinderModule @} @}
   */


  /** This module uses the simulated truth information (MCParticles and their relations) to determine which hits belong to which particles
   * and writes track candidates filled with necessary information into the DataStore.
   *
   *  The Relations MCParticles -> Hits for PXD, SVD and CDC are used.
   *  At the moment CDCHits, PXDTrueHits, SVDTrueHits, PXDCluster hits and SVDCluster hits can be used
   *  By default only track candidates for primary particles (= particles from the generator) are created
   *  but this can be changed with the WhichParticles option.
   *  For every hit the true time information is extracted from the trueHits or simHit hits.
   *  This Information is used to sort the hits in the correct order for the fitting of curling tracks.
   *  The created GFTrackCandidates can be fitted with GenFitterModule.
   *
   *  @todo: maybe the asingment of planeIds and the true timing information is not 100 % accurate in every use case
   */
  class MCTrackFinderModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    MCTrackFinderModule();

    /** Destructor of the module. */
    ~MCTrackFinderModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize();

    /** Called when entering a new run.
     */
    void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event();
    /** This method is called if the current run ends.
     */
    void endRun();

    /** End of the event processing. */
    void terminate();


  private:

    bool m_usePXDHits;                                          /**< Boolean to select if PXDHits should be used*/
    bool m_useSVDHits;                                          /**< Boolean to select if SVDHits should be used*/
    bool m_useCDCHits;                                          /**< Boolean to select if CDCHits should be used*/

    bool m_useClusters;                                         /**< Boolean to select if PXD and SVD Clusters should be used instead of default TrueHits. */
    bool m_enforceTrueHit;                                      /**< If set true only cluster hits that have a relation to a TrueHit will be included in the track candidate */
    std::vector<std::string> m_whichParticles;                  /**< List of keywords to mark what properties particles must have to get a track candidate . */
    int m_particleProperties;                                   /**< Internal encoding of m_whichParticles to avoid string comparisons */
    double m_energyCut;                                         /**< Create track candidates only for MCParticles with energy above this cut*/
    bool m_neutrals;                                            /**< Boolean to mark if track candidates should also be created for neutral particles.*/

    double m_smearing;                                          /**< Smearing of MCMomentum and MCVertex in %. This adds a relative error to the initial values without changing the default large initial covariance matrix using for fitting*/
    std::vector<double> m_smearingCov;                          /**< Covariance matrix used to smear the true pos and mom before passed to track candidate. This matrix will also passed to Genfit as the initial covarance matrix. If any diagonal value is negative this feature will not be used. OFF DIAGNOLA ELEMENTS DO NOT HAVE AN EFFECT AT THE MOMENT */
    TMatrixDSym m_initialCov;                                      /**< The std::vector m_smearingCov will be translated into this TMatrixD*/
    int m_notEnoughtHitsCounter;                                /**< will hold number of tracks that do not have enough hits to form a track candidate (total NDF less than 5)*/
    int m_noTrueHitCounter;                                     /**< will hold number of cluster hits that do not have a corresponding true hit*/
    int m_nTrackCands;                                          /**< will hold the total number of created track candidates*/
    std::string m_gfTrackCandsColName;                          /**< TrackCandidates collection name */
    int m_minimalNdf;                                           /**< Minimum number of hits per track to allow track candidate creation*/
    std::vector<int> m_fromPdgCodes;                            /**< if size() is not 0, only for particles having an ancestor (mother or mother of mother etc) with PDG codes same as in this vector a track candidate be created*/
    std::vector<int> m_particlePdgCodes;                        /**< if size() is not 0, only for particles with PDG codes same as in this vector a track candidate will be created*/
  };
}

#endif /* MCTRACKFINDERMODULE_H_ */


