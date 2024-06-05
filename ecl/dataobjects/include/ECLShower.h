/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/datastore/RelationsObject.h>

/* ROOT headers. */
#include <Math/Vector3D.h>
#include <TMatrixDSym.h>

/* C++ headers. */
#include <cmath>

namespace Belle2 {

  /*! Class to store ECL Showers
   */

  class ECLShower : public RelationsObject {
  public:

    /** The hypothesis ID for  ECLShowers. Unlike ECLClusters, ECLShowers have one and only one hypothesis ID. */
    enum Hypothesis : unsigned int {
      /** CR is split into a muon and n photons (T1) */
      c_muonNPhotons = 1,
      /** CR is reconstructed as a charged hadron (T2) */
      c_chargedHadron = 2,
      /** CR is split into an electron and n photons (T3) */
      c_electronNPhotons = 3,
      /** CR is split into n photons (N1) */
      c_nPhotons = 5,
      /** CR is reconstructed as a neutral hadron (N2) */
      c_neutralHadron = 6,
      /** CR is reconstructed as merged pi0 (N3) */
      c_mergedPi0 = 7
    };

    /** The status information for the ECLShowers. */
    enum StatusBit : unsigned int  {
      /** bit 0:  Dead crystal within nominal shower neighbour region.  */
      c_hasDeadCrystal = 1 << 0,

      /** bit 1:  Hot crystal within nominal shower neighbour region.  */
      c_hasHotCrystal = 1 << 1,

      /** combined flag to test whether the shower is 'problematic' */
      c_hasProblematicCrystal = c_hasDeadCrystal | c_hasHotCrystal,

      /** bit 2:  Shower has pulse shape discrimination variables.  */
      c_hasPulseShapeDiscrimination = 1 << 2,

    };

    /** Default constructor for ROOT */
    ECLShower() :
      m_isTrk(false),                          /**< Match with track */
      m_status(0),                             /**< Status (e.g. shower contains one hot crystal) */
      m_showerId(0),                           /**< Shower ID */
      m_connectedRegionId(0),                  /**< Connected Region ID */
      m_hypothesisId(0),                       /**< Hypothesis ID */
      m_centralCellId(0),                      /**< Central Cell ID */
      m_energy(0.0),                           /**< Energy (GeV) */
      m_energyRaw(0.0),                        /**< Raw Energy Sum (GeV) */
      m_theta(0.0),                            /**< Theta (rad) */
      m_phi(0.0),                              /**< Phi (rad) */
      m_r(0.0),                                /**< R (cm) */
      m_Error{0.0, 0.0, 0.0, 0.0, 0.0, 0.0},   /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
      m_time(0),                               /**< Time */
      m_deltaTime99(0),                        /**< Time that contains 99% of true signals */
      m_energyHighestCrystal(0.0),             /**< Highest energy in Shower*/
      m_lateralEnergy(0.0),                    /**< Lateral Energy  */
      m_minTrkDistance(0.0),                   /**< Distance between shower and closest track  */
      m_trkDepth(0.0),                         /**< Path on track extrapolation to POCA to average cluster direction   */
      m_showerDepth(0.0),                      /**< Same as above, but on the cluster average direction */
      m_numberOfCrystals(0.0),                 /**< Sum of weights of crystals (~number of crystals) */
      m_absZernikeMoments{ -999.0},            /**< Shower shape variables, absolute values of Zernike Moments 10 to 55 */
      m_zernikeMVA(0.0),                       /**< Shower shape variable, Zernike MVA output */
      m_secondMoment(0.0),                     /**< Shower shape variable, second moment (needed for merged pi0) */
      m_E1oE9(0.0),                            /**< Shower shape variable, E1oE9 */
      m_E9oE21(0.0),                           /**< Shower shape variable, E9oE21 */
      m_ShowerHadronIntensity(0.0),            /**< Shower Hadron Intensity. Will be removed in release-04.*/
      m_PulseShapeDiscriminationMVA(0.5),      /**< Digit level MVA classifier that uses pulse shape discrimination.*/
      m_NumberOfHadronDigits(0.0),             /**< Shower Number of hadron digits*/
      m_numberOfCrystalsForEnergy(0.0),        /**< number of crystals used for energy calculation*/
      m_nominalNumberOfCrystalsForEnergy(0.0), /**< nominal number of crystals used for energy calculation*/
      m_nOptimalGroupIndex(-1),                /**< index of group of crystals used to find m_nominalNumberOfCrystalsForEnergy */
      m_nOptimalEnergyBin(-1),                      /**< energy bin used to find m_nominalNumberOfCrystalsForEnergy */
      m_nOptimalEnergy(0.0),                    /**< energy used to find nOptimalEnergyBin in ECLSplitterN1 */
      m_listOfCrystalsForEnergy{},             /**< list of cell ids used for energy calculation*/
      m_listOfCrystalEnergyRankAndQuality{}    /**< list of ECLCalDigit indices sorted by online energy*/

