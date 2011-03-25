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
   *  First it is checked which MCParticles contributed to CDCTrackCandidates, the Id of the matched MCParticle and the purity are then assigned to CDCTrackCandidates.
   *  In the next step it is checked how well were the MCParticles reconstructed.
   *
   */

  class CDCMCMatchingModule : public Module {

  public:


    CDCMCMatchingModule();


    virtual ~CDCMCMatchingModule();


    virtual void initialize();


    virtual void beginRun();


    virtual void event();


    virtual void endRun();


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


  };
} // end namespace Belle2


#endif /* CDCMCMATCHINGMODULE_H */
