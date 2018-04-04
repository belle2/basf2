/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING: PLEASE, PLEASE, PLEASE USE ONLY WITH CAUTION AT YOUR OWN RISK!*
 **************************************************************************/

#pragma once
#include <vector>
#include <bitset>
#include <climits>

#include <svd/dbobjects/SVDCalibrationsBase.h>

namespace Belle2 {

  class SVDCalibrationsBitmap {

  public:
    typedef bool calibrationType;
    typedef unsigned long bundleType;
    typedef std::vector< bundleType > payloadContainerType;

    static const int nBitsInBundle = CHAR_BIT * sizeof(bundleType);

    SVDCalibrationsBitmap() {};
    ~SVDCalibrationsBitmap() {};
    static inline calibrationType get(const payloadContainerType& svdBitmap, unsigned int strip)
    {
      std::bitset<nBitsInBundle> bundle = svdBitmap.at(strip / (nBitsInBundle));
      return bundle[strip % nBitsInBundle];
    }

    static inline void set(payloadContainerType& svdBitmap, unsigned int strip,
                           calibrationType value)
    {
      std::bitset<nBitsInBundle> bundle = svdBitmap.at(strip / (nBitsInBundle));
      bundle[strip % nBitsInBundle] = value;
      svdBitmap.at(strip / (nBitsInBundle)) =  bundle.to_ulong();
    }

    static void init(payloadContainerType& svdBitmap, unsigned int layer,
                     unsigned int /*ladder*/ , unsigned int /*sensor*/,
                     unsigned int side, bool defaultB)
    {
      unsigned int numberOfStrips = 0;

      if (layer < 3)
        return;

      switch (side) {
        case SVDCalibrationsBase< SVDCalibrationsBitmap >::Uindex :
          numberOfStrips = 768;
          break;

        case SVDCalibrationsBase< SVDCalibrationsBitmap >::Vindex :
          numberOfStrips = layer == 3 ? 768 : 512;
      }

      if (!defaultB)
        svdBitmap.resize(numberOfStrips / nBitsInBundle, 0);
      else
        svdBitmap.resize(numberOfStrips / nBitsInBundle, ULONG_MAX);
    }
  };
}
