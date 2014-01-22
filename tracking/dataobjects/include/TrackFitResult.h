/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>
#include <TVector3.h>
#include <TMatrixF.h>
#include <cstdlib>
#include <vector>

//class GFAbsTrackRep;
//#include cmath

//#include <bitset>

namespace Belle2 {

  /** Values of the result of a track fit with a given particle hypothesis.
   */
  class TrackFitResult : public RelationsObject {
  public:
    /** Constructor for I/O purpuses.
     */
    TrackFitResult();

    /** Constructor taking position and momentum vector + charge, pdg and pValue of the particle.
     * The cartesian coordinates will immediately be transformed into perigee helix parameters and stored as such.
     * Until perigee parametrization works reliable one can switch to store the cartesian coordinates
     */
    TrackFitResult(const TVector3& position, const TVector3& momentum, const short int& charge, const TMatrixF& covariance,
                   const unsigned int& pdg, const float& pValue, bool storeAsPerigee = false);

    /** Constructor taking an object used during the fitting process.
     * @TODO Add constructor taking the object
     */
    //TrackFitResult(const GFAbsTrackRep& gfAbsTrackRep);

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Deprecated setters
    //---------------------------------------------------------------------------------------------------------------------------
    /** Setter for position of the poca. This has no functionality. */
    void setPosition(const TVector3& position) {
      B2WARNING("setPosition is deprecated. Set member variables with constructor call instead.");
    }
    /** Setter for momentum at the poca. This has no functionality. */
    void setMomentum(const TVector3& momentum) {
      B2WARNING("setMomentum is deprecated. Set member variables with constructor call instead.");
    }
    /** Position and Momentum Covariance Matrix.
     *
     *  This is a copy from the genfit::Track getPosMomCov matrix (implicating the order of the matrix),
     *  however, it uses just floating point precision rather than double.
     *  As well currently no TMatrixSym is used (which might change, but doesn't matter much due to the misconstruction of TMatrixSym).
     *  @TODO Study if double precision matrix is needed and if TMatrixSym helps somewhere.
     */
    /** Setter for cartesian covariance matrix. This has no functionality. */
    void setCovariance6(const TMatrixF& covariance) {
      B2WARNING("setCovariance6 is deprecated. Set member variables with constructor call instead.");
    }
    /* Setter for the PDGCode. This has no functionality. */
    void setParticleType(const Const::ParticleType& pType) {
      B2WARNING("setParticleType is deprecated. Set member variables with constructor call instead.");
      //m_pdg = std::abs(pType.getPDGCode());
    }
    /** Setter for Charge. This has no functionality. */
    void setCharge(int charge) {
      B2WARNING("setCharge is deprecated. Set member variables with constructor call instead.");
      //m_charge = (charge < 0) ? -1 : 1;
    }
    /** Setter for Chi2 Probability of the track fit. This has no functionality. */
    void setPValue(float pValue) {
      B2WARNING("setPValue is deprecated. Set member variables with constructor call instead.");
      //m_pValue = pValue;
    }

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Getters
    //---------------------------------------------------------------------------------------------------------------------------
    /** Getter for Chi2 Probability of the track fit. */
    float getPValue() const {
      return m_pValue;
    }
    /** Getter for the charge of the particle.
     * Return track charge (1 or -1).
     * */
    short getCharge() const {
      if (m_storeAsPerigee == true) {
        if (getOmega() >= 0)
          return 1;
        else
          return -1;
      } else {
        return m_charge;
      }

    }
    /** Get back a ParticleCode of the hypothesis of the track fit. */
    Const::ParticleType getParticleType() const {
      return Const::ParticleType(m_pdg);
    }

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Getters for the perigee helix parameters
    //---------------------------------------------------------------------------------------------------------------------------
    /** Getter for helix parameter d0 */
    float getD0() const;
    /** Getter for helix parameter phi */
    float getPhi0() const;
    /** Getter for helix parameter omega */
    float getOmega() const;
    /** Getter for helix parameter z0, this is also the cartesian value of z0 at the poca */
    float getZ0() const;
    /** Getter for helix parameter cotTheta */
    float getCotTheta() const;
    /** Getter for the helix parameter covariance matrix */
    TMatrixF getCovariance5() const;

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Getters for the cartesian parameters
    //---------------------------------------------------------------------------------------------------------------------------
    /** Getter for the x value of the poca */
    float getX0() const;
    /** Getter for the y value of the poca */
    float getY0() const;
    /** Getter for the px value at the poca */
    float getPx0() const;
    /** Getter for the py value at the poca */
    float getPy0() const;
    /** Getter for the pz value at the poca */
    float getPz0() const;
    /** Getter for the poca vector */
    TVector3 getPosition() const;
    /** Getter for the momentum vector at the poca */
    TVector3 getMomentum() const;
    /** Getter for the cartesian covariance matrix */
    TMatrixF getCovariance6() const;


