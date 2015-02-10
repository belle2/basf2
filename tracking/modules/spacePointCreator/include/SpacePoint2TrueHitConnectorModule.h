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

#include <unordered_map> // needef for typedef of defaultMap

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

    typedef std::unordered_multimap<int, float> baseMapT; /**< typedef to have the same type of map throughout the module */

    /**
     * enum to distinguish the detectortypes
     */
    enum e_detTypes {
      c_PXD = Belle2::VXD::SensorInfoBase::PXD,
      c_SVD = Belle2::VXD::SensorInfoBase::SVD,
    };

    std::string m_PARAMoutputSuffix; /**< suffix that will be appended to the StoreArray names of the output StoreArrays */

    std::vector<std::string> m_PARAMtrueHitNames; /**< names of containers of TrueHits */

    std::vector<std::string> m_PARAMspacePointNames; /**< names of containers of SpacePoints */

    std::vector<std::string> m_PARAMdetectorTypes; /**< detector type names as strings to determine which name belongs to which detector type */

    bool m_PARAMstoreSeparate; /**< switch for storing the SpacePoints that can be related to a TrueHit into separate StoreArrays, where only such SpacePoints are stored */

    bool m_PARAMregisterAll; /**< switch for registereing all relations for all TrueHits for all SpacePoints (there can be more than 1 related TrueHit to each SpacePoint). The module can be used to get all TrueHits for a SpacePoint and then the user can decide what to do with it. */;

    double m_PARAMmaxGlobalDiff; /**< maximum difference of global position coordinates for each direction between TrueHit and SpacePoint */

