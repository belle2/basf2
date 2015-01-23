/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ASICWINDOWCONSTANTS_H
#define ASICWINDOWCONSTANTS_H

#include <TObject.h>
#include <TProfile.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Calibration constants of an ASIC window: pedestals, gains
     */
    class ASICWindowConstants : public TObject {
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
      ASICWindowConstants(): m_asicWindow(0), m_offset(0), m_unit(c_Unit), m_gainError(0) {
        for (unsigned i = 0; i < c_WindowSize; i++) m_pedestals[i] = 0;
        for (unsigned i = 0; i < c_WindowSize; i++) m_gains[i] = c_Unit;
      }

      /**
       * Constructor with ASIC window
       * @param asicWindow ASIC window ID
       */
      ASICWindowConstants(unsigned short asicWindow):
        m_asicWindow(asicWindow), m_offset(0), m_unit(c_Unit), m_gainError(0) {
        for (unsigned i = 0; i < c_WindowSize; i++) m_pedestals[i] = 0;
        for (unsigned i = 0; i < c_WindowSize; i++) m_gains[i] = c_Unit;
      }

      /**
       * Set pedestals from profile histogram with c_WindowSize bins
       * @param profile profile histogram
       * @return true, on success
       */
      bool setPedestals(const TProfile* profile);

      /**
       * Set gains
       * @param gains vector of relative gains (size must be c_WindowSize)
       * @param error error on gains
       * @return true, on success
       */
      bool setGains(const std::vector<float> gains, float error = 0);

      /**
       * Return ASIC window number
       * @return window ID
       */
      unsigned getAsicWindow() const {return m_asicWindow;}

      /**
       * Return window size (number of pedestal samples)
       * @return window size
       */
      unsigned getSize() const {return c_WindowSize;}

      /**
       * Return pedestal value of i-th sample
       * @param i sample number
       * @return pedestal value
       */
      float getPedestal(unsigned i) const {
        if (i < c_WindowSize) {
          return (m_pedestals[i] & 0x00FF) + m_offset;
        }
        return 0;
      }

      /**
       * Return pedestal uncertainly
       * @param i sample number
       * @return pedestal uncertainty
       */
      float getPedestalError(unsigned i) const {
        if (i < c_WindowSize) {
          return (m_pedestals[i] >> 8) / sqrt(12.0);
        }
        return 0;
      }

      /**
       * Return relative gain for i-th sample
       * @param i sample number
       * @return relative gain
       */
      float getGain(unsigned i) const {
        if (i < c_WindowSize) {
          return float(m_gains[i]) / float(m_unit);
        }
        return 0;
      }

      /**
       * Return gain uncertainly
       * @return gain uncertainty
       */
      float getGainError() const {
        return float(m_gainError) / float(m_unit);
      }


    private:

      unsigned short m_asicWindow;  /**< ASIC window number */
      unsigned short m_offset;      /**< common pedestal offset */
      unsigned short m_unit;        /**< unit of fixed point number */
      unsigned short m_gainError;   /**< gain error (fixed point format) */
      unsigned short m_pedestals[c_WindowSize]; /**< pedestals (packed: value, error) */
      unsigned short m_gains[c_WindowSize];  /**< gains (in fixed point format) */

      ClassDef(ASICWindowConstants, 1); /**< ClassDef */

    };

  } // end namespace TOP
} // end namespace Belle2

#endif
