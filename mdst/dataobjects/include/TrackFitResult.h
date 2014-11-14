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

//These two classes need to be moved to the MDST package as well on monday.
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TMatrixD.h>
#include <TMatrixDSym.h>

#include <cstdlib>
#include <vector>

namespace Belle2 {

  /** Values of the result of a track fit with a given particle hypothesis.
   *  The values are stored as perigee parameters with a corresponding covariance matrix.
   *  The used perigee parameters are:
   *  1) d0 which is the signed distance to there perigee. The sign positive (negative) if the angle between
   *    the transverse momentum and d0 is +pi/2 (-pi/2)
   *  2) phi which is the angle between the transverse momentum and the x axis and in [-pi, pi]
   *  3) the signed curvature of the track where the sign is given by the charge of the particle
   *  4) z0 which is the distance of the perigee from the origin in the r-z plane
   *  5) cotTheta which is the inverse slope of the track in the r-z plane
   *
   *  The class is able to return the track fit result as either perigee parameters or cartesian parameters (x,y,z,px,py,pz)
   */
  class TrackFitResult : public RelationsObject {
  public:
    /** Constructor initializing everything to zero. */
    TrackFitResult();

    /** Constructor initializing class with fit result.
     *
     *  This is the only way to set the values of the TrackFitResult.
     *  We don't have any setters, as we assume, that once we create the MDST object, we don't want
     *  to change the values of the tracks any more.
     *  Scaling can be applied during readout, by setting the value for the magnetic field.
     *  @param position      Position of the track at the perigee.
     *  @param momentum      Momentum of the track at the perigee.
     *  @param covariance    Covariance matrix for position and momentum of the track at the perigee.
     *  @param charge        Charge of the particle.
     *  @param particelType  Particle Type used for mass hypothesis of the fit.
     *  @param pValue        p-value of the fit.
     *  @param bField        Magnetic field to be used for the calculation of the curvature;
                             It is assumed, that the B-field is parallel to the z-Axis.
     */
    TrackFitResult(const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance,
                   const short int charge, const Const::ParticleType& particleType, const float pValue,
                   const float bField,
                   const unsigned long hitPatternCDCInitializer, const unsigned short hitPatternVXDInitializer);

    /** Constructor initializing class with perigee parameters.
     *
     *  This constructor is needed for testing the class.
     *  @param tau           Helix parameters of the track; @sa m_tau .
     *  @param cov5          Covariance matrix of the helix paramters of the track; @sa m_cov5
     *  @param particleType  Particle Type used for the mass hypothesis of the fit.
     *  @param pValue        p-value of the fit.
     */
    TrackFitResult(const std::vector<float>& tau, const std::vector<float>& cov5,
                   const Const::ParticleType& particleType, const float pValue,
                   const unsigned long hitPatternCDCInitializer, const unsigned short hitPatternVXDInitializer);

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

    /** Position and Momentum Covariance Matrix.
     *
     *  This is a copy from the genfit::Track getPosMomCov matrix (implicating the order of the matrix).
     *  However, it uses just floating point precision rather than double.
     *  As well currently no TMatrixSym is used (which might change, but doesn't matter much due to the misconstruction of TMatrixSym).
     *  @TODO Study if double precision matrix is needed and if TMatrixSym helps somewhere.
     */
    TMatrixDSym getCovariance6(const float bField = 1.5) const;

    /** Getter for ParticleCode of the mass hypothesis of the track fit. */
    Const::ParticleType getParticleType() const {
      return Const::ParticleType(m_pdg);
    }

    /** Return track charge (1 or -1).
     *
     *  @TODO For very small values, we might be unsure and return zero.
     */
    short getChargeSign() const {
      return getOmega() >= 0 ? 1 : -1;
    }

    /** Getter for Chi2 Probability of the track fit. */
    float getPValue() const {
      return m_pValue;
    }

    //---------------------------------------------------------------------------------------------------------------------------
    // --- Getters for perigee helix parameters
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

    /** Getter for all covariance matrix elements of perigee parameters
     *
     *  @return vector with 15 elements
     */
    std::vector<float> getCov() const { return m_cov5; }

    /** Getter for covariance matrix of perigee parameters in matrix form.
     *
     *  @return
     */
    TMatrixDSym getCovariance5() const;

    /** Getter for the hit pattern in the CDC; @sa HitPatternCDC */
    HitPatternCDC getHitPatternCDC()const {
      return HitPatternCDC(m_hitPatternCDCInitializer);
    }

    /** Getter for the hit pattern in the CDC; @sa HitPatternCDC */
    HitPatternVXD getHitPatternVXD()const {
      return HitPatternVXD(m_hitPatternVXDInitializer);
    }

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

    /**
     * Calculate the 6 dimensional covariance matrix from the internal 5 dimensional representation.
     * @param cov5
     * @param bField: correction factor if different from original bField
     * @return
     */
    TMatrixDSym transformCov5ToCov6(const TMatrixDSym& cov5, const float bField) const;

    /** Cartesian to Perigee conversion.
     *
     *  Everything happens internally, m_tau and m_cov5 will be set and cartesian values dropped
     */
    void cartesianToPerigee(const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance,
                            const short int charge, const float bField);

    //---------------------------------------------------------------------------------------------------------------------------
    /** PDG Code for hypothesis with which the corresponding fit was performed. */
    const unsigned int m_pdg;

    /** Chi2 Probability of the fit. */
    const float m_pValue;

    /** perigee helix parameters; tau = d0, phi, omega, z0, cotTheta. */
    std::vector<float> m_tau;

    /** covariance matrix elements.
     *
     *  (0,0), (0,1) ... (1,1), (1,2) ... (2,2) ...
     */
    std::vector<float> m_cov5;

    /** Member for initializing the information about hits in the CDC.
     *
     *  @sa HitPatternCDC
     */
    const unsigned long m_hitPatternCDCInitializer;

    /** Member for initializing the information about hits in the VXD.
     *
     *  @sa HitPatternVXD
     */
    const unsigned short m_hitPatternVXDInitializer;

    /** Streamer version 3. */
    ClassDef(TrackFitResult, 3);
  };
}

