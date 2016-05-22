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

#include <TObject.h>

namespace Belle2 {

  /**
   * Database object for signal propagation speed along the wire.
   */
  class CDCPropSpeeds: public TObject {
  public:

    enum {c_nSlayers = 56};

    /**
     * Default constructor
     */
    CDCPropSpeeds() {}

    /**
     * Set the speed in the list.
     */
    void setSpeed(unsigned short iCLayer, double speed)
    //    void setSpeed(double speed)
    {
      //      m_speedList.push_back(speed);
      //      m_speedList[iCLayer] = speed;
      m_speedList.at(iCLayer) = speed;
    }

    /**
     * Update the speed in the list.
     */
    void addSpeed(unsigned short iCLayer, double delta)
    {
      m_speedList.at(iCLayer) += delta;
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_speedList.size();
    }

    /**
     * Get the whole list
     */
    std::vector<float> getList() const
    {
      return m_speedList;
    }

    /**
     * Get the speed for the specified cLayer
     */
    double getSpeed(unsigned short iCLayer) const
    {
      return m_speedList[iCLayer];
    }

    /**
     * Print out contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "PropSpeed list" << std::endl;
      std::cout << "#entries= " << m_speedList.size() << std::endl;
      std::cout << "in order of clayer and speed (cm/ns)" << std::endl;

      for (unsigned short iCL = 0; iCL < m_speedList.size(); ++iCL) {
        std::cout << iCL << " " << m_speedList[iCL] << std::endl;
      }
    }

  private:
    //    std::vector<float> m_speedList; /**< list of speed*/
    std::vector<float> m_speedList = std::vector<float>(c_nSlayers); /**< list of speed*/
    ClassDef(CDCPropSpeeds, 1); /**< ClassDef */
  };

} // end namespace Belle2
