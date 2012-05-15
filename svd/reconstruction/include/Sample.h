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
     * ordered by time in ascending order.
     * For these purpose, the class uses time discretization based on a fixed
     * time corresponding to index 0, and the sampling time. These are implemented
     * as static members of the class, and are set by static setters.
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
      Sample(SVDDigit* digit, unsigned int index): m_digit(digit), m_index(index) {}
      /** Comparison operator */
      bool operator<(const Sample& b)  const { return getCellID() < b.getCellID() || (getCellID() == b.getCellID() && getTime() < b.getTime()); }
      /** Equality operator */
      bool operator==(const Sample& b) const { return getCellID() == b.getCellID() && getTime() == b.getTime(); }
      /** Return the sensorID of the pixel */
      VxdID getSensorID() const { return m_digit ? m_digit->getSensorID() : VxdID(0); }
      /** Check whether we have u- or v-strip. */
      bool isUStrip() const { return m_digit ? m_digit->isUStrip() : true; }
      /** Shorthand to get the sample time */
      unsigned int getTime() const
      { return m_digit ? int((m_digit->getTime() - m_baseTime) / m_samplingTime + 0.1) : -1; }
      /** Shorthand to get the strip ID */
      unsigned int getCellID() const { return m_digit ? m_digit->getCellID() : -1; }
      /** Shorthand to get the sample charge */
      float getCharge() const { return m_digit ? m_digit->getCharge() : 0;  }
      /** Return pointer to the wrapped Digit */
      SVDDigit* getDigit() const { return m_digit; }
      /** Return the index of the Digit in the collection */
      unsigned int getIndex() const { return m_index; }

      /** Set the base time for indexing on the time axis. */
      static void setBaseTime(double time);
      /** Set the sampling time. */
      static void setSamplingTime(double time);

    protected:
      /** Pointer to the wrapped digit */
      SVDDigit* m_digit;
      /** Index of the wrapped digit */
      unsigned int m_index;
      /** Time corresponding to the first sample (index 0) */
      static double m_baseTime;
      /** Sampling time is the time interval between consecutive signal samples.*/
      static double m_samplingTime;

    };

  }

}

#endif //SVD_SAMPLE_H
