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
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TObject.h>

namespace Belle2 {

  /**
   * Database object for signal propagation speed along the wire.
   */
  class CDCPropSpeeds: public TObject {
  public:

    /**
     * Constant
     */
    enum {c_nSlayers = 56};  /**< total no. of layers */

    /**
     * Default constructor
     */
    CDCPropSpeeds() {}

    /**
     * Set the speed in the list.
     * @param iCLayer (continuous) layer-id,
     * @param speed   propagation speed for the layer
     */
    void setSpeed(unsigned short iCLayer, float speed)
    {
      m_speeds.at(iCLayer) = speed;
    }

    /**
     * Update the speed in the list.
     * @param iClayer (continuous) layer-id,
     * @param delta   delta-speed for the layer
     */
    void addSpeed(unsigned short iCLayer, float delta)
    {
      m_speeds.at(iCLayer) += delta;
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_speeds.size();
    }

    /**
     * Get the whole list
     */
    std::vector<float> getSpeeds() const
    {
      return m_speeds;
    }

    /**
     * Get the speed for the specified layer
     * @param  iCLayer (continuous) layer-id.
     * @return propagation speed for the layer
     */
    float getSpeed(unsigned short iCLayer) const
    {
      return m_speeds[iCLayer];
    }

    /**
     * Print out contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "PropSpeed list" << std::endl;
      std::cout << "#entries= " << m_speeds.size() << std::endl;
      std::cout << "in order of clayer and speed (cm/ns)" << std::endl;

      for (unsigned short iCL = 0; iCL < m_speeds.size(); ++iCL) {
        std::cout << iCL << " " << m_speeds[iCL] << std::endl;
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
        for (unsigned short iCL = 0; iCL < m_speeds.size(); ++iCL) {
          fout << std::setw(2) << std::right << iCL << "  " << std::setw(15) << std::scientific << std::setprecision(
                 8) << m_speeds[iCL] << std::endl;
        }
        fout.close();
      }
    }

  private:
    //    std::vector<float> m_speeds; /**< list of speed*/
    std::vector<float> m_speeds = std::vector<float>(c_nSlayers); /**< list of speed*/
    ClassDef(CDCPropSpeeds, 1); /**< ClassDef */
  };

} // end namespace Belle2
