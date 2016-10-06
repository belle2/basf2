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

#include <tracking/trackFindingVXD/sectorMap/filterFramework/Shortcuts.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/dataobjects/SectorMapConfig.h>
#include <tracking/trackFindingVXD/sectorMapTools/MinMaxCollector.h>
#include <tracking/trackFindingVXD/sectorMap/map/SectorMap.h>
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
    std::vector<VxdID> getCompatibleVxdIDs(const SectorMapConfig& config);


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


    /// WARNING TODO clean up and documentation!
    template <class FilterType> unsigned addAllSectorsToSecMapThingy(const SectorMapConfig& config, SectorGraph<FilterType>& mainGraph,
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
      if (chain->GetEntries() == 0) { B2WARNING("raw data for map " << config.secMapName << " with " << nHit << " is empty! skipping"); return; }

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
      mainGraph.updateSubLayerIDs();

      B2INFO("processSectorCombinations: training finished.\n" << mainGraph.print(m_PARAMprintFullGraphs););

      // TODO this is not yet capable of dealing with other than twoHitFilters. -> generalize!
      getSegmentFilters(config, mainGraph, xHitFilters, secChainLength);

      if (xHitFilters->size() == 0) {
        delete xHitFilters;
        B2FATAL("processSectorCombinations: an empty VXDTFFilters was returned, training data did not work!");
      }

      // some testing:
      if (secChainLength == 2) {
        testSegmentFilters(config, mainGraph, *xHitFilters);
      }

      return;

//    // get all sensors relevant for this secMap:
//    std::vector<VxdID> vxdIDs = getCompatibleVxdIDs(config);
//
//    VXDTFFilters segmentFilters = VXDTFFilters();
//    std::vector< std::vector< FullSecID>> allSecIDsOfThisSensor; // refilled for each sensor.
//
//    // WARNING temporal solution until we agree how to define sectorDividers:
//    std::vector< double > uDividersMinusLastOne = config.uSectorDivider;
//    uDividersMinusLastOne.pop_back();
//    std::vector< double > vDividersMinusLastOne = config.vSectorDivider;
//    vDividersMinusLastOne.pop_back();
//
//    // collect all secIDs occured in training and add all sectors for this sensor to the VXDTFFilters-thingy (those which were not found are filled with FullSecIDs too, but no cuts will exist for them):
//    for (VxdID sensor : vxdIDs) {
//    allSecIDsOfThisSensor.clear();
//
//    std::vector< FullSecID> allTrainedSecIDsOfSensor = mainGraph.getAllFullSecIDsOfSensor(sensor);
//
//    std::sort(allTrainedSecIDsOfSensor.begin(), allTrainedSecIDsOfSensor.end());
//    allTrainedSecIDsOfSensor.erase(
//      std::unique(
//      allTrainedSecIDsOfSensor.begin(),
//      allTrainedSecIDsOfSensor.end() ),
//      allTrainedSecIDsOfSensor.end() );
//
//    // lambda for finding the correct sector-position
//    auto findSector = [] (std::vector< FullSecID>& secIDs, int counter)
//    {
//      std::vector< FullSecID>::iterator iter = secIDs.begin();
//      for( ; iter != secIDs.end(); ++iter) {
//      if (iter->getSecID() == counter) return iter;
//      }
//      return secIDs.end();
//    };
//
//
//    std::vector< std::vector< FullSecID > > sectors;
//
//    //   sectors.resize(uSup.size() + 1);
//    sectors.resize(config.uSectorDivider.size());
//    unsigned nSectorsInU = config.uSectorDivider.size(),
//    nSectorsInV = config.vSectorDivider.size();
//
//
//    int counter = 0;
//    for (unsigned int i = 0; i < nSectorsInU; i++) {
//      allSecIDsOfThisSensor.push_back({});
//      for (unsigned int j = 0; j < nSectorsInV ; j++) {
//      auto pos = findSector(allTrainedSecIDsOfSensor, counter);
//      if (pos == allTrainedSecIDsOfSensor.end()) {
//        allSecIDsOfThisSensor.at(i).push_back(FullSecID(sensor, false, counter));
//      } else { allSecIDsOfThisSensor.at(i).push_back(*pos); }
//      counter ++;
//      }
//    }
//    segmentFilters.addSectorsOnSensor(uDividersMinusLastOne , vDividersMinusLastOne, allSecIDsOfThisSensor) ;
//
//
// // // //     int counter = 0;
// // // //     unsigned nUCuts = config.uDirectionCuts.size(), nVCuts = config.vDirectionCuts.size();
// // // //     // add all secIDs for this sensor found during training and fill standard ones for the rest.
// // // //     for (unsigned i = 0; i < nUCuts; i++) {
// // // // //      allSecIDsOfThisSensor[i].resize(nVCuts + 1);
// // // //       for (unsigned j = 0; j < nVCuts; j++) {
// // // //       auto pos = findSector(allTrainedSecIDsOfSensor, counter);
// // // //       if (pos == allTrainedSecIDsOfSensor.end()) {
// // // //         allSecIDsOfThisSensor.at(i).push_back(FullSecID(sensor, false, counter));
// // // //       } else { allSecIDsOfThisSensor.at(i).push_back(*pos); }
// // // //       counter ++;
// // // //       }
// // // //     }
// // // //     segmentFilters.addSectorsOnSensor(config.uDirectionCuts , config.vDirectionCuts, allSecIDsOfThisSensor);
//
//    B2DEBUG(1, "Sensor: " << sensor << " had " << allTrainedSecIDsOfSensor.size() << " trained IDs and " << counter << " sectors in Total")
//    } // end loop sensor of vxdIDs.
//
//    // and add the virtual IP:
//    std::vector<double> uCuts4vIP = {}, vCuts4vIP = {};
//    allSecIDsOfThisSensor.clear();
//    allSecIDsOfThisSensor = {{0}};
//    segmentFilters.addSectorsOnSensor(uCuts4vIP, vCuts4vIP, allSecIDsOfThisSensor);
//
//       /**
//        * Here the data will be loaded in the new secMap-Design:
//        *
//        * // start creating new secMap:
//        * VXDTFFilters allFilters;
//
//        *  have to be added in one batch per sensor (sensor ordering not important, but sector-on-sensor-ordering is
//        * for (layer, ladder, sensor in config.allowedLayers) {
//
//       *    auto thisSensorID = VxdID(layer, ladder , sensor);
//       *   // get all sectors seen in Training of this sensor -> they have got the subLayerID! (but may be incomplete as a set)
//       *   std::vector< FullSecID> allTrainedSecIDsOfThisSensor;
//
//       *   protoMap.getAllFullSecIDsOfSensor(thisSensor, allTrainedSecIDsOfThisSensor);
//
//       *   //create full vector of FullSecIDs of this sensor -> if TrainedID already exists, that one gets stored.
//       *   std::vector< std::vector< FullSecID>> allSecIDsOfThisSensor;
//       *   allSecIDsOfThisSensor.resize(config.uDirectionCuts(layer).size() + 1);
//       *   int counter = 0;
//       *   for (unsigned int i = 0; i < config.uDirectionCuts(layer).size() + 1; i++) {
//       *   allSecIDsOfThisSensor[i].resize(config.vDirectionCuts(layer).size() + 1);
//       *   for (unsigned int j = 0; j < config.vDirectionCuts(layer).size() + 1 ; j++) {
//       *   auto tempID = FullSecID(thisSensorID, false, counter);
//
//       *   bool found = false;
//       *   for (FullSecID& trainedID : allTrainedSecIDsOfThisSensor) {
//       *   if (tempID.getVxdID() == trainedID.getVxdID()
//       *   and tempID.getSecID() == trainedID.getSecID())
//       *   {
//       *   found = true;
//       *   allSecIDsOfThisSensor[i][j] = trainedID;
//       * }
//       * }
//
//       * if (found == false) {
//       * allSecIDsOfThisSensor[i][j] = FullSecID(thisSensorID, false, counter);
//       * }
//       * counter ++;
//       * }
//       * }
//
//       * B2DEBUG(1, "Sensor: " << thisSensorID << " had " << allTrainedSecIDsOfThisSensor.size() << " trained IDs and " << counter << " sectors in Total")
//
//       * allFilters.addSectorsOnSensor(config.uDirectionCuts(layer), config.vDirectionCuts(layer), allSecIDsOfThisSensor); // TODO u/vDirectionCuts -> layerspecific!
//
//       *     // and add the virtual IP:
//       *     uSup = {};
//       *     vSup = {};
//       *     allSecIDsOfThisSensor = {{0}};
//       *     allFilters.addSectorsOnSensor(uSup, vSup, allSecIDsOfThisSensor);
//
//       *     // TODO:
//       *     // Now we need the selectionVariables. -> how to solve the issue of hardcoded filters (is it possible?)
//       * }
//       * }
//       *
//       */
//
//    const auto filterIDs = mainGraph.getFilterTypes();
//
//    for (auto& subGraph : mainGraph) {
//    // TODO tune cutoffs
//
//    auto filterCutsMap = subGraph.getFinalQuantileValues();
//
//    auto friendSectorsSegmentFilter =
//    (
//      (
//      filterCutsMap[SelectionVariableType::getTypeEnum("Distance3DSquared")].second.getMin() <
//      Distance3DSquared<SpacePoint>() <
//      filterCutsMap[SelectionVariableType::getTypeEnum("Distance3DSquared")].second.getMax()
//      ).observe(Observer()).enable(true) &&
//
//      (
//      filterCutsMap[SelectionVariableType::getTypeEnum("Distance2DXYSquared")].second.getMin() <
//      Distance2DXYSquared<SpacePoint>() <
//      filterCutsMap[SelectionVariableType::getTypeEnum("Distance2DXYSquared")].second.getMin()
//      ).observe(Observer()).enable(true) &&
//
//      (
//      filterCutsMap[SelectionVariableType::getTypeEnum("Distance1DZ")].second.getMin() <
//      Distance1DZ<SpacePoint>() <
//      filterCutsMap[SelectionVariableType::getTypeEnum("Distance1DZ")].second.getMin()
//      )/*.observe(Observer())*/.enable(true) &&
//
//
//      (
//      filterCutsMap[SelectionVariableType::getTypeEnum("SlopeRZ")].second.getMin() <
//      SlopeRZ<SpacePoint>() <
//      filterCutsMap[SelectionVariableType::getTypeEnum("SlopeRZ")].second.getMin()
//      ).observe(Observer()).enable(VariableEnable(FilterID::slopeRZ, setupIndex)) &&
//
//      (
//      Distance3DNormed<SpacePoint>() <
//      filterCutsMap[SelectionVariableType::getTypeEnum("Distance3DNormed")].second.getMax()
//      ).observe(Observer()).enable(true)
//
//    );
//
//    if (segmentFilters->addFriendsSectorFilter(innerSectorID, outerSectorID,
//                         friendSectorsSegmentFilter) == 0)
//      B2WARNING("Problem adding the friendship relation from the inner sector:" <<
//      innerSectorID << " -> " << outerSectorID << " outer sector");
//    }
//
//    return std::move(segmentFilters);
    }



    /** Initializes the Module.
     */
    virtual void initialize()
    {
      StoreObjPtr< SectorMap<SpacePoint> > sectorMap("", DataStore::c_Persistent);
      B2INFO("RawSecMapMerger::initialize():");

      // loop over all the setups in the sectorMap:
      for (auto& setup : sectorMap->getAllSetups()) {

        auto config = setup.second->getConfig();
        B2INFO("RawSecMapMerger::initialize(): loading mapName: " << config.secMapName);

        if (config.secMapName != "lowTestRedesign") { continue; } // TODO WARNING DEBUG we do not want to run more than one run yet!

        VXDTFFilters<SpacePoint>* xHitFilters = new VXDTFFilters<SpacePoint>;

        B2INFO("\n\nRawSecMapMerger::initialize(): for mapName " << config.secMapName << ": process 2-hit-combinations:\n\n");
        processSectorCombinations(config, xHitFilters, 2);

        // for debugging:
        printVXDTFFilters(*xHitFilters, config.secMapName, 2, true);


        // catching case of empty xHitFilters:
        if (xHitFilters->size() == 0) {
          delete xHitFilters;
          B2FATAL("RawSecMapMerger:initialize: after processSectorCombinations an empty VXDTFFilters was returned, training data did not work!");
        }
        B2INFO("\n\nRawSecMapMerger::initialize(): for mapName " << config.secMapName << ": process 3-hit-combinations:\n\n");
        processSectorCombinations(config, xHitFilters, 3);
        sectorMap->assignFilters(config.secMapName, xHitFilters);
        return; // TODO WARNING DEBUG we do not want to run more than one run yet!

        B2INFO("\n\nRawSecMapMerger::initialize(): for mapName " << config.secMapName << ": process 4-hit-combinations:\n\n");
        B2INFO(" assigning no filters to sectorMap: ");
        processSectorCombinations(config, xHitFilters, 4);

      }
    }





    /// following stuff is maybe yet relevant -> to be checked!