    { }

    /*! Set Match with Track
     */
    void setIsTrack(bool val) { m_isTrk = val; }

    /*! Set Status
     */
    void setStatus(int Status) { m_status = Status; }

    /*! Set Shower ID
     */
    void setShowerId(int ShowerId) { m_showerId = ShowerId; }

    /*! Set Connected region ID
     */
    void setConnectedRegionId(int connectedRegionId) { m_connectedRegionId = connectedRegionId; }

    /*! Set Hypothesis identifier
     */
    void setHypothesisId(int hypothesisId) { m_hypothesisId = hypothesisId; }

    /*! Set central cell id
     */
    void setCentralCellId(int centralCellId) { m_centralCellId = centralCellId; }

    /*! Set Energy
     */
    void setEnergy(double Energy) { m_energy = Energy; }

    /*! Set Raw Energy Sum
     */
    void setEnergyRaw(double EnergySum) { m_energyRaw = EnergySum; }

    /*! Set Theta (rad)
     */
    void setTheta(double Theta) { m_theta = Theta; }

    /*! Set Phi (rad)
     */
    void setPhi(double Phi) { m_phi = Phi; }

    /*! Set R
     */
    void setR(double R) { m_r = R; }

    /*! Set symmetric Error Array(3x3)  for
            [0]->Error on Energy
            [2]->Error on Phi
            [5]->Error on Theta
     */
    void setCovarianceMatrix(double covArray[6])
    {
      for (unsigned int i = 0; i < 6; i++) {
        m_Error[i] = covArray[i];
      }
    }

    /*! Set Time
     */
    void setTime(double Time) { m_time = Time; }

    /*! Set Time Resolution
     */
    void setDeltaTime99(double TimeReso) { m_deltaTime99 = TimeReso; }

    /*! Set Highest Energy
     */
    void setEnergyHighestCrystal(double HighestEnergy) { m_energyHighestCrystal = HighestEnergy; }

    /*! Set Lateral Energy
     */
    void setLateralEnergy(double lateralEnergy) { m_lateralEnergy = lateralEnergy; }

    /*! Set Distance to closest track
     */
    void setMinTrkDistance(double dist) { m_minTrkDistance = dist; }

    /*! Set path on track extrapolation line to POCA to average cluster direction
     */
    void setTrkDepth(double trkDepth) { m_trkDepth = trkDepth; }

    /*! Set path on the average cluster direction
     */
    void setShowerDepth(double showerDepth) { m_showerDepth = showerDepth; }

    /*! Set sum of weights of crystals
     */
    void setNumberOfCrystals(double nofCrystals) { m_numberOfCrystals = nofCrystals; }

    /*! Set absolute value of Zernike Moment nm, for nm between 10 and 55
     */
    void setAbsZernikeMoment(unsigned int n, unsigned int m, double absZernikeMoment) { m_absZernikeMoments[(n * (n + 1)) / 2 + m - 1] = absZernikeMoment; }

    /*! SetZernike MVA value
     */
    void setZernikeMVA(double zernikeMVA) {m_zernikeMVA = zernikeMVA; }

    /*! Set second moment
     */
    void setSecondMoment(double secondMoment) { m_secondMoment = secondMoment; }

    /*! Set energy ration E1 over E9
     */
    void setE1oE9(double E1oE9) { m_E1oE9 = E1oE9; }

