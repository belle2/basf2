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

#include <TObject.h>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /**
   * Database object for time walk.
   */
  class CDCTimeWalks: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCTimeWalks() {}

    /**
     * Set the time-walk coefficient in the list
     */
    void setTimeWalk(unsigned short boardID, double tw)
    {
      m_twList.insert(std::pair<unsigned short, float>(boardID, tw));
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_twList.size();
    }

    /**
     * Get the time-walk coefficient for the board
     */
    double getTimeWalk(unsigned short boardID)
    {
      std::map<unsigned short, float>::iterator it = m_twList.find(boardID);
      return it->second;
    }

    /**
     * Print all contents
     */
    void dump()
    {
      std::cout << " " << std::endl;
      std::cout << "Time walk coefficient list" << std::endl;
      std::cout << "#entries= " << m_twList.size() << std::endl;
      std::cout << "in order of boardID and coefficient (ns*sqrt(ADCcount))" << std::endl;

      for (auto const& ent : m_twList) {
        std::cout << ent.first << " " << ent.second << std::endl;
      }
    }

  private:
    std::map<unsigned short, float> m_twList; /**< tw list*/

    ClassDef(CDCTimeWalks, 1); /**< ClassDef */
  };

} // end namespace Belle2
