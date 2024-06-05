/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//basf2 framework headers
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <simulation/dataobjects/SimClockState.h>

//cdc package headers
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/CDCGeoControlPar.h>
#include <cdc/dbobjects/CDCFEElectronics.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
//#include <cdc/dbobjects/CDCEDepToADCConversions.h>
#include <cdc/dbobjects/CDCCrossTalkLibrary.h>
#include <cdc/dbobjects/CDCCorrToThresholds.h>

//C++/C standard lib elements.
#include <string>
//#include <queue>
#include <limits>

namespace Belle2 {

  /** The Class for Detailed Digitization of CDC.
   *
   *  Currently a float Gaussian with steerable parameters is used for the digitization.
   *  If there are two or more hits in one cell, only the shortest drift length is selected.
   *  The signal amplitude is the sum of all hits deposited energy in this cell.
   *
   *  @todo CDCDigitizerModule: Event time jitter needs a different approach than the one, which is taken here; Instead of adding
   *        a constant to all events, there needs to be an event by event time jitter, most likely determined by
   *        the trigger time uncertainties; <br>
   *        If the definition of the time format in the CDCHit is changed, it needs to be changed here, too, and in the SimpleDriftTimeTranslator.
   */
  class CDCDigitizerModule : public Module {

  public:
    /** Constructor.*/
    CDCDigitizerModule();

    /** Initialize variables, print info, and start CPU clock. */
    void initialize() override;

    /** Actual digitization of all hits in the CDC.
     *
     *  The digitized hits are written into the DataStore.
     */
    void event() override;

    /** Terminate func. */
    void terminate() override
    {
      if (m_fEElectronicsFromDB) delete m_fEElectronicsFromDB;
      //      if (m_eDepToADCConversionsFromDB) delete m_eDepToADCConversionsFromDB;
      if (m_runGainFromDB) delete m_runGainFromDB;
      if (m_gain0FromDB) delete m_gain0FromDB;
      if (m_wireGainFromDB) delete m_wireGainFromDB;
      if (m_xTalkFromDB) delete m_xTalkFromDB;
      if (m_corrToThresholdFromDB) delete m_corrToThresholdFromDB;
    };

  private:
    /** Method used to smear the drift length.
     *
     *  @param driftLength The value of drift length.
     *  @param dDdt        dD/dt (drift velocity).
     *  @return Drift length after smearing.
     */
    double smearDriftLength(double driftLength, double dDdt);


    /** The method to get dD/dt
     *
     *  In this method, X-T function will be used to calculate dD/dt (drift velocity).
     *
     *  @param driftLength The value of drift length.
     *
     *  @return dDdt.
     *
     */
    double getdDdt(double driftLength);


    /** The method to get drift time based on drift length
     *
     *  In this method, X-T function will be used to calculate drift time.
     *
     *  @param driftLength The value of drift length.
     *  @param addTof   Switch for adding time of flight.
     *  @param addDelay Switch for adding signal propagation delay in the wire.
     *
     *  @return Drift time.
     *
     */
    double getDriftTime(double driftLength, bool addTof, bool addDelay);


    /** Function to write ADC-count and conversion factor for threshold
     *
     *  @param wid wire id.
     *  @param edep energy deposit (GeV).
     *  @param dx step length (cm).
     *  @param costh cos(theta) of particle.
     *  @param adcCount ADC-count.
     *  @param convFactorForThreshold conversion factor needed for threshold setting.
     *
     */
    void makeSignalsAfterShapers(const WireID& wid, double edep, double dx, double costh, unsigned short& adcCount,
                                 double& convFactorForThreshold);

    /** Modify t0 for negative-t0 case */
    double getPositiveT0(const WireID&);

    /** Set FEE parameters (from DB) */
    void setFEElectronics();

    /** Generate random number according to Polya distribution
     *  @param xmax max of no. generated
     *  @return random no.
     */
    double Polya(double xmax = 10);

    /** Set semi-total gain (from DB) */
    void setSemiTotalGain();

    //! Return semi-total gain of the specified wire
    /*!
      \param clayer layer no. (0-56)
      \param cell   cell  no.
      \return       gain
    */
    double getSemiTotalGain(int clayer, int cell) const
    {
      return m_semiTotalGain[clayer][cell];
    }

