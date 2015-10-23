/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/FullSecID.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/logging/Logger.h>

#include <string>
#include <vector>
#include <utility> // std::pair, std::move
#include <functional>
#include <limits>       // std::numeric_limits
#include <memory> // std::unique_ptr


namespace Belle2 {

  /** proto-class for sectors allowing some simple relevant things. */
  template<class DataCollectorType>
  class ProtoSectorGraph {
  protected:
    /** contains all innerSectors found (can be used iteratively for 3- and 4-sector-chains). */
    std::vector<ProtoSectorGraph<DataCollectorType>> m_innerSectors;
    //    std::set<ProtoSectorGraph<DataCollectorType>> m_innerSectors;


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
    /** loops over all branches to update their entries. */
    template <class ValueType>
    static void updateBranchEntries(Long64_t thisEntry,
                                    std::deque<std::pair<TBranch*, ValueType>>& sectorBranches)
    {
      std::string out;
      for (std::pair<TBranch*, ValueType>& branchPack : sectorBranches) {
        branchPack.first->GetEntry(thisEntry);
        out += FullSecID(branchPack.second).getFullSecString() + " ";
      }
      B2DEBUG(1, "updateBranchEntries: secBranches carries the following sectors: " << out);
    }


    /** shortcut for easier readibility. */
    using Sector = ProtoSectorGraph<DataCollectorType>;

    /** Typedef for inner-sector-iterator. */
    using Iterator = typename std::vector<Sector>::iterator;


    /** overloaded '=='-operator */
    bool operator==(const Sector& secB) const { return m_rawID == secB.m_rawID; }
    bool operator==(const unsigned& rawID) const { return m_rawID == rawID; }


    /** overloaded '<'-operator for sorting algorithms */
    bool operator<(const Sector& secB)  const { return m_rawID < secB.m_rawID; }


    /** Constructor, add rawID. */
    ProtoSectorGraph(unsigned rawID) :
      m_rawID(rawID),
      m_nTimesFound(1), // first time found when creation is triggered
      m_isLocked(false) { B2DEBUG(100, "creating secGraph with sector " << std::string(FullSecID(rawID))); }


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
    Iterator addInnerSector(unsigned rawID)
    {
      //    auto foundPos = m_innerSectors.find(rawID);
      if (rawID == m_rawID) {
        B2WARNING("addInnerSector: given ID " << FullSecID(rawID).getFullSecString() <<
                  " is identical with ID of this sector - can not add sector to itself!");
        return --(end());
      }
      auto foundPos = find(rawID);
      if (foundPos == end()) {
        m_innerSectors.push_back(Sector(rawID));
        //      m_innerSectors.insert(Sector(rawID));
        return --(end());
      }
      B2WARNING("addInnerSector: attempt of adding sector with ID " << FullSecID(rawID).getFullSecString() << "twice - sector not added!")
      return foundPos;
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


    /** for c++11-like for loops: begin of inner sectors. */
    Iterator begin() { return m_innerSectors.begin(); }


    /** for c++11-like for loops: end of inner sectors. */
    Iterator end() { return m_innerSectors.end(); }


    /** returns true, if sector has no inner sectors. */
    bool empty() { return m_innerSectors.empty(); }
//  Iterator empty() { return (m_innerSectors.empty() ? true : false); }


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
      wasFound();
      B2DEBUG(1, "ProtoSectorGraph iD: " << FullSecID(m_rawID).getFullSecString() << " for filterID " << filterID << " the value " <<
              value << " has been found")
      auto pos = m_rawDataForFilter.find(filterID);
      if (pos != m_rawDataForFilter.end()) { pos->second.append(value); return true; }
      B2ERROR("ProtoSectorGraph::addRawDataValue: attempted to add value " << value <<
              " to filter " << filterID <<
              ": the latter was not existing - adding did not work!")
      return false;
    }


