/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/reconstruction/PXDClusterShape.h>
#include <pxd/geometry/SensorInfo.h>

using namespace std;

namespace Belle2 {

  namespace PXD {

    pxdClusterShapeDescr PXDClusterShape::pxdClusterShapeDescription = {
      {pxdClusterShapeType::no_shape_set, "pxd cluster shape: no shape setting"},

      {pxdClusterShapeType::shape_1, "pxd cluster shape: cluster 1x1 (single) pixel"},
      {pxdClusterShapeType::shape_2_u, "pxd cluster shape: cluster 2x1 (u,v) pixels"},
      {pxdClusterShapeType::shape_2_v, "pxd cluster shape: cluster 1x2 (u,v) pixels"},
      {pxdClusterShapeType::shape_2_uv_diag, "pxd cluster shape: cluster 2x2 diagonal (u,v) pixels"},
      {pxdClusterShapeType::shape_2_uv_antidiag, "pxd cluster shape: cluster 2x2 anti-diagonal (u,v) pixels"},
      {pxdClusterShapeType::shape_N1, "pxd cluster shape: cluster Nx1 (u,v) pixels, N > 2"},
      {pxdClusterShapeType::shape_1M, "pxd cluster shape: cluster 1xM (u,v) pixels, M > 2"},
      {pxdClusterShapeType::shape_N2, "pxd cluster shape: cluster Nx2 (u,v) pixels, N > 2"},
      {pxdClusterShapeType::shape_2M, "pxd cluster shape: cluster 2xM (u,v) pixels, M > 2"},
      {pxdClusterShapeType::shape_4, "pxd cluster shape: cluster 2x2 (u,v) four pixels"},
      {pxdClusterShapeType::shape_3_L, "pxd cluster shape: cluster 2x2 (u,v) three pixels: (L)"},
      {pxdClusterShapeType::shape_3_L_mirr_u, "pxd cluster shape: cluster 2x2 (u,v) three pixels: (mirror_u_L)"},
      {pxdClusterShapeType::shape_3_L_mirr_v, "pxd cluster shape: cluster 2x2 (u,v) three pixels: (mirror_v_L)"},
      {pxdClusterShapeType::shape_3_L_mirr_uv, "pxd cluster shape: cluster 2x2 (u,v) three pixels: (mirror_u+v_L)"},
      {pxdClusterShapeType::shape_large, "pxd cluster shape: larger cluster over 2 pixels in u and v"}

    };

    PXDClusterShape::~PXDClusterShape() {}


    /** Set cluster shape ID  */
    pxdClusterShapeType PXDClusterShape::setClsShape(const ClusterCandidate& cls, VxdID sensorID)
    {
      const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(
                                                                 sensorID));
      pxdClusterShapeType clsShape;
      clsShape = pxdClusterShapeType::no_shape_set;
      ClusterProjection projU, projV;
      float fCategU = 0.0;
      float fCategV = 0.0;
      float fDiagU = 0.0;
      float fDiagV = 0.0;
      int CSFull = 0;
      for (const PXD::Pixel& px : cls.pixels()) {
        if (CSFull == 0) {
          fDiagU = px.getU();
          fDiagV = px.getV();
        }
        if (CSFull < 3) {
          fCategU += px.getU();
          fCategV += px.getV();
        }
        CSFull++;
        projU.add(px.getU(), info.getUCellPosition(px.getU()), px.getCharge());
        projV.add(px.getV(), info.getVCellPosition(px.getV()), px.getCharge());


      }
      projU.finalize();
      projV.finalize();

