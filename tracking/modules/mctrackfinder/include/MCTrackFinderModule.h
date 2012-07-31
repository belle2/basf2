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

namespace Belle2 {
  /**
   *  \addtogroup modules
   *  @{
   *  \addtogroup tracking_modules
   *  \ingroup modules
   *  @{ MCTrackFinderModule @} @}
   */


  /** This module use MC true Relations to define which hits belong to which particles and writes track candidates filled with necessary information into the DataStore.
   *
   *  The Relations MCParticles -> Hits for PXD, SVD and CDC are used.
   *  At the moment CDCHits, PXDTrueHits, SVDTrueHits, PXDCluster hits and SVDCluster hits can be used
   *  At the moment track candidates are created only for primary particles.
   *  For every hit the true time information is extracted from the trueHits or simHit hits.
   *  This Information is used to sort the hits in the correct order for the fitting of curling tracks.
   *  The created GFTrackCandidates can be fitted with GenFitterModule.
   *
   *  @todo: check planeIds when adding hits to GFTrackCand, maybe create track candidates not only for primary particles
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

    int m_whichParticles;                                       /**< Boolean to mark for which particles a track candidate should be created: 0 for all primaries, 1 for all tracks which reach PXD, 2 for all tracks which reach SVD, 3 for all tracks which reach CDC . */
    double m_energyCut;                                         /**< Create track candidates only for MCParticles with energy above this cut*/
    bool m_neutrals;                                            /**< Boolean to mark if track candidates should also be created for neutral particles.*/


    double m_smearing;                                          /**< Smearing of MCMomentum and MCVertex in % */

    int m_notEnoughtHitsCounter;                                /**< will hold number of tracks that do not have enough hits to form a track candidate (less than 3)*/
    int m_noTrueHitCounter;                                     /**< will hold number of cluster hits that do not have a corresponding true hit*/

  };
}

#endif /* MCTRACKFINDERMODULE_H_ */


