/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <math.h>
#include <root/TObject.h>
#include <root/TVector2.h>
#include <root/TVector3.h>
#include <root/TMath.h>


namespace Belle2 {
//   namespace SVD {

  /**
  * The DATCON ROI class.
  */
  class DATCONROI : public TObject {
  public:

    DATCONROI() : sensorID(), v1Coordinate(), v2Coordinate(), v1Pixel(), v2Pixel() {}

    /** Constructor for DATCON ROI */
    DATCONROI(VxdID _sensorID, TVector2 _v1Coordinate, TVector2 _v2Coordinate):
      sensorID(_sensorID), v1Coordinate(_v1Coordinate), v2Coordinate(_v2Coordinate) {}

    /** Constructor providing coordinate and pixel information of the ROI */
    DATCONROI(VxdID _sensorID, TVector2 _v1Coordinate, TVector2 _v2Coordinate, TVector2 _v1Pixel, TVector2 _v2Pixel):
      sensorID(_sensorID), v1Coordinate(_v1Coordinate), v2Coordinate(_v2Coordinate), v1Pixel(_v1Pixel), v2Pixel(_v2Pixel) {}

    virtual ~DATCONROI() {}

    /** Get sensor ID */
    VxdID getSensorID() { return sensorID; }
    /** Get v1 and v2 */
    TVector2 getV1Coordinate() { return v1Coordinate; }
    TVector2 getV2Coordinate() { return v2Coordinate; }
    TVector2 getV1Pixel()      { return v1Pixel; }
    TVector2 getV2Pixel()      { return v2Pixel; }
  private:
    /** Sensor ID */
    VxdID sensorID;
    /** v1 (down left edge) and v2 (upper right edge) */
    TVector2 v1Coordinate;
    TVector2 v2Coordinate;
    TVector2 v1Pixel;
    TVector2 v2Pixel;
    /** Pixel IDs */

    ClassDef(DATCONROI, 1)

  }; // class DATCONROI

} // end namespace Belle2
