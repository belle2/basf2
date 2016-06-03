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

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Class representing the covariance matrix of a Line in perigee parameters.
    class LineCovariance  {

    public:
      /// Default constructor for ROOT compatibility.
      LineCovariance() :
        m_matrix(std::min(ELineParameter::c_Phi0, ELineParameter::c_I),
                 std::max(ELineParameter::c_Phi0, ELineParameter::c_I))
      { m_matrix.Zero(); }

      /// Setup the covariance with the given covariance matrx
      explicit LineCovariance(const TMatrixDSym& covarianceMatrix) :
        m_matrix(covarianceMatrix)
      { checkMatrix(); }


      /// Down cast operator to symmetric matrix
      operator const TMatrixDSym& () const
      { return m_matrix; }

    private:
      /// Checks the covariance matrix for consistence
      inline void checkMatrix() const
      {
        using namespace NLineParameter;
        if (matrix().GetNrows() != 2 or
            matrix().GetNcols() != 2 or
            matrix().GetColLwb() != c_Phi0 or
            matrix().GetColUpb() != c_I) {
          B2ERROR("Line covariance matrix is a  " <<
                  matrix().GetNrows() << "x" <<
                  matrix().GetNcols() << " matrix starting from " <<
                  matrix().GetColLwb() << ". " <<
                  "Expected 2x2 matrix starting from "  << c_Phi0 << "."
                 );
        }
      }

    public:
      /// Setter for the whole covariance matrix of the line parameters
      inline void setMatrix(const TMatrixDSym& covarianceMatrix)
      { m_matrix = covarianceMatrix; checkMatrix(); }

      /// Getter for the whole covariance matrix of the line parameters
      const TMatrixDSym& matrix() const
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

      /// Sets the covariance matrix to zero.
      void invalidate()
      { m_matrix.Zero(); }

      /// Sets the covariance matrix to a unit matrix.
      void setUnit()
      { m_matrix.UnitMatrix(); }

      /// Transforms the covariance by the given jacobian matrix in place.
      void similarityTransform(const TMatrixD& jacobian)
      {
        if (jacobian.GetNrows() != 2 or jacobian.GetNcols() != 2) {
          B2ERROR("Cannot transform LineCovariance with a " <<
                  jacobian.GetNrows() << "x"  <<
                  jacobian.GetNcols() << " inplace.");
          return;
        }
        m_matrix.Similarity(jacobian);
      }

      /// Transforms a copy the covariance by the given jacobian matrix.
      TMatrixDSym similarityTransformed(const TMatrixD& jacobian) const
      {
        TMatrixDSym matrix = m_matrix;
        matrix.Similarity(jacobian);
        return matrix;
      }

    private:
      /// Memory for the 2x2 matrix presentation of the covariance.
      TMatrixDSym m_matrix;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
