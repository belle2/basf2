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


namespace Belle2 {

  /** The payload collecting some meta information from
   *  running the masking algorithm.
   *
   *  This includes the following attributes:
   *  1) Number of events for computation
   *  2) Mean occupancy per sensor
   */

  class PXDOccupancyInfoPar: public TObject {
  public:

    /** Default constructor */
    PXDOccupancyInfoPar() : m_nEvents(0),  m_MapOccupancy() {}

    /** Set number of events used for occupancy estimation
     */
    void setNumberOfEvents(int nEvents) { m_nEvents = nEvents; }

    /** Get number of events used for occupancy estimation
     */
    int getNumberOfEvents() const { return m_nEvents; }

    /** Set occupancy
     *
     * @param sensorID unique ID of the sensor
     * @param occupancy calibrated hit occupancy
     */
    void setOccupancy(unsigned short sensorID, float occupancy)
    {
      m_MapOccupancy[sensorID] = occupancy;
    }

    /** Get occupancy
     *
     * @param sensorID unique ID of the sensor
     * @return calibrated occupancy, or -1 if sensor is not found
     */
    float getOccupancy(unsigned short sensorID) const
    {
      auto mapIter = m_MapOccupancy.find(sensorID);
      if (mapIter != m_MapOccupancy.end()) {
        return mapIter->second;
      }
      // Fallback when user asks for sensor that was not calibrated
      return -1;
    }

  private:

    /** Number of collected events used for occupancy computation, may decide to not trust calibrations with too few events */
    int m_nEvents;

    /** Map containing mean occupancy per sensor */
    std::unordered_map<unsigned short, float> m_MapOccupancy;

    ClassDef(PXDOccupancyInfoPar, 2);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
