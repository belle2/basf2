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
#include <iostream>

#include <TObject.h>

namespace Belle2 {

  /**
   * Database object of CDC.
   */
  class CDCPropSpeeds: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCPropSpeeds() {}

    /**
     * Set the speed in the list
     */
    //    void setSpeed(unsigned short iCLayer, float speed)
    void setSpeed(float speed)
    {
      m_speedList.push_back(speed);
      //      m_speedList[iCLayer] = speed;
      //      std::cout << m_speedList.size() <<" "<< iCLayer <<" "<< speed << std::endl;
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_speedList.size();
    }

    /**
     * Get the speed
     */
    float getSpeed(unsigned short iCLayer) const
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

      for (unsigned short CL = 0; CL < m_speedList.size(); ++CL) {
        std::cout << CL << " " << m_speedList[CL] << std::endl;
      }
    }

  private:
    std::vector<float> m_speedList; /**< list of speed*/
    //    std::vector<float> m_speedList(56); /**< list of speed*/

    ClassDef(CDCPropSpeeds, 1); /**< ClassDef */
  };

} // end namespace Belle2
