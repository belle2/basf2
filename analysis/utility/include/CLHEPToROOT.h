/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <TLorentzVector.h>
#include <TMatrixFSym.h>
#include <TVector3.h>

namespace Belle2 {

  namespace CLHEPToROOT {

    /**
     * Convert CLHEP::HepLorentzVector to TLorentzVector.
     * @param[in] vector Vector.
     */
    inline TLorentzVector
    getTLorentzVector(const CLHEP::HepLorentzVector& vector)
    {
      return TLorentzVector(vector.x(), vector.y(), vector.z(), vector.t());
    }

    /**
     * Convert HepGeom::Point3D<double> to TVector3.
     * @param[in] point Point.
     */
    inline TVector3 getTVector3(const HepGeom::Point3D<double>& point)
    {
      return TVector3(point.x(), point.y(), point.z());
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

  }

}
