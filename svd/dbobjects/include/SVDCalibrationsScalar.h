/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Eugenio Paoloni, Giulia Casarosa             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING: PLEASE, PLEASE, PLEASE USE ONLY WITH CAUTION AT YOUR OWN RISK!*
 **************************************************************************/

#pragma once
#include <vector>

#include <svd/dbobjects/SVDCalibrationsBase.h>

namespace Belle2 {

  template <class T >
  class SVDCalibrationsScalar {

  public:
    typedef T calibrationType;
    typedef calibrationType payloadContainerType;

    /*
     * this class is used to deal with a value for side
     */

    SVDCalibrationsScalar() {};
    ~SVDCalibrationsScalar() {};
    static inline calibrationType get(const payloadContainerType& svdScalar, unsigned int /*strip*/)
    {
      return svdScalar;
    }

    static inline void set(payloadContainerType& svdScalar, unsigned int /*strip*/,
                           calibrationType value)
    {
      svdScalar = value;
    }

    static void init(payloadContainerType& svdScalar, unsigned int /*layer*/,
                     unsigned int /*ladder*/ , unsigned int /*sensor*/,
                     unsigned int /*side*/, const T& defaultT)
    {
      svdScalar = defaultT;
    }
  };
}
