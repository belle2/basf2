/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/dataobjects/SectorMapConfig.h>
#include <tracking/trackFindingVXD/sectorMapTools/MinMaxCollector.h>
#include <tracking/trackFindingVXD/filterMap/map/FiltersContainer.h>
#include <framework/datastore/StoreObjPtr.h>


#include <tracking/trackFindingVXD/sectorMapTools/BranchInterface.h>
#include <tracking/trackFindingVXD/sectorMapTools/MinMax.h>
#include <tracking/trackFindingVXD/sectorMapTools/RawDataCollectedMinMax.h>
#include <tracking/trackFindingVXD/sectorMapTools/SectorGraph.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraph.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraphID.h>
#include <tracking/spacePointCreation/SpacePoint.h>

// stl:
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility> // std::pair
#include <memory> // std::unique_ptr
#include <limits>       // std::numeric_limits
#include <functional>

#include <TChain.h>
#include <boost/chrono.hpp>

namespace Belle2 {

  /** The RawSecMapMergerModule
   *
   * this module takes a root file containing a raw sectorMap created by the SecMapTrainerBaseModule
   * and converts it to a sectormap which can be read by the SegmentNetworkProducer (VXDTF redesigned). Please check the parameters to be set...
   *
   */
  class RawSecMapMergerModule : public Module {
  protected:
    /// ///////////////////////////////////////////////////////////////////////////////// member variables of module:
    /** List of files (wildcards not allowed - use python glob.glob() to expand to list of files) */
    std::vector<std::string> m_PARAMrootFileNames;

    /** contains names of sectorMaps to be loaded. */
    std::vector<std::string> m_PARAMmapNames;

    /** If true, the full trained graphs will be printed to screen. WARNING: produces a lot of output for full detector-cases! */
    bool m_PARAMprintFullGraphs;
    // ///////////////////////////////////////////////////////////////////////////////// member variables END:
  public:

    /** Constructor of the module. */
    RawSecMapMergerModule();


    /** Destructor of the module. */
    virtual ~RawSecMapMergerModule() {}


    /** returns all names of root-files fitting given parameter mapName  */
    std::vector<std::string> getRootFiles(std::string mapName);


    /** bundle all relevant files to a TChain */
    std::unique_ptr<TChain> createTreeChain(const SectorMapConfig& configuration, std::string nHitString);


    /** for given chain and names of branches:
     * this function returns their pointers to the branch and the containing value
     * in the same order as the input vector of branch names. */
    template<class ValueType> std::vector<BranchInterface<ValueType>> getBranches(
          std::unique_ptr<TChain>& chain,
          const std::vector<std::string>& branchNames);


    /** sets everything which is hit-dependent. */
    std::string prepareNHitSpecificStuff(
      unsigned nHits,
      const SectorMapConfig& config,
      std::vector<std::string>& secBranchNames,
      std::vector<std::string>& filterBranchNames);


    /**  returns secIDs of current entry in the secBranches. */
    std::vector<unsigned> getSecIDs(std::vector<BranchInterface<unsigned>>& secBranches, Long64_t entry)
    {
      std::vector<unsigned> iDs;
      for (BranchInterface<unsigned>& branch : secBranches) {
        branch.update(entry);
        iDs.push_back(branch.value);
      }
      return iDs;
    }


    /**  fill the graphs with raw data fitting to their filters respectively. */
    template <class FilterType> void trainGraph(
      SectorGraph<FilterType>& mainGraph,
      std::unique_ptr<TChain>& chain,
      std::vector<BranchInterface<unsigned>>& sectorBranches,
      std::vector<BranchInterface<double>>& filterBranches);


    /**  build graph with secChains found in TChain. */
    template <class FilterType> SectorGraph<FilterType> buildGraph(
      std::unique_ptr<TChain>& chain,
      std::vector<BranchInterface<unsigned>>& sectorBranches,
      std::vector<BranchInterface<double>>& filterBranches);


    /** for debugging: print data for crosschecks. for small sample sizes < 100 the whole sample will be printed, for bigger ones only 100 entries will be printed. */
    void printData(
      std::unique_ptr<TChain>& chain,
      std::vector<BranchInterface<unsigned>>& sectorBranches,
      std::vector<BranchInterface<double>>& filterBranches);


    /** for debugging purposes: print VXDTFFilters into a file of name of the sectorMapConfig. */
    void printVXDTFFilters(const VXDTFFilters<SpacePoint>& filters, std::string configName,
                           unsigned int nHitCombinations, bool print2File);



    /** returns all VxdIDs (sensors) compatible with given configData. */
    // TODO: remove
    // std::vector<VxdID> getCompatibleVxdIDs(const SectorMapConfig& config);


    /// WARNING TODO clean up and documentation!
    template <class FilterType> void getSegmentFilters(
      const SectorMapConfig& config,
      SectorGraph<FilterType>& mainGraph,
      VXDTFFilters<SpacePoint>* xHitFilters,
      int nSecChainLength);


    /// WARNING TODO clean up and documentation!
    template <class FilterType> void add2HitFilters(VXDTFFilters<SpacePoint>& filterContainer,
                                                    SubGraph<FilterType>& subGraph, const SectorMapConfig&  config);


    /// WARNING TODO clean up and documentation!
    template <class FilterType> void add3HitFilters(VXDTFFilters<SpacePoint>& filterContainer,
                                                    SubGraph<FilterType>& subGraph, const SectorMapConfig&  config);


    /// WARNING TODO clean up and documentation!
    template <class FilterType> void add4HitFilters(VXDTFFilters<SpacePoint>& filterContainer,
                                                    SubGraph<FilterType>& subGraph, const SectorMapConfig&  config);


