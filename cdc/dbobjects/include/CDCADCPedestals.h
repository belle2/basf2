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
#include <TObject.h>

namespace Belle2 {

  /**
   * Database object for ADC pedestals.
   */
  class CDCADCPedestals: public TObject {
  public:

    /**
     * Default constructor.
     */
    CDCADCPedestals() {}

    /**
     * Set ADC pedestals in the list.
     * @param board id
     * @param pedestal
     */
    void setPedestal(unsigned short board, float pedestal)
    {
      m_pedestals.insert(std::pair<unsigned short, float>(board, pedestal));
    }

    /**
     * Set all ADC pedestals to be 0.
     */
    void setZeros()
    {
      const float pedestal = 0.0;
      for (int ib = 0; ib < 300; ++ib) {
        m_pedestals.insert(std::pair<unsigned short, float>(ib, pedestal));
      }
    }

    /**
     * Get the no. of entries in the list.
     */
    unsigned short getEntries() const
    {
      return m_pedestals.size();
    }

    /**
     * Get the whole list.
     */
    std::map<unsigned short, float> getPedestals() const
    {
      return m_pedestals;
    }

    /**
     * Get ADC pedestal for the specified board.
     * @param  board id.
     * @return pedestal for the board
     */
    float getPedestal(const unsigned short& board) const
    {
      std::map<unsigned short, float>::const_iterator it = m_pedestals.find(board);
      return it->second;
    }

    /**
     * Print out all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "ADC pedestal list" << std::endl;
      std::cout << "# of entries= " << m_pedestals.size() << std::endl;
      std::cout << "in order of board#, pedestal" << std::endl;
      for (auto const& ent : m_pedestals) {
        std::cout << ent.first << " " << ent.second << std::endl;
      }
    }

  private:
    std::map<unsigned short, float> m_pedestals; /**< ADC pedestal list*/

    ClassDef(CDCADCPedestals, 0); /**< ClassDef */
  };

} // end namespace Belle2

