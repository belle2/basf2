/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXD_PIXEL_H
#define PXD_PIXEL_H

#include <pxd/dataobjects/PXDDigit.h>

namespace Belle2 {

  namespace PXD {
    /**
     * Class to represent one pixel
     *
     * This class is a wrapper around PXDDigit to provide ordered access.
     * Ordering is rowwise: first all digits belonging to the first row,
     * ordered by column in ascending order.
     *
     * This class also remembers the index of the PXDDigit which is needed
     * for relation creation.
     */
    class Pixel {
    public:
      /** Constructor
       * @param digit Pointer to the digit to be wrapped by this pixel
       * @param index Index of the PXDDigit in the collection
       */
      Pixel(PXDDigit* digit, unsigned int index): m_digit(digit), m_index(index) {}
      /** Comparison operator */
      bool operator<(const Pixel &b)  const { return getV() < b.getV() || (getV() == b.getV() && getU() < b.getU()); }
      /** Equality operator */
      bool operator==(const Pixel &b) const { return getV() == b.getV() && getU() == b.getU(); }
      /** Shorthand to get the pixel column ID */
      unsigned int getU() const { return m_digit->getUCellID(); }
      /** Shorthand to get the pixel row ID */
      unsigned int getV() const { return m_digit->getVCellID(); }
      /** Shorthand to get the pixel charge */
      float getCharge() const { return m_digit ? m_digit->getCharge() : 0;  }
      /** Return pointer to the wrapped Digit */
      PXDDigit *get() const { return m_digit; }
      /** Return the index of the Digit in the collection */
      unsigned int getIndex() const { return m_index; }
    protected:
      /** Pointer to the wrapped digit */
      PXDDigit *m_digit;
      /** Index of the wrapped digit */
      unsigned int m_index;
    };

  }

}

#endif //PXD_PIXEL_H
