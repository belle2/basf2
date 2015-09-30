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

#include <tracking/trackFindingVXD/sectorMapTools/XHitDataSet.h>
#include <framework/logging/Logger.h>

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
    void checkLocked(std::string name)
    {
      if (!m_locked) {
        B2WARNING("FilterMill-checkLocked: Function " << name <<
                  " was executed although the Mill was not locked yet. This might be unintended...");
      }
    }



  public:

    /** Constructor. */
    FilterMill() :
      m_locked(false) {}


    /** to block adding new filters, execute this member. */
    void lockMill() { m_locked(true); }


    /** add new Filter for 2 Hits . */
    void add2HitFilter(std::pair<std::string, TwoHitFunction> newFilter)
    {
      if (m_locked) {
        B2ERROR("FilterMill-add2HitFilter: someone tried to add filter " << newFilter.first <<
                " after lockdown! This is unintended behavior - doing nothing instead...");
        return;
      }
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
      m_4Hitfilters.push_back(std::move(newFilter));
    }


    /** on given dataSet, apply all filters stored in the mill and store the results in collectedData. */
    void grindData(const XHitDataSet<std::string, PointType>& dataSet,
                   std::vector<std::pair<std::string, double> >& collectedData) const
    {
      checkLocked("grindData");
      unsigned nHits = dataSet.hits.size();
      if (nHits == 2) {
        for (const auto& filterPack : m_2Hitfilters) {
          double result = filterPack.second(
                            *dataSet.hits[0],
                            *dataSet.hits[1]);
          collectedData.push_back({filterPack.first, result});
        }
      } else if (nHits == 3) {
        for (const auto& filterPack : m_3Hitfilters) {
          double result = filterPack.second(
                            *dataSet.hits[0],
                            *dataSet.hits[1],
                            *dataSet.hits[2]);
          collectedData.push_back({filterPack.first, result});
        }
      } else if (nHits == 4) {
        for (const auto& filterPack : m_4Hitfilters) {
          double result = filterPack.second(
                            *dataSet.hits[0],
                            *dataSet.hits[1],
                            *dataSet.hits[2],
                            *dataSet.hits[3]);
          collectedData.push_back({filterPack.first, result});
        }
      } else {
        B2ERROR("FilterMill-grindData: someone use " << nHits <<
                " to be grinded! This number is currently not supported - doing nothing instead...");
      }
    }

  };

}

