/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Markus Prim                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TMatrixD.h>
#include <TMatrixDSym.h>

#include <cstdlib>
#include <vector>

namespace Belle2 {

  /** This class represents an ideal helix in perigee parameterization.
   *  The used perigee parameters are:
   *  1) omega - the signed curvature of the track where the sign is given by the charge of the particle
   *  2) phi - the polar angle between the transverse momentum and the x axis which is in [-pi, pi]
   *  3) d0 - the signed distance to the perigee. The sign positive (negative) if the angle between
   *          the transverse momentum and perigee position is +pi/2 (-pi/2)
   *  4) tanLambda - the slope of the track in the sz plane (dz/ds)
   *  5) z0 - the distance of the perigee from the origin
   *
   *  Each point on the helix can adressed by the arc length s, which has to be traversed to get to it from the perigee.
   *  More precisely the arc length means the transverse part of the particles travel distance,
   *  hence the arc length of the circle in the xy projection.
   */
  class Helix : public RelationsObject {
  public:

    /** Constructor initializing all perigee parameters to zero. */
    Helix();

    /** Constructor initializing class with fit result.
     *
     *  This is the only way to set the values of the Helix.
     *  We don't have any setters, as we assume, that once we create the MDST object, we don't want
     *  to change the values of the tracks any more.
     *  Scaling can be applied during readout, by setting the value for the magnetic field.
     *  @param position      Position of the track at the perigee.
     *  @param momentum      Momentum of the track at the perigee.
     *  @param charge        Charge of the particle.
     *  @param bField        Magnetic field to be used for the calculation of the curvature;
                             It is assumed, that the B-field is parallel to the z-Axis.
     */
    Helix(const TVector3& position,
          const TVector3& momentum,
          const short int charge,
          const float bField);

    /** Constructor initializing class with perigee parameters.
     *
     *  @param omega         The signed curvature of the track where the sign is given by the charge of the particle
     *  @param phi0          The angle between the transverse momentum and the x axis and in [-pi, pi]
     *  @param d0            The signed distance from origin to the perigee.
     *                       The sign is positive (negative) if the angle between the transverse momentum and d0 is +pi/2 (-pi/2)
     *  @param tanLambda     the slope of the track in the sz plane (dz/ds)
     *  @param z0            The z coordinate of the perigee.
     */
    Helix(const float& omega,
          const float& phi0,
          const float& d0,
          const float& tanLambda,
          const float& z0);

    /** Output operator for debugging and the generation of unittest error messages.*/
    friend std::ostream& operator<<(std::ostream& output, const Helix& helix);


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Getters for cartesian parameters of the perigee
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------

    /** Getter for the perigee position. */
    TVector3 getPosition() const;

    /** Getter for vector of momentum at the perigee position
     *
     *  As we calculate recalculate the momentum from a geometric helix, we need an estimate
     *  of the magnetic field along the z-axis to give back the momentum.
     *  @TODO This has a default value so that the basf2 compiles; default should be removed.
     *  @param bField  Magnetic field at the perigee.
     */
    TVector3 getMomentum(const float bField = 1.5) const;

    /** Getter for the absolute value of the transverse momentum at the perigee.
     *
     * @param bField Magnetic field at the perigee
     */
    float getTransverseMomentum(const float bField = 1.5) const;

    /** Return track charge (1 or -1).
     *
     *  @TODO For very small values, we might be unsure and return zero.
     */
    short getChargeSign() const {
      return getOmega() >= 0 ? 1 : -1;
    }
    /// @}


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Simple extrapolations of the ideal helix
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------


    /** Calculates the transverse travel distance at the point the helix first reaches the given polar radius.
     *
     *
     *  Gives the circle arc length in the forward direction that is traversed until a certain polar radius is reached
     *  Returns NAN, if the polar radius can not be reached, either because it is to far outside or inside of the perigee.
     *
     *  Forward the result to getPositionAtArcLength() or getMomentumAtArcLength() in order to extrapolate to the concentrical detector boundaries.
     *
     *  The result always has a positive sign. Hence it refers to the forward direction. Adding a minus sign yields the point at the same polar radius
     *  but in he backward direction.
     *
     *  @param polarR  The polar radius in question
     *  @return        The circle arc length traversed to reach the polar radius. NAN if it can not be reached.
     */
    float getArcLengthAtPolarR(const float& polarR) const;


    /** Calculates the position on the helix at the given arc length
     *
     *  @param arcLength       Transverse travel distance on the helix, which is the length of the circle arc as seen in the xy projection.
     */
    TVector3 getPositionAtArcLength(const float& arcLength) const;

    /** Calculates the unit tangential vector to the helix curve at the given circle arc length
     *
     *  @param arcLength       Transverse travel distance on the helix, which is the length of the circle arc as seen in the xy projection.
     */
    TVector3 getUnitTangentialAtArcLength(const float& arcLength) const;


    /** Calculates the momentum vector at the given arc length.
     *
     *  @param arcLength       Transverse travel distance on the helix, which is the length of the circle arc as seen in the xy projection.
     *  @param bz              Magnetic field strength in the z direction.
     */
    TVector3 getMomentumAtArcLength(const float& arcLength, const float& bz) const;


