/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {

  /** Function to set name of PDF for cluster quality estimation.
   *  Legacy naming convention for PDFs distributed by sensor type
   *  rather than sensor ID, not used after 2018 MC calibrated PDFs.
   */

  inline void clusterPDFName(const VxdID& sensor, int size, int side, int maxClusterSize, std::string& PDFName,
                             std::string& errorPDFName, bool useLegacyNaming)
  {
    if (useLegacyNaming == true) {
      std::string sensorSide;
      if (side == 1) sensorSide = "u";
      if (side == 0) sensorSide = "v";

      if (size > maxClusterSize) size = maxClusterSize;
      std::string sensorName;
      if (sensor.getLayerNumber() == 3)  sensorName = "l3";
      if (sensor.getLayerNumber() > 3 && sensor.getSensorNumber() == 1)  sensorName = "trap";
      if (sensor.getLayerNumber() > 3 && sensor.getSensorNumber() > 1)  sensorName = "large";

      PDFName =  sensorSide + sensorName +  std::to_string(size);
      errorPDFName = PDFName + "Error";
    } else {

      std::string sensorSide;
      if (side == 1) sensorSide = "u";
      if (side == 0) sensorSide = "v";
      int layer = sensor.getLayerNumber();
      int ladder = sensor.getLadderNumber();
      int sens = sensor.getSensorNumber();

      if (size > maxClusterSize) size = maxClusterSize;

      PDFName =  std::to_string(layer) + "." + std::to_string(ladder) + "." + std::to_string(sens) + "."  + sensorSide + "."  +
                 std::to_string(size);
      errorPDFName = PDFName + "_Error";

    }
  }
} //Belle2 namespace