      if (CSFull == 1) {
        clsShape = pxdClusterShapeType::shape_1;                  // 1x1 pixels
      } else if (CSFull == 2) {
        if ((projU.getSize() == 2) && (projV.getSize() == 1)) {
          clsShape = pxdClusterShapeType::shape_2_u;              // 2x1 (u,v) pixels
        } else if ((projU.getSize() == 1) && (projV.getSize() == 2)) {
          clsShape = pxdClusterShapeType::shape_2_v;              // 1x2 (u,v) pixels
        } else if ((fDiagU < (fCategU / 2.0)) && (fDiagV < (fCategV / 2.0))) {
          clsShape = pxdClusterShapeType::shape_2_uv_diag;        // 2x2 diagonal (u,v) pixels
        } else {
          clsShape = pxdClusterShapeType::shape_2_uv_antidiag;    // 2x2 anti-diagonal (u,v) pixels
        }
      } else if (CSFull == 3) {
        if ((projU.getSize() == 2) && (projV.getSize() == 2)) {
          int MisU;                  // missing pixel in "L" cluster - u marker
          int MisV;                  // missing pixel in "L" cluster - v marker
          int MissingU;              // missing pixel in "L" cluster - u
          int MissingV;              // missing pixel in "L" cluster - v
          MisU = 0;
          MisV = 0;
          fCategU /= 3.0;
          fCategV /= 3.0;
          MissingU = (int)fCategU;
          if ((fCategU - (int)fCategU) < 0.5) {
            MissingU++;
            MisU = 1;
          }
          MissingV = (int)fCategV;
          if ((fCategV - (int)fCategV) < 0.5) {
            MissingV++;
            MisV = 1;
          }
          if ((MisU == 1) && (MisV == 1)) {
            clsShape = pxdClusterShapeType::shape_3_L;            // 2x2 (u,v) three pixels: o* (L)
            //                         oo
          } else if ((MisU == 0) && (MisV == 1)) {
            clsShape = pxdClusterShapeType::shape_3_L_mirr_u;     // 2x2 (u,v) three pixels: *o (mirror_u_L)
            //                         oo
          } else if ((MisU == 1) && (MisV == 0)) {
            clsShape = pxdClusterShapeType::shape_3_L_mirr_v;     // 2x2 (u,v) three pixels: oo (mirror_v_L)
            //                         o*
          } else if ((MisU == 0) && (MisV == 0)) {
            clsShape = pxdClusterShapeType::shape_3_L_mirr_uv;    // 2x2 (u,v) three pixels: oo (mirror_u+v_L)
            //                         *o
          }
        } else if (projV.getSize() == 1) {
          clsShape = pxdClusterShapeType::shape_N1;               // Nx1 (u,v) pixels, N > 2
        } else if (projU.getSize() == 1) {
          clsShape = pxdClusterShapeType::shape_1M;               // 1xM (u,v) pixels, M > 2
        } else if (projV.getSize() == 2) {
          clsShape = pxdClusterShapeType::shape_N2;               // Nx2 (u,v) pixels, N > 2
        } else if (projU.getSize() == 2) {
          clsShape = pxdClusterShapeType::shape_2M;               // 2xM (u,v) pixels, M > 2
        } else {
          clsShape = pxdClusterShapeType::shape_large;            // big cluster over 2x2 pixels
        }
      } else if (CSFull == 4) {
        if ((projU.getSize() == 2) && (projV.getSize() == 2)) {
          clsShape = pxdClusterShapeType::shape_4;                // 2x2 (u,v) four pixels
        } else if (projV.getSize() == 1) {
          clsShape = pxdClusterShapeType::shape_N1;               // Nx1 (u,v) pixels, N > 2
        } else if (projU.getSize() == 1) {
          clsShape = pxdClusterShapeType::shape_1M;               // 1xM (u,v) pixels, M > 2
        } else if (projV.getSize() == 2) {
          clsShape = pxdClusterShapeType::shape_N2;               // Nx2 (u,v) pixels, N > 2
        } else if (projU.getSize() == 2) {
          clsShape = pxdClusterShapeType::shape_2M;               // 2xM (u,v) pixels, M > 2
        } else {
          clsShape = pxdClusterShapeType::shape_large;            // big cluster over 2x2 pixels
        }
      } else if (CSFull >= 5) {
        if (projV.getSize() == 1) {
          clsShape = pxdClusterShapeType::shape_N1;          // Nx1 (u,v) pixels, N > 2
        } else if (projU.getSize() == 1) {
          clsShape = pxdClusterShapeType::shape_1M;               // 1xM (u,v) pixels, M > 2
        } else if (projV.getSize() == 2) {
          clsShape = pxdClusterShapeType::shape_N2;               // Nx2 (u,v) pixels, N > 2
        } else if (projU.getSize() == 2) {
          clsShape = pxdClusterShapeType::shape_2M;               // 2xM (u,v) pixels, M > 2
        } else {
          clsShape = pxdClusterShapeType::shape_large;       // big cluster over 2x2 pixels
        }
      }

      return clsShape;
    } //setClsShape

  } //PXD namespace
} //Belle2 namespace
