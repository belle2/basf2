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

#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TObject.h>
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
     * @param iCLayer (continuous) layer id.
     * @param iWire   wire id. in the layer
     * @param t0      offset
     */
    void setT0(unsigned short iCLayer, unsigned short iWire, float t0)
    {
      m_t0s.insert(std::pair<unsigned short, float>(WireID(iCLayer, iWire).getEWire(), t0));
    }

    /**
     * Set t0 in the list
     * @param wid wire id.
     * @param t0  offset
     */
    void setT0(const WireID& wid, float t0)
    {
      m_t0s.insert(std::pair<unsigned short, float>(wid.getEWire(), t0));
    }

    /**
     * Update t0 in the list
     * @param iCLayer (continuous) layer id.
     * @param iWire   wire id. in the layer
     * @param delta   delta-offset
     */
    void addT0(unsigned short iCLayer, unsigned short iWire, float delta)
    {
      WireID wid(iCLayer, iWire);
      std::map<unsigned short, float>::iterator it = m_t0s.find(wid.getEWire());
      it->second += delta;
    }

    /**
     * Update t0 in the list
     * @param wid   wire id.
     * @param delta delta-offset
     */
    void addT0(const WireID& wid, float delta)
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
     * Get the whole list
     */
    std::map<unsigned short, float> getT0s() const
    {
      return m_t0s;
    }

    /**
     * Get t0 for the specified wire
     * @param  wire id.
     * @return t0 for the wire
     */
    float getT0(const WireID& wid) const
    {
      std::map<unsigned short, float>::const_iterator it = m_t0s.find(wid.getEWire());
      return it->second;
    }

    /**
     * Print out all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "t0 list" << std::endl;
      std::cout << "# of entries= " << m_t0s.size() << std::endl;
      std::cout << "in order of clayer#, wire#, t0" << std::endl;
      for (auto const& ent : m_t0s) {
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
        for (auto const& ent : m_t0s) {
          fout << std::setw(2) << std::right << WireID(ent.first).getICLayer() << "  " << std::setw(3) << WireID(
                 ent.first).getIWire() << "  " << std::setw(15) << std::scientific << std::setprecision(8) << ent.second << std::endl;
        }
        fout.close();
      }
    }

    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 25;}
    /// Get global parameter
    double getGlobalParam(unsigned short element, unsigned short)
    {
      return getT0(WireID(element));
    }
    /// Set global parameter
    void setGlobalParam(double value, unsigned short element, unsigned short)
    {
      WireID wire(element);
      //This does not work, tries to insert, but we need an update
      //setT0(wire.getICLayer(), wire.getIWire(), value);
      m_t0s[wire.getEWire()] = value;
    }
    /// list stored global parameters
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams()
    {
      std::vector<std::pair<unsigned short, unsigned short>> result;
      for (auto id_t0 : getT0s()) {
        result.push_back({id_t0.first, 0});
      }
      return result;
    }
    // ------------------------------------------------------------------------

  private:
    std::map<unsigned short, float> m_t0s; /**< t0 list*/

    ClassDef(CDCTimeZeros, 2); /**< ClassDef */
  };

} // end namespace Belle2

