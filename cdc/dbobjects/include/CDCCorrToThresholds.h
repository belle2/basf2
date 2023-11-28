/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <array>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TObject.h>

#include <cdc/geometry/CDCGeometryParConstants.h>

namespace Belle2 {

  /**
   * Database object for correcting a simple threshold model in MC
   */
  class CDCCorrToThresholds: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCCorrToThresholds() {}

    /**
     * Set the paramseter in the array
     * @param id layerID(0-55)
     * @param param parameter for correction
     */
    void setParam(unsigned short id, double param)
    {
      m_crs[id] = param;
    }

    /**
     * Get the no. of entries in the array
     */
    unsigned short getEntries() const
    {
      return m_crs.size();
    }

    /**
     * Get the whole array
     */
    std::array<float, c_maxNSenseLayers> getParams() const
    {
      return m_crs;
    }

    /**
     * Get the parameter for the id
     * @param  id layerID
     * @return parameter for the id
     */
    float getParam(unsigned short id) const
    {
      return m_crs[id];
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Correction parametres" << std::endl;
      std::cout << "#entries= " << m_crs.size() << std::endl;
      std::cout << "in order of id and parameter" << std::endl;

      for (unsigned id = 0; id < c_maxNSenseLayers; ++id) {
        std::cout << id << " " << m_crs[id] << std::endl;
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
        for (unsigned id = 0; id < c_maxNSenseLayers; ++id) {
          fout << id << " " << m_crs[id] << std::endl;
        }
      }
      fout.close();
    }

  private:
    std::array<float, c_maxNSenseLayers> m_crs; /**< cr array */

    ClassDef(CDCCorrToThresholds, 2); /**< ClassDef */
    // Version histroy:
    // v2: original.
  };

} // end namespace Belle2
