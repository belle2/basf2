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

// root:
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TChain.h>
//boost:
// #ifndef __CINT__
#include <boost/chrono.hpp>
// #endif

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


    /// cross-check if everything is working as expected. WARNING TODO find out if this does fulfill its purpose!
    template <class FilterType> void testSegmentFilters(const SectorMapConfig& config, SectorGraph<FilterType>& mainGraph,
                                                        VXDTFFilters<SpacePoint>& segFilters)
    {
      B2DEBUG(1, "testSegmentFilters - now testing config: " << config.secMapName);
#include <vxd/geometry/SensorInfoBase.h>
      // store the dummy sensors and clusters:
      std::vector<VXD::SensorInfoBase> allSensors;
      std::vector<Belle2::PXDCluster> allPXDClusters;

      /// small lambda for creating a SpacePoint with given global coordinates:
      auto getSpacePoint = [&](VxdID aVxdID, double globalX, double globalY, double globalZ) -> SpacePoint {
        VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 1., 1., 0.3, 2, 4, -1.);
        TGeoRotation r1; r1.SetAngles(45, 20, 30);
        TGeoTranslation t1(globalX, globalY, globalZ);
        TGeoCombiTrans c1(t1, r1);
        TGeoHMatrix transform = c1;
        sensorInfoBase.setTransformation(transform);
        allSensors.push_back(sensorInfoBase);
        PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
        allPXDClusters.push_back(aCluster);

        return SpacePoint(&(allPXDClusters.back()), &(allSensors.back()));
      };

      /// small lambda for testing a distance-based filter:
      auto testDistanceFilter = [&](std::vector<FullSecID> secIDs, std::string fName, std::string cutName, double cut, bool doX, bool doY,
      bool doZ) -> bool {
        double divider = 0.;
        if (doX) divider++;
        if (doY) divider++;
        if (doZ) divider++;
        if (divider == 0.) B2FATAL("RawSecMapMerger::testSegmentFilters::testDistanceFilter: was used in an illegal way!");
        double outerX = 1., outerY = 2., outerZ = 3.;
        if (doX) outerX += cut / divider;
        if (doY) outerY += cut / divider;
        if (doZ) outerZ += cut / divider;
        SpacePoint outer = getSpacePoint(secIDs.at(0).getVxdID(), outerX, outerY, outerZ);
        SpacePoint inner = getSpacePoint(secIDs.at(1).getVxdID(), 1., 2., 3.);
        if (segFilters.getCompactID(secIDs.at(0)) != 0 and
        segFilters.getTwoHitFilters(secIDs.at(0), secIDs.at(1)).accept(outer, inner))
        {
          B2DEBUG(1, "testSegmentFilters-" << fName << ": for secCombi o/i: " << secIDs.at(0).getFullSecString() << "/" << secIDs.at(
            1).getFullSecString() << " test " << cutName << "/" << cut << " was working fine!");
          return true;
        }
        B2DEBUG(1, "testSegmentFilters-" << fName <<
        ": forsecCombi o/i: " << secIDs.at(0).getFullSecString() <<
        "/" << secIDs.at(1).getFullSecString() <<
        " test " << cutName <<
        "/" << cut <<
        " was _not_ working! " <<
        (segFilters.getCompactID(secIDs.at(0)) == 0 ? "OuterSector was _not_ in map!" : ""));
        return false;
      };

      for (auto& entry : mainGraph) {
        SubGraph<FilterType>& subGraph = entry.second;
        const auto& filterCutsMap = subGraph.getFinalQuantileValues();
        auto secIDs = subGraph.getID().getFullSecIDs();

        {
          // Distance3DSquared:
          double min = filterCutsMap.at("Distance3DSquared").getMin();
          testDistanceFilter(secIDs, "Distance3DSquared", "min", min, true, true, true);
          double max = filterCutsMap.at("Distance3DSquared").getMax();
          testDistanceFilter(secIDs, "Distance3DSquared", "max", max, true, true, true);
        }

        // JKL JAN 2016: commented out for minimal working example - test
//         {
//           // Distance2DXYSquared:
//           double min = filterCutsMap.at("Distance2DXYSquared").getMin();
//           testDistanceFilter(secIDs, "Distance2DXYSquared", "min", min, true, true, false);
//           double max = filterCutsMap.at("Distance2DXYSquared").getMax();
//           testDistanceFilter(secIDs, "Distance2DXYSquared", "max", max, true, true, false);
//         }
//
//         {
//           // Distance1DZ:
//           double min = filterCutsMap.at("Distance3DSquared").getMin();
//           testDistanceFilter(secIDs, "Distance3DSquared", "min", min, false, false, true);
//           double max = filterCutsMap.at("Distance3DSquared").getMax();
//           testDistanceFilter(secIDs, "Distance3DSquared", "max", max, false, false, true);
//         }

        {
          // SlopeRZ: WARNING: TODO  write a functioning test for that (b4: fix test-sample to have slopeRZ stored too)
//      double min = filterCutsMap.at("Distance3DSquared").getMin();
//      testFilter(secIDs, "Distance3DSquared", "min", min, false, false, false);
//      double max = filterCutsMap.at("Distance3DSquared").getMax();
//      testFilter(secIDs, "Distance3DSquared", "max", max, false, false, false);
        }

        {
          // Distance3DNormed: WARNING: TODO  write a functioning test for that (b4: fix test-sample to have slopeRZ stored too)
//      double max = filterCutsMap.at("Distance3DNormed").getMax();
//      testFilter(secIDs, "Distance3DNormed", "max", max, false, false, false);
        }
      }
    }



    /// WARNING TODO: Filter-> String to big->unsigned is better (or FilterID)
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

      // TODO this is not yet capable of dealing with other than twoHitFilters. -> generalize!
      getSegmentFilters(config, mainGraph, xHitFilters, secChainLength);

      if (xHitFilters->size() == 0) {
        // thou shall not delete the filters!
        // delete xHitFilters;
        B2FATAL("processSectorCombinations: an empty VXDTFFilters was returned, training data did not work!");
      }

      // some testing:
      if (secChainLength == 2) {
        testSegmentFilters(config, mainGraph, *xHitFilters);
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

        return; // TODO WARNING DEBUG we do not want to run more than one run yet!

        B2INFO("\n\nRawSecMapMerger::initialize(): for mapName " << config.secMapName << ": process 4-hit-combinations:\n\n");
        B2INFO(" assigning no filters to the FiltersContainer: ");
        processSectorCombinations(config, xHitFilters, 4);

      }
    }

  };

}
