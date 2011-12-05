/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 510 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKINGOUTPUTMODULE_H_
#define TRACKINGOUTPUTMODULE_H_

#include <framework/core/Module.h>

namespace Belle2 {

  /** This is an auxiliary output module for tracking.
   *  It uses the MC information, the Tracks from MCFitting and the Tracks from fitting with pattern recognition.
   *  Many different variables describing the fit results are written out to analyze the performance.
   *  The idea is to have for each primary MCParticle (or MCParticles which reach certain subdetectors) an TrackingOutput, where all the information concerning this particle can be stored (MCtruth together with fit results).
   *  The produced TrackingOutput collection can be written out with SimpleOutputModule.
   */
  class TrackingOutputModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    TrackingOutputModule();

    /** Destructor of the module. */
    ~TrackingOutputModule();

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

    /** Returns Ids of the Tracks created by the given MCParticle.
     */
    std::vector <int> getTrackIdsForMCId(std::string Tracks, int MCId);

  private:

    std::string m_mcParticlesColName;            /**< MCParticles collection name */

    std::string m_tracksMCColName;               /**< MCTracks (Tracks created by fitting MC truth) collection name */

    std::string m_tracksPRColName;               /**< PRTracks (Tracks created by fitting pattern reco results) collection name */

    std::string m_gfTracksMCColName;             /**< MCGFTracks (GFTracks created by fitting MC truth) collection name */

    std::string m_gfTracksPRColName;             /**< PRGFTracks (GFTracks created by fitting pattern reco results) collection name */

    std::string m_outputCollectionName;          /**< Name of the output collection (TrackingOutput) */

    int m_whichParticles;                        /**< Boolean to mark for which particles an output should be created: 0 for all primaries, 1 for tracks which reach PXD, 2 for tracks which reach SVD, 3 for tracks which reach CDC . */
    double m_energyCut;                          /**< Create track candidates only for MCParticles with energy above this cut*/
    bool m_neutrals;                             /**< Boolean to mark if track candidates should also be created for neutral particles.*/

    int m_nMCPart;                               /**< Number of interesting MCParticles */

    int m_mcSuccessCounter;                      /**< Counter for successfully fitted MCTracks */
    int m_prSuccessCounter;                      /**< Counter for successfully fitted PRTracks */

    std::vector <int> m_mcFitTracks;             /** MCTracks Ids for the given MCParticle */
    std::vector <int> m_prFitTracks;             /** PRTracks Ids for the given MCParticle */

    int m_nMCFitTracks;                          /** Number of MCTrackIds for a given MCParticle. */
    int m_nPRFitTracks;                          /** Number of PRTrackIds for a given MCParticle. */


  };
}

#endif /* TRACKINGOUTPUTMODULE_H_ */
