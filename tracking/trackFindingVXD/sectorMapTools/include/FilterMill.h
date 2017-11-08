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

#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <string>
#include <vector>
#include <utility> // std::pair, std::move
#include <functional>


namespace Belle2 {

  /** Small class which stores the filters/selectionVariables to be used for a secMap and has an interface for applying them. */
  template <class PointType>
  class FilterMill {
  public:


    /** typedef for more readable function-type - to be used for 2-hit-selectionVariables. */
    using TwoHitFunction = typename std::function<double(const PointType&, const PointType&)>;


    /** typedef for more readable function-type - to be used for 3-hit-selectionVariables. */
    using ThreeHitFunction = typename std::function<double(const PointType&, const PointType&, const PointType&)>;


    /** typedef for more readable function-type - to be used for 4-hit-selectionVariables. */
    using FourHitFunction = typename std::function<double(const PointType&, const PointType&, const PointType&, const PointType&)>;



  protected:
    /** Contains all 2-hit-Filters and their names to be applied. */
    std::vector<std::pair<std::string, TwoHitFunction> > m_2Hitfilters;


    /** Contains all 3-hit-Filters and their names to be applied. */
    std::vector<std::pair<std::string, ThreeHitFunction> > m_3Hitfilters;


    /** Contains all 4-hit-Filters and their names to be applied. */
    std::vector<std::pair<std::string, FourHitFunction> > m_4Hitfilters;


    /** Simple saveguard for not changin any filters after preparing phase. */
    bool m_locked = false;


    /** to be executed by functions which shall work with unchangeable set of filters. */
    void checkLocked(std::string name) const
    {
      if (!m_locked) {
        B2WARNING("FilterMill-checkLocked: Function " << name <<
                  " was executed although the Mill was not locked yet. This might be unintended...");
      }
    }



  public:

    /** small struct containing pointers to two hits. */
    struct HitPair {
      const PointType* outer; /**< outer hit. */
      const PointType* inner; /**< inner hit. */
    };

    /** small struct containing pointers to three hits. */
    struct HitTriplet {
      const PointType* outer; /**< outer hit. */
      const PointType* center; /**< center hit. */
      const PointType* inner; /**< inner hit. */
    };

    /** small struct containing pointers to four hits. */
    struct HitQuadruplet {
      const PointType* outer; /**< outer hit. */
      const PointType* outerCenter; /**< outerCenter hit. */
      const PointType* innerCenter; /**< innerCenter hit. */
      const PointType* inner; /**< inner hit. */
    };

    /** Constructor. */
    FilterMill() :
      m_locked(false) {}


    /** to block adding new filters, execute this member. */
    void lockMill() { m_locked = true; }


    /** add new Filter for 2 Hits . */
    void add2HitFilter(std::pair<const std::string, TwoHitFunction> newFilter)
    {
      if (m_locked) {
        B2ERROR("FilterMill-add2HitFilter: someone tried to add filter " << newFilter.first <<
                " after lockdown! This is unintended behavior - doing nothing instead...");
        return;
      }
      B2DEBUG(5, "FilterMill::add2HitFilter: filter " << newFilter.first << " added");
      m_2Hitfilters.push_back(std::move(newFilter));
    }


    /** add new Filter for 3 Hits . */
    void add3HitFilter(std::pair<std::string, ThreeHitFunction> newFilter)
    {
      if (m_locked) {
        B2ERROR("FilterMill-add3HitFilter: someone tried to add filter " << newFilter.first <<
                " after lockdown! This is unintended behavior - doing nothing instead...");
        return;
      }
      B2DEBUG(5, "FilterMill::add3HitFilter: filter " << newFilter.first << " added");
      m_3Hitfilters.push_back(std::move(newFilter));
    }


    /** add new Filter for 4 Hits . */
    void add4HitFilter(std::pair<std::string, FourHitFunction> newFilter)
    {
      if (m_locked) {
        B2ERROR("FilterMill-add4HitFilter: someone tried to add filter " << newFilter.first <<
                " after lockdown! This is unintended behavior - doing nothing instead...");
        return;
      }
      B2DEBUG(5, "FilterMill::add4HitFilter: filter " << newFilter.first << " added");
      m_4Hitfilters.push_back(std::move(newFilter));
    }


