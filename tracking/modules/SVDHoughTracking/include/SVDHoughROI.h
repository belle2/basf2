/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDHoughROI_H
#define SVDHoughROI_H

#include <tracking/modules/SVDHoughTracking/SVDHoughTrackingModule.h>
#include <tracking/modules/SVDHoughTracking/SVDHoughClusterCand.h>
//#include <tracking/modules/svdHoughtracking/SVDHoughRootOutput.h>
#include <framework/core/Module.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/simulation/SVDSignal.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/Sample.h>
#include <pxd/geometry/SensorInfo.h>
#include <framework/dataobjects/RelationElement.h>
#include <boost/array.hpp>
#include <string>
#include <set>
#include <vector>
#include <fstream>

#include <root/TVector3.h>
#include <root/TVector2.h>
#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TH1D.h>
#include <root/TH2D.h>
#include <root/TF1.h>


namespace Belle2 {
  namespace SVD {
    /**
      * Hough ROI class.
      */
    class SVDHoughROI {
    public:
      /** Constructor for Hough ROI */
      //SVDHoughROI(VxdID _sensorID, TVector2 _v1, TVector2 _v2): sensorID(_sensorID), v1(_v1), v2(_v2)
      SVDHoughROI(Belle2::VxdID _sensorID, TVector2 _v1, TVector2 _v2): sensorID(_sensorID), v1(_v1), v2(_v2)
      {
      }

      /** Deconstructor */
      ~SVDHoughROI() {}

      /** Get sensor ID */
      //VxdID getSensorID() { return sensorID; }
      Belle2::VxdID getSensorID() { return sensorID; }
      /** Get v1 = lower left corner of ROI in (u,v) system */
      TVector2 getV1() { return v1; }
      /** Get v2 = upper right corner of ROI in (u,v) system */
      TVector2 getV2() { return v2; }
    private:
      /** Sensor ID of extrapolated hit sensor/pixel */
      //VxdID sensorID;
      Belle2::VxdID sensorID;
      /** v1 (lower left edge) */
      TVector2 v1;
      /** v2 (upper right edge) */
      TVector2 v2;
      /** Pixel IDs */
    };

  }
}

#endif