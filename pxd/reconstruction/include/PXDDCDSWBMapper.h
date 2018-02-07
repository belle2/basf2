/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDCDSWBMAPPER_H
#define PXDDCDSWBMAPPER_H

#include <cstring>
#include <deque>
#include <vxd/geometry/GeoCache.h>

namespace Belle2 {
  namespace PXD {


    /** Class to return ID of DCD and SWB (switcher) chip in sensor.
     * Deatails: in PXD WhiteBook 3.1.3 Sensor Design and Appendix #3
     * https://confluence.desy.de/display/BI/PXD+WebHome?preview=/34029260/56330158/PXDwb.pdf
     *
     */
    class PXDDCDSWBMapper {
    public:

      /** Delete the cache and free the memory */
      ~PXDDCDSWBMapper();

      /** get ID of DCD for giving pixel, range: 0..3.
       * @param u pixels coordinate in u
       * @param v pixels coordinate in u
       * @param sensorID sensor details for PXD sensors
       * @returns DCD ID
       */
      int getDCDID(const int u, const int v, const VxdID sensorID) const;
      /** get ID of SWB for giving pixel, range: 0..5.
       * @param v pixels coordinate in u
       * @returns SWB ID
       */
      int getSWBID(const int v) const;

    };

  }

}

#endif //PXDDCDSWBMAPPER_H
