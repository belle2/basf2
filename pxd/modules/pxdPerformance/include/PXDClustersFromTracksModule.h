/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan LIU                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

namespace Belle2 {
  /**
   * The PXDClustersFromTracks module
   *
   * This module generates a new StoreArray for track matched PXDClusters.
   * Its input StoreArray is PXDClusters
   * The selection criterion is based on the relation with the StoreArray RecoTracks.
   * New relation between PXDClustersFromTracks and RecoTracks is also derived.
   *
   */
  class PXDClustersFromTracksModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDClustersFromTracksModule();

    /** if required */
    virtual ~PXDClustersFromTracksModule();

    /** initializes the module */
    virtual void beginRun() override;

    /** processes the event */
    virtual void event() override;

    /** end the run */
    virtual void endRun() override;

    /** terminates the module */
    virtual void terminate() override;

    /** init the module */
    virtual void initialize() override;

  private:

    std::string m_pxdClustersName; /**<SVDCLuster store array*/
    std::string m_recoTracksName; /**<reco track store array*/
    std::string m_tracksName; /**<Track store array*/

    bool m_inheritance; /**< if true all relations are inherited*/

    std::string m_outputArrayName;  /**< StoreArray with the selected PXD clusters*/

    SelectSubset<PXDCluster> m_selectedPXDClusters; /**< all PXD clusters */

    static bool isRelatedToTrack(const PXDCluster* pxdCluster); /**< select the PXD clusters related to tracks **/

  };
}
