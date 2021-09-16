/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/RelationsObject.h>

#include <framework/geometry/B2Vector3.h>
#include <TVector3.h>
#include <TMatrixDfwd.h>

#include <cmath>

namespace Belle2 {

  namespace HelixParameterIndex {
    /// Helix parameter index in covariance and parameter vector.
    enum HelixParameterIndex {
      iD0 = 0,
      iPhi0 = 1,
      iOmega = 2,
      iZ0 = 3,
      iTanLambda = 4,
      /// Dummy parameter to refer to the two dimensional arc length.
      /// Technically this is not a parameter of the fundamental helix representation, but it is essential for translating between the cartesian parameters and helix parameters to be exact.
      /// Only after the translation has been performed completely this additional parameter can be dropped.
      iArcLength2D = 5,
    };
  }

  /** This class represents an ideal helix in perigee parameterization.
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
   *  Each point on the helix can be adressed by the two dimensional arc length s, which has to be traversed to get to it from the perigee.
   *  More precisely the two dimensional arc length means the transverse part of the particles travel distance,
   *  hence the arc length of the circle in the xy projection.
   *
   *  If you need different kind of methods / interfaces to the helix please do not hesitate to contact oliver.frost@desy.de
   *  Contributions are always welcome.
   *
   */
  class Helix : public RelationsObject {
  public:

    /** Constructor initializing all perigee parameters to zero. */
    Helix();

    /** Constructor initializing class with a fit result.
     *
     *  The given position and momentum are extrapolated to the perigee assuming a homogeneous magnetic field in the z direction.
     *
     *  @param position      Position of the track at the perigee.
     *  @param momentum      Momentum of the track at the perigee.
     *  @param charge        Charge of the particle.
     *  @param bZ            Magnetic field to be used for the calculation of the curvature.
     *                       It is assumed, that the B-field is homogeneous parallel to the z axis.
     */
    Helix(const TVector3& position,
          const TVector3& momentum,
          const short int charge,
          const double bZ);

    /** Constructor initializing class with perigee parameters.
     *
     *  @param d0            The signed distance from the origin to the perigee. The sign is positive (negative),
     *                       if the angle from the xy perigee position vector to the transverse momentum vector is +pi/2 (-pi/2).
     *                       d0 has the same sign as `getPosition().Cross(getMomentum()).Z()`.
     *  @param phi0          The angle between the transverse momentum and the x axis and in [-pi, pi].
     *  @param omega         The signed curvature of the track where the sign is given by the charge of the particle.
     *  @param z0            The z coordinate of the perigee.
     *  @param tanLambda     The slope of the track in the sz plane (dz/ds).
     */
    Helix(const double& d0,
          const double& phi0,
          const double& omega,
          const double& z0,
          const double& tanLambda);

    /** Output operator for debugging and the generation of unittest error messages.*/
    friend std::ostream& operator<<(std::ostream& output, const Helix& helix);


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Getters for cartesian parameters of the perigee
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------
  public:
    /** Calculates the x coordinate of the perigee point.*/
    double getPerigeeX() const;

    /** Calculates the y coordinate of the perigee point.*/
    double getPerigeeY() const;

    /** Calculates the z coordinate of the perigee point.*/
    double getPerigeeZ() const;

    /** Getter for the perigee position. */
    TVector3 getPerigee() const;

    /** Calculates the x momentum of the particle at the perigee point.
     *  @param bZ            Z component of the magnetic field in Tesla
     */
    double getMomentumX(const double bZ) const;

    /** Calculates the y momentum of the particle at the perigee point.
     *  @param bZ            Z component of the magnetic field in Tesla
     */
    double getMomentumY(const double bZ) const;

    /** Calculates the z momentum of the particle at the perigee point.
     *  @param bZ            Z component of the magnetic field in Tesla
     */
    double getMomentumZ(const double bZ) const;

    /** Getter for vector of momentum at the perigee position
     *
     *  As we calculate recalculate the momentum from a geometric helix, we need an estimate
     *  of the magnetic field along the z-axis to give back the momentum.
     *  @param bZ            Magnetic field at the perigee.
     */
    TVector3 getMomentum(const double bZ) const;