    /*! Set energy ration E9 over E21
     */
    void setE9oE21(double E9oE21) { m_E9oE21 = E9oE21; }

    /*! Set shower hadron intensity
     */
    void setShowerHadronIntensity(double hadronIntensity) { m_ShowerHadronIntensity = hadronIntensity; }

    /*! Set shower hadron intensity
     */
    void setPulseShapeDiscriminationMVA(double mvaVal) { m_PulseShapeDiscriminationMVA = mvaVal; }

    /*! Set numver of hadron digits
     */
    void setNumberOfHadronDigits(double NumberOfHadronDigits) { m_NumberOfHadronDigits = NumberOfHadronDigits; }

    /*! Set number of crystals used for energy calculation
     */
    void setNumberOfCrystalsForEnergy(double numberOfCrystalsForEnergy) { m_numberOfCrystalsForEnergy = numberOfCrystalsForEnergy; }

    /*! Set nominal number of crystals used for energy calculation
     */
    void setNominalNumberOfCrystalsForEnergy(double nominalNumberOfCrystalsForEnergy) { m_nominalNumberOfCrystalsForEnergy = nominalNumberOfCrystalsForEnergy; }

    /*! Set group number used to find nominalNumberOfCrystalsForEnergy
     */
    void setNOptimalGroupIndex(int nOptimalGroupIndex) { m_nOptimalGroupIndex = nOptimalGroupIndex; }

    /*! Set energy bin number used to find nominalNumberOfCrystalsForEnergy
     */
    void setNOptimalEnergyBin(int nOptimalEnergyBin) { m_nOptimalEnergyBin = nOptimalEnergyBin; }

    /*! Set energy used to find nOptimalEnergyBin
     */
    void setNOptimalEnergy(double nOptimalEnergy) { m_nOptimalEnergy = nOptimalEnergy; }

    /*! Set list of cell ids used for energy calculation
     */
    void setListOfCrystalsForEnergy(const std::vector<unsigned int>& listofcrystals) { m_listOfCrystalsForEnergy = listofcrystals;}

    /*! Set list of indexes of related ECLCalDigit objects sorted by calibrated energy with flag for PSD useability for charged PID.
     */
    void setListOfCrystalEnergyRankAndQuality(std::vector<std::pair<unsigned int, bool>>
                                              listOfCrystalEnergyRankAndQuality) {m_listOfCrystalEnergyRankAndQuality = listOfCrystalEnergyRankAndQuality;}

    /*! Get if matched with a Track
     * @return flag for track Matching
     */
    bool getIsTrack() const { return m_isTrk; }

    /*! Get Status
     * @return Status
     */
    int getStatus() const { return m_status; }

    /*! Get Shower Id
     * @return Shower Id
     */
    int getShowerId() const { return m_showerId; }

    /*! Get Connected region Id
     * @return Connected region Id
     */
    int getConnectedRegionId() const { return m_connectedRegionId; }

    /*! Get Hypothesis Id
     * @return Hypothesis Id
     */
    int getHypothesisId() const { return m_hypothesisId; }

    /*! Get central cell Id
     * @return central cell Id
     */
    int getCentralCellId() const { return m_centralCellId; }

    /*! Get Energy
     * @return Energy
     */
    double getEnergy() const { return m_energy; }

    /*! Get Energy Sum
     * @return Energy Sum
     */
    double getEnergyRaw() const { return m_energyRaw; }

    /*! Get Theta
     * @return Theta
     */
    double getTheta() const { return m_theta; }

    /*! Get Phi
     * @return Phi
     */
    double getPhi() const { return m_phi; }

    /*! Get R
     * @return R
     */
    double getR() const { return m_r; }

    /*! Get Error Array for Energy->[0], Phi->[2], Theta->[5]
     * @param covArray array which gets filled with errors for Energy->[0], Phi->[2], Theta->[5]
     */
    void getCovarianceMatrixAsArray(double covArray[6]) const
    {
      for (unsigned int i = 0; i < 6; i++) {
        covArray[i] = m_Error[i];
      }
    }

    /*! Get Error of Energy
     * @return Error of Energy
     */
    double getUncertaintyEnergy() const { return sqrt(m_Error[0]);}

