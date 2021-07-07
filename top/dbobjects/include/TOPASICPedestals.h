/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TProfile.h>
#include <vector>

namespace Belle2 {

  /**
   * Calibration constants of a single ASIC window: pedestals
   */
  class TOPASICPedestals : public TObject {
  public:

    /**
     * Various constants
     */
    enum {c_WindowSize = 64, /**< number of samples */
          c_Bits       = 9   /**< number of bits reserved for pedestal value */
         };

    /**
     * Default constructor
     */
    TOPASICPedestals(): m_asicWindow(0), m_offset(0)
    {
      for (unsigned i = 0; i < c_WindowSize; i++) m_pedestals[i] = 0;
    }

    /**
     * Constructor with ASIC window number
     * @param asicWindow ASIC window number
     */
    explicit TOPASICPedestals(unsigned short asicWindow): m_asicWindow(asicWindow),
      m_offset(0)
    {
      for (unsigned i = 0; i < c_WindowSize; i++) m_pedestals[i] = 0;
    }

    /**
     * Set pedestals from profile histogram with c_WindowSize bins
     * @param profile profile histogram
     * @param average optional value to add
     * @return number of pedestals that can not be packed into 16-bit word
     */
    int setPedestals(const TProfile* profile, double average = 0);

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
     * Return pedestal value of i-th sample
     * @param i sample number
     * @return pedestal value
     */
    float getValue(unsigned i) const
    {
      if (i < c_WindowSize) {
        unsigned short mask = (1 << c_Bits);
        mask--;
        return (m_pedestals[i] & mask) + m_offset;
      }
      return 0;
    }

    /**
     * Return pedestal uncertainly of i-th sample
     * @param i sample number
     * @return pedestal uncertainty
     */
    float getError(unsigned i) const
    {
      if (i < c_WindowSize) {
        return (m_pedestals[i] >> c_Bits);
      }
      return 0;
    }

    /**
     * Check whether the pedestal of i-th sample is valid
     * @param i sample number
     * @return true if available
     */
    bool isValid(unsigned i) const
    {
      if (i < c_WindowSize) {
        return (m_pedestals[i] != 0);
      }
      return false;
    }


    /**
     * Return number of un-valid pedestals (e.g. those cannot be packed into 16-bit word)
     * @return number of un-valid pedestals
     */
    unsigned getNumofUnvalid() const
    {
      unsigned bad = 0;
      for (int i = 0; i < c_WindowSize; i++) {
        if (!isValid(i)) bad++;
      }
      return bad;
    }

  private:

    /**
     * Return the offset that can allow for the maximal number of good pedestal samples
     * @param values pedestal values
     * @param errors pedestal errors
     * @param maxDif maximal number that can be stored in c_Bits
     * @param maxErr maximal number that can be stored in 16 - c_Bits
     * @return offset value
     */
    unsigned getOptimizedOffset(const std::vector<unsigned>& values,
                                const std::vector<unsigned>& errors,
                                unsigned maxDif,
                                unsigned maxErr);

    unsigned short m_asicWindow;  /**< ASIC window number */
    unsigned short m_offset;      /**< common pedestal offset */
    unsigned short m_pedestals[c_WindowSize]; /**< pedestals (packed: value, error) */

    ClassDef(TOPASICPedestals, 1); /**< ClassDef */

  };

} // end namespace Belle2

