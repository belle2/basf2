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
//#include <fstream>

namespace Belle2 {

  /** Module to match the GFTrackCandidates with MCParticles to be able to evaluate the performance of pattern recognition.
   *  It is checked which MCParticles created the CDCHits assigned to this TrackCandidates.
   *  The MCParticle with the largest contribution is evaluated and its ID is assigned to the GFTrackCandidate.
   *  @todo: at the moment this module is only usable for CDC pattern recognition, at some point VTX detectors also could be included
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

    /** This method adds a new MCParticle Id to the pair vector mcParticleContributions.
     *  This vector stores pairs <MCParticleId, Number of Hits from this MCParticle>.
     *  If there are already entries from this MCId, the corresponding number of hits is increased by 1.
     *  If there are no entries from this MCId, a new entry with 1 hit is created.
     */
    virtual void addMCParticle(std:: vector <std::pair<int, int> >  & mcParticleContributions, int mcId);

    /** This method evaluates the MCParticle Id with the largest contribution from the vector mcParticleContributions.
     *  This vector stores pairs <MCParticleId, Number of Hits from this MCParticle>.
     *  The method returns the Id of the MCParticle with the most hits and the fraction contributed hits/total hits * 100 .
     */
    virtual std::pair <int, float> getBestMCId(std:: vector <std::pair<int, int> >  mcParticleContributions, int nHit);



  protected:


  private:

    std::string m_mcParticlesCollectionName;            /**< MC particle collection name */

    std::string m_cdcHitsCollectionName;                /**< CDCHits collection name */

    std::string m_mcPartToCDCHits;                      /**< MCParticles to CDCHits relation name */

    std::string m_gfTrackCandsCollectionName;           /**< GFTrackCandidates collection name */

    std::string m_gfTrackCandsToMCParticles;            /**< GFTrackCandidates to MCParticles relation name */



  };
} // end namespace Belle2


#endif /* CDCMCMATCHINGMODULE_H */

