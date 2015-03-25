/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013, 2014, 2015 - Belle II Collaboration                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Markus Prim, Tobias Schlüter                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/dataobjects/Helix.h>
#include <framework/dataobjects/UncertainHelix.h>

#include <TVector3.h>
#include <TMatrixD.h>
#include <TMatrixDSym.h>

#include <stdint.h>
#include <vector>

namespace Belle2 {

  /** Values of the result of a track fit with a given particle hypothesis.
   *  The values are stored as perigee parameters with a corresponding covariance matrix.
   *  The used perigee parameters are:
   *  1) d0 which is the signed distance to the perigee. Its sign is positive (negative)
   *     if the angle between the transverse momentum and d0 is +pi/2 (-pi/2)
   *  2) phi0 which is the angle between the transverse momentum and the x axis and in [-pi, pi]
   *  3) the signed curvature of the track where the sign is given by the charge of the particle
   *  4) z0 which is the distance of the perigee from the origin in the r-z plane
   *  5) tanLambda which is the inverse slope of the track in the r-z plane
   *
   *  The class is able to return the track fit result as either perigee parameters or
   *  cartesian parameters (x,y,z,px,py,pz)
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
     *  @param position      Position of the track near the perigee.
     *  @param momentum      Momentum of the track near the perigee.
     *  @param covariance    Covariance matrix for position and momentum given.
     *  @param charge        Charge of the particle.
     *  @param particleType  Particle Type used for mass hypothesis of the fit.
     *  @param pValue        p-value of the corresponding track fit.
     *  @param bField        Magnetic field to be used for the calculation of the curvature;
                             It is assumed, that the B-field is parallel to the z-Axis.
     */
    TrackFitResult(const TVector3& position, const TVector3& momentum,
                   const TMatrixDSym& covariance, const short int charge,
                   const Const::ParticleType& particleType, const float pValue,
                   const float bField,
                   const uint64_t hitPatternCDCInitializer,
                   const uint32_t hitPatternVXDInitializer);

    /** Constructor initializing class with perigee parameters.
     *
     *  This constructor is needed for testing the class.
     *  @param tau           Helix parameters of the track; @sa m_tau .
     *  @param cov5          Covariance matrix of the helix paramters of the track; @sa m_cov5
     *  @param particleType  Particle Type used for the mass hypothesis of the fit.
     *  @param pValue        p-value of the corresponding track fit.
     */
    TrackFitResult(const std::vector<float>& tau, const std::vector<float>& cov5,
                   const Const::ParticleType& particleType, const float pValue,
                   const uint64_t hitPatternCDCInitializer,
                   const uint32_t hitPatternVXDInitializer);

    /** Getter for vector of position at closest approach of track in r/phi projection. */
    TVector3 getPosition() const { return getHelix().getPerigee(); }

    /** Getter for vector of momentum at closest approach of track in r/phi projection.
     *
     *  As we calculate recalculate the momentum from a geometric helix, we need an estimate
     *  of the magnetic field along the z-axis to give back the momentum.
     *  @TODO This has a default value so that the basf2 compiles; default should be removed.
     *  @param bField  Magnetic field at the perigee.
     */
    TVector3 getMomentum(const float bField = 1.5) const
    { return getHelix().getMomentum(bField); }

    /** Getter for the absolute value of the transverse momentum at the perigee.
     *
     * @param bField Magnetic field at the perigee
     */
    Double32_t getTransverseMomentum(const float bField = 1.5) const
    { return getHelix().getTransverseMomentum(bField); }

    /** Position and Momentum Covariance Matrix.  */
    TMatrixDSym getCovariance6(const float bField = 1.5) const
    { return getUncertainHelix().getCartesianCovariance(bField); }

    /** Getter for ParticleCode of the mass hypothesis of the track fit. */
    Const::ParticleType getParticleType() const { return Const::ParticleType(m_pdg); }

    /** Return track charge (1 or -1).
     *
     *  @TODO For very small values, we might be unsure and return zero.
     */
    short getChargeSign() const { return getOmega() >= 0 ? 1 : -1; }

    /** Getter for Chi2 Probability of the track fit. */
    Double32_t getPValue() const { return m_pValue; }

    //------------------------------------------------------------------------
    // --- Getters for perigee helix parameters
    //------------------------------------------------------------------------
    /** Getter for d0. This is the signed distance to the IP in the r-phi plane.
     *
     *  @return
     */
    Double32_t getD0() const { return m_tau[iD0]; }

