/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TTimeStamp.h>
#include <arich/dbobjects/ARICHCableInfo.h>

namespace Belle2 {
  /**
   *   Configuration parameters of the High Voltage Channel
   */

  class ARICHHighVoltageConfig: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHHighVoltageConfig(): m_id(0), m_cable(Belle2::ARICHCableInfo()), m_vSet(0), m_vMax(0), m_iMax(0), m_rampUp(0), m_rampDown(0),
      m_timeStamp() {};

    /**
     * Constructor
     */
    ARICHHighVoltageConfig(int id, float vSet, float vMax, float iMax, float rampU, float rampD, TTimeStamp timeStamp): m_id(id),
      m_cable(Belle2::ARICHCableInfo()), m_vSet(vSet), m_vMax(vMax), m_iMax(iMax), m_rampUp(rampU), m_rampDown(rampD),
      m_timeStamp(timeStamp) {};

    /**
     * Constructor
     */
    ARICHHighVoltageConfig(int id, ARICHCableInfo cable, float vSet, float vMax, float iMax, float rampU, float rampD,
                           TTimeStamp timeStamp): m_id(id), m_cable(cable), m_vSet(vSet), m_vMax(vMax), m_iMax(iMax), m_rampUp(rampU), m_rampDown(rampD),
      m_timeStamp(timeStamp) {};

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
    float getHighVoltage() const {return m_vSet; }

    /**
     * Set High Voltage
     * @param High Voltage
     */
    void setHighVoltage(float hvSet) { m_vSet = hvSet;}

    /** Return Maximum High Voltage
     * @return Maximum High Voltage
     */
    float getMaximumHighVoltage() const {return m_vMax; }

    /** Set Maximum High Voltage
     * @param Maximum High Voltage
     */
    void setMaximumHighVoltage(float vMax) {m_vMax = vMax; }

    /** Return Maximum Current
     * @return Maximum Current
     */
    float getMaximumCurrent() const {return m_iMax; }

    /** Set Maximum Current
     * @param Maximum Current
     */
    void setMaximumCurrent(float iMax) {m_iMax = iMax; }

    /** Return Rate of the Voltage increase during ramp up
     * @return Rate during ramp up
     */
    float getRateDuringRampUp() const {return m_rampUp; }

    /**
     * Set Rate of the Voltage increase during ramp up
     * @param Rate during ramp up
     */
    void setRateDuringRampUp(float rampU) { m_rampUp = rampU;}

    /** Return Rate of the Voltage increase during ramp down
     * @return Rate during ramp up
     */
    float getRateDuringRampDown() const {return m_rampDown; }

    /**
     * Set Rate of the Voltage increase during ramp down
     * @param Rate during ramp down
     */
    void setRateDuringRampDown(float rampD) { m_rampDown = rampD;}

    /** Return Time of the constant creation
     * @return Time of the constant creation
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /** Set Time of the constant creation
     * @param Time of the constant creation
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

  private:
    int m_id;                  /**< Physical Identifier on the device */
    ARICHCableInfo m_cable;    /**< Physical Cable Identifier */
    float m_vSet;              /**< Set High Voltage */
    float m_vMax;              /**< Set Maximum High Voltage */
    float m_iMax;              /**< Set Maximum Current */
    float m_rampUp;            /**< Rate of the Voltage increase during ramp up */
    float m_rampDown;          /**< Rate of the Voltage increase during ramp down */
    TTimeStamp m_timeStamp;    /**< Time of the constant creation */

    ClassDef(ARICHHighVoltageConfig, 1);  /**< ClassDef */
  };
} // end namespace Belle2