    /*! Get Error of theta
     * @return Error of theta
     */
    double getUncertaintyTheta() const { return  sqrt(m_Error[5]);}

    /*! Get Error of phi
     * @return Error of phi
     */
    double getUncertaintyPhi() const {return sqrt(m_Error[2]);}

    /*! Get Time
     * @return Time
     */
    double getTime() const { return m_time; }

    /*! Get Time Resolution
     * @return deltat99
     */
    double getDeltaTime99() const { return m_deltaTime99; }

    /*! Get Highest Energy in Shower
     * @return Highest Energy
     */
    double getEnergyHighestCrystal() const { return m_energyHighestCrystal; }

    /*! Get Lateral Energy in Shower
     * @return Lateral Energy
     */
    double getLateralEnergy() const { return m_lateralEnergy; }

    /*! Get distance to closest Track
     * @return distance to closest Track
     */
    double getMinTrkDistance() const { return m_minTrkDistance; }

    /*! path on track extrapolation to POCA to average cluster direction
     * @return path lenght in cm
     */
    double getTrkDepth() const { return m_trkDepth; }

    /*! path on track extrapolation to POCA to average cluster direction
     * @return path lenght in cm starting from cluster center
     */
    double getShowerDepth() const { return m_showerDepth; }

    /*! Get NofCrystals
     * @return NofCrystals
     */
    double getNumberOfCrystals() const { return m_numberOfCrystals; }

    /*! Get absolute value of Zernike Moment nm
     * @return Absolute Value of Zernike Moment nm, for nm between 10 and 55
     */
    double getAbsZernikeMoment(unsigned int n, unsigned int m) const { return m_absZernikeMoments[(n * (n + 1)) / 2 + m - 1]; }

    /*! Get Zernike MVA
    * @return Zernike MVA
    */
    double getZernikeMVA() const {return m_zernikeMVA; }

    /*! Get second moment
     * @return second moment
     */
    double getSecondMoment() const { return m_secondMoment; }

    /*! Get energy ratio E1oE9
     * @return E1oE9
     */
    double getE1oE9() const { return m_E1oE9; }

    /*! Get energy ratio E9oE21
     * @return E9oE21
     */
    double getE9oE21() const { return m_E9oE21; }

    /*! Get shower hadron intensity
     * @return m_ShowerHadronIntensity
     */
    double getShowerHadronIntensity() const { return m_ShowerHadronIntensity; }

    /*! Get shower hadron intensity
     * @return m_PulseShapeDiscriminationMVA
     */
    double getPulseShapeDiscriminationMVA() const { return m_PulseShapeDiscriminationMVA; }

    /*! Get number of hadron digits
     * @return m_NumberOfHadronDigits
     */
    double getNumberOfHadronDigits() const { return m_NumberOfHadronDigits; }

    /*! Get number of crystals used for energy calculation
     * @return m_numberOfCrystalsForEnergy
     */
    double getNumberOfCrystalsForEnergy() const { return m_numberOfCrystalsForEnergy; }

    /*! Get nominal number of crystals used for energy calculation
     * @return m_nominalNumberOfCrystalsForEnergy
     */
    double getNominalNumberOfCrystalsForEnergy() const { return m_nominalNumberOfCrystalsForEnergy; }

    /*! Get index of group of crystals used to find nominalNumberOfCrystalsForEnergy
     * @return m_nOptimalGroupIndex
     */
    int getNOptimalGroupIndex() const { return m_nOptimalGroupIndex; }

    /*! Get energy bin used to find nominalNumberOfCrystalsForEnergy
     * @return m_nOptimalEnergyBin
     */
    int getNOptimalEnergyBin() const { return m_nOptimalEnergyBin; }

    /*! Get energy used to find nOptimalEnergyBin
     * @return m_nOptimalEnergy
     */
    double getNOptimalEnergy() const { return m_nOptimalEnergy; }

    /*! Get list of cellids used for energy calculation
     * @return m_listOfCrystalsForEnergy
     */
    std::vector<unsigned int>& getListOfCrystalsForEnergy()  { return m_listOfCrystalsForEnergy; }

