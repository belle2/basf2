/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>


namespace Belle2 {
  /** Module turning SpacePointsTrackCands to RecoTracks. */
  class SPTC2RTConverterModule : public Module {
  public:
    SPTC2RTConverterModule();

    void initialize() override;

    void event() override;

    void terminate() override;


  protected:
    // store array names
    std::string m_param_spacePointTCsStoreArrayName; /**< Input SpacePointTrackCands StoreArray name. */
    std::string m_param_recoTracksStoreArrayName; /**< Output RecoTracks StoreArray name. */

    std::string m_param_recoHitInformationStoreArrayName; /**< StoreArray name of RecoHitInformation. Related to RecoTracks. */
    std::string m_param_pxdHitsStoreArrayName; /**< StoreArray name of PXDhits. Related to RecoTracks. */
    std::string m_param_svdHitsStoreArrayName; /**< StoreArray name of SVDhits. Related to RecoTracks. */

    std::string m_param_pxdClustersName; /**< StoreArray name of PXDClusters. Related to SpacePoints. */
    std::string m_param_svdClustersName; /**< StoreArray name of SVDClusters. Related to SpacePoints. */

    // store array members
    StoreArray<RecoTrack> m_recoTracks; /**< StoreArray as class member to prevent relinking for every SPTC. */
    StoreArray<SpacePointTrackCand> m_spacePointTCs; /**< StoreArray as class member to prevent relinking for every event. */

    // some counters for summary output
    unsigned int m_SPTCCtr; /**< Counter for SpacePointTrackCands presented to the module */
    unsigned int m_RTCtr; /**< Counter for RecoTracks that were actually created by the module */

    /** Creates a RecoTrack corresponding to the given SpacePointTrackCand and appends it to the RecoTracks StoreArray.
     * Relations between the SpacePointTrackCandidate and the RecoTrack are added in the process.
     * @param spacePointTC SpacePointTrackCandidate to be converted. */
    void createRecoTrack(const SpacePointTrackCand& spacePointTC);

    void initializeCounters(); /**< reset counters to 0 to avoid indeterministic behaviour */
  };
}
