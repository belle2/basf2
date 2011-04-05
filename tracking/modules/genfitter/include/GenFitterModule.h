/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef GENFITTERMODULE_H
#define GENFITTERMODULE_H

#include <framework/core/Module.h>
#include <fstream>

namespace Belle2 {


  class GenFitterModule : public Module {

  public:

    /** Constructor .
     */
    GenFitterModule();

    /** Destructor.
     */
    virtual ~GenFitterModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     */
    virtual void terminate();

  protected:


  private:

    std::string m_mcParticlesCollectionName;                /**< MC particle collection name */
    std::string m_cdcRecoHitsCollectionName;                /**< CDCRecoHits collection name */

    std::string m_tracksCollectionName;                     /**< Tracks collection name */
    std::string m_trackToCDCRecoHitCollectionName;          /**< Tracks to CDCRecoHits relation name */
    std::string m_trackToMCParticleCollectionName;          /**< Tracks to MCParticles relation name  */

    std::string m_cdcTrackCandsCollectionName;              /**< CDCTrackCandidates collection name */
    std::string m_cdcTrackCandToRecoHitsCollectionName;     /**< CDCTrackCandidates to CDCRecoHits relation name */
    std::string m_cdcTrackCandToMCParticleCollectionName;   /**< CDCTrackCandidates to MCParticles relation name */

    std::string m_mcMatchParticlesCollectionName;           /**< MCMatchParticles collection name */

    bool m_fitMCTracks;                                     /**< True if MC tracks should be fitted */
    bool m_fitRecoTracks;                                   /**< True if track candidates from pattern recognition should be fitted */


  };
} // end namespace Belle2


#endif /* GENFITTERMODULE_H */
