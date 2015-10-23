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

#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/trackFindingVXD/sectorMapTools/TrainerConfigData.h>
#include <tracking/trackFindingVXD/sectorMapTools/MinMaxCollector.h>
#include <tracking/trackFindingVXD/sectorMapTools/SectorMap.h>
#include <framework/datastore/StoreObjPtr.h>


#include <tracking/trackFindingVXD/sectorMapTools/BranchInterface.h>
#include <tracking/trackFindingVXD/sectorMapTools/MinMax.h>
#include <tracking/trackFindingVXD/sectorMapTools/RawDataCollectedMinMax.h>
#include <tracking/trackFindingVXD/sectorMapTools/SectorGraph.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraph.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraphID.h>

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
   * and converts it to a sectormap which can be read by the VXDTF. Please check the parameters to be set...
   *
   */
  class RawSecMapMergerModule : public Module {
  protected:
    /// ///////////////////////////////////////////////////////////////////////////////// member variables of module:
    /** List of files (wildcards not allowed - use python glob.glob() to expand to list of files) */
    std::vector<std::string> m_PARAMrootFileNames;

    /** contains names of sectorMaps to be loaded. */
    std::vector<std::string> m_PARAMmapNames;

    // ///////////////////////////////////////////////////////////////////////////////// member variables END:
  public:

    /** Constructor of the module. */
    RawSecMapMergerModule();


    /** Destructor of the module. */
    virtual ~RawSecMapMergerModule() {}


    /** returns all names of root-files fitting given parameter mapName  */
    std::vector<std::string> getRootFiles(std::string mapName);


    /** bundle all relevant files to a TChain */
    std::unique_ptr<TChain> createTreeChain(TrainerConfigData& configuration, std::string nHitString);


    /** for given chain and names of branches:
     * this function returns their pointers to the branch and the containing value
     * in the same order as the input vector of branch names. */
    template<class ValueType> std::vector<BranchInterface<ValueType>> getBranches(
          std::unique_ptr<TChain>& chain,
          const std::vector<std::string>& branchNames);


    /** sets everything which is hit-dependent. */
    std::string prepareNHitSpecificStuff(
      unsigned nHits,
      TrainerConfigData& config,
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
      return std::move(iDs);
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


    /// WARNING TODO: Filter-> String to big->unsigned is better (or FilterID)
    /** does everything needed for given chainLength of sectors (e.g.: 2 -> twoHitFilters)*/
    void processSectorCombinations(TrainerConfigData& config, unsigned secChainLength)
    {
      B2DEBUG(1, "processSectorCombinations: training map " << config.secMapName << " with secChainLength: " << secChainLength)

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

      B2DEBUG(1, "processSectorCombinations: nKilled " << nKilled)

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

      /// TODO update sublayers!
      // checks for sectors which have inner neighbour and updates the sublayerID of the sensors.
      //    auto nsectorsRenamed = relationsGraph.updateSubLayers();

      B2INFO("processSectorCombinations: training finished.\n" << mainGraph.print();)
      /**
       * Here the data will be loaded in the new secMap-Design:
       *
       * // start creating new secMap:
       * VXDTFFilters allFilters;

       *  have to be added in one batch per sensor (sensor ordering not important, but sector-on-sensor-ordering is
       * for (layer, ladder, sensor in config.allowedLayers) { // TODO minMaxLayer in config -> change e,g {3,6} to {3,4,5,6}!

      *    auto thisSensorID = VxdID(layer, ladder , sensor);
      *   // get all sectors seen in Training of this sensor -> they have got the subLayerID! (but may be incomplete as a set)
      *   std::vector<FullSecID> allTrainedSecIDsOfThisSensor;

      *   protoMap.getAllFullSecIDsOfSensor(thisSensor, allTrainedSecIDsOfThisSensor);

      *   //create full vector of FullSecIDs of this sensor -> if TrainedID already exists, that one gets stored.
      *   std::vector<std::vector<FullSecID>> allSecIDsOfThisSensor;
      *   allSecIDsOfThisSensor.resize(config.uDirectionCuts(layer).size() + 1);
      *   int counter = 0;
      *   for (unsigned int i = 0; i < config.uDirectionCuts(layer).size() + 1; i++) {
      *   allSecIDsOfThisSensor[i].resize(config.vDirectionCuts(layer).size() + 1);
      *   for (unsigned int j = 0; j < config.vDirectionCuts(layer).size() + 1 ; j++) {
      *   auto tempID = FullSecID(thisSensorID, false, counter);

      *   bool found = false;
      *   for (FullSecID& trainedID : allTrainedSecIDsOfThisSensor) {
      *   if (tempID.getVxdID() == trainedID.getVxdID()
      *   and tempID.getSecID() == trainedID.getSecID())
      *   {
      *   found = true;
      *   allSecIDsOfThisSensor[i][j] = trainedID;
      * }
      * }

      * if (found == false) {
      * allSecIDsOfThisSensor[i][j] = FullSecID(thisSensorID, false, counter);
      * }
      * counter ++;
      * }
      * }

      * B2DEBUG(1, "Sensor: " << thisSensorID << " had " << allTrainedSecIDsOfThisSensor.size() << " trained IDs and " << counter << " sectors in Total")

      * allFilters.addSectorsOnSensor(config.uDirectionCuts(layer), config.vDirectionCuts(layer), allSecIDsOfThisSensor); // TODO u/vDirectionCuts -> layerspecific!

      *     // and add the virtual IP:
      *     uSup = {};
      *     vSup = {};
      *     allSecIDsOfThisSensor = {{0}};
      *     allFilters.addSectorsOnSensor(uSup, vSup, allSecIDsOfThisSensor);

      *     // TODO:
      *     // Now we need the selectionVariables. -> how to solve the issue of hardcoded filters (is it possible?)
      * }
      * }
      *
      */
    }



    /** Initializes the Module.
     */
    virtual void initialize()
    {
      StoreObjPtr< SectorMap > sectorMap("", DataStore::c_Persistent);
      B2INFO("RawSecMapMerger::initialize():");

      // loop over all the setups in the sectorMap:
      for (auto& setup : sectorMap->getAllSetups()) {

        auto config = setup.second->getConfig();
        B2INFO("RawSecMapMerger::initialize(): loading mapName: " << config.secMapName);

//         if (config.secMapName != "lowTestRedesign") { continue; } // TODO WARNING DEBUG we do not want to run more than one run yet!

        processSectorCombinations(config, 2);

// //         return; // TODO WARNING DEBUG we do not want to run more than one run yet!
        processSectorCombinations(config, 3);
        processSectorCombinations(config, 4);

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
// // // //     auto getConfig = [] (std::string) { return TrainerConfigData(); };
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
