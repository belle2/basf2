/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Eugenio Paoloni                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING: PLEASE, PLEASE, PLEASE USE ONLY WITH CAUTION AT YOUR OWN RISK!*
 **************************************************************************/

#pragma once
#include <vector>

#include <svd/dbobjects/SVDCalibrationsBase.h>

namespace Belle2 {

  template <class T >
  class SVDCalibrationsVector {

  public:
    typedef T calibrationType;
    typedef std::vector< calibrationType > payloadContainerType;

    SVDCalibrationsVector() {};
    ~SVDCalibrationsVector() {};
    static inline calibrationType get(const payloadContainerType& svdVector, unsigned int strip)
    {
      return svdVector.at(strip);
    }

    static inline void set(payloadContainerType& svdVector, unsigned int strip,
                           calibrationType value)
    {
      svdVector.at(strip) = value;
    }

    static void init(payloadContainerType& svdVector, unsigned int layer,
                     unsigned int /*ladder*/ , unsigned int /*sensor*/,
                     unsigned int side, const T& defaultT)
    {
      unsigned int numberOfStrips = 0;

      if (layer < 3)
        return;

      switch (side) {
        case SVDCalibrationsBase< SVDCalibrationsVector >::Uindex :
          numberOfStrips = 768;
          break;

        case SVDCalibrationsBase< SVDCalibrationsVector >::Vindex :
          numberOfStrips = layer == 3 ? 768 : 512;
      }

      svdVector.resize(numberOfStrips , defaultT);


    }
  };
}
