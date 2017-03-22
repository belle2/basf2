/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <tracking/dataobjects/SpacePointInfo.h>
#include <tracking/dataobjects/FilterInfo.h>

namespace Belle2 {


  class ObserverInfo : public TObject {
  public:
    ObserverInfo();
    ~ObserverInfo();

    // resets all member variables
    void clear()
    {
      m_filterInfos.clear();
      m_hits.clear();
      m_mainMCParticleID = -666;
      m_mainPurity = 0;
    };

    FilterInfo getFilterInfo(std::string const&   key)
    {
      for (int i = 0; i < (int) m_filterInfos.size(); i++)
        if (m_filterInfos.at(i).getName() == key) return m_filterInfos.at(i);
      //if nothing was found return an empty object
      return FilterInfo();
    };

    //access the filter infos directly, as inderect access seems not to work within the Draw of root
    std::string getFilterName(int i)
    {
      return (0 <= i && i < (int)m_filterInfos.size()) ? m_filterInfos[i].getName() : std::string("");
    }

    double getFilterResult(std::string const& key)
    {
      return getFilterInfo(key).getResult();
    }
    bool getFilterAccepted(std::string const& key)
    {
      return getFilterInfo(key).getWasAccepted();
    }
    double getFilterUsed(std::string const& key)
    {
      return getFilterInfo(key).getWasUsed();
    }


    void addFilterInfo(FilterInfo info) { m_filterInfos.push_back(info); };


    std::vector<SpacePointInfo> getHits() { return m_hits; };
    SpacePointInfo getHit(int i)
    {
      if (i < 0 || i > (int)m_hits.size()) return SpacePointInfo();  // return a dummy object
      else return m_hits[i];
    };
    void setHits(std::vector<SpacePointInfo>& newHits) { m_hits = newHits; };

    int getMainMCParticleID() {return m_mainMCParticleID;};
    void setMainMCParticleID(int id) { m_mainMCParticleID = id; };

    double getMainPurity() { return m_mainPurity; };
    void setMainPurity(double val) { m_mainPurity = val; };
  private:
    /** stores the information for each Filter */
    std::vector<FilterInfo> m_filterInfos;

    /** stores the hits used in the filter, should have the order as used in the filter from outer to inner! */
    std::vector<SpacePointInfo>  m_hits;

    /** dominating mcParticleID. */
    int m_mainMCParticleID;

    /** purity for the dominating particleID. */
    double m_mainPurity;

    //! Needed to make the ROOT object storable
    ClassDef(ObserverInfo, 1);
  };

}// end namespace Belle2
