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
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>


namespace Belle2 {

  /** This module tries to use Relations to define which hits belong to which particles and writes track candidates into the DataStore.
   *
   *  The Relations MCParticles -> RecoHits for PXD, SVD and CDC are used.
   *
   *
   *  @todo further testing of the code. Maybe clean up and more consistency in variable names
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

    void initialize();

    /** Here the actual work is done. */
    void event();

    /** End of the event processing. */
    void terminate();


  private:

    /** Returns the list of 'from' indices for the given 'to' index for the given relation.
     */
    std::list<int> getFromForTo(const std::string& relationName, const unsigned short int& toIndex);

    /** Returns the 'to' index for the given 'from' index for the given relation.
     */
    int getToForFrom(const std::string relationName, int fromIndex) {
      StoreArray<Relation> relations(relationName);
      for (int ii = 0; ii < relations->GetEntriesFast(); ii++) {
        if (relations[ii]->getFromIndex() == fromIndex) {
          return (relations[ii]->getToIndex());
        }
      }
      return (-999);
    }

    std::string m_mcParticlesColName;                           /**< MCParticles collection name */

    std::string m_cdcRecoHitColName;                            /**< CDCRecoHits collection name */
    std::string m_mcParticleToCdcRecoHits;                      /**< MCParticles -> CDCRecoHits relation name */

    std::string m_pxdRecoHitColName;                            /**< PXDRecoHits collection name */
    std::string m_mcParticleToPxdRecoHits;                      /**< MCParticles -> PXDRecoHits relation name */

    std::string m_svdRecoHitColName;                            /**< SVDRecoHits collection name */
    std::string m_mcParticleToSvdRecoHits;                      /**< MCParticles -> SVDRecoHits relation name */

    std::string m_gfTrackCandsColName;                          /**< TrackCandidates collection name */
    std::string m_gfTrackCandToMCParticleColName;               /**< TrackCandidates to MCParticles relation name */
    std::string m_gfTrackCandToCdcRecoHitsColName;              /**< TrackCandidates to CDCRecoHits relation name */
    std::string m_gfTrackCandToPxdRecoHitsColName;              /**< TrackCandidates to PXDRecoHits relation name */
    std::string m_gfTrackCandToSvdRecoHitsColName;              /**< TrackCandidates to SVDRecoHits relation name */
  };
}

#endif /* MCTRACKFINDERMODULE_H_ */