    /** Getter for phi0. This is the angle of the transverse momentum in the r-phi plane.
     *
     *  @return
     */
    Double32_t getPhi0() const { return m_tau[iPhi0]; }

    /** Getter for phi0 with CDF naming convention.  */
    Double32_t getPhi() const { return getPhi0(); }

    /** Getter for omega. This is the curvature of the track. It's
     * sign is defined by the charge of the particle.
     *
     *  @return
     */
    Double32_t getOmega() const { return m_tau[iOmega]; }

    /** Getter for z0. This is the z coordinate of the perigee.
     *
     *  @return
     */
    Double32_t getZ0() const { return m_tau[iZ0]; }

    /** Getter for tanLambda. This is the slope of the track in the r-z plane.
     *
     *  @return
     */
    Double32_t getTanLambda() const { return m_tau[iTanLambda]; }

    /** Getter for tanLambda with CDF naming convention.  */
    Double32_t getCotTheta() const { return getTanLambda(); }

    /** Getter for all perigee parameters
     *
     *  @return vector with 5 elements
     */
    std::vector<float> getTau() const { return std::vector<float>(m_tau, m_tau + c_NPars); }

    /** Getter for all covariance matrix elements of perigee parameters
     *
     *  @return vector with 15 elements
     */
    std::vector<float> getCov() const { return std::vector<float>(m_cov5, m_cov5 + c_NCovEntries); }

    /** Getter for covariance matrix of perigee parameters in matrix form.
     *
     *  @return
     */
    TMatrixDSym getCovariance5() const;

    /** Conversion to framework Helix (without covariance).  */
    Helix getHelix() const
    { return Helix(getD0(), getPhi0(), getOmega(), getZ0(), getTanLambda()); }

    /** Conversion to framework Uncertain Helix (i.e., with covariance).  */
    UncertainHelix getUncertainHelix() const
    {
      return UncertainHelix(getD0(), getPhi0(), getOmega(), getZ0(),
                            getTanLambda(), getCovariance5(), getPValue());
    }

    /** Getter for the hit pattern in the CDC; @sa HitPatternCDC */
    HitPatternCDC getHitPatternCDC() const
    {
      return HitPatternCDC(m_hitPatternCDCInitializer);
    }

    /** Getter for the hit pattern in the VXD; @sa HitPatternVXD */
    HitPatternVXD getHitPatternVXD() const
    {
      return HitPatternVXD(m_hitPatternVXDInitializer);
    }

    ///--------------------------------------------------------------------------------------------------------------------------
  private:

    //---------------------------------------------------------------------------------------------------------------------------
    /** PDG Code for hypothesis with which the corresponding fit was performed. */
    const unsigned int m_pdg;

    /** Chi2 Probability of the fit. */
    const Double32_t m_pValue;

    /** A helix has five parameters and the covariance matrix is a
     *  symmetric five-by-five matrix.  */
    static const unsigned int c_NPars = 5;
    static const unsigned int c_NCovEntries = 5 * 6 / 2;

    /** Names for the parameter indices.  */
    static const unsigned int iD0 = 0;
    static const unsigned int iPhi0 = 1;
    static const unsigned int iOmega = 2;
    static const unsigned int iZ0 = 3;
    static const unsigned int iTanLambda = 4;

    /** perigee helix parameters; tau = d0, phi0, omega, z0, tanLambda. */
    Double32_t m_tau[c_NPars];

    /** The 15 = 5*(5+1)/2 covariance matrix elements.
     *
     *  (0,0), (0,1) ... (1,1), (1,2) ... (2,2) ...
     */
    Double32_t m_cov5[c_NCovEntries];

    /** Member for initializing the information about hits in the CDC.
     *
     *  @sa HitPatternCDC
     */
    uint64_t m_hitPatternCDCInitializer;
    /** Member for initializing the information about hits in the VXD.
     *
     *  @sa HitPatternVXD
     */
    const uint32_t m_hitPatternVXDInitializer;

    /** Streamer version. */
    ClassDef(TrackFitResult, 6);
    /* Version history:
       ver 6: use fixed size arrays instead of vectors (add schema evolution rule), use Double32_t.
       ver 5: CDC Hit Pattern now a single variable (add schema evolution rule).
    */
  };
}
