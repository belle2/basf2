/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef HELIXCOVARIANCE_H
#define HELIXCOVARIANCE_H

#include <cmath>
#include <iostream>
#include "TMatrixDSym.h"
#include "TMatrixD.h"

#include <framework/logging/Logger.h>
#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include "PerigeeCovariance.h"
#include "SZCovariance.h"

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Class representing the covariance matrix of a Helix in perigee parameters.
    class HelixCovariance : public UsedTObject {

    public:
      /// Default constructor for ROOT compatibility.
      HelixCovariance() :
        m_matrix(5)
      { m_matrix.Zero(); }


      /// Merges the covariance matrix of perigee and sz parameters
      HelixCovariance(const PerigeeCovariance& perigeeCovariance,
                      const SZCovariance& szCovariance) :
        m_matrix(5) {
        m_matrix.Zero();
        m_matrix.SetSub(perigeeCovariance.matrix().GetRowLwb(), perigeeCovariance.matrix());
        m_matrix.SetSub(szCovariance.matrix().GetRowLwb(), szCovariance.matrix());
      }



      /// Setup the covariance with the given covariance matrx
      explicit HelixCovariance(const TMatrixDSym& covarianceMatrix) :
        m_matrix(covarianceMatrix)
      { checkMatrix(); }



      /// Empty destructor
      ~HelixCovariance() {;}



      /// Down cast operator to symmetric matrix
      operator const TMatrixDSym& () const
      { return m_matrix; }



    private:
      /// Checks the covariance matrix for consistence
      inline void checkMatrix() const {
        if (matrix().GetNrows() != 5 or matrix().GetNcols() != 5) {
          B2ERROR("Helix covariance matrix is a  " <<
                  matrix().GetNrows() << "x" <<
                  matrix().GetNcols() << " matrix. " <<
                  "Expected 5x5 matrix."
                 );
        }
      }



    public:
      /// Setter for the whole covariance matrix of the helix parameters
      inline void setMatrix(const TMatrixDSym& covarianceMatrix)
      { m_matrix = covarianceMatrix; }



      /// Getter for the whole covariance matrix of the helix parameters
      const TMatrixDSym& matrix() const
      { return m_matrix; }

      /// Non constant access to the matrix elements return a reference to the underlying matrix entry.
      double& operator()(const HelixParameterIndex& iRow, const HelixParameterIndex& iCol)
      { return m_matrix(iRow, iCol); }

      /// Constant access to the matrix elements.
      double operator()(const HelixParameterIndex& iRow, const HelixParameterIndex& iCol) const
      { return m_matrix(iRow, iCol); }



      /// Getter for the perigee subcovariance
      PerigeeCovariance perigeeCovariance() const {
        //Note upper bound is inclusive (not exclusive like in e.g. Python)
        return matrix().GetSub(iCurv, iI, iCurv, iI);
      }

      /// Getter for the sz subcovariance
      SZCovariance szCovariance() const {
        //Note upper bound is inclusive (not exclusive like in e.g. Python)
        return matrix().GetSub(iSZ, iZ0, iSZ, iZ0);
      }


      /// Modifies to helix covariance matrix inplace to represent the reverse travel direction.
      void reverse();



      /// Returns the helix covariance for the reversed travel direction as a copy.
      HelixCovariance reversed() const {
        HelixCovariance result(*this);
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
        if (jacobian.GetNrows() != 5 or jacobian.GetNcols() != 5) {
          B2ERROR("Cannot transform HelixCovariance with a " <<
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
      /// Memory for the 5x5 matrix presentation of the covariance.
      TMatrixDSym m_matrix;

      /// ROOT Macro to make HelixCovariance a ROOT class.
      ClassDefInCDCLocalTracking(HelixCovariance, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // HELIXCOVARIANCE_H
