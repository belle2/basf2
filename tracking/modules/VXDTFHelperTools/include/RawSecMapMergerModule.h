/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/GearDir.h> // needed for reading xml-files

#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/trackFindingVXD/sectorMapTools/TrainerConfigData.h>
#include <tracking/trackFindingVXD/sectorMapTools/MinMaxCollector.h>
#include <tracking/trackFindingVXD/sectorMapTools/ProtoSectorGraph.h>
#include "tracking/trackFindingVXD/sectorMapTools/SectorMap.h"
#include "framework/datastore/StoreObjPtr.h"

// stl:
#include <string>
#include <vector>
#include <deque>
// #include <set>
// #include <list>
// #include <map>
#include <utility> // std::pair
#include <memory> // std::unique_ptr

// root:
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TChain.h>
//boost:
#include <boost/tuple/tuple.hpp> // a generalized version of pair
#ifndef __CINT__
#include <boost/chrono.hpp>
#endif

namespace Belle2 {

  /** The RawSecMapMergerModule
   *
   * this module takes a root file containing a raw sectorMap created by the SecMapTrainerBaseModule
   * and converts it to a sectormap which can be read by the VXDTF. Please check the parameters to be set...
   *
   */
  class RawSecMapMergerModule : public Module {

  public:
    /**
     * Constructor of the module.
     */
    RawSecMapMergerModule();



    /** Destructor of the module. */
    virtual ~RawSecMapMergerModule() {}

    /** returns all names of root-files fitting given parameter mapName  */
    std::vector<std::string> getRootFiles(std::string mapName)
    {
      B2INFO("RawSecMapMerger::getRootFiles(): loading mapName: " << mapName)
      // TODO
      // INFO: root files will contain the mapName and some extensions: mapName_rngNumber.root
      // in current directory read all files matching this.
      return {"mapName"};
    }



    /** bundle all relevant files to a TChain */
    std::unique_ptr<TChain> createTreeChain(TrainerConfigData& configuration, std::string nHitString)
    {
      B2INFO("RawSecMapMerger::createTreeChain(): loading mapName: " << configuration.secMapName << " with extension " << nHitString)
      std::unique_ptr<TChain> treeChain = std::unique_ptr<TChain>(new TChain((configuration.secMapName + nHitString).c_str()));

      // dummy code yet:
      auto fileList = getRootFiles(configuration.secMapName);
      for (auto file : fileList) { /*treeChain->Add(file);*/ }

//       TFile* input = TFile::Open("highTestRedesign_454970355.root");
      treeChain->Add("highTestRedesign_454970355.root");
//    TTree* tree = (TTree*) input->Get("m_treePtr"); // name of tree in root file

      return std::move(treeChain);
    }



    /** for given chain and names of branches:
     * this function returns their pointers to the branch and the containing value
     * in the same order as the input vector of branch names. */
    template<class ValueType>
    void getBranches(
      std::unique_ptr<TChain>& chain,
      const std::vector<std::string>& branchNames,
      std::deque<std::pair<TBranch*, ValueType>>& branches)
    {
      B2INFO("RawSecMapMerger::getBranches(): loading branches: " << branchNames.size())
      branches.clear();
      unsigned nBranches = branchNames.size();

      branches.resize(nBranches, {nullptr, ValueType(0)});
      for (unsigned fPos = 0; fPos < nBranches; fPos++) {
        chain->SetBranchAddress(
          branchNames[fPos].c_str(),
          &(branches[fPos].second),
          &(branches[fPos].first));
      }
      B2INFO("RawSecMapMerger::getBranches():  done")
    }



