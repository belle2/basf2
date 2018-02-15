/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                            *
 *                                                                        *
 **************************************************************************/

# pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/trackFindingVXD/sectorMapTools/NoKickRTSel.h>
#include <tracking/trackFindingVXD/sectorMapTools/NoKickCuts.h>

#include <boost/optional.hpp>
#include <bitset>

namespace Belle2 {
  /**
   * Module for converting RecoTracks to SpacePointTrackCands
   *
   * NOTE: currently only working with SVD clusters!
   *
   * Intended Behaviour: The Module takes a RecoTrack and converts it to a SpacePointTrackCand
   * by taking the Clusters (PXD and SVD) of the RecoTrack and then look for Relations of these Clusters to SpacePoints.
   * NOTE: other hits than PXD and SVD are ignored!
   *
   */
  class RT2SPTCConverterModule : public Module {

  public:

    RT2SPTCConverterModule(); /**< Constructor*/

    void initialize()
    override; /**< initialize module (e.g. check if all required StoreArrays are present or registering new StoreArrays) */

    void event() override; /**< event: convert RecoTracks to SpacePointTrackCands */

    void endRun() override;

    void terminate() override; /**< terminate: print some summary information on the processed events */

  protected:

    /**
     * enum for differentiating reasons why a conversion failed
     */
    enum conversionFlags {
      c_singleCluster = 0,
      c_undefinedError = 1
    };

    using ConversionState = std::bitset<2>; /**< Used to store conversionFlags and pass them between methods */

    /* Convert Clusters to SpacePoints using the Relation: Cluster->SpacePoint */
    std::pair<std::vector<const SpacePoint*>, ConversionState>
    getSpacePointsFromRecoHitInformations(std::vector<RecoHitInformation*> hitInfos);

    /* Convert Clusters to SpacePoints using the Relation: Cluster->TrueHit->SpacePoint */
    std::pair<std::vector<const SpacePoint*>, ConversionState>
    getSpacePointsFromRecoHitInformationViaTrueHits(std::vector<RecoHitInformation*> hitInfos);

    /** reset counters to 0 to avoid indeterministic behaviour */
    void initializeCounters()
    {
      m_noFailCtr = 0;
      m_singleClusterUseCtr = 0;
      m_undefinedErrorCtr = 0;
      m_missingTrueHitCtr = 0;
      m_minSPCtr = 0;
      m_mcParticlesPresent = false;
    }

    std::string m_SVDClusterName; /**< SVDCluster collection name */

    boost::optional<std::string> m_pxdSpacePointsStoreArrayName; /**< PXD SpacePoints collection names */
    boost::optional<std::string> m_svdSpacePointsStoreArrayName; /**< Non SingleCluster SVD SpacePoints collection names */

    std::string m_SVDSingleClusterSPName; /**< Single Cluster SVD SpacePoints collection name */

    std::string m_RecoTracksName; /**< Name of collection of RecoTrack StoreArray */

    std::string m_SPTCName; /**< Name of collection under which SpacePointTrackCands will be stored in the StoreArray */

    // parameters
    bool m_ignorePXDHits; /**< PXD hits will be ignored when creating the SP track candidate */

    int m_minSP; /**< parameter for specifying a minimal number of SpacePoints a SpacePointTrackCand has to have in order to be registered in the DataStore */
    bool m_useTrueHits; /**< If true the method getSpacePointsFromSVDClustersViaTrueHits is utilized. Requires TrueHits to be present and to have relations to SpacePoints! */
    bool m_skipProblematicCluster; /**< If true problematic clusters are ignored. If false the conversion of a RecoTrack containing such a cluster is aborted */
    bool m_useSingleClusterSP; /**< If true use single cluster SpacePoint collection as fallback */
    bool m_markRecoTracks; /**< If True RecoTracks where conversion problems occurred are marked dirty */

    /** NoKickCuts members */
    NoKickRTSel* m_trackSel; /**< member to call method of NoKickCuts selection */
    std::string m_noKickCutsFile; /**< name of TFile of the cuts */
    bool m_noKickOutput; /**< true=produce TFile with effects of NoKickCuts on tracks */

    int m_ncut = 0; /**< counter of the cuttet tracks */
    int m_npass = 0; /**< counter of the selected tracks */


    // state variables

    bool m_mcParticlesPresent;

    unsigned int m_minSPCtr; /** < Counts how many tracks didn't contain enough SpacePoints after conversion */

    unsigned int m_noFailCtr; /**< Counts how many tracks could be converted without any problems */

    unsigned int m_singleClusterUseCtr; /**< Counts how many tracks contained a single cluster */

    unsigned int m_undefinedErrorCtr; /**< Counts how many tracks failed to be converted */

    unsigned int m_missingTrueHitCtr; /**< Counts how many times a SpacePoint had no relation to a SVDTrueHit */


  };
}
