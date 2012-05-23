/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_STRIP_H
#define SVD_STRIP_H

#include <svd/dataobjects/SVDDigit.h>
#include <utility>
#include <map>
#include <vector>

namespace Belle2 {

  namespace SVD {
    /**
     * Class to represent signal in one strip.
     *
     * This class wraps the data of SVDDigits for one strip. It also remembers
     * the indices of the contributing SVDDigits needed to create relations.
     */
    class StripData {
    public:

      /** (time, charge) data of signal samples. */
      typedef std::map<double, double> data_type;
      /** StoreArray indices to generating Digits. */
      typedef std::vector<int> indices_type;

      /** Constructor. */
      StripData(): m_charge(0), m_time(0), m_data(), m_indices() {}

      /** Add a digit (signal sample) to the strip data.
       * @param digit Pointer to the digit to be added.
       * @param index DataStore index of the digit.
       */
      void add(SVDDigit* digit, int index) {
        if (m_charge < digit->getCharge()) {
          m_charge = digit->getCharge();
          m_time = digit->getTime();
        }
        m_data.insert(std::make_pair(digit->getTime(), digit->getCharge()));
        m_indices.push_back(index);
      }
      /** Get maximum signal on the strip. */
      double getCharge() const { return m_charge; }
      /** Get time of signal maximum. */
      double getTime() const { return m_time; }
      /** Return the map of (time,charge) pairs. */
      const data_type& get() const { return m_data; }
      /** Return the index of the Digit in the collection */
      const indices_type& getIndices() const { return m_indices; }

    protected:
      /** maximum signal on the strip. */
      double m_charge;
      /** time of the maximum signal. FIXME: Do we need better estimates here? */
      double m_time;
      /** map of (time,signal) pairs */
      data_type m_data;
      /** DataStore indices of contributing digits */
      indices_type m_indices;
    };

  }

}

#endif //SVD_STRIP_H