    //! Return semi-total gain of the specified wire
    /*!
      \param wireID Wire id.
      \return       gain
    */
    double getSemiTotalGain(const WireID& wireID) const
    {
      return m_semiTotalGain[wireID.getICLayer()][wireID.getIWire()];
    }

    /** Add crosstalk */
    void addXTalk();

    /** Set edep-to-ADC conversion params. (from DB) */
    //    void setEDepToADCConversions();

    StoreArray<MCParticle> m_mcParticles; /**< MCParticle array */
    StoreArray<CDCSimHit>  m_simHits;     /**< CDCSimHit  array */
    StoreArray<CDCHit>     m_cdcHits;     /**< CDCHit     array */
    StoreArray<CDCHit>     m_cdcHits4Trg; /**< CDCHit4trg array */

    std::string m_inputCDCSimHitsName;       /**< Input array name.  */
    std::string m_outputCDCHitsName;         /**< Output array name. */
    std::string m_outputCDCHitsName4Trg;     /**< Output array name for trigger. */

    std::string m_MCParticlesToSimHitsName;    /**< Relation for origin of incoming SimHits. */
    std::string m_SimHitsTOCDCHitsName;      /**< Relation for outgoing CDCHits. */

    std::string m_OptionalFirstMCParticlesToHitsName;      /**< Relation name for optional matching of up to first three MCParticles. */
    std::string m_OptionalAllMCParticlesToHitsName;      /**< Relation name for optional matching of all MCParticles. */

    bool m_useSimpleDigitization;            /**< Use float Gaussian Smearing instead of proper digitization. */
    //--- Parameters for simple digitization -------------------------------------------------------------------------------------
    double m_fraction;          /**< Fraction of the first Gaussian used to smear drift length */
    double m_mean1;             /**< Mean value of the first Gaussian used to smear drift length */
    double m_resolution1;       /**< Resolution of the first Gaussian used to smear drift length */
    double m_mean2;             /**< Mean value of the second Gaussian used to smear drift length */
    double m_resolution2;       /**< Resolution of the second Gaussian used to smear drift length */
    double m_tdcThreshold4Outer; /**< TDC threshold for outer layers in unit of eV */
    double m_tdcThreshold4Inner; /**< TDC threshold for inner layers in unit of eV */
    int    m_eDepInGasMode;     /**< Mode for extracting dE(gas) from dE(gas+wire) */
    int    m_adcThreshold;      /**< Threshold for ADC in unit of count */
    double m_tMin;              /**< Lower edge of time window in ns */
    double m_tMaxOuter;         /**< Upper edge of time window in ns for the outer layers*/
    double m_tMaxInner;         /**< Upper edge of time window in ns for the inner layers */
    //    unsigned short m_tdcOffset; /**< Offset of TDC count (in ns)*/
    double m_trigTimeJitter;   /**< Magnitude of trigger timing jitter (ns). */

    CDC::CDCGeometryPar* m_cdcgp;  /**< Cached Pointer to CDCGeometryPar */
    CDC::CDCGeoControlPar* m_gcp;  /**< Cached pointer to CDCGeoControlPar */
    CDCSimHit* m_aCDCSimHit;    /**< Pointer to CDCSimHit */
    WireID m_wireID;            /**< WireID of this hit */
    unsigned short m_posFlag;   /**< left or right flag of this hit */
    unsigned short m_boardID = 0; /**< FEE board ID */
    B2Vector3D m_posWire;         /**< wire position of this hit */
    B2Vector3D m_posTrack;        /**< track position of this hit */
    B2Vector3D m_momentum;        /**< 3-momentum of this hit */
    double m_driftLength;       /**< drift length of this hit */
    double m_flightTime;        /**< flight time of this hit */
    double m_globalTime;        /**< global time of this hit */

    double m_tdcBinWidth;       /**< Width of a TDC bin (in ns)*/
    double m_tdcBinWidthInv;    /**< m_tdcBinWidth^-1 (in ns^-1)*/
    double m_tdcResol;          /**< TDC resolution (in ns)*/
    double m_driftV;            /**< Nominal drift velocity (in cm/ns)*/
    double m_driftVInv;         /**< m_driftV^-1 (in ns/cm)*/
    double m_propSpeedInv;      /**< Inv. of nominal signal propagation speed in a wire (in ns/cm)*/

