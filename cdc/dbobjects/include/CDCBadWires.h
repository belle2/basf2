/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <algorithm>
#include <iostream>

#include <TObject.h>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /**
   * Database object of CDC.
   */
  class CDCBadWires: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCBadWires() {}

    /**
     * Set the wire in the list
     */
    void setWire(const WireID& wid)
    {
      m_wireList.push_back((wid.getEWire()));
    }

    /**
     * Set the wire in the list
     */
    void setWire(unsigned short ICLayer, unsigned short IWire)
    {
      m_wireList.push_back(WireID(ICLayer, IWire).getEWire());
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_wireList.size();
    }

    /**
     * Get the list of bad wires
     */
    std::vector<unsigned short> getBadWires() const
    {
      return m_wireList;
    }

    /**
     * Inquire if the wire is bad
     */
    bool isBadWire(const WireID& wid)
    {
      std::vector<unsigned short>::iterator it = std::find(m_wireList.begin(), m_wireList.end(), wid.getEWire());
      bool torf = (it != m_wireList.end()) ? true : false;
      return torf;
    }

    /**
     * Print out contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Bad wire list" << std::endl;
      std::cout << "#entries= " << m_wireList.size() << std::endl;
      std::cout << "in order of ICLayer and IWire and EWire" << std::endl;

      for (auto const ent : m_wireList) {
        std::cout << WireID(ent).getICLayer() << " " << WireID(ent).getIWire() << " " << ent << std::endl;
      }
    }

  private:
    std::vector<unsigned short> m_wireList; /**< badwire list*/

    ClassDef(CDCBadWires, 1); /**< ClassDef */
  };

} // end namespace Belle2
