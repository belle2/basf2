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
     * Set tw parameterization mode
     * mode=0: tw (in ns) = p0/sqrt(FADCsum);
     * mode=1: tw( in ns) = p0*exp(-p1*FADCsum).
     */
    void setTwParamMode(unsigned short mode)
    {
      m_twParamMode = mode;
    }

    /**
     * Set the time-walk coefficients in the list
     * @param boardID front-end board id.
     * @param params  coefficients for the time-walk corr. term
     */
    void setTimeWalkParams(unsigned short boardID, const std::vector<float>& params)
    {
      m_nTwParams = params.size();
      m_tws.insert(std::pair<unsigned short, std::vector<float>>(boardID, params));
    }

    /**
     * Update the time-walk coefficients in the list
     * @param boardID front-end board id.
     * @param deltas  delta-coefficients
     */
    void addTimeWalks(unsigned short boardID, const std::vector<float>& deltas)
    {
      std::map<unsigned short, std::vector<float>>::iterator it = m_tws.find(boardID);
      if (it != m_tws.end()) {
        for (unsigned short i = 0; i < m_nTwParams; ++i) {
          (it->second)[i] += deltas[i];
        }
      } else {
        B2FATAL("Specified tw params not found in addTimeWalks !");
      }
    }


    /**
     * Get tw parameterization mode
     */
    unsigned short getTwParamMode()
    {
      return m_twParamMode;
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
    std::map<unsigned short, std::vector<float>> getTimeWalkParams() const
    {
      return m_tws;
    }

    /**
     * Get the time-walk coefficients for the board
     * @param  boardID front-end board id.
     * @return time-walk coefficients for the board id.
     */
    const std::vector<float>& getTimeWalkParams(unsigned short boardID) const
    {
      std::map<unsigned short, std::vector<float>>::const_iterator it = m_tws.find(boardID);
      if (it != m_tws.end()) {
        return it->second;
      } else {
        B2FATAL("Specified tw params not found in getTimeWalks !");
      }
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Time walk coefficient list" << std::endl;
      std::cout << "#entries= " << m_tws.size() << std::endl;
      std::cout << m_twParamMode << "  " << m_nTwParams << std::endl;
      std::cout << "in order of board# and coefficients" << std::endl;

      for (auto const& ent : m_tws) {
        std::cout << ent.first;
        for (unsigned short i = 0; i < m_nTwParams; ++i) {
          std::cout << "  " << (ent.second)[i];
        }
        std::cout << std::endl;
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
        fout << m_twParamMode << "  " << m_nTwParams << std::endl;
        for (auto const& ent : m_tws) {
          fout << std::setw(3) << std::right << ent.first;
          for (unsigned short i = 0; i < m_nTwParams; ++i) {
            fout << "  " << std::setw(15) << std::scientific << std::setprecision(8) << ent.second[i];
          }
          fout << std::endl;
        }
        fout.close();
      }
    }

    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 26;}
    /// Get global parameter
    double getGlobalParam(unsigned short element, unsigned short i)
    {
      return (getTimeWalkParams(element))[i];
    }
    /// Set global parameter
    void setGlobalParam(double value, unsigned short element, unsigned short i)
    {
      // TODO: this does not allow updates
      //setTimeWalkParam(element, value);
      // directly access the map
      //      m_tws[element] = value;
      std::map<unsigned short, std::vector<float>>::iterator it = m_tws.find(element);
      if (it != m_tws.end()) {
        it->second[i] = value;
      } else {
        B2FATAL("Specified tw params not found in setGlobalParam !");
      }
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
    unsigned short m_twParamMode =
      0; /*!< Mode for tw parameterization; the initial value should be the one for classdef=1; do not modify it. */
    unsigned short m_nTwParams =
      1;   /*!< No. of tw parameters; the initial value should be the one for classdef=1; do not modify it. */
    std::map<unsigned short, std::vector<float>> m_tws; /**< tw list */

    ClassDef(CDCTimeWalks, 2); /**< ClassDef */
  };

} // end namespace Belle2
