/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/dataobjects/Helix.h>

#include <TMatrixDSym.h>


class TVector3;

namespace Belle2 {

  /** This class represents an ideal helix in perigee parameterization including the covariance matrix of the 5 perigee parameters.
   *  The used perigee parameters are:
   *
   *  1. @f$ d_0 @f$ - the signed distance from the origin to the perigee. The sign is positive (negative),
   *                       if the angle from the xy perigee position vector to the transverse momentum vector is +pi/2 (-pi/2).
   *                       @f$d_0@f$ has the same sign as `getPerigee().Cross(getMomentum()).Z()`.
   *  2. @f$ \phi_0 @f$ - the angle in the xy projection between the transverse momentum and the x axis, which is in [-pi, pi]
   *  3. @f$ \omega @f$ - the signed curvature of the track where the sign is given by the charge of the particle
   *  4. @f$ z_0 @f$ - z coordinate of the perigee
   *  5. @f$ \tan \lambda @f$ - the slope of the track in the sz plane (dz/ds)
   *
   *  in that exact order.
   *
   *  It may also store a p-value obtained from the fit of the helix.
   */
  class UncertainHelix : public Helix {
  public:
    /** Default constuctor initialising all members to zero.*/
    UncertainHelix();

    /** Constructor initializing class with fit result.
     *
     *  The given position, momentum and their covariance matrix are extrapolated to the perigee assuming a homogeneous magnetic field in the z direction.
     *
     *  @param position               Position of the track at the perigee.
     *  @param momentum               Momentum of the track at the perigee.
     *  @param charge                 Charge of the particle.
     *  @param bZ                     Magnetic field to be used for the calculation of the curvature;
     *  @param cartesianCovariance    6x6 Covariance matrix for position and momentum of the track at the perigee.
     *  @param pValue                 p-value of the fit.
     *                                It is assumed, that the B-field is parallel to the z-Axis.
     */
    UncertainHelix(const TVector3& position,
                   const TVector3& momentum,
                   const short int charge,
                   const double bZ,
                   const TMatrixDSym& cartesianCovariance,
                   const double pValue);

    /** Constructor initializing class with perigee parameters.
     *
     *  @param d0            The signed distance from the origin to the perigee. The sign is positive (negative),
     *                       if the angle from the xy perigee position vector to the transverse momentum vector is +pi/2 (-pi/2).
     *                       d0 has the same sign as `getPosition().Cross(getMomentum()).Z()`.
     *  @param phi0          The angle between the transverse momentum and the x axis and in [-pi, pi]
     *  @param omega         The signed curvature of the track where the sign is given by the charge of the particle
     *  @param z0            The z coordinate of the perigee.
     *  @param tanLambda     The slope of the track in the sz plane (dz/ds)
     *  @param covariance    5x5 Covariance matrix for the five helix parameters. Indices correspond to the order d0, phi0, omega, z0, tanLambda.
     *  @param pValue        p-value of the Helix fit
     */
    UncertainHelix(const double& d0,
                   const double& phi0,
                   const double& omega,
                   const double& z0,
                   const double& tanLambda,
                   const TMatrixDSym& covariance,
                   const double pValue);

    /** Getter for the position and momentum covariance matrix.
     *
     *  @note Because the position and momentum covariance matrix is regenerated
     *  from a 5x5 perigee parameter covariance matrix there is necessarily a missing
     *  rank in the resulting matrix. The rank has to be filled by a convention
     *  essentially expressing, which points around the perigee are considered to be at s = 0.
     *  For backwards compatability with the original Belle experiment we apply
     *  the convention used there. See the Belle II note for more details.
     *
     *  @param bZ_tesla      Magnetic field to be used for the calculation from the curvature;
     */
    TMatrixDSym getCartesianCovariance(const double bZ_tesla = 1.5) const;

    /** Getter for Chi2 Probability of the track fit. */
    double getPValue() const
    { return m_pValue; }

    /** Getter for covariance matrix of perigee parameters in matrix form.
     *
     */
    const TMatrixDSym& getCovariance() const
    { return m_covariance; }

    /** Reverses the direction of travel of the helix in place.
     *
     *  The same points that are located on the helix stay are the same after the transformation,
     *  but have the opposite two dimensional arc length associated to them.
     *  The momentum at each point is reversed.
     *  The charge sign is changed to its opposite by this transformation.
     */
    void reverse();

    /** Moves origin of the coordinate system (passive transformation) by the given vector. Updates the helix inplace.
     *
     *  @param by            Vector by which the origin of the coordinate system should be moved.
     *  @return              The double value is the two dimensional arc length, which has the be traversed from the old perigee to the new.
     */
    double passiveMoveBy(const TVector3& by)
    { return passiveMoveBy(by.X(), by.Y(), by.Z()); }

    /** Moves origin of the coordinate system (passive transformation) by the given vector. Updates the helix inplace.
     *
     *  @param byX           X displacement by which the origin of the coordinate system should be moved.
     *  @param byY           Y displacement by which the origin of the coordinate system should be moved.
     *  @param byZ           Z displacement by which the origin of the coordinate system should be moved.
     *  @return              The double value is the two dimensional arc length, which has the be traversed from the old perigee to the new.
     *
     */
    double passiveMoveBy(const double& byX,
                         const double& byY,
                         const double& byZ);

  private:
    /** 5x5 covariance of the perigee parameters. */
    TMatrixDSym m_covariance;

    /** Chi2 Probability of the fit. */
    Double32_t m_pValue;

    ClassDef(UncertainHelix, 2); /**< represents an ideal helix in perigee parameterization including covariance matrix */
  };
}

