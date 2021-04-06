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

#pragma once

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <TMatrixDSym.h>

#include <cmath>

namespace Belle2 {

  /**
   * ECL cluster data.
   */
  class ECLCluster : public RelationsObject {
  public:

    /** The hypothesis bits for this ECLCluster (Connected region (CR) is split using this hypothesis.*/
    enum class EHypothesisBit : short unsigned int {
      /** None as initializer */
      c_none = 1 << 0,
      /** CR is split into a muon and n photons (T1) */
      c_muonNPhotons = 1 << 1,
      /** CR is reconstructed as a charged hadron (T2) */
      c_chargedHadron = 1 << 2,
      /** CR is split into an electron and n photons (T3) */
      c_electronNPhotons = 1 << 3,
      /** CR is split into n photons (N1) */
      c_nPhotons = 1 << 4,
      /** CR is reconstructed as a neutral hadron (N2) */
      c_neutralHadron = 1 << 5,
      /** CR is reconstructed as merged pi0 (N3)*/
      c_mergedPi0 = 1 << 6
    };

    /** The status information for the ECLCluster. */
    enum class EStatusBit : short unsigned int {
      /** bit 0: ECLCluster is matched to a ECL trigger cluster */
      c_TriggerCluster   = 1 << 0,
      /** bit 1: ECLCluster to ECLTRGCluster matcher was run */
      c_TriggerClusterMatching = 1 << 1,
      /** bit 2: ECLCluster has pulse shape discrimination variables.*/
      c_PulseShapeDiscrimination = 1 << 2,
      /** bit 3: ECLCluster has fit time that failed.*/
      c_fitTimeFailed = 1 << 3,
      /** bit 4: ECLCluster has time resolution calculation that failed.*/
      c_timeResolutionFailed = 1 << 4,

    };

    /**
     *default constructor : all values are set to 0, IDs set to -1, flags to false
     */

    ECLCluster() :
      m_isTrack(false),
      m_status(0),
      m_hypotheses(static_cast<unsigned short>(EHypothesisBit::c_nPhotons)), // set to c_nPhotons for b2bii
      m_maxECellId(0),
      m_connectedRegionId(0),
      m_clusterId(0),
      m_sqrtcovmat_00(0.),
      m_covmat_10(0.),
      m_sqrtcovmat_11(0.),
      m_covmat_20(0.),
      m_covmat_21(0.),
      m_sqrtcovmat_22(0.),
      m_deltaL(0.),
      m_minTrkDistance(0.),
      m_minTrkDistanceID(65535),
      m_absZernike40(0.),
      m_absZernike51(0.),
      m_zernikeMVA(0.),
      m_E1oE9(0.),
      m_E9oE21(0.),
      m_secondMoment(0.),
      m_LAT(0.),
      m_numberOfCrystals(0.),
      m_time(0.),
      m_deltaTime99(0.),
      m_theta(0.),
      m_phi(0.),
      m_r(0.),
      m_logEnergy(-5.),
      m_logEnergyRaw(-5.),
      m_logEnergyHighestCrystal(-5.),
      m_PulseShapeDiscriminationMVA(0.5),
      m_ClusterHadronIntensity(0.),
      m_NumberOfHadronDigits(0.) {}

    /** Set m_isTrack true if the cluster matches with a track. */
    void setIsTrack(bool istrack) { m_isTrack = istrack; }

    /** Set status. */
    void setStatus(EStatusBit status) { m_status = static_cast<short unsigned>(status); }

    /**
     * Add bitmask to current status.
     * @param bitmask The status code which should be added.
     */
    void addStatus(EStatusBit bitmask) { m_status |= static_cast<short unsigned>(bitmask); }

    /**
     * Remove bitmask from current status.
     * @param bitmask The status code which should be removed.

     */
    void removeStatus(EStatusBit bitmask) { m_status &= (~static_cast<short unsigned>(bitmask)); }

    /** Set hypotheses. */
    void setHypothesis(EHypothesisBit hypothesis) { m_hypotheses = static_cast<short unsigned>(hypothesis); }

