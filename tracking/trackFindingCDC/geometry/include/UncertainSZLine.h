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

#include <tracking/trackFindingCDC/geometry/SZLine.h>

#include <tracking/trackFindingCDC/geometry/SZParameters.h>

#include <cstddef>
#include <iosfwd>

namespace Belle2 {

  namespace TrackFindingCDC {
    class Vector2D;

    /// A line in sz where s is the transverse travel distance as seen in the xy projection with
    /// uncertainties in the slope and intercept of the line.
    class UncertainSZLine {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainSZLine()
      {
      }

      /// Composes an uncertain sz line from the  perigee parameters and a 2x2 covariance matrix.
      /// Covariance matrix defaults to a zero matrix.
      UncertainSZLine(const double tanLambda,
                      const double z0,
                      const SZCovariance& szCovariance = SZUtil::identity(),
                      const double chi2 = 0.0,
                      const size_t ndf = 0)
        : m_szLine(tanLambda, z0)
        , m_szCovariance(szCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Composes an uncertain sz line from the  perigee parameters and a 2x2 covariance matrix.
      /// Covariance matrix defaults to a zero matrix.
      UncertainSZLine(const SZParameters szParameters,
                      const SZCovariance& szCovariance = SZUtil::identity(),
                      const double chi2 = 0.0,
                      const size_t ndf = 0)
        : m_szLine(szParameters)
        , m_szCovariance(szCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Augments a plain two dimensional line with a covariance matrix. Covariance defaults to
      /// zero.
      UncertainSZLine(const SZLine& szLine,
                      const SZCovariance& szCovariance = SZUtil::identity(),
                      const double chi2 = 0.0,
                      const size_t ndf = 0)
        : m_szLine(szLine)
        , m_szCovariance(szCovariance)
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
      const SZLine* operator->() const
      {
        return &m_szLine;
      }

      /// Downcast to the "super" class
      operator const SZLine& () const
      {
        return m_szLine;
      }

      /// Getter for the  underlying sz line
      const SZLine& szLine() const
      {
        return m_szLine;
      }

      /// Getter for the sz parameters in the order defined by ESZParameter.h
      SZParameters szParameters() const
      {
        return m_szLine.szParameters();
      }

    public:
      /// Setter for the whole covariance matrix of the sz line parameters
      void setSZCovariance(const SZCovariance& szCovariance)
      {
        m_szCovariance = szCovariance;
      }

      /// Getter for the whole covariance matrix of the sz line parameters
      const SZCovariance& szCovariance() const
      {
        return m_szCovariance;
      }

      /// Getter for individual elements of the covariance matrix
      double covariance(const ESZParameter& iRow, const ESZParameter& iCol) const
      {
        return szCovariance()(iRow, iCol);
      }

      /// Getter for individual diagonal elements of the covariance matrix
      double variance(const ESZParameter& i) const
      {
        return szCovariance()(i, i);
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
        m_szLine.invalidate();
        m_szCovariance = SZUtil::identity();
        m_chi2 = 0.0;
        m_ndf = 0;
      }

    public:
      /// Reverses the direction of flight represented by this sz line
      void reverse()
      {
        m_szLine.reverse();
        SZUtil::reverse(m_szCovariance);
      }

      /// Returns a copy of the line corresponding to the reverse direction of travel.
      UncertainSZLine reversed() const
      {
        return UncertainSZLine(m_szLine.reversed(), SZUtil::reversed(szCovariance()), chi2(), ndf());
      }

    public:
      /// Moves the coordinate system by the vector by and calculates the new sz line and its
      /// covariance matrix. Change is inplace.
      void passiveMoveBy(const Vector2D& bySZ)
      {
        // Move the covariance matrix first to have access to the original parameters
        SZJacobian jacobian = m_szLine.passiveMoveByJacobian(bySZ);
        SZUtil::transport(jacobian, m_szCovariance);
        m_szLine.passiveMoveBy(bySZ);
      }

      /**
       *  Calculates the jacobian matrix that is needed for the translation of the parameter
       * covariance
       *  for a passive move of the coordinate system.
       */
      SZCovariance passiveMovedCovarianceBy(const Vector2D& bySZ) const
      {
        SZJacobian jacobian = m_szLine.passiveMoveByJacobian(bySZ);
        return SZUtil::transported(jacobian, szCovariance());
      }

    private:
      /// Memory for the underlying line
      SZLine m_szLine;

      /// Memory for the 2x2 covariance matrix of sz slope (aka tan lambda) and z0.
      SZCovariance m_szCovariance = SZUtil::identity();

      /// Memory for the chi square value of the fit of this line.
      double m_chi2 = 0.0;

      /// Memory for the number of degrees of freedim of the fit of this line.
      size_t m_ndf = 0;
    };

    /// Debug helper
    std::ostream& operator<<(std::ostream& output, const UncertainSZLine& uncertainSZLine);
  }
}
