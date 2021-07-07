/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vector>
#include <bitset>
#include <climits>

#include <svd/dbobjects/SVDCalibrationsBase.h>

namespace Belle2 {

  /** class for digital (0/1) calibration values per srtip*/
  class SVDCalibrationsBitmap {

  public:
    typedef bool calibrationType; /**< typedef of the calibration value (1/0)*/
    typedef unsigned long bundleType; /**< typedef for the bundle type*/
    typedef std::vector< bundleType > payloadContainerType; /**< typedef of the vector of bundle type*/

    static const int nBitsInBundle = CHAR_BIT * sizeof(bundleType); /**< nuber of bits in the bundle*/

    /** default constructor*/
    SVDCalibrationsBitmap() {};

    /** default destructor*/
    ~SVDCalibrationsBitmap() {};

    /** get the calibration value of that strip*/
    static inline calibrationType get(const payloadContainerType& svdBitmap, unsigned int strip)
    {
      std::bitset<nBitsInBundle> bundle = svdBitmap.at(strip / (nBitsInBundle));
      return bundle[strip % nBitsInBundle];
    }

    /** set the calibration value of the strip*/
    static inline void set(payloadContainerType& svdBitmap, unsigned int strip,
                           calibrationType value)
    {
      std::bitset<nBitsInBundle> bundle = svdBitmap.at(strip / (nBitsInBundle));
      bundle[strip % nBitsInBundle] = value;
      svdBitmap.at(strip / (nBitsInBundle)) =  bundle.to_ulong();
    }

    /** initialize the calibration bitmap*/
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