    //---------------------------------------------------------------------------------------------------------------------------
    //--- Hit Pattern Arithmetics
    //---------------------------------------------------------------------------------------------------------------------------
    /** Number of PXD hits used in the TrackFitResult. */
    /*    unsigned short getNPXDHits() const {
          return (m_hitPattern[0] + m_hitPattern[1]);
        }
    */
    /** Number of SVD hits used in the TrackFitResult. */
    /*    unsigned short getNSVDHits() const {
          return (m_hitPattern[2] + m_hitPattern[3] + m_hitPattern[4] + m_hitPattern[5]);
        }
    */
    /** Number of CDC hits used in the TrackFitResult. */
    /*    unsigned short getNCDCHits() const {
          return (m_hitPattern.count() - getNPXDHits() - getNSVDHits());
        }
    */
    /** Was there a hit in the specified layer?
     *
     *  @param  iVXDLayer  layer for which the information is requested.
     */
    /*    bool hitInVXDLayer(unsigned short iVXDLayer) const {
          return m_hitPattern[iVXDLayer];
        }
    */
    /** Was there a hit in the specified layer?
     *
     *  @param  iCDCLayer  layer for which the information is requested.
     */
    /*    bool hitInCDCLayer(unsigned short iCDCLayer) const {
          return m_hitPattern[iCDCLayer + 6];
        }
    */
    /** Similar as above, but asking for any hit in the corresponding SuperLayer.*/
//    bool hitInSuperLayer(unsigned int iSuperLayer) const;

    /** Were any Stereo hits in the CDC. */
    /*    bool hitInStereoLayer() const {
          return (hitInSuperLayer(2) || hitInSuperLayer(4) || hitInSuperLayer(6) || hitInSuperLayer(8));
        }
    */
    /** Returns the count of the innermost Layer. */
//    unsigned short getIInnermostLayer() const;

    ///--------------------------------------------------------------------------------------------------------------------------
  private:

    /* Keeps track how data is stored -- Temporary*/
    bool m_storeAsPerigee;
    /* Position of the poca -- Temporary */
    TVector3 m_position;
    /* Momentum at the poca -- Temporary */
    TVector3 m_momentum;
    /* Charge -- Temporary */
    short int m_charge;
    /* Cov6 -- Temporary */
    TMatrixF m_cov6;

    /** PDG Code for hypothesis with which the corresponding fir was performed. */
    unsigned int m_pdg;

    /** Chi2 Probability of the fit. */
    float m_pValue;

    ///--------------------------------------------------------------------------------------------------------------------------
    /** Hit Pattern of the corresponding Hit.
     *
     *  Bits 0-1:   PXD <br>
     *  Bits 2-5:   SVD <br>
     *  Bits 6-61:  CDC <br>
     *  Bits 62-63: unused.
     */
//    std::bitset<64> m_hitPattern;

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Internal Variables to store track parameters with uncertainties in perigee description of helix.
    //---------------------------------------------------------------------------------------------------------------------------
    /** Perigee Parameters are stored as:
     * Tau=(d0, phi0, omega, z0, cotTheta)
     * */
    std::vector<float> m_perigeeParams;

    /** Uncertainities of the perigee parameters are stored as:
     * [0]cov(d0, d0) [1]cov(d0, phi0)    [2]cov(d0, omega)     [3]cov(d0, z0)      [4]cov(d0, cotTheta)
     *                [5]cov(phi0, phi0)  [6]cov(phi0, omega)   [7]cov(phi0, z0)    [8]cov(phi0, cotTheta)
     *                                    [9]cov(omega, omega)  [10]cov(omega, z0)  [11]cov(omega, cotTheta)
     *                                                          [12]cov(z0, z0)     [13]cov(z0, cotTheta)
     *                                                                              [14]cov(cotTheta, cotTheta)
     */
    std::vector<float> m_perigeeUncertainties;


    /** Calculate helix parameters and covariance matrix from cartesian parameters and covariance matrix
     * Function arguments will come from the gfAbsTrackRep constructor and not be saved to further use
     * */
    void cartesianToHelix(const TVector3& position, const TVector3& momentum, const short int& charge);

    /**
     *
     */
    void cartesianCovToHelixCov(const TVector3& position, const TVector3& momentum, const short int& charge, const TMatrixF& covariance);

    ClassDef(TrackFitResult, 2);
  };
}
