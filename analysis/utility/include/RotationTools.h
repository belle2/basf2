/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TMatrixDSym.h>
#include <TMatrixD.h>
#include <TRotation.h>

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
     * Convert squared matrix to symmetric matric
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

  }

}