// // // //   void dummyMergerSketch()
// // // //   {
// // // //     /// WARNING ATTENTION pseudo code:
// // // //
// // // //     ///includes needed:
// // // //     #include <functional> // just for the dummy-lambdas
// // // //
// // // //     /// dummy lambda-functions explaining what we need at some point:
// // // //
// // // //
// // // //     /** loads all root-files fitting given mapName  */
// // // //     auto getRootFiles = [] (std::string mapName) { return {"fName1", "fname2", "fName3"}; };
// // // //
// // // //
// // // //     /** loads configuration for given mapName */
// // // //     auto getConfig = [] (std::string) { return SectorMapConfig(); };
// // // //
// // // //
// // // //     /** takes a relationMap and checks for each outer sector:
// // // //     *   how often did it occur (-> mainSampleSize)
// // // //     * for each inner sector:
// // // //     *   how often did it occur (-> innerSampleSize)
// // // //     * sort innerSectors by rareness (rarest one comes first):
// // // //     *   if innerSampleSize is below threshold -> remove, update mainSampleSize
// // // //     * Produces a Map of ProtoSectorGraphs which contain innerSectors
// // // //     * (ProtoSectorGraphs which are no innerSector and have no innerSectors themselves will not be stored).  */
// // // //     auto pruneMap = [] (auto threshold, auto& map) { return 23; };
// // // //
// // // //
// // // //     /** for each outerSecID:
// // // //     *  if outerSecID got innerSectors on same layer:
// // // //     *     -> subLayerID == 1 (for all cases: protoSector.setFullSecID(true/false))
// // // //     *     storeOuterSecID in container
// // // //     * for each newiD in container:
// // // //     *   for each innerSector of outerSecID:
// // // //     *     if innerSector == newiD: ->subLayerID == 1 (for all cases: protoSector.setFullSecID(true/false))
// // // //     * returns nsectorsRenamed.  */
// // // //     auto renameSubLayers = [] (auto&relationsMap) { return 5; };
// // // //
// // // //     /** parameter passed with names of secMaps (have to match the corresponding FileNames)  */
// // // //     auto secMaps = { "map1", "map2"};
// // // //
// // // //
// // // //
// // // //     for (auto& secMapName : secMaps) {
// // // //     auto config = getConfig(secMapName);
// // // //
// // // //     // bundle all relevant files to a TChain
// // // //     TChain treeChain("treeNameOftwoHitFiltersOfSecMap");
// // // //     auto fileList = getRootFiles(secMapName);
// // // //     for (auto file : fileList) { treeChain.add(file); }
// // // //
// // // //     // preparing root-stuff:
// // // //     unsigned outerID;
// // // //     TBranch *outerSecIDs = nullptr;
// // // //     treeChain.SetBranchAddress("outerSecID", &outerID, &outerSecIDs);
// // // //
// // // //     unsigned innerID;
// // // //     TBranch *innerSecIDs = nullptr;
// // // //     treeChain.SetBranchAddress("innerSecID", &innerID, &innerSecIDs);
// // // //
// // // //     RelationsMap relationsMap;
// // // //     // log all sector-combinations and determine their absolute number of appearances:
// // // //     for (int i = 0 ; treeChain.GetEntries(); i++) {
// // // //       auto thisEntry = treeChain->LoadTree(i);
// // // //
// // // //       // load and count outerSecID:
// // // //       int nBytesLoaded = outerSecIDs->GetEntry(thisEntry);
// // // //       if (nBytesLoaded < 1) { B2WARNING("outerSecIDs-entry " << i << " with localEntry " << thisEntry << " has no bytes loaded!"); continue; }
// // // //
// // // //       auto outerIdPos = relationsMap.find(outerID);
// // // //       if (outerIdPos == relationsMap.end()) {
// // // //       // found it once now, therefore setting counter for outerID to 1
// // // //       outerIdPos = relationsMap.insert( {outerID, { 1, {/*this is an empty map*/}}} );
// // // //       } else { outerIdPos->second.first += 1; }
// // // //
// // // //       // load and count innerSecID:
// // // //       nBytesLoaded = innerSecIDs->GetEntry(thisEntry);
// // // //       if (nBytesLoaded < 1) { B2WARNING("innerSecIDs-entry " << i << " with localEntry " << thisEntry << " has no bytes loaded!"); continue; }
// // // //
// // // //       auto& innerIDsMap = outerIdPos->second.second;
// // // //       auto innerIdPos = innerIDsMap.find(innerID);
// // // //       if (innerIdPos == innerIDsMap.end()) {
// // // //       // found it once now, therefore setting counter for innerID to 1
// // // //       innerIdPos = innerIDsMap.insert( {innerID, 1 } );
// // // //       } else { innerIdPos->second += 1; }
// // // //     }
// // // //
// // // //     // use rareness-threshold to find sector-combinations which are very rare and remove them:
// // // //     // Sidefact: relationsMap gets moved to reduce memory consumption:
// // // //     ProtoSectorGraphMap<MinMaxCollector> protoMap = pruneMap(config.rarenessThreshold, std::move(relationsMap));
// // // //
// // // //     // prepare branches for the filters:
// // // //     std::vector<std::pair<TBranch*, double>> filterBranches = getBranches(treeChain, config.twoHitFilters);
// // // //
// // // //     // loop over the tree to find the entries matching this outerSector:
// // // //     for (int i = 0 ; treeChain.GetEntries(); i++) {
// // // //       // collect raw data of each sector-combination and find cuts:
// // // //       for (auto& outerSector : protoMap) {
// // // //
// // // //       outerSector.prepareRawDataContainers(config.quantiles, config.twoHitFilters);
// // // //       auto thisEntry = treeChain->LoadTree(i);
// // // //       outerSecIDs->GetEntry(thisEntry);
// // // //
// // // //       if (outerID != outerSector) continue;
// // // //
// // // //       // collect raw data for each inner sector:
// // // //       for(auto& innerSector : outerSector) {
// // // //         innerSecIDs->GetEntry(thisEntry);
// // // //         if (innerID != innerSector) continue;
// // // //         for (unsigned fPos = 0; fPos < n2HitFilters; fPos++) {
// // // //         filterBranches[fPos].first->GetEntry(thisEntry);
// // // //         innerSector.addRawDataValue(
// // // //           config.twoHitFilters[fPos],
// // // //           filterBranches[fPos].second);
// // // //         }
// // // //       } // innerSector-loop
// // // //       } // outerSector-loop
// // // //     } // leaves-loop
// // // //
// // // //     // find cuts of each sector-combination:
// // // //     for (auto& outerSector : protoMap) {
// // // //       // after collecting all data, store only the cuts to keep ram consumption down:
// // // //       bool wentWell = outerSector.determineCutsAndCleanRawData();
// // // //       if (!wentWell) { B2WARNING("TODO..."); }
// // // //     }
// // // //
// // // //     // checks for sectors which have inner neighbour and updates the sublayerID of the sensors.
// // // //     auto nsectorsRenamed = renameSubLayers(protoMap);
// // // //     B2DEBUG(1, "results ... TODO")
// // // //
// // // //
// // // //     /** What do we have at this point?
// // // //      * we have all final-secIDs, we know all two-sector-combinations, all cuts are determined.
// // // //      * Now we fill this data into the new secMap (VXDTFFilters):
// // // //      */
// // // //
// // // //     // start creating new secMap:
// // // //     VXDTFFilters allFilters;
// // // //
// // // //     // have to be added in one batch per sensor (sensor ordering not important, but sector-on-sensor-ordering is
// // // //     for (layer, ladder, sensor in config.allowedLayers) { // TODO minMaxLayer in config -> change e,g {3,6} to {3,4,5,6}!
// // // // //      if (FullSecID onSensor == protoMap.renamedSectors) // consider renamed Sectors!
// // // //
// // // //       auto thisSensorID = VxdID(layer, ladder , sensor);
// // // //       // get all sectors seen in Training of this sensor -> they have got the subLayerID! (but may be incomplete as a set)
// // // //       std::vector<FullSecID> allTrainedSecIDsOfThisSensor;
// // // //
// // // //       protoMap.getAllFullSecIDsOfSensor(thisSensor, allTrainedSecIDsOfThisSensor);
// // // //
// // // //       //create full vector of FullSecIDs of this sensor -> if TrainedID already exists, that one gets stored.
// // // //       std::vector<std::vector<FullSecID>> allSecIDsOfThisSensor;
// // // //       allSecIDsOfThisSensor.resize(config.uDirectionCuts(layer).size() + 1);
// // // //       int counter = 0;
// // // //       for (unsigned int i = 0; i < config.uDirectionCuts(layer).size() + 1; i++) {
// // // //       allSecIDsOfThisSensor[i].resize(config.vDirectionCuts(layer).size() + 1);
// // // //       for (unsigned int j = 0; j < config.vDirectionCuts(layer).size() + 1 ; j++) {
// // // //         auto tempID = FullSecID(thisSensorID, false, counter);
// // // //
// // // //         bool found = false;
// // // //         for (FullSecID& trainedID : allTrainedSecIDsOfThisSensor) {
// // // //         if (tempID.getVxdID() == trainedID.getVxdID()
// // // //           and tempID.getSecID() == trainedID.getSecID())
// // // //         {
// // // //           found = true;
// // // //           allSecIDsOfThisSensor[i][j] = trainedID;
// // // //         }
// // // //         }
// // // //
// // // //         if (found == false) {
// // // //         allSecIDsOfThisSensor[i][j] = FullSecID(thisSensorID, false, counter);
// // // //         }
// // // //         counter ++;
// // // //       }
// // // //       }
// // // //
// // // //       B2DEBUG(1, "Sensor: " << thisSensorID << " had " << allTrainedSecIDsOfThisSensor.size() << " trained IDs and " << counter << " sectors in Total")
// // // //
// // // //       allFilters.addSectorsOnSensor(config.uDirectionCuts(layer), config.vDirectionCuts(layer), allSecIDsOfThisSensor); // TODO u/vDirectionCuts -> layerspecific!
// // // //
// // // //       // and add the virtual IP:
// // // //       uSup = {};
// // // //       vSup = {};
// // // //       allSecIDsOfThisSensor = {{0}};
// // // //       allFilters.addSectorsOnSensor(uSup, vSup, allSecIDsOfThisSensor);
// // // //
// // // //       // TODO:
// // // //       /** Now we need the selectionVariables.
// // // //        *  -> how to solve the issue of hardcoded filters (is it possible?)
// // // //        */
// // // //     }
// // // //     }
// // // //
// // // //
// // // //   }




  protected:

