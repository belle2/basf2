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
#include <tracking/trackFindingVXD/sectorMapTools/RawDataCollectedMinMax.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraphID.h>
#include <tracking/dataobjects/FullSecID.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <utility> // std::pair, std::move


namespace Belle2 {

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
}

