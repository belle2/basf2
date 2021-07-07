/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <map>
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
     * @param eff wire efficiency; specify 0 <= eff < 1 for dead wire; eff > 1 for hot/noisy wire
     */
    void setWire(const WireID& wid, double eff = 0)
    {
      m_wires.insert(std::pair(wid.getEWire(), eff));
    }

    /**
     * Set the wire in the list
     * @param iCLayer (continuous) layer-id of badwire
     * @param iWire   wire-id of badwire
     * @param eff wire efficiency; specify 0 <= eff < 1 for dead wire; eff > 1 for hot/noisy wire
     */
    void setWire(unsigned short iCLayer, unsigned short iWire, double eff = 0)
    {
      m_wires.insert(std::pair(WireID(iCLayer, iWire).getEWire(), eff));
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
    std::map<unsigned short, float> getWires() const
    {
      return m_wires;
    }

    /**
     * Check if the wire is totally-dead (eff=0); to be replaced by isDeadWire() in future...
     * @param  wid wire id to be checked
     * @return true if dead wire; false if not
     */
    bool isBadWire(const WireID& wid) const
    {
      std::map<unsigned short, float>::const_iterator it = m_wires.find(wid.getEWire());
      if (it != m_wires.end() && it->second == 0.) {
        return true;
      } else {
        return false;
      }
    }

    /**
     * Check if dead wire
     * @param  wid wire id to be checked
     * @param  eff efficiency of the wirte
     * @return true if dead wire; false if not
     */
    bool isDeadWire(const WireID& wid, double& eff) const
    {
      std::map<unsigned short, float>::const_iterator it = m_wires.find(wid.getEWire());
      if (it != m_wires.end() && 0. <= it->second && it->second < 1.) {
        eff = it->second;
        return true;
      } else {
        return false;
      }
    }

    /**
     * Check if the wire is hot/noisy
     * @param  wid wire id to be checked
     * @return true if hot wire; false if not
     */
    bool isHotWire(const WireID& wid) const
    {
      std::map<unsigned short, float>::const_iterator it = m_wires.find(wid.getEWire());
      if (it != m_wires.end() && it->second > 1.) {
        return true;
      } else {
        return false;
      }
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
        std::cout << WireID(ent.first).getICLayer() << " " << WireID(ent.first).getIWire() << " " << ent.second << std::endl;
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
          fout << std::setw(2) << std::right << WireID(ent.first).getICLayer() << "  " << std::setw(3) << WireID(
                 ent.first).getIWire() << " " << ent.second << std::endl;
        }
        fout.close();
      }
    }

  private:
    std::map<unsigned short, float> m_wires; /**< badwire list*/

    ClassDef(CDCBadWires, 2); /**< ClassDef */
  };

} // end namespace Belle2
