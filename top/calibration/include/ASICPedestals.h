/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ASICPEDESTALS_H
#define ASICPEDESTALS_H

#include <TObject.h>
#include <TProfile.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Calibration constants of a single ASIC window: pedestals
     */
    class ASICPedestals : public TObject {
    public:

      /**
       * Various constants
       */
      enum {c_WindowSize = 64, /**< number of samples */
           };

      /**
       * Default constructor
       */
      ASICPedestals(): m_asicWindow(0), m_offset(0)
      {
        for (unsigned i = 0; i < c_WindowSize; i++) m_pedestals[i] = 0;
      }

      /**
       * Constructor with ASIC window
       * @param asicWindow ASIC window ID
       */
      ASICPedestals(unsigned short asicWindow): m_asicWindow(asicWindow), m_offset(0)
      {
        for (unsigned i = 0; i < c_WindowSize; i++) m_pedestals[i] = 0;
      }

      /**
       * Set pedestals from profile histogram with c_WindowSize bins
       * @param profile profile histogram
       * @return true, on success
       */
      bool setPedestals(const TProfile* profile);

      /**
       * Return ASIC window number
       * @return window ID
       */
      unsigned getAsicWindowID() const {return m_asicWindow;}

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
      float getValue(unsigned i) const
      {
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
      float getError(unsigned i) const
      {
        if (i < c_WindowSize) {
          return (m_pedestals[i] >> 8) / sqrt(12.0);
        }
        return 0;
      }

    private:

      unsigned short m_asicWindow;  /**< ASIC window number */
      unsigned short m_offset;      /**< common pedestal offset */
      unsigned short m_pedestals[c_WindowSize]; /**< pedestals (packed: value, error) */

      ClassDef(ASICPedestals, 1); /**< ClassDef */

    };

  } // end namespace TOP
} // end namespace Belle2

#endif
