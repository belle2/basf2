/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vishal     (ECL Software Group)                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCLUSTER_H
#define ECLCLUSTER_H

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMatrixFSym.h>

#include <cmath>

namespace Belle2 {

  /*! Class to store ECLCluster reconstructed from ECLDigit as Belle method
   * relation to ECLShower
   * filled in ecl/modules/eclRecShower/src/ECLReconstructorModule.cc
   */

  class ECLCluster : public RelationsObject {
  public:
    /**
     *default constructor : all values are set to 0
     */
    ECLCluster() :
      m_isTrack(false), m_energy(0), m_theta(0), m_phi(0), m_r(0),
      m_energyDepositSum(0), m_timing(0), m_errorTiming(0), m_E9oE25(0), m_highestE(0),
      m_LAT(0), m_numberOfCrystals(0), m_crystalHealth(0), m_mergedPi0(0)
    {
      m_error[0] = 0;
      m_error[1] = 0;
      m_error[2] = 0;
      m_error[3] = 0;
      m_error[4] = 0;
      m_error[5] = 0;
    };

    /*******************************
     *********** Setters ***********
     *******************************/

    /*! Set Corrected Energy (GeV) */
    void setEnergy(float Energy) { m_energy = Energy; }

    /*! Set Theta of Shower (radian) */
    void setTheta(float Theta) { m_theta = Theta; }

    /*! Set Phi of Shower (radian) */
    void setPhi(float Phi) { m_phi = Phi; }

    /*! Set R (in cm) */
    void setR(float R) { m_r = R; }

    /*! Set Uncorrect Energy deposited (GeV) */
    void setEnedepSum(float EnedepSum) { m_energyDepositSum = EnedepSum; }

    /*! Set Timing information */
    void setTiming(float Timing) { m_timing = Timing; }

    /*! Set Error on Timing information */
    void setErrorTiming(float ErrorTiming) { m_errorTiming = ErrorTiming; }

    /*! Set E9/E25 */
    void setE9oE25(float E9oE25) { m_E9oE25 = E9oE25; }

    /*! Set Highest Energy stored in a Crystal in a shower (GeV) */
    void setHighestE(float HighestE) { m_highestE = HighestE; }

    /*! Set Lateral distribution parameter */
    void setLAT(float LAT) { m_LAT = LAT; }

    /*! Set Number of Crystals in a shower */
    void setNofCrystals(int NofCrystals) { m_numberOfCrystals = NofCrystals; }

    /*! Set Crystal health (Not now) */
    void setCrystHealth(int CrystHealth) { m_crystalHealth = CrystHealth; }

    /*! Set high momentum pi0 likelihood */
    void setMergedPi0(float MergedPi0) { m_mergedPi0 = MergedPi0; }

    /*! Set  Error Array(3x3)  for
      [0]->Error on Energy
      [2]->Error on Phi
      [5]->Error on Theta
    */
    void setError(float ErrorArray[6])
    {
      for (int i = 0; i < 6; ++i) {
        m_error[i] = ErrorArray[i];
      }
    }

    /*! Set m_isTrack true if the cluster matches with cluster */
    void setisTrack(bool istrack) { m_isTrack = istrack; }

    /*******************************
     *********** Getters ***********
     *******************************/

    /*! Return Correct Energy (GeV) */
    float getEnergy() const { return m_energy; }

    /*! Return Theta of Shower (radian) */
    float getTheta() const { return m_theta; }

    /*! Return Phi of Shower (radian) */
    float getPhi() const { return m_phi; }

    /*! Return R */
    float getR() const { return m_r; }

    /*! Return Error on Energy */
    float getErrorEnergy() const { return m_error[0]; }

    /*! Return Error on Theta of Shower */
    float getErrorTheta() const { return m_error[5]; }

    /*! Return Error on Phi of Shower */
    float getErrorPhi() const { return m_error[2]; }

    /*! Return Uncorrect Energy deposited (GeV) */
    float getEnedepSum() const { return m_energyDepositSum; }

    /*! Return Timing information */
    float getTiming() const { return m_timing; }

    /*! Return Error on Timing information */
    float getErrorTiming() const { return m_errorTiming; }

    /*! Return E9/E25 */
    float getE9oE25() const { return m_E9oE25; }

    /*! Return HighestEnergy in a crystal in a shower */
    float getHighestE() const {return m_highestE; }

    /*! Return LAT (shower variable) */
    float getLAT() const { return m_LAT; }

    /*! Return Number of a Crystals in a shower */
    int getNofCrystals() const { return m_numberOfCrystals; }

    /*! Return Crystal health (Not now) */
    int getCrystHealth() const { return m_crystalHealth; }

    /*! Return high momentum pi0 likelihood */
    float getMergedPi0() const { return m_mergedPi0; }

    /*! Return Px (GeV/c) */
    float getPx() const
    {
      return float(m_energy * sin(m_theta) * cos(m_phi));
    }

    /*! Return Py (GeV/c) */
    float getPy() const
    {
      return float(m_energy * sin(m_theta) * sin(m_phi));
    }

    /*! Return Pz (GeV/c) */
    float getPz() const
    {
      return float(m_energy * cos(m_theta));
    }

    /*! Return TVector3 momentum (Px,Py,Pz) */
    TVector3 getMomentum() const { return TVector3(getPx(), getPy(), getPz()); }

