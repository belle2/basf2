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
#include <vector>
#include <TTimeStamp.h>
#include <arich/dbobjects/ARICHMergerInfo.h>
#include <arich/dbobjects/ARICHSensorModuleInfo.h>
#include <arich/dbobjects/ARICHCableInfo.h>

namespace Belle2 {
  /**
   *   Mapping of the Sensor Board Connections to the detector
   */

  class ARICHSensorModuleMap: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHSensorModuleMap(): m_sextant(0), m_ring(0), m_column(0), m_sensor(Belle2::ARICHSensorModuleInfo()),
      m_merger(Belle2::ARICHMergerInfo()), m_port(0), m_cableHv(Belle2::ARICHCableInfo()), m_cableGuard(Belle2::ARICHCableInfo()),
      m_timeStamp()
    {
      for (unsigned i = 0; i < 4; i++) m_cableBias[i] = Belle2::ARICHCableInfo();
    };

    /**
     * Constructor
     */
    ARICHSensorModuleMap(int sextant, int ring, int column, int port, ARICHCableInfo* cableBias,
                         TTimeStamp timeStamp): m_sextant(sextant), m_ring(ring), m_column(column), m_sensor(Belle2::ARICHSensorModuleInfo()),
      m_merger(Belle2::ARICHMergerInfo()), m_port(port), m_cableHv(Belle2::ARICHCableInfo()), m_cableGuard(Belle2::ARICHCableInfo()),
      m_timeStamp(timeStamp)
    {
      for (unsigned i = 0; i < 4; i++) m_cableBias[i] = cableBias[i];
    }

    /**
     * Constructor
     */
    ARICHSensorModuleMap(int sextant, int ring, int column, ARICHSensorModuleInfo sensor, ARICHMergerInfo merger, int port,
                         ARICHCableInfo cableHv, ARICHCableInfo* cableBias, ARICHCableInfo cableGuard, TTimeStamp timeStamp): m_sextant(sextant),
      m_ring(ring), m_column(column), m_sensor(sensor), m_merger(merger), m_port(port), m_cableHv(cableHv), m_cableGuard(cableGuard),
      m_timeStamp(timeStamp)
    {
      for (unsigned i = 0; i < 4; i++) m_cableBias[i] = cableBias[i];
    }

    /**
     * Destructor
     */
    ~ARICHSensorModuleMap() {};

    /**
      * Return SensorModule Sextant Identifier
      * @return SensorModule Sextant
      */
    int getSensorModuleSextantID() const {return m_sextant;}

    /**
     * Set SensorModule Sextant Identifier
     * @param Sextant Identifier
     */
    void setSensorModuleSextantID(int sextant) {m_ring = sextant;}

    /**
      * Return SensorModule Ring Identifier
      * @return SensorModule Ring
      */
    int getSensorModuleRingID() const {return m_ring;}

    /**
     * Set SensorModule Ring Identifier
     * @param Ring Identifier
     */
    void setSensorModuleRingID(int ring) {m_ring = ring;}

    /**
     * Return SensorModule Column Identifier
     * @return SensorModule Column
     */
    int getSensorModuleColumnID() const {return m_column;}

    /**
     * Set SensorModule Column Identifier
     * @param Column Identifier
     */
    void setSensorModuleColumnID(int column) {m_column = column;}

    /** Get Sensor Module Identifier
     * @return Sensor Module Identifier
     */
    ARICHSensorModuleInfo getSensorModuleId() const {return m_sensor; }

    /** Set Sensor Module Identifier
     * @param Sensor Module Identifier
     */
    void setSensorModuleId(ARICHSensorModuleInfo sensor) {m_sensor = sensor; }

    /** Get Merger Identifier
     * @return Merger Identifier
     */
    ARICHMergerInfo getMergerId() const {return m_merger; }

    /** Set Merger Identifier
     * @param Merger Identifier
     */
    void setMergerId(ARICHMergerInfo merger) {m_merger = merger; }

    /** Get Cable High Voltage Identifier
     * @return Cable High Voltage Identifier
     */
    ARICHCableInfo getCableHighVoltageId() const {return m_cableHv; }

    /** Set Cable High Voltage Identifier
     * @param Cable High Voltage Identifier
     */
    void setCableHighVoltageId(ARICHCableInfo cableHv) {m_cableHv = cableHv; }

    /** Get Cable Bias Voltage Identifier
     * @param i index of the chip
     * @return Cable Bias Voltage Identifier
     */
    ARICHCableInfo getCableBiasVoltageId(unsigned int i) {if (i < 4) return m_cableBias[i]; else return ARICHCableInfo(0, 0, 0); }

    /** Set Cable Bias Voltage Identifier
     * @param i index of the chip
     * @param Cable Bias Voltage Identifier
     */
    void setCableBiasVoltageId(unsigned int i, ARICHCableInfo* cableBias) {if (i < 4) m_cableBias[i] = cableBias[i]; }

    /** Get Cable Guard Identifier
     * @return Cable Identifier
     */
    ARICHCableInfo getCableGuard() const {return m_cableGuard; }

    /** Set Cable Guard Identifier
     * @param Cable Guard Identifier
     */
    void setCableGuard(ARICHCableInfo cableGuard) {m_cableGuard = cableGuard; }

    /** Get Installation date
     * @return Installation date
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /** Set Installation date
     * @param Installation date
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }


  private:
    int m_sextant;                   /**< Sextant identification */
    int m_ring;                      /**< Photon Detector Ring Identifier */
    int m_column;                    /**< Photon Detector Column_id Identifier */
    ARICHSensorModuleInfo m_sensor;  /**< Sensor Module identifier */
    ARICHMergerInfo       m_merger;  /**< Merger Board identifier  */
    int m_port;                      /**< Merger Board port identifier */
    ARICHCableInfo m_cableHv;        /**< HV Cable Identifier */
    ARICHCableInfo m_cableBias[4];   /**< Bias Cable Identifier */
    ARICHCableInfo m_cableGuard;     /**< Guard Cable Identifier */
    TTimeStamp m_timeStamp;          /**< Installation Date */

    ClassDef(ARICHSensorModuleMap, 1);  /**< ClassDef */
  };
} // end namespace Belle2