//     boostNsec m_fillStuff; /**< time consumption of the secMap-creation (initialize) */
//
//
//     std::string m_PARAMrootFileName; /**<  sets the root filename */
//     bool m_PARAMprintFinalMaps; /**<   if true, a complete list of sectors (B2INFO) and its friends (B2DEBUG-1) will be printed on screen */
//
//     bool m_PARMfilterRareCombinations; /**<   use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not. Set true if you want to filter these combinations or set false if otherwise. */
//
//     double
//     m_PARAMrarenessFilter; /**<   use this member if you want to steer whether rare sector-friend-combinations shall be filtered or not, here you can set the threshold for filter. 100% = 1. 1% = 0.01%. Example: if you choose 0.01, all friendsectors which occur less often than in 1% of all cases when main sector was used, are deleted in the friendship-relations", double(0.0)) */
//
//     std::vector<int>
//     m_PARAMsampleThreshold; /**<   exactly two entries allowed: first: minimal sample size for sector-combination, second: threshold for 'small samples' where behavior is less strict. If sampleSize is bigger than second, normal behavior is chosen */
//     std::vector<double>
//     m_PARAMsmallSampleQuantiles; /**<   behiavior of small sample sizes, exactly two entries allowed: first: lower quantile, second: upper quantile. only values between 0-1 are allowed */
//     std::vector<double>
//     m_PARAMsampleQuantiles; /**<   behiavior of normal sample sizes, exactly two entries allowed: first: lower quantile, second: upper quantile. only values between 0-1 are allowed */
//     std::vector<double>
//     m_PARAMstretchFactor; /**<   exactly two entries allowed: first: stretchFactor for small sample size for sector-combination, second: stretchFactor for normal sample size for sector-combination: WARNING if you simply want to produce wider cutoffs in the VXDTF, please use the tuning parameters there! This parameter here is only if you know what you are doing, since it changes the values in the XML-file directly */


  private:

  };

}
