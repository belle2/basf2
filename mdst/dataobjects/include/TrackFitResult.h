/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/dataobjects/Helix.h>
#include <framework/dataobjects/UncertainHelix.h>
#include <framework/geometry/BFieldManager.h>

#include <TVector3.h>
#include <TMatrixDSym.h>
#include <Math/Vector4D.h>

#include <stdint.h>
#include <vector>
#include <cmath>

namespace Belle2 {
  class HitPatternCDC;
  class HitPatternVXD;

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
   *  These are the parameters used by BaBar, see D.N. Brown et al., The BaBar
   *  Track Fitting Algorithm, appendix A, eq. (15).
   *  Available at http://rhicii-science.bnl.gov/public/comp/reco/babar.ps
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
     *  @param hitPatternCDCInitializer  bits for initializing CDC hit pattern.
     *  @param hitPatternVXDInitializer  bits for initializing VXD hit pattern.
     *  @param NDF  number of degrees of freedom for the fit
     */
    TrackFitResult(const TVector3& position, const TVector3& momentum,
                   const TMatrixDSym& covariance, const short int charge,
                   const Const::ParticleType& particleType, const float pValue,
                   const float bField,
                   const uint64_t hitPatternCDCInitializer,
                   const uint32_t hitPatternVXDInitializer,
                   const uint16_t NDF);

    /** Constructor initializing class with perigee parameters.
     *
     *  This constructor is needed for testing the class.
     *  @param tau           Helix parameters of the track; @sa m_tau .
     *  @param cov5          Covariance matrix of the helix paramters of the track; @sa m_cov5
     *  @param particleType  Particle Type used for the mass hypothesis of the fit.
     *  @param pValue        p-value of the corresponding track fit.
     *  @param hitPatternCDCInitializer  bits for initializing CDC hit pattern.
     *  @param hitPatternVXDInitializer  bits for initializing VXD hit pattern.
     *  @param NDF  number of degrees of freedom for the fit
     */
    TrackFitResult(const std::vector<float>& tau, const std::vector<float>& cov5,
                   const Const::ParticleType& particleType, const float pValue,
                   const uint64_t hitPatternCDCInitializer,
                   const uint32_t hitPatternVXDInitializer,
                   const uint16_t NDF
                  );

    /** Getter for vector of position at closest approach of track in r/phi projection. */
    TVector3 getPosition() const { return getHelix().getPerigee(); }

    /** Getter for vector of momentum at closest approach of track in r/phi projection.
     *
     *  As we calculate recalculate the momentum from a geometric helix, we need an estimate
     *  of the magnetic field along the z-axis to give back the momentum.
     */
    TVector3 getMomentum() const
    {
      const double bField = BFieldManager::getField(getPosition()).Z() / Unit::T;
      return getHelix().getMomentum(bField);
    }

    /** Getter for the 4Momentum at the closest approach of the track in the r/phi projection.
     * P = (px, py, pz, E) where E is calculated via the momentum and the particle hypothesis of the TrackFitResult.
     */
    ROOT::Math::PxPyPzEVector get4Momentum() const
    {
      const B2Vector3D momentum = getMomentum();
      return ROOT::Math::PxPyPzEVector(momentum.x(), momentum.y(), momentum.z(), getEnergy());
    }

    /** Getter for the Energy at the closest approach of the track in the r/phi projection.
     * E is calculated via the momentum and the particle hypothesis of the TrackFitResult.
     */
    double getEnergy() const
    {
      return std::sqrt(getMomentum().Mag2() + getParticleType().getMass() * getParticleType().getMass());
    }

    /** Getter for the absolute value of the transverse momentum at the perigee.
     */
    double getTransverseMomentum() const
    {
      const double bField = BFieldManager::getField(getPosition()).Z() / Unit::T;
      return getHelix().getTransverseMomentum(bField);
    }

    /** Position and Momentum Covariance Matrix.  */
    TMatrixDSym getCovariance6() const
    {
      const double bField = BFieldManager::getField(getPosition()).Z() / Unit::T;
      return getUncertainHelix().getCartesianCovariance(bField);
    }

    /** Getter for ParticleType of the mass hypothesis of the track fit. */
    Const::ParticleType getParticleType() const { return Const::ParticleType(m_pdg); }

    /** Return track charge (1 or -1).
     *
     *  @todo For very small values, we might be unsure and return zero.
     */
    short getChargeSign() const { return (getOmega() > 0) - (getOmega() < 0); }

    /** Getter for Chi2 Probability of the track fit. */
    double getPValue() const { return m_pValue; }

    /** Getter for number of degrees of freedom of the track fit. */
    int getNDF() const;

