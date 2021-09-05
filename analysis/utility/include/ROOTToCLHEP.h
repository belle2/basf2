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
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMatrixFSym.h>
#include <TMatrixDSym.h>

namespace Belle2 {

  namespace ROOTToCLHEP {

    /**
     * Convert PxPyPzEVector to CLHEP::HepLorentzVector.
     * @param[in] vector Vector.
     */
    inline CLHEP::HepLorentzVector
    getHepLorentzVector(const ROOT::Math::PxPyPzEVector& vector)
    {
      return CLHEP::HepLorentzVector(vector.X(), vector.Y(), vector.Z(),
                                     vector.T());
    }

    /**
     * Convert B2Vector3 to HepGeom::Point3D<double>.
     * @param[in] vector Vector.
     */
    inline HepGeom::Point3D<double> getPoint3DFromB2Vector(const B2Vector3D& vector)
    {
      return HepGeom::Point3D<double>(vector.X(), vector.Y(), vector.Z());
    }

    /**
     * Convert XYZVector to HepGeom::Point3D<double>.
     * @param[in] vector Vector.
     */
    inline HepGeom::Point3D<double> getPoint3D(const ROOT::Math::XYZVector& vector)
    {
      return HepGeom::Point3D<double>(vector.X(), vector.Y(), vector.Z());
    }

    /**
     * Convert TMatrixFSym to CLHEP::HepSymMatrix.
     * @param[in] matrix Matrix.
     */
    inline CLHEP::HepSymMatrix getHepSymMatrix(const TMatrixFSym& matrix)
    {
      int n = matrix.GetNrows();
      CLHEP::HepSymMatrix m(n);
      /*
       * CLHEP::HepSymMatrix is stored as a lower triangular matrix,
       * thus it is sufficient to set only the corresponding elements.
       */
      for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j)
          m[i][j] = matrix[i][j];
      }
      return m;
    }

    /**
     * Convert TMatrixDSym to CLHEP::HepSymMatrix.
     * @param[in] matrix Matrix.
     */
    inline CLHEP::HepSymMatrix getHepSymMatrix(const TMatrixDSym& matrix)
    {
      int n = matrix.GetNrows();
      CLHEP::HepSymMatrix m(n);
      /*
       * CLHEP::HepSymMatrix is stored as a lower triangular matrix,
       * thus it is sufficient to set only the corresponding elements.
       */
      for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j)
          m[i][j] = matrix[i][j];
      }
      return m;
    }

  }

}