    /**
     * Add bitmask to current hypothesis.
     * @param bitmask The status code which should be added.
     */
    void addHypothesis(EHypothesisBit bitmask) { m_hypotheses |= static_cast<short unsigned>(bitmask); }

    /**
     * Remove bitmask from current hypothesis.
     * @param bitmask The hypothesis code which should be removed.

     */
    void removeHypothesis(EHypothesisBit bitmask) { m_hypotheses &= (~static_cast<short unsigned>(bitmask)); }

    /** Set cellID of maximum energy crystal */
    void setMaxECellId(unsigned short cellid) {m_maxECellId = cellid;}

    /** Set connected region id. */
    void setConnectedRegionId(int crid) { m_connectedRegionId = crid; }

    /** Set cluster id. */
    void setClusterId(int clusterid) { m_clusterId = clusterid; }

    /** Set covariance matrix (3x3), i.e. squared entries, for
     [0]->energy,
     [2]->phi,
     [5]->theta.
     */
    void setCovarianceMatrix(double covArray[6])
    {
      m_sqrtcovmat_00 = sqrt(fabs(covArray[0])); // energy
      m_covmat_10 = covArray[1];
      m_sqrtcovmat_11 = sqrt(fabs(covArray[2])); // phi
      m_covmat_20 = covArray[3];
      m_covmat_21 = covArray[4];
      m_sqrtcovmat_22 = sqrt(fabs(covArray[5])); // theta
    }

    /** Set energy uncertainty. */
    void setUncertaintyEnergy(double energyunc) { m_sqrtcovmat_00 = fabs(energyunc); }

    /** Set theta uncertainty. */
    void setUncertaintyTheta(double thetaunc) { m_sqrtcovmat_22 = fabs(thetaunc); }

    /** Set phi uncertainty. */
    void setUncertaintyPhi(double phiunc) { m_sqrtcovmat_11 = fabs(phiunc); }

    /** Set deltaL for shower shape. */
    void setdeltaL(double deltaL) { m_deltaL = deltaL; }

    /** Set distance between cluster COG and track extrapolation to ECL. */
    void setMinTrkDistance(double distance) { m_minTrkDistance = distance; }

    /** Set array index of the nearest track */
    void setMinTrkDistanceID(unsigned short distanceID) { m_minTrkDistanceID = distanceID; }

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

    /** set Pulse Shape Discrimination MVA */
    void setPulseShapeDiscriminationMVA(double PulseShapeDiscriminationMVA) { m_PulseShapeDiscriminationMVA = PulseShapeDiscriminationMVA; }

    /** set Number of hadron digits in cluster . */
    void setNumberOfHadronDigits(double NumberOfHadronDigits) { m_NumberOfHadronDigits = NumberOfHadronDigits; }

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
    bool isTrack() const { return m_isTrack; }

    /** Return true if cluster has no match with track. */
    bool isNeutral() const { return !m_isTrack; }

    /** Return status. */
    unsigned short getStatus() const {return m_status;}

    /** Return hypothesis (expert only, this returns a bti pattern). */
    unsigned short getHypotheses() const {return m_hypotheses;}

    /** Return cellID of maximum energy crystal */
    unsigned short getMaxECellId() const {return m_maxECellId;}

    /** Return connected region id. */
    int getConnectedRegionId() const {return m_connectedRegionId;}

    /** Return cluster id */
    int getClusterId() const {return m_clusterId;}

    /** Get distance between cluster COG and track extrapolation to ECL. */
    double getMinTrkDistance() const { return m_minTrkDistance; }

    /** Get array index of the nearest track */
    double getMinTrkDistanceID() const { return m_minTrkDistanceID; }

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

    /** Return MVA classifier that uses pulse shape discrimination to identify electromagnetic vs hadronic showers.*/
    double getPulseShapeDiscriminationMVA() const { return m_PulseShapeDiscriminationMVA; }

    /** Return number of hadron digits in cluster*/
    double getNumberOfHadronDigits() const { return m_NumberOfHadronDigits; }

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

    /** Return Energy (GeV). */
    double getEnergy(const EHypothesisBit& hypothesis) const;

    /** Return Uncorrected Energy deposited (GeV) */
    double getEnergyRaw() const {return exp(m_logEnergyRaw);}

