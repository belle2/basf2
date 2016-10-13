/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Alon Hershenhorn (hershen@phas.ubc.ca)                   *
 *               Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWER_H
#define ECLSHOWER_H

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <math.h>

namespace Belle2 {

  /*! Class to store ECL Showers
   */

  class ECLShower : public RelationsObject {
  public:

    /** The status information for the ECLShowers. */
    enum StatusBit {
      /** bit 0:  Dead crystal within nominal shower neighbour region.  */
      c_HasDeadCrystal = 1 << 0,

      /** bit 1:  Hot crystal within nominal shower neighbour region.  */
      c_HasHotCrystal = 2 << 0,

      /** combined flag to test whether the shower is 'problematic' */
      c_HasProblematicCrystal = c_HasDeadCrystal | c_HasHotCrystal,
    };

    /** Default constructor for ROOT */
    ECLShower()
    {
      m_isTrk = false;         /**< Match with track */
      m_Status = 0;            /**< Status (e.g. shower contains one hot crystal) */
      m_ShowerId = 0;          /**< Shower ID */
      m_connectedRegionId = 0; /**< Connected Region ID */
      m_hypothesisId = 0;      /**< Hypothesis ID */
      m_centralCellId = 0;     /**< Central Cell ID */
      m_Energy = 0.0;          /**< Energy (GeV) */
      m_EnedepSum = 0.0;       /**< Raw Energy Sum (GeV) */
      m_Theta = 0.0;           /**< Theta (rad) */
      m_Phi = 0.0;             /**< Phi (rad) */
      m_R = 0.0;               /**< R (cm) */
      m_Error[0] = 0.0;        /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
      m_Error[1] = 0.0;        /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
      m_Error[2] = 0.0;        /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
      m_Error[3] = 0.0;        /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
      m_Error[4] = 0.0;        /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
      m_Error[5] = 0.0;        /**< Error Array for Energy->[0], Phi->[2], Theta->[5] */
      m_Time = 0;              /**< Time */
      m_timeResolution = 0;    /**< Time resolution */
      m_HighestEnergy = 0.0;   /**< Highest energy in Shower*/
      m_lateralEnergy = 0.0;   /**< Lateral Energy  */
      m_minTrkDistance = 0.0;  /**< Distance between shower and closest track  */
      m_trkDepth = 0.0;        /**< Path on track extrapolation to POCA to average cluster direction   */
      m_showerDepth = 0.0;     /**< Same as above, but on the cluster average direction */
      m_NofCrystals = 0.0;     /**< Sum of weights of crystals (~number of crystals) */
      m_absZernike40 = 0.0;    /**< Shower shape variable, absolute value of Zernike Moment 40 */
      m_absZernike51 = 0.0;    /**< Shower shape variable, absolute value of Zernike Moment 51 */
      m_zernikeMVA = 0.0;      /**< Shower shape variable, Zernike MVA output */
      m_secondMoment = 0.0;    /**< Shower shape variable, second moment (needed for merged pi0) */
      m_E1oE9 = 0.0;           /**< Shower shape variable, E1oE9 */
      m_E9oE25 = 0;            /**< Shower shape variable, E9oE25 */

    }

    /*! Set Match with Track
     */
    void setIsTrack(bool val) { m_isTrk = val; }

    /*! Set Status
     */
    void setStatus(int Status) { m_Status = Status; }

    /*! Set Shower ID
     */
    void setShowerId(int ShowerId) { m_ShowerId = ShowerId; }

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
    void setEnergy(double Energy) { m_Energy = Energy; }

    /*! Set Raw Energy Sum
     */
    void setEnedepsum(double EnergySum) { m_EnedepSum = EnergySum; }

    /*! Set Theta (rad)
     */
    void setTheta(double Theta) { m_Theta = Theta; }

    /*! Set Phi (rad)
     */
    void setPhi(double Phi) { m_Phi = Phi; }

    /*! Set R
     */
    void setR(double R) { m_R = R; }

