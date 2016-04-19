/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef pxdCheckClusterShapeCorrectionModule_H
#define pxdCheckClusterShapeCorrectionModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/ClusterCache.h>
#include <pxd/reconstruction/ClusterProjection.h>
#include <pxd/reconstruction/NoiseMap.h>
#include <string>
#include <memory>
#include <iostream>
#include <stdio.h>
#include <TMath.h>

#include <calibration/CalibrationCollectorModule.h>
#include <pxd/calibration/PXDClusterShapeCalibrationAlgorithm.h>
//#include <boost/multi_array.hpp>
//#include <boost/functional/hash.hpp>

namespace genfit {
  class Track;
}

namespace Belle2 {

  namespace PXD {

    /** The pxdCheckClusterShapeCorrection module.
     *
     * This module tests whether TrueHits are accessible by following the clusters-to-truehits
     * relation.

     */
    class pxdCheckClusterShapeCorrectionModule : public Module {

    public:
      /** Constructor defining the parameters */
      pxdCheckClusterShapeCorrectionModule();
      /** Initialize the module */
      virtual void initialize();

    private:
      //std::string m_storeTrackName;           /**< genfit::Track StoreArray name */

      /** Exist file with basic corrections, default=0 */
      int m_ExistCorrectionBasic = 0;
      /** Exist file with corrections for PK=0, default=0 */
      int m_ExistCorrectionPK0 = 0;
      /** Exist file with corrections for PK=1, default=0 */
      int m_ExistCorrectionPK1 = 0;
      /** Exist file with corrections for PK=2, default=0 */
      int m_ExistCorrectionPK2 = 0;
      /** Exist file with corrections for PK=3, default=0 */
      int m_ExistCorrectionPK3 = 0;

      /** Map of corrections for Bias for 6 cases: 0: realistic data, 1..4: simulation for ful angle range for all pixel types, 5: combine 1..4 */
      map_typeCorrs TCorrection_BiasMap[6];
      /** Map of corrections for Error Estimation for 6 cases: 0: realistic data, 1..4: simulation for ful angle range for all pixel types, 5: combine 1..4 */
      map_typeCorrs TCorrection_ErrorEstimationMap[6];
      /** Map of corrections for Bias errors for 6 cases: 0: realistic data, 1..4: simulation for ful angle range for all pixel types, 5: combine 1..4 */
      map_typeCorrs TCorrection_BiasMapErr[6];
      /** Map of corrections for In-Pixel positions for 6 cases: 0: realistic data, 1..4: simulation for ful angle range for all pixel types, 5: combine 1..4 */
      map_typeInPics TInPixelPositionMap[6];

      /** To use simulations rather than real data calculated corrections, default=False */
      Bool_t m_PrefereSimulation = kFALSE;

      /** Name of file contain basic calibration, default="pxdCalibrationBasic */
      std::string m_CalFileBasicName = "pxdCalibrationBasic";
      /** Name of file contain calibration for pixel kind 0 (PitchV=55um), default="pxdCalibrationPixelKind0 */
      std::string m_CalFilePK0Name = "pxdCalibrationPixelKind0";
      /** Name of file contain calibration for pixel kind 1 (PitchV=60um), default="pxdCalibrationPixelKind1 */
      std::string m_CalFilePK1Name = "pxdCalibrationPixelKind1";
      /** Name of file contain calibration for pixel kind 2 (PitchV=70um), default="pxdCalibrationPixelKind2 */
      std::string m_CalFilePK2Name = "pxdCalibrationPixelKind2";
      /** Name of file contain calibration for pixel kind 3 (PitchV=85um), default="pxdCalibrationPixelKind3 */
      std::string m_CalFilePK3Name = "pxdCalibrationPixelKind3";

      /** Presets for usinf of pxd cluster shape calibration - shapes */
      int m_shapes = 0;     // 1 .. 15, shapeID = 0: not setting shape
      /** Presets for usinf of pxd cluster shape calibration - pixelkinds */
      int m_pixelkinds = 0;
      /** Presets for usinf of pxd cluster shape calibration - dimensions */
      int m_dimensions = 0; // cases: 15 x 8 x 2 = 240
      /** Presets for usinf of pxd cluster shape calibration - anglesU */
      int m_anglesU = 0;    // 18 x 18 = 324 (angles)
      /** Presets for usinf of pxd cluster shape calibration - anglesV */
      int m_anglesV = 0;
      /** Presets for usinf of pxd cluster shape calibration - in_pixelU */
      int m_in_pixelU = 0;  // 7 x 7 = 49 (in-pixel positions)
      /** Presets for usinf of pxd cluster shape calibration - in_pixelV */
      int m_in_pixelV = 0;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // pxdCheckClusterShapeCorrectionModule_H
