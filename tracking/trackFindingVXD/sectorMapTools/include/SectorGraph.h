/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraphID.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraph.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility> // std::pair, std::move
#include <TH1.h>


namespace Belle2 {

  /** contains all subgraphs. The ids used for the subgraphs have to be sorted from outer to inner sectors. */
  template<class FilterType>  class SectorGraph {
  protected:
    std::unordered_map<SubGraphID, SubGraph<FilterType>> m_subgraphs; /**< contains all subgraphs. */

    std::vector<FilterType> m_filterIDs; /**< ids of all filterTypes to be stored by subGraphs. */
  public:

    /** constructor expects filterIDs. */
    explicit SectorGraph(const std::vector<FilterType>& fIDs) : m_filterIDs(fIDs)
    { if (m_filterIDs.empty()) { B2FATAL("SectorGraph-constructor: passed filterIDs are empty, this is an illegal usage of this class!"); } }

    /** for better readability. */
    using Iterator = typename std::unordered_map<SubGraphID, SubGraph<FilterType>>::iterator;

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
      B2DEBUG(1, "SectorGraph::add: new subgraph added: " << pos.first->second.print());
      return pos.first;
    }

    /** returns a string giving an overview of the graph. */
    std::string print(bool fullPrint = true) const
    {
      unsigned nSubgraphs = m_subgraphs.size();
      std::string out = "graph has got " + std::to_string(nSubgraphs) + " entries:\n";
      out += "now printing " + (fullPrint ? std::string("full") : std::string("short version of the")) + " graph:\n";
      for (const auto& entry : m_subgraphs) {
        if (!fullPrint and nSubgraphs % 100 != 0) continue; // printing only 100 subgraphs of mainGraph for the short version.
        out += entry.second.print() + "\n";
      }
      return out;
    }

