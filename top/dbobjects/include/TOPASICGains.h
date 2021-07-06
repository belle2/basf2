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
   * Calibration constants of a single ASIC window: gains
   */
  class TOPASICGains : public TObject {
  public:

    /**
     * Various constants
     */
    enum {c_WindowSize = 64, /**< number of samples */
          c_Unit = 0x1000    /**< unit of fixed point number */
         };

    /**
     * Default constructor
     */
    TOPASICGains(): m_asicWindow(0), m_unit(c_Unit), m_gainError(0)
    {
      for (unsigned i = 0; i < c_WindowSize; i++) m_gains[i] = c_Unit;
    }

    /**
     * Constructor with ASIC window
     * @param asicWindow ASIC window number
     */
    explicit TOPASICGains(unsigned short asicWindow):
      m_asicWindow(asicWindow), m_unit(c_Unit), m_gainError(0)
    {
      for (unsigned i = 0; i < c_WindowSize; i++) m_gains[i] = c_Unit;
    }

    /**
     * Set gains
     * @param gains vector of relative gains (size must be c_WindowSize)
     * @param error error on gains
     * @return true, on success
     */
    bool setGains(const std::vector<float>& gains, float error = 0);

    /**
     * Return ASIC window number
     * @return window number
     */
    unsigned getASICWindow() const {return m_asicWindow;}

    /**
     * Return window size (number of pedestal samples)
     * @return window size
     */
    unsigned getSize() const {return c_WindowSize;}

    /**
     * Return relative gain for i-th sample
     * @param i sample number
     * @return relative gain
     */
    float getValue(unsigned i) const
    {
      if (i < c_WindowSize) {
        return float(m_gains[i]) / float(m_unit);
      }
      return 0;
    }

    /**
     * Return gain uncertainly
     * @return gain uncertainty
     */
    float getError() const
    {
      return float(m_gainError) / float(m_unit);
    }


  private:

    unsigned short m_asicWindow;  /**< ASIC window number */
    unsigned short m_unit;        /**< unit of fixed point number */
    unsigned short m_gainError;   /**< gain error (fixed point format) */
    unsigned short m_gains[c_WindowSize];  /**< gains (in fixed point format) */

    ClassDef(TOPASICGains, 1); /**< ClassDef */

  };

} // end namespace Belle2

