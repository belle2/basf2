/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/EventExtraInfo.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <TRotation.h>

#include <string>

namespace Belle2 {

  /**
   * If the content of two DataStores are merged using the 'MergeDataStoreModule', several kinematic properties might need fix.
   */
  class PostMergeUpdaterModule : public Module {

  public:

    /**
     * Constructor
     */
    PostMergeUpdaterModule();

    /**
     * Initialize the Module.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    /**
     * Helper function to determine rotation matrix
     */
    TRotation tag_vertex_rotation();

    /**
     * Rotate clusters
     */
    void cluster_rotation(const TRotation& rot);

    StoreArray<TrackFitResult> m_trackFits; /**< track fits */
    StoreArray<Track> m_tracks; /**< tracks */
    StoreArray<ECLCluster> m_eclclusters; /**< StoreArray of ECLCluster */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo; /**< Event extra info */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo_orig; /**<  Event extra info original */
    StoreObjPtr<EventExtraInfo> m_mergedArrayIndices; /**< indices where the StoreArrays were merged */
    bool m_mixing; /**< Fix to common vertex */
    bool m_isCharged; /**< B+ or B0 */
  };

} // Belle2 namespace