    /** Return energy of highest energetic crystal in cluster (GeV) */
    double getEnergyHighestCrystal() const {return exp(m_logEnergyHighestCrystal);}

    /** Return Uncertainty on Energy of Shower */
    double getUncertaintyEnergy() const {return (m_sqrtcovmat_00);}

    /** Return Uncertainty on Theta of Shower */
    double getUncertaintyTheta() const {return (m_sqrtcovmat_22);}

    /** Return Uncertainty on Phi of Shower */
    double getUncertaintyPhi() const { return (m_sqrtcovmat_11);}

    /** Return TVector3 on cluster position (x,y,z) */
    TVector3 getClusterPosition() const;

    /** Return TMatrixDsym 3x3 covariance matrix for E, Phi and Theta */
    TMatrixDSym getCovarianceMatrix3x3() const;

    /** Return detector region: 0: below acceptance, 1: FWD, 2: BRL, 3: BWD, 11: FWDGAP, 13: BWDGAP */
    int getDetectorRegion() const;

    /**
     * Return if specific status bit is set.
     * @param bitmask The bitmask which is compared to the status of the cluster.
     * @return Returns true if the bitmask matches the status code of the cluster.
     */
    bool hasStatus(EStatusBit bitmask) const { return (m_status & static_cast<short unsigned>(bitmask)) == static_cast<short unsigned>(bitmask); }

    /**
     * Return if specific hypothesis bit is set.
     * @param bitmask The bitmask which is compared to the hypothesis of the cluster.
     * @return Returns true if the bitmask matches the hypothesis code of the cluster.
     */
    bool hasHypothesis(EHypothesisBit bitmask) const { return (m_hypotheses & static_cast<short unsigned>(bitmask)) == static_cast<short unsigned>(bitmask); }

    /** Check if ECLCluster is matched to an ECLTRGCluster */
    bool isTriggerCluster() const {return hasStatus(EStatusBit::c_TriggerCluster);}

    /** Check if ECLTRGCluster to ECLCluster matcher has run */
    bool hasTriggerClusterMatching() const {return hasStatus(EStatusBit::c_TriggerClusterMatching);}

    /** Check if ECLCluster has any ECLDigits with waveforms that also passed two component fit chi2 threshold in eclClusterPSD module. */
    bool hasPulseShapeDiscrimination() const {return hasStatus(EStatusBit::c_PulseShapeDiscrimination);}

    /** Check if ECLCluster has a fit time that failed. */
    bool hasFailedFitTime() const {return hasStatus(EStatusBit::c_fitTimeFailed);}

    /** Check if ECLCluster has a time resolution calculation that failed. */
    bool hasFailedTimeResolution() const {return hasStatus(EStatusBit::c_timeResolutionFailed);}


  private:

    /** Is related to track (true) or not (false). */
    bool m_isTrack;

    /** Cluster status. */
    unsigned short m_status;

    /** Hypothesis. */
    unsigned short m_hypotheses;

    /** CellID of maximum energy crystal */
    unsigned short m_maxECellId;

    /** Connected Region of this cluster. */
    int m_connectedRegionId;

    /** Cluster id */
    int m_clusterId;

    //Covariance matrix:
    //       E    phi   theta
    // E     00   01    02
    // phi   10   11    12
    // theta 20   21    22
    /** Covariance entry 00 sigma_E (1% to 25% between 10 MeV and 8 GeV) */
    Double32_t m_sqrtcovmat_00; //[0.0, 0.3, 10]

    /** Covariance matrix 10, not used yet */
    Double32_t m_covmat_10; //[0.0, 10., 12]

    /** Covariance matrix 11, sigma_phi, between 0 and 50 mrad */
    Double32_t m_sqrtcovmat_11; //[0.0, 0.05, 8]

    /** Covariance matrix 20, not used yet */
    Double32_t m_covmat_20; //[0.0, 10., 12]

    /** Covariance matrix 21, not used yet */
    Double32_t m_covmat_21; //[0.0, 10., 12]

    /** Covariance matrix 22, sigma_theta, between 0 and 50 mrad */
    Double32_t m_sqrtcovmat_22; //[0.0, 0.050, 8]

