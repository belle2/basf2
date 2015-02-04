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
#include <tuple>

#include <numeric> // std::accumulate

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

    unsigned int m_negWeightCtr; /**< number of negative weights */

    unsigned int m_totWeightsCtr; /**< total number of weights */

    unsigned int m_noTrueHitCtr; /**< Number of SpacePoints that contained a Cluster to which no TrueHit has been found */

    std::array<unsigned int, 5> m_nRelPXDTrueHitsCtr; /**< counter for counting different numbers of related TrueHits (to a SpacePoint) with one variable */

    std::array<unsigned int, 5> m_nRelSVDTrueHitsCtr; /**< counter for counting different numbers of related TrueHits (to a SpacePoint) with one variable */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_inputPXDSpacePoints; /**< StoreArrays of input SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_inputSVDSpacePoints; /**< StoreArrays of input SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::PXDTrueHit> > m_PXDTrueHits; /**< StoreArrays of PXDTrueHits to which relations will get registered */

    std::vector<Belle2::StoreArray<Belle2::SVDTrueHit> > m_SVDTrueHits; /**< StoreArrays of SVDTrueHits to which relations will get registered */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_outputPXDSpacePoints; /**< StoreArrays of output SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_outputSVDSpacePoints; /**< StoreArrays of output SpacePoints */

    std::vector<e_detTypes> m_detectorTypes; /**< vector storing the detector types for easier access during initialization */

    std::vector<float> m_weightDiffsByAvg; /**< To investigate how the difference of weigts divided by the average of the weights is distributed -> maybe learn something from it */

    unsigned int m_moreThan2Weights; /**< Count the cases whith more than two weights */

    void initializeCounters(); /**< initialize all counters to 0 */

    // COULDDO: place the (rather) general functions below into a header to make it accessible to more than just this module

    /**
     * get the contents of the map as string. NOTE: only templated to such a degree that is actually needed in this module! should compile without warning for any map (e.g. map, multimap, unordered_map,...) with unsigned int as key and values of a type that have a defined stream insertion operator (only tested for multimap and unordered_multimap!)
     */
    template <typename MapType>
    std::string printMap(const MapType& aMap);

    /**
     * get the unique keys of a map
     * WARNING: major shortcoming at the moment: expects keys to be of type unsigned int!
     */
    template <typename MapType>
    std::vector<unsigned int> getUniqueKeys(const MapType& aMap);

    /**
     * get the number of unique keys inside the map
     */
    template <typename MapType>
    unsigned int getUniqueSize(const MapType& aMap) { return getUniqueKeys<MapType>(aMap).size(); }

    /**
     * get the unique keys of a map together with the number of values associated to each key. first elements are keys, second are number of values
     * WARNING: major shortcoming at the moment: expects keys to be of type unsigned int!
     */
    template <typename MapType>
    std::vector<std::pair<unsigned int, unsigned int> > getNValuesPerKey(const MapType& aMap);

    /**
     * get all values stored in the map for a given keys
     * WARNING: major shortcoming at the moment: expects keys to be of type unsigned int and values to be of type float
     */
    template <typename MapType>
    std::vector<float> getValuesToKey(const MapType& aMap, unsigned int aKey);

    /**
     * calculate the mean value of all values in vector<T>
     * NOTE: uses std::accumulate so T has to be compatible with it!
     */
    template <typename T>
    T calculateMean(std::vector<T> V) {
      T sum = std::accumulate(V.begin(), V.end(), 0.0);
      return sum / V.size();
    }

    /** tuple containing the indices and the weights of all unique TrueHits related to a SpacePoint as well as the information if all TrueHits are shared by all Clusters of the SpacePoint */
    typedef std::tuple<std::vector<unsigned int>, std::vector<float>, bool> TrueHitsInfo;

    /**
     * get the unique indices of all TrueHits that are related to a SpacePoint (done via their Clusters). The boolean entry of TrueHitsInfo is set to true if a SpacePoint with only one Cluster is passed!
     * NOTE: using RelationArray instead of the RelationInterface to be independent of different types of PXDTrueHit and SVDTrueHit (Possible to get the related Clusters from the passed SpacePoint with a template parameter, but not to get the related TrueHit from that Cluster then with another template parameter)
     */
    template<typename ClusterType, typename TrueHitType>
    TrueHitsInfo getUniqueTrueHitsInfo(Belle2::SpacePoint* spacePoint);//, RelationArray& clusters2TrueHits);

    BELLE2_DEFINE_EXCEPTION(NoTrueHitToCluster, "Found no related TrueHit for a Cluster!"); /**< Exception for when no related TrueHit can be found for a Cluster */
  };
}