    /** Getter for unit vector of momentum at the perigee position
     *
     * This is mainly useful cases where curvature is zero (pT is infinite)
     */
    TVector3 getDirection() const;

    /** Getter for the absolute value of the transverse momentum at the perigee.
     *
     *  @param bZ            Magnetic field at the perigee
     */
    double getTransverseMomentum(const double bZ) const;

    /** Getter for kappa, which is charge / transverse momentum or equivalently omega * alpha */
    double getKappa(const double bZ) const;

    /** Calculates the alpha value for a given magnetic field in Tesla */
    static double getAlpha(const double bZ);

    /** Return track charge sign (1, 0 or -1).*/
    short getChargeSign() const;
    /// @}


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Simple extrapolations of the ideal helix
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------
  public:
    /** Calculates the transverse travel distance at the point the helix first reaches the given cylindrical radius.
     *
     *  Gives the two dimensional arc length in the forward direction that is traversed until a certain cylindrical radius is reached.
     *  Returns NAN, if the cylindrical radius can not be reached, either because it is to far outside or inside of the perigee.
     *
     *  Forward the result to getPositionAtArcLength2D() or getMomentumAtArcLength2D() in order to extrapolate to the cylinder detector boundaries.
     *
     *  The result always has a positive sign. Hence it refers to the forward direction.
     *  Adding a minus sign yields the point at the same cylindrical radius but in the backward direction.
     *
     *  @param cylindricalR  The cylinder radius to extrapolate to.
     *  @return              The two dimensional arc length traversed to reach the cylindrical radius. NAN if it can not be reached.
     */
    double getArcLength2DAtCylindricalR(const double& cylindricalR) const;

    /** Calculates the two dimensional arc length at which the circle in the xy projection is closest to the point
     *
     *  This calculates the dimensional arc length to the closest approach in xy projection.
     *  Hence, it only optimizes the distance in x and y.
     *  This is sufficent to extrapolate to an axial wire.
     *  For stereo wires this is not optimal, since the distance in the z direction also plays a role.
     *
     *  @param x        X coordinate of the point to which to extrapolate
     *  @param y        Y coordinate of the point to which to extrapolate
     *  @return         The two dimensional arc length from the perigee at which the closest approach is reached
     */
    double getArcLength2DAtXY(const double& x, const double& y) const;

    /** Calculates the arc length to reach a plane parallel to the z axes.
     *
     *  If there is no intersection with the plane NAN is returned
     *
     *  @param x        X coordinate of the support point of the plane
     *  @param y        Y coordinate of the support point of the plane
     *  @param nX       X coordinate of the normal vector of the plane
     *  @param nY       Y coordinate of the normal vector of the plane
     *  @return         Shortest two dimensional arc length to the plane
     */
    double getArcLength2DAtNormalPlane(const double& x, const double& y,
                                       const double& nX, const double& nY) const;

    /** Calculates the position on the helix at the given two dimensional arc length
     *
     *  @param arcLength2D       Two dimensional arc length to be traversed.
     */
    TVector3 getPositionAtArcLength2D(const double& arcLength2D) const;

    /** Calculates the tangential vector to the helix curve at the given two dimensional arc length.
     *
     *  The tangential vector is the derivative of the position with respect to the two dimensional arc length
     *  It is normalised such that the cylindrical radius of the result is 1
     *
     *  getTangentialAtArcLength2D(arcLength2D).Perp() == 1 holds.
     *
     *  @param arcLength2D       Two dimensional arc length to be traversed.
     *  @return                  Tangential vector normalised to unit transverse component / cylindrical radius.
     */
    TVector3 getTangentialAtArcLength2D(const double& arcLength2D) const;

    /** Calculates the unit tangential vector to the helix curve at the given two dimensional arc length.
     *
     *  @param arcLength2D       Two dimensional arc length to be traversed.
     */
    TVector3 getUnitTangentialAtArcLength2D(const double& arcLength2D) const;

