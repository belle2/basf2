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
     * @param fraction fraction of reconstructed buckets which are matched with filled ones
     */
    void set(int offset, double fraction = 0)
    {
      m_offset = offset;
      m_fraction = fraction;
      m_calibrated = true;
    }

    /**
     * Switch calibration status to uncalibrated
     */
    void setUnusable() {m_calibrated = false;}

    /**
     * Getter for offset
     * @return offset
     */
    int get() const {return m_offset;}

    /**
     * Getter for fraction
     * @return fraction of reconstructed buckets matched with filled ones
     */
    double getFraction() const {return m_fraction;}

    /**
     * Is calibrated?
     * @return true, if calibrated
     */
    bool isCalibrated() const {return m_calibrated;}

  private:

    int m_offset = 0; /**< offset [RF clock] */
    float m_fraction = 0; /**< fraction of reconstructed buckets matched with filled ones */
    bool m_calibrated = false; /**< calibration status */

    ClassDef(TOPCalFillPatternOffset, 2); /**< ClassDef */

  };

}


