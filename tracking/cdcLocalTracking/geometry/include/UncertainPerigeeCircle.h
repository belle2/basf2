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
#include "TMatrixD.h"

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "Vector2D.h"
#include "PerigeeCircle.h"


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
        m_perigeeCovariance(3, 3) {
        m_perigeeCovariance.Zero();
      }

      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance matrix. Covariance matrix defaults to a zero matrix
      UncertainPerigeeCircle(const FloatType& curvature,
                             const FloatType& tangentialPhi,
                             const FloatType& impact,
                             const TMatrixD& perigeeCovariance = TMatrixD(3, 3)) :
        PerigeeCircle(PerigeeCircle::fromPerigeeParameters(curvature, tangentialPhi, impact)),
        m_perigeeCovariance(perigeeCovariance) {
        checkCovarianceMatrix();
      }

      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero
      UncertainPerigeeCircle(const PerigeeCircle& perigeeCircle,
                             const TMatrixD& perigeeCovariance = TMatrixD(3, 3)) :
        PerigeeCircle(perigeeCircle),
        m_perigeeCovariance(perigeeCovariance) {
        checkCovarianceMatrix();
      }

      /// Augments a plain perigee circle with a covariance matrix. Covariance defaults to zero
      UncertainPerigeeCircle(const GeneralizedCircle& generalizedCircle,
                             const TMatrixD& perigeeCovariance = TMatrixD(3, 3)) :
        PerigeeCircle(generalizedCircle),
        m_perigeeCovariance(perigeeCovariance) {
        checkCovarianceMatrix();
      }

      /// Empty destructor
      ~UncertainPerigeeCircle() {;}



    private:
      /// Checks the covariance matrix for consistence
      inline void checkCovarianceMatrix() const {
        if (m_perigeeCovariance.GetNrows() != 3 or m_perigeeCovariance.GetNcols() != 3) {
          B2ERROR("Perigee covariance is a  " <<
                  m_perigeeCovariance.GetNrows() << "x" <<
                  m_perigeeCovariance.GetNcols() << " matrix. " <<
                  "Expected 3x3 matrix"
                 );
        }
      }

    public:
      /// Setter for the whole covariance matrix of the perigee parameters
      inline void perigeeCovariance(const TMatrixD& perigeeCovariance)
      { m_perigeeCovariance = perigeeCovariance; }

      /// Getter for the whole covariance matrix of the perigee parameters
      const TMatrixD& getPerigeeCovariance() const
      { return m_perigeeCovariance; }

    private:
      /// Modifies to perigee covariance matrix inplace to represent the reverse travel direction.
      void reversePerigeeCovariance(TMatrixD& perigeeCovariance) const {
        // Sign of impact and curvature are reversed by a reversion of the circle parameters,
        // while the tangential phi gets only an offset by a constant
        // Hence corralations between the tangential phi and impact, tangential phi and curvatutre
        // receive a minus sign upon reversion.
        perigeeCovariance(0, 1) = -perigeeCovariance(0, 1);
        perigeeCovariance(1, 0) = -perigeeCovariance(1, 0);

        perigeeCovariance(2, 1) = -perigeeCovariance(2, 1);
        perigeeCovariance(1, 2) = -perigeeCovariance(1, 2);
      }

    public:
      /// Returns the reversed covariance matrix as a copy.
      TMatrixD reversedPerigeeCovariance() const {
        TMatrixD result = m_perigeeCovariance;
        reversePerigeeCovariance(result);
        return result;
      }

      /// Sets all circle parameters to zero including the covariance matrix
      inline void setNull() {
        PerigeeCircle::setNull();
        m_perigeeCovariance.Zero();
      }

    public:
      /// Flips the orientation of the circle in place
      inline void reverse() {
        PerigeeCircle::reverse();
        reversePerigeeCovariance(m_perigeeCovariance);
      }

      /// Returns a copy of the circle with opposite orientation.
      inline UncertainPerigeeCircle reversed() const {
        return UncertainPerigeeCircle(PerigeeCircle::reversed(),
                                      reversedPerigeeCovariance());
      }


    public:
      /// Moves the coordinate system by the vector by and calculates the new perigee and its covariance matrix. Change is inplace.
      void passiveMoveBy(const Vector2D& by) {
        // Move the covariance matrix first to have access to the original parameters
        passiveMoveCovarianceBy(by);
        PerigeeCircle::passiveMoveBy(by);
      }

    private:
      /// Helper function to calculate and update the covariance matrix inplace.
      void passiveMoveCovarianceBy(const Vector2D& by) {
        TMatrixD newPerigeeCovariance = passiveMovedCovarianceBy(by);
        m_perigeeCovariance = newPerigeeCovariance;
      }

    public:
      /// Transfers the covariance matrix of the perigee coordinates from the origin to the new reference point by.
      TMatrixD passiveMovedCovarianceBy(const Vector2D& by) const
      { return passiveMovedCovarianceBy(by, m_perigeeCovariance); }

      /// Transfers the given covariance matrix of the perigee coordinates from the origin to the new reference point by.
      TMatrixD passiveMovedCovarianceBy(const Vector2D& by, const TMatrixD& perigeeCovariance) const;

      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      TMatrixD passiveMoveByJacobian(const Vector2D& by) const;


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
      /** Matrix indices to parameter correspondency is as follows
       *  * 0 <-> curvature
       *  * 1 <-> tangentialPhi at Perigee
       *  * 2 <-> signed impact parameter
       */
      TMatrixD m_perigeeCovariance;

      /// ROOT Macro to make StandardOriginCircle2D a ROOT class.
      ClassDefInCDCLocalTracking(UncertainPerigeeCircle, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // UNCERTAINPERIGEECIRCLE
