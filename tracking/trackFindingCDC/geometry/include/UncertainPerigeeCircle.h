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

#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>

#include <cstddef>
#include <iosfwd>

namespace Belle2 {

  namespace TrackFindingCDC {
    class GeneralizedCircle;
    class Vector2D;

    /**
     *  Adds an uncertainty matrix to the circle in perigee parameterisation.
     */
    class UncertainPerigeeCircle {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainPerigeeCircle()
      {
      }

      /**
       * Composes an uncertain perigee circle from the perigee parameters
       * and a 3x3 covariance matrix.
       * Covariance matrix defaults to a zero matrix.
       */
      UncertainPerigeeCircle(const double curvature,
                             const double phi0Vec,
                             const double impact,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             std::size_t ndf = 0)
        : m_perigeeCircle(curvature, phi0Vec, impact)
        , m_perigeeCovariance(perigeeCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /**
       *  Composes an uncertain perigee circle from the perigee parameters
       *  and a 3x3 covariance matrix.
       *  Covariance matrix defaults to a zero matrix
       */
      UncertainPerigeeCircle(const double curvature,
                             const Vector2D& phi0,
                             const double impact,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             std::size_t ndf = 0)
        : m_perigeeCircle(curvature, phi0, impact)
        , m_perigeeCovariance(perigeeCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero.
      UncertainPerigeeCircle(const PerigeeCircle& perigeeCircle,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             std::size_t ndf = 0)
        : m_perigeeCircle(perigeeCircle)
        , m_perigeeCovariance(perigeeCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero.
      UncertainPerigeeCircle(const GeneralizedCircle& generalizedCircle,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             std::size_t ndf = 0)
        : m_perigeeCircle(generalizedCircle)
        , m_perigeeCovariance(perigeeCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Constructor taking all stored parameters as a SVector.
      explicit UncertainPerigeeCircle(
        const PerigeeParameters& parameters,
        const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
        const double chi2 = 0.0,
        std::size_t ndf = 0)
        : m_perigeeCircle(parameters)
        , m_perigeeCovariance(perigeeCovariance)
        , m_chi2(chi2)
        , m_ndf(ndf)
      {
      }

      /// Average the parameters of the two given perigee circles properly considering their
      /// covariance matrix.
      static UncertainPerigeeCircle average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                            const UncertainPerigeeCircle& toPerigeeCircle);

    public:
      /**
       *  Access to the constant interface of the underlying parameter line
       *  Allows the user of this "super" class to use the getters and
       *  other methods to get inforamation about the line but disallows mutation
       *  of internal state.
       *  This ensures that the parameters are not changed without proper adjustment to
       *  the covariance matrix in this class, which can be achieved by the more limited
       *  set of methods in this class accessable by normal . method calls
       */
      const PerigeeCircle* operator->() const
      {
        return &m_perigeeCircle;
      }

      /// Downcast to the "super" class
      operator const PerigeeCircle& () const
      {
        return m_perigeeCircle;
      }

      /// Getter for the underlying circle
      const PerigeeCircle& perigeeCircle() const
      {
        return m_perigeeCircle;
      }

      /// Getter for the perigee parameters in the order defined by EPerigeeParameter.h
      PerigeeParameters perigeeParameters() const
      {
        return m_perigeeCircle.perigeeParameters();
      }

    public:
      /// Setter for the whole covariance matrix of the perigee parameters
      void setPerigeeCovariance(const PerigeeCovariance& perigeeCovariance)
      {
        m_perigeeCovariance = perigeeCovariance;
      }

      /// Getter for the whole covariance matrix of the perigee parameters
      const PerigeeCovariance& perigeeCovariance() const
      {
        return m_perigeeCovariance;
      }

      /// Getter for individual elements of the covariance matrix
      double covariance(const EPerigeeParameter& iRow, const EPerigeeParameter& iCol) const
      {
        return perigeeCovariance()(iRow, iCol);
      }

      /// Getter for individual diagonal elements of the covariance matrix
      double variance(const EPerigeeParameter& i) const
      {
        return perigeeCovariance()(i, i);
      }

      /// Getter for the chi square value of the circle fit
      double chi2() const
      {
        return m_chi2;
      }

      /// Setter for the chi square value of the circle fit
      void setChi2(const double chi2)
      {
        m_chi2 = chi2;
      }

      /// Getter for the number of degrees of freediom used in the circle fit
      std::size_t ndf() const
      {
        return m_ndf;
      }

      /// Setter for the number of degrees of freediom used in the circle fit
      void setNDF(std::size_t ndf)
      {
        m_ndf = ndf;
      }

      /// Sets all circle parameters to zero including the covariance matrix
      void invalidate()
      {
        m_perigeeCircle.invalidate();
        m_perigeeCovariance = PerigeeUtil::identity();
        m_chi2 = 0.0;
      }

    public:
      /// Flips the orientation of the circle in place
      void reverse()
      {
        m_perigeeCircle.reverse();
        PerigeeUtil::reverse(m_perigeeCovariance);
      }

      /// Returns a copy of the circle with opposite orientation.
      UncertainPerigeeCircle reversed() const
      {
        return UncertainPerigeeCircle(m_perigeeCircle.reversed(),
                                      PerigeeUtil::reversed(perigeeCovariance()),
                                      chi2(),
                                      ndf());
      }

    public:
      /**
       *  Moves the coordinate system by the vector by and
       *  calculates the new perigee and its covariance matrix.
       *  Change is inplace.
       */
      void passiveMoveBy(const Vector2D& by)
      {
        // Move the covariance matrix first to have access to the original parameters
        PerigeeJacobian jacobian = m_perigeeCircle.passiveMoveByJacobian(by);
        PerigeeUtil::transport(jacobian, m_perigeeCovariance);
        m_perigeeCircle.passiveMoveBy(by);
      }

      /**
       *  Calculates the jacobian matrix that is needed for the translation of the parameter
       * covariance
       *  for a passive move of the coordinate system.
       */
      PerigeeCovariance passiveMovedCovarianceBy(const Vector2D& by) const
      {
        PerigeeJacobian jacobian = m_perigeeCircle.passiveMoveByJacobian(by);
        return PerigeeUtil::transported(jacobian, perigeeCovariance());
      }

    private:
      /// Memory for the underlying circle
      PerigeeCircle m_perigeeCircle;

      /// Memory for the 3x3 covariance matrix of the perigee parameters.
      PerigeeCovariance m_perigeeCovariance = PerigeeUtil::identity();

      /// Memory for the chi square value of the fit of this circle.
      double m_chi2 = 0.0;

      /// Memory for the number of degrees of freedim of the fit of this circle.
      size_t m_ndf = 0.0;

    };

    /// Debug helper
    std::ostream& operator<<(std::ostream& output, const UncertainPerigeeCircle& circle);
  }
}
