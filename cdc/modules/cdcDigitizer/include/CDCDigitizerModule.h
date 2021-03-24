/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, CDC group                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
//#include <cdc/dbobjects/CDCEDepToADCConversions.h>
#include <cdc/dbobjects/CDCCrossTalkLibrary.h>

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
      if (m_xTalkFromDB) delete m_xTalkFromDB;
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


    /** Edep to ADC Count converter */
    //    unsigned short getADCCount(unsigned short layer, unsigned short cell, double edep, double dx, double costh);
    unsigned short getADCCount(const WireID& wid, double edep, double dx, double costh);

    /** Set FEE parameters (from DB) */
    void setFEElectronics();

    /** Set run-gain (from DB) */
    void setRunGain();

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

    bool m_useSimpleDigitization;            /**< Use float Gaussian Smearing instead of proper digitization. */
    //--- Paramters for simple digitization -------------------------------------------------------------------------------------
    double m_fraction;          /**< Fraction of the first Gaussian used to smear drift length */
    double m_mean1;             /**< Mean value of the first Gassian used to smear drift length */
    double m_resolution1;       /**< Resolution of the first Gassian used to smear drift length */
    double m_mean2;             /**< Mean value of the second Gassian used to smear drift length */
    double m_resolution2;       /**< Resolution of the second Gassian used to smear drift length */
    double m_tdcThreshold4Outer; /**< TDC threshold for outer layers in unit of eV */
    double m_tdcThreshold4Inner; /**< TDC threshold for inner layers in unit of eV */
    double m_gasToGasWire;      /**< Approx. ratio of dE(gas) to dE(gas+wire) */
    double m_scaleFac = 1.;     /**< Factor to mutiply to edep */
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
    TVector3 m_posWire;         /**< wire position of this hit */
    TVector3 m_posTrack;        /**< track position of this hit */
    TVector3 m_momentum;        /**< 3-momentum of this hit */
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

    double m_runGain = 1.;  /**< run gain. */
    double m_overallGainFactor = 1.;  /**< Overall gain factor. */
    //--- Universal digitization parameters -------------------------------------------------------------------------------------
    bool m_doSmearing; /**< A switch to control drift length smearing */
    bool m_addTimeWalk; /**< A switch used to control adding time-walk delay into the total drift time or not */
    bool m_addInWirePropagationDelay; /**< A switch used to control adding propagation delay into the total drift time or not */
    bool m_addTimeOfFlight;     /**< A switch used to control adding time of flight into the total drift time or not */
    bool m_addInWirePropagationDelay4Bg; /**< A switch used to control adding propagation delay into the total drift time or not for beam bg. */
    bool m_addTimeOfFlight4Bg;     /**< A switch used to control adding time of flight into the total drift time or not for beam bg. */
    bool m_outputNegativeDriftTime;     /**< A switch to output negative drift time to CDCHit */
    bool m_output2ndHit;         /**< A switch to output 2nd hit */
    bool m_align;             /**< A switch to control alignment */
    bool m_correctForWireSag;    /**< A switch to control wire sag */
//    float m_eventTime;         /**< It is a timing of event, which includes a time jitter due to the trigger system */

    bool m_useDB4FEE;             /**< Fetch FEE params from DB */
    DBArray<CDCFEElectronics>* m_fEElectronicsFromDB = nullptr; /*!< Pointer to FE electronics params. from DB. */
    float m_lowEdgeOfTimeWindow[nBoards] = {0}; /*!< Lower edge of time-window */
    float m_uprEdgeOfTimeWindow[nBoards] = {0}; /*!< Upper edge of time-window */
    float m_tdcThresh          [nBoards] = {0}; /*!< Threshold for timing-signal */
    float m_adcThresh          [nBoards] = {0}; /*!< Threshold for FADC */
    unsigned short m_widthOfTimeWindowInCount  [nBoards] = {0}; /*!< Width of time window */

    bool m_useDB4EDepToADC;             /**< Fetch edep-to-ADC conversion params. from DB */
    bool m_useDB4RunGain;               /**< Fetch run gain from DB */
    bool m_spaceChargeEffect;           /**< Space charge effect */

    DBObjPtr<CDCDedxRunGain>* m_runGainFromDB = nullptr; /*!< Pointer to run gain from DB. */
    //    DBObjPtr<CDCEDepToADCConversions>* m_eDepToADCConversionsFromDB = nullptr; /*!< Pointer to edep-to-ADC conv. params. from DB. */
    //    float m_eDepToADCParams[MAX_N_SLAYERS][4]; /*!< edep-to-ADC conv. params. */

    bool m_addXTalk;           /**< Flag to switch on/off crosstalk */
    bool m_issue2ndHitWarning; /**< Flag to switch on/off a warning on the 2nd TDC hit */
    bool m_includeEarlyXTalks; /**< Flag to switch on/off xtalks earlier than the hit */
    int  m_debugLevel      ;   /**< Debug level */
    int  m_debugLevel4XTalk;   /**< Debug level for crosstalk */
    DBObjPtr<CDCCrossTalkLibrary>* m_xTalkFromDB = nullptr; /*!< Pointer to cross-talk from DB. */

    StoreObjPtr<SimClockState> m_simClockState; /**< generated hardware clock state */
    bool m_synchronization = true; /**< Flag to switch on/off timing synchronization */
    bool m_randomization = true; /**< Flag to switch on/off timing randmization */
    int m_tSimMode = 0; /**< Timing simulation mode */
    int m_offsetForTriggerBin = 1; /**< Input to getCDCTriggerBin(offset) */
    int m_trgTimingOffsetInCount   = 4; /**< Trigger timing offset in unit of count */
    int m_shiftOfTimeWindowIn32Count = 153; /**< Shift of time window for synchronization in 32count */
    unsigned short m_trgDelayInCount[nBoards] = {0}; /**< Trigger delay in frontend electronics in count */

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
