/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/geometry/B2Vector3.h>

/* External headers. */
#include <TMatrixDSym.h>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <TRotation.h>
#include <Math/Vector3D.h>

namespace Belle2 {

  namespace RotationTools {

    /**
     * Convert TRotation to TMatrixD.
     * @param[in] r TRotation object.
     */
    inline TMatrixD toMatrix(TRotation r)
    {
      TMatrixD rM(3, 3);
      for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
          rM(i, j) = r(i, j);
      return rM;
    }


    /**
     * Construct rotation matrix describing
     * rotation around Y-axis by angleY and then around Z-axis by angleZ
     * @param[in] angleY angle in radians
     * @param[in] angleZ angle in radians
     */
    inline TMatrixD getRotationMatrixYZ(double angleY, double angleZ)
    {
      TRotation r;
      r.RotateY(angleY);
      r.RotateZ(angleZ);
      return toMatrix(r);
    }


    /**
     * Construct rotation matrix describing
     * rotation around X-axis by angleX and then around Y-axis by angleY
     * @param[in] angleX angle in radians
     * @param[in] angleY angle in radians
     */
    inline TMatrixD getRotationMatrixXY(double angleX, double angleY)
    {
      TRotation r;
      r.RotateX(angleX);
      r.RotateY(angleY);
      return toMatrix(r);
    }



    /**
     * Rotate tensor orgMat, where the rotation is described by a transformation
     * which transform vector (0,0,1) -> vTo
     * @param[in] vTo  vector defining rotation
     * @param[in] orgMat tensor before rotation
     */
    inline TMatrixD rotateTensor(const B2Vector3D& vTo, const TMatrixD& orgMat)
    {
      TMatrixD r = getRotationMatrixYZ(vTo.Theta(), vTo.Phi());
      TMatrixD rT = r; rT.T();
      return r * orgMat * rT;
    }

    /**
     * Rotate tensor orgMat, where the rotation is described by a transformation
     * which transform vector vTo -> (0,0,1)
     * @param[in] vTo  vector defining rotation
     * @param[in] orgMat tensor before rotation
     */
    inline TMatrixD rotateTensorInv(const ROOT::Math::XYZVector& vTo, const TMatrixD& orgMat)
    {
      TMatrixD r = getRotationMatrixYZ(vTo.Theta(), vTo.Phi());
      TMatrixD rT = r; rT.T();
      return rT * orgMat * r;
    }


    /**
     * Convert squared matrix to symmetric matrix
     * as S = (A + At) / 2
     * the "asymmetric component" is not calculated
     * @param[in] m Original matrix
     */
    inline TMatrixDSym toSymMatrix(const TMatrixD& m)
    {
      TMatrixDSym mS(m.GetNrows());
      for (int i = 0; i < m.GetNrows(); ++i)
        for (int j = 0; j < m.GetNcols(); ++j) {
          mS(i, j) = (m(i, j) + m(j, i)) / 2;
        }
      return mS;
    }


    /**
     * Convert B2Vector3D to TVectorD
     *
     */
    inline TVectorD toVec(B2Vector3D v)
    {
      return TVectorD(0, 2, v.X(), v.Y(), v.Z(), "END");
    }

    /**
     * Get a vector orthogonal to v of the unit length
     *
     */
    B2Vector3D getUnitOrthogonal(B2Vector3D v)
    {
      return B2Vector3D(v.Z(), 0, -v.X()).Unit();
    }

  }

}
