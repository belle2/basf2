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
  /** Helper class that stores the information an Observer stores: i.e.
   * all the responses of the subfilters contained in a Filter.
  */
  class ObserverInfo : public TObject {
  public:
    /// constructor
    ObserverInfo();
    /// destructor
    ~ObserverInfo();

    /// resets all member variables
    void clear()
    {
      m_filterInfos.clear();
      m_hits.clear();
      m_mainMCParticleID = -666;
      m_mainPurity = 0;
    };

    /** returns the filter info with the requested key
     * @param key: name of the key for that filter
    */
    FilterInfo getFilterInfo(std::string const&   key)
    {
      for (int i = 0; i < (int) m_filterInfos.size(); i++)
        if (m_filterInfos.at(i).getName() == key) return m_filterInfos.at(i);
      //if nothing was found return an empty object
      return FilterInfo();
    };

    /** function to access the filter infos directly, as inderect access seems not to work within the Draw of root:
     * @param i: the index under which the filter was stored in m_filterinfos
    */
    std::string getFilterName(int i)
    {
      return (0 <= i && i < (int)m_filterInfos.size()) ? m_filterInfos[i].getName() : std::string("");
    }

    /** returns the requested filter result
     * @param key: the key under which the filter was stored
    */
    double getFilterResult(std::string const& key)
    {
      return getFilterInfo(key).getResult();
    }

    /** returns if the filter was accepted
     * @param key: name under which filter was stored
     */
    bool getFilterAccepted(std::string const& key)
    {
      return getFilterInfo(key).getWasAccepted();
    }

    /** returns if the filter was used
     * @param key: key under which filter was stored
    */
    double getFilterUsed(std::string const& key)
    {
      return getFilterInfo(key).getWasUsed();
    }

    /** add a new filter info:
     * @param info: the filterinfo to be added
    */
    void addFilterInfo(FilterInfo info) { m_filterInfos.push_back(info); };

    /** returns the vector of spacepoints which have been used to evaluate the filters */
    std::vector<SpacePointInfo> getHits() { return m_hits; };
    SpacePointInfo getHit(int i)
    {
      if (i < 0 || i > (int)m_hits.size()) return SpacePointInfo();  // return a dummy object
      else return m_hits[i];
    };

    /** sets the hits the filter has been evaluated with
     * @param newHits: vector of spacepoint which have been used in this filter
    */
    void setHits(std::vector<SpacePointInfo>& newHits) { m_hits = newHits; };

    /** return the MCParticle ID of the related MCParticle */
    int getMainMCParticleID() {return m_mainMCParticleID;};

    /** set the MCParticle id
     * @param id: the new id
    */
    void setMainMCParticleID(int id) { m_mainMCParticleID = id; };

    /** returns the purity */
    double getMainPurity() { return m_mainPurity; };

    /** sets the purity
     * @param val: new value of the purity
    */
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
