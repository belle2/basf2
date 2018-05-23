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
   *  There are multiple reasons for dead pixel:
   *  1) Brocken (open) drain line (-> current cannot flow into DCD)
   *  2) DEPFET current into DCD out of dynamic range
   *  3) DHP subtracts too high (255) pedestal correction
   *  4) ...
   *
   *  The subtraction of (programmable) very large pedestals on the DHP offers a method for
   *  online masking of problematic pixels. Pedestals masks are uploaded to the configDB.
   */

  class PXDDeadPixelPar: public TObject {
  public:
    /** Structure to hold set of dead channel indexed by their unique id (unsigned int), stored in hash table */
    typedef std::unordered_set< unsigned int> DeadChannelSet;

    /** Structure to hold set of dead rows indexed by their v cell ID (unsigned int), stored in hash table */
    //typedef std::unordered_set< unsigned int> DeadRowSet;

    /** Structure to hold set of dead drains indexed by their drain ID (unsigned int), stored in hash table */
    //typedef std::unordered_set< unsigned int> DeadDrainSet;

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
        m_MapSensors[sensorID] = true;
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

    /** Check whether a pixel on a given sensor is OK or not.
     * @param sensorID unique ID of the sensor
     * @param pixID unique ID of single pixel to mask
     * @return true if pixel is not dead, otherwise false.
     */
    bool pixelOK(unsigned short sensorID, unsigned int pixID) const
    {
      // Check if sensor is dead
      auto mapIterSensors = m_MapSensors.find(sensorID);
      if (mapIterSensors != m_MapSensors.end()) {
        return false;
      }

      // Check row is dead

      // Check drain is dead

      // Check single pixel is dead
      auto mapIterSingles = m_MapSingles.find(sensorID);
      if (mapIterSingles != m_MapSingles.end()) {
        // Found some masked single pixels on sensor
        auto& singles = mapIterSingles->second;
        // Look if this is a single masked pixel
        if (singles.find(pixID) != singles.end())
          return false;
      }
      // Pixel not found in the mask => pixel OK
      return true;
    }

    /** Return unordered_map with all dead single pixels in PXD. */
    const std::unordered_map<unsigned short, DeadChannelSet>& getDeadSinglePixelMap() const {return m_MapSingles;}

  private:



    /** Structure holding sets of dead single pixels for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, DeadChannelSet> m_MapSingles;

    /** Structure holding sets of dead rows for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, DeadChannelSet> m_MapRows;

    /** Structure holding sets of dead drains for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, DeadChannelSet> m_MapDrains;

    /** Structure holding flag to mask complete sensor by sensor id (unsigned short). */
    std::unordered_map<unsigned short, bool> m_MapSensors;

    ClassDef(PXDDeadPixelPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
