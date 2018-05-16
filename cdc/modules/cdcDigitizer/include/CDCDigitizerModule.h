/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, CDC group                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDIGITIZER_H
#define CDCDIGITIZER_H

//basf2 framework headers
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

//cdc package headers
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dbobjects/CDCFEEParams.h>

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>
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
    void initialize();

    /** Actual digitization of all hits in the CDC.
     *
     *  The digitized hits are written into the DataStore.
     */
    void event();

    /** Terminate func. */
    void terminate()
    {
      if (m_feeParamsFromDB) delete m_feeParamsFromDB;
    };

  private:
    /** Method used to smear the drift length.
     *
     *  @param driftLength The value of drift length.
     *  @param fraction Fraction of the first Gaussian used to smear drift length.
     *  @param mean1 Mean value of the first Gassian used to smear drift length.
     *  @param resolution1 Resolution of the first Gassian used to smear drift length.
     *  @param mean2 Mean value of the second Gassian used to smear drift length.
     *  @param resolution2 Resolution of the second Gassian used to smear drift length.
     *
     *  @return Drift length after smearing.
     */
    float smearDriftLength(float driftLength, float dDdt);


    /** The method to get dD/dt
     *
     *  In this method, X-T function will be used to calculate dD/dt (drift velocity).
     *
     *  @param driftLength The value of drift length.
     *
     *  @return dDdt.
     *
     */
    float getdDdt(float driftLength);


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
    float getDriftTime(float driftLength, bool addTof, bool addDelay);


    /** Charge to ADC Count converter. */
    unsigned short getADCCount(float charge);

    /** Set FEE parameters (from DB) */
    void setFEEParams();

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
    double m_gasToGasWire;      /**< Approx. conv. factor from dE(gas) to dE(gas+wire) */
    int m_adcThreshold;         /**< Threshold for ADC in unit of count */
    double m_tMin;              /**< Lower edge of time window in ns */
    double m_tMaxOuter;         /**< Upper edge of time window in ns for the outer layers*/
    double m_tMaxInner;         /**< Upper edge of time window in ns for the inner layers */
    //    unsigned short m_tdcOffset; /**< Offset of TDC count (in ns)*/
    double m_trigTimeJitter;   /**< Magnitude of trigger timing jitter (ns). */

    CDC::CDCGeometryPar* m_cdcgp;  /**< Pointer to CDCGeometryPar */
    CDCSimHit* m_aCDCSimHit;    /**< Pointer to CDCSimHit */
    WireID m_wireID;            /**< WireID of this hit */
    unsigned short m_posFlag;   /**< left or right flag of this hit */
    unsigned short m_boardID;   /**< FEE board ID */
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

    //--- Universal digitization parameters -------------------------------------------------------------------------------------
    bool m_doSmearing; /**< A switch to control drift length smearing */
    //    bool m_2015AprRun; /**< A flag indicates cosmic runs in April 2015. */
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

    bool m_useDB;             /**< Fetch FEE params from DB */
    DBArray<CDCFEEParams>* m_feeParamsFromDB = nullptr; /*!< Pointer to FE electronics params. from DB. */
    float m_lowEdgeOfTimeWindow[nBoards]; /*!< Lower edge of time-window */
    float m_uprEdgeOfTimeWindow[nBoards]; /*!< Upper edge of time-window */
    float m_tdcThresh          [nBoards]; /*!< Threshold for timing-signal */
    float m_adcThresh          [nBoards]; /*!< Threshold for FADC */

    /** Structure for saving the signal information. */
    struct SignalInfo {
      /** Constructor that initializes all members. */
      SignalInfo(int simHitIndex = 0, float driftTime = 0, float charge = 0, int simHitIndex2 = -1,
                 float driftTime2 = std::numeric_limits<float>::max(), int simHitIndex3 = -1, float driftTime3 = std::numeric_limits<float>::max()) :
        m_simHitIndex(simHitIndex), m_driftTime(driftTime), m_charge(charge), m_simHitIndex2(simHitIndex2), m_driftTime2(driftTime2),
        m_simHitIndex3(simHitIndex3), m_driftTime3(driftTime3) {}
      int            m_simHitIndex;   /**< SimHit Index number. */
      float          m_driftTime;     /**< Shortest drift time of any SimHit in the cell. */
      float          m_charge;        /**< Sum of charge for all SimHits in the cell. */
      int            m_simHitIndex2;   /**< SimHit index for 2nd drift time. */
      float          m_driftTime2;     /**< 2nd-shortest drift time in the cell. */
      int            m_simHitIndex3;   /**< SimHit index for 3rd drift time. */
      float          m_driftTime3;     /**< 3rd-shortest drift time in the cell. */
    };
  };

} // end of Belle2 namespace

#endif // CDCDIGITIZER_H
