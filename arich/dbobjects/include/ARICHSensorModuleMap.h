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
#include <arich/dbobjects/ARICHMergerInfo.h>
#include <arich/dbobjects/ARICHSensorModuleInfo.h>
#include <arich/dbobjects/ARICHCableInfo.h>

namespace Belle2 {

  /**
   *Mapping of the sensor-board connections to the detector.
   */
  class ARICHSensorModuleMap: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHSensorModuleMap(): m_sextant(0), m_ring(0), m_column(0), m_sensorID(0), m_sensor(NULL), m_merger(NULL), m_port(0),
      m_cableHv(NULL), m_cableGuard(NULL), m_timeStamp()
    {
      for (unsigned i = 0; i < 4; i++) m_cableBias[i] = NULL;
    };


    /**
     * Constructor.
     */
    ARICHSensorModuleMap(int sextant, int ring, int column, int sensorID, ARICHSensorModuleInfo* sensor, ARICHMergerInfo* merger,
                         int port,
                         ARICHCableInfo* cableHv, ARICHCableInfo** cableBias, ARICHCableInfo* cableGuard, TTimeStamp timeStamp): m_sextant(sextant),
      m_ring(ring), m_column(column), m_sensorID(sensorID), m_sensor(sensor), m_merger(merger), m_port(port), m_cableHv(cableHv),
      m_cableGuard(cableGuard),
      m_timeStamp(timeStamp)
    {
      for (unsigned i = 0; i < 4; i++) m_cableBias[i] = cableBias[i];
    }

    /**
     * Destructor.
     */
    ~ARICHSensorModuleMap() {};

    /**
     * Get sensor-module sextant identifier.
     * @return Sensor-module sextant.
     */
    int getSensorModuleSextantID() const {return m_sextant;}

    /**
     * Set sensor-module sextant identifier.
     * @param[in] sextant Sextant identifier.
     */
    void setSensorModuleSextantID(int sextant) {m_sextant = sextant;}

    /**
     * Get sensor-module ring identifier.
     * @return Sensor-module ring.
     */
    int getSensorModuleRingID() const {return m_ring;}

    /**
     * Set sensor-module ring identifier.
     * @param[in] ring Ring Identifier.
     */
    void setSensorModuleRingID(int ring) {m_ring = ring;}

    /**
     * Get sensor global identifier.
     * @return Global identifier.
     */
    int getSensorGlobalID() const {return m_sensorID;}

    /**
     * Set sensor global identifier.
     * @param[in] sensorID Global identifier
     */
    void setSensorGlobalID(int sensorID) {m_sensorID = sensorID;}

    /**
     * Get sensor-module column identifier.
     * @return Sensor-module column.
     */
    int getSensorModuleColumnID() const {return m_column;}

    /**
     * Set sensor-module column identifier.
     * @param[in] column Column identifier
     */
    void setSensorModuleColumnID(int column) {m_column = column;}

    /**
     * Get sensor-module identifier.
     * @return Sensor-module identifier.
     */
    ARICHSensorModuleInfo* getSensorModuleId() const {return m_sensor; }

    /**
     * Set sensor-module identifier.
     * @param[in] sensor Sensor-module identifier.
     */
    void setSensorModuleId(ARICHSensorModuleInfo* sensor) {m_sensor = sensor; }

    /**
     * Get merger identifier.
     * @return Merger identifier.
     */
    ARICHMergerInfo* getMergerId() const {return m_merger; }

    /**
     * Set merger identifier.
     * @param[in] merger Merger identifier.
     */
    void setMergerId(ARICHMergerInfo* merger) {m_merger = merger; }

    /**
     * Get high-voltage cable identifier.
     * @return High-voltage cable identifier.
     */
    ARICHCableInfo* getCableHighVoltageId() const {return m_cableHv; }

    /**
     * Set high-voltage cable identifier.
     * @param[in] cableHv High-voltage cable identifier.
     */
    void setCableHighVoltageId(ARICHCableInfo* cableHv) {m_cableHv = cableHv; }

    /**
     * Get bias-voltage cable identifier.
     * @param i Index of the chip.
     * @return Bias-voltage cable identifier.
     */
    ARICHCableInfo* getCableBiasVoltageId(unsigned int i) {if (i < 4) return m_cableBias[i]; else return NULL; }

    /**
     * Set bias-voltage cable identifier.
     * @param i Index of the chip.
     * @param[in] cableBias Bias-voltage cable identifier.
     */
    void setCableBiasVoltageId(unsigned int i, ARICHCableInfo** cableBias) {if (i < 4) m_cableBias[i] = cableBias[i]; }

    /**
     * Get guard-cable identifier.
     * @return Guard-cable identifier.
     */
    ARICHCableInfo* getCableGuard() const {return m_cableGuard; }

    /**
     * Set guard-cable identifier.
     * @param[in] cableGuard Guard-cable Identifier.
     */
    void setCableGuard(ARICHCableInfo* cableGuard) {m_cableGuard = cableGuard; }

    /**
     * Get installation date.
     * @return Installation date.
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /**
     * Set installation date.
     * @param[in] timeStamp Installation date.
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }


  private:
    int m_sextant;                     /**< Sextant identification */
    int m_ring;                        /**< Photon Detector Ring Identifier */
    int m_column;                      /**< Photon Detector Column_id Identifier */
    int m_sensorID;                    /**< Global ID number */
    ARICHSensorModuleInfo* m_sensor;   /**< Sensor Module identifier */
    ARICHMergerInfo*       m_merger;   /**< Merger Board identifier  */
    int m_port;                        /**< Merger Board port identifier */
    ARICHCableInfo* m_cableHv;         /**< HV Cable Identifier */
    ARICHCableInfo* m_cableBias[4];    /**< Bias Cable Identifier */
    ARICHCableInfo* m_cableGuard;      /**< Guard Cable Identifier */
    TTimeStamp m_timeStamp;            /**< Installation Date */

    ClassDef(ARICHSensorModuleMap, 2);  /**< ClassDef */
  };
} // end namespace Belle2