    /** Reverses the direction of travel of the helix in place.
     *
     *  The same points are located on the helix stay the same after the transformation,
     *  but have the opposite arc length.
     *  The momentum at each point is reversed.
     *  The charge sign is changed to its opposite by this transformation.
     */
    void reverse();

    /** Reverses a polar angle to the opposite direction
     *
     *  @param phi             A polar angle in [-pi, pi]
     *  @return                The polar angle for the opposite direction in [-pi, pi]
     */
    static float reversePhi(const float& phi) {
      return phi < 0 ? phi + M_PI : phi - M_PI;
    }

    /** Helper function to calculate the circle arc length from the length of a secant.
     *
     *  Translates the direct length between two point on the circle in the xy projection to the arc length on the circle
     *  Behaves smoothly in the limit of vanishing curvature.
     */
    double calcArcLengthFromSecantLength(const double& secantLength) const;
    /// @}


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Getters for perigee helix parameters
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------

    /** Getter for omega. This is the curvature of the track. It's sign is defined by the charge of the particle. */
    float getOmega() const { return m_omega; }

    /** Getter for phi0. This is the angle of the transverse momentum in the r-phi plane.
     *
     *  getMomentum().Phi() == getPhi0() holds.
     */
    float getPhi0() const { return m_phi0; }

    /** Getter for d0. This is the signed distance to the perigee in the r-phi plane. */
    float getD0() const { return m_d0; }

    /** Getter for tanLambda. This is the slope of the track in the r-z plane. */
    float getTanLambda() const { return m_tanLambda; }

    /** Getter for z0. This is the z coordinate of the perigee. */
    float getZ0() const { return m_z0; }
    /// @}



    ///--------------------------------------------------------------------------------------------------------------------------
  private:
    /** Calculates the alpha value for a given magnetic field in Tesla */
    double getAlpha(const float bField) const;

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Functions for internal conversions between cartesian and perigee helix parameters
    //--- This can be placed in a seperate header which handles all the conversion stuff
    //---------------------------------------------------------------------------------------------------------------------------
    /**
     * Calculates the signed distance between the origin and the perigee in the r-phi plane. The sign is positive (negative) if
     * the angle between the perigee vector (d0) and the transverse momentum is +(-) pi/2.
     * @param position in cartesian coordinates
     * @param momentum in cartesian coordinates
     * @return signed absolute value of d0
     */
    double calcD0FromCartesian(const TVector3& position, const TVector3& momentum) const;

    /**
     * Calculates the angle of the transverse momentum in the r-phi plane.
     * @param momentum in cartesian coordinates
     * @return phi in [-pi, [pi]
     */
    double calcPhiFromCartesian(const TVector3& momentum) const;

    /**
     * Calculates the signed curvature of the track.
     * @param momentum in cartesian coordinates
     * @param charge of the particle
     * @param bField at the perigee point
     * @return q/omega
     */
    double calcOmegaFromCartesian(const TVector3& momentum, const short int charge, const float bField) const;

    /**
     * Calculates the z component of the perigee vector.
     * @param position in cartesian coordinates
     * @return z0
     */
    double calcZ0FromCartesian(const TVector3& position) const;

    /**
     * Calculates the inverse slope of the track in the r-z plane.
     * @param momentum in cartesian coordinates
     * @return tanLambda = pz/pt
     */
    double calcTanLambdaFromCartesian(const TVector3& momentum) const;

    /**
     * Calculates the x coordinates of the perigee point with the internal stored parameters.
     * @return
     */
    double calcXFromPerigee() const;

    /**
     * Calculates the y coordinates of the perigee point with the internal stored parameters.
     * @return
     */
    double calcYFromPerigee() const;

    /**
     * Calculates the z coordinates of the perigee point with the internal stored parameters.
     * @return
     */
    double calcZFromPerigee() const;

    /**
     * Calculates the x momentum of the particle at the perigee point from the internal stored parameters.
     * @param bField: correction factor if different from original bField
     * @return
     */
    double calcPxFromPerigee(const float bField) const;

    /**
     * Calculates the y momentum of the particle at the perigee point from the internal stored parameters.
     * @param bField: correction factor if different from original bField
     * @return
     */
    double calcPyFromPerigee(const float bField) const;

    /**
     * Calculates the z momentum of the particle at the perigee point from the internal stored parameters.
     * @param bField: correction factor if different from original bField
     * @return
     */
    double calcPzFromPerigee(const float bField) const;

    /** Cartesian to Perigee conversion.
     *
     *  Everything happens internally, m_tau will be set and cartesian values dropped
     */
    void cartesianToPerigee(const TVector3& position,
                            const TVector3& momentum,
                            const short int charge,
                            const float bField);


    /** Memory for the curvature of the signed curvature*/
    float m_omega;

    /** Memory for the polar angle between the transverse momentum and the x axis which is in [-pi, pi]. */
    float m_phi0;

    /** Memory for the signed distance to the perigee. The sign positive (negative).*/
    float m_d0;

    /** Memory for the slope of the track in the sz plane (dz/ds)*/
    float m_tanLambda;

    /** Memory for the z coordinate of the perigee. */
    float m_z0;

    /** Streamer version 1. */
    ClassDef(Helix, 1);
  };
}

