/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <pxd/reconstruction/ClusterCandidate.h>

namespace Belle2 {
  namespace PXD {

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
      shape_large

    };

    /** Type specifies cluster shape type description */
    typedef std::map<pxdClusterShapeType, std::string> pxdClusterShapeDescr;

    /** Class to correct estimation of cluster error and position base on its shape.
     * This class will set a shape ID of cluster in clusterization time.
     * ID is describe on list "pxdClusterShapeType".
     * Function "pxdClusterShapeDescription" give more detail desription of shape.
     *
     * Function "setClsShape" set shape ID for cluster.
     * Correction is apply for RecoHit if available direction of reco track
     *
     * @verbatim
     *   u → → → → → → → → → →
     * v┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐v
     * ↑│-│-│-│-│-│-│-│-│-│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑
     * ↑│-│-│-│x│-│-│-│-│-│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑"L"
     * ↑│-│-│-│x│x│-│-│-│x│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑"mirror_u_L"
     * ↑│-│-│-│-│-│-│-│x│x│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑
     * ↑│-│-│-│x│x│-│-│-│-│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑"mirror_v_L"
     * ↑│-│-│-│x│-│-│-│x│x│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑"mirror_u+v_L"
     * ↑│-│-│-│-│-│-│-│-│x│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑
     * ↑│-│-│-│-│x│-│-│-│-│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑"2x2 diagonal"
     * ↑│-│-│-│x│-│-│-│x│-│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑"2x2 anti-diagonal"
     * ↑│-│-│-│-│-│-│-│-│x│-│-│↑
     * ↑├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤↑
     * ↑│-│-│-│-│-│-│-│-│-│-│-│↑
     * v└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘v
     * - u → → → → → → → → → →
     * @endverbatim
     *
     */
    class PXDClusterShape {
    public:

      /** Delete the cache and free the memory */
      ~PXDClusterShape();

      /** Create detail description of cluster shape */
      static pxdClusterShapeDescr pxdClusterShapeDescription;

      /** Set cluster shape ID.
       * @param cls ClusterCandidate container of pixels creating cluster in PXD
       * @param sensorID sensor details for PXD sensors
       * @returns cluster shape ID
       */
      pxdClusterShapeType setClsShape(const ClusterCandidate& cls, VxdID sensorID);

    private:

    };

  }

}
