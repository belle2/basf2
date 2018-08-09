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
#include <tracking/trackFindingVXD/sectorMapTools/MinMax.h>
#include <tracking/trackFindingVXD/sectorMapTools/SubGraphID.h>
#include <tracking/dataobjects/FullSecID.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>


namespace Belle2 {

  /** contains all relevant stuff needed for dealing with a subGraph. ~ 404 bytes per subGraph. */
  template<class FilterType> class SubGraph {
  protected:
    SubGraphID m_id; /**< contains the IDs in the correct order (from outer to inner) as a unique identifier for this graph. */

    /// contains all min- and max-values found so far for all filters relevant for this Graph. */
    std::unordered_map<FilterType, MinMax> m_minMaxValues;

    unsigned m_found; /**< counts number of times this subgraph was found. */

    std::unordered_map<FilterType, RawDataCollectedMinMax>* m_rawDataCollected; /**< takes care of collecting the raw data. */

    /** set newID for this subgraph. */
    void updateID(SubGraphID& newID)
    { m_id = newID; }
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
      return out;
    }

    /** returns iD of this graph*/
    const SubGraphID& getID() const { return m_id; }

    /// takes care of being able to use the data collector. please use for quantiles [min, max] min ~0 & max ~1 (range 0-1)
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
      for (auto& entry : *m_rawDataCollected) {
        std::pair<double, double> results = entry.second.getMinMax();
        checkAndReplaceIfMinMax(entry.first, results.first);
        checkAndReplaceIfMinMax(entry.first, results.second);
      }

      std::string out;
      for (auto& entry : m_minMaxValues) {
        out += entry.first + " " + entry.second.print() + "  ";
      }
      B2DEBUG(1, "SubGraph::getFinalQuantileValues: minMaxFound:\n" << out);
      return m_minMaxValues;
    }

    /// for given vector of ids check if any of them is part of subGraphID. If yes, update their SubLayerID. returns number of updated secIDs
    unsigned idCheckAndUpdate(const std::vector<unsigned>& ids)
    {
      unsigned nUpdated = 0;
      for (unsigned id : ids) {

        if (m_id.hasElement(id)) {
          m_id.updateID(id);
          nUpdated++;
        }
      }
      return nUpdated;
    }

    /// returns vector containing all sectors for given sensor (if any) and empty vector if no sector of that sensor is here.
    std::vector<FullSecID> getSectorsOfSensor(VxdID& sensor)
    {
      std::vector<FullSecID> foundIDs;

      for (auto& sector : m_id) {
        if (sensor != FullSecID(sector).getVxdID()) continue;
        foundIDs.push_back(FullSecID(sector));
      }
      return foundIDs;
    }
  };
}