    /** builds a graph of related sectors by using training data. */
    ProtoSectorGraph<MinMaxCollector<double>> createRelationSecMap(
                                             std::unique_ptr<TChain>& chain,
                                             std::deque<std::pair<TBranch*, unsigned>>& sectorBranches)
    {
      unsigned nEntries = chain->GetEntries();
      B2INFO("RawSecMapMerger::createRelationSecMap():  start of " << nEntries << " entries in tree and " << sectorBranches.size() <<
             " branches")
      // creating master sector containing all inner sectors and therefore the full graph in the end:
      ProtoSectorGraph<MinMaxCollector<double>> fullRawSectorGraph(std::numeric_limits<unsigned>::max());

      unsigned percentMark = nEntries / 10;
      unsigned progressCounter = 0;
      // log all sector-combinations and determine their absolute number of appearances:
      for (unsigned i = 0 ;  i <= nEntries; i++) {
        if ((i % percentMark) == 0) {
          progressCounter += 10;
          B2INFO("RawSecMapMerger::createRelationSecMap(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
                 "% related, mainGraph has got " << fullRawSectorGraph.size() << " sectors...")
        }

        // do hard copy of the branches:
        std::deque<std::pair<TBranch*, unsigned>> secBranches = sectorBranches;

        auto thisEntry = chain->LoadTree(i);
        secBranches[0].first->GetEntry(thisEntry);
        unsigned currentID = secBranches[0].second;

        bool found = false;
        for (auto& sector : fullRawSectorGraph) {
          if (sector.getRawSecID() != currentID) { continue; }
          found = true;
          sector.wasFound();
          break;
//      secBranches.pop_front();
//      sector.addRelationToGraph(thisEntry, secBranches);
        }

        // if combination does not exist yet, it will get added to the graph, if it exists, simply counters are increased.
        if (!found) {
          auto pos = fullRawSectorGraph.addInnerSector(currentID);
//      auto pos = fullRawSectorGraph.find(currentID);
          pos->addRelationToGraph(thisEntry, sectorBranches);
        }

//         fullRawSectorGraph.addRelationToGraph(thisEntry, sectorBranches);
      }
      B2INFO("RawSecMapMerger::createRelationSecMap(): fullRawSectorGraph has now size: " << fullRawSectorGraph.size())

      return std::move(fullRawSectorGraph);
    }



    /** takes a relationMap and checks for each outer sector:
     *   how often did it occur (-> mainSampleSize)
     * for each inner sector:
     *   how often did it occur (-> innerSampleSize)
     * sort innerSectors by rareness (rarest one comes first):
     *   if innerSampleSize is below threshold -> remove, update mainSampleSize
     * Produces a Map of ProtoSectorGraphs which contain innerSectors
     * (ProtoSectorGraphs which are no innerSector and have no innerSectors themselves will not be stored).  */
    /** use rareness-threshold to find sector-combinations which are very rare and remove them: */
    unsigned pruneMap(double rarenessThreshold, ProtoSectorGraph<MinMaxCollector<double>>& relationsGraph)
    {
      unsigned nSectorsKilled = 0;

      // TODO one could also remove subGraphs here (the pruneGraph-function is recursive), but maybe we want to have those seldom cases too.
      for (auto& subGraph : relationsGraph) {
        nSectorsKilled += subGraph.pruneGraph(rarenessThreshold);
      }
      B2INFO("RawSecMapMerger::pruneMap(): nSectorsKilled: " << nSectorsKilled << " with rarenessCut: " << rarenessThreshold)
      return nSectorsKilled;
    }



    /** get the raw data and determine the cuts for the filters/selectionVariables: */
    void distillRawData4FilterCuts(
      std::unique_ptr<TChain>& chain,
      TrainerConfigData& config,
      std::deque<std::pair<TBranch*, unsigned>>& sectorBranches,
      ProtoSectorGraph<MinMaxCollector<double>>& relationsGraph,
      unsigned secChainLength)
    {
      unsigned nEntries = chain->GetEntries();
      B2INFO("RawSecMapMerger::distillRawData4FilterCuts():  start of " << nEntries << " entries in tree and " << sectorBranches.size() <<
             " branches")

      // prepare the links for the filter-value-branches:
      std::deque<std::pair<TBranch*, double>> filterBranches;
      if (secChainLength == 2) {
        getBranches<double>(chain, config.twoHitFilters, filterBranches);
      } else if (secChainLength == 3) {
        getBranches<double>(chain, config.threeHitFilters, filterBranches);
      } else {
        getBranches<double>(chain, config.fourHitFilters, filterBranches);
      }

      // loop over the tree to find the entries matching this outerSector:
      unsigned percentMark = nEntries / 50;
      unsigned progressCounter = 0;
      for (unsigned i = 0 ;  i <= nEntries; i++) {
        if ((i % percentMark) == 0) {
          progressCounter += 2;
          B2INFO("RawSecMapMerger::distillRawData4FilterCuts(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
                 "% related, mainGraph has got " << relationsGraph.size() << " sectors...")
        }

        auto thisEntry = chain->LoadTree(i);
        // collect raw data of each sector-combination and find cuts:
        for (auto& graph : relationsGraph) {
          graph.distillRawData4FilterCuts(thisEntry, config, sectorBranches, filterBranches, secChainLength);
        }
      } // leaves-loop

      // find cuts of each sector-combination:
      percentMark = relationsGraph.size() / 25;
      progressCounter = 0;
      unsigned currentSector = 0;
      for (auto& sector : relationsGraph) {

        // after collecting all data, store only the cuts to keep ram consumption down:
        unsigned nSamples = sector.determineCutsAndCleanRawData();

        if ((currentSector % percentMark) == 0) {
          progressCounter += 2;
          B2INFO("RawSecMapMerger::distillRawData4FilterCuts(): with mark: " << percentMark << " and i=" << currentSector << ", " <<
                 progressCounter << "% processed,  sector " << std::string(FullSecID(sector.getRawSecID())) << " had " << nSamples << " samples")
        }
        currentSector++;
      }
    }



