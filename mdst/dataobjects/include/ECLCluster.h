/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *                                                                        *
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

  /** Class to store reconstructed clusters in ECL */

  class ECLCluster : public RelationsObject {
  public:
    /**
     *default constructor : all values are set to 0, IDs set to -1, flags to false
     */
    ECLCluster() :
      m_isTrack(false),
      m_status(0),
      m_connectedRegionId(0),
      m_hypothesisId(0),
      m_covmat_rel_00(0.),
      m_covmat_10(0.),
      m_covmat_11(0.),
      m_covmat_20(0.),
      m_covmat_21(0.),
      m_covmat_22(0.),
      m_deltaL(0),
      m_minTrkDistance(0),
      m_absZernike40(0),
      m_absZernike51(0),
      m_zernikeMVA(0),
      m_E1oE9(0),
      m_E9oE21(0),
      m_secondMoment(0),
      m_LAT(0),
      m_numberOfCrystals(0),
      m_time(0),
      m_deltaTime99(0),
      m_theta(0),
      m_phi(0),
      m_r(0),
      m_logEnergy(-5.),
      m_logEnergyRaw(-5.),
      m_logEnergyHighestCrystal(-5.) {}

    /** Set m_isTrack true if the cluster matches with cluster. */
    void setisTrack(bool istrack) { m_isTrack = istrack; }

    /** Set status. */
    void setStatus(int status) { m_status = status; }

    /** Set connected region id. */
    void setConnectedRegionId(int crid) { m_connectedRegionId = crid; }

    /** Set hypothesis id. */
    void setHypothesisId(int hypothesisid) { m_hypothesisId = hypothesisid; }

    /** Set covariance matrix (3x3) for
     [0]->energy,
     [2]->phi,
     [5]->theta.
     */
    void setCovarianceMatrix(double covArray[6])
    {
      m_covmat_rel_00 = covArray[0] / (getEnergy() * getEnergy()); // store the fractional energy
      m_covmat_10 = covArray[1];
      m_covmat_11 = covArray[2]; // phi
      m_covmat_20 = covArray[3];
      m_covmat_21 = covArray[4];
      m_covmat_22 = covArray[5]; // theta
    }

    /** Set energy uncertainty. */
    void setUncertaintyEnergy(double energyunc) { m_covmat_rel_00 = energyunc * energyunc / (getEnergy() * getEnergy()); }

    /** Set theta error. */
    void setUncertaintyTheta(double thetaunc) { m_covmat_22 = thetaunc * thetaunc; }

    /** Set phi error. */
    void setUncertaintyPhi(double phiunc) { m_covmat_11 = phiunc * phiunc; }

    /** Set deltaL for shower shape. */
    void setdeltaL(double deltaL) { m_deltaL = deltaL; }

    /** Set distance between cluster COG and track extrapolation to ECL. */
    void setMinTrkDistance(double distance) { m_minTrkDistance = distance; }

    /** Set Zernike 40. */
    void setAbsZernike40(double zernike40) { m_absZernike40 = zernike40; }

    /** Set Zernike 51. */
    void setAbsZernike51(double zernike51) { m_absZernike51 = zernike51; }

    /** Set Zernike 20. */
    void setZernikeMVA(double zernikemva) { m_zernikeMVA = zernikemva; }

    /** Set E1/E9 energy ratio. */
    void setE1oE9(double E1oE9) { m_E1oE9 = E1oE9; }

    /** Set E9/E21 energy ratio. */
    void setE9oE21(double E9oE21) { m_E9oE21 = E9oE21; }

    /** Set SecondMoment. */
    void setSecondMoment(double secondmoment) { m_secondMoment = secondmoment; }

    /** Set Lateral distribution parameter. */
    void setLAT(double LAT) { m_LAT = LAT; }

    /** Set number of crystals (sum of weights). */
    void setNumberOfCrystals(double noc) { m_numberOfCrystals = noc; }

    /** Set time information. */
    void setTime(double time) { m_time = time; }

    /** Set 99% time containment range */
    void setDeltaTime99(double dtime99) { m_deltaTime99 = dtime99; }

    /** Set Theta of Shower (radian). */
    void setTheta(double theta) { m_theta = theta; }

    /** Set Phi of Shower (radian). */
    void setPhi(double phi) { m_phi = phi; }

    /** Set R (in cm). */
    void setR(double r) { m_r = r; }

    /** Set Corrected Energy (GeV). */
    void setEnergy(double energy) { m_logEnergy = log(energy); }

    /** Set Uncorrect Energy deposited (GeV). */
    void setEnergyRaw(double energyraw) { m_logEnergyRaw = log(energyraw); }

    /** Set energy of highest energetic crystal (GeV). */
    void setEnergyHighestCrystal(double energyhighestcrystal) { m_logEnergyHighestCrystal = log(energyhighestcrystal); }

    /** Return true if the cluster matches with track. */
    bool getisTrack() const { return m_isTrack; }

    /** Return true if the cluster matches with track. */
    bool isTrack() const { return m_isTrack; }

    /** Return true if cluster has no match with track. */
    bool getisNeutral() const { return !m_isTrack; }

    /** Return true if cluster has no match with track. */
    bool isNeutral() const { return !m_isTrack; }

    /** Return status. */
    int getStatus() const {return m_status;}

    /** Return connected region id. */
    int getConnectedRegionId() const {return m_connectedRegionId;}

    /** Return hypothesis id */
    int getHypothesisId() const {return m_hypothesisId;}

    /** Get distance between cluster COG and track extrapolation to ECL. */
    double getMinTrkDistance() const { return m_minTrkDistance; }

    /** Return deltaL. */
    double getDeltaL() const { return m_deltaL; }

    /** Return Zernike moment 40 (shower shape variable). */
    double getAbsZernike40() const { return m_absZernike40; }

    /** Return Zernike moment 51 (shower shape variable). */
    double getAbsZernike51() const { return m_absZernike51; }

    /** Return MVA based hadron/photon value based on Zernike moments (shower shape variable). */
    double getZernikeMVA() const { return m_zernikeMVA; }

    /** Return E1/E9 (shower shape variable). */
    double getE1oE9() const { return m_E1oE9; }

    /** Return E9/E21 (shower shape variable). */
    double getE9oE21() const { return m_E9oE21; }

    /** Return second moment (shower shape variable). */
    double getSecondMoment() const { return m_secondMoment; }

    /** Return LAT (shower shape variable). */
    double getLAT() const {return m_LAT;}

    /** Return number of a crystals in a shower (sum of weights). */
    double getNumberOfCrystals() const {return m_numberOfCrystals;}

    /** Return cluster time. */
    double getTime() const {return m_time;}

    /** Return cluster delta time 99 */
    double getDeltaTime99() const {return m_deltaTime99;}

    /** Return Corrected Phi of Shower (radian). */
    double getPhi() const { return m_phi;}

    /** Return Corrected Theta of Shower (radian). */
    double getTheta() const {return m_theta;}

    /** Return R. */
    double getR() const { return m_r; }

    /** Return Corrected Energy (GeV). */
    double getEnergy() const {return exp(m_logEnergy);}

    /** Return Uncorrect Energy deposited (GeV) */
    double getEnergyRaw() const {return exp(m_logEnergyRaw);}

    /** Return energy of highest energetic crystal in cluster (GeV) */
    double getEnergyHighestCrystal() const {return exp(m_logEnergyHighestCrystal);}

    /** Return Uncertainty on Energy of Shower */
    double getUncertaintyEnergy() const {return sqrt(m_covmat_rel_00) * getEnergy();}

    /** Return Uncertainty on Theta of Shower */
    double getUncertaintyTheta() const {return sqrt(m_covmat_22);}

    /** Return Uncertainty on Phi of Shower */
    double getUncertaintyPhi() const { return sqrt(m_covmat_11);}

    /** Return Px (GeV/c) */
    double getPx() const { return getEnergy() * sin(getTheta()) * cos(getPhi()); }

    /** Return Py (GeV/c)  */
    double getPy() const { return getEnergy() * sin(getTheta()) * sin(getPhi()); }

    /** Return Pz (GeV/c) */
    double getPz() const { return getEnergy() * cos(getTheta()); }

    /** Return TVector3 momentum (Px,Py,Pz) */
    TVector3 getMomentum() const
    {
      return TVector3(getPx(), getPy(), getPz());
    }

    /** Return 4Vector  (Px,Py,Pz,E) */
    TLorentzVector get4Vector() const
    {
      return TLorentzVector(getPx(), getPy(), getPz(), getEnergy());
    }

    /** Return TVector3 on cluster position / Shower center (x,y,z) */
    TVector3 getClusterPosition() const
    {
      const double cluster_x =  getR() * sin(getTheta()) * cos(getPhi());
      const double cluster_y =  getR() * sin(getTheta()) * sin(getPhi());
      const double cluster_z =  getR() * cos(getTheta());
      return TVector3(cluster_x, cluster_y, cluster_z);
    }

    /** Return TVector3 on  position on gamma's production
     By default the position of gamma's production is (0,0,0) */
    TVector3 getPosition() const { return TVector3(0, 0, 0); }

    /** Return TMatrixDsym 4x4 covariance matrix (order should be: px,py,pz,E) */
    TMatrixDSym getCovarianceMatrix4x4() const
    {
      TMatrixDSym covmatecl = getCovarianceMatrix3x3();

      TMatrixD  jacobian(4, 3);
      const double cosPhi = cos(getPhi());
      const double sinPhi = sin(getPhi());
      const double cosTheta = cos(getTheta());
      const double sinTheta = sin(getTheta());
      const double E = getEnergy();

      jacobian(0, 0) =            cosPhi * sinTheta;
      jacobian(0, 1) = -1.0 * E * sinPhi * sinTheta;
      jacobian(0, 2) =        E * cosPhi * cosTheta;
      jacobian(1, 0) =            sinPhi * sinTheta;
      jacobian(1, 1) =        E * cosPhi * sinTheta;
      jacobian(1, 2) =        E * sinPhi * cosTheta;
      jacobian(2, 0) =                     cosTheta;
      jacobian(2, 1) =           0.0;
      jacobian(2, 2) = -1.0 * E          * sinTheta;
      jacobian(3, 0) =           1.0;
      jacobian(3, 1) =           0.0;
      jacobian(3, 2) =           0.0;
      TMatrixDSym covmatCart(4);
      covmatCart = covmatecl.Similarity(jacobian);
      return covmatCart;
    }

    /** Return TMatrixDsym 7x7 covariance matrix (order should be: px,py,pz,E,x,y,z) */
    TMatrixDSym getCovarianceMatrix7x7() const
    {
      const TMatrixDSym covmatCart = getCovarianceMatrix4x4();

      TMatrixDSym covmatMatrix(7);
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= i ; j++) {
          covmatMatrix(i, j) = covmatMatrix(j, i) = covmatCart(i, j);
        }
      }
      for (int i = 4; i < 7; ++i) {
        covmatMatrix(i, i) = 1.0; // 1.0*1.0 cm^2 (default treatment as Belle ?)
      }
      return covmatMatrix;
    }

    /** Return TMatrixDsym 3x3 covariance matrix for E, Phi and Theta */
    TMatrixDSym getCovarianceMatrix3x3() const
    {
      TMatrixDSym covmatecl(3);
      covmatecl(0, 0) = m_covmat_rel_00 * (getEnergy() * getEnergy());
      covmatecl(1, 0) = m_covmat_10;
      covmatecl(1, 1) = m_covmat_11;
      covmatecl(2, 0) = m_covmat_20;
      covmatecl(2, 1) = m_covmat_21;
      covmatecl(2, 2) = m_covmat_22;

      //make symmetric
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < i ; j++)
          covmatecl(j, i) = covmatecl(i, j);
      return covmatecl;
    }

    /** Return detector region: 0: below acceptance, 1: FWD, 2: BRL, 3: BWD, 11: FWDGAP, 13: BWDGAP */
    int getDetectorRegion() const
    {
      const double theta = getTheta();

      if (theta < 0.2164208) return 0;   // < 12.4deg
      if (theta < 0.5480334) return 1;   // < 31.4deg
      if (theta < 0.561996) return 11;   // < 32.2deg
      if (theta < 2.2462387) return 2;   // < 128.7
      if (theta < 2.2811453) return 13;   // < 130.7
      if (theta < 2.7070057) return 3;   // < 155.1
      else return 0;
    }


    // DEPRECATED (WILL BE REMOVED FOR RELEASE-00-08-00)
    void setError(double covArray[6]) {setCovarianceMatrix(covArray);}
    void setEnedepSum(double enedepsum) {setEnergyRaw(enedepsum);}
    void setE9oE25(double e9oe21) {setE9oE21(e9oe21);}
    void setHighestE(double highteste) {setEnergyHighestCrystal(highteste);}
    void setTiming(double time) {setTime(time);}
    void setNofCrystals(int noc) {setNumberOfCrystals((double) noc);}

    double getTiming() const {return getTime();}
    double getE9oE25() const {return getE9oE21();}
    TVector3 getclusterPosition() const {return getClusterPosition();}

    double getErrorEnergy() const {return getUncertaintyEnergy();}
    double getErrorTheta() const {return getUncertaintyTheta();}
    double getErrorPhi() const {return getUncertaintyPhi();}
    TMatrixDSym getError3x3() const {return getCovarianceMatrix3x3();}
  private:

    /**< Is related to track (true) or not (false). */
    bool m_isTrack;

    /**< Cluster status. */
    int m_status;

    /**< Connected Region of this cluster. */
    int m_connectedRegionId;

    /**< Cluster hypothesis. */
    int m_hypothesisId;

    //Covariance matrix:
    //       E    phi   theta
    // E     00   01    02
    // phi   10   11    12
    // theta 20   21    22

    /**< Relative covariance entry 00 sigma_E*sigma_E / (E*E), 0.01*0.01 and 0.25*0.25 */
    Double32_t m_covmat_rel_00; //[0.0001, 0.0625, 10]

    /**< Covariance matrix 10, not used yet */
    Double32_t m_covmat_10; //[0.0, 10., 8]

    /**< Covariance matrix 11, sigma_phi*sigma_phi, between 0 and 50*50 mrad^2 */
    Double32_t m_covmat_11; //[0.0, 0.0025, 8]

    /**< Covariance matrix 20, not used yet */
    Double32_t m_covmat_20; //[0.0, 10., 8]

    /**< Covariance matrix 21, not used yet */
    Double32_t m_covmat_21; //[0.0, 10., 8]

    /**< Covariance matrix 22, sigma_theta*sigma_theta, between 0 and 50*50 mrad^2 */
    Double32_t m_covmat_22; //[0.0, 0.0025, 8]

    /**<  Delta L as defined in arXiv:0711.1593. */
    Double32_t  m_deltaL;  //[-250, 250., 10]

    /**<  Distance between cluster COG and track extrapolation to ECL. */
    Double32_t  m_minTrkDistance;  //[0.0, 250., 10]

    /**<  Zernike 40. */
    Double32_t  m_absZernike40;  //[0.0, 1.7, 10]

    /**<  Zernike 51. */
    Double32_t  m_absZernike51;  //[0.0, 1.2, 10]

    /**<  Zernike MVA. */
    Double32_t  m_zernikeMVA;  //[0.0, 1., 10]

    /**<  E1oE9. */
    Double32_t  m_E1oE9;  //[0.0, 1., 10]

    /**<  E9oE21. */
    Double32_t  m_E9oE21;  //[0.0, 1., 10]

    /**<  Second Moment. */
    Double32_t  m_secondMoment;  //[0.0, 100., 10]

    /**<  LAT. */
    Double32_t  m_LAT;  //[0.0, 1., 10]

    /**<  Number of Crystals in a shower (sum of weights). */
    Double32_t  m_numberOfCrystals;  //[0.0, 200.0, 10]

    /**<  Time. */
    Double32_t  m_time;  //[-1000.0, 1000.0, 12]

    /**<  Delta Time 99. */
    Double32_t  m_deltaTime99;  //[0.0, 1000.0, 12]

    /**<  Theta [rad]. */
    Double32_t  m_theta;  //[0.0, pi, 16]

    /**<  Phi [rad]. */
    Double32_t  m_phi;  //[-pi, pi, 16]

    /**<  Radius [cm]. */
    Double32_t  m_r;  //[75.0, 300.0, 16]

    /**<  Log. Energy [GeV]. */
    Double32_t  m_logEnergy;  //[-5, 3., 18]

    /**<  Log. Raw Energy [GeV]. */
    Double32_t  m_logEnergyRaw;  //[-5, 3., 18]

    /**<  Log. Highest Crystal Energy [GeV]. */
    Double32_t  m_logEnergyHighestCrystal;  //[-5, 3., 18]

    /**< Class definition */
    ClassDef(ECLCluster, 4);
    // 4: Complete revision and new variables. Introduction of Double32_t. Some new setters and getters.

  };

}// end namespace Belle2

#endif