    /** returns total number of samples occured in this graph. */
    unsigned determineCutsAndCleanRawData()
    {
      unsigned nSamples = 0;
      // sanity check:
      if (m_rawDataForFilter.empty() and m_innerSectors.empty()) { B2DEBUG(50, "determineCutsAndCleanRawData: no rawData!"); return 0; }

      // will not be executed when empty:
      for (auto& innerSector : m_innerSectors) {
        nSamples += innerSector.determineCutsAndCleanRawData();
      }

      // will not be executed when empty:
      for (auto& entry : m_rawDataForFilter) {
        if (entry.second.empty()) { B2WARNING("TODO... determineCutsAndCleanRawData-found badCase..."); }
        auto cuts = entry.second.getMinMax(m_minMaxQuantiles.first, 1. - m_minMaxQuantiles.second);
        m_finalCuts.insert({entry.first, cuts});
        nSamples += entry.second.size();
        // deleting raw data now:
        entry.second.clear();
      }
      m_rawDataForFilter.clear();

      return nSamples;
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


    /** if combination does not exist yet, it will get added to the graph, if it exists, simply counters are increased.
    * returns true, if everything was okay, false if there was an issue.
    *
    * deques:
    * .first pointer to branch, .second value of branch for given thisEntry. Hardcopy because of recursive approach.
    */
    bool addRelationToGraph(Long64_t thisEntry, // of the tree/chain
                            std::deque<std::pair<TBranch*, unsigned>> sectorBranches, // Hardcopy because of recursive approach
                            std::deque<std::pair<TBranch*, double>>& filterBranches)
    {
      // case: we are finished:
      if (sectorBranches.empty()) { B2ERROR("addRelationToGraph: should not happen!"); return false; }

      // case: reached final sector:
      if (sectorBranches.size() == 1) {
        wasFound();
        /// TODO filterBranches: collect min&max...
        return true;
      }

      sectorBranches.pop_front();

      // set correct entry:
      updateBranchEntries<unsigned>(thisEntry, sectorBranches);
      unsigned innerID = sectorBranches[0].second;

      Iterator pos = find(innerID);

//       if (currentID == m_rawID) {
//         wasFound();
//    return
//       } else {
//    B2ERROR("addRelationToGraph: should not happen!")
//    return false;
//    }
//
//    sectorBranches.pop_front();
//    unsigned innerID = sectorBranches[0].second;
//    Iterator pos = find(innerID);

      // case: it is not this one and and none of the inner sectors:
      if (pos == end()) {
        pos = addInnerSector(innerID);
      }

//    sectorBranches.pop_front();
      return pos->addRelationToGraph(thisEntry, sectorBranches, filterBranches);

//       }

//    // now only two cases left to capture: isThisSector or isInnerSector (both at the same time is not possible).
//    // case: is this sector -> is not inner Sector:
//    if (currentID == m_rawID) {
//    sectorBranches.pop_front();
//    }
//
//    // case: is not this sector -> is innerSector:
//    if (currentID != m_rawID) { /* nothing has to be done, so just continue */ }
//
//    for (auto& innerSector : m_innerSectors) {
//    innerSector.addRelationToGraph(thisEntry, sectorBranches);
//    }
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



    /** collect raw data of each sector-combination and find cuts, return value counts number of compatible cases */
    unsigned distillRawData4FilterCutsV2(
      Long64_t thisEntry,
      TrainerConfigData& config,
      std::deque<std::pair<TBranch*, unsigned>>
      sectorBranches, // hardcopy, since it will be pruned during the process. TODO: could be solved in a less time-consuming way (iterators?)
      std::deque<std::pair<TBranch*, double>>& filterBranches,
      unsigned nSectorChain)
    {

      // case: this graph is walked through (should not be possible):
      if (sectorBranches.empty()) {
        B2WARNING("distillRawData4FilterCutsV2: no sectorBranches, return...")
        return 0;
      }

      updateBranchEntries<unsigned>(thisEntry, sectorBranches);
//    sectorBranches[0].first->GetEntry(thisEntry);
      unsigned currentRawID = sectorBranches[0].second;

      // case this iD is not compatible with this sector (should not be possible):
      if (currentRawID != getRawSecID()) {
        B2WARNING("distillRawData4FilterCutsV2: rawSecID (" << FullSecID(m_rawID).getFullSecString() << ") not compatible to given ID (" <<
                  FullSecID(currentRawID).getFullSecString() << "), return...")
        return 0;
      }


      // case: we have found the innermost Sector of the graph we wanted: (and we are currently in it)
      if (sectorBranches.size() == 1) {
        std::vector<std::string> nHitFilters;
        if (nSectorChain == 2) {
          nHitFilters = config.twoHitFilters;
        } else if (nSectorChain == 3) {
          nHitFilters = config.threeHitFilters;
        } else {
          nHitFilters = config.fourHitFilters;
        }

        // prepare for collection if not done yet:
        if (m_rawDataForFilter.empty()) {
          prepareRawDataContainers(config.quantiles, nHitFilters);
        }

        // collect values for filtering:
        unsigned nFilters = filterBranches.size();
        for (unsigned fPos = 0; fPos < nFilters; fPos++) {
          filterBranches[fPos].first->GetEntry(thisEntry);
          addRawDataValue(
            nHitFilters[fPos],
            filterBranches[fPos].second);
        }

        B2DEBUG(1, "distillRawData4FilterCutsV2: was able to add rawData of " << nFilters << ", return...")
        return 1;
      }

      // all other cases- we are not yet done yet but at least current sector is correct.
      B2DEBUG(1, "distillRawData4FilterCutsV2: before doing pop_front, size: " <<
              sectorBranches.size() <<
              " (this/inner: " << FullSecID(currentRawID).getFullSecString() <<
              "/" <<
              FullSecID(sectorBranches[1].second).getFullSecString() <<
              ")")
      sectorBranches.pop_front();
//    sectorBranches[0].first->GetEntry(thisEntry);

      // find inner sector with correct id:
      unsigned innerRawID = sectorBranches[0].second;
      if (currentRawID == innerRawID) {
        B2ERROR(" this and inner raw iD are identical, size: " <<
                sectorBranches.size() <<
                "! " <<
                FullSecID(currentRawID).getFullSecString());
      }
      auto secPos = find(innerRawID);

      // case: this is not the right graph, skipping...
      if (secPos == m_innerSectors.end()) {
        B2DEBUG(1, "distillRawData4FilterCutsV2: are in the wrong graph (this=good/inner=bad: " << FullSecID(
                  currentRawID).getFullSecString() << "/" << FullSecID(innerRawID).getFullSecString() << "), return...")
        return 0;
      }

      // case: we have found the correct inner sector, passing data:
      B2DEBUG(1, "distillRawData4FilterCutsV2: are yet on the right way (this/inner: " << FullSecID(
                currentRawID).getFullSecString() << "/" << FullSecID(innerRawID).getFullSecString() << "), moving on to next link in the chain.")
      return secPos->distillRawData4FilterCutsV2(thisEntry, config, sectorBranches, filterBranches, nSectorChain);
    }








    /** collect raw data of each sector-combination and find cuts, return value counts number of compatible cases */
    unsigned distillRawData4FilterCuts(
      Long64_t thisEntry,
      TrainerConfigData& config,
      std::deque<std::pair<TBranch*, unsigned>>
      sectorBranches, // hardcopy, since it will be pruned during the process. TODO: could be solved in a less time-consuming way (iterators?)
      std::deque<std::pair<TBranch*, double>>& filterBranches,
      unsigned nSectorChain)
    {
      // case: this graph is walked through:
      if (sectorBranches.empty()) {
        B2DEBUG(1, "distillRawData4FilterCuts: no sectorBranches, return...")
        return 0;
      }

      std::vector<std::string> nHitFilters;
      if (nSectorChain == 2) {
        nHitFilters = config.twoHitFilters;
      } else if (nSectorChain == 3) {
        nHitFilters = config.threeHitFilters;
      } else {
        nHitFilters = config.fourHitFilters;
      }

      // update branch-entry:
      sectorBranches[0].first->GetEntry(thisEntry);

      // case: we are in the wrong graph:
      if (sectorBranches[0].second != getRawSecID()) {
        B2DEBUG(50, "distillRawData4FilterCuts: rawSecID (" << FullSecID(m_rawID).getFullSecString() << ") not compatible to given ID (" <<
                FullSecID(sectorBranches[0].second).getFullSecString() << "), return...")
        return 0;
      }

      // case: we have found the innermost Sector of the graph we wanted:
      if (sectorBranches.size() == 1) {
        B2DEBUG(1, "case: we have found the innermost Sector of the graph we wanted - now writing data")

        // prepare for collection if not done yet:
        if (m_rawDataForFilter.empty()) {
          prepareRawDataContainers(config.quantiles, nHitFilters);
        }

        // collect values for filtering:
        unsigned nFilters = filterBranches.size();
        for (unsigned fPos = 0; fPos < nFilters; fPos++) {
          filterBranches[fPos].first->GetEntry(thisEntry);
          addRawDataValue(
            nHitFilters[fPos],
            filterBranches[fPos].second);
        }

        B2DEBUG(1, "distillRawData4FilterCuts: was able to add rawData of " << nFilters << ", return...")
        return 1;
      }

      // case: we might be in the correct graph (so far so good), but this is not the end yet -> continue looping through the next sectors in chain:
      B2DEBUG(50, "distillRawData4FilterCuts: we are in the correct graph(rawSecID/givenID: " << FullSecID(
                m_rawID).getFullSecString() << "/" << FullSecID(sectorBranches[0].second).getFullSecString() <<
              "), but not at the end yet, going further.")
      unsigned countGood = 0;
      sectorBranches.pop_front();
      for (Sector& subGraph : m_innerSectors) {
        countGood += subGraph.distillRawData4FilterCuts(thisEntry, config, sectorBranches, filterBranches, nSectorChain);
      }
      return countGood;
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


    std::string print()
    {
      FullSecID myTempID(getRawSecID());

      std::string out = " " + myTempID.getFullSecString();
      out += (" nInner/nTimesFound/nSample: " + std::to_string(size()) + "/" + std::to_string(m_nTimesFound) + "/" + std::to_string(
                m_rawDataForFilter.size())) + " ";
      if (m_innerSectors.empty()) { out += "||\n"; return std::move(out); }
      out += " ";
      for (auto& innerSector : m_innerSectors) {
        out += innerSector.print();
      }

//    out += "\n";
      return std::move(out);
    }
  }; // end of ProtoSectorGraph-declaration.
}

