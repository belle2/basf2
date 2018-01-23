/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
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
#include <fstream>
#include <iomanip>
#include <TObject.h>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /**
   * Database object for bad wires.
   */
  class CDCBadWires: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCBadWires() {}

    /**
     * Set the wire in the list
     * @param wid wire-id to be registered
     */
    void setWire(const WireID& wid)
    {
      m_wires.push_back((wid.getEWire()));
    }

    /**
     * Set the wire in the list
     * @param iCLayer (continuous) layer-id of badwire
     * @param iWire   wire-id of badwire
     */
    void setWire(unsigned short iCLayer, unsigned short iWire)
    {
      m_wires.push_back(WireID(iCLayer, iWire).getEWire());
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_wires.size();
    }

    /**
     * Get the whole list
     */
    std::vector<unsigned short> getWires() const
    {
      return m_wires;
    }

    /**
     * Check if the wire is bad
     * @param  wid wire id to be checked
     * @return true if badwire; false if not
     */
    bool isBadWire(const WireID& wid) const
    {
      std::vector<unsigned short>::const_iterator it = std::find(m_wires.begin(), m_wires.end(), wid.getEWire());
      bool torf = (it != m_wires.end()) ? true : false;
      return torf;
    }

    /**
     * Print out contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Bad wire list" << std::endl;
      std::cout << "#entries= " << m_wires.size() << std::endl;
      std::cout << "in order of ICLayer and IWire and EWire" << std::endl;

      for (auto const ent : m_wires) {
        std::cout << WireID(ent).getICLayer() << " " << WireID(ent).getIWire() << " " << ent << std::endl;
      }
    }

    /**
     * Output the contents in text file format
     */
    void outputToFile(std::string fileName) const
    {
      std::ofstream fout(fileName);

      if (fout.bad()) {
        B2ERROR("Specified output file could not be opened!");
      } else {
        for (auto const ent : m_wires) {
          fout << std::setw(2) << std::right << WireID(ent).getICLayer() << "  " << std::setw(3) << WireID(ent).getIWire() << std::endl;
        }
        fout.close();
      }
    }

  private:
    std::vector<unsigned short> m_wires; /**< badwire list*/

    ClassDef(CDCBadWires, 1); /**< ClassDef */
  };

} // end namespace Belle2
