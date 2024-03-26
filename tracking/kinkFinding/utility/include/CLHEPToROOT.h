/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMatrixFSym.h>

namespace Belle2 {

  namespace CLHEPToROOT {

    /**
     * Convert CLHEP::HepLorentzVector to PxPyPzEVector.
     * @param[in] vector Vector.
     */
    inline ROOT::Math::PxPyPzEVector
    getLorentzVector(const CLHEP::HepLorentzVector& vector)
    {
      return ROOT::Math::PxPyPzEVector(vector.x(), vector.y(), vector.z(), vector.t());
    }

    /**
     * Convert HepGeom::Point3D<double> to XYZVector.
     * @param[in] point Point.
     */
    inline ROOT::Math::XYZVector getXYZVector(const HepGeom::Point3D<double>& point)
    {
      return ROOT::Math::XYZVector(point.x(), point.y(), point.z());
    }

    /**
     * Convert CLHEP::HepSymMatrix to TMatrixFSym.
     * @param[in] matrix Matrix.
     */
    inline TMatrixFSym getTMatrixFSym(const CLHEP::HepSymMatrix& matrix)
    {
      int n = matrix.num_row();
      TMatrixFSym m(n);
      /*
       * TMatrixFSym is stored as a full matrix, thus all elements must be set.
       */
      for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
          m[i][j] = matrix[i][j];
      }
      return m;
    }

    /**
     * Convert CLHEP::HepSymMatrix to TMatrixFSym.
     * @param[in] matrix Matrix.
     */
    inline TMatrixDSym getTMatrixDSym(const CLHEP::HepSymMatrix& matrix)
    {
      int n = matrix.num_row();
      TMatrixDSym m(n);
      /*
       * TMatrixFSym is stored as a full matrix, thus all elements must be set.
       */
      for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
          m[i][j] = matrix[i][j];
      }
      return m;
    }

  }

}
