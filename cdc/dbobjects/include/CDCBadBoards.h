/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TObject.h>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /**
   * Database object for bad boards.
   */
  class CDCBadBoards: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCBadBoards() {}

    /**
     * Set a board  in the list
     * @param boardID wire-id to be registered
     * @param eff wire efficiency; specify 0 <= eff < 1 for dead wire; eff > 1 for hot/noisy wire
     */
    void setBoard(const short boardID, double eff = 0)
    {
      m_boards.insert(std::pair(boardID, eff));
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_boards.size();
    }

    /**
     * Get the whole map
     */
    std::map<unsigned short, float> getBoards() const
    {
      return m_boards;
    }

    /**
     * Check if dead board
     * @param  boardID wire id to be checked
     * @param  eff efficiency of the wirte
     * @return true if dead board; false if not
     */
    bool isDeadBoard(const short boardID, double& eff) const
    {
      std::map<unsigned short, float>::const_iterator it = m_boards.find(boardID);
      if (it != m_boards.end() && 0. <= it->second && it->second < 1.) {
        eff = it->second;
        return true;
      } else {
        return false;
      }
    }

    /**
     * Check if the board is hot/noisy
     * @param  boardID to be checked
     * @return true if hot wire; false if not
     */
    bool isHotBoard(const short boardID) const
    {
      std::map<unsigned short, float>::const_iterator it = m_boards.find(boardID);
      if (it != m_boards.end() && it->second > 1.) {
        return true;
      } else {
        return false;
      }
    }

    /**
     * Print out contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Bad CDC board list" << std::endl;
      std::cout << "#entries= " << m_boards.size() << std::endl;
      if (m_boards.size() > 0) {
        for (auto const& ent : m_boards) {
          std::cout << "BoardID: " << ent.first << " Efficiency: " << ent.second << std::endl;
        }
      }
    }

  private:
    std::map<unsigned short, float> m_boards; /**< badwire list*/

    ClassDef(CDCBadBoards, 1); /**< ClassDef */
  };

} // end namespace Belle2
