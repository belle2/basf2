/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vishal     (ECL Software Group)                          *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCLUSTER_H
#define ECLCLUSTER_H

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMatrixDSym.h>
#include <TMatrixD.h>

#include <cmath>

namespace Belle2 {

  /*! Class to store reconstructed cluster in ECL */


  class ECLCluster : public RelationsObject {
  public:
    /**
     *default constructor : all values are set to 0
     */
    ECLCluster() :
      m_isTrack(false),
      m_Energy(0),
      m_Theta(0),
      m_Phi(0),
      m_R(0),
      m_EnedepSum(0),
      m_Timing(0),
      m_ErrorTiming(0),
      m_E9oE25(0),
      m_HighestE(0),
      m_LAT(0),
      m_NofCrystals(0),
      m_CrystHealth(0),
      m_MergedPi0(0),
      m_minTrkDistance(-1),
      m_deltaL(-1)
    {
      m_Error[0] = 0;
      m_Error[1] = 0;
      m_Error[2] = 0;
      m_Error[3] = 0;
      m_Error[4] = 0;
      m_Error[5] = 0;
    }

    /*! Set Corrected Energy (GeV) */
    void setEnergy(double Energy) { m_Energy = Energy; }

    /*! Set Theta of Shower (radian) */
    void setTheta(double Theta) { m_Theta = Theta; }

    /*! Set Phi of Shower (radian) */
    void setPhi(double Phi) { m_Phi = Phi; }

    /*! Set R (in cm) */
    void setR(double R) { m_R = R; }

    /*! Set Uncorrect Energy deposited (GeV)*/
    void setEnedepSum(double EnedepSum) { m_EnedepSum = EnedepSum; }

    /*! Set Timing information */
    void setTiming(double Timing) { m_Timing = Timing; }

    /*! Set Error on Timing information */
    void setErrorTiming(double ErrorTiming) { m_ErrorTiming = ErrorTiming; }

    /*! Set E9/E25 */
    void setE9oE25(double E9oE25) { m_E9oE25 = E9oE25; }

    /*! Set Highest Energy stored in a Crystal in a shower (GeV) */
    void setHighestE(double HighestE) { m_HighestE = HighestE; }

    /*! Set Lateral distribution parameter */
    void setLAT(double LAT) { m_LAT = LAT; }

    /*! Set distance between cluster COG and track extrapolation to ECL */ \
    void setMinTrkDistance(double distance) { m_minTrkDistance = distance; }

    /*! Set deltaL for shower shape */
    void setdeltaL(double deltaL) { m_deltaL = deltaL; }

    /*! Set Number of Crystals in a shower */
    void setNofCrystals(int NofCrystals) { m_NofCrystals = NofCrystals; }

    /*! Set Crystal health (Not now) */
    void setCrystHealth(int CrystHealth) { m_CrystHealth = CrystHealth; }

    /*! Set high momentum pi0 likelihood */
    void setMergedPi0(double MergedPi0) { m_MergedPi0 = MergedPi0; }

    /*! Set  Error Array(3x3)  for
      [0]->Error on Energy
      [2]->Error on Phi
      [5]->Error on Theta
    */
    void setError(double ErrorArray[6])
    {
      for (int i = 0; i < 6; ++i) {
        m_Error[i] = ErrorArray[i];
      }
    }

    /*! Set m_isTrack true if the cluster matches with cluster */
    void setisTrack(bool istrack) { m_isTrack = istrack; }

    /*! Return Correct Energy (GeV) */
    double getEnergy() const {return m_Energy;}

    /*! Return Theta of Shower (radian) */
    double getTheta() const {return m_Theta;}

    /*! Return Phi of Shower (radian) */
    double getPhi() const { return m_Phi;}

    /*! Return R */
    double getR() const { return m_R ; }

    /*! Return Error on Energy */
    double getErrorEnergy() const {return m_Error[0];}

    /*! Return Error on Theta of Shower */
    double getErrorTheta() const {return m_Error[5];}

    /*! Return Error on Phi of Shower */
    double getErrorPhi() const { return m_Error[2];}

    /*! Return Uncorrect Energy deposited (GeV) */
    double getEnedepSum() const {return m_EnedepSum;}

    /*! Return Timing information */
    double getTiming() const {return m_Timing;}

    /*! Return Error on Timing information */
    double getErrorTiming() const {return m_ErrorTiming;}

    /*! Return E9/E25 */
    double getE9oE25() const { return m_E9oE25; }

    /*! Return HighestEnergy in a crystal in a shower */
    double getHighestE() const {return m_HighestE; }

    /*! Return LAT (shower variable) */
    double getLAT() const {return m_LAT;}

    /*! Get distance between cluster COG and track extrapolation to ECL. Note that this variable may be removed in later versions. */
    double getTemporaryMinTrkDistance() const { return m_minTrkDistance; }

    /*! Return deltaL for shower shape. Note that this variable may be removed in later versions. */
    double getTemporaryDeltaL() const { return m_deltaL; }

    /*! Return Number of a Crystals in a shower */
    int getNofCrystals() const {return m_NofCrystals;}

    /*! Return Crystal health (Not now) */
    int getCrystHealth() const { return m_CrystHealth;}

    /*! Return high momentum pi0 likelihood */
    double getMergedPi0() const { return m_MergedPi0;}

    /*! Return Px (GeV/c) */
    double getPx() const { return getEnergy() * sin(getTheta()) * cos(getPhi()); }

    /*! Return Py (GeV/c)  */
    double getPy() const { return getEnergy() * sin(getTheta()) * sin(getPhi()); }

    /*! Return Pz (GeV/c) */
    double getPz() const { return getEnergy() * cos(getTheta()); }

