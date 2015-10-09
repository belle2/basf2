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
// stl:
#include <string>
#include <vector>
#include <deque>
#include <set>
#include <list>
#include <map>
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



    /** proto-class for sectors allowing some simple relevant things. */
    template<class DataCollectorType>
    class ProtoSector {
    protected:
      /** contains all innerSectors found (can be used iteratively for 3- and 4-sector-chains). */
      std::vector<ProtoSector<DataCollectorType>> m_innerSectors;
//    std::set<ProtoSector<DataCollectorType>> m_innerSectors;


      /** contains the ID stored in the Ttrees. */
      unsigned m_rawID;


      /** stores final FullSecID. */
      FullSecID m_finalID;


      /** counts the number of times this Sector appeared in the trainingsample. */
      unsigned m_nTimesFound;


      /** stores the min and the max quantiles for this sector. */
      std::pair<double, double> m_minMaxQuantiles;


      /** .first: name/id of filter. second a container storing the collected data sample. has to fulfill the requirements of MinMaxCollector.
       * This data has to be stored at the innermost of the chain of sectors. */
      std::map<std::string, DataCollectorType> m_rawDataForFilter;


      /** will be locked after setting the finalID. */
      bool m_isLocked;


      /** contains the finalCuts when determineCutsAndCleanRawData() was called before, is empty if not.
       * key is the filterName,
       * value.first is lower cut (if any) and
       * value.second is upper cut (if any). */
      std::map<std::string, std::pair<double, double>> m_finalCuts;


      /** enforces the correct usage of the class. */
      bool checkLocked(std::string funcName) const
      {
        if (m_isLocked) {
          B2ERROR("attempting to modify the sector " << std::string(m_finalID)
                  << " after it was locked! Function " << funcName
                  << " will be aborted!")
          return true;
        }
        return false;
      }

    public:


      /** shortcut for easier readibility. */
      using Sector = ProtoSector<DataCollectorType>;

      /** Typedef for inner-sector-iterator. */
      using Iterator = typename std::vector<Sector>::iterator;


      /** overloaded '=='-operator */
      bool operator==(const Sector& secB) const { return m_rawID == secB.m_rawID; }
      bool operator==(const unsigned& rawID) const { return m_rawID == rawID; }


      /** overloaded '<'-operator for sorting algorithms */
      bool operator<(const Sector& secB)  const { return m_rawID < secB.m_rawID; }


      /** Constructor, add rawID. */
      ProtoSector(unsigned rawID) :
        m_rawID(rawID),
        m_nTimesFound(1), // first time found when creation is triggered
        m_isLocked(false) {}


      /** sets the FullSecID and locks the object against further secID-changes. */
      void setFullSecID(bool upgradeSubLayer)
      {
        if (checkLocked("setFullSecID")) { return; }
        FullSecID tempID = FullSecID(m_rawID);
        m_finalID = FullSecID(tempID.getVxdID(), upgradeSubLayer, tempID.getSecID());
        m_isLocked = true;
      }


      /** returns iterator position of inner Sector matching rawID, returns end() if nothing was found. */
      Iterator find(unsigned rawID)
      { return std::find(m_innerSectors.begin(), m_innerSectors.end(), rawID); }


      /** add a new inner sector. */
      void addInnerSector(unsigned rawID)
      {
//    auto foundPos = m_innerSectors.find(rawID);
        auto foundPos = find(rawID);
        if (foundPos == end()) {
          m_innerSectors.push_back(Sector(rawID));
//      m_innerSectors.insert(Sector(rawID));
          return;
        }
        B2WARNING("addInnerSector: attempt of adding sector with ID " << FullSecID(rawID).getFullSecString() << "twice - sector not added!")
      }


      /** creates RawDataContainers in the this Sector. */
      unsigned prepareRawDataContainers(std::pair<double, double>& quantiles, std::vector<std::string>& filterIDs)
      {
        /// /////////////////////////////////////////////////////////////////////////////////////////////////// TODO what about small-quantiles?
        m_minMaxQuantiles = quantiles;
        unsigned nFiltersAdded = 0;
        // just add some safety-margin:
        double maxQuantileWithPuffer = m_minMaxQuantiles.first + m_minMaxQuantiles.second + 0.005;
        for (auto& name : filterIDs) {
          m_rawDataForFilter[name] = DataCollectorType(maxQuantileWithPuffer);
          ++nFiltersAdded;
        }
        return nFiltersAdded;
      }


      /** for given rawID find the corresponding inner sector, returns nullptr if nothing was found. */
      Sector* getInnerSector(unsigned rawID)
      {
//    auto foundPos = m_innerSectors.find(rawID);
        auto foundPos = find(rawID);
        return (foundPos == end()) ? nullptr : &(*foundPos);
      }

//    /** collects all FullSecIDs found for given sensor in all sectors and their inner chain-mates. */
//    std::vector<FullSecID> getAllSecIDofSensor (VxdID thisSensor)
//    {
//    std::vector<FullSecID> allSecIDs;
//    for (auto& sector : m_innerSectors) {
//      sector.getAllFullSecIDsOfSensor(thisSensor, allSecIDs);
//    }
//    return std::move(allSecIDs);
//    }
//
//
//    /** returns true if given rawID is part of the main sectors. */
//    bool isInMap(unsigned rawID)
//    {
//    Iterator pos = std::find(begin(), end(), rawID);
//    return ((getSector(rawID) == nullptr) ? false : true);
//    }

      /** for c++11-like for loops: begin of inner sectors. */
      Iterator begin() { return m_innerSectors.begin(); }


      /** for c++11-like for loops: end of inner sectors. */
      Iterator end() { return m_innerSectors.end(); }


      /** returns raw ID. */
      unsigned getRawSecID() const { return m_rawID; }


      /** returns final FullSecID if setFullSecID was already called and converted rawID if not. */
      FullSecID getFullSecID() const
      {
        if (!m_isLocked) {
          B2WARNING("only FullSecID(rawID) is returned, subLayerID might be wrong!");
          return FullSecID(m_rawID);
        }
        return m_finalID;
      }


      /** add value for given filter, return true if successful, false if not. */
      bool addRawDataValue(std::string filterID, double value)
      {
        auto pos = m_rawDataForFilter.find(filterID);
        if (pos != m_rawDataForFilter.end()) { pos->second.append(value); return true; }
        B2ERROR("ProtoSector::addRawDataValue: attempted to add value " << value <<
                " to filter " << filterID <<
                ": the latter was not existing - adding did not work!")
        return false;
      }


      /** returns number of times when there was a problem. */
      unsigned determineCutsAndCleanRawData()
      {
        unsigned nBadCases = 0;
        // sanity check:
        if (m_rawDataForFilter.empty() and m_innerSectors.empty()) { B2WARNING("no rawData!"); return 1; }

        // will not be executed when empty:
        for (auto& innerSector : m_innerSectors) {
          nBadCases += innerSector.determineCutsAndCleanRawData();
        }

        // will not be executed when empty:
        for (auto& entry : m_rawDataForFilter) {
          if (entry.second.empty()) { B2WARNING("TODO..."); nBadCases += 1; }
          auto cuts = entry.second.getMinMax(m_minMaxQuantiles.first, 1. - m_minMaxQuantiles.second);
          m_finalCuts.insert({entry.first, cuts});
          // deleting raw data now:
          entry.second.clear();
        }
        m_rawDataForFilter.clear();

        return nBadCases;
      }


      /** returns true if this sector has finalCuts. */
      bool hasFinalCuts() const { return !m_finalCuts.empty(); }


      /** loop over all inner sectors recursively to collect the FullSecIDs matching given VxdID. (quasi recursive) */
      void getAllFullSecIDsOfSensor(VxdID thisSensor, std::vector<FullSecID>& allSecIDs) const
      {
        if (!m_isLocked) {
          B2ERROR("getAllFullSecIDsOfSensor: trying to get final Id but Sector "
                  << std::string(FullSecID(m_rawID))
                  << " has no final FullSecID yet! abborting...");
        }

        if (getFullSecID().getVxdID() == thisSensor)
        { allSecIDs.push_back(getFullSecID()); return; } // no 2sectors on same sensor in a sector-chain!

        for (auto& innerSector : m_innerSectors) {
          getAllFullSecIDsOfSensor(thisSensor, allSecIDs);
        }
      }


      /** if combination does not exist yet, it will get added to the graph (returns true), if it exists, simply counters are increased (returns false). */
      bool addRelationToGraph(Long64_t thisEntry, // of the tree/chain
                              std::deque<std::pair<TBranch*, unsigned>>
                              sectorBranches) // .first pointer to branch, .second value of branch for given thisEntry. Hardcopy because of recursive approach.
      {
        // case: we are finished:
        if (sectorBranches.empty()) return false;

        // set correct entry:
        sectorBranches[0].first->GetEntry(thisEntry);
        unsigned currentID = sectorBranches[0].second;

        // case: it is not this one and and none of the inner sectors:
        if (currentID != m_rawID and find(currentID) == end()) {
          addInnerSector(currentID);
          sectorBranches.pop_front();
        }

        // now only two cases left to capture: isThisSector or isInnerSector (both at the same time is not possible).
        // case: is this sector -> is not inner Sector:
        if (currentID == m_rawID) {
          wasFound();
          sectorBranches.pop_front();
        }

        // case: is not this sector -> is innerSector:
        if (currentID != m_rawID) { /* nothing has to be done, so just continue */ }

        for (auto& innerSector : m_innerSectors) {
          innerSector.addRelationToGraph(thisEntry, sectorBranches);
        }
        return true;
      }


      /** check each inner graph for its rareness and delete those which are too rare. repeats that recursively for each inner subGraph. */
      unsigned pruneGraph(double rarenessThreshold)
      {
        if (rarenessThreshold <= 0.) return 0;
        if (size() == 0) return 0;

        unsigned nSectorsKilled = 0;
        // want to collect all innerSectors which lie below threshold:
        std::deque<Sector*> rareInnerGraphs;

        double totalSampleSize = double(m_nTimesFound);
        for (Sector& innerGraph : m_innerSectors) {
          if (totalSampleSize * rarenessThreshold > double (innerGraph.getNTimesFound())) {
            rareInnerGraphs.push_back(&innerGraph);
          }
        }

        // want to have the rarest combination first:
        std::sort(rareInnerGraphs.begin(),
                  rareInnerGraphs.end(),
                  [](const Sector * a, const Sector * b) -> bool { return a->getNTimesFound() < b->getNTimesFound(); });

        // counts how many occurences had to be removed:
        unsigned removedSamples = 0;

        // kill innerGraphs starting with the smallest one and stop when reaching those which have been slipping above the threshold in the process:
        while (!rareInnerGraphs.empty()) {
          // do the housekeeping:
          unsigned nTimesFound = rareInnerGraphs.front()->getNTimesFound();
          removedSamples += nTimesFound;
//      double dFound = double(nTimesFound);
          unsigned rawID = rareInnerGraphs.front()->getRawSecID();

          // remove the most rarest entry:
          rareInnerGraphs.pop_front();
          deleteInnerSector(rawID);
          totalSampleSize -= nTimesFound; // deleting reduces the sample size
          nSectorsKilled++;

          // reached the point when all graphs left are now good enough for us: -> stopping loop.
          if (double(rareInnerGraphs.front()->getNTimesFound()) > totalSampleSize * rarenessThreshold) {
            rareInnerGraphs.clear();
          }
        }

        m_nTimesFound -= removedSamples;
        // now repeat the process for the remaining inner graphs (so they can prune themselves):
        for (auto& innerGraph : m_innerSectors) {
          nSectorsKilled += innerGraph.pruneGraph(rarenessThreshold);
        }
        return nSectorsKilled;
      }


      /** increases the counter for the nTimesFound. */
      void wasFound() { m_nTimesFound++; }


      /** returns how often this sector was found. */
      unsigned getNTimesFound() const { return m_nTimesFound; }


      /** returns true if given rawID is part of the next level of inner sectors. */
      bool isPartOfNextInLine(unsigned rawID)
      { return ((getInnerSector(rawID) == nullptr) ? false : true); }


      /** the sector matching given ID will be deleted. returns true if it could be deleted. */
      bool deleteInnerSector(unsigned rawID)
      {
        Iterator pos = find(rawID);
        if (pos == end()) return false;
        m_innerSectors.erase(pos);
        return true;
      }


      /** returns the number of innerGraphs collected. */
      unsigned size() const { return m_innerSectors.size(); }


      /** collect raw data of each sector-combination and find cuts */
      void distillRawData4FilterCuts(
        Long64_t thisEntry,
        TrainerConfigData& config,
        std::deque<std::pair<TBranch*, unsigned>> sectorBranches, // hardcopy, since it will be pruned during the process.
        std::deque<std::pair<TBranch*, double>>& filterBranches)
      {
        // case: this graph is walked through:
        if (sectorBranches.empty()) { return; }

        // update branch-entry:
        sectorBranches[0].first->GetEntry(thisEntry);

        // case: we are in the wrong graph:
        if (sectorBranches[0].second != getRawSecID()) { return; }

        // case: we have found the innermost Sector of the graph we wanted:
        if (sectorBranches.size() == 1) {

          // prepare for collection if not done yet:
          if (m_rawDataForFilter.empty()) {
            prepareRawDataContainers(config.quantiles, config.twoHitFilters);
          }

          // collect values for filtering:
          unsigned nFilters = filterBranches.size();
          for (unsigned fPos = 0; fPos < nFilters; fPos++) {
            filterBranches[fPos].first->GetEntry(thisEntry);
            addRawDataValue(
              config.twoHitFilters[fPos],
              filterBranches[fPos].second);
          }

          return;
        }

        // case: we might be in the correct graph (so far so good), but this is not the end yet -> continue looping through the next sectors in chain:
        sectorBranches.pop_front();
        for (Sector& subGraph : m_innerSectors) {
          subGraph.distillRawData4FilterCuts(thisEntry, config, sectorBranches, filterBranches);
        }
      }


      /** for each outerSecID:
       *  if outerSecID got innerSectors on same layer:
       *     -> subLayerID == 1 (for all cases: protoSector.setFullSecID(true/false))
       *     storeOuterSecID in container
       * for each newiD in container:
       *   for each innerSector of outerSecID:
       *     if innerSector == newiD: ->subLayerID == 1 (for all cases: protoSector.setFullSecID(true/false))
       * returns nsectorsRenamed.  */
      unsigned updateSubLayers()
      {
        unsigned nUpdated = 0;
        FullSecID myTempID(getRawSecID());
        for (auto& innerSector : m_innerSectors) {
          FullSecID innerTempID(innerSector.getRawSecID());

          // case: does not have to be updated:
          if (myTempID.getLayerNumber() != innerTempID.getLayerNumber()) {
            nUpdated += innerSector.updateSubLayers();
            continue;
          }

          // case: was already updated:
          if (myTempID.getSubLayerID()) { continue; }
          setFullSecID(true);
          nUpdated++;
        }

        if (!m_isLocked) {setFullSecID(false); }
        return nUpdated;
      }
    }; // end of ProtoSector-declaration.





    /** loads configuration for given parameter mapName */
    TrainerConfigData getConfig(std::string)
    {
      // TODO
      // INFO: this will be a parameter or loaded from a csvFile/database-entry:
      return TrainerConfigData();
    }



    /** returns all names of root-files fitting given parameter mapName  */
    std::vector<std::string> getRootFiles(std::string)
    {
      // TODO
      // INFO: root files will contain the mapName and some extensions: mapName_rngNumber.root
      // in current directory read all files matching this.
      return {"fName1", "fname2", "fName3"};
    }



    /** bundle all relevant files to a TChain */
    std::unique_ptr<TChain> createTreeChain(TrainerConfigData& configuration, std::string nHitString)
    {
      std::unique_ptr<TChain> treeChain = std::unique_ptr<TChain>(new TChain((configuration.secMapName + nHitString).c_str()));

      // dummy code yet:
      auto fileList = getRootFiles(configuration.secMapName);
      for (auto file : fileList) { /*treeChain.add(file);*/ }

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
      branches.clear();
      unsigned nBranches = branchNames.size();

      branches.resize(nBranches, {nullptr, ValueType(0)});
//    std::deque<std::pair<TBranch*, double>> branches(nBranches, {nullptr, 0});
      for (unsigned fPos = 0; fPos < nBranches; fPos++) {
        chain->SetBranchAddress(
          branchNames[fPos].c_str(),
          &(branches[fPos].second),
          &(branches[fPos].first));
      }
    }



    /** builds a graph of related sectors by using training data. */
    ProtoSector<MinMaxCollector<double>> createRelationSecMap(
                                        std::unique_ptr<TChain>& chain,
                                        std::deque<std::pair<TBranch*, unsigned>>& sectorBranches)
    {
      // creating master sector containing all inner sectors and therefore the full graph in the end:
      ProtoSector<MinMaxCollector<double>> fullRawSectorGraph(std::numeric_limits<unsigned>::max());

      // log all sector-combinations and determine their absolute number of appearances:
      for (int i = 0 ; chain->GetEntries(); i++) {
        auto thisEntry = chain->LoadTree(i);

        // if combination does not exist yet, it will get added to the graph, if it exists, simply counters are increased.
        fullRawSectorGraph.addRelationToGraph(thisEntry, sectorBranches);
      }

      return std::move(fullRawSectorGraph);
    }



    /** takes a relationMap and checks for each outer sector:
     *   how often did it occur (-> mainSampleSize)
     * for each inner sector:
     *   how often did it occur (-> innerSampleSize)
     * sort innerSectors by rareness (rarest one comes first):
     *   if innerSampleSize is below threshold -> remove, update mainSampleSize
     * Produces a Map of ProtoSectors which contain innerSectors
     * (ProtoSectors which are no innerSector and have no innerSectors themselves will not be stored).  */
    /** use rareness-threshold to find sector-combinations which are very rare and remove them: */
    unsigned pruneMap(double rarenessThreshold, ProtoSector<MinMaxCollector<double>>& relationsGraph)
    {
      unsigned nSectorsKilled = 0;

      // TODO one could also remove subGraphs here (the pruneGraph-function is recursive), but maybe we want to have those seldom cases too.
      for (auto& subGraph : relationsGraph) {
        nSectorsKilled += relationsGraph.pruneGraph(rarenessThreshold);
      }
      return nSectorsKilled;
    }



    /** get the raw data and determine the cuts for the filters/selectionVariables: */
    void distillRawData4FilterCuts(
      std::unique_ptr<TChain>& chain,
      TrainerConfigData& config,
//    std::deque<std::pair<TBranch*, double>> filterBranches,
      std::deque<std::pair<TBranch*, unsigned>>& sectorBranches,
      ProtoSector<MinMaxCollector<double>>& relationsGraph)
    {
      // prepare the links for the filter-value-branches:
      std::deque<std::pair<TBranch*, double>> filterBranches;
      getBranches<double>(chain, config.twoHitFilters, filterBranches);


      // loop over the tree to find the entries matching this outerSector:
      for (int i = 0 ; chain->GetEntries(); i++) {
        auto thisEntry = chain->LoadTree(i);
        // collect raw data of each sector-combination and find cuts:
        for (auto& graph : relationsGraph) {
          graph.distillRawData4FilterCuts(thisEntry, config, sectorBranches, filterBranches);
        }
      } // leaves-loop

      // find cuts of each sector-combination:
      for (auto& outerSector : relationsGraph) {
        // after collecting all data, store only the cuts to keep ram consumption down:
        unsigned nBadCases = outerSector.determineCutsAndCleanRawData();
        if (!nBadCases) { B2WARNING("TODO... nBadCases: " << nBadCases); }
      }

    }




    /** does everything needed for given chainLength of sectors (e.g.: 2 -> twoHitFilters)*/
    void processSectorCombinations(TrainerConfigData& configuration, unsigned secChainLength)
    {
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
      std::vector<std::string> branchNames = { "outerSecID", "innerSecID"};
      /// ////////////////////////////////////       WARNING hardcoded bla end.
      // TODO WARNING: check all following functions for hardcoded twoHitFilters-stuff -> fix!

      std::unique_ptr<TChain> chain = createTreeChain(configuration, nHit);

      // prepare links to sector branches:
      std::deque<std::pair<TBranch*, unsigned>> sectorBranches;
      getBranches<unsigned>(chain, branchNames, sectorBranches);

      // create proto-graph containing all sector-chains occured in the training sample.
      ProtoSector<MinMaxCollector<double>> relationsGraph = createRelationSecMap(chain, sectorBranches);

      // use rareness-threshold to find sector-combinations which are very rare and remove them:
      unsigned nSectorsRemoved = pruneMap(configuration.rarenessThreshold, relationsGraph);

      // get the raw data and determine the cuts for the filters/selectionVariables:
      distillRawData4FilterCuts(chain, configuration, sectorBranches, relationsGraph);

      // checks for sectors which have inner neighbour and updates the sublayerID of the sensors.
      auto nsectorsRenamed = relationsGraph.updateSubLayers();
      B2DEBUG(1, "results ... TODO" << nSectorsRemoved << " " << nsectorsRenamed)


      /***
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

      // loop over sectorMaps:
      for (std::string& mapName : m_PARAMmapNames) {

        auto config = getConfig(mapName);

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
// // // //     * Produces a Map of ProtoSectors which contain innerSectors
// // // //     * (ProtoSectors which are no innerSector and have no innerSectors themselves will not be stored).  */
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
// // // //     ProtoSectorMap<MinMaxCollector> protoMap = pruneMap(config.rarenessThreshold, std::move(relationsMap));
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