    /** returns removed occurances. */
    unsigned pruneGraph(double rarenessCut)
    {
      //sanity checks:
      if (rarenessCut < 0 or rarenessCut >= 1)
      { B2WARNING("pruneGraph: rarenessCut is rubbish: " << rarenessCut << ", stopping prune-process."); return 0; }
      if (rarenessCut == 0) { B2DEBUG(1, "pruneGraph: nothing to be done, stopping prune-process."); return 0; }

      /// vector of trunks (= outer sector(s)):
      // .first counts total number of occurances of branches of this trunk (= outer sector(s))
      // .second is a vector of pointers to the subgraphs of that trunk.
      std::vector< std::pair<unsigned, std::vector<SubGraph<FilterType>*> >> trunks;

      // find those sharing a trunk (trunkTotal) and cluster them in these trunks:
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
          // this should really be avoided but it in this case it seems to be on purpose (push_back to container that is iterated)
          // tell cppcheck it is fine..
          // cppcheck-suppress invalidContainerLoop
          trunks.push_back({graph.getFound(), {&graph} });
        }
      }

      unsigned long nFoundB4 = nFoundTotal(), nKilled = 0;
      unsigned sizeb4 = size();
      B2DEBUG(1, "pruneGraph - before pruning: graph of size " << sizeb4 << " has " << trunks.size() << " trunks with " << nFoundB4 <<
              " total found.");

      // collect subGraphs (=branches) to be deleted:
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
              ") and nFound (before/after/killed " << nFoundB4 << "/" << nFoundTotal() << "/" << nKilled);

      return nKilled;
    }

    /** Get the absolute treshold (# nfound) given a relative threshold. */
    int getAbsThreshold(int relThreshold)
    {
      if (relThreshold == 0) {return 0;}
      B2INFO("Relative threshold : " << relThreshold << " %");
      int xmax = 100000;

      TH1D* h_nfound = new TH1D("h", "# times that subgraphs were found n_found times", xmax, 0, xmax);

      for (auto& subGraphEntry : m_subgraphs) {
        SubGraph<FilterType>& graph = subGraphEntry.second;
        h_nfound->Fill(graph.getFound());
      }

      if (h_nfound->GetEntries() == 0) {
        B2ERROR("nfound histogram empty.");
        delete h_nfound;
        return 0;
      }

      TH1* hc_nfound = h_nfound->GetCumulative();
      hc_nfound->Scale(1 / h_nfound->GetEntries());

      for (int nfound = 1; nfound < hc_nfound->GetNbinsX(); nfound++) {
        if (hc_nfound->GetBinContent(nfound) > relThreshold / 100.) {
          B2INFO("Absolute threshold : remove every graph with nfound < " << nfound);
          delete h_nfound;
          delete hc_nfound;
          return nfound;
        }
      }

      // In case no value for nfound is found
      B2ERROR("No nfound value found.");
      delete h_nfound;
      delete hc_nfound;
      return 0;
    }

    /** returns removed occurances. */
    unsigned pruneGraphBeforeTraining(int absThreshold)
    {
      if (absThreshold == 0) {return 0;}

      int killed = 0;

      std::vector<SubGraph<FilterType>*> deadBranches;

      for (auto& subGraphEntry : m_subgraphs) {
        SubGraph<FilterType>& graph = subGraphEntry.second;
        if (int(graph.getFound()) <= absThreshold) {
          deadBranches.push_back(&graph);
        }
      }

      for (auto& graph : deadBranches) {
        m_subgraphs.erase(graph->getID());
        killed ++;
      }

      return killed;
    }

    /** Output in a txt file id & nfound of subgraphs */
    void output_nfound()
    {
      std::ofstream out;
      out.open("output_nfound.txt");
      for (auto& subGraphEntry : m_subgraphs) {
        SubGraph<FilterType>& graph = subGraphEntry.second;
        out << graph.print() << std::endl;
      }
      out.close();
    }

    /**
    * finds sectors having inner sectors in same layer and update them in the subGraph-ID.
    * */
    void updateSubLayerIDs()
    {
      unsigned nUpdated = 0, // counts sectors which shall be updated
               nFound = 0, // counts sectors which were found (without double entry removal)
               nGraphsUpdated = 0; // counts graphs which were updated

      // collects the secIDs which have got inner sectors on same sensor:
      std::vector<unsigned> idsFound;
      std::string idsPrinted;

      // collect all SecIDs where SubLayer has to be updated.
      for (auto& subGraphEntry : m_subgraphs) {
        SubGraphID graphID = subGraphEntry.second.getID();
        std::vector<unsigned> found = graphID.hasSharedLayer();
        if (found.empty()) continue;
        idsFound.insert(idsFound.end(), found.begin(), found.end());
      }
      for (unsigned id : idsFound) { idsPrinted += FullSecID(id).getFullSecString() + " "; }
      B2DEBUG(1, "updateSubLayerIDs: before unique of found ids, following IDs are recorded: \n" << idsPrinted);
      nFound += idsFound.size();
      std::sort(idsFound.begin(), idsFound.end());
      idsFound.erase(std::unique(idsFound.begin(), idsFound.end()), idsFound.end());
      nUpdated += idsFound.size();

      idsPrinted = "";
      for (unsigned id : idsFound) { idsPrinted += FullSecID(id).getFullSecString() + " "; }
      B2DEBUG(1, "updateSubLayerIDs: before updating Subgraphs, following IDs have to be updated: \n" << idsPrinted);


      // update all subGraphIDs where subLayerID has to be increased:
      for (auto& subGraphEntry : m_subgraphs) {
        SubGraph<FilterType>& graph = subGraphEntry.second;
        unsigned nSecsUpdated = graph.idCheckAndUpdate(idsFound);
        if (nSecsUpdated == 0) {
          B2DEBUG(50, "updateSubLayerIDs: was _not_ updated: " << graph.getID().print());
          continue;
        }
        nGraphsUpdated++;
        B2DEBUG(50, "updateSubLayerIDs: was updated " << nSecsUpdated << " times: " << graph.getID().print());
      }

      B2DEBUG(1, "updateSubLayerIDs: nSectors found/updated: " << nFound << "/" << nUpdated << ", nSubgraphs updated: " <<
              nGraphsUpdated);

      // create new map of Subgraphs with updated LayerIDs
      // subGraph: copy with updated iD.
      // SubGraphID: a.isElementOf(SubGraphID& b) <- checks if given sectorPack ( >= 1 sector) is part of a, ignores subLayerID.
      // SubGraphID: a.replaceElement(SubGraphID& b) <- checks if given sectorPack ( >= 1 sector) is part of a, ignores subLayerID. if yes, replaces element(s). WARNING only works with complete replacement of SubGraphID (entries are const).
      // SubGraphID: a.areTheSameSectors(SubGraphID& b) <- checks if sectors are identical (while ignoring the subLayerID)
      // SubGraphID: a.sharesLayer() <- returns IDs of entries being inner friend of sectors on same layer.

      // return new map of Subgraphs.
    }

    /// returns a Vector containing all FullSecIDs found for given sensor.
    std::vector<FullSecID> getAllFullSecIDsOfSensor(VxdID sensor)
    {
      std::vector<FullSecID> foundIDs;

      for (auto& subGraph : m_subgraphs) {
        std::vector<FullSecID> sectorsFound = subGraph.second.getSectorsOfSensor(sensor);
        if (sectorsFound.empty()) continue;
        foundIDs.insert(foundIDs.end(), sectorsFound.begin(), sectorsFound.end());
      }
      B2DEBUG(1, "getAllFullSecIDsOfSensor: VxdID " << sensor << " has " << foundIDs.size() << " sectors in this graph");
      return foundIDs;
    }

    /// returns a const reference to the filterTypes stored in this graph
    const std::vector<FilterType>& getFilterTypes() const { return m_filterIDs; }
  };
}

