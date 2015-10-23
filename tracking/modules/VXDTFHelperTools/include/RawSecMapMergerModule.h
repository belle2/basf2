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
//       treeChain->Add("highTestRedesign_454970355.root");
//    treeChain->Add("lowTestRedesign_1990122242.root");
      treeChain->Add("lowTestRedesign_454970355.root");
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
                                             std::deque<std::pair<TBranch*, unsigned>>& sectorBranches,
                                             std::deque<std::pair<TBranch*, double>>& filterBranches)
    {
      unsigned nEntries = chain->GetEntries();
      B2INFO("RawSecMapMerger::createRelationSecMap():  start of " << nEntries << " entries in tree and " << sectorBranches.size() <<
             " branches")
      // creating master sector containing all inner sectors and therefore the full graph in the end:
      ProtoSectorGraph<MinMaxCollector<double>> fullRawSectorGraph(std::numeric_limits<unsigned>::max());

      if (nEntries == 0) { B2WARNING("createRelationSecMap: valid file but no data stored!"); return std::move(fullRawSectorGraph); }
      unsigned percentMark = nEntries / 10;
      unsigned progressCounter = 0;
      // log all sector-combinations and determine their absolute number of appearances:
      for (unsigned i = 0 ;  i <= nEntries; i++) {
        if ((i % percentMark) == 0) {
          progressCounter += 10;
          B2INFO("RawSecMapMerger::createRelationSecMap(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
                 "% related, mainGraph has got " << fullRawSectorGraph.size() << " sectors...")
        }

        // d/*/*/*/*/*/*/*/*/*/*/*o*/*/*/*/*/*/*/*/*/*/*/ hard copy of the branches:
        std::deque<std::pair<TBranch*, unsigned>>& secBranches = sectorBranches;

        auto thisEntry = chain->LoadTree(i);
//         secBranches[0].first->GetEntry(thisEntry);
        ProtoSectorGraph<MinMaxCollector<double>>::updateBranchEntries<unsigned>(thisEntry, secBranches);
        unsigned currentID = secBranches[0].second;

        auto pos = fullRawSectorGraph.find(currentID);

//         bool found = false;
//         for (auto& sector : fullRawSectorGraph) {
//           if (sector.getRawSecID() != currentID) { continue; }
//           found = true;
//           sector.wasFound();
//           break;
// //      secBranches.pop_front();
// //      sector.addRelationToGraph(thisEntry, secBranches);
//         }

        // if combination does not exist yet, it will get added to the graph, if it exists, simply counters are increased.
//         if (!found) {
        if (pos == fullRawSectorGraph.end()) {
          pos = fullRawSectorGraph.addInnerSector(currentID);
        }


//      auto pos = fullRawSectorGraph.find(currentID);
//    secBranches.pop_front();
//           secBranches[0].first->GetEntry(thisEntry);
        if (secBranches.size() > 1 && FullSecID(currentID).getLayerID() < FullSecID(secBranches[1].second).getLayerID()) {
          B2ERROR("createRelationSecMap: outerID is not outermore than innerID! (outer/inner: " <<
                  FullSecID(currentID).getFullSecString() << "/" <<
                  FullSecID(secBranches[0].second).getFullSecString() << ")");
        }
        pos->addRelationToGraph(thisEntry, secBranches, filterBranches);
      }

//         fullRawSectorGraph.addRelationToGraph(thisEntry, sectorBranches);
//       }
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
             " branches and chainLength " << secChainLength)

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
      unsigned percentMark = 1;
      if (nEntries > 100) { percentMark = nEntries / 50; }
      unsigned progressCounter = 0, goodCasesCtr = 0;
      for (unsigned i = 0 ;  i < nEntries; i++) {
        if ((i % percentMark) == 0) {
          progressCounter += 2;
          B2INFO("RawSecMapMerger::distillRawData4FilterCuts(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
                 "% related, mainGraph has got " << relationsGraph.size() << " sectors...")
        }

        auto thisEntry = chain->LoadTree(i);

        sectorBranches[0].first->GetEntry(thisEntry);
        auto foundSector = relationsGraph.find(sectorBranches[0].second);
        if (foundSector == relationsGraph.end()) {
          B2DEBUG(1, "RawSecMapMerger::distillRawData4FilterCuts(): entry " << i << " with outerSecID: " << FullSecID(
                    sectorBranches[0].second).getFullSecString() << ", did not find sector, skipping!");
          continue;
        }

        // collect raw data of each sector-combination and find cuts:
        // variant 1:
//    goodCasesCtr += foundSector->distillRawData4FilterCuts(thisEntry, config, sectorBranches, filterBranches, secChainLength);

        // variant 2:
//    auto secBranches = sectorBranches; // hardCopy
//    secBranches.pop_front();
        goodCasesCtr += foundSector->distillRawData4FilterCutsV2(thisEntry, config, sectorBranches, filterBranches, secChainLength);

        // old stuff (to be deleted):
//         for (auto& graph : relationsGraph) {
//           graph.distillRawData4FilterCuts(thisEntry, config, sectorBranches, filterBranches, secChainLength);
//         }
      } // leaves-loop
      B2INFO("RawSecMapMerger::distillRawData4FilterCuts(): within " << nEntries << "  sectorGraphs found " << goodCasesCtr <<
             " compatible sectors. now starting determining cuts.")


      // find cuts of each sector-combination:
      percentMark = 1;
      if (relationsGraph.size() > 100) { percentMark = relationsGraph.size() / 25; }
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



    /** for debugging: print data for crosschecks. for small sample sizes < 100 the whole sample will be printed, for bigger ones only 100 entries will be printed. */
    void printData(std::unique_ptr<TChain>& chain, TrainerConfigData& config, std::deque<std::pair<TBranch*, unsigned>>& sectorBranches,
                   std::vector<std::string>& secBranchNames)
    {
      // prepare everything:
      unsigned nEntries = chain->GetEntries();
      unsigned percentMark = 1;
      if (nEntries > 100) { percentMark = nEntries / 50; }
      unsigned progressCounter = 0;

      std::deque<std::pair<TBranch*, double>> filterBranches;
      std::vector<std::string>* filterBranchNames;
      if (secBranchNames.size() == 2) {
        filterBranchNames = &(config.twoHitFilters);
      } else if (secBranchNames.size() == 3) {
        filterBranchNames = &(config.threeHitFilters);
      } else {
        filterBranchNames = &(config.fourHitFilters);
      }
      getBranches<double>(chain, *filterBranchNames, filterBranches);

      B2INFO("RawSecMapMerger::printData():  start of " << nEntries <<
             " entries in tree and " << sectorBranches.size() <<
             "/" << filterBranchNames->size() <<
             " sector-/filter-branches")

      for (unsigned i = 0 ;  i < nEntries; i++) {
        if ((i % percentMark) != 0) { continue; }

        auto thisEntry = chain->LoadTree(i);

        progressCounter += 2;
        B2INFO("RawSecMapMerger::printData(): entry " << i << " of " << nEntries << ":")

        std::string out;
        for (unsigned i = 0 ; i < secBranchNames.size(); i++) {
          sectorBranches[i].first->GetEntry(thisEntry);
          out += secBranchNames[i] + ": " + FullSecID(sectorBranches[i].second).getFullSecString() + ". ";
        }
        out += "\n";

        for (unsigned i = 0 ; i < filterBranchNames->size(); i++) {
          filterBranches[i].first->GetEntry(thisEntry);
          out += (*filterBranchNames)[i] + ": " + std::to_string(filterBranches[i].second) + ". ";
        }
        B2INFO(out << "\n")
      }
    }



    /** does everything needed for given chainLength of sectors (e.g.: 2 -> twoHitFilters)*/
    void processSectorCombinations(TrainerConfigData& configuration, unsigned secChainLength)
    {
      B2DEBUG(1, "processSectorCombinations: training map " << configuration.secMapName << " with secChainLength: " << secChainLength)


      // branch-names sorted from outer to inner:
      std::vector<std::string> secBranchNames;
      // filter names are different for different secChainLengths:
      std::vector<std::string> filterBranchNames;
      std::string nHit = prepareNHitSpecificStuff(secChainLength, configuration, secBranchNames, filterBranchNames);


      std::unique_ptr<TChain> chain = createTreeChain(configuration, nHit);

      // prepare links to branches branches:
      std::deque<std::pair<TBranch*, unsigned>> sectorBranches;
      getBranches<unsigned>(chain, secBranchNames, sectorBranches);
      std::deque<std::pair<TBranch*, double>> filterBranches;
      getBranches<double>(chain, filterBranchNames, filterBranches);

      // for debugging: print data for crosschecks:
      printData(chain, configuration, sectorBranches, secBranchNames);


      // create proto-graph containing all sector-chains occured in the training sample.
      ProtoSectorGraph<MinMaxCollector<double>> relationsGraph = createRelationSecMap(chain, sectorBranches, filterBranches);

      std::string output = relationsGraph.print();
      B2INFO("after createRelationSecMap: relationsGraph looks like this: \n\n" << output)


      if (relationsGraph.empty()) { return; }

      // use rareness-threshold to find sector-combinations which are very rare and remove them:
//       unsigned nSectorsRemoved = pruneMap(configuration.rarenessThreshold, relationsGraph);

      // get the raw data and determine the cuts for the filters/selectionVariables:
      distillRawData4FilterCuts(chain, configuration, sectorBranches, relationsGraph, secChainLength);

      // checks for sectors which have inner neighbour and updates the sublayerID of the sensors.
      auto nsectorsRenamed = relationsGraph.updateSubLayers();
//       B2DEBUG(1, "results ... nSecRemoved/renamed " << nSectorsRemoved << "/" << nsectorsRenamed)
      B2DEBUG(1, "results ... nSecrenamed " << nsectorsRenamed)


      std::string output2 = relationsGraph.print();
      B2INFO("end of processing: relationsGraph looks like this: \n\n" << output2)
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
      doALLTEMP(); /// DEBUG WARNING TEST...
      return;

      StoreObjPtr< SectorMap > sectorMap("", DataStore::c_Persistent);
      B2INFO("RawSecMapMerger::initialize():");

      // loop over all the setups in the sectorMap:
      for (auto setup : sectorMap->getAllSetups()) {

        auto config = setup.second->getConfig();
        B2INFO("RawSecMapMerger::initialize(): loading mapName: " << config.secMapName);

        if (config.secMapName != "lowTestRedesign") { continue; } // TODO WARNING DEBUG we do not want to run more than one run yet!

        processSectorCombinations(config, 2);

        return; // TODO WARNING DEBUG we do not want to run more than one run yet!
        processSectorCombinations(config, 3);
        processSectorCombinations(config, 4);

      }

    }



    /** simple struct for interfacing the Branch.  */
    template <class ValueType> struct BranchInterface {
      std::string name; /**< carries name of branch. */
      TBranch* branch; /**< carries pointer to branch. */
      ValueType value; /**< carries value of current branchEntry. */

      /** constructor */
      BranchInterface() : name(""), branch(nullptr), value(ValueType(0)) {}

      /** update entry */
      void update(Long64_t entry) { branch->GetEntry(entry); }
    };



    /** for given chain and names of branches:
    * this function returns their pointers to the branch and the containing value
    * in the same order as the input vector of branch names. */
    template<class ValueType>
    std::vector<BranchInterface<ValueType>> getBranchesV2(
                                           std::unique_ptr<TChain>& chain,
                                           const std::vector<std::string>& branchNames)
    {
      std::vector<BranchInterface<ValueType>> branches;
      B2INFO("RawSecMapMerger::getBranches(): loading branches: " << branchNames.size())
      unsigned nBranches = branchNames.size();

      branches.resize(nBranches, BranchInterface<ValueType>());
      for (unsigned fPos = 0; fPos < nBranches; fPos++) {
        branches[fPos].name = branchNames[fPos];
        chain->SetBranchAddress(
          branches[fPos].name.c_str(),
          &(branches[fPos].value),
          &(branches[fPos].branch));
      }
      B2INFO("RawSecMapMerger::getBranches():  done")
      return std::move(branches);
    }



    /** for debugging: print data for crosschecks. for small sample sizes < 100 the whole sample will be printed, for bigger ones only 100 entries will be printed. */
    void printDataV2(
      std::unique_ptr<TChain>& chain,
      std::vector<BranchInterface<unsigned>>& sectorBranches,
      std::vector<BranchInterface<double>>& filterBranches)
    {
      // prepare everything:
      unsigned nEntries = chain->GetEntries();
      unsigned percentMark = 1;
      if (nEntries > 100) { percentMark = nEntries / 50; }
      unsigned progressCounter = 0;

      B2INFO("RawSecMapMerger::printData():  start of " << nEntries <<
             " entries in tree and " << sectorBranches.size() <<
             "/" << filterBranches.size() <<
             " sector-/filter-branches")

      for (unsigned i = 0 ;  i < nEntries; i++) {
        if ((i % percentMark) != 0) { continue; }

        auto thisEntry = chain->LoadTree(i);

        progressCounter += 2;
        B2INFO("RawSecMapMerger::printData(): entry " << i << " of " << nEntries << ":")

        std::string out;
        for (unsigned i = 0 ; i < sectorBranches.size(); i++) {
          sectorBranches[i].branch->GetEntry(thisEntry);
          out += sectorBranches[i].name + ": " + FullSecID(sectorBranches[i].value).getFullSecString() + ". ";
        }
        out += "\n";

        for (unsigned i = 0 ; i < filterBranches.size(); i++) {
          filterBranches[i].branch->GetEntry(thisEntry);
          out += filterBranches[i].name + ": " + std::to_string(filterBranches[i].value) + ". ";
        }
        B2INFO(out << "\n")
      }
    }



    std::string prepareNHitSpecificStuff(unsigned nHits, TrainerConfigData& config, std::vector<std::string>& secBranchNames,
                                         std::vector<std::string>& filterBranchNames)
    {
      if (nHits == 2) {
        secBranchNames = { "outerSecID", "innerSecID"};
        filterBranchNames = config.twoHitFilters;
        return "2Hit";
      }

      if (nHits == 3) {
        secBranchNames = { "outerSecID", "centerSecID", "innerSecID"};
        filterBranchNames = config.threeHitFilters;
        return "3Hit";
      }

      if (nHits == 4) {
        secBranchNames = { "outerSecID", "outerCenterSecID", "innerCenterSecID", "innerSecID"};
        filterBranchNames = config.fourHitFilters;
        return "4Hit";
      }

      B2ERROR("prepareNHitSpecificStuff: wrong chainLength!");
      return "";
    }



    /** small class for storing min and max. */
    class MinMax {
    protected:
      std::pair<double, double> m_minMax; /**< .first is min, .second is .max. */
    public:

      /** construtor, starts with max for min and min for max (will later be replaced by valid entries). */
      MinMax() :
        m_minMax( {std::numeric_limits< double >::max(), std::numeric_limits< double >::min()}) {}

      /// reset to start values.
      void reset() { m_minMax = {std::numeric_limits< double >::max(), std::numeric_limits< double >::min()}; }

      /** checks value and replaces old ones if new one is better. returns true if minMax was updated. */
      bool checkAndReplaceIfMinMax(double newVal)
      {
        bool wasAdded = false;
        if (m_minMax.first > newVal) { m_minMax.first = newVal; wasAdded = true; }
        if (m_minMax.second < newVal) { m_minMax.second = newVal; wasAdded = true; }
        return wasAdded;
      }

      double getMin() const { return m_minMax.first; } /**< returns smallest value stored so far. */

      double getMax() const { return m_minMax.second; } /**< returns biggest value stored so far. */

      std::string print() const
      {
        std::string min = m_minMax.first == std::numeric_limits< double >::max() ? "max<double>" : std::to_string(m_minMax.first);
        std::string max = m_minMax.second == std::numeric_limits< double >::min() ? "min<double>" : std::to_string(m_minMax.second);
        return "min: " + min + ", max: " + max;
      }
    };



