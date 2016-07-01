/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/Line2D.h>

#include <tracking/trackFindingCDC/geometry/SZParameters.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// A line in sz where s is the transverse travel distance as seen in the xy projection with uncertainties in the slope and intercept of the line.
    class UncertainSZLine : public Line2D {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainSZLine()
      {}

      /// Composes an uncertain sz line from the  perigee parameters and a 2x2 covariance matrix. Covariance matrix defaults to a zero matrix.
      UncertainSZLine(const double tanLambda,
                      const double z0,
                      const SZCovariance& szCovariance = SZCovariance(),
                      const double chi2 = 0.0,
                      const size_t ndf = 0)
        : Line2D(Line2D::fromSlopeIntercept(tanLambda, z0)),
          m_szCovariance(szCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}


      /// Augments a plain two dimensional line with a covariance matrix. Covariance defaults to zero.
      UncertainSZLine(const Line2D& line2D,
                      const SZCovariance& szCovariance = SZCovariance(),
                      const double chi2 = 0.0,
                      const size_t ndf = 0)
        : Line2D(line2D),
          m_szCovariance(szCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Getter for the sz parameters in the order defined by ESZParameter.h
      SZParameters parameters() const
      {
        using namespace NSZParameterIndices;
        SZParameters result;
        result(c_TanL) = slope();
        result(c_Z0) = intercept();
        return result;
      }

    public:
      /// Setter for the whole covariance matrix of the sz line parameters
      inline void setSZCovariance(const SZCovariance& szCovariance)
      { m_szCovariance = szCovariance; }

      /// Getter for the whole covariance matrix of the sz line parameters
      const SZCovariance& szCovariance() const
      { return m_szCovariance; }

      /// Getter for individual elements of the covariance matrix
      double covariance(const ESZParameter& iRow, const ESZParameter& iCol) const
      { return szCovariance()(iRow, iCol); }

      /// Getter for individual diagonal elements of the covariance matrix
      double variance(const ESZParameter& i) const
      { return szCovariance()(i, i); }

      /// Getter for the chi square value of the line fit
      double chi2() const
      { return m_chi2; }

      /// Setter for the chi square value of the line fit
      void setChi2(const double chi2)
      { m_chi2 = chi2; }

      /// Getter for the number of degrees of freediom used in the line fit
      const size_t& ndf() const
      { return m_ndf; }

      /// Setter for the number of degrees of freediom used in the line fit
      void setNDF(const size_t& ndf)
      { m_ndf = ndf; }


      /// Sets all line parameters to zero including the covariance matrix
      inline void invalidate()
      {
        Line2D::invalidate();
        m_szCovariance = SZUtil::identity();
        m_chi2 = 0.0;
        m_ndf = 0;
      }

    public:
      /// Reverses the direction of flight represented by this sz line
      inline void reverse()
      {
        Line2D::flipFirst();
        SZUtil::reverse(m_szCovariance);
      }

      /// Returns a copy of the line corresponding to the reverse direction of travel.
      inline UncertainSZLine reversed() const
      {
        return UncertainSZLine(Line2D::flippedFirst(),
                               SZUtil::reversed(szCovariance()),
                               chi2(),
                               ndf());
      }

    public:
      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      SZJacobian passiveMoveByJacobian(const Vector2D& bySZ) const
      {
        using namespace NSZParameterIndices;
        SZJacobian result = SZUtil::identity();
        result(c_Z0, c_TanL) = bySZ.first();
        return result;
      }

      /// Moves the coordinate system by the vector by and calculates the new sz line and its covariance matrix. Change is inplace.
      void passiveMoveBy(const Vector2D& bySZ)
      {
        // Move the covariance matrix first to have access to the original parameters
        SZJacobian jacobian = passiveMoveByJacobian(bySZ);
        SZUtil::transport(jacobian, m_szCovariance);
        Line2D::passiveMoveBy(bySZ);
      }

      /**
       *  Calculates the jacobian matrix that is needed for the translation of the parameter covariance
       *  for a passive move of the coordinate system.
       */
      SZCovariance passiveMovedCovarianceBy(const Vector2D& bySZ) const
      {
        SZJacobian jacobian = passiveMoveByJacobian(bySZ);
        return SZUtil::transported(jacobian, szCovariance());
      }


    private:
      /// Memory for the 2x2 covariance matrix of sz slope (aka tan lambda) and z0.
      SZCovariance m_szCovariance = SZUtil::identity();

      /// Memory for the chi square value of the fit of this line.
      double m_chi2 = 0.0;

      /// Memory for the number of degrees of freedim of the fit of this line.
      size_t m_ndf = 0.0;


    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
