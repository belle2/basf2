/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

  /** template class for scalar (one per side) calibrations */
  template <class T >
  class SVDCalibrationsScalar {

  public:
    typedef T calibrationType; /**< typedef of the calibration type class*/
    typedef calibrationType payloadContainerType; /**< typedef of the payload container, one per side */

    /*
     * this class is used to deal with a value for side
     */
    SVDCalibrationsScalar() {};

    /** default destructor*/
    ~SVDCalibrationsScalar() {};

    /** get the calibration of the side*/
    static inline calibrationType get(const payloadContainerType& svdScalar, unsigned int /*strip*/)
    {
      return svdScalar;
    }

    /** get a reference to the calibration of the side*/
    const static inline calibrationType& getReference(const payloadContainerType& svdScalar, unsigned int /*strip*/)
    {
      return svdScalar;
    }

    /** set the calibration of the side*/
    static inline void set(payloadContainerType& svdScalar, unsigned int /*strip*/,
                           const calibrationType& value)
    {
      svdScalar = value;
    }

    /**initialize the calibration scalar*/
    static void init(payloadContainerType& svdScalar, unsigned int /*layer*/,
                     unsigned int /*ladder*/ , unsigned int /*sensor*/,
                     unsigned int /*side*/, const T& defaultT)
    {
      svdScalar = defaultT;
    }
  };
}
