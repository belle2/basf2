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

  /** Function to maintain common naming convention between calibration occupancy file
   *  generation and occupancy value extraction in main crossTalkFinder module.
   *
   */

  inline void occupancyPDFName(const VxdID& sensor, int side, std::string& PDFName)
  {

    int layer = sensor.getLayerNumber();
    int ladder = sensor.getLadderNumber();
    int sens = sensor.getSensorNumber();


    PDFName =  std::to_string(layer) + "." + std::to_string(ladder) + "." + std::to_string(sens) + "."  + std::to_string(side);


  }
} //Belle2 namespace
