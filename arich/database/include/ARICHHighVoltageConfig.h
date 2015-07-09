/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rok Pestotnik, Manca Mrvar                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TTimeStamp.h>
#include <string>

class ARICHCableInfo;

namespace Belle2 {
  /**
   *   Configuration parameters of the High Voltage Channel
   */

  class ARICHHighVoltageConfig: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHHighVoltageConfig(): m_id(0), m_vSet(0), m_vMax(0), m_iMax(0), m_rampUp(0), m_rampDown(0), m_timeStamp(0, 0, 0, kTRUE, 0) {};

    /**
     * Constructor
     */
    ARICHHighVoltageConfig(int id, double vSet, double vMax, double iMax, double rampU, double rampD, TTimeStamp timeStamp)
    {
      m_id = id;
      m_vSet = vSet;
      m_vMax = vMax;
      m_iMax = iMax;
      m_rampUp = rampU;
      m_rampDown = rampD;
      m_timeStamp = timeStamp;
    }

    /**
     * Destructor
     */
    ~ARICHHighVoltageConfig() {};

    /** Return Physical Identifier on the device
     * @return Physical Identifier on the device
     */
    int getHighVoltageID() const {return m_id; }

    /**
     * Set Physical Identifier on the device
     * @param Physical Identifier
     */
    void setHighVoltageID(int id) { m_id = id;}

    /** Return Cable Identifier
     * @return Cable Identifier
     */
    ARICHCableInfo getArichCable() const {return m_cable; }

    /** Set Cable Identifier
     * @param Cable Identifier
     */
    void setArichCable(ARICHCableInfo cable) {m_cable = cable; }

    /** Return High Voltage
     * @return High Voltage
     */
    double getHighVoltage() const {return m_vSet; }

    /**
     * Set High Voltage
     * @param High Voltage
     */
    void setHighVoltage(double hvSet) { m_vSet = hvSet;}

    /** Return Maximum High Voltage
     * @return Maximum High Voltage
     */
    double getMaximumHighVoltage() const {return m_vMax; }

    /** Set Maximum High Voltage
     * @param Maximum High Voltage
     */
    void setMaximumHighVoltage(double vMax) {m_vMax = vMax; }

    /** Return Maximum Current
     * @return Maximum Current
     */
    double getMaximumCurrent() const {return m_iMax; }

    /** Set Maximum Current
     * @param Maximum Current
     */
    void setMaximumCurrent(double iMax) {m_iMax = iMax; }

    /** Return Rate of the Voltage increase during ramp up
     * @return Rate during ramp up
     */
    double getRateDuringRampUp() const {return m_rampUp; }

    /**
     * Set Rate of the Voltage increase during ramp up
     * @param Rate during ramp up
     */
    void setRateDuringRampUp(double rampU) { m_rampUp = rampU;}

    /** Return Rate of the Voltage increase during ramp down
     * @return Rate during ramp up
     */
    double getRateDuringRampDown() const {return m_rampDown; }

    /**
     * Set Rate of the Voltage increase during ramp down
     * @param Rate during ramp down
     */
    void setRateDuringRampDown(double rampD) { m_rampDown = rampD;}

    /** Return Time of the constant creation
     * @return Time of the constant creation
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /** Set Time of the constant creation
     * @param Time of the constant creation
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

  private:
    int m_id;                     /**< Physical Identifier on the device */
    ARICHCableInfo m_cable;       /**< Physical Cable Identifier */
    double m_vSet;                /**< Set High Voltage */
    double m_vMax;                /**< Set Maximum High Voltage */
    double m_iMax;                /**< Set Maximum Current */
    double m_rampUp;              /**< Rate of the Voltage increase during ramp up */
    double m_rampDown;            /**< Rate of the Voltage increase during ramp down */
    TTimeStamp m_timeStamp;       /**< Time of the constant creation */

    ClassDef(ARICHHighVoltageConfig, 1);  /**< ClassDef */
  };
} // end namespace Belle2