    /*! Set symmetric Error Array(3x3)  for
            [0]->Error on Energy
            [2]->Error on Phi
            [5]->Error on Theta
     */
    void setError(double ErrorArray[6]) { for (unsigned int i = 0; i < 6; i++) { m_Error[i] = ErrorArray[i];} }

    /*! Set Time
     */
    void setTime(double Time) { m_Time = Time; }

    /*! Set Time Resolution
     */
    void setTimeResolution(double TimeReso) { m_timeResolution = TimeReso; }

    /*! Set Highest Energy (TF)
     */
    void setHighestEnergy(double HighestEnergy) { m_HighestEnergy = HighestEnergy; }

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
    void setNofCrystals(double nofCrystals) { m_NofCrystals = nofCrystals; }

    /*! Set absolute value of Zernike moment 40
     */
    void setAbsZernike40(double absZernike40) { m_absZernike40 = absZernike40; }

    /*! Set absolute value of Zernike moment 51
     */
    void setAbsZernike51(double absZernike51) { m_absZernike51 = absZernike51; }

    /*! SetZernike MVA value
     */
    void setZernikeMVA(double zernikeMVA) {m_zernikeMVA = zernikeMVA; }

    /*! Set second moment
     */
    void setSecondMoment(double secondMoment) { m_secondMoment = secondMoment; }

    /*! Set energy ration E1 over E9
     */
    void setE1oE9(double E1oE9) { m_E1oE9 = E1oE9; }

    /*! Set energy ration E9 over E25
     */
    void setE9oE25(double E9oE25) { m_E9oE25 = E9oE25; }

    /*! Get if matched with a Track
     * @return flag for track Matching
     */
    bool getIsTrack() const { return m_isTrk; }

    /*! Get Status
     * @return Status
     */
    int getStatus() const { return m_Status; }

    /*! Get Shower Id
     * @return Shower Id
     */
    int getShowerId() const { return m_ShowerId; }

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
    double getEnergy() const { return m_Energy; }

    /*! Get Energy Sum
     * @return Energy Sum
     */
    double getEnedepSum() const { return m_EnedepSum; }

    /*! Get Theta
     * @return Theta
     */
    double getTheta() const { return m_Theta; }

    /*! Get Phi
     * @return Phi
     */
    double getPhi() const { return m_Phi; }

    /*! Get R
     * @return R
     */
    double getR() const { return m_R; }

    /*! Get Error Array for Energy->[0], Phi->[2], Theta->[5]
     * @return Error Array for Energy->[0], Phi->[2], Theta->[5]
     */
    void getError(double  ErrorArray[6]) const
    {
      for (unsigned int i = 0; i < 6; i++) {
        ErrorArray[i] = m_Error[i];
      }
    }

    /*! Get Error of Energy
     * @return Error of Energy
     */
    double getEnergyError() const { return m_Error[0];}

    /*! Get Error of theta
     * @return Error of theta
     */
    double getThetaError() const { return  m_Error[5];}

    /*! Get Error of phi
     * @return Error of phi
     */
    double getPhiError() const {return m_Error[2];}

    /*! Get Time
     * @return Time
     */
    double getTime() const { return m_Time; }

    /*! Get Time Resolution
     * @return Time Resolution
     */
    double getTimeResolution() const { return m_timeResolution; }

    /*! Get Highest Energy in Shower
     * @return Highest Energy
     */
    double getHighestEnergy() const { return m_HighestEnergy; }

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
    double getNofCrystals() const { return m_NofCrystals; }

    /*! Get absolute value of Zernike moment 40
     * @return Absolute value of Zernike moment 40
     */
    double getAbsZernike40() const { return m_absZernike40; }

    /*! Get absolute value of Zernike moment 51
     * @return Absolute value of Zernike moment 51
     */
    double getAbsZernike51() const { return m_absZernike51; }

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

    /*! Get energy ratio E9oE25
     * @return E9oE25
     */
    double getE9oE25() const { return m_E9oE25; }

