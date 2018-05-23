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
    PXDOccupancyInfoPar() : m_nEvents(0),  m_MapOccupancy(), m_MapOccupancyRaw() {}

  private:

    /** Number of events used for occupancy computation */
    int m_nEvents;

    /** Map containing mean occupancy per sensor after masking */
    std::unordered_map<unsigned short, float> m_MapOccupancy;

    /** Map containing mean occupancy per sensor before masking */
    std::unordered_map<unsigned short, float> m_MapOccupancyRaw;

    ClassDef(PXDOccupancyInfoPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