    /// updates the sublayer ID of the FullSecIDs used in the VXDTFFilters with the one used during the training contained in the SectorGraph
    /// @param mainGrapgh : the graph from which the updated FullSecIDs are retrieved
    /// @param segFilters : the filters which need to be updated
    template <class FilterType> unsigned updateFilterSubLayerIDs(SectorGraph<FilterType>& mainGraph,
        VXDTFFilters<SpacePoint>& segFilters);


    /** does everything needed for given chainLength of sectors (e.g.: 2 -> twoHitFilters)*/
    void processSectorCombinations(const SectorMapConfig& config, VXDTFFilters<SpacePoint>* xHitFilters,
                                   unsigned secChainLength)
    {
      B2INFO("processSectorCombinations: training map " << config.secMapName << " with secChainLength: " << secChainLength);

      // branch-names sorted from outer to inner:
      std::vector<std::string> secBranchNames;
      // filter names are different for different secChainLengths:
      std::vector<std::string> filterBranchNames;
      std::string nHit = prepareNHitSpecificStuff(secChainLength, config, secBranchNames, filterBranchNames);

      // contains the raw data
      std::unique_ptr<TChain> chain = createTreeChain(config, nHit);
      if (chain->GetEntries() == 0) {
        B2WARNING("raw data for map " << config.secMapName << " with " << nHit << " is empty! skipping");
        return;
      }

      // prepare links to branches branches:
      std::vector<BranchInterface<unsigned>> sectorBranches = getBranches<unsigned>(chain, secBranchNames);
      std::vector<BranchInterface<double>> filterBranches = getBranches<double>(chain, filterBranchNames);

      // for debugging: print data for crosschecks:
      printData(chain, sectorBranches, filterBranches);

      // create graph containing all sector-chains occured in the training sample
      SectorGraph<std::string> mainGraph = buildGraph<std::string>(chain, sectorBranches, filterBranches);

      // use rareness-threshold to find sector-combinations which are very rare and remove them:
      unsigned nKilled = mainGraph.pruneGraph(config.rarenessThreshold);

      B2INFO("processSectorCombinations: nKilled after graph-pruning: " << nKilled);

      // get the raw data and determine the cuts for the filters/selectionVariable
      for (auto& subgraph : mainGraph) {
        subgraph.second.prepareDataCollection(config.quantiles); // TODO small-sample-case!
      }

      trainGraph(mainGraph, chain, sectorBranches, filterBranches);
      /// TODO next steps: implement nice and neat way to take care of the ram-threshold!
      /**
       * 404 bytes per graph max (no trainer-values) + 3 (id in mainGraph)
       * 100.000 graphs max
       * -> 40.700.000 ~ 40 MB for empty mainGraph containing 100k subgraphs.
       * per subgraph space available: (2GB - 40 MB) / 100.000 = 20 MB per graph. (double)-> 2.500.000 values max. / 20 Filters
       * 125k value-sets max.
       * */

      // checks for sectors which have inner neighbour and updates the sublayerID of the sensors.
      // TODO: check if all FullSecIDs are updated and not only those in the corresponding graphs as this may cause problems in a later step
      mainGraph.updateSubLayerIDs();

      B2INFO("processSectorCombinations: training finished.\n" << mainGraph.print(m_PARAMprintFullGraphs););

      getSegmentFilters(config, mainGraph, xHitFilters, secChainLength);

      if (xHitFilters->size() == 0) {
        // thou shall not delete the filters!
        // delete xHitFilters;
        B2FATAL("processSectorCombinations: an empty VXDTFFilters was returned, training data did not work!");
      }

      return;

    }



    /** Initializes the Module.
     */
    virtual void initialize()
    {
      FiltersContainer<SpacePoint>& filtersContainer = Belle2::FiltersContainer<SpacePoint>::getInstance();
      B2INFO("RawSecMapMerger::initialize():");

      // loop over all the setups in the filtersContainer:
      for (auto& setup : filtersContainer.getAllSetups()) {

        // TODO: remove the config from all the following functions as it is contained in the filters!
        auto config = setup.second->getConfig();
        B2INFO("RawSecMapMerger::initialize(): loading mapName: " << config.secMapName);

        VXDTFFilters<SpacePoint>* xHitFilters = setup.second;

        B2INFO("\n\nRawSecMapMerger::initialize(): for mapName " << config.secMapName << ": process 2-hit-combinations:\n\n");
        processSectorCombinations(config, xHitFilters, 2);

        // for debugging:
        printVXDTFFilters(*xHitFilters, config.secMapName, 2, true);


        // catching case of empty xHitFilters:
        // TODO: check if that causes problems as these are the filters from bootstrapping
        //       this statement is useless as the filters from bootstrapping have already sectors added so the size is !=0
        if (xHitFilters->size() == 0) {
          B2FATAL("This should not happen!");
        }
        B2INFO("\n\nRawSecMapMerger::initialize(): for mapName " << config.secMapName << ": process 3-hit-combinations:\n\n");
        processSectorCombinations(config, xHitFilters, 3);

        /* // return; // TODO WARNING DEBUG we do not want to run more than one run yet! */

      }
    }

    /** check that the vector of FullSecIDs @param ids is meaningfull for us.
     * if @param ids size == 2 it checks that the two sectors are not on the same
     * layer and ladder
     * if @param ids size == 3 it checks that both segments satisfy the same requirement
     */
    bool good(const std::vector< unsigned>& ids);
  };

}
