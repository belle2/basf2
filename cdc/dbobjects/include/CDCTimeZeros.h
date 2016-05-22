/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida (original), CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <iostream>
#include <map>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /**
   * Database object for timing offset (t0).
   */
  class CDCTimeZeros: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCTimeZeros() {}

    /**
     * Set t0 in the list
     */
    void setT0(unsigned short ICLayer, unsigned short IWire, double t0)
    {
      m_t0s.insert(std::pair<unsigned short, float>(WireID(ICLayer, IWire).getEWire(), t0));
    }

    /**
     * Set t0 in the list
     */
    void setT0(const WireID wid, double t0)
    {
      m_t0s.insert(std::pair<unsigned short, float>(wid.getEWire(), t0));
    }

    /**
     * Update t0 in the list
     */
    void addT0(unsigned short ICLayer, unsigned short IWire, double delta)
    {
      WireID wid(ICLayer, IWire);
      std::map<unsigned short, float>::iterator it = m_t0s.find(wid.getEWire());
      it->second += delta;
    }

    /**
     * Update t0 in the list
     */
    void addT0(const WireID wid, double delta)
    {
      std::map<unsigned short, float>::iterator it = m_t0s.find(wid.getEWire());
      it->second += delta;
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_t0s.size();
    }

    /**
     * Get the t0 list
     */
    std::map<unsigned short, float> getT0s()
    {
      return m_t0s;
    }

    /**
     * Get t0 for the specified wire
     */
    float getT0(const WireID& wid)
    {
      std::map<unsigned short, float>::iterator it = m_t0s.find(wid.getEWire());
      return it->second;
    }

    /**
     * Print out all contents
     */
    void dump()
    {
      std::cout << " " << std::endl;
      std::cout << "t0 list" << std::endl;
      std::cout << "# of entries= " << m_t0s.size() << std::endl;
      std::cout << "in order of clayer#, wire#, t0" << std::endl;
      for (auto const& ent : m_t0s) {
        std::cout << WireID(ent.first).getICLayer() << " " << WireID(ent.first).getIWire() << " " << ent.second << std::endl;
      }
    }

  private:
    std::map<unsigned short, float> m_t0s; /**< t0 list*/

    ClassDef(CDCTimeZeros, 2); /**< ClassDef */
  };

} // end namespace Belle2

