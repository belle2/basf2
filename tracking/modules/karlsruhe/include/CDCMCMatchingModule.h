/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCMCMATCHINGMODULE_H
#define CDCMCMATCHINGMODULE_H

#include <framework/core/Module.h>
#include <fstream>

namespace Belle2 {

  /** Module to match the CDCTrackCandidates with MCParticles and evaluate the performance of pattern recognition.
   *  This modules perfoms MCMatching from 'two different sides'.
   *  First it is checked which MCParticles contributed to CDCTrackCandidates, the Id of the matched MCParticle and the purity are then assigned to CDCTrackCandidates and a relation between MCParticle and TrackCandidate is created.
   *  In the next step it is checked how well were the MCParticles reconstructed.
   *  This is done by creating a Collection of MCMatchParticles, where the information about the matched track is stored.
   */

  class CDCMCMatchingModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    CDCMCMatchingModule();

    /**
     * Destructor of the module.
     */
    virtual ~CDCMCMatchingModule();

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

    std::string m_mcParticlesCollectionName;             /**< MC particle collection name */
    std::string m_mcPartToCDCSimHitsCollectionName;      /**< MC particle to CDCSimHits relation name */
    std::string m_cdcSimHitToCDCHitCollectioName;        /**< CDCSimHits to CDCHits relation name */

    std::string m_cdcRecoHitsCollectionName;             /**< CDCRecoHits collection name */

    std::string m_cdcTrackCandsCollectionName;           /**< CDCTrackCandidates collection name */
    std::string m_cdcTrackCandsToRecoHits;               /**< CDCTrackCandidates to CDCRecoHits relation name */

    std::string m_cdcTrackCandsToMCParticles;            /**< CDCTrackCandidates to MCParticles relation name */
    std::string m_mcMatchParticlesCollectionName;        /**< MCMatchParticles collection name */


  };
} // end namespace Belle2


#endif /* CDCMCMATCHINGMODULE_H */
