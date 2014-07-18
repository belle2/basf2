/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef UNCERTAINPERIGEECIRCLE_H
#define UNCERTAINPERIGEECIRCLE_H

#include <cmath>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "Vector2D.h"
#include "PerigeeCircle.h"
#include "PerigeeCovariance.h"


namespace Belle2 {

  namespace CDCLocalTracking {

    ///A circle through the origin which center lies on the positive x-axis
    /** Makes a smooth generalization from a two dimensional normal line ( like Line2D ) to a circle \n
     *  since its single parameter is its curvature. This may serve as a base class for other low curvature
     *  circles, because the general case is similar the the standard circle up to translational and rotational
     *  transformations.
     *  This circle is implemented as a parameter curve. The parameter along the curve is the distance on the
     *  circle from the origin. It can carry an orientation in the sign of the curvature.
     */
    class UncertainPerigeeCircle : public PerigeeCircle {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainPerigeeCircle() :
        PerigeeCircle(),
        m_perigeeCovariance()
      {;}

      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance matrix. Covariance matrix defaults to a zero matrix
      UncertainPerigeeCircle(const FloatType& curvature,
                             const FloatType& tangentialPhi,
                             const FloatType& impact,
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance()) :
        PerigeeCircle(PerigeeCircle::fromPerigeeParameters(curvature, tangentialPhi, impact)),
        m_perigeeCovariance(perigeeCovariance)
      {;}



      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance matrix. Covariance matrix defaults to a zero matrix
      UncertainPerigeeCircle(const FloatType& curvature,
                             const Vector2D& tangential,
                             const FloatType& impact,
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance()) :
        PerigeeCircle(PerigeeCircle::fromPerigeeParameters(curvature, tangential, impact)),
        m_perigeeCovariance(perigeeCovariance)
      {;}



      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero
      UncertainPerigeeCircle(const PerigeeCircle& perigeeCircle,
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance()) :
        PerigeeCircle(perigeeCircle),
        m_perigeeCovariance(perigeeCovariance)
      {;}


      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero
      UncertainPerigeeCircle(const GeneralizedCircle& generalizedCircle,
                             const PerigeeCovariance& perigeeCovariance = PerigeeCovariance()) :
        PerigeeCircle(generalizedCircle),
        m_perigeeCovariance(perigeeCovariance)
      {;}

      /// Empty destructor
      ~UncertainPerigeeCircle() {;}

    public:
      /// Setter for the whole covariance matrix of the perigee parameters
      inline void setPerigeeCovariance(const PerigeeCovariance& perigeeCovariance)
      { m_perigeeCovariance = perigeeCovariance; }

      /// Getter for the whole covariance matrix of the perigee parameters
      const PerigeeCovariance& perigeeCovariance() const
      { return m_perigeeCovariance; }

      /// Getter for the chi square value of the circle fit
      const FloatType& chi2() const
      { return m_chi2; }

      /// Setter for the chi square value of the circle fit
      void setChi2(const FloatType& chi2)
      { m_chi2 = chi2; }


      /// Sets all circle parameters to zero including the covariance matrix
      inline void setNull() {
        PerigeeCircle::setNull();
        m_perigeeCovariance.setNull();
      }

    public:
      /// Flips the orientation of the circle in place
      inline void reverse() {
        PerigeeCircle::reverse();
        m_perigeeCovariance.reverse();
      }

      /// Returns a copy of the circle with opposite orientation.
      inline UncertainPerigeeCircle reversed() const {
        return UncertainPerigeeCircle(PerigeeCircle::reversed(),
                                      perigeeCovariance().reversed());
      }

    public:
      /// Moves the coordinate system by the vector by and calculates the new perigee and its covariance matrix. Change is inplace.
      void passiveMoveBy(const Vector2D& by) {
        // Move the covariance matrix first to have access to the original parameters
        TMatrixD jacobian = passiveMoveByJacobian(by);
        m_perigeeCovariance.similarityTransform(jacobian);
        PerigeeCircle::passiveMoveBy(by);
      }

      PerigeeCovariance passiveMovedCovarianceBy(const Vector2D& by) const {
        TMatrixD jacobian = passiveMoveByJacobian(by);
        return perigeeCovariance().similarityTransformed(jacobian);
      }


    public:
      /// Debug helper
      friend std::ostream& operator<<(std::ostream& output, const UncertainPerigeeCircle& perigeeCircle) {
        return output <<
               "UncertainPerigeeCircle(" <<
               "curvature=" << perigeeCircle.curvature() << "," <<
               "tangentialPhi=" << perigeeCircle.tangentialPhi() << "," <<
               "impact=" << perigeeCircle.impact() << ")" ;
      }


    private:
      /// Memory for the 3x3 covariance matrix of the perigee parameters
      PerigeeCovariance m_perigeeCovariance;

      /// Memory for the chi square value of the fit of this circle.
      FloatType m_chi2;

      /// ROOT Macro to make UncertainPerigeeCircle a ROOT class.
      ClassDefInCDCLocalTracking(UncertainPerigeeCircle, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // UNCERTAINPERIGEECIRCLE
