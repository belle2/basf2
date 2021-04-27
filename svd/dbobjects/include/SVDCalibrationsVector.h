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

  /** template class for vector (one per strip) calibrations */
  template <class T >
  class SVDCalibrationsVector {

  public:
    typedef T calibrationType; /**< typedef of the calibration type class*/
    typedef std::vector< calibrationType > payloadContainerType;  /**< typedef of the payload container, one per strip */

    /** default constructor*/
    SVDCalibrationsVector() {};

    /**default destructor*/
    ~SVDCalibrationsVector() {};

    /** get the calibration of the strip*/
    static inline calibrationType get(const payloadContainerType& svdVector, unsigned int strip)
    {
      return svdVector.at(strip);
    }

    /** get a reference to the calibration of the strip*/
    const static inline calibrationType& getReference(const payloadContainerType& svdVector, unsigned int strip)
    {
      return svdVector.at(strip);
    }

    /** set the calibration of the strip*/
    static inline void set(payloadContainerType& svdVector, unsigned int strip,
                           calibrationType value)
    {
      svdVector.at(strip) = value;
    }

    /**initialize the calibration vector*/
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
