/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/Module.h>


namespace Belle2 {

  /** The quality estimator module for SpacePointTrackCandidates using a MC information.
   * WARNING: Only working for SVDClusters!
   * */
  class MCQualityEstimatorVXDModule : public Module {

  public:


    /** Constructor of the module. */
    MCQualityEstimatorVXDModule();

    virtual void initialize()
    {
      m_spacePointTrackCands.isRequired(m_SPTCsName);
      m_mcRecoTracks.isRequired(m_mcRecoTracksName);
    }

    virtual void event();


  protected:

    void fillSVDClusterToRecoTrackMap();

    int countMatchesToMCRecoTrack(SpacePointTrackCand sptc);


    // module parameters


    /** sets the name of expected StoreArray with mcRecoTracks in it. */
    std::string m_mcRecoTracksName;

    /** sets the name of expected StoreArray with SPTCs in it. */
    std::string m_SPTCsName;

    /** the storeArray for SpacePointTrackCands as member */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** the storeArray for RecoTracks as member */
    StoreArray<RecoTrack> m_mcRecoTracks;

    /** store relation SVDClusterIndex to MCRecoTrackIndex */
    std::map<int, int> m_mapIndexSVDToIndexRecoTrack;
  };
}
