/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Front-end settings: storage depths, lookback, readout windows etc.
   */
  class TOPFrontEndSetting: public TObject {

  public:

    /**
     * Default constructor
     */
    TOPFrontEndSetting()
    {}

    /**
     * Sets write depths.
     * Write depths are given as the number 128-sample wide write windows.
     * @param depths a vector of write depths
     */
    void setWriteDepths(const std::vector<int>& depths) {m_writeDepths = depths;}

    /**
     * Sets the number of lookback windows.
     * Lookback is given as the number of 64-sample wide storage windows.
     * @param lookback the number of lookback windows
     */
    void setLookbackWindows(int lookback) {m_lookbackWindows = lookback;}

    /**
     * Sets the number of readout windows.
     * These are 64-sample wide storage windows.
     * @param readoutWindows the number of readout windows
     */
    void setReadoutWindows(int readoutWindows) {m_readoutWindows = readoutWindows;}

    /**
     * Sets the number of extra windows between the one determined from lookback and
     * the first readout window.
     * These are 64-sample wide storage windows.
     * @param extra the number of extra windows
     */
    void setExtraWindows(int extra) {m_extraWindows = extra;}

    /**
     * Sets window shifts
     * @param shifts vector of size = 6 with window shifts according to revo9count
     */
    void setWindowShifts(std::vector<int> shifts);

    /**
     * Sets the offset of a photon peak within the readout window region.
     * This has to be calibrated with data in a such way that the reconstructed bunch
     * distribution peaks at zero.
     * @param offset offset given in RF clocks
     */
    void setOffset(int offset) {m_offset = offset;}

    /**
     * Returns write depths.
     * Write depths are given as the number 128-sample wide write windows.
     * @return write depths
     */
    const std::vector<int>& getWriteDepths() const {return m_writeDepths;}

    /**
     * Returns the number of lookback windows.
     * Lookback is given as the number of 64-sample wide storage windows.
     * @return the number of lookback windows
     */
    int getLookbackWindows() const {return m_lookbackWindows;}

    /**
     * Returns the number of readout windows.
     * These are 64-sample wide storage windows.
     * @return the number of readout windows
     */
    int getReadoutWindows() const {return m_readoutWindows;}

    /**
     * Returns the number of extra windows between the one determined from lookback and
     * the first readout window.
     * These are 64-sample wide storage windows.
     * @return the number of extra windows
     */
    int getExtraWindows() const {return m_extraWindows;}

    /**
     * Returns window shift at a given revo9count
     * @param revo9count revo9 count
     * @return window shift
     */
    int getWindowShift(unsigned revo9count) const {return m_windowShifts[revo9count % 6];}

    /**
     * Returns the offset of a photon peak within the readout window region.
     * @return the offset [RF clocks]
     */
    int getOffset() const {return m_offset;}


  private:

    std::vector<int> m_writeDepths; /**< write depths */
    int m_lookbackWindows = 0;  /**< number of lookback windows */
    int m_readoutWindows = 0;   /**< number of readout windows */
    int m_extraWindows = 0;     /**< number of extra windows */
    int m_windowShifts[6] = {0}; /**< window shifts as a function of revo9count%6 */
    int m_offset = 0; /**< position of photon peak within readout region [RF clocks] */

    ClassDef(TOPFrontEndSetting, 1); /**< ClassDef */

  };

} // end namespace Belle
