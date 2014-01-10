/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_SAMPLE_H
#define SVD_SAMPLE_H

#include <svd/dataobjects/SVDDigit.h>

namespace Belle2 {

  namespace SVD {
    /**
     * Class to represent one signal sample
     *
     * This class is a wrapper around SVDDigit to provide ordered access.
     * Ordering is "strip-wise": first all digits belonging to the first strip,
     * ordered by time (index) in ascending order.
     *
     * This class also remembers the index of the SVDDigit which is needed
     * to manage relations.
     */
    class Sample {
    public:
      /** Constructor
       * @param digit Pointer to the digit to be wrapped by this pixel
       * @param index Index of the SVDDigit in the collection
       */
      Sample(SVDDigit* digit, unsigned int index): m_digit(digit), m_arrayIndex(index) {}
      /** Comparison operator */
      bool operator<(const Sample& b)  const { return getCellID() < b.getCellID() || (getCellID() == b.getCellID() && getSampleIndex() < b.getSampleIndex()); }
      /** Equality operator */
      bool operator==(const Sample& b) const { return getCellID() == b.getCellID() && getSampleIndex() == b.getSampleIndex(); }
      /** Return the sensorID of the pixel */
      VxdID getSensorID() const { return m_digit ? m_digit->getSensorID() : VxdID(0); }
      /** Check whether we have u- or v-strip. */
      bool isUStrip() const { return m_digit ? m_digit->isUStrip() : true; }
      /** Shorthand to get the sample time */
      short getSampleIndex() const
      { return m_digit ? m_digit->getIndex() : -1; }
      /** Shorthand to get the strip ID */
      unsigned int getCellID() const { return m_digit ? m_digit->getCellID() : -1; }
      /** Shorthand to get the sample charge */
      float getCharge() const { return m_digit ? m_digit->getCharge() : 0;  }
      /** Return pointer to the wrapped Digit */
      SVDDigit* getDigit() const { return m_digit; }
      /** Return the index of the Digit in the collection */
      unsigned int getArrayIndex() const { return m_arrayIndex; }

    protected:
      /** Pointer to the wrapped digit */
      SVDDigit* m_digit;
      /** Index of the wrapped digit */
      unsigned int m_arrayIndex;

    };

  }

}

#endif //SVD_SAMPLE_H
