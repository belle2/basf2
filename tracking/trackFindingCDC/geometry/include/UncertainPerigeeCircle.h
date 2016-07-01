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
    class UncertainPerigeeCircle : public PerigeeCircle {

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
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : PerigeeCircle(curvature, tangentialPhi, impact),
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
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : PerigeeCircle(curvature, tangential, impact),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero.
      UncertainPerigeeCircle(const PerigeeCircle& perigeeCircle,
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : PerigeeCircle(perigeeCircle),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero.
      UncertainPerigeeCircle(const GeneralizedCircle& generalizedCircle,
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : PerigeeCircle(generalizedCircle),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Constructor taking all stored parameters as a SVector.
      explicit
      UncertainPerigeeCircle(const PerigeeParameters& parameters,
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance(),
                             const double chi2 = 0.0,
                             const size_t& ndf = 0)
        : PerigeeCircle(parameters),
          m_perigeeCovariance(perigeeCovariance),
          m_chi2(chi2),
          m_ndf(ndf)
      {}

      /// Average the parameters of the two given perigee circles properly considering their covariance matrix.
      static UncertainPerigeeCircle average(const UncertainPerigeeCircle& startPerigeeCircle,
                                            const UncertainPerigeeCircle& endPerigeeCircle);


    public:
      /// Setter for the whole covariance matrix of the perigee parameters
      inline void setPerigeeCovariance(const PerigeeCovariance& perigeeCovariance)
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
      inline void invalidate()
      {
        PerigeeCircle::invalidate();
        m_perigeeCovariance = PerigeeUtil::identity();
        m_chi2 = 0.0;
      }

    public:
      /// Flips the orientation of the circle in place
      inline void reverse()
      {
        PerigeeCircle::reverse();
        PerigeeUtil::reverse(m_perigeeCovariance);
      }

      /// Returns a copy of the circle with opposite orientation.
      inline UncertainPerigeeCircle reversed() const
      {
        return UncertainPerigeeCircle(PerigeeCircle::reversed(),
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
        PerigeeJacobian jacobian = passiveMoveByJacobian(by);
        PerigeeUtil::transport(jacobian, m_perigeeCovariance);
        PerigeeCircle::passiveMoveBy(by);
      }

      /**
       *  Calculates the jacobian matrix that is needed for the translation of the parameter covariance
       *  for a passive move of the coordinate system.
       */
      PerigeeCovariance passiveMovedCovarianceBy(const Vector2D& by) const
      {
        PerigeeJacobian jacobian = passiveMoveByJacobian(by);
        return PerigeeUtil::transported(jacobian, perigeeCovariance());
      }

    public:
      /// Debug helper
      friend std::ostream& operator<<(std::ostream& output,
                                      const UncertainPerigeeCircle& perigeeCircle)
      {
        return output <<
               "UncertainPerigeeCircle(" <<
               "curvature=" << perigeeCircle.curvature() << "," <<
               "tangentialPhi=" << perigeeCircle.tangentialPhi() << "," <<
               "impact=" << perigeeCircle.impact() << ")" ;
      }


    private:
      /// Memory for the 3x3 covariance matrix of the perigee parameters.
      PerigeeCovariance m_perigeeCovariance = PerigeeUtil::identity();

      /// Memory for the chi square value of the fit of this circle.
      double m_chi2 = 0.0;

      /// Memory for the number of degrees of freedim of the fit of this circle.
      size_t m_ndf = 0.0;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
