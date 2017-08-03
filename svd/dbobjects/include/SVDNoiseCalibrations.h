/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING: Do not try to fry it with water. Use only olive oil.          *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/logging/Logger.h>

namespace Belle2 {

  /** This class defines the dbobject and the method to access SVD
   * calibrations from the noise local runs. It provides the strip noise
   * in ADC units.
   *
   * Currently the returned values are the default ones and they
   * are not read from any file.
   *
   */
  class SVDNoiseCalibrations : public TObject {
  public:
    /** Constructor, no input argument is required */
    SVDNoiseCalibrations()
    {}

    /** This is the method for getting the noise.
     *
     * Input:
     * @param sensor ID: identitiy of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     *
     * Output: integer corresponding to the strip noise in ADC counts.
     */
    int getNoise(VxdID , bool , unsigned char)
    {
      return 2;
    }
  private:

    ClassDef(SVDNoiseCalibrations, 0);
  };
}