    /** Delta L as defined in arXiv:0711.1593. */
    Double32_t  m_deltaL;  //[-250, 250., 10]

    /** Distance between cluster center and track extrapolation to ECL. */
    Double32_t  m_minTrkDistance;  //[0.0, 250., 10]

    /** Array index of the nearest track */
    unsigned short m_minTrkDistanceID;

    /** Zernike 40. */
    Double32_t  m_absZernike40;  //[0.0, 1.7, 10]

    /** Zernike 51. */
    Double32_t  m_absZernike51;  //[0.0, 1.2, 10]

    /** Zernike MVA. */
    Double32_t  m_zernikeMVA;  //[0.0, 1., 10]

    /** E1oE9. */
    Double32_t  m_E1oE9;  //[0.0, 1., 10]

    /** E9oE21. */
    Double32_t  m_E9oE21;  //[0.0, 1., 10]

    /** Second Moment. */
    Double32_t  m_secondMoment;  //[0.0, 40.0, 10]

    /** LAT. */
    Double32_t  m_LAT;  //[0.0, 1., 10]

    /** Number of Crystals in a shower (sum of weights). */
    Double32_t  m_numberOfCrystals;  //[0.0, 200.0, 10]

    /** Time. */
    Double32_t  m_time;  //[-1000.0, 1000.0, 12]

    /** Delta Time 99. */
    Double32_t  m_deltaTime99;  //[0.0, 1000.0, 12]

    /** Theta [rad]. */
    Double32_t  m_theta;  //[0.0, pi, 16]

    /** Phi [rad]. */
    Double32_t  m_phi;  //[-pi, pi, 16]

    /** Radius [cm]. */
    Double32_t  m_r;  //[75.0, 300.0, 16]

    /**< Log. Energy [GeV]. */
    Double32_t  m_logEnergy;  //[-5, 3., 18]

    /** Log. Raw Energy [GeV]. */
    Double32_t  m_logEnergyRaw;  //[-5, 3., 18]

    /** Log. Highest Crystal Energy [GeV]. */
    Double32_t  m_logEnergyHighestCrystal;  //[-5, 3., 18]

    /** MVA classifier that uses pulse shape discrimination to identify electromagnetic vs hadronic showers. Classifier value is 1.0 EM showers and 0.0 for hadronic showers. */
    Double32_t  m_PulseShapeDiscriminationMVA;  //[0.0, 1.0, 18]

    /** Cluster Hadron Component Intensity (pulse shape discrimination variable). Sum of the CsI(Tl) hadron scintillation component emission normalized to the sum of CsI(Tl) total scintillation emission.  Computed only using cluster digits with energy greater than 50 MeV and good offline waveform fit chi2. Will be removed in release-04*/
    Double32_t  m_ClusterHadronIntensity;  //[-0.1, 0.8, 18]

    /** Number of hadron digits in cluster (pulse shape discrimination variable).  Weighted sum of digits in cluster with significant scintillation emission (> 3 MeV) in the hadronic scintillation component.*/
    Double32_t m_NumberOfHadronDigits;  //[0, 255, 18]

    /** Class definition */
    ClassDef(ECLCluster, 15);
    // 15: Added m_minTrkDistanceID
    // 14: Added m_maxECellId
    // 13: Added m_hypotheses
    // 12: Added m_PulseShapeDiscriminationMVA. Indicated that m_ClusterHadronIntensity will be removed in release-04.
    // 11: Added m_ClusterHadronIntensity an m_NumberOfHadronDigits variables
    // 10: Added status enum, added status setter
    // 9: Removed all momentum, 4x4, and 7x7 covariance matrix getters.
    // 8: Added clusterId, getUniqueId
    // 7: Changed range of SecondMoment from 0..100 to 0..40
    // 6: Changed stored variances to sqrt(covmat_ii).
    // 5: New HypothesisId default, removed relative covariance entries, renamed some setters/getters, adjusted covariance variable ranges.
    // 4: Complete revision and new variables. Introduction of Double32_t. Some new setters and getters.

  };

}// end namespace Belle2