    /** Get chi2 given NDF and p-value */
    double getChi2() const;

    //------------------------------------------------------------------------
    // --- Getters for perigee helix parameters
    //------------------------------------------------------------------------
    /** Getter for d0. This is the signed distance to the IP in the r-phi plane.
     *
     *  @return
     */
    double getD0() const { return m_tau[iD0]; }

    /** Getter for phi0. This is the angle of the transverse momentum in the r-phi plane.
     *
     *  @return
     */
    double getPhi0() const { return m_tau[iPhi0]; }

    /** Getter for phi0 with CDF naming convention.  */
    double getPhi() const { return getPhi0(); }

    /** Getter for omega. This is the curvature of the track. It's
     * sign is defined by the charge of the particle.
     *
     *  @return
     */
    double getOmega() const { return m_tau[iOmega]; }

    /** Getter for z0. This is the z coordinate of the perigee.
     *
     *  @return
     */
    double getZ0() const { return m_tau[iZ0]; }

    /** Getter for tanLambda. This is the slope of the track in the r-z plane.
     *
     *  @return
     */
    double getTanLambda() const { return m_tau[iTanLambda]; }

    /** Getter for tanLambda with CDF naming convention.  */
    double getCotTheta() const { return getTanLambda(); }

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
     * The order is given by TFRParName: d0, phi0, omega, z0, lambda.
     */
    TMatrixDSym getCovariance5() const;

    /** Conversion to framework Helix (without covariance).  */
    Helix getHelix() const
    { return Helix(getD0(), getPhi0(), getOmega(), getZ0(), getTanLambda()); }

    /** Conversion to framework Helix with momentum scaling (without covariance).  */
    Helix getHelix(float momentumScale) const
    { return Helix(getD0(), getPhi0(), getOmega() / momentumScale, getZ0(), getTanLambda()); }

    /** Conversion to framework Uncertain Helix (i.e., with covariance).  */
    UncertainHelix getUncertainHelix() const
    {
      return UncertainHelix(getD0(), getPhi0(), getOmega(), getZ0(),
                            getTanLambda(), getCovariance5(), getPValue());
    }

    /** Getter for the hit pattern in the CDC; @sa HitPatternCDC */
    HitPatternCDC getHitPatternCDC() const;

    /** Getter for the hit pattern in the VXD; @sa HitPatternVXD */
    HitPatternVXD getHitPatternVXD() const;

    /** Return a short summary of this object's contents in HTML format. */
    virtual std::string getInfoHTML() const override;

    ///--------------------------------------------------------------------------------------------------------------------------
  private:

    //---------------------------------------------------------------------------------------------------------------------------
    /** PDG Code for hypothesis with which the corresponding fit was performed. */
    const unsigned int m_pdg;

    /** Chi2 Probability of the fit. */
    const Double32_t m_pValue;

    /** \name TFRStorageSizes
     *  Constants for Storage sizes */
    /**@{
     *  A helix has five parameters and the covariance matrix is a
     *  symmetric five-by-five matrix.  */
    static const unsigned int c_NPars = 5; ///< Number of helix parameters
    static const unsigned int c_NCovEntries = 5 * 6 / 2; ///< Number of covariance entries
    /**@}*/

    /** \name TFRParName
     *  Names for the parameter indices into m_tau.  */
    /**@{ */
    static const unsigned int iD0 = 0;        ///< Index for d0
    static const unsigned int iPhi0 = 1;      ///< Index for phi0
    static const unsigned int iOmega = 2;     ///< Index for omega
    static const unsigned int iZ0 = 3;        ///< Index for z0
    static const unsigned int iTanLambda = 4; ///< Index tan lambda
    /**@}*/

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

    /** backward compatibility initialisation for NDF */
    static const uint16_t c_NDFFlag = 0xFFFF;

    /** Memeber for number of degrees of freedom*/
    uint16_t m_NDF;

    ClassDefOverride(TrackFitResult, 8); /**< Values of the result of a track fit with a given particle hypothesis. */
    /* Version history:
       ver 8: add NDF
       ver 7: fixed sign errors in the translation of position and momentum covariances.
       ver 6: use fixed size arrays instead of vectors (add schema evolution rule), use Double32_t.
       ver 5: CDC Hit Pattern now a single variable (add schema evolution rule).
       ver 4: added hit pattern
       ver 3: back to vectors for the elements
       ver 4: fixed-size arrays for the elements  <------- incompatible with later version 4
       ver 3: add data fields for hit patterns
       ver 2: re-arranging of members
       ver 1:
       ver 2:                                     <------- incompatible with later version 2
       ver 1:
    */
  };
}
