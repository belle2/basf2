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
   * Class to store the offset of reconstructed fill pattern
   */
  class TOPCalFillPatternOffset: public TObject {

  public:

    /**
     * Default constructor
     */
    TOPCalFillPatternOffset()
    {}

    /**
     * Setter
     * @param offset value to be set
     */
    void set(int offset)
    {
      m_offset = offset;
      m_calibrated = true;
    }

    /**
     * Getter
     * @return offset
     */
    int get() const {return m_offset;}

    /**
     * Is calibrated?
     * @return true, if calibrated
     */
    bool isCalibrated() const {return m_calibrated;}

  private:

    int m_offset = 0; /**< offset [RF clock] */
    bool m_calibrated = false; /**< calibration status */

    ClassDef(TOPCalFillPatternOffset, 1); /**< ClassDef */

  };

}


