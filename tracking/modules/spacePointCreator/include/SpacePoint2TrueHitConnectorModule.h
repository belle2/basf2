/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <string>
#include <vector>
#include <array>

namespace Belle2 {

  /**
   * Module that tries to register a relation between SpacePoints and TrueHits, hence making some MC Information easily accesible for other modules working with SpacePoints (e.g. CurlingTrackCandSplitter or GFTC2SPTCConverter).
   *
   * NOTE: this module is not yet fully functional! The future goal is to get all the stuff that concerns SpacePoint <-> TrueHit relations into this module and to keep the Converter (and the CurlingTrackCandSplitter) modules free of it and only feeding them SpacePoints that were processed by this module before.
   */
  class SpacePoint2TrueHitConnectorModule : public Module {

  public:

    SpacePoint2TrueHitConnectorModule(); /**< Constructor */

    virtual void initialize(); /**< initialize: initialize counters, check StoreArrays, register StoreArrays, ... */

    virtual void event(); /**< event: try to find the appropriate TrueHit to all SpacePoints */

    virtual void terminate(); /**< terminate: print some summary information */

  protected:

    enum e_detTypes {
      c_PXD = 1,
      c_SVD = 2
    };

    std::string m_PARAMoutputSuffix; /**< suffix that will be appended to the StoreArray names of the output StoreArrays */

    std::vector<std::string> m_PARAMtrueHitNames; /**< names of containers of TrueHits */

    std::vector<std::string> m_PARAMspacePointNames; /**< names of containers of SpacePoints */

    std::vector<std::string> m_PARAMdetectorTypes; /**< detector type names as strings to determine which name belongs to which detector type */

    bool m_PARAMstoreSeparate; /**< switch for storing the SpacePoints that can be related to a TrueHit into separate StoreArrays, where only such SpacePoints are stored */

    unsigned int m_nContainers; /**< number of passed containers -> storing the size of an input vector for not having to obtain it every time */

    unsigned int m_nPXDarrays; /**< number of PXD StoreArrays */

    unsigned int m_nSVDarrays; /**< number os SVD StoreArrays */

    unsigned int m_nPXDSpacePointsCtr; /**< Number of PXD SpacePoints */

    unsigned int m_nSVDSpacePointsCtr; /**< Number SVD SpacePoints */

    unsigned int m_regPXDrelCtr; /**< number of (newly) registerd relations for PXD SpacePoints */

    unsigned int m_regSVDrelCtr; /**< number of (newly) registerd relations for SVD SpacePoints */

    std::array<unsigned int, 5> m_nRelPXDTrueHitsCtr; /**< counter for counting different numbers of related TrueHits (to a SpacePoint) with one variable */

    std::array<unsigned int, 5> m_nRelSVDTrueHitsCtr; /**< counter for counting different numbers of related TrueHits (to a SpacePoint) with one variable */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_inputPXDSpacePoints; /**< StoreArrays of input SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_inputSVDSpacePoints; /**< StoreArrays of input SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::PXDTrueHit> > m_PXDTrueHits; /**< StoreArrays of PXDTrueHits to which relations will get registered */

    std::vector<Belle2::StoreArray<Belle2::SVDTrueHit> > m_SVDTrueHits; /**< StoreArrays of SVDTrueHits to which relations will get registered */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_outputPXDSpacePoints; /**< StoreArrays of output SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_outputSVDSpacePoints; /**< StoreArrays of output SpacePoints */

    std::vector<e_detTypes> m_detectorTypes; /**< vector storing the detector types for easier access during initialization */

    void initializeCounters(); /**< initialize all counters to 0 */

    /**< get the unique indices of all TrueHits that are related to a SpacePoint (done via their Clusters) */
    template<typename ClusterType>
    std::vector<unsigned int> getUniqueTrueHitIndices(Belle2::SpacePoint* spacePoint, RelationArray& clusters2TrueHits);
  };
}