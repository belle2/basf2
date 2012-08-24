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

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>

namespace Belle2 {

  /** @addtogroup CDC_modules
   *  @ingroup modules
   *  @{CDCDigitizerModule
   *  @todo CDCDigitizerModule: Event time jitter needs a different approach than the one, which is taken here. Instead of adding
   *        a constant to all events, there needs to be an event by event time jitter, most likely determined by
   *        the trigger time uncertainties. <br>
   *        If the definition of the time format in the CDCHit is changed, it needs to be changed here, too, and in the SimpleDriftTimeTranslator.
   *  @}
   */
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
    float smearDriftLength(float driftLength, float fraction, float mean1, float resolution1, float mean2, float resolution2);

    /** The method to get drift time based on drift length
     *
     *  In this method, X-T function will be used to calculate drift time.
     *
     *  @param driftLength The value of drift length.
     *  @param tof The value of time of flight.
     *  @param propLength The length that signal needs to propagation in the wire.
     *
     *  @return Drift time.
     *
     *  @todo implementation of non-cicular surfaces of constant drift time (in reverse).
     */
    float getDriftTime(float driftLength, float tof, float propLength);

    /** Charge to ADC Count converter. */
    unsigned short getADCCount(const float charge);

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

    //--- Universal digitization parameters -------------------------------------------------------------------------------------
    bool m_addInWirePropagationDelay; /**< A switch used to control adding propagation delay into the total drift time or not */
    bool m_addTimeOfFlight;     /**< A switch used to control adding time of flight into the total drift time or not */
//    float m_eventTime;         /**< It is a timing of event, which includes a time jitter due to the trigger system */

    /** Structure for saving the signal information. */
    struct SignalInfo {
      /** Constructor that initializes all members. */
      SignalInfo(unsigned short simHitIndex = 0, WireID wireID = WireID(), float driftTime = 0, float charge = 0) :
        m_simHitIndex(simHitIndex), m_wireID(wireID), m_driftTime(driftTime), m_charge(charge) {}
      unsigned short m_simHitIndex;   /**< SimHit Index number. */
      WireID         m_wireID;        /**< Wire Number object. */
      float          m_driftTime;     /**< Shortest drift time of any SimHit in the cell. */
      float          m_charge;        /**< Sum of charge for all SimHits in the cell. */
    };
  };

} // end of Belle2 namespace

#endif // CDCDIGITIZER_H
