/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/reconstruction/PXDDCDSWBMapper.h>

using namespace std;

namespace Belle2 {

  namespace PXD {

    PXDDCDSWBMapper::~PXDDCDSWBMapper() {}

    /** get ID of DCD for giving pixel, range: 0..3. */
    int PXDDCDSWBMapper::getDCDID(const int u, const int v, const VxdID sensorID) const
    {
      //const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(
      //                                                              sensorID));
      int ret = 0;
      int iLayer = sensorID.getLayerNumber();
      int iSensor = sensorID.getSensorNumber();
      int chUMap;
      chUMap = u;
      int Direction = 1;
      if (((iLayer == 1) && (iSensor == 1)) || ((iLayer == 2) && (iSensor == 2))) {
        // inverted counting in U direction:
        chUMap = 249 - u;
        Direction = -1;
      }
      float fID;
      fID = (float)chUMap / 62.5;
      if ((chUMap == 62) || (chUMap == 187)) {
        if ((v % 4) >= 2) fID = fID + Direction;
      }
      ret = (int)fID;
      return ret;
    } //getDCDID

    /** get ID of SWB for giving pixel, range: 0..5. */
    int PXDDCDSWBMapper::getSWBID(const int v) const
    {
      return (int)(v / 128);
    }  //getSWBID

  } //PXD namespace
} //Belle2 namespace
