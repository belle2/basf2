/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vxd/dataobjects/VxdID.h>

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

    ClassDef(SVDLocalRunBadStrips, 1); /**< needed by root*/
  };
}