    /** Calculates the momentum vector at the given two dimensional arc length.
     *
     *  @param arcLength2D       Two dimensional arc length to be traversed.
     *  @param bz                Magnetic field strength in the z direction.
     */
    TVector3 getMomentumAtArcLength2D(const double& arcLength2D, const double& bz) const;

    /** Moves origin of the coordinate system (passive transformation) by the given vector. Updates the helix inplace.
     *
     *  @param by            Vector by which the origin of the coordinate system should be moved.
     *  @return              The double value is the two dimensional arc length, which has the be traversed from the old perigee to the new.
     */
    double passiveMoveBy(const TVector3& by)
    { return passiveMoveBy(by.X(), by.Y(), by.Z()); }

    /** Moves origin of the coordinate system (passive transformation) orthogonal to the z axis by the given vector. Updates the helix inplace.
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


    /** Calculate the 5x5 jacobian matrix for the transport of the helix parameters,
     *  when moving the origin of the coordinate system to a new location.
     *  Does not update the helix parameters in any way.
     *
     *  @param by              Vector by which the origin of the coordinate system should be moved.
     *  @param expandBelowChi  Control parameter below, which absolute value of chi an expansion of divergent terms shall be used.
     *                         This parameter exists for testing the consistency of the expansion with the closed form.
     *                         In other applications the parameter should remain at its default value.
     *  @return                The jacobian matrix containing the derivatives of the five helix parameters after the move relative the orignal parameters.
     */
    TMatrixD calcPassiveMoveByJacobian(const TVector3& by, const double expandBelowChi = M_PI / 8) const;


  protected:
  public:
    /** Calculate the jacobian matrix for the transport of the helix parameters,
     *  when moving the origin of the coordinate system to a new location.
     *  Does not update the helix parameters in any way.
     *
     *  The jacobian matrix is written into the output parameter 'jacobian'.
     *  If output parameter is a 5x5 matrix only the derivatives of the 5 helix parameters are written
     *  If output parameter is a 6x6 matrix the derivatives of the 5 helix parameters *and* derivates of the two dimensional arc length are written.
     *  The derivatives of the arcLength2D are in the 6th row of the matrix.
     *
     *  @param byX           X displacement by which the origin of the coordinate system should be moved.
     *  @param byY           Y displacement by which the origin of the coordinate system should be moved.
     *  @param[out] jacobian The jacobian matrix containing the derivatives of the five helix parameters
     *                       after the move relative the orignal parameters.
     *  @param expandBelowChi Control parameter below, which absolute value of chi an expansion of
     *                        divergent terms shall be used. This parameter exists for testing the
     *                        consistency of the expansion with the closed form.
     *                        In other applications the parameter should remain at its default value.
     *
     */
    void calcPassiveMoveByJacobian(const double& byX,
                                   const double& byY,
                                   TMatrixD& jacobian,
                                   const double expandBelowChi = M_PI / 8) const;

  public:
    /** Reverses the direction of travel of the helix in place.
     *
     *  The same points that are located on the helix stay are the same after the transformation,
     *  but have the opposite two dimensional arc length associated to them.
     *  The momentum at each point is reversed.
     *  The charge sign is changed to its opposite by this transformation.
     */
    void reverse();

    /** Reverses an azimuthal angle to the opposite direction
     *
     *  @param phi             A angle in [-pi, pi]
     *  @return                The angle for the opposite direction in [-pi, pi]
     */
    static double reversePhi(const double& phi);

    /** Helper function to calculate the two dimensional arc length from the length of a secant.
     *
     *  Translates the direct length between two point on the circle in the xy projection to the two dimensional arc length on the circle
     *  Behaves smoothly in the limit of vanishing curvature.
     */
    double calcArcLength2DFromSecantLength(const double& secantLength2D) const;

    /** Helper function to calculate the factor between the dimensional secant length and the two dimensional arc length as seen in xy projection of the helix
     */
    double calcSecantLengthToArcLength2DFactor(const double& secantLength2D) const;
    /// @}

  protected:
    /** Implementation of the function asin(x) / x which handles small x values smoothly. */
    static double calcASinXDividedByX(const double& x);

    /** Implementation of the function atan(x) / x which handles small x values smoothly. */
    static double calcATanXDividedByX(const double& x);