    double m_tdcThresholdOffset; /**< Offset for TDC(digital) threshold (mV)*/
    double m_analogGain;         /**< analog gain (V/pC) */
    double m_digitalGain;        /**< digital gain (V/pC) */
    double m_adcBinWidth;        /**< ADC bin width (mV) */

    double m_addFudgeFactorForSigma; /**< additional fudge factor for space resol. */
    double m_totalFudgeFactor = 1.;  /**< total fudge factor for space resol. */

    bool m_gasGainSmearing = true;  /**< Switch for gas gain smearing */
    double m_effWForGasGainSmearing = 0.0266;  /**< Effective energy (keV) for one electron prod. for gas gain smearing */
    double m_thetaOfPolya = 0.5;     /**< theta of Polya function for gas gain smearing */
    bool m_extraADCSmearing = false; /**< Switch for extra ADC smearing */
    //    double m_sigmaForExtraADCSmearing = 0.3;  /**< Gaussian sigma for extra ADC smearing */
    double m_runGain = 1.;  /**< run gain. */
    float m_semiTotalGain[c_maxNSenseLayers][c_maxNDriftCells] = {{}}; /**< total gain per wire */
    double m_overallGainFactor = 1.;  /**< Overall gain factor. */
    double m_degOfSPEOnThreshold = 0; /**< Degree of space charge effect on timing threshold */
    //--- Universal digitization parameters -------------------------------------------------------------------------------------
    bool m_doSmearing; /**< A switch to control drift length smearing */
    bool m_addTimeWalk; /**< A switch used to control adding time-walk delay into the total drift time or not */
    bool m_addInWirePropagationDelay; /**< A switch used to control adding propagation delay into the total drift time or not */
    bool m_addTimeOfFlight; /**< A switch used to control adding time of flight into the total drift time or not */
    bool m_addInWirePropagationDelay4Bg; /**< A switch used to control adding propagation delay into the total drift time or not for beam bg. */
    bool m_addTimeOfFlight4Bg; /**< A switch used to control adding time of flight into the total drift time or not for beam bg. */
    bool m_outputNegativeDriftTime;     /**< A switch to output negative drift time to CDCHit */
    bool m_output2ndHit;         /**< A switch to output 2nd hit */
    bool m_align;             /**< A switch to control alignment */
    bool m_correctForWireSag;    /**< A switch to control wire sag */
    bool m_treatNegT0WiresAsGood;    /**< A switch for negative-t0 wires */
    bool m_matchFirstMCParticles;    /**< A switch to match first three MCParticles, not just the one with smallest drift time */
    bool m_matchAllMCParticles;    /**< A switch to match all particles to a hit, regardless whether they produced a hit or not */
//    float m_eventTime;         /**< It is a timing of event, which includes a time jitter due to the trigger system */

    bool m_useDB4FEE;             /**< Fetch FEE params from DB */
    DBArray<CDCFEElectronics>* m_fEElectronicsFromDB = nullptr; /*!< Pointer to FE electronics params. from DB. */
    float m_lowEdgeOfTimeWindow[c_nBoards] = {0}; /*!< Lower edge of time-window */
    float m_uprEdgeOfTimeWindow[c_nBoards] = {0}; /*!< Upper edge of time-window */
    float m_tdcThresh          [c_nBoards] = {0}; /*!< Threshold for timing-signal */
    float m_adcThresh          [c_nBoards] = {0}; /*!< Threshold for FADC */
    unsigned short m_widthOfTimeWindowInCount  [c_nBoards] = {0}; /*!< Width of time window */

    bool m_useDB4EDepToADC;             /**< Fetch edep-to-ADC conversion params. from DB */
    bool m_useDB4RunGain;               /**< Fetch run gain from DB */
    bool m_spaceChargeEffect;           /**< Space charge effect */