//     double m_PARAMmaxLocalDiff; /**< maximum difference of local position coordinates for each direction between TrueHit and SpacePoint */

    double m_PARAMmaxPosSigma; /**< defining th maximum difference of local coordinates in units of PitchSize / sqrt(12) */

    unsigned int m_nContainers; /**< number of passed containers -> storing the size of an input vector for not having to obtain it every time */

    unsigned int m_nPXDarrays; /**< number of PXD StoreArrays */

    unsigned int m_nSVDarrays; /**< number os SVD StoreArrays */

    unsigned int m_nPXDSpacePointsCtr; /**< Number of PXD SpacePoints */

    unsigned int m_nSVDSpacePointsCtr; /**< Number SVD SpacePoints */

    unsigned int m_negWeightCtr; /**< number of negative weights */

    unsigned int m_totWeightsCtr; /**< total number of weights */

    unsigned int m_noTrueHitCtr; /**< Number of SpacePoints that contained a Cluster to which no TrueHit has been found (i.e. counts how many times the NoTrueHitToCluster exception gets thrown */

    unsigned int m_noClusterCtr; /**< Number of SpacePoints that contained no Cluster (i.e. counts how many times the NoClusterToTrueHit exception gets thrown) */

    unsigned int m_ghostHitCtr; /**< counter for SpacePoints that are considered ghost hits */

    unsigned int m_regRelationsCtr; /**< counter for all registered relations */

    unsigned int m_single2WTHCtr; /**< counter for SpacePoints with more than two possible TrueHits, but only one of them has two weights */

    unsigned int m_nonSingle2WTHCtr; /**< counter for SpacePoints with more than two possible TrueHits, with more than one of them having two weights */

    unsigned int m_all2WTHCtr; /**< Counter for SpacePoints where alle possible TrueHits have two weights */

    unsigned int m_accSingle2WTHCtr; /**< counter for SpacePoints with more than two possible TrueHits, but only one of them has two weights, where a relation was registered to a TrueHit */

    unsigned int m_accNonSingle2WTHCtr; /**< counter for SpacePoints with more than two possible TrueHits, with more than one of them having two weights where a relation was registered to a TrueHit */

    unsigned int m_accAll2WTHCtr; /**< Counter for SpacePoints where alle possible TrueHits have two weights, where a relation was registered to a TrueHit */

    unsigned int m_oneCluster2THCtr; /**< Counter for SpacePoints with only one Cluster but two possible TrueHits (in these cases, the one with the bigger weight gets automatically accepted) */

    std::array<unsigned int, 5> m_nRelPXDTrueHitsCtr; /**< counter for counting different numbers of related TrueHits (to a SpacePoint) with one variable */

    std::array<unsigned int, 5> m_nRelSVDTrueHitsCtr; /**< counter for counting different numbers of related TrueHits (to a SpacePoint) with one variable */

    std::vector<std::pair<Belle2::StoreArray<Belle2::SpacePoint>, e_detTypes> > m_inputSpacePoints; /**< StoreArray of all input SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint > > m_outputSpacePoints; /**< StoreArray of all output SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::PXDTrueHit> > m_PXDTrueHits; /**< StoreArrays of PXDTrueHits to which relations will get registered */

    std::vector<Belle2::StoreArray<Belle2::SVDTrueHit> > m_SVDTrueHits; /**< StoreArrays of SVDTrueHits to which relations will get registered */

    std::vector<e_detTypes> m_detectorTypes; /**< vector storing the detector types for easier access during initialization */

    std::vector<float> m_weightDiffsByAvg; /**< To investigate how the difference of weigts divided by the average of the weights is distributed -> maybe learn something from it */

    unsigned int m_moreThan2Weights; /**< Count the cases whith more than two weights */

    void initializeCounters(); /**< initialize all counters to 0 */

    // COULDDO: place the (rather) general functions below into a header to make it accessible to more than just this module

    /**
     * get the contents of the map as string. NOTE: only templated to such a degree that is actually needed in this module! should compile without warning for any map (e.g. map, multimap, unordered_map,...) with key and values of a type that have a defined stream insertion operator (only tested for multimap and unordered_multimap!)
     */
    template <typename MapType>
    std::string printMap(const MapType& aMap);

    /**
     * get the unique keys of a map
     */
    template <typename MapType>
    std::vector<typename MapType::key_type> getUniqueKeys(const MapType& aMap);

    /**
     * get the number of unique keys inside the map
     */
    template <typename MapType>
    unsigned int getUniqueSize(const MapType& aMap) { return getUniqueKeys<MapType>(aMap).size(); }

    /**
     * get the unique keys of a map together with the number of values associated to each key. first elements are keys, second are number of values
     */
    template <typename MapType>
    std::vector<std::pair<typename MapType::key_type, unsigned int> > getNValuesPerKey(const MapType& aMap);

    /**
     * get all values stored in the map for a given key
     */
    template <typename MapType>
    std::vector<typename MapType::mapped_type> getValuesToKey(const MapType& aMap, typename MapType::key_type aKey);

    /**
     * get the TrueHit from information that is stored in the map (conditions are checked in the following order):
     * +) if there is only one TrueHit in the map, return a pointer to it and as weight the sum of the weights of the relations between the Clusters and the TrueHits
     * +) if there is only one TrueHit in the map with two weights associated (and all other TrueHits have only one weight or there is no other TrueHit), return a pointer to it and the weight is again the sum of the weights in the map
     * +) if there are more than one TrueHits with two weights associated, return the one with the biggest sum of weights
     * +) if there are only TrueHits with one weight associated, return the one with the biggest weight
     * NOTE: as this method is rather specific, it is not very much templated!
     * NOTE: throws if one of the further checks that are then done fails!
     */
    template <typename MapType, typename TrueHitType>
    std::pair<TrueHitType*, double> getTHwithWeight(const MapType& aMap, Belle2::StoreArray<TrueHitType> trueHits, Belle2::SpacePoint* spacePoint);

    /**
     * compares the TrueHit and the SpacePoint positions (global) to decide whether they are compatible
     * NOTE: if a SpacePoint and a TrueHit do not 'pass' this test they are most proably not compatible, however just because this test is passed does not guarantee that the 'right' TrueHit has been chosen before calling this function!
     */
    template <typename TrueHitType>
    bool compatibleCombination(Belle2::SpacePoint* spacePoint, TrueHitType* trueHit);

    /**
     * calculate the mean value of all values in vector<T>
     * NOTE: uses std::accumulate so T has to be compatible with it!
     * NOTE: unused at the moment!
     */
    template <typename T>
    T calculateMean(std::vector<T> V) {
      T sum = std::accumulate(V.begin(), V.end(), 0.0);
      return sum / V.size();
    }

    /** get all the related TrueHits to the SpacePoint, including their weights in a map (multimap!) where the StoreArray indices of the TrueHits are the keys and the weights are the associated values
     * MapType has to have key value pairs of <int, float> !!
     * throws: + NoTrueHitToCluster
     *         + NoSpacePointToCluster
     * @param clusterName: Name of the StoreArray of Clusters to be searched (defaults to "ALL")
     * @param trueHitName: Name of the StoreArray of TrueHits to be searched (defaults to "ALL")
     */
    template<typename MapType, typename ClusterType, typename TrueHitType>
    MapType getRelatedTrueHits(Belle2::SpacePoint* spacePoint, std::string clusterName = "ALL", std::string trueHitName = "ALL");

    BELLE2_DEFINE_EXCEPTION(NoTrueHitToCluster, "Found no related TrueHit for a Cluster!"); /**< Exception for when no related TrueHit can be found for a Cluster */

    BELLE2_DEFINE_EXCEPTION(NoClusterToSpacePoint, "Found no related Cluster for a SpacePoint!"); /**< Exception for when no related Cluster can be found for a SpacePoint */

    BELLE2_DEFINE_EXCEPTION(SpacePointIsGhostHit, "The combination of related TrueHits suggest that this SpacePoint is a ghost hit!"); /**< Exception thrown, when there is no TrueHit related to both of the Clusters of a SpacePoint (can only happen in SVD) */

    std::pair<double, double> getLocalPos(Belle2::SpacePoint* spacePoint);
  };
}