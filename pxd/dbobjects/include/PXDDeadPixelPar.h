/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <unordered_map>
#include <unordered_set>


namespace Belle2 {

  /** The payload telling which PXD pixel is dead (=Readout system does not receive signals)
   *
   *  There are multiple reasons for dead pixels:
   *  1) Brocken (open) drain line (-> current cannot flow into DCD channel)
   *  2) DEPFET current into DCD out of dynamic range
   *  3) DHP subtracts too high pedestal correction
   *  4) ...
   *
   *  Here, we cannot distinguish the true reason, but report that a pixel gets never hit
   *  during some IoV.
   */

  class PXDDeadPixelPar: public TObject {
  public:
    /** Structure to hold set of dead channel indexed by their unique id (unsigned int), stored in hash table */
    typedef std::unordered_set< unsigned int> DeadChannelSet;

    /** Default constructor */
    PXDDeadPixelPar() : m_MapSingles(), m_MapRows(), m_MapDrains(), m_MapSensors()  {}

    /** Mask sensor
     *
     * @param sensorID unique ID of the sensor
     */
    void maskSensor(unsigned short sensorID)
    {
      auto mapIter = m_MapSensors.find(sensorID);
      if (mapIter == m_MapSensors.end()) {
        // Sensor not already masked as dead. Mask it.
        m_MapSensors.insert(sensorID);
      }
    }

    /** Mask single drain
     *
     * @param sensorID unique ID of the sensor
     * @param drainID unique ID of single pixel to mask
     */
    void maskDrain(unsigned short sensorID, unsigned int drainID)
    {
      auto mapIter = m_MapDrains.find(sensorID);
      if (mapIter != m_MapDrains.end()) {
        // Already some masked drain on sensor
        auto& drains = mapIter->second;
        // Only add drain, if it is not already in
        if (drains.find(drainID) == drains.end())
          drains.insert(drainID);
      } else {
        // Create an empty set of masked drains
        PXDDeadPixelPar::DeadChannelSet drains;
        // drainID will be used to generate hash in unordered_set for quick access
        drains.insert(drainID);
        m_MapDrains[sensorID] = drains;
      }
    }

    /** Mask single row
     *
     * @param sensorID unique ID of the sensor
     * @param vCellID unique ID of row to mask
     */
    void maskRow(unsigned short sensorID, unsigned int vCellID)
    {
      auto mapIter = m_MapRows.find(sensorID);
      if (mapIter != m_MapRows.end()) {
        // Already some masked rows on sensor
        auto& rows = mapIter->second;
        // Only add row, if it is not already in
        if (rows.find(vCellID) == rows.end())
          rows.insert(vCellID);
      } else {
        // Create an empty set of masked rows
        PXDDeadPixelPar::DeadChannelSet rows;
        // vCellID will be used to generate hash in unordered_set for quick access
        rows.insert(vCellID);
        m_MapRows[sensorID] = rows;
      }
    }

    /** Mask single pixel
     *
     * @param sensorID unique ID of the sensor
     * @param pixID unique ID of single pixel to mask
     */
    void maskSinglePixel(unsigned short sensorID, unsigned int pixID)
    {
      auto mapIter = m_MapSingles.find(sensorID);
      if (mapIter != m_MapSingles.end()) {
        // Already some masked single pixels on sensor
        auto& singles = mapIter->second;
        // Only add pixel, if it is not already in
        if (singles.find(pixID) == singles.end())
          singles.insert(pixID);
      } else {
        // Create an empty set of masked single pixels
        PXDDeadPixelPar::DeadChannelSet singles;
        // pixID will be used to generate hash in unordered_set for quick access
        singles.insert(pixID);
        m_MapSingles[sensorID] = singles;
      }
    }

    /** Check whether a sensor is  dead.
     * @param sensorID unique ID of the sensor
     * @return true if sensor is dead
     */
    bool isDeadSensor(unsigned short sensorID) const
    {
      if (m_MapSensors.find(sensorID) != m_MapSensors.end()) {
        return false;
      }
      return true;
    }

    /** Check whether a row is dead.
     * @param sensorID unique ID of the sensor
     * @param vCellID unique ID of row
     * @return true if row is dead
     */
    bool isDeadRow(unsigned short sensorID, unsigned int vCellID) const
    {
      auto mapIter = m_MapRows.find(sensorID);
      if (mapIter != m_MapRows.end()) {
        // Found some dead rows
        auto& deadRows = mapIter->second;
        // Look if this row is dead
        if (deadRows.find(vCellID) != deadRows.end())
          return true;
      }
      return false;
    }

    /** Check whether a drain is dead.
     * @param sensorID unique ID of the sensor
     * @param drainID unique ID of drain
     * @return true if drain is dead
     */
    bool isDeadDrain(unsigned short sensorID, unsigned int drainID) const
    {
      auto mapIter = m_MapDrains.find(sensorID);
      if (mapIter != m_MapDrains.end()) {
        // Found some dead drains
        auto& deadDrains = mapIter->second;
        // Look if this drain is dead
        if (deadDrains.find(drainID) != deadDrains.end())
          return true;
      }
      return false;
    }

    /** Check whether a single pixel is dead.
     * @param sensorID unique ID of the sensor
     * @param pixID unique ID of pixel
     * @return true if pixel is dead
     */
    bool isDeadSinglePixel(unsigned short sensorID, unsigned int pixID) const
    {
      auto mapIter = m_MapSingles.find(sensorID);
      if (mapIter != m_MapSingles.end()) {
        // Found some dead singles
        auto& deadSingles = mapIter->second;
        // Look if this single is dead
        if (deadSingles.find(pixID) != deadSingles.end())
          return true;
      }
      return false;
    }

    /** Return unordered_map with all dead single pixels in PXD. */
    const std::unordered_map<unsigned short, DeadChannelSet>& getDeadSinglePixelMap() const {return m_MapSingles;}

    /** Return unordered_map with all dead rows in PXD. */
    const std::unordered_map<unsigned short, DeadChannelSet>& getDeadDrainMap() const {return m_MapDrains;}

    /** Return unordered_map with all dead drains in PXD. */
    const std::unordered_map<unsigned short, DeadChannelSet>& getDeadRowMap() const {return m_MapRows;}

    /** Return unordered_set with all dead sensors in PXD. */
    const std::unordered_set< unsigned int>& getDeadSensorMap() const {return m_MapSensors;}

  private:

    /** Structure holding sets of dead single pixels for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, DeadChannelSet> m_MapSingles;

    /** Structure holding sets of dead rows for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, DeadChannelSet> m_MapRows;

    /** Structure holding sets of dead drains for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, DeadChannelSet> m_MapDrains;

    /** Structure holding dead sensors by sensor id. */
    std::unordered_set< unsigned int> m_MapSensors;

    ClassDef(PXDDeadPixelPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
