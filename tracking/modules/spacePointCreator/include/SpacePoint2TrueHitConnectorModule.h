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

// root output
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  /**
   * Module that tries to register a relation between SpacePoints and TrueHits, hence making some MC Information easily accesible for other modules working with SpacePoints (e.g. CurlingTrackCandSplitter or GFTC2SPTCConverter).
   *
   * As there is no direct relation between SpacePoints and TrueHits, the relations between SpacePoints and Clusters and the relations between Clusters and TrueHits are used to relate SpacePoints and TrueHits. In a first step all (unique) TrueHits that are related to all Clusters of a SpacePoint are collected (including the weight of the relation). It is then possible that there is more than one TrueHit after this.
   * + If 'storeSeperate' is set to true a new StoreArray of SpacePoints is created for every input StoreArray of SpacePoints with the same name and the 'outputSuffix' appended to it. This can be useful to have a StoreArray of SpacePoints where only 'checked' SpacePoints are (e.g. without ghost hits) that can then be used to feed the GFTC2SPTCConverter for example. If it is set to false, the relations will get registered in the existing StoreArray.
   * + If 'registerAll' is set to true, the module simply registers a relation to all of these TrueHits. The weight of this new built relation is the sum of the weights of the relations between the Cluster(s) and the according TrueHit. In this way the decision which TrueHit to use is left to the user. NOTE: The information how many Clusters of a SpacePoint were related to a TrueHit and the weights of the single relations between Clusters and TrueHits is 'lost' (can be retrieved again if actually needed).
   * + If 'registerAll' is set to false, the module tries to find a relation to only one TrueHit:
   *  1) If there is only one TrueHit -> register relation to SpacePoint with weight being the sum of the weights of the relations between Cluster(s) of SpacePoint and TrueHit <BR>
   *  2) If there is more than one TrueHit but only one TrueHit has a relation to both Clusters of a SpacePoint -> register relation to this TrueHit (again the weight of the relation is the sum of the weights of the relation between the Clusters and the TrueHit) <BR>
   *  3) If there is more than one TrueHit with a relation to both Clusters of a SpacePoint -> register relation to the TrueHit with the largest sum of weights (of relation between Clusters and TrueHits)<BR>
   *  4) If the SpacePoint has only one Cluster (e.g. PXD) and there is more than one TrueHit -> register relation to the TrueHit with the largest weight (of relation between Cluster and TrueHit)<BR>
   *
   * NOTE: It is not guaranteed that every SpacePoint gets related to a TrueHit if 'registerAll' is set to false! E.g. 'ghost hits' should be sorted out.
   * NOTE: Choosing the TrueHit with the biggest weight (or sum of weights) if there is more than one possible TrueHit does not guarantee that the 'right' TrueHit is chosen! It is possible that in such cases no relation will get registered in the future if this proves to be a source of errors!
   *
   * NOTE: This module should be used to connect SpacePoints and TrueHits if MC information is needed afterwards (e.g. in the SPTCRefereeModule for SpacePointTrackCands) to avoid having to look up the relations to obtain these informations seperately in every Module. Furthermore some modules are no longer able to determine the related TrueHits themselves (e.g. GFTC2SPTCConverter)
   */
  class SpacePoint2TrueHitConnectorModule : public Module {

  public:

    SpacePoint2TrueHitConnectorModule(); /**< Constructor */

    virtual void initialize(); /**< initialize: initialize counters, check StoreArrays, register StoreArrays, ... */

    virtual void event(); /**< event: try to find the appropriate TrueHit to all SpacePoints */

    virtual void terminate(); /**< terminate: print some summary information */

    BELLE2_DEFINE_EXCEPTION(NoTrueHitToCluster,
                            "Found no related TrueHit for a Cluster!"); /**< Exception for when no related TrueHit can be found for a Cluster */

    BELLE2_DEFINE_EXCEPTION(NoClusterToSpacePoint,
                            "Found no related Cluster for a SpacePoint!"); /**< Exception for when no related Cluster can be found for a SpacePoint */

    /** helper struct to access root variables inside the module */
    struct RootVariables {
      std::vector<double> SpacePointULocal; /**< SpacePoint local U-position */
      std::vector<double> SpacePointVLocal; /**< SpacePoint local V-position */
      std::vector<double> SpacePointXGlobal; /**< SpacePoint global X-position */
      std::vector<double> SpacePointYGlobal; /**< SpacePoint global Y-position */
      std::vector<double> SpacePointZGlobal; /**< SpacePoint global Z-position */

      std::vector<double> TrueHitULocal; /**< TrueHit local U-position */
      std::vector<double> TrueHitVLocal; /**< TrueHit local V-position */
      std::vector<double> TrueHitXGlobal; /**< TrueHit global X-position */
      std::vector<double> TrueHitYGlobal; /**< TrueHit global Y-position */
      std::vector<double> TrueHitZGlobal; /**< TrueHit global Z-position */

      std::vector<double> WeightU; /**< weight of the relation between the U-Cluster of the SpacePoint and the TrueHit */
      std::vector<double> WeightV; /**< weight of the relation between the V-Cluster of the SpacePoint and the TrueHit */

      std::vector<unsigned short int> HitVxdID; /**< VxdID of the SpacePoint/TrueHit */
      std::vector<unsigned short int> RelationStatus; /**< different flags of the relation stored in here (see c_relationStatus) */
      std::vector<unsigned int> NRelations; /**< Number of related TrueHits to a SpacePoint */

      // TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)
      std::vector<unsigned int> ClusterSizeU;
      std::vector<unsigned int> ClusterSizeV;

      std::vector<double> SpacePointErrorU; /**< position error of SpacePoint in U direction */
      std::vector<double> SpacePointErrorV; /**< position error of SpacePoint in V direction */

      std::vector<double> SpacePointErrorX; /**< positiion error of SpacePoint in X direction (global) */
      std::vector<double> SpacePointErrorY; /**< positiion error of SpacePoint in Y direction (global) */
      std::vector<double> SpacePointErrorZ; /**< positiion error of SpacePoint in Z direction (global) */
      // TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)
    }; // end RootVariables

  protected:

    typedef std::unordered_multimap<int, double> baseMapT; /**< typedef to have the same type of map throughout the module */

    /**
     * enum to distinguish the detectortypes
     */
    enum e_detTypes {
      c_PXD = Belle2::VXD::SensorInfoBase::PXD, /**< PXD */
      c_SVD = Belle2::VXD::SensorInfoBase::SVD, /**< SVD */
    };

    /** enum for better code readability */
    enum e_relationStatus {
      c_clearHit = 1, //< bit 0: Only one TrueHit to SpacePoint
      c_ghostHit = 2, //< bit 1: ghost hit as described in this module
      c_SpacePointU = 4, //< bit 2: SpacePoint U-coordinate is set
      c_SpacePointV = 8, //< bit 3: SpacePoint V-coordinate is set
      c_primaryParticle = 16, //< bit 4: Particle related to TrueHit is primary
      c_bgParticle = 32, //< bit 5: particle related to TrueHit is background
      c_nonUniqueRelation = 64, //< bit 6: more than one TrueHit related to SpacePoint
      c_registeredRelation = 128, //<bit 7: this relation got registered
    }; // end e_relationStatus

    // ================================================== PARAMETERS ==============================================================
    std::string m_PARAMoutputSuffix; /**< suffix that will be appended to the StoreArray names of the output StoreArrays */

    std::vector<std::string> m_PARAMtrueHitNames; /**< names of containers of TrueHits */

    std::vector<std::string> m_PARAMspacePointNames; /**< names of containers of SpacePoints */

    std::vector<std::string>
    m_PARAMdetectorTypes; /**< detector type names as strings to determine which name belongs to which detector type */

    std::vector<std::string> m_PARAMclusterNames; /**< names of containers of Clusters */

    std::vector<std::string> m_PARAMrootFileName; /**< name and update status of root file */

    bool m_PARAMstoreSeparate; /**< switch for storing the SpacePoints that can be related to a TrueHit into separate StoreArrays, where only such SpacePoints are stored */

    bool m_PARAMregisterAll; /**< switch for registereing all relations for all TrueHits for all SpacePoints (there can be more than 1 related TrueHit to each SpacePoint). The module can be used to get all TrueHits for a SpacePoint and then the user can decide what to do with it. */

    bool m_PARAMpositionAnalysis; /**< switch for doing the analysis of positions of SpacePoints and TrueHits */

    bool m_PARAMrequirePrimary; /**< require the TrueHit to be related to a primary particle in order for the relation to get registered! */

    double m_PARAMmaxGlobalDiff; /**< maximum difference of global position coordinates for each direction between TrueHit and SpacePoint */

//     double m_PARAMmaxLocalDiff; /**< maximum difference of local position coordinates for each direction between TrueHit and SpacePoint */

    double m_PARAMmaxPosSigma; /**< defining th maximum difference of local coordinates in units of PitchSize / sqrt(12) */

    double m_PARAMminWeight; /**< define a minimal weight a relation between Cluster and TrueHit. Below this limit the relation will not be registered */

    // ================================================= INTERMALLY USED MEMBERS ==================================================

    unsigned int
    m_nContainers; /**< number of passed containers -> storing the size of an input vector for not having to obtain it every time */

    std::vector<e_detTypes> m_detectorTypes; /**< storing the detector types for each container in vector, needed in initialize */

    Belle2::StoreArray<Belle2::SVDTrueHit> m_SVDTrueHits; /**< SVDTrueHits StoreArray used throughout the module */

    Belle2::StoreArray<Belle2::PXDTrueHit> m_PXDTrueHits; /**< PXDTrueHits StoreArray used throughout the module */

    Belle2::StoreArray<Belle2::PXDCluster> m_PXDClusters; /**< PXDTClusters StoreArray used throughout the module */

    Belle2::StoreArray<Belle2::SVDCluster> m_SVDClusters; /**< PXDClusters StoreArray used throughout the module */

    std::vector<std::pair<Belle2::StoreArray<Belle2::SpacePoint>, e_detTypes> >
    m_inputSpacePoints; /**< StoreArray of all input SpacePoints */

    std::vector<Belle2::StoreArray<Belle2::SpacePoint> > m_outputSpacePoints; /**< StoreArray of all output SpacePoints */

    double m_maxGlobalDiff; /**< storing the squared value of m_PARAMmaxGlobalDiff here to not alter the parameter input */

    /** 'helper variable' needed to not have to pass one integer down to processSpacePoint only to have a handle in cases where an exception gets caught */
    unsigned int m_iCont;

    RootVariables m_rootVariables; /**< Root variables used for collecting data eventwise */

    TFile* m_rootFilePtr; /**< pointer to root file */

    TTree* m_treePtr; /**< pointer to tree in root file */


    // ================================================= COUNTERS =================================================================
    std::vector<unsigned int> m_SpacePointsCtr; /**< Number of SpacePoints presented to the module */

    std::vector<std::array<unsigned int, 5> >
    m_nRelTrueHitsCtr; /**< counting different numbers of related TrueHits (to a SpacePoint) with one variable */

    /** Number of SpacePoints without relation to a Cluster (i.e. counts how many times the NoClusterTrueHit exception gets thrown) */
    std::vector<unsigned int> m_noClusterCtr;

    std::vector<unsigned int> m_regRelationsCtr; /**< Number of registered relations */

    std::vector<unsigned int> m_ghostHitCtr; /**< Number of SpacePoints that are considered ghost hits */

    /** Number of SpacePoints that contained a Cluster to which no TrueHit could be found (i.e. counts how many times the NoTrueHitToCluster exception gets thrown) */
    std::vector<unsigned int> m_noTrueHitCtr;

    /** Number of SpacePoints that were not related to a TrueHit (i.e. getTHwithWeight returned NULL) */
    std::vector<unsigned int> m_rejectedRelsCtr;

    unsigned int m_weightTooSmallCtr; /**< Count the omitted relations because of a too small weight */

    unsigned int m_rejectedNoPrimaryCtr; /**< Count how many times a relation was rejected because TrueHit was not related to primary */

    // TODO: make these counters for every container not only for all together!
//     unsigned int m_negWeightCtr; /**< number of negative weights */

//     unsigned int m_totWeightsCtr; /**< total number of weights */

//     unsigned int m_single2WTHCtr; /**< counter for SpacePoints with more than two possible TrueHits, but only one of them has two weights */
//
//     unsigned int m_nonSingle2WTHCtr; /**< counter for SpacePoints with more than two possible TrueHits, with more than one of them having two weights */
//
//     unsigned int m_all2WTHCtr; /**< Counter for SpacePoints where alle possible TrueHits have two weights */
//
//     unsigned int m_accSingle2WTHCtr; /**< counter for SpacePoints with more than two possible TrueHits, but only one of them has two weights, where a relation was registered to a TrueHit */
//
//     unsigned int m_accNonSingle2WTHCtr; /**< counter for SpacePoints with more than two possible TrueHits, with more than one of them having two weights where a relation was registered to a TrueHit */
//
//     unsigned int m_accAll2WTHCtr; /**< Counter for SpacePoints where alle possible TrueHits have two weights, where a relation was registered to a TrueHit */
//
//     unsigned int m_oneCluster2THCtr; /**< Counter for SpacePoints with only one Cluster but two possible TrueHits (in these cases, the one with the bigger weight gets automatically accepted) */
//
//     unsigned int m_moreThan2Weights; /**< Count the cases whith more than two weights */
//

    void initializeCounters(); /**< initialize all counters to 0 */

    /**
     * get the TrueHit from information that is stored in the map (conditions are checked in the following order):
     * + if there is only one TrueHit in the map, return a pointer to it and as weight the sum of the weights of the relations between the Clusters and the TrueHits
     * + if there is only one TrueHit in the map with two weights associated (and all other TrueHits have only one weight or there is no other TrueHit), return a pointer to it and the weight is again the sum of the weights in the map
     * + if there are more than one TrueHits with two weights associated, return the one with the biggest sum of weights
     * + if there are only TrueHits with one weight associated, return the one with the biggest weight ONLY if the SpacePoint is related to only one Cluster (e.g. PXD)
     * NOTE: as this method is rather specific, it is not very much templated!
     * NOTE: the possible return of a NULL pointer has to be handled!
     */
    template <typename MapType, typename TrueHitType>
    std::pair<TrueHitType*, double> getTHwithWeight(const MapType& aMap, Belle2::StoreArray<TrueHitType> trueHits,
                                                    Belle2::SpacePoint* spacePoint, e_detTypes detType);

    /**
     * compares the TrueHit and the SpacePoint positions (global) to decide whether they are compatible
     * NOTE: if a SpacePoint and a TrueHit do not 'pass' this test they are most proably not compatible, however just because this test is passed does not guarantee that the 'right' TrueHit has been chosen before calling this function!
     */
    template <typename TrueHitType>
    bool compatibleCombination(Belle2::SpacePoint* spacePoint, TrueHitType* trueHit);

    /** get all the related TrueHits to the SpacePoint, including their weights in a map (multimap!) where the StoreArray indices of the TrueHits are the keys and the weights are the associated values
     * MapType has to have key value pairs of <int, float> !!
     * throws: + NoTrueHitToCluster
     *         + NoSpacePointToCluster
     * @param clusterName: Name of the StoreArray of Clusters to be searched (defaults to "ALL")
     * @param trueHitName: Name of the StoreArray of TrueHits to be searched (defaults to "ALL")
     */
    template<typename MapType, typename ClusterType, typename TrueHitType>
    MapType getRelatedTrueHits(Belle2::SpacePoint* spacePoint, std::string clusterName = "ALL", std::string trueHitName = "ALL");

    /**
     * process a SpacePoint. This is essentially a wrapper that handles the different types of Clusters and TrueHits and then calls getRelatedTrueHits(args) appropriately and returns the map that is created by that
     * NOTE: could declare as const (after developement)
     * @returns: empty map if something went wrong (i.e. exception), or map of TrueHit indices to weights if all was good
     */
    template<typename MapType>
    MapType processSpacePoint(Belle2::SpacePoint* spacePoint, e_detTypes detType);

    /**
     * register a Relation to all the TrueHits in the trueHitMap for the passed SpacePoint
     */
    template<typename MapType>
    void registerAllRelations(Belle2::SpacePoint* spacePoint, MapType trueHitMap, e_detTypes detType);

    /**
     * register Relation between SpacePoint and TrueHit (and if neccessary also between SpacePoint and Cluster)
     */
    template<typename TrueHitType>
    void registerOneRelation(Belle2::SpacePoint* spacePoint, std::pair<TrueHitType*, double> trueHitwWeight, e_detTypes);

    /**
     * register all the relations to Clusters that origSpacePoint had for newSpacePoint
     */
    template<typename ClusterType>
    void
    reRegisterClusterRelations(Belle2::SpacePoint* origSpacePoint, Belle2::SpacePoint* newSpacePoint, std::string clusterName = "ALL");

    /**
     * get the pointers to the related Clusters and the weight of the original Relations between the spacePoint and these Clusters
     */
    template<typename ClusterType>
    std::vector<std::pair<ClusterType*, double> >
    getRelatedClusters(Belle2::SpacePoint* spacePoint, std::string clusterName = "ALL");

    /**
     * register the relation between a SpacePoint and the TrueHit (passed by index in the correspoinding TrueHit Array)
     * with the passed weight
     * NOTE: defining a function here mainly to avoid having duplicate code for every TrueHitType
     */
    template<typename TrueHitType>
    void registerTrueHitRelation(Belle2::SpacePoint* spacePoint, int index, double weight, Belle2::StoreArray<TrueHitType> trueHits);

    /**
     * Analyze the position of SpacePoints and corresponding TrueHits
     * @param spacePoint pointer to the SpacePoint
     * @param trueHitMap map with the information to the (possibly) related TrueHits
     * @param index (StoreArray) index of the TrueHit to which the relation actually got registered (if < 0 it is assumed that all possible relations were registered)
     * @param detType the detector type
     */
    template<typename MapType>
    void positionAnalysis(Belle2::SpacePoint* spacePoint, MapType trueHitMap, int index, e_detTypes detType);

    /** increase the appropriate counter to the exception */
    void increaseExceptionCounter(std::runtime_error& exception);

    /** get the local position of a SpacePoint */
    std::pair<double, double> getLocalPos(Belle2::SpacePoint* spacePoint);

    /** get the local (.first) and global (.second) position of a TrueHit (passed by index) */
    template<typename TrueHitType>
    std::pair<TVector3, TVector3> getTrueHitPositions(TrueHitType* trueHit);

    /** initialize the root file that is used for output */
    void initializeRootFile();

    /** close root file */
    void closeRootFile();

    // TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)
    /** get the sizes of the related Clusters of a SpacePoint
     * @returns .first is U cluster size, .second is V cluster size
     */
    std::pair<unsigned short int, unsigned short int> getClusterSizes(Belle2::SpacePoint* spacePoint, e_detTypes detType);

    /** get the position error of SpacePoints in local coordinates
     * @retuns .first is U position error, .second is V position error
     */
    std::pair<double, double> getLocalError(Belle2::SpacePoint* spacePoint);
    // TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for testing at the moment (i.e. do not commit)
  }; // end module

  /** helper class for setting up a bitfield that can be used to store several flags in one variable
  * TODO: move this from this header (possibly to some helperStuff header) and make some tests!
  */
  template<typename T>
  class simpleBitfield {

  public:
    simpleBitfield() { __bits = T(); } /**< default constructor */
    simpleBitfield(const simpleBitfield<T>& __otherBitfield) { __bits = __otherBitfield.__bits; } /**< constructor from other bitfield */

    /** check if a certain status has been set to the bitfield */
    const T hasStatus(T __statusBits) const { return (__bits & __statusBits) == __statusBits; }
    /** get the status of the bitfield */
    const T getStatus() const { return __bits; }
    /** set the status of the bitfield (CAUTION: overwrites any previously defined status!) */
    void setStatus(T __statusBits) { __bits = __statusBits; }
    /** add a status to the bitfield (if it has not already been added) */
    void addStatus(T __statusBits) { __bits |= __statusBits; }
    /** remove a status from the bitfield */
    void removeStatus(T __statusBits) { __bits &= __statusBits; }
    /** reset bitfield */
    void clear() { __bits = T(); }

  private:
    T __bits; /**< member holding the bits that are maniuplated */
  }; // end class bitfield
} // end namespace Belle2