    /*! Get list of indexes of related ECLCalDigit objects sorted by calibrated energy with flag for PSD useability for charged PID.
     *  @return m_listOfCrystalEnergyRankAndQuality
     */
    std::vector<std::pair<unsigned int, bool>> getListOfCrystalEnergyRankAndQuality() {return m_listOfCrystalEnergyRankAndQuality; }



    //! The method to get return  ROOT::Math::XYZVector Momentum
    ROOT::Math::XYZVector getMomentum() const
    {
      return ROOT::Math::XYZVector(
               m_energy * std::sin(m_theta) * std::cos(m_phi),
               m_energy * std::sin(m_theta) * std::sin(m_phi),
               m_energy * std::cos(m_theta)
             );
    }

    /** Return TMatrixDsym 3x3 covariance matrix for E, Phi and Theta */
    TMatrixDSym getCovarianceMatrix3x3() const
    {
      TMatrixDSym covmatecl(3);
      covmatecl(0, 0) = m_Error[0];
      covmatecl(1, 0) = m_Error[1];
      covmatecl(1, 1) = m_Error[2];
      covmatecl(2, 0) = m_Error[3];
      covmatecl(2, 1) = m_Error[4];
      covmatecl(2, 2) = m_Error[5];

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
      if (theta < 2.2462387) return 2;   // < 128.7deg
      if (theta < 2.2811453) return 13;   // < 130.7deg
      if (theta < 2.7070057) return 3;   // < 155.1deg
      else return 0;
    }

    /**
     * Return if specific status bit is set.
     * @param bitmask The bitmask which is compared to the status of the shower.
     * @return Returns true if the bitmask matches the status code of the shower.
     */
    bool hasStatus(unsigned short int bitmask) const { return (m_status & bitmask) == bitmask; }

    /**
     * Add bitmask to current status.
     * @param bitmask The status code which should be added.
     */
    void addStatus(unsigned short int bitmask) { m_status |= bitmask; }

    /*! Check if shower contains a hot crystal
     */
    bool hasHotCrystal() const;

    /*! Check if shower contains a dead crystal
     */
    bool hasDeadCrystal() const;

    /*! Check if shower contains a problematic crystal
     */
    bool hasProblematicCrystal() const;

    /*! Check if shower contains pulse shape discrimination information
     */
    bool hasPulseShapeDiscrimination() const;

    /** Return unique identifier */
    int getUniqueId() const
    {
      return 100000 * m_connectedRegionId + 1000 * m_hypothesisId + m_showerId;
    }


  private:
    bool m_isTrk;                   /**< Match with track (GDN) */

    int m_status;                   /**< Status */
    int m_showerId;                 /**< Shower ID */
    int m_connectedRegionId;        /**< Connected Region ID (TF)*/
    int m_hypothesisId;             /**< Hypothesis ID (TF)*/
    int m_centralCellId;            /**< Central cell ID (TF)*/

