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

#include <tracking/trackFindingCDC/geometry/LineParameters.h>
#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <cstddef>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A parameter line including including an line covariance matrix which is interpreted as
    /// located in the support point
    class UncertainParameterLine2D {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainParameterLine2D()
      {
      }

      /// Attaching a covariance matrix to a parameter line.
      explicit UncertainParameterLine2D(const ParameterLine2D& parameterLine2D,
                                        const LineCovariance& lineCovariance = LineUtil::identity(),
                                        const double chi2 = 0.0,
                                        const size_t ndf = 0)
        : m_parameterLine2D(parameterLine2D)
        , m_lineCovariance(lineCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /**
       *  Access to the constant interface of the underlying parameter line
       *  Allows the user of this "super" class to use the getters and
       *  other methods to get inforamation about the line but disallows mutation
       *  of internal state.
       *  This ensures that the parameters are not changed without proper adjustment to
       *  the covariance matrix in this class, which can be achieved by the more limited
       *  set of methods in this class accessable by normal . method calls
       */
      const ParameterLine2D* operator->() const
      {
        return &m_parameterLine2D;
      }

      /// Downcast to the "super" class
      operator ParameterLine2D() const
      {
        return m_parameterLine2D;
      }

    public:
      /// Getter for the three perigee parameters in the order defined by EPerigeeParameter.h
      LineParameters lineParameters() const
      {
        using namespace NLineParameterIndices;
        LineParameters result;
        result(c_Phi0) = m_parameterLine2D.tangential().phi();
        result(c_I) = 0;
        return result;
      }

      /// Setter for the whole covariance matrix of the line parameters
      void setLineCovariance(const LineCovariance& lineCovariance)
      {
        m_lineCovariance = lineCovariance;
      }

      /// Getter for the whole covariance matrix of the line parameters
      const LineCovariance& lineCovariance() const
      {
        return m_lineCovariance;
      }

      /// Getter for individual elements of the covariance matrix
      double covariance(const ELineParameter& iRow, const ELineParameter& iCol) const
      {
        return lineCovariance()(iRow, iCol);
      }

      /// Getter for individual diagonal elements of the covariance matrix
      double variance(const ELineParameter& i) const
      {
        return lineCovariance()(i, i);
      }

      /// Getter for the chi square value of the line fit
      double chi2() const
      {
        return m_chi2;
      }

      /// Setter for the chi square value of the line fit
      void setChi2(const double chi2)
      {
        m_chi2 = chi2;
      }

      /// Getter for the number of degrees of freediom used in the line fit
      std::size_t ndf() const
      {
        return m_ndf;
      }

      /// Setter for the number of degrees of freediom used in the line fit
      void setNDF(std::size_t ndf)
      {
        m_ndf = ndf;
      }

      /// Sets all line parameters to zero including the covariance matrix
      void invalidate()
      {
        m_parameterLine2D.invalidate();
        m_lineCovariance = LineUtil::identity();
        m_chi2 = 0.0;
        m_ndf = 0;
      }

    public:
      /// Reverses the direction of flight represented by this line
      void reverse()
      {
        m_parameterLine2D.reverse();
        LineUtil::reverse(m_lineCovariance);
      }

      /// Returns a copy of the line corresponding to the reverse direction of travel.
      UncertainParameterLine2D reversed() const
      {
        return UncertainParameterLine2D(m_parameterLine2D.reversed(),
                                        LineUtil::reversed(lineCovariance()),
                                        m_chi2,
                                        m_ndf);
      }

    public:
      /**
       *  Moves the coordinate system by the vector by.
       *  Because the covariance matrix is pinned in the support point
       *  it does not change in this move operation
       */
      void passiveMoveBy(const Vector2D& by)
      {
        m_parameterLine2D.passiveMoveBy(by);
      }

      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      LineJacobian moveSupportByJacobian(double byAt) const
      {
        using namespace NLineParameterIndices;
        LineJacobian result = LineUtil::identity();
        result(c_I, c_Phi0) = -byAt * m_parameterLine2D.tangential().norm();
        return result;
      }

      /// Moves the coordinate system by the vector by and calculates the new line and its
      /// covariance matrix. Change is inplace.
      void moveSupportBy(double byAt)
      {
        // Move the covariance matrix first to have access to the original parameters
        LineJacobian jacobian = moveSupportByJacobian(byAt);
        LineUtil::transport(jacobian, m_lineCovariance);
        m_parameterLine2D.passiveMoveAtBy(byAt);
      }

      /// Getter for the covariance as if he coordinate system was moved by the given vector.
      LineCovariance movedSupportCovarianceBy(double byAt) const
      {
        LineJacobian jacobian = moveSupportByJacobian(byAt);
        return LineUtil::transported(jacobian, lineCovariance());
      }

    private:
      /// Memory for the underlying parameter line
      ParameterLine2D m_parameterLine2D;

      /// Memory for the 2x2 covariance matrix of the line phi0 and impact parameter relative to the
      /// support point
      LineCovariance m_lineCovariance = LineUtil::identity();

      /// Memory for the chi square value of the fit of this line.
      double m_chi2 = 0.0;

      /// Memory for the number of degrees of freedim of the fit of this line.
      size_t m_ndf = 0;

    };

  }
}
