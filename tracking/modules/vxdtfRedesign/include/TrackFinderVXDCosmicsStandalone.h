/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <utility>

// fw
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>

// tracking
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/tcTools/SpacePointTrackCandCreator.h>
#include <tracking/trackFindingVXD/algorithms/StandaloneCosmicsCollector.h>


namespace Belle2 {
  /** The TrackFinderVXDCosmicsStandaloneModule
   *
   */
  class TrackFinderVXDCosmicsStandaloneModule final : public Module {

  public:

    /** Constructor */
    TrackFinderVXDCosmicsStandaloneModule();

    /** initialize */
    void initialize() override;

    /** beginRun */
    void beginRun() override;

    /** event */
    void event() override;


  protected:

    /// module parameters
    /** SpacePoint collection name */
    std::string m_spacePointsName;

    /** The storeArray for SpacePoints */
    StoreArray<SpacePoint> m_spacePoints;

    /** Name for StoreArray< SpacePointTrackCand> to be filled */
    std::string m_PARAMSpacePointTrackCandArrayName;

    StandaloneCosmicsCollector m_SCC;

    /// member variables
    /** Tool for creating SPTCs, which fills storeArray directly. */
    SpacePointTrackCandCreator<StoreArray<Belle2::SpacePointTrackCand>> m_sptcCreator;

    /// output containers
    /** StoreArray for the TCs created in this module. */
    StoreArray<Belle2::SpacePointTrackCand> m_TCs;

    double m_qualityCut = 0.0001;
    int m_minSPs = 3;
    int m_maxRejectedSPs = 5;

    /** Event number counter. */
    unsigned int m_eventCounter = 0;

  private:
  };
} // Belle2 namespace