    Double32_t m_energy;            /**< Energy (GeV) */
    Double32_t m_energyRaw;         /**< Raw Energy Sum(GeV) */
    Double32_t m_theta;             /**< Theta (rad) */
    Double32_t m_phi;               /**< Phi (rad) */
    Double32_t m_r;                 /**< R (cm) */
    Double32_t m_Error[6];          /**< Error of Energy, Theta and Phi */
    Double32_t m_time;              /**< Time */
    Double32_t m_deltaTime99;       /**< Time that contains 99% of signal crystals */
    Double32_t m_energyHighestCrystal;     /**< Highest Energy in Shower (GeV) (TF) */
    Double32_t m_lateralEnergy;     /**< Lateral Energy in Shower (GDN) */
    Double32_t m_minTrkDistance;    /**< Distance between shower and closest track (GDN) */
    Double32_t m_trkDepth;          /**< Path on track ext. to POCA to avg. cluster dir. (GDN) */
    Double32_t m_showerDepth;       /**< Same as above, but on the cluster average direction (GDN) */
    Double32_t m_numberOfCrystals;       /**< Sum of weights of crystals (~number of crystals) (TF) */
    Double32_t m_absZernikeMoments[20];  /**< Shower shape variables, absolute values of Zernike Moments (MH) */
    Double32_t m_zernikeMVA;        /**< Shower shape variable, zernike MVA output */
    Double32_t m_secondMoment;      /**< Shower shape variable, second moment (for merged pi0) (TF) */
    Double32_t m_E1oE9;             /**< Shower shape variable, E1oE9 (TF) */
    Double32_t m_E9oE21;            /**< Shower shape variable, E9oE25 */
    Double32_t
    m_ShowerHadronIntensity;        /**< Shower Hadron Component Intensity (pulse shape discrimination variable). Sum of the CsI(Tl) hadron scintillation component emission normalized to the sum of CsI(Tl) total scintillation emission.  Computed only using showers digits with energy greater than 50 MeV and good offline waveform fit chi2. (SL) */
    Double32_t
    m_PulseShapeDiscriminationMVA;        /**< MVA classifier that uses pulse shape discrimination to identify electromagnetic vs hadronic showers. */
    Double32_t
    m_NumberOfHadronDigits;         /**< Number of hadron digits in shower (pulse shape discrimination variable).  Weighted sum of digits in shower with significant scintillation emission (> 3 MeV) in the hadronic scintillation component. (SL)*/
    Double32_t m_numberOfCrystalsForEnergy; /**< number of crystals used for energy calculation (TF)*/
    Double32_t m_nominalNumberOfCrystalsForEnergy; /**< number of crystals used for energy calculation (TF)*/
    int m_nOptimalGroupIndex; /**< group (of crystals) number used to find nominalNumberOfCrystalsForEnergy (CH) */
    int m_nOptimalEnergyBin; /**< energy bin used to find nominalNumberOfCrystalsForEnergy (CH) */
    Double32_t m_nOptimalEnergy; /**< energy used in ECLSplitterN1 to find nOptimalEnergyBin (CH) */
    std::vector<unsigned int> m_listOfCrystalsForEnergy; /**< list of cell ids used for energy calculation (TF)*/
    std::vector<std::pair<unsigned int, bool>>
                                            m_listOfCrystalEnergyRankAndQuality; /**< list of indices of related ECLCalDigits by energy. Also stores a quality flag for each digit denoting whether the PSD information can be used for charged particle ID. Cached here to avoid resorting the ECLCalDigit vector 90 times per track.*/

    // 2: added uniqueID and highestE (TF)
    // 3: added LAT and distance to closest track and trk match flag (GDN)
    // 4: added time resolution (TF)
    // 5: clean up, float to Double32_t, and new variables (TF)
    // 6: changed names of Zernike moment variables/getters/setters to indicate that they are the absolute values of the moments (TF and AH)
    // 7: added centralCellId (TF)
    // 8: added zernikeMVA, removed absZernike20, 42, 53 (AH)
    // 9: renamed variables according to the new mdst scheme (TF)
    // 10: added getUniqueId()
    // 11: added m_ShowerHadronIntensity and m_NumberOfHadronDigits variables (SL)
    // 12: added m_PulseShapeDiscriminationMVA.  Noted m_ShowerHadronIntensity will be removed in release-04 (SL)
    // 13: made enums strongly typed
    // 14: added m_numberOfCrystalsForEnergy of crystals for energy determination
    // 15: added m_listOfCrystalsForEnergy, m_nominalNumberOfCrystalsForEnergy
    // 16: removed m_absZernike40 and 51, added m_absZernikeMoments, m_listOfCrystalEnergyRankAndQuality (MH)
    // 17: added m_nOptimalGroupIndex, m_nOptimalEnergyBin, and m_nOptimalEnergy (CH)
    ClassDef(ECLShower, 17);/**< ClassDef */

  };

  inline bool ECLShower::hasHotCrystal() const
  {
    return hasStatus(c_hasHotCrystal);
  }

  inline bool ECLShower::hasDeadCrystal() const
  {
    return hasStatus(c_hasDeadCrystal);
  }

  inline bool ECLShower::hasProblematicCrystal() const
  {
    return hasStatus(c_hasProblematicCrystal);
  }

  inline bool ECLShower::hasPulseShapeDiscrimination() const
  {
    return hasStatus(c_hasPulseShapeDiscrimination);
  }

} // end namespace Belle2

