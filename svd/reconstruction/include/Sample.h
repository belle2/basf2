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
      Sample(const SVDDigit* digit, unsigned int index, float prev_charge = -1.0, float next_charge = -1.0):
        m_arrayIndex(index), m_cellID(digit->getCellID()), m_iTime(digit->getIndex()),
        m_charge(digit->getCharge()), m_prev_charge(prev_charge), m_next_charge(next_charge) {}
      /** Bare constructor,
       * to construct an empty sample, eg. for a seed in ClusterCandidate.
       */
      Sample():
        m_arrayIndex(0), m_cellID(0), m_iTime(0), m_charge(0) {}
      /** Less-than operator */
      bool operator<(const Sample& b)  const {
        return (m_cellID < b.getCellID()) || (m_cellID == b.getCellID() && m_iTime < b.getSampleIndex());
      }
      /** Greater-than operator */
      bool operator>(const Sample& b)  const {
        return (m_cellID > b.getCellID()) || (m_cellID == b.getCellID() && m_iTime > b.getSampleIndex());
      }
      /** Equality operator */
      bool operator==(const Sample& b) const {
        return m_cellID == b.getCellID() && m_iTime == b.getSampleIndex();
      }
      /** Shorthand to get the sample time */
      short getSampleIndex() const { return m_iTime; }
      /** Shorthand to get the strip ID */
      unsigned int getCellID() const { return m_cellID; }
      /** Shorthand to get the sample charge */
      float getCharge() const { return m_charge; }
      /** Return the index of the Digit in the collection */
      unsigned int getArrayIndex() const { return m_arrayIndex; }

      /** Shorthand to get the previous sample charge */
      float getPrevCharge() const { return m_prev_charge; }
      /** Shorthand to get the next sample charge */
      float getNextCharge() const { return m_next_charge; }

    protected:
      /** StoreArray index of the digit */
      unsigned int m_arrayIndex;
      /** Strip number of the digit */
      unsigned int m_cellID;
      /** Sample number of the digit */
      unsigned short m_iTime;
      /** Charge of the digit */
      float m_charge;
      /** Charge of the previous digit */
      float m_prev_charge;
      /** Charge of the next digit */
      float m_next_charge;
    };

  }

}

#endif //SVD_SAMPLE_H
