/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef SZCOVARIANCE_H
#define SZCOVARIANCE_H

#include <cmath>
#include <iostream>
#include "TMatrixDSym.h"
#include "TMatrixD.h"

#include <framework/logging/Logger.h>
#include <tracking/cdcLocalTracking/rootification/SwitchableRootificationBase.h>

#include "HelixParameterIndex.h"

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Class representing the covariance matrix of a set of perigee parameters.
    class SZCovariance : public SwitchableRootificationBase {

    public:
      /// Default constructor for ROOT compatibility.
      SZCovariance() :
        m_matrix(iSZ, iZ0) //From to indices
      { m_matrix.Zero(); }



      /// Setup the covariance with the given covariance matrx
      SZCovariance(const TMatrixDSym& covarianceMatrix) :
        m_matrix(covarianceMatrix)
      { checkMatrix(); }



      /// Empty destructor
      ~SZCovariance() {;}

      /// Down cast operator to symmetric matrix
      operator const TMatrixDSym& () const
      { return m_matrix; }


    private:
      /// Checks the covariance matrix for consistence
      inline void checkMatrix() const {
        if (matrix().GetNrows() != 2 or
            matrix().GetNcols() != 2 or
            matrix().GetColLwb() != iSZ or
            matrix().GetColUpb() != iZ0) {
          B2ERROR("SZ covariance matrix is a  " <<
                  matrix().GetNrows() << "x" <<
                  matrix().GetNcols() << " matrix starting from " <<
                  matrix().GetColLwb() << ". " <<
                  "Expected 2x2 matrix starting from "  << iSZ << "."
                 );
        }
      }



    public:
      /// Setter for the whole covariance matrix of the sz parameters
      inline void setMatrix(const TMatrixDSym& covarianceMatrix) {
        checkMatrix();
        m_matrix = covarianceMatrix;
      }



      /// Getter for the whole covariance matrix of the sz parameters
      const TMatrixDSym& matrix() const
      { return m_matrix; }

      /// Non constant access to the matrix elements return a reference to the underlying matrix entry.
      double& operator()(const HelixParameterIndex& iRow, const HelixParameterIndex& iCol)
      { return m_matrix(iRow, iCol); }

      /// Constant access to the matrix elements.
      double operator()(const HelixParameterIndex& iRow, const HelixParameterIndex& iCol) const
      { return m_matrix(iRow, iCol); }



      /// Modifies to sz covariance matrix inplace to represent the reverse travel direction.
      void reverse();

      /// Returns the sz covariance for the reversed travel direction as a copy.
      SZCovariance reversed() const {
        SZCovariance result(*this);
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
        if (jacobian.GetNrows() != 2 or jacobian.GetNcols() != 2) {
          B2ERROR("Cannot transform SZCovariance with a " <<
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
      /// Memory for the 2x2 matrix presentation of the covariance.
      TMatrixDSym m_matrix;

      /// ROOT Macro to make SZCovariance a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(SZCovariance, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // SZCOVARIANCE_H