    DBObjPtr<CDCDedxRunGain>* m_runGainFromDB = nullptr; /*!< Pointer to run gain from DB. */
    DBObjPtr<CDCDedxScaleFactor>* m_gain0FromDB = nullptr; /*!< Pointer to overall gain factor from DB. */
    DBObjPtr<CDCDedxWireGain>* m_wireGainFromDB = nullptr; /*!< Pointer to wire gain from DB. */
    //    DBObjPtr<CDCEDepToADCConversions>* m_eDepToADCConversionsFromDB = nullptr; /*!< Pointer to edep-to-ADC conv. params. from DB. */
    //    float m_eDepToADCParams[c_maxNSenseLayers][4]; /*!< edep-to-ADC conv. params. */

    bool m_addXTalk;           /**< Flag to switch on/off crosstalk */
    bool m_issue2ndHitWarning; /**< Flag to switch on/off a warning on the 2nd TDC hit */
    bool m_includeEarlyXTalks; /**< Flag to switch on/off xtalks earlier than the hit */
    int  m_debugLevel      ;   /**< Debug level */
    int  m_debugLevel4XTalk;   /**< Debug level for crosstalk */
    DBObjPtr<CDCCrossTalkLibrary>* m_xTalkFromDB = nullptr; /*!< Pointer to cross-talk from DB. */
    DBObjPtr<CDCCorrToThresholds>* m_corrToThresholdFromDB = nullptr; /*!< Pointer to threshold correction from DB. */

    StoreObjPtr<SimClockState> m_simClockState; /**< generated hardware clock state */
    bool m_synchronization = true; /**< Flag to switch on/off timing synchronization */
    bool m_randomization = true; /**< Flag to switch on/off timing randomization */
    int m_tSimMode = 0; /**< Timing simulation mode */
    int m_offsetForTriggerBin = 1; /**< Input to getCDCTriggerBin(offset) */
    int m_trgTimingOffsetInCount   = 4; /**< Trigger timing offset in unit of count */
    int m_shiftOfTimeWindowIn32Count = 153; /**< Shift of time window for synchronization in 32count */
    unsigned short m_trgDelayInCount[c_nBoards] = {0}; /**< Trigger delay in frontend electronics in count */

    /** Structure for saving the signal information. */
    struct SignalInfo {
      /** Constructor that initializes all members. */
      SignalInfo(int simHitIndex = 0, float driftTime = 0, float charge = 0, float maxDriftL = 0, float minDriftL = 0,
                 int simHitIndex2 = -1,
                 float driftTime2 = std::numeric_limits<float>::max(), int simHitIndex3 = -1, float driftTime3 = std::numeric_limits<float>::max()) :
        m_simHitIndex(simHitIndex), m_driftTime(driftTime), m_charge(charge), m_maxDriftL(maxDriftL), m_minDriftL(minDriftL),
        m_simHitIndex2(simHitIndex2), m_driftTime2(driftTime2),
        m_simHitIndex3(simHitIndex3), m_driftTime3(driftTime3) {}
      int            m_simHitIndex;   /**< SimHit Index number. */
      float          m_driftTime;     /**< Shortest drift time of any SimHit in the cell. */
      float          m_charge;        /**< Sum of charge for all SimHits in the cell. */
      float          m_maxDriftL;      /**< Max of drift length. */
      float          m_minDriftL;      /**< Min of drift length. */
      int            m_simHitIndex2;   /**< SimHit index for 2nd drift time. */
      float          m_driftTime2;     /**< 2nd-shortest drift time in the cell. */
      int            m_simHitIndex3;   /**< SimHit index for 3rd drift time. */
      float          m_driftTime3;     /**< 3rd-shortest drift time in the cell. */
    };

    /** Structure for saving the x-talk information. */
    struct XTalkInfo {
      /** Constructor that initializes all members. */
      XTalkInfo(unsigned short tdc, unsigned short adc, unsigned short tot, unsigned short status) :
        m_tdc(tdc), m_adc(adc), m_tot(tot), m_status(status) {}
      unsigned short m_tdc; /**< TDC count */
      unsigned short m_adc; /**< ADC count */
      unsigned short m_tot; /**< TOT       */
      unsigned short m_status; /**< status */
    };
  };

} // end of Belle2 namespace
