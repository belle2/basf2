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
   *  1) d0 which is the signed distance to the perigee. The sign positive (negative) if the angle between
   *    the transverse momentum and perigee position is +pi/2 (-pi/2)
   *  2) phi which is the angle between the transverse momentum and the x axis and in [-pi, pi]
   *  3) the signed curvature of the track where the sign is given by the charge of the particle
   *  4) z0 which is the distance of the perigee from the origin in the r-z plane
   *  5) cotTheta which is the inverse slope of the track in the r-z plane
   *
   *
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
     *  @param d0            The signed distance from origin to the perigee.
     *                       The sign is positive (negative) if the angle between the transverse momentum and d0 is +pi/2 (-pi/2)
     *  @param phi           The angle between the transverse momentum and the x axis and in [-pi, pi]
     *  @param omega         The signed curvature of the track where the sign is given by the charge of the particle
     *  @param z0            The z coordinate of the perigee.
     *  @param cotTheta      The inverse slope of the track in the r-z plane,
                             where r significes the transverse travel distance as seen in the xy projection.
     */
    Helix(const float& d0,
          const float& phi,
          const float& omega,
          const float& z0,
          const float& cotTheta);



    /** Constructor initializing class with perigee parameters.
     *  @param tau           A vector of length 5 containing the perigee coordinates in the order d0, phi, omega, z0, cotTheta.
     *
     *  @sa Helix(const float& d0, const float& phi, const float& omega, const float& z0, const float& cotTheta)
     *  for a describtion of the individual parameters.
     */
    Helix(const std::vector<float>& tau);



    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Getters for cartesian parameters of the perigee
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------

    /** Getter for vector of position at closest approach of track in r/phi projection. */
    TVector3 getPosition() const;

    /** Getter for vector of momentum at closest approach of track in r/phi projection.
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


    /** Calculates the position on the helix that lies at the given transverse travel distance.

     *  @param arcS          Transverse travel distance on the helix, which is the length of the circle arc as seen in the xy projection.
     */
    TVector3 getPositionAtS(const float& arcS) const;

    /** Reverses the direction of travel of the helix in place. */
    void reverse();

    /** Calculates the transverse travel distance at the point the helix first reaches the given polar radius.
     */
    float getSAtPolarR(const float& polarR) const;

    /** Sinus cardinalis function, which is the real part of (exp(ix) - 1) / x being sin(x) / x
     *
     *  @note The current implementation just uses the boost version of this function.
     */
    static double sinc(const double& x);

    /** Cosinus cardinalis function, which is the imaginary part of (exp(ix) - 1) / x being (cos(x) - 1)  /x
     *
     *  @note The implementation is inspired by the sinc_pi of boost.
     */
    static double cosc(const double& x);

    /// Helper function the calculate the factor between the length of a secant line and the transverse travel distance
    /**
     *  Function expressing the relation between arc length and direct length
     *  only using the omega as additional information.
     *  Handles to line case smoothly.
     *
     */


    double calcSFromSecantLength(const double& secantLength) const;
    /// @}




    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Getters for perigee helix parameters
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------

    /** Getter for d0. This is the signed distance to the POCA in the r-phi plane.
     *
     *  @return
     */
    float getD0() const { return m_tau.at(0); }

    /** Getter for phi. This is the angle of the transverse momentum in the r-phi plane.
     *
     *  @return
     */
    float getPhi() const { return m_tau.at(1); }

    /** Getter for omega. This is the curvature of the track. It's sign is defined by the charge of the particle.
     *
     *  @return
     */
    float getOmega() const { return m_tau.at(2); }

    /** Getter for z0. This is the z coordinate of the POCA.
     *
     *  @return
     */
    float getZ0() const { return m_tau.at(3); }

    /** Getter for cotTheta. This is the slope of the track in the r-z plane.
     *
     *  @return
     */
    float getCotTheta() const { return m_tau.at(4); }

    /** Getter for all perigee parameters
     *
     *  @return vector with 5 elements
     */
    std::vector<float> getTau() const { return m_tau; }

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
     * @return cotTheta = pz/pt
     */
    double calcCotThetaFromCartesian(const TVector3& momentum) const;

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
     *  Everything happens internally, m_tau and m_cov5 will be set and cartesian values dropped
     */
    void cartesianToPerigee(const TVector3& position,
                            const TVector3& momentum,
                            const short int charge,
                            const float bField);

    /** perigee helix parameters; tau = d0, phi, omega, z0, cotTheta. */
    std::vector<float> m_tau;

    /** Streamer version 1. */
    ClassDef(Helix, 1);
  };
}