    /*! Return TVector3 momentum (Px,Py,Pz) */
    TVector3 getMomentum() const
    {
      return TVector3(getPx(), getPy(), getPz());
    }

    /*! Return 4Vector  (Px,Py,Pz,E) */
    TLorentzVector get4Vector() const
    {
      return TLorentzVector(getPx(), getPy(), getPz(), getEnergy());
    }

    /*! Return TVector3 on cluster position /Shower center (x,y,z) */
    TVector3 getclusterPosition() const
    {
      const double cluster_x =  getR() * sin(getTheta()) * cos(getPhi());
      const double cluster_y =  getR() * sin(getTheta()) * sin(getPhi());
      const double cluster_z =  getR() * cos(getTheta());
      return TVector3(cluster_x, cluster_y, cluster_z);
    }

    /*! Return TVector3 on  position on gamma's production
      By default the position of gamma's production is (0,0,0) */
    TVector3 getPosition() const { return TVector3(0, 0, 0); }

    /*! Return TMatrixDsym 4x4  error matrix (order should be: px,py,pz,E) */
    TMatrixDSym getError4x4() const
    {
      TMatrixDSym errorecl = getError3x3();

      TMatrixD  jacobian(4, 3);
      const double cosPhi = cos(getPhi());
      const double sinPhi = sin(getPhi());
      const double cosTheta = cos(getTheta());
      const double sinTheta = sin(getTheta());
      const double E = getEnergy();

      jacobian(0, 0) =            cosPhi * sinTheta;
      jacobian(0, 1) =  -1.0 * E * sinPhi * sinTheta;
      jacobian(0, 2) =        E * cosPhi * cosTheta;
      jacobian(1, 0) =            sinPhi * sinTheta;
      jacobian(1, 1) =        E * cosPhi * sinTheta;
      jacobian(1, 2) =        E * sinPhi * cosTheta;
      jacobian(2, 0) =                     cosTheta;
      jacobian(2, 1) =           0.0;
      jacobian(2, 2) =  -1.0 * E          * sinTheta;
      jacobian(3, 0) =           1.0;
      jacobian(3, 1) =           0.0;
      jacobian(3, 2) =           0.0;
      TMatrixDSym errCart(4);
      errCart = errorecl.Similarity(jacobian);
      return errCart;
    }

    /*! Return TMatrixDsym 7x7  error matrix (order should be: px,py,pz,E,x,y,z) */
    TMatrixDSym getError7x7() const
    {
      const TMatrixDSym errCart = getError4x4();

      TMatrixDSym errorMatrix(7);
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

    /*! Return TMatrixDsym 3x3 error matrix for E, Phi and Theta */
    TMatrixDSym getError3x3() const
    {
      TMatrixDSym errorecl(3);
      errorecl(0, 0) = m_Error[0] * m_Error[0]; //Energy
      errorecl(1, 0) = m_Error[1];
      errorecl(1, 1) = m_Error[2] * m_Error[2]; // Phi
      errorecl(2, 0) = m_Error[3];
      errorecl(2, 1) = m_Error[4];
      errorecl(2, 2) = m_Error[5] * m_Error[5]; // Theta

      //make symmetric
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < i ; j++)
          errorecl(j, i) = errorecl(i, j);
      return errorecl;
    }

    /*! Return m_isTrack true if the cluster matches with cluster, otherwise
      false (for time being). Please use isNeutral function (as it will be kept
      for future)
     */
    bool getisTrack() const { return m_isTrack; }

    /*! Return true if cluster has no match with cluster, otherwise
      return false if cluster has match with track.
     */
    bool isNeutral() const { return !m_isTrack; }

    //..... For FUTURE (to DO)

    /*! Return pi0Likelihood for a shower (for future, Not available now) */

    double getpi0Likelihood() const { return 0.5; }
    /*! Return etaLikelihood for a shower (for future, Not available now) */
    double getetaLikelihood() const { return 0.5; }

    /*! Return deltaL for shower shape (for future, Not available now) */
    double getdeltaL() const { return 0;}

    /*! Return beta for shower shape (for future, Not available now) */
    double getbeta() const { return 0; }

  private:

    float m_Error[6]; /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
    bool m_isTrack;  /**< To store relation to tracks or not (for timebeing, Pleaseuse function  isNeutral() [as it is for future] */
    float  m_Energy;     /**< Corrected energy (GeV) */
    float  m_Theta;         /**< Theta of Shower (radian) */
    float  m_Phi;         /**< Phi of Shower (radian)  */
    float  m_R;         /**< R (cm) */
    float  m_EnedepSum;  /**< Uncorrected  Energy Deposited (GeV) */
    float  m_Timing;     /**< Timing information (study going on) */
    float  m_ErrorTiming;    /**< Timing Error (NA) */
    float  m_E9oE25;     /**< E9/E25: photon-like & isolation cut */
    float  m_HighestE;  /**< Highest energy stored in a crystal in Shower */
    float  m_LAT;  /**< Lateral distribution parameter */
    int    m_NofCrystals; /**< Number of Crystals in a shower (Not available now) */
    int    m_CrystHealth;/**< Crystal Health (Not available now)
          -100 : healthy
          10 : Bad
          20 : Broken
          30 : Dead  */
    float m_MergedPi0; /**< Likelihood of being Merged Pi0 (Not available now) */
    float  m_minTrkDistance; /**<  Distance between cluster COG and track extrapolation to ECL. Note that this variable may be removed in later versions. */
    float  m_deltaL; /**<  Delta L as defined in arXiv:0711.1593. Note that this variable may be removed in later versions. */

    /**< Class definition */
    ClassDef(ECLCluster, 3); /**< Needed to make objects storable */
  };

}// end namespace Belle2

#endif


