/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TMatrixDSym.h>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <Math/RotationX.h>
#include <Math/RotationY.h>
#include <Math/RotationZ.h>
#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>

namespace Belle2 {

  namespace RotationTools {

    /**
     * Convert TRotation to TMatrixD.
     * @param[in] r TRotation object.
     */
    inline TMatrixD toMatrix(ROOT::Math::Rotation3D r)
    {
      TMatrixD rM(3, 3);
      r.GetRotationMatrix(rM);
      return rM;
    }


    /**
     * Construct rotation matrix describing
     * rotation from zAxis to axis zPrime
     * @param[in] zPrime vector to which (0,0,1) vector is rotated
     */
    inline TMatrixD getRotationMatrixZtoZp(ROOT::Math::XYZVector zPrime)
    {
      ROOT::Math::XYZVector zAxis(0, 0, 1);
      double angle = ROOT::Math::VectorUtil::Angle(zAxis, zPrime);

      ROOT::Math::XYZVector rotAxis = zAxis.Cross(zPrime).Unit();
      double x = rotAxis.x();
      double y = rotAxis.y();

      double cosA = std::cos(angle);
      double sinA = std::sin(angle);
      double oneMinusCosA = 1 - cosA;

      double xx = cosA + x * x * oneMinusCosA;
      double xy = x * y * oneMinusCosA;
      double xz = y * sinA;

      double yx = y * x * oneMinusCosA;
      double yy = cosA + y * y * oneMinusCosA;
      double yz = - x * sinA;

      double zx = - y * sinA;
      double zy = x * sinA;
      double zz = cosA;

      return toMatrix(ROOT::Math::Rotation3D(xx, xy, xz, yx, yy, yz, zx, zy, zz));
    }


    /**
     * Construct rotation matrix describing
     * rotation around X-axis by angleX and then around Y-axis by angleY
     * @param[in] angleX angle in radians
     * @param[in] angleY angle in radians
     */
    inline TMatrixD getRotationMatrixXY(double angleX, double angleY)
    {
      ROOT::Math::RotationX rX(angleX);
      ROOT::Math::RotationY rY(angleY);
      ROOT::Math::Rotation3D r = rX * rY;
      return toMatrix(r);
    }



    /**
     * Rotate tensor orgMat, where the rotation is described by a transformation
     * which transform vector (0,0,1) -> vTo
     * @param[in] vTo  vector defining rotation
     * @param[in] orgMat tensor before rotation
     */
    inline TMatrixD rotateTensor(const ROOT::Math::XYZVector& vTo, const TMatrixD& orgMat)
    {
      TMatrixD r = getRotationMatrixZtoZp(vTo);
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
      TMatrixD r = getRotationMatrixZtoZp(vTo);
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
     * Convert XYZVector to TVectorD
     *
     */
    inline TVectorD toVec(ROOT::Math::XYZVector v)
    {
      return TVectorD(0, 2, v.X(), v.Y(), v.Z(), "END");
    }

    /**
     * Get a vector orthogonal to v of the unit length
     *
     */
    ROOT::Math::XYZVector getUnitOrthogonal(ROOT::Math::XYZVector v)
    {
      return ROOT::Math::XYZVector(v.Z(), 0, -v.X()).Unit();
    }

  }

}
