/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCLUSTERSHAPE_H
#define PXDCLUSTERSHAPE_H

#include <cstring>
#include <deque>
#include <pxd/reconstruction/Pixel.h>
#include <pxd/reconstruction/ClusterCandidate.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/reconstruction/ClusterProjection.h>

namespace Belle2 {
  namespace PXD {

    /** Class to correct estimation of cluster error and position base on its shape
     * This class will set a shape ID of cluster in clusterization time.
     * ID is describe on list "pxdClusterShapeType"
     * Function "pxdClusterShapeDescription" give more detail desription of shape.
     *
     * Function "setClsShape" set shape ID for cluster
     * Function "ClsShapeCorrection" correct estimation of eror and reco position
     * base on type of shape, if is available correcting method
     * Correction is apply in advance also if available method using
     *  - direction of reco track
     *  - signal of cluster
     *  - seed of cluster
     * Function "CalibrationClsShapeCorrection" calibrate corrections for
     * estimation of error and position of cluster
     *
     * @verbatim
     *   u → → → → → → → → → →
     * v┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
     * ↑│ │ │ │ │ │ │ │ │ │ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↑│ │ │ │X│ │ │ │ │ │ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤ "L"
     * ↑│ │ │ │x│x│ │ │ │x│ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤ "mirror_u_L"
     * ↑│ │ │ │ │ │ │ │x│x│ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↑│ │ │ │x│x│ │ │ │ │ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤ "mirror_v_L"
     * ↑│ │ │ │x│ │ │ │x│x│ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤ "mirror_u+v_L"
     * ↑│ │ │ │ │ │ │ │ │x│ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * ↑│ │ │ │ │x│ │ │ │ │ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤ "2x2 diagonal"
     * ↑│ │ │ │x│ │ │ │x│ │ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤ "2x2 anti-diagonal"
     * ↑│ │ │ │ │ │ │ │ │x│ │ │↑
     *  ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤
     * v│ │ │ │ │ │ │ │ │ │ │ │v
     *  └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
     *   u → → → → → → → → → →
     * @endverbatim
     *
     */

    /** Type specifies cluster shape type */
    enum class pxdClusterShapeType {
      no_shape_set = 0,

      shape_1,
      shape_2_u,
      shape_2_v,
      shape_2_uv_diag,
      shape_2_uv_antidiag,
      shape_N1,
      shape_1M,
      shape_N2,
      shape_2M,
      shape_4,
      shape_3_L,
      shape_3_L_mirr_u,
      shape_3_L_mirr_v,
      shape_3_L_mirr_uv,
      shape_large,

      shape_corr_1 = -100,
      shape_corr_2_u,
      shape_corr_2_v,
      shape_corr_2_uv_diag,
      shape_corr_2_uv_antidiag,
      shape_corr_N1,
      shape_corr_1M,
      shape_corr_N2,
      shape_corr_2M,
      shape_corr_4,
      shape_corr_3_L,
      shape_corr_3_L_mirr_u,
      shape_corr_3_L_mirr_v,
      shape_corr_3_L_mirr_uv,
      shape_corr_large
    };

    typedef std::map<pxdClusterShapeType, std::string> pxdClusterShapeDescr;

    class PXDClusterShape {
    public:

      /** Delete the cache and free the memory */
      ~PXDClusterShape();

      /** Create detail description of cluster shape */
      static pxdClusterShapeDescr pxdClusterShapeDescription;

      /** Set cluster shape ID.
       * @param ClusterCandidate container of pixels creating cluster in PXD
       * @param sensorID sensor details for PXD sensors
       * @returns cluster shape ID
       */
      pxdClusterShapeType setClsShape(const ClusterCandidate& cls, VxdID sensorID);

    private:

    };

  }

}

#endif //PXDCLUSTERSHAPE_H
