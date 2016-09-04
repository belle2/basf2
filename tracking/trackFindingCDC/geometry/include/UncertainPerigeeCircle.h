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
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     *  Adds an uncertainty matrix to the circle in perigee parameterisation.
     */
    class UncertainPerigeeCircle {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainPerigeeCircle()
      {}

      /**
       * Composes an uncertain perigee circle from the perigee parameters
       * and a 3x3 covariance matrix.
       * Covariance matrix defaults to a zero matrix.
       */
      UncertainPerigeeCircle(const double curvature,
                             const double tangentialPhi,
                             const double impact,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : m_perigeeCircle(curvature, tangentialPhi, impact),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /**
       *  Composes an uncertain perigee circle from the perigee parameters
       *  and a 3x3 covariance matrix.
       *  Covariance matrix defaults to a zero matrix
       */
      UncertainPerigeeCircle(const double curvature,
                             const Vector2D& tangential,
                             const double impact,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : m_perigeeCircle(curvature, tangential, impact),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero.
      UncertainPerigeeCircle(const PerigeeCircle& perigeeCircle,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : m_perigeeCircle(perigeeCircle),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero.
      UncertainPerigeeCircle(const GeneralizedCircle& generalizedCircle,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : m_perigeeCircle(generalizedCircle),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Constructor taking all stored parameters as a SVector.
      explicit
      UncertainPerigeeCircle(const PerigeeParameters& parameters,
                             const PerigeeCovariance& perigeeCovariance = PerigeeUtil::identity(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : m_perigeeCircle(parameters),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Average the parameters of the two given perigee circles properly considering their covariance matrix.
      static UncertainPerigeeCircle average(const UncertainPerigeeCircle& startPerigeeCircle,
                                            const UncertainPerigeeCircle& endPerigeeCircle);


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
      { return &m_perigeeCircle; }

      /// Downcast to the "super" class
      operator const PerigeeCircle& () const
      { return m_perigeeCircle; }

      /// Getter for the underlying circle
      const PerigeeCircle& perigeeCircle() const
      { return m_perigeeCircle; }

      /// Getter for the perigee parameters in the order defined by EPerigeeParameter.h
      PerigeeParameters perigeeParameters() const
      { return m_perigeeCircle.perigeeParameters(); }

    public:
      /// Setter for the whole covariance matrix of the perigee parameters
      void setPerigeeCovariance(const PerigeeCovariance& perigeeCovariance)
      { m_perigeeCovariance = perigeeCovariance; }

      /// Getter for the whole covariance matrix of the perigee parameters
      const PerigeeCovariance& perigeeCovariance() const
      { return m_perigeeCovariance; }

      /// Getter for individual elements of the covariance matrix
      double covariance(const EPerigeeParameter& iRow,
                        const EPerigeeParameter& iCol) const
      { return perigeeCovariance()(iRow, iCol); }

      /// Getter for individual diagonal elements of the covariance matrix
      double variance(const EPerigeeParameter& i) const
      { return perigeeCovariance()(i, i); }

      /// Getter for the chi square value of the circle fit
      double chi2() const
      { return m_chi2; }

      /// Setter for the chi square value of the circle fit
      void setChi2(const double chi2)
      { m_chi2 = chi2; }

      /// Getter for the number of degrees of freediom used in the circle fit
      const size_t& ndf() const
      { return m_ndf; }

      /// Setter for the number of degrees of freediom used in the circle fit
      void setNDF(const size_t& ndf)
      { m_ndf = ndf; }


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
       *  Calculates the jacobian matrix that is needed for the translation of the parameter covariance
       *  for a passive move of the coordinate system.
       */
      PerigeeCovariance passiveMovedCovarianceBy(const Vector2D& by) const
      {
        PerigeeJacobian jacobian = m_perigeeCircle.passiveMoveByJacobian(by);
        return PerigeeUtil::transported(jacobian, perigeeCovariance());
      }

    public:
      /// Debug helper
      friend std::ostream& operator<<(std::ostream& output, const UncertainPerigeeCircle& circle)
      {
        return output << "UncertainPerigeeCircle("
               << "curvature=" << circle->curvature() << ","
               << "phi0=" << circle->phi0() << ","
               << "impact=" << circle->impact() << ")" ;
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

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
