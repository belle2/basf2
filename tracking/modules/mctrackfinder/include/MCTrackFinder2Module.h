/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* This is modified version of the MCTrackFinderModule to also use reco hits form
 * pxd and svd at a hopfully not so far away point in the future this module
 * will be merged with MCTrackFinderModule */

#ifndef MCTRACKFIDNER2MODULE_H_
#define MCTRACKFIDNER2MODULE_H_

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>
#include <cdc/hitcdc/CDCSimHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDHit.h>
#include <fstream>

namespace Belle2 {

  /** This module tries to use Relations to define which hits belong to which particles and writes track candidates into the DataStore.
   *
   *  The Relations between MCParticles <-> SimHits <-> Hits <-> RecoHits have to be used for this purpose.
   *
   *  @author <a href="mailto:moritz_nadler@gmx.de?subject=MCTrackFinder2">Martin Heck</a>
   *
   *  @todo   Currently only the CDC is used in this module. This is mainly for testing purposes. Once everything is defined nicely, SVD and PXD will be added.
   */
  class MCTrackFinder2Module : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    MCTrackFinder2Module();

    /** Destructor of the module. */
    ~MCTrackFinder2Module();

    void initialize();

    /** Here the actual work is done. */
    void event();

    /** End of the event processing. */
    void terminate();


  private:

    std::string m_mcParticlesCollectionName;                /**< MCParticles collection name */
    std::string m_mcPartToCDCSimHitsCollectionName;         /**< MCParticles to CDCSimHits relation name */
    std::string m_cdcRecoHitsCollectionName;                /**< CDCRecoHits collection name */
    std::string m_cdcSimHitToCDCHitCollectioName;           /**< CDCSimHits to CDCHits relation name */
    std::string m_tracksCollectionName;                     /**< Tracks collection name */
    std::string m_trackToMCParticleCollectionName;          /**< Tracks to MCParticles relation name */
    std::string m_trackToCDCRecoHitCollectionName;          /**< Tracks to CDCRecoHits relation name */

    std::string m_mcPartToPxdSimHitsColName;
    std::string m_mcPartToSvdSimHitsColName;
    std::string m_pxdRecoHitColName;
    std::string m_svdRecoHitColName;
    std::string m_trackToPxdRecoHitCollectionName;
    std::string m_trackToSvdRecoHitCollectionName;
    ofstream dataOutCdc;
    ofstream dataOutPxd;
    ofstream dataOutSvd;
  };
}

#endif /* MCTRACKFINDERMODULE_H_ */
