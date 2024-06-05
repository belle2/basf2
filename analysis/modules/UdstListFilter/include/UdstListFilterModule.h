/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>


// framework - DataStore
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// DataObjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

namespace Belle2 {
  /**
   *  Module to filter udst content based on a particle list
   */
  class UdstListFilterModule : public Module {
  public:
    /**
     * Constructor
     */
    UdstListFilterModule();
    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;
    /**
     * Event processor.
     */
    virtual void event() override;
  private:

    /**
     * Update V0 mdst datastore object
     */
    void swapV0s();


    StoreObjPtr<ParticleList> m_plist;    /**< list used for cleaning */
    std::string m_listName;  /**< Breco particle list name */
    bool m_reverse; /**<Reverse selection: keep objects NOT mentioned in the list */
    StoreArray<MCParticle> m_mcparticles; /**< StoreArray of MCParticles */
    StoreArray<ECLCluster> m_eclclusters; /**< StoreArray of ECLCluster */
    StoreArray<KLMCluster> m_klmclusters; /**< StoreArray of KLMCluster */
    StoreArray<PIDLikelihood> m_pidlikelihoods; /**< StoreArray of PIDLikelihoods */
    StoreArray<Track> m_tracks; /**< StoreArray of Tracks */
    StoreArray<TrackFitResult> m_trackfitresults; /**< StoreArray of TrackFitResults */
    StoreArray<V0> m_v0s; /**< StoreArray of V0s */
    StoreArray<V0>*  m_selectedV0s; /**< New selected V0 array */
    SelectSubset< ECLCluster > m_ecl_selector;  /**< Selector of sub-set of ECL clusters */
    SelectSubset< Track > m_track_selector;     /**< Selector of sub-set of tracks */
    SelectSubset< KLMCluster > m_klm_selector;  /**< Selector of sub-set of KLM clusters */
    SelectSubset< V0 > m_v0_selector;           /**< Selector of sub-set of V0s */
  };
}
