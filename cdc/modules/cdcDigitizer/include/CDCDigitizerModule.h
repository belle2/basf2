/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDIGITIZER_H
#define CDCDIGITIZER_H

//basf2 framework headers
#include <framework/core/Module.h>

//cdc package headers
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>

namespace Belle2 {

  /** The Class for Detailed Digitization of CDC.
   *
   *  Currently a float Gaussian with steerable parameters is used for the digitization.
   *  If there are two or more hits in one cell, only the shortest drift length is selected.
   *  The signal amplitude is the sum of all hits deposited energy in this cell.
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

    std::string m_inputCDCSimHitsName;       /**< Input array name.  */
    std::string m_outputCDCHitsName;         /**< Output array name. */

    std::string m_MCParticlesToSimHitsName;    /**< Relation for origin of incoming SimHits. */
    std::string m_SimHitsTOCDCHitsName;      /**< Relation for outgoing CDCHits. */

    bool m_useSimpleDigitization;            /**< Use float Gaussian Smearing instead of proper digitization. */
    //--- Paramters for simple digitization -------------------------------------------------------------------------------------
    double m_fraction;          /**< Fraction of the first Gaussian used to smear drift length */
    double m_mean1;             /**< Mean value of the first Gassian used to smear drift length */
    double m_resolution1;       /**< Resolution of the first Gassian used to smear drift length */
    double m_mean2;             /**< Mean value of the second Gassian used to smear drift length */
    double m_resolution2;       /**< Resolution of the second Gassian used to smear drift length */
    double m_tdcThreshold;      /**< dEdx value for TDC Threshold in unit of eV */
    double m_tMin;              /**< Lower edge of time window in ns */
    double m_tMaxOuter;         /**< Upper edge of time window in ns for the outer layers*/
    double m_tMaxInner;         /**< Upper edge of time window in ns for the inner layers */
    unsigned short m_tdcOffset; /**< Offset of TDC count (in ns)*/

    CDC::CDCGeometryPar* m_cdcp;  /**< Pointer to CDCGeometryPar */
    CDCSimHit* m_aCDCSimHit;    /**< Pointer to CDCSimHit */
    WireID m_wireID;            /**< WireID of this hit */
    unsigned short m_posFlag;   /**< left or right flag of this hit */
    TVector3 m_posWire;         /**< wire position of this hit */
    TVector3 m_posTrack;        /**< track position of this hit */
    TVector3 m_momentum;        /**< 3-momentum of this hit */
    double m_driftLength;       /**< drift length of this hit */
    double m_flightTime;        /**< flight time of this hit */
    double m_globalTime;        /**< global time of this hit */

    double m_tdcBinWidth;       /**< Width of a TDC bin (in ns)*/
    double m_tdcBinWidthInv;    /**< m_tdcBinWidth^-1 (in ns^-1)*/
    double m_tdcBinHwidth;      /**< Half width of a TDC bin (in ns)*/
    double m_tdcResol;          /**< TDC resolution (in ns)*/
    double m_driftV;            /**< Nominal drift velocity (in cm/ns)*/
    double m_driftVInv;         /**< m_driftV^-1 (in ns/cm)*/
    double m_propSpeedInv;      /**< Inv. of nominal signal propagation speed in a wire (in ns/cm)*/

    //--- Universal digitization parameters -------------------------------------------------------------------------------------
    bool m_doSmearing; /**< A switch to control drift length smearing */
    bool m_addInWirePropagationDelay; /**< A switch used to control adding propagation delay into the total drift time or not */
    bool m_addTimeOfFlight;     /**< A switch used to control adding time of flight into the total drift time or not */
    bool m_addInWirePropagationDelay4Bg; /**< A switch used to control adding propagation delay into the total drift time or not for beam bg. */
    bool m_addTimeOfFlight4Bg;     /**< A switch used to control adding time of flight into the total drift time or not for beam bg. */
    bool m_outputNegativeDriftTime;     /**< A switch to output negative drift time to CDCHit */
    bool m_misalign;             /**< A switch to control misalignment */
    bool m_correctForWireSag;    /**< A switch to control wire sag */
//    float m_eventTime;         /**< It is a timing of event, which includes a time jitter due to the trigger system */

    /** Structure for saving the signal information. */
    struct SignalInfo {
      /** Constructor that initializes all members. */
      SignalInfo(unsigned short simHitIndex = 0, float driftTime = 0, float charge = 0) :
        m_simHitIndex(simHitIndex), m_driftTime(driftTime), m_charge(charge) {}
      unsigned short m_simHitIndex;   /**< SimHit Index number. */
      float          m_driftTime;     /**< Shortest drift time of any SimHit in the cell. */
      float          m_charge;        /**< Sum of charge for all SimHits in the cell. */
    };
  };

} // end of Belle2 namespace

#endif // CDCDIGITIZER_H
