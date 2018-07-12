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

  /** This class defines the dbobject and the method to access the information
   * on bad strips flagged during local runs. It provides a
   * boolean indicating whether the strip is bad.
   *
   *
   */
  class SVDLocalRunBadStrips : public TObject {
  public:
    /** Constructor, no input argument is required */
    SVDLocalRunBadStrips()
    {}

    /** This is the method for knowing whether a strip is bad,
     * according to local runs results.
     * Currently, return "false" as default (= all good strips).
     *
     * Input:
     * @param sensor ID: identitiy of the sensor for which the
     * calibration is required
     * @param isU: sensor side, true for p side, false for n side
     * @param strip: strip number
     *
     * Output: boolean variable as flag for bad strips.
     */
    bool isBad(VxdID , bool , unsigned char) const
    {
      return false;
    }
  private:

    ClassDef(SVDLocalRunBadStrips, 1);
  };
}

