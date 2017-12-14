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
  class CDCADCDeltaPedestals: public TObject {
  public:

    /**
     * Default constructor.
     */
    CDCADCDeltaPedestals() {}


    /**
     * Set time window for sampling.
     * @param sample
     */
    void setSamplingWindow(unsigned short sample)
    {
      m_samplingWindow = sample;
    }

    /**
     * Set ADC pedestals in the list.
     * @param board id
     * @param pedestal
     */
    void setPedestal(int board, int ch, float pedestal)
    {
      unsigned short mask =  ch == -1 ? 0x8000 : 0;
      unsigned short id = mask == 0x8000 ? (mask | board) : (mask | (ch << 9) | board);
      pedestal *= m_samplingWindow;
      m_pedestals.insert(std::pair<unsigned short, float>(id, pedestal));
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
     * @param  board channel.
     * @return pedestal for the board (and channel).
     */
    float getPedestal(const unsigned short& board, const unsigned short& ch) const
    {
      unsigned short id0 = (0x8000 | board);
      std::map<unsigned short, float>::const_iterator it0 = m_pedestals.find(id0);
      if (it0 != m_pedestals.end()) { // board by board delta pedestal
        return it0->second;
      } else {
        unsigned short id = ((ch << 9) | board);
        std::map<unsigned short, float>::const_iterator it = m_pedestals.find(id);
        if (it != m_pedestals.end()) { //delta pedetal with (board, ch).
          return it->second;
        } else {
          return 0.0;
        }
      }
    }

    /**
     * Print out all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "ADC pedestal list" << std::endl;
      std::cout << "samle" << m_samplingWindow <<  std::endl;
      std::cout << "# of entries= " << m_pedestals.size() << std::endl;
      std::cout << "in order of board#, ch#, pedestal" << std::endl;
      for (auto const& ent : m_pedestals) {
        if ((ent.first & 0x8000) > 0) {
          std::cout << (ent.first & 0x1ff) << " " << -1 << " " << ent.second << std::endl;
        } else {
          std::cout << (ent.first & 0x1ff) << " " << ((ent.first & 0x7e00) >> 9) << " " << ent.second << std::endl;
        }
      }
    }

  private:
    std::map<unsigned short, float> m_pedestals; /**< ADC pedestal list */
    unsigned short m_samplingWindow = 10;  /**< ADC sampling window */
    ClassDef(CDCADCDeltaPedestals, 1); /**< ClassDef */
  };

} // end namespace Belle2

