/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PERIGEECOVARIANCE_H
#define PERIGEECOVARIANCE_H

#include <cmath>
#include <iostream>
#include "TMatrixDSym.h"
#include "TMatrixD.h"

#include <framework/logging/Logger.h>
#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>


namespace Belle2 {

  namespace CDCLocalTracking {

    /// Class representing the covariance matrix of a set of perigee parameters.
    class PerigeeCovariance : public UsedTObject {

    public:
      /// Default constructor for ROOT compatibility.
      PerigeeCovariance() :
        m_matrix(3)
      { m_matrix.Zero(); }



      /// Setup the covariance with the given covariance matrx
      PerigeeCovariance(const TMatrixDSym& covarianceMatrix) :
        m_matrix(covarianceMatrix)
      { checkMatrix(); }



      /// Empty destructor
      ~PerigeeCovariance() {;}

      /// Down cast operator to symmetric matrix
      operator TMatrixDSym() const
      { return m_matrix; }


    private:
      /// Checks the covariance matrix for consistence
      inline void checkMatrix() const {
        if (matrix().GetNrows() != 3 or matrix().GetNcols() != 3) {
          B2ERROR("Perigee covariance matrix is a  " <<
                  matrix().GetNrows() << "x" <<
                  matrix().GetNcols() << " matrix. " <<
                  "Expected 3x3 matrix."
                 );
        }
      }



    public:
      /// Setter for the whole covariance matrix of the helix parameters
      inline void setMatrix(const TMatrixDSym& covarianceMatrix) {
        checkMatrix();
        m_matrix = covarianceMatrix;
      }



      /// Getter for the whole covariance matrix of the helix parameters
      const TMatrixDSym& matrix() const
      { return m_matrix; }



      /// Modifies to helix covariance matrix inplace to represent the reverse travel direction.
      void reverse();


      /// Returns the helix covariance for the reversed travel direction as a copy.
      PerigeeCovariance reversed() const {
        PerigeeCovariance result(*this);
        result.reverse();
        return result;
      }



      /// Sets the covariance matrix to zero.
      void setNull()
      { m_matrix.Zero(); }



      /// Sets the covariance matrix to a unit matrix.
      void setUnit()
      { m_matrix.UnitMatrix(); }


      /// Transforms the covariance by the given jacobian matrix in place.
      void similarityTransform(const TMatrixD& jacobian) {
        if (jacobian.GetNrows() != 3 or jacobian.GetNcols() != 3) {
          B2ERROR("Cannot transform PerigeeCovariance with a " <<
                  jacobian.GetNrows() << "x"  <<
                  jacobian.GetNcols() << " inplace.");
          return;
        }
        m_matrix.Similarity(jacobian);
      }



      /// Transforms a copy the covariance by the given jacobian matrix.
      TMatrixDSym similarityTransformed(const TMatrixD& jacobian) const {
        TMatrixDSym matrix = m_matrix;
        matrix.Similarity(jacobian);
        return matrix;
      }



    private:
      /// Memory for the 3x3 matrix presentation of the covariance.
      TMatrixDSym m_matrix;

      /// ROOT Macro to make PerigeeCovariance a ROOT class.
      ClassDefInCDCLocalTracking(PerigeeCovariance, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // PERIGEECOVARIANCE_H
