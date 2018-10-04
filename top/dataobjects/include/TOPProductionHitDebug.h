/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oskar Hartbrich                                          *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to store debugging information about the hit headers in the TOP production debugging raw data format. Access these through their relations to TOPRawDigits, otherwise there is no information which scrod/carrier/asic/channel they belong to.
   */
  class TOPProductionHitDebug : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPProductionHitDebug()
    {}

    /**
     * Full constructor
     * @param scrodID hardware SCROD ID
     */
    TOPProductionHitDebug(bool hasWaveform,
                          bool isOnHeap,
                          unsigned short logicWindow,
                          unsigned short physicalWindow):
      m_hasWaveform(hasWaveform),
      m_isOnHeap(isOnHeap),
      m_logicWindow(logicWindow),
      m_physicalWindow(physicalWindow)
    {
      m_extraWords.clear();
    }


    /**
     * Returns hasWaveform flag
     * @return hasWaveform
     */
    bool hasWaveform() const { return m_hasWaveform; }

    /**
     * Returns isOnHeap flag
     * @return isOnHeap
     */
    bool isOnHeap() const { return m_isOnHeap; }


    /**
     * Returns logic window address
     * @return logic window
     */
    unsigned short getLogicWindow() const { return m_logicWindow; }

    /**
     * Returns physical window address
     * @return physical window
     */
    unsigned short getPhysicalWindow() const { return m_physicalWindow; }

    /**
     * Appends extra word to vector of additional event words
     */
    void appendExtraWord(unsigned int extraWord) { m_extraWords.push_back(extraWord); }

    /**
     * Returns additional event words
     * @return extraWords
     */
    std::vector<unsigned int> getExtraWords() const { return m_extraWords; }


  private:
    bool m_hasWaveform = false;       /**< hit header waveform flag */
    bool m_isOnHeap = false;      /**< hit header heap flag */
    unsigned short m_logicWindow = 0;       /**< hit header logic window */
    unsigned short m_physicalWindow = 0;       /**< hit header heap window */

    std::vector<unsigned int> m_extraWords; /**< additional event words */

    ClassDef(TOPProductionHitDebug, 1); /**< ClassDef */

  };


} // end namespace Belle2
