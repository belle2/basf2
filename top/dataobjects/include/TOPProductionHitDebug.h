/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * @param hasWaveform has waveform or not
     * @param isOnHeap is on heap or not
     * @param logicWindow logic window
     * @param physicalWindow physical window
     * @param isWindowStraddle is window straddle
     * @param isOnHeapStraddle is on heap straddle
     * @param logicWindowStraddle logic window straddle
     * @param physicalWindowStraddle physical window straddle
     */
    TOPProductionHitDebug(bool hasWaveform,
                          bool isOnHeap,
                          unsigned short logicWindow,
                          unsigned short physicalWindow,
                          bool isWindowStraddle = false,
                          bool isOnHeapStraddle = false,
                          unsigned short logicWindowStraddle = 0,
                          unsigned short physicalWindowStraddle = 0):
      m_hasWaveform(hasWaveform),
      m_isOnHeap(isOnHeap),
      m_logicWindow(logicWindow),
      m_physicalWindow(physicalWindow),
      m_isWindowStraddle(isWindowStraddle),
      m_isOnHeapStraddle(isOnHeapStraddle),
      m_logicWindowStraddle(logicWindowStraddle),
      m_physicalWindowStraddle(physicalWindowStraddle)
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
     * Returns isWindowStraddle flag showing if the hit was extracted from a window-straddling waveform
     * @return isWindowStraddle
     */
    bool isWindowStraddle() const { return m_isWindowStraddle; }


    /**
     * Returns isOnHeap flag for straddled window
     * @return isOnHeapStraddle
     */
    bool isOnHeapStraddle() const { return m_isOnHeapStraddle; }


    /**
     * Returns logic window address for straddled window
     * @return logicWindowStraddle
     */
    unsigned short getLogicWindowStraddle() const { return m_logicWindowStraddle; }

    /**
     * Returns physical window address for straddled window
     * @return physical window
     */
    unsigned short getPhysicalWindowStraddle() const { return m_physicalWindowStraddle; }

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
    bool m_isWindowStraddle = false;  /**< hit is from window-straddling waveform segment flag */
    bool m_isOnHeapStraddle = false;      /**< hit header heap flag for second half of waveform segment in case of straddling window*/
    unsigned short m_logicWindowStraddle =
      0;       /**< hit header logic window for second half of waveform segment in case of straddling window*/
    unsigned short m_physicalWindowStraddle =
      0;       /**< hit header heap window for second half of waveform segment in case of straddling window*/

    std::vector<unsigned int> m_extraWords; /**< additional event words */

    ClassDef(TOPProductionHitDebug, 1); /**< ClassDef */

  };


} // end namespace Belle2
