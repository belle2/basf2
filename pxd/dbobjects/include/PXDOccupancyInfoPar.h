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
   *  2) Mean occupancy before masking (per sensor)
   *  3) Mean occupancy after masking (per sensor)
   */

  class PXDOccupancyInfoPar: public TObject {
  public:

    /** Default constructor */
    PXDOccupancyInfoPar() : m_nEvents(0),  m_MapOccupancy(), m_MapRawOccupancy() {}

    /** Set number of events used for occupancy estimation
     */
    void setNumberOfEvents(int nEvents) { m_nEvents = nEvents; }

    /** Get number of events used for occupancy estimation
     */
    int getNumberOfEvents() { return m_nEvents; }

    /** Get sensor occupancy map, after offline hot pixel masking
     */
    const std::unordered_map<unsigned short, float>& getOccupancyMap() const {return m_MapOccupancy;}

    /** Get sensor occupancy map, after offline hot pixel masking
     */
    std::unordered_map<unsigned short, float>& getOccupancyMap() {return m_MapOccupancy;}

    /** Get sensor occupancy map, before offline hot pixel masking
     */
    const std::unordered_map<unsigned short, float>& getRawOccupancyMap() const {return m_MapOccupancy;}

    /** Get sensor occupancy map, before offline hot pixel masking
     */
    std::unordered_map<unsigned short, float>& getRawOccupancyMap() {return m_MapOccupancy;}

  private:

    /** Number of collected events used for occupancy computation, may decide to not trust calibrations with too few events */
    int m_nEvents;

    /** Map containing mean occupancy per sensor after masking */
    std::unordered_map<unsigned short, float> m_MapOccupancy;

    /** Map containing mean occupancy per sensor before masking */
    std::unordered_map<unsigned short, float> m_MapRawOccupancy;

    ClassDef(PXDOccupancyInfoPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