    /*! Return 4Vector  (Px,Py,Pz,E) */
    TLorentzVector get4Vector() const { return TLorentzVector(getPx(), getPy(), getPz(), m_energy); }

    /*! Return TVector3 on cluster position /Shower center (x,y,z) */
    TVector3 getclusterPosition() const
    {
      return TVector3(
               m_r * sin(m_theta) * cos(m_phi),
               m_r * sin(m_theta) * sin(m_phi),
               m_r * cos(m_theta)
             );
    }

    /*! Return TVector3 on  position on gamma's production
      By default the position of gamma's production is (0,0,0) */
    TVector3 getPosition() const { return TVector3(0, 0, 0); }

    /*! Return TMatrixFsym 4x4  error matrix (order should be: px,py,pz,E) */
    TMatrixFSym getError4x4() const
    {
      TMatrixFSym errorecl = getError3x3();

      TMatrixF  jacobian(4, 3);
      const double cosPhi = cos(m_phi);
      const double sinPhi = sin(m_phi);
      const double cosTheta = cos(m_theta);
      const double sinTheta = sin(m_theta);
      const double E = m_energy;
      jacobian(0, 0) = cosPhi * sinTheta;
      jacobian(0, 1) = -1.0 * E * sinPhi * sinTheta;
      jacobian(0, 2) = E * cosPhi * cosTheta;
      jacobian(1, 0) = sinPhi * sinTheta;
      jacobian(1, 1) = E * cosPhi * sinTheta;
      jacobian(1, 2) = E * sinPhi * cosTheta;
      jacobian(2, 0) = cosTheta;
      jacobian(2, 1) = 0.0;
      jacobian(2, 2) = -1.0 * E * sinTheta;
      jacobian(3, 0) = 1.0;
      jacobian(3, 1) = 0.0;
      jacobian(3, 2) = 0.0;
      TMatrixFSym errCart(4);
      errCart = errorecl.Similarity(jacobian);
      return errCart;
    }

    /*! Return TMatrixFsym 7x7  error matrix (order should be: px,py,pz,E,x,y,z) */
    TMatrixFSym getError7x7() const
    {
      const TMatrixFSym errCart = getError4x4();

      TMatrixFSym errorMatrix(7);
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= i ; j++) {
          errorMatrix(i, j) = errorMatrix(j, i) = errCart(i, j);
        }
      }
      for (int i = 4; i < 7; ++i) {
        errorMatrix(i, i) = 1.0; // 1.0*1.0 cm^2 (default treatment as Belle ?)
      }
      return errorMatrix;
    }

    /*! Return TMatrixFsym 3x3 error matrix for E, Phi and Theta */
    TMatrixFSym getError3x3() const
    {
      TMatrixFSym errorecl(3);
      errorecl(0, 0) = m_error[0] * m_error[0]; //Energy
      errorecl(1, 0) = errorecl(0, 1) = m_error[1];
      errorecl(1, 1) = m_error[2] * m_error[2]; // Phi
      errorecl(2, 0) = errorecl(0, 2) = m_error[3];
      errorecl(2, 1) = errorecl(1, 2) = m_error[4];
      errorecl(2, 2) = m_error[5] * m_error[5]; // Theta
      return errorecl;
    }

    /*! Return m_isTrack true if the cluster matches with cluster, otherwise
      false (for time being). Please use isNeutral function (as it will be kept
      for future)
     */
    bool getisTrack() const { return m_isTrack; }

    /*! Return true if cluster has no match with cluster, otherwise
      return false if cluster has match with track. */
    bool isNeutral() const { return !m_isTrack; }

    //..... For FUTURE (to DO)

    /*! Return pi0Likelihood for a shower (for future, Not available now) */
    float getpi0Likelihood() const { return 0.5; }

    /*! Return etaLikelihood for a shower (for future, Not available now) */
    float getetaLikelihood() const { return 0.5; }

    /*! Return deltaL for shower shape (for future, Not available now) */
    float getdeltaL() const { return 0; }

    /*! Return beta for shower shape (for future, Not available now) */
    float getbeta() const { return 0; }

  private:

    bool m_isTrack;          /**< To store relation to tracks or not (for timebeing, Please use function  isNeutral() [as it is for future] */
    float m_energy;              /**< Corrected energy (GeV) */
    float m_theta;               /**< Theta of Shower (radian) */
    float m_phi;               /**< Phi of Shower (radian)  */
    float m_r;                 /**< R (cm) */
    float m_energyDepositSum;        /**< Uncorrected  Energy Deposited (GeV) */
    float m_timing;            /**< Timing information (study going on) */
    float m_errorTiming;           /**< Timing Error (NA) */
    float m_E9oE25;            /**< E9/E25: photon-like & isolation cut */
    float m_highestE;          /**< Highest energy stored in a crystal in Shower */
    float m_LAT;             /**< Lateral distribution parameter (Not available now) */
    int m_numberOfCrystals;      /**< Number of Crystals in a shower (Not available now) */
    int m_crystalHealth;       /**< Crystal Health (Not available now)
                      -100 : healthy
                        10 : Bad
                        20 : Broken
                        30 : Dead  */
    float m_mergedPi0;         /**< Likelihood of being Merged Pi0 (Not available now) */
    float m_error[6];            /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */

    /**< Class definition */
    ClassDef(ECLCluster, 2); /**< Needed to make objects storable */
  };

}// end namespace Belle2

#endif