    /** does everything needed for given chainLength of sectors (e.g.: 2 -> twoHitFilters)*/
    void processSectorCombinations(TrainerConfigData& configuration, unsigned secChainLength)
    {
      B2DEBUG(1, "processSectorCombinations: training map " << configuration.secMapName << " with secChainLength: " << secChainLength)

      /// ////////////////////////////////////       WARNING hardcoded bla for minimal working environment!
      // TODO
      // INFO: sectorChainLength -> have to get chainLength-specific stuff (name, selectionVariables, cuts, ...)
      // at the moment, hardcoded for 2hit-combi...
      std::string nHit = (secChainLength == 2) ? "2Hit" :
                         (secChainLength == 3) ? "3Hit" :
                         (secChainLength == 4) ? "4Hit" : "";
      if (nHit == "") {
        B2ERROR("wrong chainLength!"); return;
      }

      // branch-names sorted from outer to inner:
      std::vector<std::string> branchNames;
      if (secChainLength == 2) {
        branchNames = { "outerSecID", "innerSecID"};
      } else if (secChainLength == 3) {
        branchNames = { "outerSecID", "centerSecID", "innerSecID"};
      } else {
        branchNames = { "outerSecID", "outerCenterSecID", "innerCenterSecID", "innerSecID"};
      }
      /// ////////////////////////////////////       WARNING hardcoded bla end.
      // TODO WARNING: check all following functions for hardcoded twoHitFilters-stuff -> fix!



      std::unique_ptr<TChain> chain = createTreeChain(configuration, nHit);

      // prepare links to sector branches:
      std::deque<std::pair<TBranch*, unsigned>> sectorBranches;
      getBranches<unsigned>(chain, branchNames, sectorBranches);

      // create proto-graph containing all sector-chains occured in the training sample.
      ProtoSectorGraph<MinMaxCollector<double>> relationsGraph = createRelationSecMap(chain, sectorBranches);

      // use rareness-threshold to find sector-combinations which are very rare and remove them:
      unsigned nSectorsRemoved = pruneMap(configuration.rarenessThreshold, relationsGraph);

      // get the raw data and determine the cuts for the filters/selectionVariables:
      distillRawData4FilterCuts(chain, configuration, sectorBranches, relationsGraph, secChainLength);

      // checks for sectors which have inner neighbour and updates the sublayerID of the sensors.
      auto nsectorsRenamed = relationsGraph.updateSubLayers();
      B2DEBUG(1, "results ... nSecRemoved/renamed" << nSectorsRemoved << "/" << nsectorsRenamed)


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



    /// ///////////////////////////////////////////////////////////////////////////////// member variables of module:
  protected:
    /** parameter of module: contains the names to be loaded. */
    std::vector<std::string> m_PARAMmapNames;
  public:
    // ///////////////////////////////////////////////////////////////////////////////// member variables END:



    /** Initializes the Module.
     */
    virtual void initialize()
    {

      StoreObjPtr< SectorMap > sectorMap("", DataStore::c_Persistent);
      B2INFO("RawSecMapMerger::initialize():");

      // loop over all the setups in the sectorMap:
      for (auto setup : sectorMap->getAllSetups()) {

        auto config = setup.second->getConfig();
        B2INFO("RawSecMapMerger::initialize(): loading mapName: " << config.secMapName);

        processSectorCombinations(config, 2);
        processSectorCombinations(config, 3);
        processSectorCombinations(config, 4);

      }

    }












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
