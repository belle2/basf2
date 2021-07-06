/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * Map of pixels and channels within the carrier board
   */
  class TOPChannelMap: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPChannelMap(): m_row(0), m_col(0), m_asic(0), m_channel(0)
    {}

    /**
     * Full constructor
     * @param row pixel row within the carrier board (0-based)
     * @param col pixel column within the carrier board (0-based)
     * @param asic ASIC number (0-based)
     * @param chan ASIC channel (0-based)
     */
    TOPChannelMap(unsigned row, unsigned col, unsigned asic, unsigned chan):
      m_row(row), m_col(col), m_asic(asic), m_channel(chan)
    {}

    /**
     * Return 0-based pixel row within carrier board
     * @return pixel row (0-based)
     */
    unsigned getRow() const {return m_row;}

    /**
     * Return 0-based pixel column within carrier board
     * @return pixel column (0-based)
     */
    unsigned getColumn() const {return m_col;}

    /**
     * Return pixel row number within carrier board
     * @return pixel row number
     */
    unsigned getRowNumber() const {return m_row + 1;}

    /**
     * Return pixel column number within carrier board
     * @return pixel column number
     */
    unsigned getColumnNumber() const {return m_col + 1;}

    /**
     * Return ASIC number
     * @return ASIC number (0-based)
     */
    unsigned getASICNumber() const {return m_asic;}

    /**
     * Return ASIC channel number
     * @return ASIC channel number (0-based)
     */
    unsigned getASICChannel() const {return m_channel;}

  private:

    unsigned m_row;     /**< pixel row within carrier board */
    unsigned m_col;     /**< pixel column within carrier board */
    unsigned m_asic;    /**< ASIC number */
    unsigned m_channel; /**< ASIC channel */

    ClassDef(TOPChannelMap, 1); /**< ClassDef */

  };

} // end namespace Belle2
