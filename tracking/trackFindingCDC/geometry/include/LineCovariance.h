/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cmath>
#include <iostream>
#include <TMatrixDSym.h>
#include <TMatrixD.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/geometry/ELineParameter.h>

#include <Math/SMatrix.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Class representing the covariance matrix of a Line in perigee parameters.
    class LineCovariance  {

    public:
      /// The matrix type implementing the repesentation of the covariance
      using Matrix = ROOT::Math::SMatrix< double, 2, 2, ROOT::Math::MatRepSym< double, 2> >;

      /// The matrix type implementing the repesentation of a jacobian matrix used in the transport operations
      using JacobianMatrix = ROOT::Math::SMatrix< double, 2, 2>;

    public:
      /// Default constructor for ROOT compatibility.
      LineCovariance() = default;

      /// Down cast operator to symmetric matrix
      operator const Matrix& () const
      { return m_matrix; }

    public:
      /// Setter for the whole covariance matrix of the line parameters
      void setMatrix(const Matrix& matrix)
      { m_matrix = matrix; }

      /// Getter for the whole covariance matrix of the line parameters
      const Matrix& matrix() const
      { return m_matrix; }

      /// Non constant access to the matrix elements return a reference to the underlying matrix entry.
      double& operator()(const ELineParameter& iRow, const ELineParameter& iCol)
      { return m_matrix(iRow, iCol); }

      /// Constant access to the matrix elements.
      double operator()(const ELineParameter& iRow, const ELineParameter& iCol) const
      { return m_matrix(iRow, iCol); }

      /// Modifies to line covariance matrix inplace to represent the reverse travel direction.
      void reverse();

      /// Returns the line covariance for the reversed travel direction as a copy.
      LineCovariance reversed() const
      {
        LineCovariance result(*this);
        result.reverse();
        return result;
      }

      /// Invalidate the covariance matrix and reset it to a none informative martix
      void invalidate()
      { m_matrix = ROOT::Math::SMatrixIdentity(); }

      /// Sets the covariance matrix to a unit matrix.
      void setUnit()
      { m_matrix = ROOT::Math::SMatrixIdentity(); }

      /// Transforms the covariance by the given jacobian matrix in place.
      void similarityTransform(const JacobianMatrix& jacobian)
      {
        ROOT::Math::Similarity(jacobian, m_matrix);
      }

      /// Transforms a copy the covariance by the given jacobian matrix.
      Matrix similarityTransformed(const JacobianMatrix& jacobian) const
      {
        Matrix matrix = m_matrix;
        ROOT::Math::Similarity(jacobian, matrix);
        return matrix;
      }

    private:
      /// Memory for the 2x2 matrix presentation of the covariance.
      Matrix m_matrix;

    }; // class

  } // namespace TrackFindingCDC
} // namespace Belle2