// // // // //  /** WARNING TODO! */
// // // // //  class RawDataCollectedBase {
// // // // //
// // // // //  };

    class RawDataCollectedMinMax { /* : public RawDataCollectedBase*/
    protected:
//    unsigned m_maxThreshold; ///< the maximum number of elements allowed to be collected. Standard value expects threshold of 2 GB RAM
//    unsigned m_expectedSize; ///< the expected size of the final data sample.
      unsigned m_currentSize; ///< the current size of the data sample.
      unsigned m_fillIntermediateThreshold; ///< an internal threshold taking care of collecting intermediate results during sample collection
      std::pair<double, double> m_minMaxQuantiles; ///< the quantiles to be collected in the end (defined in [0;1])
      std::vector<std::pair<double, double>> m_intermediateValues; ///< collects intermediate threshold if expected size is too big.
      MinMaxCollector<double> m_collector; ///< collects raw data in an RAM-saving way.
    public:

      /// constructor.
      RawDataCollectedMinMax(unsigned expectedSize,
                             std::pair<double, double> quantiles,
                             unsigned maxSizeThreshold = 100000) :
//    m_maxThreshold(maxSizeThreshold),
//    m_expectedSize(expectedSize),
        m_currentSize(0),
        m_fillIntermediateThreshold(std::numeric_limits<unsigned>::max()),
        m_minMaxQuantiles(quantiles),
        m_collector((quantiles.first > (/*1.-*/quantiles.second) ? quantiles.first * 2 : (/*1.-*/quantiles.second) * 2))
      {
        if (double(expectedSize) / (double(maxSizeThreshold) * 0.05) > double(maxSizeThreshold))
        { B2FATAL("RawDataCollectedMinMax: expected data to big, can not execute!") }

        if (maxSizeThreshold < expectedSize) {
          m_fillIntermediateThreshold = maxSizeThreshold / 10;
        }
      }

      /// adds value to collector.
      void add(double newVal)
      {
        m_collector.append(newVal);
        m_currentSize++;

        // if threshold reached, collect results and fill into intermediate value-container:
        if (m_collector.totalSize() > m_fillIntermediateThreshold) {
          std::pair<double, double> results = m_collector.getMinMax(m_minMaxQuantiles.first, m_minMaxQuantiles.second);
          m_intermediateValues.push_back(std::move(results));
          m_collector.clear();
        }
      }

      /// returns current sample size (which is not the actual size of the container).
      unsigned getSampleSize() const { return m_currentSize; }

      /// returns current best estimates for min and max cuts.
      std::pair<double, double> getMinMax()
      {
        if (m_intermediateValues.empty()) {
          return m_collector.getMinMax(m_minMaxQuantiles.first, m_minMaxQuantiles.second);
        }

        // issue: m_collector-sample could be too small and therefore distort results for small intermediateValue-samples. Therefore neglect m_collector for that case.
        if (m_intermediateValues.size() == 1) {
          return { m_intermediateValues.at(0).first, m_intermediateValues.at(0).second};
        }
        if (m_intermediateValues.size() == 2) {
          return {
            0.5 * (m_intermediateValues.at(0).first + m_intermediateValues.at(0).first),
            0.5 * (m_intermediateValues.at(0).second + m_intermediateValues.at(0).second) };
        }

        if (!m_collector.empty()) {
          std::pair<double, double> results = m_collector.getMinMax(m_minMaxQuantiles.first, m_minMaxQuantiles.second);
          m_intermediateValues.push_back(results);
        }

        unsigned index = std::floor(double(m_intermediateValues.size()) * 0.5);
        double min, max;

        std::sort(m_intermediateValues.begin(), m_intermediateValues.end(),
                  [](const std::pair<double, double>& a, const std::pair<double, double>& b) -> bool { return a.first < b.first; });
        min = m_intermediateValues.at(index).first;

        std::sort(m_intermediateValues.begin(), m_intermediateValues.end(),
                  [](const std::pair<double, double>& a, const std::pair<double, double>& b) -> bool { return a.second < b.second; });
        max = m_intermediateValues.at(index).second;

        return {min, max};
      }
    };


    class SubGraphID {
    protected:
      const std::vector<unsigned> m_idChain;
    public:

      /** overloaded '=='-operator */
      bool operator==(const SubGraphID& b) const
      {
        unsigned size = m_idChain.size();
        if (b.m_idChain.size() != size) { return false; }

        for (unsigned i = 0; i < size; i++) {
          if (b.m_idChain[i] != m_idChain[i]) { return false; }
        }

        return true;
      }

      /** overloaded '<'-operator for sorting algorithms */
      bool operator<(const SubGraphID& b)  const
      {
        // sorting: if outermost (== first) sector > of b.outermost this > b. If outermost == b.outermost, step is repeated until end of chain length is reached. last straw: longer chain is bigger.
        unsigned size = m_idChain.size() < b.m_idChain.size() ? m_idChain.size() : b.m_idChain.size();
        for (unsigned i = 0 ; i < size; i++) {
          if (m_idChain[i] < b.m_idChain[i]) return true;
          if (m_idChain[i] == b.m_idChain[i]) continue;
          return false;
        }
        return m_idChain.size() < b.m_idChain.size();
      }

      /** constructor, mandatory iDChain musst at least contain one iD. */
      SubGraphID(const std::vector<unsigned>& idChain) : m_idChain(idChain)
      {
        if (m_idChain.empty()) { B2FATAL("SubGraphID-Constructor, given idChain is empty - illegal usage!") }
      }

      /** returns chainLength of this id. */
      unsigned size() const { return m_idChain.size(); }

      /** if both graphs have got the same IDs except the last one, they share a trunk. */
      bool checkSharesTrunk(const SubGraphID& b) const
      {
        unsigned size = m_idChain.size();
        if (b.m_idChain.size() != size) { return false; }

        for (unsigned i = 0; i < size - 1; i++) {
          if (b.m_idChain[i] != m_idChain[i]) { return false; }
        }

        return true;
      }

      /** returns string of entries. */
      std::string print() const
      {
        std::string out;
        for (unsigned iD : m_idChain) {
          out += FullSecID(iD).getFullSecString() + " ";
        }
        return std::move(out);
      }

    };



    /** contains all relevant stuff needed for dealing with a subGraph. ~ 404 bytes per subGraph. */
    template<class FilterType> class SubGraph {
    protected:
      const SubGraphID m_id; /**< contains the IDs in the correct order (from outer to inner) as a unique identifier for this graph. */

      std::unordered_map<FilterType, MinMax>
      m_minMaxValues; /**< contains all min- and max-values found so far for all filters relevant for this Graph. */

      unsigned m_found; /**< counts number of times this subgraph was found. */

      std::unordered_map<FilterType, RawDataCollectedMinMax>* m_rawDataCollected; /**< takes care of collecting the raw data. */
//    std::unique_ptr<RawDataCollectedBase> m_rawDataCollected; /**< contains pointer to the data container taking care of collecting the raw data. */
    public:

      /** overloaded '=='-operator */
      bool operator==(const SubGraph<FilterType>& b) const
      { return (m_id == b.m_id); }

      /** for better readability. */
      using Iterator = typename std::unordered_map<FilterType, MinMax>::iterator;

      /** constructor, mandatory iDChain musst at least contain one iD. */
      SubGraph(SubGraphID& id, std::vector<FilterType>& fIDs) : m_id(id), m_found(1), m_rawDataCollected(nullptr)
      {
        for (auto& iD : fIDs) {
          m_minMaxValues.insert({iD, MinMax()});
        }
      }

      /// destructor
      ~SubGraph() { if (m_rawDataCollected != nullptr) delete m_rawDataCollected; }

      /** add filter, if not added yet, checks value and replaces old ones if new one is better. returns true if minMax was updated. */
      bool checkAndReplaceIfMinMax(FilterType fID, double value)
      {
        Iterator pos = m_minMaxValues.find(fID);
        if (pos == m_minMaxValues.end()) {
          B2WARNING("FilterID " << fID << " not known to this subgraph " << m_id.print() << " - nothing added!");
          return false;
        }
        return pos->second.checkAndReplaceIfMinMax(value);
      }

      void wasFound() { m_found++; } /**< increases found-counter. */

      unsigned getFound() const { return m_found; } /**< returns found-counter. */

      /** if both graphs have got the same IDs except the last one, they share a trunk. */
      bool checkSharesTrunk(const SubGraph<FilterType>& b) const { return (m_id.checkSharesTrunk(b.m_id)); }

      std::string print() const
      {
        std::string out = "id: " + m_id.print() + " was found " + std::to_string(m_found) + " times. Filters with values collected:\n";
        for (const auto& entry : m_minMaxValues) {
          out += entry.first + " " + entry.second.print() + " _||_ ";
        }
        return std::move(out);
      }

      /** returns iD of this graph*/
      const SubGraphID& getID() const { return m_id; }

      /// takes care of being able to use the data collector.
      void prepareDataCollection(std::pair<double, double> quantiles)
      {
        if (m_rawDataCollected != nullptr) delete m_rawDataCollected;
        m_rawDataCollected = new std::unordered_map<FilterType, RawDataCollectedMinMax>();

        for (const auto& entry : m_minMaxValues) {
          m_rawDataCollected->insert({entry.first, RawDataCollectedMinMax(m_found, quantiles)});
        }
      }

      /// adds value to rawDataCollector
      void addValue(FilterType fID, double value)
      {
        if (m_rawDataCollected == nullptr) { B2FATAL("SubGraph::addValue: attempt to add data before prepareDataCollection(...) was executed! Aborting illegal procedure."); }
        // TODO bau um auf m_rawDataCollected!
        auto pos = m_rawDataCollected->find(fID);
        if (pos == m_rawDataCollected->end()) {
          B2WARNING("addValue: FilterID " << fID << " not known to this subgraph " << m_id.print() << " - nothing added!");
          return;
        }
        pos->second.add(value);
        return;
      }

      /// this deletes the old min and max values stored and replaces them with the quantiles to be found.
      const std::unordered_map<FilterType, MinMax>& getFinalQuantileValues()
      {
        for (const auto& entry : *m_rawDataCollected) {
          std::pair<double, double> results = entry.second.getMinMax();
          checkAndReplaceIfMinMax(entry.first, results.first);
          checkAndReplaceIfMinMax(entry.first, results.second);
        }
        return m_minMaxValues;
      }

    };



    /** contains all subgraphs. */
    template<class FilterType>  class SectorGraph {
    protected:
      std::map<SubGraphID, SubGraph<FilterType>> m_subgraphs; /**< contains all subgraphs. */

      std::vector<FilterType> m_filterIDs; /**< ids of all filterTypes to be stored by subGraphs. */
    public:

      /** constructor expects filterIDs. */
      SectorGraph(std::vector<FilterType>& fIDs) : m_filterIDs(fIDs)
      { if (m_filterIDs.empty()) { B2FATAL("SectorGraph-constructor: passed filterIDs are empty, this is illegal usage of this class!") } }

      /** for better readability. */
      using Iterator = typename std::map<SubGraphID, SubGraph<FilterType>>::iterator;

      /** find entry. returns end() if not found. */
      Iterator find(SubGraphID idChain) { return m_subgraphs.find(idChain); }

      /** returns begin of subGraphs. */
      Iterator begin() { return m_subgraphs.begin(); }

      /** returns end of subGraphs. */
      Iterator end() { return m_subgraphs.end(); }

      /** returns number of collected subgraphs so far. */
      unsigned size() const { return m_subgraphs.size(); }

      /** returns number of occurances for all subGraphs found together. */
      unsigned long nFoundTotal() const
      {
        unsigned long nFound = 0;
        for (auto& pack : m_subgraphs) { nFound += pack.second.getFound(); }
        return nFound;
      }

      /** add new subgraph if not added already. */
      Iterator add(SubGraphID& newID)
      {
        if (m_subgraphs.find(newID) != end())
        { B2WARNING("SectorGraph::add: given ID " << newID.print() << " is already in graph, not added again..."); return end(); }
        std::pair<Iterator, bool> pos = m_subgraphs.insert({newID, SubGraph<FilterType>(newID, m_filterIDs)});
        B2DEBUG(1, "SectorGraph::add: new subgraph added: " << pos.first->second.print())
        return pos.first;
      }

      /** returns a string giving an overview of the graph. */
      std::string print() const
      {
        std::string out = "graph has got " + std::to_string(m_subgraphs.size()) + " entries:\n";
        for (const auto& entry : m_subgraphs) {
          out += entry.second.print() + "\n";
        }
        return std::move(out);
      }

      /** returns removed occurances. */
      unsigned pruneGraph(double rarenessCut)
      {
        //sanity checks:
        if (rarenessCut < 0 or rarenessCut >= 1)
        { B2WARNING("pruneGraph: rarenessCut is rubbish: " << rarenessCut << ", stopping prune-process."); return 0; }
        if (rarenessCut == 0) { B2DEBUG(1, "pruneGraph: nothing to be done, stopping prune-process."); return 0; }

        /// vector of trunks:
        // .first counts total number of occurances of branches of this trunk
        // .second is a vector of pointers to the subgraphs of that trunk.
        std::vector< std::pair<unsigned, std::vector<SubGraph<FilterType>*> >> trunks;

        // find those sharing a trunk (trunkTotal) and cluster them in trunks:
        for (auto& subGraphEntry : m_subgraphs) {
          SubGraph<FilterType>& graph = subGraphEntry.second;
          bool found = false;
          for (auto& trunk : trunks) {
            if (graph.checkSharesTrunk(*(trunk.second.at(0)))) {
              trunk.first += graph.getFound();
              trunk.second.push_back(&graph);
              found = true;
              continue;
            }

            if (found) continue;
            trunks.push_back({graph.getFound(), {&graph} });
          }
        }

        unsigned long nFoundB4 = nFoundTotal(), nKilled = 0;
        unsigned sizeb4 = size();
        B2DEBUG(1, "pruneGraph - before pruning: graph of size " << sizeb4 << " has " << trunks.size() << " trunks with " << nFoundB4 <<
                " total found.")

        // collect subGraphs (branches) to be deleted:
        std::vector<SubGraph<FilterType>*> deadBranches;
        for (auto& trunk : trunks) {
          double trunkCut = rarenessCut * double(trunk.first);

          // sort branches of trunk by rareness (rarest one first):
          std::sort(trunk.second.begin(),
                    trunk.second.end(),
                    [](const SubGraph<FilterType>* a, const SubGraph<FilterType>* b)
                    -> bool { return a->getFound() < b->getFound(); });

          // skip trunk, if there are no rare branches.
          auto pos = trunk.second.begin();
          if (double((**pos).getFound()) >= trunkCut) continue;

          // collect branches to be killed starting with the smallest one and stop when reaching those which have been slipping above the threshold in the process:
          while (pos != trunk.second.end()) {
            // mark the most rarest entry:
            deadBranches.push_back(*pos);
            trunkCut -= double((**pos).getFound()) * rarenessCut;

            // reached the point when all graphs left are now good enough for us: -> stopping loop.
            if (double((**pos).getFound()) >= trunkCut) break;
            ++pos;
          }
        } // looping over trunks

        if (deadBranches.empty()) { B2DEBUG(1, "pruneGraph: no rare branches found - stopping pruning process."); return 0; }

        // kill selected ones:
        for (auto* graph : deadBranches) {
          nKilled += graph->getFound();
          m_subgraphs.erase(graph->getID());
        }

        B2DEBUG(1, "pruneGraph - after pruning graph with size (before/after " << sizeb4 << "/" << size() <<
                ") and nFound (before/after/killed " << nFoundB4 << "/" << nFoundTotal() << "/" << nKilled)

        return nKilled;
      }

    };



    std::vector<unsigned> getSecIDs(std::vector<BranchInterface<unsigned>>& secBranches, Long64_t entry)
    {
      std::vector<unsigned> iDs;
      for (auto& branch : secBranches) {
        branch.update(entry);
        iDs.push_back(branch.value);
      }
      return std::move(iDs);
    }


    /// train graph
    template <class FilterType>
    void trainGraph(
      SectorGraph<FilterType>& mainGraph,
      std::unique_ptr<TChain>& chain,
      std::vector<BranchInterface<unsigned>>& sectorBranches,
      std::vector<BranchInterface<double>>& filterBranches)
    {
      auto nEntries = chain->GetEntries();
      B2DEBUG(10, "RawSecMapMerger::trainGraph():  start of " << nEntries << " entries in tree and " << sectorBranches.size() <<
              " branches")
      if (nEntries == 0) { B2WARNING("trainGraph: valid file but no data stored!"); return; }

      auto percentMark = nEntries / 10; auto progressCounter = 0;
      // log all sector-combinations and determine their absolute number of appearances:
      for (auto i = 0 ;  i <= nEntries; i++) {
        if ((i % percentMark) == 0) {
          progressCounter += 10;
          B2INFO("RawSecMapMerger::trainGraph(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
                 "% related, mainGraph has got " << mainGraph.size() << " sectors...")
        }
        auto thisEntry = chain->LoadTree(i);

        auto ids = getSecIDs(sectorBranches, thisEntry);
        auto currentID = SubGraphID(ids);

        auto pos = mainGraph.find(currentID);

        if (pos == mainGraph.end()) { B2WARNING("trainGraph: could not find subgraph " << currentID.print() << " - skipping entry..."); continue; }

        for (auto& filter : filterBranches) {
          filter.update(thisEntry);
          pos->second.addValue(FilterType(filter.name), filter.value);
        }
      } // entry-loop-end
    }


    template <class FilterType>
    SectorGraph<FilterType> buildGraph(
      std::unique_ptr<TChain>& chain,
      std::vector<BranchInterface<unsigned>>& sectorBranches,
      std::vector<BranchInterface<double>>& filterBranches)
    {
      auto nEntries = chain->GetEntries();
      B2INFO("RawSecMapMerger::buildGraph():  start of " << nEntries << " entries in tree and " << sectorBranches.size() <<
             " branches")

      // creating main graph containing all subgraphs:
      std::vector<std::string> filterNames;
      for (auto& entry : filterBranches) { filterNames.push_back(entry.name); }
      SectorGraph<FilterType> mainGraph(filterNames);

      if (nEntries == 0) { B2WARNING("buildGraph: valid file but no data stored!"); return std::move(mainGraph); }
      auto percentMark = nEntries / 10;
      auto progressCounter = 0;
      // log all sector-combinations and determine their absolute number of appearances:
      for (auto i = 0 ;  i <= nEntries; i++) {
        if ((i % percentMark) == 0) {
          progressCounter += 10;
          B2INFO("RawSecMapMerger::buildGraph(): with mark: " << percentMark << " and i=" << i << ", " << progressCounter <<
                 "% related, mainGraph has got " << mainGraph.size() << " sectors...")
        }
        auto thisEntry = chain->LoadTree(i);

        auto ids = getSecIDs(sectorBranches, thisEntry);
        auto currentID = SubGraphID(ids);
        B2DEBUG(10, "buildGraph-SubGraphID-print: id: " << currentID.print());

        auto pos = mainGraph.find(currentID);

        if (pos == mainGraph.end()) { pos = mainGraph.add(currentID); }

        if (pos == mainGraph.end()) { B2WARNING("could not find nor add subgraph - skipping entry..."); continue; }

        pos->second.wasFound();

        for (auto& filter : filterBranches) {
          filter.update(thisEntry);
          pos->second.checkAndReplaceIfMinMax(FilterType(filter.name), filter.value);
        }
      } // entry-loop-end

      B2INFO("RawSecMapMerger::buildGraph(): mainGraph finished - has now size: " << mainGraph.size())
      B2DEBUG(1, "fullGraph-Print: " << mainGraph.print());

      return std::move(mainGraph);
    }



    /// WARNING TODO: Filter-> String to big->unsigned is better (or FilterID)
    void processSectorCombinationsV2(TrainerConfigData& config, unsigned secChainLength)
    {
      B2DEBUG(1, "processSectorCombinationsV2: training map " << config.secMapName << " with secChainLength: " << secChainLength)

      // branch-names sorted from outer to inner:
      std::vector<std::string> secBranchNames;
      // filter names are different for different secChainLengths:
      std::vector<std::string> filterBranchNames;
      std::string nHit = prepareNHitSpecificStuff(secChainLength, config, secBranchNames, filterBranchNames);

      // contains the raw data
      std::unique_ptr<TChain> chain = createTreeChain(config, nHit);

      // prepare links to branches branches:
      std::vector<BranchInterface<unsigned>> sectorBranches = getBranchesV2<unsigned>(chain, secBranchNames);
      std::vector<BranchInterface<double>> filterBranches = getBranchesV2<double>(chain, filterBranchNames);

      // for debugging: print data for crosschecks:
      printDataV2(chain, sectorBranches, filterBranches);

      SectorGraph<std::string> mainGraph = buildGraph<std::string>(chain, sectorBranches, filterBranches);

      unsigned nKilled = mainGraph.pruneGraph(config.rarenessThreshold);

      B2DEBUG(1, "processSectorCombinationsV2: nKilled " << nKilled)

      for (auto& subgraph : mainGraph) {
        subgraph.second.prepareDataCollection(config.quantiles); // TODO small-sample-case!
      }

      trainGraph(mainGraph, chain, sectorBranches, filterBranches);
      /// TODO next steps:
      // selectDataContainer!
      /**
       * 404 bytes per graph max (no trainer-values) + 3 (id in mainGraph)
       * 100.000 graphs max
       * -> 40.700.000 ~ 40 MB for empty mainGraph containing 100k subgraphs.
       * per subgraph space available: (2GB - 40 MB) / 100.000 = 20 MB per graph. (double)-> 2.500.000 values max. / 20 Filters
       * 125k value-sets max.
       * */

      B2INFO("processSectorCombinationsV2: training finished.\n" << mainGraph.print();)
    }



    /** pseudo code */
    void doALLTEMP()
    {
      StoreObjPtr< SectorMap > sectorMap("", DataStore::c_Persistent);
      B2INFO("RawSecMapMerger::doALLTEMP():");

      // loop over all the setups in the sectorMap:
      for (auto setup : sectorMap->getAllSetups()) {

        auto config = setup.second->getConfig();
        B2INFO("RawSecMapMerger::doALLTEMP(): loading mapName: " << config.secMapName);

        if (config.secMapName != "lowTestRedesign") { continue; } // TODO WARNING DEBUG we do not want to run more than one run yet!

        processSectorCombinationsV2(config, 2);

        return; // TODO WARNING DEBUG we do not want to run more than one run yet!
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