    //! The method to get return  TVector3 Momentum
    TVector3 getMomentum() const
    {
      return TVector3(
               m_Energy * sin(m_Theta) * cos(m_Phi),
               m_Energy * sin(m_Theta) * sin(m_Phi),
               m_Energy * cos(m_Theta)
             );
    }

    /**
     * Return if specific status bit is set.
     * @param bitmask The bitmask which is compared to the status of the shower.
     * @return Returns true if the bitmask matches the status code of the shower.
     */
    bool hasStatus(unsigned short int bitmask) const { return (m_Status & bitmask) == bitmask; }

    /*! Check if shower contains a hot crystal
     */
    bool hasHotCrystal() const;

    /*! Check if shower contains a dead crystal
     */
    bool hasDeadCrystal() const;

    /*! Check if shower contains a problematic crystal
     */
    bool hasProblematicCrystal() const;

  private:
    bool m_isTrk;                   /**< Match with track (GDN) */

    int m_Status;                   /**< Status */
    int m_ShowerId;                 /**< Shower ID */
    int m_connectedRegionId;        /**< Connected Region ID (TF)*/
    int m_hypothesisId;             /**< Hypothesis ID (TF)*/
    int m_centralCellId;            /**< Central cell ID (TF)*/

    Double32_t m_Energy;            /**< Energy (GeV) */
    Double32_t m_EnedepSum;         /**< Raw Energy Sum(GeV) */
    Double32_t m_Theta;             /**< Theta (rad) */
    Double32_t m_Phi;               /**< Phi (rad) */
    Double32_t m_R;                 /**< R (cm) */
    Double32_t m_Error[6];          /**< Error of Energy, Theta and Phi */
    Double32_t m_Time;              /**< Time */
    Double32_t m_timeResolution;    /**< Time resolution */
    Double32_t m_HighestEnergy;     /**< Highest Energy in Shower (GeV) (TF) */
    Double32_t m_lateralEnergy;     /**< Lateral Energy in Shower (GDN) */
    Double32_t m_minTrkDistance;    /**< Distance between shower and closest track (GDN) */
    Double32_t m_trkDepth;          /**< Path on track ext. to POCA to avg. cluster dir. (GDN) */
    Double32_t m_showerDepth;       /**< Same as above, but on the cluster average direction (GDN) */
    Double32_t m_NofCrystals;       /**< Sum of weights of crystals (~number of crystals) (TF) */
    Double32_t m_absZernike40;      /**< Shower shape variable, absolute value of Zernike Moment 40 (TF) */
    Double32_t m_absZernike51;      /**< Shower shape variable, absolute value of Zernike Moment 51 (TF) */
    Double32_t m_zernikeMVA;        /**< Shower shape variable, zernike MVA output */
    Double32_t m_secondMoment;      /**< Shower shape variable, second moment (for merged pi0) (TF) */
    Double32_t m_E1oE9;             /**< Shower shape variable, E1oE9 (TF) */
    Double32_t m_E9oE25;            /**< Shower shape variable, E9oE25 */

    // 2: added uniqueID and highestE (TF)
    // 3: added LAT and distance to closest track and trk match flag (GDN)
    // 4: added time resolution (TF)
    // 5: clean up, float to Double32_t, and new variables (TF)
    // 6: changed names of Zernike moment variables/getters/setters to indicate that they are the absolute values of the moments (TF and AH)
    // 7: added centralCellId (TF)
    // 8: added zernikeMVA, removed absZernike20, 42, 53 (AH)
    ClassDef(ECLShower, 8);/**< ClassDef */

  };

  inline bool ECLShower::hasHotCrystal() const
  {
    return hasStatus(c_HasHotCrystal);
  }

  inline bool ECLShower::hasDeadCrystal() const
  {
    return hasStatus(c_HasDeadCrystal);
  }

  inline bool ECLShower::hasProblematicCrystal() const
  {
    return hasStatus(c_HasProblematicCrystal);
  }


} // end namespace Belle2

#endif