    /** on given dataSet, apply all filters stored in the mill and store the results in collectedData. */
    void grindData2Hit(const HitPair& dataSet,
                       std::vector<std::pair<std::string, double> >& collectedData) const
    {
      checkLocked("grindData2Hit");
      if (m_2Hitfilters.empty()) { B2DEBUG(5, "there are no 2-hit-filters stored, skipping grinding"); return; }

      std::vector<std::pair<std::string, double>> tempData;
      try {
        for (const auto& filterPack : m_2Hitfilters) {

          double result = filterPack.second(
                            *(dataSet.outer),
                            *(dataSet.inner));
          tempData.push_back({filterPack.first, result});
        }
      } catch (...) {
        std::string filterNames;
        for (const auto& filterPack : m_2Hitfilters) {
          filterNames += filterPack.first + " ";
        }
        B2WARNING("FilterMill:grindData2Hit: an exception was thrown by one of the Filters/SelectionVariables, that indicates strange input data, no data collected! Filters "
                  << filterNames << " were used and one of them did not work.");
        return;
      }

      collectedData.insert(collectedData.end(), tempData.begin(), tempData.end());

      B2DEBUG(5, "FilterMill::grindData2Hit: collectedData has now " << collectedData.size() << " entries");
    }


    /** on given dataSet, apply all filters stored in the mill and store the results in collectedData. */
    void grindData3Hit(const HitTriplet& dataSet,
                       std::vector<std::pair<std::string, double> >& collectedData) const
    {
      checkLocked("grindData3Hit");
      if (m_3Hitfilters.empty()) { B2DEBUG(5, "there are no 3-hit-filters stored, skipping grinding"); return; }

      std::vector<std::pair<std::string, double>> tempData;
      try {
        for (const auto& filterPack : m_3Hitfilters) {

          double result = filterPack.second(
                            *(dataSet.outer),
                            *(dataSet.center),
                            *(dataSet.inner));
          tempData.push_back({filterPack.first, result});
        }
      } catch (...) {
        std::string filterNames;
        for (const auto& filterPack : m_2Hitfilters) {
          filterNames += filterPack.first + " ";
        }
        B2WARNING("FilterMill:grindData3Hit: an exception was thrown by one of the Filters/SelectionVariables, that indicates strange input data, no data collected! Filters "
                  << filterNames << " were used and one of them did not work.");
        return;
      }
      collectedData.insert(collectedData.end(), tempData.begin(), tempData.end());

      B2DEBUG(5, "FilterMill::grindData3Hit: collectedData has now " << collectedData.size() << " entries");
    }


    /** on given dataSet, apply all filters stored in the mill and store the results in collectedData. */
    void grindData4Hit(const HitQuadruplet& dataSet,
                       std::vector<std::pair<std::string, double> >& collectedData) const
    {
      checkLocked("grindData4Hit");
      if (m_4Hitfilters.empty()) { B2DEBUG(5, "there are no 4-hit-filters stored, skipping grinding"); return; }

      std::vector<std::pair<std::string, double>> tempData;
      try {
        for (const auto& filterPack : m_4Hitfilters) {

          double result = filterPack.second(
                            *(dataSet.outer),
                            *(dataSet.outerCenter),
                            *(dataSet.innerCenter),
                            *(dataSet.inner));
          tempData.push_back({filterPack.first, result});
        }
      } catch (...) {
        B2WARNING("FilterMill:grindData4Hit: an exception was thrown by one of the Filters/SelectionVariables, that indicates strange input data, no data collected!");
        return;
      }
      collectedData.insert(collectedData.end(), tempData.begin(), tempData.end());

      B2DEBUG(5, "FilterMill::grindData4Hit: collectedData has now " << collectedData.size() << " entries");
    }

  };

}