    /** Implementation of the function d / dx (atan(x) / x) which handles small x values smoothly. */
    static double calcDerivativeOfATanXDividedByX(const double& x);

    /** Helper method to calculate the signed two dimensional arc length and the signed distance to the circle of a point in the xy projection.
     *
     *  This function is an implementation detail that prevents some code duplication.
     *
     *  @param x                   X coordinate of the point to which to extrapolate
     *  @param y                   Y coordinate of the point to which to extrapolate
     *  @param[out] arcLength2D    The two dimensional arc length from the perigee at which the closest approach is reached
     *  @param[out] dr             Signed distance of the point to circle in the xy projection.
     */
    void calcArcLength2DAndDrAtXY(const double& x, const double& y, double& arcLength2D, double& dr) const;

    /** Helper method to calculate the two dimensional arc length *from the perigee* to a point at cylindrical radius, which also has the distance dr from the circle in the xy projection
     *
     *  This function is an implementation detail that prevents some code duplication.
     *
     *  @param deltaCylindricalR             The absolute distance of the point in question to the perigee in the xy projection
     *  @param dr                            Signed distance of the point to circle in the xy projection.
     *  @return                              The absolute two dimensional arc length from the perigee to the point.
     */
    double calcArcLength2DAtDeltaCylindricalRAndDr(const double& deltaCylindricalR, const double& dr) const;


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Getters for perigee helix parameters
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------
  public:
    /** Getter for d0, which is the signed distance to the perigee in the r-phi plane.
     *
     *  The signed distance from the origin to the perigee. The sign is positive (negative),
     *  if the angle from the xy perigee position vector to the transverse momentum vector is +pi/2 (-pi/2).
     *  d0 has the same sign as `getPerigee().Cross(getMomentum()).Z()`.
     */
    double getD0() const { return m_d0; }

    /** Getter for phi0, which is the azimuth angle of the transverse momentum at the perigee.
     *
     *  getMomentum().Phi() == getPhi0() holds.
     */
    double getPhi0() const { return m_phi0; }

    /** Getter for the cosine of the azimuth angle of travel direction at the perigee. */
    double getCosPhi0() const { return std::cos(getPhi0()); }

    /** Getter for the cosine of the azimuth angle of travel direction at the perigee. */
    double getSinPhi0() const { return std::sin(getPhi0()); }

    /** Getter for omega, which is a signed curvature measure of the track. The sign is equivalent to the charge of the particle. */
    double getOmega() const { return m_omega; }

    /** Getter for z0, which is the z coordinate of the perigee. */
    double getZ0() const { return m_z0; }

    /** Getter for tan lambda, which is the z over two dimensional arc length slope of the track. */
    double getTanLambda() const { return m_tanLambda; }

    /** Getter for cot theta, which is the z over two dimensional arc length slope of the track. Synomym to tan lambda. */
    double getCotTheta() const { return m_tanLambda; }
    /// @}

    ///--------------------------------------------------------------------------------------------------------------------------
  private:
    //---------------------------------------------------------------------------------------------------------------------------
    //--- Functions for internal conversions between cartesian and perigee helix parameters
    //--- This can be placed in a seperate header which handles all the conversion stuff
    //---------------------------------------------------------------------------------------------------------------------------

    /** Cartesian to Perigee conversion.
     */
    void setCartesian(const TVector3& position,
                      const TVector3& momentum,
                      const short int charge,
                      const double bZ);

    /** Memory for the signed distance to the perigee. The sign is the same as of the z component of getPerigee().Cross(getMomentum()).*/
    Double32_t m_d0;

    /** Memory for the azimuth angle between the transverse momentum and the x axis, which is in [-pi, pi]. */
    Double32_t m_phi0;

    /** Memory for the curvature of the signed curvature*/
    Double32_t m_omega;

    /** Memory for the z coordinate of the perigee. */
    Double32_t m_z0;

    /** Memory for the slope of the track in the z coordinate over the two dimensional arc length (dz/ds)*/
    Double32_t m_tanLambda;

    ClassDef(Helix, 2); /**< This class represents an ideal helix in perigee parameterization. */
  };
}

