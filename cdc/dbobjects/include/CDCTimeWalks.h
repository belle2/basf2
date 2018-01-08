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

#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TObject.h>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /**
   * Database object for time-walk.
   */
  class CDCTimeWalks: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCTimeWalks() {}

    /**
     * Set the time-walk coefficient in the list
     * @param boardID front-end board id.
     * @param param   coefficient for the time-walk corr. term
     */
    void setTimeWalkParam(unsigned short boardID, float param)
    {
      m_tws.insert(std::pair<unsigned short, float>(boardID, param));
    }

    /**
     * Update the time-walk coefficient in the list
     * @param boardID front-end board id.
     * @param delta   delta-coefficient
     */
    void addTimeWalk(unsigned short boardID, float delta)
    {
      std::map<unsigned short, float>::iterator it = m_tws.find(boardID);
      it->second += delta;
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_tws.size();
    }

    /**
     * Get the whole list
     */
    std::map<unsigned short, float> getTimeWalkParams() const
    {
      return m_tws;
    }

    /**
     * Get the time-walk coefficient for the board
     * @param  boardID front-end board id.
     * @return time-walk coefficient for the board id.
     */
    float getTimeWalkParam(unsigned short boardID) const
    {
      std::map<unsigned short, float>::const_iterator it = m_tws.find(boardID);
      return it->second;
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Time walk coefficient list" << std::endl;
      std::cout << "#entries= " << m_tws.size() << std::endl;
      std::cout << "in order of board# and coefficient (ns*sqrt(ADCcount))" << std::endl;

      for (auto const& ent : m_tws) {
        std::cout << ent.first << " " << ent.second << std::endl;
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
        for (auto const& ent : m_tws) {
          fout << std::setw(3) << std::right << ent.first << "  " << std::setw(15) << std::scientific << std::setprecision(
                 8) << ent.second << std::endl;
        }
        fout.close();
      }
    }

    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 26;}
    /// Get global parameter
    double getGlobalParam(unsigned short element, unsigned short)
    {
      return getTimeWalkParam(element);
    }
    /// Set global parameter
    void setGlobalParam(double value, unsigned short element, unsigned short)
    {
      // TODO: this does not allow updates
      //setTimeWalkParam(element, value);
      // directly access the map
      m_tws[element] = value;
    }
    /// list stored global parameters
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams()
    {
      std::vector<std::pair<unsigned short, unsigned short>> result;
      for (auto id_timewalk : m_tws) {
        result.push_back({id_timewalk.first, 0});
      }
      return result;
    }
    // ------------------------------------------------------------------------

  private:
    std::map<unsigned short, float> m_tws; /**< tw list*/

    ClassDef(CDCTimeWalks, 1); /**< ClassDef */
  };

} // end namespace Belle2
