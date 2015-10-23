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

#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraphID.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraph.h>

#include <string>
#include <vector>
#include <map>
#include <utility> // std::pair, std::move


namespace Belle2 {

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
}

